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

#include "StdAfx.h"
#include "mixpatchmaker.h"
#include "ffactory.h"
#include "mixfile.h"
#include "rawfile.h"
#include "bittype.h"
#include "mixcombiningdialog.h"




MixPatchMakerClass::MixPatchMakerClass(void) :
	Dialog(NULL)
{
}



void MixPatchMakerClass::Make_Patch(char *old_input_file, char *new_input_file, char *output_file, char *old_art_dir, char *new_art_dir)
{
	strcpy(OldInputFile, old_input_file);
	strcpy(NewInputFile, new_input_file);
	strcpy(OutputFile, output_file);
	strcpy(OldArtDir, old_art_dir);
	strcpy(NewArtDir, new_art_dir);

	//
	//	Kick off the worker thread...
	//
	::AfxBeginThread(Do_Stuff, (LPVOID)this);

	//
	//	Show the UI
	//
	MixCombiningDialogClass dialog;
	Dialog = &dialog;
	dialog.DoModal();
}




unsigned int MixPatchMakerClass::Do_Stuff(void *param)
{
	((MixPatchMakerClass*)param)->Thread_Make();
	return(1);
}


void MixPatchMakerClass::Thread_Make(void)
{
	char name[_MAX_PATH];
	char name_compare[_MAX_PATH];

	while (Dialog == NULL) {
		Sleep(0);
	}

	/*
	** Get just the file name.
	*/
	char justname[_MAX_PATH];
	char justname_new[_MAX_PATH];
	_splitpath(OldInputFile, NULL, NULL, justname, NULL);
	_splitpath(NewInputFile, NULL, NULL, justname_new, NULL);
	char text[_MAX_PATH + 128];
	sprintf(text, "Looking for differences between %s and %s...", justname, justname_new);

	Dialog->Set_Title("Building patch mixfile...");
	Dialog->Set_Status_Text(text);
	Dialog->Set_Progress_Percent (0);

	MixFileFactoryClass old_mix(OldInputFile, _TheFileFactory);
	MixFileFactoryClass new_mix(NewInputFile, _TheFileFactory);
	if (new_mix.Is_Valid() && new_mix.Build_Internal_Filename_List()) {
		if (old_mix.Is_Valid() && old_mix.Build_Internal_Filename_List()) {

			/*
			** Get the list of files in the new source mix file.
			*/
			DynamicVectorClass<StringClass> new_filename_list;
			new_mix.Get_Filename_List(new_filename_list);

			/*
			** Get the list of files in the old source mix file.
			*/
			DynamicVectorClass<StringClass> old_filename_list;
			old_mix.Get_Filename_List(old_filename_list);

			/*
			** Create the output mix file.
			*/
			MixFileCreator mix_out(OutputFile);

			/*
			** Loop through all the files in the new source mix. If a file doesn't exist in the old one, or it does exist but is
			** different, then copy it into the output mix file.
			*/
			for (int i=0 ; i<new_filename_list.Count() ; i++) {

				strcpy(name, new_filename_list[i].Peek_Buffer());
                openw3d::string_to_upper(name);
				bool copy = false;

				if (stricmp(name, "STRINGS.TDB") == 0) {
					continue;
				}

				/*
				** Search for the file in the old mixfile.
				*/
				bool found = false;
				for (int j=0 ; j<old_filename_list.Count() ; j++) {
					strcpy(name_compare, old_filename_list[j].Peek_Buffer());
					if (stricmp(name, name_compare) == 0) {
						found = true;

						/*
						** There's a file with the same name in the old mix file. See if it's byte for byte identical. If it's not
						** then we need to use the newer file in the patch mix.
						*/
						bool same = Compare_File(&new_mix, &old_mix, new_filename_list[i].Peek_Buffer());
						if (!same) {

							/*
							** If it's a .dds (texture) file, compare the source art for changes. The .dds can change even when the
							** source art doesn't.
							*/
							if (strstr(name, ".DDS")) {
								char targa_name[_MAX_PATH];
								strcpy(targa_name, name);
								char *ext = strstr(targa_name, ".DDS");
								if (ext) {
									strcpy(ext, ".TGA");

									/*
									** If the two source art files are the same then break out.
									*/
									if (Compare_Source_Art_File(targa_name)) {
										break;
									}
								}
							}

							copy = true;
						}
						break;
					}
				}

				/*
				** If the file doesn't exist at all in the older mixfile then it needs to be in the patch mix.
				*/
				if (!found) {
					copy = true;
				}

				if (copy) {
					Copy_File(&new_mix, &mix_out, new_filename_list[i].Peek_Buffer());
				}

				Dialog->Set_Progress_Percent((float)i / float(new_filename_list.Count() + 1));
			}
		}
	}
	Dialog->PostMessage(WM_COMMAND, MAKELPARAM(IDOK, BN_CLICKED));
}




void MixPatchMakerClass::Copy_File(MixFileFactoryClass *src_mix, MixFileCreator *dest_mix, char *filename)
{
	//
	//	Get the file data from the source mix
	//
	FileClass *src_file = src_mix->Get_File (filename);
	src_file->Open ();
	int size = src_file->Size();

	//
	//	Create a temporary destination file for the data
	//
	char temp_file_name[_MAX_PATH];
	char temp_path[_MAX_PATH];
	int chars = GetTempPath(_MAX_PATH, temp_path);
	if (chars) {
		int res = GetTempFileName(temp_path, "MIX", 0, temp_file_name);
		if (res == 0) {
			WWDEBUG_SAY(("GetTempFileName failed with error code %d\n", GetLastError()));
		} else {
			RawFileClass temp_file(temp_file_name);
			if (temp_file.Open(RawFileClass::WRITE)) {

				//
				// Save the data in the temp file.
				//
				void *bigbuf = new char [size + 1024];
				src_file->Read(bigbuf, size);
				temp_file.Write(bigbuf, size);
				delete [] bigbuf;
				temp_file.Close();

				//
				// Add the temp file to the mix file.
				//
				dest_mix->Add_File(temp_file_name, filename);

				//
				// Delete the temp file.
				//
				DeleteFile(temp_file_name);
			}
		}
	}

	src_mix->Return_File(src_file);
}





