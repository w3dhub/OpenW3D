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

/*
** Linux implementation of RegistryClass backed by an INI file.
** Persists settings to $HOME/.config/renegade/registry.ini.
*/

#if !defined(_WIN32)

#include "registry.h"
#include "ini.h"
#include "inisup.h"
#include "rawfile.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

bool RegistryClass::IsLocked = false;

static const char *Registry_File_Path()
{
	static char path[512] = {};
	if (path[0] == '\0') {
		const char *home = getenv("HOME");
		if (home) {
			snprintf(path, sizeof(path), "%s/.config/renegade/registry.ini", home);
		} else {
			strncpy(path, "registry.ini", sizeof(path) - 1);
		}
	}
	return path;
}

static void Ensure_Registry_Dir()
{
	const char *home = getenv("HOME");
	if (!home) return;
	char dir[512];
	snprintf(dir, sizeof(dir), "%s/.config", home);
	mkdir(dir, 0755);
	snprintf(dir, sizeof(dir), "%s/.config/renegade", home);
	mkdir(dir, 0755);
}

static INIClass &Registry_INI()
{
	static INIClass ini;
	static bool loaded = false;
	if (!loaded) {
		loaded = true;
		RawFileClass file(Registry_File_Path());
		if (file.Is_Available()) {
			ini.Load(file);
		}
	}
	return ini;
}

static void Save_Registry_INI()
{
	Ensure_Registry_Dir();
	RawFileClass file(Registry_File_Path());
	Registry_INI().Save(file);
}

static const char *Key_Section(void *key)
{
	return reinterpret_cast<const char *>(key);
}

bool RegistryClass::Exists(const char *sub_key)
{
	return Registry_INI().Section_Present(sub_key);
}

RegistryClass::RegistryClass(const char *sub_key, bool create) :
	IsValid(false),
	Key(NULL)
{
	if (sub_key) {
		Key = strdup(sub_key);
		if (create && !IsLocked) {
			IsValid = true;
		} else {
			IsValid = Registry_INI().Section_Present(sub_key);
		}
	}
}

RegistryClass::~RegistryClass()
{
	if (Key) {
		free(Key);
		Key = NULL;
	}
	IsValid = false;
}

int RegistryClass::Get_Int(const char *name, int def_value)
{
	assert(IsValid);
	return Registry_INI().Get_Int(Key_Section(Key), name, def_value);
}

void RegistryClass::Set_Int(const char *name, int value)
{
	assert(IsValid);
	if (IsLocked) return;
	Registry_INI().Put_Int(Key_Section(Key), name, value);
	Save_Registry_INI();
}

bool RegistryClass::Get_Bool(const char *name, bool def_value)
{
	return (Get_Int(name, def_value ? 1 : 0) != 0);
}

void RegistryClass::Set_Bool(const char *name, bool value)
{
	Set_Int(name, value ? 1 : 0);
}

float RegistryClass::Get_Float(const char *name, float def_value)
{
	assert(IsValid);
	return Registry_INI().Get_Float(Key_Section(Key), name, def_value);
}

void RegistryClass::Set_Float(const char *name, float value)
{
	assert(IsValid);
	if (IsLocked) return;
	Registry_INI().Put_Float(Key_Section(Key), name, value);
	Save_Registry_INI();
}

char *RegistryClass::Get_String(const char *name, char *value, int value_size, const char *default_string)
{
	assert(IsValid);
	const char *def = (default_string != NULL) ? default_string : "";
	Registry_INI().Get_String(Key_Section(Key), name, def, value, value_size);
	return value;
}

void RegistryClass::Get_String(const char *name, StringClass &string, const char *default_string)
{
	assert(IsValid);
	const char *def = (default_string != NULL) ? default_string : "";
	Registry_INI().Get_String(string, Key_Section(Key), name, def);
}

void RegistryClass::Set_String(const char *name, const char *value)
{
	assert(IsValid);
	if (IsLocked) return;
	Registry_INI().Put_String(Key_Section(Key), name, value);
	Save_Registry_INI();
}

