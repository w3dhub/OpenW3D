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
 *                 Project Name : combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/raveshawbossgameobj.h                 $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/13/02 11:42a                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __RAVESHAWBOSSGAMEOBJ_H
#define __RAVESHAWBOSSGAMEOBJ_H

#include "soldier.h"
#include "statemachine.h"


//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////
class ManualTransitionEffectClass;
class StealthEffectClass;
class SimpleGameObj;


//////////////////////////////////////////////////////////////////////
//
//	RaveshawBossGameObjDefClass
//
//////////////////////////////////////////////////////////////////////
class RaveshawBossGameObjDefClass : public SoldierGameObjDef
{
public:

	DECLARE_EDITABLE (RaveshawBossGameObjDefClass, SoldierGameObjDef);

	//////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////
	RaveshawBossGameObjDefClass (void);
	~RaveshawBossGameObjDefClass (void);

	//////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////
	uint32								Get_Class_ID (void) const override;
	PersistClass *						Create (void) const override;
	bool									Save (ChunkSaveClass &csave) override;
	bool									Load (ChunkLoadClass &cload) override;
	const PersistFactoryClass &	Get_Factory (void) const override;

protected:

	//////////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////////
	void									Save_Variables (ChunkSaveClass &csave);
	void									Load_Variables (ChunkLoadClass &cload);

	//////////////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	//	Friends
	//////////////////////////////////////////////////////////////////////
	friend class						RaveshawBossGameObjClass;
};

//////////////////////////////////////////////////////////////////////
//
//	RaveshawBossGameObjClass
//
//////////////////////////////////////////////////////////////////////
class RaveshawBossGameObjClass : public SoldierGameObj
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	RaveshawBossGameObjClass  (void);
	~RaveshawBossGameObjClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////
	
	//
	// Definitions
	//
	virtual	void								Init (void) override;
	void											Init (const RaveshawBossGameObjDefClass &definition);
	const RaveshawBossGameObjDefClass &	Get_Definition (void) const;

	//
	// From save/load
	//
	bool									Save (ChunkSaveClass & csave) override;
	bool									Load (ChunkLoadClass & cload) override;
	void									On_Post_Load (void) override;
	const	PersistFactoryClass &	Get_Factory (void) const override;

	//
	//	Inherited
	//
	bool									Allow_Special_Damage_State_Lock (void) override	{ return false; }

	//
	// Think
	//
	void									Think (void) override;
	void									Apply_Control (void) override;

	//
	// Damage
	//
   virtual	void						Apply_Damage_Extended (const OffenseObjectClass &offense, float scale = 1.0f, const	Vector3 & direction = Vector3 (0, 0, 0), const char *collision_box_name = NULL) override;

