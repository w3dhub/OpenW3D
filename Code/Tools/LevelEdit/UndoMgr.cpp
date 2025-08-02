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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/UndoMgr.cpp                  $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 4/19/01 11:34a                                              $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "undomgr.h"
#include "utils.h"
#include "nodemgr.h"
#include "node.h"
#include "leveleditdoc.h"
#include "leveleditview.h"
#include "mover.h"
#include "zonenode.h"
#include "damagezonenode.h"


////////////////////////////////////////////////////////////////////////////////////
//
//	Local constants
//
////////////////////////////////////////////////////////////////////////////////////
const LPCTSTR OPERATION_NAMES[OPERATION_COUNT] =
{
	"Move",
	"Rotate",
	"Delete",
	"Resize"
};


//**************************************************************************************************************//
//
//	Start of UndoMgrClass
//
//**************************************************************************************************************//


///////////////////////////////////////////////////////////
//
//	Free_Buffer_List
//
///////////////////////////////////////////////////////////
void
UndoMgrClass::Free_Buffer_List (void)
{
	// Free each buffer in our array
	for (int index = 0; index < m_iCurrentBuffer; index ++) {
		SAFE_DELETE (m_pBuffers[index]);
	}

	// Reset the current index
	m_iCurrentBuffer = 0;
	return ;
}


///////////////////////////////////////////////////////////
//
//	Begin_Operation
//
///////////////////////////////////////////////////////////
void
UndoMgrClass::Begin_Operation
(
	OPERATION_TYPE type,
	const NODE_LIST &nodes_affected
)
{
	UndoBufferClass *pbuffer = new UndoBufferClass;
	ASSERT (pbuffer != NULL);
	if (pbuffer != NULL) {

		// Pass the identifcation information onto the new buffer
		pbuffer->Set_Operation_Type (type);
		pbuffer->Set_ID (m_dwNextOperationID++);
		
		// Capture the current state of the world in the buffer
		switch (type)
		{
			case OPERATION_MOVE:
			case OPERATION_ROTATE:
			{
				// Record the positions and orientations of the nodes
				pbuffer->Capture_Position (nodes_affected);
			}
			break;
			
			case OPERATION_DELETE:
			{
				// Make 'backups' of the node that will be deleted
				pbuffer->Capture_Existence (nodes_affected);
			}
			break;

			case OPERATION_RESIZE:
			{
				// Record the width, height, and depth of the nodes
				pbuffer->Capture_Dimensions (nodes_affected);
			}
			break;
		}

		// Push this undo buffer onto the end of our 'stack'
		Push_Buffer (pbuffer);
	}

	return ;
}


///////////////////////////////////////////////////////////
//
//	Push_Buffer
//
///////////////////////////////////////////////////////////
void
UndoMgrClass::Push_Buffer (UndoBufferClass *pbuffer)
{
	// Param OK?
	ASSERT (pbuffer != NULL);
	if (pbuffer != NULL) {
		
		// Can we just add the buffer onto the end of the array?
		if (m_iCurrentBuffer < UNDO_LEVELS) {
			m_pBuffers[m_iCurrentBuffer++] = pbuffer;
		} else {
			
			// Push the first entry off the list and move everything else down a position
			SAFE_DELETE (m_pBuffers[0]);
			::memmove (&m_pBuffers[0], &m_pBuffers[1], sizeof (UndoBufferClass *) * (UNDO_LEVELS - 1));

			// Now simply add the buffer to the end of the array
			m_iCurrentBuffer = UNDO_LEVELS;
			m_pBuffers[m_iCurrentBuffer-1] = pbuffer;
		}
	}

	return ;
}


//////////////////////////////////////////////////////////
//
//	Get_Next_Undo_Name
//
//////////////////////////////////////////////////////////
LPCTSTR
UndoMgrClass::Get_Next_Undo_Name (void) const
{
	LPCTSTR pundo_name = NULL;

	// If we have an undo stack, then return the name of the topmost buffer
	if (m_iCurrentBuffer > 0) {
		pundo_name = m_pBuffers[m_iCurrentBuffer - 1]->Get_Operation_Name ();
	}

	// Return the undo name
	return pundo_name;
}


//////////////////////////////////////////////////////////
//
//	Perform_Undo
//
//////////////////////////////////////////////////////////
void
UndoMgrClass::Perform_Undo (int ilevels)
{
	// Loop through the undo levels and restore the world state at each...
	int imin_index = ::max((m_iCurrentBuffer - ilevels), 0);
	for (int index = (m_iCurrentBuffer - 1); index >= imin_index; index --) {
		
		// Undo this level
		m_pBuffers[index]->Restore_State ();
		SAFE_DELETE (m_pBuffers[index]);
		m_iCurrentBuffer --;
	}

	return ;
}


//**************************************************************************************************************//
//
//	End of UndoMgrClass
//
//**************************************************************************************************************//

//**************************************************************************************************************//
//
//	Start of UndoBufferClass
//
//**************************************************************************************************************//


//////////////////////////////////////////////////////////
//
//	operator=
//
//////////////////////////////////////////////////////////
const UndoBufferClass::NodeStateClass &
UndoBufferClass::NodeStateClass::operator= (const NodeStateClass &src)
{
	MEMBER_ADD (node_ptr, src.node_ptr);
	transform = src.transform;
	dimensions = src.dimensions;
	return *this;
}


