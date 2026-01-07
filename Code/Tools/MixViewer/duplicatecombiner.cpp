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
 *                 Project Name : WWAudio                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/MixViewer/duplicatecombiner.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/21/02 5:25p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "duplicatecombiner.h"
#include "ffactory.h"
#include "mixfile.h"
#include "rawfile.h"
#include "bittype.h"
#include "mixcombiningdialog.h"
#include <algorithm>


//////////////////////////////////////////////////////////////
//
//	DuplicateRemoverClass
//
//////////////////////////////////////////////////////////////
DuplicateRemoverClass::DuplicateRemoverClass (void)	:
	TempFilenameStart (0),
	Dialog (NULL)
{
	return ;
}


//////////////////////////////////////////////////////////////
//
//	~DuplicateRemoverClass
//
//////////////////////////////////////////////////////////////
DuplicateRemoverClass::~DuplicateRemoverClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////
//
//	Process
//
//////////////////////////////////////////////////////////////
void
DuplicateRemoverClass::Process (void)
{
	//
	//	Kick off the worker thread...
	//
	::AfxBeginThread (fnThreadProc, (LPVOID)this);

	//
	//	Show the UI
	//
	MixCombiningDialogClass dialog;
	Dialog = &dialog;
	dialog.DoModal ();
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  fnThreadProc
//
////////////////////////////////////////////////////////////////////////////
UINT
DuplicateRemoverClass::fnThreadProc (LPVOID pParam)
{
	//
	//	Simply ask the combiner to start processing
	//
	DuplicateRemoverClass *remover = (DuplicateRemoverClass *)pParam;
	if (remover != NULL) {
		remover->Internal_Process ();
	}

	return 1;
}


//////////////////////////////////////////////////////////////
//
//	Internal_Process
//
//////////////////////////////////////////////////////////////
void
DuplicateRemoverClass::Internal_Process (void)
{
	Make_Temp_Directory ();

	//
	//	Open the source mix files
	//
	DynamicVectorClass<MixFileFactoryClass *> mix_file_list;
	Open_Mix_Files (mix_file_list);

	//
	//	Open the destination mix factory
	//
	MixFileFactoryClass dest_factory (DestinationMixFilename, _TheFileFactory);
	if (dest_factory.Is_Valid () && dest_factory.Build_Internal_Filename_List ()) {
		Dialog->Set_Status_Text ("Building temporary files...");
		Dialog->Set_Progress_Percent (0);
		
		//
		//	Loop over each mix file...
		//
		for (int mix_index = 0; mix_index < mix_file_list.Count (); mix_index ++) {

			//
			//	Get the list of filenames inside this mix file
			//
			DynamicVectorClass<StringClass> filename_list;
			mix_file_list[mix_index]->Get_Filename_List (filename_list);			

			for (int file_index = 0; file_index < filename_list.Count (); file_index ++) {
				StringClass filename = ::strlwr (filename_list[file_index].Peek_Buffer ());				

				DynamicVectorClass<int> dup_indices;
				dup_indices.Add (mix_index);

				//
				//	Try to find this file in any of the other mix files
				//
				for (int test_mix_index = 0; test_mix_index < mix_file_list.Count (); test_mix_index ++) {
					if (test_mix_index != mix_index && Is_File_In_Factory (filename, mix_file_list[test_mix_index])) {
						dup_indices.Add (test_mix_index);
					}
				}

				//
				//	Check to see if this file is already in the "destination" file
				//
				bool is_in_dest = Is_File_In_Factory (filename, &dest_factory);

				//
				//	Were there any duplicates?
				//
				if (dup_indices.Count () > 1 || is_in_dest) {

					//
					//	Copy the file from the source to the destination factory
					//
					if (is_in_dest == false) {
						Copy_File (mix_file_list[mix_index], &dest_factory, filename);
					}
					
					//
					//	Remove all the duplicates from the mix files
					//
					for (int dup_index = 0; dup_index < dup_indices.Count (); dup_index ++) {
						mix_file_list[dup_indices[dup_index]]->Delete_File (filename);
					}
				}
			}

			//
			//	Update the UI
			//
			Dialog->Set_Progress_Percent ((float)mix_index / float(mix_file_list.Count () + 1));
		}

		//
		//	Save the changes
		//
		Dialog->Set_Status_Text ("Reconstructing mix files...");
		Dialog->Set_Progress_Percent (0);
		dest_factory.Flush_Changes ();
		Dialog->Set_Progress_Percent (1.0F / float(mix_file_list.Count () + 1));
		for (int index = 0; index < mix_file_list.Count (); index ++) {
			mix_file_list[index]->Flush_Changes ();
			Dialog->Set_Progress_Percent ((float)index / float(mix_file_list.Count () + 1));			
		}
		Dialog->Set_Progress_Percent (1.0F);
	}

	//
	//	Free each of the mix file factories
	//
	Close_Mix_Files (mix_file_list);

	//
	//	Remove the temporary directory we created
	//
	Delete_Temp_Directory ();

	//
	//	Close the dialog
	//
	Dialog->PostMessage (WM_COMMAND, MAKELPARAM (IDOK, BN_CLICKED));
	return ;
}


//////////////////////////////////////////////////////////////
//
//	Copy_File
//
//////////////////////////////////////////////////////////////
void
DuplicateRemoverClass::Copy_File
(
	MixFileFactoryClass *	src_mix,
	MixFileFactoryClass *	dest_mix,
	const char *				filename
)
{
	//
	//	Get the file data from the source mix
	//
	FileClass *src_file = src_mix->Get_File (filename);
	src_file->Open ();

	//
	//	Create a destination file for the data
	//
	StringClass full_path;
	Get_Temp_Filename (full_path);
	RawFileClass dest_file;
	dest_file.Set_Name (full_path);
	if (dest_file.Open (RawFileClass::WRITE)) {

		//
		//	Copy the data from the source mix file to the destination file
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
			//	Copy the data to the dest file
			//
			dest_file.Write (buffer, copied_size);
		}

		//
		//	Add the file to the destination mix file
		//
		dest_mix->Add_File (full_path, filename);

		//
		//	Close the temporary data file
		//
		dest_file.Close ();
	}

	//
	//	Return the file
	//
	src_mix->Return_File (src_file);
	return ;
}


