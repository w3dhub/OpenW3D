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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/buildingchildnode.cpp  $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/26/01 10:00a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "buildingnode.h"
#include "buildingchildnode.h"
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
SimplePersistFactoryClass<BuildingChildNodeClass, CHUNKID_NODE_BUILDING_CHILD> _BuildingChildNodePersistFactory;


enum
{
	CHUNKID_VARIABLES			= 0x05260946,
	CHUNKID_BASE_CLASS
};

enum
{
	VARID_REQUIRES_CROUCH	= 0x01,
	VARID_ATTACK_POINT,
};


//////////////////////////////////////////////////////////////////////////////
//
//	BuildingChildNodeClass
//
//////////////////////////////////////////////////////////////////////////////
BuildingChildNodeClass::BuildingChildNodeClass (PresetClass *preset)
	:	PhysObj (NULL),
		Building (NULL),
		NodeClass (preset)		
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	BuildingChildNodeClass
//
//////////////////////////////////////////////////////////////////////////////
BuildingChildNodeClass::BuildingChildNodeClass (const BuildingChildNodeClass &src)
	:	PhysObj (NULL),
		Building (NULL),
		NodeClass (NULL)
{
	*this = src;
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	~BuildingChildNodeClass
//
//////////////////////////////////////////////////////////////////////////////
BuildingChildNodeClass::~BuildingChildNodeClass (void)
{	
	Remove_From_Scene ();
	MEMBER_RELEASE (PhysObj);

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
BuildingChildNodeClass::Initialize (void)
{
	MEMBER_RELEASE (PhysObj);

	//
	//	Create the attack-point render object
	//
	RenderObjClass *render_obj = ::Create_Render_Obj ("CARMARKER");
	WWASSERT (render_obj != NULL);
	if (render_obj != NULL) {
		
		// Create the new physics object
		PhysObj = new DecorationPhysClass;
		
		//
		// Configure the physics object with information about
		// its new render object and collision data.
		//
		PhysObj->Set_Model (render_obj);
		PhysObj->Set_Transform (Matrix3D (1));
		PhysObj->Set_Collision_Group (EDITOR_COLLISION_GROUP);
		PhysObj->Peek_Model ()->Set_User_Data ((PVOID)&m_HitTestInfo, FALSE);
		PhysObj->Set_Transform (m_Transform);
		::Set_Model_Collision_Type (PhysObj->Peek_Model (), COLLISION_TYPE_0);
		
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
BuildingChildNodeClass::Get_Factory (void) const
{	
	return _BuildingChildNodePersistFactory;
}


/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
bool
BuildingChildNodeClass::Save (ChunkSaveClass &csave)
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
BuildingChildNodeClass::Load (ChunkLoadClass &cload)
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
BuildingChildNodeClass::Load_Variables (ChunkLoadClass &cload)
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


/////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////
const BuildingChildNodeClass &
BuildingChildNodeClass::operator= (const BuildingChildNodeClass &src)
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
BuildingChildNodeClass::Pre_Export (void)
{
	//
	//	Remove ourselves from the 'system' so we don't get accidentally
	// saved during the export. 
	//
	Add_Ref ();
	if (PhysObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Remove_Object (PhysObj);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Post_Export
//
//////////////////////////////////////////////////////////////////////
void
BuildingChildNodeClass::Post_Export (void)
{
	//
	//	Put ourselves back into the system
	//
	if (PhysObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Add_Dynamic_Object (PhysObj);
	}

	Release_Ref ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Delete
//
//////////////////////////////////////////////////////////////////////
void
BuildingChildNodeClass::On_Delete (void)
{
	//
	//	Remove ourselves from the cover spot
	//
	if (Building) {
		Building->Remove_Child_Node (this);
	}

	return ;
}

