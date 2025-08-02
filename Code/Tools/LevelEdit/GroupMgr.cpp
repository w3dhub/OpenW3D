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

///////////////////////////////////////////////////////////////////////////////////////
//
//	GroupMgrClass.cpp
//
//	Class definition for managing groups.
//

#include "StdAfx.h"
#include "groupmgr.h"
#include "leveleditdoc.h"
#include "cameramgr.h"
#include "node.h"
#include "sceneeditor.h"
#include "utils.h"
#include "wwmath.h"

#include "waypointnode.h"
#include "waypathnode.h"


/////////////////////////////////////////////////////////////
//
//	Add_Node
//
/////////////////////////////////////////////////////////////
const GroupMgrClass &
GroupMgrClass::operator= (const GroupMgrClass &src)
{
	m_BoundingSphere	= src.m_BoundingSphere;
	m_BoundingBox		= src.m_BoundingBox;
	m_LowZCenter		= src.m_LowZCenter;
	m_AbsCenter			= src.m_AbsCenter;
	m_Name				= src.m_Name;
	m_bDirty				= src.m_bDirty;	

	Free_List ();
	Import_Nodes (src.m_GroupList);

	// Return a reference to ourselves	
	return *this;
}


/////////////////////////////////////////////////////////////
//
//	Export_Nodes
//
/////////////////////////////////////////////////////////////
void
GroupMgrClass::Export_Nodes (NODE_LIST &node_list)
{
	//
	// Add all the nodes from this group to the list
	//
	for (int index = 0; index < m_GroupList.Count (); index ++) {
		NodeClass *node = m_GroupList[index];
		if (node != NULL) {
			node_list.Add_Unique (node);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//	Import_Nodes
//
/////////////////////////////////////////////////////////////
void
GroupMgrClass::Import_Nodes (const NODE_LIST &node_list)
{
	// Loop through all the entries in the list and add them
	// to our group.
	for (int index = 0; index < node_list.Count (); index ++) {
		Add_Node (node_list[index]);
	}

	return;
}


/////////////////////////////////////////////////////////////
//
//	Add_Node
//
/////////////////////////////////////////////////////////////
void
GroupMgrClass::Add_Node (NodeClass *node)
{
	if ((node != NULL) && (Is_Item_In_Group (node) == false)) {

		//
		// Add this item to the list
		//
		SAFE_ADD_REF (node);
		m_GroupList.Add (node);
		Recalc_Stats ();
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//	Remove_Node
//
/////////////////////////////////////////////////////////////
void
GroupMgrClass::Remove_Node (NodeClass *node_to_remove)
{
	bool found = false;
	for (int index = 0; !found && index < m_GroupList.Count (); index ++) {
		NodeClass *node = m_GroupList[index];
		
		//
		//	If this is the node we were looking for, then remove it
		// from the list and free our hold on it.
		//
		if (node == node_to_remove) {
			m_GroupList.Delete (index);
			MEMBER_RELEASE (node);
			found = true;
		}
	}

	if (found) {
		Recalc_Stats ();
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//	Reset
//
/////////////////////////////////////////////////////////////
void
GroupMgrClass::Reset (void)
{
	Free_List ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//	Free_List
//
/////////////////////////////////////////////////////////////
void
GroupMgrClass::Free_List (void)
{
	for (int index = 0; index < m_GroupList.Count (); index ++) {
		NodeClass *node = m_GroupList[index];
		MEMBER_RELEASE (node);
	}

	m_GroupList.Delete_All ();
	Recalc_Stats ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//	Is_Item_In_Group
//
/////////////////////////////////////////////////////////////
bool
GroupMgrClass::Is_Item_In_Group (NodeClass *node_to_find)
{
	bool retval = false;

	//
	//	Loop until we've found the node
	//
	for (	int index = 0;
			(retval == false) && (index < m_GroupList.Count ());
			index ++) {
		NodeClass *node	= m_GroupList[index];
		retval				= (node == node_to_find);
	}

	return retval;
}


/////////////////////////////////////////////////////////////
//
//	Clone_Group
//
/////////////////////////////////////////////////////////////
void
GroupMgrClass::Clone_Group (void)
{		
	NODE_LIST temp_list;

	//
	// Duplicate the group list
	//
	int index;
	for (index = 0; index < m_GroupList.Count (); index ++) {
		NodeClass *node = m_GroupList[index];
		temp_list.Add (node);
	}

	// Remove all items from the group
	m_GroupList.Delete_All ();

	//
	// Loop through and create clones of the items
	//
	for (index = 0; index < temp_list.Count (); index ++) {
		NodeClass *node		= temp_list[index];

		if (node->Get_Type () == NODE_TYPE_WAYPOINT) {
			
			//
			//	Find where this waypoint is on the path
			//
			WaypointNodeClass *waypoint	= (WaypointNodeClass *)node;
			WaypathNodeClass *path			= waypoint->Peek_Waypath ();
			int index							= path->Find_Index (waypoint);
			if (index >= 0) {
				
				//
				//	Insert a new point one after the given point
				//
				path->Insert_Point (index + 1, waypoint->Get_Position ());

				//
				//	Add this new point to our group
				//
				WaypointNodeClass *new_point = NULL;
				path->Get_Point (index + 1, &new_point);
				if (new_point != NULL) {
					SAFE_ADD_REF (new_point);
					m_GroupList.Add (new_point);
				}
			}

		} else if (node->Get_Parent_Node () != NULL) {

			if (node->Get_Parent_Node ()->Can_Add_Child_Nodes ()) {

				//
				//	Ask the parent node to create us a new child node, then add
				// it to the list
				//
				NodeClass *new_node = node->Get_Parent_Node ()->Add_Child_Node (node->Get_Transform ());
				if (new_node != NULL) {
					SAFE_ADD_REF (new_node);	
					m_GroupList.Add (new_node);
				}
			}
			
		} else {

			//
			//	Create a new instance of this node, and add it to our list
			//
			NodeClass *new_node = ::Get_Scene_Editor ()->Clone_Node (node);
			if (new_node != NULL) {				
				m_GroupList.Add (new_node);
			}
		}

		MEMBER_RELEASE (node);
	}

	Recalc_Stats ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//	Recalc_Stats
//
/////////////////////////////////////////////////////////////
void
GroupMgrClass::Recalc_Stats (void)
{
	m_BoundingSphere.Init (Vector3 (0, 0, 0), 0);
	m_BoundingBox.Init (Vector3 (0, 0, 0), Vector3 (0, 0, 0));
	m_LowZCenter	= Vector3 (0, 0, 0);
	float low_z		= WWMATH_FLOAT_MAX;

	//
	// Loop through all the nodes in the group
	//
	for (int index = 0; index < m_GroupList.Count (); index ++) {
		NodeClass *node = m_GroupList[index];
		RenderObjClass *render_obj = node->Peek_Render_Obj ();
		if (render_obj != NULL) {

			if (index == 0) {
				m_BoundingBox		= render_obj->Get_Bounding_Box ();
				m_BoundingSphere	= render_obj->Get_Bounding_Sphere ();
				m_LowZCenter		= render_obj->Get_Position ();
				low_z					= m_LowZCenter.Z;
			} else {
			
				//
				//	Add this node's stats to the pot
				//
				m_BoundingBox.Add_Box (render_obj->Get_Bounding_Box ());
				m_BoundingSphere	+= render_obj->Get_Bounding_Sphere ();
				m_LowZCenter		+= render_obj->Get_Position ();

				if (render_obj->Get_Position ().Z < low_z) {
					low_z = render_obj->Get_Position ().Z;
				}
			}
		}
	}

	m_LowZCenter	= m_LowZCenter / (float)m_GroupList.Count ();
	m_AbsCenter		= m_LowZCenter;
	m_LowZCenter.Z	= low_z;

	m_bDirty = false;
	return ;
}


/////////////////////////////////////////////////////////////
//
//	Hide
//
/////////////////////////////////////////////////////////////
void
GroupMgrClass::Hide (bool hide)
{
	// Loop through all the nodes in the group
	for (int index = 0; index < m_GroupList.Count (); index ++) {
		NodeClass *node = m_GroupList[index];
		if (node != NULL) {
		
			//
			// Hide this node
			//
			node->Hide (hide);
		}
	}

	// Remember our 'state'
	m_bHidden = hide;
	return ;
}


//*********************************************************************************************//
//
//	End of GroupMgrClass
//
//*********************************************************************************************//

//*********************************************************************************************//
//
//	Start of SelectionMgrClass
//
//*********************************************************************************************//


/////////////////////////////////////////////////////////////
//
//	Add_Node
//
/////////////////////////////////////////////////////////////
void
SelectionMgrClass::Add_Node (NodeClass *node)
{
	if ((node != NULL) && (Is_Item_In_Group (node) == false)) {

		//
		//	Show the selection box
		//
		node->Show_Selection_Box (true);
	}

	GroupMgrClass::Add_Node (node);
	return ;
}


/////////////////////////////////////////////////////////////
//
//	Remove_Node
//
/////////////////////////////////////////////////////////////
void
SelectionMgrClass::Remove_Node (NodeClass *node)
{
	//
	// Remove the selection box from this node
	//
	if (node != NULL) {		
		node->Show_Selection_Box (false);
	}	

	GroupMgrClass::Remove_Node (node);
	return ;
}


/////////////////////////////////////////////////////////////
//
//	Reset
//
/////////////////////////////////////////////////////////////
void
SelectionMgrClass::Reset (void)
{	
	// Loop through all the nodes in the group
	for (int index = 0; index < m_GroupList.Count (); index ++) {
		NodeClass *node = m_GroupList[index];
		if (node != NULL) {
			
			//
			// Remove the selection item from this node
			//
			node->Show_Selection_Box (false);
		}
	}

	GroupMgrClass::Reset ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//	Clone_Group
//
/////////////////////////////////////////////////////////////
void
SelectionMgrClass::Clone_Group (void)
{	
	//
	// Remove the selection box from around all the items
	//
	int index;
	for (index = 0; index < m_GroupList.Count (); index ++) {
		NodeClass *node = m_GroupList[index];
		node->Show_Selection_Box (false);		
	}

	//
	// Let the base class do the clone operation
	//
	GroupMgrClass::Clone_Group ();

	//
	// Show the selection box for all the cloned items
	//
	for (index = 0; index < m_GroupList.Count (); index ++) {
		NodeClass *node = m_GroupList[index];
		node->Show_Selection_Box (true);
	}

	return ;
}


//*********************************************************************************************//
//
//	End of GroupMgrClass
//
//*********************************************************************************************//

//*********************************************************************************************//
//
//	Start of UserGroupMgrClass
//
//*********************************************************************************************//



/////////////////////////////////////////////////////////////
//
//	Recalc_Stats
//
/////////////////////////////////////////////////////////////
void
UserGroupMgrClass::Recalc_Stats (void)
{
	// Allow the base class to process this call
	GroupMgrClass::Recalc_Stats ();

	// Remove this group from the scene if its empty
	Update_Global_Group_List ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//	Update_Global_Group_List
//
/////////////////////////////////////////////////////////////
void
UserGroupMgrClass::Update_Global_Group_List (void)
{
	if (m_GroupList.Count () == 0) {
		::Get_Scene_Editor ()->Remove_Global_Group (this);
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//	Add_Node
//
/////////////////////////////////////////////////////////////
void
UserGroupMgrClass::Add_Node (NodeClass *node)
{
	GroupMgrClass::Add_Node (node);
	if (node != NULL) {

	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//	Remove_Node
//
/////////////////////////////////////////////////////////////
void
UserGroupMgrClass::Remove_Node (NodeClass *node)
{
	bool in_list = Is_Item_In_Group (node);
	GroupMgrClass::Remove_Node  (node);

	// OK to process?
	if (in_list && node != NULL) {
		
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//	~UserGroupMgrClass
//
/////////////////////////////////////////////////////////////
UserGroupMgrClass::~UserGroupMgrClass (void)
{
	return ;
}


//*********************************************************************************************//
//
//	End of UserGroupMgrClass
//
//*********************************************************************************************//
