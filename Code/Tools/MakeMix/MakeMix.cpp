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
#include "StdAfx.h"
#include "mixfile.h"
#include <filesystem>
#include <system_error>
#include <string>
#include <algorithm>


// Private functions.
static unsigned Add_Files (const std::filesystem::path &basepath, const std::filesystem::path &subpath, MixFileCreator &mixfile);

int main (int argc, char *argv[])
{
	// Must have at least 3 command line arguments - the executable, a full source path, and a mixfile name.
	if (argc >= 3) {

		MixFileCreator	mixfile (argv [argc - 1]);

		for (int c = 1; c < argc - 1; c++) {

			unsigned		filecount;
			std::filesystem::path basepath (argv [c]);
			std::filesystem::path subpath;

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


unsigned Add_Files (const std::filesystem::path &basepath, const std::filesystem::path &subpath, MixFileCreator &mixfile)
{
	unsigned filecount = 0;
	std::filesystem::path searchpath = basepath / subpath;
	std::error_code ec;
	std::filesystem::directory_iterator iterator(searchpath, ec);
	const std::filesystem::directory_iterator end;
	if (!ec) {
		while (iterator != end) {

			const std::filesystem::path filename = iterator->path().filename();
			const std::string name = filename.string();

			// Skip names beginning with a dot.
			if (!name.empty() && name[0] != '.') {

				std::filesystem::path subpathname = subpath / filename;

				// Is it a subdirectory?
				std::filesystem::file_status status = iterator->symlink_status(ec);
				if (ec) {
					break;
				}
				if (std::filesystem::is_directory(status)) {

					// Recurse on subdirectory.
					filecount += Add_Files (basepath, subpathname, mixfile);

				} else if (std::filesystem::is_regular_file(status)) {

					std::string fullpathname = (basepath / subpathname).string();
					std::string savedname = subpathname.generic_string();
					std::replace(savedname.begin(), savedname.end(), '/', '\\');
					mixfile.Add_File (fullpathname.c_str(), savedname.c_str());
					filecount++;
				}
			}
			iterator.increment(ec);
			if (ec) {
				break;
			}
		}
	}
	if (ec) {
		fprintf(stderr, "Unable to read directory %s: %s\n",
			searchpath.string().c_str(), ec.message().c_str());
	}
	return (filecount);
}
