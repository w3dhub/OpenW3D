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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/EditorChunkIDs.h             $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/05/02 3:25p                                               $*
 *                                                                                             *
 *                    $Revision:: 22                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __EDITOR_CHUNK_IDS_H
#define __EDITOR_CHUNK_IDS_H

#include "saveloadids.h"
#include "definitionclassids.h"


//////////////////////////////////////////////////////////////////////////////////
//
//	These are the chunk IDs that serve as 'globally-unique' persist identifiers for
//	all persist objects inside the editor.  These are used when building the
//	PersistFactoryClass's for definitions.
//
//////////////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_TERRAIN_DEF	= CHUNKID_COMMANDO_EDITOR_BEGIN,
	CHUNKID_TILE_DEF,
	CHUNKID_LIGHT_DEF,
	CHUNKID_WAYPATH_DEF,
	CHUNKID_EDITOR_ZONE_DEF,
	CHUNKID_TRANSITION_DEF,
	CHUNKID_EDITOR_PHYS,
	CHUNKID_PRESET,
	CHUNKID_PRESETMGR,
	CHUNKID_NODEMGR,
	CHUNKID_NODE_TERRAIN,
	CHUNKID_NODE_TILE,
	CHUNKID_NODE_OBJECTS,
	CHUNKID_NODE_LIGHT,
	CHUNKID_NODE_OLD_SOUND,
	CHUNKID_NODE_WAYPATH,
	CHUNKID_NODE_ZONE,
	CHUNKID_NODE_TRANSITION,
	CHUNKID_EDITOR_SAVELOAD,
	CHUNKID_XXX,
	CHUNKID_NODE_TERRAIN_SECTION,
	CHUNKID_NODE_VIS_POINT,
	CHUNKID_VIS_POINT_DEF,
	CHUNKID_NODE_PATHFIND_START,
	CHUNKID_PATHFIND_START_DEF,
	CHUNKID_DUMMY_OBJECT_DEF,
	CHUNKID_DUMMY_OBJECT,
	CHUNKID_NODE_WAYPOINT,
	CHUNKID_COVERSPOT_DEF,
	CHUNKID_NODE_COVER_ATTACK_POINT,
	CHUNKID_NODE_COVER_SPOT,
	CHUNKID_NODE_DAMAGE_ZONE,
	XXX_CHUNKID_NODE_BUILDING,
	CHUNKID_NODE_SPAWN_POINT,
	CHUNKID_NODE_SPAWNER,
	CHUNKID_NODE_BUILDING,
	CHUNKID_NODE_BUILDING_CHILD,
	CHUNKID_NODE_NEW_SOUND,
	CHUNKID_EDITOR_ONLY_OBJECTS_DEF,
	CHUNKID_EDITOR_ONLY_OBJECTS,
	CHUNKID_EDITOR_PATHFIND_IMPORTER_EXPORTER,
	CHUNKID_EDITOR_LIGHT_SOLVE_SAVELOAD,
	CHUNKID_HEIGHTFIELD_MGR
};


//////////////////////////////////////////////////////////////////////////////////
//
//	These are the globally-unique class identifiers that the definition system
// uses inside the editor.
//
//////////////////////////////////////////////////////////////////////////////////
enum
{
	CLASSID_EDITOR_START				= CLASSID_EDITOR_OBJECTS + 1,
	CLASSID_VIS_POINT_DEF,
	CLASSID_PATHFIND_START_DEF,
	CLASSID_LIGHT_DEF,
	CLASSID_COVERSPOT,
	CLASSID_EDITOR_ONLY_OBJECTS
};

#endif //__EDITOR_CHUNK_IDS_H
