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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/Node.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/18/02 11:54a                                              $*
 *                                                                                             *
 *                    $Revision:: 49                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "Node.h"
#include "phys.h"
#include "sceneeditor.h"
#include "Utils.h"
#include "selectionbox.h"
#include "mover.h"
#include "chunkio.h"
#include "preset.h"
#include "wwstring.h"
#include "presetmgr.h"
#include "vispointgenerator.h"
#include "mesh.h"
#include "preset.h"
#include "staticphys.h"
#include "NodeInfoPage.h"
#include "EditorPropSheet.h"
#include "LevelEditView.h"
#include "NodeMgr.h"
#include "PositionPage.h"


///////////////////////////////////////////////////////////////////////
//	Constants
///////////////////////////////////////////////////////////////////////
const Vector3 SEL_COLOR_NORMAL	= Vector3 (0.8F, 0.8F, 0.8F);
const Vector3 SEL_COLOR_LOCKED	= Vector3 (0.0F, 0.0F, 0.8F);
const Vector3 SEL_COLOR_INVALID	= Vector3 (1.0F, 0.0F, 0.0F);

enum
{
	CHUNKID_VARIABLES			= 0x00000100,
	CHUNKID_COMMENTS
};

enum
{
	VARID_NAME			= 0x01,
	VARID_IS_LOCKED,
	VARID_PRESETID,
	VARID_ID,
	VARID_ROT_RESTRICT,
	VARID_THISPTR,
	VARID_TRANSFORM,
	VARID_VISID,
	VARID_COMMENTS,
	VARID_VISSECTORID,
	VARID_CULLLINK,
	VARID_NEEDS_SAVE,
	VARID_CONTAINER_ID,
	VARID_IS_PROXIED
};


