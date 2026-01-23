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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/SceneEditor.cpp              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/12/02 3:31p                                               $*
 *                                                                                             *
 *                    $Revision:: 116                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "sceneeditor.h"
#include "nodemgr.h"
#include "groupmgr.h"
#include "Utils.h"
#include "matrix3d.h"
#include "vector.h"
#include "leveleditdoc.h"
#include "filemgr.h"
#include "cameramgr.h"
#include "LevelEditView.h"
#include "hittestinfo.h"
#include "collisiongroups.h"
#include "mainfrm.h"
#include "WWAudio.h"
#include "AudibleSound.h"
#include "mesh.h"
#include "staticphys.h"
#include "phys3.h"
#include "_assetmgr.h"
#include "editorassetmgr.h"
#include "humanphys.h"
#include "pathfindsectorbuilder.h"
#include "Pathfind.h"
#include "node.h"
#include "mover.h"
#include "nodefunction.h"
#include "chunkio.h"
#include "decophys.h"
#include "VisPointNode.h"
#include "editorchunkids.h"
#include "presetmgr.h"
#include "combat.h"
#include "ccamera.h"
#include "visenum.h"
#include "staticaabtreecull.h"
#include "dynamicaabtreecull.h"
#include "SoundScene.h"
#include "backgroundmgr.h"
#include "texture.h"
#include "smartgameobj.h"
#include "objectnode.h"
#include "MouseMgr.h"
#include "lightnode.h"
#include "light.h"
#include "euler.h"
#include "w3d_file.h"
#include "part_emt.h"
#include "gameobjmanager.h"
#include "selectpresetdialog.h"
#include "actionparams.h"
#include "generatingvisdialog.h"
#include "generatingedgesampledvisdialog.h"
#include "generatinglightvisdialog.h"
#include "visgenprogress.h"
#include "vissectorsampler.h"
#include "vismgr.h"
#include "generatingmanualvisdialog.h"
#include "heightdb.h"
#include "staticanimphys.h"
#include "conversationmgr.h"
#include "building.h"
#include "buildingnode.h"
#include "vistable.h"
#include "rendobj.h"
#include "pscene.h"
#include "LevelEdit.h"
#include "visrendercontext.h"
#include "mapmgr.h"
#include "editableheightfield.h"
#include "heightfieldeditor.h"
#include "heightfieldmgr.h"
#include "pathmgr.h"
#include <algorithm>


