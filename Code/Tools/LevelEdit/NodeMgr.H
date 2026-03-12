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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/NodeMgr.H                    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/11/01 9:41a                                               $*
 *                                                                                             *
 *                    $Revision:: 32                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __NODEMGR_H
#define __NODEMGR_H

#include "saveloadsubsystem.h"
#include "vector.h"
#include "bittype.h"
#include "nodetypes.h"
#include "listtypes.h"


//////////////////////////////////////////////////////////////////////////
// Forward declarations
//////////////////////////////////////////////////////////////////////////
class NodeClass;
class PresetClass;
class ChunkLoadClass;
class ChunkSaveClass;


//////////////////////////////////////////////////////////////////////////
// Singleton instance
//////////////////////////////////////////////////////////////////////////
extern class NodeMgrClass _TheNodeMgr;


//////////////////////////////////////////////////////////////////////////
//
//	NodeMgrClass
//
//////////////////////////////////////////////////////////////////////////
class NodeMgrClass : public SaveLoadSubSystemClass
{
public:

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	NodeMgrClass (void) { }
	virtual ~NodeMgrClass (void);

	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////

	// From SaveLoadSubSystemClass
	virtual uint32				Chunk_ID (void) const;
	virtual void				On_Post_Load (void);
	virtual const char *		Name (void) const			{ return "NodeMgrClass"; }

	// List management
	static NodeClass *		Create_Node (PresetClass *preset, uint32 id);
	static void					Add_Node (NodeClass *node);
	static void					Remove_Node (NodeClass *node);
	static void					Free_Nodes (void);
	static void					Reload_Nodes (void);
	static void					Reload_Nodes (PresetClass *preset);

	// Enumeration
	static NodeClass *		Find_Node (uint32 id);
	static NodeClass *		Find_Node (const char *name);
	static NodeClass *		Get_First (void);
	static NodeClass *		Get_First (NODE_TYPE type);
	static NodeClass *		Get_First (PresetClass *preset);
	static NodeClass *		Find_First (uint32 class_id);
	static NodeClass *		Get_Next (NodeClass *current);
	static NodeClass *		Get_Next (NodeClass *current, NODE_TYPE type);
	static NodeClass *		Get_Next (NodeClass *current, PresetClass *preset);
	static NodeClass *		Find_Next (NodeClass *current, uint32 class_id);

	// Identification
	static void					Reset_New_ID (void);
	static uint32				Get_Node_ID (NODE_TYPE type);
	static uint32				Get_Max_Used_ID (void);
	static void					Setup_Node_Identity (NodeClass &node);
	static bool					Verify_Unique_ID(uint32 id);
	static void					Build_ID_Collision_List (NODE_LIST &node_list);
	static void					Build_Unimportant_ID_Collision_List (NODE_LIST &node_list);
	static void					Get_ID_Range (NODE_TYPE type, uint32 *min_id, uint32 *max_id);
	static uint32				Find_Max_Used_ID (NODE_TYPE type);

	static void					Put_Objects_Back (const NODE_LIST &dynamic_obj_list);
	static void					Remove_Dynamic_Objects (NODE_LIST &dynamic_obj_list);
	static void					Remove_Static_Objects (NODE_LIST &dynamic_obj_list);

	// StaticAnimPhys control
	static void					Make_Static_Anim_Phys_Collideable (bool onoff);

	// Complete node access
	static void					Build_Full_Node_List (DynamicVectorClass<NodeClass *> &node_list);
	static void					Add_Nodes_To_List (DynamicVectorClass<NodeClass *> &node_list, NodeClass *node);

	//
	//	Save/load support
	//
	static bool					Save_Node_List (ChunkSaveClass &csave, NODE_LIST &node_list);
	static bool					Load_Node_List (ChunkLoadClass &cload, NODE_LIST &node_list);

	//
	//	Embedded node support
	//
	static void					Create_All_Embedded_Nodes (void);

protected:

	//////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////
	virtual bool				Contains_Data(void) const;
	virtual bool				Save (ChunkSaveClass &csave);
	virtual bool				Load (ChunkLoadClass &cload);

	bool							Save_Nodes (ChunkSaveClass &csave);
	bool							Load_Nodes (ChunkLoadClass &cload);
	bool							Load_Variables (ChunkLoadClass &cload);

private:

	/////////////////////////////////////////////////////////////////////
	//	Private methods
	/////////////////////////////////////////////////////////////////////
	static void					Link_Node (NodeClass *node);
	static void					Unlink_Node (NodeClass *node);

	/////////////////////////////////////////////////////////////////////
	//	Static member data
	/////////////////////////////////////////////////////////////////////
	static NodeClass *		_NodeListHead;
	static uint32				_NextObjectNodeID;
	static uint32				_NextStaticNodeID;
	static uint32				_NextLightNodeID;
	static uint32				_NextMiscNodeID;
};


/////////////////////////////////////////////////////////////////////
//	Get_First
/////////////////////////////////////////////////////////////////////
inline NodeClass  *
NodeMgrClass::Get_First (void)
{
	return _NodeListHead;
}


#endif //__NODEMGR_H

