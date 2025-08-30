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
 *                 Project Name : wwtranslatedb																  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwtranslatedb/tdbcategory.h       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/22/00 10:53a                                             $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __TDB_CATEGORY_H
#define __TDB_CATEGORY_H

#include "persist.h"
#include "wwstring.h"
#include "vector.h"
#include "bittype.h"


//////////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////////
class ChunkSaveClass;
class ChunkLoadClass;


//////////////////////////////////////////////////////////////////////////
//
//	TDBCategoryClass
//
//////////////////////////////////////////////////////////////////////////
class TDBCategoryClass : public PersistClass
{
public:
	
	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	TDBCategoryClass (void);
	TDBCategoryClass (const TDBCategoryClass &src);
	virtual ~TDBCategoryClass (void);

	//////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////
	const TDBCategoryClass &operator= (const TDBCategoryClass &src);

	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////
	
	//
	// From PersistClass
	//
	const PersistFactoryClass &Get_Factory (void) const override;
	bool								Save (ChunkSaveClass &csave) override;
	bool								Load (ChunkLoadClass &cload) override;

	//
	// Copy methods
	//
	TDBCategoryClass *			Clone (void) const				{ return new TDBCategoryClass (*this); }

	//
	// TDBCategoryClass specific
	//
	
	const StringClass &			Get_Name (void) const			{ return Name; }
	void								Set_Name (const char *name)	{ Name = name; }

	uint32							Get_ID (void) const				{ return ID; }
	void								Set_ID (uint32 id)				{ ID = id; }
	
	
protected:

	//////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////
	void								Save_Variables (ChunkSaveClass &csave);
	void								Load_Variables (ChunkLoadClass &cload);

private:

	//////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////
	StringClass			Name;
	uint32				ID;
};


#endif //__TDB_CATEGORY_H
