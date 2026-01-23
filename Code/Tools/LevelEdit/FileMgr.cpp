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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/FileMgr.cpp                  $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/29/02 2:59p                                               $*
 *                                                                                             *
 *                    $Revision:: 36                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "filemgr.h"
#include "Utils.h"
#include "rendobj.h"
#include "matinfo.h"
#include "regkeys.h"
#include "LevelEdit.h"
#include "_assetmgr.h"
#include "editorassetmgr.h"
#include "updatingdbdialog.h"
#include "uniquelist.h"
#include "part_emt.h"
#include "filelocations.h"
#include "editorini.h"
#include "ffactory.h"
#include "ww3d.h"
#include "node.h"
#include "preset.h"
#include "presetmgr.h"
#include "spawnernode.h"
#include "wwstring.h"
#include "assetdatabase.h"
#include "editorbuild.h"
#include "assetpackagemgr.h"
#include "mixfiledatabase.h"

#ifndef PUBLIC_EDITOR_VER
#include "vssclass.h"
#endif


////////////////////////////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////////////////////////////
const char *SPECIAL_USER_NAME		= "Artist";
const char *SPECIAL_USER_FOLDER	= "<Artist Tests>";
const char *PROXY_TESTS_FOLDER	= "<Proxy Tests>";


////////////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////////////
bool FileMgrClass::_bAutoUpdateOn = false;


////////////////////////////////////////////////////////////////////////
//
//	FileMgrClass
//
////////////////////////////////////////////////////////////////////////
FileMgrClass::FileMgrClass (void)
	:	m_bVSSInitialized (false),
		m_bReadOnlyVSS (true),
		m_IsSpecialUser (false)
{

#ifdef PUBLIC_EDITOR_VER

	//
	//	Allocate the database interface layer we'll need to access assets
	//
	m_DatabaseInterface = new MixFileDatabaseClass;

	//
	// Get the asset directory from the asset package manager
	//
	m_BasePath = AssetPackageMgrClass::Get_Current_Package_Path ();	

#else
	
	//
	//	Allocate the database interface layer we'll need to access assets
	//
	m_DatabaseInterface = new VSSClass;

	//
	// Read the asset directory from the registry
	//
	m_BasePath = theApp.GetProfileString (CONFIG_KEY, ASSET_DIR_VALUE);

#endif //PUBLIC_EDITOR_VER

	//
	// Ensure the base has a directory delimiter on the end
	//
	if ((m_BasePath.GetLength () > 0) &&
	     m_BasePath[::lstrlen (m_BasePath) - 1] != '\\')
	{
		m_BasePath += "\\";
	}

	//
	// The new cache directory is simply a subdirectory of the base
	//
	m_TextureCachePath = m_BasePath + "EditorCache";
	if (::GetFileAttributes (m_TextureCachePath) == 0xFFFFFFFF) {
		::CreateDirectory (m_TextureCachePath, NULL);
	}

	return ;	
}


////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////////////
void
FileMgrClass::Initialize (void)
{
	Free_Data ();
	return ;
}


////////////////////////////////////////////////////////////////////////
//
//	Free_Data
//
////////////////////////////////////////////////////////////////////////
void
FileMgrClass::Free_Data (void)
{	
	// Loop through all the files in our array
	for (int index = 0; index < m_FileList.Count (); index ++) {
			
		// Free this entry
		LevelFileStruct *pentry = m_FileList[index];
		SAFE_DELETE (pentry);
	}
		
	// Remove all the entries from our list.
	m_FileList.Delete_All ();	
	return ;
}


////////////////////////////////////////////////////////////////////////
//
//	Make_Full_Path
//
////////////////////////////////////////////////////////////////////////
CString
FileMgrClass::Make_Full_Path (LPCTSTR rel_path)
{
	CString full_path = rel_path;

	// Is this path really relative?
	if (::Is_Path_Relative (rel_path)) {
		
		// Strip of the preceding directory delimiter if there is one.
		if ((full_path.GetLength () > 0) && (full_path[0] == '\\')) {
			full_path = &(((LPCTSTR)full_path)[1]);
		}

		// Return the full path
		full_path = m_BasePath + rel_path;
		
		// Strip off the last delimiter if necessary
		if (full_path[::lstrlen (full_path) - 1] == '\\') {
			full_path = full_path.Left (full_path.GetLength () - 1);
		}
	}

	// Return a string which contains the full path to the data file
	return full_path;
}


