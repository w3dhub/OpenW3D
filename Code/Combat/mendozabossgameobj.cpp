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
 *                     $Archive:: /Commando/Code/Combat/mendozabossgameobj.cpp                $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/23/02 3:28p                                               $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "mendozabossgameobj.h"
#include "wwhack.h"
#include "simpledefinitionfactory.h"
#include "persistfactory.h"
#include "combatchunkid.h"
#include "debug.h"
#include "animcontrol.h"
#include "crandom.h"
#include "weapons.h"
#include "rendobj.h"
#include "WWAudio.h"
#include "AudibleSound.h"
#include "explosion.h"
#include "phys.h"
#include "pscene.h"
#include "waypath.h"
#include "waypoint.h"
#include "Pathfind.h"
#include "pathmgr.h"
#include "string_ids.h"
#include "translateobj.h"
#include "translatedb.h"
#include "wwprofile.h"
#include "conversationmgr.h"
#include "activeconversation.h"
#include "conversation.h"
#include "combat.h"
#include "assetmgr.h"
#include "ccamera.h"
#include "gameobjmanager.h"
#include "transitioneffect.h"
#include "coltest.h"
#include "phys3.h"
#include "animobj.h"
#include "powerup.h"
#include "weaponbag.h"
#include "humanphys.h"
#include "physcoltest.h"
#include "objlibrary.h"
#include "hudinfo.h"
#include "colmathaabox.h" // Agressive inlining causes linker issues if this isn't here.
#include <algorithm>


DECLARE_FORCE_LINK (MendozaBoss)


//////////////////////////////////////////////////////////////////////////
//	Constants
//////////////////////////////////////////////////////////////////////////
static const AABoxClass BOSS_AREA_BOX01 (Vector3 (-19.2585F, 27.7224F, 0.434677F), Vector3 (5.06771F, 15.6146F, 1.0F));
static const AABoxClass BOSS_AREA_BOX02 (Vector3 (-42.3353F, 32.8650F, 0.434677F), Vector3 (18.1846F, 22.7105F, 1.0F));
static const AABoxClass BOSS_AREA_BOX03 (Vector3 (-64.4650F, 29.5306F, 0.434677F), Vector3 (4.15638F, 18.2853F, 1.0F));

static Vector3		MENDOZA_END_POS (-58.088F, 15.389F, 0.334F);
static Vector3		SYDNEY_END_POS (-57.931F, 12.133F, 0.412F);
static Vector3		HELIPAD_CENTER_POS (-43.329F, 30.796F, 1.605F);

static const float	UNINITIALIZED_TIMER	= -5000.0F;
static const float	FALLDOWN_TIMER			= 6.0F;

//////////////////////////////////////////////////////////////////////////
//	Waypath IDs
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//	Weapon and state enumerations
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
//	Taunt constants
//////////////////////////////////////////////////////////////////////////

static const char *TAUNT_IDS[6] =
{
	"IDS_SAKURA_BOSS_TAUNT1",
	"IDS_SAKURA_BOSS_TAUNT2",
	"IDS_SAKURA_BOSS_TAUNT3",
	"IDS_SAKURA_BOSS_TAUNT4",
	"IDS_SAKURA_BOSS_TAUNT5",
	"IDS_SAKURA_BOSS_TAUNT6"
};


//////////////////////////////////////////////////////////////////////////
//	Save/load constants
//////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_DEF_PARENT								=	0x09230242,
	CHUNKID_DEF_VARIABLES,

	VARID_DEF_GATLING_DEF_ID						= 1,
	VARID_DEF_ROCKET_DEF_ID,
	VARID_DEF_GATLING_REV_SOUND_ID,
	VARID_DEF_ROCKET_DOOR_SOUND_ID,
	VARID_DEF_ROCKET_DESTROYED_EXPLOSION_ID
};

enum
{
	CHUNKID_PARENT										=	0x09230243,
	CHUNKID_VARIABLES,
	CHUNKID_STATE_MACHINES,
	CHUNKID_CAMERA_SPLINE,
	CHUNKID_OVERALL_STATE_MACHINE,
	CHUNKID_MENDOZA_STATE_MACHINE,
	CHUNKID_SYDNEY_STATE_MACHINE,
	CHUNKID_MOVE_STATE_MACHINE,
	CHUNKID_HEAD_STATE_MACHINE,
	CHUNKID_CAMERA_STATE_MACHINE,
	CHUNKID_ATTACK_STATE_MACHINE,

	VARID_OVERALLSTATE_TIMER					= 0,
	VARID_MENDOZASTATE_TIMER,
	VARID_CAMERASTATE_TIMER,
	VARID_CAMERASHAKE_TIMER,
	VARID_ATTACKSTATE_TIMER,
	VARID_RANDOMTARGET_TIMER,
	VARID_SYDNEYSTATE_TIMER,
	VARID_MOVESTATE_TIMER,
	VARID_NEXT_MELEE_ATTACK_TIME,
	VARID_FIREBALL_START_TIMER,
	VARID_NEXT_KNOCKDOWN_AVAILABLE_TIME,
	VARID_HAS_MELEE_ATTACK_HIT,
	VARID_LAST_MELEE_ANIM_FRAME,
	VARID_START_TIMER,
	VARID_SIDE_KICK_POS,
	VARID_SHOOT_GROUND_POS,
	VARID_ATTACKING_BONE_NAME,
	VARID_CAMERA_BONE_PTR,
	VARID_CAMERA_BONE_TM
};


//////////////////////////////////////////////////////////////////////////
//	Factories
//////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<MendozaBossGameObjDefClass,		CHUNKID_GAME_OBJECT_DEF_MENDOZA_BOSS>						_MendozaBossGameObjDefPersistFactory;
SimplePersistFactoryClass<MendozaBossGameObjClass,			CHUNKID_GAME_OBJECT_MENDOZA_BOSS>							_MendozaBossGameObjPersistFactory;
DECLARE_DEFINITION_FACTORY(MendozaBossGameObjDefClass,	CLASSID_GAME_OBJECT_DEF_MENDOZA_BOSS, "Mendoza Boss")	_MendozaBossGameObjDefDefFactory;



//////////////////////////////////////////////////////////////////////////
//
//	MendozaBossGameObjDefClass
//
//////////////////////////////////////////////////////////////////////////
MendozaBossGameObjDefClass::MendozaBossGameObjDefClass (void)
{
	UseInnateBehavior = false;

	PARAM_SEPARATOR (MendozaBossGameObjDefClass, "Boss Settings");
	PARAM_SEPARATOR (MendozaBossGameObjDefClass, "");
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	~MendozaBossGameObjDefClass
//
//////////////////////////////////////////////////////////////////////////
MendozaBossGameObjDefClass::~MendozaBossGameObjDefClass (void)
{	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Get_Class_ID
//
//////////////////////////////////////////////////////////////////////////
uint32
MendozaBossGameObjDefClass::Get_Class_ID (void) const	
{ 
	return CLASSID_GAME_OBJECT_DEF_MENDOZA_BOSS; 
}


//////////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////////
PersistClass *
MendozaBossGameObjDefClass::Create (void) const
{
	MendozaBossGameObjClass *obj = new MendozaBossGameObjClass;
	obj->Init(*this);
	return obj;
}


//////////////////////////////////////////////////////////////////////////
//
//	Save
//
//////////////////////////////////////////////////////////////////////////
bool
MendozaBossGameObjDefClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_DEF_PARENT);
		SoldierGameObjDef::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_DEF_VARIABLES);
		Save_Variables (csave);
	csave.End_Chunk();

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	Load
//
//////////////////////////////////////////////////////////////////////////
bool
MendozaBossGameObjDefClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		switch(cload.Cur_Chunk_ID ()) {

			case CHUNKID_DEF_PARENT:
				SoldierGameObjDef::Load (cload);
				break;

			case CHUNKID_DEF_VARIABLES:
				Load_Variables (cload);
				break;

			default:
				Debug_Say (("Unrecognized MendozaBossGameObjDefClass chunkID\n"));
				break;
		}

		cload.Close_Chunk ();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//	Save_Variables
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjDefClass::Save_Variables (ChunkSaveClass &/* csave */)
{
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjDefClass::Load_Variables (ChunkLoadClass &/* cload */)
{
	/*while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			default:
				Debug_Say (("Unrecognized MendozaBossGameObjDefClass Variable chunkID %d\n", cload.Cur_Micro_Chunk_ID ()));
				break;
		}

		cload.Close_Micro_Chunk ();
	}*/

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
///////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
MendozaBossGameObjDefClass::Get_Factory (void) const
{ 
	return _MendozaBossGameObjDefPersistFactory; 
}


/*
**
**	Start of MendozaBossGameObjClass
**
*/


///////////////////////////////////////////////////////////////////////////
//
//	MendozaBossGameObjClass
//
///////////////////////////////////////////////////////////////////////////
MendozaBossGameObjClass::MendozaBossGameObjClass (void)	:
	MendozaTauntTimeLeft (0),
	AvailableTaunts (0xFFFFFF),
	OverallState (this),
	MendozaState (this),
	SydneyState (this),
	AttackState (this),
	CameraState (this),
	MoveState (this),
	HeadState (this),
	OverallStateTimer (0),
	MendozaStateTimer (0),	
	CameraShakeTimer (0),
	CameraStateTimer (0),
	StartTimer (0),
	AttackStateTimer (0),
	RandomTargetTimer (0),
	FireballStartTimer (0),
	NextKnockdownAvailableTime (0),
	SydneyStateTimer (0),
	HasMeleeAttackHit (false),
	LastMeleeAnimFrame (0),
	NextMeleeAttackTime (0),
	MoveStateTimer (0),
	CameraBoneModel (NULL),
	Sydney (NULL),
	MendozaPos (0, 0, 0),
	StarPos (0, 0, 0),
	ShootGroundPos (0, 0, 0),
	SideKickPos (0, 0, 0)
{
	Shuffle_Taunt_List ();
	TauntList[0] = 1;//IDS_SAKURA_BOSS_TAUNT1;
	TauntList[1] = 2;//IDS_SAKURA_BOSS_TAUNT2;
	TauntList[2] = 3;//IDS_SAKURA_BOSS_TAUNT3;
	TauntList[3] = 4;//IDS_SAKURA_BOSS_TAUNT4;
	TauntList[4] = 5;//IDS_SAKURA_BOSS_TAUNT5;
	TauntList[5] = 6;//IDS_SAKURA_BOSS_TAUNT6;

	//
	//	Register the Overall states with its state machine
	//	
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, OverallState, OVERALL_STATE_FLYING_SIDEKICK);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, OverallState, OVERALL_STATE_MELEE_ATTACK);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, OverallState, OVERALL_STATE_RANGED_ATTACK);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, OverallState, OVERALL_STATE_FIREBALL_ATTACK);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, OverallState, OVERALL_STATE_FIND_HEALTH);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, OverallState, OVERALL_STATE_SYDNEY_BOLTS);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, OverallState, OVERALL_STATE_RUN_AFTER_SYDNEY);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, OverallState, OVERALL_STATE_TOY_WITH_SYDNEY);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, OverallState, OVERALL_STATE_DEATH_SEQUENCE);
	
	//
	//	Register the Move states with its state machine
	//
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, MoveState, MOVE_STATE_STOP);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, MoveState, MOVE_STATE_GET_CLOSE_TO_PLAYER);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, MoveState, MOVE_STATE_ATTACK_PATTERN1);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, MoveState, MOVE_STATE_ATTACK_PATTERN2);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, MoveState, MOVE_STATE_GOTO_HELIPAD);	
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, MoveState, MOVE_STATE_RUN_TO_HEALTH);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, MoveState, MOVE_STATE_CHASE_SYDNEY);

	//
	//	Register the Mendoza states with its state machine
	//
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, MendozaState, MENDOZA_STATE_STANDING);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, MendozaState, MENDOZA_STATE_FALLING);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, MendozaState, MENDOZA_STATE_KIPPING);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, MendozaState, MENDOZA_STATE_PACK_EXPLODING);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, MendozaState, MENDOZA_STATE_DYING);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, MendozaState, MENDOZA_STATE_DEAD);
	MendozaState.Set_State (MENDOZA_STATE_STANDING);
	
	//
	//	Register the Sydney states with its state machine
	//
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, SydneyState, SYDNEY_STATE_WAITING);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, SydneyState, SYDNEY_STATE_BOLTING);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, SydneyState, SYDNEY_STATE_TRIPPING);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, SydneyState, SYDNEY_STATE_GETTING_UP);	
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, SydneyState, SYDNEY_STATE_COWERING);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, SydneyState, SYDNEY_STATE_STRIKE_A_POSE);	

	//
	//	Register the Head states with its state machine
	//
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, HeadState, HEAD_STATE_NONE);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, HeadState, HEAD_STATE_LOOKING_AT_STAR);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, HeadState, HEAD_STATE_LOOKING_AT_SYDNEY);

	//
	//	Register the attack states with its state machine
	//
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, AttackState, ATTACK_STATE_NONE);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, AttackState, ATTACK_STATE_MELEE);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, AttackState, ATTACK_STATE_MELEE_FLYING_SIDEKICK_RUN);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, AttackState, ATTACK_STATE_MELEE_FLYING_SIDEKICK);	
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, AttackState, ATTACK_STATE_MELEE_CRESENT_KICK);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, AttackState, ATTACK_STATE_MELEE_SIDE_KICK);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, AttackState, ATTACK_STATE_MELEE_SIDE_KICK_RETRACT);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, AttackState, ATTACK_STATE_MELEE_PUNCH);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, AttackState, ATTACK_STATE_FLAMETHROWER);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, AttackState, ATTACK_STATE_FIREBALL);	
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, AttackState, ATTACK_STATE_HANDGUN);
	ADD_STATE_TO_MACHINE (MendozaBossGameObjClass, AttackState, ATTACK_STATE_SYDNEY);
	AttackState.Set_State (ATTACK_STATE_NONE);	

	//
	//	Register the camera states with its state machine
	//
	CameraState.Add_State (NULL,													NULL,		&MendozaBossGameObjClass::On_CAMERA_STATE_NORMAL_Begin,					NULL);
	CameraState.Add_State (&MendozaBossGameObjClass::On_CAMERA_STATE_FACE_ZOOM_Think,				NULL,		&MendozaBossGameObjClass::On_CAMERA_STATE_FACE_ZOOM_Begin,				NULL);
	CameraState.Add_State (&MendozaBossGameObjClass::On_CAMERA_STATE_WAYPATH_FOLLOW_Think,		NULL,		&MendozaBossGameObjClass::On_CAMERA_STATE_WAYPATH_FOLLOW_Begin,		NULL);
	CameraState.Add_State (&MendozaBossGameObjClass::On_CAMERA_STATE_LOOK_AT_DEAD_BOSS_Think,	NULL,		&MendozaBossGameObjClass::On_CAMERA_STATE_LOOK_AT_DEAD_BOSS_Begin,	NULL);	
	CameraState.Set_State (CAMERA_STATE_NORMAL);

	//
	//	Create the camera bone model...
	//
	CameraBoneModel = WW3DAssetManager::Get_Instance ()->Create_Render_Obj ("CAMBONE");
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	~MendozaBossGameObjClass
//
///////////////////////////////////////////////////////////////////////////
MendozaBossGameObjClass::~MendozaBossGameObjClass (void)
{
	REF_PTR_RELEASE (CameraBoneModel);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
///////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
MendozaBossGameObjClass::Get_Factory (void) const 
{
	return _MendozaBossGameObjPersistFactory;
}


////////////////////////////////////////////////////////////////
//
//	Init
//
////////////////////////////////////////////////////////////////
void MendozaBossGameObjClass::Init (void)
{
	Init ( Get_Definition ());
	return ;
}

///////////////////////////////////////////////////////////////////////////
//
//	Init
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::Init (const MendozaBossGameObjDefClass &definition)
{
	SoldierGameObj::Init (definition);
	Initialize_Boss ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Definition
//
///////////////////////////////////////////////////////////////////////////
const MendozaBossGameObjDefClass &
MendozaBossGameObjClass::Get_Definition (void) const
{
	return (const MendozaBossGameObjDefClass &)BaseGameObj::Get_Definition ();
}


///////////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////////
bool
MendozaBossGameObjClass::Save (ChunkSaveClass & csave)
{
	csave.Begin_Chunk (CHUNKID_PARENT);
		SoldierGameObj::Save (csave);
	csave.End_Chunk ();

	//
	//	Save the state machines to their own chunk
	//
	csave.Begin_Chunk (CHUNKID_OVERALL_STATE_MACHINE);
		OverallState.Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_MENDOZA_STATE_MACHINE);
		MendozaState.Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_SYDNEY_STATE_MACHINE);
		SydneyState.Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_MOVE_STATE_MACHINE);
		MoveState.Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_HEAD_STATE_MACHINE);
		HeadState.Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_CAMERA_STATE_MACHINE);
		CameraState.Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_ATTACK_STATE_MACHINE);
		AttackState.Save (csave);		
	csave.End_Chunk ();

	//
	//	Save the camera spline
	//
	csave.Begin_Chunk (CHUNKID_CAMERA_SPLINE);
		CameraSpline.Save (csave);
	csave.End_Chunk ();
	
	csave.Begin_Chunk (CHUNKID_VARIABLES);
		Save_Variables (csave);
	csave.End_Chunk ();
	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load
