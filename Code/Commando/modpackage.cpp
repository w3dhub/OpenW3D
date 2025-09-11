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
 *                 Project Name : commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/commando/modpackage.cpp                      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/14/02 4:09p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "modpackage.h"
#include "init.h"
#include "mixfile.h"
#include "realcrc.h"
#include "crc.h"
#include "ffactory.h"
#include "wwfile.h"


//////////////////////////////////////////////////////////////////////
//
//	ModPackageClass
//
//////////////////////////////////////////////////////////////////////
ModPackageClass::ModPackageClass (void)	:
	FileCRC (0)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~ModPackageClass
//
//////////////////////////////////////////////////////////////////////
ModPackageClass::~ModPackageClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Package_Filename
//
//////////////////////////////////////////////////////////////////////
void
ModPackageClass::Set_Package_Filename (const char *name)
{
	PackageFilename = name;

	//
	//	Name the package from its filename minus the file extension.
	//
	StringClass temp_str = name;
	char *extension		= ::strrchr (temp_str.Peek_Buffer (), '.');
	if (extension != NULL) {
		extension[0]	= 0;
		Name				= temp_str;
	} else {
		Name = name;
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Build_Level_List
//
//////////////////////////////////////////////////////////////////////
void
ModPackageClass::Build_Level_List (DynamicVectorClass<StringClass> &list) const
{
	MixFileFactoryClass factory (PackageFilename, &RenegadeBaseFileFactory);

	//
	//	Get a list of all the files in this mix file
	//
	DynamicVectorClass<StringClass> files_in_mix;
	files_in_mix.Set_Growth_Step (1000);
	factory.Build_Filename_List (files_in_mix);

	//
	//	Now, add any files that match the supplied wildcard to our master list
	//
	for (int file_index = 0; file_index < files_in_mix.Count (); file_index ++) {
		const char *filename = files_in_mix[file_index];
		StringClass temp_str (filename, true);
        temp_str.To_Lower();

		//
		//	Add this file to our list if it matches the mask
		//
		if (::strstr (filename, ".lsd") != 0) {
			list.Add (filename);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Find_Map_From_CRC
//
//////////////////////////////////////////////////////////////////////
bool
ModPackageClass::Find_Map_From_CRC (uint32 crc, StringClass *map_name) const
{
	bool retval = false;

	//
	//	Build a list of levels inside this mod
	//
	DynamicVectorClass<StringClass> list;
	Build_Level_List (list);

	//
	//	Check each level filename until we've found the one that
	//	matches the CRC
	//
	for (int index = 0; index < list.Count (); index ++) {
		if (::CRC_Stringi (list[index]) == crc) {
			(*map_name) = list[index];
			retval = true;
		}
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Compute_CRC
//
//////////////////////////////////////////////////////////////////////
void
ModPackageClass::Compute_CRC (void)
{
	FileCRC = 0;
	
	//
	//	Get a file object for the package
	//
	FileClass *file = _TheFileFactory->Get_File (PackageFilename);
	if (file != NULL) {

		//
		//	Open the file
		//
		if (file->Is_Available ()) {
			file->Open (FileClass::READ);

			CRCEngine crc_engine;

			//
			//	Crc the contents of the file
			//
			int file_size = file->Size ();
			uint8 buffer[4096];
			while (file_size > 0) {
				
				//
				//	Read the data from the source file
				//
				int bytes			= min (file_size, (int)sizeof (buffer));
				int copied_size	= file->Read (buffer, bytes);
				file_size			-= copied_size;
				if (copied_size <= 0) {
					break;
				}

				//
				//	Add this data to the CRC engine
				//
				crc_engine (buffer, copied_size);
			}

			//
			//	Get the accumulated CRC result from the CRC engine
			//
			FileCRC = crc_engine ();

			//
			//	Close the file
			//
			file->Close ();
		}
		
		//
		//	Return the file
		//
		_TheFileFactory->Return_File (file);
	}	

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_CRC
//
//////////////////////////////////////////////////////////////////////
uint32
ModPackageClass::Get_CRC (void)
{
	if (FileCRC == 0) {
		Compute_CRC ();
	}

	return FileCRC;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Map_Index
//
//////////////////////////////////////////////////////////////////////
int
ModPackageClass::Get_Map_Index (const char *map_name)
{
	int retval = 0;

	//
	//	Build the list of all maps in this mod
	//
	DynamicVectorClass<StringClass> list;
	Build_Level_List (list);

	//
	//	Try to find the map in the list
	//
	for (int index = 0; index < list.Count (); index ++) {
		if (list[index].Compare_No_Case (map_name) == 0) {
			retval = index;
			break;
		}
	}

	return retval;
}
