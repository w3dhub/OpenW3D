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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/Export.cpp                   $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 6/26/02 2:13p                                               $*
 *                                                                                             *
 *                    $Revision:: 34                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"

#include "LevelEdit.h"
#include "leveleditdoc.h"
#include "export.h"
#include "editorini.h"
#include "Utils.h"
#include "resource.h"
#include "nodemgr.h"
#include "vector3.h"
#include "matrix3d.h"
#include "ffactory.h"
#include "filemgr.h"
#include "_assetmgr.h"
#include "editorassetmgr.h"
#include "..\..\commando\lev_file.h"
#include "chunkio.h"
#include "sceneeditor.h"
#include "filelocations.h"
#include "inisections.h"
#include "hlod.h"
#include "w3derr.h"
#include "saveload.h"
#include "physstaticsavesystem.h"
#include "physdynamicsavesystem.h"
#include "combatsaveload.h"
#include "terrainnode.h"
#include "staticphys.h"
#include "savegame.h"
#include "regkeys.h"
#include "combat.h"
#include "assetdep.h"
#include "presetslibform.h"
#include "Pathfind.h"
#include "gameobjmanager.h"
#include "stringsmgr.h"
#include "assetpackagemgr.h"
#include "presetmgr.h"


/////////////////////////////////////////////////////////
//	Local constants
/////////////////////////////////////////////////////////
const TCHAR * const STATIC_EXTENSION			= TEXT (".lsd");
const TCHAR * const DYNAMIC_EXTENSION			= TEXT (".ldd");
const TCHAR * const MIX_EXTENSION				= TEXT (".mix");

const TCHAR * const ALWAYS_ROOT_NAME			= TEXT ("always");
const TCHAR * const ALWAYS_DATA_FILENAME		= TEXT ("always.dat");
const TCHAR * const ALWAYS_DB_FILENAME			= TEXT ("always.dbs");


/////////////////////////////////////////////////////////
//
//	Export_Level
//
/////////////////////////////////////////////////////////
void
ExporterClass::Export_Level (LPCTSTR filename)
{	
	CWaitCursor wait_cursor;
	m_AssetList.Delete_All ();

	//
	//	Prepare the level for exporting...
	//
	DynamicVectorClass<NodeClass *> node_list;
	Pre_Level_Export (node_list);

	//
	//	Get the base destination path
	//
	Path = ::Strip_Filename_From_Path (filename);	

	//
	//	Strip the path and extension from the filename to give
	// us a base for use with other filenames
	//
	FilenameBase = ::Get_Filename_From_Path (filename);
	int extension_index = FilenameBase.ReverseFind ('.');
	if (extension_index != -1) {
		FilenameBase = FilenameBase.Left (extension_index);
	}

	//
	//	Setup the temporary directory
	//	
	Make_Temp_Directory ();

	//
	//	Export the level files
	//
	Export_Level_File ();
	Export_Level_Data ();
	Export_Dependency_File ();
	Export_Definition_Databases (false);

	//
	//	Now build the mix file...
	//
	MixFileCreator.Generate_Mix_File (filename);

	//
	//	Cleanup any temp files we needed
	//
	Delete_Temp_Directory ();

	//
	//	Resetore the level after exporting
	//
	Post_Level_Export (node_list);

	//
	//	Cache the directory path we just exported into
	//
	theApp.WriteProfileString (CONFIG_KEY, LAST_EXPORT_DIR_VALUE, Path);
	return ;	
}


/////////////////////////////////////////////////////////
//
//	Make_Temp_Directory
//
/////////////////////////////////////////////////////////
void
ExporterClass::Make_Temp_Directory (void)
{
	//
	//	Get the path of the temp directory
	//
	char temp_dir[MAX_PATH] = { 0 };
	::GetTempPath (sizeof (temp_dir), temp_dir);

	CString temp_path = ::Make_Path (temp_dir, FilenameBase);
	
	//
	//	Try to find a unique temp directory to store our data
	//
	int index = 0;	
	do {		
		TempDirectory.Format ("%s%.2d.DIR", (const char *)temp_path, index++);
	} while (GetFileAttributes (TempDirectory) != 0xFFFFFFFF);

	//
	//	Create the directory
	//
	::CreateDirectory (TempDirectory, NULL);	
	::Set_Current_Directory (TempDirectory);
	::SetCurrentDirectory (TempDirectory);
	return ;
}


