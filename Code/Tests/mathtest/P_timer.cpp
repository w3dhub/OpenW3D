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

/* $Header: /Commando/Code/Tests/mathtest/P_timer.cpp 2     7/22/97 1:14p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Voxel Technology                                             *
 *                                                                                             *
 *                    File Name : P_TIMER.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Greg Hjelstrom                                               *
 *                                                                                             *
 *                   Start Date : 02/24/97                                                     *
 *                                                                                             *
 *                  Last Update : February 24, 1997 [GH]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "P_timer.h"

#if defined(OPENW3D_WIN32)
#include <windows.h>
#elif defined(OPENW3D_SDL3)
#include <SDL3/SDL_timer.h>
#endif

uint64_t Get_CPU_Clock ( void )
{
#if defined(OPENW3D_WIN32)
	LARGE_INTEGER LargeInt;

	if (QueryPerformanceFrequency(&LargeInt)) {
		return(LargeInt.QuadPart);
	}
	return 0;
#elif defined(OPENW3D_SDL3)
	return SDL_GetPerformanceCounter();
#else
	assert(0);
#endif
}
