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
 *                     $Archive:: /Commando/Code/Commando/nettgas.h                           $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 9/12/00 1:36p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if defined(_MSV_VER)
#pragma once
#endif

#include "bittype.h"

//
// This is a storing house for ingame tga's relating to networking
// and multiplayer.
// The const preceding each LPCSTR permits inclusion in multiple source files.
//
// Purposes:
// - to maintain a list of multiplay tga's for creation by the sound guys.
// - the usual purposes for using defines
// - ability to jump to these effects in the code.
//

#ifndef NETTGAS_H
#define NETTGAS_H

const LPCSTR NETTGA_SMALL_FONT				= "FONT6x8.TGA";
const LPCSTR NETTGA_MEDIUM_FONT				= "FONT9x12.TGA";
const LPCSTR NETTGA_BIG_FONT					= "FONT12x16.TGA";
const LPCSTR NETTGA_YOU_INDICATOR			= "you.tga";

#endif // NETTGAS_H








//const LPCSTR NETTGA_SPECTATABLE_INDICATOR	= "specable.tga";
//const LPCSTR NETTGA_SPECTATING_EYE			= "eye.tga";

