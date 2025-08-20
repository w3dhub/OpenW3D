/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/pathaction.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/16/01 1:51p                                              $*
 *                                                                                             *
 *                    $Revision:: 18                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "pathaction.h"
#include "smartgameobj.h"
#include "soldier.h"
#include "doors.h"
#include "humanphys.h"
#include "obbox.h"
#include "Path.h"
#include "chunkio.h"
#include "saveload.h"


////////////////////////////////////////////////////////////////
//	Save/Load constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_GAMEOBJ_REF		= 0x11020954,
	CHUNKID_VARIABLES
};

enum
{
	VARID_ELEVATOR_STATE		= 0,
	VARID_DOOR_STATE,
	VARID_STATE,
	VARID_TYPE,
	VARID_PATH_PTR,
	VARID_MECHANISM_PTR,
	VARID_DESTINATION,
	VARID_FACEPOS,
	VARID_GAME_OBJ_PTR,
	VARID_LADDER_STATE,
	VARID_LADDER_INDEX
};


////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
DynamicVectorClass<GameObjReference>	PathActionClass::LadderList;


////////////////////////////////////////////////////////////////////////////////////////////
//
//	PathActionClass
//
////////////////////////////////////////////////////////////////////////////////////////////
PathActionClass::PathActionClass (void) :
	ElevatorState (ELEVATOR_STATE_NONE),
	DoorState (DOOR_STATE_NONE),
	LadderState (LADDER_STATE_NONE),
	State (STATE_FINISHED),
	Type (TYPE_UNKNOWN),
	Mechanism (NULL),
	GameObj (NULL),
	Path (NULL),
	LadderIndex (-1),
	Destination (0, 0, 0),
	FacePos (0, 0, 0),
	Timer (0)
{
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	~PathActionClass
//
////////////////////////////////////////////////////////////////////////////////////////////
PathActionClass::~PathActionClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////////////////////////////////
void
PathActionClass::Initialize
(
	TYPE					type,
	PathClass *			path,
	SmartGameObj *		game_obj,
	StaticPhysClass *	mechanism
)
{
	Type			= type;
	GameObj		= game_obj;
	Mechanism	= mechanism;
	Path			= path;

	//
	//	Reset the states
	//
	State				= STATE_WAITING;
	ElevatorState	= ELEVATOR_STATE_WAITING;
	DoorState		= DOOR_STATE_GETTING_IN_POSITION;
	LadderState		= LADDER_STATE_WAITING;
	Timer				= 5;
	assert(Path != NULL);
	assert(Path->Get_Path_Vector_Length() >= Path->Get_Path_Vector_Count());
	Path->Get_Action_Entrance (Destination);
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Process
//
////////////////////////////////////////////////////////////////////////////////////////////
bool
PathActionClass::Process (void)
{
	switch (Type)
	{
		case TYPE_JUMPING:
			Handle_Jump ();
			break;

		case TYPE_LADDER:
			Handle_Ladder ();
			break;

		case TYPE_ELEVATOR:
			Handle_Elevator ();
			break;

		case TYPE_DOOR:
			Handle_Door ();
			break;

		default:
			Set_Finished ();
			break;
	}

	return (State == STATE_FINISHED);
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Handle_Ladder
//
////////////////////////////////////////////////////////////////////////////////////////////
void
PathActionClass::Handle_Ladder (void)
{
	SoldierGameObj *soldier = GameObj->As_SoldierGameObj ();
	WWASSERT (soldier != NULL);

	switch (LadderState)
	{
		case LADDER_STATE_WAITING:
		{
			//
			//	Is there already someone on the ladder?
			//
			if (Get_Ladder_Occupant (LadderIndex) == NULL) {
				Set_Ladder_Occupant (LadderIndex, GameObj);

				//
				//	Get on the ladder
				//
				soldier->Set_Boolean_Control (ControlClass::BOOLEAN_ACTION);
				LadderState = LADDER_STATE_GETTING_ON;
			}

			break;
		}

		case LADDER_STATE_GETTING_ON:

			//
			//	Have we gotten on yet?
			//
			if (soldier->Is_On_Ladder ()) {

				//
				//	Start climbing
				//
				State			= STATE_MOVING;
				LadderState	= LADDER_STATE_CLIMBING;
				assert(Path != NULL);
				assert(Path->Get_Path_Vector_Length() > Path->Get_Path_Vector_Count());
				Path->Get_Action_Destination (Destination);
				Path->Set_Movement_Directions (PathClass::MOVE_Z);
			} else {
				soldier->Set_Boolean_Control (ControlClass::BOOLEAN_ACTION);
			}

			break;

		case LADDER_STATE_CLIMBING:

			//
			//	Have we finished climbing yet?
			//
			if (soldier->Is_On_Ladder () == false) {
				LadderState	= LADDER_STATE_NONE;
				Set_Ladder_Occupant (LadderIndex, NULL);
				Set_Finished ();
			} else {

				//
				//	Get the object's current position
				//
				Vector3 curr_pos;
				GameObj->Get_Position (&curr_pos);

				//
				//	Is the object within a small distance of the destination?
				//
				float dist = WWMath::Fabs(curr_pos.Z - Destination.Z);
				if (dist < 0.25F) {

					//
					//	Tell the object to get off the ladder (he may not be able to
					// if someone is blocking his way)
					//
					soldier->Set_Boolean_Control (ControlClass::BOOLEAN_ACTION);
					State	= STATE_WAITING;
				}
			}

			break;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Handle_Jump
//
////////////////////////////////////////////////////////////////////////////////////////////
void
PathActionClass::Handle_Jump (void)
{
	//
	// Is this a human?
	//
	SoldierGameObj *soldier_obj = GameObj->As_SoldierGameObj ();
	if (soldier_obj != NULL) {

		HumanPhysClass *human_phys = soldier_obj->Peek_Physical_Object ()->As_HumanPhysClass ();
		if (human_phys != NULL) {

			//
			//	If we are still in the air, then turn to face
			// our destination point.
			//
			if (soldier_obj->Is_Airborne ()) {
				Vector3 dest(0,0,0);
				assert(Path != NULL);
				assert(Path->Get_Path_Vector_Length() > Path->Get_Path_Vector_Count());
				Path->Get_Action_Destination (dest);

				Vector3 face_pt = dest + Vector3 (0, 0, soldier_obj->Get_Bullseye_Offset_Z() );
				soldier_obj->Set_Targeting (face_pt, false);
			}

			//
			//	Revert when we've finished landing
			//
			if (	human_phys->Has_Just_Jumped () == false &&
					soldier_obj->Get_Human_State ()->Get_State () == HumanStateClass::UPRIGHT)
			{
				Set_Finished ();
			}
		}

	} else {
		Set_Finished ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Handle_Elevator
//
////////////////////////////////////////////////////////////////////////////////////////////
void
PathActionClass::Handle_Elevator (void)
{
	ElevatorPhysClass *elevator = Mechanism->As_ElevatorPhysClass ();
	WWASSERT (elevator != NULL);

	switch (ElevatorState)
	{
		case ELEVATOR_STATE_WAITING:

			//
			//	Switch states if the elevator has arrived
			//
			if (elevator->Can_Object_Enter (GameObj)) {
				elevator->Set_Current_Rider (GameObj);
				ElevatorState	= ELEVATOR_STATE_ENTERING;
				State				= STATE_MOVING;

				//
				//	Calculate where we want the unit to walk to in order
				// to enter the elevator
				//
				if (elevator->Get_Floor () == 0) {
					Get_Elevator_Zone_Pos (ZONE_LOWER_INSIDE, &Destination);
					Get_Elevator_Zone_Pos (ZONE_UPPER_CALL, &FacePos);
				} else {
					Get_Elevator_Zone_Pos (ZONE_UPPER_INSIDE, &Destination);
					Get_Elevator_Zone_Pos (ZONE_LOWER_CALL, &FacePos);
				}
			} else {
				elevator->Request_Elevator (GameObj);
			}

			break;

		case ELEVATOR_STATE_RIDING:

			//
			//	Switch states if the elevator has arrived
			//
			if (elevator->Can_Object_Exit (GameObj)) {
				ElevatorState	= ELEVATOR_STATE_EXITING;
				State				= STATE_MOVING;
				Timer				= 5.0F;
				assert(Path != NULL);
				assert(Path->Get_Path_Vector_Length() > Path->Get_Path_Vector_Count());
				Path->Get_Action_Destination (Destination);
			} else {

				//
				//	Make the unit face the exit
				//
				GameObj->Set_Targeting (FacePos, false);
			}

			break;

		case ELEVATOR_STATE_ENTERING:

			//
			//	Switch states if the elevator has started moving
			//
			if (elevator->Is_Moving ()) {
				ElevatorState	= ELEVATOR_STATE_RIDING;
				State				= STATE_WAITING;
			} else if (Has_Arrived ()) {

				//
				//	Make the unit face the exit
				//
				GameObj->Set_Targeting (FacePos, false);
				State	= STATE_WAITING;

				//
				//	Make the elevator start moving
				//
				elevator->Request_Elevator (GameObj);
			}

			break;

		case ELEVATOR_STATE_EXITING:

			//
			//	Has the unit made it through the elevator door, or has time expired?
			//
			Timer -= TimeManager::Get_Frame_Seconds ();
			if (Has_Arrived () || Timer <= 0) {

				//
				//	Let the elevator know that its rider has left
				//
				if (elevator->Get_Current_Rider () == GameObj) {
					elevator->Set_Current_Rider (NULL);
				}

				//
				//	Reset the state
				//
				ElevatorState = ELEVATOR_STATE_NONE;
				Set_Finished ();
			}
			break;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Handle_Door
//
////////////////////////////////////////////////////////////////////////////////////////////
void
PathActionClass::Handle_Door (void)
{
	DoorPhysClass *door = Mechanism->As_DoorPhysClass ();
	WWASSERT (door != NULL);

	switch (DoorState)
	{
		case DOOR_STATE_GETTING_IN_POSITION:
		{
			//
			//	Has the unit made it into position, or has time expired?
			//
			Timer -= TimeManager::Get_Frame_Seconds ();
			if (Has_Arrived () || Timer <= 0) {
				DoorState	= DOOR_STATE_WAITING;
				State			= STATE_WAITING;
			} else {
				State	= STATE_MOVING;
			}
			break;
		}

		case DOOR_STATE_WAITING:

			//
			//	Keep the door open for as long as we need it
			//
			door->Request_Door_Open ();

			//
			//	If the door is opened, then move onto the next state
			//
			if (door->Is_Door_Open ()) {
				DoorState	= DOOR_STATE_ENTERING;
				State			= STATE_MOVING;
				Timer			= 5.0F;
				assert(Path != NULL);
				assert(Path->Get_Path_Vector_Length() >= Path->Get_Path_Vector_Count());
				Path->Get_Action_Destination (Destination);
			}

			break;

		case DOOR_STATE_ENTERING:

			//
			//	Has the unit made it through the door, or has time expired?
			//
			Timer -= TimeManager::Get_Frame_Seconds ();
			if (Has_Arrived () || Timer <= 0) {
				DoorState = DOOR_STATE_NONE;
				Set_Finished ();
			}
			break;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Has_Arrived
//
////////////////////////////////////////////////////////////////////////////////////////////
bool
PathActionClass::Has_Arrived (void)
{
	bool retval = false;

	//
	//	Get the object's current position
	//
	Vector3 curr_pos;
	GameObj->Get_Position (&curr_pos);

	//
	//	Is the object within a small distance of the destination?
	//
	Vector3 delta	= curr_pos - Destination;
	delta.Z			= 0;
	float dist2 = delta.Length2 ();
	if (dist2 < 0.0225F) {
		retval = true;
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Elevator_Zone_Pos
//
////////////////////////////////////////////////////////////////////////////////////////////
void
PathActionClass::Get_Elevator_Zone_Pos (ELEVATOR_ZONE zone_id, Vector3 *position)
{
	WWASSERT (position != NULL);

	//
	//	Make sure we have an elevator
	//
	ElevatorPhysClass *elevator = Mechanism->As_ElevatorPhysClass ();
	WWASSERT (elevator != NULL);

	//
	//	Get information about this type of elevator
	//
	const ElevatorPhysDefClass *definition = elevator->Get_ElevatorPhysDef ();

	//
	//	Transform the requested zone into world-space
	//
	const Matrix3D &tm = elevator->Get_Transform ();
	OBBoxClass zone_box;
	OBBoxClass::Transform (tm, definition->Get_Zone (zone_id), &zone_box);

	//
	//	Return the center of the zone to the caller
	//
	(*position) = zone_box.Center;
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////////////////////////////////
void
PathActionClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_VARIABLES);

		//
		//	Save each variable to its own microchunk
		//
		WRITE_MICRO_CHUNK (csave, VARID_ELEVATOR_STATE,	ElevatorState);
		WRITE_MICRO_CHUNK (csave, VARID_DOOR_STATE,		DoorState);
		WRITE_MICRO_CHUNK (csave, VARID_STATE,				State);
		WRITE_MICRO_CHUNK (csave, VARID_TYPE,				Type);
		WRITE_MICRO_CHUNK_PTR (csave, VARID_PATH_PTR,			Path);
		WRITE_MICRO_CHUNK_PTR (csave, VARID_MECHANISM_PTR,	Mechanism);
		WRITE_MICRO_CHUNK (csave, VARID_DESTINATION,		Destination);
		WRITE_MICRO_CHUNK (csave, VARID_FACEPOS,			FacePos);
		WRITE_MICRO_CHUNK_PTR (csave, VARID_GAME_OBJ_PTR,	GameObj);
		WRITE_MICRO_CHUNK (csave, VARID_LADDER_STATE,	LadderState);
		WRITE_MICRO_CHUNK (csave, VARID_LADDER_INDEX,	LadderIndex);

	csave.End_Chunk ();

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////////////////////////////////
void
PathActionClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////
void
PathActionClass::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK (cload, VARID_ELEVATOR_STATE,	ElevatorState);
			READ_MICRO_CHUNK (cload, VARID_DOOR_STATE,		DoorState);
			READ_MICRO_CHUNK (cload, VARID_STATE,				State);
			READ_MICRO_CHUNK (cload, VARID_TYPE,				Type);
			READ_MICRO_CHUNK_PTR (cload, VARID_PATH_PTR,			Path);
			READ_MICRO_CHUNK_PTR (cload, VARID_MECHANISM_PTR,	Mechanism);
			READ_MICRO_CHUNK (cload, VARID_DESTINATION,		Destination);
			READ_MICRO_CHUNK (cload, VARID_FACEPOS,			FacePos);
			READ_MICRO_CHUNK_PTR (cload, VARID_GAME_OBJ_PTR,		GameObj);
			READ_MICRO_CHUNK (cload, VARID_LADDER_STATE,		LadderState);
			READ_MICRO_CHUNK (cload, VARID_LADDER_INDEX,		LadderIndex);
		}

		cload.Close_Micro_Chunk ();
	}

	//
	//	Request that the mechanism ptr gets remapped
	//
	if (GameObj != NULL) {
		REQUEST_POINTER_REMAP ((void **)&GameObj);
	}

	//
	//	Request that the mechanism ptr gets remapped
	//
	if (Mechanism != NULL) {
		REQUEST_REF_COUNTED_POINTER_REMAP ((RefCountClass **)&Mechanism);
	}

	//
	//	Request that the path ptr gets remapped
	//
	if (Path != NULL) {
		REQUEST_REF_COUNTED_POINTER_REMAP ((RefCountClass **)&Path);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Set_Ladder_Occupant
//
///////////////////////////////////////////////////////////////////////
void
PathActionClass::Set_Ladder_Occupant (int ladder_index, ScriptableGameObj	*object)
{
	WWASSERT (ladder_index < 256);
	if (ladder_index < 0) {
		return ;
	}

	//
	//	Grow the ladder list until it contains enough entries
	//
	while (ladder_index >= LadderList.Count ()) {
		LadderList.Add (NULL);
	}

	//
	//	Fill in this slot in the list
	//
	LadderList[ladder_index] = object;
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Ladder_Occupant
//
///////////////////////////////////////////////////////////////////////
ScriptableGameObj	*
PathActionClass::Get_Ladder_Occupant (int ladder_index)
{
	ScriptableGameObj	*object = NULL;

	//
	//	Return the object that's in this slot
	//
	if (ladder_index >= 0 && ladder_index < LadderList.Count ()) {
		object = LadderList[ladder_index];
	}

	return object;
}


///////////////////////////////////////////////////////////////////////
//
//	Reset
//
///////////////////////////////////////////////////////////////////////
void
PathActionClass::Reset (void)
{
	if (GameObj != NULL) {
		
		//
		//	Check to see if this unit is "reserving" any of the ladders...
		//
		for (int index = 0; index < LadderList.Count (); index ++) {
			if (LadderList[index] == GameObj) {
				
				//
				//	Clear the ladder reservation
				//
				LadderList[index] = NULL;
				break;
			}
		}
	}

	GameObj		= NULL;
	Mechanism	= NULL;
	Path			= NULL;
	State			= STATE_FINISHED;
	Type			= TYPE_UNKNOWN;
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Set_Finished
//
///////////////////////////////////////////////////////////////////////
void
PathActionClass::Set_Finished (void)
{
	State = STATE_FINISHED;
	Reset ();
	return ;
}