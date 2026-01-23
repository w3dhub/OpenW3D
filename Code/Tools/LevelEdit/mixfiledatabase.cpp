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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/mixfiledatabase.cpp          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 6/19/02 3:58p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "mixfiledatabase.h"
#include "registry.h"
#include "Utils.h"
#include "mixfile.h"
#include "rawfile.h"
#include "shlwapi.h"
#include <algorithm>


//////////////////////////////////////////////////////////////////////
//	Static member initialization
//////////////////////////////////////////////////////////////////////
MixFileDatabaseClass *MixFileDatabaseClass::_TheInstance = NULL;


//////////////////////////////////////////////////////////////////////
//
//	MixFileDatabaseClass
//
//////////////////////////////////////////////////////////////////////
MixFileDatabaseClass::MixFileDatabaseClass (void)
{
	//
	//	Open Renegade's registry
	//
	const char * const RENEGADE_REG_KEY	= "Software\\Westwood\\Renegade";
	RegistryClass registry (RENEGADE_REG_KEY);
	if (registry.Is_Valid ()) {

		//
		//	Read the installation path from the registry
		//
		StringClass install_path;
		const char * const RENEGADE_INSTALL_VALUE	= "InstallPath";
		registry.Get_String (RENEGADE_INSTALL_VALUE, install_path);

		if (install_path.Get_Length () > 0) {			

			//
			//	The mix files are contained in the data sub-directory
			//
			install_path	= ::Strip_Filename_From_Path (install_path);
			MixFilePath		= ::Make_Path (install_path, "DATA");
		}
	}

	_TheInstance = this;
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~MixFileDatabaseClass
//
//////////////////////////////////////////////////////////////////////
MixFileDatabaseClass::~MixFileDatabaseClass (void)
{
	_TheInstance = NULL;
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Open_Database
//
//////////////////////////////////////////////////////////////////////
bool
MixFileDatabaseClass::Open_Database
(
	LPCTSTR /* ini_filename */,
	LPCTSTR /* username */,
	LPCTSTR /* password */
)
{
	//
	//	Add the always mix files to the file factory list
	//
	CString always_dbs_path = ::Make_Path (MixFilePath, "Always.dbs");
	CString always_dat_path = ::Make_Path (MixFilePath, "Always.dat");
	MainFileFactory.Add_FileFactory (new MixFileFactoryClass (always_dbs_path, &RenegadeDataFileFactory), "always.dat");
	MainFileFactory.Add_FileFactory (new MixFileFactoryClass (always_dat_path, &RenegadeDataFileFactory), "always.dbs");

	//
	//	Build a search path for mix files in the data directory
	//
	CString search_path = ::Make_Path (MixFilePath, "*.mix");

	//
	//	Search for all mix files in the installation directory
	//
	WIN32_FIND_DATA find_info	= { 0 };
	BOOL keep_going				= true;
	HANDLE file_find				= NULL;
	for (file_find = ::FindFirstFile (search_path, &find_info);
		 (file_find != INVALID_HANDLE_VALUE) && keep_going;
		  keep_going = ::FindNextFile (file_find, &find_info))
	{
		//
		//	Add this mix file to our mix file factory list
		//
		CString full_path = ::Make_Path (MixFilePath, find_info.cFileName);		
		MainFileFactory.Add_FileFactory (new MixFileFactoryClass (full_path, &RenegadeDataFileFactory), find_info.cFileName);
	}

	//
	//	Close the search handle
	//
	if (file_find != INVALID_HANDLE_VALUE) {			  
		::FindClose (file_find); 
	}

	return true;
}


//////////////////////////////////////////////////////////////////////
//
//	Check_Out
//
//////////////////////////////////////////////////////////////////////
bool
MixFileDatabaseClass::Check_Out (LPCTSTR local_filename, bool get_locally)
{
	bool retval = true;

	//
	//	Simply do a get as necessary
	//
	if (get_locally) {
		retval = Get (local_filename);
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Get
//
//////////////////////////////////////////////////////////////////////
bool
MixFileDatabaseClass::Get (LPCTSTR local_filename)
{
	StringClass filename;
	Get_Filename (local_filename, filename);

	StringClass full_local_path = local_filename;

	//
	//	Check to see if we need to swap texture extensions
	//
	if (Internal_Does_File_Exist (filename) == false && Is_Texture (filename)) {
		Swap_Texture_Extension (filename);
		
		StringClass local_path = (const char *)::Strip_Filename_From_Path (local_filename);
		full_local_path = ::Make_Path (local_path, filename);
	}

	//
	//	Get the file
	//
	return Internal_Get (filename, full_local_path);
}


//////////////////////////////////////////////////////////////////////
//
//	Internal_Get
//
//////////////////////////////////////////////////////////////////////
bool
MixFileDatabaseClass::Internal_Get (LPCTSTR filename, LPCSTR local_path)
{
	bool retval = false;

	//
	//	Get the file from one of the mix files
	//
	FileClass *file =  MainFileFactory.Get_File (filename);
	if (file != NULL) {

		//
		//	Copy the file if it exists
		//
		if (file->Is_Available () && file->Open ()) {
			retval = Copy_File (file, local_path);
			file->Close ();
		}

		MainFileFactory.Return_File (file);
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Copy_File
//
//////////////////////////////////////////////////////////////////////
bool
MixFileDatabaseClass::Copy_File (FileClass *src_file, LPCTSTR local_filename)
{
	if (	src_file == NULL || local_filename == NULL ||
			::GetFileAttributes (local_filename) != 0xFFFFFFFF)
	{
		return false;
	}

	bool retval = false;

	//
	//	Ensure the directory structure exists before we attempt to create the file
	//
	StringClass path = (const char *)::Strip_Filename_From_Path (local_filename);
	Create_Directory_Structure (path);

	//
	//	Create the destination file
	//
	RawFileClass dest_file;
	dest_file.Set_Name (local_filename);
	if (dest_file.Open (RawFileClass::WRITE)) {
		retval = true;

		//
		//	Copy the data from the source file to the destination file
		//
		int file_size = src_file->Size ();
		uint8 buffer[4096];
		while (file_size > 0) {
			
			//
			//	Read the data from the source file
			//
			int bytes			= std::min (file_size, (int)sizeof (buffer));
			int copied_size	= src_file->Read (buffer, bytes);
			file_size			-= copied_size;
			if (copied_size <= 0) {
				break;
			}
			
			//
			//	Copy the data to the dest file (kick out of the loop on error)
			//
			if (dest_file.Write (buffer, copied_size) != copied_size) {
				break;
			}
		}

		//
		//	Close the destination file
		//
		dest_file.Close ();
	}	

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Does_File_Exist
//
//////////////////////////////////////////////////////////////////////
bool
MixFileDatabaseClass::Does_File_Exist (LPCTSTR local_filename)
{
	bool retval = false;

	//
	//	Test to see if the file exists
	//
	StringClass filename;
	Get_Filename (local_filename, filename);
	retval = Internal_Does_File_Exist (filename);

	//
	//	If the file did not exists, check to see if its a texture...
	//
	if (retval == false && Is_Texture (filename)) {
		
		//
		//	Check to see if either the compressed or uncompressed
		// texture exists
		//
		Swap_Texture_Extension (filename);
		retval = Internal_Does_File_Exist (filename);
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Internal_Does_File_Exist
//
//////////////////////////////////////////////////////////////////////
bool
MixFileDatabaseClass::Internal_Does_File_Exist (LPCSTR filename)
{
	bool retval = false;

	//
	//	Get the file from one of the mix files
	//
	FileClass *file =  MainFileFactory.Get_File (filename);
	if (file != NULL) {

		//
		//	Does the file exist?
		//
		if (file->Is_Available ()) {
			retval = true;
		}

		MainFileFactory.Return_File (file);
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Create_Directory_Structure
//
//////////////////////////////////////////////////////////////////////////////////
void
MixFileDatabaseClass::Create_Directory_Structure (LPCTSTR path)
{
	if (path != NULL && path[0] != 0 && ::GetFileAttributes (path) == 0xFFFFFFFF) {
		Create_Directory_Structure (::Strip_Filename_From_Path (path));
		::CreateDirectory (path, NULL);
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Is_Texture
//
//////////////////////////////////////////////////////////////////////////////////
bool
MixFileDatabaseClass::Is_Texture (LPCSTR filename)
{
	StringClass temp_str (filename, true);
	::strlwr (temp_str.Peek_Buffer ());

	//
	//	Check to see if this is either a compressed or uncompressed texture
	//
	bool retval = false;
	if (::strstr (temp_str.Peek_Buffer (), ".tga") || ::strstr (temp_str.Peek_Buffer (), ".dds")) {
		retval = true;
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Swap_Texture_Extension
//
//////////////////////////////////////////////////////////////////////////////////
void
MixFileDatabaseClass::Swap_Texture_Extension (StringClass &filename)
{
	::strlwr (filename.Peek_Buffer ());

	//
	//	Is this a tga file (uncompressed), or a dds file (compressed)?
	//
	char *tga_extension = ::strstr (filename.Peek_Buffer (), ".tga");
	if (tga_extension != NULL) {
		
		//
		//	Simply copy the new extension into the string
		//
		::strcpy (tga_extension, ".dds");

	} else {
		char *dds_extension = ::strstr (filename.Peek_Buffer (), ".dds");
		if (dds_extension != NULL) {

			//
			//	Simply copy the new extension into the string
			//
			::strcpy (dds_extension, ".tga");
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Find_Files
//
//////////////////////////////////////////////////////////////////////////////////
void
MixFileDatabaseClass::Find_Files (DynamicVectorClass<StringClass> &file_list, LPCTSTR search_mask)
{
	//
	//	Loop over all the factories
	//
	int factory_count = MainFileFactory.Get_Factory_Count ();
	for (int index = 0; index < factory_count; index ++) {
		
		//
		//	Get a pointer to the current factory
		//
		FileFactoryClass *factory = MainFileFactory.Get_Factory (index);
		if (factory != NULL) {
			
			//
			//	We assume that this is a mix file factory (since all the factories
			// we're adding are mix file factories).  Note:  This can easily
			// break.
			//
			MixFileFactoryClass *mix_factory = static_cast<MixFileFactoryClass *> (factory);

			//
			//	Get a list of all the files in this mix file
			//
			DynamicVectorClass<StringClass> files_in_mix;
			files_in_mix.Set_Growth_Step (1000);
			mix_factory->Build_Filename_List (files_in_mix);

			//
			//	Now, add any files that match the supplied wildcard to our master list
			//
			for (int file_index = 0; file_index < files_in_mix.Count (); file_index ++) {
				const char *filename = files_in_mix[file_index];

				//
				//	Add this file to our list if it matches the mask
				//
				if (::PathMatchSpec (filename, search_mask)) {
					file_list.Add (filename);
				}
			}
		}
	}

	return ;
}



//////////////////////////////////////////////////////////////////////////////////
//
//	Get_All
//
//////////////////////////////////////////////////////////////////////////////////
bool
MixFileDatabaseClass::Get_All (LPCTSTR dest_path, LPCTSTR search_mask)
{
	DynamicVectorClass<StringClass> file_list;

	//
	//	Loop over all the factories
	//
	int factory_count = MainFileFactory.Get_Factory_Count ();
	int index;
	for (index = 0; index < factory_count; index ++) {
		
		//
		//	Get a pointer to the current factory
		//
		FileFactoryClass *factory = MainFileFactory.Get_Factory (index);
		if (factory != NULL) {
			
			//
			//	We assume that this is a mix file factory (since all the factories
			// we're adding are mix file factories).  Note:  This can easily
			// break.
			//
			MixFileFactoryClass *mix_factory = static_cast<MixFileFactoryClass *> (factory);

			//
			//	Get a list of all the files in this mix file
			//
			DynamicVectorClass<StringClass> files_in_mix;
			files_in_mix.Set_Growth_Step (1000);
			mix_factory->Build_Filename_List (files_in_mix);

			//
			//	Now, add any files that match the supplied wildcard to our master list
			//
			for (int file_index = 0; file_index < files_in_mix.Count (); file_index ++) {
				const char *filename = files_in_mix[file_index];

				//
				//	Add this file to our list if it matches the mask
				//
				if (::PathMatchSpec (filename, search_mask)) {
					file_list.Add (filename);
				}
			}
		}
	}

	//
	//	Now, get all the matching files to the specified path
	//
	for (index = 0; index < file_list.Count (); index ++) {
		StringClass full_path = (const char *)::Make_Path (dest_path, file_list[index]);
		Get (full_path);
	}

	return true;
}


///////////////////////////////////////////////////////////////////
//
//	Get_File
//
///////////////////////////////////////////////////////////////////
FileClass *
MixFileDatabaseClass::Get_File (LPCTSTR local_filename)
{
	FileClass *retval = NULL;

	if (::GetFileAttributes (local_filename) != 0xFFFFFFFF) {

		//
		//	Do this to get around an oddity of the class when you pass in
		// the filename to the constructor
		//
		retval = new RawFileClass;
		retval->Set_Name (local_filename);
	} else {
		
		//
		//	Get a pointer to the file in the mix file
		//
		StringClass filename;
		Get_Filename (local_filename, filename);

		retval = MainFileFactory.Get_File (filename);
	}

	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	Get_Filename
//
///////////////////////////////////////////////////////////////////
void
MixFileDatabaseClass::Get_Filename (LPCTSTR path, StringClass &filename)
{
	filename = path;
	::strlwr (filename.Peek_Buffer ());

	//
	//	Check to see if the sub-directory is important, if it is, then
	// return the sub-directory as well
	//
	const char *subdir_token = ::strstr (filename, "+\\");
	if (subdir_token != NULL) {

		//
		//	Try to find the preceeding directory delimiter
		//
		int index = (subdir_token - filename.Peek_Buffer ());
		for (; index >= 0; index --) {
			if (filename[index] == '\\') {
				StringClass temp_str	= filename;
				filename					= (temp_str.Peek_Buffer () + index + 1);
				break;
			}
		}

	} else {

		//
		//	Simply strip the filename from the path
		//
		filename = (const char *)::Get_Filename_From_Path (filename);
	}

	return ;
}

