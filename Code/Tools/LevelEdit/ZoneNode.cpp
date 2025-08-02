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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/ZoneNode.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/22/01 11:17a                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "zonenode.h"
#include "sceneeditor.h"

#include "filemgr.h"
#include "_assetmgr.h"
#include "editorassetmgr.h"
#include "w3d_file.h"
#include "cameramgr.h"
#include "collisiongroups.h"
#include "persistfactory.h"
#include "editorchunkids.h"
#include "preset.h"
#include "presetmgr.h"
#include "decophys.h"
#include "scriptzone.h"
#include "chunkio.h"
#include "nodemgr.h"


//////////////////////////////////////////////////////////////////////////////
//	Persist factory
//////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<ZoneNodeClass, CHUNKID_NODE_ZONE> _ZoneNodePersistFactory;


enum
{
	CHUNKID_VARIABLES			= 0x11110255,
	CHUNKID_BASE_CLASS
};

enum
{
	VARID_ZONE_SIZE	= 1
};


//////////////////////////////////////////////////////////////////////////////
//
//	ZoneNodeClass
//
//////////////////////////////////////////////////////////////////////////////
ZoneNodeClass::ZoneNodeClass (PresetClass *preset)
	:	m_PhysObj (NULL),
		m_CachedSize (1, 1, 1),
		ObjectNodeClass (preset)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	ZoneNodeClass
