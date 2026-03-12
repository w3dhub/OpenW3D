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
 *                     $Archive:: /Commando/Code/Commando/modpackagemgr.h                     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/14/02 4:33p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __MODPACKAGEMGR_H
#define __MODPACKAGEMGR_H


#include "modpackage.h"


//////////////////////////////////////////////////////////////////////
//
//	ModPackageMgrClass
//
//////////////////////////////////////////////////////////////////////
class ModPackageMgrClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Initialization
	//
	static void		Initialize (void);
	static void		Shutdown (void);

	//
	//	List support
	//
	static void		Build_List (void);
	static void		Reset_List (void);

	//
	//	Package enumeration
	//
	static int								Get_Package_Count (void)	{ return PackageList.Count (); }
	static ModPackageClass *	Get_Package (int index)		{ return &PackageList[index]; }

	//
	//	Package lookup
	//
	static ModPackageClass *	Find_Package (const char *name);

	//
	//	Current package support
	//
	static ModPackageClass &	Get_Current_Package (void)	{ return CurrentPackage; }
	static void								Set_Current_Package (const char *package_filename);
	static void								Set_Current_Package (int index);

	//
	//	Current MOD loading support
	//
	static void								Load_Current_Mod (void);
	static void								Unload_Current_Mod (void);

	//
	//	Mod/map package name
	//
	static bool								Get_Mod_Map_Name_From_CRC (uint32 mod_crc, uint32 map_crc, StringClass *mod_name, StringClass *map_name);
	static bool								Get_Mod_Map_Name_From_CRC_Index (uint32 mod_file_crc, int map_index, StringClass *mod_name, StringClass *map_name);
	static bool								Find_Filename_From_CRC (const char *search_mask, uint32 filename_crc, StringClass *filename);
	static bool								Find_Package_From_CRC (uint32 file_crc, StringClass *filename);

protected:

	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////
	static DynamicVectorClass<ModPackageClass>	PackageList;
	static ModPackageClass								CurrentPackage;
};


#endif //__MODPACKAGEMGR_H
