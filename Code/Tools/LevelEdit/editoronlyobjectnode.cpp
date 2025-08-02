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
 *                 Project Name : leveledit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/editoronlyobjectnode.cpp     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/22/01 12:56p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"

#include "editoronlyobjectnode.h"
#include "editoronlydefinition.h"
#include "phys.h"
#include "rendobj.h"
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


//////////////////////////////////////////////////////////////////////////////
//	Persist factory
//////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<EditorOnlyObjectNodeClass, CHUNKID_EDITOR_ONLY_OBJECTS> _EditorOnlyObjNodePersistFactory;


//////////////////////////////////////////////////////////////////////////////
//
//	EditorOnlyObjectNodeClass
//
//////////////////////////////////////////////////////////////////////////////
EditorOnlyObjectNodeClass::EditorOnlyObjectNodeClass (PresetClass *preset)
	:	DisplayObj (NULL),
		NodeClass (preset)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	EditorOnlyObjectNodeClass
//
//////////////////////////////////////////////////////////////////////////////
EditorOnlyObjectNodeClass::EditorOnlyObjectNodeClass (const EditorOnlyObjectNodeClass &src)
	:	DisplayObj (NULL),
		NodeClass (NULL)
{
	*this = src;
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	~EditorOnlyObjectNodeClass
//
//////////////////////////////////////////////////////////////////////////////
EditorOnlyObjectNodeClass::~EditorOnlyObjectNodeClass (void)
{	
	Remove_From_Scene ();
	MEMBER_RELEASE (DisplayObj);
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
EditorOnlyObjectNodeClass::Initialize (void)
{
	EditorOnlyDefinitionClass *definition = static_cast<EditorOnlyDefinitionClass *> (m_Preset->Get_Definition ());
	if (definition != NULL) {
		MEMBER_RELEASE (DisplayObj);

		//
		//	Make sure we have all the assets loaded into memory that this object needs.
		//			
		m_Preset->Load_All_Assets ();
		
		//
		//	Create the display object
		//
		DisplayObj						= new DecorationPhysClass;
		CString render_obj_name		= ::Asset_Name_From_Filename (definition->Get_Model_Name ());
		RenderObjClass *render_obj	= ::Create_Render_Obj (render_obj_name);		
		if (render_obj != NULL) {
			DisplayObj->Set_Model (render_obj);
			MEMBER_RELEASE (render_obj);

			DisplayObj->Set_Collision_Group (GAME_COLLISION_GROUP);
			DisplayObj->Peek_Model ()->Set_User_Data ((PVOID)&m_HitTestInfo, FALSE);
			::Set_Model_Collision_Type (DisplayObj->Peek_Model (), 15);
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
EditorOnlyObjectNodeClass::Get_Factory (void) const
{	
	return _EditorOnlyObjNodePersistFactory;
}


/////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////
const EditorOnlyObjectNodeClass &
EditorOnlyObjectNodeClass::operator= (const EditorOnlyObjectNodeClass &src)
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
EditorOnlyObjectNodeClass::Pre_Export (void)
{
	//
	//	Remove ourselves from the 'system' so we don't get accidentally
	// saved during the export. 
	//
	Add_Ref ();
	if (DisplayObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Remove_Object (DisplayObj);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Post_Export
//
//////////////////////////////////////////////////////////////////////
void
EditorOnlyObjectNodeClass::Post_Export (void)
{
	//
	//	Put ourselves back into the system
	//
	if (DisplayObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Add_Dynamic_Object (DisplayObj);
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
EditorOnlyObjectNodeClass::Add_To_Scene (void)
{
	NodeClass::Add_To_Scene ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Remove_From_Scene
//
//////////////////////////////////////////////////////////////////////
void
EditorOnlyObjectNodeClass::Remove_From_Scene (void)
{
	NodeClass::Remove_From_Scene ();
	return ;
}

