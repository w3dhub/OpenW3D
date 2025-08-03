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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/NodeMgr.cpp                  $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/07/02 1:54p                                               $*
 *                                                                                             *
 *                    $Revision:: 42                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "NodeMgr.H"
#include "node.h"
#include "editorchunkids.h"
#include "preset.h"
#include "wwdebug.h"
#include "chunkio.h"
#include "persistfactory.h"
#include "sceneeditor.h"
#include "LevelEditView.h"
#include "instancespage.h"
#include "gameobjmanager.h"
#include "terrainnode.h"
#include "LevelEdit.h"
#include "regkeys.h"
#include "presetmgr.h"
#include "presetremapdialog.h"
#include "heightfieldmgr.h"


///////////////////////////////////////////////////////////////////////
// Singleton instance
///////////////////////////////////////////////////////////////////////
NodeMgrClass _TheNodeMgr;


///////////////////////////////////////////////////////////////////////
//	Constants
///////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES			= 0x00000100,
	CHUNKID_NODES
};


enum
{
	XXX_VARID_NEXT_NODE_ID		= 0x01,
};


enum
{
	FIRST_OBJECT_NODE_ID		= 100000,
	FIRST_STATIC_NODE_ID		= 1100000000,
	FIRST_LIGHT_NODE_ID		= 1200000000,
	FIRST_MISC_NODE_ID		= 1300000000,
};


///////////////////////////////////////////////////////////////////////
// Static member initialization
///////////////////////////////////////////////////////////////////////
NodeClass * NodeMgrClass::_NodeListHead	= NULL;

uint32 NodeMgrClass::_NextObjectNodeID		= FIRST_OBJECT_NODE_ID;
uint32 NodeMgrClass::_NextStaticNodeID		= FIRST_STATIC_NODE_ID;
uint32 NodeMgrClass::_NextLightNodeID		= FIRST_LIGHT_NODE_ID;
uint32 NodeMgrClass::_NextMiscNodeID		= FIRST_MISC_NODE_ID;


//////////////////////////////////////////////////////////////////////////////////////////
//
//	~NodeMgrClass
//
//////////////////////////////////////////////////////////////////////////////////////////
NodeMgrClass::~NodeMgrClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////////////////
//
//	Chunk_ID
//
//////////////////////////////////////////////////////////////////////////////////////////
uint32
NodeMgrClass::Chunk_ID (void) const
{
	return CHUNKID_NODEMGR;
}


