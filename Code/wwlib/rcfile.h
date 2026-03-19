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
 *                     $Archive:: /Commando/Code/wwlib/rcfile.h                               $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/02/01 1:21p                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef RCFILE_H
#define RCFILE_H

#include "always.h"
#include "wwfile.h"
#include "win.h"

#ifdef OPENW3D_SDL3
#include <cstdint>
#include <string>
#include <unordered_map>
#endif

/*
** ResourceFileClass
** This is a file class which allows you to read from a binary file that you have
** imported into your resources.
** - OPENW3D_WIN32:
**   Just import the file as a custom resource of the
**   type "File".  Replace the "id" of the resource with its filename (change
**   IDR_FILE1 to "MyFile.w3d") and then you will be able to access it by using this
**   class.
** - OPENW3D_SDL3:
**   Embed the file in a C++ source, and create a static StaticResourceFileClass object which
**   must remain valid for the duration of the program.
*/
class ResourceFileClass final : public FileClass
{
	public:
		ResourceFileClass(char const *filename);
#ifdef OPENW3D_WIN32
		ResourceFileClass(HMODULE hmodule, char const *filename);
#endif
		virtual ~ResourceFileClass(void);

		char const * File_Name(void) const override					{ return ResourceName; }
		char const * Set_Name(char const *filename) override;
		int Create(void) override											{ return false; }
		int Delete(void) override											{ return false; }
		bool Is_Available(int /*forced=false*/) override				{ return Is_Open (); }
		bool Is_Open(void) const override									{ return (FileBytes != NULL); }

		int Open(char const * /*fname*/, int /*rights=READ*/) override	{ return Is_Open(); }
		int Open(int /*rights=READ*/) override							{ return Is_Open(); }

		int Read(void *buffer, int size) override;
		int Seek(int pos, int dir=SEEK_CUR) override;
		int Size(void) override;
		int Write(void const * /*buffer*/, int /*size*/) override	{ return 0; }
		void Close(void) override											{ }
		void Error(int error, int canretry = false, char const * filename=NULL) override;
		void Bias(int /* start */, int /* length */=-1) override {}

		virtual const unsigned char *Peek_Data(void) const					{ return FileBytes; }

	protected:

		char *				ResourceName;

#ifdef OPENW3D_WIN32
		HMODULE				hModule;
#endif

		const unsigned char *	FileBytes;
		const unsigned char *	FilePtr;
		const unsigned char *	EndOfFile;

};

#ifdef OPENW3D_SDL3
struct StaticResourceFileClass {
	const char *filename;
	const std::uint8_t *data;
	size_t size;
};

// Executables can embed resources by defining a Static_Resources global variable.
// The CMake embed_resources macro does this automatically.
// This variable can be defined only ONCE for a executable.
extern std::unordered_map<std::string, StaticResourceFileClass> Static_Resources;

#endif

#endif