//////////////////////////////////////////////////////////////////////
//
//	SceneEditorClass
//
//////////////////////////////////////////////////////////////////////
SceneEditorClass::SceneEditorClass (void)
	: m_SelectionMgr (NULL),
	  m_bLightsOn (true),
	  m_bAggregateChildrenVisible (true),
	  m_bVisPointsDisplayed (0),
	  m_TotalVisPoints (0),
	  m_ManualVisPointsVisible (true),
	  m_MovingObject (NULL),
	  m_DisplaySoundSpheres (false),
	  m_DisplayLightSpheres (false),
	  m_ShowStaticAnimPhys (true),
	  m_ShowEditorObjects (true),
	  m_BuildingPowerEnabled (true),
	  m_CreateProxies (true),
	  PhysicsSceneClass ()
{
	//
	// Ensure that we are initialized
	//
	Initialize ();

	//
	//	Set some default light settings
	//
	Enable_Sun_Light (true);	
	Set_Ambient_Light (Vector3 (0.3F, 0.3F, 0.3F));
	Set_Sun_Light_Orientation (0.0F, DEG_TO_RADF (50.0F));	
	Update_Lighting ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~SceneEditorClass
//
//////////////////////////////////////////////////////////////////////
SceneEditorClass::~SceneEditorClass (void)
{
	Empty_Local_Clipboard ();
	Cleanup_Resources ();
	Remove_All_Vis_Points ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////
void
SceneEditorClass::Initialize (void)
{
	//
	//	Load the lighting mode from the registry
	//
	int prelit_mode = theApp.GetProfileInt ("Config", "PrelitMode", WW3D::PRELIT_MODE_LIGHTMAP_MULTI_PASS);
	WW3D::Set_Prelit_Mode ((WW3D::PrelitModeEnum)prelit_mode);

	//
	//	Initialize some misc variables
	//
	ParticleEmitterClass::Set_Default_Remove_On_Complete (false);
	Cleanup_Resources ();
	m_ManualVisPointsVisible	= true;
	m_BuildingPowerEnabled		= true;

	PathfindClass::Get_Instance ()->Reset_Sectors ();

	//
	// Create the node and selection managers
	//
	m_SelectionMgr	= new SelectionMgrClass;	

	//
	// Reset our undo stack
	//
	m_UndoMgr.Purge_Buffers ();

	//
	// Set-up our collision groups
	//
	Disable_All_Collision_Detections (EDITOR_COLLISION_GROUP);
	Disable_All_Collision_Detections (DEF_COLLISION_GROUP);
	Enable_Collision_Detection (DEF_COLLISION_GROUP, DEF_COLLISION_GROUP);
	Enable_All_Collision_Detections (MOUSE_CLICK_COLLISION_GROUP);
	Enable_All_Collision_Detections (15);
	Disable_All_Collision_Detections (STATIC_OBJ_COLLISION_GROUP);
	Enable_Collision_Detection (STATIC_OBJ_COLLISION_GROUP, 15);
	Disable_Collision_Detection (MOUSE_CLICK_COLLISION_GROUP, 15);

	Enable_All_Collision_Detections (GAME_COLLISION_GROUP);
	Disable_Collision_Detection (GAME_COLLISION_GROUP, EDITOR_COLLISION_GROUP);

	//
	// Refresh the per-level include file list
	//
	STRING_LIST &level_includes =::Get_File_Mgr ()->Get_Include_File_List ();
	level_includes.Delete_All ();

	//
	//	Reset the pathfind data
	//
	PathfindClass::Get_Instance ()->Reset_Sectors ();
	PathfindClass::Get_Instance ()->Reset_Portals ();

	//
	// Reset the list of conversations
	//
	ConversationMgrClass::Reset_Conversations (ConversationMgrClass::CATEGORY_LEVEL, true);

	//
	//	Reset the map
	//
	MapMgrClass::Set_Map_Title (0);
	MapMgrClass::Set_Map_Texture ("");
	MapMgrClass::Set_Map_Center (Vector2 (0.0F, 0.0F));
	MapMgrClass::Set_Map_Scale (Vector2 (0.0F, 0.0F));
	MapMgrClass::Cloud_All_Cells ();

	//
	//	Initialize the heightfield system
	//
	HeightfieldMgrClass::Initialize ();
	HeightfieldEditorClass::Initialize ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Cleanup_Resources
//
//////////////////////////////////////////////////////////////////////
void
SceneEditorClass::Cleanup_Resources (void)
{
	//
	//	Free the background sound
	//
	if (WWAudioClass::Get_Instance () != NULL) {
		WWAudioClass::Get_Instance ()->Set_Background_Music (NULL);
	}

	//
	//	Cleanup the different managers
	//	
	m_UndoMgr.Purge_Buffers ();
	SAFE_DELETE (m_SelectionMgr);

	NodeMgrClass::Free_Nodes ();
	NetworkObjectMgrClass::Delete_Pending ();

	// Remove all entries from the group list
	Reset_Global_Groups_List ();

	m_MovingObject = NULL;

	//
	//	Shutdown the heightfield system
	//
	HeightfieldEditorClass::Shutdown ();
	HeightfieldMgrClass::Shutdown ();	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Create_Node
//
//////////////////////////////////////////////////////////////////////
NodeClass *
SceneEditorClass::Create_Node
(
	PresetClass *	preset,
	Matrix3D *		transform,
	DWORD				node_id,
	bool				add_to_scene
)
{
	::Get_Main_View ()->Allow_Repaint (false);
	CWaitCursor wait_cursor;

	//
	// Create the new node
	//
	NodeClass *node = NodeMgrClass::Create_Node (preset, node_id);
	ASSERT (node != NULL);
	if (node != NULL) {

		//
		//	Find a good starting position for this node in the scene
		//
		if (transform != NULL) {
			node->Set_Transform (*transform);
		} else {
			MoverClass::Position_Node (node);
		}

		//
		//	Add the new node to the scene
		//
		if (add_to_scene) {
			node->Add_To_Scene ();
		}
	
		//
		//	Update the file manager's database
		//	
		Update_File_Mgr (node);

		//
		//	Repartition the static culling systems if we are dropping
		// in a terrain.  
		//
		if (node->Get_Type () == NODE_TYPE_TERRAIN) {
			Re_Partition_Static_Lights ();
			Re_Partition_Audio_System ();
		} else if (node->Get_Type () == NODE_TYPE_WAYPATH) {
			::Get_Mouse_Mgr ()->Set_Mouse_Mode (MouseMgrClass::MODE_WAYPATH_EDIT);
			MMWaypathEditClass *mode = (MMWaypathEditClass *)::Get_Mouse_Mgr ()->Get_Mode_Mgr ();
			if (mode != NULL) {
				mode->Set_Waypath ((WaypathNodeClass *)node);
			}
		}

		//
		//	Create any linked nodes and put them into the scene
		//
		if (preset != NULL) {
			preset->Create_Linked_Nodes (node);
		}

		//
		//	Release our hold on the node
		//
		node->Release_Ref ();
	}

	Set_Modified (true);
	::Get_Main_View ()->Allow_Repaint (true);

	return node;
}


//////////////////////////////////////////////////////////////////////
//
//	Clone_Node
//
//////////////////////////////////////////////////////////////////////
NodeClass *
SceneEditorClass::Clone_Node (NodeClass *node)
{
	NodeClass *new_node = node->Clone ();
	if (new_node != NULL) {

		//
		//	Make sure the new node has a name and ID
		//
		NodeMgrClass::Setup_Node_Identity (*new_node);
		NodeMgrClass::Add_Node (new_node);

		//
		//	Add the new node to the scene
		//
		new_node->Add_To_Scene ();
	
		//
		//	Update the file manager's database
		//	
		Update_File_Mgr (new_node);
	}

	return new_node;
}


//////////////////////////////////////////////////////////////////////
//
//	Reload_Lightmap_Models
//
//////////////////////////////////////////////////////////////////////
void
SceneEditorClass::Reload_Lightmap_Models (void)
{
	//
	//	Save the setting in the registry
	//
	theApp.WriteProfileInt ("Config", "PrelitMode", WW3D::Get_Prelit_Mode ());

	//
	//	Force free the assets...
	//
	WW3DAssetManager::Get_Instance ()->Free_Assets ();

	//
	//	Reload the necessary "tool" assets
	//
	CLevelEditDoc *doc = ::Get_Current_Document ();
	if (doc != NULL) {
		doc->Preload_Human_Data ();
		doc->Preload_Tool_Assets ();
	}

	//
	//	Reload all the nodes
	//
	NodeMgrClass::Reload_Nodes ();

	//
	//	Make sure we don't toss vis
	//
	Validate_Vis ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Add_Node
//
//////////////////////////////////////////////////////////////////////
void
SceneEditorClass::Add_Node (NodeClass *node)
{
	NodeMgrClass::Add_Node (node);
	node->Add_To_Scene ();
	Update_File_Mgr (node);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Delete_Node
//
//////////////////////////////////////////////////////////////////////
bool
SceneEditorClass::Delete_Node (NodeClass *node, bool allow_undo)
{
	// Add this operation to our undo manager
	if (allow_undo) {		
		Begin_Operation (OPERATION_DELETE, node);
	}
	
	//
	// Remove all files that this node was dependent on from the file manager's list
	//
	Update_File_Mgr (node, false);

	//
	// Remove the node from the level
	//
	node->Remove_From_Scene ();
	node->Show_Selection_Box (false);
	node->On_Delete ();
	NodeMgrClass::Remove_Node (node);	

	// Notify the undo manager that this operation is over
	if (allow_undo) {		
		End_Operation ();
	}

	Set_Modified (true);
	return true;
}


//////////////////////////////////////////////////////////////////////
//
//	Delete_Nodes
//
//////////////////////////////////////////////////////////////////////
bool
SceneEditorClass::Delete_Nodes (PresetClass *preset)
{
	bool retval = true;

	//
	//	Build a list of nodes to that are dependent on this preset
	//
	DynamicVectorClass<NodeClass *> node_removal_list;
	for (	NodeClass *node = NodeMgrClass::Get_First ();
			node != NULL;
			node = NodeMgrClass::Get_Next (node))
	{
		if (node->Get_Preset () == preset) {
			node_removal_list.Add (node);
		}
	}

	//
	//	Remove these nodes from the level
	//
	for (int index = 0; index < node_removal_list.Count (); index ++) {
		retval &= Delete_Node (node_removal_list[index], false);
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Update_File_Mgr
//
//////////////////////////////////////////////////////////////////////
void
SceneEditorClass::Update_File_Mgr
(
	NodeClass *	node,
	bool			add_node
)
{
	::Get_File_Mgr ()->Update (node, add_node);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Clone_Objects
//
//////////////////////////////////////////////////////////////////////
bool
SceneEditorClass::Clone_Objects (void)
{
	m_SelectionMgr->Clone_Group ();

	//
	// Loop through all the objects in the current selection
	// and update the file manager's information based on them
	//
	int count = m_SelectionMgr->Get_Count ();
	for (int index = 0; index < count; index ++) {
		NodeClass *node = m_SelectionMgr->Get_At (index);
		Update_File_Mgr (node);
	}

	Set_Modified ();
	return true;
}


//////////////////////////////////////////////////////////////////////
//
//	Add_Groups_To_List
//
//////////////////////////////////////////////////////////////////////
void
SceneEditorClass::Add_Groups_To_List
(
	NodeClass &	/* node */,
	GROUP_LIST &/* group_list */
)
{	
	/*group_list += node.Get_Groups ();

	// Loop through all the children of this node and
	// add their groups to the list as well
	for (NodeInstanceClass *child = node.Get_Child ();
		  child != NULL;
		  child = child->Get_Sibling ()) {
		Add_Groups_To_List (*child, group_list);
	}*/

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Build_Group_List
//
//////////////////////////////////////////////////////////////////////
void
SceneEditorClass::Build_Group_List
(
	GROUP_LIST &	group_list,
	NodeClass *		node
)
{	
	if (node != NULL) {
		Add_Groups_To_List (*node, group_list);
	} else {

		//
		// Loop through all the objects in the current selection
		// and add thier groups to this list
		//
		for (int index = 0; index < m_SelectionMgr->Get_Count (); index ++) {
			NodeClass *next_node = m_SelectionMgr->Get_At (index);
			if (next_node != NULL) {
				Add_Groups_To_List (*next_node, group_list);
			}
		}
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Add_Nodes_To_List
//
//////////////////////////////////////////////////////////////////////
void
SceneEditorClass::Add_Nodes_To_List
(
	NodeClass &/* node */,
	NODE_LIST &/* node_list */
)
{
	//
	// Loop through all the children of this node and
	// add them and their children to the list as well
	//
	/*for (NodeInstanceClass *child = node.Get_Child ();
		  child != NULL;
		  child = child->Get_Sibling ()) {
		
		if (child->Is_Owner_Only () == false) {
			node_list.Add_Unique (child);
		}
		
		Add_Nodes_To_List (*child, node_list);
	}*/
	
	return ;	
}


//////////////////////////////////////////////////////////////////////
//
//	Build_Node_List
//
//////////////////////////////////////////////////////////////////////
Vector3
SceneEditorClass::Build_Node_List (NODE_LIST &node_list)
{	
	//
	//	Add all the nodes from the selection set to the list
	//
	for (int index = 0; index < m_SelectionMgr->Get_Count (); index ++) {
		NodeClass *node = m_SelectionMgr->Get_At (index);
		if (node != NULL) {
			node_list.Add_Unique (node);
		}
	}

	m_SelectionMgr->Set_Dirty ();
	return m_SelectionMgr->Get_Center ();
}


//////////////////////////////////////////////////////////////////////
//
//	Build_Node_List
//
//////////////////////////////////////////////////////////////////////
void
SceneEditorClass::Build_Node_List
(
	NODE_LIST &	node_list,
	NodeClass *	parent_node
)
{	
	//
	// Build a complete list of all the groups in the selection set
	//
	GROUP_LIST group_list;
	Build_Group_List (group_list, parent_node);

	// Loop through all the objects of all the groups in the selection
	// set and add them to the list
	for (int group = 0; group < group_list.Count (); group ++) {
		GroupMgrClass *group_mgr = group_list[group];
		group_mgr->Set_Dirty ();

		for (int index = 0; index < group_mgr->Get_Count (); index ++) {
			NodeClass *node = group_mgr->Get_At (index);
			if (node != NULL) {

				node_list.Add_Unique (node);
				Add_Nodes_To_List (*node, node_list);
			}
		}
	}

	//
	// Don't forget to add the nodes from the selection group as well!
	//
	if (parent_node == NULL) {			
		
		for (int index = 0; index < m_SelectionMgr->Get_Count (); index ++) {
			NodeClass *node = m_SelectionMgr->Get_At (index);
			if (node != NULL) {

				node_list.Add_Unique (node);
				Add_Nodes_To_List (*node, node_list);
			}
		}

	} else {

		/*if (parent_node->Is_Owner_Only () == false) {
			node_list.Add_Unique (pparent_node);
		}
		Add_Nodes_To_List (*pparent_node, node_list);*/
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Delete_Nodes
//
///////////////////////////////////////////////////////////////
void
SceneEditorClass::Delete_Nodes (void)
{
	NODE_LIST affected_list;

	//
	// Build a list of all the nodes that are affected by this operation
	//
	int index;
	for (index = 0; index < m_SelectionMgr->Get_Count (); index ++) {
		NodeClass *node = m_SelectionMgr->Get_At (index);
		if (node != NULL) {
			
			// Add this node to our list of affected nodes
			affected_list.Add_Unique (node);
		}
	}

	// Add this operation to our undo manager
	Begin_Operation (OPERATION_DELETE, &affected_list);

	//
	// Perfrom the actual delete operation
	//
	for (index = 0; index < affected_list.Count (); index ++) {
		Delete_Node (affected_list[index], false);
	}
	
	// Notify the undo manager that this operation is over
	End_Operation ();

	// Reset the contents of the selection.
	// Note:  We create a temporary group here to keep a refcount
	// locked on the selected nodes while we delete the selection group.
	// Remember, freeing the selection group decrements the node's refcounts.
	//GroupMgrClass temp_group = *m_SelectionMgr;
	m_SelectionMgr->Free_List ();
	return ;
}


///////////////////////////////////////////////////////////////
//
//  Set_Selection
//
///////////////////////////////////////////////////////////////
void
SceneEditorClass::Set_Selection (NodeClass *node)
{
	// Is this item already in the list?
	if (m_SelectionMgr->Is_Item_In_Group (node) == false) {		

		//
		// Make this node the only selected node
		//
		m_SelectionMgr->Reset ();
		m_SelectionMgr->Add_Node (node);
		Update_Toolbars ();
	}
	
	return ;
}


///////////////////////////////////////////////////////////////
//
//  Toggle_Selection
//
///////////////////////////////////////////////////////////////
void
SceneEditorClass::Toggle_Selection (NodeClass *node)
{
	//
	// If this node isn't already in the selection set, then
	// add it.  However, if its in the selection set, remove it.
	//
	if (m_SelectionMgr->Is_Item_In_Group (node)) {
		m_SelectionMgr->Remove_Node (node);
	} else {
		m_SelectionMgr->Add_Node (node);
	}

	Update_Toolbars ();
	return ;
}


///////////////////////////////////////////////////////////////
//
//  Update_Toolbars
//
///////////////////////////////////////////////////////////////
void
SceneEditorClass::Update_Toolbars (void)
{
	bool show_ani_toolbar	= false;
	int frame					= 0;
	int max_frames				= 0;

	if (m_SelectionMgr->Get_Count () > 0) {
		
		//
		// Loop through all the objects in the current selection
		// and output their names to the debug window...
		//
		CString selection_set = "Selection set: ";
		int sel_count = std::min (m_SelectionMgr->Get_Count (), 4);
		for (int index = 0; index < sel_count; index ++) {
			NodeClass *node	= m_SelectionMgr->Get_At (index);
			selection_set		+= node->Get_Name ();
			
			//
			// Print vis-id and occluder status if we're only printing one object
			//
			PhysClass *phys_obj = node->Peek_Physics_Obj ();
			if (m_SelectionMgr->Get_Count () == 1 && phys_obj != NULL) {
				StaticPhysClass *static_phys_obj = phys_obj->As_StaticPhysClass();
				if (static_phys_obj != NULL) {
					selection_set += (" (");
					CString visid_string;
					visid_string.Format(" VisObjectId = %d, ",static_phys_obj->Get_Vis_Object_ID());
					selection_set += visid_string;
					visid_string.Format(" VisSectorId = %d, ",static_phys_obj->Get_Vis_Sector_ID());
					selection_set += visid_string;

					AABTreeLinkClass *link = (AABTreeLinkClass *)static_phys_obj->Get_Cull_Link ();
					if (link != NULL) {
						visid_string.Format(" CullLink = %d, ",link->Node->Index);
						selection_set += visid_string;
					}

					if (!static_phys_obj->Is_Occluder()) {
						selection_set += CString(" Non-Occluder ");
					}
					selection_set += (")");
				} else if (node->Get_Type () == NODE_TYPE_LIGHT) {
					selection_set += (" (");
					CString visid_string;
					visid_string.Format(" VisSectorId = %d, ",((LightNodeClass *)node)->Get_Vis_Sector_ID());
					selection_set += visid_string;
					selection_set += (")");						
				} else {
					selection_set += (" (");
					CString visid_string;
					visid_string.Format(" VisObjectId = %d, ",phys_obj->Get_Vis_Object_ID());
					selection_set += visid_string;
					selection_set += (")");					
				}
			}

			selection_set += ", ";
		}

		if (sel_count < m_SelectionMgr->Get_Count ()) {
			selection_set += "and many more....";
		}

		//
		// Now output this string to the debug window
		//
		selection_set += "\r\n";
		::Output_Message (selection_set);
	}

	// Show/hide the animation toolbar depending on the current state
	CMainFrame *pmainwnd = (CMainFrame *)::AfxGetMainWnd ();
	if (pmainwnd != NULL) {
		pmainwnd->Show_Ani_Toolbar (show_ani_toolbar);
		pmainwnd->Update_Ani_Frame (frame, max_frames);
	}

	return ;		
}

///////////////////////////////////////////////////////////////
//
//  Find_Node_At_Point
//
///////////////////////////////////////////////////////////////
NodeClass *
SceneEditorClass::Find_Node_At_Point (CPoint point, Vector3 *intersect_pt)
{
	//
	//	Calculate the ray from the current camera pos through
	// the mouse cursor.
	//
	Vector3 ray_start;
	Vector3 ray_end;
	MoverClass::Get_Mouse_Ray (point, 250.0F, ray_start, ray_end);

	//
	// Cast the ray into the world
	//
	CastResultStruct res;
	if (intersect_pt != NULL) {
		res.ComputeContactPoint = true;
	}

	PhysClass *physobj = MoverClass::Cast_Ray (res, ray_start, ray_end, MOUSE_CLICK_COLLISION_GROUP);	
	if ((physobj != NULL) && (physobj->Peek_Model() != NULL)) {
		WWDEBUG_SAY(("Picked model: %s\r\n",physobj->Peek_Model()->Get_Name()));
	}

	//
	// Get the node's pointer that was hit from the raycast
	//
	NodeClass *node = NULL;
	if (physobj != NULL) {

		HITTESTINFO *hit_info = (HITTESTINFO *)physobj->Peek_Model ()->Get_User_Data ();
		if (hit_info != NULL && hit_info->Type == HITTESTINFO::Node) {
			node = hit_info->node;

			//
			//	Return the intersection point (if requested)
			//
			if (intersect_pt != NULL) {
				(*intersect_pt) = res.ContactPoint;
			}
		}
	}
	
	// Return a pointer to the node
	return node;
}


///////////////////////////////////////////////////////////////
//
//  Execute_Function_At_Point
//
///////////////////////////////////////////////////////////////
bool
SceneEditorClass::Execute_Function_At_Point (CPoint point)
{
	bool processed = false;

	//
	//	Calculate the ray from the current camera pos through
	// the mouse cursor.
	//
	Vector3 ray_start;
	Vector3 ray_end;
	MoverClass::Get_Mouse_Ray (point, 250.0F, ray_start, ray_end);
	

	//
	// Cast the ray into the world
	//
	CastResultStruct res;
	PhysClass *physobj = MoverClass::Cast_Ray (res, ray_start, ray_end, MOUSE_CLICK_COLLISION_GROUP);

	//
	// Determine if the user clicked on a 'function'
	//
	if (physobj != NULL) {
		
		HITTESTINFO *hit_info = (HITTESTINFO *)physobj->Peek_Model ()->Get_User_Data ();
		if ((hit_info != NULL) && hit_info->Type == HITTESTINFO::Function) {

			hit_info->function->On_Click ();
			processed = true;
		}
	}
	
	// Return the true/false result code
	return processed;
}


///////////////////////////////////////////////////////////////
//
//  Move_Selected_Nodes
//
///////////////////////////////////////////////////////////////
void
SceneEditorClass::Move_Selected_Nodes (void)
{
	float speed_modifier		= ::Get_Camera_Mgr ()->Get_Speed_Modifier ();
	float modifier				= (::GetAsyncKeyState (VK_CONTROL) < 0) ? speed_modifier : 1.0F;	
	Matrix3D coord_system	= ::Get_Camera_Mgr ()->Get_Camera ()->Get_Transform ();		
	bool move_object			= false;
	Vector3 delta (0, 0, 0);

	if (::GetAsyncKeyState (VK_LEFT) < 0) {

		//
		//	Move the objects along the x-axis of the current view
		//
		Vector3 movement (-0.05F * modifier, 0, 0);
		delta += coord_system.Rotate_Vector (movement);
		move_object = true;

	} else if (::GetAsyncKeyState (VK_RIGHT) < 0) {

		//
		//	Move the objects along the x-axis of the current view
		//
		Vector3 movement (0.05F * modifier, 0, 0);
		delta += coord_system.Rotate_Vector (movement);
		move_object = true;
	}

	if (::GetAsyncKeyState (VK_UP) < 0) {

		//
		//	Should we move the object 'up'?
		//		
		if (::GetAsyncKeyState (VK_SHIFT) < 0) {
			delta += Vector3 (0, 0, 0.05F * modifier);
		} else {

			//
			//	Move the object along the 'ground-plane' at an axis orthogonal to the view's x-axis.
			//
			Vector3 y_vector = Vector3::Cross_Product (coord_system.Get_X_Vector (), Vector3 (0, 0, 1));
			y_vector.Normalize ();
			delta += y_vector * (-0.05F * modifier);
		}

		move_object = true;

	} else if (::GetAsyncKeyState (VK_DOWN) < 0) {

		//
		//	Should we move the object 'up'?
		//		
		if (::GetAsyncKeyState (VK_SHIFT) < 0) {
			delta += Vector3 (0, 0, -0.05F * modifier);
		} else {

			//
			//	Move the object along the 'ground-plane' at an axis orthogonal to the view's x-axis.
			//
			Vector3 y_vector = Vector3::Cross_Product (coord_system.Get_X_Vector (), Vector3 (0, 0, 1));
			y_vector.Normalize ();
			delta += y_vector * (0.05F * modifier);			
		}

		move_object = true;
	}

	//
	//	Move the selected nodes if necessary
	//
	if (move_object) {
		MoverClass::Move_Nodes (delta);
	}
	
	return ;
}


///////////////////////////////////////////////////////////////
//
//  On_Frame
//
///////////////////////////////////////////////////////////////
void
SceneEditorClass::On_Frame (void)
{
	//
	//	Let the heightfield editor think
	//
	HeightfieldEditorClass::On_Frame_Update ();

	//
	//	Get the currently 'focussed' process
	//
	HWND current_focus_wnd = ::GetFocus ();
	DWORD process_id = 0;
	if (current_focus_wnd != NULL) {
		::GetWindowThreadProcessId (current_focus_wnd, &process_id);
	}

	//
	//	If we are the current process, then move the nodes
	//
	if (process_id == ::GetCurrentProcessId ()) {
		Move_Selected_Nodes ();
	}

	if (m_MovingObject != NULL) {
		PathMgrClass::Resolve_Paths (Vector3 (0, 0, 0));
		PhysicalGameObj *game_obj = ((ObjectNodeClass *)m_MovingObject)->Peek_Game_Obj ();
		((SmartGameObj *)game_obj)->Generate_Control ();
		game_obj->Think ();

		PhysClass *phys_obj = game_obj->Peek_Physical_Object ();
		if (phys_obj != NULL) {
			phys_obj->Timestep (TimeManager::Get_Frame_Seconds ());
		}			

		game_obj->Post_Think ();

#if 0
		ActionClass	*action = ((SmartGameObj *)game_obj)->Get_Action ();
		if (action != NULL && action->Get_Movement () == NULL) {
			//m_MovingObject = NULL;
		}
#endif
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Record_Vis_Info
//
///////////////////////////////////////////////////////////////
void
SceneEditorClass::Record_Vis_Info (const Matrix3D &view_transform, const Vector3 &sample_location)
{
	// Put up an hour glass
	CWaitCursor wait_cursor;

	PresetClass *preset = PresetMgrClass::Get_First (CLASSID_VIS_POINT_DEF, PresetMgrClass::ID_CLASS);
	if (preset == NULL) {
		CString message = "Missing 'Manual Vis Point' preset.  Please create a default preset under the 'Manual Vis Point' folder in the preset library.";
		::MessageBox (::AfxGetMainWnd ()->m_hWnd, message, "Missing Preset", MB_OK | MB_ICONEXCLAMATION);
	} else {

		//
		// When doing a manual vis, use the FOV and clip settings of the in-game camera
		//
		CameraClass *scene_cam = CombatManager::Get_Camera ();
		WWASSERT (scene_cam != NULL);
		
		float scene_znear = 0;
		float scene_zfar = 0;
		scene_cam->Get_Clip_Planes (scene_znear, scene_zfar);
		scene_cam->Set_Clip_Planes (scene_znear, VIS_FAR_CLIP);

		//
		// Generate the 'vis' information
		//
		VisSampleClass vis_sample = Update_Vis (view_transform * Vector3 (0, 0, -scene_znear), view_transform, (VisDirBitsType)(VIS_FORWARD_BIT | VIS_DONT_RECENTER), scene_cam);
		if (vis_sample.Get_Bits_Changed () > 0 && vis_sample.Sample_Rejected () == false) {
			
			//
			//	Convert the matrix to an obj-space transform
			//
			Matrix3D cam_to_world (Vector3 (0, 0, -1), Vector3 (-1, 0, 0), Vector3 (0, 1, 0), Vector3 (0, 0, 0));
			Matrix3D obj_tm = view_transform * cam_to_world;

			//
			//	Create the UI
			//
			Create_Vis_Point (view_transform);	
			VisPointNodeClass *vis_point = new VisPointNodeClass (preset);
			vis_point->Set_ID (::Get_Node_Mgr ().Get_Node_ID (vis_point->Get_Type ()));
			vis_point->Initialize ();
			vis_point->Add_To_Scene ();
			vis_point->Set_Transform (obj_tm);
			vis_point->Set_Vis_Tile_Location (sample_location);
			vis_point->Save_Camera_Settings (*scene_cam);
			vis_point->Hide (!m_ManualVisPointsVisible);
			::Get_Node_Mgr ().Add_Node (vis_point);
			vis_point->Release_Ref ();
			Set_Modified (true);
		}

		// 
		// Put the normal settings back into the scene camera
		//
		scene_cam->Set_Clip_Planes (scene_znear, scene_zfar);				
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Begin_Operation
//
///////////////////////////////////////////////////////////////
void
SceneEditorClass::Begin_Operation
(
	OPERATION_TYPE		type,
	NodeClass *			node
)
{
	if (node != NULL) {
		
		// Build a list containing the node
		NODE_LIST node_list;
		node_list.Add_Unique (node);

		// Pass this list onto the undo manager
		Begin_Operation (type, &node_list);
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Begin_Operation
//
///////////////////////////////////////////////////////////////
void
SceneEditorClass::Begin_Operation
(
	OPERATION_TYPE	type,
	NODE_LIST *		affected_list
)
{
	NODE_LIST node_list;

	//
	// If necessary, build a complete list of all the nodes that will be
	// affected by this operation
	//
	if (affected_list == NULL) {
		Build_Node_List (node_list, NULL);
		affected_list = &node_list;
	}

	m_UndoMgr.Begin_Operation (type, *affected_list);
	return ;
}


///////////////////////////////////////////////////////////////
//
//  End_Operation
//
///////////////////////////////////////////////////////////////
void
SceneEditorClass::End_Operation (void)
{
	m_UndoMgr.End_Operation ();
	return ;
}


///////////////////////////////////////////////////////////////
//
//  Undo
//
///////////////////////////////////////////////////////////////
bool
SceneEditorClass::Undo (void)
{
	m_UndoMgr.Perform_Undo ();
	return true;
}


///////////////////////////////////////////////////////////////
//
//  Cut_Objects
//
///////////////////////////////////////////////////////////////
bool
SceneEditorClass::Cut_Objects (void)
{
	bool retval = false;

	// Turn painting off
	CLevelEditView::Allow_Repaint (false);

	// Copy the objects to the clipboard, then delete them
	if (Copy_Objects ()) {
		Delete_Nodes ();
	}

	// Turn painting back on
	CLevelEditView::Allow_Repaint (true);

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////
//
//  Copy_Objects
//
///////////////////////////////////////////////////////////////
bool
SceneEditorClass::Copy_Objects (void)
{	
	Empty_Local_Clipboard ();
	
	// Loop through all the nodes in the current selection
	// and 'copy' them.
	int count = m_SelectionMgr->Get_Count ();
	for (int index = 0; index < count; index ++) {
		NodeClass *node = m_SelectionMgr->Get_At (index);

		//
		//	Copy the node and add it to our 'clipboard'
		//
		NodeClass *node_copy = node->Clone ();
		ASSERT (node_copy != NULL);
		if (node_copy != NULL) {
			node_copy->Remove_From_Scene ();
			m_LocalClipboard.Add (node_copy);
		}
	}		
	
	return true;
}


///////////////////////////////////////////////////////////////
//
//  Paste_Objects
//
///////////////////////////////////////////////////////////////
bool
SceneEditorClass::Paste_Objects (void)
{
	bool retval = (m_LocalClipboard.Count () > 0);

	// Turn painting off
	CLevelEditView::Allow_Repaint (false);

	// Reset the selection group
	m_SelectionMgr->Reset ();
	for (int index = 0; index < m_LocalClipboard.Count (); index ++) {
		
		//
		//	Copy the node on the clipboard and add it to the scene
		//
		NodeClass *node		= m_LocalClipboard[index];
		NodeClass *node_copy	= Clone_Node (node);
		if (node_copy != NULL) {
			m_SelectionMgr->Add_Node (node_copy);
			MEMBER_RELEASE (node_copy);
		}
	}
	
	//
	//	Cast the nodes along the line-of-sight ray for 500 meters
	//
	Vector3 ray_start;
	Vector3 ray_end;
	MoverClass::Get_LOS_Ray (500.0F, ray_start, ray_end);
	MoverClass::Position_Nodes_Along_Ray (ray_start, ray_end);

	// Turn painting back on
	CLevelEditView::Allow_Repaint (true);
	Update_Toolbars ();
	
	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////
//
//  Can_Paste
//
///////////////////////////////////////////////////////////////
bool
SceneEditorClass::Can_Paste (void)
{
	// Return the true/false result code
	return bool(m_LocalClipboard.Count () > 0);
}


///////////////////////////////////////////////////////////////
//
//  Empty_Local_Clipboard
//
///////////////////////////////////////////////////////////////
void
SceneEditorClass::Empty_Local_Clipboard (void)
{
	// Loop through all the entries in the clipboard and delete them
	for (int index = 0; index < m_LocalClipboard.Count (); index ++) {
		MEMBER_RELEASE (m_LocalClipboard[index]);
	}

	// Reset the list
	m_LocalClipboard.Delete_All ();
	return ;
}


///////////////////////////////////////////////////////////////
//
//  Add_Global_Group
//
///////////////////////////////////////////////////////////////
GroupMgrClass *
SceneEditorClass::Add_Global_Group
(
	const CString &	name,
	NODE_LIST *			initial_list
)
{
	// Create a new group with the speicfied name
	UserGroupMgrClass *group = new UserGroupMgrClass;
	group->Set_Name (name);

	// If we have a list of nodes to add to this group,
	// then do so now.
	if (initial_list != NULL) {
		for (int index = 0; index < initial_list->Count (); index ++) {
			group->Add_Node ((*initial_list)[index]);
		}
	}

	// Add this new group manager to our global list
	m_GroupsList.Add_Unique (group);

	// Return a pointer to the new group
	return group;
}


///////////////////////////////////////////////////////////////
//
//  Remove_Global_Group
//
///////////////////////////////////////////////////////////////
void
SceneEditorClass::Remove_Global_Group (GroupMgrClass *group)
{
	// If this group is in our global list, then remove
	// it from the list, remove it from any UI, and free its memory.
	if (m_GroupsList.Is_Item_In_List (group)) {
		m_GroupsList.Remove (group);
		Remove_Group_From_Toolbar (group);
		SAFE_DELETE (group);
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Reset_Global_Groups_List
//
///////////////////////////////////////////////////////////////
void
SceneEditorClass::Reset_Global_Groups_List (void)
{
	// Loop through all the groups in our list and
	// free their associated memory.
	for (int index = 0; index < m_GroupsList.Count (); index ++) {		
		GroupMgrClass *group = m_GroupsList[index];
		if (group != NULL) {
			Remove_Group_From_Toolbar (group);
			SAFE_DELETE (group);
		}
	}

	// Remove all the entries from the list
	m_GroupsList.Delete_All ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Add_Group_To_Toolbar
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Add_Group_To_Toolbar ([[maybe_unused]] GroupMgrClass *group)
{
	ASSERT (group != NULL);
			
	// Get the form representing the groups list, and add this group to it
	/*GroupsPageClass *group_page = ::Get_Groups_Form ();
	if (group_page != NULL) {
		group_page->Add_Group (group);
	}*/

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Remove_Group_From_Toolbar
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Remove_Group_From_Toolbar ([[maybe_unused]] GroupMgrClass *group)
{
	ASSERT (group != NULL);
			
	// Get the form representing the groups list, and remove this group from it
	/*GroupsPageClass *group_page = ::Get_Groups_Form ();
	if (group_page != NULL) {
		group_page->Remove_Group (group);
	}*/

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Turn_Lights_On
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Turn_Lights_On (bool onoff)
{
	m_bLightsOn = onoff;

	//
	// Loop through all the lights in the world
	//
	/*LightInstanceClass *plight = NULL;
	for (plight = (LightInstanceClass *)::Get_Node_Mgr ().Get_First (NODE_TYPE_LIGHT);
		  plight != NULL;
		  plight = (LightInstanceClass *)::Get_Node_Mgr ().Get_Next (NODE_TYPE_LIGHT)) {		
		plight->Turn_Light_On (onoff);
	}*/
		
	::Refresh_Main_View ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Lock_Nodes
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Lock_Nodes (bool lock)
{
	//
	// Loop through all the nodes in the current selection
	//
	for (int index = 0; index < m_SelectionMgr->Get_Count (); index ++) {
		NodeClass *node = m_SelectionMgr->Get_At (index);
		if (node != NULL) {
			
			//
			// Lock the node, this keep the node from moving during
			// drag operations.
			//
			node->Lock (lock);
		}
	}
	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Background_Music
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Set_Background_Music (LPCTSTR filename)
{
	//
	//	Stop the old sound
	//
	WWAudioClass::Get_Instance ()->Set_Background_Music (NULL);

	//
	// Remove the sound file from the file manager
	//
	if (m_BackgroundSoundFilename.GetLength () > 0) {
		
		CString old_path = ::Get_File_Mgr ()->Make_Full_Path (m_BackgroundSoundFilename);
		::Get_File_Mgr ()->Remove_File (::Get_File_Mgr ()->Find_File (old_path));
		m_BackgroundSoundFilename.Empty ();
	}

	//
	// Set the new background music
	//
	if ((filename != NULL) && (filename[0] != 0)) {
		CString path = ::Get_File_Mgr ()->Make_Full_Path (filename);
		if (::Get_File_Mgr ()->Does_File_Exist (path, true)) {
			
			//
			//	Set the background music
			//
			::Set_Current_Directory (::Strip_Filename_From_Path (path));
			WWAudioClass::Get_Instance ()->Set_Background_Music (::Get_Filename_From_Path (path));

			//
			// Add this sound to the file manager
			//
			::Get_File_Mgr ()->Add_File (path);
			m_BackgroundSoundFilename = path;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	View_Aggregate_Children
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::View_Aggregate_Children (bool /* view */)
{
	// Did the state change?
	/*if (view != m_bAggregateChildrenVisible) {
		m_bAggregateChildrenVisible = view;

		// Make sure to reset the selection for this to work
		m_SelectionMgr->Reset ();

		// Update all the aggregates in the level
		::Get_Node_Mgr ().Update_Aggregate_Instances ();
	}*/

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Display_Vis_Points
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Display_Vis_Points (bool onoff)
{
	// Did the state change?
	if (onoff != m_bVisPointsDisplayed) {
		m_bVisPointsDisplayed = onoff;

		//
		//	Show/hide all the vis points
		//
		for (int index = 0; index < m_VisPoints.Count (); index ++) {
			DecorationPhysClass *vis_point = m_VisPoints[index];
			if (vis_point != NULL) {
				if (onoff) {
					Add_Dynamic_Object (vis_point);
				} else {
					Remove_Object (vis_point);
				}
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Vis_Point
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Create_Vis_Point (const Matrix3D &transform)
{	
	//
	//	Create a new vis-point marker at the location
	//
	DecorationPhysClass *new_point = new DecorationPhysClass;
	new_point->Set_Model_By_Name ("WAYSTART");
	new_point->Set_Transform (transform);
	new_point->Set_Collision_Group (0);

	//
	//	Add this new point to our list
	//
	m_VisPoints.Add (new_point);
	if (m_bVisPointsDisplayed) {
		Add_Dynamic_Object (new_point);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Remove_All_Vis_Points
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Remove_All_Vis_Points (void)
{
	::Get_Main_View ()->Allow_Repaint (false);

	CWaitCursor wait_cursor;

	//
	//	Build a list of all the vis points in the world
	//
	for (int index = 0; index < m_VisPoints.Count (); index ++) {
		DecorationPhysClass *vis_point = m_VisPoints[index];
		if (vis_point != NULL) {
			Remove_Object (vis_point);
			vis_point->Release_Ref ();
		}
	}

	m_VisPoints.Delete_All ();

	::Get_Main_View ()->Allow_Repaint (true);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Discard_Vis
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Discard_Vis (void)
{
	m_VisLog.Reset_Log ();
	Remove_All_Vis_Points ();
	PhysicsSceneClass::Internal_Vis_Reset ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Reset_Vis
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Reset_Vis (bool doitnow)
{
	m_VisLog.Reset_Log ();
	Remove_All_Vis_Points ();
	PhysicsSceneClass::Reset_Vis (doitnow);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Internal_Vis_Reset
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Internal_Vis_Reset (void)
{
	if (VisResetNeeded) {
		PhysicsSceneClass::Internal_Vis_Reset ();

		//
		//	Ask all the nodes 
		//
		NodeClass *node = NULL;
		for (	node = NodeMgrClass::Get_First ();
				node != NULL;
				node = NodeMgrClass::Get_Next (node))
		{
			node->Update_Cached_Vis_IDs ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Vis_Camera_FOV
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Get_Vis_Camera_FOV (double &hfov, double &vfov)
{
	CameraClass *vis_camera = Get_Vis_Camera ();
	WWASSERT (vis_camera != NULL);
	if (vis_camera != NULL) {
		hfov = vis_camera->Get_Horizontal_FOV ();
		vfov = vis_camera->Get_Vertical_FOV ();		
		MEMBER_RELEASE (vis_camera);
	} else {		
		hfov = 0;
		vfov = 0;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Vis_Camera_Clip_Planes
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Get_Vis_Camera_Clip_Planes (float &znear, float &zfar)
{
	CameraClass *vis_camera = Get_Vis_Camera ();
	WWASSERT (vis_camera != NULL);
	if (vis_camera != NULL) {
		vis_camera->Get_Clip_Planes (znear, zfar);
		MEMBER_RELEASE (vis_camera);
	} else {
		znear = 0;
		zfar = 0;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Render_Camera
//
////////////////////////////////////////////////////////////////
CameraClass *
SceneEditorClass::Get_Render_Camera (void)
{
	CameraClass *camera = NULL;

	if (::Get_Camera_Mgr ()->Get_Camera_Mode () == CameraMgr::MODE_WALK_THROUGH) {
		camera = CombatManager::Get_Camera ();
	} else {
		camera = ::Get_Camera_Mgr ()->Get_Camera ();
	}

	return camera;
}


////////////////////////////////////////////////////////////////
//
//	Generate_Pathfind_Portals
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Generate_Pathfind_Portals (void)
{
	CWaitCursor wait_cursor;

	//
	//	Create a pathfind floodfiller object
	//
	PathfindSectorBuilderClass builder;
	builder.Allow_Water_Floodfill (true);
	builder.Initialize ();	

	//
	//	Find all the pathfind start-points in the level and add
	// them to the floodfiller
	//
	NodeClass *node = NULL;
	for (	node = NodeMgrClass::Get_First ();
			node != NULL;
			node = NodeMgrClass::Get_Next (node))
	{
		if (node->Get_Type () == NODE_TYPE_PATHFIND_START) {
			builder.Add_Start_Point (node->Get_Transform ().Get_Translation ());
		}
	}
	
	//
	//	Perform the floodfill from each of the start locations
	//
	builder.Generate_Sectors ();
	builder.Compress_Sectors_For_Pathfind ();

	//
	//	Build height data for flying vehicles
	//
	HeightDBClass::Generate ();

	//
	//	Cleanup
	//
	builder.Shutdown ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Export_VIS
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Export_VIS (LPCTSTR filename)
{
	//
	//	Create the file
	//
	HANDLE hfile = ::CreateFile (filename,
										  GENERIC_WRITE,
										  0,
										  NULL,
										  CREATE_ALWAYS,
										  0L,
										  NULL);

	ASSERT (hfile != INVALID_HANDLE_VALUE);
	if (hfile != INVALID_HANDLE_VALUE) {

		RawFileClass file_obj;
		file_obj.Attach (hfile);
		ChunkSaveClass chunk_save (&file_obj);

		//
		//	Export the data to this file
		//
		Export_Vis_Data (chunk_save);
	}
	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Selection_Boxes
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Update_Selection_Boxes (void)
{
	//
	// Loop through all the nodes in the current selection
	//
	for (int index = 0; index < m_SelectionMgr->Get_Count (); index ++) {
		NodeClass *node = m_SelectionMgr->Get_At (index);
		if (node != NULL) {
			
			//
			// Ask the node to update its selection box (color and pos)
			//
			node->Update_Selection_Box ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Re_Partition_Audio_System
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Re_Partition_Audio_System (void)
{
	const AABoxClass & level_bounds = StaticCullingSystem->Get_Bounding_Box();
	Vector3 min,max;
	min = level_bounds.Center - level_bounds.Extent;
	max = level_bounds.Center + level_bounds.Extent;

	SoundSceneClass *sound_scene = WWAudioClass::Get_Instance ()->Get_Sound_Scene ();
	if (sound_scene != NULL) {
		sound_scene->Re_Partition (min, max);
	}
	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Are_Manual_Vis_Points_Visible
//
////////////////////////////////////////////////////////////////
bool
SceneEditorClass::Are_Manual_Vis_Points_Visible (void)
{	
	return m_ManualVisPointsVisible;
}


////////////////////////////////////////////////////////////////
//
//	Show_Manual_Vis_Points
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Show_Manual_Vis_Points (bool show_points)
{
	if (m_ManualVisPointsVisible != show_points) {
		m_ManualVisPointsVisible = show_points;

		NodeClass *node = NULL;
		for (	node = NodeMgrClass::Get_First ();
				node != NULL;
				node = NodeMgrClass::Get_Next (node))
		{
			if (node->Get_Type () == NODE_TYPE_VIS_POINT) {
				node->Hide (!m_ManualVisPointsVisible);
			}
		}
	}
		
	return ;
}


////////////////////////////////////////////////////////////////
//
//	DoObjectGoto
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::DoObjectGoto (NodeClass *node1, NodeClass *node2)
{
	SANITY_CHECK(node1 != NULL && node2 != NULL && node1->Get_Type () == NODE_TYPE_OBJECT) {
		return ;
	}

	//
	//	Register all the waypaths with the pathfinding system
	//
	NodeClass *node;
	for (	node = NodeMgrClass::Get_First (NODE_TYPE_WAYPATH);
			node != NULL;
			node = NodeMgrClass::Get_Next (node, NODE_TYPE_WAYPATH))
	{
		node->Pre_Export ();
	}

	//
	//	Force the pathfind system to use any available waypath data
	// in its evaluation
	//
	PathfindClass::Get_Instance ()->Generate_Waypath_Sectors_And_Portals ();

	m_MovingObject					= node1;
	PhysicalGameObj *game_obj	= ((ObjectNodeClass *)m_MovingObject)->Peek_Game_Obj ();

	if (game_obj != NULL && game_obj->As_SmartGameObj () != NULL) {
		ActionClass	*action = ((SmartGameObj *)game_obj)->Get_Action ();
		
		ActionParamsStruct parameters;
		parameters.MoveLocation				= node2->Get_Transform ().Get_Translation ();
		parameters.MoveArrivedDistance	= 3.0F;
		action->Goto( parameters );
	}

	//
	//	Unregister all the waypaths with the pathfinding system
	//
	for (	node = NodeMgrClass::Get_First (NODE_TYPE_WAYPATH);
			node != NULL;
			node = NodeMgrClass::Get_Next (node, NODE_TYPE_WAYPATH))
	{
		node->Post_Export ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Export_Lights
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Export_Lights (LPCTSTR filename)
{
	FileClass *file = _TheFileFactory->Get_File (filename);
	if (file != NULL && file->Create () && file->Open (FileClass::WRITE)) {

		//
		//	Write the first chunk header
		//
		ChunkSaveClass csave (file);
		csave.Begin_Chunk (W3D_CHUNK_LIGHTSCAPE);
		
			//
			//	Loop over all the lights in the scene
			//
			for (	NodeClass *node = NodeMgrClass::Get_First (NODE_TYPE_LIGHT);
					node != NULL;
					node = NodeMgrClass::Get_Next (node, NODE_TYPE_LIGHT))
			{
				LightNodeClass *light_node = (LightNodeClass *)node;

				//
				//	Get the actual light from the editor object
				//
				LightClass *light = light_node->Peek_Light ();
				if (light != NULL) {
					
					//
					//	Save this light
					//
					csave.Begin_Chunk (W3D_CHUNK_LIGHTSCAPE_LIGHT);
						light->Save_W3D (csave);
					
						//
						//	Save the light's transform as well
						//
						csave.Begin_Chunk (W3D_CHUNK_LIGHT_TRANSFORM);

							Matrix3D tm = light->Get_Transform ();
							W3dLightTransformStruct w3d_tm ={	tm[0][0], tm[0][1], tm[0][2], tm[0][3],
																		tm[1][0], tm[1][1], tm[1][2], tm[1][3],
																		tm[2][0], tm[2][1], tm[2][2], tm[2][3] };
							csave.Write (&w3d_tm, sizeof (w3d_tm));

						csave.End_Chunk ();
					csave.End_Chunk ();
				}
			}

		csave.End_Chunk ();

		//
		//	Close the file
		//
		file->Close ();		
	}

	_TheFileFactory->Return_File (file);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Import_Lights
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Import_Lights
(
	DynamicVectorClass<StringClass> &		filename_list,
	DynamicVectorClass<LightNodeClass *> *	node_list
)
{
	DynamicVectorClass<LightClass *> light_list;
	::Get_Main_View ()->Allow_Repaint (false);

	//
	//	Build a list of light objects from the LIG files
	//
	int index;
	for (index = 0; index < filename_list.Count (); index ++) {		
		FileClass *file = _TheFileFactory->Get_File (filename_list[index]);
		if (file != NULL && file->Open ()) {

			//
			//	Read the lights from this file
			//
			ChunkLoadClass cload (file);
			uintptr_t group_id = 0;

			while (cload.Open_Chunk ()) {
				switch (cload.Cur_Chunk_ID ()) {
					
					case W3D_CHUNK_LIGHTSCAPE:
						Build_Light_List (cload, light_list, group_id ++);
						break;

					default:
						ASSERT (0);
						TRACE ("Unrecognized chunk id found while importing lights!\n");
						break;
				}

				cload.Close_Chunk ();
			}

			file->Close ();			
		}

		//
		//	Delete the file object
		//
		if (file != NULL) {
			_TheFileFactory->Return_File (file);
		}
	}

	//
	//	Make sure we don't get duplicate lights
	//
	Filter_Lights (light_list);

	//
	//	Get the preset we will use to derive each imported light-node from
	//
	PresetClass *preset = PresetMgrClass::Find_Preset ("Lightscape Imported");
	ASSERT (preset != NULL);

	CString base_name = ::Asset_Name_From_Filename (filename_list[0]);

	//
	//	Add the lights to the level
	//
	for (index = 0; index < light_list.Count (); index ++) {
		
		LightClass *light = light_list[index];
		if (light != NULL) {
			
			//
			//	Create an editor 'node' for this light and configure it from the
			// light render-object.
			//
			if (light->Get_Type () != LightClass::DIRECTIONAL) {
				Matrix3D tm						= light->Get_Transform ();
				LightNodeClass *light_node	= (LightNodeClass *)Create_Node (preset, &tm);
				light_node->Initialize_From_Light (light);				
				light_node->Set_Group_ID ((uintptr_t)light->Get_User_Data ());
				
				CString light_name;
				light_name.Format ("%s%04d", (LPCTSTR)base_name, index + 1);
				light_node->Peek_Light_Phys ()->Set_Name (light_name);

				//
				//	Add this light to the list the caller supplied us with (if necessary)
				//
				if (node_list != NULL) {
					node_list->Add (light_node);
					light_node->Add_Ref ();
				}
			}			

			//
			//	Free our hold on the light
			//
			MEMBER_RELEASE (light);
		}
	}

	::Get_Main_View ()->Allow_Repaint (true);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Build_Light_List
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Build_Light_List
(
	ChunkLoadClass &							cload,
	DynamicVectorClass<LightClass *> &	light_list,
	uintptr_t											group_id
)
{
	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			case W3D_CHUNK_LIGHTSCAPE_LIGHT:
			{
				//
				//	Load the light and add it to the list
				//
				cload.Open_Chunk ();
				LightClass *light = new LightClass;
				light->Load_W3D (cload);
				light->Set_User_Data ((void *)group_id);
				light_list.Add (light);
				cload.Close_Chunk ();

				//
				//	Read the light's transform from the file
				//
				cload.Open_Chunk ();
				ASSERT (cload.Cur_Chunk_ID () == W3D_CHUNK_LIGHT_TRANSFORM);
				if (cload.Cur_Chunk_ID () == W3D_CHUNK_LIGHT_TRANSFORM) {
					
					//
					//	Read the transform
					//
					W3dLightTransformStruct w3d_tm = { 0 };
					cload.Read (&w3d_tm, sizeof (w3d_tm));
					
					//
					//	Convert the persistent-safe transform to a
					// render-obj friendly transform;
					//
					Matrix3D tm (	w3d_tm.Transform [0][0], w3d_tm.Transform [0][1], w3d_tm.Transform [0][2], w3d_tm.Transform [0][3],
										w3d_tm.Transform [1][0], w3d_tm.Transform [1][1], w3d_tm.Transform [1][2], w3d_tm.Transform [1][3],
										w3d_tm.Transform [2][0], w3d_tm.Transform [2][1], w3d_tm.Transform [2][2], w3d_tm.Transform [2][3]);
					light->Set_Transform (tm);
				}
				cload.Close_Chunk ();
			}
			break;
			
			default:
				::Output_Message ("Found invalid chunk in light importer.\r\n");
				break;
		}

		cload.Close_Chunk ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Filter_Lights
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Filter_Lights (DynamicVectorClass<LightClass *> &light_list)
{
	//
	//	Loop over every light
	//
	for (int index1 = 0; index1 < light_list.Count (); index1 ++) {
		LightClass *light1	= light_list[index1];
		bool is_unique			= true;
		
		//
		//	Compare this light against every other light
		//
		for (int index2 = 0; index2 < light_list.Count () && is_unique; index2 ++) {
			
			//
			//	Don't check against itself
			//
			if (index2 != index1) {
				LightClass *light2 = light_list[index2];
				is_unique = (Compare_Lights (light1, light2) == false);
			}
		}

		//
		//	Remove this light from the list if it is not unique
		//
		if (is_unique == false) {
			MEMBER_RELEASE (light1);
			light_list.Delete (index1);
			index1 --;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Display_Sound_Spheres
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Display_Sound_Spheres (bool onoff)
{
	if (m_DisplaySoundSpheres != onoff) {
		m_DisplaySoundSpheres = onoff;

		//
		//	Loop over all the sounds and tell them to turn the spheres
		// on or off.
		//
		for (	NodeClass *node = NodeMgrClass::Get_First (NODE_TYPE_SOUND);
				node != NULL;
				node = NodeMgrClass::Get_Next (node, NODE_TYPE_SOUND))
		{
			node->Show_Attenuation_Spheres (m_DisplaySoundSpheres);
		}
	}

	return ;
}



////////////////////////////////////////////////////////////////
//
//	Display_Light_Spheres
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Display_Light_Spheres (bool onoff)
{
	if (m_DisplayLightSpheres != onoff) {
		m_DisplayLightSpheres = onoff;

		//
		//	Loop over all the lights and tell them to turn the spheres
		// on or off.
		//
		for (	NodeClass *node = NodeMgrClass::Get_First (NODE_TYPE_LIGHT);
				node != NULL;
				node = NodeMgrClass::Get_Next (node, NODE_TYPE_LIGHT))
		{
			node->Show_Attenuation_Spheres (m_DisplayLightSpheres);
		}
	}
		
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Import_Sunlight
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Import_Sunlight (LPCTSTR filename)
{
	DynamicVectorClass<LightClass *> light_list;
	
	//
	//	Open the file
	//
	FileClass *file = _TheFileFactory->Get_File (filename);
	if (file != NULL && file->Open ()) {

		//
		//	Read the lights from this file
		//
		ChunkLoadClass cload (file);
		cload.Open_Chunk ();
			ASSERT (cload.Cur_Chunk_ID () == W3D_CHUNK_LIGHTSCAPE);
			Build_Light_List (cload, light_list);
		cload.Close_Chunk ();

		file->Close ();
		_TheFileFactory->Return_File (file);
	}

	//
	//	Try to find all the directional lights
	//
	LightClass *sunlight		= NULL;
	bool more_than_one		= false;
	for (int index = 0; index < light_list.Count () && !more_than_one; index ++) {
		LightClass *light = light_list[index];
		if (light != NULL) {
			
			//
			//	We assume that every directional light is the 'sunlight'.
			//
			if (light->Get_Type () == LightClass::DIRECTIONAL) {

				if (sunlight == NULL) {
					sunlight = light;
				} else {
					
					//
					//	Check to make sure all directional lights are the same
					//
					if (Compare_Lights (sunlight, light) == false) {
						more_than_one = true;
					}

					MEMBER_RELEASE (light);
				}
			}
		}
	}

	bool apply_changes = (sunlight != NULL);
	if (more_than_one) {

		//
		//	Ask the user if they wish to make these changes
		//
		if (::MessageBox (	::AfxGetMainWnd ()->m_hWnd,
									"There are multiple sunlight definitions in this file which have different settings.  Do you wish to continue?",
									"Ambiguous Lights",
									MB_ICONQUESTION | MB_YESNO) == IDNO)
		{
			apply_changes = false;
		}
	}

	//
	//	Reconfigure the sunlight (if necessary)
	//
	if (apply_changes) {
		LightClass *global_sunlight_obj = Get_Sun_Light ();
		Vector3 diffuse;
		sunlight->Get_Diffuse (&diffuse);
		global_sunlight_obj->Set_Diffuse (diffuse);
		global_sunlight_obj->Set_Intensity (sunlight->Get_Intensity ());
		global_sunlight_obj->Set_Transform (sunlight->Get_Transform ());
		MEMBER_RELEASE (global_sunlight_obj);

		//
		//	Convert the sun's 3x3 roational matrix to a
		// 'rotation' and 'elevation'.
		//
		Matrix3D tm = sunlight->Get_Transform ();
		
		Vector3 test_vector (0, 0, -1);
		test_vector = tm.Rotate_Vector (test_vector);
		test_vector.Normalize ();

		float rotation		= ::atan2 (test_vector.Y, test_vector.X);
		float elevation	= ::asin (test_vector.Z);
		
		Set_Sun_Light_Orientation (rotation, elevation);
		Update_Lighting ();
	}
	
	MEMBER_RELEASE (sunlight);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Compare_Lights
//
////////////////////////////////////////////////////////////////
bool
SceneEditorClass::Compare_Lights (LightClass *light1, LightClass *light2)
{
	bool retval = false;

	//
	//	Are the positions and orientations approximately the same?
	//	Are they both the same type of light?
	//
	const Matrix3D &tm1 = light1->Get_Transform ();
	const Matrix3D &tm2 = light2->Get_Transform ();
	if (	tm1 == tm2 &&
			light1->Get_Type () == light2->Get_Type ())
	{					
		//
		//	Are their color's and intensities approx the same?
		//
		Vector3 ambient1;
		Vector3 ambient2;
		Vector3 specular1;
		Vector3 specular2;
		Vector3 diffuse1;
		Vector3 diffuse2;
		light1->Get_Ambient (&ambient1);
		light2->Get_Ambient (&ambient2);
		light1->Get_Specular (&specular1);
		light2->Get_Specular (&specular2);
		light1->Get_Diffuse (&diffuse1);
		light2->Get_Diffuse (&diffuse2);

		float intensity1 = light1->Get_Intensity ();
		float intensity2 = light2->Get_Intensity ();

		if (	ambient1 == ambient2 &&
				specular1 == specular2 &&
				diffuse1 == diffuse2 &&
				intensity1 == intensity2)
		{
			//
			//	Are their attenuation ranges the same?
			//
			double inner1 = 0;
			double inner2 = 0;
			double outer1 = 0;
			double outer2 = 0;
			light1->Get_Far_Attenuation_Range (inner1, outer1);
			light2->Get_Far_Attenuation_Range (inner2, outer2);
			if (inner1 == inner2 && outer1 == outer2) {
				retval = true;
			}
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Replace_Selection
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Replace_Selection (void)
{
	//
	//	Is there a selection set to replace?
	//
	int count = m_SelectionMgr->Get_Count ();
	if (count > 0) {

		//
		//	Configure a dialog that will allow the user to select a preset
		//
		SelectPresetDialogClass dialog (::AfxGetMainWnd ());
		dialog.Set_Title ("Object Replace");
		dialog.Set_Message ("Select the new preset you want to replace the selected objects with.");
		dialog.Allow_None_Selection (false);

		//
		//	Let the user pick a preset
		//
		if (dialog.DoModal () == IDOK) {

			PresetClass *preset = dialog.Get_Selection ();
			DynamicVectorClass<NodeClass *> new_nodes;
			CWaitCursor wait_cursor;

			//
			//	Loop over all the selected nodes
			//
			int index;
			for (index = 0; index < count; index ++) {
				
				//
				//	Get the transform of this selected node
				//
				NodeClass *node	= m_SelectionMgr->Get_At (index);
				Matrix3D tm			= node->Get_Transform ();

				//
				//	Create a new node with this same transform
				//
				NodeClass *new_node = Create_Node (preset, &tm, 0, true);
				new_node->Set_ID (node->Get_ID ());
				new_nodes.Add (new_node);

				//
				//	Test to see if both node's are game objects of some sort.
				// If they are, then we need to copy the scripts.
				//
				ObjectNodeClass *old_obj_node	= node->As_ObjectNodeClass ();
				ObjectNodeClass *new_obj_node	= new_node->As_ObjectNodeClass ();
				if (old_obj_node != NULL && new_obj_node != NULL) {
					new_obj_node->Copy_Scripts (*old_obj_node);					
				}

				//
				//	Delete the old node
				//
				Delete_Node (node, false);
			}
			
			//
			//	Clear the selection
			//
			m_SelectionMgr->Reset ();

			//
			//	Add the new nodes to the selection set
			//
			for (index = 0; index < new_nodes.Count (); index ++) {
				NodeClass *new_node = new_nodes[index];
				m_SelectionMgr->Add_Node (new_node);
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Generate_Uniform_Sampled_Vis
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Generate_Uniform_Sampled_Vis
(
	float	granularity,
	float	sample_height,
	bool	ignore_bias,
	bool	selection_only,
	bool	farm_mode,
	int	farm_cpu_index,
	int	farm_cpu_total
)
{		
	GeneratingVisDialogClass dialog (granularity, ::AfxGetMainWnd ());
	dialog.Set_Sample_Height (sample_height);
	dialog.Set_Ignore_Bias (ignore_bias);
	dialog.Do_Selection_Only (selection_only);
	if (farm_mode) {
		dialog.Set_Farm_Mode (farm_cpu_index,farm_cpu_total);
	}
	dialog.DoModal ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Generate_Edge_Sampled_Vis
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Generate_Edge_Sampled_Vis
(
	float	granularity,
	bool	/* ignore_bias */,
	bool  farm_mode,
	int	farm_cpu_index,
	int	farm_cpu_total
)
{
	//
	//	Build a list of nodes that should be vis-sampled
	//
	DynamicVectorClass<NodeClass *> node_list;
	VisMgrClass::Build_Node_List (node_list);

	// 
	// Determine the first and last node that this cpu is to process.  In farm
	// mode, the processing is split up amongst several computers...
	//
	int first_node = 0;
	int last_node = node_list.Count();
	if (farm_mode) {
		int node_count = node_list.Count();
		float nodes_per_processor = (float)node_count / (float)farm_cpu_total;
		first_node	= (int)::floor (nodes_per_processor * (float)farm_cpu_index);
		last_node	= (int)::ceil (nodes_per_processor * (float)(farm_cpu_index+1));
	}
		  
	VisGenProgressClass progress_obj;
	progress_obj.Set_Node_Count (last_node - first_node);	

	GeneratingEdgeSampledVisDialogClass *dialog = GeneratingEdgeSampledVisDialogClass::Display ();
	dialog->Set_Progress_Obj (&progress_obj);
	dialog->Enable_Farm_Mode (farm_mode);

	//
	//	Perform the per-node vis sampling
	//
	VisSectorSamplerClass sampler (this, &progress_obj, granularity, MOUSE_CLICK_COLLISION_GROUP);
	for (	int index = first_node;
			index < last_node && !progress_obj.Is_Cancel_Requested ();
			index ++)
	{
		NodeClass *node = node_list[index];
		if (node != NULL) {
			
			//
			// Simply pass off the render object onto the sampler class
			// for point generation
			//
			RenderObjClass *render_obj = node->Peek_Render_Obj ();
			if (render_obj != NULL) {
				sampler.Process (render_obj);
			}
		}

		::General_Pump_Messages ();
	}

	//
	//	Now render any manual vis points
	//
	VisMgrClass::Render_Manual_Vis_Points ();

	dialog->Set_Progress_Obj (NULL);
	dialog->Set_Finished (true);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Generate_Light_Vis
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Generate_Light_Vis
(
	bool	farm_mode,
	int	farm_cpu_index,
	int	farm_cpu_total
)
{
	GeneratingLightVisDialogClass dialog (::AfxGetMainWnd ());
	if (farm_mode) {
		dialog.Set_Farm_Mode (farm_cpu_index,farm_cpu_total);
	}
	dialog.DoModal ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Generate_Manual_Vis
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Generate_Manual_Vis
(
	bool	farm_mode,
	int	farm_cpu_index,
	int	farm_cpu_total
)
{
	GeneratingManualVisDialogClass dialog (::AfxGetMainWnd ());
	if (farm_mode) {
		dialog.Set_Farm_Mode (farm_cpu_index, farm_cpu_total);
	}

	dialog.DoModal ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Display_Editor_Objects
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Display_Editor_Objects (bool onoff)
{
	m_ShowEditorObjects = onoff;

	//
	//	Loop over all the nodes, picking out the editor-only
	// nodes to show or hide.
	//
	DynamicVectorClass<NodeClass *> node_list;
	NodeMgrClass::Build_Full_Node_List (node_list);
	for (int index = 0; index < node_list.Count (); index ++) {
		NodeClass *node = node_list[index];
		if (node != NULL) {
			int type = node->Get_Type ();

			//
			//	Does this node have an editor-only display object?
			//
			if (	type == NODE_TYPE_ZONE ||
					type == NODE_TYPE_WAYPATH ||
					type == NODE_TYPE_WAYPOINT ||
					type == NODE_TYPE_LIGHT ||
					type == NODE_TYPE_TRANSITION ||
					type == NODE_TYPE_TRANSITION_CHARACTER ||
					type == NODE_TYPE_SOUND ||
					type == NODE_TYPE_VIS ||
					type == NODE_TYPE_VIS_POINT ||
					type == NODE_TYPE_PATHFIND_START ||
					type == NODE_TYPE_DUMMY_OBJECT ||
					type == NODE_TYPE_COVER_SPOT ||
					type == NODE_TYPE_COVER_ATTACK_POINT ||
					type == NODE_TYPE_DAMAGE_ZONE ||
					type == NODE_TYPE_BUILDING ||
					type == NODE_TYPE_EDITOR_ONLY_OBJ)
			{
				node->Hide (!onoff);
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Display_Static_Anim_Phys
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Display_Static_Anim_Phys (bool onoff)
{
	m_ShowStaticAnimPhys = onoff;

	//
	//	Get the list of all static objects from the physics scene
	//
	RefPhysListIterator iterator = ::Get_Scene_Editor ()->Get_Static_Object_Iterator ();
	for (iterator.First (); !iterator.Is_Done (); iterator.Next ()) {

		//
		//	Is this object a static anim phys?
		//
		PhysClass *phys_obj							= iterator.Peek_Obj ();
		StaticAnimPhysClass *static_phys_obj	= phys_obj->As_StaticAnimPhysClass ();
		if (static_phys_obj != NULL && static_phys_obj->Peek_Model () != NULL) {
			
			//
			//	Hide the static anim phys object and turn off its collision
			//
			static_phys_obj->Peek_Model ()->Set_Hidden (!onoff);
			if (onoff == false) {
				phys_obj->Inc_Ignore_Counter ();
			} else if (phys_obj->Is_Ignore_Me ()) {
				phys_obj->Dec_Ignore_Counter ();
			}
		}
	}


	/*for (	NodeClass *node = NodeMgrClass::Get_First ();
			node != NULL;
			node = NodeMgrClass::Get_Next (node))
	{
		PhysClass *phys_obj = node->Peek_Physics_Obj ();
		if (phys_obj != NULL && phys_obj->As_StaticAnimPhysClass () != NULL) {
			node->Hide (!onoff);
		}
	}*/

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Enable_Building_Power
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Enable_Building_Power (bool onoff)
{
	//
	//	Make sure the buildings are in a good state
	//
	GameObjManager::Update_Building_Collection_Spheres ();
	GameObjManager::Init_Buildings ();

	//
	//	Loop over all the lights and tell them to turn the spheres
	// on or off.
	//
	for (	NodeClass *node = NodeMgrClass::Get_First (NODE_TYPE_BUILDING);
			node != NULL;
			node = NodeMgrClass::Get_Next (node, NODE_TYPE_BUILDING))
	{
		((BuildingNodeClass *)node)->Enable_Power (onoff);
		((BuildingNodeClass *)node)->Set_Normalized_Health (onoff * 1.0F);
	}

	
	m_BuildingPowerEnabled = onoff;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Reset_Dynamic_Object_Visibility_Status
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Reset_Dynamic_Object_Visibility_Status (void)
{
	//
	//	Get the list of all dynamic objects from the physics scene
	//
	RefPhysListIterator iterator = ::Get_Scene_Editor ()->Get_Dynamic_Object_Iterator ();
	for (iterator.First (); !iterator.Is_Done (); iterator.Next ()) {

		//
		//	Make sure this phys obj is a dynamic object
		//
		PhysClass *phys_obj					= iterator.Peek_Obj ();
		DynamicPhysClass *dyn_phys_obj	= phys_obj->As_DynamicPhysClass ();
		if (dyn_phys_obj != NULL) {

			//
			//	Reset this dynamic object's visibility
			//
			dyn_phys_obj->Update_Visibility_Status ();			
		}
	}
	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Reset_Vis_For_Node
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Reset_Vis_For_Node (NodeClass *node)
{
	if (node == NULL || node->Is_Static () == false) {
		return ;
	}

	//
	//	Make sure this is a static phys object
	//
	PhysClass *phys = node->Peek_Physics_Obj ();
	if (phys != NULL && phys->As_StaticPhysClass () != NULL) {
		StaticPhysClass *static_phys = phys->As_StaticPhysClass ();
		
		//
		//	Get this sector's id
		//
		int vis_id = static_phys->Get_Vis_Sector_ID ();
		if (vis_id > 0) {

			//
			//	Get the vis table for this sector
			//
			VisTableClass *vis_table = VisTableManager.Get_Vis_Table(vis_id, false);
			if (vis_table != NULL) {
				
				//
				//	Reset the visibility data for this sector
				//
				vis_table->Reset_All ();
				VisTableManager.Update_Vis_Table(vis_id,vis_table);
				REF_PTR_RELEASE(vis_table);
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Lighting
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Update_Lighting (void)
{
	Vector3 lev_min;
	Vector3 lev_max;
	Vector3 sundirection;
	
	Get_Level_Extents (lev_min, lev_max);

	AABoxClass box;
	box.Center = lev_min + ((lev_max - lev_min) * 0.5F);
	box.Extent = ((lev_max - lev_min) * 0.5F);
	Invalidate_Lighting_Caches (box);

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Sun_Light_Orientation
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Set_Sun_Light_Orientation (float yaw, float pitch)
{
	PhysicsSceneClass::Set_Sun_Light_Orientation (yaw, pitch);
}


////////////////////////////////////////////////////////////////
//
//	Show_Vis_Window
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::Show_Vis_Window (bool onoff)
{
	if (m_VisWindow.m_hWnd == NULL) {
		m_VisWindow.Create ();
	}

	m_VisWindow.ShowWindow (onoff ? SW_SHOW : SW_HIDE);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Is_Vis_Window_Visible
//
////////////////////////////////////////////////////////////////
bool
SceneEditorClass::Is_Vis_Window_Visible (void)
{
	bool retval = true;

	if (	m_VisWindow.m_hWnd == NULL ||
			m_VisWindow.IsWindowVisible () == false)
	{
		retval = false;
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	On_Vis_Occluders_Rendered
//
////////////////////////////////////////////////////////////////
void
SceneEditorClass::On_Vis_Occluders_Rendered
(
	VisRenderContextClass &	context,
	VisSampleClass &			/* sample */
)
{
	m_VisWindow.Update_Display (*context.VisRasterizer);
	return ;
}
