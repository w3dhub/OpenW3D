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
 *                     $Archive:: /Commando/Code/commando/skinpackage.h                       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/07/02 10:55a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __SKINPACKAGE_H
#define __SKINPACKAGE_H


#include "wwstring.h"
#include "vector.h"


//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////
class INIClass;


//////////////////////////////////////////////////////////////////////
//
//	SkinPackageClass
//
//////////////////////////////////////////////////////////////////////
class SkinPackageClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	SkinPackageClass  (void);
	~SkinPackageClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public operators
	///////////////////////////////////////////////////////////////////
	bool operator== (const SkinPackageClass &/* src*/)	{ return false; }
	bool operator!= (const SkinPackageClass &/* src*/)	{ return true; }

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Simple data accessors
	//
	const StringClass &	Get_Name (void) const			{ return Name; }
	void						Set_Name (const char *name)	{ Name = name; }

	const StringClass &	Get_Package_Filename (void) const			{ return PackageFilename; }
	void						Set_Package_Filename (const char *name)	{ PackageFilename = name; }

protected:
	
	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////
	StringClass			Name;	
	StringClass			PackageFilename;
};


#endif //__SKINPACKAGE_H
