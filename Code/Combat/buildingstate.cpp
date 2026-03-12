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
 *                 Project Name : Combat                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/buildingstate.cpp                     $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 9/08/00 10:37a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "buildingstate.h"
#include "wwdebug.h"

/*
** Static data used by BuildingStateClass
*/
static int _EquivalentPowerOnState[] =
{
	BuildingStateClass::HEALTH100_POWERON,
	BuildingStateClass::HEALTH75_POWERON,
	BuildingStateClass::HEALTH50_POWERON,
	BuildingStateClass::HEALTH25_POWERON,
	BuildingStateClass::DESTROYED_POWERON,

	BuildingStateClass::HEALTH100_POWERON,
	BuildingStateClass::HEALTH75_POWERON,
	BuildingStateClass::HEALTH50_POWERON,
	BuildingStateClass::HEALTH25_POWERON,
	BuildingStateClass::DESTROYED_POWERON,

};

static int _EquivalentPowerOffState[] =
{
	BuildingStateClass::HEALTH100_POWEROFF,
	BuildingStateClass::HEALTH75_POWEROFF,
	BuildingStateClass::HEALTH50_POWEROFF,
	BuildingStateClass::HEALTH25_POWEROFF,
	BuildingStateClass::DESTROYED_POWEROFF,

	BuildingStateClass::HEALTH100_POWEROFF,
	BuildingStateClass::HEALTH75_POWEROFF,
	BuildingStateClass::HEALTH50_POWEROFF,
	BuildingStateClass::HEALTH25_POWEROFF,
	BuildingStateClass::DESTROYED_POWEROFF,
};

static const char * _StateNames[] =
{
	"Building State: Health 100%, Power ON",
	"Building State: Health 75%, Power ON",
	"Building State: Health 50%, Power ON",
	"Building State: Health 25%, Power ON",
	"Building State: Destroyed, Power ON",
	"Building State: Health 100%, Power OFF",
	"Building State: Health 75%, Power OFF",
	"Building State: Health 50%, Power OFF",
	"Building State: Health 25%, Power OFF",
	"Building State: Destroyed, Power OFF",
};




int BuildingStateClass::Get_Health_State(int building_state)
{
	int state = building_state;
	if (state >= HEALTH100_POWEROFF) {
		state -= HEALTH100_POWEROFF;
	}
	return state;
}

int BuildingStateClass::Percentage_To_Health_State(float health)
{
	if (health <= 0.0f) {
		return HEALTH_0;
	}
	if (health <= 25.0f) {
		return HEALTH_25;
	}
	if (health <= 50.0f) {
		return HEALTH_50;
	}
	if (health <= 75.0f) {
		return HEALTH_75;
	}
	return HEALTH_100;
}

bool BuildingStateClass::Is_Power_On(int building_state)
{
	return (building_state < HEALTH100_POWEROFF);
}

int BuildingStateClass::Enable_Power(int input_state,bool onoff)
{
	if (onoff) {
		return _EquivalentPowerOnState[input_state];
	} else {
		return _EquivalentPowerOffState[input_state];
	}
}

int BuildingStateClass::Compose_State(int health_state,bool power_onoff)
{
	int state = health_state;
	if (power_onoff == false) {
		state += HEALTH100_POWEROFF;
	}
	return state;
}

const char * BuildingStateClass::Get_State_Name(int state)
{
	WWASSERT(state >= 0);
	WWASSERT(state < STATE_COUNT);

	return _StateNames[state];
}






