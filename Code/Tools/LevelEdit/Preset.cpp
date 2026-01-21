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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/Preset.cpp                   $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/29/02 4:04p                                               $*
 *                                                                                             *
 *                    $Revision:: 49                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"

#include "preset.h"
#include "presetgeneraltab.h"
#include "presetsettingstab.h"
#include "presetpropsheet.h"
#include "chunkio.h"
#include "persistfactory.h"
#include "editorchunkids.h"
#include "definition.h"
#include "definitionmgr.h"
#include "filemgr.h"
#include "parameter.h"
#include "terrainnode.h"
#include "terraindefinition.h"
#include "tilenode.h"
#include "objectnode.h"
#include "assetmgr.h"
#include "wwstring.h"
#include "combatchunkid.h"
#include "spawnernode.h"
#include "soundnode.h"
#include "VisPointNode.h"
#include "presettransitiontab.h"
#include "vehicle.h"
#include "zonenode.h"
#include "damagezonenode.h"
#include "transitionnode.h"
#include "pathfindstartnode.h"
#include "dummyobjectnode.h"
#include "editoronlyobjectnode.h"
#include "presetmgr.h"
#include "lightnode.h"
#include "physobjedittab.h"
#include "definitionutils.h"
#include "WaypathNode.h"
#include "Utils.h"
#include "coverspotnode.h"
#include "presetdependencytab.h"
#include "presetzonetab.h"
#include "presetdialoguetab.h"
#include "buildingnode.h"
#include "nodescriptsproppage.h"
#include "nodemgr.h"
#include "sceneeditor.h"
#include "editorbuild.h"


///////////////////////////////////////////////////////////////////////
//
//	Global persist factory for presets
//
///////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<PresetClass, CHUNKID_PRESET> _PresetPersistFactory;


//////////////////////////////////////////////////////////////////////////////////
//	Constants
//////////////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES			= 0x00000100,
	CHUNKID_COMMENTS,
	XXX_CHUNKID_NODES
};

enum
{
	VARID_DEFINITIONID	= 0x01,
	VARID_ISTEMPORARY,
	VARID_COMMENTS,
	VARID_PARENTPTR,
	VARID_THISPTR,
	XXX_VARID_FILEDEPENDENCY,
	VARID_PARENT_ID,
	VARID_MANUALDEPENDENCY
};


