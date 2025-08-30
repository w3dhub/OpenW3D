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
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/specialeffectsgameobj.h      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 7/16/01 11:20a                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __SPECIALEFFECTSGAMEOBJ_H
#define __SPECIALEFFECTSGAMEOBJ_H

#include "physicalgameobj.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class ChunkSaveClass;
class ChunkLoadClass;


////////////////////////////////////////////////////////////////
//
//	SpecialEffectsGameObjDef
//
////////////////////////////////////////////////////////////////
class SpecialEffectsGameObjDef : public PhysicalGameObjDef
{
public:
	
	DECLARE_EDITABLE (SpecialEffectsGameObjDef, PhysicalGameObjDef);
	
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	SpecialEffectsGameObjDef (void);
	virtual ~SpecialEffectsGameObjDef (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	uint32								Get_Class_ID (void) const override;
	PersistClass *						Create (void) const override;
	bool									Save (ChunkSaveClass &csave) override;
	bool									Load (ChunkLoadClass &cload) override;
	const PersistFactoryClass &	Get_Factory (void) const override;

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void									Save_Variables (ChunkSaveClass &csave);
	void									Load_Variables (ChunkLoadClass &cload);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	StringClass							AnimationName;
	int									SoundDefID;

	////////////////////////////////////////////////////////////////
	//	Friends
	////////////////////////////////////////////////////////////////
	friend class SpecialEffectsGameObj;
};


////////////////////////////////////////////////////////////////
//
//	SpecialEffectsGameObj
//
////////////////////////////////////////////////////////////////
class SpecialEffectsGameObj : public PhysicalGameObj
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	SpecialEffectsGameObj (void);
	virtual ~SpecialEffectsGameObj (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	// Definition support
	//
	virtual	void							Init( void ) override;
	void										Init (const SpecialEffectsGameObjDef &definition);
	const SpecialEffectsGameObjDef &	Get_Definition (void) const;

	//
	// From PersistClass
	//
	bool										Save (ChunkSaveClass &csave) override;
	bool										Load (ChunkLoadClass &cload) override;
	const	PersistFactoryClass &		Get_Factory (void) const override;

	//
	//	Thinking
	//
	void										Think (void) override;

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void										Do_Effect (void);
	void										Save_Variables (ChunkSaveClass &csave);
	void										Load_Variables (ChunkLoadClass &cload);


	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	float				LifeRemaining;
	bool				IsInitialized;
};


#endif //__SPECIALEFFECTSGAMEOBJ_H

