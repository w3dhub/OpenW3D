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
*     Mission9.h
*
* DESCRIPTION
*     Mission 9 definitions
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: Dan_e $
*     $Revision: 57 $
*     $Modtime: 1/02/02 10:11a $
*     $Archive: /Commando/Code/Scripts/Mission9.h $
*
******************************************************************************/

#ifndef _MISSION9_H_
#define _MISSION9_H_


// Defines and Includes

#include "Toolkit.h"

#define check		10000


// Predefined Constants

// Public Variables

// Enumerations

#define RUN_TO_TANK			40000
#define ATTACK				40001
#define CHANGE_ATTACK		40002
#define CHARGE_COMPLETE		40003
#define ACTIVATE			40004
#define COUNTER				40005
#define CONVERSATION		40006
#define TRIGGERED			40007
#define CHANGE_PLAYERTYPE	40008
#define MUTANT				40009
#define ENTERED				40010
#define DEACTIVATE			40011
#define SPAWN_LOC			40012
#define INCREMENT			40013
#define KEY_COUNTER			40014
#define CHECK				40015
#define COUNT				40016
#define VERIFY				40017
#define GO					40018
#define STOP				40019
#define WAYPATH				40020
#define LOCKBOX				40021
#define FOLLOW				40022
#define SET_STAR			40023
#define SET_MOBIUS			40024
#define ENTER				40025
#define EXIT				40026
#define CHECK_STAR			40027
#define CHECK_MOBIUS		40028
#define ELEVATOR			40029
#define START				40030
#define ANIMATION			40031
#define RUN_TO_TARGET		40032
#define RUN_TO_STAR			40033
#define ANIMATION_STAR		40034
#define	ATTACK_STAR			40035
#define MUTANT_ATTACKER		40036
#define MUTANT_ATTACKEE		40037
#define ATTACKEE_CHECK		40038
#define ATTACKER_CHECK		40039
#define ELEV_WAYPOINT		40040
#define ELEVATOR_DOWN		40041
#define ELEV_WAYPOINT2		40042
#define ACTIVATEDOWN		40043
#define STAR_STATUS			40044
#define DEAD_MUTANT			40045
#define MOBIUS_GOTO			40046
#define GOAL				40047
#define DECREMENT			40048
#define ENDED				40049
#define FLYOVER				40050
#define COLLISION			40051
#define RESEND_GOTO			40052
#define GOTO				40053
#define RESET				40054
#define MUTANT_GOTO			40055
#define DOING_ANIMATION		40056
#define M09_INNATE_ENABLE	40057
#define ATTACKING			40058
#define AMB_SOUND			40059
#define NEW_KLAXON			40060
#define AMB_EXPLOSION		40061
#define ANIM_DELAY			40062
#define INITIATION			40063
#define CLARK_KENT			40064
#define M09_CHECK_IN		40065
#define	INNATE_RECYCLE		40066
#define BAD_PATH			40067
#define DIST_CHECK			40068
#define ALREADY_ENTERED		40069
#define TIMER_RESET			40070
#define ELEVATOR_EXIT		40071
#define TOO_FAR				40072
#define NO_FOLLOW			40073
#define ON					40074
#define OFF					40075
#define MOBIUS_KILLED		40076
#define BLOCK_ON			40077
#define BLOCK_OFF			40078
#define SUIT_ANIMATION		40079

// Timer Enumerations
#define CHECK_DISTANCE			1000
#define M09_DEAD_HAVOC			1001
#define M09_MISSION_COMPLETE	1002

typedef enum
{
	M9TIMER_START = STIMER_MISSION9,
} M9TIMER;

#endif // _MISSION9_H_
