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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/PresetLogger.cpp             $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 6/14/02 6:20p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "presetlogger.h"
#include "textfile.h"
#include "wwstring.h"
#include "preset.h"
#include "definitionfactorymgr.h"
#include "definitionfactory.h"


//////////////////////////////////////////////////////////////////////////
//
//	Log_Created
//
//////////////////////////////////////////////////////////////////////////
void
PresetLoggerClass::Log_Created (PresetClass *preset)
{
	//
	//	Try to open the log file
	//
	TextFileClass file;
	if (Open_Log_File (file)) {

		//
		//	Lookup the users name
		//
		char computer_name[256];
		DWORD size = sizeof (computer_name);
		::GetComputerName (computer_name, &size);

		//
		//	Determine what type of preset this is
		//
		StringClass type_name;
		DefinitionFactoryClass *factory = DefinitionFactoryMgrClass::Find_Factory (preset->Get_Class_ID ());	
		if (factory != NULL) {
			type_name = factory->Get_Name ();
		}
		
		//
		//	Build a log entry with relevant information about the operation
		//
		StringClass message;
		message.Format ("(%s)	Created: %s, Type: %s", computer_name, (const char *)preset->Get_Name (), (const char *)type_name);

		//
		//	Write the entry to the end of the log
		//
		file.Seek (0, SEEK_END);
		file.Write_Line (message);

		//
		//	Close the file
		//
		file.Close ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Log_Moved
//
//////////////////////////////////////////////////////////////////////////
void
PresetLoggerClass::Log_Moved (PresetClass *preset, const char *new_parent_name)
{
	//
	//	Try to open the log file
	//
	TextFileClass file;
	if (Open_Log_File (file)) {
		
		//
		//	Determine what the parent's name should be
		//
		CString real_parent_name = new_parent_name;
		if (new_parent_name == NULL) {
			real_parent_name = "Root";
		}

		//
		//	Lookup the users name
		//
		char computer_name[256];
		DWORD size = sizeof (computer_name);
		::GetComputerName (computer_name, &size);

		//
		//	Write an entry to the end of the log
		//
		StringClass message;
		message.Format ("(%s)	Moved: %s. New Parent: %s", computer_name, (const char *)preset->Get_Name (), (const char *)real_parent_name);
		file.Seek (0, SEEK_END);
		file.Write_Line (message);

		//
		//	Close the file
		//
		file.Close ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Log_Renamed
//
//////////////////////////////////////////////////////////////////////////
void
PresetLoggerClass::Log_Renamed (const char *old_name, const char *new_name)
{
	//
	//	Try to open the log file
	//
	TextFileClass file;
	if (Open_Log_File (file)) {
		
		//
		//	Lookup the users name
		//
		char computer_name[256];
		DWORD size = sizeof (computer_name);
		::GetComputerName (computer_name, &size);

		//
		//	Write an entry to the end of the log
		//
		StringClass message;
		message.Format ("(%s)	Renamed: Old Name: %s, New Name: %s", computer_name, old_name, new_name);
		file.Seek (0, SEEK_END);
		file.Write_Line (message);

		//
		//	Close the file
		//
		file.Close ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Log_Deleted
//
//////////////////////////////////////////////////////////////////////////
void
PresetLoggerClass::Log_Deleted (const char *preset_name)
{
	//
	//	Try to open the log file
	//
	TextFileClass file;
	if (Open_Log_File (file)) {

		//
		//	Lookup the users name
		//
		char computer_name[256];
		DWORD size = sizeof (computer_name);
		::GetComputerName (computer_name, &size);
		
		//
		//	Write an entry to the end of the log
		//
		StringClass message;
		message.Format ("(%s)	Deleted: %s", computer_name, preset_name);
		file.Seek (0, SEEK_END);
		file.Write_Line (message);

		//
		//	Close the file
		//
		file.Close ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Log_File_Reference_Changed
//
//////////////////////////////////////////////////////////////////////////
void
PresetLoggerClass::Log_File_Reference_Changed
(
	PresetClass *	preset,
	const char *	param_name,
	const char *	new_value
)
{
	//
	//	Try to open the log file
	//
	TextFileClass file;
	if (Open_Log_File (file)) {
		
		//
		//	Write an entry to the end of the log
		//
		StringClass message;
		message.Format ("Reference Changed:	Preset: %s.  Param: %s.  Value:  %s", (const char *)preset->Get_Name (), param_name, new_value);
		file.Seek (0, SEEK_END);
		file.Write_Line (message);

		//
		//	Close the file
		//
		file.Close ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Open_Log_File
//
//////////////////////////////////////////////////////////////////////////
bool
PresetLoggerClass::Open_Log_File (TextFileClass &file_obj)
{
	bool retval = false;

	HANDLE file = INVALID_HANDLE_VALUE;
	
	//
	//	Try 10 times to open the file
	//
#ifndef PUBLIC_EDITOR_VER	
	for (int index = 0; index < 10; index ++) {
		
		//
		//	Try to open the file exclusively
		//
		file = ::CreateFile (	"\\\\mobius\\project7\\projects\\renegade\\asset management\\logs\\presets.log",
										GENERIC_WRITE,
										FILE_SHARE_READ,
										NULL,
										OPEN_ALWAYS,
										0L,
										NULL);

		//
		//	If we succeeded then break out of the loop
		//
		if (file != INVALID_HANDLE_VALUE) {
			break;
		}

		//
		//	Wait a quarter of a second before we try again.
		//
		::Sleep (250);
	}
#endif //PUBLIC_EDITOR_VER
	
	if (file != INVALID_HANDLE_VALUE) {
		file_obj.Attach (file);
		retval = true;
	}
	
	return retval;
}