/////////////////////////////////////////////////////////
//
//	Delete_Temp_Directory
//
/////////////////////////////////////////////////////////
void
ExporterClass::Delete_Temp_Directory (void)
{
	//
	//	Change the current directory so we can remove the temporary one
	//
	::Set_Current_Directory (Path);
	::SetCurrentDirectory (Path);
	
	//
	//	Remove the temporary directory
	//
	Clean_Directory (TempDirectory);
	::RemoveDirectory (TempDirectory);
	return ;
}


/////////////////////////////////////////////////////////
//
//	Export_Dependency_File
//
/////////////////////////////////////////////////////////
void
ExporterClass::Export_Dependency_File (void)
{
	//
	//	Make a file that contains a list of all the W3D files that
	// were part of this export
	//
	StringClass dep_filename	= static_cast<const char *>(FilenameBase + ".dep");
	StringClass dep_path			= static_cast<const char *>(::Make_Path (TempDirectory, dep_filename));
	AssetDependencyManager::Save_Level_Dependencies (dep_path, m_AssetList);

	//
	//	Add this file to the mix
	//
	MixFileCreator.Add_File (dep_path, ::Get_Filename_From_Path (dep_path));
	return ;
}


/////////////////////////////////////////////////////////
//
//	Export_Level_File
//
/////////////////////////////////////////////////////////
bool
ExporterClass::Export_Level_File (void)
{
	//
	//	Create filenames for both the static and dynamic data
	//
	CString ldd_filename = FilenameBase + DYNAMIC_EXTENSION;
	CString lsd_filename = FilenameBase + STATIC_EXTENSION;

	//
	//	Save the LSD and LDD files to a temporary directory
	//
	SaveGameManager::Set_Map_Filename (lsd_filename);
	SaveGameManager::Save_Level ();
	SaveGameManager::Save_Game (ldd_filename, NULL);

	//
	//	Now add these files to the mix file
	//
	CString lsd_path = Make_Path (TempDirectory, lsd_filename);
	CString ldd_path = Make_Path (TempDirectory, ldd_filename);	
	MixFileCreator.Add_File (lsd_path, lsd_filename);
	MixFileCreator.Add_File (ldd_path, ldd_filename);
	return true;
}


/////////////////////////////////////////////////////////
//
//	Export_Always_Files
//
/////////////////////////////////////////////////////////
void
ExporterClass::Export_Always_Files (LPCTSTR path)
{
	CWaitCursor wait_cursor;
	m_AssetList.Delete_All ();

	//
	// Does the path exist?
	//
	if (::GetFileAttributes (path) != 0xFFFFFFFF) {

		//
		//	Get the base destination path
		//
		Path = path;
		FilenameBase = ALWAYS_ROOT_NAME;
		
		//
		//	Setup the temporary directory
		//	
		Make_Temp_Directory ();

		//
		// Copy all the files used by the framework to the directory
		//
		STRING_LIST &global_list = ::Get_File_Mgr ()->Get_Global_Include_File_List ();		
		Process_Include_Files (global_list);
				
		//
		//	Get the always dependency search list
		//
		STRING_LIST search_list;
		CString always_dep_search_ini = ::Get_File_Mgr ()->Make_Full_Path (ALWAYS_DEP_INI_PATH);
		::Get_File_Mgr ()->Build_Include_List (always_dep_search_ini, search_list);
		
		//
		//	Find all the files in the search list and add them to a list
		//
		STRING_LIST file_list;
		Build_File_List (search_list, file_list);

		//
		//	Now add all the W3D files from the list to another list. *sigh*
		//
		DynamicVectorClass<StringClass> dep_list;
		for (int index = 0; index < file_list.Count (); index ++) {
			CString &filename = file_list[index];
			if (::Is_W3D_Filename (filename)) {
				dep_list.Add ((LPCTSTR)::Get_Filename_From_Path (filename));
			}
		}

		//
		//	Make a file that contains a list of all these W3D files
		//
		AssetDependencyManager::Save_Always_Dependencies (TempDirectory, dep_list);
		CString dep_file_path = Make_Path (TempDirectory, "always.dep");
		MixFileCreator.Add_File (dep_file_path, "always.dep");

		//
		//	Now build the mix file...
		//
		CString mix_filename = Make_Path (Path, ALWAYS_DATA_FILENAME);
		MixFileCreator.Generate_Mix_File (mix_filename);

		//
		//	Cleanup any temp files we needed
		//
		Delete_Temp_Directory ();

	} else {

		//
		// Invalid file, let the user know
		//
		::Message_Box (::AfxGetMainWnd ()->m_hWnd, IDS_CANT_EXPORT_MSG, IDS_CANT_EXPORT_TITLE, MB_ICONERROR | MB_OK);
	}

	return ;	
}


