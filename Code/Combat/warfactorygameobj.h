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
 *                     $Archive:: /Commando/Code/Combat/warfactorygameobj.h                                                                                                                                       $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 1/05/02 11:54a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 7                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef	__WARFACTORYGAMEOBJ_H
#define	__WARFACTORYGAMEOBJ_H

#include "always.h"
#include "vehiclefactorygameobj.h"


////////////////////////////////////////////////////////////////
//	Forward delcarations
////////////////////////////////////////////////////////////////
class BaseControllerClass;


////////////////////////////////////////////////////////////////
//
//	WarFactoryGameObjDef
//
////////////////////////////////////////////////////////////////
class WarFactoryGameObjDef : public VehicleFactoryGameObjDef
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Friends
	////////////////////////////////////////////////////////////////
	friend class WarFactoryGameObj;

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	WarFactoryGameObjDef (void);
	~WarFactoryGameObjDef (void);

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
	DECLARE_EDITABLE (WarFactoryGameObjDef, VehicleFactoryGameObjDef);

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void					Load_Variables (ChunkLoadClass &cload);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////

};


////////////////////////////////////////////////////////////////
//
//	WarFactoryGameObj
//
////////////////////////////////////////////////////////////////
class WarFactoryGameObj : public VehicleFactoryGameObj
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	WarFactoryGameObj (void);
	~WarFactoryGameObj (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	// Definition support
	//
	virtual	void						Init( void ) override;
	void									Init (const WarFactoryGameObjDef & definition);
	const WarFactoryGameObjDef &	Get_Definition (void) const;

	//
	// RTTI
	//
	WarFactoryGameObj *				As_WarFactoryGameObj (void) override	{ return this; }

	//
	// Persist support
	//
	bool									Save (ChunkSaveClass &csave) override;
	bool									Load (ChunkLoadClass &cload) override;
	const	PersistFactoryClass &	Get_Factory (void) const override;

	//
	//	From BuildingGameObj
	//
	void					CnC_Initialize (BaseControllerClass *base) override;
	
	//
	//	GameObj methods
	//
	void					Think (void) override;

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void					Load_Variables (ChunkLoadClass &cload);
	void					Play_Creation_Animation (bool onoff);
	void					Begin_Generation (void) override;


	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	int					CreationAnimationID;
	float					CreationFinishedTimer;
};


#endif	// __WARFACTORYGAMEOBJ_H

