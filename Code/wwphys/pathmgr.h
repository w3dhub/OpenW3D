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
 *                 Project Name : WWPhys																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/pathmgr.h           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/08/01 6:07p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __PATHMGR_H
#define __PATHMGR_H


#include "vector.h"
#include "vector3.h"
#include "bittype.h"


/////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////
class PathSolveClass;
class ChunkSaveClass;
class ChunkLoadClass;


/////////////////////////////////////////////////////////////////////////
//
//	PathMgrClass
//
/////////////////////////////////////////////////////////////////////////
class PathMgrClass
{
public:

	/////////////////////////////////////////////////////////////////////////
	// Public methods
	/////////////////////////////////////////////////////////////////////////		
	
	//
	//	Initialization
	//
	static void						Initialize (void);
	static void						Shutdown (void);

	//
	//	Path resolution
	//
	static void						Resolve_Paths (const Vector3 &camera_pos, uint32 milliseconds = 5);
	static PathSolveClass *		Peek_Active_Path (void)	{ return ActivePath; }

	//
	//	Save/Load
	//
	static void						Save (ChunkSaveClass &csave);
	static void						Load (ChunkLoadClass &cload);

	//
	//	Path management
	//
	static PathSolveClass *		Request_Path_Object (void);
	static void						Return_Path_Object (PathSolveClass *path);

private:

	/////////////////////////////////////////////////////////////////////////
	// Private methods
	/////////////////////////////////////////////////////////////////////////
	static void						Allocate_Objects (void);
	static void						Free_Objects (void);
	static void						Activate_New_Priority_Path (const Vector3 &camera_pos);

	/////////////////////////////////////////////////////////////////////////
	// Private member data
	/////////////////////////////////////////////////////////////////////////
	static DynamicVectorClass<PathSolveClass *>	AvailablePathList;
	static DynamicVectorClass<PathSolveClass *>	UsedPathList;
	static PathSolveClass *								ActivePath;
	static int64_t											TicksPerMilliSec;
};


#endif //__PATHMGR_H
