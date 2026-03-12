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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/vistablemgr.h                         $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/16/00 7:30p                                              $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef VISTABLEMGR_H
#define VISTABLEMGR_H

#include "always.h"
#include "simplevec.h"

class VisTableClass;
class CompressedVisTableClass;
class ChunkSaveClass;
class ChunkLoadClass;
class VisDecompressionCacheClass;


/*
** VisTableMgrClass
** This class collects the visibility tables and vis-id allocation into one place.
** The physics scene contains an instance of this class and will save and load it as
** part of the (L)evel (S)tatic (D)ata file...
*/
class VisTableMgrClass
{

public:

	VisTableMgrClass(void);
	~VisTableMgrClass(void);

	/*
	** Allocation of vis-object and vis-sector IDs
	*/
	void								Reset();
	int								Allocate_Vis_Object_ID(int count = 1);
	int								Allocate_Vis_Sector_ID(int count = 1);
	int								Get_Vis_Table_Size(void) const;
	int								Get_Vis_Table_Count(void) const;
	void								Set_Optimized_Vis_Object_Count(int count);

	/*
	** Access to the actual vis data.  Decompressed tables are cached
	** behind the scenes so just call this whenever you want a vis table.
	** DO NOT EVER modify a vis table you get from this function.  You should
	** always make a copy, modify the copy, then call Update_Vis_Table if
	** you need to modify the contents of a vis table...
	*/
	VisTableClass *				Get_Vis_Table(int id,bool allocate = false);
	void								Update_Vis_Table(int id,VisTableClass * new_pvs);
	bool								Has_Vis_Table(int id);

	/*
	** Pulse this once per frame to purge the LRU cache
	*/
	void								Notify_Frame_Ended(void);

	/*
	** Save/Load interface
	*/
	void								Save(ChunkSaveClass & csave);
	void								Load(ChunkLoadClass & cload);

protected:

	int								VisSectorCount;
	int								VisObjectCount;

	void								Delete_All_Vis_Tables(void);

	SimpleDynVecClass<CompressedVisTableClass *>		VisTables;
	VisDecompressionCacheClass *							Cache;
	unsigned int												FrameCounter;
};


#endif
