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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/WaypathNode.cpp      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 7/03/01 2:35p                                               $*
 *                                                                                             *
 *                    $Revision:: 17                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "WaypathNode.h"
#include "sceneeditor.h"
#include "collisiongroups.h"
#include "persistfactory.h"
#include "editorchunkids.h"
#include "preset.h"
#include "chunkio.h"
#include "nodemgr.h"
#include "segline.h"
#include "WaypointNode.h"
#include "waypath.h"
#include "waypoint.h"
#include "Pathfind.h"
#include "nodeinfopage.h"
#include "positionpage.h"
#include "editorpropsheet.h"
#include "PathfindPortal.h"


//////////////////////////////////////////////////////////////////////////////
//	Persist factory
//////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<WaypathNodeClass, CHUNKID_NODE_WAYPATH> _WaypathNodePersistFactory;


enum
{
	CHUNKID_VARIABLES			= 0x02291130,
	CHUNKID_BASE_CLASS,
	CHUNKID_WAYPOINTS
};

enum
{
	VARID_WAYPOINT_PTR		= 0x01,
	VARID_OLD_PTR				= 0x02,
	VARID_FLAGS					= 0x03,
};


//////////////////////////////////////////////////////////////////////////////
//
//	WaypathNodeClass
//
//////////////////////////////////////////////////////////////////////////////
WaypathNodeClass::WaypathNodeClass (PresetClass *preset)
	:	m_PhysObj (NULL),
		m_Flags (FLAG_HUMAN | FLAG_GROUND_VEHICLE),
		m_RuntimeWaypath (NULL),
		m_HasLoadCompleted (true),
		NodeClass (preset)		
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	WaypathNodeClass
//
//////////////////////////////////////////////////////////////////////////////
WaypathNodeClass::WaypathNodeClass (const WaypathNodeClass &src)
	:	m_PhysObj (NULL),
		m_Flags (FLAG_HUMAN | FLAG_GROUND_VEHICLE),
		m_RuntimeWaypath (NULL),
		m_HasLoadCompleted (true),
		NodeClass (NULL)
{
	*this = src;
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	~WaypathNodeClass
//
//////////////////////////////////////////////////////////////////////////////
WaypathNodeClass::~WaypathNodeClass (void)
{	
	Remove_From_Scene ();	
	MEMBER_RELEASE (m_PhysObj);

	Free_Waypoints ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//	Note:  This may be called more than once.  It is used as an 'initialize'
// and a 're-initialize'.
//
//////////////////////////////////////////////////////////////////////////////
void
WaypathNodeClass::Initialize (void)
{
	MEMBER_RELEASE (m_PhysObj);
		
	SegmentedLineClass *line = new SegmentedLineClass;
	line->Set_Width (0.075F);
	line->Set_Color (Vector3 (0, 0.75F, 0));
	line->Set_Opacity (1.0F);
	//line->Set_Shader (ShaderClass::_PresetOpaqueSolidShader);
	
	// Create the new physics object
	m_PhysObj = new DecorationPhysClass;
	
	//
	// Configure the physics object with information about
	// its new render object and collision data.
	//
	m_PhysObj->Set_Model (line);
	m_PhysObj->Set_Transform (Matrix3D(1));
	m_PhysObj->Set_Collision_Group (EDITOR_COLLISION_GROUP);
	m_PhysObj->Peek_Model ()->Set_Collision_Type (COLLISION_TYPE_6);
	m_PhysObj->Peek_Model ()->Set_User_Data ((PVOID)&m_HitTestInfo, false);
	m_PhysObj->Set_Transform (m_Transform);
	
	// Release our hold on the render object pointer
	MEMBER_RELEASE (line);

	//
	//	Make sure the path has at least one waypoint
	//
	if (m_PointList.Count () == 0) {
		Add_Point (m_Transform.Get_Translation ());
	}

	if (m_HasLoadCompleted) {
		Update_Line ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
WaypathNodeClass::Get_Factory (void) const
{	
	return _WaypathNodePersistFactory;
}


/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
bool
WaypathNodeClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		NodeClass::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);
		WaypathNodeClass *this_ptr = this;
		WRITE_MICRO_CHUNK_PTR (csave, VARID_OLD_PTR, this_ptr);
		WRITE_MICRO_CHUNK (csave, VARID_FLAGS, m_Flags);		
	csave.End_Chunk ();

	//
	//	Save the waypoints in their own chunk
	//
	csave.Begin_Chunk (CHUNKID_WAYPOINTS);

		NODE_LIST node_list;
		for (int index = 0; index < m_PointList.Count (); index ++) {
			node_list.Add (m_PointList[index]);
		}

		NodeMgrClass::Save_Node_List (csave, node_list);

	csave.End_Chunk ();

	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////
bool
WaypathNodeClass::Load (ChunkLoadClass &cload)
{
	m_OldStylePointList.Delete_All ();
	Free_Waypoints ();
	m_HasLoadCompleted = false;

	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_BASE_CLASS:
				NodeClass::Load (cload);
				break;
			
			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;

			case CHUNKID_WAYPOINTS:
			{
				//
				//	Load the list of nodes
				//
				NODE_LIST node_list;
				NodeMgrClass::Load_Node_List (cload, node_list);

				//
				//	Store each node (as a waypoint) in our list
				//
				for (int index = 0; index < node_list.Count (); index ++) {
					WaypointNodeClass *node = node_list[index]->As_WaypointNodeClass ();
					if (node != NULL) {
						m_PointList.Add (node);
					}
				}
			}
			break;
		}

		cload.Close_Chunk ();
	}

	SaveLoadSystemClass::Register_Post_Load_Callback (this);
	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////
bool
WaypathNodeClass::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK (cload, VARID_FLAGS, m_Flags);

			case VARID_WAYPOINT_PTR:
			{
				//
				//	Read the old waypoint ptr from the chunk and add it to our
				// list.  We will remap it later.
				//				
				WaypointNodeClass *waypoint = NULL;
				cload.Read (&waypoint, sizeof (waypoint));				
				m_OldStylePointList.Add (waypoint);				
			}
			break;

			case VARID_OLD_PTR:
			{
				//
				//	Read the old pointer from the chunk and submit it
				// to the remapping system.
				//				
				WaypathNodeClass *old_ptr = NULL;
				cload.Read (&old_ptr, sizeof (int));
				SaveLoadSystemClass::Register_Pointer (old_ptr, this);
			}
			break;
		}

		cload.Close_Micro_Chunk ();
	}

	//
	//	Register each of the pointers in our waypoint list for remapping
	//
	for (int index = 0; index < m_OldStylePointList.Count (); index ++) {
		REQUEST_POINTER_REMAP ((void **)&m_OldStylePointList[index]);
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	On_Post_Load
//
///////////////////////////////////////////////////////////////////////
void
WaypathNodeClass::On_Post_Load (void)
{
	//
	//	Do we need to import any waypoints from an old-style save?
	//
	if (m_OldStylePointList.Count () > 0) {
		Free_Waypoints ();
		
		//
		//	Add a ref-count to each of the waypoints and remove it from the node manager
		//
		DynamicVectorClass<WaypointNodeClass *> new_point_list;
		for (int index = 0; index < m_OldStylePointList.Count (); index ++) {
			WaypointNodeClass *waypoint = m_OldStylePointList[index];
			if (waypoint != NULL) {
				new_point_list.Add (waypoint);
				SAFE_ADD_REF (waypoint);
				
				//
				//	Remove the node from the manager (we'll manage the waypoints)
				//
				NodeMgrClass::Remove_Node (waypoint);
			}
		}

		m_PointList = new_point_list;
		m_OldStylePointList.Delete_All ();
	}

	//
	//	Add a reference count to each of the points
	//
	/*for (int index = 0; index < m_PointList.Count (); index ++) {
		WaypointNodeClass *waypoint = m_PointList[index];
		SAFE_ADD_REF (waypoint);
	}*/

	//
	//	Make sure the line is refreshed
	//
	Update_Line ();
	m_HasLoadCompleted = true;
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Set_Transform
//
///////////////////////////////////////////////////////////////////////
void
WaypathNodeClass::Set_Transform (const Matrix3D &tm)
{

	SegmentedLineClass *line = Peek_Line ();
	if (line != NULL) {
			
		//
		//	Transform all the points in the path
		//
		int index = m_PointList.Count ();
		while (index --) {
			
			//
			//	Get this point's object space location
			//
			Vector3 point_loc (0, 0, 0);
			line->Get_Point_Location (index, point_loc);

			//
			//	Reposition the point relative to the new path transform
			//
			Vector3 new_pos = tm * point_loc;
			m_PointList[index]->Parent_Set_Position ((new_pos - Vector3 (0, 0, 0.2F)));
		}
	}

	NodeClass::Set_Transform (tm);
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////
const WaypathNodeClass &
WaypathNodeClass::operator= (const WaypathNodeClass &src)
{
	//
	//	Allow the base class to process
	//
	NodeClass::operator= (src);
	Free_Waypoints ();

	//
	//	Copy the src waypath's list of waypoints...
	//
	int count = src.m_PointList.Count ();
	for (int index = 0; index < count; index ++) {
		WaypointNodeClass *waypoint		= src.m_PointList[index];
		WaypointNodeClass *new_waypoint	= (WaypointNodeClass *)waypoint->Clone ();
		new_waypoint->Set_Waypath (this);
		m_PointList.Add (new_waypoint);
		//NodeMgrClass::Add_Node (new_waypoint);
	}

	//
	//	Copy the flags
	//
	m_Flags = src.m_Flags;

	//
	//	Refresh the line
	//
	Update_Line ();
	return *this;
}


//////////////////////////////////////////////////////////////////////
//
//	Pre_Export
//
//////////////////////////////////////////////////////////////////////
void
WaypathNodeClass::Pre_Export (void)
{
	//
	//	Remove ourselves from the 'system' so we don't get accidentally
	// saved during the export. 
	//
	Add_Ref ();
	if (m_PhysObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Remove_Object (m_PhysObj);
	}

	//
	//	Create a runtime waypath object and register it with
	// the system so it gets exported with the game.
	//
	m_RuntimeWaypath = new WaypathClass;
	m_RuntimeWaypath->Set_ID (Get_ID ());
	m_RuntimeWaypath->Set_Flags (m_Flags);

	//
	//	Add all the waypoints to the new waypath
	//
	Vector3 last_pos (0, 0, 0);
	for (int index = 0; index < m_PointList.Count (); index ++) {
		Vector3 curr_pos (0, 0, 0);
		WaypointNodeClass *waypoint = m_PointList[index];
		if (waypoint != NULL) {

			//
			//	Calculate the position of this waypoint
			//
			curr_pos = waypoint->Get_Position () + Vector3 (0, 0, 0.2F);

			//
			//	Check to see if the line segment formed by the last point and this point
			// intersects any pathfind-action portals (transition zones)
			//
			if (index > 0) {				
				DynamicVectorClass<PathfindPortalClass *> list;
				PathfindClass::Get_Instance ()->Find_Portals (last_pos, curr_pos, list, true);

				//
				//	Now add new nodes for any portals this segment intersects
				//
				for (int list_index = 0; list_index < list.Count (); list_index ++) {
					PathfindActionPortalClass *portal = list[list_index]->As_PathfindActionPortalClass ();
					AABoxClass portal_box;
					portal->Get_Bounding_Box (portal_box);

					//
					//	Create a runtime waypoint and add it to the runtime
					// waypath.
					//			
					WaypointClass *new_waypoint = new WaypointClass;
					new_waypoint->Set_ID (NodeMgrClass::Get_Node_ID (NODE_TYPE_WAYPOINT));
					new_waypoint->Set_Position (portal_box.Center);
					new_waypoint->Set_Action_Portal (portal);					
					m_RuntimeWaypath->Add_Point (*new_waypoint);
					MEMBER_RELEASE (new_waypoint);
				}
			}

			//
			//	Create a runtime waypoint and add it to the runtime
			// waypath.
			//			
			WaypointClass *runtime_waypoint = new WaypointClass;
			runtime_waypoint->Set_ID (waypoint->Get_ID ());
			runtime_waypoint->Set_Position (curr_pos);
			
			runtime_waypoint->Set_Flags (0);
			runtime_waypoint->Set_Flag (WaypointClass::FLAG_REQUIRES_JUMP, waypoint->Get_Flag (WaypointNodeClass::FLAG_REQUIRES_JUMP));
			
			m_RuntimeWaypath->Add_Point (*runtime_waypoint);
			MEMBER_RELEASE (runtime_waypoint);
		}

		last_pos = curr_pos;
	}

	PathfindClass::Get_Instance ()->Add_Waypath (m_RuntimeWaypath);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Post_Export
//
//////////////////////////////////////////////////////////////////////
void
WaypathNodeClass::Post_Export (void)
{
	//
	//	Put ourselves back into the system
	//
	if (m_PhysObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Add_Dynamic_Object (m_PhysObj);
	}
	Release_Ref ();

	//
	//	Remove the waypath object we just exported from the system
	//
	if (m_RuntimeWaypath != NULL) {
		PathfindClass::Get_Instance ()->Remove_Waypath (m_RuntimeWaypath);
		MEMBER_RELEASE (m_RuntimeWaypath);
	}
	return ;
}
	

//////////////////////////////////////////////////////////////////////
//
//	Free_Waypoints
//
//////////////////////////////////////////////////////////////////////
void
WaypathNodeClass::Free_Waypoints (void)
{
	//
	//	Free our hold on each of the waypoints
	//
	for (int index = 0; index < m_PointList.Count (); index ++) {
		WaypointNodeClass *waypoint = m_PointList[index];

		if (waypoint != NULL) {
			waypoint->Set_Waypath (NULL);
			waypoint->Remove_From_Scene ();
		}

		MEMBER_RELEASE (waypoint);
	}

	m_PointList.Delete_All ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Add_To_Scene
//
//////////////////////////////////////////////////////////////////////////////
void
WaypathNodeClass::Add_To_Scene (void)
{
	//
	//	Add all the waypoints to the scene
	//
	for (int index = 0; index < m_PointList.Count (); index ++) {
		WaypointNodeClass *waypoint = m_PointList[index];
		if (waypoint != NULL) {
			waypoint->Add_To_Scene ();
		}
	}

	NodeClass::Add_To_Scene ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Remove_From_Scene
//
//////////////////////////////////////////////////////////////////////////////
void
WaypathNodeClass::Remove_From_Scene (void)
{
	SceneEditorClass *scene = ::Get_Scene_Editor ();
	if (scene != NULL && m_IsInScene) {

		//
		//	Remove all the waypoints from the scene
		//
		for (int index = 0; index < m_PointList.Count (); index ++) {
			WaypointNodeClass *waypoint = m_PointList[index];
			if (waypoint != NULL) {
				waypoint->Remove_From_Scene ();
			}
		}
	}
	
	NodeClass::Remove_From_Scene ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Delete_Point
//
//////////////////////////////////////////////////////////////////////////////
void
WaypathNodeClass::Delete_Point (int index)
{
	SANITY_CHECK(index >= 0 && index < m_PointList.Count ()) {
		return ;
	}

	//
	//	Remove the point from this path
	//
	WaypointNodeClass *waypoint = m_PointList[index];
	waypoint->Set_Waypath (NULL);
	waypoint->Remove_From_Scene ();
	//NodeMgrClass::Remove_Node (waypoint);
	MEMBER_RELEASE (waypoint);	
	m_PointList.Delete (index);

	if (m_PointList.Count () > 0) {

		//
		//	Refresh the line
		//
		Update_Line ();

	} else {

		//
		//	Delete this path if there are no more points left
		//
		::Get_Scene_Editor ()->Delete_Node (this, false);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Insert_Point
//
//////////////////////////////////////////////////////////////////////////////
void
WaypathNodeClass::Insert_Point (int index, const Vector3 &point)
{
	SANITY_CHECK(index >= 0 && index <= m_PointList.Count ()) {
		return ;
	}

	//
	//	Create a new waypoint and insert it into the list
	//
	WaypointNodeClass *waypoint = Create_Waypoint (point);
	m_PointList.Insert (index, waypoint);

	//
	//	Refresh the line
	//
	Update_Line ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Find_Index
//
//////////////////////////////////////////////////////////////////////////////
int
WaypathNodeClass::Find_Index (WaypointNodeClass *waypoint)
{
	int retval	= -1;
	
	//
	//	Loop over all the waypoints in our list until
	// we have found the matching pointer...
	//
	int index	= m_PointList.Count ();
	while (index --) {
		if (waypoint == m_PointList[index]) {
			retval = index;
			break;
		}
	}

	return index;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Add_Point
//
//////////////////////////////////////////////////////////////////////////////
int
WaypathNodeClass::Add_Point (const Vector3 &point)
{			
	//
	//	Create a new waypoint and add it to the end of our list...
	//
	WaypointNodeClass *waypoint = Create_Waypoint (point);
	m_PointList.Add (waypoint);

	//
	//	Refresh the line
	//
	Update_Line ();
	return (m_PointList.Count () - 1);
}


//////////////////////////////////////////////////////////////////////////////
//
//	On_Point_Moved
//
//////////////////////////////////////////////////////////////////////////////
void
WaypathNodeClass::On_Point_Moved (int index, const Vector3 &new_pos)
{
	SegmentedLineClass *line = Peek_Line ();
	if (line != NULL) {

		//
		//	Convert the position from world-space to object-space
		//
		Vector3 obj_space_pos (0, 0, 0);
		Vector3 line_pos = new_pos + Vector3 (0, 0, 0.2F);
		Matrix3D::Inverse_Transform_Vector (line->Get_Transform (), line_pos, &obj_space_pos);

		//
		//	Pass the new position onto the line...
		//
		line->Set_Point_Location (index, obj_space_pos);
	
		if (index == 0 && (Get_Flag (FLAG_LOOPING) || m_PointList.Count () == 1)) {
			line->Set_Point_Location (m_PointList.Count (), obj_space_pos);
		}

		m_PhysObj->Update_Cull_Box ();
	}

	return ;	
}


//////////////////////////////////////////////////////////////////////////////
//
//	Update_Line
//
//////////////////////////////////////////////////////////////////////////////
void
WaypathNodeClass::Update_Line (void)
{	
	SegmentedLineClass *line = Peek_Line ();
	if (line != NULL) {
		
		//
		//	Allocate a new array of points for the line render object
		//
		int count			= m_PointList.Count ();
		Vector3 *points	= new Vector3[count + 1];

		//
		//	Fill in the array of points with our waypoint locations
		//
		int index;
		for (index = 0; index < count; index ++) {

			WaypointNodeClass *waypoint = m_PointList[index];

			//
			//	Determine which model this waypoint should use...
			//
			if (index == 0) {
				waypoint->Set_Model (WaypointNodeClass::MODEL_START_PT);
			} else if (index == count-1) {
				if (Get_Flag (FLAG_TWO_WAY) == false) {
					waypoint->Set_Model (WaypointNodeClass::MODEL_END_PT);
				} else {
					waypoint->Set_Model (WaypointNodeClass::MODEL_START_PT);
				}
				
			} else {
				waypoint->Set_Model (WaypointNodeClass::MODEL_MIDDLE_PT);
			}

			//
			//	Convert the world space position of this waypoint into
			// a local coordinate system for the path.
			//
			Vector3 world_pos	= waypoint->Get_Position () + Vector3 (0, 0, 0.2F);
			Matrix3D::Inverse_Transform_Vector (m_Transform, world_pos, &points[index]);			
		}

		//
		//	Handle the single point path or a looping path
		//
		if (count == 1 || Get_Flag (FLAG_LOOPING)) {
			points[index++] = points[0];
			count ++;
		}

		//
		//	Pass the new point list onto the line render object
		//
		line->Set_Points (count, points);
		SAFE_DELETE_ARRAY (points);

		m_PhysObj->Update_Cull_Box ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Create_Waypoint
//
//////////////////////////////////////////////////////////////////////////////
WaypointNodeClass *
WaypathNodeClass::Create_Waypoint (const Vector3 &point)
{
	//
	//	Create the new waypoint
	//
	WaypointNodeClass *waypoint = new WaypointNodeClass;
	waypoint->Set_Waypath (this);
	waypoint->Parent_Set_Transform (Matrix3D (point));
	waypoint->Initialize ();
	NodeMgrClass::Setup_Node_Identity (*waypoint);

	//
	//	Insert the new waypoint into the scene if the
	// path is supposed to be in the scene...
	//
	if (m_IsInScene) {
		waypoint->Add_To_Scene ();		
	}

	//NodeMgrClass::Add_Node (waypoint);

	return waypoint;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Peek_Line
//
//////////////////////////////////////////////////////////////////////////////
SegmentedLineClass *
WaypathNodeClass::Peek_Line (void)
{
	SegmentedLineClass *line = NULL;
	if (m_PhysObj != NULL) {
		line = (SegmentedLineClass *)m_PhysObj->Peek_Model ();
	}

	return line;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Set_Flags
//
//////////////////////////////////////////////////////////////////////////////
void
WaypathNodeClass::Set_Flags (int flags)
{
	m_Flags = flags;
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Set_Flag
//
//////////////////////////////////////////////////////////////////////////////
void
WaypathNodeClass::Set_Flag (int flag, bool onoff)
{
	m_Flags &= ~flag;
	if (onoff) {
		m_Flags |= flag;
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Get_Flag
//
//////////////////////////////////////////////////////////////////////////////
bool
WaypathNodeClass::Get_Flag (int flag)
{
	return bool((m_Flags & flag) == flag);
}


//////////////////////////////////////////////////////////////////////////////
//
//	Hide
//
//////////////////////////////////////////////////////////////////////////////
void
WaypathNodeClass::Hide (bool hide)
{
	//
	//	Apply the same operation to all the points along the path
	//
	for (int index = 0; index < m_PointList.Count (); index ++) {
		WaypointNodeClass *waypoint = m_PointList[index];
		if (waypoint != NULL) {
			waypoint->Hide (hide);
		}
	}
	
	NodeClass::Hide (hide);
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Show_Settings_Dialog
//
//////////////////////////////////////////////////////////////////////////////
bool
WaypathNodeClass::Show_Settings_Dialog (void)
{
	NodeInfoPageClass	info_tab (this);
	PositionPageClass	pos_tab (this);
	
	//
	//	Add each tab to the property sheet
	//
	EditorPropSheetClass prop_sheet;
	prop_sheet.Add_Page (&info_tab);
	prop_sheet.Add_Page (&pos_tab);

	// Show the property sheet
	UINT ret_code = prop_sheet.DoModal ();
	
	// Return true if the user clicked OK
	return (ret_code == IDOK);
}


//////////////////////////////////////////////////////////////////////////////
//
//	On_Delete
//
//////////////////////////////////////////////////////////////////////////////
void
WaypathNodeClass::On_Delete (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	On_Restore
//
//////////////////////////////////////////////////////////////////////////////
void
WaypathNodeClass::On_Restore (void)
{
	return ;
}

