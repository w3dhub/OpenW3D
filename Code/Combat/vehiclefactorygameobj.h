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
 *                     $Archive:: /Commando/Code/Combat/vehiclefactorygameobj.h                                                                                                                                               $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 12/10/01 4:41p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 12                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef	__VEHICLEFACTORYGAMEOBJ_H
#define	__VEHICLEFACTORYGAMEOBJ_H

#include "always.h"
#include "building.h"


////////////////////////////////////////////////////////////////
//	Forward delcarations
////////////////////////////////////////////////////////////////
class BaseControllerClass;
class VehicleGameObj;
class SoldierGameObj;


////////////////////////////////////////////////////////////////
//
//	VehicleFactoryGameObjDef
//
////////////////////////////////////////////////////////////////
class VehicleFactoryGameObjDef : public BuildingGameObjDef
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Friends
	////////////////////////////////////////////////////////////////
	friend class VehicleFactoryGameObj;

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	VehicleFactoryGameObjDef (void);
	~VehicleFactoryGameObjDef (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	uint32								Get_Class_ID (void) const override;
	PersistClass *						Create (void) const override;
	bool									Save (ChunkSaveClass &csave) override;
	bool									Load (ChunkLoadClass &cload) override;
	const PersistFactoryClass &	Get_Factory (void) const override;

	int									Get_Pad_Clearing_Warhead(void) const { return PadClearingWarhead; }
	float									Get_Total_Building_Time(void) const { return TotalBuildingTime; }

	////////////////////////////////////////////////////////////////
	//	Editable support
	////////////////////////////////////////////////////////////////
	DECLARE_EDITABLE (VehicleFactoryGameObjDef, BuildingGameObjDef);

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void					Load_Variables (ChunkLoadClass &cload);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	int					PadClearingWarhead;  // warhead used to destroy objects blocking construction area
	float					TotalBuildingTime;	// total time for slowest vehicle to be constructed and driven out.
};


////////////////////////////////////////////////////////////////
//
//	VehicleFactoryGameObj
//
////////////////////////////////////////////////////////////////
class VehicleFactoryGameObj : public BuildingGameObj
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	VehicleFactoryGameObj (void);
	~VehicleFactoryGameObj (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	// Definition support
	//
	virtual	void							Init( void ) override;
	void										Init (const VehicleFactoryGameObjDef & definition);
	const VehicleFactoryGameObjDef &	Get_Definition (void) const;

	//
	// RTTI
	//
	VehicleFactoryGameObj *			As_VehicleFactoryGameObj (void)	override { return this; }

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
	void					On_Destroyed (void) override;
	
	//
	//	GameObj methods
	//
	void					Think (void) override;

	//
	//	Factory state access
	//
	bool					Is_Available (void) const	{ return (IsBusy == false) && (IsDestroyed == false); }
	bool					Is_Available_For_Purchase (void) const;
	bool					Is_Busy (void) const			{ return IsBusy; }
	int					Get_Team_Vehicle_Count(void) const; 

	//
	//	Factory access
	//
	bool					Request_Vehicle (int defintion_id, float generation_time,SoldierGameObj * player = NULL);

	//
	//	Creation transform access
	//
	const Matrix3D &	Get_Creation_TM (void)						{ return CreationTM; }
	void					Set_Creation_TM (const Matrix3D &tm)	{ CreationTM = tm; }

	//
	//	Network object support
	//
	void					Import_Rare (BitStreamClass &packet) override;
	void					Export_Rare (BitStreamClass &packet) override;

	//
	// Vehicle limits
	// 
	static void			Set_Max_Vehicles_Per_Team(int max)		{ WWASSERT(max > 0); MaxVehiclesPerTeam = max; }
	static int			Get_Max_Vehicles_Per_Team(void)			{ return MaxVehiclesPerTeam; }

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void					Load_Variables (ChunkLoadClass &cload);
	virtual void		Destroy_Blocking_Objects (void);
	void					Deliver_Vehicle(void);
	VehicleGameObj *	Create_Vehicle (void);
	virtual void		Begin_Generation (void)	{};
	virtual void		On_Generation_Complete (void);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	GameObjReference	Vehicle;
	Matrix3D				CreationTM;
	OBBoxClass			GeneratingRegion;
	float					GenerationTime;
	int					GeneratingVehicleID;
	bool					IsBusy;
	GameObjReference	Purchaser;
	int					LastDeliveryPath;
	float					EndTimer;

	static int			MaxVehiclesPerTeam;
};


#endif	// __VEHICLEFACTORYGAMEOBJ_H

