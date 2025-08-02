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
#include "avassetsuck.h"
#include "ffactory.h"
#include "mixfile.h"
#include "rawfile.h"
#include "bittype.h"
#include "mixcombiningdialog.h"




AVAssetSuckerClass::AVAssetSuckerClass(void) :
	Dialog(NULL)
{
}



void AVAssetSuckerClass::Suck(char *input_file, char *output_file)
{
	strcpy(InputFile, input_file);
	strcpy(OutputFile, output_file);

	//
	//	Kick off the worker thread...
	//
	::AfxBeginThread (Do_Stuff, (LPVOID)this);

	//
	//	Show the UI
	//
	MixCombiningDialogClass dialog;
	Dialog = &dialog;
	dialog.DoModal();
}




unsigned int AVAssetSuckerClass::Do_Stuff(void *param)
{
	((AVAssetSuckerClass*)param)->Thread_Suck();
	return(1);
}


void AVAssetSuckerClass::Thread_Suck(void)
{
	char name[_MAX_PATH];

	while (Dialog == NULL) {
		Sleep(0);
	}

	/*
	** Get just the file name.
	*/
	char justname[_MAX_PATH];
	_splitpath(InputFile, NULL, NULL, justname, NULL);
	char text[_MAX_PATH + 128];
	sprintf(text, "Copying files from %s...", justname);

	Dialog->Set_Title("Stripping AV assets...");
	Dialog->Set_Status_Text(text);
	Dialog->Set_Progress_Percent (0);

	MixFileFactoryClass mix_in(InputFile, _TheFileFactory);
	if (mix_in.Is_Valid() && mix_in.Build_Internal_Filename_List()) {

		/*
		** Get the list of files in the source mix file.
		*/
		DynamicVectorClass<StringClass> filename_list;
		mix_in.Get_Filename_List(filename_list);

		/*
		** Create the output mix file.
		*/
		MixFileCreator mix_out(OutputFile);

		/*
		** Loop through all the files copying anything that isn't audio or texture.
		*/
		for (int i=0 ; i<filename_list.Count() ; i++) {

			strcpy(name, filename_list[i].Peek_Buffer());
			strupr(name);

			if (strstr(name, ".WAV") == 0) {
				if (strstr(name, ".TGA") == 0) {
					if (strstr(name, ".DDS") == 0) {
						if (strstr(name, ".MP3") == 0) {
							Copy_File(&mix_in, &mix_out, filename_list[i].Peek_Buffer());
						}
					}
				}
			}

			Dialog->Set_Progress_Percent((float)i / float(filename_list.Count() + 1));
		}
	}
	Dialog->PostMessage(WM_COMMAND, MAKELPARAM(IDOK, BN_CLICKED));
}




void AVAssetSuckerClass::Copy_File(MixFileFactoryClass *src_mix, MixFileCreator *dest_mix, char *filename)
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
