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
 *                     $Archive:: /Commando/Code/commando/modpackage.h                        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/14/02 4:00p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __MODPACKAGE_H
#define __MODPACKAGE_H


#include "wwstring.h"
#include "bittype.h"


//////////////////////////////////////////////////////////////////////
//
//	ModPackageClass
//
//////////////////////////////////////////////////////////////////////
class ModPackageClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	ModPackageClass  (void);
	virtual ~ModPackageClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public operators
	///////////////////////////////////////////////////////////////////
	bool operator== (const ModPackageClass &/* src*/)	{ return false; }
	bool operator!= (const ModPackageClass &/* src*/)	{ return true; }

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Simple data accessors
	//
	const StringClass &	Get_Name (void) const			{ return Name; }
	void						Set_Name (const char *name)	{ Name = name; }

	const StringClass &	Get_Package_Filename (void) const			{ return PackageFilename; }
	void						Set_Package_Filename (const char *name);

	//
	//	CRC Access
	//
	uint32					Get_CRC (void);
	void						Compute_CRC (void);

	//
	//	Informational
	//
	void						Build_Level_List (DynamicVectorClass<StringClass> &list) const;
	bool						Find_Map_From_CRC (uint32 crc, StringClass *map_name) const;
	int						Get_Map_Index (const char *map_name);

protected:
	
	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////
	StringClass			Name;	
	StringClass			PackageFilename;
	uint32				FileCRC;
};


#endif //__MODPACKAGE_H
