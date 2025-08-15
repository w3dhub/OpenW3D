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
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Combat/airstripgameobj.cpp                                                                                                                                      $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 2/24/02 2:06p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 19                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "airstripgameobj.h"
#include "basecontroller.h"
#include "vehicle.h"
#include "wwhack.h"
#include "simpledefinitionfactory.h"
#include "persistfactory.h"
#include "definitionmgr.h"
#include "combatchunkid.h"
#include "debug.h"
#include "scriptzone.h"
#include "wwprofile.h"
#include "basecontroller.h"
#include "combatchunkid.h"
#include "objlibrary.h"
#include "cinematicgameobj.h"
#include "combat.h"


////////////////////////////////////////////////////////////////
//	Hacks
////////////////////////////////////////////////////////////////
DECLARE_FORCE_LINK (AirStrip)

////////////////////////////////////////////////////////////////
//	Editable and persist factories
////////////////////////////////////////////////////////////////
SimplePersistFactoryClass	<AirStripGameObjDef,	CHUNKID_GAME_OBJECT_DEF_AIRSTRIP>					_AirStripGameObjDefPersistFactory;
SimplePersistFactoryClass	<AirStripGameObj,		CHUNKID_GAME_OBJECT_AIRSTRIP>							_AirStripGameObjPersistFactory;
DECLARE_DEFINITION_FACTORY (AirStripGameObjDef,	CLASSID_GAME_OBJECT_DEF_AIRSTRIP, "Airstrip")	_AirStripGameObjDefDefFactory;


////////////////////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////////////////////
static const float UNITIALIZED_TIMER	= -100.0F;
static const float VEHICLE_LOCK_TIME	= 30.0f;

////////////////////////////////////////////////////////////////
//	Save/Load constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_DEF_PARENT										=	0x02200638,
	CHUNKID_DEF_VARIABLES,

	MICROCHUNKID_DEF_CINEMATIC_DEFID						= 1,
	MICROCHUNKID_DEF_CINEMATIC_LENGTH_TO_DROPOFF,
	MICROCHUNKID_DEF_CINEMATIC_SLOT_INDEX,
	MICROCHUNKID_DEF_DISPLAY_VEHICLE_TIME
};

enum
{
	CHUNKID_PARENT								=	0x0219043,
	CHUNKID_VARIABLES,

	MICROCHUNKID_UNUSED						= 1,
};


////////////////////////////////////////////////////////////////
//
//	AirStripGameObjDef
//
////////////////////////////////////////////////////////////////
AirStripGameObjDef::AirStripGameObjDef (void)	:	
	CinematicLengthToDropOff (0),
	CinematicLengthToVehicleDisplay (0),
	CinematicDefID (0),
	CinematicSlotIndex (0)
{
	//
	//	Editable support
	//		
	EDITABLE_PARAM (AirStripGameObjDef, ParameterClass::TYPE_FLOAT,		CinematicLengthToDropOff);
	EDITABLE_PARAM (AirStripGameObjDef, ParameterClass::TYPE_INT,			CinematicSlotIndex);
	EDITABLE_PARAM (AirStripGameObjDef, ParameterClass::TYPE_FLOAT,		CinematicLengthToVehicleDisplay);	

	#ifdef PARAM_EDITING_ON
		GenericDefParameterClass *param = new GenericDefParameterClass (&CinematicDefID);
		param->Set_Class_ID (CLASSID_GAME_OBJECT_DEF_SIMPLE);
		param->Set_Name ("Drop-Off Cinematic");
		GENERIC_EDITABLE_PARAM (AirStripGameObjDef, param)
	#endif //PARAM_EDITING_ON

	return ;
}


