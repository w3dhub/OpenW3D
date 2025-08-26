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
 *                     $Archive:: /Commando/Code/Combat/sakurabossgameobj.h       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/26/01 10:36a                                             $*
 *                                                                                             *
 *                    $Revision:: 18                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __SAKURABOSS_GAMEOBJ_H
#define __SAKURABOSS_GAMEOBJ_H

#include "vehicle.h"
#include "damage.h"
#include "pilot.h"
#include "Path.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////
//
//	SakuraBossGameObjDef
//
////////////////////////////////////////////////////////////////
class SakuraBossGameObjDef : public VehicleGameObjDef
{
public:

	DECLARE_EDITABLE (SakuraBossGameObjDef, VehicleGameObjDef);

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	SakuraBossGameObjDef (void);
	~SakuraBossGameObjDef (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	uint32								Get_Class_ID (void) const;
	PersistClass *						Create (void) const;
	bool									Save (ChunkSaveClass &csave);
	bool									Load (ChunkLoadClass &cload);
	const PersistFactoryClass &	Get_Factory (void) const;	

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void									Save_Variables (ChunkSaveClass &csave);
	void									Load_Variables (ChunkLoadClass &cload);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	DefenseObjectDefClass			RocketsDefense;
	int									GattlingGunDefID;
	int									RocketLauncherDefID;
	int									GattlingGunRevSoundDefID;
	int									RocketDestroyedExplosionID;
	int									RocketDoorOpenSoundID;

	////////////////////////////////////////////////////////////////
	//	Friends
	////////////////////////////////////////////////////////////////
	friend class						SakuraBossGameObj;
};


////////////////////////////////////////////////////////////////
//
//	SakuraBossGameObj
//
////////////////////////////////////////////////////////////////
class SakuraBossGameObj : public VehicleGameObj
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	SakuraBossGameObj (void);
	~SakuraBossGameObj (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	// Definitions
	//
	virtual	void						Init( void );
	void									Init (const SakuraBossGameObjDef &definition);
	const SakuraBossGameObjDef &	Get_Definition (void) const;

	//
	// From save/load
	//
	bool									Save (ChunkSaveClass & csave);
	bool									Load (ChunkLoadClass & cload);
	void									On_Post_Load (void);
	const	PersistFactoryClass &	Get_Factory (void) const;

	//
	// Think
	//
	void									Think (void);
	void									Apply_Control (void);

	//
	//	Muzzle override
	//
	const Matrix3D &					Get_Muzzle (int index = 0);

	//
	// Damage
	//
   virtual	void						Apply_Damage_Extended( const OffenseObjectClass & offense, 
														float scale = 1.0f, 
														const	Vector3 & direction = Vector3( 0,0,0 ), 
														const char * collision_box_name = NULL );
	
	//
	//	Occupant control
	//
	bool									Is_Entry_Permitted (SoldierGameObj * /*p_soldier*/)	{ return false; }	

	//
	// Network support
	//
   //void									Import_Frequent (BitStreamClass &packet);
   //void									Export_Frequent (BitStreamClass &packet);

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void									Save_Variables (ChunkSaveClass &csave);
	void									Load_Variables (ChunkLoadClass &cload);

	void									Blow_Off_Weapon (int weapon_id);
	void									Open_Rocket_Launchers (bool onoff);
	bool									Are_Rocker_Launchers_Ready (void);

	//
	//	Rocket launcher state management
	//
	void									Set_Rocket_State (int new_state);
	void									Update_Rocket_State (void);

	//
	//	Gatling gun state management
	//
	void									Set_Gattling_Gun_State (int new_state);
	void									Update_Gattling_Gun_State (void);	

	//
	//	Sakura state management
	//
	void									Set_Sakura_State (int new_state);
	void									Update_Sakura_State (void);

	//
	//	Vehicle state management
	//
	void									Set_Vehicle_State (int new_state);	
	void									Update_Vehicle_State (void);	

	//
	//	Overall state management
	//
	void									Set_Overall_State (int new_state);
	void									Decide_New_Overall_State (void);
	bool									Should_Change_Overall_State (void);
	void									Update_Overall_State (void);
	void									Request_Overall_State (int new_state);
	void									Update_Decision_Data (void);

	//
	//	Attack control
	//
	void									Attack_Target (float rocket_priority, float gattling_priority);
	void									Stop_Attacking (void);

	//
	//	Target management
	//
	void									Update_Target (void);
	SoldierGameObj *					Find_Closest_Human_Player (void);

	//
	//	Movement helper functions
	//
	void									Move_To_Location (const Vector3 &pos, float speed = 0.75F);
	void									Do_Waypath (int waypath_id, int start_id = -1, int end_id = -1);

	//
	//	Taunt control
	//
	void									Shuffle_Taunt_List (void);

	//
	//	Tilt control
	//
	void									Update_Tilt (void);

private:

	////////////////////////////////////////////////////////////////
	//	Private constants
	////////////////////////////////////////////////////////////////
	enum
	{
		MAX_TAUNTS	= 6
	};


	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////

	int							AvailableWeapons;
	
	int							OverallState;
	int							NextOverallState;
	bool							FaceTargetInTransition;
	
	int							RocketLauncherState;
	int							GattlingGunState;
	int							SakuraState;
	int							VehicleState;
	DefenseObjectClass		LeftRocketDefenseObject;
	DefenseObjectClass		RightRocketDefenseObject;	
	
	GameObjReference			LastDamager;
	GameObjReference			CurrentTarget;
	Vector3						MoveToLocation;

	float							GattlingGunStateTimeLeft;
	float							SakuraTauntTimeLeft;
	float							RocketLauncherStateTimeLeft;
	float							OverallStateTimeLeft;
	float							TargetTimeLeft;

	//
	//	Decision information
	//
	float							CurrentHealth;
	float							TargetAngle;
	Vector3						TargetPos;
	bool							IsAttacking;

	WeaponClass *				GattlingGun;
	WeaponClass *				RockerLauncherLeft;
	WeaponClass *				RockerLauncherRight;

	Matrix3D						CurrentMuzzleTM;
	int							CurrentMuzzleIndex;

	PilotClass					Pilot;
	PathClass *					Path;

	int							TauntList[MAX_TAUNTS];
	int							AvailableTaunts;

	float							TiltAngle;
	int							ChopperTiltBoneIndex;
};

#endif //__SAKURABOSS_GAMEOBJ_H