//
//////////////////////////////////////////////////////////////////////////////
ZoneNodeClass::ZoneNodeClass (const ZoneNodeClass &src)
	:	m_PhysObj (NULL),
		m_CachedSize (1, 1, 1),
		ObjectNodeClass (NULL)
{
	*this = src;
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	~ZoneNodeClass
//
//////////////////////////////////////////////////////////////////////////////
ZoneNodeClass::~ZoneNodeClass (void)
{	
	Remove_From_Scene ();
	MEMBER_RELEASE (m_PhysObj);
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
ZoneNodeClass::Initialize (void)
{
	MEMBER_RELEASE (m_PhysObj);

	if (m_GameObj != NULL) {
		m_GameObj->Set_Delete_Pending ();
		m_GameObj = NULL;
	}

	DefinitionClass *definition = m_Preset->Get_Definition ();
	if (definition != NULL) {
		
		//
		//	Create the game object
		//
		m_GameObj = (PhysicalGameObj *)definition->Create ();

		//
		//	Assign 'hit-test' information to this game object
		//
		if (m_GameObj != NULL) {

			//
			// Create the new box physics object
			//
			m_PhysObj = new Box3DPhysClass;
			m_PhysObj->Set_Collision_Group (EDITOR_COLLISION_GROUP);
			m_PhysObj->Peek_Model ()->Set_User_Data ((PVOID)&m_HitTestInfo, FALSE);
			m_PhysObj->Peek_Model ()->Set_Collision_Type (COLLISION_TYPE_6);
			m_PhysObj->Set_Transform (m_Transform);
			m_PhysObj->Get_Box ()->Set_Color (((ScriptZoneGameObjDef *)definition)->Get_Color ());
			m_PhysObj->Get_Box ()->Set_Dimensions (m_CachedSize);			

			//
			//	Make sure the physics object has the correct position
			//
			Set_Transform (m_Transform);

			//
			//	Update the zone object we embed
			//
			Update_Zone_Obj ();
			Get_Zone_Obj ()->Set_ID (m_ID);
		}

		Assign_Scripts ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
ZoneNodeClass::Get_Factory (void) const
{	
	return _ZoneNodePersistFactory;
}


/////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////
const ZoneNodeClass &
ZoneNodeClass::operator= (const ZoneNodeClass &src)
{
	m_CachedSize = src.m_CachedSize;
	ObjectNodeClass::operator= (src);
	return *this;
}



//////////////////////////////////////////////////////////////////////
//
//	Add_To_Scene
//
//////////////////////////////////////////////////////////////////////
void
ZoneNodeClass::Add_To_Scene (void)
{
	m_GrabHandles.Position_Around_Node (this);
	ObjectNodeClass::Add_To_Scene ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Remove_From_Scene
//
//////////////////////////////////////////////////////////////////////
void
ZoneNodeClass::Remove_From_Scene (void)
{
	m_GrabHandles.Remove_From_Scene ();
	ObjectNodeClass::Remove_From_Scene ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Vertex_Drag_Begin
//
//////////////////////////////////////////////////////////////////////
void
ZoneNodeClass::On_Vertex_Drag_Begin (int vertex_index)
{
	//
	// Store the location of the 'locked' vertex, so we can
	// use this when resizing the box
	//
	m_FirstPoint = m_PhysObj->Get_Box ()->Get_Vertex_Lock_Position (vertex_index);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Vertex_Drag
//
//////////////////////////////////////////////////////////////////////
void
ZoneNodeClass::On_Vertex_Drag
(
	int		vertex_index,
	POINT		point
)
{
	//
	//	Resize the box render object based on the new point
	//
	Box3DClass *box = m_PhysObj->Get_Box ();
	if (::GetKeyState (VK_SHIFT) < 0) {
		box->Drag_VertexZ (vertex_index, point, m_FirstPoint);
	} else {
		box->Drag_VertexXY (vertex_index, point, m_FirstPoint);
	}

	//
	// Position the grab handles around ourselves
	//
	m_GrabHandles.Position_Around_Node (this);

	//
	//	Update the zone's bounding box
	//
	m_Transform = box->Get_Transform ();
	Update_Zone_Obj ();

	//
	//	Cache the size of the box
	//
	m_CachedSize = box->Get_Dimensions ();
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Update_Zone_Obj
//
/////////////////////////////////////////////////////////////////
void
ZoneNodeClass::Update_Zone_Obj (void)
{
	ScriptZoneGameObj *zone_obj	= Get_Zone_Obj ();
	Box3DClass *box					= m_PhysObj->Get_Box ();

	//
	//	Update the zone's bounding box
	//	
	if (box != NULL && zone_obj != NULL) {
		Vector3 pos		= box->Get_Transform ().Get_Translation ();
		Vector3 size	= box->Get_Dimensions () / 2;

		OBBoxClass obbox;
		obbox.Center	= pos;
		obbox.Extent	= size;
		obbox.Basis.Set (m_Transform);
		zone_obj->Set_Bounding_Box (obbox);
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
bool
ZoneNodeClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		ObjectNodeClass::Save (csave);
	csave.End_Chunk ();		

	csave.Begin_Chunk (CHUNKID_VARIABLES);
		WRITE_MICRO_CHUNK (csave, VARID_ZONE_SIZE, m_CachedSize);
	csave.End_Chunk ();		
	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////
bool
ZoneNodeClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_BASE_CLASS:
				ObjectNodeClass::Load (cload);
				break;
			

			case CHUNKID_VARIABLES:
			{
				//
				//	Read all the variables from their micro-chunks
				//
				while (cload.Open_Micro_Chunk ()) {
					switch (cload.Cur_Micro_Chunk_ID ()) {

						READ_MICRO_CHUNK (cload, VARID_ZONE_SIZE, m_CachedSize);
					}

					cload.Close_Micro_Chunk ();
				}				
			}
			break;
		}

		cload.Close_Chunk ();
	}

	return true;
}


//////////////////////////////////////////////////////////////////////
//
//	Pre_Export
//
//////////////////////////////////////////////////////////////////////
void
ZoneNodeClass::Pre_Export (void)
{
	//
	//	Make sure the game object's size and position is up to date
	//
	Update_Zone_Obj ();

	//
	//	Remove ourselves from the 'system' so we don't get accidentally
	// saved during the export. 
	//
	Add_Ref ();
	if (m_PhysObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Remove_Object (m_PhysObj);
		m_GrabHandles.Remove_From_Scene ();
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Post_Export
//
//////////////////////////////////////////////////////////////////////
void
ZoneNodeClass::Post_Export (void)
{
	//
	//	Put ourselves back into the system
	//
	if (m_PhysObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Add_Dynamic_Object (m_PhysObj);
		m_GrabHandles.Position_Around_Node (this);
	}

	Release_Ref ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Hide
//
//////////////////////////////////////////////////////////////////////
void
ZoneNodeClass::Hide (bool hide)
{
	m_GrabHandles.Hide (hide);	
	NodeClass::Hide (hide);
	return ;
}
