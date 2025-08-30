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
 *                 Project Name : combat                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/mendozabossgameobj.h                  $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/18/01 5:04p                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __MENDOZABOSSGAMEOBJ_H
#define __MENDOZABOSSGAMEOBJ_H

#include "soldier.h"
#include "statemachine.h"
#include "cardinalspline.h"


//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////
class PowerUpGameObj;


//////////////////////////////////////////////////////////////////////
//
//	MendozaBossGameObjDefClass
//
//////////////////////////////////////////////////////////////////////
class MendozaBossGameObjDefClass : public SoldierGameObjDef
{
public:

	DECLARE_EDITABLE (MendozaBossGameObjDefClass, SoldierGameObjDef);

	//////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////
	MendozaBossGameObjDefClass (void);
	~MendozaBossGameObjDefClass (void);

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
	friend class						MendozaBossGameObjClass;
};


//////////////////////////////////////////////////////////////////////
//
//	MendozaBossGameObjClass
//
//////////////////////////////////////////////////////////////////////
class MendozaBossGameObjClass : public SoldierGameObj
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	MendozaBossGameObjClass  (void);
	~MendozaBossGameObjClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////
	
	//
	// Definitions
	//
	virtual	void								Init (void) override;
	void											Init (const MendozaBossGameObjDefClass &definition);
	const MendozaBossGameObjDefClass &	Get_Definition (void) const;

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
	bool									Wants_Powerups (void) override						{ return true; }
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
	void									Save_Variables (ChunkSaveClass &csave);
	void									Load_Variables (ChunkLoadClass &cload);

	void									Attach_Fire_Sound (void);
	void									Attach_Sound (const char *sound_name, const char *bone_name);
	bool									Apply_Bone_Collision_Damage (const char *bone_name);
	bool									Fly_Move (const Vector3 &vector);
	void									Initialize_Boss (void);

	PowerUpGameObj *					Find_Best_Powerup (void);
	void									Spawn_Health_Powerups (void);

	//
	//	Taunt control
	//
	void									Shuffle_Taunt_List (void);

	//
	//	State support
	//
	enum
	{
		OVERALL_STATE_FLYING_SIDEKICK		= 0,
		OVERALL_STATE_MELEE_ATTACK,
		OVERALL_STATE_RANGED_ATTACK,
		OVERALL_STATE_FIREBALL_ATTACK,
		OVERALL_STATE_FIND_HEALTH,
		OVERALL_STATE_SYDNEY_BOLTS,
		OVERALL_STATE_RUN_AFTER_SYDNEY,
		OVERALL_STATE_TOY_WITH_SYDNEY,
		OVERALL_STATE_DEATH_SEQUENCE
	};

	enum
	{
		MOVE_STATE_STOP						= 0,
		MOVE_STATE_GET_CLOSE_TO_PLAYER,
		MOVE_STATE_ATTACK_PATTERN1,
		MOVE_STATE_ATTACK_PATTERN2,
		MOVE_STATE_GOTO_HELIPAD,
		MOVE_STATE_RUN_TO_HEALTH,
		MOVE_STATE_CHASE_SYDNEY,
	};

	enum
	{
		MENDOZA_STATE_STANDING				= 0,
		MENDOZA_STATE_FALLING,
		MENDOZA_STATE_KIPPING,
		MENDOZA_STATE_PACK_EXPLODING,
		MENDOZA_STATE_DYING,
		MENDOZA_STATE_DEAD
	};

	enum
	{
		SYDNEY_STATE_WAITING				= 0,
		SYDNEY_STATE_BOLTING,
		SYDNEY_STATE_TRIPPING,
		SYDNEY_STATE_GETTING_UP,
		SYDNEY_STATE_COWERING,
		SYDNEY_STATE_STRIKE_A_POSE
	};

	enum
	{
		HEAD_STATE_NONE					= 0,
		HEAD_STATE_LOOKING_AT_STAR,
		HEAD_STATE_LOOKING_AT_SYDNEY,
	};

	enum
	{
		ATTACK_STATE_NONE						= 0,
		ATTACK_STATE_MELEE,
		ATTACK_STATE_MELEE_FLYING_SIDEKICK_RUN,
		ATTACK_STATE_MELEE_FLYING_SIDEKICK,
		ATTACK_STATE_MELEE_CRESENT_KICK,
		ATTACK_STATE_MELEE_SIDE_KICK,
		ATTACK_STATE_MELEE_SIDE_KICK_RETRACT,
		ATTACK_STATE_MELEE_PUNCH,
		ATTACK_STATE_FLAMETHROWER,
		ATTACK_STATE_FIREBALL,
		ATTACK_STATE_HANDGUN,
		ATTACK_STATE_SYDNEY
	};

	enum
	{
		CAMERA_STATE_NORMAL					= 0,
		CAMERA_STATE_FACE_ZOOM,
		CAMERA_STATE_WAYPATH_FOLLOW,
		CAMERA_STATE_LOOK_AT_DEAD_BOSS
	};

	//
	//	Overall state handlers
	//	
	DECLARE_STATE (OVERALL_STATE_FLYING_SIDEKICK,	SM_BEGIN,		SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (OVERALL_STATE_MELEE_ATTACK,		SM_BEGIN,		SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (OVERALL_STATE_RANGED_ATTACK,		SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (OVERALL_STATE_FIREBALL_ATTACK,	SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);	
	DECLARE_STATE (OVERALL_STATE_FIND_HEALTH,			SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (OVERALL_STATE_SYDNEY_BOLTS,		SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (OVERALL_STATE_RUN_AFTER_SYDNEY,	SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (OVERALL_STATE_TOY_WITH_SYDNEY,	SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (OVERALL_STATE_DEATH_SEQUENCE,		SM_BEGIN,		SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);

	//
	//	Move state handlers
	//
	DECLARE_STATE (MOVE_STATE_STOP,						SM_BEGIN,		SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (MOVE_STATE_GET_CLOSE_TO_PLAYER,	SM_BEGIN,		SM_END,		SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (MOVE_STATE_ATTACK_PATTERN1,		SM_BEGIN,		SM_END,		SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (MOVE_STATE_ATTACK_PATTERN2,		SM_BEGIN,		SM_END,		SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (MOVE_STATE_GOTO_HELIPAD,			SM_BEGIN,		SM_END,		SM_THINK,		SM_NO_REQ_END);	
	DECLARE_STATE (MOVE_STATE_RUN_TO_HEALTH,			SM_BEGIN,		SM_END,		SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (MOVE_STATE_CHASE_SYDNEY,			SM_BEGIN,		SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);

	//
	//	Mendoza state handlers
	//
	DECLARE_STATE (MENDOZA_STATE_STANDING,				SM_BEGIN,		SM_NO_END,	SM_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (MENDOZA_STATE_FALLING,				SM_BEGIN,		SM_NO_END,	SM_THINK,	SM_REQ_END);
	DECLARE_STATE (MENDOZA_STATE_KIPPING,				SM_BEGIN,		SM_END,		SM_THINK,	SM_REQ_END);
	DECLARE_STATE (MENDOZA_STATE_PACK_EXPLODING,		SM_BEGIN,		SM_NO_END,	SM_THINK,	SM_REQ_END);
	DECLARE_STATE (MENDOZA_STATE_DYING,					SM_BEGIN,		SM_NO_END,	SM_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (MENDOZA_STATE_DEAD,					SM_BEGIN,		SM_NO_END,	SM_THINK,	SM_NO_REQ_END);

	//
	//	Sydney state handlers
	//
	DECLARE_STATE (SYDNEY_STATE_WAITING,			SM_BEGIN,		SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (SYDNEY_STATE_BOLTING,			SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (SYDNEY_STATE_TRIPPING,			SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (SYDNEY_STATE_GETTING_UP,		SM_BEGIN,		SM_NO_END,	SM_THINK,		SM_NO_REQ_END);	
	DECLARE_STATE (SYDNEY_STATE_COWERING,			SM_BEGIN,		SM_END,		SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (SYDNEY_STATE_STRIKE_A_POSE,	SM_BEGIN,		SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);

	//
	//	Head state handlers
	//
	DECLARE_STATE (HEAD_STATE_NONE,						SM_BEGIN,		SM_NO_END,	SM_NO_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (HEAD_STATE_LOOKING_AT_STAR,		SM_NO_BEGIN,	SM_NO_END,	SM_THINK,		SM_NO_REQ_END);
	DECLARE_STATE (HEAD_STATE_LOOKING_AT_SYDNEY,		SM_NO_BEGIN,	SM_NO_END,	SM_THINK,		SM_NO_REQ_END);

	//
	//	Attack state handlers
	//
	DECLARE_STATE (ATTACK_STATE_NONE,								SM_BEGIN,		SM_NO_END,	SM_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (ATTACK_STATE_MELEE,								SM_BEGIN,		SM_NO_END,	SM_THINK,	SM_NO_REQ_END);	
	DECLARE_STATE (ATTACK_STATE_MELEE_FLYING_SIDEKICK_RUN,	SM_BEGIN,		SM_NO_END,	SM_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (ATTACK_STATE_MELEE_FLYING_SIDEKICK,			SM_BEGIN,		SM_END,		SM_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (ATTACK_STATE_MELEE_CRESENT_KICK,				SM_BEGIN,		SM_END,		SM_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (ATTACK_STATE_MELEE_SIDE_KICK,					SM_BEGIN,		SM_END,		SM_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (ATTACK_STATE_MELEE_SIDE_KICK_RETRACT,		SM_BEGIN,		SM_END,		SM_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (ATTACK_STATE_MELEE_PUNCH,						SM_BEGIN,		SM_END,		SM_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (ATTACK_STATE_FLAMETHROWER,						SM_BEGIN,		SM_END,		SM_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (ATTACK_STATE_FIREBALL,							SM_BEGIN,		SM_END,		SM_THINK,	SM_NO_REQ_END);	
	DECLARE_STATE (ATTACK_STATE_HANDGUN,							SM_BEGIN,		SM_END,		SM_THINK,	SM_NO_REQ_END);
	DECLARE_STATE (ATTACK_STATE_SYDNEY,								SM_BEGIN,		SM_END,		SM_THINK,	SM_NO_REQ_END);

	//
	//	Camera state handlers
	//
	void						On_CAMERA_STATE_NORMAL_Begin (void);

	void						On_CAMERA_STATE_FACE_ZOOM_Begin (void);
	void						On_CAMERA_STATE_FACE_ZOOM_Think (void);

	void						On_CAMERA_STATE_WAYPATH_FOLLOW_Begin (void);
	void						On_CAMERA_STATE_WAYPATH_FOLLOW_Think (void);

	void						On_CAMERA_STATE_LOOK_AT_DEAD_BOSS_Begin (void);
	void						On_CAMERA_STATE_LOOK_AT_DEAD_BOSS_Think (void);

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
	
	///////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////
	float							MendozaTauntTimeLeft;
	int							TauntList[MAX_TAUNTS];
	int							AvailableTaunts;

	float							OverallStateTimer;
	float							MendozaStateTimer;
	float							CameraStateTimer;
	float							CameraShakeTimer;
	float							AttackStateTimer;
	float							RandomTargetTimer;
	float							SydneyStateTimer;
	float							MoveStateTimer;
	float							NextMeleeAttackTime;
	float							FireballStartTimer;
	float							NextKnockdownAvailableTime;
	bool							HasMeleeAttackHit;
	float							LastMeleeAnimFrame;
	float							StartTimer;

	RenderObjClass *			CameraBoneModel;
	GameObjReference			Sydney;
	StringClass					AttackingBoneName;

	Vector3						SideKickPos;
	Vector3						ShootGroundPos;
	CardinalSpline3DClass	CameraSpline;

	Vector3						MendozaPos;
	Vector3						StarPos;

	StateMachineClass<MendozaBossGameObjClass>	OverallState;
	StateMachineClass<MendozaBossGameObjClass>	MendozaState;
	StateMachineClass<MendozaBossGameObjClass>	SydneyState;
	StateMachineClass<MendozaBossGameObjClass>	MoveState;
	StateMachineClass<MendozaBossGameObjClass>	HeadState;
	StateMachineClass<MendozaBossGameObjClass>	CameraState;
	StateMachineClass<MendozaBossGameObjClass>	AttackState;
};


#endif //__MENDOZABOSSGAMEOBJ_H
