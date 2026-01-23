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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/TerrainNode.cpp              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/12/02 4:07p                                               $*
 *                                                                                             *
 *                    $Revision:: 36                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "terrainnode.h"
#include "staticphys.h"
#include "sceneeditor.h"
#include "terraindefinition.h"
#include "filemgr.h"
#include "_assetmgr.h"
#include "editorassetmgr.h"
#include "w3d_file.h"
#include "hlod.h"
#include "cameramgr.h"
#include "collisiongroups.h"
#include "persistfactory.h"
#include "editorchunkids.h"
#include "preset.h"
#include "collect.h"
#include "presetmgr.h"
#include "nodemgr.h"
#include "editorsaveload.h"
#include "terrainsectionpersist.h"
#include "lightphys.h"
#include "lightnode.h"
#include "LevelEditView.h"
#include "hlod.h"

//////////////////////////////////////////////////////////////////////////////
//	Persist factory
//////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<TerrainNodeClass, CHUNKID_NODE_TERRAIN> _TerrainNodePersistFactory;
SimplePersistFactoryClass<TerrainNodeClass, CHUNKID_NODE_TERRAIN_SECTION> _TerrainSectionNodePersistFactory;


enum
{
	CHUNKID_VARIABLES					= 0x10251130,
	CHUNKID_BASE_CLASS,
	CHUNKID_SECTION_PERSISTDATA,
	CHUNKID_SECTION_PERSIST_LIST
};

enum
{
	XXX_VARID_VISID			= 0x01,
	VARID_SECTIONID_OLD,
	VARID_SECTIONID,
	VARID_TM
};


//////////////////////////////////////////////////////////////////////////////
//
//	TerrainNodeClass
//
//////////////////////////////////////////////////////////////////////////////
TerrainNodeClass::TerrainNodeClass (PresetClass *preset)	:
	Transform (1),
	LoadedTransform (1),
	NodeClass (preset)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	~TerrainNodeClass