//////////////////////////////////////////////////////////////////////////////////////////
//
//	Setup_Node_Identity
//
//////////////////////////////////////////////////////////////////////////////////////////
void
NodeMgrClass::Setup_Node_Identity (NodeClass &node)
{
	PresetClass *preset	= node.Get_Preset ();
	int id					= node.Get_ID ();

	//
	//	Give the node an ID if it doesn't already
	// have one.
	//
	if (id == 0) {
		id = Get_Node_ID (node.Get_Type ());
		node.Set_ID (id);
	}	

	//
	//	Give the node a name
	//
	if (preset !=  NULL) {
		CString name;
		name.Format ("%s.%d", preset->Get_Name (), id);
		node.Set_Name (name);
	} else {
		
		RenderObjClass *model = node.Peek_Render_Obj ();
		if (model != NULL) {
			CString name;
			name.Format ("%s.%d", model->Get_Name (), id);
			node.Set_Name (name);		
		} else {
			CString name;
			name.Format ("Unknown.%d", id);
			node.Set_Name (name);		
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Node
//
//////////////////////////////////////////////////////////////////////////////////////////
NodeClass *
NodeMgrClass::Create_Node (PresetClass *preset, uint32 id)
{
	NodeClass *node = preset->Create ();
	if (node != NULL) {

		//
		//	Give the new node a unique id and name
		//
		node->Set_ID (id);
		Setup_Node_Identity (*node);

		//
		//	Give the new node a chance to initialize its data
		//
		node->Initialize ();

		//
		//	Keep track of this node
		//
		Add_Node (node);
	}

	return node;
}


//////////////////////////////////////////////////////////////////////////////////////////
//
//	Add_Node
//
//////////////////////////////////////////////////////////////////////////////////////////
void
NodeMgrClass::Add_Node (NodeClass *node)
{
	WWASSERT (node->m_NextNode == 0);
	WWASSERT (node->m_PrevNode == 0);
	
	//
	//	Add the node to the the list and lock
	// a refcount on the node
	//
	if (	node->m_NextNode == NULL &&
			node->m_PrevNode == NULL &&
			node != _NodeListHead)
	{
		Link_Node (node);
		SAFE_ADD_REF (node);

		//
		//	Update the UI
		//
		InstancesPageClass *instances_form = ::Get_Instances_Form ();
		if (instances_form != NULL) {
			instances_form->Add_Node (node);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////////////////
//
//	Remove_Node
//
//////////////////////////////////////////////////////////////////////////////////////////
void
NodeMgrClass::Remove_Node (NodeClass *node)
{
	WWASSERT (node != 0);

	if (	node->m_NextNode != NULL ||
			node->m_PrevNode != NULL ||
			node == _NodeListHead)
	{
		//
		//	Remove the node from our list and
		//	free our hold on the node
		//	
		Unlink_Node (node);		

		//
		//	Update the UI
		//
		InstancesPageClass *instances_form = ::Get_Instances_Form ();
		if (instances_form != NULL) {
			instances_form->Remove_Node (node);
		}

		MEMBER_RELEASE (node);
	}
	
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Link_Node
//
////////////////////////////////////////////////////////////////////////////
void
NodeMgrClass::Link_Node (NodeClass *node)
{
	WWASSERT (node->m_NextNode == 0);
	WWASSERT (node->m_PrevNode == 0);

	// Add this preset in front of the current head of the list
	node->m_NextNode = _NodeListHead;
	
	// If the list wasn't empty, link the next definiton back to this node
	if (node->m_NextNode != 0) {
		node->m_NextNode->m_PrevNode = node;
	}

	// Point the head of the list at this node now
	_NodeListHead = node;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Unlink_Node
//
////////////////////////////////////////////////////////////////////////////
void
NodeMgrClass::Unlink_Node (NodeClass *node)
{
	WWASSERT(node != 0);

	// Handle the node's prev pointer:
	if (node->m_PrevNode == 0) {

		// this node is the head
		WWASSERT (_NodeListHead == node);
		_NodeListHead = node->m_NextNode;
	
	} else {

		// link it's prev with it's next
		node->m_PrevNode->m_NextNode = node->m_NextNode;

	}

	// Handle the node's next pointer if its not at the end of the list:
	if (node->m_NextNode != 0) {		
		node->m_NextNode->m_PrevNode = node->m_PrevNode;
	}

	// The node is now un-linked
	node->m_NextNode = 0;
	node->m_PrevNode = 0;
	return ;
}


//////////////////////////////////////////////////////////////////////////////////////////
//
//	Free_Nodes
//
//////////////////////////////////////////////////////////////////////////////////////////
void
NodeMgrClass::Free_Nodes (void)
{
	NodeClass *curr_node = NULL;
	NodeClass *next_node = NULL;
	InstancesPageClass *instances_form = ::Get_Instances_Form ();

	//
	//	Unlink the nodes
	//
	DynamicVectorClass<NodeClass *> node_list;
	for (curr_node = _NodeListHead; curr_node != NULL; curr_node = next_node) {
		next_node = curr_node->m_NextNode;
		curr_node->m_PrevNode = NULL;
		curr_node->m_NextNode = NULL;
		node_list.Add (curr_node);
	}

	//
	//	Free the nodes
	//
	_NodeListHead = NULL;
	for (int index = 0; index < node_list.Count (); index ++) {
		MEMBER_RELEASE (node_list[index]);
	}	
	
	//
	//	Reset the node ID ranges
	//
	uint32 foo = 0;
	Get_ID_Range (NODE_TYPE_OBJECT,		&_NextObjectNodeID,	&foo);
	Get_ID_Range (NODE_TYPE_TERRAIN,		&_NextStaticNodeID,	&foo);
	Get_ID_Range (NODE_TYPE_LIGHT,		&_NextLightNodeID,	&foo);
	Get_ID_Range (NODE_TYPE_VIS_POINT,	&_NextMiscNodeID,		&foo);

	//
	//	Update the UI
	//
	if (instances_form != NULL) {
		instances_form->Reset_List ();
	}

	NetworkObjectMgrClass::Delete_Pending ();
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Find_Node
//
///////////////////////////////////////////////////////////////////////
NodeClass *
NodeMgrClass::Find_Node (uint32 id)
{
	NodeClass *req_node = NULL;

	for (	NodeClass *node = _NodeListHead;
			(req_node == NULL) && (node != NULL);
			node = node->m_NextNode)
	{
		//
		//	Is this the node we are looking for?
		//
		if (node->Get_ID () == id) {
			req_node = node;
		}
	}

	return req_node;
}


///////////////////////////////////////////////////////////////////////
//
//	Find_Node
//
///////////////////////////////////////////////////////////////////////
NodeClass *
NodeMgrClass::Find_Node (const char *name)
{
	NodeClass *req_node = NULL;

	for (	NodeClass *node = _NodeListHead;
			(req_node == NULL) && (node != NULL);
			node = node->m_NextNode)
	{
		//
		//	Is this the node we are looking for?
		//
		if (::lstrcmpi (node->Get_Name (), name) == 0) {
			req_node = node;
		}
	}

	return req_node;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Next
//
///////////////////////////////////////////////////////////////////////
NodeClass *
NodeMgrClass::Get_Next (NodeClass *node)
{
	return node->m_NextNode;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_First
//
///////////////////////////////////////////////////////////////////////
NodeClass *
NodeMgrClass::Get_First (NODE_TYPE type)
{
	NodeClass *req_node = NULL;

	//
	//	Loop through all the nodes until we've found the
	// first one that matches the criteria
	//
	for (	NodeClass *node = _NodeListHead;
			(req_node == NULL) && (node != NULL);
			node = node->m_NextNode)
	{
		if (node->Get_Type () == type) {
			req_node = node;
		}
	}

	return req_node;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Next
//
///////////////////////////////////////////////////////////////////////
NodeClass *
NodeMgrClass::Get_Next (NodeClass *current, NODE_TYPE type)
{
	NodeClass *req_node = NULL;

	//
	//	Loop through all the nodes until we've found the
	// first one that matches the criteria
	//
	while ((req_node == NULL) && ((current = current->m_NextNode) != NULL))
	{
		if (current->Get_Type () == type) {
			req_node = current;
		}
	}

	return req_node;
}



///////////////////////////////////////////////////////////////////////
//
//	Get_First
//
///////////////////////////////////////////////////////////////////////
NodeClass *
NodeMgrClass::Get_First (PresetClass *preset)
{
	NodeClass *req_node = NULL;

	//
	//	Loop through all the nodes until we've found the
	// first one that matches the criteria
	//
	for (	NodeClass *node = _NodeListHead;
			(req_node == NULL) && (node != NULL);
			node = node->m_NextNode)
	{
		if (node->Get_Preset () == preset) {
			req_node = node;
		}
	}

	return req_node;
}


///////////////////////////////////////////////////////////////////////
//
//	Find_First
//
///////////////////////////////////////////////////////////////////////
NodeClass *
NodeMgrClass::Find_First (uint32 class_id)
{
	NodeClass *req_node = NULL;

	//
	//	Loop through all the nodes until we've found the
	// first one that matches the criteria
	//
	for (	NodeClass *node = _NodeListHead;
			(req_node == NULL) && (node != NULL);
			node = node->m_NextNode)
	{
		//
		//	Get the preset this node was created from
		//
		PresetClass *preset = node->Get_Preset ();
		if (preset != NULL) {

			//
			//	Is this preset belong to the requested class?
			//
			DefinitionClass *definition = preset->Get_Definition ();
			if (definition != NULL && definition->Get_Class_ID () == class_id) {
				req_node = node;
			}
		}
	}

	return req_node;
}


///////////////////////////////////////////////////////////////////////
//
//	Find_Next
//
///////////////////////////////////////////////////////////////////////
NodeClass *
NodeMgrClass::Find_Next (NodeClass *current, uint32 class_id)
{
	NodeClass *req_node = NULL;

	//
	//	Loop through all the nodes until we've found the
	// first one that matches the criteria
	//
	while ((req_node == NULL) && ((current = current->m_NextNode) != NULL))
	{
		//
		//	Get the preset this node was created from
		//
		PresetClass *preset = current->Get_Preset ();
		if (preset != NULL) {

			//
			//	Is this preset belong to the requested class?
			//
			DefinitionClass *definition = preset->Get_Definition ();
			if (definition != NULL && definition->Get_Class_ID () == class_id) {
				req_node = current;
			}
		}
	}

	return req_node;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Next
//
///////////////////////////////////////////////////////////////////////
NodeClass *
NodeMgrClass::Get_Next (NodeClass *current, PresetClass *preset)
{
	NodeClass *req_node = NULL;

	//
	//	Loop through all the nodes until we've found the
	// first one that matches the criteria
	//
	while ((req_node == NULL) && ((current = current->m_NextNode) != NULL))
	{
		if (current->Get_Preset () == preset) {
			req_node = current;
		}
	}

	return req_node;
}


///////////////////////////////////////////////////////////////////////
//
//	Contains_Data
//
///////////////////////////////////////////////////////////////////////
bool
NodeMgrClass::Contains_Data (void) const
{
	return (_NodeListHead != NULL);
}


///////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////
bool
NodeMgrClass::Save (ChunkSaveClass &csave)
{
	bool retval = true;

	//
	//	Save the variables to their own chunk
	//
	csave.Begin_Chunk (CHUNKID_VARIABLES);	
		//WRITE_MICRO_CHUNK (csave, VARID_NEXT_NODE_ID, _NextNodeID);
	csave.End_Chunk ();


	//
	//	Save the nodes to their own chunk
	//
	csave.Begin_Chunk (CHUNKID_NODES);
	retval &= Save_Nodes (csave);
	csave.End_Chunk ();
	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Load
//
//	Note:  It is the outside caller's responsibility to make sure
// all nodes have been removed from the manager before a load.  This
// is done so we can import/export nodes without trashing the existing
// node set.
//
///////////////////////////////////////////////////////////////////////
bool
NodeMgrClass::Load (ChunkLoadClass &cload)
{
	bool retval = true;	
	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {
			
			//
			//	Load all the nodes from this chunk
			//
			case CHUNKID_NODES:
				retval &= Load_Nodes (cload);
				break;

			case CHUNKID_VARIABLES:
				retval &= Load_Variables (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	SaveLoadSystemClass::Register_Post_Load_Callback (this);
	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Save_Node_List
//
///////////////////////////////////////////////////////////////////////
bool
NodeMgrClass::Save_Node_List (ChunkSaveClass &csave, NODE_LIST &node_list)
{	
	//
	//	Loop over all the nodes in the list
	//
	for (int index = 0; index < node_list.Count (); index ++) {
		NodeClass *node = node_list[index];
		if (node != NULL) {
			
			//
			//	Save this node to its own chunk
			//
			csave.Begin_Chunk (node->Get_Factory ().Chunk_ID ());
			node->Get_Factory ().Save (csave, node);
			csave.End_Chunk ();
		}
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Node_List
//
///////////////////////////////////////////////////////////////////////
bool
NodeMgrClass::Load_Node_List (ChunkLoadClass &cload, NODE_LIST &node_list)
{	
	while (cload.Open_Chunk ()) {

		//
		//	Load this node from the chunk (if possible)
		//
		PersistFactoryClass *factory = SaveLoadSystemClass::Find_Persist_Factory (cload.Cur_Chunk_ID ());
		if (factory != NULL) {
			NodeClass *node = (NodeClass *)factory->Load (cload);
			if (node != NULL) {
				
				//
				// Does this node's preset still exist?
				//
				if (node->Get_Preset () != NULL || node->Get_Type () == NODE_TYPE_WAYPOINT) {
					
					//
					//	Initialize the node
					//
					node->Initialize ();
					if (node->Needs_Save () == false) {
						node->Lock (true);
					}

					//
					//	Add this node to the list
					//
					node->Add_Ref ();
					node_list.Add (node);

				} else {
					WWDEBUG_SAY (("No preset for linked node %s.\r\n", node->Get_Name ()));
				}

				MEMBER_RELEASE (node);
			}
		}

		cload.Close_Chunk ();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	Save_Nodes
//
///////////////////////////////////////////////////////////////////////
bool
NodeMgrClass::Save_Nodes (ChunkSaveClass &csave)
{	
	for (	NodeClass *node = _NodeListHead;
			node != NULL;
			node = node->m_NextNode)
	{
		if (node->Needs_Save ()) {
			
			//
			//	Save this node to its own chunk
			//
			csave.Begin_Chunk (node->Get_Factory ().Chunk_ID ());
			node->Get_Factory ().Save (csave, node);
			csave.End_Chunk ();
		}
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Nodes
//
///////////////////////////////////////////////////////////////////////
bool
NodeMgrClass::Load_Nodes (ChunkLoadClass &cload)
{
	bool retval = true;

	DynamicVectorClass<NodeClass *> removed_node_list;

	while (cload.Open_Chunk ()) {

		//
		//	Load this node from the chunk (if possible)
		//
		PersistFactoryClass *factory = SaveLoadSystemClass::Find_Persist_Factory (cload.Cur_Chunk_ID ());
		if (factory != NULL) {
			NodeClass *node = (NodeClass *)factory->Load (cload);
			if (node != NULL) {
				
				// Does this node's preset still exist?
				if (node->Get_Preset () != NULL || node->Get_Type () == NODE_TYPE_WAYPOINT) {
					
					//
					//	Initialize the node
					//
					node->Initialize ();
					if (node->Needs_Save () == false) {
						node->Lock (true);
					}

					//
					//	Put the node back into the world
					//
					Add_Node (node);

				} else {
					node->Add_Ref ();
					removed_node_list.Add (node);

					WWDEBUG_SAY (("No preset for %s, removing from level.\r\n", node->Get_Name ()));
				}

				MEMBER_RELEASE (node);
			}
		}

		cload.Close_Chunk ();
	}

	//
	//	Display a dialog to the user allowing them to remap removed nodes
	// to other presets...
	//
	if (removed_node_list.Count () > 0) {
		PresetRemapDialogClass dialog (::AfxGetMainWnd ());
		dialog.Set_Node_list (removed_node_list);
		dialog.DoModal ();
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	On_Post_Load
//
///////////////////////////////////////////////////////////////////////
void
NodeMgrClass::On_Post_Load (void)
{
	SceneEditorClass *scene = ::Get_Scene_Editor ();

	//
	//	Loop over all the nodes
	//
	for (	NodeClass *node = _NodeListHead;
			node != NULL;
			node = node->m_NextNode)
	{
		//
		//	Add them to the scene...
		//
		node->Add_To_Scene ();
		scene->Update_File_Mgr (node);
	}
	
	Reset_New_ID ();
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////
bool
NodeMgrClass::Load_Variables (ChunkLoadClass &cload)
{
	bool retval = true;

	while (cload.Open_Micro_Chunk ()) {
		/*switch (cload.Cur_Micro_Chunk_ID ()) {

		}*/

		cload.Close_Micro_Chunk ();
	}
	
	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Reload_Nodes
//
///////////////////////////////////////////////////////////////////////
void
NodeMgrClass::Reload_Nodes (void)
{
	::Get_Main_View ()->Allow_Repaint (false);

	//
	//	First, make sure each node has the correct preset pointer
	//
	DynamicVectorClass<NodeClass *> node_list;
	Build_Full_Node_List (node_list);
	for (int index = 0; index < node_list.Count (); index ++) {
		NodeClass *node = node_list[index];
		if (node != NULL) {

			//
			//	Lookup the preset and re-assign it to this node
			//
			uint32 preset_id		= node->Get_Preset_ID ();
			PresetClass *preset	= PresetMgrClass::Find_Preset (preset_id);
			node->Set_Preset (preset);
		}
	}

	//
	//	Ask each node to recreate itself
	//
	for (	NodeClass *node = _NodeListHead; node != NULL; node = node->m_NextNode) {
		node->Reload ();
	}

	::Get_Scene_Editor ()->Validate_Vis ();
	::Get_Main_View ()->Allow_Repaint (true);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Reload_Nodes
//
///////////////////////////////////////////////////////////////////////
void
NodeMgrClass::Reload_Nodes (PresetClass *preset)
{
	::Get_Main_View ()->Allow_Repaint (false);

	//
	//	Loop over all the nodes
	//
	for (	NodeClass *node = _NodeListHead;
			node != NULL;
			node = node->m_NextNode)
	{
		//
		//	Simply ask the node to re-create itself
		//
		if (node->Get_Preset () == preset) {
			node->Reload ();
		}
	}
	
	::Get_Scene_Editor ()->Validate_Vis ();
	::Get_Main_View ()->Allow_Repaint (true);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Put_Objects_Back
//
///////////////////////////////////////////////////////////////////////
void
NodeMgrClass::Put_Objects_Back (const NODE_LIST &obj_list)
{
	//
	//	Insert the nodes back into the system
	//
	for (int index = 0; index < obj_list.Count (); index ++) {
		NodeClass *node = obj_list[index];
		NodeMgrClass::Add_Node (node);
		node->Release_Ref ();
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Remove_Dynamic_Objects
//
///////////////////////////////////////////////////////////////////////
void
NodeMgrClass::Remove_Dynamic_Objects (NODE_LIST &dynamic_obj_list)
{
	//
	//	Build a list of nodes to that are not dynamic
	//
	for (	NodeClass *node = NodeMgrClass::Get_First ();
			node != NULL;
			node = NodeMgrClass::Get_Next (node))
	{
		if (node->Is_Static () == false) {
			dynamic_obj_list.Add (node);
		}
	}

	//
	//	Remove these nodes from the level
	//
	for (int index = 0; index < dynamic_obj_list.Count (); index ++) {
		NodeClass *node = dynamic_obj_list[index];
		if (node != NULL) {
			node->Add_Ref ();
			NodeMgrClass::Remove_Node (node);
		}
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Remove_Static_Objects
//
///////////////////////////////////////////////////////////////////////
void
NodeMgrClass::Remove_Static_Objects (NODE_LIST &static_obj_list)
{
	//
	//	Build a list of nodes to that are not dynamic
	//
	for (	NodeClass *node = NodeMgrClass::Get_First ();
			node != NULL;
			node = NodeMgrClass::Get_Next (node))
	{
		if (node->Is_Static ()) {
			static_obj_list.Add (node);
		}
	}

	//
	//	Remove these nodes from the level
	//
	for (int index = 0; index < static_obj_list.Count (); index ++) {
		NodeClass *node = static_obj_list[index];
		if (node != NULL) {
			node->Add_Ref ();
			NodeMgrClass::Remove_Node (node);
		}
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Max_Used_ID
//
///////////////////////////////////////////////////////////////////////
uint32
NodeMgrClass::Get_Max_Used_ID (void)
{
	//
	//	Build the full list of nodes
	//
	DynamicVectorClass<NodeClass *> full_node_list;
	Build_Full_Node_List (full_node_list);

	uint32 max_used = 0;

	//
	//	Determine what the largest used ID is
	//
	for (int index = 0; index < full_node_list.Count (); index ++) {
		NodeClass *node = full_node_list[index];
		max_used = max (node->Get_ID (), max_used);
	}

	return max_used;
}


///////////////////////////////////////////////////////////////////////
//
//	Verify_Unique_ID - Verify that given ID is unique.
//
///////////////////////////////////////////////////////////////////////
bool
NodeMgrClass::Verify_Unique_ID(uint32 id)
{
	NodeClass* node = Get_First();

	while (node != NULL) {
		if (node->Get_ID() == id) {
			return false;
		}

		node = Get_Next(node);
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	Make_Static_Anim_Phys_Collideable
//
///////////////////////////////////////////////////////////////////////
void
NodeMgrClass::Make_Static_Anim_Phys_Collideable (bool onoff)
{
	//
	//	Loop over all the terrains in the level
	//
	NodeClass *node;
	for (	node = NodeMgrClass::Get_First (NODE_TYPE_TERRAIN);
			node != NULL;
			node = NodeMgrClass::Get_Next (node, NODE_TYPE_TERRAIN))
	{
		TerrainNodeClass *terrain	= (TerrainNodeClass *)node;
		int count						= terrain->Get_Sub_Node_Count ();
		
		//
		//	Loop over all the sections of this terrain object
		//
		for (int index = 0; index < count; index ++) {
			NodeClass *sub_node = terrain->Get_Sub_Node (index);
			if (sub_node != NULL) {				

				//
				//	Check to make sure this is a static anim phys object
				//				
				PhysClass *phys_obj = sub_node->Peek_Physics_Obj ();
				if (phys_obj != NULL && phys_obj->As_StaticAnimPhysClass () != NULL) {
					sub_node->Hide (onoff == false);
					
					//
					//	Turn off collision on or off this object
					//
					if (onoff == false) {
						phys_obj->Inc_Ignore_Counter ();
					} else if (phys_obj->Is_Ignore_Me ()) {
						phys_obj->Dec_Ignore_Counter ();
					}
				}			
			}
		}
	}

	//
	//	Loop over all the tiles in the level
	//
	for (	node = NodeMgrClass::Get_First (NODE_TYPE_TILE);
			node != NULL;
			node = NodeMgrClass::Get_Next (node, NODE_TYPE_TILE))
	{
		//
		//	Check to make sure this is a static anim phys object
		//
		PhysClass *phys_obj = node->Peek_Physics_Obj ();
		if (phys_obj != NULL && phys_obj->As_StaticAnimPhysClass () != NULL) {
			node->Hide (onoff);
			
			//
			//	Turn off collision on or off this object
			//
			if (onoff == false) {
				phys_obj->Inc_Ignore_Counter ();
			} else if (phys_obj->Is_Ignore_Me ()) {
				phys_obj->Dec_Ignore_Counter ();
			}
		}
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Build_ID_Collision_List
//
///////////////////////////////////////////////////////////////////////
void
NodeMgrClass::Build_ID_Collision_List (NODE_LIST &node_list)
{
	CWaitCursor wait_cursor;

	//
	//	Build a flat list of nodes
	//
	DynamicVectorClass<NodeClass *> full_node_list;
	Build_Full_Node_List (full_node_list);

	//
	//	Loop over all the nodes, checking every one against the other
	//
	for (int index1 = 0; index1 < full_node_list.Count (); index1 ++) {
		for (int index2 = 0; index2 < full_node_list.Count (); index2 ++) {

			NodeClass *node1 = full_node_list[index1];
			NodeClass *node2 = full_node_list[index2];

			//
			//	Do these nodes share the same ID?
			//
			if (node1 != node2 && node1->Get_ID () == node2->Get_ID ()) {
				if (node_list.ID (node1) == -1) {
					node_list.Add (node1);
				}
			}
			
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Build_Unimportant_ID_Collision_List
//
///////////////////////////////////////////////////////////////////////
void
NodeMgrClass::Build_Unimportant_ID_Collision_List (NODE_LIST &node_list)
{
	CWaitCursor wait_cursor;

	//
	//	Build a flat list of nodes
	//
	DynamicVectorClass<NodeClass *> full_node_list;
	Build_Full_Node_List (full_node_list);

	//
	//	Loop over all the nodes, checking every one against the other
	//
	for (int index1 = 0; index1 < full_node_list.Count (); index1 ++) {
		for (int index2 = 0; index2 < full_node_list.Count (); index2 ++) {

			NodeClass *node1 = full_node_list[index1];
			NodeClass *node2 = full_node_list[index2];

			//
			//	Do these nodes share the same ID?
			//
			if (node1 != node2 && node1->Get_ID () == node2->Get_ID ()) {
				if (node_list.ID (node1) == -1) {

					//
					//	Check to make sure this isn't an important node type
					//
					if (	node1->As_ObjectNodeClass () == NULL &&
							node1->Get_Type () != NODE_TYPE_SPAWNER &&
							node1->Get_Type () != NODE_TYPE_TILE &&
							node1->Get_Type () != NODE_TYPE_WAYPATH &&
							node1->Get_Type () != NODE_TYPE_WAYPOINT &&
							node1->Get_Type () != NODE_TYPE_SOUND)
					{
						node_list.Add (node1);
					}
				}
			}
			
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Reset_New_ID
//
///////////////////////////////////////////////////////////////////////
void
NodeMgrClass::Reset_New_ID (void)
{
	//
	//	Reset each of the id ranges
	//
	_NextObjectNodeID = Find_Max_Used_ID (NODE_TYPE_OBJECT) + 1;
	_NextStaticNodeID = Find_Max_Used_ID (NODE_TYPE_TERRAIN) + 1;
	_NextLightNodeID	= Find_Max_Used_ID (NODE_TYPE_LIGHT) + 1;
	_NextMiscNodeID	= Find_Max_Used_ID (NODE_TYPE_VIS_POINT) + 1;

	//
	//	Update each heightfield with a guaranteed unique ID
	//
	HeightfieldMgrClass::Assign_Unique_IDs ();
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Find_Max_Used_ID
//
///////////////////////////////////////////////////////////////////////
uint32
NodeMgrClass::Find_Max_Used_ID (NODE_TYPE type)
{
	//
	//	Determine what our minimum ID is
	//
	uint32 min_id = 0;
	uint32 max_id = 0;
	Get_ID_Range (type, &min_id, &max_id);

	//
	//	First, build a complete list of all the nodes
	//
	DynamicVectorClass<NodeClass *> node_list;
	Build_Full_Node_List (node_list);

	//
	//	Loop over all the nodes in the level and find the largest ID
	// in our range
	//
	uint32 largest_id = min_id;
	for (int index = 0; index < node_list.Count (); index ++) {
		NodeClass *node = node_list[index];
		if (node != NULL) {

			//
			//	Is this the largest ID in our range?
			//
			uint32 curr_id = node->Get_ID ();
			if (curr_id >= min_id && curr_id < max_id) {
				largest_id = max (largest_id, curr_id);
			}
		}
	}

	return largest_id;
}


//////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Node_ID
//
//////////////////////////////////////////////////////////////////////////////////////////
uint32
NodeMgrClass::Get_Node_ID (NODE_TYPE type)
{
	uint32 new_id = 0;

	switch (type)
	{
		case NODE_TYPE_TERRAIN:
		case NODE_TYPE_TERRAIN_SECTION:
		case NODE_TYPE_TILE:
		case NODE_TYPE_BUILDING:
			new_id = _NextStaticNodeID ++;
			break;

		case NODE_TYPE_DAMAGE_ZONE:
		case NODE_TYPE_ZONE:
			new_id = _NextObjectNodeID ++;
			break;

		case NODE_TYPE_WAYPATH:
		case NODE_TYPE_WAYPOINT:
			new_id = _NextObjectNodeID ++;
			break;
		
		case NODE_TYPE_OBJECT:		
			new_id = _NextObjectNodeID ++;
			break;

		
		case NODE_TYPE_COVER_SPOT:
		case NODE_TYPE_COVER_ATTACK_POINT:
			new_id = _NextObjectNodeID ++;
			break;
		
		case NODE_TYPE_SPAWNER:
			new_id = _NextObjectNodeID ++;
			break;

		case NODE_TYPE_SOUND:
		case NODE_TYPE_TRANSITION:
		case NODE_TYPE_TRANSITION_CHARACTER:
		case NODE_TYPE_DUMMY_OBJECT:
			new_id = _NextObjectNodeID ++;
			break;

		case NODE_TYPE_LIGHT:
			new_id = _NextLightNodeID ++;
			break;

		case NODE_TYPE_UNKNOWN:		
		case NODE_TYPE_VIS:		
		case NODE_TYPE_VIS_POINT:
		case NODE_TYPE_PATHFIND_START:
		default:
			new_id = _NextMiscNodeID ++;
			break;
	}

	return new_id;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_ID_Range
//
///////////////////////////////////////////////////////////////////////
void
NodeMgrClass::Get_ID_Range (NODE_TYPE type, uint32 *min_id, uint32 *max_id)
{
	switch (type)
	{
		case NODE_TYPE_TERRAIN:
		case NODE_TYPE_TERRAIN_SECTION:
		case NODE_TYPE_TILE:		
		case NODE_TYPE_BUILDING:
			(*min_id) = theApp.GetProfileInt (CONFIG_KEY, NODE_ID_START_VALUE, FIRST_OBJECT_NODE_ID);
			(*min_id) += 50000;
			(*max_id) = (*min_id) + 50000;
			break;

		case NODE_TYPE_DAMAGE_ZONE:
		case NODE_TYPE_ZONE:
		case NODE_TYPE_WAYPATH:
		case NODE_TYPE_WAYPOINT:
		case NODE_TYPE_OBJECT:		
		case NODE_TYPE_COVER_SPOT:
		case NODE_TYPE_COVER_ATTACK_POINT:
		case NODE_TYPE_SPAWNER:
		case NODE_TYPE_SOUND:
		case NODE_TYPE_TRANSITION:
		case NODE_TYPE_TRANSITION_CHARACTER:
		case NODE_TYPE_DUMMY_OBJECT:
			(*min_id) = theApp.GetProfileInt (CONFIG_KEY, NODE_ID_START_VALUE, FIRST_OBJECT_NODE_ID);
			(*max_id) = (*min_id) + 50000;
			break;

		case NODE_TYPE_LIGHT:
			(*min_id) = FIRST_LIGHT_NODE_ID;
			(*max_id) = (*min_id) + 100000;
			break;

		case NODE_TYPE_UNKNOWN:		
		case NODE_TYPE_VIS:		
		case NODE_TYPE_VIS_POINT:
		case NODE_TYPE_PATHFIND_START:		
			(*min_id) = FIRST_MISC_NODE_ID;
			(*max_id) = (*min_id) + 100000;
			break;

		default:
			(*min_id) = FIRST_MISC_NODE_ID;
			break;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Build_Full_Node_List
//
///////////////////////////////////////////////////////////////////////
void
NodeMgrClass::Build_Full_Node_List (DynamicVectorClass<NodeClass *> &node_list)
{
	//
	//	Add all nodes and sub-nodes to this list
	//
	for (NodeClass *node = ::Get_Node_Mgr ().Get_First ();
		  node != NULL;
		  node = ::Get_Node_Mgr ().Get_Next (node))
	{
		Add_Nodes_To_List (node_list, node);
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Add_Nodes_To_List
//
///////////////////////////////////////////////////////////////////////
void
NodeMgrClass::Add_Nodes_To_List
(
	DynamicVectorClass<NodeClass *> &node_list,
	NodeClass *node
)
{
	node_list.Add (node);

	//
	// If the node is an aggregate (like a terrain), we are adding the 
	// sub-nodes directly into the list
	//
	int sub_count = node->Get_Sub_Node_Count ();
	if (sub_count > 0) {
		for (int index = 0; index < sub_count; index ++) {
			NodeClass *sub_node = node->Get_Sub_Node (index);
			
			//
			//	Recurse into this node
			//
			Add_Nodes_To_List (node_list, sub_node);
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Create_All_Embedded_Nodes
//
///////////////////////////////////////////////////////////////////////
void
NodeMgrClass::Create_All_Embedded_Nodes (void)
{
	//
	//	Generate a list of nodes
	//
	DynamicVectorClass<NodeClass *> node_list;
	Build_Full_Node_List (node_list);

	//
	//	Loop over all the nodes
	//
	for (int index = 0; index < node_list.Count (); index ++) {
		if (node_list[index]->Is_Proxied () == false) {
			PresetClass *preset = node_list[index]->Get_Preset ();
			if (preset != NULL) {
				
				//
				//	Create the nodes that are embedded inside this preset
				//
				preset->Create_Linked_Nodes (node_list[index]);
			}
		}
	}

	return ;
}