//////////////////////////////////////////////////////////////
//
//	Open_Mix_Files
//
//////////////////////////////////////////////////////////////
void
DuplicateRemoverClass::Open_Mix_Files (DynamicVectorClass<MixFileFactoryClass *> &list)
{
	//
	//	Loop over all the mix filenames in our list
	//
	for (int index = 0; index < MixFileList.Count (); index ++) {
		
		//
		//	Create this mix file factory and add it to our list
		//
		MixFileFactoryClass *mix_factory = new MixFileFactoryClass (MixFileList[index], _TheFileFactory);
		if (mix_factory->Is_Valid () && mix_factory->Build_Internal_Filename_List ()) {
			list.Add (mix_factory);
		} else {
			delete mix_factory;
			mix_factory = NULL;
		}

		//
		//	Update the UI
		//
		Dialog->Set_Progress_Percent ((float)index / float(MixFileList.Count () + 1));
	}

	return ;
}


//////////////////////////////////////////////////////////////
//
//	Close_Mix_Files
//
//////////////////////////////////////////////////////////////
void
DuplicateRemoverClass::Close_Mix_Files (DynamicVectorClass<MixFileFactoryClass *> &list)
{
	//
	//	Simply free each entry in the list
	//
	for (int index = 0; index < list.Count (); index ++) {
		delete list[index];
	}

	list.Delete_All ();
	return ;
}


/////////////////////////////////////////////////////////
//
//	Make_Temp_Directory
//
/////////////////////////////////////////////////////////
void
DuplicateRemoverClass::Make_Temp_Directory (void)
{
	//
	//	Get the path of the temp directory
	//
	char temp_dir[MAX_PATH] = { 0 };
	::GetTempPath (sizeof (temp_dir), temp_dir);

	CString temp_path = static_cast<const char *>(Make_Path (temp_dir, "mixcombiner"));
	
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
	::SetCurrentDirectory (TempDirectory);
	return ;
}


