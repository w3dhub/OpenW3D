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
 *                     $Archive:: /Commando/Code/Combat/comcentergameobj.h                    $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
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

#ifndef	__COMCENTERGAMEOBJ_H
#define	__COMCENTERGAMEOBJ_H

#include "always.h"
#include "building.h"


////////////////////////////////////////////////////////////////
//	Forward delcarations
////////////////////////////////////////////////////////////////
class BaseControllerClass;


////////////////////////////////////////////////////////////////
//
//	ComCenterGameObjDef
//
////////////////////////////////////////////////////////////////
class ComCenterGameObjDef : public BuildingGameObjDef
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	ComCenterGameObjDef (void);
	~ComCenterGameObjDef (void);

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
	DECLARE_EDITABLE (ComCenterGameObjDef, BuildingGameObjDef);

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
	friend class ComCenterGameObj;
};


////////////////////////////////////////////////////////////////
//
//	ComCenterGameObj
//
////////////////////////////////////////////////////////////////
class ComCenterGameObj : public BuildingGameObj
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	ComCenterGameObj (void);
	~ComCenterGameObj (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	// Definition support
	//
	virtual	void						Init( void ) override;
	void									Init (const ComCenterGameObjDef & definition);
	const ComCenterGameObjDef &	Get_Definition (void) const;

	//
	// RTTI
	//
	ComCenterGameObj *				As_ComCenterGameObj (void) override	{ return this; }

	//
	// Persist support
	//
	bool									Save (ChunkSaveClass &csave) override;
	bool									Load (ChunkLoadClass &cload) override;
	const	PersistFactoryClass &	Get_Factory (void) const override;

	//
	//	From BuildingGameObj
	//
	void									On_Destroyed (void) override;
	

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	void					Load_Variables (ChunkLoadClass &cload);
};


#endif	//__COMCENTERGAMEOBJ_H
