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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/TerrainSectionPersist.cpp    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/29/01 12:22p                                             $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "TerrainSectionPersist.h"
#include "node.h"
#include "chunkio.h"
#include "staticphys.h"
#include "lightnode.h"
#include "terrainnode.h"
#include "nodemgr.h"


//////////////////////////////////////////////////////////////////////////////
//	Constants
//////////////////////////////////////////////////////////////////////////////

enum
{
	CHUNKID_VARIABLES						= 0x10251130,
	CHUNKID_TERRAIN_SECTION_INFO,
	CHUNKID_SECTION_PERSISTDATA
};

enum
{
	VARID_POSITION							= 0x01,
	VARID_DEF_ID,
	VARID_INSTANCE_ID,
	VARID_VIS_OBJ_ID,
	VARID_VIS_SECTOR_ID,
	VARID_NAME,
	VARID_CULLLINK
};


//////////////////////////////////////////////////////////////////////////////
//
//	TerrainSectionPersistClass
//
//////////////////////////////////////////////////////////////////////////////
TerrainSectionPersistClass::TerrainSectionPersistClass (void)
	:	m_Position (0, 0, 0),
		m_DefinitionID (0),
		m_InstanceID (0),
		m_VisObjectID (0),
		m_VisSectorID (0),
		m_CullLink (uint32(-1))
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	~TerrainSectionPersistClass
//
//////////////////////////////////////////////////////////////////////////////
TerrainSectionPersistClass::~TerrainSectionPersistClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Apply
//
//////////////////////////////////////////////////////////////////////////////
void
TerrainSectionPersistClass::Apply (NodeClass *node)
{
	SANITY_CHECK (node != NULL)
	{
		return ;
	}
	
	//
	//	Assign the id to the section
	//
	node->Set_ID (Get_Instance_ID ());
	node->Set_Cull_Link (m_CullLink);
	
	//
	//	Assign the other IDs (these differ based on type)
	//
	if (node->Get_Type () == NODE_TYPE_TERRAIN) {		
		m_TerrainSectionInfo.Assign_Section_IDs (reinterpret_cast<TerrainNodeClass *>(node));
	} else {
		
		//
		//	Assign this physics object its vis ID (if applicable)
		//
		PhysClass *phys_obj = node->Peek_Physics_Obj ();
		if (phys_obj != NULL) {			
			
			if (phys_obj->As_StaticPhysClass () != NULL) {
				((StaticPhysClass *)phys_obj)->Set_Vis_Object_ID (Get_Vis_Obj_ID ());
				((StaticPhysClass *)phys_obj)->Set_Vis_Sector_ID (Get_Vis_Sector_ID ());
			} else if (node->Get_Type () == NODE_TYPE_LIGHT) {
				((LightNodeClass *)node)->Set_Vis_Sector_ID (Get_Vis_Sector_ID ());
			}
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////////////
void
TerrainSectionPersistClass::Initialize (NodeClass *node)
{
	SANITY_CHECK (node != NULL)
	{
		return ;
	}

	m_Position			= node->Get_Position ();	
	m_InstanceID		= node->Get_ID ();
	m_Name				= node->Get_Model_Name ();
	
	node->Update_Cached_Cull_Link ();
	m_CullLink = node->Get_Cull_Link ();

	//
	//	Lookup the node's definition ID
	//
	PresetClass *preset = node->Get_Preset ();
	if (preset != NULL) {
		m_DefinitionID	= preset->Get_Definition ()->Get_ID ();
	}

	if (node->Get_Type () == NODE_TYPE_LIGHT) {
		
		//
		//	For lights we have to get the ID straight from the light object (not its phys obj)
		//
		m_VisSectorID = ((LightNodeClass *)node)->Get_Vis_Sector_ID ();
	} else if (node->Get_Type () == NODE_TYPE_TERRAIN) {

		//
		//	For terrain nodes we need to save (recursively possibly) information
		// about all of its sections
		//
		m_TerrainSectionInfo.Build_List (((TerrainNodeClass *)node)->m_Sections);
	} else {

		//
		//	Lookup the vis IDs for this object (if it has any)
		//
		PhysClass *phys_obj = node->Peek_Physics_Obj ();
		if (phys_obj != NULL) {
			StaticPhysClass *static_phys_obj = phys_obj->As_StaticPhysClass ();
			if (static_phys_obj != NULL) {
				m_VisObjectID = static_phys_obj->Get_Vis_Object_ID ();
				m_VisSectorID = static_phys_obj->Get_Vis_Sector_ID ();

				//
				//	Record the cull-link index from the static object
				//
				AABTreeLinkClass *link = (AABTreeLinkClass *)static_phys_obj->Get_Cull_Link ();
				if (link != NULL) {
					m_CullLink = link->Node->Index;
				}
			}
		}
	}
	
	return ;
}



/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
bool
TerrainSectionPersistClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_VARIABLES);
		Save_Variables (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_TERRAIN_SECTION_INFO);
		m_TerrainSectionInfo.Save (csave);
	csave.End_Chunk ();
	
	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////
bool
TerrainSectionPersistClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;

			case CHUNKID_TERRAIN_SECTION_INFO:
				m_TerrainSectionInfo.Load (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Save_Variables
//
/////////////////////////////////////////////////////////////////
bool
TerrainSectionPersistClass::Save_Variables (ChunkSaveClass &csave)
{
	WRITE_MICRO_CHUNK (csave, VARID_POSITION,			m_Position);
	WRITE_MICRO_CHUNK (csave, VARID_DEF_ID,			m_DefinitionID);
	WRITE_MICRO_CHUNK (csave, VARID_INSTANCE_ID,		m_InstanceID);
	WRITE_MICRO_CHUNK (csave, VARID_VIS_OBJ_ID,		m_VisObjectID);
	WRITE_MICRO_CHUNK (csave, VARID_VIS_SECTOR_ID,	m_VisSectorID);
	WRITE_MICRO_CHUNK (csave, VARID_CULLLINK,			m_CullLink);	
	WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_NAME,	m_Name);
	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
/////////////////////////////////////////////////////////////////
bool
TerrainSectionPersistClass::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			
			READ_MICRO_CHUNK (cload, VARID_POSITION,			m_Position);
			READ_MICRO_CHUNK (cload, VARID_DEF_ID,				m_DefinitionID);
			READ_MICRO_CHUNK (cload, VARID_INSTANCE_ID,		m_InstanceID);
			READ_MICRO_CHUNK (cload, VARID_VIS_OBJ_ID,		m_VisObjectID);
			READ_MICRO_CHUNK (cload, VARID_VIS_SECTOR_ID,	m_VisSectorID);
			READ_MICRO_CHUNK (cload, VARID_CULLLINK,			m_CullLink);	
			READ_MICRO_CHUNK_WWSTRING (cload, VARID_NAME,	m_Name);
		}

		cload.Close_Micro_Chunk ();
	}

	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Build_List
//
/////////////////////////////////////////////////////////////////
void
TerrainSectionPersistListClass::Build_List (DynamicVectorClass<NodeClass *> &node_list)
{
	Free_List ();

	//
	//	Build an identifier for each of the sections (based on position
	// and type).
	//
	for (int index = 0; index < node_list.Count (); index ++) {
		NodeClass *node = node_list[index];

		//
		//	Create a new perist object to store the node's data
		//
		TerrainSectionPersistClass *persist_obj = new TerrainSectionPersistClass;
		persist_obj->Initialize (node);
		Add (persist_obj);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Free_List
//
//////////////////////////////////////////////////////////////////////////////
void
TerrainSectionPersistListClass::Free_List (void)
{
	//
	//	Free each persist object
	//
	for (int index = 0; index < Count (); index ++) {
		SAFE_DELETE ((*this)[index]);
	}

	Delete_All ();
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
void
TerrainSectionPersistListClass::Save (ChunkSaveClass &csave)
{
	//
	//	Write a chunk for each section so we can store instance
	// specific data.
	//
	for (int index = 0; index < Count (); index ++) {
		TerrainSectionPersistClass *persist_obj = (*this)[index];

		csave.Begin_Chunk (CHUNKID_SECTION_PERSISTDATA);
			persist_obj->Save (csave);
		csave.End_Chunk ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////
void
TerrainSectionPersistListClass::Load (ChunkLoadClass &cload)
{
	Free_List ();

	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_SECTION_PERSISTDATA:
			{
				//
				//	Create a persist object and get it to load the data
				//
				TerrainSectionPersistClass *persist_info = new TerrainSectionPersistClass;
				persist_info->Load (cload);
				Add (persist_info);
			}
			break;
		}

		cload.Close_Chunk ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Initialize_Virgin_Sections
//
////////////////////////////////////////////////////////////////
void
TerrainSectionPersistListClass::Initialize_Virgin_Sections (void)
{
	//
	//	Give each section a unique ID if we couldn't match it up with
	// its saved version.
	//
	int index;
	for (index = 0; index < m_VirginSections.Count (); index ++) {
		NodeClass *node = m_VirginSections[index];
		if (node != NULL) {
			node->Set_ID (NodeMgrClass::Get_Node_ID (node->Get_Type ()));
			NodeMgrClass::Setup_Node_Identity (*node);
		}
	}

	//
	//	Recurse into each sub-section
	//
	for (index = 0; index < Count (); index ++) {
		(*this)[index]->Initialize_Virgin_Sections ();
	}

	m_VirginSections.Delete_All ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Assign_Section_IDs
//
////////////////////////////////////////////////////////////////
void
TerrainSectionPersistListClass::Assign_Section_IDs (TerrainNodeClass *node)
{
	m_VirginSections = node->m_Sections;

	for (int index = 0; index < Count (); index ++) {
		TerrainSectionPersistClass *persist_obj = (*this)[index];
		
		//
		//	Try to match this ID up with 
		//
		bool found = false;
		for (int node_index = 0; node_index < m_VirginSections.Count () && !found; node_index ++) {
			NodeClass *sub_node = m_VirginSections[node_index];

			StringClass model_name = sub_node->Get_Model_Name ();

			//
			//	Do the types match up?
			//
			if (	sub_node->Get_Preset_ID () == persist_obj->Get_Def_ID () &&
					::lstrcmpi (model_name, persist_obj->Get_Name ()) == 0)
			{				
				//
				//	Do the positions match up?
				//
				const Vector3 &pos = sub_node->Get_Position ();
				float delta = (pos - persist_obj->Get_Position ()).Length2 ();
				if (delta < 1.0F) {

					//
					//	Success! Assign the ID(s)
					//
					persist_obj->Apply (sub_node);
					m_VirginSections.Delete (node_index);
					found = true;
				}
			}
		}

		//
		//	Warn the user
		//
		if (!found) {
			
			CString message;
			message.Format (	"Unable to match static ID %d (%s) with a terrain section!\r\n",
									persist_obj->Get_Instance_ID (),
									(LPCTSTR)persist_obj->Get_Name ());

			::Output_Message (message);
		}		
	}

	return ;
}