/////////////////////////////////////////////////////////
//
//	Export_Level_Data
//
/////////////////////////////////////////////////////////
bool
ExporterClass::Export_Level_Data (void)
{
	bool retval = true;

	//
	// Loop through all the files used by this level and copy them
	// to the same directory that the level definition was saved in.
	//
	int count = ::Get_File_Mgr ()->Get_File_Count ();
	for (int file_index = 0; file_index < count; file_index ++) {

		//
		// Build the complete paths of both the original file location and the new file location
		//
		CString file_path  = ::Get_File_Mgr ()->Get_File_Name (file_index);
		CString entry_name;

		//
		//	Is this file's subdirectory important?
		//
		bool path_important = ::Get_File_Mgr ()->Is_File_Path_Important (file_index);
		if (path_important) {
			
			//
			//	Pre-pend the subdirectory name to form the entry name
			//
			CString sub_dir = ::Get_Subdir_From_Full_Path (file_path);
			entry_name = Make_Path (sub_dir, ::Get_Filename_From_Path (file_path));

		} else {
			entry_name = ::Get_Filename_From_Path (file_path);
		}

		//
		// Add this file to the mix file with the given name
		//
		MixFileCreator.Add_File (file_path, entry_name);

		//
		//	Add this file to the level-asset list if its a W3D file
		//
		if (::Is_W3D_Filename (entry_name)) {
			m_AssetList.Add ((LPCTSTR)::Get_Filename_From_Path (entry_name));
		}
	}

	//
	//	Add the temp definition database
	//
	CString temp_db_filename  = ::Get_File_Mgr ()->Make_Full_Path (TEMP_DB_PATH);
	CString temp_db_name		  = FilenameBase + ".ddb";
	MixFileCreator.Add_File (temp_db_filename, temp_db_name);

	//
	// Copy all the files used by the framework to the directory
	//
	STRING_LIST &level_list = ::Get_File_Mgr ()->Get_Include_File_List ();
	Process_Include_Files (level_list);
	return retval;
}


/////////////////////////////////////////////////////////
//
//	Export_Database_Mix
//
/////////////////////////////////////////////////////////
void
ExporterClass::Export_Database_Mix (LPCTSTR path)
{
	CWaitCursor wait_cursor;
	m_AssetList.Delete_All ();

	Path				= path;
	FilenameBase	= "always.dbs";

	//
	//	Setup the temporary directory
	//		
	Make_Temp_Directory ();

	//
	//	Export the definition databases
	//
	Export_Definition_Databases (true);

	//
	//	Cleanup any temp files we needed
	//
	Delete_Temp_Directory ();
	return ;
}


/////////////////////////////////////////////////////////
//
//	Export_Definition_Databases
//
/////////////////////////////////////////////////////////
bool
ExporterClass::Export_Definition_Databases (bool use_temp_strings_library)
{
	EditorMixFileCreator	always_db_mix_creator;

	//
	//	Save the preset library to the temp directory
	//
	PresetsFormClass *presets_form = ::Get_Presets_Form ();
	if (presets_form != NULL) {
		CString path = ::Make_Path (TempDirectory, "objects.ddb");
		presets_form->Save_Presets (path, 0, false, false);		
		always_db_mix_creator.Add_File (path, ::Get_Filename_From_Path (path));
	}
	
	//
	//	Add the strings table
	//
	if (use_temp_strings_library == false) {
		CString filename = ::Get_File_Mgr ()->Make_Full_Path (STRINGS_DB_PATH);
		always_db_mix_creator.Add_File (filename, ::Get_Filename_From_Path (filename));
	} else {		
		//
		//	Save the strings table to the temporary directory, then
		// add it to the mix file
		//
		CString path = ::Make_Path (TempDirectory, STRINGS_DB_NAME);
		StringsMgrClass::Save_Translation_Database (path);
		always_db_mix_creator.Add_File (path, STRINGS_DB_NAME);
	}

	//
	//	Add the global conversation database
	//
	CString filename = ::Get_File_Mgr ()->Make_Full_Path (CONV_DB_PATH);
	always_db_mix_creator.Add_File (filename, ::Get_Filename_From_Path (filename));

	//
	//	Now generate the definition databases mix file
	//
	CString mix_filename = ::Make_Path (Path, ALWAYS_DB_FILENAME);
	always_db_mix_creator.Generate_Mix_File (mix_filename);	
	return true;
}


