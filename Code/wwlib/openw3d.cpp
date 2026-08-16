/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 OpenW3D Contributors.
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

#include "openw3d.h"

#include "ini.h"
#include "pathutil.h"
#include "wwstring.h"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace
{
	constexpr char CONFIG_ORGANIZATION[] = "W3DHub";
	constexpr char CONFIG_APPLICATION[] = "OpenW3D";

	std::string g_config_file_path;
	bool g_config_file_path_initialized = false;
	bool g_config_file_path_override = false;

	std::vector<std::string> Tokenize_Command_Line(const char *command_line)
	{
		std::vector<std::string> args;

		if (command_line == nullptr) {
			return args;
		}

		const char *current = command_line;
		while (*current != '\0') {
			while (*current != '\0' && std::isspace(static_cast<unsigned char>(*current))) {
				++current;
			}

			if (*current == '\0') {
				break;
			}

			std::string arg;
			bool in_quotes = false;
			while (*current != '\0') {
				if (*current == '"') {
					in_quotes = !in_quotes;
					++current;
					continue;
				}

				if (!in_quotes && std::isspace(static_cast<unsigned char>(*current))) {
					break;
				}

				arg.push_back(*current);
				++current;
			}

			args.push_back(arg);
		}

		return args;
	}

	std::filesystem::path Get_Default_Config_File_Path()
	{
		const char *config_override = std::getenv("OPENW3D_CONFIG_INI");
		if (config_override == nullptr || config_override[0] == '\0') {
			config_override = std::getenv("RENEGADE_CONFIG_INI");
		}
		if (config_override != nullptr && config_override[0] != '\0') {
			return cPathUtil::MakeAbsolutePath(config_override);
		}

		const std::filesystem::path executable_dir = cPathUtil::GetExecutableDirectory();
		const std::filesystem::path portable_config = executable_dir / W3D_CONF_FILENAME;
		if (cPathUtil::PathExists(portable_config)) {
			return portable_config;
		}

#if defined(OPENW3D_WIN32) || defined(OPENW3D_SDL3)
		std::filesystem::path config_dir =
			cPathUtil::GetUserConfigDirectory(CONFIG_ORGANIZATION, CONFIG_APPLICATION);
		if (!config_dir.empty()) {
			return config_dir / W3D_CONF_FILENAME;
		}
#endif

		return portable_config;
	}

	void Store_Config_File_Path(const std::filesystem::path &path, bool is_override)
	{
		const std::filesystem::path resolved_path = path.empty() ? std::filesystem::path(W3D_CONF_FILENAME) : path;
		g_config_file_path = resolved_path.string();
		g_config_file_path_initialized = true;
		g_config_file_path_override = is_override;
	}
}

void OpenW3D::Set_Config_File_Path_Override(const char *path)
{
	Store_Config_File_Path(cPathUtil::MakeAbsolutePath(path), true);
}

bool OpenW3D::Set_Config_File_Path_From_Command_Line(int argc, char *argv[])
{
	for (int index = 1; index < argc; ++index) {
		if (std::strcmp(argv[index], "--ini") != 0) {
			continue;
		}

		++index;
		if (index >= argc || argv[index] == nullptr || argv[index][0] == '\0') {
			return false;
		}

		Set_Config_File_Path_Override(argv[index]);
	}

	return true;
}

bool OpenW3D::Set_Config_File_Path_From_Command_Line(const char *command_line)
{
	const std::vector<std::string> args = Tokenize_Command_Line(command_line);
	for (size_t index = 0; index < args.size(); ++index) {
		if (args[index] != "--ini") {
			continue;
		}

		++index;
		if (index >= args.size() || args[index].empty()) {
			return false;
		}

		Set_Config_File_Path_Override(args[index].c_str());
	}

	return true;
}

const char *OpenW3D::Get_Config_File_Path()
{
	if (!g_config_file_path_initialized) {
		Store_Config_File_Path(Get_Default_Config_File_Path(), false);
	}

	return g_config_file_path.c_str();
}

bool OpenW3D::Has_Config_File_Path_Override()
{
	return g_config_file_path_override;
}

bool OpenW3D::Command_Line_Has_Arg(const char *command_line, const char *arg)
{
	const std::vector<std::string> args = Tokenize_Command_Line(command_line);
	for (const std::string &current : args) {
		if (current == arg) {
			return true;
		}
	}

	return false;
}

void OpenW3D::Append_Config_File_Arg(StringClass &command_line)
{
	if (!Has_Config_File_Path_Override()) {
		return;
	}

	command_line += " --ini \"";
	command_line += Get_Config_File_Path();
	command_line += "\"";
}

bool OpenW3D::Save_Config(const INIClass &ini)
{
	const std::filesystem::path config_path = Get_Config_File_Path();
	if (!cPathUtil::EnsureDirectoryExists(config_path.parent_path())) {
		return false;
	}

	const std::string native_path = config_path.string();
	return ini.Save(native_path.c_str()) != 0;
}
