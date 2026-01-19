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
 *                     $Archive:: /Commando/Code/Combat/staticnetworkobject.cpp               $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 1/15/02 1:02p                                               $*
 *                                                                                             *
 *                    $Revision:: 15                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "staticnetworkobject.h"
#include "staticanimphys.h"
#include "doors.h"
#include "elevator.h"
#include "bitpackids.h"
#include "combat.h"
#include "damageablestaticphys.h"
#include "explosion.h"
#include "vistable.h"
#include "networkobjectmgr.h"
#include "apppackettypes.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
DynamicVectorClass<StaticNetworkObjectClass *> StaticNetworkObjectClass::StaticNetworkObjectList;


////////////////////////////////////////////////////////////////
//
//	StaticNetworkObjectClass
//
////////////////////////////////////////////////////////////////
StaticNetworkObjectClass::StaticNetworkObjectClass (void)	:
	PhysObj (NULL),
	AnimationMode (0),
	LoopStart (0),
	LoopEnd (0),
	CurrFrame (0),
	TargetFrame (0)
{
	//
	//	Add ourselves to the master list
	//
	StaticNetworkObjectList.Add (this);

	//
	//	Make sure we don't try to create the object on the client
	//
	//TSS2001d Clear_Object_Dirty_Bits ();
	Set_Object_Dirty_Bit (BIT_RARE, true);

	Set_App_Packet_Type(APPPACKETTYPE_STATIC);
}


