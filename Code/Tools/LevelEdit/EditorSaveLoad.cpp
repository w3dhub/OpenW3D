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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/EditorSaveLoad.cpp           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/05/02 3:30p                                                $*
 *                                                                                             *
 *                    $Revision:: 37                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "leveleditdoc.h"
#include "editorsaveload.h"
#include "persist.h"
#include "persistfactory.h"
#include "preset.h"
#include "utils.h"
#include "definition.h"
#include "editorchunkids.h"
#include "physstaticsavesystem.h"
#include "chunkio.h"
#include "cameramgr.h"
#include "filemgr.h"
#include "matrix3d.h"
#include "rawfile.h"
#include "nodemgr.h"
#include "sceneeditor.h"
#include "node.h"
#include "lightambientform.h"
#include "soundscene.h"
#include "backgroundmgr.h"
#include "conversationmgr.h"
#include "weathermgr.h"
#include "combat.h"
#include "mapmgr.h"
#include "pathfind.h"
#include "lightsolvesavesystem.h"
#include "heightfieldmgr.h"


///////////////////////////////////////////////////////////////////////
// Global singleton instance
///////////////////////////////////////////////////////////////////////
EditorSaveLoadClass _TheEditorSaveLoadSubsystem;
PathfindImportExportSaveLoadClass _ThePathfindImporterExporter;


///////////////////////////////////////////////////////////////////////
// Static member initialization
///////////////////////////////////////////////////////////////////////
bool	EditorSaveLoadClass::m_LoadedValidVis = true;


///////////////////////////////////////////////////////////////////////
//	Constants
///////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_MICRO_CHUNKS		= 0x10140738,
	CHUNKID_OBSOLETE,
	CHUNKID_PATHFIND_DATA
};


enum
{
	VARID_INCLUDE_FILE		= 0x01,
	VARID_CAMERA_TM,
	VARID_OBSOLETE_0,
	VARID_BACK_MUSIC,
	VARID_AMBIENT_LIGHT,
	VARID_OBSOLETE_1,
	VARID_OBSOLETE_2,
	VARID_FAR_CLIP_PLANE_DOUBLE,
	VARID_FOG_COLOR,
	VARID_FOG_PLANES,
	VARID_FOG_ENABLED,
	VARID_FAR_CLIP_PLANE,
	VARID_RESTART_SCRIPT_NAME,
	VARID_RESPAWN_SCRIPT_NAME,
};

enum 
{
	CHUNKID_LVL_DATA			= 304021447,
	CHUNKID_LIGHT_SOLVE,
};

///////////////////////////////////////////////////////////////////////
//
//	Chunk_ID
//
///////////////////////////////////////////////////////////////////////
uint32
EditorSaveLoadClass::Chunk_ID (void) const
{
	return CHUNKID_EDITOR_SAVELOAD;
}


