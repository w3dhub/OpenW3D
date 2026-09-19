/*
**	Command & Conquer Renegade(tm)
**	Copyright 2026 OpenW3D Contributors.
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

#include "tempfile.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <cstdlib>
#include <unistd.h>
#endif
#include <filesystem>

RawTempFileClass::RawTempFileClass(const char *prefix)
{
	Valid = false;
#ifdef _WIN32
	char temp_dir[MAX_PATH];
	char temp_file_path[MAX_PATH];

	DWORD len = GetTempPathA(MAX_PATH, temp_dir);
	if (len == 0 || len >= MAX_PATH) {
		return;
	}

	if (GetTempFileNameA(temp_dir, prefix ? prefix : "tmp-", 0, temp_file_path) == 0) {
		return;
	}
	Set_Name(temp_file_path);
	Valid = true;
#else
	std::filesystem::path path =
		std::filesystem::temp_directory_path() / (prefix ? prefix :"tmp-");
	std::string pattern = path.string();
	pattern += "XXXXXXXX";

	// mkstemp modifies pattern
	int fd = mkstemp(pattern.data());
	if (fd == -1) {
		return;
	}
	close(fd);
	Set_Name(pattern.data());
	Valid = true;
#endif
}

RawTempFileClass::~RawTempFileClass()
{
	Close();
}

void RawTempFileClass::Close()
{
	RawFileClass::Close();
	if (RemoveOnClose) {
		std::error_code ec;
		std::filesystem::remove(File_Name(), ec);

		if (ec) {
			WWDEBUG_SAY((
				"Failed to remove temporary file %s\n",
				File_Name()
			));
		}
	}
}
