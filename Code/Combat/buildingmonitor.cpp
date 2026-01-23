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
 *                     $Archive:: /Commando/Code/Combat/buildingmonitor.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/05/01 1:46p                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "buildingmonitor.h"
#include "building.h"


////////////////////////////////////////////////////////////////
//
//	Killed
//
////////////////////////////////////////////////////////////////
void
BuildingMonitorClass::Killed (GameObject * /* game_obj */, GameObject * /* killer */)
{
	WWASSERT (Building != NULL);

	//
	//	Notify the building
	//
	Building->On_Destroyed ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Damaged
//
////////////////////////////////////////////////////////////////
void
BuildingMonitorClass::Damaged (GameObject * /* game_obj */, GameObject *killer, float amount )
{
	// Only report damage not heal
	if (amount > 0.0f) {
		WWASSERT (Building != NULL);

		// If the building has been damaged to the point of being destroyed
		// then do not report the damaged event. A killed event will be sent
		// later.
		DefenseObjectClass* defense = Building->Get_Defense_Object();

		if (defense && (defense->Get_Health() <= 0.0f)) {
			return;
		}

		DamageableGameObj* damager = NULL;

		if (killer) {
			damager = killer->As_DamageableGameObj();
		}

		bool friendlyFire = (damager && (damager->Get_Player_Type() == Building->Get_Player_Type()));

		//	Notify the building
		if (!friendlyFire) {
			Building->On_Damaged();
		}
	}
}


////////////////////////////////////////////////////////////////
//
//	Custom
//
////////////////////////////////////////////////////////////////
void
BuildingMonitorClass::Custom (GameObject * /* game_obj */, int type, intptr_t /* param */, GameObject * /* sender */)
{
	WWASSERT (Building != NULL);

	//
	//	Notify the controller
	//
	if (type == CUSTOM_EVENT_BUILDING_POWER_CHANGED) {
		//Building->On_Building_Damaged (game_obj->As_BuldingGameObj ());
	}

	return ;
}
