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
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Installer                                                    * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Installer/SafeTimer.h  $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 11/05/01 7:41p                $* 
 *                                                                                             * 
 *                    $Revision:: 2                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef _SAFE_TIMER_H
#define _SAFE_TIMER_H


// Includes.
#include "win.h"
#include <mmsystem.h>


// This timer increments at 1000Hz, is DWORD size and uses the OS timer timeGetTime().
// Therefore it will wrap ~49 days after boot time. This class attempts to minimize
// the possibility of the timer wrapping by offsetting time from the point at which
// the application starts up.	It will still, however, fail if the application is
// active for > ~49 days.

class SafeTimerClass
{
	public:
		int operator () (void) const;
		operator int (void) const;

	private:
		static DWORD _StartTime;
};


inline int SafeTimerClass::operator () (void) const
{
	DWORD time = timeGetTime();

	// Has the timer wrapped? 
	if (time >= _StartTime) {
		return (time - _StartTime);
	} else {
		return ((0xffffffff - _StartTime) + time);
	}
}


inline SafeTimerClass::operator int (void) const
{
	DWORD time = timeGetTime();

	// Has the timer wrapped? 
	if (time >= _StartTime) {
		return (time - _StartTime);
	} else {
		return ((0xffffffff - _StartTime) + time);
	}
}


#endif // _SAFE_TIMER_H