int RegistryClass::Get_Bin_Size(const char *name)
{
	assert(IsValid);
	unsigned char buf[8192];
	return Registry_INI().Get_UUBlock(Key_Section(Key), name, buf, sizeof(buf));
}

void RegistryClass::Get_Bin(const char *name, void *buffer, int buffer_size)
{
	assert(IsValid);
	assert(buffer != NULL);
	assert(buffer_size > 0);
	Registry_INI().Get_UUBlock(Key_Section(Key), name, buffer, buffer_size);
}

void RegistryClass::Set_Bin(const char *name, const void *buffer, int buffer_size)
{
	assert(IsValid);
	assert(buffer != NULL);
	assert(buffer_size > 0);
	if (IsLocked) return;
	Registry_INI().Put_UUBlock(Key_Section(Key), name, buffer, buffer_size);
	Save_Registry_INI();
}

void RegistryClass::Get_Value_List(DynamicVectorClass<StringClass> &list)
{
	const char *section = Key_Section(Key);
	int index = 0;
	const char *entry = NULL;
	while ((entry = Registry_INI().Get_Entry(section, index++)) != NULL) {
		list.Add(StringClass(entry));
	}
}

void RegistryClass::Delete_Value(const char *name)
{
	if (IsLocked) return;
	Registry_INI().Clear(Key_Section(Key), name);
	Save_Registry_INI();
}

void RegistryClass::Deleta_All_Values()
{
	if (IsLocked) return;
	Registry_INI().Clear(Key_Section(Key));
	Save_Registry_INI();
}

void RegistryClass::Delete_Registry_Tree(char *path)
{
	if (IsLocked) return;
	Registry_INI().Clear(path);
	Save_Registry_INI();
}

void RegistryClass::Save_Registry_Tree(char *path, INIClass *ini)
{
	const char *section = path;
	int index = 0;
	const char *entry = NULL;
	char buf[1024];
	while ((entry = Registry_INI().Get_Entry(section, index++)) != NULL) {
		Registry_INI().Get_String(section, entry, "", buf, sizeof(buf));
		ini->Put_String(section, entry, buf);
	}
}

void RegistryClass::Save_Registry(const char *filename, char *path)
{
	RawFileClass file(filename);
	INIClass ini;
	Save_Registry_Tree(path, &ini);
	ini.Save(file);
}

void RegistryClass::Load_Registry(const char *filename, char *old_path, char *new_path)
{
	if (IsLocked) return;

	RawFileClass file(filename);
	INIClass ini;
	ini.Load(file);

	const size_t old_path_len = ::strlen(old_path);
	char path[1024];
	char string[1024];
	unsigned char buffer[8192];

	List<INISection *> &section_list = ini.Get_Section_List();

	for (INISection *section = section_list.First(); section != NULL; section = section->Next_Valid()) {
		char *section_name = section->Section;
		strcpy(path, new_path);
		char *cut = strstr(section_name, old_path);
		if (cut) {
			strcat(path, cut + old_path_len);
		}

		RegistryClass reg(path);
		if (reg.Is_Valid()) {
			char *entry = (char*)1;
			int index = 0;

			while (entry) {
				entry = (char*)ini.Get_Entry(section_name, index++);
				if (entry) {
					if (strncmp(entry, "BIN_", 4) == 0) {
						int len = ini.Get_UUBlock(section_name, entry, buffer, sizeof(buffer));
						reg.Set_Bin(entry + 4, buffer, len);
					} else if (strncmp(entry, "DWORD_", 6) == 0) {
						int temp = ini.Get_Int(section_name, entry, 0);
						reg.Set_Int(entry + 6, temp);
					} else if (strncmp(entry, "STRING_", 7) == 0) {
						ini.Get_String(section_name, entry, "", string, sizeof(string));
						reg.Set_String(entry + 7, string);
					}
				}
			}
		}
	}
}

#endif // !_WIN32
