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
 *                     $Archive:: /Commando/Code/Combat/assetdep.h                            $*
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 4/20/00 5:59p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __ASSET_DEP_H
#define __ASSET_DEP_H


#include "vector.h"


///////////////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////////////
class ChunkSaveClass;
class ChunkLoadClass;
class StringClass;


///////////////////////////////////////////////////////////////////////
//
//	AssetDependencyManager
//
///////////////////////////////////////////////////////////////////////
class AssetDependencyManager
{
public:

	////////////////////////////////////////////////////////////////////
	//	Public data types
	////////////////////////////////////////////////////////////////////
	typedef DynamicVectorClass<StringClass>	ASSET_LIST;


	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	static void				Save_Always_Dependencies (const char *path, ASSET_LIST &asset_list);
	static void				Save_Level_Dependencies (const char *full_path, ASSET_LIST &asset_list);
	static void				Save_Dependencies (ChunkSaveClass &csave, ASSET_LIST &asset_list);

	static void				Load_Level_Assets (const char *level_name);
	static void				Load_Always_Assets (void);
	static void				Load_Assets (const char *filename);
	static void				Load_Assets (ChunkLoadClass &cload);
};


#endif //__ASSET_DEP_H
