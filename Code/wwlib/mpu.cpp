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
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/mpu.cpp                                $*
 *                                                                                             *
 *                      $Author:: Denzil_l                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/23/01 5:07p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Get_CPU_Rate -- Fetch the rate of CPU ticks per second.                                   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"mpu.h"
#include "math.h"

#if defined(OPENW3D_WIN32)
#include <windows.h>
#elif defined(OPENW3D_SDL3)
#include <SDL3/SDL_timer.h>
#endif

/***********************************************************************************************
 * Get_CPU_Rate -- Fetch the rate of CPU ticks per second.                                     *
 *                                                                                             *
 *    This routine will query the CPU to determine how many clock per second it is.            *
 *                                                                                             *
 * OUTPUT:  Returns the result.                                                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/20/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
uint64_t Get_CPU_Rate()
{
#if defined(OPENW3D_WIN32)
	LARGE_INTEGER LargeInt;

	if (QueryPerformanceFrequency(&LargeInt)) {
		return(LargeInt.QuadPart);
	}
	return 0;
#elif defined(OPENW3D_SDL3)
	return SDL_GetPerformanceFrequency();
#else
	assert(0);
#endif
}


uint64_t Get_CPU_Clock()
{
#if defined(OPENW3D_WIN32)
	LARGE_INTEGER LargeInt;
	if (QueryPerformanceCounter(&LargeInt)) {
		return LargeInt.QuadPart;
	}
	return 0;
#elif defined(OPENW3D_SDL3)
	return SDL_GetPerformanceCounter();
#else
	assert(0);
#endif
}
