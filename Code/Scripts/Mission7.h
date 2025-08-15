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

/******************************************************************************
*
* FILE
*     Mission7.h
*
* DESCRIPTION
*     Mission 7 definitions
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: Dave_s $
*     $Revision: 17 $
*     $Modtime: 11/08/01 1:59p $
*     $Archive: /Commando/Code/Scripts/Mission7.h $
*
******************************************************************************/

#ifndef _MISSION7_H_
#define _MISSION7_H_


// Defines and Includes

#include "Toolkit.h"

// Customs
#define M07_GO_EVAC_SITE					7000
#define M07_NUKE_BLAST						7001
#define	M07_CUSTOM_ACTIVATE					7002
#define	M07_REINFORCEMENT_KILLED			7002
#define	M07_EVAC_SITE_VEHICLE_KILLED		7003
#define M07_CAPTURE_SAM						7004
#define	M07_HOTWIRE_CAPTURE_SAMS			7005
#define	M07_CHANGE_SAM_TEAM					7006
#define M07_SAM_CONVERTED					7007
#define M07_SAM_KILLED						7008
#define M07_TOWNSQUARE_CHINOOK				7009
#define	M07_RADAR_KILLED					7010
#define	M07_FREE_CIV						7011
#define	M07_PLAYER_VEHICLE_KILLED			7012
#define	M07_VEHICLE_DROP_ZONE				7013
#define	M07_ARTILLERY_KILLED				7014
#define	M07_PLAYERTYPE_CHANGE_OBELISK		7015
#define M07_DEACTIVATE_ENCOUNTER			7016
#define	M07_REINFORCEMENT_UNIT_KILLED		7017
#define	M07_REINFORCEMENT_UNIT_CREATED		7018
#define	M07_ACTIVATE_ENCOUNTER				7019
#define	M07_WINDOW_ACTIVATE					7020
#define	M07_WINDOW_REINFORCEMENTS_KILLED	7021
#define	M07_V01_UNIT_KILLED					7022
#define	M07_DEACTIVATE_V01					7023
#define	M07_ACTIVATE_V01					7024
#define	M07_TRIANGLE_APC_KILLED				7025
#define	M07_TRIANGLE_UNIT_KILLED			7026
#define	M07_SPAWN_TRIANGLE_CIV				7027
#define M07_PLAYER_VEHICLE_ID				7028
#define	M07_E10_TANK_CREATED				7029
#define	M07_PARADROP_UNIT_KILLED			7030
#define	M07_DEACTIVATE_PARADROP				7031
#define	M07_GO_ASSEMBLY						7032
#define	M07_MOVE_HOTWIRE					7033
#define M07_V05_OFFICER_KILLED				7034
#define	M07_EVAC_INN						7035
#define	M07_INN_HELICOPTER_OUT				7036
#define	M07_INN_HELO_COLLISIONS				7037
#define	M07_DEAD6_EVAC						7038
#define	M07_MOVE_STEALTH_TANK				7039
#define	M07_SSM_DAMAGED						7040
#define	M07_SSM_FIXED						7041
#define	M07_DEACTIVATE_NUKE_BLAST			7042
#define	M07_SSM_CRATE_KILLED				7043
#define	M07_FREE_CIV_RESIST					7044
#define	M07_OBELISK_KILLED					7045		
	

// Logical Sound Types
#define M07_NUKE_IMPACT						17000
#define M07_RADAR_DAMAGED					17001
#define M07_RADAR_FIXED						17002
#define	M07_EXPLODE_BARRELS					17003
#define	M07_ENGINEER_DAMAGED				17004
#define	M07_ENGINEER_FIXED					17005
#define	M07_RELOCATE_TRIANGLE_APACHE		17006

// Predefined Constants
#define GUNNER (Commands->Find_Object(100661))
#define PATCH (Commands->Find_Object(100659))
#define DEADEYE (Commands->Find_Object(100660))
#define SYDNEY (Commands->Find_Object(100662))
#define HOTWIRE (Commands->Find_Object(100658))



typedef enum
{
	M7TIMER_START = STIMER_MISSION7,
} M7TIMER;

#endif // _MISSION7_H_