//
//////////////////////////////////////////////////////////////////////////////
TerrainNodeClass::~TerrainNodeClass (void)
{
	Remove_From_Scene ();
	Free_Sections ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Free_Section_Data
//
//////////////////////////////////////////////////////////////////////////////
void
TerrainNodeClass::Free_Section_Data (void)
{
	m_TerrainSectionInfo.Free_List ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Free_Sections
//
//////////////////////////////////////////////////////////////////////////////
void
TerrainNodeClass::Free_Sections (void)
{
	//
	//	Release our hold on all the physics pointers
	//
	for (int index = 0; index < m_Sections.Count (); index ++) {
		NodeClass *sub_node = m_Sections[index];
		MEMBER_RELEASE (sub_node);

		//
		//	Remove the file dependencies for this tile from the file manager.
		//
		if (sub_node != NULL && sub_node->Get_Type () != NODE_TYPE_TERRAIN_SECTION) {
			::Get_File_Mgr ()->Update (sub_node, false);
		}
	}

	m_Sections.Delete_All ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Add_To_Scene
//
//////////////////////////////////////////////////////////////////////////////
void
TerrainNodeClass::Add_To_Scene (void)
{
	//
	//	Add all the sections to the scene
	//
	for (int index = 0; index < m_Sections.Count (); index ++) {
		NodeClass *sub_node = m_Sections[index];
		sub_node->Add_To_Scene ();
	}

	m_IsInScene = true;
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Remove_From_Scene
//
//////////////////////////////////////////////////////////////////////////////
void
TerrainNodeClass::Remove_From_Scene (void)
{
	SceneEditorClass *scene = ::Get_Scene_Editor ();
	if (scene != NULL && m_IsInScene) {
		Build_Section_ID_List ();

		//
		//	Remove all the sections from the scene
		//
		for (int index = 0; index < m_Sections.Count (); index ++) {
			NodeClass *sub_node = m_Sections[index];
			sub_node->Remove_From_Scene ();
		}

		m_IsInScene = false;
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Set_Transform
//
//////////////////////////////////////////////////////////////////////////////
void
TerrainNodeClass::Set_Transform (const Matrix3D &tm)
{
	Special_Set_Transform (tm);
	Transform = tm;	
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Special_Set_Transform
//
//////////////////////////////////////////////////////////////////////////////
void
TerrainNodeClass::Special_Set_Transform (const Matrix3D &tm)
{
	Matrix3D curr_tm = Get_Transform ();
	Matrix3D inv_tm (1);
	curr_tm.Get_Orthogonal_Inverse (inv_tm);

	//
	//	Move all the sub-sections
	//
	for (int index = 0; index < m_Sections.Count (); index ++) {
		NodeClass *sub_node = (TerrainSectionNodeClass *)m_Sections[index];

		if (sub_node->Get_Type () == NODE_TYPE_TERRAIN_SECTION) {
			
			//
			//	Check to see if this sub-object is a dazzle.  If it is, then
			// transform it relative to the terrain...
			//
			RenderObjClass *model = sub_node->Peek_Render_Obj ();		
			if (model != NULL && model->Class_ID () == RenderObjClass::CLASSID_DAZZLE) {
				Matrix3D sub_obj_tm		= sub_node->Get_Transform ();
				Matrix3D rel_sub_obj_tm	= inv_tm * sub_obj_tm;

				Matrix3D new_tm = tm * rel_sub_obj_tm;
				((TerrainSectionNodeClass *)sub_node)->Special_Set_Transform (new_tm);
			} else {
				((TerrainSectionNodeClass *)sub_node)->Special_Set_Transform (tm);
			}
			
		} else if (sub_node->Get_Type () == NODE_TYPE_TERRAIN) {
			((TerrainNodeClass *)sub_node)->Special_Set_Transform (tm);
		} else  {
			//
			//	Transform this object relative to the terrain
			//
			Matrix3D sub_obj_tm		= sub_node->Get_Transform ();
			Matrix3D rel_sub_obj_tm	= inv_tm * sub_obj_tm;

			Matrix3D new_tm = tm * rel_sub_obj_tm;
			sub_node->Set_Transform (new_tm);
		}
	}

	NodeClass::Set_Transform (tm);
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
TerrainNodeClass::Initialize (void)
{
	Build_Section_ID_List ();
	Free_Sections ();

	TerrainDefinitionClass *definition = static_cast<TerrainDefinitionClass *> (m_Preset->Get_Definition ());
	if (definition != NULL) {

		//
		//	Make sure all assets are loaded into memory before this tile is created...
		//
		m_Preset->Load_All_Assets ();
		
		CString filename		= definition->Get_Model_Name ();
		CString asset_name	= ::Asset_Name_From_Filename (filename);

		//
		//	Filename valid?
		//
		if (filename.GetLength () > 0) {
			filename = ::Get_File_Mgr ()->Make_Full_Path (filename);
			_pThe3DAssetManager->Set_Current_Directory (::Strip_Filename_From_Path (filename));

			//
			// Create the terrain
			//
			RenderObjClass *terrain = ::Create_Render_Obj (asset_name);
			if (terrain != NULL) {

				//
				// Loop through all the sections inside the mesh collection and
				// create static phys objects for each one.
				//
				int section_count = terrain->Get_Num_Sub_Objects ();
				if (section_count > 0) {
					for (int index = 0; index < section_count; index ++) {
						RenderObjClass *sub_obj = terrain->Get_Sub_Object (index);
						if (sub_obj != NULL) {

							//
							//	Create a new terrain section and add it to our list
							//
							TerrainSectionNodeClass *sub_node = new TerrainSectionNodeClass;
							sub_node->Create (sub_obj);
							sub_node->Set_Terrain (this);
							m_Sections.Add (sub_node);
							
							MEMBER_RELEASE (sub_obj);
						}																		
					}
				}
				
				//
				//	Build a list of proxy objects
				//
				DynamicVectorClass<ProxyClass> proxy_list;
				if (terrain->Class_ID () == RenderObjClass::CLASSID_COLLECTION) {

					//
					//	Add all the proxies from the collection to our list
					//
					CollectionClass *collection = (CollectionClass *)terrain;
					for (int index = 0; index < collection->Get_Proxy_Count (); index ++) {
						ProxyClass proxy;
						if (collection->Get_Proxy (index, proxy)) {
							proxy_list.Add (proxy);
						}
					}

				} else  if (terrain->Class_ID () == RenderObjClass::CLASSID_HLOD) {

					//
					//	Add all the proxies from the HLOD to our list
					//
					HLodClass *hlod = reinterpret_cast<HLodClass *>(terrain);
					for (int index = 0; index < hlod->Get_Proxy_Count (); index ++) {
						ProxyClass proxy;
						if (hlod->Get_Proxy (index, proxy)) {
							proxy_list.Add (proxy);
						}
					}					
				}
				
				//
				//	Create the proxy objects
				//
				Create_Proxies (proxy_list);

				//
				//	Create all the lights that are associated with this terrain
				//
				Create_Lights ();

				MEMBER_RELEASE (terrain);
			}
		}

		//
		//	Make sure we restore all the VIS ids (if necessary)
		//
		Assign_Section_IDs ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Assign_Section_IDs
//
////////////////////////////////////////////////////////////////
void
TerrainNodeClass::Assign_Section_IDs (void)
{
	m_TerrainSectionInfo.Assign_Section_IDs (this);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Build_Section_ID_List
//
////////////////////////////////////////////////////////////////
void
TerrainNodeClass::Build_Section_ID_List (void)
{
	//
	//	Rebuild the list from scratch (if there is anything to do)
	//
	if (In_Scene () && m_Sections.Count () > 0) {
		Free_Section_Data ();
		m_TerrainSectionInfo.Build_List (m_Sections);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
TerrainNodeClass::Get_Factory (void) const
{	
	return _TerrainNodePersistFactory;
}


/////////////////////////////////////////////////////////////////
//
//	Add_Vis_Points
//
/////////////////////////////////////////////////////////////////
void
TerrainNodeClass::Add_Vis_Points
(
	VisPointGeneratorClass &	/* generator */,
	RenderObjClass *				/* render_obj */
)
{
	/*for (int index = 0; index < m_Sections.Count (); index ++) {
		StaticPhysClass *phys_obj = m_Sections[index];

		//
		//	Pass all the sections onto the generator
		//
		RenderObjClass *render_obj = phys_obj->Peek_Model ();
		if (render_obj != NULL) {
			NodeClass::Add_Vis_Points (generator, render_obj);
		}
	}*/

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Hide
//
/////////////////////////////////////////////////////////////////
void
TerrainNodeClass::Hide (bool hide)
{
	for (int index = 0; index < m_Sections.Count (); index ++) {
		NodeClass *node = m_Sections[index];
		node->Hide (hide);
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Is_Hidden
//
/////////////////////////////////////////////////////////////////
bool
TerrainNodeClass::Is_Hidden (void) const
{
	bool retval = false;

	if (m_Sections.Count () > 0) {
		NodeClass *node = m_Sections[0];
		retval = node->Is_Hidden ();
	}

	return retval;
}


/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
bool
TerrainNodeClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		NodeClass::Save (csave);
	csave.End_Chunk ();	

	//
	//	Write a chunk for each section so we can store instance
	// specific data.
	//
	Build_Section_ID_List ();
	csave.Begin_Chunk (CHUNKID_SECTION_PERSIST_LIST);
		m_TerrainSectionInfo.Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);
		WRITE_MICRO_CHUNK (csave, VARID_TM, Transform)
	csave.End_Chunk ();
	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////
bool
TerrainNodeClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_BASE_CLASS:
				NodeClass::Load (cload);
				break;
			
			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;

			case CHUNKID_SECTION_PERSIST_LIST:
				m_TerrainSectionInfo.Load (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	SaveLoadSystemClass::Register_Post_Load_Callback (this);
	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
/////////////////////////////////////////////////////////////////
bool
TerrainNodeClass::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK (cload, VARID_TM, LoadedTransform)
			
			case VARID_SECTIONID_OLD:
			case VARID_SECTIONID:
			{
				//
				//	Force vis to be reset
				//
				PhysicsSceneClass::Get_Instance ()->Reset_Vis ();
				EditorSaveLoadClass::Set_Loaded_Vis_Valid (false);
				::Output_Message ("Old-style terrain section ID found, resetting VIS data.\r\n");
			}
			break;
		}

		cload.Close_Micro_Chunk ();
	}

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Create_Lights
//
////////////////////////////////////////////////////////////////
void
TerrainNodeClass::Create_Lights (void)
{	
	if (::Get_Scene_Editor ()->Is_Proxy_Creation_Enabled () == false) {
		return ;
	}

	//
	//	Get the terrain's definition
	//
	TerrainDefinitionClass *definition = static_cast<TerrainDefinitionClass *> (m_Preset->Get_Definition ());
	if (definition == NULL) {
		return ;
	}
	::Get_Main_View ()->Allow_Repaint (false);

	//
	//	Get the filename for the light database
	//
	CString full_path = ::Get_File_Mgr ()->Make_Full_Path (definition->Get_Light_Filename ());
	DynamicVectorClass<StringClass> filename_list;
	filename_list.Add ((LPCTSTR)full_path);

	//
	//	Import the lights into the level
	//
	DynamicVectorClass<LightNodeClass *> node_list;
	::Get_Scene_Editor ()->Import_Lights (filename_list, &node_list);
	
	//
	//	Add all these lights to our section list
	//
	for (int index = 0; index < node_list.Count (); index ++) {
		LightNodeClass *node = node_list[index];
		if (node != NULL) {
			
			//
			//	Transform this light from world-relative to terrain relative
			//
			node->Set_Transform (Get_Transform () * node->Get_Transform ());

			//
			//	Add the light to our section list and 'remove' it from the node manager
			//			
			node->Lock (true);
			m_Sections.Add (node);
			NodeMgrClass::Remove_Node (node);
		}
	}

	::Get_Main_View ()->Allow_Repaint (true);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Find_Proxy_Preset
//
////////////////////////////////////////////////////////////////
PresetClass *
TerrainNodeClass::Find_Proxy_Preset (const char *preset_name)
{
	bool is_restricted_user = ::Get_File_Mgr ()->Is_Special_User ();
	
	PresetClass *retval = NULL;

	//
	//	Loop over all the presets, until we've found one that matches the
	// requirements
	//
	bool keep_going	= true;
	bool is_preferred	= false;
	for (	PresetClass *preset = PresetMgrClass::Get_First ();
			preset != NULL && keep_going;
			preset = PresetMgrClass::Get_Next (preset))
	{
		//
		//	Is this the preset we are looking for?
		//
		if (::lstrcmpi (preset->Get_Name (), preset_name) == 0) {
			
			if (is_preferred == false) {
				retval = preset;
			}

			if (preset->Is_A_Parent (PROXY_TESTS_FOLDER)) {
				retval = preset;
				keep_going = false;
			} else if (is_restricted_user == preset->Is_A_Parent (SPECIAL_USER_FOLDER)) {
				
				//
				//	Restricted users prefer presets under their folder, other's prefer
				// presets not under the restricted presets folder.
				//
				is_preferred = true;
			}
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Create_Proxies
//
////////////////////////////////////////////////////////////////
void
TerrainNodeClass::Create_Proxies (DynamicVectorClass<ProxyClass> &proxy_list)
{
	if (::Get_Scene_Editor ()->Is_Proxy_Creation_Enabled () == false) {
		return ;
	}

	//
	//	Loop over all the proxy objects in the list
	//
	int count = proxy_list.Count ();
	for (int index = 0; index < count; index ++) {
		
		//
		// Get information about this proxy
		//
		Matrix3D rel_transform	= proxy_list[index].Get_Transform ();
		CString preset_name		= proxy_list[index].Get_Name ();

		//
		// Find the preset this placeholder references
		//
		PresetClass *preset = Find_Proxy_Preset (preset_name);
		if (preset != NULL) {						
			
			//
			// Create the node from the base
			//
			NodeClass *node = ::Get_Scene_Editor ()->Create_Node (preset, &rel_transform, 0, false);
			ASSERT (node != NULL);
			if (node != NULL) {
								
				//
				//	Change some flags on the object
				//
				node->Restrict_Rotation (false);
				node->Set_Is_Proxied (true);

				//
				//	Is the node configured correctly?
				//
				if (node->Peek_Physics_Obj () != NULL || node->Get_Type () == NODE_TYPE_TERRAIN) {

					//
					// Normalize the rotation of this node
					//
					node->Rotate (Matrix3D (1), Matrix3D (1));
					node->Lock (true);

					//
					//	We have to tell the terrain it can really be moved...
					//
					if (node->Get_Type () == NODE_TYPE_TERRAIN) {
						((TerrainNodeClass *)node)->Special_Set_Transform (rel_transform);
					}

					//
					// Remove this node from the system, and add it to our
					// local sub-node list.
					//
					node->Add_Ref ();
					m_Sections.Add (node);
					NodeMgrClass::Remove_Node (node);
					::Get_File_Mgr ()->Update (node, true);

				} else {

					::Get_Scene_Editor ()->Delete_Node (node, false);

					CString message;
					message.Format ("Unable to create physics object for placeholder %s.\r\n", (LPCTSTR)preset_name);
					::Output_Message (message);
				}
			}

		} else {

			CString message;
			message.Format ("Unable to find preset for placeholder %s.\r\n", (LPCTSTR)preset_name);
			::Output_Message (message);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Update_Cached_Vis_IDs
//
/////////////////////////////////////////////////////////////////
void
TerrainNodeClass::Update_Cached_Vis_IDs (void)
{
	//
	//	Pass this call onto all subobjects
	//
	for (int index = 0; index < m_Sections.Count (); index ++) {
		NodeClass *node = m_Sections[index];
		if (node != NULL) {
			node->Update_Cached_Vis_IDs ();
		}
	}

	Build_Section_ID_List ();
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Reload
//
/////////////////////////////////////////////////////////////////
void
TerrainNodeClass::Reload (void)
{
	NodeClass::Reload ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Pre_Export
//
//////////////////////////////////////////////////////////////////////
void
TerrainNodeClass::Pre_Export (void)
{
	NodeClass::Pre_Export ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Post_Export
//
//////////////////////////////////////////////////////////////////////
void
TerrainNodeClass::Post_Export (void)
{
	NodeClass::Post_Export ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Is_A_Child_Node
//
//////////////////////////////////////////////////////////////////////
bool
TerrainNodeClass::Is_A_Child_Node (NodeClass *node) const
{
	bool retval = false;

	//
	//	Test each sub object
	//
	for (int index = 0; retval == false && index < m_Sections.Count (); index ++) {
		NodeClass *curr_node = m_Sections[index];
		if (curr_node != NULL) {
			
			//
			//	Is this the node we are looking for?
			//
			if (curr_node == node) {
				retval = true;
			} else {
				
				//
				//	Pass this call onto the child
				//
				retval = curr_node->Is_A_Child_Node (node);
			}			
		}
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	On_Post_Load
//
///////////////////////////////////////////////////////////////////////
void
TerrainNodeClass::On_Post_Load (void)
{
	m_TerrainSectionInfo.Initialize_Virgin_Sections ();

	//
	//	Transform the terrain to its new position
	//
	if (m_IsProxied == false && LoadedTransform != Matrix3D::Identity) {
		Set_Transform (LoadedTransform);	
	}

	return ;
}


//******************************************************************************//
//*
//*	Start of TerrainSectionNodeClass
//*
//******************************************************************************//


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
TerrainSectionNodeClass::Get_Factory (void) const
{	
	return _TerrainSectionNodePersistFactory;
}


////////////////////////////////////////////////////////////////
//
//	Create
//
////////////////////////////////////////////////////////////////
void
TerrainSectionNodeClass::Create (RenderObjClass *render_obj)
{
	MEMBER_RELEASE (m_PhysObj);
	
	//
	// Create the new terrain section from the render object
	//
	m_PhysObj = new StaticPhysClass;
	m_PhysObj->Set_Model (render_obj);
	m_PhysObj->Set_Transform (render_obj->Get_Transform ());	

	//
	//	Give this section an ID (and pass it along to its physics obj)
	//
	m_PhysObj->Set_ID (Get_ID ());
	Set_ID (NodeMgrClass::Get_Node_ID (Get_Type ()));
	m_PhysObj->Peek_Model ()->Set_User_Data ((PVOID)&m_HitTestInfo, false);

	//
	//	Give the new node a name
	//
	Set_Name (render_obj->Get_Name ());
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Transform
//
////////////////////////////////////////////////////////////////
void
TerrainSectionNodeClass::Set_Transform (const Matrix3D &tm)
{
	if (Terrain == NULL) {
		return ;
	}

	//
	//	Get the inverse tranform of the terrain section
	//
	Matrix3D curr_tm = Get_Transform ();
	Matrix3D inv_tm;
	curr_tm.Get_Orthogonal_Inverse (inv_tm);

	Matrix3D delta_tm = inv_tm * tm;

	//
	//	Transform the terrain object relative to this section
	//
	Matrix3D terrain_obj_tm			= Terrain->Get_Transform ();
	//Matrix3D rel_terrain_obj_tm	= inv_tm * terrain_obj_tm;

	Matrix3D new_tm = terrain_obj_tm * delta_tm;
	Terrain->Set_Transform (new_tm);
	return ;
}
