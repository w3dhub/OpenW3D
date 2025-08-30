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
 *                     $Archive:: /Commando/Code/Combat/repairbaygameobj.h                    $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 7/16/01 11:20a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 6                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef	__REPAIRBAYGAMEOBJ_H
#define	__REPAIRBAYGAMEOBJ_H

#include "always.h"
#include "building.h"


////////////////////////////////////////////////////////////////
//	Forward delcarations
////////////////////////////////////////////////////////////////
class BaseControllerClass;
class SimpleGameObj;


////////////////////////////////////////////////////////////////
//
//	RepairBayGameObjDef
//
////////////////////////////////////////////////////////////////
class RepairBayGameObjDef : public BuildingGameObjDef
{
public:

	////////////////////////////////////////////////////////////////
	//	Friends
	////////////////////////////////////////////////////////////////
	friend class RepairBayGameObj;

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	RepairBayGameObjDef (void);
	~RepairBayGameObjDef (void);

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
	DECLARE_EDITABLE (RepairBayGameObjDef, BuildingGameObjDef);

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void					Load_Variables (ChunkLoadClass &cload);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	float					RepairPerSec;
	int					RepairingStaticAnimDefID;
};


////////////////////////////////////////////////////////////////
//
//	RepairBayGameObj
//
////////////////////////////////////////////////////////////////
class RepairBayGameObj : public BuildingGameObj
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	RepairBayGameObj (void);
	~RepairBayGameObj (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	// Definition support
	//
	virtual	void						Init( void ) override;
	void									Init (const RepairBayGameObjDef & definition);
	const RepairBayGameObjDef &	Get_Definition (void) const;

	//
	// RTTI
	//
	RepairBayGameObj *				As_RepairBayGameObj (void) override	{ return this; }

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

	//
	// Network support
	//
	void								Export_Creation (BitStreamClass &packet) override;
	void								Import_Creation (BitStreamClass &packet) override;

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	void					Load_Variables (ChunkLoadClass &cload);
	bool					Repair_Vehicle (void);
	void					Play_Repairing_Animation (bool onoff);
	void					Update_Repairing_Animations (void);
	void					Emit_Welding_Arc (RenderObjClass *vehicle_model);

	////////////////////////////////////////////////////////////////
	//	Private constants
	////////////////////////////////////////////////////////////////
	enum
	{
		ARC_OBJ_COUNT	= 4,
		BONE_COUNT		= 6
	};

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////	
	float					RepairTimer;
	OBBoxClass			RepairZone;
	int					RepairAnimationID;
	bool					IsReparing;
	
	DynamicVectorClass<GameObjReference>	VehicleList;
	StaticPhysClass *								RepairMesh;

	SimpleGameObj *		ArcObjects[ARC_OBJ_COUNT];
	float						ArcLifeRemaining[ARC_OBJ_COUNT];
	Matrix3D					EndTM;
	Matrix3D					Bones[BONE_COUNT];
	static const char *	BoneNames[BONE_COUNT];
};


#endif	// __REPAIRBAYGAMEOBJ_H