/***********************************************************************************************
 * MixPatchMakerClass::Compare_File -- Compare two files in seperate mixfiles.                 *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    First mixfile                                                                     *
 *           Second mixfile                                                                    *
 *           Name of file to compare                                                           *
 *                                                                                             *
 * OUTPUT:   true if files are the same                                                        *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/5/2002 9:54PM ST : Created                                                              *
 *=============================================================================================*/
bool MixPatchMakerClass::Compare_File(MixFileFactoryClass *src_mix, MixFileFactoryClass *dest_mix, char *filename)
{
	/*
	**	Get the file data from the source mix
	*/
	FileClass *src_file = src_mix->Get_File(filename);
	int size = src_file->Size();


	/*
	**	Get the file data from the dest mix
	*/
	FileClass *dest_file = dest_mix->Get_File(filename);

	/*
	** If the two files are a different size then we know they aren't the same.
	*/
	if (size != dest_file->Size()) {
		src_mix->Return_File(src_file);
		dest_mix->Return_File(dest_file);
		return(false);
	}

	/*
	** Read the source file into memory.
	*/
	src_file->Open();
	unsigned char *src_data = new unsigned char [size + 32];
	src_file->Read(src_data, size);
	src_file->Close();

	/*
	** Read the dest file into memory.
	*/
	dest_file->Open();
	unsigned char *dest_data = new unsigned char [size + 32];
	dest_file->Read(dest_data, size);
	dest_file->Close();

	/*
	** Compare them.
	*/
	bool same = (memcmp(src_data, dest_data, size) == 0) ? true : false;

	/*
	** Clean up.
	*/
	delete [] src_data;
	delete [] dest_data;
	src_mix->Return_File(src_file);
	dest_mix->Return_File(dest_file);

	return(same);
}











/***********************************************************************************************
 * MixPatchMakerClass::Compare_Source_Art_File -- Compare source art .tga files                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Filename                                                                          *
 *                                                                                             *
 * OUTPUT:   True if same                                                                      *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/21/2002 4:31PM ST : Created                                                             *
 *=============================================================================================*/
bool MixPatchMakerClass::Compare_Source_Art_File(char *filename)
{
	/*
	** Find the file in the old source art directory.
	*/
	char path_to_old_file[_MAX_PATH + 256];
	bool got = Find_File(filename, OldArtDir, path_to_old_file);

	if (got) {

		/*
		** Find the file in the old source art directory.
		*/
		char path_to_new_file[_MAX_PATH + 256];
		got = Find_File(filename, NewArtDir, path_to_new_file);

		if (got) {

			/*
			** Both files exist. Load them and compare.
			*/
			RawFileClass file1(path_to_old_file);
			RawFileClass file2(path_to_new_file);

			if (file1.Size() == file2.Size()) {

				int size = file1.Size();

				unsigned char * buf1 = new unsigned char [size + 1024];
				unsigned char * buf2 = new unsigned char [size + 1024];

				file1.Read(buf1, size);
				file2.Read(buf2, size);
				int diff = memcmp(buf1, buf2, size);

				delete [] buf1;
				delete [] buf2;

				if (diff == 0) {
					return(true);
				}
			}
		}
	}
	return(false);
}





/***********************************************************************************************
 * MixPatchMakerClass::Find_File -- Find a file in a directory or sub directory                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Name of file to search for                                                        *
 *           Path to start search                                                              *
 *           Ptr to string to receive path/filename of file if found                           *
 *                                                                                             *
 * OUTPUT:   True if file was found                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/21/2002 8:45PM ST : Created                                                             *
 *=============================================================================================*/
bool MixPatchMakerClass::Find_File(char *file_name, char *path, char *found_path)
{
	/*
	** Look in the current directory.
	*/
	char path_to_file[_MAX_PATH + 256];
	char *pointless_pointer = NULL;
	int len = SearchPath(path, file_name, NULL, sizeof(path_to_file), path_to_file, &pointless_pointer);

	if (len) {
		strcpy(found_path, path_to_file);
		return(true);
	}

	/*
	** Look for a subdirectory to search.
	*/
	WIN32_FIND_DATA find_info	= { 0 };
	HANDLE find_handle;
	char search_path[_MAX_PATH + 128];
	strcpy(search_path, path);
	strcat(search_path, "\\*.*");
	find_handle = FindFirstFile(search_path, &find_info);

	while (find_handle != INVALID_HANDLE_VALUE) {
		if ((find_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			if (strcmp(find_info.cFileName, ".") != 0) {
				if (strcmp(find_info.cFileName, "..") != 0) {
					char new_dir_name[_MAX_PATH + 128];
					strcpy(new_dir_name, path);
					strcat(new_dir_name, "\\");
					strcat(new_dir_name, find_info.cFileName);
					bool found = Find_File(file_name, new_dir_name, found_path);
					if (found) {
						FindClose(find_handle);
						return(true);
					}
				}
			}
		}
		if (FindNextFile(find_handle, &find_info) == 0) {
			FindClose(find_handle);
			break;
		}
	}

	return(false);
}