///////////////////////////////////////////////////////////////////////
//
//	Contains_Data
//
///////////////////////////////////////////////////////////////////////
bool
EditorSaveLoadClass::Contains_Data (void) const
{
	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////
bool
EditorSaveLoadClass::Save (ChunkSaveClass &csave)
{
	bool retval = true;
	int  index;

	csave.Begin_Chunk (CHUNKID_MICRO_CHUNKS);

	//
	//	Write the camera transform to the chunk
	//
	Matrix3D camera_tm = ::Get_Camera_Mgr ()->Get_Camera ()->Get_Transform ();
	WRITE_MICRO_CHUNK (csave, VARID_CAMERA_TM, camera_tm);

	//
	//	Write the miscellaneous level settings to the chunk
	//
	Vector3 ambient_light	= ::Get_Scene_Editor  ()->Get_Ambient_Light ();
	WRITE_MICRO_CHUNK (csave, VARID_AMBIENT_LIGHT, ambient_light);

	//
	//	Save the fog settings
	//
	bool fog_enabled	= false;
	struct
	{
		float z_near;
		float z_far;
	} fog_planes = { 50.0F, 100.0F };

	Vector3 fog_color (0, 0, 0);	
	::Get_Scene_Editor ()->Get_Fog_Range (&fog_planes.z_near, &fog_planes.z_far);
	fog_color	= ::Get_Scene_Editor ()->Get_Fog_Color ();
	fog_enabled	= ::Get_Scene_Editor ()->Get_Fog_Enable ();	

	WRITE_MICRO_CHUNK (csave, VARID_FOG_ENABLED, fog_enabled);
	WRITE_MICRO_CHUNK (csave, VARID_FOG_COLOR, fog_color);
	WRITE_MICRO_CHUNK (csave, VARID_FOG_PLANES, fog_planes);

	//
	//	Save the restart and respawn script names
	//
	StringClass restart_script_name = CombatManager::Get_Start_Script ();
	StringClass respawn_script_name = CombatManager::Get_Respawn_Script ();
	WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_RESTART_SCRIPT_NAME, restart_script_name);
	WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_RESPAWN_SCRIPT_NAME, respawn_script_name);

	//
	//	Save the far clip plane...
	//
	float znear = 0;
	float zfar = 0;
	::Get_Camera_Mgr ()->Get_Camera ()->Get_Clip_Planes (znear, zfar);
	WRITE_MICRO_CHUNK (csave, VARID_FAR_CLIP_PLANE, zfar);	

	//
	//	Write the background music filename out to the chunk
	//
	CString filename = ::Get_Scene_Editor ()->Get_Background_Music_Filename ();
	CString path = ::Get_File_Mgr ()->Make_Relative_Path (filename);
	WRITE_MICRO_CHUNK_STRING (csave, VARID_BACK_MUSIC, (LPCTSTR)path);	

	//
	// Write the list of include files for the level to the chunk.
	//
	STRING_LIST &include_list = ::Get_File_Mgr ()->Get_Include_File_List ();
	for (index = 0; index < include_list.Count (); index ++) {
		StringClass filename = (LPCTSTR)include_list[index];
		WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_INCLUDE_FILE, filename);
	}

	csave.End_Chunk ();

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Load
//
///////////////////////////////////////////////////////////////////////
bool
EditorSaveLoadClass::Load (ChunkLoadClass &cload)
{
	bool retval = true;	
	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {
			
			//
			//	Load all the presets from this chunk
			//
			case CHUNKID_MICRO_CHUNKS:
				retval &= Load_Micro_Chunks (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	SaveLoadSystemClass::Register_Post_Load_Callback (this);
	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Micro_Chunks
//
///////////////////////////////////////////////////////////////////////
bool
EditorSaveLoadClass::Load_Micro_Chunks (ChunkLoadClass &cload)
{
	bool retval = true;

	Vector3 fog_color (0, 0, 0);
	bool fog_enabled = false;

	StringClass restart_script_name;
	StringClass respawn_script_name;

	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			
			//
			//	Read the camera's transformation matrix from the chunk
			//	and pass it onto the camera.
			//
			case VARID_CAMERA_TM:
			{
				Matrix3D camera_tm;
				cload.Read(&camera_tm,sizeof (camera_tm));
				::Get_Camera_Mgr ()->Get_Camera ()->Set_Transform (camera_tm);
			}
			break;

			//
			//	Read the include file from the chunk and add it to the list
			// of level include files.
			//
			case VARID_INCLUDE_FILE:
			{
				StringClass filename;
				cload.Read(filename.Get_Buffer(cload.Cur_Micro_Chunk_Length()),cload.Cur_Micro_Chunk_Length());
				::Get_File_Mgr ()->Get_Include_File_List ().Add ((LPCTSTR)filename);
			}
			break;

			case VARID_FAR_CLIP_PLANE_DOUBLE:
			{
				float znear = 0;
				float zfar = 0;				
				::Get_Camera_Mgr ()->Get_Camera ()->Get_Clip_Planes (znear, zfar);

				double double_zfar = 0;
				cload.Read (&double_zfar, sizeof (double_zfar));
				zfar = double_zfar;
				::Get_Camera_Mgr ()->Get_Camera ()->Set_Clip_Planes (znear, zfar);
			}
			break;

			case VARID_FAR_CLIP_PLANE:
			{
				float znear = 0;
				float zfar = 0;				
				::Get_Camera_Mgr ()->Get_Camera ()->Get_Clip_Planes (znear, zfar);
				cload.Read (&zfar, sizeof (zfar));
				::Get_Camera_Mgr ()->Get_Camera ()->Set_Clip_Planes (znear, zfar);
			}
			break;

			case VARID_AMBIENT_LIGHT:
			{
				Vector3 ambient_light;
				cload.Read (&ambient_light, sizeof (ambient_light));
				::Get_Scene_Editor ()->Set_Ambient_Light (ambient_light);
				::Get_Scene_Editor ()->Update_Lighting ();
				
				LightAmbientFormClass *light_form = Get_Ambient_Light_Form ();
				light_form->Update_Settings ();
			}
			break;

			READ_MICRO_CHUNK (cload, VARID_FOG_ENABLED, fog_enabled);
			READ_MICRO_CHUNK (cload, VARID_FOG_COLOR, fog_color);

			READ_MICRO_CHUNK_WWSTRING (cload, VARID_RESTART_SCRIPT_NAME, restart_script_name);
			READ_MICRO_CHUNK_WWSTRING (cload, VARID_RESPAWN_SCRIPT_NAME, respawn_script_name);
			
			case VARID_FOG_PLANES:
			{
				struct
				{
					float z_near;
					float z_far;
				} fog_planes = { 50.0F, 100.0F };

				cload.Read (&fog_planes, sizeof (fog_planes));
				::Get_Scene_Editor ()->Set_Fog_Range (fog_planes.z_near, fog_planes.z_far);
			}	
			break;

			case VARID_BACK_MUSIC:
			{
				StringClass filename;
				cload.Read(filename.Get_Buffer(cload.Cur_Micro_Chunk_Length()),cload.Cur_Micro_Chunk_Length());
				::Get_Scene_Editor ()->Set_Background_Music (filename);
			}
			break;
		}

		cload.Close_Micro_Chunk ();
	}

	//
	//	Apply the script settings
	//
	CombatManager::Set_Start_Script (restart_script_name);
	CombatManager::Set_Respawn_Script (respawn_script_name);

	//
	//	Apply the fog settings
	//
	::Get_Scene_Editor ()->Set_Fog_Color (fog_color);
	::Get_Scene_Editor ()->Set_Fog_Enable (fog_enabled);	

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Save_Level
//
///////////////////////////////////////////////////////////////////////
void
EditorSaveLoadClass::Save_Level (LPCTSTR filename)
{
	//
	//	Create the file
	//
	HANDLE hfile = ::CreateFile (filename,
										  GENERIC_WRITE,
										  0,
										  NULL,
										  CREATE_ALWAYS,
										  0L,
										  NULL);

	ASSERT (hfile != INVALID_HANDLE_VALUE);
	if (hfile != INVALID_HANDLE_VALUE) {

		RawFileClass file_obj;
		file_obj.Attach (hfile);
		ChunkSaveClass chunk_save (&file_obj);

		//
		// (gth) March 4, 2002 - modifying this function to embed two "save files" into this 
		// file.  The first chunk will contain an entire save for the normal level data.  The second
		// chunk will contain the lighting save
		//
		chunk_save.Begin_Chunk(CHUNKID_LVL_DATA);
		Save_Level_Data(chunk_save);
		chunk_save.End_Chunk();

		chunk_save.Begin_Chunk(CHUNKID_LIGHT_SOLVE);
		Save_Light_Solve(chunk_save);
		chunk_save.End_Chunk();


		//
		//	Remember that we are now up-to-date
		//
		Set_Modified (false);
	}
	return ;
}

///////////////////////////////////////////////////////////////////////
//
//	Save_Level_Data - saves the level data for an LVL file
//
///////////////////////////////////////////////////////////////////////
void
EditorSaveLoadClass::Save_Level_Data(ChunkSaveClass & chunk_save)
{
	//
	//	Repartition the static culling systems
	// (gth) can't re-partition the object culling systems or
	// we'll lose hierarchical vis
	//
	::Get_Scene_Editor ()->Re_Partition_Static_Lights ();	
	::Get_Scene_Editor ()->Re_Partition_Audio_System ();		

	//
	//	Basically just save the list of nodes and some other
	// miscellaneous information (camera, sky, etc).
	//		
	SaveLoadSystemClass::Save (chunk_save, _PhysStaticDataSaveSystem);
	SaveLoadSystemClass::Save (chunk_save, _TheNodeMgr);
	SaveLoadSystemClass::Save (chunk_save, _TheEditorSaveLoadSubsystem);
	SaveLoadSystemClass::Save (chunk_save, _TheBackgroundMgr);
	SaveLoadSystemClass::Save (chunk_save, _TheWeatherMgr);
	SaveLoadSystemClass::Save (chunk_save, _TheMapMgrSaveLoadSubsystem);		
	SaveLoadSystemClass::Save (chunk_save, _TheHeightfieldMgrSaveLoadSubsystem);
		
	//
	//	Save the level-specific conversations
	//
	_ConversationMgrSaveLoad.Set_Category_To_Save (ConversationMgrClass::CATEGORY_LEVEL);
	SaveLoadSystemClass::Save (chunk_save, _ConversationMgrSaveLoad);
	return ;
}

///////////////////////////////////////////////////////////////////////
//
//	Save_Light_Solve - saves the light-solve data for an LVL file
//
///////////////////////////////////////////////////////////////////////
void EditorSaveLoadClass::Save_Light_Solve(ChunkSaveClass & chunk_save)
{
	SaveLoadSystemClass::Save( chunk_save, _TheLightSolveSaveSystem);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Level
//
///////////////////////////////////////////////////////////////////////
void
EditorSaveLoadClass::Load_Level (LPCTSTR filename)
{
	//
	//	Create the file
	//
	HANDLE hfile = ::CreateFile (filename,
										  GENERIC_READ,
										  FILE_SHARE_READ,
										  NULL,
										  OPEN_EXISTING,
										  0L,
										  NULL);

	ASSERT (hfile != INVALID_HANDLE_VALUE);
	if (hfile != INVALID_HANDLE_VALUE) {

		RawFileClass file_obj;
		file_obj.Attach (hfile);
		ChunkLoadClass chunk_load (&file_obj);

		SoundSceneClass *sound_scene = WWAudioClass::Get_Instance ()->Get_Sound_Scene ();
		if (sound_scene != NULL) {
			sound_scene->Set_Batch_Mode (true);
		}

		//
		//	Get the save-load subsystem to load
		//
		m_LoadedValidVis = true;
		NodeMgrClass::Free_Nodes ();

		//
		// (gth) March 4, 2002 - LVL files are now two independent save-load operations,
		// one for the normal level data and one for the light solve data.  Here, we
		// detect whether we are loading a file that was created before this change.
		//
		uint32 id,size;
		if (chunk_load.Peek_Next_Chunk(&id,&size) && (id == CHUNKID_LVL_DATA)) {
		
			// Current file format, multiple saves embedded into this file
			while (chunk_load.Open_Chunk()) {

				switch (chunk_load.Cur_Chunk_ID()) {
					case CHUNKID_LVL_DATA:
					case CHUNKID_LIGHT_SOLVE:
						SaveLoadSystemClass::Load (chunk_load);
						break;
					default:
						break;
				}
				
				chunk_load.Close_Chunk();
			}

		} else {

			// Legacy file format support
			SaveLoadSystemClass::Load (chunk_load);

		}

		//
		//	Repartition the static culling systems
		// (gth) can't re-partition the object culling systems or
		// we'll lose hierarchical vis
		//
		::Get_Scene_Editor ()->Re_Partition_Static_Lights ();
		::Get_Scene_Editor ()->Re_Partition_Audio_System ();

		//
		//	Now create any embedded nodes (nodes that are embedded inside
		// the preset of any nodes that are currently in the level).
		//
		NodeMgrClass::Create_All_Embedded_Nodes ();
		
		//
		//	Validate VIS information if necessary
		//
		if (m_LoadedValidVis) {
			::Get_Scene_Editor ()->Validate_Vis ();
			::Get_Scene_Editor ()->Update_Culling_System_Bounding_Boxes ();
		} else {
			::MessageBox (NULL, "Static geometry has changed since the last time this level was loaded.  All previously generated VIS data has been discarded.", "Vis Discarded", MB_ICONEXCLAMATION | MB_OK);
			::Get_Scene_Editor ()->Discard_Vis ();
		}

		if (sound_scene != NULL) {
			sound_scene->Set_Batch_Mode (false);
		}

		//
		//	Remember that we are now up-to-date
		//
		Set_Modified (false);
	}
	
	return ;
}

///////////////////////////////////////////////////////////////////////
//
//	Export_Dynamic_Objects
//
///////////////////////////////////////////////////////////////////////
void
EditorSaveLoadClass::Export_Dynamic_Objects (LPCTSTR filename)
{
	//
	//	Create the file
	//
	HANDLE hfile = ::CreateFile (filename,
										  GENERIC_WRITE,
										  0,
										  NULL,
										  CREATE_ALWAYS,
										  0L,
										  NULL);

	ASSERT (hfile != INVALID_HANDLE_VALUE);
	if (hfile != INVALID_HANDLE_VALUE) {

		RawFileClass file_obj;
		file_obj.Attach (hfile);
		ChunkSaveClass chunk_save (&file_obj);

		//
		//	Remove the static objects from the level, save the nodes,
		// then put the static objects back
		//
		NODE_LIST static_obj_list;
		NodeMgrClass::Remove_Static_Objects (static_obj_list);
		SaveLoadSystemClass::Save (chunk_save, _TheNodeMgr);		
		NodeMgrClass::Put_Objects_Back (static_obj_list);
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Import_Dynamic_Objects
//
///////////////////////////////////////////////////////////////////////
void
EditorSaveLoadClass::Import_Dynamic_Objects (LPCTSTR filename)
{
	//
	//	Create the file
	//
	HANDLE hfile = ::CreateFile (filename,
										  GENERIC_READ,
										  FILE_SHARE_READ,
										  NULL,
										  OPEN_EXISTING,
										  0L,
										  NULL);

	ASSERT (hfile != INVALID_HANDLE_VALUE);
	if (hfile != INVALID_HANDLE_VALUE) {

		RawFileClass file_obj;
		file_obj.Attach (hfile);
		ChunkLoadClass chunk_load (&file_obj);

		//
		//	Find the largest used ID
		//
		uint32 start_id = (NodeMgrClass::Get_Max_Used_ID () + 1);

		//
		//	Remove the dynamic objects from the level and load the new objects.
		//
		NODE_LIST dynamic_obj_list;
		NodeMgrClass::Remove_Dynamic_Objects (dynamic_obj_list);
		SaveLoadSystemClass::Load (chunk_load);

		::Output_Message ("Resetting existing static node IDs.\r\n");
				
		//
		//	Re-assign IDs to the existing static nodes
		//
		for (	NodeClass *node = NodeMgrClass::Get_First ();
				node != NULL;
				node = NodeMgrClass::Get_Next (node))
		{
			if (node->Is_Static ()) {
				node->Set_ID (start_id++);
			}
		}

		//
		//	Check all the dynamic objects for ID collision
		//
		CString id_collision_msg = "The following dynamic object IDs collided on import:\n\n";
		bool show_msg = false;
		DynamicVectorClass<NodeClass *> bad_node_list;
		for (int index = 0; index < dynamic_obj_list.Count (); index ++) {
			NodeClass *node = dynamic_obj_list[index];
			if (NodeMgrClass::Find_Node (node->Get_ID ()) != NULL) {
				CString entry;
				entry.Format ("Object %d\n", node->Get_ID ());				
				id_collision_msg += entry;
				bad_node_list.Add (node);
				show_msg = true;
			}
		}

		//
		//	Put the original objects back
		//
		NodeMgrClass::Put_Objects_Back (dynamic_obj_list);
		NodeMgrClass::Reset_New_ID ();

		//
		//	Let the user know we had ID collision (if necessary)
		//
		if (show_msg) {
			::MessageBox (NULL, id_collision_msg, "ID Collision", MB_ICONERROR | MB_OK);
			
			//
			//	Ask the user if we should fix the collisions or not
			//
			if (::MessageBox (NULL, "Would you like to repair the newly imported objects with ambiguous IDs?", "ID Collision", MB_ICONQUESTION | MB_YESNO) == IDYES) {
				
				//
				//	Repair all collisions
				//
				for (int index = 0; index < bad_node_list.Count (); index ++) {
					NodeClass *node = bad_node_list[index];
					if (node != NULL) {
						node->Set_ID (NodeMgrClass::Get_Node_ID (node->Get_Type ()));
					}
				}				
			}
		}
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	On_Post_Load
//
///////////////////////////////////////////////////////////////////////
void
EditorSaveLoadClass::On_Post_Load (void)
{
}












///////////////////////////////////////////////////////////////////////
//
//	Export_Pathfind
//
///////////////////////////////////////////////////////////////////////
void
PathfindImportExportSaveLoadClass::Export_Pathfind (LPCTSTR filename)
{
	//
	//	Create the file
	//
	HANDLE hfile = ::CreateFile (filename,
										  GENERIC_WRITE,
										  0,
										  NULL,
										  CREATE_ALWAYS,
										  0L,
										  NULL);

	ASSERT (hfile != INVALID_HANDLE_VALUE);
	if (hfile != INVALID_HANDLE_VALUE) {

		RawFileClass file_obj;
		file_obj.Attach (hfile);
		ChunkSaveClass chunk_save (&file_obj);

		//
		//
		//	Basically just save the list of nodes and some other
		// miscellaneous information (camera, sky, etc).
		//		
		SaveLoadSystemClass::Save (chunk_save, _ThePathfindImporterExporter);
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Import_Pathfind
//
///////////////////////////////////////////////////////////////////////
void
PathfindImportExportSaveLoadClass::Import_Pathfind (LPCTSTR filename)
{
	//
	//	Create the file
	//
	HANDLE hfile = ::CreateFile (filename,
										  GENERIC_READ,
										  FILE_SHARE_READ,
										  NULL,
										  OPEN_EXISTING,
										  0L,
										  NULL);

	ASSERT (hfile != INVALID_HANDLE_VALUE);
	if (hfile != INVALID_HANDLE_VALUE) {

		RawFileClass file_obj;
		file_obj.Attach (hfile);
		ChunkLoadClass chunk_load (&file_obj);

		//
		//	Get the save-load subsystem to load
		//
		SaveLoadSystemClass::Load (chunk_load);
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////
bool
PathfindImportExportSaveLoadClass::Save (ChunkSaveClass &csave)
{
	//
	//	Save the pathfind data
	//
	csave.Begin_Chunk (CHUNKID_PATHFIND_DATA);
		PathfindClass::Get_Instance()->Save (csave);
	csave.End_Chunk ();

	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	Load
//
///////////////////////////////////////////////////////////////////////
bool
PathfindImportExportSaveLoadClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {
			
			//
			//	Import the pathfind data...
			//
			case CHUNKID_PATHFIND_DATA:
				PathfindClass::Get_Instance()->Load (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	return true;
}
