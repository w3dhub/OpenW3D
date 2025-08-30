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
 *                     $Archive:: /Commando/Code/Combat/airstripgameobj.h                                                                                                                                        $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 10/01/01 3:44p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 6                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef	__AIRSTRIPGAMEOBJ_H
#define	__AIRSTRIPGAMEOBJ_H

#include "always.h"
#include "vehiclefactorygameobj.h"


////////////////////////////////////////////////////////////////
//	Forward delcarations
////////////////////////////////////////////////////////////////
class BaseControllerClass;


////////////////////////////////////////////////////////////////
//
//	AirStripGameObjDef
//
////////////////////////////////////////////////////////////////
class AirStripGameObjDef : public VehicleFactoryGameObjDef
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Friends
	////////////////////////////////////////////////////////////////
	friend class AirStripGameObj;

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	AirStripGameObjDef (void);
	~AirStripGameObjDef (void);

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
	DECLARE_EDITABLE (AirStripGameObjDef, VehicleFactoryGameObjDef);

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void					Load_Variables (ChunkLoadClass &cload);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	int					CinematicDefID;
	int					CinematicSlotIndex;
	float					CinematicLengthToDropOff;
	float					CinematicLengthToVehicleDisplay;
};


////////////////////////////////////////////////////////////////
//
//	AirStripGameObj
//
////////////////////////////////////////////////////////////////
class AirStripGameObj : public VehicleFactoryGameObj
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	AirStripGameObj (void);
	~AirStripGameObj (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	// Definition support
	//
	virtual	void					Init( void ) override;
	void								Init (const AirStripGameObjDef & definition);
	const AirStripGameObjDef &	Get_Definition (void) const;

	//
	// RTTI
	//
	AirStripGameObj *				As_AirStripGameObj (void) override	{ return this; }

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
	void					Begin_Generation (void) override;
	void					Start_Cinematic (void);
	void					Attach_Vehicle (void);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	float					CinematicStartTimer;	
	float					EndTimer;
	float					DisplayVehicleTimer;
	float					ClearDropoffZoneTimer;
	bool					IsCinematicStarted;
	PhysicalGameObj *	CinematicObject;
};


#endif	// __AIRSTRIPGAMEOBJ_H

