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
 *                 Project Name : wwphys																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/accessiblephys.h        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/17/01 8:27p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __ACCESSIBLE_PHYS_H
#define __ACCESSIBLE_PHYS_H

#include "staticanimphys.h"


/////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////
class ChunkSaveClass;
class ChunkLoadClass;
class AccessiblePhysDefClass;


/////////////////////////////////////////////////////////////////////////
//
//	AccessiblePhysClass
//
/////////////////////////////////////////////////////////////////////////
class AccessiblePhysClass : public StaticAnimPhysClass
{
public:

	/////////////////////////////////////////////////////////////////////////
	// Public constructors/destructors
	/////////////////////////////////////////////////////////////////////////
	AccessiblePhysClass (void);
	~AccessiblePhysClass (void);
	
	/////////////////////////////////////////////////////////////////////////
	// Public methods
	/////////////////////////////////////////////////////////////////////////		

	//
	//	RTTI
	//
	AccessiblePhysClass *			As_AccessiblePhysClass (void) override	{ return this; }

	//
	//	Definition support
	//
	AccessiblePhysDefClass *		Get_AccessiblePhysDefClass (void)	{ return (AccessiblePhysDefClass *)Definition; }
	void									Init (const AccessiblePhysDefClass &definition);

	//
	// Save-Load Support
	//
	const PersistFactoryClass &	Get_Factory (void) const override;
	bool									Save (ChunkSaveClass &csave) override;
	bool									Load (ChunkLoadClass &cload) override;

	//
	//	Lock support
	//
	void									Set_Lock_Code (int code)	{ LockCode = code; }
	int									Get_Lock_Code (void) const	{ return LockCode; }

	bool									Can_Unlock (int key_mask)	{ return ((1 << LockCode) & key_mask) != 0; }

protected:

	/////////////////////////////////////////////////////////////////////////
	// Protected methods
	/////////////////////////////////////////////////////////////////////////
	bool									Save_Variables (ChunkSaveClass &csave);
	bool									Load_Variables (ChunkLoadClass &cload);

	/////////////////////////////////////////////////////////////////////////
	// Protected member data
	/////////////////////////////////////////////////////////////////////////
	int				LockCode;
};


/////////////////////////////////////////////////////////////////////////
//
//	AccessiblePhysDefClass
//
/////////////////////////////////////////////////////////////////////////
class AccessiblePhysDefClass : public StaticAnimPhysDefClass
{
public:
	
	/////////////////////////////////////////////////////////////////////////
	// Public constructors/destructors
	/////////////////////////////////////////////////////////////////////////
	AccessiblePhysDefClass (void);
	
	//
	// From DefinitionClass
	//
	uint32								Get_Class_ID (void) const override;
	PersistClass *						Create (void) const override;

	//
	// From PhysDefClass
	//
	const char *						Get_Type_Name (void) override			{ return "AccessiblePhysDefClass"; }
	bool									Is_Type (const char *) override;

	//
	// From PersistClass
	//
	const PersistFactoryClass &	Get_Factory (void) const override;
	bool									Save (ChunkSaveClass &csave) override;
	bool									Load (ChunkLoadClass &cload) override;

	//
	//	Editable interface requirements
	//
	DECLARE_EDITABLE (AccessiblePhysDefClass, StaticAnimPhysDefClass);

protected:

	/////////////////////////////////////////////////////////////////////////
	// Protected methods
	/////////////////////////////////////////////////////////////////////////
	bool									Save_Variables (ChunkSaveClass &csave);
	bool									Load_Variables (ChunkLoadClass &cload);
	
	/////////////////////////////////////////////////////////////////////////
	// Protected member data
	/////////////////////////////////////////////////////////////////////////		
	int				LockCode;
	
	/////////////////////////////////////////////////////////////////////////
	// Friends
	/////////////////////////////////////////////////////////////////////////
	friend class	AccessiblePhysClass;
};


#endif //__ACCESSIBLE_PHYS_H
