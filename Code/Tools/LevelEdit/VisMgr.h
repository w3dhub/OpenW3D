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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/VisMgr.h                     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/02/00 3:26p                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif


#ifndef __VISMGR_H
#define __VISMGR_H

#include "vector.h"


/////////////////////////////////////////////////////////////////////////
//	Forward declarations
/////////////////////////////////////////////////////////////////////////
class ChunkLoadClass;
class ChunkSaveClass;
class StaticPhysClass;
class Matrix3D;
class NodeClass;


/////////////////////////////////////////////////////////////////////////
//	Typedefs
/////////////////////////////////////////////////////////////////////////
typedef bool (*VIS_POINT_RENDERED_CALLBACK) (DWORD milliseconds, DWORD_PTR param);


/////////////////////////////////////////////////////////////////////////
//
//	VisMgrClass
//
/////////////////////////////////////////////////////////////////////////
class VisMgrClass
{
	public:

		//////////////////////////////////////////////////////////
		//	Public methods
		//////////////////////////////////////////////////////////

		//
		//	Vis remapping (exporting vis from one level and importing
		// it into another).
		//
		static void		Import_Remap_Data (ChunkLoadClass &cload);
		static void		Export_Remap_Data (ChunkSaveClass &csave);

		//
		//	Vis generation
		//
		static void		Build_Node_List (DynamicVectorClass<NodeClass *> &node_list, bool selection_only = false);
		static void		Render_Manual_Vis_Points (bool farm_mode = false, int processor_index = 0, int total_processors = 1, VIS_POINT_RENDERED_CALLBACK callback = NULL, DWORD_PTR param = 0);

	protected:

		//////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////
		
		//
		//	Utility methods
		//
		static StaticPhysClass *	Find_Static_Phys_Object (LPCTSTR mesh_name, const Matrix3D &tm);
		static void						Add_Nodes_To_List (DynamicVectorClass<NodeClass *> &node_list, NodeClass *node);


	private:

		//////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////
};


#endif //__VISMGR_H
