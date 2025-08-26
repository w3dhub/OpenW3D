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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/VisMgr.cpp                   $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/26/01 2:46p                                               $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "vismgr.h"
#include "sceneeditor.h"
#include "matrix3d.h"
#include "w3d_file.h"
#include "chunkio.h"
#include "Utils.h"
#include "staticphys.h"
#include "node.h"
#include "nodemgr.h"
#include "cameramgr.h"
#include "camera.h"
#include "VisPointNode.h"
#include "groupmgr.h"


///////////////////////////////////////////////////////////////////////
//	Local constants
///////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VIS_OBJECT	=	0x06150222,
	CHUNKID_VIS_DATA	
};


///////////////////////////////////////////////////////////////////////
//	Local structures
///////////////////////////////////////////////////////////////////////
typedef struct
{
	char			mesh_name[W3D_NAME_LEN * 2];
	Matrix3D		transform;
	int			vis_id;
}
VIS_OBJECT_INFO;


///////////////////////////////////////////////////////////////////////
//
//	Export_Remap_Data
//
///////////////////////////////////////////////////////////////////////
void
VisMgrClass::Export_Remap_Data (ChunkSaveClass &csave)
{
	//
	//	Get the list of all static objects from the physics scene
	//
	RefPhysListIterator iterator = ::Get_Scene_Editor ()->Get_Static_Object_Iterator ();
	for (iterator.First (); !iterator.Is_Done (); iterator.Next ()) {

		//
		//	Double-check to make sure this object is static
		//
		PhysClass *phys_obj					= iterator.Peek_Obj ();
		StaticPhysClass *static_phys_obj	= phys_obj->As_StaticPhysClass ();
		if (static_phys_obj != NULL && static_phys_obj->Peek_Model () != NULL) {

			//
			//	Build a structure of information about this static object
			//
			VIS_OBJECT_INFO obj_info;
			::lstrcpy (obj_info.mesh_name, static_phys_obj->Peek_Model ()->Get_Name ());
			obj_info.transform	= static_phys_obj->Get_Transform ();
			obj_info.vis_id		= static_phys_obj->Get_Vis_Object_ID ();
			
			//
			//	Save the object info to its own chunk
			//
			csave.Begin_Chunk (CHUNKID_VIS_OBJECT);
				csave.Write (&obj_info, sizeof (obj_info));
			csave.End_Chunk ();
		}			
	}		
	

	//
	//	Now export all the vis data
	//
	csave.Begin_Chunk (CHUNKID_VIS_DATA);
		::Get_Scene_Editor ()->Export_Vis_Data (csave);
	csave.End_Chunk ();
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Import_Remap_Data
//
///////////////////////////////////////////////////////////////////////
void
VisMgrClass::Import_Remap_Data (ChunkLoadClass &cload)
{
	bool is_valid = true;

	//
	//	Start with a clean slate
	//
	::Get_Scene_Editor ()->Discard_Vis ();

	//
	//	Process all the chunks in this file
	//
	while (is_valid && cload.Open_Chunk ()) {
		
		switch (cload.Cur_Chunk_ID ())
		{
			//
			//	Try to remap the vis-id of this object with the
			// vis-id of the object in the level where vis was generated.
			//
			case CHUNKID_VIS_OBJECT:
			{
				VIS_OBJECT_INFO info;
				cload.Read (&info, sizeof (info));

				//
				//	Attempt to find this physics object in the scene
				//
				StaticPhysClass *vis_object = Find_Static_Phys_Object (info.mesh_name, info.transform);
				if (vis_object != NULL) {
					vis_object->Set_Vis_Object_ID (info.vis_id);
				} else {
					is_valid = false;
				}
			}
			break;

			//
			//	Import the actual vis data.  If we successfully
			// remapped the vis-id's of all the objects, then this
			// should work seamlessly
			//
			case CHUNKID_VIS_DATA:
			{
				::Get_Scene_Editor ()->Import_Vis_Data (cload);
			}
			break;
		}

		cload.Close_Chunk ();
	}

	//
	//	Warn the user if we failed
	//
	if (is_valid == false) {
		::MessageBox (	NULL,
							"The static geometry in this level does not match the geometry of the level in which vis was generated.  Unable to import vis data into this level.",
							"Import Error",
							MB_ICONERROR | MB_OK);
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Find_Static_Phys_Object
//
///////////////////////////////////////////////////////////////////////
StaticPhysClass *
VisMgrClass::Find_Static_Phys_Object (LPCTSTR mesh_name, const Matrix3D &tm)
{
	StaticPhysClass *retval = NULL;

	//
	//	Get the list of all static objects from the physics scene
	//
	RefPhysListIterator iterator = ::Get_Scene_Editor ()->Get_Static_Object_Iterator ();
	for (iterator.First (); retval == NULL && !iterator.Is_Done (); iterator.Next ()) {
		
		//
		//	Double-check to make sure this object is static
		//
		PhysClass *phys_obj					= iterator.Peek_Obj ();
		StaticPhysClass *static_phys_obj	= phys_obj->As_StaticPhysClass ();
		if (static_phys_obj != NULL && static_phys_obj->Peek_Model () != NULL) {

			//
			//	Is this the object we are looking for?
			//
			LPCTSTR curr_name	= static_phys_obj->Peek_Model ()->Get_Name ();
			Matrix3D curr_tm	= static_phys_obj->Get_Transform ();
			if ((::lstrcmpi (curr_name, mesh_name) == 0) && (curr_tm == tm)) {							
				retval = static_phys_obj;
			}
		}
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Build_Node_List
//
///////////////////////////////////////////////////////////////////////
void
VisMgrClass::Build_Node_List (DynamicVectorClass<NodeClass *> &node_list, bool selection_only)
{
	if (selection_only) {
		SelectionMgrClass &sel_mgr = ::Get_Scene_Editor ()->Get_Selection_Mgr ();
		
		//
		//	Build a list of the selected nodes that should be used to calculate
		// vis renders
		//		
		for (int index = 0; index < sel_mgr.Get_Count (); index ++) {
			NodeClass *node = sel_mgr.Get_At (index);
			if (node != NULL && node->Is_Static ()) {
				Add_Nodes_To_List (node_list, node);
			}
		}

	} else {

		//
		//	Build a list of nodes that should be used to calculate
		// vis renders
		//
		for (NodeClass *node = ::Get_Node_Mgr ().Get_First ();
			  node != NULL;
			  node = ::Get_Node_Mgr ().Get_Next (node))
		{
			if (node->Is_Static ()) {
				Add_Nodes_To_List (node_list, node);
			}
		}
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Add_Nodes_To_List
//
///////////////////////////////////////////////////////////////////////
void
VisMgrClass::Add_Nodes_To_List
(
	DynamicVectorClass<NodeClass *> &node_list,
	NodeClass *node
)
{
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
	} else {
		if (node->Has_Vis_Sectors ()) {
			node_list.Add (node);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Render_Manual_Vis_Points
//
//////////////////////////////////////////////////////////////////////////////
void
VisMgrClass::Render_Manual_Vis_Points
(
	bool									farm_mode,
	int									processor_index,
	int									total_processors,
	VIS_POINT_RENDERED_CALLBACK	callback,
	DWORD									param
)
{
	SceneEditorClass *scene_editor	= ::Get_Scene_Editor ();
	VisLogClass &vis_log					= scene_editor->Get_Vis_Log ();
	CameraClass *camera					= new CameraClass (*::Get_Camera_Mgr ()->Get_Camera ());
	bool keep_going						= true;

	//
	//	Bulid a list of vis points
	//
	DynamicVectorClass<VisPointNodeClass *> point_list;
	VisPointNodeClass *vis_point = NULL;
	for (	vis_point = (VisPointNodeClass *)NodeMgrClass::Get_First (NODE_TYPE_VIS_POINT);
			vis_point != NULL && keep_going;
			vis_point = (VisPointNodeClass *)NodeMgrClass::Get_Next (vis_point, NODE_TYPE_VIS_POINT))
	{
		point_list.Add (vis_point);
	}

	//
	//	Determine which nodes to process if we
	// are in farm mode.
	//
	if (farm_mode) {
		int	points					= point_list.Count ();
		float	points_per_processor = ((float)points) / ((float)total_processors);

		int starting_point	= (int)::floor (points_per_processor* (float)processor_index);
		int ending_point		= (int)::ceil (points_per_processor * (float)(processor_index + 1));
		ending_point = min (ending_point, points);

		//
		//	Copy the points from the total list into a temporary list
		//
		DynamicVectorClass<VisPointNodeClass *> temp_list;
		for (int index = starting_point; index < ending_point; index ++) {
			temp_list.Add (point_list[index]);
		}

		point_list.Delete_All ();
		point_list = temp_list;
	}

	//
	//	Loop over all manual vis points in the level
	//	
	for (int index = 0; index < point_list.Count (); index ++)
	{
		DWORD before = ::GetTickCount ();
		VisPointNodeClass *vis_point = point_list[index];

		//	Set the camera up to use the same settings we used when
		// we generated this manual vis point
		//
		vis_point->Setup_Camera (*camera);

		//
		//	Get the camera's clip plane settings
		//
		float znear = 0;
		float zfar = 0;
		camera->Get_Clip_Planes (znear, zfar);

		//
		// Convert the object transform to a camera transform
		//
		Matrix3D	transform = vis_point->Get_Transform ();
		Matrix3D cam_transform (Vector3 (0, -1, 0), Vector3 (0, 0, 1), Vector3 (-1, 0, 0), Vector3 (0, 0, 0));
		Matrix3D new_transform	= transform * cam_transform;
		Vector3 sample_location	= (new_transform * Vector3 (0, 0, -znear));

		//
		//
		//	Render vis for this point
		//
		VisSampleClass vis_sample = scene_editor->Update_Vis (sample_location,
																				new_transform,
																				VisDirBitsType(VIS_FORWARD_BIT | VIS_DONT_RECENTER),
																				camera);
		vis_log.Log_Sample (vis_sample);
		scene_editor->Create_Vis_Point (transform);

		//
		//	Notify the callback that we've renedered a point
		//				
		if (callback != NULL) {
			DWORD after	= ::GetTickCount ();		
			keep_going	= (*callback) (after - before, param);
		}
	}

	MEMBER_RELEASE (camera);
	return ;
}
