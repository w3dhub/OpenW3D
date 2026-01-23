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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/MouseMgr.cpp                 $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/25/02 4:38p                                               $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "MouseMgr.h"
#include "Utils.h"
#include "LevelEditView.h"
#include "cameramgr.h"
#include "node.h"
#include "quat.h"
#include "groupmgr.h"
#include "resource.h"
#include "sceneeditor.h"
#include "collisiongroups.h"
#include "mover.h"
#include "WaypathNode.h"
#include "heightfieldeditor.h"


///////////////////////////////////////////////////////////////
//
//	MouseMgrClass
//
///////////////////////////////////////////////////////////////
MouseMgrClass::MouseMgrClass (void)	:
	m_MouseMode (MODE_OBJECT_MANIPULATE),
	IsLButtonDown (false),
	IsRButtonDown (false)
{
	// Allocate each entry in the state table
	m_pModeObjects[MOUSE_MODE::MODE_CAMERA_DEFAULT]		= new MMCameraDefaultClass;
	m_pModeObjects[MOUSE_MODE::MODE_CAMERA_WALK]			= new MMCameraWalkClass;
	m_pModeObjects[MOUSE_MODE::MODE_CAMERA_FLY]			= new MMCameraFlyClass;
	m_pModeObjects[MOUSE_MODE::MODE_CAMERA_ORBIT]		= new MMCameraOrbitClass;
	m_pModeObjects[MOUSE_MODE::MODE_OBJECT_DROP]			= new MMObjectDropClass;
	m_pModeObjects[MOUSE_MODE::MODE_OBJECT_MANIPULATE]	= new MMObjectManipulateClass;
	m_pModeObjects[MOUSE_MODE::MODE_OBJECT_SELECT]		= new MMObjectSelectClass;
	m_pModeObjects[MOUSE_MODE::MODE_GRABHANDLE_DRAG]	= new MMGrabHandleDragClass;
	m_pModeObjects[MOUSE_MODE::MODE_WAYPATH_EDIT]		= new MMWaypathEditClass;
	m_pModeObjects[MOUSE_MODE::MODE_HEIGHTFIELD_EDIT]	= new MMHeightfieldEditClass;
	return ;
}


