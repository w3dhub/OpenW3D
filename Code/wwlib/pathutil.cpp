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

#include "pathutil.h"

#include <cstdlib>
#include <system_error>

#if defined(OPENW3D_WIN32)
#include <windows.h>
#include <shlobj.h>
#elif defined(OPENW3D_SDL3)
#include <SDL3/SDL_filesystem.h>
#endif

std::filesystem::path cPathUtil::GetWorkingDirectory()
{
	std::error_code error;
	std::filesystem::path current_dir = std::filesystem::current_path(error);
	return error ? std::filesystem::path(".") : current_dir;
}

bool cPathUtil::SetWorkingDirectory(const std::filesystem::path &path)
{
	std::error_code error;
	std::filesystem::current_path(path, error);
	return !error;
}

bool cPathUtil::SetWorkingDirectory(const char *path)
{
	return path != nullptr && SetWorkingDirectory(std::filesystem::path(path));
}

std::filesystem::path cPathUtil::MakeAbsolutePath(const std::filesystem::path &path)
{
	if (path.empty()) {
		return std::filesystem::path();
	}

	std::filesystem::path resolved_path = path;
	if (!resolved_path.is_absolute()) {
		resolved_path = GetWorkingDirectory() / resolved_path;
	}

	return resolved_path.lexically_normal();
}

std::filesystem::path cPathUtil::MakeAbsolutePath(const char *path)
{
	return path == nullptr ? std::filesystem::path() : MakeAbsolutePath(std::filesystem::path(path));
}

std::filesystem::path cPathUtil::GetExecutableDirectory()
{
#if defined(OPENW3D_WIN32)
	char path[32768] = { 0 };
	DWORD length = GetModuleFileNameA(nullptr, path, sizeof(path));
	if (length > 0 && length < sizeof(path)) {
		return std::filesystem::path(path).parent_path();
	}
#elif defined(OPENW3D_SDL3)
	const char *base_path = SDL_GetBasePath();
	if (base_path != nullptr && base_path[0] != '\0') {
		return std::filesystem::path(base_path);
	}
#endif

	return GetWorkingDirectory();
}

std::filesystem::path cPathUtil::GetUserConfigDirectory(const char *organization, const char *application)
{
#if defined(OPENW3D_WIN32)
	const char *appdata = std::getenv("APPDATA");
	if (appdata != nullptr && appdata[0] != '\0') {
		return MakeAbsolutePath(appdata) / organization / application;
	}

	std::filesystem::path appdata_path;
	HMODULE shfolder = LoadLibraryA("shfolder.dll");
	if (shfolder != nullptr) {
		typedef HRESULT(__stdcall *SHGetFolderPathAType)(HWND, int, HANDLE, DWORD, LPSTR);
		SHGetFolderPathAType get_folder_path =
			(SHGetFolderPathAType)GetProcAddress(shfolder, "SHGetFolderPathA");
		if (get_folder_path != nullptr) {
			char path[MAX_PATH] = { 0 };
			if (get_folder_path(nullptr, CSIDL_APPDATA, nullptr, 0, path) == S_OK) {
				appdata_path = std::filesystem::path(path);
			}
		}
		FreeLibrary(shfolder);
	}

	return appdata_path.empty() ? std::filesystem::path() : appdata_path / organization / application;
#elif defined(OPENW3D_SDL3)
	char *pref_path = SDL_GetPrefPath(organization, application);
	if (pref_path != nullptr && pref_path[0] != '\0') {
		const std::filesystem::path config_path(pref_path);
		SDL_free(pref_path);
		return config_path;
	}
	SDL_free(pref_path);

	return std::filesystem::path();
#else
	#error OpenW3D does not have an implementation of cPathUtil::GetUserConfigDirectory() for this platform
#endif
}

std::filesystem::path cPathUtil::ExtractFilename(const std::filesystem::path &path)
{
	return path.filename();
}

bool cPathUtil::PathExists(const std::filesystem::path &path)
{
	std::error_code error;
	return std::filesystem::exists(path, error) && !error;
}

bool cPathUtil::PathExists(const char *path)
{
	return path != nullptr && PathExists(std::filesystem::path(path));
}

bool cPathUtil::EnsureDirectoryExists(const std::filesystem::path &path)
{
	if (path.empty()) {
		return true;
	}

	std::error_code error;
	std::filesystem::create_directories(path, error);
	return !error;
}
