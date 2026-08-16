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

#pragma once

#include <filesystem>

class cPathUtil
{
public:
	static std::filesystem::path GetWorkingDirectory();
	static bool SetWorkingDirectory(const std::filesystem::path &path);
	static bool SetWorkingDirectory(const char *path);
	static std::filesystem::path MakeAbsolutePath(const std::filesystem::path &path);
	static std::filesystem::path MakeAbsolutePath(const char *path);
	static std::filesystem::path GetExecutableDirectory();
	static std::filesystem::path GetUserConfigDirectory(const char *organization, const char *application);

	static std::filesystem::path ExtractFilename(const std::filesystem::path &path);

	static bool PathExists(const std::filesystem::path &path);
	static bool PathExists(const char *path);
	static bool EnsureDirectoryExists(const std::filesystem::path &path);
};
