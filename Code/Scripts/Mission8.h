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
*     Mission8.h
*
* DESCRIPTION
*     Mission 8 definitions
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: Dave_s $
*     $Revision: 17 $
*     $Modtime: 12/10/01 5:20p $
*     $Archive: /Commando/Code/Scripts/Mission8.h $
*
******************************************************************************/

#ifndef _MISSION8_H_
#define _MISSION8_H_


// Defines and Includes

#include "Toolkit.h"


// Predefined Constants

// Public Variables

// Enumerations
// Customs
#define M08_ALARMED							8000
#define M08_CHECK_ALARM						8001
#define M08_SOUND_ALARM						8002
#define M08_YOU_ATTACK_HAVOC				8003
#define M08_DEAD_HAVOC_ATTACKER				8004
#define	M08_DONT_MOVE						8005
#define	M08_NOD_ATTACKEE_KILLED				8006
#define	M08_PRISONER_ATTACKER_KILLED		8007	
#define M08_GDI_FREE_PRISON_KILLED			8008
#define	M08_CUSTOM_ACTIVATE					8009
#define	M08_REINFORCEMENT_KILLED			8010
#define M08_UNIT_ID							8011
#define M08_ARCHAELOGICAL_KILLED			8012
#define	M08_PETRAA25_KILLED					8013

#define	M08_DEACTIVATE_ENCOUNTER			8014
#define	M08_INNATE_ON						8015
#define	M08_BASKETBALL_GUN_EMP_KILLED		8016
#define M08_PETRA_A_CREATED					8017
#define	M08_PETRA_A_KILLED					8018
#define	M08_PETRA_A_HELO_KILLED				8019
#define	M08_PETRA_A_ACTIVATE				8020
#define	M08_PETRA_A_DEACTIVATE				8021
#define	M08_PETRA_A_DEACTIVATE_LOGICAL		8022
#define	M08_PETRA_A_UNIT_STATUS				8023

#define M08_PETRA_B_CREATED					8024
#define	M08_PETRA_B_KILLED					8025
#define	M08_PETRA_B_HELO_KILLED				8026
#define	M08_PETRA_B_ACTIVATE				8027
#define	M08_PETRA_B_DEACTIVATE				8028
#define	M08_PETRA_B_DEACTIVATE_LOGICAL		8029
#define	M08_PETRA_B_UNIT_STATUS				8030

#define M08_PETRA_C_CREATED					8031
#define	M08_PETRA_C_KILLED					8032
#define	M08_PETRA_C_HELO_KILLED				8033
#define	M08_PETRA_C_ACTIVATE				8034
#define	M08_PETRA_C_DEACTIVATE				8035
#define	M08_PETRA_C_DEACTIVATE_LOGICAL		8036
#define	M08_PETRA_C_UNIT_STATUS				8037

#define	M08_SCIENTIST_KILLED				8038
#define	M08_REINFORCEMENTS					8039
#define	M08_CAVERN_ENTRANCE_TRUCK			8040
#define	M08_PATROL_KILLED					8041
#define	M08_FREE_PRISONER					8042
#define	M08_SAKURA_ID						8043
#define M08_MOVE_SAKURA						8044
#define	M08_PLAYER_VEHICLE_ID				8045
#define	M08_MOBILE_APACHE_FLEE				8046
#define M08_RELOCATE						8047
#define	M08_STAR_IMMORTAL					8048

		
// Logical Sound Types
#define M08_SOUND_ALARM_ON			18000
#define M08_SOUND_ALARM_OFF			18001
#define M08_ATTACK_HAVOC_SOUND		18002
#define M08_ATTACK_HAVOC			18003
#define	M08_FIX_ALARM				18004
#define	M08_HELIPAD_DESTROYED		18006

#define M08_PETRA_A_LOGICAL			18007
#define	M08_PETRA_B_LOGICAL			18008
#define	M08_PETRA_C_LOGICAL			18009


// Timer Enumerations
#define M08_DEAD_HAVOC			28001

typedef enum
{
	M8TIMER_START = STIMER_MISSION8,
} M8TIMER;

#endif // _MISSION8_H_
