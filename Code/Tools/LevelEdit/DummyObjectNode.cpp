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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/DummyObjectNode.cpp    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/01/00 2:25p                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"

#include "phys.h"
#include "rendobj.h"

#include "dummyobjectnode.h"
#include "dummyobjectdefinition.h"
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
#include "nodemgr.h"
#include "part_emt.h"
#include "modelutils.h"

//////////////////////////////////////////////////////////////////////////////
//	Local prototypes
//////////////////////////////////////////////////////////////////////////////
void Remove_Particle_Buffers (RenderObjClass *render_obj);


//////////////////////////////////////////////////////////////////////////////
//	Persist factory
//////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<DummyObjectNodeClass, CHUNKID_DUMMY_OBJECT> _DummyNodePersistFactory;


//////////////////////////////////////////////////////////////////////////////
//
//	DummyObjectNodeClass
//
//////////////////////////////////////////////////////////////////////////////
DummyObjectNodeClass::DummyObjectNodeClass (PresetClass *preset)
	:	m_DisplayObj (NULL),
		m_RealObj (NULL),
		NodeClass (preset)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	DummyObjectNodeClass
//
//////////////////////////////////////////////////////////////////////////////
DummyObjectNodeClass::DummyObjectNodeClass (const DummyObjectNodeClass &src)
	:	m_DisplayObj (NULL),
		m_RealObj (NULL),
		NodeClass (NULL)
{
	*this = src;
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	~DummyObjectNodeClass
//
//////////////////////////////////////////////////////////////////////////////
DummyObjectNodeClass::~DummyObjectNodeClass (void)
{	
	Remove_From_Scene ();
	MEMBER_RELEASE (m_DisplayObj);
	MEMBER_RELEASE (m_RealObj);
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
DummyObjectNodeClass::Initialize (void)
{
	//
	// Build the phys obj that is used as the position locator in the world.
	// This is usefull for things like emitters which do not have anything
	// to 'grab'...
	//	
	if (m_DisplayObj == NULL) {
		m_DisplayObj = new DecorationPhysClass;
		m_DisplayObj->Set_Model_By_Name ("DUMMY");
		m_DisplayObj->Set_Collision_Group (EDITOR_COLLISION_GROUP);
		m_DisplayObj->Peek_Model ()->Set_User_Data ((PVOID)&m_HitTestInfo, false);
		::Set_Model_Collision_Type (m_DisplayObj->Peek_Model (), COLLISION_TYPE_0);
	}
		
	DummyObjectDefinitionClass *definition = static_cast<DummyObjectDefinitionClass *> (m_Preset->Get_Definition ());
	if (definition != NULL) {
		MEMBER_RELEASE (m_RealObj);

		//
		//	Make sure we have all the assets loaded into memory that this object needs.
		//			
		m_Preset->Load_All_Assets ();
		
		//
		//	Create the real physics object that will get exported to the game
		//
		m_RealObj						= new DecorationPhysClass;
		CString render_obj_name		= ::Asset_Name_From_Filename (definition->Get_Model_Name ());
		RenderObjClass *render_obj	= ::Create_Render_Obj (render_obj_name);		
		if (render_obj != NULL) {
			
			//
			//	Start the emitter if necessary
			//
			if (render_obj->Class_ID () == RenderObjClass::CLASSID_PARTICLEEMITTER) {
				((ParticleEmitterClass *)render_obj)->Start ();
				render_obj->Set_User_Data (m_RealObj);
			}
			
			m_RealObj->Set_Model (render_obj);			
			MEMBER_RELEASE (render_obj);
		}		

		//
		//	Update the transforms of both objects
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
DummyObjectNodeClass::Get_Factory (void) const
{	
	return _DummyNodePersistFactory;
}


/////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////
const DummyObjectNodeClass &
DummyObjectNodeClass::operator= (const DummyObjectNodeClass &src)
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
DummyObjectNodeClass::Pre_Export (void)
{
	//
	//	Remove ourselves from the 'system' so we don't get accidentally
	// saved during the export. 
	//
	Add_Ref ();
	if (m_DisplayObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Remove_Object (m_DisplayObj);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Post_Export
//
//////////////////////////////////////////////////////////////////////
void
DummyObjectNodeClass::Post_Export (void)
{
	//
	//	Put ourselves back into the system
	//
	if (m_DisplayObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Add_Dynamic_Object (m_DisplayObj);
	}
	Release_Ref ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Add_To_Scene
//
//////////////////////////////////////////////////////////////////////
void
DummyObjectNodeClass::Add_To_Scene (void)
{
	if (m_RealObj == NULL) {
		Initialize ();
	}

	if (m_RealObj != NULL && m_RealObj->Peek_Model ()->Peek_Scene () == NULL) {
		::Get_Scene_Editor ()->Add_Dynamic_Object (m_RealObj);
		::Get_Scene_Editor ()->Add_To_Dirty_Cull_List (m_RealObj);
	}

	NodeClass::Add_To_Scene ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Remove_From_Scene
//
//////////////////////////////////////////////////////////////////////
void
DummyObjectNodeClass::Remove_From_Scene (void)
{
	if (m_RealObj != NULL && m_RealObj->Peek_Model ()->Peek_Scene () != NULL) {

		//
		//	If this is a particle emitter, then be sure to remove the buffer as well
		//
		RenderObjClass *render_obj = m_RealObj->Peek_Model ();
		Remove_Particle_Buffers (render_obj);

		::Get_Scene_Editor ()->Remove_From_Dirty_Cull_List (m_RealObj);
		::Get_Scene_Editor ()->Remove_Object (m_RealObj);

		MEMBER_RELEASE (m_RealObj);
	}
	
	NodeClass::Remove_From_Scene ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Handle_Emitter_Transform
//
//////////////////////////////////////////////////////////////////////
void
DummyObjectNodeClass::Handle_Emitter_Transform (void)
{
	if (m_RealObj != NULL && m_RealObj->Peek_Model () != NULL) {

		//
		//	If this is a particle emitter, then be sure to move the buffer as well
		//
		RenderObjClass *render_obj = m_RealObj->Peek_Model ();
		if (render_obj->Class_ID () == RenderObjClass::CLASSID_PARTICLEEMITTER) {
			ParticleBufferClass *buffer = ((ParticleEmitterClass *)render_obj)->Peek_Buffer ();
			if (buffer != NULL) {
				buffer->Set_Transform (m_Transform);
			}
		}	
	}

	return ;
}



///////////////////////////////////////////////////////////////
//
//  Remove_Particle_Buffers
//
///////////////////////////////////////////////////////////////
void
Remove_Particle_Buffers (RenderObjClass *render_obj)
{
	//
	// Recursively walk through the objects in the scene
	//
	for (int index = 0; index < render_obj->Get_Num_Sub_Objects (); index ++) {
		RenderObjClass *sub_obj = render_obj->Get_Sub_Object (index);
		if (sub_obj != NULL) {
			Remove_Particle_Buffers (sub_obj);
		}
		MEMBER_RELEASE (sub_obj);
	}
	
	//
	// Is this the emitter we are requesting?
	//
	if ((render_obj != NULL) &&
		 (render_obj->Class_ID () == RenderObjClass::CLASSID_PARTICLEEMITTER))
	{		
		ParticleBufferClass *buffer = ((ParticleEmitterClass *)render_obj)->Peek_Buffer ();
		if (buffer != NULL && buffer->Peek_Scene () != NULL) {
			::Get_Scene_Editor ()->Remove_Render_Object (buffer);
		}
	}

	return ;
}