////////////////////////////////////////////////////////////////////////
//
//	Make_Relative_Path
//
////////////////////////////////////////////////////////////////////////
CString
FileMgrClass::Make_Relative_Path (LPCTSTR full_path)
{
	CString base_path = m_BasePath;
	CString lower_path = full_path;
	lower_path.MakeLower ();
	base_path.MakeLower ();
	LPCTSTR rel_path = lower_path;

	// Is our 'base' part of the full path?
	if (::strstr (rel_path, base_path) == rel_path) {

		// The relative path is the full path minus the base path
		rel_path += m_BasePath.GetLength ();
		
		// Strip of the preceding directory delimiter if there is one.
		if (rel_path[0] == '\\') {
			rel_path = &(((LPCTSTR)rel_path)[1]);
		}
	}

	// Return a string which contains the relative path to the data file
	return CString (rel_path);
}


////////////////////////////////////////////////////////////////////////
//
//	Is_Empty_Path
//
////////////////////////////////////////////////////////////////////////
bool
FileMgrClass::Is_Empty_Path (LPCTSTR path)
{
	bool retval = true;

	if (path[0] != 0) {
		CString temp1 = path;
		CString temp2 = m_BasePath;
		::Delimit_Path (temp1);
		::Delimit_Path (temp2);

		retval = bool(::stricmp (temp1, temp2) ==  0);
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////
//
//	Is_Path_Valid
//
////////////////////////////////////////////////////////////////////////
bool
FileMgrClass::Is_Path_Valid (LPCTSTR path)
{
	// The path is valid if its located under the asset tree
	return bool(::strnicmp (path, m_BasePath, m_BasePath.GetLength ()) == 0);
}


////////////////////////////////////////////////////////////////////////
//
//	Is_Path_In_Asset_Tree
//
////////////////////////////////////////////////////////////////////////
bool
FileMgrClass::Is_Path_In_Asset_Tree (LPCTSTR path)
{
	CString full_path = Make_Full_Path (path);		
	return bool(::strnicmp (full_path, m_BasePath, m_BasePath.GetLength ()) == 0);
}


////////////////////////////////////////////////////////////////////////
//
//	Does_File_Exist
//
////////////////////////////////////////////////////////////////////////
bool
FileMgrClass::Does_File_Exist
(
	LPCTSTR	filename,
	[[maybe_unused]] bool		update_from_vss
)
{
	// Assume it doesn't exist
	bool file_exists = false;

	// Param valid?
	ASSERT (filename != NULL);
	if (filename != NULL) {

		// Ensure the path is complete
		CString path = Make_Full_Path (filename);
		file_exists = bool(::GetFileAttributes (path) != 0xFFFFFFFF);

#ifndef PUBLIC_EDITOR_VER

		//
		// Should we try to get this file from VSS?
		//
		if (update_from_vss && (file_exists == false)) {
			
			//
			// If the file is in VSS, then get it and return the success code
			//
			if (Is_File_In_VSS (path)) {
				file_exists = Update_File (path);
			}
		}

#endif //!PUBLIC_EDITOR_VER

	}

	// Return the true/false result code
	return file_exists;
}


/////////////////////////////////////////////////////////////////
//
//	Find_File
//
/////////////////////////////////////////////////////////////////
int
FileMgrClass::Find_File (LPCTSTR filename) const
{
	// Assume failure
	int index = -1;

	// Can we add another property to our list?
	ASSERT (filename != NULL);
	if (filename != NULL) {

		// Loop through all the files in our array
		for (int file_index = 0;
			  (file_index < m_FileList.Count ()) && (index == -1);
			  file_index ++) {

			// Was this the file we were looking for?
			if (::lstrcmpi (m_FileList[file_index]->m_Filename, filename) == 0) {
				index = file_index;
			}
		}
	}

	// Return the file's index (-1) on error
	return index;
}


/////////////////////////////////////////////////////////////////
//
//	Remove_File
//
/////////////////////////////////////////////////////////////////
bool
FileMgrClass::Remove_File (int index)
{
	// Assume failure
	bool retval = false;

	// Param valid?
	if ((index >= 0) && (index < m_FileList.Count ())) {
		
		// Decrement the ref count on this entry
		LevelFileStruct *pentry = m_FileList[index];
		pentry->m_RefCount --;

		// Do we need to remove this item from the list?
		if (pentry->m_RefCount == 0) {
			
			// Free the memory used by this entry and remove it
			// from the list
			SAFE_DELETE (pentry);
			m_FileList.Delete (index);
		}

		// Success!
		retval = true;
	}

	// Return the true/false result code
	return retval;
}


/////////////////////////////////////////////////////////////////
//
//	Add_Files_To_Database
//
/////////////////////////////////////////////////////////////////
bool
FileMgrClass::Add_Files_To_Database (LPCTSTR filename)
{
	// Assume success
	//bool retval = true;
	ASSERT (filename != NULL);

	m_CurrentFile = filename;
	Add_Files_To_Database ();

	// Return the true/false success code
	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Add_File_To_Database
//
/////////////////////////////////////////////////////////////////
bool
FileMgrClass::Add_File_To_Database (LPCTSTR filename)
{
	CString path = Make_Full_Path (filename);
	if (Does_File_Exist (path) &&
		 (m_DatabaseInterface->Does_File_Exist (path) == false))
	{
		m_DatabaseInterface->Add_File (path);
	}

	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Add_Files_To_Database
//
/////////////////////////////////////////////////////////////////
void
FileMgrClass::Add_Files_To_Database (void)
{
	// Build a unique list of dependencies on this W3D file
	UniqueListClass<CString> unique_list;

	//
	//	If this is a W3D model, enumerate its dependencies, otherwise
	// just add it to the list
	//
	if (::Is_W3D_Filename (m_CurrentFile)) {
		Build_Dependency_List (m_CurrentFile, unique_list);
	} else {
		unique_list.Add (Make_Full_Path (m_CurrentFile));
	}

	CString current_dir = ::Strip_Filename_From_Path (m_CurrentFile);
	current_dir = Make_Full_Path (current_dir);
	
	//
	// Now loop through all the files in the unique list and
	// add them to VSS.
	//
	for (int index = 0; index < unique_list.Count (); index ++) {					
		CString full_path = unique_list[index];
		
		//
		//	Does this file live outside of our asset tree?
		//
		if (Is_Path_In_Asset_Tree (full_path) == false) {
			
			CString filename		= ::Get_Filename_From_Path (full_path);
			CString local_path	= ::Make_Path (current_dir, filename);

			//
			//	Copy the file to the asset tree
			//
			if (::Quick_Compare_Files (full_path, local_path) != 0) {
				::Copy_File (full_path, local_path, false);
			}
			
			full_path = local_path;
		}
		
		//
		//	Add this file to VSS (if needs be)
		//
		if (Does_File_Exist (full_path) &&
			 (m_DatabaseInterface->Does_File_Exist (full_path) == false))
		{
			m_DatabaseInterface->Add_File (full_path);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Add_File
//
/////////////////////////////////////////////////////////////////
int
FileMgrClass::Add_File (LPCTSTR filename, bool subdir_important)
{
	// Assume failure
	int index = -1;

	// Param valid?
	ASSERT (filename != NULL);
	if ((filename != NULL) && (index == -1)) {

		// Ensure the path is complete
		CString path = Make_Full_Path (filename);
		LevelFileStruct *pentry = NULL;

		// Is this file already in the list?
		index = Find_File (path);
		if (index == -1) {
			
			// Allocate a new entry for our list
			pentry = new LevelFileStruct;
			ASSERT (pentry != NULL);

			// Set the file information
			pentry->m_Filename = path;
			pentry->m_RefCount = 0;
			
			// Add this entry to the list
			m_FileList.Add (pentry);
			index = Find_File (path);
		} else {
			pentry = m_FileList[index];
		}

		//
		//	Record whether or not we think the file's subdirectory
		// is important.
		//
		pentry->m_SubdirImportant |= subdir_important;

		// Increment the refcount on this entry
		pentry->m_RefCount ++;
	}

	// Return the index of the file
	return index;
}


/////////////////////////////////////////////////////////////////
//
//	Update_Texture_Cache
//
/////////////////////////////////////////////////////////////////
bool
FileMgrClass::Update_Texture_Cache
(
	LPCTSTR start_path,
	RenderObjClass *prender_obj
)
{
	// Assume failure
	bool retval = false;

	ASSERT (prender_obj != NULL);
	if (prender_obj != NULL) {
		
		// Assume success from here on out
		retval = true;

		// Generate a list of all the texture files required by this render object
		DynamicVectorClass<CString> texture_list;
		int count = Build_Texture_List (start_path, prender_obj, texture_list);

		// Loop through all the textures in our list and make sure the latest
		// version is in the cache directory
		for (int texture = 0; texture < count; texture ++) {
											
			// Should we now copy the texture file over?
			CString texture_file = texture_list[texture];
			if (::GetFileAttributes (texture_file) != 0xFFFFFFFF) {
				
				// Is the texture file newer than the one we have in the cache?
				CString cache_path = m_TextureCachePath + CString ("\\") + ::Get_Filename_From_Path (texture_file);
				if (::Quick_Compare_Files (texture_file, cache_path) > 0) {
					
					// Everything is in order, so copy the texture file to the cache
					retval &= ::Copy_File (texture_file, cache_path, false);
				}
			}
		}
	}
	
	// Return the true/false result code
	return retval;
}


/////////////////////////////////////////////////////////////////
//
//	Build_Texture_List
//
/////////////////////////////////////////////////////////////////
int
FileMgrClass::Build_Texture_List
(
	LPCTSTR start_path,
	RenderObjClass *prender_obj,
	DynamicVectorClass<CString> &texture_list,
	bool /* brecurse */
)
{
	// Param valid?
	ASSERT (prender_obj != NULL);
	if (prender_obj) {

		CString path = start_path;
		::Delimit_Path (path);

		// Get a list of texture files from the render object
		DynamicVectorClass<StringClass> dependency_list;
		prender_obj->Build_Texture_List (dependency_list);

		// Loop through all the texture files and try to assign paths to each one.
		for (int index = 0; index < dependency_list.Count (); index ++) {
			
			// Build a full path to the current texture file
			const char *filename = dependency_list[index];
			CString full_path = path + CString (filename);
			full_path = ::Get_File_Mgr ()->Make_Full_Path (full_path);

			// Does the texture file exist in this location?
			if (Does_File_Exist (full_path) == false) {

				// Assume the texture file exists in the parent directory
				full_path = ::Up_One_Directory (::Strip_Filename_From_Path (full_path));
				::Delimit_Path (full_path);
				full_path += filename;
			}

			// Add this texture file to the list
			texture_list.Add (full_path);
		}

		dependency_list.Delete_All ();
	}

	// Return the count of textures in the list
	return texture_list.Count ();
}


/////////////////////////////////////////////////////////////////
//
//	Determine_File_Location
//
/////////////////////////////////////////////////////////////////
bool
FileMgrClass::Determine_File_Location (LPCTSTR full_path, CString &real_location)
{
	bool retval	= false;

	//
	// Does the file exist in this location?
	//
	if (Does_File_Exist (full_path) == false) {

		CString filename		= ::Get_Filename_From_Path (full_path);
		CString parent_dir	= ::Up_One_Directory (::Strip_Filename_From_Path (full_path));
		CString parent_path	= ::Make_Path (parent_dir, filename);

		//
		//	Does the file exist in the parent directory?
		//
		if (Does_File_Exist (parent_path) == false) {
			
			if (::Is_Texture_Filename (filename)) {
				
				//
				//	Does this texture file exist in the global texture directory?
				//
				CString texture_path = ::Make_Path (GLOBAL_TEXTURE_PATH, filename);
				if (Does_File_Exist (texture_path)) {
					real_location	= texture_path;
					retval			= true;
				}
			}

		} else {
			real_location	= parent_path;
			retval			= true;
		}

	} else {
		real_location	= full_path;
		retval			= true;
	}
	
	return retval;
}


/////////////////////////////////////////////////////////////////
//
//	Build_File_List
//
/////////////////////////////////////////////////////////////////
void
FileMgrClass::Build_File_List
(
	LPCTSTR				start_path,
	RenderObjClass *	render_obj,
	FILE_LIST &			file_list
)
{
	CString path = start_path;
	::Delimit_Path (path);

	// Get a list of dependent files from the render object
	DynamicVectorClass<StringClass> dependency_list;
	render_obj->Build_Dependency_List (dependency_list);

	// Loop through all the files and try to assign paths to each one.
	for (int index = 0; index < dependency_list.Count (); index ++) {
		
		// Build a full path to the current file
		const char *filename		= dependency_list[index];
		CString full_path			= ::Make_Path (path, filename);
		full_path					= Make_Full_Path (full_path);

		//
		//	Record that the subdir where this file lives is important
		// IF the subdir is returned by the render object.
		//
		bool subdir_important = (::strstr (filename, "+\\") != NULL);

		//
		//	Attempt to find the file
		//
		CString real_location = full_path;
		Determine_File_Location (full_path, real_location);

		//
		// Add this file to the list
		//
		file_list.Add (FileInfoStruct (real_location, subdir_important));
	}

	dependency_list.Delete_All ();
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Initialize_VSS
//
/////////////////////////////////////////////////////////////////
bool
FileMgrClass::Initialize_VSS
(
	LPCTSTR ini_file_path,
	LPCTSTR username,
	LPCTSTR password
)
{
	// Do we need to initialize the VSS database?
	if (m_bVSSInitialized == false) {

		// Attempt to open the database
		m_bVSSInitialized = m_DatabaseInterface->Open_Database (ini_file_path,
																		  username ? username : "User",
																		  password ? password : "");
		
		m_bReadOnlyVSS = (m_DatabaseInterface->Is_Read_Only ());

		//
		//	Is this a special user?  A special user is someone who doesn't
		// have full write access to the database.
		//
		if (::lstrcmpi (username, SPECIAL_USER_NAME) == 0) {
			m_IsSpecialUser = true;
		}
	}

	// Return the true/false result code
	return m_bVSSInitialized;
}


/////////////////////////////////////////////////////////////////
//
//	Is_File_In_VSS
//
/////////////////////////////////////////////////////////////////
bool
FileMgrClass::Is_File_In_VSS (LPCTSTR filename)
{
	// Assume failure
	bool retval = false;

	// State OK?
	if (m_bVSSInitialized) {
		retval = m_DatabaseInterface->Does_File_Exist (filename);
	}

	// Return the true/false result code
	return retval;
}


/////////////////////////////////////////////////////////////////
//
//	Build_Update_List
//
/////////////////////////////////////////////////////////////////
int
FileMgrClass::Build_Update_List (DynamicVectorClass<LevelFileStruct *> &update_list)
{
	int count = 0;

	// State OK?
	ASSERT (m_bVSSInitialized);
	if (m_bVSSInitialized) {

		// Loop through all the files in our array
		for (int index = 0;
			  index < m_FileList.Count ();
			  index ++) {
				
			// Get a pointer to this entry
			LevelFileStruct *pentry = m_FileList[index];
			
			// Ask VSS if we need to update this file.
			if (m_DatabaseInterface->Is_File_Different (pentry->m_Filename)) {
				update_list.Add (pentry);
			}
		}		
	}

	// Return the count of items we added to the list
	return count;
}


/////////////////////////////////////////////////////////////////
//
//	Update_Files
//
/////////////////////////////////////////////////////////////////
bool
FileMgrClass::Update_Files (DynamicVectorClass<LevelFileStruct *> *pupdate_list)
{
	// Asssume success
	bool retval = true;

	ASSERT (m_bVSSInitialized);

	// Do we need to build a list of files to generate ourselves?
	DynamicVectorClass<LevelFileStruct *> *plist = pupdate_list;
	if (plist == NULL) {
		
		// Create a new list object
		plist = new DynamicVectorClass<LevelFileStruct *>;
		ASSERT (plist != NULL);

		// Generate a list of files that need to be updated from VSS
		Build_Update_List (*plist);
	}

	// Loop through all the files in our array
	for (int index = 0;
		  index < plist->Count ();
		  index ++) {
			
		// Get a pointer to this entry
		LevelFileStruct *pentry = (*plist)[index];
		
		//
		// Get the latest version from VSS if we don't already have the file
		// checked out.
		//
		if (m_DatabaseInterface->Get_File_Status (pentry->m_Filename) != AssetDatabaseClass::CHECKED_OUT_TO_ME) {
			retval &= m_DatabaseInterface->Get (pentry->m_Filename);
		}
	}

	// Free the temporary list if we need to
	if (plist != pupdate_list) {
		SAFE_DELETE (plist);
	}

	// Return the true/false result code
	return retval;
}


/////////////////////////////////////////////////////////////////
//
//	Update_File
//
/////////////////////////////////////////////////////////////////
bool
FileMgrClass::Update_File (int index)
{
	// Asssume failure
	bool retval = false;

	// Param valid?
	if ((index >= 0) && (index < m_FileList.Count ())) {
		
		// Update the file using its filename
		retval = Update_File (m_FileList[index]->m_Filename);
	}
	
	// Return the true/false result code
	return retval;
}


/////////////////////////////////////////////////////////////////
//
//	Update_File
//
/////////////////////////////////////////////////////////////////
bool
FileMgrClass::Update_File (LPCTSTR filename)
{
	// Asssume failure
	bool retval = false;

	// State valid?
	ASSERT (filename != NULL);
	ASSERT (m_bVSSInitialized);
	if ((filename != NULL) && m_bVSSInitialized) {
		
		// Assume success from here on out
		retval = true;

		//
		// Ensure the path is complete for the file
		//
		CString path = Make_Full_Path (filename);

		//
		// Get the latest version from VSS if we don't already have the file
		// checked out.
		//
		if (	::GetFileAttributes (path) == 0xFFFFFFFF ||
				m_DatabaseInterface->Get_File_Status (filename) != AssetDatabaseClass::CHECKED_OUT_TO_ME)
		{
			retval = m_DatabaseInterface->Get (path);
		}		
	}
	
	// Return the true/false result code
	return retval;
}


/////////////////////////////////////////////////////////////////
//
//	Update_All_Files
//
/////////////////////////////////////////////////////////////////
bool
FileMgrClass::Update_All_Files (LPCTSTR dest_path, LPCTSTR search_mask)
{
	bool retval = false;

	// State valid?
	ASSERT (dest_path != NULL);
	ASSERT (m_bVSSInitialized);
	if ((dest_path != NULL) && m_bVSSInitialized) {
		
		//
		// Assume success from here on out
		//
		retval = m_DatabaseInterface->Get_All (dest_path, search_mask);
	}
	
	return retval;
}


/////////////////////////////////////////////////////////////////
//
//	Get_Subproject
//
/////////////////////////////////////////////////////////////////
bool
FileMgrClass::Get_Subproject (LPCTSTR path)
{
	// Asssume failure
	bool retval = false;

	// State valid?
	ASSERT (path != NULL);
	ASSERT (m_bVSSInitialized);
	if ((path != NULL) && m_bVSSInitialized) {	

		// Ensure the path is complete for the file
		CString full_path = Make_Full_Path (path);

		// Get the files from VSS
		retval = m_DatabaseInterface->Get (full_path);
	}

	// Return the true/false result code
	return retval;
}


/////////////////////////////////////////////////////////////////
//
//	Set_Base_Path
//
/////////////////////////////////////////////////////////////////
void
FileMgrClass::Set_Base_Path (LPCTSTR base)
{
	// Store this base both in memory and in the registry
	m_BasePath = base;
	theApp.WriteProfileString (CONFIG_KEY, ASSET_DIR_VALUE, m_BasePath);
	
	// Ensure the directory is created
	if (Does_File_Exist (m_BasePath) == false) {
		::CreateDirectory (base, NULL);
	}

	// Ensure the new base has a directory delimiter on the end
	if (m_BasePath[::lstrlen (m_BasePath) - 1] != '\\') {
		m_BasePath += "\\";
	}

	// The new cache directory is simply a subdirectory of the base
	m_TextureCachePath = m_BasePath + "EditorCache";
	if (::GetFileAttributes (m_TextureCachePath) == 0xFFFFFFFF) {
		::CreateDirectory (m_TextureCachePath, NULL);
	}

	//
	// Add the texture cache to the search path
	//
	EditorFileFactoryClass::Add_Search_Path (m_TextureCachePath);
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Find_Files
//
/////////////////////////////////////////////////////////////////
void
FileMgrClass::Find_Files
(
	LPCTSTR								start_path,
	LPCTSTR								wildcard,
	DynamicVectorClass<CString> &	file_list,
	bool									recurse
)
{	
	CString path = start_path;
	if (path[::lstrlen(path)-1] != '\\') {
		path += "\\";
	}

	// Build a file spec from the starting path and the wildcard
	CString file_spec = path + wildcard;

	// Find all files that match this wildcard
	WIN32_FIND_DATA find_info = { 0 };
	BOOL bcontinue = true;
	HANDLE hfile_find = ::FindFirstFile (file_spec, &find_info);
	for (;(hfile_find != INVALID_HANDLE_VALUE) && bcontinue;
		  bcontinue = ::FindNextFile (hfile_find, &find_info)) {

		// Is this a file?
		if (!(find_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			
			// Add this file to our list
			CString filename = path + find_info.cFileName;
			file_list.Add (filename);			
		}
	}
	
	// Close the search handle
	::FindClose (hfile_find);
	hfile_find = INVALID_HANDLE_VALUE;

	if (recurse) {
	
		// Process all the subdirs
		file_spec = path + "*.*";
		bcontinue = true;
		for (hfile_find = ::FindFirstFile (file_spec, &find_info);
			  (hfile_find != INVALID_HANDLE_VALUE) && bcontinue;
			  bcontinue = ::FindNextFile (hfile_find, &find_info)) {

			// If this is a subdir, then recursively build the file list
			if ((find_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
			    (find_info.cFileName[0] != '.')){

				CString sub_dir = path + find_info.cFileName;
				Find_Files (sub_dir, wildcard, file_list, recurse);
			}
		}

		// Close the search handle
		::FindClose (hfile_find);
		hfile_find = INVALID_HANDLE_VALUE;
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Update_Model
//
/////////////////////////////////////////////////////////////////
bool
FileMgrClass::Update_Model
(
	LPCTSTR local_path,
	LPCTSTR new_filename
)
{
	// Assume failure
	bool retval = false;
	CWaitCursor wait_cursor;

	//
	//	Display a dialog so the user doesn't close down the app
	//
	HWND hmain_wnd		= ::AfxGetMainWnd ()->m_hWnd;
	HWND hupdate_dlg	= ::Show_VSS_Update_Dialog (hmain_wnd);

	CString new_path			= ::Strip_Filename_From_Path (new_filename);
	CString local_path_temp	= local_path;
	::Delimit_Path (local_path_temp);
	::Delimit_Path (new_path);

	bool path_different = bool(::lstrcmpi (new_path, local_path_temp) != 0);

	//
	// Build a unique list of dependencies on this W3D file
	//
	UniqueListClass<CString> file_list;
	Build_Dependency_List (new_filename, file_list);	

	//
	// Loop through all the files in our list
	//
	for (int index = 0; index < file_list.Count (); index ++) {
		
		// Get the full path, and its 2 components, of the current file.
		CString &full_path	= file_list[index];
		CString path			= ::Strip_Filename_From_Path (full_path);
		CString filename		= ::Get_Filename_From_Path (full_path);

		// The local version (in our asset tree) of this file should exist here.
		CString local_file = local_path_temp + filename;

		//
		//	Check to see if we need to maintain this file's subdirectory...
		//
		bool subdir_important = (::strstr (full_path, "+\\") != NULL);
		if (subdir_important) {

			//
			//	Ensure the subdirectory exists
			//
			CString sub_dir = local_path_temp + ::Get_Subdir_From_Full_Path (full_path);
			::Create_Dir_If_Necessary (sub_dir);

			//
			//	Build a path where the destination file lives in the sub-dir
			//
			local_file = sub_dir + CString ("\\") + filename;
		}
		
		//
		// Attempt to check out the latest version of this file
		//
		bool need_checkin = m_DatabaseInterface->Check_Out (local_file, false);

		//
		// If this file is newer than its local counterpart, copy it!
		//
		//if (path_different && ::Quick_Compare_Files (full_path, local_file) > 0) {
		if (path_different) {
			::Copy_File (full_path, local_file, true);
		}

		//
		// Should we add this file to VSS, or update the existing?
		//
		if (need_checkin == false) {
			m_DatabaseInterface->Add_File (local_file);
		} else {
			m_DatabaseInterface->Check_In (local_file);
		}
	}

	// Remove the updating dialog
	::Kill_VSS_Update_Dialog (hupdate_dlg);
	return retval;
}


/////////////////////////////////////////////////////////////////
//
//	Build_Dependency_List
//
/////////////////////////////////////////////////////////////////
void
FileMgrClass::Build_Dependency_List
(
	LPCTSTR							asset_filename,
	UniqueListClass<CString> &	list
)
{
	CString full_path		= Make_Full_Path (asset_filename);
	CString path			= ::Strip_Filename_From_Path (full_path);
	CString filename		= ::Get_Filename_From_Path (asset_filename);	
	CString asset_name	= ::Asset_Name_From_Filename (filename);

	// Create an instance of the model so we can enumerate all the other
	// files this model is dependant on
	_pThe3DAssetManager->Set_Current_Directory (path);
	RenderObjClass *render_obj = _pThe3DAssetManager->Create_Render_Obj (asset_name);
	if (render_obj != NULL) {

		// Build a list of all the files used by this render object
		FILE_LIST file_list;
		Build_File_List (path, render_obj, file_list);

		// Make a unique list out of the file list
		for (int index = 0; index < file_list.Count (); index ++) {
			FileInfoStruct &info = file_list[index];
			list.Add_Unique (info.m_Filename);
		}

		// Free the temporary render object
		MEMBER_RELEASE (render_obj);
	} else {
		
		//
		//	This wasn't a render object file (could of been an animation), so
		// just add the file to the list (no other dependencies).
		//
		list.Add (full_path);
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Update_Asset_Tree
//
/////////////////////////////////////////////////////////////////
bool
FileMgrClass::Update_Asset_Tree (void)
{	
	return m_DatabaseInterface->Get_Subproject (m_BasePath);
}


/////////////////////////////////////////////////////////////////
//
//	Build_Global_Include_List
//
void
FileMgrClass::Build_Global_Include_List (void)
{
	// Ensure we have the latest copy from VSS
	Update_File (ALWAYS_INI_PATH);

	// Build the global include list
	Build_Include_List (ALWAYS_INI_PATH, m_GlobalIncludeFiles);
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Build_Include_List
//
/////////////////////////////////////////////////////////////////
void
FileMgrClass::Build_Include_List
(
	LPCTSTR ini_filename,
	DynamicVectorClass<CString> &list
)
{
	// Start with a fresh list of files
	list.Delete_All ();

	// Get a pointer to the INI file from the asset manager
	CString ini_path = Make_Full_Path (ini_filename);
	EditorINIClass *pini = _pThe3DAssetManager->Get_INI (ini_path);
	
	// Were we successful in getting a pointer to the INI file?
	ASSERT (pini != NULL);
	if (pini != NULL) {

		// Loop through all the assets in the ini file
		int instance_count =  pini->Entry_Count ("Assets");
		for (int index = 0; index < instance_count; index ++) {			
			
			// Get the current file spec from the INI
			TCHAR filespec[MAX_PATH];
			pini->Get_String ("Assets",
									pini->Get_Entry ("Assets", index),
									"",
									filespec,
									sizeof (filespec));

			// Add the filespec to our list
			list.Add (filespec);
		}
		
		// Free the INI object
		SAFE_DELETE (pini);					
	}
		
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Update_Global_Include_File_List
//
/////////////////////////////////////////////////////////////////
bool
FileMgrClass::Update_Global_Include_File_List (void)
{
	bool retval = false;

	// Attempt to checkout the INI file
	CString ini_path = Make_Full_Path (ALWAYS_INI_PATH);
	if (m_DatabaseInterface->Retry_Check_Out (ini_path, 10, 1000)) {

		// Create the INI file from our in-memory list
		EditorINIClass ini_file;
		for (int index = 0; index < m_GlobalIncludeFiles.Count (); index ++) {
			CString index_string;
			index_string.Format ("%d", index + 1);
			ini_file.Put_String ("Assets", index_string, (LPCTSTR)m_GlobalIncludeFiles[index]);
		}

		// Save the INI file
		FileClass * pini_file = _TheFileFactory->Get_File (ini_path);
		if (pini_file) {
			ini_file.Save (*pini_file);
			_TheFileFactory->Return_File (pini_file);
		}

		// Now check the INI file back in
		retval = m_DatabaseInterface->Retry_Check_In (ini_path, 10, 1000);
	}

	// Reutrn the true/false result code
	return retval;
}


/////////////////////////////////////////////////////////////////
//
//	Update
//
/////////////////////////////////////////////////////////////////
void
FileMgrClass::Update (NodeClass *node, bool add_node)
{
	//
	//	Update the files this preset is dependent on
	//
	PresetClass *preset = node->Get_Preset ();
	if (preset != NULL) {
		Update (preset, add_node);
	}

	//
	//	If this is a spawner, then update the files
	// its spawned object is dependent on as well.
	//
	if (node->Get_Type () == NODE_TYPE_SPAWNER) {

		SpawnerDefClass *definition = static_cast<SpawnerDefClass *> (preset->Get_Definition ());
		if (definition != NULL) {

			//
			//	Loop over all the possible spawned objects and make sure to load their assets
			//
			const DynamicVectorClass<int> &list = definition->Get_Spawn_Definition_ID_List ();
			for (int index = 0; index < list.Count (); index ++) {
				PresetClass *preset = PresetMgrClass::Find_Preset (list[index]);
				if (preset != NULL) {
					Update (preset, add_node);
				}
			}
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Update
//
/////////////////////////////////////////////////////////////////
void
FileMgrClass::Update (PresetClass *preset, bool add_node)
{
	//
	//	Get the list of dependencies from the preset
	//
	STRING_LIST file_list;
	preset->Get_All_Dependencies (file_list);
	
	//
	//	Loop through all the files this preset is dependent on
	//
	for (int index = 0; index < file_list.Count (); index ++) {			
		CString filename = file_list[index];
	
		if (Is_Empty_Path (filename) == false)
		{
			if (add_node) {
				Add_Asset (filename);
			} else {
				Remove_Asset (filename);
			}
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Add_Asset
//
/////////////////////////////////////////////////////////////////
void
FileMgrClass::Add_Asset (LPCTSTR filename)
{
	RenderObjClass *render_obj = NULL;

	//
	//	If this is a render object, then build a list of
	// dependencies
	//
	if (::Is_W3D_Filename (filename)) {		
		CString asset_name	= ::Asset_Name_From_Filename (filename);
		render_obj				= ::Create_Render_Obj (asset_name);
	}

	if (render_obj != NULL) {

		//
		// Build a list of all the files used by this render object
		//
		CString full_path		= Make_Full_Path (filename);		
		CString start_path	= ::Strip_Filename_From_Path (full_path);
		FILE_LIST file_list;
		Build_File_List (start_path, render_obj, file_list);

		// Loop through all the files in the list and add them to our internal
		// list (or update its ref count if its already there)
		for (int index = 0; index < file_list.Count (); index ++) {
			FileInfoStruct &info = file_list[index];
			Add_File (Make_Full_Path (info.m_Filename), info.m_SubdirImportant);
		}

		MEMBER_RELEASE (render_obj);

	} else {
		Add_File (Make_Full_Path (filename), false);
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Remove_Asset
//
/////////////////////////////////////////////////////////////////
void
FileMgrClass::Remove_Asset (LPCTSTR filename)
{
	RenderObjClass *render_obj = NULL;

	//
	//	If this is a render object, then build a list of
	// dependencies
	//
	if (::Is_W3D_Filename (filename)) {		
		CString asset_name	= ::Asset_Name_From_Filename (filename);
		render_obj				= ::Create_Render_Obj (asset_name);
	}

	if (render_obj != NULL) {

		// Build a list of all the files used by this render object
		CString full_path		= Make_Full_Path (filename);		
		CString start_path	= ::Strip_Filename_From_Path (full_path);
		FILE_LIST file_list;
		Build_File_List (start_path, render_obj, file_list);

		// Loop through all the files in the list and add them to our internal
		// list (or update its ref count if its already there)
		for (int index = 0; index < file_list.Count (); index ++) {
			FileInfoStruct &info = file_list[index];
			Remove_File (Find_File (Make_Full_Path (info.m_Filename)));
		}

		MEMBER_RELEASE (render_obj);

	} else {
		Remove_File (Find_File (Make_Full_Path (filename)));
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Get_Preset_Library_Path
//
/////////////////////////////////////////////////////////////////
void
FileMgrClass::Get_Preset_Library_Path (uint32 class_id, bool is_temp, CString &path)
{
	CString directory	= Make_Full_Path (PRESETS_PATH);
	
	//
	//	Build a filename for the presets library
	//
	CString filename;
	if (is_temp == false) {
		filename.Format ("%d.ddb", class_id);
	} else {
		filename = TEMP_DB_FILENAME;
	}
	
	//
	//	Build a fully qualified path from the directory and filename
	//
	path = ::Make_Path (directory, filename);	
	return ;
}