/////////////////////////////////////////////////////////
//
//	Build_File_List
//
/////////////////////////////////////////////////////////
bool
ExporterClass::Build_File_List (const STRING_LIST &search_list, STRING_LIST &file_list)
{
	bool retval = true;

	//
	// Loop through all the files in our include list
	//
	for (int index = 0; index < search_list.Count (); index ++) {
		CString wildcard = search_list[index];
		CString search = ::Get_File_Mgr ()->Make_Full_Path (wildcard);
		
		//
		// Make sure we have all the assets for this always group
		//
		if (DontGetVersionsFromVSS == false) {
			::Get_File_Mgr ()->Get_Subproject (::Strip_Filename_From_Path (wildcard));
		}
		
		//
		//	Add the all the files that match the search mask to this list
		//
		Find_Files (search, file_list);		
	}

	return retval;
}


/////////////////////////////////////////////////////////
//
//	Find_Files
//
/////////////////////////////////////////////////////////
void
ExporterClass::Find_Files (const char *search_mask, STRING_LIST &file_list)
{
	//
	//	Get the directory where we'll be copying the files from
	//
	CString directory = ::Strip_Filename_From_Path (search_mask);
	CString mask		= ::Get_Filename_From_Path (search_mask);

	//
	// Find all files that match this wildcard
	//
	WIN32_FIND_DATA find_info = { 0 };
	BOOL keep_going = true;
	HANDLE hfile_find = ::FindFirstFile (search_mask, &find_info);
	for (;(hfile_find != INVALID_HANDLE_VALUE) && keep_going;
		  keep_going = ::FindNextFile (hfile_find, &find_info))
	{			
		//
		//	Don't do this if its a directory
		//
		if (!(find_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

			//
			//	Add this file to the list
			//
			CString full_path = Make_Path (directory, ::Get_Filename_From_Path (find_info.cFileName));
			file_list.Add (full_path);

		}
	}

	if (hfile_find != INVALID_HANDLE_VALUE) {
		::FindClose (hfile_find);
	}

	CString full_search_mask = Make_Path (directory, "*.*");

	//
	// Now recurse into any sub-directories
	//
	keep_going = true;
	for (hfile_find = ::FindFirstFile (full_search_mask, &find_info);
		  (hfile_find != INVALID_HANDLE_VALUE) && keep_going;
		  keep_going = ::FindNextFile (hfile_find, &find_info))
	{			
		//
		//	Don't do this if its a directory
		//
		if ((find_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (find_info.cFileName[0] != '.')) {

			//
			//	Build a search mask for this sub-directory
			//
			CString full_path	= Make_Path (directory, ::Get_Filename_From_Path (find_info.cFileName));
			full_path			= Make_Path (full_path, mask);

			//
			//	Recurse into this sub-directory
			//
			Find_Files (full_path, file_list);
		}
	}

	if (hfile_find != INVALID_HANDLE_VALUE) {
		::FindClose (hfile_find);
	}


	return ;
}


/////////////////////////////////////////////////////////
//
//	Process_Include_Files
//
/////////////////////////////////////////////////////////
void
ExporterClass::Process_Include_Files (const STRING_LIST &search_list)
{
	STRING_LIST file_list;
	Build_File_List (search_list, file_list);
	Export_Include_Files (file_list);
	return ;
}


/////////////////////////////////////////////////////////
//
//	Export_Include_Files
//
/////////////////////////////////////////////////////////
bool
ExporterClass::Export_Include_Files (const STRING_LIST &file_list)
{	
	//
	//	Add all the files from the list
	//
	for (int index = 0; index < file_list.Count (); index ++) {
		const CString &full_path = file_list[index];

		//
		// Add this file to the mix file
		//
		MixFileCreator.Add_File (full_path, ::Get_Filename_From_Path (full_path));

		//
		//	Add this file to the level-asset list if its a W3D file
		//
		if (::Is_W3D_Filename (full_path)) {
			m_AssetList.Add ((LPCTSTR)::Get_Filename_From_Path (full_path));
		}
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////
//
//  Delete_File
//
////////////////////////////////////////////////////////////////////////////
bool
ExporterClass::Delete_File (LPCTSTR filename)
{
	bool retval = false;

	ASSERT (filename != NULL);
	if (filename != NULL) {

		//
		// Strip the readonly bit off if necessary
		//
		DWORD attributes = ::GetFileAttributes (filename);
		if ((attributes != 0xFFFFFFFF) &&
			 ((attributes & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY))
		{
			::SetFileAttributes (filename, attributes & (~FILE_ATTRIBUTE_READONLY));
		}

		//
		// Perform the delete operation!
		//
		if ((attributes != 0xFFFFFFFF) &&
			 ((attributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY))
		{
			retval = (::RemoveDirectory (filename) != 0);
		} else {
			retval = (::DeleteFile (filename) != 0);
		}
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Clean_Directory
//
//////////////////////////////////////////////////////////////////////////////////
bool
ExporterClass::Clean_Directory (LPCTSTR local_dir)
{
	bool retval = true;

	//
	// Build a search mask from the directory
	//
	CString search_mask = CString (local_dir) + "\\*.*";	
	
	//
	// Loop through all the files in this directory and add them	
	// to our list
	//
	DynamicVectorClass<CString> file_list;
	BOOL keep_going = true;
	WIN32_FIND_DATA find_info = { 0 };
	HANDLE hfind = ::FindFirstFile (search_mask, &find_info);
	for (;(hfind != INVALID_HANDLE_VALUE) && keep_going;
		  keep_going = ::FindNextFile (hfind, &find_info))
	{
		
		//
		// If this file isn't a directory, add it to the list
		//
		if (!(find_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			CString filename = find_info.cFileName;
			file_list.Add (filename);
		} else if (find_info.cFileName[0] != '.') {
			
			//
			//	Recurse into this subdirectory
			//
			CString full_path = local_dir;
			::Delimit_Path (full_path);
			full_path += find_info.cFileName;
			Clean_Directory (full_path);
			
			//
			//	Add this directory to the list so it will get
			// deleted with the files...
			//
			CString filename = find_info.cFileName;
			file_list.Add (filename);
		}
	}

	//
	// Close the search handle
	//
	if (hfind != NULL) {
		::FindClose (hfind);
	}
	
	//
	//	Now loop through all the files and delete them
	//
	for (int index = 0; index < file_list.Count (); index ++) {
		CString &filename = file_list[index];
		CString full_path = ::Make_Path (local_dir, filename);
		Delete_File (full_path);
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Pre_Level_Export
//
//////////////////////////////////////////////////////////////////////////////////
void
ExporterClass::Pre_Level_Export (DynamicVectorClass<NodeClass *> &node_list)
{
	//
	//	Force static anim phys to be displayed
	//
	::Get_Scene_Editor ()->Display_Static_Anim_Phys (true);
	::Get_Scene_Editor ()->Display_Editor_Objects (true);

	//
	// Make sure the scenes are repartitioned
	// (gth) can't re-partition the object culling systems or
	// we'll lose hierarchical vis
	//
	::Get_Scene_Editor ()->Re_Partition_Static_Lights ();
	::Get_Scene_Editor ()->Re_Partition_Audio_System ();
	::Get_Scene_Editor ()->Set_Selection (NULL);
	::Get_Scene_Editor ()->Display_Vis_Points (false);	
	
	//
	//	Make sure the buildings are in a good state
	//
	GameObjManager::Update_Building_Collection_Spheres ();
	GameObjManager::Init_Buildings ();	

	//
	//	Let the nodes know we are about to start exporting them.
	// This way they can remove any 'editor-only' artifacts from
	// the system.
	//	
	NodeMgrClass::Build_Full_Node_List (node_list);
	for (int index = 0; index < node_list.Count (); index ++) {
		node_list[index]->Pre_Export ();
	}	

	//
	//	Force the pathfind system to use any available waypath data
	// in its evaluation
	//
	PathfindClass::Get_Instance ()->Generate_Waypath_Sectors_And_Portals ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Post_Level_Export
//
//////////////////////////////////////////////////////////////////////////////////
void
ExporterClass::Post_Level_Export (DynamicVectorClass<NodeClass *> &node_list)
{
	//
	//	Let the nodes know we're done exporting
	//
	for (int index = 0; index < node_list.Count (); index ++) {
		NodeClass *node = node_list[index];
		node->Post_Export ();
	}

	return ;
}


/////////////////////////////////////////////////////////
//
//	Export_Package
//
/////////////////////////////////////////////////////////
void
ExporterClass::Export_Package (LPCTSTR full_path)
{	
	CWaitCursor wait_cursor;

	//
	//	Save the preset changes to disk as necessary
	//
	if (PresetMgrClass::Are_Presets_Dirty ()) {
		PresetMgrClass::Check_In_Presets ();
	}

	//
	//	Get the base destination path
	//
	Path = ::Strip_Filename_From_Path (full_path);	

	//
	//	Strip the path and extension from the filename to give
	// us a base for use with other filenames
	//
	FilenameBase = ::Get_Filename_From_Path (full_path);
	int extension_index = FilenameBase.ReverseFind ('.');
	if (extension_index != -1) {
		FilenameBase = FilenameBase.Left (extension_index);
	}

	//
	//	Add all the files from our asset tree to this package
	//
	Add_Files_To_Mod_Package (AssetPackageMgrClass::Get_Current_Package_Path ());

	//
	//	Create the mix file...
	//
	MixFileCreator.Generate_Mix_File (full_path);

	//
	//	Cache the directory path we just exported into
	//
	theApp.WriteProfileString (CONFIG_KEY, LAST_EXPORT_DIR_VALUE, Path);
	return ;	
}


/////////////////////////////////////////////////////////
//
//	Add_Files_To_Mod_Package
//
/////////////////////////////////////////////////////////
void
ExporterClass::Add_Files_To_Mod_Package (LPCTSTR full_path)
{
	//
	//	Build a search path for all the files in this directory
	//
	CString search_path = ::Make_Path (full_path, "*.*");

	//
	//	Search for all the files in this directory
	//
	WIN32_FIND_DATA find_info	= { 0 };
	BOOL keep_going				= true;
	HANDLE file_find				= NULL;
	for (file_find = ::FindFirstFile (search_path, &find_info);
		 (file_find != INVALID_HANDLE_VALUE) && keep_going;
		  keep_going = ::FindNextFile (file_find, &find_info))
	{
		if (find_info.cFileName[0] != '.') {
			
			if ((find_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
				
				//
				//	Recurse into this sub-directory
				//
				CString subdir_path = ::Make_Path (full_path, find_info.cFileName);
				Add_Files_To_Mod_Package (subdir_path);

			} else {
				
				//
				//	Skip all lvl files
				//
				if (::strstr (find_info.cFileName, ".lvl") == NULL) {
					
					//
					//	Simply add this file to the mix
					//
					CString file_path = ::Make_Path (full_path, find_info.cFileName);
					MixFileCreator.Add_File (file_path, find_info.cFileName);
				}
			}
		}
	}

	//
	//	Close the search handle
	//
	if (file_find != INVALID_HANDLE_VALUE) {			  
		::FindClose (file_find); 
	}
	
	return ;	
}


/////////////////////////////////////////////////////////
//
//	Export_Level_Only
//
/////////////////////////////////////////////////////////
void
ExporterClass::Export_Level_Only (LPCTSTR path)
{	
	CWaitCursor wait_cursor;
	m_AssetList.Delete_All ();

	//
	//	Prepare the level for exporting...
	//
	DynamicVectorClass<NodeClass *> node_list;
	Pre_Level_Export (node_list);

	//
	//	Get the base destination path
	//
	Path				= ::Strip_Filename_From_Path (path);	
	TempDirectory	= Path;
	::Set_Current_Directory (TempDirectory);
	::SetCurrentDirectory (TempDirectory);

	//
	//	Strip the path and extension from the filename to give
	// us a base for use with other filenames
	//
	FilenameBase = ::Get_Filename_From_Path (path);
	int extension_index = FilenameBase.ReverseFind ('.');
	if (extension_index != -1) {
		FilenameBase = FilenameBase.Left (extension_index);
	}	

	//
	//	Export the level in game format
	//
	Export_Level_File ();

	//
	//	Add the temp definition database
	//
	CString temp_db_source  = ::Get_File_Mgr ()->Make_Full_Path (TEMP_DB_PATH);
	CString temp_db_dest		= ::Make_Path(Path,FilenameBase + ".ddb");
	::CopyFile(temp_db_source,temp_db_dest,false);

	//
	//	Resetore the level after exporting
	//
	Post_Level_Export (node_list);

	//
	//	Cache the directory path we just exported into
	//
	theApp.WriteProfileString (CONFIG_KEY, LAST_EXPORT_DIR_VALUE, Path);
	return ;
}
