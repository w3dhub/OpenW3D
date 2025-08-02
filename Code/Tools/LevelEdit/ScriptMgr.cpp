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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/ScriptMgr.cpp                $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 4/27/00 8:53a                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "scriptmgr.h"
#include "..\..\scripts\ScriptEvents.H"
#include "EditScript.h"
#include "Utils.h"
#include "FileMgr.h"
#include "FileLocations.h"


//////////////////////////////////////////////////////////////////////////
//	Static member initialization
//////////////////////////////////////////////////////////////////////////
SCRIPT_LIST ScriptMgrClass::_ScriptList;


//////////////////////////////////////////////////////////////////////////
//
//	~ScriptMgrClass
//
//////////////////////////////////////////////////////////////////////////
ScriptMgrClass::~ScriptMgrClass (void)
{
	Shutdown ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////////
void
ScriptMgrClass::Initialize (void)
{
	CString scripts_path	= ::Get_File_Mgr ()->Make_Full_Path (SCRIPTS_PATH);
	//CString search_path	= ::Make_Path (scripts_path, "*.dll");

	CString filename;

// Denzil 4/3/00 - Use scripts.dll for all builds
#if(0)

	#ifdef WWDEBUG
	#ifndef NDEBUG
		filename = "SCRIPTSD.DLL";
	#else
		filename = "SCRIPTSP.DLL";
	#endif
	#else
		filename = "SCRIPTS.DLL";
	#endif

#else // Denzil
		filename = "SCRIPTS.DLL";
#endif // Denzil


	//
	// Find all files that match this wildcard
	//
	/*WIN32_FIND_DATA find_info = { 0 };
	BOOL keep_going = TRUE;
	for (HANDLE file_find = ::FindFirstFile (search_path, &find_info);
		  (file_find != INVALID_HANDLE_VALUE) && keep_going;
		  keep_going = ::FindNextFile (file_find, &find_info))
	{*/

		//
		// Build a path to the DLL
		//
		//CString dll_name = ::Make_Path (scripts_path, Get_Filename_From_Path (find_info.cFileName));
		CString dll_name = ::Make_Path (scripts_path, filename);
		if (::GetFileAttributes (dll_name) != 0xFFFFFFFF) {
			HMODULE module_handle = ::LoadLibrary (dll_name);
			if (module_handle != NULL) {
				
				// Lookup the function pointer we need to call to determine
				// a filename list
				LPFN_GET_SCRIPT_COUNT pfn_get_script_count		= (LPFN_GET_SCRIPT_COUNT)::GetProcAddress (module_handle, LPSTR_GET_SCRIPT_COUNT);
				LPFN_GET_SCRIPT_NAME pfn_get_script_name			= (LPFN_GET_SCRIPT_NAME)::GetProcAddress (module_handle, LPSTR_GET_SCRIPT_NAME);
				LPFN_GET_SCRIPT_PARAM_DESCRIPTION pfn_get_param_desc	= (LPFN_GET_SCRIPT_PARAM_DESCRIPTION)::GetProcAddress (module_handle, LPSTR_GET_SCRIPT_PARAM_DESCRIPTION);
				ASSERT (pfn_get_script_count != NULL);
				ASSERT (pfn_get_script_name != NULL);
				ASSERT (pfn_get_param_desc != NULL);
				if ((pfn_get_script_count != NULL) &&
					 (pfn_get_script_name != NULL) &&
					 (pfn_get_param_desc != NULL))
				{
					int count = (*pfn_get_script_count) ();
					
					//
					// Loop through all the scripts in the list and add their names
					// to our list
					//
					for (int index = 0; index < count; index ++) {
						EditScriptClass *script = new EditScriptClass;
						
						//
						// Pass the script name, and the script params onto our object
						//
						script->Set_Name ((*pfn_get_script_name) (index));
						script->Set_Param_Desc ((*pfn_get_param_desc) (index));

						// Add this script to the list
						_ScriptList.Add (script);
					}
				}

				// Unload the DLL from memory
				::FreeLibrary (module_handle);
				module_handle = NULL;
			}
		}
	/*}

	if (file_find != INVALID_HANDLE_VALUE) {
		::FindClose (file_find);
	}*/
	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Shutdown
//
//////////////////////////////////////////////////////////////////////////
void
ScriptMgrClass::Shutdown (void)
{
	for (int index = 0; index < _ScriptList.Count (); index++) {
		EditScriptClass *script = _ScriptList[index];
		SAFE_DELETE (script);
	}

	_ScriptList.Delete_All ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Find_Script
//
//////////////////////////////////////////////////////////////////////////
EditScriptClass *
ScriptMgrClass::Find_Script (LPCTSTR name)
{
	EditScriptClass *script = NULL;

	for (int index = 0; (index < _ScriptList.Count ()) && (script == NULL); index++) {
		EditScriptClass *curr_script = _ScriptList[index];
		
		//
		//	Is this the script we are looking for?
		//
		if (::lstrcmpi (curr_script->Get_Name (), name) == 0) {
			script = curr_script;
		}
	}

	return script;
}