//////////////////////////////////////////////////////////////////////
//
//	NodeClass
//
//////////////////////////////////////////////////////////////////////
NodeClass::NodeClass (PresetClass *preset)
	:	m_Preset (NULL),
		m_PresetID (0),
		m_ID (0),
		m_SelectionBox (NULL),
		m_IsLocked (false),
		m_RotationRestricted (false),
		m_Orientation (true),
		m_IsInScene (false),
		m_SelColor (SEL_COLOR_NORMAL),
		m_NextNode (NULL),
		m_PrevNode (NULL),
		m_HitTestInfo (this),
		m_NeedsSave (true),
		m_IsProxied (false),
		m_Transform (1),
		m_CullLink (uint32(-1)),
		m_ContainerPresetID (0)
{
	Set_Preset (preset);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	NodeClass
//
//////////////////////////////////////////////////////////////////////
NodeClass::NodeClass (const NodeClass &src)
	:	m_Preset (NULL),
		m_PresetID (0),
		m_ID (0),
		m_SelectionBox (NULL),
		m_IsLocked (false),
		m_RotationRestricted (false),
		m_Orientation (true),
		m_IsInScene (false),
		m_SelColor (SEL_COLOR_NORMAL),
		m_NextNode (NULL),
		m_PrevNode (NULL),
		m_HitTestInfo (this),
		m_NeedsSave (true),
		m_IsProxied (false),
		m_Transform (1),
		m_CullLink (uint32(-1)),
		m_ContainerPresetID (0)
{
	*this = src;
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~NodeClass
//
//////////////////////////////////////////////////////////////////////
NodeClass::~NodeClass (void)
{
	SAFE_DELETE (m_SelectionBox);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Add_To_Scene
//
//////////////////////////////////////////////////////////////////////
void
NodeClass::Add_To_Scene (void)
{
	PhysClass *phys_obj = Peek_Physics_Obj ();
	if (phys_obj != NULL && phys_obj->Get_Culling_System () == NULL) {
		
		//
		//	Add the object to either the static or dynamic culling system
		//
		SceneEditorClass *scene = ::Get_Scene_Editor ();
		if (Is_Static ()) {
			scene->Add_Static_Object ((StaticPhysClass *)phys_obj, Get_Cull_Link ());
		} else {
			scene->Add_Dynamic_Object (phys_obj);
		}
	}
	
	m_IsInScene = true;
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Remove_From_Scene
//
//////////////////////////////////////////////////////////////////////
void
NodeClass::Remove_From_Scene (void)
{
	PhysClass *phys_obj = Peek_Physics_Obj ();
	if (phys_obj != NULL && phys_obj->Get_Culling_System () != NULL) {

		//
		//	Record the cull-link index (if necessary)
		//
		Update_Cached_Cull_Link ();
		
		//
		//	Remove this object from either the static or dynamic culling system
		//
		SceneEditorClass *scene = ::Get_Scene_Editor ();
		if (Is_Static ()) {
			scene->Remove_Object ((StaticPhysClass *)phys_obj);
		} else {
			scene->Remove_Object (phys_obj);
		}
	}
	
	m_IsInScene = false;
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Update_Cached_Cull_Link
//
//////////////////////////////////////////////////////////////////////
void
NodeClass::Update_Cached_Cull_Link (void)
{
	//
	//	We can only do this if the physics object is currently in the scene
	//
	PhysClass *phys_obj = Peek_Physics_Obj ();
	if (phys_obj != NULL && phys_obj->Get_Culling_System () != NULL) {

		//
		//	Make sure this is a static physics object
		//
		StaticPhysClass *static_phys_obj = phys_obj->As_StaticPhysClass ();
		if (static_phys_obj != NULL) {

			//
			//	Dig the cull link out from the culling system structure
			//
			AABTreeLinkClass *link = (AABTreeLinkClass *)static_phys_obj->Get_Cull_Link ();
			if (link != NULL) {
				m_CullLink = link->Node->Index;
			}
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Peek_Render_Obj
//
//////////////////////////////////////////////////////////////////////
RenderObjClass *
NodeClass::Peek_Render_Obj (void) const
{
	RenderObjClass *render_obj	= NULL;
	PhysClass *phys_obj			= Peek_Physics_Obj ();
	if (phys_obj != NULL) {		
		render_obj = phys_obj->Peek_Model ();
	}
	
	return render_obj;
}


//////////////////////////////////////////////////////////////////////
//
//	Hide
//
//////////////////////////////////////////////////////////////////////
void
NodeClass::Hide (bool hide)
{
	RenderObjClass *render_obj	= Peek_Render_Obj ();
	if (render_obj != NULL) {
		render_obj->Set_Hidden (hide);
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Is_Hidden
//
//////////////////////////////////////////////////////////////////////
bool
NodeClass::Is_Hidden (void) const
{
	bool is_hidden = false;

	RenderObjClass *render_obj	= Peek_Render_Obj ();
	if (render_obj != NULL) {
		is_hidden = (render_obj->Is_Not_Hidden_At_All () == false);
	}
	
	return is_hidden;
}


//////////////////////////////////////////////////////////////////////
//
//	Show_Selection_Box
//
//////////////////////////////////////////////////////////////////////
void
NodeClass::Show_Selection_Box (bool show)
{
	if (show) {

		//
		//	Create the selection box if necessary
		//
		if (m_SelectionBox == NULL) {
			m_SelectionBox = new SelectionBoxClass;
		}
		Update_Selection_Color ();
		m_SelectionBox->Display_Around_Node (*this);

	} else {

		//
		//	Hide and destroy the selection box
		//
		if (m_SelectionBox != NULL) {
			m_SelectionBox->Remove_From_Scene ();
			SAFE_DELETE (m_SelectionBox);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Update_Selection_Color
//
/////////////////////////////////////////////////////////////////
void
NodeClass::Update_Selection_Color (void)
{
	m_SelColor = m_IsLocked ? SEL_COLOR_LOCKED : SEL_COLOR_NORMAL;

	//
	// Only perform the bounding box check if we have a valid phys obj pointer
	//
	PhysClass *collision_obj = Peek_Collision_Obj ();
	if (collision_obj != NULL) {
		
		AABoxClass aabox;
		OBBoxClass obbox;
		bool is_aabox = ::Get_Collision_Box (collision_obj->Peek_Model (), aabox, obbox);
		if (is_aabox) {
			Vector3 move (0, 0, 0);

			//
			//	See if the AABox intersects anything
			//
			collision_obj->Inc_Ignore_Counter();
			CastResultStruct res;
			PhysAABoxCollisionTestClass boxtest (aabox, move, &res, GAME_COLLISION_GROUP);
			PhysicsSceneClass::Get_Instance()->Cast_AABox (boxtest);
			collision_obj->Dec_Ignore_Counter();
			
			if (boxtest.Result->StartBad) {
				m_SelColor = SEL_COLOR_INVALID;
			}

		} else {
			Vector3 move (0, 0, 0);

			//
			//	See if the OBBox intersects anything
			//
			collision_obj->Inc_Ignore_Counter();
			CastResultStruct res;
			PhysOBBoxCollisionTestClass boxtest (obbox, move, &res, GAME_COLLISION_GROUP);
			PhysicsSceneClass::Get_Instance()->Cast_OBBox (boxtest);
			collision_obj->Dec_Ignore_Counter();
			
			if (boxtest.Result->StartBad) {
				m_SelColor = SEL_COLOR_INVALID;
			}
		}
	}

	//
	//	Update the selection box if it exists
	//
	if (m_SelectionBox != NULL) {
		m_SelectionBox->Set_Color (m_SelColor);
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Update_Selection_Box
//
/////////////////////////////////////////////////////////////////
void
NodeClass::Update_Selection_Box (void)
{
	//
	// If we have a selection box, then update its color and position
	//
	if (m_SelectionBox != NULL) {
		Update_Selection_Color ();
		m_SelectionBox->Display_Around_Node (*this);
	}

	return ;	
}


/////////////////////////////////////////////////////////////////
//
//	Lock
//
/////////////////////////////////////////////////////////////////
void
NodeClass::Lock (bool lock)
{
	m_IsLocked = lock;
	Update_Selection_Color ();
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Is_Locked
//
/////////////////////////////////////////////////////////////////
bool
NodeClass::Is_Locked (void) const
{
	return m_IsLocked;
}


/////////////////////////////////////////////////////////////////
//
//	Translate
//
/////////////////////////////////////////////////////////////////
void
NodeClass::Translate (const Vector3 &vector)
{
	MoverClass::Translate_Node (this, vector);
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Rotate
//
/////////////////////////////////////////////////////////////////
void
NodeClass::Rotate
(
	const Matrix3D &rotation_matrix,
	const Matrix3D &coord_system
)
{
	MoverClass::Rotate_Node (this, rotation_matrix, coord_system);
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	On_Rotate
//
/////////////////////////////////////////////////////////////////
void
NodeClass::On_Rotate (void)
{
	m_Orientation = ::Build_Quaternion (Get_Transform ());
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	On_Translate
//
/////////////////////////////////////////////////////////////////
void
NodeClass::On_Translate (void)
{
	m_Orientation = ::Build_Quaternion (Get_Transform ());
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	On_Transform
//
/////////////////////////////////////////////////////////////////
void
NodeClass::On_Transform (void)
{
	m_Orientation = ::Build_Quaternion (Get_Transform ());
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
bool
NodeClass::Save (ChunkSaveClass &csave)
{
	//
	//	Update the preset-id
	//	
	if (m_Preset != NULL) {
		m_PresetID = m_Preset->Get_ID ();
	}

	//
	//	Make sure we've got the latest cull-link information
	//
	Update_Cached_Cull_Link ();
	Update_Cached_Vis_IDs ();

	//
	//	Save the comments to their own chunk
	//
	WRITE_WWSTRING_CHUNK (csave, CHUNKID_COMMENTS, m_Comments);

	//
	//	Save the variables
	//
	csave.Begin_Chunk (CHUNKID_VARIABLES);
	
		WRITE_MICRO_CHUNK_STRING (csave, VARID_NAME, (LPCTSTR)m_Name);
		WRITE_MICRO_CHUNK (csave, VARID_IS_LOCKED,		m_IsLocked);
		WRITE_MICRO_CHUNK (csave, VARID_ID,					m_ID);
		WRITE_MICRO_CHUNK (csave, VARID_ROT_RESTRICT,	m_RotationRestricted);
		WRITE_MICRO_CHUNK (csave, VARID_TRANSFORM,		m_Transform);
		WRITE_MICRO_CHUNK (csave, VARID_PRESETID,			m_PresetID);
		WRITE_MICRO_CHUNK (csave, VARID_CULLLINK,			m_CullLink);
		WRITE_MICRO_CHUNK (csave, VARID_NEEDS_SAVE,		m_NeedsSave);
		WRITE_MICRO_CHUNK (csave, VARID_IS_PROXIED,		m_IsProxied);
		WRITE_MICRO_CHUNK (csave, VARID_CONTAINER_ID,	m_ContainerPresetID);

		//
		//	Save the node's vis ID (if it has one)
		//
		PhysClass *phys_obj = Peek_Physics_Obj ();
		if (phys_obj != NULL) {
			StaticPhysClass *static_phys_obj = phys_obj->As_StaticPhysClass ();
			if (static_phys_obj != NULL) {
				uint32 vis_id = static_phys_obj->Get_Vis_Object_ID ();
				WRITE_MICRO_CHUNK (csave, VARID_VISID, vis_id);
				vis_id = static_phys_obj->Get_Vis_Sector_ID();
				WRITE_MICRO_CHUNK (csave, VARID_VISSECTORID, vis_id);
			}
		}

		//
		//	For pointer remapping
		//
		NodeClass *this_ptr = this;
		WRITE_MICRO_CHUNK_PTR (csave, VARID_THISPTR, this_ptr);

	csave.End_Chunk ();
	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////
bool
NodeClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			READ_WWSTRING_CHUNK (cload, CHUNKID_COMMENTS, m_Comments);
			
			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
/////////////////////////////////////////////////////////////////
bool
NodeClass::Load_Variables (ChunkLoadClass &cload)
{
	PresetClass *old_this_ptr = NULL; 

	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			
			READ_MICRO_CHUNK (cload, VARID_IS_LOCKED,				m_IsLocked);
			READ_MICRO_CHUNK (cload, VARID_ID,						m_ID);
			READ_MICRO_CHUNK (cload, VARID_ROT_RESTRICT,			m_RotationRestricted);			
			READ_MICRO_CHUNK (cload, VARID_PRESETID,				m_PresetID);
			READ_MICRO_CHUNK_PTR (cload, VARID_THISPTR,				old_this_ptr)
			READ_MICRO_CHUNK (cload, VARID_TRANSFORM,				m_Transform)
			READ_MICRO_CHUNK_WWSTRING (cload, VARID_NAME,		m_Name);
			READ_MICRO_CHUNK_WWSTRING (cload, VARID_COMMENTS,	m_Comments);
			READ_MICRO_CHUNK (cload, VARID_CULLLINK,				m_CullLink);
			READ_MICRO_CHUNK (cload, VARID_NEEDS_SAVE,			m_NeedsSave);
			READ_MICRO_CHUNK (cload, VARID_IS_PROXIED,			m_IsProxied);
			READ_MICRO_CHUNK (cload, VARID_CONTAINER_ID,			m_ContainerPresetID);

			case VARID_VISID:
			{
				uint32 vis_id = 0;
				cload.Read (&vis_id, sizeof (vis_id));

				//
				//	Pass the vis-object-id onto the physics object (if it needs one)
				//
				PhysClass *phys_obj = Peek_Physics_Obj ();
				if (phys_obj != NULL) {
					StaticPhysClass *static_phys_obj = phys_obj->As_StaticPhysClass ();
					if (static_phys_obj != NULL) {
						static_phys_obj->Set_Vis_Object_ID (vis_id);
					}
				}
			}

			case VARID_VISSECTORID:
			{
				uint32 vis_id = 0;
				cload.Read (&vis_id, sizeof (vis_id));

				//
				//	Pass the vis-sector-id onto the physics object (if it needs one)
				//
				PhysClass *phys_obj = Peek_Physics_Obj ();
				if (phys_obj != NULL) {
					StaticPhysClass *static_phys_obj = phys_obj->As_StaticPhysClass ();
					if (static_phys_obj != NULL) {
						static_phys_obj->Set_Vis_Sector_ID (vis_id);
					}
				}
			}
			break;
		}

		cload.Close_Micro_Chunk ();
	}

	//
	//	Handle pointer remapping
	//
	WWASSERT (old_this_ptr != NULL);
	SaveLoadSystemClass::Register_Pointer (old_this_ptr, this);

	//
	//	Get the preset pointer from its ID
	//
	m_Preset = PresetMgrClass::Find_Preset (m_PresetID);
	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Has_Vis_Sectors
//
/////////////////////////////////////////////////////////////////
bool
NodeClass::Has_Vis_Sectors (RenderObjClass *render_obj)
{
	bool retval = false;

	//
	//	Start with the parent render obj if none is supplied.
	//
	if (render_obj == NULL) {
		render_obj = Peek_Render_Obj ();
	}

	if (render_obj != NULL) {

		//
		// Loop through all the render objects sub-objects
		//
		int count = render_obj->Get_Num_Sub_Objects ();
		for (int index = 0; (index < count) && !retval; index ++) {
			
			// Get a pointer to this subobject
			RenderObjClass *sub_object = render_obj->Get_Sub_Object (index);
			if (sub_object != NULL) {
				
				retval |= Has_Vis_Sectors (sub_object);
				MEMBER_RELEASE (sub_object);
			}
		}	

		//
		// Is this render object a mesh?
		//
		if ((render_obj->Class_ID () == RenderObjClass::CLASSID_MESH) &&
			 (render_obj->Get_Collision_Type () & COLLISION_TYPE_VIS))
		{
			retval = true;
		}
	}

	return retval;
}


/////////////////////////////////////////////////////////////////
//
//	Add_Vis_Points
//
/////////////////////////////////////////////////////////////////
void
NodeClass::Add_Vis_Points
(
	VisPointGeneratorClass &	generator,
	RenderObjClass *				render_obj
)
{
	//
	// Let the generator know which node its processing
	//
	generator.Set_Current_Node (this);

	//
	//	Start with the parent render obj if none is supplied.
	//
	if (render_obj == NULL) {
		render_obj = Peek_Render_Obj ();
	}

	//
	// Loop through all the render objects sub-objects
	//
	int count = render_obj->Get_Num_Sub_Objects ();
	for (int index = 0; index < count; index ++) {
		
		// Get a pointer to this subobject
		RenderObjClass *sub_object = render_obj->Get_Sub_Object (index);
		if (sub_object != NULL) {
			
			Add_Vis_Points (generator, sub_object);
			MEMBER_RELEASE (sub_object);
		}
	}	

	//
	// Is this render object a mesh?
	//
	if ((render_obj->Class_ID () == RenderObjClass::CLASSID_MESH) &&
		 (render_obj->Get_Collision_Type () & COLLISION_TYPE_VIS))
	{
		//
		//	Pass this mesh off to the generator
		//
		MeshClass *mesh = static_cast<MeshClass *> (render_obj);
		generator.Submit_Mesh (*mesh);
	}	

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Reload
//
/////////////////////////////////////////////////////////////////
void
NodeClass::Reload (void)
{
	CLevelEditView::Allow_Repaint (false);

	bool in_scene = m_IsInScene;
	if (in_scene) {
		Remove_From_Scene ();
	}

	//
	//	Reinitialize the node from its preset
	//
	//m_Preset = PresetMgrClass::Find_Preset (m_PresetID);
	Initialize ();

	if (in_scene) {
		Add_To_Scene ();
	}

	CLevelEditView::Allow_Repaint (true);
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Show_Settings_Dialog
//
/////////////////////////////////////////////////////////////////
bool
NodeClass::Show_Settings_Dialog (void)
{	
	NodeInfoPageClass info_tab (this);
	PositionPageClass	pos_tab (this);

	EditorPropSheetClass prop_sheet;
	prop_sheet.Add_Page (&info_tab);
	prop_sheet.Add_Page (&pos_tab);

	// Show the property sheet
	UINT ret_code = prop_sheet.DoModal ();
	
	// Return true if the user clicked OK
	return (ret_code == IDOK);
}


/////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////
const NodeClass &
NodeClass::operator= (const NodeClass &src)
{
	//
	//	Copy the data that makes sense to copy.
	//
	m_Comments				= src.m_Comments;
	m_PresetID				= src.m_PresetID;
	m_IsLocked				= src.m_IsLocked;
	m_RotationRestricted	= src.m_RotationRestricted;
	m_Preset					= src.m_Preset;
	m_Orientation			= src.m_Orientation;
	m_SelColor				= src.m_SelColor;
	m_Transform				= src.m_Transform;

	//
	//	Make sure the node has a unique id and name
	//
	NodeMgrClass::Setup_Node_Identity (*this);

	//
	//	Give the node a chance to create any internal
	// data (game objects, phys objects, etc) it needs.
	//
	Initialize ();
	return *this;
}


/////////////////////////////////////////////////////////////////
//
//	Set_ID
//
/////////////////////////////////////////////////////////////////
void
NodeClass::Set_ID (uint32 id)
{
	m_ID = id;
	NodeMgrClass::Setup_Node_Identity (*this);
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Get_Model_Name
//
/////////////////////////////////////////////////////////////////
LPCTSTR
NodeClass::Get_Model_Name (void) const
{
	LPCTSTR name = NULL;
	RenderObjClass *render_obj = Peek_Render_Obj ();
	if (render_obj != NULL) {
		name = render_obj->Get_Name ();
	}

	return name;
}
