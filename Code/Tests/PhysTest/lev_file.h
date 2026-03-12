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
 *                 Project Name : Combat                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/lev_file.h                          $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 3/08/00 8:16p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LEV_FILE_H
#define LEV_FILE_H

#if 0 //OBSOLETE!!!

/*

  LEV File Format.

  This is another chunk-based file format which will initially describe the map for
  a commando level and may eventually expand to include the description of the entire
  level.  Due to the structure of our code, this file format will define several
  "wrapper" chunks whose contents are defined in another header file in the physics
  library (physchunks.h).  Game code will make calls to the physics library when it
  encounters those chunks.

  The chunk 'LEV_CHUNK_STATIC_SCENE' can be handled by the physics scene's Load_Static_Scene
  function and the 'LEV_CHUNK_DYNAMIC_SCENE' can be handled by the physics scene's
  Load_Dynamic_Scene function.  The definitions for the data inside those chunks can be
  found inside the header file 'physchunks.h'

*/

enum {

	LEV_CHUNK_LEVEL						= 0x00000000,	// wraps entire level definition

		LEV_CHUNK_MAP						= 0x00000100,	// wraps the map definition
			LEV_CHUNK_STATIC_SCENE		= 0x00000102,	// wraps physics definition of static objs (terrain)
			LEV_CHUNK_STATIC_GAMEOBJS	= 0x00000103,	// game object definitions which link to above phys objs

		LEV_CHUNK_OBJECTS					= 0x00000200,	// wraps defintion of all of the dynamic objects
			LEV_CHUNK_DYNAMIC_SCENE		= 0x00000201,	// wraps physics definition of dynamic objs
			LEV_CHUNK_DYNAMIC_GAMEOBJS	= 0x00000202,	// game obj definitions which link to above phys objs

};

#endif 0 // OBSOLETE!!!

#endif

