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
*     Mission5.h
*
* DESCRIPTION
*     Mission 5 definitions
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: Darren_k $
*     $Revision: 49 $
*     $Modtime: 11/19/01 2:29p $
*     $Archive: /Commando/Code/Scripts/Mission5.h $
*
******************************************************************************/

#ifndef _MISSION5_H_
#define _MISSION5_H_


// Defines and Includes

#include "Toolkit.h"



#define M05_CUSTOM_ACTIVATE			5001
#define	M05_PARK_ENGINEER_KILLED	5003
#define M05_STAR_INN				5004
#define M05_REINFORCEMENT_KILLED	5005
#define	M05_TRIANGLE_REINFORCE		5006
#define	M05_TOWNSQUARE_REINFORCE	5007
#define	M05_FREE_OVERLOOK_CAPTIVES	5008
#define	M05_OVERLOOK_REINFORCE		5009
#define	M05_CACHE_REINFORCE			5010
#define	M05_FREE_DUMP_CAPTIVES		5011
#define M05_DUMP_REINFORCE			5012
#define	M05_INN_REINFORCE			5013
#define	M05_INITIATE_CATHEDRAL		5014
#define M05_CATHEDRAL_VEHICLE_KILLED	5015
#define	M05_CATHEDRAL_REINFORCE		5016
#define	M05_ROADBLOCK_REINFORCE		5017
#define M05_PARK_UNIT_KILLED		5018
#define M05_PARK_VEHICLE_KILLED		5019
#define M05_BLOW_BUILDING			5020
#define	M05_CACHE_CIV_KILLED		5021
#define	M05_CIV_ARRIVED				5022
#define M05_PROTECT_HAVOC			5023
#define	M05_MOVE_DEADEYE			5024
#define	M05_DESTROY_OBJECT			5025
#define	M05_DEADEYE_FREED			5026
#define	M05_HEAL_DEAD6				5027
#define	M05_RELOCATE				5028
#define	M05_SWAP_ARTILLERY			5029
#define M05_CATHEDRAL_VEHICLE_CREATED	5030

// Predefined Constants

// Logical Sound Types
#define M05_PARK_ALERT			15500
#define	M05_CATHEDRAL_FREE		15501


// Public Variables

// Enumerations

// Timer Enumerations

#define	M05_DEAD_HAVOC				100500

typedef enum
{
	M5TIMER_START = STIMER_MISSION5,
} M5TIMER;

#endif // _MISSION5_H_
