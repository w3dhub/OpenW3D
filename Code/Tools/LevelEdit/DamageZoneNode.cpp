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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/DamageZoneNode.cpp     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/22/01 11:17a                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "damagezonenode.h"
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
#include "damagezone.h"
#include "chunkio.h"
#include "nodemgr.h"


//////////////////////////////////////////////////////////////////////////////
//	Persist factory
//////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<DamageZoneNodeClass, CHUNKID_NODE_DAMAGE_ZONE> _DamageZoneNodePersistFactory;


//////////////////////////////////////////////////////////////////////////////
//	Constants
//////////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES			= 0x06291059,
	CHUNKID_BASE_CLASS
};

enum
{
	VARID_ZONE_SIZE	= 1
};


//////////////////////////////////////////////////////////////////////////////
//
//	DamageZoneNodeClass
//
//////////////////////////////////////////////////////////////////////////////
DamageZoneNodeClass::DamageZoneNodeClass (PresetClass *preset)
	:	m_PhysObj (NULL),
		m_GameObj (NULL),
		m_CachedSize (1, 1, 1),
		NodeClass (preset)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	DamageZoneNodeClass
//
//////////////////////////////////////////////////////////////////////////////
DamageZoneNodeClass::DamageZoneNodeClass (const DamageZoneNodeClass &src)
	:	m_PhysObj (NULL),
		m_GameObj (NULL),
		m_CachedSize (1, 1, 1),
		NodeClass (NULL)
{
	*this = src;
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	~DamageZoneNodeClass
//
//////////////////////////////////////////////////////////////////////////////
DamageZoneNodeClass::~DamageZoneNodeClass (void)
{
	Destroy_Game_Obj ();
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
DamageZoneNodeClass::Initialize (void)
{
	MEMBER_RELEASE (m_PhysObj);
	Destroy_Game_Obj ();

	DefinitionClass *definition = m_Preset->Get_Definition ();
	if (definition != NULL) {

		//
		// Create the new box physics object
		//
		m_PhysObj = new Box3DPhysClass;
		m_PhysObj->Set_Collision_Group (EDITOR_COLLISION_GROUP);
		m_PhysObj->Peek_Model ()->Set_User_Data ((PVOID)&m_HitTestInfo, false);
		m_PhysObj->Peek_Model ()->Set_Collision_Type (COLLISION_TYPE_0);
		m_PhysObj->Set_Transform (m_Transform);
		m_PhysObj->Get_Box ()->Set_Color (((DamageZoneGameObjDef *)definition)->Get_Color ());
		m_PhysObj->Get_Box ()->Set_Dimensions (m_CachedSize);
		
		//
		//	Create the game object
		//
		Create_Game_Obj ();

		//
		//	Make sure the physics object has the correct position
		//
		Set_Transform (m_Transform);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
DamageZoneNodeClass::Get_Factory (void) const
{	
	return _DamageZoneNodePersistFactory;
}


/////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////
const DamageZoneNodeClass &
DamageZoneNodeClass::operator= (const DamageZoneNodeClass &src)
{
	m_CachedSize = src.m_CachedSize;
	NodeClass::operator= (src);
	return *this;
}



//////////////////////////////////////////////////////////////////////
//
//	Add_To_Scene
//
//////////////////////////////////////////////////////////////////////
void
DamageZoneNodeClass::Add_To_Scene (void)
{
	Create_Game_Obj ();

	m_GrabHandles.Position_Around_Node (this);
	NodeClass::Add_To_Scene ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Remove_From_Scene
//
//////////////////////////////////////////////////////////////////////
void
DamageZoneNodeClass::Remove_From_Scene (void)
{
	Destroy_Game_Obj ();

	m_GrabHandles.Remove_From_Scene ();
	NodeClass::Remove_From_Scene ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Vertex_Drag_Begin
//
//////////////////////////////////////////////////////////////////////
void
DamageZoneNodeClass::On_Vertex_Drag_Begin (int vertex_index)
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
DamageZoneNodeClass::On_Vertex_Drag
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
	Update_Game_Obj ();

	//
	//	Cache the size of the box
	//
	m_CachedSize = box->Get_Dimensions ();
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Update_Game_Obj
//
/////////////////////////////////////////////////////////////////
void
DamageZoneNodeClass::Update_Game_Obj (void)
{
	Box3DClass *box = m_PhysObj->Get_Box ();

	//
	//	Update the zone's bounding box
	//	
	if (box != NULL && m_GameObj != NULL) {
		Vector3 pos		= box->Get_Transform ().Get_Translation ();
		Vector3 size	= box->Get_Dimensions () / 2;

		OBBoxClass obbox;
		obbox.Center	= pos;
		obbox.Extent	= size;
		obbox.Basis.Set (m_Transform);
		m_GameObj->Set_Bounding_Box (obbox);
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
bool
DamageZoneNodeClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		NodeClass::Save (csave);
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
DamageZoneNodeClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_BASE_CLASS:
				NodeClass::Load (cload);
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
DamageZoneNodeClass::Pre_Export (void)
{
	//
	//	Make sure the game object's size and position is up to date
	//
	Update_Game_Obj ();

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
DamageZoneNodeClass::Post_Export (void)
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
//	Destroy_Game_Obj
//
//////////////////////////////////////////////////////////////////////
void
DamageZoneNodeClass::Destroy_Game_Obj (void)
{
	if (m_GameObj != NULL) {
		m_GameObj->Set_Delete_Pending ();
		m_GameObj = NULL;
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Create_Game_Obj
//
//////////////////////////////////////////////////////////////////////
void
DamageZoneNodeClass::Create_Game_Obj (void)
{
	if (m_GameObj == NULL) {

		//
		//	Get a pointer to the definition that this object uses
		//
		DefinitionClass *definition = m_Preset->Get_Definition ();
		if (definition != NULL) {
			
			//
			//	Create the game object
			//
			m_GameObj = (DamageZoneGameObj *)definition->Create ();

			//
			//	Assign 'hit-test' information to this game object
			//
			if (m_GameObj != NULL) {

				//
				//	Update the zone object we embed
				//
				Update_Game_Obj ();
				m_GameObj->Set_ID (m_ID);
			}
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Hide
//
//////////////////////////////////////////////////////////////////////
void
DamageZoneNodeClass::Hide (bool hide)
{
	m_GrabHandles.Hide (hide);	
	NodeClass::Hide (hide);
	return ;
}
