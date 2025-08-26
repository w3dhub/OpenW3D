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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/assetpackagemgr.cpp          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/31/02 2:01p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "assetpackagemgr.h"
#include "Utils.h"
#include "regkeys.h"
#include "choosemodpackagedialog.h"
#include "editorbuild.h"
#include "LevelEdit.h"
#include "filelocations.h"


//////////////////////////////////////////////////////////////////////
//	Static member initialization
//////////////////////////////////////////////////////////////////////
StringClass		AssetPackageMgrClass::CurrentPackageName;
StringClass		AssetPackageMgrClass::CurrentPackagePath;


//////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////
void
AssetPackageMgrClass::Initialize (void)
{
	//
	//	Read the current package name from the registy
	//
	CString package_name = theApp.GetProfileString (CONFIG_KEY, CURRENT_PACKAGE_NAME, "");
	Set_Current_Package (package_name);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Shutdown
//
//////////////////////////////////////////////////////////////////////
void
AssetPackageMgrClass::Shutdown (void)
{
	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Current_Package
//
//////////////////////////////////////////////////////////////////////
void
AssetPackageMgrClass::Set_Current_Package (const char *name)
{
	StringClass path = (const char *)::Get_Startup_Directory ();

	//
	//	Save the package name and the path to the package directory
	//
	CurrentPackageName = name;
	CurrentPackagePath = ::Make_Path (path, name);

	if (CurrentPackageName.Get_Length () > 0) {
		_pThe3DAssetManager->Set_Current_Directory (CurrentPackagePath);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Build_Package_List
//
//////////////////////////////////////////////////////////////////////
void
AssetPackageMgrClass::Build_Package_List (STRING_LIST &list)
{
	StringClass path			= (const char *)::Get_Startup_Directory ();
	StringClass search_path	= static_cast<const char *>(::Make_Path (path, "*.*"));

	//
	//	Search for all sub-directories in our current folder
	//
	WIN32_FIND_DATA find_info	= { 0 };
	BOOL keep_going				= TRUE;
	HANDLE file_find				= NULL;
	for (file_find = ::FindFirstFile (search_path, &find_info);
		 (file_find != INVALID_HANDLE_VALUE) && keep_going;
		  keep_going = ::FindNextFile (file_find, &find_info))
	{
		if (	find_info.cFileName[0] != '.' &&
				(find_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
			//
			//	Add this sub-directory to the list
			//
			list.Add (find_info.cFileName);
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


//////////////////////////////////////////////////////////////////////
//
//	Show_Package_Selection_UI
//
//////////////////////////////////////////////////////////////////////
void
AssetPackageMgrClass::Show_Package_Selection_UI (void)
{
#ifdef PUBLIC_EDITOR_VER

	ChooseModPackageDialogClass dialog;
	dialog.DoModal ();

#endif //PUBLIC_EDITOR_VER
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Create_Package
//
//////////////////////////////////////////////////////////////////////
void
AssetPackageMgrClass::Create_Package (const char *name)
{
	StringClass path		= (const char *)::Get_Startup_Directory ();
	StringClass new_path	= static_cast<const char *>(::Make_Path (path, name));

	//
	//	Attempt to create the directory
	//
	if (::CreateDirectory (new_path, NULL) == FALSE) {
		
		//
		//	Warn the user on error
		//
		CString message;
		message.Format ("Unable to create the directory: %s", (const char *)new_path);
		::MessageBox (NULL, message, "File Error", MB_ICONERROR | MB_OK | MB_TOPMOST);
	} else {
		
		//
		//	Make a directory for the levels to be stored in
		//
		CString levels_full_path = ::Make_Path (new_path, LEVELS_ASSET_DIR);
		::CreateDirectory (levels_full_path, NULL);
	}

	return ;
}
