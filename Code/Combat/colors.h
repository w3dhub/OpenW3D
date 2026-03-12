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
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/colors.h                              $*
 *                                                                                             *
 *                      $Author:: Denzil_l                                                    $*
 *                                                                                             *
 *                     $Modtime:: 11/07/01 7:53p                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef COLORS_H
#define COLORS_H

#include "vector3.h"

const Vector3 COLOR_TEAM_0				= Vector3(1.0f, 0.0f, 0.0f);
const Vector3 COLOR_TEAM_1				= Vector3(1.0f, 0.8f, 0.0f);
const Vector3 COLOR_NEUTRAL			= Vector3(0.8f, 0.8f, 0.8f);
const Vector3 COLOR_RENEGADE			= Vector3(1.0f, 1.0f, 1.0f);
const Vector3 COLOR_PUBLIC_TEXT		= Vector3(1.0f, 1.0f, 1.0f);
const Vector3 COLOR_PRIVATE_TEXT		= Vector3(0.0f, 0.0f, 1.0f);
const Vector3 COLOR_PAGED_TEXT		= Vector3(0.0f, 1.0f, 1.0f);
const Vector3 COLOR_INVITE_TEXT		= Vector3(1.0f, 0.0f, 1.0f);
const Vector3 COLOR_CONSOLE_TEXT		= Vector3(1.0f, 1.0f, 1.0f);

Vector3 Get_Color_For_Team(int team);

#endif // COLORS_H











//const Vector3 COLOR_KOTH				= Vector3(0.2f, 1.0f, 0.2f);
//const Vector3 COLOR_CHAMPION			= Vector3(0.0f, 1.0f, 1.0f);