///////////////////////////////////////////////////////////////////////
//
//	PresetClass
//
///////////////////////////////////////////////////////////////////////
PresetClass::PresetClass (void)
	:	m_Definition (NULL),
		m_DefinitionID (0),
		m_Parent (NULL),
		m_ParentID (0),
		m_NextPreset (NULL),
		m_PrevPreset (NULL),
		m_IsTemporary (false),
		m_IsValid (true)
{
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	~PresetClass
//
///////////////////////////////////////////////////////////////////////
PresetClass::~PresetClass (void)
{
	//
	//	Remove this preset from its parent (if possible or necessary)
	//
	PresetClass *parent = PresetMgrClass::Find_Preset (m_ParentID);
	if (parent != NULL) {
		parent->Remove_Child_Preset (m_DefinitionID);
	}

	//
	//	Unlink this definition from its preset (if possible or necessary)
	//
	DefinitionClass *definition = DefinitionMgrClass::Find_Definition (m_DefinitionID, false);
	if (definition != NULL && (definition->Get_User_Data () == (uintptr_t)this)) {
		definition->Set_User_Data (NULL);
	}

	Free_Node_List ();
	return ;	
}


///////////////////////////////////////////////////////////////////////
//
//	Show_Properties
//
///////////////////////////////////////////////////////////////////////
bool
PresetClass::Show_Properties (bool read_only)
{	
	PresetGeneralTabClass		general_tab (this);
	PhysObjEditTabClass			phys_model_tab;
	PresetSettingsTabClass		settings_tab (this);
	PresetDependencyTabClass	dependencies_tab (this);
	PresetTransitionTabClass	transition_tab (this);	
	PresetZoneTabClass			zone_tab (this);	
	PresetDialogueTabClass		dialogue_tab (this);
	NodeScriptsPropPage			scripts_tab;

	//
	//	Add the general page first
	//
	PresetPropSheetClass prop_sheet;
	prop_sheet.Set_Preset (this);
	prop_sheet.Add_Page (&general_tab);

	//
	// Should we add a physics-definition page?
	//
	ModelDefParameterClass *phys_def_param = Get_Phys_Def_Param ();
	if (phys_def_param != NULL) {
		phys_model_tab.Set_PhysDef_Param (phys_def_param);
		phys_model_tab.Set_Filter (phys_def_param->Get_Base_Class ());
		phys_model_tab.Set_Is_Temp (m_IsTemporary);
		phys_model_tab.Set_Definition_ID (phys_def_param->Get_Value ());		
		prop_sheet.Add_Page (&phys_model_tab);		
	}	

	//
	//	Add the settings and file dependencies page
	//
	prop_sheet.Add_Page (&settings_tab);
	prop_sheet.Add_Page (&dependencies_tab);

	//
	// Should we add a scripts page?
	//
	ScriptListParameterClass *script_list_param = Get_Script_List_Param ();
	if (script_list_param != NULL) {
		scripts_tab.Set_Script_List_Parameter (script_list_param);
		prop_sheet.Add_Page (&scripts_tab);
	}
	
	//
	//	Add a dialogue tab to the property sheet if this preset represents a human.
	//
	if (Is_Soldier_Preset ()) {
		prop_sheet.Add_Page (&dialogue_tab);
	}

	//
	//	Should we add a transition-list page?
	//
	TRANSITION_DATA_LIST *transition_list = Get_Transition_List ();
	if (transition_list != NULL && read_only == false) {
		transition_tab.Set_Transition_List (transition_list);
		prop_sheet.Add_Page (&transition_tab);
	}

	//
	//	Should we add a zone-list page?
	//
	ZONE_PARAM_LIST zone_list;
	Build_Zone_List (m_Definition, zone_list);
	if (zone_list.Count () > 0 && read_only == false) {
		zone_tab.Set_Zone_Param_List (&zone_list);
		prop_sheet.Add_Page (&zone_tab);		
	}

	//
	//	Make sure things are read-only if needs be
	//
	prop_sheet.Set_Read_Only (read_only);
	general_tab.Set_Read_Only (read_only);
	settings_tab.Set_Read_Only (read_only);
	dependencies_tab.Set_Read_Only (read_only);
	phys_model_tab.Set_Read_Only (read_only);
	dialogue_tab.Set_Read_Only (read_only);

	//
	// Show the property sheet
	//
	UINT ret_code = prop_sheet.DoModal ();
	
	// Return true if the user clicked OK
	return (ret_code == IDOK);
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//

///////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
PresetClass::Get_Factory (void) const
{
	return _PresetPersistFactory;
}


///////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////
bool
PresetClass::Save (ChunkSaveClass &csave)
{
	bool retval = true;

	//
	//	Save the comments to their own chunk
	//
	WRITE_WWSTRING_CHUNK (csave, CHUNKID_COMMENTS, m_Comments);
	
	//
	//	Save the other variables to a micro chunk
	//
	csave.Begin_Chunk (CHUNKID_VARIABLES);
		retval &= Save_Variables (csave);
	csave.End_Chunk ();

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Save_Variables
//
///////////////////////////////////////////////////////////////////////
bool
PresetClass::Save_Variables (ChunkSaveClass &csave)
{
	bool retval = true;

	//
	//	Write the ID of the definition to the chunk	
	//
	ASSERT (m_Definition != NULL);
	if (m_Definition != NULL) {
		m_DefinitionID = m_Definition->Get_ID ();
		WRITE_MICRO_CHUNK (csave, VARID_DEFINITIONID, m_DefinitionID);
	}

	WRITE_MICRO_CHUNK (csave, VARID_ISTEMPORARY, m_IsTemporary);

	//
	//	Write the file dependencies to their own micro-chunks
	//
	for (int index = 0; index < m_ManualDependencies.Count (); index ++) {
		StringClass filename = (LPCTSTR)m_ManualDependencies[index];
		WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_MANUALDEPENDENCY, filename);
	}

	//
	//	For pointer remapping
	//
	PresetClass *this_ptr = this;
	WRITE_MICRO_CHUNK_PTR (csave, VARID_THISPTR, this_ptr);
	
	if (m_Parent != NULL) {
		m_ParentID = m_Parent->Get_ID ();
		WRITE_MICRO_CHUNK (csave, VARID_PARENT_ID, m_ParentID);
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Load
//
///////////////////////////////////////////////////////////////////////
bool
PresetClass::Load (ChunkLoadClass &cload)
{
	bool retval = true;

	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ())
		{
			READ_WWSTRING_CHUNK (cload, CHUNKID_COMMENTS, m_Comments);

			case CHUNKID_VARIABLES:
				retval &= Load_Variables (cload);
				break;
		}

		cload.Close_Chunk ();
	}
	
	SaveLoadSystemClass::Register_Post_Load_Callback (this);
	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////
bool
PresetClass::Load_Variables (ChunkLoadClass &cload)
{
	m_DefinitionID = 0;
	PresetClass *old_this_ptr = NULL; 
	m_ManualDependencies.Delete_All ();

	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			
			READ_MICRO_CHUNK (cload, VARID_DEFINITIONID, m_DefinitionID)
			READ_MICRO_CHUNK (cload, VARID_ISTEMPORARY, m_IsTemporary)
			READ_MICRO_CHUNK_WWSTRING (cload, VARID_COMMENTS, m_Comments);

			READ_MICRO_CHUNK_PTR (cload, VARID_PARENTPTR, m_Parent)
			READ_MICRO_CHUNK_PTR (cload, VARID_THISPTR, old_this_ptr)

			READ_MICRO_CHUNK (cload, VARID_PARENT_ID, m_ParentID);

			case VARID_MANUALDEPENDENCY:
			{
				StringClass filename;
				cload.Read(filename.Get_Buffer(cload.Cur_Micro_Chunk_Length()),cload.Cur_Micro_Chunk_Length());
				m_ManualDependencies.Add ((LPCTSTR)filename);				
			}
			break;
		}

		cload.Close_Micro_Chunk ();
	}

	//
	//	Check for a recursive linkage (could happen with temp presets if the
	// user deletes his registry)...
	//
	if (m_ParentID == Get_ID ()) {
		m_IsValid = false;
	}

	//
	//	Handle pointer remapping.  LEGACY CODE -- IS NOW OBSOLETE.
	//
	WWASSERT (old_this_ptr != NULL);
	SaveLoadSystemClass::Register_Pointer (old_this_ptr, this);
	if (m_Parent != NULL) {
		REQUEST_POINTER_REMAP ((void **)&m_Parent);
	}

	//
	//	Lookup the definition pointer from the definition id
	//
	WWASSERT (m_DefinitionID != 0);
	m_Definition = DefinitionMgrClass::Find_Definition (m_DefinitionID, false);
	
	//
	//	Associate this preset with the definition
	//
	if (m_Definition != NULL) {
		m_Definition->Set_User_Data ((uintptr_t)this);
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	On_Post_Load
//
///////////////////////////////////////////////////////////////////////
void
PresetClass::On_Post_Load (void)
{
	PersistClass::On_Post_Load ();

	//
	//	Lookup the parent based on its ID (instead of its pointer)
	//
	if (m_ParentID != 0) {
		m_Parent = PresetMgrClass::Find_Preset (m_ParentID);
		ASSERT (m_Parent != NULL);
		
		//
		//	Add ourselves as a child of our parent
		//
		if (m_Parent != NULL) {
			m_Parent->Add_Child_Preset (Get_ID ());
		}

#ifdef _DEBUG
		if (m_Parent == this || m_ParentID == Get_ID ()) {
			CString message;
			message.Format ("Preset %s is recursively linked to itself.\r\n", Get_Name ());
			::Output_Message (message);
		}
#endif // _DEBUG

	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Create
//
///////////////////////////////////////////////////////////////////////
NodeClass *
PresetClass::Create (void)
{
	NodeClass *node = NULL;

	StringClass error_message;
	if (m_Definition != NULL && m_Definition->Is_Valid_Config (error_message)) {
		
		uint32 class_id = m_Definition->Get_Class_ID ();
		switch (::SuperClassID_From_ClassID (class_id))
		{			
			case CLASSID_TERRAIN:
				node = new TerrainNodeClass (this);
				break;

			case CLASSID_TILE:
				node = new TileNodeClass (this);
				break;

			case CLASSID_BUILDINGS:
				node = new BuildingNodeClass (this);
				break;

			case CLASSID_GAME_OBJECTS:
				
				if (class_id == CLASSID_SPAWNER_DEF) {
					node = new SpawnerNodeClass (this);
				} else if (class_id == CLASSID_GAME_OBJECT_DEF_BUILDING) {
					node = new BuildingNodeClass (this);
				} else if (class_id == CLASSID_GAME_OBJECT_DEF_SCRIPT_ZONE) {
					node = new ZoneNodeClass (this);
				} else if (class_id == CLASSID_GAME_OBJECT_DEF_DAMAGE_ZONE) {
					node = new DamageZoneNodeClass (this);
				} else if (class_id == CLASSID_GAME_OBJECT_DEF_TRANSITION) {
					node = new TransitionNodeClass (this);
				} else if (class_id == CLASSID_GAME_OBJECT_DEF_C4) {
					::AfxGetMainWnd ()->MessageBox ("You cannot place C4 objects in the editor.", "Non-Placeable Object", MB_ICONEXCLAMATION | MB_OK);
				} else {
					node = new ObjectNodeClass (this);
				}

				break;

			case CLASSID_LIGHT:
				node = new LightNodeClass (this);
				break;

			case CLASSID_SOUND:
				node = new SoundNodeClass (this);
				break;

			case CLASSID_WAYPATH:
				node = new WaypathNodeClass (this);
				break;

			case CLASSID_DUMMY_OBJECTS:
				node = new DummyObjectNodeClass (this);
				break;

			default:
				if (class_id == CLASSID_VIS_POINT_DEF) {
					node = new VisPointNodeClass (this);
				} else if (class_id == CLASSID_PATHFIND_START_DEF) {
					node = new PathfindStartNodeClass (this);
				} else if (class_id == CLASSID_COVERSPOT) {
					node = new CoverSpotNodeClass (this);
				} else if (class_id == CLASSID_EDITOR_ONLY_OBJECTS) {
					node = new EditorOnlyObjectNodeClass (this);
				}

				break;
		}

	} else if (m_Definition != NULL) {
		
		//
		//	Warn the user
		//
		StringClass msg ("Unable to instantiate preset!\nReason: preset not configured correctly.\n\n");
		msg += error_message;
		::AfxGetMainWnd ()->MessageBox (msg, "Preset Error", MB_ICONEXCLAMATION | MB_OK);
	}

	return node;
}


///////////////////////////////////////////////////////////////////////
//
//	Copy_Properties
//
///////////////////////////////////////////////////////////////////////
void
PresetClass::Copy_Properties (const PresetClass &preset)
{
	DefinitionClass *src_def = preset.Get_Definition ();
	if ((m_Definition != NULL) && (src_def != NULL)) {

		//
		//	Simply copy the parameters from one definition to the other...
		//
		::Copy_Definition (src_def, m_Definition, m_IsTemporary);

		//
		//	Copy the transitions as well
		//
		const TRANSITION_DATA_LIST *src_list	= preset.Get_Transition_List ();
		TRANSITION_DATA_LIST *dest_list	= Get_Transition_List ();
		if (src_list != NULL && dest_list != NULL) {
			
			//
			//	Free any existing transitions
			//
			int index;
			for (index = 0; index < dest_list->Count (); index ++) {
				TransitionDataClass *data = (*dest_list)[index];
				SAFE_DELETE (data);
			}
			dest_list->Delete_All ();
			
			//
			//	Now copy each of the transitions from the source list
			//
			for (index = 0; index < src_list->Count (); index ++) {
				const TransitionDataClass *data = (*src_list)[index];
				
				//
				//	Create a new transition object and copy the src's settings
				//
				TransitionDataClass *new_data = new TransitionDataClass;
				new_data->Set_Type (data->Get_Type ());
				new_data->Set_Zone (data->Get_Zone ());
				new_data->Set_Animation_Name (data->Get_Animation_Name ());
				new_data->Set_Ending_TM (data->Get_Ending_TM ());

				//
				//	Add this new transition to our list
				//
				dest_list->Add (new_data);
			}
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_All_Assets
//
///////////////////////////////////////////////////////////////////////
void
PresetClass::Load_All_Assets (void)
{
	STRING_LIST dependencies;
	Get_All_Dependencies (dependencies);

	for (int index = 0; index < dependencies.Count (); index ++) {
		CString filename		= dependencies[index];
		CString full_path		= ::Get_File_Mgr ()->Make_Full_Path (filename);
		CString path			= ::Strip_Filename_From_Path (full_path);

		//
		//	Load all the assets contained in this file
		//
		::Set_Current_Directory (path);
		if (::Is_W3D_Filename (filename)) {
			WW3DAssetManager::Get_Instance ()->Load_3D_Assets (full_path);
			RenderObjClass *model = ::Create_Render_Obj (::Asset_Name_From_Filename (full_path));
			REF_PTR_RELEASE (model);
		} else {
			
#ifndef PUBLIC_EDITOR_VER

			if (::GetFileAttributes (full_path) == 0xFFFFFFFF) {
				::Get_File_Mgr ()->Update_File (full_path);
			}

#endif //!PUBLIC_EDITOR_VER

		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_All_Dependencies
//
///////////////////////////////////////////////////////////////////////
void
PresetClass::Get_All_Dependencies (STRING_LIST &list) const
{
	//
	//	Start fresh
	//
	list.Delete_All ();

	//
	//	Copy the manual dependencies to the list
	//
	list = m_ManualDependencies;

	//
	//	Add the implicit dependencies to the list
	//
	Add_Definition_Dependencies (m_Definition, list);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Implicit_Dependencies
//
///////////////////////////////////////////////////////////////////////
void
PresetClass::Get_Implicit_Dependencies (STRING_LIST &list) const
{
	//
	//	Start fresh
	//
	list.Delete_All ();

	//
	//	Add the implicit dependencies to the list
	//
	Add_Definition_Dependencies (m_Definition, list);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Add_Definition_Dependencies
//
///////////////////////////////////////////////////////////////////////
void
PresetClass::Add_Definition_Dependencies (DefinitionClass *definition, STRING_LIST &list)
{
	if (definition != NULL) {
		
		//
		//	Find all 'filename' parameters to this definition.
		//
		int count = definition->Get_Parameter_Count ();
		for (int index = 0; index < count; index ++) {

			ParameterClass *parameter = definition->Lock_Parameter (index);
			if (	parameter->Get_Type () == ParameterClass::TYPE_FILENAME ||
					parameter->Get_Type () == ParameterClass::TYPE_SOUND_FILENAME) {
				
				//
				//	Add this filename dependency to the list
				//
				CString filename	= ((FilenameParameterClass *)parameter)->Get_String ();
				CString full_path = ::Get_File_Mgr ()->Make_Full_Path (filename);
				if (	::Get_File_Mgr ()->Is_Path_Valid (full_path) &&
						::Get_File_Mgr ()->Is_Empty_Path (full_path) == false)
				{					
					list.Add (::Get_File_Mgr ()->Make_Relative_Path (full_path));
				}

			} else if (parameter->Get_Type () == ParameterClass::TYPE_MODELDEFINITIONID) {
				
				//
				//	If this is param references a physics-definition, then add all its dependencies as well..
				//
				DefinitionClass *phys_def = NULL;
				phys_def = DefinitionMgrClass::Find_Definition (((ModelDefParameterClass *)parameter)->Get_Value (), false);
				Add_Definition_Dependencies (phys_def, list);
			}
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Script_List_Param
//
///////////////////////////////////////////////////////////////////////
ScriptListParameterClass *
PresetClass::Get_Script_List_Param (void)
{
	ScriptListParameterClass *param = NULL;

	//
	//	Try to find a script list parameter somewhere in the preset...
	//	
	int index = m_Definition->Get_Parameter_Count ();
	while ((index --) && (param == NULL)) {
		ParameterClass *parameter = m_Definition->Lock_Parameter (index);

		//
		//	Is this the parameter we are looking for?
		//
		if (parameter->Get_Type () == ParameterClass::TYPE_SCRIPTLIST) {
			param = (ScriptListParameterClass *)parameter;
		}

		m_Definition->Unlock_Parameter (index);
	}
	
	return param;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Phys_Def_Param
//
///////////////////////////////////////////////////////////////////////
ModelDefParameterClass *
PresetClass::Get_Phys_Def_Param (void)
{
	ModelDefParameterClass *param = NULL;

	//
	//	Try to find a physics-def parameter somewhere in the preset...
	//	
	int index = m_Definition->Get_Parameter_Count ();
	while ((index --) && (param == NULL)) {
		ParameterClass *parameter = m_Definition->Lock_Parameter (index);

		//
		//	Is this the parameter we are looking for?
		//
		if (parameter->Get_Type () == ParameterClass::TYPE_MODELDEFINITIONID) {
			param = (ModelDefParameterClass *)parameter;
		}

		m_Definition->Unlock_Parameter (index);
	}
	

	return param;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Transition_List
//
///////////////////////////////////////////////////////////////////////
TRANSITION_DATA_LIST *
PresetClass::Get_Transition_List (void) const
{
	TRANSITION_DATA_LIST *list = NULL;

	if (m_Definition != NULL) {
		switch (m_Definition->Get_Class_ID ()) {
			
			//
			//	Vehicles from Combat Lib
			//
			case CLASSID_GAME_OBJECT_DEF_VEHICLE:
			{
				VehicleGameObjDef *vehicle_def = (VehicleGameObjDef *)m_Definition;
				list = vehicle_def->Get_Transition_List ();
			}
			break;

			case CLASSID_GAME_OBJECT_DEF_TRANSITION:
			{
				TransitionGameObjDef *trans_def = (TransitionGameObjDef *)m_Definition;
				list = trans_def->Get_Transition_List ();
			}
			break;
		}
	}

	return list;
}


///////////////////////////////////////////////////////////////////////
//
//	Build_Zone_List
//
///////////////////////////////////////////////////////////////////////
void
PresetClass::Build_Zone_List (DefinitionClass *definition, ZONE_PARAM_LIST &zone_list) const
{
	if (definition == NULL) {
		definition = m_Definition;
	}

	if (definition != NULL) {

		//
		//	Loop over all the parameters
		//
		int count = definition->Get_Parameter_Count ();
		for (int index = 0; index < count; index ++) {
			ParameterClass *parameter = definition->Lock_Parameter (index);

			//
			//	If this is a zone parameter, then add it to the list
			//
			if (parameter->Get_Type () == ParameterClass::TYPE_ZONE) {
				ZoneParameterClass *zone_param = (ZoneParameterClass *)parameter;
				zone_list.Add (zone_param);
			} else if (parameter->Get_Type () == ParameterClass::TYPE_MODELDEFINITIONID) {
				
				//
				//	Recurse into the physics definition (if necessary)
				//
				int def_id = ((ModelDefParameterClass *)parameter)->Get_Value ();
				DefinitionClass *model_def= DefinitionMgrClass::Find_Definition (def_id, false);
				if (model_def !=  NULL) {
					Build_Zone_List (model_def, zone_list);
				}
			}

			definition->Unlock_Parameter (index);
		}
	}

	return ;
}



///////////////////////////////////////////////////////////////////////
//
//	Add_Files_To_VSS
//
///////////////////////////////////////////////////////////////////////
void
PresetClass::Add_Files_To_VSS (void)
{
	if (::Get_File_Mgr ()->Is_VSS_Read_Only () == false) {
		HWND hdlg = Show_VSS_Update_Dialog (::AfxGetMainWnd ()->m_hWnd);

		//
		//	Build a list of dependencies
		//
		STRING_LIST dependencies;
		Get_All_Dependencies (dependencies);

		//
		//	Loop over all the files and add them to VSS
		//	
		for (int index = 0; index < dependencies.Count (); index ++) {
			CString filename		= dependencies[index];
			CString full_path		= ::Get_File_Mgr ()->Make_Full_Path (filename);		
			::Get_File_Mgr ()->Add_Files_To_Database (full_path);
		}

		::Kill_VSS_Update_Dialog (hdlg);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Is_Valid_Sound_Preset
//
///////////////////////////////////////////////////////////////////////
bool
PresetClass::Is_Valid_Sound_Preset (void)
{
	bool retval = false;
	
	//
	//	First off, is this a sound preset at all?
	//
	if (	m_Definition != NULL &&
			m_Definition->Get_Class_ID () == CLASSID_SOUND)
	{		
		AudibleSoundDefinitionClass *definition = (AudibleSoundDefinitionClass *)m_Definition;

		//
		//	Does this preset point to a file?
		//
		CString filename = static_cast<const char *>(definition->Get_Filename ());
		DWORD file_attrs = ::GetFileAttributes (definition->Get_Filename ());
		if (	filename.GetLength () > 0 && 
				(file_attrs == 0xFFFFFFFF ||
				 file_attrs != FILE_ATTRIBUTE_DIRECTORY))
		{
			retval = true;
		}
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Is_A_Parent
//
///////////////////////////////////////////////////////////////////////
bool
PresetClass::Is_A_Parent (PresetClass *preset)
{
	bool retval = false;

	//
	//	Look over all the parents
	//
	for (	PresetClass *parent = this;
			parent != NULL && !retval;
			parent = parent->Get_Parent ())
	{
		retval = (parent == preset);
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Is_A_Parent
//
///////////////////////////////////////////////////////////////////////
bool
PresetClass::Is_A_Parent (LPCTSTR parent_name)
{
	bool retval = false;

	//
	//	Look over all the parents
	//
	for (	PresetClass *parent = this;
			parent != NULL && !retval;
			parent = parent->Get_Parent ())
	{
		retval = (::lstrcmpi (parent->Get_Name (), parent_name) == 0);
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Icon_Index
//
///////////////////////////////////////////////////////////////////////
int
PresetClass::Get_Icon_Index (void) const
{
	int index = 0;

	//
	//	What type is it?
	//
	uint32 class_id = m_Definition->Get_Class_ID ();
	switch (::SuperClassID_From_ClassID (class_id))
	{			
		case CLASSID_TERRAIN:
			index = TERRAIN_ICON;
			break;

		case CLASSID_TILE:
			index = TILE_ICON;
			break;

		case CLASSID_MUNITIONS:
		case CLASSID_DUMMY_OBJECTS:
		case CLASSID_GAME_OBJECTS:
			index = OBJECT_ICON;
			break;

		case CLASSID_LIGHT:
			index = LIGHT_ICON;
			break;

		case CLASSID_SOUND:
			index = SOUND_ICON;
			break;

		case CLASSID_WAYPATH:
			index = WAYPATH_ICON;
			break;

		case CLASSID_ZONE:
			index = ZONE_ICON;
			break;

		case CLASSID_TRANSITION:
			index = TRANSITION_ICON;
			break;

		case CLASSID_BUILDINGS:
			index = BUILDING_ICON;
			break;

		case CLASSID_TWIDDLERS:
			index = RAND_ICON;
			break;

		default:
			if (class_id == CLASSID_VIS_POINT_DEF) {
				index = VIS_ICON;
			} else if (class_id == CLASSID_PATHFIND_START_DEF) {
				index = PATHFIND_ICON;
			} else if (class_id == CLASSID_COVERSPOT) {
				index = OBJECT_ICON;
			}
			break;
	}

	return index;
}


///////////////////////////////////////////////////////////////////////
//
//	Collect_Definitions
//
///////////////////////////////////////////////////////////////////////
void
PresetClass::Collect_Definitions (DEFINITION_LIST &list)
{
	if (m_Definition != NULL) {
		list.Add (m_Definition);
		
		//
		//	Get the physics definition that this definition embeds (if any)
		//
		::Build_Embedded_Definition_List (list, m_Definition);
	}

	return ;
}


////////////////////////////////////////////////////////////////////
//
//	Is_Soldier_Preset
//
////////////////////////////////////////////////////////////////////
bool
PresetClass::Is_Soldier_Preset (void)
{
	bool is_soldier = false;
	if (m_Definition != NULL) {
		is_soldier = (m_Definition->Get_Class_ID () == CLASSID_GAME_OBJECT_DEF_SOLDIER);
	}

	return is_soldier;
}


////////////////////////////////////////////////////////////////////
//
//	Build_Node_List
//
////////////////////////////////////////////////////////////////////
void
PresetClass::Build_Node_List (NodeClass *parent_node)
{
	Free_Node_List ();

	//
	//	Build a matrix that we can use to transform each
	// node into the coordinate system of the parent node
	//
	Matrix3D parent_tm = parent_node->Get_Transform ();
	Matrix3D world_to_parent_tm;
	parent_tm.Get_Orthogonal_Inverse (world_to_parent_tm);

	//
	//	Loop over all the nodes in the scene
	//
	for (	NodeClass *node = NodeMgrClass::Get_First ();
			node != NULL;
			node = NodeMgrClass::Get_Next (node))
	{
		//
		//	Is this a node we want to bind to this preset?
		//
		if (	node != parent_node &&
				node->Is_Proxied () == false &&
				node->Get_Parent_Node () == NULL &&
				node->Get_Type () != NODE_TYPE_WAYPOINT)
		{			
			//
			//	Make a copy of the node
			//
			NodeClass *new_node = node->Clone ();
			if (new_node != NULL) {

				//
				//	Transform the new node into the coordinate system
				// of the parent node
				//
				Matrix3D curr_tm	= node->Get_Transform ();
				Matrix3D rel_tm	= world_to_parent_tm * curr_tm;
				new_node->Set_Transform (rel_tm);
				new_node->Set_Needs_Save (false);
				new_node->Set_Container_Preset_ID (Get_ID ());

				//
				//	Add this node to our list
				//
				m_NodeList.Add (new_node);
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////
//
//	Free_Node_List
//
////////////////////////////////////////////////////////////////////
void
PresetClass::Free_Node_List (void)
{
	for (int index = 0; index < m_NodeList.Count (); index ++) {
		REF_PTR_RELEASE (m_NodeList[index]);
	}

	m_NodeList.Delete_All ();
	return ;
}


////////////////////////////////////////////////////////////////////
//
//	Create_Linked_Nodes
//
////////////////////////////////////////////////////////////////////
void
PresetClass::Create_Linked_Nodes (NodeClass *parent_node)
{
	//
	//	Build a matrix that we can use to transform each
	// node into the coordinate system of the parent node
	//
	Matrix3D parent_tm = parent_node->Get_Transform ();

	for (int index = 0; index < m_NodeList.Count (); index ++) {
		
		//
		//	Clone this node via the scene so it will get installed into
		// all the appropriate systems
		//
		NodeClass *new_node = ::Get_Scene_Editor ()->Clone_Node (m_NodeList[index]);

		//
		//	Transform the node into world space
		//
		Matrix3D rel_tm	= m_NodeList[index]->Get_Transform ();
		Matrix3D new_tm	= parent_tm * rel_tm;
		new_node->Set_Transform (new_tm);
		new_node->Lock (true);
		new_node->Set_Is_Proxied (true);
		new_node->Set_Needs_Save (false);
		new_node->Set_Container_Preset_ID (Get_ID ());

		//
		//	Release our hold on the node
		//
		REF_PTR_RELEASE (new_node);
	}

	return ;
}


////////////////////////////////////////////////////////////////////
//
//	Get_Child_Preset
//
////////////////////////////////////////////////////////////////////
PresetClass *
PresetClass::Get_Child_Preset (int index)
{
	PresetClass *retval = NULL;

	if (index >= 0 && index < m_ChildIDList.Count ()) {
		retval = PresetMgrClass::Find_Preset (m_ChildIDList[index]);
	}

	return retval;
}


////////////////////////////////////////////////////////////////////
//
//	Set_Definition
//
////////////////////////////////////////////////////////////////////
void
PresetClass::Set_Definition (DefinitionClass *definition)
{
	if (m_Definition != NULL) {
		m_Definition->Set_User_Data (NULL);
	}

	m_Definition		= definition;
	m_DefinitionID		= 0;

	//
	//	Store our "this" pointer in the definition so we can
	// quickly find the preset given its definition
	//
	if (m_Definition != NULL) {
		m_Definition->Set_User_Data ((uintptr_t)this);
		m_DefinitionID = m_Definition->Get_ID ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////
//
//	Add_Child_Preset
//
////////////////////////////////////////////////////////////////////
void
PresetClass::Add_Child_Preset (int child_id)
{
	//
	//	Don't allow ourselves to become recursively linked
	//
	if (child_id == (int)Get_ID ()) {
		return ;
	}

	//
	//	Make sure the child isn't already in our list
	//
	int index = m_ChildIDList.ID (child_id);
	WWASSERT (index == -1);

	//
	//	Add the child to the list
	//
	if (index == -1) {
		m_ChildIDList.Add (child_id);
	}

	return ;
}


////////////////////////////////////////////////////////////////////
//
//	Remove_Child_Preset
//
////////////////////////////////////////////////////////////////////
void
PresetClass::Remove_Child_Preset (int child_id)
{
	int index = m_ChildIDList.ID (child_id);
	if (index != -1) {
		// Cast needed to disambiguate Delete(int) and Delete(const T &) where T = int
		(m_ChildIDList.*(static_cast<bool (DynamicVectorClass<int>::*) (int)>(&DynamicVectorClass<int>::Delete))) (index);
	}
		
	return ;
}


////////////////////////////////////////////////////////////////////
//
//	Set_Parent
//
////////////////////////////////////////////////////////////////////
void
PresetClass::Set_Parent (PresetClass *parent)
{
	//
	//	If changing parents, unlink ourselves from our old parent
	//
	if (m_Parent != NULL) {
		m_Parent->Remove_Child_Preset (Get_ID ());
	}

	m_Parent = parent;

	//
	//	Cache the parent's ID
	//
	if (m_Parent != NULL) {
		m_ParentID = m_Parent->Get_ID ();
	} else {
		m_ParentID = 0;
	}
	
	return ;
}
