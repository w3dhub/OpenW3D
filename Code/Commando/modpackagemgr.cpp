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
 *                 Project Name : commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/modpackagemgr.cpp                   $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 7/09/02 11:13a                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "modpackagemgr.h"

#include "registry.h"
#include "_globals.h"
#include "realcrc.h"
#include "ffactorylist.h"
#include "ffactory.h"
#include "mixfile.h"
#include "gametype.h"


/////////////////////////////////////////////////////////////////////
//	Local constants
/////////////////////////////////////////////////////////////////////
static const char *	CURR_MOD_REG_VALUE	= "CurrModPackage";


//////////////////////////////////////////////////////////////////////
//	Static member initialization
//////////////////////////////////////////////////////////////////////
DynamicVectorClass<ModPackageClass>	ModPackageMgrClass::PackageList;
ModPackageClass							ModPackageMgrClass::CurrentPackage;


/////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////
void
ModPackageMgrClass::Initialize (void)
{
	Shutdown ();

// (gth) Day 120 patch, don't re-load the package name from the registry
#if 0
	//
	//	Get the currently selected package name from the registry
	//
	RegistryClass registry (APPLICATION_SUB_KEY_NAME_OPTIONS);
	if (registry.Is_Valid ()) {

		//
		//	Get the current package name from the registry
		//
		StringClass package_filename;
		registry.Get_String (CURR_MOD_REG_VALUE, package_filename, "");

		//
		//	Initialize the current package
		//
		CurrentPackage.Set_Package_Filename (package_filename);
	}
#endif

	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	Shutdown
//
//////////////////////////////////////////////////////////////////////
void
ModPackageMgrClass::Shutdown (void)
{
	Reset_List ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Build_List
//
//////////////////////////////////////////////////////////////////////
void
ModPackageMgrClass::Build_List (void)
{
	WIN32_FIND_DATAA find_info	= { 0 };
	BOOL keep_going				= true;
	HANDLE file_find				= NULL;

	//
	//	Build a list of all the saved games we know about
	//
	for (file_find = ::FindFirstFileA ("data/*.pkg", &find_info);
		 (file_find != INVALID_HANDLE_VALUE) && keep_going;
		  keep_going = ::FindNextFileA (file_find, &find_info))
	{		
		//
		//	Create the package from the data in this mix file
		//
		ModPackageClass package;
		package.Set_Package_Filename (find_info.cFileName);

		//
		//	Add the package to our list
		//
		PackageList.Add (package);
	}

	if (file_find != INVALID_HANDLE_VALUE) {			  
		::FindClose (file_find); 
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Reset_List
//
//////////////////////////////////////////////////////////////////////
void
ModPackageMgrClass::Reset_List (void)
{
	PackageList.Delete_All ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Current_Package
//
//////////////////////////////////////////////////////////////////////
void
ModPackageMgrClass::Set_Current_Package (const char *package_filename)
{
	//
	//	Initialize the current package
	//
	CurrentPackage.Set_Package_Filename (package_filename);

	//
	//	Write the name of hte package to the registry
	//
	RegistryClass registry (APPLICATION_SUB_KEY_NAME_OPTIONS);
	if (registry.Is_Valid ()) {
		registry.Set_String (CURR_MOD_REG_VALUE, package_filename);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Current_Package
//
//////////////////////////////////////////////////////////////////////
void
ModPackageMgrClass::Set_Current_Package (int index)
{
	Set_Current_Package (PackageList[index].Get_Package_Filename ());
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Find_Package
//
//////////////////////////////////////////////////////////////////////
ModPackageClass *
ModPackageMgrClass::Find_Package (const char *name)
{
	ModPackageClass *retval = NULL;

	//
	//	Loop over all the packages in the list until we've found the
	// one we want
	//
	for (int index = 0; index < PackageList.Count (); index ++) {
		if (PackageList[index].Get_Package_Filename ().Compare_No_Case (name) == 0) {
			retval = &PackageList[index];
		}
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Mod_Map_Name_From_CRC
//
//////////////////////////////////////////////////////////////////////
bool
ModPackageMgrClass::Get_Mod_Map_Name_From_CRC_Index
(
	uint32			mod_file_crc,
	int				map_index,
	StringClass *	mod_name,
	StringClass *	map_name
)
{
	bool retval = false;

	(*mod_name) = "";
	(*map_name) = "";

	//
	//	Try to find the mod from its CRC
	//
	if (Find_Package_From_CRC (mod_file_crc, mod_name)) {

		//
		//	Get a pointer to this mod package
		//
		ModPackageClass *package = ModPackageMgrClass::Find_Package (mod_name->Peek_Buffer ());
		if (package != NULL) {

			//
			//	Get the list of maps in this mod
			//
			DynamicVectorClass<StringClass> list;
			package->Build_Level_List (list);

			//
			//	Is this map index in the valid range for the list?
			//
			if (map_index >= 0 && map_index < list.Count ()) {
				(*map_name) = list[map_index];
				retval = true;
			}			
		}
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Mod_Map_Name_From_CRC
//
//////////////////////////////////////////////////////////////////////
bool
ModPackageMgrClass::Get_Mod_Map_Name_From_CRC
(
	uint32			mod_name_crc,
	uint32			map_name_crc,
	StringClass *	mod_name,
	StringClass *	map_name
)
{
	bool retval = false;

	//
	//	Is there a mod name we need to lookup, or is this just the mapname?
	//
	if (mod_name_crc == 0) {

		//
		//	Try to find the map from its CRC
		//
		retval = Find_Filename_From_CRC ("*.mix", map_name_crc, map_name);

	} else {
		(*mod_name) = "";
		(*map_name) = "";

		//
		//	Try to find the mod from its CRC
		//
		if (Find_Filename_From_CRC ("*.pkg", mod_name_crc, mod_name)) {

			//
			//	Get a pointer to this mod package
			//
			ModPackageClass *package = ModPackageMgrClass::Find_Package (mod_name->Peek_Buffer ());
			if (package != NULL) {
				
				//
				//	Try to find the map from its CRC inside the mod package
				//
				retval = package->Find_Map_From_CRC (map_name_crc, map_name);
			}
		}
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Find_Filename_From_CRC
//
//////////////////////////////////////////////////////////////////////
bool
ModPackageMgrClass::Find_Filename_From_CRC
(
	const char *	search_mask,
	uint32			filename_crc,
	StringClass *	filename
)
{
	if (search_mask == NULL || filename == NULL) {
		return false;
	}

	bool retval = false;


	WIN32_FIND_DATAA find_info	= { 0 };
	BOOL keep_going				= true;
	HANDLE file_find				= NULL;

	(*filename) = "";

	StringClass full_search_mask	= "data/";
	full_search_mask					+= search_mask;

	//
	//	Build a list of all the saved games we know about
	//
	for (file_find = ::FindFirstFileA (full_search_mask, &find_info);
		 (file_find != INVALID_HANDLE_VALUE) && keep_going;
		  keep_going = ::FindNextFileA (file_find, &find_info))
	{
		//
		//	Is this the map we were looking for?
		//
		if (::CRC_Stringi (find_info.cFileName) == filename_crc) {

			//
			//	Return the file name to the caller
			//
			(*filename) = find_info.cFileName;
			retval = true;
			break;
		}
	}

	if (file_find != INVALID_HANDLE_VALUE) {
		::FindClose (file_find);
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Find_Package_From_CRC
//
//////////////////////////////////////////////////////////////////////
bool
ModPackageMgrClass::Find_Package_From_CRC
(
	uint32			file_crc,
	StringClass *	filename
)
{
	if (filename == NULL) {
		return false;
	}

	bool retval = false;

	//
	//	Loop over all the packages
	//
	for (int index = 0; index < PackageList.Count (); index ++) {
		
		//
		//	Does this package's CRC match the one we're looking for?
		//
		if (PackageList[index].Get_CRC () == file_crc) {
			(*filename) = PackageList[index].Get_Package_Filename ();
			retval = true;
			break;
		}
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Load_Current_Mod
//
//////////////////////////////////////////////////////////////////////
void
ModPackageMgrClass::Load_Current_Mod (void)
{
	//
	//	Return if there isn't a current mod selected
	//
	if (IS_SOLOPLAY || CurrentPackage.Get_Package_Filename ().Is_Empty ()) {
		return ;
	}

	FileFactoryListClass::Get_Instance ()->Remove_Temp_FileFactory ();
	FileFactoryListClass::Get_Instance ()->Add_Temp_FileFactory (new MixFileFactoryClass (CurrentPackage.Get_Package_Filename (), _TheFileFactory));
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Unload_Current_Mod
//
//////////////////////////////////////////////////////////////////////
void
ModPackageMgrClass::Unload_Current_Mod (void)
{
	//
	//	Return if there isn't a current mod selected
	//
	if (	CurrentPackage.Get_Package_Filename ().Is_Empty () ||
			FileFactoryListClass::Get_Instance () == NULL)
	{
		return ;
	}

	//
	//	Delete the temp file factory
	//
	FileFactoryClass *factory = FileFactoryListClass::Get_Instance ()->Remove_Temp_FileFactory ();
	if (factory != NULL) {
		delete factory;
		factory = NULL;
	}

	return ;
}
