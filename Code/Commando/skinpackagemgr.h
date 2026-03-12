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
 *                     $Archive:: /Commando/Code/commando/skinpackagemgr.h                    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/05/02 12:00p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __SKINPACKAGEMGR_H
#define __SKINPACKAGEMGR_H


#include "vector.h"
#include "skinpackage.h"


//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//
//	SkinPackageMgrClass
//
//////////////////////////////////////////////////////////////////////
class SkinPackageMgrClass
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
	static const SkinPackageClass *	Get_Package (int index)		{ return &PackageList[index]; }

	//
	//	Current package support
	//
	static const SkinPackageClass &	Get_Current_Package (void)	{ return CurrentPackage; }
	static void								Set_Current_Package (const char *package_filename);
	static void								Set_Current_Package (int index);

protected:

	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////
	static DynamicVectorClass<SkinPackageClass>	PackageList;
	static SkinPackageClass								CurrentPackage;
};


#endif //__SKINPACKAGEMGR_H