///////////////////////////////////////////////////////////////
//
//	~MouseMgrClass
//
///////////////////////////////////////////////////////////////
MouseMgrClass::~MouseMgrClass (void)
{
	// Free the state table
	for (int mode = 0; mode < MOUSE_MODE::MODE_COUNT; mode ++) {
		SAFE_DELETE (m_pModeObjects[mode]);
		m_pModeObjects[mode] = NULL;
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//	Move_Node
//
///////////////////////////////////////////////////////////////
void
MouseMgrClass::Move_Node (NodeClass *node)
{
	::Get_Scene_Editor ()->Set_Selection (node);
	
	Set_Mouse_Mode (MouseMgrClass::MODE_OBJECT_MANIPULATE);
	((MMObjectManipulateClass *)m_pModeObjects[MOUSE_MODE::MODE_OBJECT_MANIPULATE])->Set_Move_Nodes_Mode ();
	return ;
}


///////////////////////////////////////////////////////////////
//
//	Set_Mouse_Mode
//
///////////////////////////////////////////////////////////////
void
MouseMgrClass::Set_Mouse_Mode (MOUSE_MODE new_mode)
{
	//
	// Change the camera mode
	//
	CameraMgr *cameramgr = ::Get_Camera_Mgr ();		
	cameramgr->Set_Camera_Mode (m_pModeObjects[new_mode]->m_LButtonMode);

	if (m_MouseMode != new_mode) {
		
		//
		//	Notify the old mode that it is ending
		//
		m_pModeObjects[m_MouseMode]->On_Mode_Exit ();

		//
		//	Set the new mode
		//
		m_MouseMode = new_mode;
		m_pModeObjects[m_MouseMode]->On_Mode_Set ();
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//	Handle_LButton_Down
//
///////////////////////////////////////////////////////////////
void
MouseModeClass::Handle_LButton_Down
(
	UINT		flags,
	CPoint	point
)
{
	// Default new mode is lbutton down only
	CameraMgr::CAMERA_MODE new_mode = m_LButtonMode;

	//
	// If both buttons are down, then set that mode
	//
	if (flags & MK_RBUTTON) {		
		new_mode = m_BothButtonMode;		
	}

	//
	// Change the camera mode
	//
	CameraMgr *cameramgr = ::Get_Camera_Mgr ();		
	cameramgr->Set_Camera_Mode (new_mode);

	m_bUpdate = true;
	m_LastMousePoint = point;
	::Get_Main_View ()->SetCapture ();
	return ;
}


///////////////////////////////////////////////////////////////
//
//	Handle_LButton_Up
//
///////////////////////////////////////////////////////////////
void
MouseModeClass::Handle_LButton_Up
(
	UINT		flags,
	CPoint	point
)
{
	// Is the right button down?
	if (flags & MK_RBUTTON) {

		// Change the camera mode
		CameraMgr *cameramgr = ::Get_Camera_Mgr ();			
		cameramgr->Set_Camera_Mode (m_RButtonMode);

	} else {

		// No need to update the camera anymore
		m_bUpdate = false;
		ReleaseCapture ();
	}

	m_LastMousePoint = point;
	return ;
}


///////////////////////////////////////////////////////////////
//
//	Handle_RButton_Down
//
///////////////////////////////////////////////////////////////
void
MouseModeClass::Handle_RButton_Down
(
	UINT		flags,
	CPoint	point
)
{
	// Default new mode is rbutton down only
	CameraMgr::CAMERA_MODE new_mode = m_RButtonMode;

	//
	// If both buttons are down, then set that mode
	//
	if (flags & MK_LBUTTON) {
		new_mode = m_BothButtonMode;		
	}

	//
	// Change the camera mode
	//
	CameraMgr *cameramgr = ::Get_Camera_Mgr ();		
	cameramgr->Set_Camera_Mode (new_mode);

	m_bUpdate = true;
	m_LastMousePoint = point;
	::Get_Main_View ()->SetCapture ();
	return ;
}


///////////////////////////////////////////////////////////////
//
//	Handle_RButton_Up
//
///////////////////////////////////////////////////////////////
void
MouseModeClass::Handle_RButton_Up
(
	UINT flags,
	CPoint point
)
{

	// Is the right button down?
	if (flags & MK_LBUTTON) {

		// Change the camera mode
		CameraMgr *cameramgr = ::Get_Camera_Mgr ();			
		cameramgr->Set_Camera_Mode (m_LButtonMode);

	} else {

		// No need to update the camera anymore
		m_bUpdate = false;
		ReleaseCapture ();
	}

	m_LastMousePoint = point;
	return ;
}


///////////////////////////////////////////////////////////////
//
//	Handle_Mouse_Move
//
///////////////////////////////////////////////////////////////
void
MouseModeClass::Handle_Mouse_Move
(
	UINT		/* flags */,
	CPoint	point
)
{
	m_MousePoint = point;
	if (m_bUpdate) {

		CLevelEditView *view = Get_Main_View ();

		// Get the bouding rectangle of the main view
		RECT rect;
		view->GetClientRect (&rect);

		//
		// Calculcate the delta's for both x and y as a scaled percent from 0 to 1.
		//
		float deltax = (float(point.x - m_LastMousePoint.x))/(float(rect.right - rect.left));
		float deltay = (float(point.y - m_LastMousePoint.y))/(float(rect.bottom - rect.top));

		// Have the camera manager update the display based on the deltas
		CameraMgr *cameramgr = ::Get_Camera_Mgr ();			
		cameramgr->Update_Camera (deltax, deltay);

		// This point now becomes our last point
		m_LastMousePoint = point;
	}

	return ;
}


//**********************************************************************************************
//*
//*	Start of MMObjectManipulateClass
//*
//**********************************************************************************************


/////////////////////////////////////////////////////////////
//
//	Change_Operation
//
///////////////////////////////////////////////////////////////
void
MMObjectManipulateClass::Change_Operation (OBJECT_MODE type)
{
	if (type != m_ObjectMode) {
		
		// Let the scene editor know we are ending a previous operation
		if ((m_ObjectMode == MODE_MOVE) ||
			 (m_ObjectMode == MODE_ROTATE)) {
			::Get_Scene_Editor ()->End_Operation ();
		}

		// Remember our new mode
		m_ObjectMode = type;

		// Let the scene editor know we staring a new operation
		if (m_ObjectMode == MODE_MOVE) {
			::Get_Scene_Editor ()->Begin_Operation (OPERATION_MOVE);

			//
			//	Determine where our initial point is
			//
			Matrix3D coord_system (1);
			m_CurrentMovePos	= MoverClass::Calc_New_Position (coord_system, m_IntersectPoint);

		} else if (m_ObjectMode == MODE_ROTATE) {
			::Get_Scene_Editor ()->Begin_Operation (OPERATION_ROTATE);
		}
	}

	m_ForceDropToGround = false;
	return ;
}


/////////////////////////////////////////////////////////////
//
//	Handle_LButton_Dblclk
//
///////////////////////////////////////////////////////////////
void
MMObjectManipulateClass::Handle_LButton_Dblclk
(
	UINT		flags,
	CPoint	point
)
{
	// Allow the base class to process this message
	MouseModeClass::Handle_LButton_Dblclk (flags, point);

	// Find the item the user clicked on
	NodeClass *node = ::Get_Scene_Editor ()->Find_Node_At_Point (point);
	if (node != NULL) {
		
		//
		// Display the settings dialog for this node
		//
		node->Show_Settings_Dialog ();
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//	Handle_LButton_Down
//
///////////////////////////////////////////////////////////////
void
MMObjectManipulateClass::Handle_LButton_Down
(
	UINT		flags,
	CPoint	point
)
{
	// Allow the base class to process this message
	MouseModeClass::Handle_LButton_Down (flags, point);

	//
	// If there was no 'special' functionality at this point (like grab handles)
	// then process as normal
	//
	if (::Get_Scene_Editor ()->Execute_Function_At_Point (point) == false) {
		
		// Only the left mouse button is down so we are
		// in select mode
		Change_Operation (MODE_SELECT);

		//
		//	Determine which node the user clicked on (and where)
		//
		NodeClass *node = ::Get_Scene_Editor ()->Find_Node_At_Point (point, &m_IntersectPoint);

		//
		// Either toggle the selection state of this node,
		// or reset the selection group to contain only this node.
		//
		if (flags & MK_CONTROL) {
			::Get_Scene_Editor ()->Toggle_Selection (node);
		} else {
			::Get_Scene_Editor ()->Set_Selection (node);
		}

		// Make sure we have the mouse captured
		::Get_Main_View ()->SetCapture ();
		m_LastMousePoint = point;
	
	} else {

		Change_Operation (MODE_NONE);
		::ReleaseCapture ();
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//	Handle_LButton_Up
//
///////////////////////////////////////////////////////////////
void
MMObjectManipulateClass::Handle_LButton_Up
(
	UINT		flags,
	CPoint	point
)
{
	MouseModeClass::Handle_LButton_Up (flags, point);

	// Is the right mouse button down?
	if (flags & MK_RBUTTON) {
		
		// Only the right button is down so we are in rotate mode
		Change_Operation (MODE_ROTATE);

		// Make sure we have the mouse captured
		::Get_Main_View ()->SetCapture ();
		m_bUpdate = true;

	} else {

		// No buttons are down so release the mouse capture
		::ReleaseCapture ();
		Change_Operation (MODE_NONE);
	}
		
	return ;
}


///////////////////////////////////////////////////////////////
//
//	Handle_RButton_Down
//
///////////////////////////////////////////////////////////////
void
MMObjectManipulateClass::Handle_RButton_Down
(
	UINT		flags,
	CPoint	point
)
{
	MouseModeClass::Handle_RButton_Down (flags, point);

	if (flags & MK_LBUTTON) {
		
		::Get_Selection_Mgr ().Clone_Group ();

	} else {

		// Only the right mouse button is down so we are
		// in select mode
		Change_Operation (MODE_ROTATE);
	}

	// Make sure we have the mouse captured
	::Get_Main_View ()->SetCapture ();	
	m_LastMousePoint = point;
	return ;
}


///////////////////////////////////////////////////////////////
//
//	Handle_RButton_Up
//
///////////////////////////////////////////////////////////////
void
MMObjectManipulateClass::Handle_RButton_Up
(
	UINT		flags,
	CPoint	point
)
{
	MouseModeClass::Handle_RButton_Up (flags, point);

	// Is the left mouse button down?
	if (flags & MK_LBUTTON) {
		
		// Only the left button is down so we are in move mode
		Change_Operation (MODE_MOVE);

		// Make sure we have the mouse captured
		::Get_Main_View ()->SetCapture ();
		m_bUpdate = true;

		// Set the 'object move' cursor.
		::SetCursor (::LoadCursor (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDC_OBJ_MOVE)));

	} else {

		// No buttons are down so release the mouse capture
		::ReleaseCapture ();
		Change_Operation (MODE_NONE);
	}
		
	return ;
}


///////////////////////////////////////////////////////////////
//
//	Handle_Mouse_Move
//
///////////////////////////////////////////////////////////////
void
MMObjectManipulateClass::Handle_Mouse_Move
(
	UINT		flags,
	CPoint	point
)
{
	if (m_ObjectMode == MODE_SELECT) {
		if ((abs (point.x - m_LastMousePoint.x) > 3) ||
			 (abs (point.y - m_LastMousePoint.y) > 3)) {
			
			// The user moved the mouse more than our
			// alloted selection fudge factor so now
			// we are in move mode.
			Change_Operation (MODE_MOVE);

			// If the control button is down, then do a copy
			// operation before the move.
			if (flags & MK_SHIFT) {
				//::Get_Selection_Mgr ().Clone_Group ();
			}

			// Set the 'object move' cursor.
			::SetCursor (::LoadCursor (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDC_OBJ_MOVE)));
		}
	}

	if (m_ObjectMode == MODE_MOVE) {
		Move_Selection (point);
	} else if (m_ObjectMode == MODE_ROTATE) {
		Rotate_Selection (point);
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//	Set_Move_Nodes_Mode
//
///////////////////////////////////////////////////////////////
void
MMObjectManipulateClass::Set_Move_Nodes_Mode (void)
{
	SelectionMgrClass &sel_mgr	= ::Get_Selection_Mgr ();
	m_CurrentMovePos				= sel_mgr.Get_Center ();
	m_ObjectMode					= MODE_MOVE;
	m_ForceDropToGround			= true;
	return ;
}


///////////////////////////////////////////////////////////////
//
//	On_Mode_Set
//
///////////////////////////////////////////////////////////////
void
MMObjectManipulateClass::On_Mode_Set (void)
{
	// Reset the state
	::ReleaseCapture ();
	Change_Operation (MODE_NONE);
	return ;
}


/*Vector2
World_To_Device (const Vector3 &world_pos)
{
	Vector2 device_pos (0, 0);

	Vector3 camera_pos = ::Get_Camera_Mgr ()->Get_Camera ()->Get_Position ();
	//Vector3 ray_start = 

	const PlaneClass *planes = ::Get_Camera_Mgr ()->Get_Camera ()->Get_Frustum_Planes ();

	float percent = 0;
	if (planes[0]->Compute_Intersection (camera_pos, world_pos, &percent)) {
		Vector3 view_pos = camera_pos + ((world_pos - camera_pos) * percent);
		device_pos.X = view_pos.X - 
	}

	return device_pos;
}*/


///////////////////////////////////////////////////////////////
//
//	Move_Selection
//
///////////////////////////////////////////////////////////////
void
MMObjectManipulateClass::Move_Selection (CPoint point)
{
	if (	m_ForceDropToGround || 
			::Get_Current_Document ()->Get_Mode_Modifiers () & MODE_MOD_DROP_TO_GROUND) {

		//
		//	Build a list of nodes from the current selection set
		//
		NODE_LIST list;
		Vector3 center = ::Get_Scene_Editor ()->Build_Node_List (list);

		//
		//	Determine what ray to cast the node along
		//
		Vector3 ray_start;
		Vector3 ray_end;
		MoverClass::Get_Mouse_Ray (500.00F, ray_start, ray_end);

		//
		//	If we are in drop-to-ground mode, then reposition the current
		//	selection set along the line-of-sight ray
		//
		Vector3 tracking_pt (m_CurrentMovePos);
		tracking_pt.Z = center.Z;
		m_CurrentMovePos = MoverClass::Position_Nodes_Along_Ray (list, tracking_pt, ray_start, ray_end);

	} else {

		//
		//	Build a list of nodes from the current selection set
		//
		NODE_LIST list;
		::Get_Scene_Editor ()->Build_Node_List (list);

		//
		//	Move the selected nodes using the current axis restrictions
		// and coordinate system
		//
		m_CurrentMovePos = MoverClass::Move_Nodes (list, m_CurrentMovePos);
	}

	m_LastMousePoint = point;
	return ;
}


///////////////////////////////////////////////////////////////
//
//	Rotate_Selection
//
///////////////////////////////////////////////////////////////
void
MMObjectManipulateClass::Rotate_Selection (CPoint point)
{
	// Get the bouding rectangle of the main view
	CRect rect;
	::Get_Main_View ()->GetClientRect (&rect);

	//
	// Calculcate the delta's for both x and y as a scaled percent from -1 to 1.
	//
	float midx	= (float)(rect.Width () >> 1);
	float midy	= (float)(rect.Height () >> 1);
	float lastx	= -(float(m_LastMousePoint.x - midx)) / midx;
	float lasty	= (float(m_LastMousePoint.y - midy)) / midy;
	float currx	= -(float(point.x - midx)) / midx;
	float curry	= (float(point.y - midy)) / midy;

	//
	// Rotaate the item like it was trackballed
	//
	Quaternion rotation = ::Trackball (lastx, lasty, currx, curry, 0.9F);

	//
	//	Now perform the rotation
	//
	float deltax = (float(point.x - m_LastMousePoint.x))/(float(rect.right - rect.left));
	float deltay = (float(point.y - m_LastMousePoint.y)) / (float(rect.bottom - rect.top));
	MoverClass::Rotate_Nodes (deltax, deltay, rotation);

	// This point now becomes our last point
	m_LastMousePoint = point;
	return ;
}


//**********************************************************************************************
//*
//*	Start of MMWaypathEditClass
//*
//**********************************************************************************************


///////////////////////////////////////////////////////////////
//
//	Exit_Mode
//
///////////////////////////////////////////////////////////////
void
MMWaypathEditClass::Exit_Mode (void)
{
	m_Waypath		= NULL;
	m_CurrentPoint	= -1;
	::Get_Mouse_Mgr ()->Set_Mouse_Mode (MouseMgrClass::MODE_OBJECT_MANIPULATE);	
	return ;
}


///////////////////////////////////////////////////////////////
//
//	On_Mode_Exit
//
///////////////////////////////////////////////////////////////
void
MMWaypathEditClass::On_Mode_Exit (void)
{
	if (m_Waypath != NULL && m_CurrentPoint >= 0) {
		m_Waypath->Delete_Point (m_CurrentPoint);
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//	Handle_LButton_Down
//
///////////////////////////////////////////////////////////////
void
MMWaypathEditClass::Handle_LButton_Down
(
	UINT		/* flags */,
	CPoint	/* point */
)
{
	/*NodeClass *node = ::Get_Scene_Editor ()->Find_Node_At_Point (point);
	if ((node != NULL) && (node->Get_Type () == NODE_TYPE_WAYPATH)) {
		WaypathNodeClass *waypath = (WaypathNodeClass *)node;
	}*/

	return ;
}


///////////////////////////////////////////////////////////////
//
//	Handle_LButton_Up
//
///////////////////////////////////////////////////////////////
void
MMWaypathEditClass::Handle_LButton_Up
(
	UINT		flags,
	CPoint	/* point */
)
{
	if (m_Waypath != NULL && m_CurrentPoint >= 0) {

		//
		//	If the user is holding the shift key, then exit the mode...
		//
		if (flags & MK_SHIFT) {
			Exit_Mode ();
		} else {

			//
			//	Determine where are last point was
			//
			Vector3 last_pos (0, 0, 0);
			m_Waypath->Get_Point (m_CurrentPoint, last_pos);

			//
			//	Lock that point and start a new point
			//
			m_CurrentPoint = m_Waypath->Add_Point (last_pos);
			::Get_Main_View ()->SetCapture ();	
		}
	}
		
	return ;
}


///////////////////////////////////////////////////////////////
//
//	Handle_RButton_Down
//
///////////////////////////////////////////////////////////////
void
MMWaypathEditClass::Handle_RButton_Down
(
	UINT		/* flags */,
	CPoint	/* point */
)
{
	return ;
}


///////////////////////////////////////////////////////////////
//
//	Handle_RButton_Up
//
///////////////////////////////////////////////////////////////
void
MMWaypathEditClass::Handle_RButton_Up
(
	UINT		/* flags */,
	CPoint	/* point */
)
{
	return ;
}


///////////////////////////////////////////////////////////////
//
//	Handle_Mouse_Move
//
///////////////////////////////////////////////////////////////
void
MMWaypathEditClass::Handle_Mouse_Move
(
	UINT		/* flags */,
	CPoint	point
)
{	
	if (m_Waypath != NULL && m_CurrentPoint >= 0) {
		
		//
		//	Get a pointer to this waypoint
		//
		WaypointNodeClass *waypoint = NULL;
		m_Waypath->Get_Point (m_CurrentPoint, &waypoint);

		//
		//	Reposition this node
		//
		MoverClass::Position_Node_Along_Ray (waypoint);
	}

	m_LastMousePoint = point;
	return ;
}


///////////////////////////////////////////////////////////////
//
//	On_Mode_Set
//
///////////////////////////////////////////////////////////////
void
MMWaypathEditClass::On_Mode_Set (void)
{
	// Reset the state
	::ReleaseCapture ();

	m_Waypath		= NULL;
	m_CurrentPoint	= -1;
	return ;
}



//**********************************************************************************************
//*
//*	Start of MMGrabHandleDragClass
//*
//**********************************************************************************************


///////////////////////////////////////////////////////////////
//
//	Handle_LButton_Up
//
///////////////////////////////////////////////////////////////
void
MMGrabHandleDragClass::Handle_LButton_Up
(
	UINT		/* flags */,
	CPoint	/* point */
)
{
	// Put the mouse mode back to what it was
	::Get_Mouse_Mgr ()->Set_Mouse_Mode (MouseMgrClass::MODE_OBJECT_MANIPULATE);		

	// Let the node know its done dragging
	if (m_Node != NULL) {
		m_Node->On_Vertex_Drag_End (m_Vertex);
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//	Handle_Mouse_Move
//
///////////////////////////////////////////////////////////////
void
MMGrabHandleDragClass::Handle_Mouse_Move
(
	UINT		/* flags */,
	CPoint	point
)
{
	//
	// Pass this information onto the node so it can process
	// the 'drag' in whatever fashion it wants
	//
	if (m_Node != NULL) {
		m_Node->On_Vertex_Drag (m_Vertex, point);
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//	Set_Node_Info
//
///////////////////////////////////////////////////////////////
void
MMGrabHandleDragClass::Set_Node_Info
(
	NodeClass *	node,
	int			vertex_index
)
{
	m_Node = node;
	m_Vertex = vertex_index;

	// Notify the undo manager of the operation
	::Get_Scene_Editor ()->Begin_Operation (OPERATION_RESIZE, m_Node);

	//
	// Let the node know its done dragging
	//
	if (m_Node != NULL) {
		m_Node->On_Vertex_Drag_Begin (m_Vertex);
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//	Handle_LButton_Down
//
///////////////////////////////////////////////////////////////
void
MMHeightfieldEditClass::Handle_LButton_Down (UINT /* flags */, CPoint /* point */)
{
	return ;
}


///////////////////////////////////////////////////////////////
//
//	Handle_LButton_Up
//
///////////////////////////////////////////////////////////////
void
MMHeightfieldEditClass::Handle_LButton_Up (UINT /* flags */, CPoint /* point */)
{
	return ;
}


///////////////////////////////////////////////////////////////
//
//	Handle_RButton_Down
//
///////////////////////////////////////////////////////////////
void
MMHeightfieldEditClass::Handle_RButton_Down (UINT /* flags */, CPoint /* point */)
{
	return ;
}


///////////////////////////////////////////////////////////////
//
//	Handle_RButton_Up
//
///////////////////////////////////////////////////////////////
void
MMHeightfieldEditClass::Handle_RButton_Up (UINT /* flags */, CPoint /* point */)
{
	return ;
}


///////////////////////////////////////////////////////////////
//
//	On_Mode_Set
//
///////////////////////////////////////////////////////////////
void
MMHeightfieldEditClass::On_Mode_Set (void)
{
	return ;
}


///////////////////////////////////////////////////////////////
//
//	On_Mode_Exit
//
///////////////////////////////////////////////////////////////
void
MMHeightfieldEditClass::On_Mode_Exit (void)
{
	return ;
}


