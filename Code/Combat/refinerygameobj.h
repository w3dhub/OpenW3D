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
 *                     $Archive:: /Commando/Code/Combat/refinerygameobj.h                     $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 1/18/02 11:17p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 10                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef	__REFINERYGAMEOBJ_H
#define	__REFINERYGAMEOBJ_H

#include "always.h"
#include "building.h"


////////////////////////////////////////////////////////////////
//	Forward delcarations
////////////////////////////////////////////////////////////////
class HarvesterClass;
class BaseControllerClass;
class VehicleGameObj;


////////////////////////////////////////////////////////////////
//
//	RefineryGameObjDef
//
////////////////////////////////////////////////////////////////
class RefineryGameObjDef : public BuildingGameObjDef
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	RefineryGameObjDef (void);
	~RefineryGameObjDef (void);

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
	DECLARE_EDITABLE (RefineryGameObjDef, BuildingGameObjDef);

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
	float			FundsDistributedPerSec;
	int			HarvesterDefID;

	////////////////////////////////////////////////////////////////
	//	Friends
	////////////////////////////////////////////////////////////////
	friend class RefineryGameObj;
};


////////////////////////////////////////////////////////////////
//
//	RefineryGameObj
//
////////////////////////////////////////////////////////////////
class RefineryGameObj : public BuildingGameObj
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	RefineryGameObj (void);
	~RefineryGameObj (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	// Definition support
	//
	virtual	void					Init( void ) override;
	void								Init (const RefineryGameObjDef & definition);
	const RefineryGameObjDef &	Get_Definition (void) const;

	//
	// RTTI
	//
	RefineryGameObj *				As_RefineryGameObj (void) override	{ return this; }

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
	void					Manage_Money_Trickle_Sound (void);

	//
	//	State access
	//
	bool					Needs_Harvester (void) const			{ return (!IsDestroyed) && (Harvester == NULL); }
	int					Get_Harvester_Def_ID (void) const	{ return Get_Definition ().HarvesterDefID; }
	void					Set_Is_Harvester_Docked(bool flag);
	bool					Get_Is_Harvester_Docked(void)			{ return IsHarvesterDocked; }

	//
	//	Harvester access
	//
	VehicleGameObj *	Get_Harvester_Vehicle (void);
	void					Set_Harvester_Vehicle (VehicleGameObj *harvester);

	HarvesterClass *	Get_Harvester (void)								{ return Harvester; }
	void					Set_Harvester (HarvesterClass *harvester)	{ Harvester = harvester; }

	//
	//	Notifications
	//
	void					On_Harvester_Docked (void);
	void					On_Harvester_Damaged(VehicleGameObj*);
	void					On_Harvester_Destroyed(VehicleGameObj*);

	//
	//	Door access
	//
	void					Play_Unloading_Animation (bool onoff);

	//
	//	Dockin access
	//
	const Matrix3D	&	Get_Dock_TM (void) const			{ return DockTM; }
	void					Set_Dock_TM (const Matrix3D &tm)	{ DockTM = tm; }

	//
	// Network support
	//
   virtual	void		Export_Rare (BitStreamClass &packet) override;
   virtual	void		Import_Rare (BitStreamClass &packet) override;

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	void					Load_Variables (ChunkLoadClass &cload);	

	////////////////////////////////////////////////////////////////
	//	Private constants
	////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	HarvesterClass *	Harvester;
	OBBoxClass			TiberiumField;
	bool					IsHarvesterDocked;
	float					UnloadTimer;
	int					UnloadAnimationID;
	float					TotalFunds;
	float					FundsPerSecond;
	float					DistributionTimer;
	Matrix3D				DockTM;
	AudibleSoundClass * MoneyTrickleSound;

};


#endif	// __REFINERYGAMEOBJ_H
