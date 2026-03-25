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
#pragma once

class INIClass;
class StringClass;

namespace OpenW3D
{
	void Set_Config_File_Path_Override(const char *path);
	bool Set_Config_File_Path_From_Command_Line(int argc, char *argv[]);
	bool Set_Config_File_Path_From_Command_Line(const char *command_line);
	const char *Get_Config_File_Path();
	bool Has_Config_File_Path_Override();
	bool Command_Line_Has_Arg(const char *command_line, const char *arg);
	bool Save_Config(const INIClass &ini);
}

// Some defines for moving game config to ini files rather than registry.
#define W3D_CONF_FILENAME "openw3d.conf"
#define W3D_CONF_FILE OpenW3D::Get_Config_File_Path()
#define W3D_SECTION_RENDER "RenderDevice"
#define W3D_SECTION_SOUND "Sound"
#define W3D_SECTION_SYSTEM "System"
#define W3D_SECTION_OPTIONS "Options"
