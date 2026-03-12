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
 *                     $Archive:: /Commando/Code/Installer/CodeControl.h $*
 *                                                                                             *
 *                      $Author:: Ian_l                   $*
 *                                                                                             *
 *                     $Modtime:: 1/14/02 9:27p                 $*
 *                                                                                             *
 *                    $Revision:: 5                     $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _CODE_CONTROL_H
#define _CODE_CONTROL_H

// Code controlling defines.
// NOTE: Set ALL of the following defines to zero in the final release build.

#define USE_QA_SERIAL_NOS				0	// Use a set of nos. for QA testing (as opposed to serial nos. for general release).
#define PERMIT_INSTALL_FROM_NETWORK	0	// Let users install from any drive (if -N is on command line).
#define VERBOSE_ERROR_MESSAGES		0	// Add filename and line no. to fatal error messages.

#endif // _CODE_CONTROL_H

