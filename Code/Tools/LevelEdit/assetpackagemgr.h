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
 *                 Project Name : leveledit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/assetpackagemgr.h            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/28/02 3:09p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __ASSETPACKAGEMGR_H
#define __ASSETPACKAGEMGR_H

#include "wwstring.h"
#include "listtypes.h"


//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//
//	AssetPackageMgrClass
//
//////////////////////////////////////////////////////////////////////
class AssetPackageMgrClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Initialization
	//
	static void				Initialize (void);
	static void				Shutdown (void);

	//
	//	Current package support
	//
	static void				Set_Current_Package (const char *name);
	static const char *	Get_Current_Package (void)						{ return CurrentPackageName; }

	//
	//	Path support
	//
	static const char *	Get_Current_Package_Path (void)				{ return CurrentPackagePath; }

	//
	//	Package creation
	//
	static void				Create_Package (const char *name);

	//
	//	Package list support
	//
	static void				Build_Package_List (STRING_LIST &list);

	//
	//	UI support
	//
	static void				Show_Package_Selection_UI (void);

private:

	///////////////////////////////////////////////////////////////////
	//	Private methods
	///////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////
	static StringClass		CurrentPackageName;
	static StringClass		CurrentPackagePath;
};


#endif //__ASSETPACKAGEMGR_H
