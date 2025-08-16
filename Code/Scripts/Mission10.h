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
*     Mission10.h
*
* DESCRIPTION
*     Mission 10 definitions
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: Dan_e $
*     $Revision: 23 $
*     $Modtime: 12/06/01 11:43a $
*     $Archive: /Commando/Code/Scripts/Mission10.h $
*
******************************************************************************/

#ifndef _MISSION10_H_
#define _MISSION10_H_


// Defines and Includes

#include "Toolkit.h"


// Predefined Constants

// Public Variables

// Enumerations
#define MAMMOTH							40000
#define ATTACK							40001
#define KILLED							40002
#define TARGET							40003
#define HON								40004
#define GRANT							40005
#define	HELIPAD							40006
#define GOTO_LOC						40007
#define FAKE_TIMER						40008
#define HAVOCS_SCRIPT					40009
#define MISSION_COMPLETE				40010
#define REBUILD							40011
#define M10_PLAYERTYPE_CHANGE_OBELISK	40012
#define FLYOVER							40013
#define	REMOVE_SECONDARY_POG			40014
#define ENTERED							40015
#define CLEAR1							40016
#define CLEAR2							40017
#define GRANT_MRLS						40018
#define OCCUPIED						40019
#define TANK_KILLED						40020
#define CARGO_DROP						40021
#define REINFORCE						40022
#define NO_DROP							40023
#define KEY_OBJ							40024
#define DAMAGED							40025
#define RESET							40026
#define KANE_CONV						40027
#define MAMMY							40028

// Timer Enumerations

typedef enum
{
	M10TIMER_START = STIMER_MISSION10,
} M10TIMER;

#endif // _MISSION10_H_
