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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/TileNode.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/09/01 10:39a                                              $*
 *                                                                                             *
 *                    $Revision:: 23                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "tilenode.h"
#include "staticphys.h"
#include "sceneeditor.h"
#include "tiledefinition.h"
#include "filemgr.h"
#include "_assetmgr.h"
#include "editorassetmgr.h"
#include "w3d_file.h"
#include "cameramgr.h"
#include "collisiongroups.h"
#include "persistfactory.h"
#include "editorchunkids.h"
#include "preset.h"


//////////////////////////////////////////////////////////////////////////////
//	Persist factory
//////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<TileNodeClass, CHUNKID_NODE_TILE> _TileNodePersistFactory;


enum
{
	CHUNKID_VARIABLES			= 0x05050253,
	CHUNKID_BASE_CLASS,
};

enum
{
	VARID_VISOBJECTID			= 0x01,
	VARID_VISSECTORID
};


//////////////////////////////////////////////////////////////////////////////
//
//	TileNodeClass
//
//////////////////////////////////////////////////////////////////////////////
TileNodeClass::TileNodeClass (PresetClass *preset)
	:	m_PhysObj (NULL),
		m_VisObjectID (0),	// (gth) init this to zero, ("always visible")
		m_VisSectorID (uint32(-1)),	// (gth) init this to -1, ("no-sector")
		NodeClass (preset)
{
	Restrict_Rotation (true);
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	TileNodeClass
//
//////////////////////////////////////////////////////////////////////////////
TileNodeClass::TileNodeClass (const TileNodeClass &src)
	:	m_PhysObj (NULL),
		m_VisObjectID (0),
		m_VisSectorID (uint32(-1)),
		NodeClass (NULL)
{
	Restrict_Rotation (true);
	(*this) = src;	
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	~TileNodeClass
//
//////////////////////////////////////////////////////////////////////////////
TileNodeClass::~TileNodeClass (void)
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
TileNodeClass::Initialize (void)
{
	//
	//	Update the cull-link index
	//
	Update_Cached_Cull_Link ();

	MEMBER_RELEASE (m_PhysObj);

	TileDefinitionClass *definition = static_cast<TileDefinitionClass *> (m_Preset->Get_Definition ());
	if (definition != NULL) {

		//
		//	Make sure all assets are loaded into memory before this tile is created...
		//
		m_Preset->Load_All_Assets ();
		
		//
		//	Lookup the physics definition this tile definition was configured with
		//
		int def_id = definition->Get_Phys_Def_ID ();
		DefinitionClass *phys_def = DefinitionMgrClass::Find_Definition (def_id, false);
		if (phys_def != NULL) {

			//
			//	Create an instance of the physics object from its definition
			//
			PhysClass *phys_obj	= (PhysClass *)phys_def->Create ();
			ASSERT (phys_obj != NULL);
			if (phys_obj != NULL && phys_obj->Peek_Model () != NULL) {
				m_PhysObj = phys_obj->As_StaticPhysClass ();
				ASSERT (m_PhysObj != NULL);
				ASSERT (m_PhysObj->Peek_Model() != NULL);

				//
				//	Configure the physics object
				//
				if (m_PhysObj != NULL) {
					m_PhysObj->Peek_Model ()->Set_User_Data ((PVOID)&m_HitTestInfo, false);
					m_PhysObj->Set_Transform (m_Transform);
					m_PhysObj->Set_Collision_Group (GAME_COLLISION_GROUP);
					m_PhysObj->Set_ID (Get_ID ());
					m_PhysObj->Set_Vis_Object_ID (m_VisObjectID);
					m_PhysObj->Set_Vis_Sector_ID (m_VisSectorID);
				}
			} else {
				MEMBER_RELEASE (phys_obj);
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
TileNodeClass::Get_Factory (void) const
{	
	return _TileNodePersistFactory;
}


/////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////
const TileNodeClass &
TileNodeClass::operator= (const TileNodeClass &src)
{
	NodeClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
bool
TileNodeClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		NodeClass::Save (csave);
	csave.End_Chunk ();	

	csave.Begin_Chunk (CHUNKID_VARIABLES);

		//
		//	Save the tile's vis-id to the chunk
		//	
		if (m_PhysObj != NULL) {
			uint32 vis_id = ((StaticPhysClass *)m_PhysObj)->Get_Vis_Object_ID ();
			WRITE_MICRO_CHUNK (csave, VARID_VISOBJECTID, vis_id);
			vis_id = ((StaticPhysClass *)m_PhysObj)->Get_Vis_Sector_ID ();
			WRITE_MICRO_CHUNK (csave, VARID_VISSECTORID, vis_id);
		}			

	csave.End_Chunk ();
	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////
bool
TileNodeClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_BASE_CLASS:
				NodeClass::Load (cload);
				break;
			
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
TileNodeClass::Load_Variables (ChunkLoadClass &cload)
{	
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			READ_MICRO_CHUNK (cload, VARID_VISOBJECTID,	m_VisObjectID);
			READ_MICRO_CHUNK (cload, VARID_VISSECTORID,	m_VisSectorID);			
		}

		cload.Close_Micro_Chunk ();
	}

	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Update_Cached_Vis_IDs
//
/////////////////////////////////////////////////////////////////
void
TileNodeClass::Update_Cached_Vis_IDs (void)
{
	if (m_PhysObj != NULL) {
		m_VisObjectID = m_PhysObj->Get_Vis_Object_ID ();
		m_VisSectorID = m_PhysObj->Get_Vis_Object_ID ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Pre_Export
//
//////////////////////////////////////////////////////////////////////
void
TileNodeClass::Pre_Export (void)
{
	//
	//	Change our collision group the collision group that the
	// game is expecting
	//
	if (m_PhysObj != NULL) {
		m_PhysObj->Set_Collision_Group (15);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Post_Export
//
//////////////////////////////////////////////////////////////////////
void
TileNodeClass::Post_Export (void)
{
	//
	//	Restore our collision group
	//
	if (m_PhysObj != NULL) {
		m_PhysObj->Set_Collision_Group (GAME_COLLISION_GROUP);
	}

	return ;
}
