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
 *                     $Archive:: /Commando/Code/Combat/buildingstate.h                       $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 2/07/01 6:00p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif


#ifndef BUILDINGSTATES_H
#define BUILDINGSTATES_H

#include "always.h"

/**
** BuildingStateClass
** This 'class' encapsulates some functions which work with the enumerated states that buildings can
** have.  These enumerations are used by BuildingGameObj and BuildingAggregateClass...
**
** WARNING: Don't change these enumerations without checking the BuildingGameObj and BuildingAggregateClass
** to see what problems you might cause with save-load and their general operation...
*/
class BuildingStateClass
{
public:

	enum
	{
		HEALTH100_POWERON				= 0,
		HEALTH75_POWERON,
		HEALTH50_POWERON,
		HEALTH25_POWERON,
		DESTROYED_POWERON,

		HEALTH100_POWEROFF,
		HEALTH75_POWEROFF,
		HEALTH50_POWEROFF,
		HEALTH25_POWEROFF,
		DESTROYED_POWEROFF,

		STATE_COUNT
	};

	enum
	{
		HEALTH_100						= 0,
		HEALTH_75,
		HEALTH_50,
		HEALTH_25,
		HEALTH_0,
	};

	static int				Get_Health_State(int building_state);
	static int				Percentage_To_Health_State(float health);

	static bool				Is_Power_On(int building_state);
	static int				Enable_Power(int input_state,bool onoff);

	static int				Compose_State(int health_state,bool power_onoff);

	static const char *	Get_State_Name(int state);
};


#endif // BUILDINGSTATES_H


