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
 *                 Project Name : wwlib                                                        *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/rcfile.cpp                             $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 7/09/99 1:37p                                               *
 *                                                                                             *
 *                    $Revision:: 5                                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "rcfile.h"
#include <stdlib.h>
#include <string.h>

const char * RESOURCE_FILE_TYPE_NAME = "File";

#ifdef _WIN32

ResourceFileClass::ResourceFileClass(HMODULE hmodule, char const *filename) :
	ResourceName(NULL),
	hModule(NULL),
	FileBytes(NULL),
	FilePtr(NULL),
	EndOfFile(NULL)
{
	Set_Name(filename);
	if (hmodule != NULL) {
		hModule = hmodule;
	}
}

ResourceFileClass::~ResourceFileClass(void)
{
	if (ResourceName != NULL) {
		free(ResourceName);
	}
}

char const * ResourceFileClass::Set_Name(char const *filename)
{
	if (ResourceName != NULL) {
		free(ResourceName);
		ResourceName = NULL;
	}
	if (filename != NULL) {
		ResourceName = strdup(filename);
	}
	return ResourceName;
}

int ResourceFileClass::Create(void)
{
	return false;
}

int ResourceFileClass::Delete(void)
{
	return false;
}

bool ResourceFileClass::Is_Available(int forced)
{
	if (forced) {
		return Is_Open();
	}
	return true;
}

bool ResourceFileClass::Is_Open(void) const
{
	return (FileBytes != NULL);
}

int ResourceFileClass::Open(char const *fname, int rights)
{
	Set_Name(fname);
	return Open(rights);
}

int ResourceFileClass::Open(int rights)
{
	if (ResourceName == NULL) {
		return false;
	}

	if (Is_Open()) {
		return true;
	}

	HRSRC hresource = FindResourceA(hModule, ResourceName, RESOURCE_FILE_TYPE_NAME);
	if (hresource == NULL) {
		return false;
	}

	HGLOBAL hglobal = LoadResource(hModule, hresource);
	if (hglobal == NULL) {
		return false;
	}

	FileBytes = (unsigned char *)LockResource(hglobal);
	if (FileBytes == NULL) {
		return false;
	}

	EndOfFile = FileBytes + SizeofResource(hModule, hresource);
	FilePtr = FileBytes;
	return true;
}

int ResourceFileClass::Read(void * buffer, int size)
{
	if (!Is_Open()) {
		return 0;
	}

	int bytes_to_copy = size;
	if (FilePtr + bytes_to_copy > EndOfFile) {
		bytes_to_copy = (int)(EndOfFile - FilePtr);
	}
	if (bytes_to_copy > 0) {
		memcpy(buffer, FilePtr, bytes_to_copy);
		FilePtr += bytes_to_copy;
	}
	return bytes_to_copy;
}

int ResourceFileClass::Seek(int pos, int dir)
{
	if (!Is_Open()) {
		return 0;
	}

	unsigned char * new_ptr = NULL;
	switch (dir) {
		case SEEK_SET:
			new_ptr = FileBytes + pos;
			break;
		case SEEK_CUR:
			new_ptr = FilePtr + pos;
			break;
		case SEEK_END:
			new_ptr = EndOfFile + pos;
			break;
	}
	if (new_ptr < FileBytes || new_ptr > EndOfFile) {
		return 0;
	}
	FilePtr = new_ptr;
	return (int)(FilePtr - FileBytes);
}

int ResourceFileClass::Size(void)
{
	if (!Is_Open()) {
		return 0;
	}
	return (int)(EndOfFile - FileBytes);
}

int ResourceFileClass::Write(void const * buffer, int size)
{
	(void)buffer;
	(void)size;
	return 0;
}

void ResourceFileClass::Close(void)
{
	FileBytes = NULL;
	FilePtr = NULL;
	EndOfFile = NULL;
}

void ResourceFileClass::Error(int error, int canretry, char const * filename)
{
	(void)error;
	(void)canretry;
	(void)filename;
}

void ResourceFileClass::Bias(int start, int length)
{
	(void)start;
	(void)length;
}

#endif // _WIN32
