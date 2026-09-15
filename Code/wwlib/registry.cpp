/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**	Copyright 2025-2026 OpenW3D Contributors.
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
 *                 Project Name : Westwood Library                                             *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/registry.cpp                           $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 11/27/01 2:03p                                              $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "registry.h"
#include "rawfile.h"
#include "ini.h"
#include "inisup.h"
#include "openw3d.h"
#include <cassert>


bool RegistryClass::IsLocked = false;


bool RegistryClass::Exists(const char* sub_key)
{
	INIClass &config = OpenW3D::Get_Config();
	return config.Is_Present(sub_key, nullptr);
}

/*
**
*/
RegistryClass::RegistryClass( const char * sub_key, bool create ) :
	SubKey(sub_key)
{
	if (create) {
		IsValid = true;
	} else {
		IsValid = Exists(sub_key);
	}
}

RegistryClass::~RegistryClass( void )
{
	if (IsLocked) {
		return;
	}
	OpenW3D::Save_Config();
}

int	RegistryClass::Get_Int( const char * name, int def_value )
{
	INIClass &config = OpenW3D::Get_Config();
	return config.Get_Int(SubKey, name, def_value);
}

void	RegistryClass::Set_Int( const char * name, int value )
{
	assert( IsValid );
	if (IsLocked) {
		return;
	}
	INIClass &config = OpenW3D::Get_Config();
	config.Put_Int(SubKey, name, value);
}


bool	RegistryClass::Get_Bool( const char * name, bool def_value )
{
	INIClass &config = OpenW3D::Get_Config();
	return config.Get_Bool(SubKey, name, def_value);
}

void	RegistryClass::Set_Bool( const char * name, bool value )
{
	assert( IsValid );
	Set_Int( name, value ? 1 : 0 );
}


float	RegistryClass::Get_Float( const char * name, float def_value )
{
	INIClass &config = OpenW3D::Get_Config();
	return config.Get_Float(SubKey, name, def_value);
}

void	RegistryClass::Set_Float( const char * name, float value )
{
	assert( IsValid );
	if (IsLocked) {
		return;
	}
	INIClass &config = OpenW3D::Get_Config();
	config.Put_Float(SubKey, name, value);
}

void	RegistryClass::Get_String( const char * name, StringClass &string, const char *default_string )
{
	INIClass &config = OpenW3D::Get_Config();
	config.Get_String(string, SubKey, name, default_string);
}


void	RegistryClass::Get_String( const char * name, char *value, int value_size,
   const char * default_string )
{
	INIClass &config = OpenW3D::Get_Config();
	config.Get_String(SubKey, name, default_string, value, value_size);
}

void	RegistryClass::Set_String( const char * name, const char *value )
{
	assert( IsValid );
	if (IsLocked) {
		return;
	}
	INIClass &config = OpenW3D::Get_Config();
	config.Put_String(SubKey, name, value);
}

void	RegistryClass::Get_Value_List( DynamicVectorClass<StringClass> &list )
{
	INIClass &config = OpenW3D::Get_Config();
	//
	//	Simply enumerate all the values in this section
	//
	INISection *section = config.Find_Section(SubKey);
	if (!section) {
		return;
	}
	for (INIEntry *ini_entry = section->EntryList.First(); ini_entry != section->EntryList.Last(); ini_entry = ini_entry->Next()) {
		list.Add( ini_entry->Entry);
	}
}

void	RegistryClass::Delete_Value( const char * name)
{
	assert( IsValid );
	if (IsLocked) {
		return;
	}
	INIClass &config = OpenW3D::Get_Config();
	config.Clear(SubKey, name);
}

void	RegistryClass::Deleta_All_Values( void )
{
	assert( IsValid );
	if (IsLocked) {
		return;
	}
	INIClass &config = OpenW3D::Get_Config();
	config.Clear(SubKey);
}


//void	RegistryClass::Get_String( const unichar_t * name, WideStringClass &string, const unichar_t *default_string )
//{
//	assert( IsValid );
//	string = (default_string == nullptr) ? U_CHAR("") : default_string;
//
//	//
//	//	Get the size of the entry
//	//
//	DWORD data_size = 0;
//	DWORD type = 0;
//	LONG result = ::RegQueryValueExW ((HKEY)Key, reinterpret_cast<LPCWSTR>(name), nullptr, &type, nullptr, &data_size);
//	if (result == ERROR_SUCCESS && type == REG_SZ) {
//
//		//
//		//	Read the entry from the registry
//		//
//		::RegQueryValueExW ((HKEY)Key, reinterpret_cast<LPCWSTR>(name), nullptr, &type,
//			(LPBYTE)string.Get_Buffer ((data_size / 2) + 1), &data_size);
//	}
//
//	return ;
//}
//
//
//void	RegistryClass::Set_String( const unichar_t * name, const unichar_t *value )
//{
//	assert( IsValid );
//
//	//
//	//	Determine the size
//	//
//	const size_t size = (::u_strlen( value ) + 1) * sizeof(unichar_t);
//
//	//
//	//	Set the registry key
//	//
//	if (IsLocked) {
//		return;
//	}
//	WWASSERT(size <= std::numeric_limits<DWORD>::max());
//	::RegSetValueExW ( (HKEY)Key, reinterpret_cast<LPCWSTR>(name), 0, REG_SZ, (LPBYTE)value, static_cast<DWORD>(size) );
//	return ;
//}