/////////////////////////////////////////////////////////
//
//	Delete_Temp_Directory
//
/////////////////////////////////////////////////////////
void
DuplicateRemoverClass::Delete_Temp_Directory (void)
{
	//
	//	Change the current directory so we can remove the temporary one
	//
	::SetCurrentDirectory ("c:\\");
	
	//
	//	Remove the temporary directory
	//
	Clean_Directory (TempDirectory);
	::RemoveDirectory (TempDirectory);
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Clean_Directory
//
//////////////////////////////////////////////////////////////////////////////////
bool
DuplicateRemoverClass::Clean_Directory (LPCTSTR local_dir)
{
	bool retval = true;

	//
	// Build a search mask from the directory
	//
	StringClass search_mask = StringClass (local_dir) + "\\*.*";	
	
	//
	// Loop through all the files in this directory and add them	
	// to our list
	//
	DynamicVectorClass<StringClass> file_list;
	BOOL keep_going = true;
	WIN32_FIND_DATA find_info = { 0 };HANDLE hfind = ::FindFirstFile (search_mask, &find_info);
	for (;(hfind != INVALID_HANDLE_VALUE) && keep_going;
		  keep_going = ::FindNextFile (hfind, &find_info))
	{
		
		//
		// If this file isn't a directory, add it to the list
		//
		if (!(find_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			StringClass filename = find_info.cFileName;
			file_list.Add (filename);
		} else if (find_info.cFileName[0] != '.') {
			
			//
			//	Recurse into this subdirectory
			//
			StringClass full_path = Make_Path (local_dir, find_info.cFileName);
			Clean_Directory (full_path);
			
			//
			//	Add this directory to the list so it will get
			// deleted with the files...
			//
			StringClass filename = find_info.cFileName;
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
		StringClass &filename = file_list[index];
		StringClass full_path = Make_Path (local_dir, filename);
		Delete_File (full_path);
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Make_Path
//
//////////////////////////////////////////////////////////////////////////////////
StringClass
DuplicateRemoverClass::Make_Path (LPCTSTR path, LPCTSTR filename)
{
	StringClass full_path = path;

	//
	//	Delimit the path if necessary
	//
	const size_t path_length = full_path.Get_Length ();
	if (path_length == 0 || full_path[static_cast<int>(path_length - 1)] != '\\') {
		full_path += "\\";
	}

	//
	//	Concatenate the filename onto the path
	//
	full_path += filename;

	return full_path;
}


/////////////////////////////////////////////////////////
//
//	Get_Temp_Filename
//
/////////////////////////////////////////////////////////
void
DuplicateRemoverClass::Get_Temp_Filename (StringClass &full_path)
{
	CString temp_path = static_cast<const char *>(Make_Path (TempDirectory, "tempfile"));
	
	//
	//	Try to find a unique temp filename
	//
	do {		
		full_path.Format ("%s%.5d.dat", (const char *)temp_path, TempFilenameStart++);
	} while (GetFileAttributes (full_path) != 0xFFFFFFFF);

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Delete_File
//
////////////////////////////////////////////////////////////////////////////
bool
DuplicateRemoverClass::Delete_File (LPCTSTR filename)
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


////////////////////////////////////////////////////////////////////////////
//
//  Is_File_In_Factory
//
////////////////////////////////////////////////////////////////////////////
bool
DuplicateRemoverClass::Is_File_In_Factory (const StringClass &filename, MixFileFactoryClass *factory)
{
	bool retval = false;

	//
	//	Get the list of filenames inside this mix file
	//
	DynamicVectorClass<StringClass> *test_filename_list = NULL;
	factory->Get_Filename_List (&test_filename_list);
	
	//
	//	Try to find the file inside this mix file
	//
	for (int index = 0; index < test_filename_list->Count (); index ++) {
		if (filename.Compare_No_Case ((*test_filename_list)[index]) == 0) {
			retval = true;
			break;
		}
	}

	return retval;
}