////////////////////////////////////////////////////////////////
//
//	~AirStripGameObjDef
//
////////////////////////////////////////////////////////////////
AirStripGameObjDef::~AirStripGameObjDef (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Class_ID
//
////////////////////////////////////////////////////////////////
uint32
AirStripGameObjDef::Get_Class_ID (void) const	
{ 
	return CLASSID_GAME_OBJECT_DEF_AIRSTRIP;
}


////////////////////////////////////////////////////////////////
//
//	Create
//
////////////////////////////////////////////////////////////////
PersistClass *
AirStripGameObjDef::Create (void) const 
{
	AirStripGameObj *building = new AirStripGameObj;
	building->Init (*this);

	return building;
}


////////////////////////////////////////////////////////////////
//
//	Create
//
////////////////////////////////////////////////////////////////
bool
AirStripGameObjDef::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_DEF_PARENT);		
		VehicleFactoryGameObjDef::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_DEF_VARIABLES);

		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_CINEMATIC_LENGTH_TO_DROPOFF,	CinematicLengthToDropOff);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_CINEMATIC_DEFID,					CinematicDefID);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_CINEMATIC_SLOT_INDEX,			CinematicSlotIndex);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_DISPLAY_VEHICLE_TIME,			CinematicLengthToVehicleDisplay);

	csave.End_Chunk ();

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////
bool
AirStripGameObjDef::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ())
	{
		switch (cload.Cur_Chunk_ID ())
		{
			case CHUNKID_DEF_PARENT:
				VehicleFactoryGameObjDef::Load (cload);
				break;

			case CHUNKID_DEF_VARIABLES:
				Load_Variables (cload);
				break;
	  
			default:
				Debug_Say (("Unrecognized AirStrip Def chunkID\n"));
				break;

		}
		cload.Close_Chunk ();
	}

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
////////////////////////////////////////////////////////////////
void
AirStripGameObjDef::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {

		switch (cload.Cur_Micro_Chunk_ID ())
		{
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_CINEMATIC_LENGTH_TO_DROPOFF,	CinematicLengthToDropOff);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_CINEMATIC_DEFID,					CinematicDefID);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_CINEMATIC_SLOT_INDEX,			CinematicSlotIndex);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_DISPLAY_VEHICLE_TIME,			CinematicLengthToVehicleDisplay);
		
			default:
				Debug_Say (("Unrecognized AirStrip Def Variable chunkID\n"));
				break;
		}

		cload.Close_Micro_Chunk();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
AirStripGameObjDef::Get_Factory (void) const 
{ 
	return _AirStripGameObjDefPersistFactory; 
}


////////////////////////////////////////////////////////////////
//
//	AirStripGameObj
//
////////////////////////////////////////////////////////////////
AirStripGameObj::AirStripGameObj (void)	:
	CinematicStartTimer (0),
	ClearDropoffZoneTimer (UNITIALIZED_TIMER),
	DisplayVehicleTimer (UNITIALIZED_TIMER),
	IsCinematicStarted (false),
	CinematicObject (NULL)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~AirStripGameObj