protected:
	
	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////
	
	//
	//	Save/load support
	//
	void									Save_Variables (ChunkSaveClass &csave);
	void									Load_Variables (ChunkLoadClass &cload);

	//
	//	Misc
	//
	void									Attach_Sound (const char *sound_name, const char *bone_name);
	void									Find_Closest_Catwalk_Pos (const Vector3 &curr_pos, Vector3 *catwalk_pos);
	bool									Apply_Bone_Collision_Damage (float damage_scale, const char *bone_name);
	void									Determine_New_Overall_State (void);
	void									Find_Death_Facing_Pos (Vector3 *facing_pos);
	
	//
	//	Movement support
	//	
	float									Get_Distance_From_Ground (void);
	bool									Fly_Move (PhysicalGameObj *game_obj, const Vector3 &vector);
	void									Jump_To_Point (const Vector3 &pos);	

	//
	//	Lightning rod support
	//
	void									Add_Lightning_Arc (const Vector3 &start_point, const Vector3 &end_point);
	void									Collect_Lightning_Rods (void);
	void									Create_Arc_Effects (void);
	void									Prepare_Arc_Effect_Data (void);

	//
	//	Stealth and thrown object support
	//
	void									Create_Stealth_Soldier (const Matrix3D &tm);
	void									Link_Stealth_Soldier_To_Hand (void);
	void									Link_Thrown_Object_To_Hands (void);
	SimpleGameObj *					Find_Object_To_Throw (void);
	WWINLINE SoldierGameObj *		Peek_Stealth_Soldier (void);
	void									Verify_Stealth_Soldier (void);

	void									Link_Player_To_Hands (void);

	//
	//	Taunt control
	//
	void									Shuffle_Taunt_List (void);

	//
	//	State support
	//
	enum
	{
		OVERALL_STATE_NOTHING				= 0,
		OVERALL_STATE_HEALING,
		OVERALL_STATE_THROWING_OBJECT,
		OVERALL_STATE_THROWING_SOLDIER,
		OVERALL_STATE_THROWING_STAR,
		OVERALL_STATE_GRAB_STAR,
		OVERALL_STATE_JUMP_TO_CATWALK,
		OVERALL_STATE_ON_CATWALK,
		OVERALL_STATE_BODYSLAM,
		OVERALL_STATE_CHASE_STAR,
		OVERALL_STATE_DAZED,
		OVERALL_STATE_FLEE,
		OVERALL_STATE_DEATH_SEQUENCE
	};

	enum
	{
		RAVESHAW_STATE_NOTHING				= 0,
		RAVESHAW_STATE_ROAR,
		RAVESHAW_STATE_GRAB_TIBERIUM,
		RAVESHAW_STATE_GRAB_SOLDIER,
		RAVESHAW_STATE_THROW_SOLDIER,
		RAVESHAW_STATE_GRAB_OBJECT,
		RAVESHAW_STATE_THROW_OBJECT,
		RAVESHAW_STATE_GRAB_STAR,
		RAVESHAW_STATE_BODYSLAM,
		RAVESHAW_STATE_JUMP_DOWN,
		RAVESHAW_STATE_STUMBLE,
		RAVESHAW_STATE_LOOK_CONFUSED,
		RAVESHAW_STATE_DYING,
		RAVESHAW_STATE_FALL,
		RAVESHAW_STATE_DEATH_LANDING
	};

	enum
	{
		MOVE_STATE_STOP						= 0,
		MOVE_STATE_GOTO_TIBERIUM,
		MOVE_STATE_GOTO_CATWALK,
		MOVE_STATE_JUMP_TO_CATWALK,
		MOVE_STATE_CIRCLE_CATWALK,		
		MOVE_STATE_GOTO_THROW_OBJECT,
		MOVE_STATE_JUMP_TO_STAR,
		MOVE_STATE_FOLLOW_STAR,
	};

	enum
	{
		ENGORGED_STATE_NONE						= 0,
		ENGORGED_STATE_ABSORBING_TIBERIUM,
		ENGORGED_STATE_FADING
	};

	enum
	{
		JUMP_STATE_NONE						= 0,
		JUMP_STATE_CROUCHING,
		JUMP_STATE_JUMPING,
		JUMP_STATE_LANDING
	};

	enum
	{
		HAVOC_STATE_NONE						= 0,
		HAVOC_STATE_GRABBED,
		HAVOC_STATE_FLYING
	};

	enum
	{
		STEALTH_SOLDIER_STATE_NONE						= 0,
		STEALTH_SOLDIER_STATE_DISPLAY,
		STEALTH_SOLDIER_STATE_FLYING
	};

	enum
	{
		THROWN_OBJECT_STATE_NONE						= 0,
		THROWN_OBJECT_STATE_PICKUP,
		THROWN_OBJECT_STATE_FLYING
	};

	enum
	{
		LIGHTNING_ROD_STATE_NONE						= 0,
		LIGHTNING_ROD_STATE_ACTIVE,
	};

	//
	//	Overall state handlers
	//	
	DECLARE_STATE (OVERALL_STATE_NOTHING,				SM_NO_BEGIN,	SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (OVERALL_STATE_HEALING,				SM_BEGIN,		SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (OVERALL_STATE_THROWING_OBJECT,	SM_BEGIN,		SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (OVERALL_STATE_THROWING_SOLDIER,	SM_BEGIN,		SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (OVERALL_STATE_THROWING_STAR,		SM_NO_BEGIN,	SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (OVERALL_STATE_GRAB_STAR,			SM_BEGIN,		SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);	
	DECLARE_STATE (OVERALL_STATE_JUMP_TO_CATWALK,	SM_BEGIN,		SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (OVERALL_STATE_ON_CATWALK,			SM_BEGIN,		SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (OVERALL_STATE_BODYSLAM,				SM_BEGIN,		SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (OVERALL_STATE_CHASE_STAR,			SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (OVERALL_STATE_DAZED,					SM_NO_BEGIN,	SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (OVERALL_STATE_FLEE,					SM_NO_BEGIN,	SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (OVERALL_STATE_DEATH_SEQUENCE,		SM_BEGIN,		SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);

	//
	//	Raveshaw state handlers
	//	
	DECLARE_STATE (RAVESHAW_STATE_NOTHING,			SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (RAVESHAW_STATE_ROAR,				SM_BEGIN,		SM_END,		SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (RAVESHAW_STATE_GRAB_TIBERIUM,	SM_BEGIN,		SM_END,		SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (RAVESHAW_STATE_GRAB_SOLDIER,	SM_BEGIN,		SM_END,		SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (RAVESHAW_STATE_THROW_SOLDIER,	SM_BEGIN,		SM_END,		SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (RAVESHAW_STATE_GRAB_OBJECT,	SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (RAVESHAW_STATE_THROW_OBJECT,	SM_BEGIN,		SM_END,		SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (RAVESHAW_STATE_GRAB_STAR,		SM_BEGIN,		SM_END,		SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (RAVESHAW_STATE_BODYSLAM,		SM_BEGIN,		SM_END,		SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (RAVESHAW_STATE_JUMP_DOWN,		SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);	
	DECLARE_STATE (RAVESHAW_STATE_STUMBLE,			SM_NO_BEGIN,	SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (RAVESHAW_STATE_LOOK_CONFUSED,	SM_NO_BEGIN,	SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (RAVESHAW_STATE_DYING,			SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (RAVESHAW_STATE_FALL,				SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (RAVESHAW_STATE_DEATH_LANDING,	SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);	

	//
	//	Move state handlers
	//	
	DECLARE_STATE (MOVE_STATE_STOP,					SM_BEGIN,		SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (MOVE_STATE_GOTO_TIBERIUM,		SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (MOVE_STATE_GOTO_CATWALK,		SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (MOVE_STATE_JUMP_TO_CATWALK,	SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);	
	DECLARE_STATE (MOVE_STATE_CIRCLE_CATWALK,		SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (MOVE_STATE_GOTO_THROW_OBJECT,	SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);	
	DECLARE_STATE (MOVE_STATE_JUMP_TO_STAR,		SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (MOVE_STATE_FOLLOW_STAR,			SM_BEGIN,		SM_END,		SM_THINK,		SM_NO_REQ_END);

	//
	//	Engorged state handlers
	//	
	DECLARE_STATE (ENGORGED_STATE_NONE,						SM_NO_BEGIN,	SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (ENGORGED_STATE_ABSORBING_TIBERIUM,	SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (ENGORGED_STATE_FADING,					SM_BEGIN,		SM_END,		SM_THINK,		SM_NO_REQ_END);

	//
	//	Jump state handlers
	//	
	DECLARE_STATE (JUMP_STATE_NONE,			SM_NO_BEGIN,	SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (JUMP_STATE_CROUCHING,	SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (JUMP_STATE_JUMPING,		SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (JUMP_STATE_LANDING,		SM_BEGIN,		SM_END,		SM_THINK,		SM_NO_REQ_END);

	//
	//	Stealth soldier state handlers
	//	
	DECLARE_STATE (STEALTH_SOLDIER_STATE_NONE,			SM_NO_BEGIN,	SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (STEALTH_SOLDIER_STATE_DISPLAY,		SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (STEALTH_SOLDIER_STATE_FLYING,			SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);

	//
	//	Havoc state handlers
	//	
	DECLARE_STATE (HAVOC_STATE_NONE,			SM_NO_BEGIN,	SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (HAVOC_STATE_GRABBED,		SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (HAVOC_STATE_FLYING,		SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);

	//
	//	Thrown object state handlers
	//	
	DECLARE_STATE (THROWN_OBJECT_STATE_NONE,		SM_NO_BEGIN,	SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (THROWN_OBJECT_STATE_PICKUP,	SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (THROWN_OBJECT_STATE_FLYING,	SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);

	//
	//	Lightning rod state handlers
	//	
	DECLARE_STATE (LIGHTNING_ROD_STATE_NONE,		SM_NO_BEGIN,	SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (LIGHTNING_ROD_STATE_ACTIVE,	SM_BEGIN,		SM_END,		SM_THINK,		SM_NO_REQ_END);

private:

	///////////////////////////////////////////////////////////////////
	//	Private constants
	///////////////////////////////////////////////////////////////////
	enum
	{
		MAX_TAUNTS	= 6
	};
	
	///////////////////////////////////////////////////////////////////
	//	Private methods
	///////////////////////////////////////////////////////////////////
	static int __cdecl	fnSortLightningRodsCallback (const void *elem1, const void *elem2);
	
	///////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////
	
	//
	//	Taunts
	//
	float							RaveshawTauntTimeLeft;
	int							TauntList[MAX_TAUNTS];
	int							AvailableTaunts;

	//
	//	Timers
	//
	float							OverallStateTimer;
	float							EngorgedStateTimer;
	float							MoveStateTimer;
	float							BodySlamTimer;
	float							RaveshawStateTimer;
	float							StealthSoldierStateTimer;
	float							LightningRodStateTimer;
	float							StartTimer;
	
	//
	//	Melee data
	//
	float							LastMeleeAnimFrame;
	bool							HasMeleeAttackHit;

	//
	//	Thrown object data
	//	
	GameObjReference			StealthSoldier;
	StealthEffectClass *		StealthEffect;
	SimpleGameObj *			ThrownObject;
	Vector3						FlyingObjectVector;
	Vector3						FlyingObjectDest;
	float							FlyingDist;
	Matrix3D						RelObjTM;
	RenderObjClass *			CameraBoneModel;
	bool							RestoreFirstPerson;
	
	//
	//	Effects data
	//
	ManualTransitionEffectClass *							TiberiumEffect;
	DynamicVectorClass<DamageableStaticPhysClass *>	LightningRodList;
	bool															IsTiberiumEffectApplied;

	//
	//	Positions
	//
	Vector3						RaveshawPos;
	Vector3						StarPos;

	Vector3						CurrentDestPos;
	Vector3						CurrentJumpToPos;

	//
	//	State machines
	//
	StateMachineClass<RaveshawBossGameObjClass>	OverallState;
	StateMachineClass<RaveshawBossGameObjClass>	RaveshawState;
	StateMachineClass<RaveshawBossGameObjClass>	MoveState;
	StateMachineClass<RaveshawBossGameObjClass>	EngorgedState;
	StateMachineClass<RaveshawBossGameObjClass>	StealthSoldierState;
	StateMachineClass<RaveshawBossGameObjClass>	HavocState;
	StateMachineClass<RaveshawBossGameObjClass>	ThrownObjectState;
	StateMachineClass<RaveshawBossGameObjClass>	JumpState;
	StateMachineClass<RaveshawBossGameObjClass>	LightningRodState;

	enum
	{
		ARC_OBJ_COUNT	= 10,
		BONE_COUNT		= 6
	};

	SimpleGameObj *		ArcObjects[ARC_OBJ_COUNT];
	float						ArcLifeRemaining[ARC_OBJ_COUNT];
	Matrix3D					EndTM;
	Matrix3D					Bones[BONE_COUNT];
};


///////////////////////////////////////////////////////////////////
//	Peek_Stealth_Soldier
///////////////////////////////////////////////////////////////////
WWINLINE SoldierGameObj *
RaveshawBossGameObjClass::Peek_Stealth_Soldier (void)
{
	SoldierGameObj *soldier = NULL;
	
	//
	//	Dig the soldier out of the game object reference
	//
	if (StealthSoldier != NULL) {
		PhysicalGameObj *phys_game_obj = StealthSoldier.Get_Ptr ()->As_PhysicalGameObj ();
		if (phys_game_obj != NULL) {
			soldier = phys_game_obj->As_SoldierGameObj ();
		}
	}

	return soldier;
}


#endif //__RAVESHAWBOSSGAMEOBJ_H
