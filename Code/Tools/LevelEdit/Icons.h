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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/Icons.h                      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/25/01 3:45p                                               $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif


#ifndef __ICONS_H
#define __ICONS_H

#include "Resource.H"

///////////////////////////////////////////////////////////////
//
//	Constants
//
typedef enum
{
	FOLDER_ICON			= 0,
	OPEN_FOLDER_ICON,
	OBJECT_ICON,
	TILE_ICON,
	TERRAIN_ICON,
	LIGHT_ICON,
	ZONE_ICON,
	TRANSITION_ICON,
	WAYPATH_ICON,
	SOUND_ICON,
	TEMP_ICON,
	FILE_ICON,
	FILES_ICON,
	VIS_ICON,
	PATHFIND_ICON,
	NAVIGATOR_ICON,
	PLUS_OVERLAY_ICON,
	BUILDING_ICON,
	NULL_ICON,
	RAND_ICON,
	DIALOGUE_ICON,
	ICON_COUNT

} ICON_INDEX;

extern const UINT ICON_RESOURCE_IDS[ICON_COUNT];

#endif //__ICONS_H

