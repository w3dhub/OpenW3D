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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/PresetMgr.cpp                $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/22/02 4:08p                                               $*
 *                                                                                             *
 *                    $Revision:: 24                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "presetmgr.h"
#include "persist.h"
#include "persistfactory.h"
#include "preset.h"
#include "Utils.h"
#include "definition.h"
#include "editorchunkids.h"
#include "filemgr.h"
#include "filelocations.h"
#include "resource.h"
#include "presetslibform.h"
#include "nodemgr.h"
#include "nodecategories.h"
#include "definitionfactory.h"
#include "definitionfactorymgr.h"
#include "twiddler.h"
#include "LevelEdit.h"
#include "regkeys.h"
#include "assetdatabase.h"
#include "editorbuild.h"


///////////////////////////////////////////////////////////////////////
// Global singleton instance
///////////////////////////////////////////////////////////////////////
PresetMgrClass _ThePresetMgr;


///////////////////////////////////////////////////////////////////////
// Static member initialization
///////////////////////////////////////////////////////////////////////
PresetClass *				PresetMgrClass::_PresetListHead			= NULL;
DynamicVectorClass<int>	PresetMgrClass::_DirtyPresetList;
bool							PresetMgrClass::_PresetsDirty				= false;
bool							PresetMgrClass::_ImmediateCheckInMode	= false;


///////////////////////////////////////////////////////////////////////
//	Constants
///////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES					= 0x00000100,
	CHUNKID_PRESETS,
	CHUNKID_EMBEDDED_NODE_DATA,
	CHUNKID_PRESET_ID,
	CHUNKID_NODE_LIST
};


