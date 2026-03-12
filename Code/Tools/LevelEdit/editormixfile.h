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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/editormixfile.h            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/10/01 12:17p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif


#ifndef __EDITOR_MIX_FILE__H
#define __EDITOR_MIX_FILE__H

#include "hashtemplate.h"


/////////////////////////////////////////////////////////////////////////
//	Forward declarations
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
//
//	EditorMixFileEntry
//
/////////////////////////////////////////////////////////////////////////
class EditorMixFileEntry
{
public:

	//////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////
	EditorMixFileEntry (void)		{}
	~EditorMixFileEntry (void)		{}

	//////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////
	//bool operator== (const EditorMixFileEntry &src);
	//bool operator!= (const EditorMixFileEntry &src)		{ return !src.Comp

	//////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////
	const char *	Get_Name (void)					{ return Name; }
	void				Set_Name (const char *name)	{ Name = name; }

	const char *	Get_Path (void)					{ return Path; }
	void				Set_Path (const char *path)	{ Path = path; }

private:

	//////////////////////////////////////////////////////////
	//	Private methods
	//////////////////////////////////////////////////////////
	StringClass		Path;
	StringClass		Name;
};


/////////////////////////////////////////////////////////////////////////
//
//	EditorMixFileCreator
//
/////////////////////////////////////////////////////////////////////////
class EditorMixFileCreator
{
public:

	//////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////
	 EditorMixFileCreator (void);
	~EditorMixFileCreator (void) { Flush (); }

	//////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////

	//
	//	File addition support (checks for duplicates and such)
	//
	void			Add_File (const char *full_path, const char *name);

	//
	//	Mix file creation support
	//
	void			Generate_Mix_File (const char *full_path);

	// Texture compression.
	static void Set_Texture_Compression (bool onoff);
	static bool Are_Textures_Compressed();

private:

	//////////////////////////////////////////////////////////
	//	Private methods
	//////////////////////////////////////////////////////////
	void			Substitute_File (const char *fullpath, const char *name, char *substitutefullpath, char *substitutename) ;
	bool			Convert_File (const char *fullpath, const char *cachedfullpath, FILETIME *tgawritetimeptr);
	void			Flush (void);

	//////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////
	HashTemplateClass<StringClass, EditorMixFileEntry *> FilenameHash;
	bool																  TexturesCompressed;
};

#endif //__EDITOR_MIX_FILE__H

