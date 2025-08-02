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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/ObjectNode.cpp     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/22/01 11:17a                                              $*
 *                                                                                             *
 *                    $Revision:: 20                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "objectnode.h"
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
#include "editscript.h"
#include "nodeinfopage.h"
#include "nodescriptsproppage.h"
#include "editorpropsheet.h"
#include "positionpage.h"
#include "modelutils.h"


//////////////////////////////////////////////////////////////////////////////
//	Persist factory
//////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<ObjectNodeClass, CHUNKID_NODE_OBJECTS> _ObjectNodePersistFactory;



enum
{
	CHUNKID_VARIABLES			= 0x10271102,
	CHUNKID_BASE_CLASS,
	CHUNKID_SCRIPT
};


//////////////////////////////////////////////////////////////////////////////
//
//	ObjectNodeClass
//
//////////////////////////////////////////////////////////////////////////////
ObjectNodeClass::ObjectNodeClass (PresetClass *preset)
	:	m_GameObj (NULL),
		NodeClass (preset)
{
	
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	ObjectNodeClass
//
//////////////////////////////////////////////////////////////////////////////
ObjectNodeClass::ObjectNodeClass (const ObjectNodeClass &src)
	:	m_GameObj (NULL),
		NodeClass (NULL)
{
	*this = src;
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	~ObjectNodeClass
//
//////////////////////////////////////////////////////////////////////////////
ObjectNodeClass::~ObjectNodeClass (void)
{	
	Remove_From_Scene ();
	Destroy_Game_Obj ();
	Free_Scripts ();
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
ObjectNodeClass::Initialize (void)
{	
	Destroy_Game_Obj ();

	if (m_Preset != NULL) {
		m_Preset->Load_All_Assets ();
		Create_Game_Obj ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
ObjectNodeClass::Get_Factory (void) const
{	
	return _ObjectNodePersistFactory;
}


////////////////////////////////////////////////////////////////
//
//	Show_Settings_Dialog
//
////////////////////////////////////////////////////////////////
bool
ObjectNodeClass::Show_Settings_Dialog (void)
{	
	NodeInfoPageClass		info_tab (this);
	PositionPageClass		pos_tab (this);
	NodeScriptsPropPage	scripts_tab (&m_Scripts);

	EditorPropSheetClass prop_sheet;
	prop_sheet.Add_Page (&info_tab);
	prop_sheet.Add_Page (&pos_tab);
	prop_sheet.Add_Page (&scripts_tab);

	// Show the property sheet
	UINT ret_code = prop_sheet.DoModal ();
	if (ret_code == IDOK) {
		
		//
		//	If the scripts changed, then we need to
		// reload the object and assign it the new
		// set of scripts.
		//
		Reload ();
	}
	
	// Return true if the user clicked OK
	return (ret_code == IDOK);
}


/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
bool
ObjectNodeClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		NodeClass::Save (csave);
	csave.End_Chunk ();		

	//
	//	Save the list of scripts
	//
	for (int index = 0; index < m_Scripts.Count (); index ++) {
		EditScriptClass *script = m_Scripts[index];
		
		//
		//	Have this script save itself
		//
		csave.Begin_Chunk (CHUNKID_SCRIPT);
		script->Save (csave);
		csave.End_Chunk ();
	}

	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////
bool
ObjectNodeClass::Load (ChunkLoadClass &cload)
{
	Free_Scripts ();

	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_BASE_CLASS:
				NodeClass::Load (cload);
				break;
			
			case CHUNKID_SCRIPT:
			{
				EditScriptClass *script = new EditScriptClass;
				if (script->Load (cload)) {
					m_Scripts.Add (script);
				}
			}
			break;
		}

		cload.Close_Chunk ();
	}

	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Free_Scripts
//
/////////////////////////////////////////////////////////////////
void
ObjectNodeClass::Free_Scripts (void)
{
	for (int index = 0; index < m_Scripts.Count (); index ++) {
		EditScriptClass *script = m_Scripts[index];
		SAFE_DELETE (script);
	}
	
	m_Scripts.Delete_All ();
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Assign_Scripts
//
/////////////////////////////////////////////////////////////////
void
ObjectNodeClass::Assign_Scripts (void)
{
	if (m_GameObj != NULL) {
		for (int index = 0; index < m_Scripts.Count (); index ++) {
			EditScriptClass *script	= m_Scripts[index];
			
			//
			//	Create the game script and pass it onto the game obj
			//
			ScriptClass *game_script	= script->Create_Script ();
			if (game_script != NULL) {
				m_GameObj->Add_Observer (game_script);
			}
		}
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Copy_Scripts
//
/////////////////////////////////////////////////////////////////
void
ObjectNodeClass::Copy_Scripts (const ObjectNodeClass &src)
{
	Free_Scripts ();

	//
	//	Loop over all the scripts in the src object and copy them.
	//
	for (int index = 0; index < src.m_Scripts.Count (); index ++) {
		EditScriptClass *script = src.m_Scripts[index];
		if (script != NULL) {

			//
			//	Make ourselves a copy of the script
			//
			EditScriptClass *our_copy = new EditScriptClass (*script);
			m_Scripts.Add (our_copy);
		}
	}

	Assign_Scripts ();
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////
const ObjectNodeClass &
ObjectNodeClass::operator= (const ObjectNodeClass &src)
{
	//
	//	Copy the script list from the source object
	//
	Copy_Scripts (src);

	NodeClass::operator= (src);
	return *this;
}


//////////////////////////////////////////////////////////////////////
//
//	Add_To_Scene
//
//////////////////////////////////////////////////////////////////////
void
ObjectNodeClass::Add_To_Scene (void)
{
	Create_Game_Obj ();
	NodeClass::Add_To_Scene ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Remove_From_Scene
//
//////////////////////////////////////////////////////////////////////
void
ObjectNodeClass::Remove_From_Scene (void)
{
	Destroy_Game_Obj ();
	NodeClass::Remove_From_Scene ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Create_Game_Obj
//
//////////////////////////////////////////////////////////////////////
void
ObjectNodeClass::Create_Game_Obj (void)
{
	if (m_GameObj == NULL) {
		DefinitionClass *definition = m_Preset->Get_Definition ();
		if (definition != NULL) {
			
			//
			//	Create the game object
			//			
			m_GameObj = (ScriptableGameObj *)::Instance_Definition (definition);

			//
			//	Assign 'hit-test' information to this game object
			//
			if (m_GameObj != NULL) {
				PhysClass *phys_obj			= Peek_Physics_Obj ();			
				RenderObjClass *render_obj	= phys_obj->Peek_Model ();
				if (render_obj != NULL) {
					render_obj->Set_User_Data ((PVOID)&m_HitTestInfo, FALSE);
					::Set_Model_Collision_Type (render_obj, COLLISION_TYPE_6);
				}

				//
				//	Make sure the physics object has the correct position
				//
				Set_Transform (m_Transform);
				m_GameObj->Set_ID (m_ID);

				PhysicalGameObj *game_obj = Peek_Game_Obj ();
				if (game_obj != NULL) {
					game_obj->Startup ();
				}

				//
				//	Make sure we don't put the object into the scene prematurely
				//
				if (phys_obj->Get_Culling_System () != NULL) {
					::Get_Scene_Editor ()->Remove_Object (phys_obj);
				}
			}

			Assign_Scripts ();
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Destroy_Game_Obj
//
//////////////////////////////////////////////////////////////////////
void
ObjectNodeClass::Destroy_Game_Obj (void)
{
	if (m_GameObj != NULL) {
		m_GameObj->Set_Delete_Pending ();
		m_GameObj = NULL;
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Set_ID
//
/////////////////////////////////////////////////////////////////
void
ObjectNodeClass::Set_ID (uint32 id)
{
	NodeClass::Set_ID (id);
	if (m_GameObj != NULL) {
		m_GameObj->Set_ID (m_ID);
	}

	return ;
}
