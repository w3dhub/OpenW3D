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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/PathfindStartNode.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/10/00 11:07a                                             $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "pathfindstartnode.h"
#include "sceneeditor.h"
#include "collisiongroups.h"
#include "persistfactory.h"
#include "editorchunkids.h"
#include "preset.h"
#include "chunkio.h"
#include "nodemgr.h"
#include "modelutils.h"

//////////////////////////////////////////////////////////////////////////////
//	Persist factory
//////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<PathfindStartNodeClass, CHUNKID_NODE_PATHFIND_START> _PathfindStartNodePersistFactory;


enum
{
	CHUNKID_VARIABLES			= 0x11011130,
	CHUNKID_BASE_CLASS
};

enum
{
	//VARID_TILE_LOCATION	= 0x01,
};


//////////////////////////////////////////////////////////////////////////////
//
//	PathfindStartNodeClass
//
//////////////////////////////////////////////////////////////////////////////
PathfindStartNodeClass::PathfindStartNodeClass (PresetClass *preset)
	:	m_PhysObj (NULL),
		NodeClass (preset)		
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	PathfindStartNodeClass
//
//////////////////////////////////////////////////////////////////////////////
PathfindStartNodeClass::PathfindStartNodeClass (const PathfindStartNodeClass &src)
	:	m_PhysObj (NULL),
		NodeClass (NULL)
{
	*this = src;
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	~PathfindStartNodeClass
//
//////////////////////////////////////////////////////////////////////////////
PathfindStartNodeClass::~PathfindStartNodeClass (void)
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
PathfindStartNodeClass::Initialize (void)
{
	MEMBER_RELEASE (m_PhysObj);

	//
	//	Create the camera render object
	//
	RenderObjClass *render_obj = ::Create_Render_Obj ("WAYMID");
	WWASSERT (render_obj != NULL);
	if (render_obj != NULL) {
		
		// Create the new physics object
		m_PhysObj = new DecorationPhysClass;
		
		//
		// Configure the physics object with information about
		// its new render object and collision data.
		//
		m_PhysObj->Set_Model (render_obj);
		m_PhysObj->Set_Transform (Matrix3D(1));
		m_PhysObj->Set_Collision_Group (EDITOR_COLLISION_GROUP);
		m_PhysObj->Peek_Model ()->Set_User_Data ((PVOID)&m_HitTestInfo, false);
		m_PhysObj->Set_Transform (m_Transform);
		::Set_Model_Collision_Type (m_PhysObj->Peek_Model (), COLLISION_TYPE_6);
		
		// Release our hold on the render object pointer
		MEMBER_RELEASE (render_obj);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
PathfindStartNodeClass::Get_Factory (void) const
{	
	return _PathfindStartNodePersistFactory;
}


/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
bool
PathfindStartNodeClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		NodeClass::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);
	csave.End_Chunk ();
	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////
bool
PathfindStartNodeClass::Load (ChunkLoadClass &cload)
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


///////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////
bool
PathfindStartNodeClass::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		/*switch (cload.Cur_Micro_Chunk_ID ()) {
		}*/

		cload.Close_Micro_Chunk ();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	Set_Transform
//
///////////////////////////////////////////////////////////////////////
void
PathfindStartNodeClass::Set_Transform (const Matrix3D &tm)
{
	NodeClass::Set_Transform (tm);
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////
const PathfindStartNodeClass &
PathfindStartNodeClass::operator= (const PathfindStartNodeClass &src)
{
	NodeClass::operator= (src);
	return *this;
}


//////////////////////////////////////////////////////////////////////
//
//	Pre_Export
//
//////////////////////////////////////////////////////////////////////
void
PathfindStartNodeClass::Pre_Export (void)
{
	//
	//	Remove ourselves from the 'system' so we don't get accidentally
	// saved during the export. 
	//
	Add_Ref ();
	if (m_PhysObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Remove_Object (m_PhysObj);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Post_Export
//
//////////////////////////////////////////////////////////////////////
void
PathfindStartNodeClass::Post_Export (void)
{
	//
	//	Put ourselves back into the system
	//
	if (m_PhysObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Add_Dynamic_Object (m_PhysObj);
	}
	Release_Ref ();
	return ;
}

