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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/NodeCategories.h             $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/23/01 1:09p                                               $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __NODE_CATEGORIES_H
#define __NODE_CATEGORIES_H

#include "definitionclassids.h"
#include "icons.h"
#include "editorchunkids.h"
#include "combatchunkid.h"


////////////////////////////////////////////////////////////////////////////////////
//
//	Structures and tables
//
////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	const char *name;
	int			clsid;
	int			icon;
} CategoryInfoStruct;

const CategoryInfoStruct PRESET_CATEGORIES[] = 
{
	{ "Terrain",			CLASSID_TERRAIN,				TERRAIN_ICON },
	{ "Tile",				CLASSID_TILE,					TILE_ICON },
	{ "Object",				CLASSID_GAME_OBJECTS,		OBJECT_ICON },
	{ "Buildings",			CLASSID_BUILDINGS,			BUILDING_ICON },
	{ "Munitions",			CLASSID_MUNITIONS,			OBJECT_ICON },	
	{ "Dummy Object",		CLASSID_DUMMY_OBJECTS,		OBJECT_ICON },	
	{ "Cover Spots",		CLASSID_COVERSPOT,			OBJECT_ICON },	
	{ "Light",				CLASSID_LIGHT,					LIGHT_ICON },
	{ "Sound",				CLASSID_SOUND,					SOUND_ICON },	
	{ "Waypath",			CLASSID_WAYPATH,				WAYPATH_ICON },
	{ "Twiddlers",			CLASSID_TWIDDLERS,			RAND_ICON },
	{ "Editor Objects",	CLASSID_EDITOR_OBJECTS,		VIS_ICON },
	{ "Global Settings",	CLASSID_GLOBAL_SETTINGS,	OBJECT_ICON },	
};

const int PRESET_CATEGORY_COUNT = sizeof (PRESET_CATEGORIES) / sizeof (CategoryInfoStruct);


#endif //__NODE_CATEGORIES_H
