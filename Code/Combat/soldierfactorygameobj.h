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
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Combat/soldierfactorygameobj.h               $* 
 *                                                                                             * 
 *                      $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 8/22/01 4:55p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 4                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef	__SOLDIERFACTORYGAMEOBJ_H
#define	__SOLDIERFACTORYGAMEOBJ_H

#include "always.h"
#include "building.h"


////////////////////////////////////////////////////////////////
//	Forward delcarations
////////////////////////////////////////////////////////////////
class BaseControllerClass;


////////////////////////////////////////////////////////////////
//
//	SoldierFactoryGameObjDef
//
////////////////////////////////////////////////////////////////
class SoldierFactoryGameObjDef : public BuildingGameObjDef
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	SoldierFactoryGameObjDef (void);
	~SoldierFactoryGameObjDef (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	uint32								Get_Class_ID (void) const override;
	PersistClass *						Create (void) const override;
	bool									Save (ChunkSaveClass &csave) override;
	bool									Load (ChunkLoadClass &cload) override;
	const PersistFactoryClass &	Get_Factory (void) const override;

	////////////////////////////////////////////////////////////////
	//	Editable support
	////////////////////////////////////////////////////////////////
	DECLARE_EDITABLE (SoldierFactoryGameObjDef, BuildingGameObjDef);

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void					Load_Variables (ChunkLoadClass &cload);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	float			UnloadTime;
	float			FundsGathered;	
	int			HarvesterDefID;

	////////////////////////////////////////////////////////////////
	//	Friends
	////////////////////////////////////////////////////////////////
	friend class SoldierFactoryGameObj;
};


////////////////////////////////////////////////////////////////
//
//	SoldierFactoryGameObj
//
////////////////////////////////////////////////////////////////
class SoldierFactoryGameObj : public BuildingGameObj
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	SoldierFactoryGameObj (void);
	~SoldierFactoryGameObj (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	// Definition support
	//
	virtual	void						Init( void ) override;
	void									Init (const SoldierFactoryGameObjDef & definition);
	const SoldierFactoryGameObjDef &	Get_Definition (void) const;

	//
	// RTTI
	//
	SoldierFactoryGameObj *			As_SoldierFactoryGameObj (void) override	{ return this; }

	//
	// Persist support
	//
	bool									Save (ChunkSaveClass &csave) override;
	bool									Load (ChunkLoadClass &cload) override;
	const	PersistFactoryClass &	Get_Factory (void) const override;

	//
	//	From BuildingGameObj
	//
	void									CnC_Initialize (BaseControllerClass *base) override;
	void									On_Destroyed (void) override;
	

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	void					Load_Variables (ChunkLoadClass &cload);
};


#endif	//__SOLDIERFACTORYGAMEOBJ_H