////////////////////////////////////////////////////////////////
//
//	~StaticNetworkObjectClass
//
////////////////////////////////////////////////////////////////
StaticNetworkObjectClass::~StaticNetworkObjectClass (void)
{
	REF_PTR_RELEASE (PhysObj);

	//
	//	Remove ourselves from the master list
	//
	int index = StaticNetworkObjectList.ID (this);
	if (index != -1) {
		StaticNetworkObjectList.Delete (index);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_World_Position
//
////////////////////////////////////////////////////////////////
bool
StaticNetworkObjectClass::Get_World_Position (Vector3 &pos) const
{
	bool retval = false;

	//
	//	Return the physic object's position
	//
	if (PhysObj != NULL) {
		PhysObj->Get_Position (&pos);
		retval = true;
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
// Get_Vis_ID - return the vis ID for this object
//
////////////////////////////////////////////////////////////////
int
StaticNetworkObjectClass::Get_Vis_ID (void)
{
	if (PhysObj != NULL) {
		return PhysObj->Get_Vis_Object_ID ();
	}

	return -1;
}


////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////
void
StaticNetworkObjectClass::Initialize (StaticAnimPhysClass *phys_obj)
{
	REF_PTR_SET (PhysObj, phys_obj);

	//
	//	Update the network ID
	//
	int network_id = 0;
	if (PhysObj != NULL) {
		network_id = PhysObj->Get_ID ();

		//
		//	Copy the object's state
		//
		AnimCollisionManagerClass &anim_mgr = PhysObj->Get_Animation_Manager ();
		AnimationMode	= anim_mgr.Get_Animation_Mode ();
		CurrFrame		= anim_mgr.Get_Current_Frame ();
		TargetFrame		= anim_mgr.Get_Target_Frame ();
		LoopStart		= anim_mgr.Get_Loop_Start ();
		LoopEnd			= anim_mgr.Get_Loop_End ();
	}

	//WWASSERT(network_id >= NETID_STATIC_OBJECT_MIN && network_id <= NETID_STATIC_OBJECT_MAX);
	Set_Network_ID (network_id);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Network_Think
//
////////////////////////////////////////////////////////////////
void
StaticNetworkObjectClass::Network_Think (void)
{
	NetworkObjectClass::Network_Think ();

	if (PhysObj != NULL) {
		AnimCollisionManagerClass &anim_mgr = PhysObj->Get_Animation_Manager ();

		//
		//	Set the dirty bit if the state has changed
		//
		if (	AnimationMode	!= anim_mgr.Get_Animation_Mode () ||
				TargetFrame		!= anim_mgr.Get_Target_Frame () ||
				LoopStart		!= anim_mgr.Get_Loop_Start () ||
				LoopEnd			!= anim_mgr.Get_Loop_End ())
		{
			AnimationMode	= anim_mgr.Get_Animation_Mode ();
			CurrFrame		= anim_mgr.Get_Current_Frame ();
			TargetFrame		= anim_mgr.Get_Target_Frame ();
			LoopStart		= anim_mgr.Get_Loop_Start ();
			LoopEnd			= anim_mgr.Get_Loop_End ();
			Set_Object_Dirty_Bit (BIT_RARE, true);
		}
	}

	return ;
}


/*
////////////////////////////////////////////////////////////////////////////
//
//	Compute_Object_Priority - TSS092201
//
/////////////////////////////////////////////////////////////////////////////
float
StaticNetworkObjectClass::Compute_Object_Priority (int client_id, const Vector3 &client_pos)
{
	float priority = 1.0F;

	if (Get_Object_Dirty_Bit (client_id, BIT_RARE) == false) {
		priority = NetworkObjectClass::Compute_Object_Priority (client_id, client_pos);
	}

	//
	// Priority value is cached, update the cached value.
	//
	Set_Cached_Priority(priority);

	return priority;
}
*/


////////////////////////////////////////////////////////////////
//
//	Generate_Static_Network_Objects
//
////////////////////////////////////////////////////////////////
void
StaticNetworkObjectClass::Generate_Static_Network_Objects (void)
{
	RefPhysListIterator iterator = COMBAT_SCENE->Get_Static_Anim_Object_Iterator ();

	//
	//	Loop over all the static anim objects in the world
	//
	for (iterator.First (); iterator.Is_Done () == false; iterator.Next ()) {
		StaticAnimPhysClass *phys_obj = (StaticAnimPhysClass *)iterator.Peek_Obj ();
		if (phys_obj != NULL) {

			//
			//	Make a new network object for this phys obj
			//
			StaticNetworkObjectClass *network_obj = NULL;
			if (phys_obj->As_ElevatorPhysClass () != NULL) {
				network_obj = new ElevatorNetworkObjectClass;
			} else if (phys_obj->As_DoorPhysClass () != NULL) {
				network_obj = new DoorNetworkObjectClass;
			} else if (phys_obj->As_DamageableStaticPhysClass () != NULL) {
				network_obj = new DSAPONetworkObjectClass;
			} else {
				network_obj = new StaticNetworkObjectClass;
			}

			//
			//	Initialize the network object
			//
			network_obj->Initialize (phys_obj);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Free_Static_Network_Objects
//
////////////////////////////////////////////////////////////////
void
StaticNetworkObjectClass::Free_Static_Network_Objects (void)
{
	DynamicVectorClass<StaticNetworkObjectClass *> temp_list = StaticNetworkObjectList;

	//
	//	Loop over and free all the entries
	//
	for (int index = temp_list.Count () - 1; index >= 0; index --) {
		StaticNetworkObjectClass *object = temp_list[index];
		if (object != NULL) {
			delete object;
		}
	}

	//
	//	Make sure the list has been completely cleared
	//
	WWASSERT (StaticNetworkObjectList.Count () == 0);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Import_Rare
//
////////////////////////////////////////////////////////////////
void
StaticNetworkObjectClass::Import_Rare (BitStreamClass &packet)
{
	NetworkObjectClass::Import_Rare (packet);

	//
	//	Read the data
	//
	packet.Get (AnimationMode);
	packet.Get (LoopStart);
	packet.Get (LoopEnd);
	packet.Get (CurrFrame);
	packet.Get (TargetFrame);

	if (PhysObj != NULL) {
		AnimCollisionManagerClass &anim_mgr = PhysObj->Get_Animation_Manager ();

		//
		//	Set the data
		//
		anim_mgr.Set_Target_Frame (TargetFrame);
		anim_mgr.Set_Current_Frame (CurrFrame);
		anim_mgr.Set_Loop_Start (LoopStart);
		anim_mgr.Set_Loop_End (LoopEnd);
		anim_mgr.Set_Animation_Mode ((AnimCollisionManagerClass::AnimModeType)AnimationMode);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Export_Rare
//
////////////////////////////////////////////////////////////////
void
StaticNetworkObjectClass::Export_Rare (BitStreamClass &packet)
{
	NetworkObjectClass::Export_Rare (packet);

	//
	//	Export information about the animation
	//
	packet.Add (AnimationMode);
	packet.Add (LoopStart);
	packet.Add (LoopEnd);
	packet.Add (CurrFrame);
	packet.Add (TargetFrame);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	DoorNetworkObjectClass
//
////////////////////////////////////////////////////////////////
DoorNetworkObjectClass::DoorNetworkObjectClass (void) :
	DoorState (0)
{
	Set_App_Packet_Type(APPPACKETTYPE_DOOR);
}


////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////
void
DoorNetworkObjectClass::Initialize (StaticAnimPhysClass *phys_obj)
{
	StaticNetworkObjectClass::Initialize (phys_obj);

	//
	//	Get a pointer to the door
	//
	if (PhysObj != NULL) {
		DoorPhysClass *door = PhysObj->As_DoorPhysClass ();
		if (door != NULL) {

			//
			//	Copy the object's state
			//
			DoorState = door->State;
			Set_Object_Dirty_Bit (BIT_RARE, true);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Network_Think
//
////////////////////////////////////////////////////////////////
void
DoorNetworkObjectClass::Network_Think (void)
{
	NetworkObjectClass::Network_Think ();

	//
	//	Get a pointer to the door
	//
	if (PhysObj != NULL) {
		DoorPhysClass *door = PhysObj->As_DoorPhysClass ();
		if (door != NULL) {

			//
			//	Set the dirty bit if the state has changed
			//
			if (DoorState != door->State) {
				DoorState = door->State;
				if (DoorState == STATE_OPENING_DOOR || DoorState == STATE_CLOSING_DOOR || DoorState == STATE_ACCESS_DENIED) {
					Set_Object_Dirty_Bit (BIT_RARE, true);
				}
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Import_Rare
//
////////////////////////////////////////////////////////////////
void
DoorNetworkObjectClass::Import_Rare (BitStreamClass &packet)
{
	//
	//	Note:  As a network optimization, we don't call the base
	// class.  Doors are now state based and don't need to
	// transmit frame numbers.
	//

	//
	//	Read the new state
	//
	packet.Get (DoorState, BITPACK_DOOR_STATE);

	//
	//	Get a pointer to the door
	//
	if (PhysObj != NULL) {
		DoorPhysClass *door = PhysObj->As_DoorPhysClass ();
		if (door != NULL) {

			//
			//	Set the new state
			//
			bool success = door->Set_State (DoorState);
			if (!success) {
				WWDEBUG_SAY(("DoorNetworkObjectClass::Import_Rare -- Door state changed failed to apply\n"));
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Export_Rare
//
////////////////////////////////////////////////////////////////
void
DoorNetworkObjectClass::Export_Rare (BitStreamClass &packet)
{
	//
	//	Note:  As a network optimization, we don't call the base
	// class.  Doors are now state based and don't need to
	// transmit frame numbers.
	//

	//
	//	Send the state
	//
	packet.Add (DoorState, BITPACK_DOOR_STATE);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Description
//
////////////////////////////////////////////////////////////////
void
DoorNetworkObjectClass::Get_Description (StringClass & description)
{
	StringClass line;

	StringClass state_string;

	switch (DoorState) {
		case STATE_CLOSED_DOOR:			state_string = "STATE_CLOSED_DOOR";		break;
		case STATE_OPENED_DOOR:			state_string = "STATE_OPENED_DOOR";		break;
		case STATE_OPENING_DOOR:		state_string = "STATE_OPENING_DOOR";	break;
		case STATE_CLOSING_DOOR:		state_string = "STATE_CLOSING_DOOR";	break;
		case STATE_ACCESS_DENIED:		state_string = "STATE_ACCESS_DENIED";	break;
		default:								DIE;
	}

   line.Format("DoorState:  %d (%s)\n", DoorState, state_string);
	description += line;

	if (PhysObj != NULL) {
		DoorPhysClass *door = PhysObj->As_DoorPhysClass ();
		if (door != NULL) {
			line.Format("Timer:      %-5.2f\n", door->Timer);
			description += line;

			line.Format("CheckTimer: %-5.2f\n", door->CheckTimer);
			description += line;

			line.Format("State:      %d\n", door->State);
			description += line;

			line.Format("ORP:        %d\n", door->OpenRequestPending);
			description += line;

			line.Format("LockState:  %d\n", door->LockState);
			description += line;
		}
	}
}


////////////////////////////////////////////////////////////////
//
//	ElevatorNetworkObjectClass
//
////////////////////////////////////////////////////////////////
ElevatorNetworkObjectClass::ElevatorNetworkObjectClass (void) :
	State (0),
	DoorStateTop (0),
	DoorStateBottom (0)
{
	Set_App_Packet_Type(APPPACKETTYPE_ELEVATOR);
}


////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////
void
ElevatorNetworkObjectClass::Initialize (StaticAnimPhysClass *phys_obj)
{
	//bug NetworkObjectClass::Network_Think ();
	StaticNetworkObjectClass::Initialize (phys_obj);

	//
	//	Get a pointer to the elevator
	//
	if (PhysObj != NULL) {
		ElevatorPhysClass *elevator = PhysObj->As_ElevatorPhysClass ();
		if (elevator != NULL) {

			//
			//	Copy the object's state
			//
			State					= elevator->State;
			DoorStateTop		= elevator->DoorStates[ElevatorPhysClass::TOP];
			DoorStateBottom	= elevator->DoorStates[ElevatorPhysClass::BOTTOM];
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Network_Think
//
////////////////////////////////////////////////////////////////
void
ElevatorNetworkObjectClass::Network_Think (void)
{
	NetworkObjectClass::Network_Think ();
	//StaticNetworkObjectClass::Network_Think ();

	//
	//	Get a pointer to the elevator
	//
	if (PhysObj != NULL) {
		ElevatorPhysClass *elevator = PhysObj->As_ElevatorPhysClass ();
		if (elevator != NULL) {

			//
			//	Set the dirty bit if the state has changed
			//
			if (	State != elevator->State ||
					DoorStateTop != elevator->DoorStates[ElevatorPhysClass::TOP] ||
					DoorStateBottom != elevator->DoorStates[ElevatorPhysClass::BOTTOM])
			{
				State					= elevator->State;
				DoorStateTop		= elevator->DoorStates[ElevatorPhysClass::TOP];
				DoorStateBottom	= elevator->DoorStates[ElevatorPhysClass::BOTTOM];
				Set_Object_Dirty_Bit (BIT_RARE, true);
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Import_Rare
//
////////////////////////////////////////////////////////////////
void
ElevatorNetworkObjectClass::Import_Rare (BitStreamClass &packet)
{
	//
	//	Note:  As a network optimization, we don't call the base
	// class.  Doors are now state based and don't need to
	// transmit frame numbers.
	//

//StaticNetworkObjectClass::Import_Rare (packet);

	//
	//	Read the new state data
	//
	packet.Get (State);
	packet.Get (DoorStateTop);
	packet.Get (DoorStateBottom);

	//
	//	Get a pointer to the elevator
	//
	if (PhysObj != NULL) {
		ElevatorPhysClass *elevator = PhysObj->As_ElevatorPhysClass ();
		if (elevator != NULL) {

			//
			//	Switch to the new states
			//
			elevator->Set_State (State);
			elevator->Set_Door_State (ElevatorPhysClass::TOP, DoorStateTop);
			elevator->Set_Door_State (ElevatorPhysClass::BOTTOM, DoorStateBottom);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Export_Rare
 //
////////////////////////////////////////////////////////////////
void
ElevatorNetworkObjectClass::Export_Rare (BitStreamClass &packet)
{
	//
	//	Note:  As a network optimization, we don't call the base
	// class.  Doors are now state based and don't need to
	// transmit frame numbers.
	//

//StaticNetworkObjectClass::Export_Rare (packet);

	//
	//	Transmit the state information
	//
	packet.Add (State);
	packet.Add (DoorStateTop);
	packet.Add (DoorStateBottom);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Description
//
////////////////////////////////////////////////////////////////
void
ElevatorNetworkObjectClass::Get_Description (StringClass & description)
{
	StringClass line;

	StringClass state_string;

	//
	// State
	//
	switch (State) {
		case ElevatorPhysClass::STATE_DOWN:				state_string = "STATE_DOWN";			break;
		case ElevatorPhysClass::STATE_MOVING_UP:		state_string = "STATE_MOVING_UP";	break;
		case ElevatorPhysClass::STATE_UP:				state_string = "STATE_UP";				break;
		case ElevatorPhysClass::STATE_MOVING_DOWN:	state_string = "STATE_MOVING_DOWN";	break;
		default:													DIE;
	}

	line.Format("State:            %d (%s)\n", State, state_string);
	description += line;

	//
	// DoorStateTop
	//
	switch (DoorStateTop) {
		case ElevatorPhysClass::DOOR_STATE_NORMAL:			state_string = "DOOR_STATE_NORMAL";				break;
		case ElevatorPhysClass::DOOR_STATE_UNLOCKED:			state_string = "DOOR_STATE_UNLOCKED";			break;
		case ElevatorPhysClass::DOOR_STATE_ACCESS_DENIED:	state_string = "DOOR_STATE_ACCESS_DENIED";	break;
		default:															DIE;
	}

	line.Format("DoorStateTop:     %d (%s)\n", DoorStateTop, state_string);
	description += line;

	//
	// DoorStateBottom
	//
	switch (DoorStateBottom) {
		case ElevatorPhysClass::DOOR_STATE_NORMAL:			state_string = "DOOR_STATE_NORMAL";				break;
		case ElevatorPhysClass::DOOR_STATE_UNLOCKED:			state_string = "DOOR_STATE_UNLOCKED";			break;
		case ElevatorPhysClass::DOOR_STATE_ACCESS_DENIED:	state_string = "DOOR_STATE_ACCESS_DENIED";	break;
		default:															DIE;
	}

	line.Format("DoorStateBottom:  %d (%s)\n", DoorStateBottom, state_string);
		description += line;


	if (PhysObj != NULL) {
		ElevatorPhysClass *elevator = PhysObj->As_ElevatorPhysClass ();
		if (elevator != NULL) {

		line.Format("State:            %d\n",	elevator->State);
		description += line;
		line.Format("DoorStates[0]:    %d\n",	elevator->DoorStates[0]);
		description += line;
		line.Format("DoorStates[1]:    %d\n",	elevator->DoorStates[1]);
		description += line;
		line.Format("CheckTimer:       %-5.2f\n",	elevator->CheckTimer);
		description += line;
		line.Format("PrevFrame:        %-5.2f\n",	elevator->PrevFrame);
		description += line;
		line.Format("IsCallTimerSet:   %d\n", elevator->IsCallTimerSet);
		description += line;
		line.Format("CallTimer:        %-5.2f\n",	elevator->CallTimer);
		description += line;
		line.Format("TriggerRequest:   %d\n", elevator->TriggerRequest);
		description += line;
		}
	}
}


////////////////////////////////////////////////////////////////
//
//	DSAPONetworkObjectClass
//
////////////////////////////////////////////////////////////////
DSAPONetworkObjectClass::DSAPONetworkObjectClass (void) :
	Health (0)
{
	Set_App_Packet_Type(APPPACKETTYPE_DSAPO);
}


////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////
void
DSAPONetworkObjectClass::Initialize (StaticAnimPhysClass *phys_obj)
{
	StaticNetworkObjectClass::Initialize (phys_obj);

	//
	//	Get a pointer to the object
	//
	if (PhysObj != NULL) {
		DamageableStaticPhysClass *dsphys_obj = PhysObj->As_DamageableStaticPhysClass ();
		if (dsphys_obj != NULL) {

			//
			//	Copy the object's state
			//
			Health = dsphys_obj->DefenseObject.Get_Health ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Network_Think
//
////////////////////////////////////////////////////////////////
void
DSAPONetworkObjectClass::Network_Think (void)
{
	StaticNetworkObjectClass::Network_Think ();

	//
	//	Get a pointer to the object
	//
	if (PhysObj != NULL) {
		DamageableStaticPhysClass *phys_obj = PhysObj->As_DamageableStaticPhysClass ();
		if (phys_obj != NULL) {

			//
			//	Set the dirty bit if the state has changed
			//
			if (Health > 0 && phys_obj->DefenseObject.Get_Health () <= 0) {
				Health = phys_obj->DefenseObject.Get_Health ();
				Set_Object_Dirty_Bit (BIT_RARE, true);
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Import_Rare
//
////////////////////////////////////////////////////////////////
void
DSAPONetworkObjectClass::Import_Rare (BitStreamClass &packet)
{
	DamageableStaticPhysClass *phys_obj = PhysObj->As_DamageableStaticPhysClass ();
	if (phys_obj == NULL) {
		return ;
	}

	//
	//	Get the health BEFORE we import the data
	//
	float old_health = phys_obj->DefenseObject.Get_Health();

	//
	//	Import the data
	//
	StaticNetworkObjectClass::Import_Rare (packet);
	phys_obj->DefenseObject.Import (packet);

	//
	//	Did the health drop below 0?
	//
	if ((old_health > 0) && (phys_obj->DefenseObject.Get_Health () <= 0)) {

		//
		// Anim data should already been sent
		//
		phys_obj->Get_Animation_Manager().Set_Animation_Mode (AnimCollisionManagerClass::ANIMATE_TARGET);

		//
		// Play the explosion
		//
		const DamageableStaticPhysDefClass *definition = phys_obj->Get_DamageableStaticPhysDef ();
		if (definition->KilledExplosion != 0) {
			ExplosionManager::Create_Explosion_At (definition->KilledExplosion, phys_obj->Get_Transform (), NULL);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Export_Rare
 //
////////////////////////////////////////////////////////////////
void
DSAPONetworkObjectClass::Export_Rare (BitStreamClass &packet)
{
	DamageableStaticPhysClass *phys_obj = PhysObj->As_DamageableStaticPhysClass ();
	if (phys_obj == NULL) {
		return ;
	}

	//
	//	Just export the defense object's settings
	//
	StaticNetworkObjectClass::Export_Rare (packet);
	phys_obj->DefenseObject.Export (packet);
	return ;
}
