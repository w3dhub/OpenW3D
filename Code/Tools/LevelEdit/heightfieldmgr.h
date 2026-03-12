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
 *                 Project Name : leveledit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/heightfieldmgr.h             $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/07/02 1:53p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __HEIGHTFIELDMGR_H
#define __HEIGHTFIELDMGR_H

#include "vector.h"
#include "saveloadsubsystem.h"
#include "editorchunkids.h"


///////////////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////////////
class EditableHeightfieldClass;
class StaticPhysClass;

///////////////////////////////////////////////////////////////////////
// Global singleton instance
///////////////////////////////////////////////////////////////////////
extern class HeightfieldMgrClass _TheHeightfieldMgrSaveLoadSubsystem;


//////////////////////////////////////////////////////////////////////
//
//	HeightfieldMgrClass
//
//////////////////////////////////////////////////////////////////////
class HeightfieldMgrClass : public SaveLoadSubSystemClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Initialization support
	//
	static void				Initialize (void);
	static void				Shutdown (void);

	//
	//	Creation
	//
	static EditableHeightfieldClass *	Create_Heightfield (const char *heightmap_filename, float width, float height, float density, float scale);
	static EditableHeightfieldClass *	Create_Heightfield (float width, float height, float density);

	//
	//	Information
	//
	static int				Get_Heightfield_Count (void)	{ return HeightfieldList.Count (); }

	//
	//	ID Support
	//
	static void				Assign_Unique_IDs (void);

	//
	//	From SaveLoadSubSystemClass
	//
	uint32					Chunk_ID (void) const			{ return CHUNKID_HEIGHTFIELD_MGR; }

protected:

	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////

	//
	//	From SaveLoadSubSystemClass
	//
	bool						Save (ChunkSaveClass &csave);
	bool						Load (ChunkLoadClass &cload);
	void						On_Post_Load (void);
	const char *			Name (void) const					{ return "HeightfieldMgrClass"; }

	//
	//	Save load support
	//
	void						Load_Variables (ChunkLoadClass &cload);

private:

	///////////////////////////////////////////////////////////////////
	//	Private methods
	///////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////
	static DynamicVectorClass<EditableHeightfieldClass *>	HeightfieldList;
};


#endif //__HEIGHTFIELDMGR_H
