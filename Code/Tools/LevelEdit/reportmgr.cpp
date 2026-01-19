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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/reportmgr.cpp                $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/10/01 8:53a                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "reportmgr.h"
#include "textfile.h"
#include "nodemgr.h"
#include "preset.h"
#include "listtypes.h"
#include "hashtemplate.h"
#include "node.h"
#include "filemgr.h"
#include "translatedb.h"
#include "translateobj.h"


//////////////////////////////////////////////////////////////////////
//	Static member initialization
//////////////////////////////////////////////////////////////////////
HashTemplateClass<StringClass, bool> ReportMgrClass::FilenameHash;


//////////////////////////////////////////////////////////////////////
//
//	Export_File_Usage_Report
//
//////////////////////////////////////////////////////////////////////
void
ReportMgrClass::Export_File_Usage_Report (const char *filename)
{
	//
	//	Open the text file
	//
	TextFileClass file;
	file.Set_Name (filename);
	if (file.Open (FileClass::WRITE)) {
		STRING_LIST file_list;
		file_list.Set_Growth_Step (500);
		
		//
		//	Loop over all the nodes in the current level
		//
		for (	NodeClass *node = NodeMgrClass::Get_First ();
				node != NULL;
				node = NodeMgrClass::Get_Next (node))
		{
			PresetClass *preset = node->Get_Preset ();
			if (preset != NULL) {

				//
				//	Build a list of definitions this preset depends on
				//
				DEFINITION_LIST definition_list;
				preset->Collect_Definitions (definition_list);

				//
				//	Add all the files from these definitions to our list
				//
				for (int index = 0; index < definition_list.Count (); index ++) {
					Add_Definition_Dependencies (definition_list[index], file_list);
				}
			}			
		}

		//
		//	Create a temporary hash that we can use to determine if
		// a filename has already been written to the file...
		//
		HashTemplateClass<StringClass, bool> filename_hash;

		//
		//	Now add all W3D file-depenedencies to this list
		//
		int index;
		for (index = 0; index < file_list.Count (); index ++) {			
			StringClass next_filename (file_list[index], true);			

			//
			//	If this is a W3D file, then enumerate its dependencies...
			//
			if (::Is_W3D_Filename (next_filename)) {
				UniqueListClass<CString> dep_list;
				::Get_File_Mgr ()->Build_Dependency_List (next_filename, dep_list);
				for (int new_index = 0; new_index < dep_list.Count (); new_index ++) {
					CString rel_path = ::Get_File_Mgr ()->Make_Relative_Path (dep_list[new_index]);
					
					StringClass lower_name (rel_path, true);
					::strlwr (lower_name.Peek_Buffer ());
					
					//
					//	Don't add this file to the list if its already there
					//
					if (filename_hash.Exists (lower_name) == false) {
						file_list.Add (CString (lower_name));
						filename_hash.Insert (lower_name, true);
					}
				}
			}
		}

		//
		//	Reset the has
		//
		filename_hash.Remove_All ();

		//
		//	Now write all the file names to the text file
		//
		for (index = 0; index < file_list.Count (); index ++) {
			StringClass lower_name (file_list[index], true);
			::strlwr (lower_name.Peek_Buffer ());

			//
			//	Only write this filename to the file if it
			// hasn't already been written
			//
			if (filename_hash.Exists (lower_name) == false) {
				file.Write_Line (lower_name);
				filename_hash.Insert (lower_name, true);
			}
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Add_Definition_Dependencies
//
///////////////////////////////////////////////////////////////////////
void
ReportMgrClass::Add_Definition_Dependencies (DefinitionClass *definition, STRING_LIST &list)
{
	if (definition == NULL) {
		return ;
	}
		
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
				CString lower_filename = ::Get_File_Mgr ()->Make_Relative_Path (full_path);
				lower_filename.MakeLower ();

				//
				//	Add this filename to the list (if its not already there)
				//
				if (FilenameHash.Exists (StringClass (lower_filename, true)) == false) {
					list.Add (lower_filename);
					FilenameHash.Insert (StringClass (lower_filename, true), true);
				}
			}

		} else if (parameter->Get_Type () == ParameterClass::TYPE_MODELDEFINITIONID) {
			
			//
			//	If this is param references a physics-definition, then add all its dependencies as well..
			//
			DefinitionClass *phys_def = NULL;
			phys_def = DefinitionMgrClass::Find_Definition (((ModelDefParameterClass *)parameter)->Get_Value (), false);
			Add_Definition_Dependencies (phys_def, list);
		} else if (parameter->As_DefParameterClass () != NULL) {
			DefinitionClass *sub_def = DefinitionMgrClass::Find_Definition (parameter->As_DefParameterClass ()->Get_Value (), false);
			Add_Definition_Dependencies (sub_def, list);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Export_Missing_Translation_Report
//
//////////////////////////////////////////////////////////////////////
void
ReportMgrClass::Export_Missing_Translation_Report (const char *filename, int lang_id)
{
	//
	//	Open the text file
	//
	TextFileClass file;
	file.Set_Name (filename);
	if (file.Open (FileClass::WRITE)) {

		//
		//	Loop over all the string objects
		//
		int count = TranslateDBClass::Get_Object_Count ();
		for (int index = 0; index < count; index ++) {
			
			//
			//	Does this string contain the necessary translation?
			//
			TDBObjClass *object = TranslateDBClass::Get_Object (index);
			if (	object != NULL &&
					object->As_StringTwiddlerClass () == NULL &&
					object->Contains_Translation (lang_id) == false)
			{
				file.Write_Line (object->Get_ID_Desc ());
			}
		}
	}

	return ;
}