//
///////////////////////////////////////////////////////////////////////////
bool
MendozaBossGameObjClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		switch(cload.Cur_Chunk_ID ()) {

			case CHUNKID_PARENT:
				SoldierGameObj::Load (cload);
				break;

			case CHUNKID_OVERALL_STATE_MACHINE:
				OverallState.Load (cload);
				break;

			case CHUNKID_MENDOZA_STATE_MACHINE:
				MendozaState.Load (cload);
				break;

			case CHUNKID_SYDNEY_STATE_MACHINE:
				SydneyState.Load (cload);
				break;

			case CHUNKID_MOVE_STATE_MACHINE:
				MoveState.Load (cload);
				break;

			case CHUNKID_HEAD_STATE_MACHINE:
				HeadState.Load (cload);
				break;

			case CHUNKID_CAMERA_STATE_MACHINE:
				CameraState.Load (cload);
				break;

			case CHUNKID_ATTACK_STATE_MACHINE:
				AttackState.Load (cload);		
				break;

			//
			//	Load the state machines from their chunk
			//
			case CHUNKID_STATE_MACHINES:
				OverallState.Load (cload);
				MendozaState.Load (cload);
				SydneyState.Load (cload);
				MoveState.Load (cload);
				HeadState.Load (cload);
				CameraState.Load (cload);
				AttackState.Load (cload);		
				break;

			//
			//	Load the camera spline
			//
			case CHUNKID_CAMERA_SPLINE:
				CameraSpline.Load (cload);
				break;

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;

			default:
				Debug_Say (("Unrecognized MendozaBossGameObjClass chunk ID\n"));
				break;

		}

		cload.Close_Chunk ();
	}

	SaveLoadSystemClass::Register_Post_Load_Callback (this);
	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_Post_Load
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_Post_Load (void)
{	
	SoldierGameObj::On_Post_Load ();
	Initialize_Boss ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Initialize_Boss
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::Initialize_Boss (void)
{
	//
	//	Find Sydney
	//
	PhysicalGameObj *game_obj = GameObjManager::Find_PhysicalGameObj (101010);
	if (game_obj != NULL) {
		Sydney = game_obj->As_SoldierGameObj ();
		
		//
		//	Turn off her innate AI...
		//
		Sydney.Get_Ptr ()->As_PhysicalGameObj ()->As_SoldierGameObj ()->Innate_Disable ();
	}

	return ;	
}


///////////////////////////////////////////////////////////////////////////
//
//	Save_Variables
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::Save_Variables (ChunkSaveClass &csave)
{
	Matrix3D cam_tm = CameraBoneModel->Get_Transform ();
	WRITE_MICRO_CHUNK_PTR (csave, VARID_CAMERA_BONE_PTR,					CameraBoneModel);
	WRITE_MICRO_CHUNK (csave, VARID_CAMERA_BONE_TM,						cam_tm);

	WRITE_MICRO_CHUNK (csave, VARID_OVERALLSTATE_TIMER,				OverallStateTimer);
	WRITE_MICRO_CHUNK (csave, VARID_MENDOZASTATE_TIMER,				MendozaStateTimer);
	WRITE_MICRO_CHUNK (csave, VARID_CAMERASTATE_TIMER,					CameraStateTimer);
	WRITE_MICRO_CHUNK (csave, VARID_CAMERASHAKE_TIMER,					CameraShakeTimer);
	WRITE_MICRO_CHUNK (csave, VARID_ATTACKSTATE_TIMER,					AttackStateTimer);
	WRITE_MICRO_CHUNK (csave, VARID_RANDOMTARGET_TIMER,				RandomTargetTimer);
	WRITE_MICRO_CHUNK (csave, VARID_SYDNEYSTATE_TIMER,					SydneyStateTimer);
	WRITE_MICRO_CHUNK (csave, VARID_MOVESTATE_TIMER,					MoveStateTimer);
	WRITE_MICRO_CHUNK (csave, VARID_NEXT_MELEE_ATTACK_TIME,			NextMeleeAttackTime);
	WRITE_MICRO_CHUNK (csave, VARID_FIREBALL_START_TIMER,				FireballStartTimer);
	WRITE_MICRO_CHUNK (csave, VARID_NEXT_KNOCKDOWN_AVAILABLE_TIME,	NextKnockdownAvailableTime);
	WRITE_MICRO_CHUNK (csave, VARID_HAS_MELEE_ATTACK_HIT,				HasMeleeAttackHit);
	WRITE_MICRO_CHUNK (csave, VARID_LAST_MELEE_ANIM_FRAME,			LastMeleeAnimFrame);
	WRITE_MICRO_CHUNK (csave, VARID_START_TIMER,							StartTimer);
	WRITE_MICRO_CHUNK (csave, VARID_SIDE_KICK_POS,						SideKickPos);
	WRITE_MICRO_CHUNK (csave, VARID_SHOOT_GROUND_POS,					ShootGroundPos);
	WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_ATTACKING_BONE_NAME,	AttackingBoneName);	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::Load_Variables (ChunkLoadClass &cload)
{
	RenderObjClass *old_camera_bone_ptr = NULL;
	Matrix3D cam_tm (1);

	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK_PTR (cload, VARID_CAMERA_BONE_PTR,						old_camera_bone_ptr);
			READ_MICRO_CHUNK (cload, VARID_CAMERA_BONE_TM,						cam_tm);

			READ_MICRO_CHUNK (cload, VARID_OVERALLSTATE_TIMER,					OverallStateTimer);
			READ_MICRO_CHUNK (cload, VARID_MENDOZASTATE_TIMER,					MendozaStateTimer);
			READ_MICRO_CHUNK (cload, VARID_CAMERASTATE_TIMER,					CameraStateTimer);
			READ_MICRO_CHUNK (cload, VARID_CAMERASHAKE_TIMER,					CameraShakeTimer);
			READ_MICRO_CHUNK (cload, VARID_ATTACKSTATE_TIMER,					AttackStateTimer);
			READ_MICRO_CHUNK (cload, VARID_RANDOMTARGET_TIMER,					RandomTargetTimer);
			READ_MICRO_CHUNK (cload, VARID_SYDNEYSTATE_TIMER,					SydneyStateTimer);
			READ_MICRO_CHUNK (cload, VARID_MOVESTATE_TIMER,						MoveStateTimer);
			READ_MICRO_CHUNK (cload, VARID_NEXT_MELEE_ATTACK_TIME,			NextMeleeAttackTime);
			READ_MICRO_CHUNK (cload, VARID_FIREBALL_START_TIMER,				FireballStartTimer);
			READ_MICRO_CHUNK (cload, VARID_NEXT_KNOCKDOWN_AVAILABLE_TIME,	NextKnockdownAvailableTime);
			READ_MICRO_CHUNK (cload, VARID_HAS_MELEE_ATTACK_HIT,				HasMeleeAttackHit);
			READ_MICRO_CHUNK (cload, VARID_LAST_MELEE_ANIM_FRAME,				LastMeleeAnimFrame);
			READ_MICRO_CHUNK (cload, VARID_START_TIMER,							StartTimer);
			READ_MICRO_CHUNK (cload, VARID_SIDE_KICK_POS,						SideKickPos);
			READ_MICRO_CHUNK (cload, VARID_SHOOT_GROUND_POS,					ShootGroundPos);
			READ_MICRO_CHUNK_WWSTRING (cload, VARID_ATTACKING_BONE_NAME,	AttackingBoneName);	

			default:
				Debug_Say (("Unrecognized MendozaBossGameObjClass Variable chunkID %d\n", cload.Cur_Micro_Chunk_ID ()));
				break;
		}

		cload.Close_Micro_Chunk ();
	}

	//
	//	Register the camera bone pointers...
	//
	if (old_camera_bone_ptr != NULL) {			
		SaveLoadSystemClass::Register_Pointer (old_camera_bone_ptr, CameraBoneModel);
		CameraBoneModel->Set_Transform (cam_tm);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Apply_Control
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::Apply_Control (void)
{
	SoldierGameObj::Apply_Control ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Think
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::Think (void)
{	
	WWPROFILE ("Mendoza Think");	

	bool ok_to_think = true;

//
//	Don't do this in the editor
//
#ifdef PARAM_EDITING_ON
	ok_to_think = false;
#endif
	
	if (COMBAT_STAR == NULL || Sydney == NULL) {
		ok_to_think = false;
	}

	if (ok_to_think) {

		//
		//	Get the position of both Mendoza and the star (this information
		// is used throughout the AI)
		//
		Get_Position (&MendozaPos);
		COMBAT_STAR->Get_Position (&StarPos);

		//
		//	Check to see if its OK to kick off the boss logic
		//
		if (StartTimer > UNINITIALIZED_TIMER) {
			StartTimer -= TimeManager::Get_Frame_Seconds ();
			if (StartTimer <= 0) {
				Spawn_Health_Powerups ();
				StartTimer = UNINITIALIZED_TIMER;
				OverallState.Set_State (OVERALL_STATE_FLYING_SIDEKICK);
			}
		} else {

			//
			//	Decrement the "global" timer that determines when its OK
			// for Mendoza to fall down due to a frontal hit
			//
			NextKnockdownAvailableTime -= TimeManager::Get_Frame_Seconds ();

			//
			//	Let the state machines think
			//
			OverallState.Think ();
			MendozaState.Think ();
			MoveState.Think ();
			HeadState.Think ();
			CameraState.Think ();
			AttackState.Think ();
			SydneyState.Think ();
		}	
	}

	//
	//	Give Mendoza 100 extra health points so he doesn't get "killed" due to any
	// "special" damage such as fire or electricity
	//
	float old_health		= DefenseObject.Get_Health ();
	float old_health_max	= DefenseObject.Get_Health_Max ();
	old_health			+= 100.0F;
	old_health_max		+= 100.0F;
	DefenseObject.Set_Health_Max (old_health_max);
	DefenseObject.Set_Health (old_health);	

	//
	//	Let the soldier think
	//
	SoldierGameObj::Think ();

	//
	//	Restore his health
	//
	float curr_health = DefenseObject.Get_Health ();
	curr_health		-= 100.0F;
	old_health_max	-= 100.0F;
	DefenseObject.Set_Health (std::max (curr_health, 1.0F));
	DefenseObject.Set_Health_Max (old_health_max);	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Shuffle_Taunt_List
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::Shuffle_Taunt_List (void)
{
	::memset (TauntList, 0, sizeof (TauntList));
	
	//
	//	Grab an entry from the taunt id list
	//
	for (int index = 0; index < MAX_TAUNTS; index ++) {

		//
		//	Choose a random list entry to plug this taunt ID into
		//
		int list_index = 0;
		do
		{
			list_index = FreeRandom.Get_Int (MAX_TAUNTS);
		} while (TauntList[list_index] != 0);

		//
		//	Stick this taunt ID into the list
		//
		TauntList[list_index] = index + 1;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Apply_Damage_Extended
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::Apply_Damage_Extended
(
	const OffenseObjectClass &		damager,
	float									scale,
	const Vector3 &					direction,
	const char *						collision_box_name
)
{
	if (	!CombatManager::I_Am_Server () ||
			OverallState.Get_State () == OVERALL_STATE_DEATH_SEQUENCE)
	{
		return ;
	}

	//
	//	Give Mendoza 100 extra health points so he doesn't get "killed"
	//
	float old_health		= DefenseObject.Get_Health ();

	//
	//	Apply the damage
	//
	DefenseObject.Set_Can_Object_Die (false);
	SoldierGameObj::Apply_Damage_Extended (damager, scale, direction, collision_box_name);

	//
	//	Readjust the current and old health
	//
	float curr_health	= DefenseObject.Get_Health ();

	//
	//	If Sydney is cowering in front of Mendoza then we should trigger
	// the end sequence soon...
	//
	if (SydneyState.Get_State () == SYDNEY_STATE_COWERING) {
		if (FreeRandom.Get_Int (4) == 1) {
			OverallState.Set_State (OVERALL_STATE_DEATH_SEQUENCE);
		}
	} else {

		//
		//	Calculate what our health percents were both before and after this damage
		//
		float old_health_percent	= old_health / DefenseObject.Get_Health_Max ();
		float health_percent			= curr_health / DefenseObject.Get_Health_Max ();
		
		//
		//	At 25% health we trigger the "chase after Sydney" event which has different behavior
		//
		if (health_percent > 0.25F) {

			//
			//	Were we shot in the front? (If so we may want to trigger our fall down event)
			//
			if (	health_percent > 0.3F &&
					NextKnockdownAvailableTime <= 0 &&
					OverallState.Get_State () >= OVERALL_STATE_MELEE_ATTACK)
			{
				Vector3 relative_direction = Get_Transform ().Inverse_Rotate_Vector (direction);
				relative_direction.Normalize ();
				if (relative_direction.X < -0.7F && (FreeRandom.Get_Int (5) == 1)) {
					MendozaState.Set_State (MENDOZA_STATE_FALLING);
				}
				NextKnockdownAvailableTime = FALLDOWN_TIMER;
			}

			//
			//	If Mendoza's health crosses the 75% mark, then switch to a ranged attack
			//
			if (old_health_percent > 0.75F && health_percent <= 0.75F) {
				OverallState.Set_State (OVERALL_STATE_RANGED_ATTACK);
			}

		} else {

			//
			//	Did we just change over to this state?
			//
			if (old_health_percent > 0.25F) {
				OverallState.Set_State (OVERALL_STATE_SYDNEY_BOLTS);
			}			
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_MENDOZA_STATE_STANDING_Begin
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_MENDOZA_STATE_STANDING_Begin (void)
{
	MendozaStateTimer = 0;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_MENDOZA_STATE_STANDING_Think
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_MENDOZA_STATE_STANDING_Think (void)
{
	if (COMBAT_STAR == NULL) {
		return ;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_MENDOZA_STATE_FALLING_Begin
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_MENDOZA_STATE_FALLING_Begin (void)
{
	Set_Animation ("S_A_HUMAN.H_A_635A", false);
	AttackState.Halt_State ();
	MoveState.Halt_State ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_MENDOZA_STATE_FALLING_Think
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_MENDOZA_STATE_FALLING_Think (void)
{
	if (Get_Anim_Control ()->Is_Complete ()) {
		MendozaState.Set_State (MENDOZA_STATE_KIPPING, true);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_MENDOZA_STATE_FALLING_Request_End
//
///////////////////////////////////////////////////////////////////////////
bool
MendozaBossGameObjClass::On_MENDOZA_STATE_FALLING_Request_End (int /*new_state*/)
{
	return false;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_MENDOZA_STATE_KIPPING_Begin
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_MENDOZA_STATE_KIPPING_Begin (void)
{
	Set_Blended_Animation ("S_A_HUMAN.H_A_H12C", false);
	HumanState.Set_State (HumanStateClass::LOCKED_ANIMATION);
	((HumanAnimControlClass *)Get_Anim_Control ())->Set_Anim_Speed_Scale (2.0F);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(MENDOZA_STATE_KIPPING)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_END(MENDOZA_STATE_KIPPING) (void)
{
	//
	//	Resume the state's we halted
	//	
	MoveState.Resume_State ();
	AttackState.Set_State (ATTACK_STATE_HANDGUN);
	AttackState.Resume_State ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_MENDOZA_STATE_KIPPING_Think
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_MENDOZA_STATE_KIPPING_Think (void)
{
	if (Get_Anim_Control ()->Is_Complete ()) {
		MendozaState.Set_State (MENDOZA_STATE_STANDING, true);
		HumanState.Set_State (HumanStateClass::ANIMATION);
		((HumanAnimControlClass *)Get_Anim_Control ())->Set_Anim_Speed_Scale (1.0F);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_MENDOZA_STATE_KIPPING_Request_End
//
///////////////////////////////////////////////////////////////////////////
bool
MendozaBossGameObjClass::On_MENDOZA_STATE_KIPPING_Request_End (int /*new_state*/)
{
	return false;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_MENDOZA_STATE_PACK_EXPLODING_Begin
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_MENDOZA_STATE_PACK_EXPLODING_Begin (void)
{
	//
	//	Create the explosion aggregate to attach to Mendoza
	//
	RenderObjClass *die_aggregate = WW3DAssetManager::Get_Instance ()->Create_Render_Obj ("AG_MENDOZA_DIE");	
	if (die_aggregate != NULL) {
		
		//
		//	Play the animation on the object
		//
		HAnimClass *anim = WW3DAssetManager::Get_Instance ()->Get_HAnim ("AG_MENDOZA_DIE.AG_MENDOZA_DIE");
		if (anim != NULL) {
			die_aggregate->Set_Animation (anim, 0, RenderObjClass::ANIM_MODE_ONCE);
			REF_PTR_RELEASE (anim);
		}
		
		//
		//	Attach the object to the back of Mendoza
		//		
		Peek_Model ()->Add_Sub_Object_To_Bone (die_aggregate, "BACKGUNBONE");
		REF_PTR_RELEASE (die_aggregate);
	}

	//
	//	Snap Sydney to the right position
	//
	SydneyState.Set_State (SYDNEY_STATE_STRIKE_A_POSE);

	//
	//	Stay in this state for 8 more seconds
	//
	MendozaStateTimer		= 8.0F;
	CameraShakeTimer		= 8.0F;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_REQUEST_END(MENDOZA_STATE_PACK_EXPLODING)
//
///////////////////////////////////////////////////////////////////////////
bool
MendozaBossGameObjClass::STATE_IMPL_REQUEST_END(MENDOZA_STATE_PACK_EXPLODING) (int /* state */)
{
	return false;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_MENDOZA_STATE_PACK_EXPLODING_Think
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_MENDOZA_STATE_PACK_EXPLODING_Think (void)
{
	float old_timer = MendozaStateTimer;

	CameraShakeTimer -= TimeManager::Get_Frame_Real_Seconds ();
	if (CameraShakeTimer <= 0 && CameraShakeTimer > UNINITIALIZED_TIMER) {

		COMBAT_SCENE->Add_Camera_Shake (MendozaPos, 50.0, 1.0F, 0.25F);
		
		CameraShakeTimer = UNINITIALIZED_TIMER;
	}

	MendozaStateTimer -= TimeManager::Get_Frame_Real_Seconds ();	
	if (MendozaStateTimer <= 0) {

		//
		//	Kick into the dying state
		//
		MendozaState.Set_State (MENDOZA_STATE_DYING, true);
	} else if (old_timer >= 7.5F && MendozaStateTimer < 7.5F) {

		//
		//	Force Mendoza to be on fire
		//
		Set_Special_Damage_Mode (ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_SUPER_FIRE);
		SpecialDamageTimer = 100.0F;		
		Set_Blended_Animation ("S_A_HUMAN.H_A_FLMA", true);

		//
		//	Play the on-fire sound
		//
		Attach_Fire_Sound ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_MENDOZA_STATE_DYING_Begin
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_MENDOZA_STATE_DYING_Begin (void)
{
	//
	//	Start the falling to your knees animation
	//
	Set_Blended_Animation ("S_A_HUMAN.H_A_FLMB", false);
	HumanState.Set_State (HumanStateClass::LOCKED_ANIMATION);

	//
	//	Look at the dying Mendoza for a few seconds
	//
	MendozaStateTimer = 7.0F;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_MENDOZA_STATE_DYING_Think
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_MENDOZA_STATE_DYING_Think (void)
{
	MendozaStateTimer -= TimeManager::Get_Frame_Real_Seconds ();
	if (MendozaStateTimer <= 0) {

		//
		//	Set the dead state
		//
		MendozaState.Set_State (MENDOZA_STATE_DEAD);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_MENDOZA_STATE_DEAD_Begin
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_MENDOZA_STATE_DEAD_Begin (void)
{
	//
	//	Give the user 15 seconds before the level ends
	//
	MendozaStateTimer = 10.0F;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_MENDOZA_STATE_DEAD_Think
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_MENDOZA_STATE_DEAD_Think (void)
{
	MendozaStateTimer -= TimeManager::Get_Frame_Real_Seconds ();
	if (MendozaStateTimer <= 0) {
		
		//
		//	For right now, just restore everything so we can do it again...
		//
		MendozaState.Set_State (MENDOZA_STATE_STANDING);
		Set_Special_Damage_Mode (ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_NONE);
		HumanState.Set_State (HumanStateClass::ANIMATION);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_CAMERA_STATE_NORMAL_Begin
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_CAMERA_STATE_NORMAL_Begin (void)
{
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_CAMERA_STATE_FACE_ZOOM_Begin
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_CAMERA_STATE_FACE_ZOOM_Begin (void)
{
	//
	//	Attach the camera to a dummy object we can move around
	//	
	CameraBoneModel->Set_Transform (COMBAT_CAMERA->Get_Transform ());
	COMBAT_CAMERA->Set_Host_Model (CameraBoneModel);

	//
	//	Slow time down...
	//
	TimeManager::Set_Time_Scale (0.25F);
	GameObjManager::Activate_Cinematic_Freeze (false);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_CAMERA_STATE_FACE_ZOOM_Think
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_CAMERA_STATE_FACE_ZOOM_Think (void)
{
	//
	//	Determine the current and destination points
	//
	Vector3 curr_pos = CameraBoneModel->Get_Transform ().Get_Translation ();
	Vector3 dest_pos = Peek_Model ()->Get_Bone_Transform ("C HEAD").Get_Translation ();
	dest_pos += Vector3 (0, 0, 0.125F);

	//
	//	Get the vector from our current position to our destination
	//
	Vector3 vector = dest_pos - curr_pos;
	if (vector.Length () < 5.0F && MendozaState.Get_State () != MENDOZA_STATE_PACK_EXPLODING) {
		MendozaState.Set_State (MENDOZA_STATE_PACK_EXPLODING, true);
	} else if (vector.Length () < 1.15F) {

		//
		//	Snap Mendoza to the right position
		//
		Matrix3D new_tm (MENDOZA_END_POS);
		new_tm.Rotate_Z (DEG_TO_RADF (270.0F));
		Set_Transform (new_tm);
		
		//
		//	Detonate the backpack and start following the waypath
		//		
		CameraState.Set_State (CAMERA_STATE_WAYPATH_FOLLOW, true);
	} else {
		
		//
		//	Move closer to the target
		//
		vector.Normalize ();
		float time = TimeManager::Get_Frame_Real_Seconds () * 8.0F;
		Vector3 new_pos = curr_pos + (vector * time);

		//
		//	Look at the target
		//
		Matrix3D new_tm;
		new_tm.Look_At (new_pos, dest_pos, 0);

		//
		//	Update the camera's position and orientation
		//
		CameraBoneModel->Set_Transform (new_tm);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_CAMERA_STATE_WAYPATH_FOLLOW_Begin
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_CAMERA_STATE_WAYPATH_FOLLOW_Begin (void)
{
	//
	//	Restore time
	//
	TimeManager::Set_Time_Scale (1.0F);		

	//
	//	Reset the spline
	//
	CameraSpline.Clear_Keys ();

	//
	//	Get the waypath we're going to follow
	//
	WaypathClass *waypath = PathfindClass::Get_Instance ()->Find_Waypath (3000100);
	WWASSERT (waypath != NULL);

	//
	//	Add the points from the waypath to the spline
	//
	float percent = 0.0F;
	float percent_inc = 1.0F / waypath->Get_Point_Count ();
	for (int index = 0; index < waypath->Get_Point_Count (); index ++) {
		
		//
		//	Add this point to the spline
		//
		WaypointClass *waypoint = waypath->Get_Point (index);
		WWASSERT (waypoint != NULL);
		CameraSpline.Add_Key (waypoint->Get_Position (), percent);
		
		//
		//	Increment the percent
		//
		percent += percent_inc;
	}

	//
	//	Let the camera follow the path for 8 seconds
	//
	CameraStateTimer = 8.0F;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_CAMERA_STATE_WAYPATH_FOLLOW_Think
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_CAMERA_STATE_WAYPATH_FOLLOW_Think (void)
{
	//
	//	Evaluate the spline at the current "time"
	//
	float percent = 1.0F - WWMath::Clamp (CameraStateTimer / 8.0F, 0.0F, 1.0F);
	Vector3 camera_position;
	CameraSpline.Evaluate (percent, &camera_position);

	float camera_twist = 0.0F;
	if (percent > 0.77F) {
		camera_twist = ((percent - 0.77F) / 0.33F) * DEG_TO_RADF (-30.0F);
	}

	Vector3 dest_pos = Peek_Model ()->Get_Bone_Transform ("C HEAD").Get_Translation ();

	if (percent < 0.22F) {
		dest_pos += Get_Transform ().Rotate_Vector (Vector3 (2.0F, 0, 0));
	} else if (percent >= 0.22F && percent < 0.55F) {
		float dist = 2.0F * (1.0F - ((percent - 0.22F) / 0.33F));
		dest_pos += Get_Transform ().Rotate_Vector (Vector3 (dist, 0, 0));
	}

	//
	//	Move along the spline, looking at the character's head
	//	
	Matrix3D new_tm;
	new_tm.Look_At (camera_position, dest_pos, camera_twist);
	CameraBoneModel->Set_Transform (new_tm);
	
	//
	//	Kick out of this state when finished
	//
	CameraStateTimer -= TimeManager::Get_Frame_Real_Seconds ();
	if (CameraStateTimer <= 0) {
		CameraState.Set_State (CAMERA_STATE_LOOK_AT_DEAD_BOSS, true);
	}
		
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_CAMERA_STATE_LOOK_AT_DEAD_BOSS_Begin
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_CAMERA_STATE_LOOK_AT_DEAD_BOSS_Begin (void)
{
	//
	//	Look at Mendoza for a few seconds
	//
	CameraStateTimer = 7.0F;
	TimeManager::Set_Time_Scale (0.5F);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_CAMERA_STATE_LOOK_AT_DEAD_BOSS_Think
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_CAMERA_STATE_LOOK_AT_DEAD_BOSS_Think (void)
{
	//
	//	Kick out of this state when finished
	//
	CameraStateTimer -= TimeManager::Get_Frame_Real_Seconds ();
	if (CameraStateTimer <= 0) {
		CameraState.Set_State (CAMERA_STATE_NORMAL, true);

		//
		//	Restore the camera state
		//
		TimeManager::Set_Time_Scale (1.0F);
		COMBAT_CAMERA->Set_Host_Model (NULL);
		REF_PTR_RELEASE (CameraBoneModel);

		//
		//	Trigger mission success
		//
		CombatManager::Mission_Complete (true);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Attach_Sound
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::Attach_Sound (const char *sound_name, const char *bone_name)
{
	//
	//	Create the sound object from its preset
	//
	AudibleSoundClass *sound = WWAudioClass::Get_Instance ()->Create_Sound (sound_name);
	if (sound != NULL) {

		//
		//	Attach the sound to the object
		//
		sound->Attach_To_Object (Peek_Model (), bone_name);

		//
		//	Add the sound to the world and release our hold on it
		//
		sound->Add_To_Scene ();
		REF_PTR_RELEASE (sound);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Attach_Fire_Sound
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::Attach_Fire_Sound (void)
{
	//
	//	Create the sound object from its preset
	//
	AudibleSoundClass *sound = WWAudioClass::Get_Instance ()->Create_Sound ("SFX.Fire_Small_01");
	if (sound != NULL) {

		//
		//	Attach the sound to the object
		//
		sound->Attach_To_Object (Peek_Model ());

		//
		//	Add the sound to the world and release our hold on it
		//
		sound->Add_To_Scene ();
		REF_PTR_RELEASE (sound);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_ATTACK_STATE_NONE_Begin
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_ATTACK_STATE_NONE_Begin (void)
{	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_ATTACK_STATE_NONE_Think
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_ATTACK_STATE_NONE_Think (void)
{
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_ATTACK_STATE_MELEE_Begin
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_ATTACK_STATE_MELEE_Begin (void)
{
	NextMeleeAttackTime = WWMath::Random_Float (0.125F, 0.7F);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_ATTACK_STATE_MELEE_Think
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_ATTACK_STATE_MELEE_Think (void)
{
	//
	//	Is it time to attack the player again?
	//
	NextMeleeAttackTime -= TimeManager::Get_Frame_Seconds ();
	if (NextMeleeAttackTime <= 0) {

		//
		//	Is Mendoza close to the player?
		//
		float dist2 = (MendozaPos - StarPos).Length2 ();
		if (dist2 < 4.0F) {		
			
			//
			//	Pick random melee attack...
			//
			int attack_id = FreeRandom.Get_Int (3);
			if (attack_id == 0) {
				AttackState.Set_State (ATTACK_STATE_MELEE_CRESENT_KICK);
			} else if (attack_id == 1) {
				AttackState.Set_State (ATTACK_STATE_MELEE_SIDE_KICK);
			} else if (attack_id == 2) {
				AttackState.Set_State (ATTACK_STATE_MELEE_PUNCH);
			}
		}
	}


	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(ATTACK_STATE_MELEE_FLYING_SIDEKICK_RUN)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(ATTACK_STATE_MELEE_FLYING_SIDEKICK_RUN) (void)
{
	//
	//	Kill the action
	//
	Get_Action ()->Reset (100);
	HasMeleeAttackHit		= false;
	LastMeleeAnimFrame	= 0;

	//
	//	Aim for the player
	//
	SideKickPos = StarPos;
	Set_Targeting (StarPos, false);

	//
	//	Play the run animation
	//
	Set_Blended_Animation ("S_A_HUMAN.H_A_A0A1", false);
	HumanState.Set_State (HumanStateClass::LOCKED_ANIMATION);

	//
	//	Stop the animation at frame 4
	//
	Get_Anim_Control()->Set_Mode (ANIM_MODE_TARGET, 0);
	Get_Anim_Control()->Set_Target_Frame (4);

	Peek_Physical_Object ()->Enable_Objects_Simulation (false);	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(ATTACK_STATE_MELEE_FLYING_SIDEKICK_RUN)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(ATTACK_STATE_MELEE_FLYING_SIDEKICK_RUN) (void)
{
	//
	//	Move onto the kick if the animation is complete
	//
	if (Get_Anim_Control ()->Is_Complete ()) {
		AttackState.Set_State (ATTACK_STATE_MELEE_FLYING_SIDEKICK);
	} else {

		//
		//	Get a direction vector between Mendoza and his destination
		//
		Vector3 dir_vector	= (SideKickPos - MendozaPos);
		dir_vector.Normalize ();

		const float SIDEKICK_RATE = 10.0F;

		//
		//	Fly through the air to our target
		//
		float rate = SIDEKICK_RATE * TimeManager::Get_Frame_Seconds ();
		Fly_Move (dir_vector * rate);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(ATTACK_STATE_MELEE_FLYING_SIDEKICK)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(ATTACK_STATE_MELEE_FLYING_SIDEKICK) (void)
{
	HasMeleeAttackHit		= false;
	LastMeleeAnimFrame	= 0;

	//
	//	Play the side kick animation
	//
	Set_Blended_Animation ("S_A_HUMAN.H_A_FLYKICK", false);
	HumanState.Set_State (HumanStateClass::LOCKED_ANIMATION);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(ATTACK_STATE_MELEE_FLYING_SIDEKICK)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_END(ATTACK_STATE_MELEE_FLYING_SIDEKICK) (void)
{
	//
	//	Now, play the animation backwards a few frams to simulate "landing"
	//
	int curr_frame = Get_Anim_Control()->Get_Current_Frame ();
	Set_Blended_Animation ("S_A_HUMAN.H_A_FLYKICK", false);
	Get_Anim_Control()->Set_Mode (ANIM_MODE_TARGET, curr_frame);
	Get_Anim_Control()->Set_Target_Frame (0);

	//
	//	Release our lock on the human state
	//
	HumanState.Set_State (HumanStateClass::ANIMATION);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(ATTACK_STATE_MELEE_FLYING_SIDEKICK)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(ATTACK_STATE_MELEE_FLYING_SIDEKICK) (void)
{
	//
	//	Get a direction vector between Mendoza and his destination
	//
	Vector3 dir_vector	= (SideKickPos - MendozaPos);
	float dist2				= dir_vector.Length2 ();
	dir_vector.Normalize ();

	//
	//	Are we close to our goal?
	//
	bool is_finished = false;
	if (dist2 < 0.2F) {
		is_finished = true;
	} else {

		const float SIDEKICK_RATE = 10.0F;

		//
		//	Fly through the air to our target
		//
		float rate = SIDEKICK_RATE * TimeManager::Get_Frame_Seconds ();
		if (Fly_Move (dir_vector * rate)) {
			is_finished = true;
		}

		//
		//	Apply any damage done by the foot that's kicking...
		//
		if (HasMeleeAttackHit == false) {
			HasMeleeAttackHit = Apply_Bone_Collision_Damage ("C L FOOT");
		}
	}

	//
	//	If we're finished, then reset back to normal Melee state
	//
	if (is_finished) {
		Peek_Physical_Object ()->Enable_Objects_Simulation (true);	
		OverallState.Set_State (OVERALL_STATE_MELEE_ATTACK);	
	}

	//
	//	Cache the last frame number...
	//
	LastMeleeAnimFrame = Get_Anim_Control ()->Get_Current_Frame ();	
	return ;
}






///////////////////////////////////////////////////////////////////////////
//
//	On_ATTACK_STATE_MELEE_CRESENT_KICK_Begin
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_ATTACK_STATE_MELEE_CRESENT_KICK_Begin (void)
{
	//
	//	Kill the action
	//
	//Get_Action ()->Reset (100.0F);
	MoveState.Halt_State ();
	HasMeleeAttackHit		= false;
	LastMeleeAnimFrame	= 0;

	//
	//	Aim for the player
	//
	Set_Targeting (StarPos, false);

	//
	//	Play the kick backwards so it becomes a reverse-cresent kick (which
	// looks a lot better).
	//
	Set_Blended_Animation ("S_A_HUMAN.H_A_CRESENTKICK", false, 0, true);
	HumanState.Set_State (HumanStateClass::LOCKED_ANIMATION);

	//
	//	Play the whoosh sound...
	//
	Attach_Sound ("Fight Whoosh Sound Twiddler", "ROOTTRANSFORM");
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(ATTACK_STATE_MELEE_CRESENT_KICK)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_END(ATTACK_STATE_MELEE_CRESENT_KICK) (void)
{
	//
	//	Release our lock on the human state
	//
	HumanState.Set_State (HumanStateClass::ANIMATION);
	MoveState.Resume_State ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_ATTACK_STATE_MELEE_CRESENT_KICK_Think
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_ATTACK_STATE_MELEE_CRESENT_KICK_Think (void)
{
	if (Get_Anim_Control ()->Is_Complete ()) {
		
		//
		//	Reset back to general melee state
		//
		HumanState.Set_State (HumanStateClass::ANIMATION);
		AttackState.Set_State (ATTACK_STATE_MELEE);
	} else if (Get_Anim_Control ()->Get_Current_Frame () < 7.0F) {
		
		//
		//	Apply any damage done by the foot that's kicking...
		//
		if (HasMeleeAttackHit == false) {
			HasMeleeAttackHit = Apply_Bone_Collision_Damage ("C R FOOT");
		}
	}

	//
	//	Cache the last frame number...
	//
	LastMeleeAnimFrame = Get_Anim_Control ()->Get_Current_Frame ();	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_ATTACK_STATE_MELEE_SIDE_KICK_Begin
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_ATTACK_STATE_MELEE_SIDE_KICK_Begin (void)
{
	//
	//	Kill the action
	//
	//Get_Action ()->Reset (100.0F);
	MoveState.Halt_State ();
	HasMeleeAttackHit = false;

	//
	//	Aim for the player
	//
	Set_Targeting (StarPos, false);

	//
	//	Do the kick!
	//
	Set_Blended_Animation ("S_A_HUMAN.H_A_SIDEKICK", false);
	HumanState.Set_State (HumanStateClass::LOCKED_ANIMATION);

	//
	//	Play the whoosh sound...
	//
	Attach_Sound ("Fight Whoosh Sound Twiddler", "ROOTTRANSFORM");
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(ATTACK_STATE_MELEE_SIDE_KICK)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_END(ATTACK_STATE_MELEE_SIDE_KICK) (void)
{
	//
	//	Release our lock on the human state
	//
	HumanState.Set_State (HumanStateClass::ANIMATION);
	MoveState.Resume_State ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_ATTACK_STATE_MELEE_SIDE_KICK_Think
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_ATTACK_STATE_MELEE_SIDE_KICK_Think (void)
{
	if (Get_Anim_Control ()->Is_Complete ()) {
		
		//
		//	Now retract the kick...
		//		
		AttackState.Set_State (ATTACK_STATE_MELEE_SIDE_KICK_RETRACT);

	} else if (Get_Anim_Control ()->Get_Current_Frame () > 6.0F) {
		
		//
		//	Apply any damage done by the foot that's kicking...
		//
		if (HasMeleeAttackHit == false) {
			HasMeleeAttackHit =Apply_Bone_Collision_Damage ("C L FOOT");
		}
	}

	//
	//	Cache the last frame number...
	//
	LastMeleeAnimFrame = Get_Anim_Control ()->Get_Current_Frame ();	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_ATTACK_STATE_MELEE_SIDE_KICK_RETRACT_Begin
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_ATTACK_STATE_MELEE_SIDE_KICK_RETRACT_Begin (void)
{
	//
	//	Play the kick backwards... :>
	//
	Set_Blended_Animation ("S_A_HUMAN.H_A_SIDEKICK", false, 0, true);
	HumanState.Set_State (HumanStateClass::LOCKED_ANIMATION);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(ATTACK_STATE_MELEE_SIDE_KICK_RETRACT)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_END(ATTACK_STATE_MELEE_SIDE_KICK_RETRACT) (void)
{
	//
	//	Release our lock on the human state
	//
	HumanState.Set_State (HumanStateClass::ANIMATION);
	MoveState.Resume_State ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_ATTACK_STATE_MELEE_SIDE_KICK_RETRACT_Think
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_ATTACK_STATE_MELEE_SIDE_KICK_RETRACT_Think (void)
{
	if (Get_Anim_Control ()->Is_Complete ()) {
		
		//
		//	Reset back to general melee state
		//
		HumanState.Set_State (HumanStateClass::ANIMATION);
		AttackState.Set_State (ATTACK_STATE_MELEE);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_ATTACK_STATE_MELEE_PUNCH_Begin
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_ATTACK_STATE_MELEE_PUNCH_Begin (void)
{
	//
	//	Kill the action
	//
	//Get_Action ()->Reset (100.0F);
	MoveState.Halt_State ();
	HasMeleeAttackHit		= false;
	LastMeleeAnimFrame	= 0;

	//
	//	Aim for the player
	//
	Set_Targeting (StarPos, false);

	//
	//	Play the punch animation
	//
	Set_Blended_Animation ("S_A_HUMAN.H_A_PunchCombo", false);
	HumanState.Set_State (HumanStateClass::LOCKED_ANIMATION);
	AttackingBoneName = "C R HAND";

	//
	//	Play the whoosh sound...
	//
	Attach_Sound ("Fight Whoosh Sound Twiddler", "ROOTTRANSFORM");
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(ATTACK_STATE_MELEE_PUNCH)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_END(ATTACK_STATE_MELEE_PUNCH) (void)
{
	//
	//	Release our lock on the human state
	//
	HumanState.Set_State (HumanStateClass::ANIMATION);
	MoveState.Resume_State ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_ATTACK_STATE_MELEE_PUNCH_Think
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::On_ATTACK_STATE_MELEE_PUNCH_Think (void)
{
	if (Get_Anim_Control ()->Is_Complete ()) {
		
		//
		//	Reset back to general melee state
		//		
		AttackState.Set_State (ATTACK_STATE_MELEE);

	} else {

		float curr_frame = Get_Anim_Control ()->Get_Current_Frame ();

		//
		//	Handy macro
		//
		#define PASSED_FRAME(frame) (LastMeleeAnimFrame < frame && curr_frame >= frame)

		//
		//	Determine what frame we've passed
		//
		bool is_new_attack = true;
		if (PASSED_FRAME (10.0F)) {
			AttackingBoneName = "C R HAND";
		} else if (PASSED_FRAME (13.0F)) {
			AttackingBoneName = "C R HAND";
		} else if (PASSED_FRAME (21.0F)) {
			AttackingBoneName = "C R HAND";
		} else if (PASSED_FRAME (30.0F)) {
			AttackingBoneName = "C L HAND";
		} else if (PASSED_FRAME (42.0F)) {
			AttackingBoneName = "C R HAND";
		} else {
			is_new_attack = false;
		}

		//
		//	Play the "whoosh" sound...  :)
		//
		if (is_new_attack) {
			HasMeleeAttackHit = false;
			Attach_Sound ("Fight Whoosh Sound Twiddler", "ROOTTRANSFORM");
		}

		if (curr_frame >= 13.0F && curr_frame < 45.0F) {

			//
			//	Apply any damage done by the foot that's kicking...
			//
			if (HasMeleeAttackHit == false) {
				HasMeleeAttackHit = Apply_Bone_Collision_Damage (AttackingBoneName);
			}
		}
	}

	//
	//	Cache the last frame number...
	//
	LastMeleeAnimFrame = Get_Anim_Control ()->Get_Current_Frame ();	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(ATTACK_STATE_FLAMETHROWER)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(ATTACK_STATE_FLAMETHROWER) (void)
{
	//
	//	Switch to the flamethrower...
	//
	WeaponBag->Select_Weapon_ID (Get_Definition ().WeaponDefID);
	AttackStateTimer = 0.0F;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(ATTACK_STATE_FLAMETHROWER)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_END(ATTACK_STATE_FLAMETHROWER) (void)
{
	//
	//	Stop firing the flamethrower
	//
	Set_Boolean_Control (ControlClass::BOOLEAN_WEAPON_FIRE_SECONDARY, false);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(ATTACK_STATE_FLAMETHROWER)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(ATTACK_STATE_FLAMETHROWER) (void)
{
	//
	//	Get the player's position relative to Mendoza
	//
	Vector3 relative_position;
	Matrix3D::Inverse_Transform_Vector (Get_Transform (), StarPos, &relative_position);

	bool turn_on_weapon = false;

	//
	//	Is the player within 10 meters of Mendoza?
	//
	if ((MendozaPos - StarPos).Length2 () < 100.0F) {
		
		//
		//	Is the player mostly in front of Mendoza?
		//
		if (relative_position.X > 0 && WWMath::Fabs (relative_position.Y) < 5.0F) {
			turn_on_weapon = true;
		}		
	}

	//
	//	Turn on or off the flamethrower
	//
	Set_Boolean_Control (ControlClass::BOOLEAN_WEAPON_FIRE_SECONDARY, turn_on_weapon);
	return ;
}





///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(ATTACK_STATE_FIREBALL)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(ATTACK_STATE_FIREBALL) (void)
{
	//
	//	Switch to the flamethrower...
	//
	WeaponBag->Select_Weapon_ID (Get_Definition ().WeaponDefID);
	AttackStateTimer	= WWMath::Random_Float (5.0F, 9.0F);
	RandomTargetTimer	= 0;
	ShootGroundPos		= StarPos;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(ATTACK_STATE_FIREBALL)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_END(ATTACK_STATE_FIREBALL) (void)
{
	//
	//	Stop firing the fireball launcher
	//
	Set_Boolean_Control (ControlClass::BOOLEAN_WEAPON_FIRE_PRIMARY,	false);
	Set_Boolean_Control (ControlClass::BOOLEAN_WEAPON_FIRE_SECONDARY, false);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(ATTACK_STATE_FIREBALL)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(ATTACK_STATE_FIREBALL) (void)
{
	//
	//	Target somewhere around the star
	//
	RandomTargetTimer -= TimeManager::Get_Frame_Seconds ();
	if (RandomTargetTimer <= 0) {
		RandomTargetTimer = WWMath::Random_Float (0.25F, 0.6F);
		ShootGroundPos = StarPos;
		ShootGroundPos.X += WWMath::Random_Float (-1.0F, 1.0F);
		ShootGroundPos.Y += WWMath::Random_Float (-1.0F, 1.0F);
		ShootGroundPos.Z += WWMath::Random_Float (0.0F, 2.0F);
	}
	Set_Targeting (ShootGroundPos, true);

	//
	//	Determine if its safe for Mendoza to fire his Fireball launcher
	//
	bool is_safe_to_fire = false;
	Vector3 weapon_hit_pos;
	if (WeaponBag->Get_Weapon ()->Cast_Weapon_Down_Muzzle (weapon_hit_pos)) {
		float hit_distance = (MendozaPos - weapon_hit_pos).Length2 ();
		if (hit_distance > 8.0F) {
			is_safe_to_fire = true;
		}
	}

	//
	//	Get the player's position relative to Mendoza
	//
	Vector3 relative_position;
	Matrix3D::Inverse_Transform_Vector (Get_Transform (), StarPos, &relative_position);	

	//
	//	Is the player mostly in front of Mendoza?
	//
	bool turn_on_weapon = false;
	if ((relative_position.X > 0) && WWMath::Fabs (relative_position.Y) < 7.0F) {
		turn_on_weapon = true;
	}

	//
	//	Turn on or off the fireball launcher (or flamethrower)
	//
	Set_Boolean_Control (ControlClass::BOOLEAN_WEAPON_FIRE_PRIMARY,	turn_on_weapon && is_safe_to_fire);
	Set_Boolean_Control (ControlClass::BOOLEAN_WEAPON_FIRE_SECONDARY,	turn_on_weapon && !is_safe_to_fire);

	//
	//	Check to see if we're finished with this state...
	//
	AttackStateTimer -= TimeManager::Get_Frame_Seconds ();
	if (AttackStateTimer <= 0) {
		OverallState.Set_State (OVERALL_STATE_RANGED_ATTACK);
	}

	return ;
}





///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(ATTACK_STATE_HANDGUN)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(ATTACK_STATE_HANDGUN) (void)
{
	//
	//	Switch to the handgun...
	//
	WeaponBag->Select_Weapon_ID (Get_Definition ().SecondaryWeaponDefID);
	AttackStateTimer = 2.0F;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(ATTACK_STATE_HANDGUN)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_END(ATTACK_STATE_HANDGUN) (void)
{
	//
	//	Stop firing the handgun
	//
	Set_Boolean_Control (ControlClass::BOOLEAN_WEAPON_FIRE_PRIMARY, false);
	WeaponBag->Deselect ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(ATTACK_STATE_HANDGUN)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(ATTACK_STATE_HANDGUN) (void)
{
	bool turn_on_weapon = false;

	//
	//	Get the player's position relative to Mendoza
	//
	Vector3 relative_position;
	Matrix3D::Inverse_Transform_Vector (Get_Transform (), StarPos, &relative_position);

	//
	//	Is the player in front of Mendoza?
	//	
	if (relative_position.X > 0) {
		turn_on_weapon = true;
	}

	//
	//	Turn on or off the handgun
	//
	Set_Boolean_Control (ControlClass::BOOLEAN_WEAPON_FIRE_PRIMARY, turn_on_weapon);

	//
	//	Switch back to the normal attack state if we're finished
	//
	AttackStateTimer -= TimeManager::Get_Frame_Seconds ();
	if (AttackStateTimer <= 0) {

		//
		//	Determine which attack state to revert to
		//
		if (OverallState.Get_State () == OVERALL_STATE_MELEE_ATTACK) {
			AttackState.Set_State (ATTACK_STATE_MELEE);
		} else if (OverallState.Get_State () == OVERALL_STATE_RANGED_ATTACK) {
			AttackState.Set_State (ATTACK_STATE_FLAMETHROWER);
		} else if (OverallState.Get_State () == OVERALL_STATE_FIREBALL_ATTACK) {
			AttackState.Set_State (ATTACK_STATE_FIREBALL);
		}		
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(ATTACK_STATE_SYDNEY)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(ATTACK_STATE_SYDNEY) (void)
{
	//
	//	Switch to the handgun...
	//
	WeaponBag->Select_Weapon_ID (Get_Definition ().WeaponDefID);

	//
	//	Set the timer so the first think call will cause the action...
	//
	AttackStateTimer = 0;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(ATTACK_STATE_SYDNEY)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_END(ATTACK_STATE_SYDNEY) (void)
{
	//
	//	Stop firing...
	//
	Set_Boolean_Control (ControlClass::BOOLEAN_WEAPON_FIRE_SECONDARY, false);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(ATTACK_STATE_SYDNEY)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(ATTACK_STATE_SYDNEY) (void)
{
	AttackStateTimer -= TimeManager::Get_Frame_Seconds ();
	if (AttackStateTimer <= 0) {
		AttackStateTimer = WWMath::Random_Float (0.5F, 1.5F);

		//
		//	Get her position
		//
		Sydney.Get_Ptr ()->Get_Position (&ShootGroundPos);

		const float RADIUS = 2.0F;

		//
		//	Pick a random position in a circle around her feet
		//
		float angle = WWMath::Random_Float (DEG_TO_RADF (0.0F), DEG_TO_RADF (180.0F));
		ShootGroundPos.X += WWMath::Fast_Cos (angle) * RADIUS;
		ShootGroundPos.Y += WWMath::Fast_Sin (angle) * RADIUS;
	}

	//
	//	Target the ground at Sydney's feet
	//
	Set_Targeting (ShootGroundPos, true);

	//
	//	Fire the weapon...
	//
	Set_Boolean_Control (ControlClass::BOOLEAN_WEAPON_FIRE_SECONDARY, true);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Fly_Move
//
///////////////////////////////////////////////////////////////////////////
bool
MendozaBossGameObjClass::Fly_Move (const Vector3 &vector)
{
	//
	//	Get Mendoza's collision box
	//
	AABoxClass collision_box = Peek_Physical_Object ()->As_HumanPhysClass ()->Get_Collision_Box ();
	collision_box.Center += MendozaPos;
	
	//
	//	Check to see if we've hit something
	//
	CastResultStruct result;
	AABoxCollisionTestClass col_test (collision_box, vector, &result, COLLISION_TYPE_PHYSICAL);
	
	//
	//	Find where we hit
	//
	bool retval = COMBAT_STAR->Peek_Model ()->Cast_AABox (col_test);
	if (result.StartBad == false) {
		
		//
		//	Calculate where to move to...
		//
		Vector3 new_pos = MendozaPos + (vector * (result.Fraction * 0.99F));

		//
		//	Move Mendoza
		//
		Set_Position (new_pos);
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Apply_Bone_Collision_Damage
//
///////////////////////////////////////////////////////////////////////////
bool
MendozaBossGameObjClass::Apply_Bone_Collision_Damage (const char *bone_name)
{
	int bone_index = Peek_Model ()->Get_Bone_Index (bone_name);

	//
	//	Lookup the current and last positions of the bone in question
	//
	Matrix3D last_bone_tm;
	Matrix3D curr_bone_tm;
	((Animatable3DObjClass *)Peek_Model ())->Simple_Evaluate_Bone (bone_index, &curr_bone_tm);
	((Animatable3DObjClass *)Peek_Model ())->Simple_Evaluate_Bone (bone_index, LastMeleeAnimFrame, &last_bone_tm);

	//
	//	Calculate where the bone moved
	//
	Vector3 move_vector = curr_bone_tm.Get_Translation () - last_bone_tm.Get_Translation ();

	//
	//	Build a box around the bone that we can use for collision detection
	//
	AABoxClass bone_box;
	bone_box.Center = curr_bone_tm.Get_Translation ();
	bone_box.Extent.Set (0.2F, 0.2F, 0.2F);

	//
	//	Try to find where (if anywhere) we hit the player
	//
	CastResultStruct result;
	result.ComputeContactPoint = true;
	AABoxCollisionTestClass col_test (bone_box, move_vector, &result, COLLISION_TYPE_PROJECTILE);
	
	//
	//	Find where we hit
	//
	bool retval = COMBAT_STAR->Peek_Model ()->Cast_AABox (col_test);
	if (retval) {

		//
		//	Dig the name of the mesh out
		//
		StringClass obj_name;
		if (col_test.CollidedRenderObj != NULL) {
			obj_name = col_test.CollidedRenderObj->Get_Name ();
		}
		
		//
		//	Apply 10 points of "steel" damage to the player
		//
		OffenseObjectClass offense_obj (5.0F, 1);
		COMBAT_STAR->Apply_Damage_Extended (offense_obj, 1.0F, Vector3 (-1, 0, 0), obj_name);

		//
		//	Knock the player back a little bit
		//
		Vector3 delta_vector = move_vector;
		delta_vector.Normalize ();
		COMBAT_STAR->Peek_Physical_Object ()->As_Phys3Class ()->Collide (delta_vector * 0.75F);		

		//
		//	Play the hit sound...
		//
		Attach_Sound ("Fight Impact Sound Twiddler", "ROOTTRANSFORM");
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(OVERALL_STATE_FLYING_SIDEKICK)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(OVERALL_STATE_FLYING_SIDEKICK) (void)
{
	//
	//	Pull the strings to get the melee battle to begin
	//
	MendozaState.Set_State (MENDOZA_STATE_STANDING);
	MoveState.Set_State (MOVE_STATE_STOP);
	AttackState.Set_State (ATTACK_STATE_MELEE_FLYING_SIDEKICK_RUN);
	CameraState.Set_State (CAMERA_STATE_NORMAL);
	SydneyState.Set_State (SYDNEY_STATE_WAITING);
	HeadState.Set_State (HEAD_STATE_LOOKING_AT_STAR);

	//
	//	Get rid of the weapon in his hands
	//
	WeaponBag->Deselect ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(OVERALL_STATE_MELEE_ATTACK)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(OVERALL_STATE_MELEE_ATTACK) (void)
{
	//
	//	Pull the strings to get the melee battle to begin
	//
	MendozaState.Set_State (MENDOZA_STATE_STANDING);
	MoveState.Set_State (MOVE_STATE_GET_CLOSE_TO_PLAYER);
	AttackState.Set_State (ATTACK_STATE_MELEE);
	CameraState.Set_State (CAMERA_STATE_NORMAL);
	SydneyState.Set_State (SYDNEY_STATE_WAITING);
	HeadState.Set_State (HEAD_STATE_LOOKING_AT_STAR);

	//
	//	Get rid of the weapon in his hands
	//
	WeaponBag->Deselect ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(OVERALL_STATE_RANGED_ATTACK)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(OVERALL_STATE_RANGED_ATTACK) (void)
{
	//
	//	Pull the strings to get the ranged battle to begin
	//
	MendozaState.Set_State (MENDOZA_STATE_STANDING);
	MoveState.Set_State (MOVE_STATE_ATTACK_PATTERN1);
	AttackState.Set_State (ATTACK_STATE_FLAMETHROWER);
	CameraState.Set_State (CAMERA_STATE_NORMAL);
	HeadState.Set_State (HEAD_STATE_LOOKING_AT_STAR);

	OverallStateTimer	= 0;

	//
	//	Reset the fireball launcher state timer as necessary.
	// We want to ensure this state happens from time to time,
	// however we don't want it happening all the time.
	//
	if (FireballStartTimer == 0) {
		FireballStartTimer = WWMath::Random_Float (6.0F, 12.0F);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(OVERALL_STATE_RANGED_ATTACK)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(OVERALL_STATE_RANGED_ATTACK) (void)
{
	//
	//	Every half second check to see if Mendoza should try to find a powerup
	// to pickup...
	//
	OverallStateTimer -= TimeManager::Get_Frame_Seconds ();
	if (OverallStateTimer <= 0) {
		OverallStateTimer = 0.5F;
		
		//
		//	Take a random roll of the dice to decide whether or not to try to pickup health...
		//
		float curr_health		= DefenseObject.Get_Health () / DefenseObject.Get_Health_Max ();
		int probability		= int(curr_health * 20.0F);
		if (FreeRandom.Get_Int (probability) == 1) {
			OverallState.Set_State (OVERALL_STATE_FIND_HEALTH);
		}

	} else {

		//
		//	Is it time to change to the "fireball" attack?
		//
		FireballStartTimer -= TimeManager::Get_Frame_Seconds ();
		if (FireballStartTimer <= 0) {
			FireballStartTimer = WWMath::Random_Float (6.0F, 12.0F);

			//
			//	Do the fireball attack
			//
			OverallState.Set_State (OVERALL_STATE_FIREBALL_ATTACK);
		}		
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(OVERALL_STATE_FIREBALL_ATTACK)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(OVERALL_STATE_FIREBALL_ATTACK) (void)
{
	//
	//	Pull the strings to get the ranged battle to begin
	//
	MendozaState.Set_State (MENDOZA_STATE_STANDING);
	MoveState.Set_State (MOVE_STATE_GOTO_HELIPAD);
	AttackState.Set_State (ATTACK_STATE_NONE);
	CameraState.Set_State (CAMERA_STATE_NORMAL);
	HeadState.Set_State (HEAD_STATE_LOOKING_AT_STAR);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(OVERALL_STATE_FIREBALL_ATTACK)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(OVERALL_STATE_FIREBALL_ATTACK) (void)
{
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(OVERALL_STATE_FIND_HEALTH)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(OVERALL_STATE_FIND_HEALTH) (void)
{
	//
	//	Reset the states
	//
	MendozaState.Set_State (MENDOZA_STATE_STANDING);
	AttackState.Set_State (ATTACK_STATE_NONE);
	CameraState.Set_State (CAMERA_STATE_NORMAL);
	MoveState.Set_State (MOVE_STATE_RUN_TO_HEALTH, true);
	HeadState.Set_State (HEAD_STATE_LOOKING_AT_STAR);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(OVERALL_STATE_FIND_HEALTH)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(OVERALL_STATE_FIND_HEALTH) (void)
{
	//
	//	Return to attacking the star if we've found our health
	//
	if (Get_Action ()->Is_Active () == false) {
		OverallState.Set_State (OVERALL_STATE_RANGED_ATTACK);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(OVERALL_STATE_SYDNEY_BOLTS)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(OVERALL_STATE_SYDNEY_BOLTS) (void)
{
	OverallStateTimer = 3.0F;

	//
	//	Stand still and look at Sydney
	//
	MendozaState.Set_State (MENDOZA_STATE_STANDING);
	AttackState.Set_State (ATTACK_STATE_NONE);
	CameraState.Set_State (CAMERA_STATE_NORMAL);
	MoveState.Set_State (MOVE_STATE_STOP);
	SydneyState.Set_State (SYDNEY_STATE_BOLTING);
	HeadState.Set_State (HEAD_STATE_LOOKING_AT_SYDNEY);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(OVERALL_STATE_SYDNEY_BOLTS)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(OVERALL_STATE_SYDNEY_BOLTS) (void)
{
	OverallStateTimer -= TimeManager::Get_Frame_Seconds ();
	if (OverallStateTimer <= 0) {

		//
		//	Start chasing Sydney
		//
		OverallState.Set_State (OVERALL_STATE_RUN_AFTER_SYDNEY);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(OVERALL_STATE_RUN_AFTER_SYDNEY)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(OVERALL_STATE_RUN_AFTER_SYDNEY) (void)
{
	//
	//	Start chasing Sydney
	//
	MendozaState.Set_State (MENDOZA_STATE_STANDING);
	AttackState.Set_State (ATTACK_STATE_NONE);
	CameraState.Set_State (CAMERA_STATE_NORMAL);
	MoveState.Set_State (MOVE_STATE_CHASE_SYDNEY);
	HeadState.Set_State (HEAD_STATE_LOOKING_AT_SYDNEY);

	//
	//	Clue the player in that they need to assist Sydney
	//
	HUDInfo::Set_HUD_Help_Text (TRANSLATE (IDS_MENDOZA_BOSS_HELP_SYDNEY), Vector3 (1.0F, 0.0F, 0.0F));
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(OVERALL_STATE_RUN_AFTER_SYDNEY)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(OVERALL_STATE_RUN_AFTER_SYDNEY) (void)
{
	//
	//	If Mendoza is within 2 meters of his goal, then switch to his "toying" state...
	//
	float dist = (MendozaPos - MENDOZA_END_POS).Length ();
	if (dist < 2.0F) {
		OverallState.Set_State (OVERALL_STATE_TOY_WITH_SYDNEY);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(OVERALL_STATE_TOY_WITH_SYDNEY)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(OVERALL_STATE_TOY_WITH_SYDNEY) (void)
{
	//
	//	Stand still so we can take pot-shots at Sydney
	//
	MendozaState.Set_State (MENDOZA_STATE_STANDING);
	AttackState.Set_State (ATTACK_STATE_SYDNEY);
	CameraState.Set_State (CAMERA_STATE_NORMAL);
	//MoveState.Set_State (MOVE_STATE_STOP);
	HeadState.Set_State (HEAD_STATE_LOOKING_AT_SYDNEY);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(OVERALL_STATE_TOY_WITH_SYDNEY)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(OVERALL_STATE_TOY_WITH_SYDNEY) (void)
{
	if (MoveState.Get_State () != MOVE_STATE_STOP) {
		
		//
		//	If Mendoza has gotten close enough to his goal, then make him stop...
		//
		float dist = (MendozaPos - MENDOZA_END_POS).Length ();
		if (dist < 1.0F) {
			MoveState.Set_State (MOVE_STATE_STOP);
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(OVERALL_STATE_DEATH_SEQUENCE)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(OVERALL_STATE_DEATH_SEQUENCE) (void)
{
	//
	//	Set the defense object to "blamo" so the game object won't die prematurely...
	//
	DefenseObject.Set_Skin ((ArmorType)1);
	DefenseObject.Set_Shield_Type ((ArmorType)1);

	//
	//	Force the states necessary for the death sequence
	//
	MendozaState.Set_State (MENDOZA_STATE_STANDING,		true);
	AttackState.Set_State (ATTACK_STATE_NONE,				true);
	CameraState.Set_State (CAMERA_STATE_FACE_ZOOM,		true);
	MoveState.Set_State (MOVE_STATE_STOP,					true);
	HeadState.Set_State (HEAD_STATE_NONE,					true);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(SYDNEY_STATE_WAITING)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(SYDNEY_STATE_WAITING) (void)
{
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(SYDNEY_STATE_BOLTING)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(SYDNEY_STATE_BOLTING) (void)
{
	//
	//	Make Sydney bolt to her "end" position
	//
	ActionParamsStruct params;
	params.Set_Basic ((int)0, 100, 777);
	params.Set_Movement (SYDNEY_END_POS, 1.0F, 0.5F);
	Sydney.Get_Ptr ()->As_PhysicalGameObj ()->As_SoldierGameObj ()->Get_Action ()->Goto (params);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(SYDNEY_STATE_BOLTING)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(SYDNEY_STATE_BOLTING) (void)
{
	Vector3 syd_pos;
	Sydney.Get_Ptr ()->Get_Position (&syd_pos);

	//
	//	Determine how far away Sydney is from her goal...
	//
	float dist = (SYDNEY_END_POS - syd_pos).Length ();

	//
	//	If Sydney is within a few meters of her goal, then make her trip.  :)
	//
	if (dist <= 5.0F) {
		SydneyState.Set_State (SYDNEY_STATE_TRIPPING);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(SYDNEY_STATE_TRIPPING)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(SYDNEY_STATE_TRIPPING) (void)
{
	SoldierGameObj *sydney = Sydney.Get_Ptr ()->As_PhysicalGameObj ()->As_SoldierGameObj ();

	//
	//	Play the animation of Sydney tripping
	//
	sydney->Set_Blended_Animation ("S_A_HUMAN.H_A_690A", false, 7.0F);
	sydney->Get_Human_State ()->Set_State (HumanStateClass::LOCKED_ANIMATION);
	((HumanAnimControlClass *)sydney->Get_Anim_Control ())->Set_Anim_Speed_Scale (2.0F);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(SYDNEY_STATE_TRIPPING)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(SYDNEY_STATE_TRIPPING) (void)
{
	SoldierGameObj *sydney = Sydney.Get_Ptr ()->As_PhysicalGameObj ()->As_SoldierGameObj ();

	//
	//	If Sydney finished tripping, and Mendoza is standing over her,
	//	then switch her to the cowering state
	//
	if (	sydney->Get_Anim_Control ()->Is_Complete () &&
			OverallState.Get_State () == OVERALL_STATE_TOY_WITH_SYDNEY)
	{
		SydneyState.Set_State (SYDNEY_STATE_GETTING_UP);
		sydney->Get_Human_State ()->Set_State (HumanStateClass::ANIMATION);
		((HumanAnimControlClass *)sydney->Get_Anim_Control ())->Set_Anim_Speed_Scale (1.0F);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(SYDNEY_STATE_GETTING_UP)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(SYDNEY_STATE_GETTING_UP) (void)
{
	SoldierGameObj *sydney = Sydney.Get_Ptr ()->As_PhysicalGameObj ()->As_SoldierGameObj ();

	//
	//	Play the animation of Sydney getting up
	//
	sydney->Set_Blended_Animation ("S_A_HUMAN.H_A_H11C", false);
	sydney->Get_Human_State ()->Set_State (HumanStateClass::LOCKED_ANIMATION);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(SYDNEY_STATE_GETTING_UP)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(SYDNEY_STATE_GETTING_UP) (void)
{
	SoldierGameObj *sydney = Sydney.Get_Ptr ()->As_PhysicalGameObj ()->As_SoldierGameObj ();

	//
	//	If Sydney finished getting up, then switch her to the cowering state
	//
	if (sydney->Get_Anim_Control ()->Is_Complete ()) {		
		sydney->Get_Human_State ()->Set_State (HumanStateClass::ANIMATION);
		SydneyState.Set_State (SYDNEY_STATE_COWERING);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(SYDNEY_STATE_COWERING)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(SYDNEY_STATE_COWERING) (void)
{
	SoldierGameObj *sydney = Sydney.Get_Ptr ()->As_PhysicalGameObj ()->As_SoldierGameObj ();
	sydney->Get_Action ()->Reset (100);

	//
	//	Play the "cowering" animation, but stop at frame 7
	//
	sydney->Set_Blended_Animation ("S_A_HUMAN.H_A_A0A0_L50", false);
	sydney->Get_Human_State ()->Set_State (HumanStateClass::LOCKED_ANIMATION);

	//
	//	Stop the animation at frame 7
	//
	sydney->Get_Anim_Control()->Set_Mode (ANIM_MODE_TARGET, 0);
	sydney->Get_Anim_Control()->Set_Target_Frame (7);

	SydneyStateTimer = 1.0F;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(SYDNEY_STATE_COWERING)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_END(SYDNEY_STATE_COWERING) (void)
{
	SoldierGameObj *sydney = Sydney.Get_Ptr ()->As_PhysicalGameObj ()->As_SoldierGameObj ();

	//
	//	Release our lock on the animation state
	//
	sydney->Get_Human_State ()->Set_State (HumanStateClass::ANIMATION);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(SYDNEY_STATE_COWERING)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(SYDNEY_STATE_COWERING) (void)
{
	SoldierGameObj *sydney = Sydney.Get_Ptr ()->As_PhysicalGameObj ()->As_SoldierGameObj ();

	//
	//	Make Sydney face Mendoza
	//
	sydney->Set_Targeting (MENDOZA_END_POS, false);

	//
	//	Loop a different part of her cowering animation as necessary...
	//
	SydneyStateTimer -= TimeManager::Get_Frame_Seconds ();
	if (SydneyStateTimer <= 0) {
		SydneyStateTimer = WWMath::Random_Float (1.0F, 2.0F);

		//
		//	Toggle the frame range so she keeps flinching
		//
		if (sydney->Get_Anim_Control()->Get_Target_Frame () == 7) {
			sydney->Get_Anim_Control()->Set_Target_Frame (12);
		} else {
			sydney->Get_Anim_Control()->Set_Target_Frame (7);
		}

		//
		//	Apply some damage to Sydney...
		//
		OffenseObjectClass off (2.0F, 1);
		sydney->Apply_Damage (off, 1.0F);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(SYDNEY_STATE_STRIKE_A_POSE)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(SYDNEY_STATE_STRIKE_A_POSE) (void)
{
	SoldierGameObj *sydney = Sydney.Get_Ptr ()->As_PhysicalGameObj ()->As_SoldierGameObj ();

	//
	//	Set the posed animation
	//
	sydney->Set_Animation ("S_B_HUMAN.H_B_A0A0_14", true);

	//
	//	Snap Sydney to the right position
	//
	Matrix3D syd_tm (SYDNEY_END_POS);
	syd_tm.Rotate_Z (DEG_TO_RADF (90.0F));
	sydney->Set_Transform (syd_tm);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(MOVE_STATE_CHASE_SYDNEY)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(MOVE_STATE_CHASE_SYDNEY) (void)
{
	//
	//	Move to our "end" position which should make us look like we're
	// either chasing Sydney or cutting her off...
	//
	ActionParamsStruct params;
	params.Set_Basic ((int)0, 100, 777);
	params.Set_Movement (MENDOZA_END_POS, 0.75F, 0.5F);
	Get_Action ()->Goto (params);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(MOVE_STATE_STOP)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(MOVE_STATE_STOP) (void)
{
	//
	//	Simply stop any action
	//
	Get_Action ()->Reset (100);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(MOVE_STATE_GET_CLOSE_TO_PLAYER)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(MOVE_STATE_GET_CLOSE_TO_PLAYER) (void)
{
	MoveStateTimer = 0.0F;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(MOVE_STATE_GET_CLOSE_TO_PLAYER)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_END(MOVE_STATE_GET_CLOSE_TO_PLAYER) (void)
{
	//
	//	Simply stop any action
	//
	Get_Action ()->Reset (100);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(MOVE_STATE_GET_CLOSE_TO_PLAYER)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(MOVE_STATE_GET_CLOSE_TO_PLAYER) (void)
{
	//
	//	Is it time to track down the player again?
	//
	MoveStateTimer	-= TimeManager::Get_Frame_Seconds ();
	if (MoveStateTimer <= 0) {
		MoveStateTimer = 1.0F;

		//
		//	Track down the player (if possible)
		//
		if (COMBAT_STAR != NULL && (StarPos.X > -70.0F)) {
			ActionParamsStruct params;
			params.Set_Basic ((int)0, 100, 777);
			params.Set_Movement (COMBAT_STAR, 1.0F, 1.0F);
			params.MoveFollow		= true;
			Get_Action ()->Goto (params);
		} else if (COMBAT_STAR != NULL) {

			//
			//	If the player goes out of range, then switch to the fireball attack
			//
			OverallState.Set_State (OVERALL_STATE_FIREBALL_ATTACK);
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(MOVE_STATE_ATTACK_PATTERN1)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(MOVE_STATE_ATTACK_PATTERN1) (void)
{
	MoveStateTimer = 0.0F;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(MOVE_STATE_ATTACK_PATTERN1)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_END(MOVE_STATE_ATTACK_PATTERN1) (void)
{
	//
	//	Simply stop any action
	//
	Get_Action ()->Reset (100);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(MOVE_STATE_ATTACK_PATTERN1)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(MOVE_STATE_ATTACK_PATTERN1) (void)
{
	//
	//	Make sure we are always facing the player...
	//
	Vector3 target_pos = StarPos;
	target_pos.Z += 1.0F;
	Set_Targeting (target_pos, true);

	//
	//	Is it time to move somewhere else?
	//
	MoveStateTimer	-= TimeManager::Get_Frame_Seconds ();
	if (MoveStateTimer <= 0) {
		MoveStateTimer = WWMath::Random_Float (1.0F, 7.0F);

		if (StarPos.X > -70.0F) {
			
			//
			//	Simply find a random spot next to the player to move to...
			//
			Vector3 position;
			if (PathfindClass::Get_Instance ()->Find_Random_Spot (StarPos, 10.0F, &position)) {

				float speed = WWMath::Random_Float (0.25, 1.0F);

				//
				//	Move to this random spot, but use a random speed as well
				//
				ActionParamsStruct params;
				params.Set_Basic ((int)0, 100, 777);
				params.Set_Movement (position, speed, 1.0F);
				params.IgnoreFacing = true;
				Get_Action ()->Goto (params);			
			}
		} else {
			
			//
			//	If the player goes out of range, then switch to the fireball attack
			//
			OverallState.Set_State (OVERALL_STATE_FIREBALL_ATTACK);
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(MOVE_STATE_ATTACK_PATTERN2)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(MOVE_STATE_ATTACK_PATTERN2) (void)
{
	MoveStateTimer = 0.0F;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(MOVE_STATE_ATTACK_PATTERN2)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_END(MOVE_STATE_ATTACK_PATTERN2) (void)
{
	//
	//	Simply stop any action
	//
	Get_Action ()->Reset (100);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(MOVE_STATE_ATTACK_PATTERN2)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(MOVE_STATE_ATTACK_PATTERN2) (void)
{
	//
	//	Is it time to move somewhere else?
	//
	MoveStateTimer	-= TimeManager::Get_Frame_Seconds ();
	if (MoveStateTimer <= 0) {
		MoveStateTimer = WWMath::Random_Float (1.0F, 5.0F);

		if (StarPos.X > -70.0F) {

			//
			//	Simply find a random spot to move to...
			//
			Vector3 position;
			if (PathfindClass::Get_Instance ()->Find_Random_Spot (StarPos, 10.0F, &position)) {

				float speed = WWMath::Random_Float (0.25, 1.0F);

				//
				//	Move to this random spot, but use a random speed as well
				//
				ActionParamsStruct params;
				params.Set_Basic ((int)0, 100, 777);
				params.Set_Movement (position, speed, 1.0F);
				params.IgnoreFacing = true;
				Get_Action ()->Goto (params);
			}

		} else {
			
			//
			//	If the player goes out of range, then switch to the fireball attack
			//
			OverallState.Set_State (OVERALL_STATE_FIREBALL_ATTACK);
		}
	}

	return ;
}




///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(MOVE_STATE_GOTO_HELIPAD)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(MOVE_STATE_GOTO_HELIPAD) (void)
{
	//
	//	Move to the center of the helipad (with all haste)
	//
	ActionParamsStruct params;
	params.Set_Basic ((int)0, 100, 777);
	params.Set_Movement (HELIPAD_CENTER_POS, 1.0F, 0.5F);
	Get_Action ()->Goto (params);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(MOVE_STATE_GOTO_HELIPAD)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_END(MOVE_STATE_GOTO_HELIPAD) (void)
{
	//
	//	Simply stop any action
	//
	Get_Action ()->Reset (100);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(MOVE_STATE_GOTO_HELIPAD)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(MOVE_STATE_GOTO_HELIPAD) (void)
{
	float dist2 = (MendozaPos - HELIPAD_CENTER_POS).Length2 ();

	//
	//	If we've gotten there, then start attacking...
	//
	if (dist2 <= 1.0F) {
		AttackState.Set_State (ATTACK_STATE_FIREBALL, true);
		MoveState.Set_State (MOVE_STATE_STOP);
	}

	return ;
}



///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(MOVE_STATE_RUN_TO_HEALTH)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(MOVE_STATE_RUN_TO_HEALTH) (void)
{
	//
	//	First, find the "best" health powerup in the region...
	//
	PowerUpGameObj *powerup = Find_Best_Powerup ();
	if (powerup != NULL) {

		//
		//	Get the position of the powerup
		//
		Vector3 position;
		powerup->Get_Position (&position);

		//
		//	Now, move to the spot where the powerup is (at all haste)
		//
		ActionParamsStruct params;
		params.Set_Basic ((int)0, 100, 777);
		params.Set_Movement (position, 1.0F, 0.125F);
		Get_Action ()->Goto (params);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(MOVE_STATE_RUN_TO_HEALTH)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_END(MOVE_STATE_RUN_TO_HEALTH) (void)
{
	//
	//	Simply stop any action
	//
	Get_Action ()->Reset (100);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Find_Best_Powerup
//
///////////////////////////////////////////////////////////////////////////
PowerUpGameObj *
MendozaBossGameObjClass::Find_Best_Powerup (void)
{
	if (COMBAT_STAR == NULL) {
		return NULL;
	}

	PowerUpGameObj *best_powerup	= NULL;
	float best_powerup_rating		= 100.0F;

	//
	//	Loop over all the game objects in the world (looking for powerups)
	//
	SLNode<BaseGameObj> *obj_node = NULL;
	for (obj_node = GameObjManager::Get_Game_Obj_List ()->Head (); obj_node; obj_node = obj_node->Next ()) {
		
		//
		//	Is this a powerup?
		//
		PhysicalGameObj *phys_game_obj = obj_node->Data ()->As_PhysicalGameObj ();
		if (phys_game_obj != NULL && phys_game_obj->As_PowerUpGameObj () != NULL) {
			PowerUpGameObj *powerup = phys_game_obj->As_PowerUpGameObj ();
			if (powerup != NULL) {

				//
				//	Get the position of the powerup
				//
				Vector3 powerup_pos;
				powerup->Get_Position (&powerup_pos);
				
				//
				//	Is this powerup in the boss area?
				//
				if (	BOSS_AREA_BOX01.Contains (powerup_pos) || 
						BOSS_AREA_BOX02.Contains (powerup_pos) || 
						BOSS_AREA_BOX03.Contains (powerup_pos))
				{
					powerup_pos.Z = 0;

					//
					//	Calculate how far this powerup is from Mendoza and the player
					//
					float dist_to_me		= (powerup_pos - MendozaPos).Length () / 10.0F;
					float dist_to_star	= (powerup_pos - StarPos).Length () / 10.0F;
					dist_to_star			= WWMath::Clamp (1.0F - dist_to_star, 0.0F, 1.0F);

					//
					//	Is this the powerup that's closest to Mendoza and farthest from the player?
					//
					float rating = (dist_to_me * 0.5F) + (dist_to_star * 0.5F);
					if (rating < best_powerup_rating) {
						best_powerup			= powerup;
						best_powerup_rating	= rating;
					}
				}
			}
		}
	}

	return best_powerup;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(HEAD_STATE_NONE)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_BEGIN(HEAD_STATE_NONE) (void)
{
	Cancel_Look_At ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(HEAD_STATE_LOOKING_AT_STAR)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(HEAD_STATE_LOOKING_AT_STAR) (void)
{
	//
	//	Look at the star's head
	//
	Vector3 position (StarPos);
	position.Z += 1.7F;

	if (Is_Looking () == false) {
		Look_At (position, 100);
	} else {
		Update_Look_At (position);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(HEAD_STATE_LOOKING_AT_SYDNEY)
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::STATE_IMPL_THINK(HEAD_STATE_LOOKING_AT_SYDNEY) (void)
{
	Vector3 syd_pos;
	Sydney.Get_Ptr ()->Get_Position (&syd_pos);

	//
	//	Look at the Sydney's head
	//
	syd_pos.Z += 1.7F;

	if (Is_Looking () == false) {
		Look_At (syd_pos, 100);
	} else {
		Update_Look_At (syd_pos);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Spawn_Health_Powerups
//
///////////////////////////////////////////////////////////////////////////
void
MendozaBossGameObjClass::Spawn_Health_Powerups (void)
{
	const int POWERUP_COUNT = 15;

	//
	//	Create some health powerups
	//
	for (int index = 0; index < POWERUP_COUNT; index ++) {

		//
		//	Pick a random part of the level to create the powerup in...
		//
		const AABoxClass *area_box = NULL;
		int area_id = FreeRandom.Get_Int (3);
		if (area_id == 0) {
			area_box = &BOSS_AREA_BOX01;
		} else if (area_id == 1) {
			area_box = &BOSS_AREA_BOX02;
		} else {
			area_box = &BOSS_AREA_BOX03;
		}

		//
		//	Pick a random spot in this area
		//
		Vector3 rand_pos;
		rand_pos.X = area_box->Center.X + WWMath::Random_Float (-area_box->Extent.X * 0.95F, area_box->Extent.X * 0.95F);
		rand_pos.Y = area_box->Center.Y + WWMath::Random_Float (-area_box->Extent.Y * 0.95F, area_box->Extent.Y * 0.95F);
		rand_pos.Z = area_box->Center.Z;

		//
		//	Ensure this is a valid spot to walk
		//
		if (PathfindClass::Get_Instance ()->Find_Random_Spot (rand_pos, 1.0F, &rand_pos)) {
			
			//
			//	Build a ray-test to determine at what height to position the powerup
			//
			rand_pos.Z			+= 10.0F;
			Vector3 delta		(0, 0, -20.0F);
			Vector3 end_pos	= rand_pos + delta;

			LineSegClass ray_cast (rand_pos, end_pos);
			CastResultStruct result;
			PhysRayCollisionTestClass ray_test (ray_cast, &result, TERRAIN_ONLY_COLLISION_GROUP, COLLISION_TYPE_PHYSICAL);
			COMBAT_SCENE->Cast_Ray (ray_test);
			
			//
			//	Calculate the powerup's new position
			//
			Vector3 powerup_pos = rand_pos + (delta * (result.Fraction * 0.99F));
			
			//
			//	Create and position the object
			//
			PhysicalGameObj *powerup = ObjectLibraryManager::Create_Object ("POW_Health_025");
			if (powerup != NULL) {
				powerup->Start_Observers ();
				powerup->Set_Position (powerup_pos);
			}
		}
	}

	return ;
}

