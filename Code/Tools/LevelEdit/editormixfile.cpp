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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/editormixfile.cpp            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/12/01 12:38p                                              $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "filemgr.h"
#include "editormixfile.h"
#include "Utils.h"
#include "mixfile.h"
#include "TGAToDXT.h"
#include "LevelEdit.h"
#include "RegKeys.h" 
#include	<stdlib.h>
#include <winbase.h>


/////////////////////////////////////////////////////////////////////////
//
//	EditorMixFileCreator
//
/////////////////////////////////////////////////////////////////////////
EditorMixFileCreator::EditorMixFileCreator()
{
	// OPTIMIZATION: Store state of texture compression for quick access.
	TexturesCompressed = Are_Textures_Compressed();
}


/////////////////////////////////////////////////////////////////////////
//
//	Add_File
//
/////////////////////////////////////////////////////////////////////////
void
EditorMixFileCreator::Add_File (const char *full_path, const char *name)
{
	char substitutefullpath [MAX_PATH];
	char substitutename [MAX_PATH];
	
	// See if another file needs to be substituted for the input file.
	Substitute_File (full_path, name, substitutefullpath, substitutename);

	StringClass lower_name = substitutename;
	::strlwr (lower_name.Peek_Buffer ());

	//
	//	Is this file already in the system?
	//
	EditorMixFileEntry *stored_path = FilenameHash.Get (lower_name);
	if (stored_path != NULL) {
		
		//
		//	Don't store the new file unless its newer then the one already
		// in the hash
		//
		if (::Quick_Compare_Files (stored_path->Get_Path (), substitutefullpath) > 0) {
			
			//
			//	Remap this entry to the new path
			//
			stored_path->Set_Path (substitutefullpath);
		}

	} else {
		
		//
		//	Allocate a new entry...
		//
		stored_path = new EditorMixFileEntry;
		stored_path->Set_Name (lower_name);
		stored_path->Set_Path (substitutefullpath);

		//
		//	Now add this entry to the hash
		//
		FilenameHash.Insert (lower_name, stored_path);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Substitute_File
//
/////////////////////////////////////////////////////////////////////////
void EditorMixFileCreator::Substitute_File (const char *fullpath, const char *name, char *substitutefullpath, char *substitutename) 
{
	bool	  substitutefile;
	char	  filename  [_MAX_FNAME];
	char	  extension [_MAX_EXT]; 
	CString cachedfullpath;
	CString cachedname;

	// Assume that another file will not be substituted for the original file.
	substitutefile = false;

	// Has compression been enabled by user?
	if (TexturesCompressed) {

		// Is this file a .TGA? If so use a compressed (.DDS) version stored in the texture cache.
		_splitpath (fullpath, NULL, NULL, filename, extension);
		if (stricmp (extension, ".tga") == 0) {

			const char *exclusionstring0 = "bump_";
			const char *exclusionstring1 = "font";

			// Does this filename not start with one of the exclusion strings?
			// NOTE 0: bump_*.* files are bump maps and cannot be compressed because the pixel data is interpreted as bump data.
			// NOTE 1: font*.* files are font files that are not processed by the run-time compressed texture loading system and, therefore, must be excluded.
			if ((_strnicmp (filename, exclusionstring0, strlen (exclusionstring0)) != 0) &&
				 (_strnicmp (filename, exclusionstring1, strlen (exclusionstring1)) != 0)) {

				const char *ddsextension 	  = ".dds";

				char	  mangleddirectory [_MAX_DIR];
				char	  mangledname [_MAX_FNAME];
				HANDLE  tgafile;
				HANDLE  ddsfile;
				char   *s;

				// OPTIMIZATION: Has the .DDS file already been created and stored in the texture cache?
				
				// NOTE:	If the name contains a subdirectory mangle it with the filename. This will ensure that the filename is unique.
				_splitpath (name, NULL, mangleddirectory, mangledname, NULL);
				
   			// NOTE: The cached name must be the same as the original name but with .DDS extension.
				cachedname	= mangleddirectory;
				cachedname += mangledname;
				cachedname += ddsextension;

				// Replace backslashes in the mangled directory with dots to 'flatten' it.
				s = mangleddirectory;
				while (*s != '\0') {
					if ((*s == '\\') || (*s == '/')) {
						*s = '.';
					}
					s++;
				}

				cachedfullpath	 = ::Get_File_Mgr()->Get_Texture_Cache_Path();
				cachedfullpath += "\\";
				cachedfullpath += mangleddirectory;
				cachedfullpath += filename;
				cachedfullpath += ddsextension;

				// Does the cached .DDS file exist?
				tgafile = CreateFile (fullpath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0L, NULL);
				ddsfile = CreateFile (cachedfullpath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0L, NULL);
				if (tgafile != INVALID_HANDLE_VALUE) {
				
					FILETIME tgawritetime;

					GetFileTime (tgafile, NULL, NULL, &tgawritetime);
					if (ddsfile != INVALID_HANDLE_VALUE) {

						FILETIME ddswritetime;

						// Does the cached .DDS file have the same time stamp as the .TGA file?
						GetFileTime (ddsfile, NULL, NULL, &ddswritetime);
						CloseHandle (tgafile);
						CloseHandle (ddsfile);
						if ((tgawritetime.dwLowDateTime == ddswritetime.dwLowDateTime) && (tgawritetime.dwHighDateTime == ddswritetime.dwHighDateTime)) {

							// Files have the same time stamp. Substitute the .DDS file.
							substitutefile = true;
						
						} else {

							// Files have a different time stamp (and therefore it is assumed that the	.TGA file has been modified since the .DDS file was created).
							// Create a new .DDS file from the .TGA file and give it the same time stamp as the .TGA file.
							substitutefile = Convert_File (fullpath, cachedfullpath, &tgawritetime);
						}	
					
					} else {

						CloseHandle (tgafile);

						// Create a new .DDS file from the .TGA file and give it the same time stamp as the .TGA file.
						substitutefile = Convert_File (fullpath, cachedfullpath, &tgawritetime);
					}

				} else {

					CString message;
				
					// Cannot open the input file!	
					message.Format ("Failed to open: %s\r\n", fullpath);
					::Output_Message (message);
				}
			}
		}
	}

	if (substitutefile) {

		// Substitute the cached file.
		strcpy (substitutefullpath, cachedfullpath);
		strcpy (substitutename, cachedname);

	} else {

		// Refer the caller back to the original input file ie. no file is substituted.
		strcpy (substitutefullpath, fullpath);
		strcpy (substitutename, name);
	}
}


/////////////////////////////////////////////////////////////////////////
//
//	Generate_Mix_File
//
/////////////////////////////////////////////////////////////////////////
bool EditorMixFileCreator::Convert_File (const char *fullpath, const char *cachedfullpath, FILETIME *tgawritetimeptr)
{
	const char *failedtocompresstext = "Failed to compress %s\r\n";
	const char *removedalphatext		= "Removed alpha channel in %s\r\n";

	bool	  success, alpharemoved;
	CString message;
	
	success = _TGAToDXTConverter.Convert (fullpath, cachedfullpath, tgawritetimeptr, alpharemoved);
	if (!success) {
		message.Format (failedtocompresstext, fullpath);
		::Output_Message (message);
	}
	if (alpharemoved) {
		message.Format (removedalphatext, fullpath);
		::Output_Message (message);
	}

	return (success);
}


/////////////////////////////////////////////////////////////////////////
//
//	Generate_Mix_File
//
/////////////////////////////////////////////////////////////////////////
void
EditorMixFileCreator::Generate_Mix_File (const char *full_path)
{
	MixFileCreator	mix_file (full_path);

	//
	//	Loop over all the entries in hash table
	//
	HashTemplateIterator<StringClass, EditorMixFileEntry *> iterator (FilenameHash);	
	for (iterator.First (); iterator.Is_Done () == false; iterator.Next ()) {
		EditorMixFileEntry *entry = iterator.Peek_Value ();
		
		//
		// Add this file to the mix
		//
		mix_file.Add_File (entry->Get_Path (), entry->Get_Name ());
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Flush
//
/////////////////////////////////////////////////////////////////////////
void
EditorMixFileCreator::Flush (void)
{
	//
	//	Loop over all the entries in hash table
	//
	HashTemplateIterator<StringClass, EditorMixFileEntry *> iterator (FilenameHash);	
	for (iterator.First (); iterator.Is_Done () == false; iterator.Next ()) {
		
		//
		//	Free this entry
		//
		EditorMixFileEntry *entry = iterator.Peek_Value ();
		delete entry;
	}

	FilenameHash.Remove_All ();
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Set_Texture_Compression
//
/////////////////////////////////////////////////////////////////////////
void EditorMixFileCreator::Set_Texture_Compression (bool onoff)
{
	// Update the registry.
	theApp.WriteProfileInt (CONFIG_KEY, TEXTURE_COMPRESSION_VALUE, onoff);
}


/////////////////////////////////////////////////////////////////////////
//
//	Are_Textures_Compressed
//
/////////////////////////////////////////////////////////////////////////
bool EditorMixFileCreator::Are_Textures_Compressed()
{
	// Read from the registry.
	return (theApp.GetProfileInt (CONFIG_KEY, TEXTURE_COMPRESSION_VALUE, 0) == 1);
}
