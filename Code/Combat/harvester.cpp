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
 *                     $Archive:: /Commando/Code/Combat/harvester.cpp         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/18/02 11:16p                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "harvester.h"
#include "vehicle.h"
#include "refinerygameobj.h"
#include "animcontrol.h"
#include "vehiclephys.h"


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
enum
{
	ACTIONID_GOTO_TIBERIUM	= 1000,
	ACTIONID_DOCK,
};


////////////////////////////////////////////////////////////////
//
//	HarvesterClass
//
////////////////////////////////////////////////////////////////
HarvesterClass::HarvesterClass (void) :
	Refinery (NULL),
	DockLocation (0, 0, 0),
	DockEntrance (0, 0, 0),
	StateTimer (0),
	State (STATE_UNINITIALIZED),
	Vehicle (NULL),
	IsHarvesting (false)
{
	Set_ID (GameObjObserverManager::Get_Next_Observer_ID ());
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~HarvesterClass
//
////////////////////////////////////////////////////////////////
HarvesterClass::~HarvesterClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////
void
HarvesterClass::Initialize (void)
{
	Go_Harvest ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Go_Harvest
//
////////////////////////////////////////////////////////////////
void
HarvesterClass::Go_Harvest (void)
{
	//
	//	Bail if the vehicle pointer is invalid, or if the vehicle
	// isn't on the ground...
	//
	if (Vehicle == NULL || Vehicle->Peek_Vehicle_Phys ()->Is_In_Contact () == false) {
		return ;
	}

	State = STATE_GOING_TO_HARVEST;

	//
	//	Choose a random offset into the tiberium box
	//
	Vector3 rand_offset;
	rand_offset.X = WWMath::Random_Float (0, TiberiumRegion.Extent.X - 5);
	rand_offset.Y = WWMath::Random_Float (0, TiberiumRegion.Extent.Y - 5);
	rand_offset.Z = 0;

	//
	//	Tranform this random offset into a world space position
	//
	Vector3 rand_pos = TiberiumRegion.Center + (TiberiumRegion.Basis * rand_offset);
	
	//
	//	Setup an action to instruct the vehicle to goto that location
	//
	ActionParamsStruct parameters;
	parameters.Priority		= 99;
	parameters.ActionID		= ACTIONID_GOTO_TIBERIUM;
	parameters.ObserverID	= Get_ID ();
	parameters.Set_Movement (rand_pos, 1.0F, 1.0F);
	Vehicle->Get_Action ()->Goto (parameters);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Go_Unload_Tiberium
//
////////////////////////////////////////////////////////////////
void
HarvesterClass::Go_Unload_Tiberium (void)
{
	Play_Harvest_Animation (false);

	//
	//	Switch states
	//
	IsHarvesting	= false;
	State				= STATE_GOING_TO_UNLOAD;

	//
	//	Setup an action to instruct the vehicle to dock in its refinery
	//
	ActionParamsStruct parameters;
	parameters.Priority		= 99;
	parameters.ActionID		= ACTIONID_DOCK;
	parameters.ObserverID	= Get_ID ();
	parameters.Dock_Vehicle (DockLocation, DockEntrance);
	parameters.MoveSpeed		= 1.0F;
	Vehicle->Get_Action ()->Dock_Vehicle (parameters);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Unload_Tiberium
//
////////////////////////////////////////////////////////////////
void
HarvesterClass::Unload_Tiberium (void)
{
	Refinery->On_Harvester_Docked ();
	State = STATE_UNLOADING;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Harvest_Tiberium
//
////////////////////////////////////////////////////////////////
void
HarvesterClass::Harvest_Tiberium (void)
{
	//
	//	Reset the harvesting timer (if necessary)
	//
	if (IsHarvesting == false) {
		HarvestTimer = 15.0F;
		IsHarvesting = true;
	}

	//
	//	Harvest the tiberium for a few seconds
	//
	State			= STATE_HARVESTING;
	StateTimer	= WWMath::Random_Float (3.0F, 7.0F);
	
	//
	//	Start playing the harvest animation
	//
	Play_Harvest_Animation (true);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Action_Complete
//
////////////////////////////////////////////////////////////////
void
HarvesterClass::Action_Complete
(
	GameObject *				/* game_obj */,
	int							action_id,
	ActionCompleteReason		complete_reason
)
{
	//
	//	Check to ensure the harvester successfully completed
	// its action...
	//
	if (complete_reason == ACTION_COMPLETE_NORMAL) {

		//
		//	Which action was completed?
		//
		if (action_id == ACTIONID_GOTO_TIBERIUM) {
			
			//
			//	Check to ensure we were going to harvest the
			// tiberium before actually start harvesting
			//
			if (State == STATE_GOING_TO_HARVEST) {
				Harvest_Tiberium ();
			}
		} else if (action_id == ACTIONID_DOCK) {
			Unload_Tiberium ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Animation_Complete
//
////////////////////////////////////////////////////////////////
void
HarvesterClass::Animation_Complete
(
	GameObject *	/* game_obj */,
	const char *	/* animation_name */
)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Attach
//
////////////////////////////////////////////////////////////////
void
HarvesterClass::Attach (GameObject *game_obj)
{
	WWASSERT (game_obj != NULL);
	if (game_obj != NULL) {

		//
		//	See if this game object is a vehicle (it better be)
		//
		PhysicalGameObj *physical_obj = game_obj->As_PhysicalGameObj ();
		if (physical_obj != NULL) {
			Vehicle = physical_obj->As_VehicleGameObj ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Detach
//
////////////////////////////////////////////////////////////////
void
HarvesterClass::Detach (GameObject * /*game_obj*/)
{
	//
	//	Remove ourselves from the refinery
	//
	if (Refinery != NULL) {
		Refinery->Set_Harvester (NULL);
	}

	Vehicle = NULL;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Think
//
////////////////////////////////////////////////////////////////
void
HarvesterClass::Think (void)
{
	if (IsHarvesting) {

		//
		//	If we are harvesting, check to see if its time to go unload
		// the tiberium.
		//
		HarvestTimer -= TimeManager::Get_Frame_Seconds ();
		if (HarvestTimer <= 0) {
			Go_Unload_Tiberium ();
		}
	}

	switch (State)
	{		
		case STATE_HARVESTING:
			
			//
			//	Check to see if we should harvest in a new location
			//
			StateTimer -= TimeManager::Get_Frame_Seconds ();
			if (StateTimer	<= 0) {
				Go_Harvest ();
			}

			break;
		
		case STATE_UNINITIALIZED:
			if (Vehicle != NULL) {
				Go_Harvest ();
			}
			break;

		case STATE_GOING_TO_HARVEST:
		case STATE_GOING_TO_UNLOAD:
		case STATE_UNLOADING:
		default:
			break;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Stop
//
////////////////////////////////////////////////////////////////
void
HarvesterClass::Stop (void)
{
	//
	//	Stopt the harvester from moving
	//
	if (Vehicle != NULL) {
		Vehicle->Get_Action ()->Reset (100);
		Vehicle->Enable_Engine (false);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Play_Harvest_Animation
//
////////////////////////////////////////////////////////////////
void
HarvesterClass::Play_Harvest_Animation (bool onoff)
{
	if (onoff) {

		//
		//	Play the animation looped...
		//
		Vehicle->Set_Animation (HarvestAnimationName, true);		

	} else {
		
		//
		//	Stop the animation
		//
		AnimControlClass *anim_control = Vehicle->Get_Anim_Control ();
		if (anim_control != NULL) {
			anim_control->Set_Animation( (const char *)NULL );
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Custom
//
////////////////////////////////////////////////////////////////
void
HarvesterClass::Custom
(
	GameObject *	/* game_obj */,
	int				type,
	intptr_t		/* param */,
	GameObject *	/* sender */
)
{
	//
	//	Are we backing into the docking bay?
	//
	if (type == CUSTOM_EVENT_DOCK_BACKING_IN) {
		Refinery->Play_Unloading_Animation (true);
	}

	return ;
}


void HarvesterClass::Damaged(GameObject* damaged, GameObject* killer, float amount)
{
	// Only report damage not heal
	if (amount > 0.0f) {
		VehicleGameObj* vehicle = static_cast<VehicleGameObj*>(damaged);
		assert(Vehicle == vehicle);

		// If the unit has been damaged to the point of being destroyed
		// then do not report the damaged event. A killed event will be sent
		// later.
		DefenseObjectClass* defense = vehicle->Get_Defense_Object();

		if (defense && (defense->Get_Health() <= 0.0f)) {
			return;
		}

		DamageableGameObj* damager = NULL;

		if (killer) {
			damager = killer->As_DamageableGameObj();
		}

		bool friendlyFire = (damager && (damager->Get_Player_Type() == vehicle->Get_Player_Type()));

		//	Notify the building
		if (!friendlyFire) {
			assert(Refinery != NULL);
			Refinery->On_Harvester_Damaged(vehicle);
		}
	}
}


void HarvesterClass::Destroyed(GameObject* destroyed)
{
	VehicleGameObj* vehicle = static_cast<VehicleGameObj*>(destroyed);
	assert(Vehicle == vehicle);
	Refinery->On_Harvester_Destroyed(vehicle);
}