///////////////////////////////////////////////////////////////////////
//
//	PresetMgrClass
//
///////////////////////////////////////////////////////////////////////
PresetMgrClass::PresetMgrClass (void)
{	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	~PresetMgrClass
//
///////////////////////////////////////////////////////////////////////
PresetMgrClass::~PresetMgrClass (void)
{
	Free_Presets ();
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////////////////
void
PresetMgrClass::Initialize (void)
{
	_ImmediateCheckInMode = (bool)(theApp.GetProfileInt (CONFIG_KEY, IMMEDIATE_CHECKIN_VALUE, 1) == 1);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Shutdown
//
////////////////////////////////////////////////////////////////////////////
void
PresetMgrClass::Shutdown (void)
{
	theApp.WriteProfileInt (CONFIG_KEY, IMMEDIATE_CHECKIN_VALUE, (int)_ImmediateCheckInMode);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Free_Presets
//
////////////////////////////////////////////////////////////////////////////
void
PresetMgrClass::Free_Presets (void)
{
	PresetClass *curr_preset = NULL;
	PresetClass *next_preset = NULL;

	PresetClass *old_head = _PresetListHead;
	_PresetListHead = NULL;

	//
	//	Delete all the presets that are currently in our list
	//
	for (curr_preset = old_head; curr_preset != NULL; curr_preset = next_preset) {
		next_preset = curr_preset->m_NextPreset;
		delete curr_preset;
	}
	
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Add_Preset
//
////////////////////////////////////////////////////////////////////////////
void
PresetMgrClass::Add_Preset (PresetClass *preset)
{
	WWASSERT (preset->m_NextPreset == NULL);
	WWASSERT (preset->m_PrevPreset == NULL);

	//
	//	Don't link the new preset unless its not in the list.
	//
	if ((preset->m_NextPreset == NULL) && 
		 (preset->m_PrevPreset == NULL)) {
		Link_Preset (preset);
	}
	
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Remove_Preset
//
////////////////////////////////////////////////////////////////////////////
void
PresetMgrClass::Remove_Preset (PresetClass *preset)
{
	WWASSERT (preset != 0);

	//
	//	Only unlink the preset if its in the list
	//
	if ((preset->m_NextPreset != NULL) || 
		 (preset->m_PrevPreset != NULL) ||
		 (preset == _PresetListHead))
	{
		Unlink_Preset (preset);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Link_Preset
//
////////////////////////////////////////////////////////////////////////////
void
PresetMgrClass::Link_Preset (PresetClass *preset)
{
	WWASSERT (preset->m_NextPreset == 0);
	WWASSERT (preset->m_PrevPreset == 0);

	// Add this preset in front of the current head of the list
	preset->m_NextPreset = _PresetListHead;
	
	// If the list wasn't empty, link the next definiton back to this preset
	if (preset->m_NextPreset != 0) {
		preset->m_NextPreset->m_PrevPreset = preset;
	}

	// Point the head of the list at this preset now
	_PresetListHead = preset;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Unlink_Preset
//
////////////////////////////////////////////////////////////////////////////
void
PresetMgrClass::Unlink_Preset (PresetClass *preset)
{
	WWASSERT(preset != 0);

	// Handle the preset's prev pointer:
	if (preset->m_PrevPreset == 0) {

		// this preset is the head
		WWASSERT (_PresetListHead == preset);
		_PresetListHead = preset->m_NextPreset;
	
	} else {

		// link it's prev with it's next
		preset->m_PrevPreset->m_NextPreset = preset->m_NextPreset;

	}

	// Handle the preset's next pointer if its not at the end of the list:
	if (preset->m_NextPreset != 0) {		
		preset->m_NextPreset->m_PrevPreset = preset->m_PrevPreset;

	}

	// The preset is now un-linked
	preset->m_NextPreset = NULL;
	preset->m_PrevPreset = NULL;
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Chunk_ID
//
///////////////////////////////////////////////////////////////////////
uint32
PresetMgrClass::Chunk_ID (void) const
{
	return CHUNKID_PRESETMGR;
}


///////////////////////////////////////////////////////////////////////
//
//	Contains_Data
//
///////////////////////////////////////////////////////////////////////
bool
PresetMgrClass::Contains_Data (void) const
{
	return (_PresetListHead != NULL);
}


///////////////////////////////////////////////////////////////////////
//
//	Save_Embedded_Node_Data
//
///////////////////////////////////////////////////////////////////////
bool
PresetMgrClass::Save_Embedded_Node_Data (ChunkSaveClass &csave)
{
	//
	//	Loop over all the presets
	//
	for (	PresetClass *preset = _PresetListHead;
			preset != NULL;
			preset = preset->m_NextPreset)
	{
		//
		//	Does this preset have any embedded nodes?
		//
		NODE_LIST &node_list = preset->Get_Node_List ();
		if (node_list.Count () > 0) {
			
			//
			//	Save this preset's ID
			//
			csave.Begin_Chunk (CHUNKID_PRESET_ID);
				int id = preset->Get_ID ();
				csave.Write (&id, sizeof (id));
			csave.End_Chunk ();

			//
			//	Save this preset's node list
			//
			csave.Begin_Chunk (CHUNKID_NODE_LIST);
				NodeMgrClass::Save_Node_List (csave, node_list);
			csave.End_Chunk ();
		}
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Embedded_Node_Data
//
///////////////////////////////////////////////////////////////////////
bool
PresetMgrClass::Load_Embedded_Node_Data (ChunkLoadClass &cload)
{
	PresetClass *curr_preset = NULL;

	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {
			
			case CHUNKID_PRESET_ID:
			{
				//
				//	Set the current preset
				//
				int preset_id = 0;
				cload.Read (&preset_id, sizeof (preset_id));
				curr_preset = Find_Preset (preset_id);
				break;
			}
			
			//
			//	Load the node list for the current preset
			//
			case CHUNKID_NODE_LIST:				
				if (curr_preset != NULL) {
					NodeMgrClass::Load_Node_List (cload, curr_preset->Get_Node_List ());
				}
				break;
		}

		cload.Close_Chunk ();
	}

	return true;
}



///////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////
bool
PresetMgrClass::Save (ChunkSaveClass &csave)
{
	bool retval = true;

	//
	//	Write the presets to their own chunk
	//
	csave.Begin_Chunk (CHUNKID_PRESETS);
		retval &= Save_Presets (csave);
	csave.End_Chunk ();

	//
	//	Write the embedded node data to its own chunk
	//
	csave.Begin_Chunk (CHUNKID_EMBEDDED_NODE_DATA);
		retval &= Save_Embedded_Node_Data (csave);
	csave.End_Chunk ();

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Load
//
///////////////////////////////////////////////////////////////////////
bool
PresetMgrClass::Load (ChunkLoadClass &cload)
{
	bool retval = true;	
	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {
			
			//
			//	Load all the presets from this chunk
			//
			case CHUNKID_PRESETS:
				retval &= Load_Presets (cload);
				break;

			//
			//	Load any embedded node data for any presets
			// we loaded
			//
			case CHUNKID_EMBEDDED_NODE_DATA:
				retval &= Load_Embedded_Node_Data (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Save_Presets
//
///////////////////////////////////////////////////////////////////////
bool
PresetMgrClass::Save_Presets (ChunkSaveClass &csave)
{	
	for (	PresetClass *preset = _PresetListHead;
			preset != NULL;
			preset = preset->m_NextPreset)
	{

		//
		//	Save this preset to its own chunk
		//
		csave.Begin_Chunk (preset->Get_Factory ().Chunk_ID ());
		preset->Get_Factory ().Save (csave, preset);
		csave.End_Chunk ();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Presets
//
///////////////////////////////////////////////////////////////////////
bool
PresetMgrClass::Load_Presets (ChunkLoadClass &cload)
{
	bool retval = true;

	while (cload.Open_Chunk ()) {

		//
		//	Load this preset from the chunk (if possible)
		//
		PersistFactoryClass *factory = SaveLoadSystemClass::Find_Persist_Factory (cload.Cur_Chunk_ID ());
		if (factory != NULL) {
			PresetClass *preset = (PresetClass *)factory->Load (cload);
			if (preset != NULL) {
				
				if (preset->Is_Valid ()) {
					Add_Preset (preset);
				} else {
					CString message;
					message.Format ("Detected bad preset '%s', removing from library.\r\n", preset->Get_Name ());
					::Output_Message (message);
				}
			}
		}

		cload.Close_Chunk ();
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Find_Preset
//
///////////////////////////////////////////////////////////////////////
PresetClass *
PresetMgrClass::Find_Preset (uint32 id)
{
	PresetClass *retval = NULL;

	//
	//	Lookup the definition, then dig out the preset from the definition
	//
	DefinitionClass *definition = DefinitionMgrClass::Find_Definition (id, false);
	if (definition != NULL) {
		retval = (PresetClass *)definition->Get_User_Data ();
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Find_Typed_Preset
//
///////////////////////////////////////////////////////////////////////
PresetClass *
PresetMgrClass::Find_Typed_Preset (uint32 class_id, LPCTSTR name)
{
	PresetClass *retval = NULL;

	//
	//	Lookup the definition, then dig out the preset from the definition
	//
	DefinitionClass *definition = DefinitionMgrClass::Find_Typed_Definition (name, class_id, false);
	if (definition != NULL) {
		retval = (PresetClass *)definition->Get_User_Data ();
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Find_Preset
//
///////////////////////////////////////////////////////////////////////
PresetClass *
PresetMgrClass::Find_Preset (LPCTSTR name)
{
	PresetClass *retval = NULL;

	//
	//	Lookup the definition, then dig out the preset from the definition
	//
	DefinitionClass *definition = DefinitionMgrClass::Find_Named_Definition (name, false);
	if (definition != NULL) {
		retval = (PresetClass *)definition->Get_User_Data ();
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Next
//
///////////////////////////////////////////////////////////////////////
PresetClass *
PresetMgrClass::Get_Next (PresetClass *preset)
{
	return preset->m_NextPreset;
}


///////////////////////////////////////////////////////////////////////
//
//	Is_One_Of
//
///////////////////////////////////////////////////////////////////////
bool
PresetMgrClass::Is_One_Of
(
	uint32			id_to_find,
	ID_TYPE			type,
	bool				include_twiddlers,
	PresetClass *	preset
)
{
	bool retval = false;

	DefinitionClass *definition = preset->Get_Definition ();
	if (definition != NULL) {
		
		//
		//	Determine which class ID to check
		//
		uint32 class_id = definition->Get_Class_ID ();
		if (include_twiddlers && class_id == CLASSID_TWIDDLERS) {
			class_id = ((TwiddlerClass *)definition)->Get_Indirect_Class_ID ();
		}

		if (type == ID_CLASS) {
			
			//
			//	Is this the class we were looking for?
			//
			if (class_id == id_to_find) {
				retval = true;
			}

		} else if (type == ID_SUPERCLASS) {
			
			//
			//	Is this the superclass we were looking for?
			//
			if (SuperClassID_From_ClassID (class_id) == id_to_find) {
				retval = true;
			}
		}			
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_First
//
///////////////////////////////////////////////////////////////////////
PresetClass *
PresetMgrClass::Get_First (uint32 id, ID_TYPE type, bool include_twiddlers)
{
	PresetClass *req_preset = NULL;

	//
	//	Loop through all the presets until we've found the
	// first one that matches the criteria
	//
	for (	PresetClass *preset = _PresetListHead;
			preset != NULL;
			preset = preset->m_NextPreset)
	{
		if (Is_One_Of (id, type, include_twiddlers, preset)) {
			req_preset = preset;
			break;
		}
	}

	return req_preset;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Next
//
///////////////////////////////////////////////////////////////////////
PresetClass *
PresetMgrClass::Get_Next
(
	PresetClass *	current,
	uint32			id,
	ID_TYPE			type,
	bool				include_twiddlers
)
{
	PresetClass *req_preset = NULL;

	//
	//	Loop through all the presets until we've found the
	// first one that matches the criteria
	//
	while ((current = current->m_NextPreset) != NULL)
	{
		if (Is_One_Of (id, type, include_twiddlers, current)) {
			req_preset = current;
			break;
		}

		/*DefinitionClass *definition = current->Get_Definition ();
		if (definition != NULL) {
			uint32 class_id = definition->Get_Class_ID ();

			if (type == ID_CLASS) {
				
				//
				//	Is this the class we were looking for?
				//
				if (class_id == id) {
					req_preset = current;
				}

			} else if (type == ID_SUPERCLASS) {
				
				//
				//	Is this the superclass we were looking for?
				//
				if (SuperClassID_From_ClassID (class_id) == id) {
					req_preset = current;
				}

			} else if (type == ID_PARENT) {
				
				//
				//	Is this a child of the parent we were looking for?
				//
				PresetClass *parent = current->Get_Parent ();
				if (parent != NULL && parent->Get_ID () == id) {
					req_preset = current;
				}
			}			
		}*/
	}

	return req_preset;
}


///////////////////////////////////////////////////////////////////////
//
//	Validate_Version
//
///////////////////////////////////////////////////////////////////////
void
PresetMgrClass::Validate_Version (void)
{

#ifndef PUBLIC_EDITOR_VER

	if (::Get_File_Mgr ()->Is_VSS_Read_Only () == false) {
		
		if (Check_Editor_Version () == false) {
			
			//
			//	Force the database to be read-only
			//
			::Get_File_Mgr ()->Set_Read_Only_VSS (true);

			//
			//	Warn the user
			//
			Message_Box (NULL, IDS_VERSION_ERROR_MSG, IDS_VERSION_ERROR_TITLE, MB_ICONEXCLAMATION | MB_OK | MB_SETFOREGROUND | MB_SYSTEMMODAL);
		}
	}

#endif //PUBLIC_EDITOR_VER

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Check_Out_Database
//
///////////////////////////////////////////////////////////////////////
bool
PresetMgrClass::Check_Out_Database (uint32 class_id, bool *should_undo_on_err)
{
	CWaitCursor wait_cursor;

	bool retval					= false;
	(*should_undo_on_err)	= false;

	//
	//	Build a path to the preset library we are going to check in
	//
	CString path;
	::Get_File_Mgr ()->Get_Preset_Library_Path (class_id, false, path);
	
	//
	//	Do a quick check to make sure the current version of the
	// editor is allowed to update the asset database.
	//
	Validate_Version ();

	//
	//	Can't check out the database if we are in read-only mode.
	//
	FileMgrClass *file_mgr			= ::Get_File_Mgr ();
	AssetDatabaseClass &asset_db	= file_mgr->Get_Database_Interface ();
	if (file_mgr->Is_VSS_Read_Only () == false) {			
		
		//
		//	If the file does not exist, then simply return success
		//
		if (asset_db.Does_File_Exist (path) == false) {
			retval						= true;
			(*should_undo_on_err)	= false;
		} else {

			//
			//	Now, attempt to check out the file
			//
			retval = Check_Out_Database (path, should_undo_on_err);
		}
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Check_Out_Database
//
///////////////////////////////////////////////////////////////////////
bool
PresetMgrClass::Check_Out_Database (LPCTSTR path, bool *should_undo_on_err)
{
	FileMgrClass *file_mgr			= ::Get_File_Mgr ();
	AssetDatabaseClass &asset_db	= file_mgr->Get_Database_Interface ();
	bool retval							= false;

	//
	//	Determine the checkout status of the file
	//
	StringClass user_name;
	AssetDatabaseClass::FILE_STATUS status = asset_db.Get_File_Status (path, &user_name);
	if (status == AssetDatabaseClass::NOT_CHECKED_OUT) {
		
		//
		//	Check out the database
		//
		retval = asset_db.Retry_Check_Out (path, 10, 250);
		if (retval) {
			::Get_Presets_Form ()->Load_Databases ();
			NodeMgrClass::Reload_Nodes ();
			::Get_Presets_Form ()->Reload_Presets ();
			(*should_undo_on_err) = true;
		} else {
			::MessageBox (::AfxGetMainWnd ()->m_hWnd, "Unable to check out the preset library.", "VSS Error", MB_ICONERROR | MB_OK);
		}

	} else if (status == AssetDatabaseClass::CHECKED_OUT) {
		
		//
		//	Let the user know who has the library checked out.
		//
		CString message;
		CString title;
		message.Format (IDS_LIBRARY_CHECKED_OUT_TO_USER_MSG, user_name.Peek_Buffer ());
		title.LoadString (IDS_CANT_CHECK_OUT_TITLE);
		::MessageBox (::AfxGetMainWnd ()->m_hWnd, message, title, MB_ICONEXCLAMATION | MB_OK);
	} else if (status == AssetDatabaseClass::CHECKED_OUT_TO_ME) {
		retval = true;
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Check_In_Database
//
///////////////////////////////////////////////////////////////////////
bool
PresetMgrClass::Check_In_Database (LPCTSTR path)
{
	CWaitCursor wait_cursor;
	
	FileMgrClass *file_mgr			= ::Get_File_Mgr ();
	AssetDatabaseClass &asset_db	= file_mgr->Get_Database_Interface ();
	CString full_path					= file_mgr->Make_Full_Path (path);		
	bool retval							= false;


	if (Is_Database_Checked_Out (full_path)) {

		//
		//	Check in the database
		//		
		retval = asset_db.Retry_Check_In (full_path, 10, 250);
		if (retval == false) {
			CString message;
			message.Format ("Unable to check in preset library: %s.", full_path);
			::MessageBox (::AfxGetMainWnd ()->m_hWnd, message, "Database Error", MB_ICONERROR | MB_OK);
		}

	} else if (asset_db.Does_File_Exist (path) == false) {

		//
		//	If the file did not exist, then add it to the database.
		//
		retval = asset_db.Add_File (full_path);
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Is_Database_Checked_Out
//
///////////////////////////////////////////////////////////////////////
bool
PresetMgrClass::Is_Database_Checked_Out (LPCTSTR full_path)
{
	//
	//	Ask VSS if the file is checked out to us or not
	//
	FileMgrClass *file_mgr			= ::Get_File_Mgr ();
	AssetDatabaseClass &asset_db	= file_mgr->Get_Database_Interface ();
	return asset_db.Get_File_Status (full_path) == AssetDatabaseClass::CHECKED_OUT_TO_ME;
}


///////////////////////////////////////////////////////////////////////
//
//	Undo_Database_Check_Out
//
///////////////////////////////////////////////////////////////////////
bool
PresetMgrClass::Undo_Database_Check_Out (uint32 class_id)
{
	bool retval = false;
	FileMgrClass *file_mgr = ::Get_File_Mgr ();

	//
	//	Build a path to the preset library we are going to check in
	//
	CString path;
	file_mgr->Get_Preset_Library_Path (class_id, false, path);

	//
	//	We only undo the checkout if its checked out to us
	//
	if (Is_Database_Checked_Out (path)) {		

		//
		//	Undo the checkout
		//
		AssetDatabaseClass &asset_db	= file_mgr->Get_Database_Interface ();
		retval								= asset_db.Undo_Check_Out (path);
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Build_Preset_Tree
//
///////////////////////////////////////////////////////////////////////
void
PresetMgrClass::Build_Preset_Tree (uint32 class_id, PRESET_TREE &tree, bool include_twiddlers)
{
	tree.Reset ();
	
	//
	//	Find all the presets that belong to this class
	//
	PresetClass *preset = NULL;
	for (	preset = Get_First (class_id, ID_CLASS, include_twiddlers);
			preset != NULL;
			preset = Get_Next (preset, class_id, ID_CLASS, include_twiddlers))
	{
		
		//
		//	Add this preset to the list
		//
		if (	preset->Get_Parent () == NULL ||
				preset->Get_Definition ()->Get_Class_ID () == CLASSID_TWIDDLERS)
		{
			PRESET_TREE_LEAF *leaf = tree.Add_Sorted (preset, preset->Get_Name ());
			if (leaf != NULL) {

				//
				//	Recursively fill in this presets's children
				//
				Add_Children_To_Tree (preset->Get_ID (), leaf, include_twiddlers);
			}
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Add_Children_To_Tree
//
///////////////////////////////////////////////////////////////////////
void
PresetMgrClass::Add_Children_To_Tree
(
	uint32					parent_id,
	PRESET_TREE_LEAF *	leaf,
	bool						include_twiddlers
)
{
	//
	//	Lookup the parent preset
	//
	PresetClass *parent_preset = Find_Preset (parent_id);	
	if (parent_preset != NULL) {

		//
		//	Loop over all the children of this preset
		//
		int count = parent_preset->Get_Child_Preset_Count ();
		for (int index = 0; index < count; index ++) {
			PresetClass *child_preset = parent_preset->Get_Child_Preset (index);
			if (child_preset != NULL) {

				//
				//	Check to see if this meets the twiddler requirement
				//
				uint32 class_id = child_preset->Get_Definition ()->Get_Class_ID ();
				if (include_twiddlers || class_id != CLASSID_TWIDDLERS) {

					//
					//	Add this preset to the tree
					//
					PRESET_TREE_LEAF *child_leaf = NULL;
					child_leaf = leaf->Add_Child_Sorted (child_preset, child_preset->Get_Name ());
					if (child_leaf != NULL) {
						
						//
						//	Recursively fill in this presets's children
						//
						Add_Children_To_Tree (child_preset->Get_ID (), child_leaf, include_twiddlers);
					}
				}				
			}
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Build_Factory_Tree
//
///////////////////////////////////////////////////////////////////////
void
PresetMgrClass::Build_Factory_Tree (uint32 class_id, FACTORY_TREE &tree)
{
	tree.Reset ();

	if (class_id == 0) {

		//
		//	Add all the node categories to the tree
		//
		for (int index = 0; index < PRESET_CATEGORY_COUNT; index ++) {

			//
			//	Add this factory to the tree
			//
			uint32 factory_class_id = PRESET_CATEGORIES[index].clsid;
			//FACTORY_TREE_LEAF *leaf = (FACTORY_TREE_LEAF *)tree.Add (factory_class_id);
			FACTORY_TREE_LEAF *leaf = tree.Add_Sorted (factory_class_id, PRESET_CATEGORIES[index].name);

			//
			//	Find all the sub-factories
			//
			if (DefinitionFactoryMgrClass::Find_Factory (PRESET_CATEGORIES[index].clsid) == NULL) {
				for (	DefinitionFactoryClass *factory = DefinitionFactoryMgrClass::Get_First (factory_class_id);
						factory != NULL;
						factory = DefinitionFactoryMgrClass::Get_Next (factory, factory_class_id))
				{
					if (factory->Is_Displayed ()) {
						leaf->Add_Child_Sorted (factory->Get_Class_ID (), factory->Get_Name ());
					}
				}
			}
		}

	} else {

		//
		//	Find all the sub-factories
		//			
		for (	DefinitionFactoryClass *factory = DefinitionFactoryMgrClass::Get_First (class_id);
				factory != NULL;
				factory = DefinitionFactoryMgrClass::Get_Next (factory, class_id))
		{
			if (factory->Is_Displayed ()) {
				tree.Add_Sorted (factory->Get_Class_ID (), factory->Get_Name ());
			}
		}		
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Put_Presets_Back
//
///////////////////////////////////////////////////////////////////////
void
PresetMgrClass::Put_Presets_Back (PRESET_LIST &preset_list)
{
	//
	//	Simply loop over the presets and add them back to the manager
	//
	for (int index = 0; index < preset_list.Count (); index ++) {
		Add_Preset (preset_list[index]);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Remove_Non_Matching_Presets
//
///////////////////////////////////////////////////////////////////////
void
PresetMgrClass::Remove_Non_Matching_Presets
(
	uint32				class_id,
	bool					class_id_matters,
	bool					is_temp,
	PRESET_LIST &		removed_preset_list
)
{
	//
	//	Loop over all the presets in the system
	//
	PresetClass *next_preset	= NULL;
	PresetClass *preset			= NULL;
	for (	preset = Get_First (); preset != NULL; preset = next_preset) {
		next_preset = Get_Next (preset);

		//
		//	Determine what class ID this preset belongs to...
		//
		uint32 curr_class_id = 0;
		DefinitionClass *definition = preset->Get_Definition ();
		if (definition != NULL) {
			curr_class_id = definition->Get_Class_ID ();
		}

		//
		//	If this preset does not meet our requirements, then remove it from the
		// the preset meanager so it won't get saved
		//
		if (	(preset->Get_IsTemporary () != is_temp) ||
				(class_id_matters && (curr_class_id != class_id)))
		{
			Remove_Preset (preset);
			removed_preset_list.Add (preset);			
		}
	}	

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Discard_Preset_Changes
//
///////////////////////////////////////////////////////////////////////
void
PresetMgrClass::Discard_Preset_Changes (void)
{
	PresetsFormClass *presets_form = ::Get_Presets_Form ();
	if (presets_form == NULL) {
		return ;
	}

	//
	//	Build a list of all the database files
	//
	DynamicVectorClass<CString> file_list;
	presets_form->Build_DDB_File_List (file_list);

	//
	//	Get access to the vss interface
	//
	AssetDatabaseClass &asset_db = ::Get_File_Mgr ()->Get_Database_Interface ();

	//
	//	Loop over all the preset databases and undo any checkouts
	//	
	for (int index = 0; index < file_list.Count (); index ++) {
		CString &filename = file_list[index];
		
		//
		//	Do we have this file checked out?
		//
		if (asset_db.Get_File_Status (filename) == AssetDatabaseClass::CHECKED_OUT_TO_ME) {
			
			//
			//	Undo the checkout
			//
			asset_db.Undo_Check_Out (filename);
		}
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Check_In_Presets
//
///////////////////////////////////////////////////////////////////////
void
PresetMgrClass::Check_In_Presets (void)
{
	PresetsFormClass *presets_form = ::Get_Presets_Form ();
	if (presets_form == NULL) {
		return ;
	}

#ifdef PUBLIC_EDITOR_VER

	//
	//	Simply save the presets together in one file
	//
	CString path = ::Get_File_Mgr ()->Make_Full_Path (OBJECTS_DDB_PATH);
	presets_form->Save_Presets (path, 0, false, false);		

#else

	//
	//	Build a list of all the database files
	//
	DynamicVectorClass<CString> file_list;
	presets_form->Build_DDB_File_List (file_list);

	//
	//	Get access to the vss interface
	//
	AssetDatabaseClass &asset_db = ::Get_File_Mgr ()->Get_Database_Interface ();

	//
	//	Update all the dirty preset files
	//
	Add_Dirty_Preset_Files_To_VSS ();

	//
	//	Check each database file to see if we have it checked out...
	// If we do have it checked out, then go ahead and check it in.
	//	
	for (int index = 0; index < file_list.Count (); index ++) {
		CString &filename = file_list[index];
		
		//
		//	If we have this file checked out then go ahead and check it in...
		//
		asset_db.Check_In_Ex (filename, ::AfxGetMainWnd ()->m_hWnd);
	}

#endif //PUBLIC_EDITOR_VER
	
	_PresetsDirty = false;
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Add_Dirty_Preset
//
///////////////////////////////////////////////////////////////////////
void
PresetMgrClass::Add_Dirty_Preset (int preset_id)
{
	//
	//	Make sure we don't add this preset multiple times
	//
	int index = _DirtyPresetList.ID (preset_id);
	if (index < 0) {
		_DirtyPresetList.Add (preset_id);
	}

	_PresetsDirty = true;

#ifndef PUBLIC_EDITOR_VER

	//
	//	Check in the presets immediately (if necessary)
	//
	if (_ImmediateCheckInMode) {
		Check_In_Presets ();
	}

#endif //!PUBLIC_EDITOR_VER

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Add_Dirty_Preset_Files_To_VSS
//
///////////////////////////////////////////////////////////////////////
void
PresetMgrClass::Add_Dirty_Preset_Files_To_VSS (void)
{
	//
	//	Loop over all the presets in our dirty list
	//	
	for (int index = 0; index < _DirtyPresetList.Count (); index ++) {
		int preset_id = _DirtyPresetList[index];
		
		//
		//	If this preset still exists, then update its files
		//		
		PresetClass *preset = Find_Preset (preset_id);
		if (preset != NULL) {
			preset->Add_Files_To_VSS ();
		}
	}

	//
	//	Empty the dirty list
	//
	_DirtyPresetList.Delete_All ();	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Create_Preset
//
///////////////////////////////////////////////////////////////////////
PresetClass *
PresetMgrClass::Create_Preset
(
	uint32			class_id,
	const char *	name,
	bool				is_temp
)
{
	PresetClass *new_preset = NULL;

	//
	//	Lookup the factory for this definition
	//
	DefinitionFactoryClass *factory = DefinitionFactoryMgrClass::Find_Factory (class_id);
	if (factory != NULL) {
		
		//
		//	Create a new definition for the preset
		//
		DefinitionClass *definition = factory->Create ();

		//
		//	Assign the definition a new ID
		//
		if (is_temp) {		
			definition->Set_ID (::Get_Next_Temp_ID ());
		} else {
			definition->Set_ID (DefinitionMgrClass::Get_New_ID (class_id));
		}
		
		//
		//	Create the new preset
		//
		new_preset = new PresetClass;
		new_preset->Set_Definition (definition);	
		new_preset->Set_IsTemporary (is_temp);

		//
		//	Give the preset a name (if necessary)
		//
		if (name != NULL) {
			new_preset->Set_Name (name);
		}
	}

	return new_preset;
}


///////////////////////////////////////////////////////////////////////
//
//	Free_All_Embedded_Nodes
//
///////////////////////////////////////////////////////////////////////
void
PresetMgrClass::Free_All_Embedded_Nodes (void)
{
	//
	//	Simply ask each preset in the list to free its
	// embedded nodes
	//
	for (	PresetClass *preset = _PresetListHead;
			preset != NULL;
			preset = preset->m_NextPreset)
	{
		preset->Free_Node_List ();
	}

	return ;
}