//
////////////////////////////////////////////////////////////////
AirStripGameObj::~AirStripGameObj (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
AirStripGameObj::Get_Factory (void) const 
{
	return _AirStripGameObjPersistFactory;
}


////////////////////////////////////////////////////////////////
//
//	Init
//
////////////////////////////////////////////////////////////////
void AirStripGameObj::Init( void )
{
	Init( Get_Definition() );
}

////////////////////////////////////////////////////////////////
//
//	Init
//
////////////////////////////////////////////////////////////////
void
AirStripGameObj::Init (const AirStripGameObjDef &definition)
{
	VehicleFactoryGameObj::Init (definition);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Definition
//
////////////////////////////////////////////////////////////////
const AirStripGameObjDef &
AirStripGameObj::Get_Definition (void) const
{
	return (const AirStripGameObjDef &)BaseGameObj::Get_Definition ();
}


////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////
bool
AirStripGameObj::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_PARENT);
		VehicleFactoryGameObj::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);
	csave.End_Chunk ();
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////
bool
AirStripGameObj::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_PARENT:
				VehicleFactoryGameObj::Load (cload);
				break;
								
			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;

			default:
				Debug_Say (("Unrecognized AirStrip chunkID\n"));
				break;
		}

		cload.Close_Chunk();
	}

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
////////////////////////////////////////////////////////////////
void
AirStripGameObj::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {
		
		/*switch (cload.Cur_Micro_Chunk_ID ())
		{
			default:
				Debug_Say (("Unrecognized AirStrip Variable chunkID\n"));
				break;
		}*/

		cload.Close_Micro_Chunk();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	CnC_Initialize
//
////////////////////////////////////////////////////////////////
void
AirStripGameObj::CnC_Initialize (BaseControllerClass *base)
{
	VehicleFactoryGameObj::CnC_Initialize (base);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Think
//
////////////////////////////////////////////////////////////////
void
AirStripGameObj::Think (void)
{
	WWPROFILE ("AirStrip Think");

	//
	//	Begin the cinematic if necessary
	//
	if (IsDestroyed == false && GeneratingVehicleID != 0) {
		
		//
		//	Start the cinematic (if necessary)
		//
		if (IsCinematicStarted == false) {
			CinematicStartTimer -= TimeManager::Get_Frame_Seconds ();
			if (CinematicStartTimer <= 0) {
				Start_Cinematic ();
				ClearDropoffZoneTimer = 0.8f * Get_Definition().CinematicLengthToDropOff;
			}
		} 
	}
	
	if ((GeneratingVehicleID != 0) && (IsCinematicStarted)) {

		//
		//	Check to see if its time to display the object or not
		//
		if (DisplayVehicleTimer > UNITIALIZED_TIMER) {
			DisplayVehicleTimer -= TimeManager::Get_Frame_Seconds ();
			if (DisplayVehicleTimer < 0) {
				
				//
				//	Display the vehicle
				//
				if (Vehicle != NULL) {
					PhysicalGameObj *physical_obj = Vehicle.Get_Ptr ()->As_PhysicalGameObj ();
					physical_obj->Peek_Model ()->Set_Hidden (false);
					physical_obj->Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);

				}
				DisplayVehicleTimer = UNITIALIZED_TIMER;

				// Tell the vehicle to drive to one of the delivery points
				Deliver_Vehicle();
			}
		}

		//
		// Check if we need to clear the dropoff zone
		//
		if (ClearDropoffZoneTimer > UNITIALIZED_TIMER) {
			ClearDropoffZoneTimer -= TimeManager::Get_Frame_Seconds ();
			if (ClearDropoffZoneTimer < 0) {
				Destroy_Blocking_Objects ();
				ClearDropoffZoneTimer = UNITIALIZED_TIMER;
			}
		}
	}

	VehicleFactoryGameObj::Think ();	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Begin_Generation
//
////////////////////////////////////////////////////////////////
void
AirStripGameObj::Begin_Generation (void)
{
	CinematicStartTimer	= GenerationTime - Get_Definition ().CinematicLengthToDropOff;
	IsCinematicStarted	= false;	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Start_Cinematic
//
////////////////////////////////////////////////////////////////
void
AirStripGameObj::Start_Cinematic (void)
{
	if (CombatManager::I_Am_Server () == false) {
		return ;
	}	

	//
	//	Create the vehicle
	//
	VehicleGameObj *vehicle = Create_Vehicle ();
	if (vehicle != NULL) {
		
		//
		//	Hide the vehicle until later
		//
		if (vehicle->Peek_Model () != NULL) {
			vehicle->Peek_Model ()->Set_Hidden (true);
			vehicle->Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
		}

		//
		// Lock the vehicle
		//
		if (Purchaser != NULL) {
			vehicle->Lock_Vehicle(Purchaser,VEHICLE_LOCK_TIME);
		}

		//
		//	Create the cinematic controller
		//
		CinematicObject = ObjectLibraryManager::Create_Object (Get_Definition ().CinematicDefID);
		if (CinematicObject != NULL) {
			CinematicObject->Start_Observers ();

			//
			//	Position the cinematic controller in the world
			//
			Matrix3D test_tm = CreationTM;
			CinematicObject->Set_Transform (test_tm);

			//
			//	Try to find the script parser so we can communicate with it
			//
			const GameObjObserverList &script_list = CinematicObject->Get_Observers ();
			for (int index = 0; index < script_list.Count (); index ++) {
				GameObjObserverClass *script = script_list[index];
				if (::stricmp (script->Get_Name (), "Test_Cinematic") == 0) {
					
					//
					//	Attach the vehicle to the given slot in the cinematic
					//
					int slot = 10000 + Get_Definition ().CinematicSlotIndex;
					script->Custom (this, slot, vehicle->Get_ID (), this);
					break;
				}
			}
		}
	}

	IsCinematicStarted = true;
	DisplayVehicleTimer = Get_Definition ().CinematicLengthToVehicleDisplay;
	return ;
}

