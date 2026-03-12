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

/* $Header: /Commando/Code/Tests/MeshTest/_GLOBALS.H 5     5/01/98 11:04a Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tests/MeshTest/_GLOBALS.H                    $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/30/98 2:33p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef _GLOBALS_H
#define _GLOBALS_H

#ifndef ALWAYS_H
#include "always.h"
#endif

#ifndef KEYBOARD_H
#include "keyboard.h"
#endif

#ifndef XMOUSE_H
#include "xmouse.h"
#endif

#ifndef STIMER_H
#include "stimer.h"
#endif

extern bool						GameInFocus;
extern Mouse *					MouseCursor;
extern WWKeyboardClass *	Keyboard;

extern int						Argc;
extern char * 					Argv[20];


#if 0
	//   60 Hz Timer
	extern	SystemTimerClass		SystemTimer;
	#define	SYSTEM_TIMER_RATE		TIMER_SECOND
#else
	//		1 KHz Timer
	#define	SystemTimer()			(int)timeGetTime()
	#define	SYSTEM_TIMER_RATE		1000
#endif

#endif
