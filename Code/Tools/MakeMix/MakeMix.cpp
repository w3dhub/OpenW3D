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

// MakeMix.cpp : Defines the entry point for the console application.
//

// Includes.
#include "stdafx.h"
#include "MixFile.h"


// Private functions.
static unsigned Add_Files (const StringClass &basepath, const StringClass &subpath, MixFileCreator &mixfile);

int main (int argc, char *argv[])
{
	// Must have at least 3 command line arguments - the executable, a full source path, and a mixfile name.
	if (argc >= 3) {

		MixFileCreator	mixfile (argv [argc - 1]);

		for (int c = 1; c < argc - 1; c++) {

			unsigned		filecount;
			StringClass basepath (argv [c]);
			StringClass subpath;
			
			filecount = Add_Files (basepath, subpath, mixfile);
			if (filecount > 0) {
				printf ("%u files added\n", filecount);
			} else {
				printf ("No files found in source directory\n");
			}
		}

	} else {
		printf ("Usage - MakeMix <source directory0>..<source directory n> <mixfilename>\n");
	}

	return (0);
}


unsigned Add_Files (const StringClass &basepath, const StringClass &subpath, MixFileCreator &mixfile)
{
	const char wildcardname [] = "*.*";

	unsigned			 filecount;
	StringClass		 findfilepathname;	
	WIN32_FIND_DATAA finddata;	
	HANDLE			 handle;

	filecount = 0;
	if (basepath.Get_Length() > 0) {
		findfilepathname  = basepath;
		findfilepathname += "\\";
	}
	if (subpath.Get_Length() > 0) {
		findfilepathname += subpath;
		findfilepathname += "\\";
	}
	findfilepathname += wildcardname;
	handle = FindFirstFileA (findfilepathname, &finddata);
	if (handle != INVALID_HANDLE_VALUE) {
		
		bool done;

		done = false;
		while (!done) {

			// Filter out system files.
  			if (finddata.cFileName [0] != '.') {

				StringClass subpathname;

				if (subpath.Get_Length() > 0) {
					subpathname += subpath;
					subpathname += "\\";
				}
				subpathname += finddata.cFileName;

				// Is it a subdirectory?
	  			if ((finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				
					// Recurse on subdirectory.
					filecount += Add_Files (basepath, subpathname, mixfile);

				} else {

					StringClass fullpathname;

					if (basepath.Get_Length() > 0) {
						fullpathname += basepath;
						fullpathname += "\\";
					}
					if (subpath.Get_Length() > 0) {
						fullpathname += subpath;
						fullpathname += "\\";
					}
					fullpathname += finddata.cFileName;
					mixfile.Add_File (fullpathname, subpathname);
					filecount++;
				}
			}
			done = !FindNextFileA (handle, &finddata);
		}
	}
	return (filecount);
}