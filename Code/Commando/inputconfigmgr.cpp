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
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/commando/inputconfigmgr.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/09/02 11:40a                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "inputconfigmgr.h"
#include "dlgcontrols.h"
#include "input.h"
#include "rawfile.h"
#include "ffactory.h"
#include "registry.h"
#include "chunkio.h"
#include "debug.h"
#include "translatedb.h"
#include "string_ids.h"
#include "slavemaster.h"


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
static const char *CFG_DICTIONARY_FILENAME = "CONFIG.DAT";

////////////////////////////////////////////////////////////////
//	Registry key names
////////////////////////////////////////////////////////////////
//static const char *REGISTRY_SUB_KEY		= "Software\\Westwood\\Renegade\\Input";
static const char *REG_CURRENT_CONFIG	= "Current Config";

////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
DynamicVectorClass<InputConfigClass>	InputConfigMgrClass::ConfigList;
int												InputConfigMgrClass::CurrentConfigIndex = -1;


////////////////////////////////////////////////////////////////
//	Save/load constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_CONFIG_LIST			= 0x07181105,
	CHUNKID_VARIABLES,
	CHUNKID_CONFIG_OBJ,
};

enum
{
	VARID_CURRENT_CONFIG			= 1,
};


////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////
void
InputConfigMgrClass::Initialize (void)
{
	//
	//	Build the configuration list and load the current configuration
	//
	Load ();
	Load_Current_Configuration ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Shutdown
//
////////////////////////////////////////////////////////////////
void
InputConfigMgrClass::Shutdown (void)
{
	//
	//	Save our current configuration list
	//
	if (!SlaveMaster.Am_I_Slave()) {
		Save ();
	}

	//
	//	Reset our data
	//
	ConfigList.Delete_All ();
	CurrentConfigIndex = -1;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Current_Configuration
//
////////////////////////////////////////////////////////////////
bool
InputConfigMgrClass::Get_Current_Configuration (InputConfigClass &config)
{
	if (CurrentConfigIndex < 0 || CurrentConfigIndex >= ConfigList.Count ()) {
		return false;
	}

	//
	//	Simply index into the config list and return the object to the caller
	//
	config = ConfigList[CurrentConfigIndex];
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load_Current_Configuration
//
////////////////////////////////////////////////////////////////
void
InputConfigMgrClass::Load_Current_Configuration (void)
{
	if (CurrentConfigIndex < 0 || CurrentConfigIndex >= ConfigList.Count ()) {

		//
		//	The current configuration is invalid, so load the
		//	default configuration
		//
		Load_Default_Configuration ();
	} else {

		//
		//	Load this configuration
		//
		Load_Configuration (ConfigList[CurrentConfigIndex]);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Save_Current_Configuration
//
////////////////////////////////////////////////////////////////
void
InputConfigMgrClass::Save_Current_Configuration (void)
{
	if (CurrentConfigIndex < 0 || CurrentConfigIndex >= ConfigList.Count ()) {
		return ;
	}

	//
	//	Save this configuration
	//
	Save_Configuration (ConfigList[CurrentConfigIndex]);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Add_Configuration
//
////////////////////////////////////////////////////////////////
int
InputConfigMgrClass::Add_Configuration (const unichar_t *display_name)
{
	//
	//	Get a filename for the configuration
	//
	StringClass filename;
	Get_Unique_Config_Filename (filename);

	//
	//	Build a new input config object
	//
	InputConfigClass config;
	config.Set_Display_Name (display_name);
	config.Set_Filename (filename);
	config.Set_Is_Custom (true);

	//
	//	Add the object to our list
	//
	CurrentConfigIndex = ConfigList.Add (config);

	//
	//	Now save the current configuration to this file
	//
	Save_Configuration (config);
	return CurrentConfigIndex;
}


////////////////////////////////////////////////////////////////
//
//	Delete_Configuration
//
////////////////////////////////////////////////////////////////
void
InputConfigMgrClass::Delete_Configuration (int index)
{
	if (index < 0 || index >= ConfigList.Count ()) {
		return ;
	}

	//
	//	Reset the current configuration (if necessary)
	//
	bool load_default = false;
	if (index == CurrentConfigIndex) {
		CurrentConfigIndex = -1;		
		load_default = true;
	}

	//
	//	Get the local path of the configuration files
	//
	StringClass config_path;
	Get_Config_Path (config_path);

	StringClass full_path;
	full_path.Format ("%s\\%s", config_path, ConfigList[index].Get_Filename ());

	//
	//	Delete the configuration file
	//
	::DeleteFileA (full_path);

	//
	//	Now remove this entry from the list
	//
	ConfigList.Delete (index);

	//
	//	Load the default configuration is we just deleted the current configuration
	//
	if (load_default) {
		Load_Default_Configuration ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Delete_Configuration
//
////////////////////////////////////////////////////////////////
void
InputConfigMgrClass::Delete_Configuration (const char *filename)
{
	//
	//	Try to find the configuration in our list
	//
	int index = Find_Configuration (filename);
	if (index != -1) {

		//
		//	We've found the conversation, so delete it
		//
		Delete_Configuration (index);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Find_Configuration
//
////////////////////////////////////////////////////////////////
int
InputConfigMgrClass::Find_Configuration (const char *filename)
{
	int retval = -1;

	for (int index = 0; index < ConfigList.Count (); index ++) {

		//
		//	Is this the entry we're looking for?
		//
		if (::stricmp (ConfigList[index].Get_Filename (), filename) == 0) {
			retval = index;
			break;
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Load_Default_Configuration
//
////////////////////////////////////////////////////////////////
void
InputConfigMgrClass::Load_Default_Configuration (void)
{
	for (int index = 0; index < ConfigList.Count (); index ++) {

		//
		//	Is this the default configuration?
		//
		if (ConfigList[index].Is_Default ()) {
			Load_Configuration (ConfigList[index]);
			break;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Load_Configuration
//
////////////////////////////////////////////////////////////////
void
InputConfigMgrClass::Load_Configuration (const InputConfigClass &config)
{
	//
	//	Anytime we loada configuration, it becomes our current config
	//
	CurrentConfigIndex = Find_Configuration (config.Get_Filename ());

	//
	//	Load the configuration into memory
	//
	Input::Load_Configuration (config.Get_Filename ());

	//
	//	Reload the UI (if necessary)
	//
	if (ControlsMenuClass::Get_Instance () != NULL) {
		ControlsMenuClass::Get_Instance ()->Reload ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Save_Configuration
//
////////////////////////////////////////////////////////////////
void
InputConfigMgrClass::Save_Configuration (const InputConfigClass &config)
{
	//
	//	We can only save custom configurations
	//
	if (config.Is_Custom ()) {

		//
		//	Anytime we save a configuration, it becomes our current config...
		//
		CurrentConfigIndex = Find_Configuration (config.Get_Filename ());
		Input::Save_Configuration (config.Get_Filename ());

		//
		//	Update the copy of this config in our internal list
		//
		if (CurrentConfigIndex != -1) {
			ConfigList[CurrentConfigIndex] = config;
		}

	} else {

		//
		//	Make a name for a new configuration
		//
		WideStringClass new_name = TRANSLATE (IDS_MENU_CUSTOM);
		new_name += config.Get_Display_Name ();

		//
		//	Add a new configuration to list (this saves the new
		// configuration as well)
		//
		Add_Configuration (new_name);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Unique_Config_Filename
//
////////////////////////////////////////////////////////////////
void
InputConfigMgrClass::Get_Unique_Config_Filename (StringClass &filename)
{
	//
	//	Get the local path of the configuration files
	//
	StringClass config_path;
	Get_Config_Path (config_path);

	int slot	= 1;

	StringClass full_path;
	do
	{
		//
		//	Build this filename
		//
		filename.Format ("input%.2d.cfg", slot ++);

		//
		//	Check to see if this file exists
		//
		full_path.Format ("%s\\%s", (const char *)config_path, filename);

	} while (::GetFileAttributesA (full_path) != 0xFFFFFFFF);

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////
void
InputConfigMgrClass::Save (void)
{
	FileClass *file = _TheFileFactory->Get_File (CFG_DICTIONARY_FILENAME);
	WWASSERT (file != NULL);
	if (file == NULL) {
		return ;
	}

	//
	//	Open the file for writing
	//
	file->Open (FileClass::WRITE);
	ChunkSaveClass csave(file);

	//
	//	Save the config list to its own chunk
	//
	csave.Begin_Chunk (CHUNKID_CONFIG_LIST);
		Save_Config_List (csave);
	csave.End_Chunk ();

	//
	//	Save the
	//
	csave.Begin_Chunk (CHUNKID_VARIABLES);
		Save_Variables (csave);
	csave.End_Chunk ();

	//
	//	Close the file
	//
	file->Close ();
	_TheFileFactory->Return_File (file);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////
void
InputConfigMgrClass::Load (void)
{
	FileClass *file = _TheFileFactory->Get_File (CFG_DICTIONARY_FILENAME);
	WWASSERT (file != NULL);
	if (file == NULL) {
		return ;
	}

	//
	//	Open the file for reading
	//
	file->Open (FileClass::READ);
	ChunkLoadClass cload(file);

	//
	//	Read all the chunks stored in this file
	//
	while (cload.Open_Chunk ()) {
		switch(cload.Cur_Chunk_ID ()) {

			case CHUNKID_CONFIG_LIST:
				Load_Config_List (cload);
				break;

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;

			default:
				Debug_Say (("Unrecognized InputConfigMgrClass Chunk ID\n"));
				break;
		}

		cload.Close_Chunk ();
	}

	//
	//	If the configuration list is empty, then add the default
	// configuration file to the list
	//
	if (ConfigList.Count () == 0) {
		InputConfigClass config;
		config.Set_Display_Name (TRANSLATE (IDS_MENU_DEFAULT_SETTINGS));
		config.Set_Filename (DEFAULT_INPUT_FILENAME);
		config.Set_Is_Custom (false);
		config.Set_Is_Default (true);
		ConfigList.Add (config);
		CurrentConfigIndex = 0;
	}

	//
	//	Close the file
	//
	file->Close ();
	_TheFileFactory->Return_File (file);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Save_Config_List
//
////////////////////////////////////////////////////////////////
void
InputConfigMgrClass::Save_Config_List (ChunkSaveClass &csave)
{
	for (int index = 0; index < ConfigList.Count (); index ++) {

		//
		//	Save this configuration object to its own chunk
		//
		csave.Begin_Chunk (CHUNKID_CONFIG_OBJ);
			ConfigList[index].Save (csave);
		csave.End_Chunk ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Load_Config_List
//
////////////////////////////////////////////////////////////////
void
InputConfigMgrClass::Load_Config_List (ChunkLoadClass &cload)
{
	//
	//	Read all the sub-chunks
	//
	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_CONFIG_OBJ:
			{
				//
				//	Load this configuration object
				//
				InputConfigClass config;
				config.Load (cload);

				//
				//	Add the object to our list
				//
				ConfigList.Add (config);
				break;
			}

			default:
				Debug_Say (("Unrecognized InputConfigMgrClass::Load_Config_List Chunk ID\n"));
				break;
		}

		cload.Close_Chunk ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Save_Variables
//
////////////////////////////////////////////////////////////////
void
InputConfigMgrClass::Save_Variables (ChunkSaveClass &csave)
{
	//
	//	Determine what the filename of the current configuration is
	//
	StringClass filename;
	if (CurrentConfigIndex != -1) {
		filename = ConfigList[CurrentConfigIndex].Get_Filename ();
	} else {
		filename = DEFAULT_INPUT_FILENAME;
	}

	WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_CURRENT_CONFIG, filename);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
////////////////////////////////////////////////////////////////
void
InputConfigMgrClass::Load_Variables (ChunkLoadClass &cload)
{
	StringClass filename;

	//
	//	Read all the microchunks
	//
	while (cload.Open_Micro_Chunk ()) {
		switch(cload.Cur_Micro_Chunk_ID ()) {
			READ_MICRO_CHUNK_WWSTRING (cload, VARID_CURRENT_CONFIG, filename);
		}

		cload.Close_Micro_Chunk ();
	}


	//
	//	Store the index of this configuration
	//
	CurrentConfigIndex = Find_Configuration (filename);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Config_Path
//
////////////////////////////////////////////////////////////////
void
InputConfigMgrClass::Get_Config_Path (StringClass &full_path)
{
	//
	//	Lookup the path of the executable
	//
	char path[MAX_PATH] = { 0 };
	::GetModuleFileNameA (NULL, path, sizeof (path));

	//
	//	Strip off the filename
	//
	char *filename_portion = ::strrchr (path, '\\');
	if (filename_portion != NULL) {
		filename_portion[0] = 0;
	}

	//
	//	Build the full path from the EXE's directory
	//
	full_path.Format ("%s\\data\\config", path);
	return ;
}