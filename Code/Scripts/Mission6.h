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
*     Mission6.h
*
* DESCRIPTION
*     Mission 6 definitions
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: Dave_s $
*     $Revision: 36 $
*     $Modtime: 12/20/01 1:38p $
*     $Archive: /Commando/Code/Scripts/Mission6.h $
*
******************************************************************************/

#ifndef _MISSION6_H_
#define _MISSION6_H_


// Defines and Includes

#include "Toolkit.h"

#define M06_OBJECTIVE_ACTIVATE	101


// Predefined Constants

// Customs
#define M06_SOUND_ALARM			6000
#define M06_MENDOZA_ID			6001
#define M06_WR_KCARD_ACQUIRED	6002
#define M06_STOP_PICKUP			6003
#define M06_CHECK_ALARM			6005
#define M06_STAND_DOWN			6006
#define M06_START_CONVERSATION	6007
#define M06_FIX_ALARM			6008
#define M06_DEAD_COURTYARD_EAGLE	6009
#define M06_DEAD_HEDGEMAZE_EAGLE	6010
#define M06_DEAD_BARRACKS_EAGLE		6011
#define	M06_DISABLE_ZONE			6012
#define M06_DEAD_INTERIOR_PATROL	6013
#define M06_ATTACK_HAVOC			6014
#define M06_DEAD_HAVOC_ATTACKER		6015
#define M06_YOU_ATTACK_HAVOC		6016
#define M06_ALARMED					6017
#define	M06_NEXT_EVAC				6018
#define	M06_EVAC_ARRIVED			6019
#define M06_TO_EVAC					6020
#define M06_FLYOVER_COMPLETE		6021
#define M06_CUSTOM_ACTIVATE			6023
#define	M06_ACTIVATE_FLYOVERS		6024
#define	M06_MOVE_SYDNEY				6025
#define M06_CHATEAU_COLLAPSE		6026
#define	M06_RELOCATE				6027
#define	M06_MIDTRO_EXPLOSION		6028
	

// Logical Sound Types
#define M06_SOUND_ALARM_ON			16600
#define M06_SOUND_ALARM_OFF			16601
#define M06_DISABLE_TOWER_SPAWN		16602
#define M06_DISABLE_COURTYARD_SPAWN	16603
#define M06_DISABLE_HEDGEMAZE_SPAWN	16604
#define M06_DISABLE_BARRACKS_SPAWN	16605
#define	M06_ATTACK_HAVOC_SOUND		16606
#define	M06_SOLDIER_TO_EVAC			16607
#define	M06_CHATEAU_DESTRUCTION		16608
#define	M06_CLEAR_FOR_MENDOZA		16609

// Timers
#define M06_DEAD_HAVOC			100600


// Public Variables

// Enumerations

// Timer Enumerations

typedef enum
{
	M6TIMER_START = STIMER_MISSION6,
} M6TIMER;

#endif // _MISSION6_H_