//////////////////////////////////////////////////////////
//
//	~NodeStateClass
//
//////////////////////////////////////////////////////////
UndoBufferClass::NodeStateClass::~NodeStateClass (void)
{
	MEMBER_RELEASE (node_ptr);
	return ;
}


//////////////////////////////////////////////////////////
//
//	Capture_Dimensions
//
//////////////////////////////////////////////////////////
void
UndoBufferClass::Capture_Dimensions (const NODE_LIST &node_list)
{
	// Loop through all the nodes in the list and copy their dimensions
	for (int index = 0; index < node_list.Count (); index ++) {

		NodeClass *node = node_list[index];
		if (node != NULL) {
		
			//
			// This operation only applies to zones.  Is this a zone?
			//
			if (	node->Get_Type () == NODE_TYPE_ZONE ||
					node->Get_Type () == NODE_TYPE_DAMAGE_ZONE)
			{
				Box3DClass *box = NULL;
				
				if (node->Get_Type () == NODE_TYPE_ZONE) {
					ZoneNodeClass *zone = (ZoneNodeClass *)node;
					box = zone->Get_Box ();
				} else {
					DamageZoneNodeClass *zone = (DamageZoneNodeClass *)node;
					box = zone->Get_Box ();
				}

				//
				// Copy the node's dimensions
				//
				NodeStateClass state;
				MEMBER_ADD (state.node_ptr, node);
				state.dimensions	= box->Get_Dimensions ();
				state.transform	= node->Get_Transform ();

				//
				// Add this state information to our list
				//
				m_StateList.Add (state);
			}
		}
	}

	return ;
}


//////////////////////////////////////////////////////////
//
//	Capture_Existence
//
//////////////////////////////////////////////////////////
void
UndoBufferClass::Capture_Existence (const NODE_LIST &node_list)
{
	// Loop through all the nodes in the list and copy their pointers
	for (int index = 0; index < node_list.Count (); index ++) {

		NodeClass *node = node_list[index];
		if (node != NULL && node->Get_Type () != NODE_TYPE_WAYPOINT) {
		
			// Store this node in our state manager			
			NodeStateClass state;
			MEMBER_ADD (state.node_ptr, node);

			// Add this state information to our list
			m_StateList.Add (state);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////
//
//	Capture_Position
//
//////////////////////////////////////////////////////////
void
UndoBufferClass::Capture_Position (const NODE_LIST &node_list)
{
	// Loop through all the nodes in the list and copy their position information
	for (int index = 0; index < node_list.Count (); index ++) {

		NodeClass *node = node_list[index];
		if (node != NULL) {
		
			// Copy the node's state information
			NodeStateClass state;
			MEMBER_ADD (state.node_ptr, node);
			state.transform = node->Get_Transform ();

			// Add this state information to our list
			m_StateList.Add (state);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////
//
//	Restore_State
//
//////////////////////////////////////////////////////////
void
UndoBufferClass::Restore_State (void)
{
	// Turn painting off
	CLevelEditView::Allow_Repaint (false);

	//
	// Loop through all the entries in our state list,
	// and restore their state
	//
	for (int index = 0; index < m_StateList.Count (); index ++) {

		// Alias the state structure
		NodeStateClass &state = m_StateList[index];

		// What type of undo operation is this?
		switch (m_Type)
		{
			case OPERATION_MOVE:
			case OPERATION_ROTATE:
			{
				//
				// Reset this node's position and orientation
				//
				if (state.node_ptr != NULL) {
					state.node_ptr->Set_Transform (state.transform);
				}
			}
			break;

			case OPERATION_RESIZE:
			{
				NodeClass *node = state.node_ptr;
				ASSERT (node != NULL);

				//
				// This operation only applies to zones, so if
				// this node is a zone, then resize it.
				//
				if (	node->Get_Type () == NODE_TYPE_ZONE ||
						node->Get_Type () == NODE_TYPE_DAMAGE_ZONE)
				{					
					Box3DClass *box = NULL;
					
					if (node->Get_Type () == NODE_TYPE_ZONE) {
						ZoneNodeClass *zone = (ZoneNodeClass *)node;
						box = zone->Get_Box ();
					} else {
						DamageZoneNodeClass *zone = (DamageZoneNodeClass *)node;
						box = zone->Get_Box ();
					}
					
					ASSERT (box != NULL);
					box->Set_Dimensions (state.dimensions);
					node->Set_Transform (state.transform);
				}
			}
			break;

			case OPERATION_DELETE:
			{
				// Add the 'deleted' node back into the world
				ASSERT (state.node_ptr != NULL);
				if (state.node_ptr != NULL) {
					::Get_Scene_Editor ()->Add_Node (state.node_ptr);
					state.node_ptr->On_Restore ();
					state.node_ptr->Hide (false);
				}
			}
			break;
		}			
	}

	// Ensure the main view is updated
	::Refresh_Main_View ();

	// Turn painting back on
	CLevelEditView::Allow_Repaint (true);
	return ;
}


//////////////////////////////////////////////////////////
//
//	Get_Operation_Name
//
//////////////////////////////////////////////////////////
LPCTSTR
UndoBufferClass::Get_Operation_Name (void)
{
	return OPERATION_NAMES[m_Type];
}


//**************************************************************************************************************//
//
//	End of UndoBufferClass
//
//**************************************************************************************************************//
