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
 *                     $Archive:: /Commando/Code/Combat/raveshawbossgameobj.cpp               $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/20/02 4:40p                                               $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "raveshawbossgameobj.h"
#include "wwhack.h"
#include "simpledefinitionfactory.h"
#include "persistfactory.h"
#include "combatchunkid.h"
#include "debug.h"
#include "animcontrol.h"
#include "crandom.h"
#include "weapons.h"
#include "rendobj.h"
#include "wwaudio.h"
#include "audiblesound.h"
#include "explosion.h"
#include "phys.h"
#include "pscene.h"
#include "waypath.h"
#include "waypoint.h"
#include "pathfind.h"
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
#include "stealtheffect.h"
#include "damageablestaticphys.h"
#include "boxrobj.h"


DECLARE_FORCE_LINK (RaveshawBoss)


//////////////////////////////////////////////////////////////////////////
//	ManualTransitionEffectClass
//////////////////////////////////////////////////////////////////////////
class ManualTransitionEffectClass : public TransitionEffectClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	ManualTransitionEffectClass (void)
	{
		RenderBaseMaterial			= true;
		RenderTransitionMaterial	= true;
		IntensityScale					= 0;
		RemoveOnComplete				= false;
		return ;
	}

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////
	
	//
	//	Inherited
	//
	void			Timestep (float dt)
	{
		//
		//	Stolen from base class
		//
		Vector2 uv_rate = MinUVRate + IntensityScale * IntensityScale * (MaxUVRate - MinUVRate);
		UVOffset += uv_rate * dt;
		UVOffset.X = fmod(UVOffset.X , 2.0f);
		UVOffset.Y = fmod(UVOffset.Y , 2.0f);
		return ;
	}

	//
	//	Manual control
	//
	void			Enable_Effect (bool onoff)				{ RenderTransitionMaterial = onoff; }
	bool			Is_Effect_Enabled (void) const		{ return RenderTransitionMaterial; }

	void			Set_Intensity (float value)			{ IntensityScale = value; }
	float			Get_Intensity (void) const				{ return IntensityScale; }

};

//////////////////////////////////////////////////////////////////////////
//	Constants
//////////////////////////////////////////////////////////////////////////
static const float	UNINITIALIZED_TIMER	= -5000.0F;

static const Vector3	TIBERIUM_POS	(-130.499F, 483.243F, -189.617F);
static const float TIBERIUM_RADIUS	= 1.35F;
static const float SAFE_JUMP_RADIUS	= 12.0F;
static const float CAMERA_RADIUS		= 10.0F;

static const float HEAL_TIME			= 3.0F;
static const float EFFECT_FADE_TIME = 12.0F;
static const float EFFECT_INTENSITY	= 0.75F;

static const char *ARC_BONE_NAMES[6] = 
{
	"BONE02",
	"BONE03",
	"BONE04",
	"BONE05",
	"BONE06",
	"BONE07",
};


//////////////////////////////////////////////////////////////////////////
//	Waypath IDs
//////////////////////////////////////////////////////////////////////////
static const int	CATWALK_WAYPATH_ID	= 3000100;


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
	CHUNKID_TIBERIUM_EFFECT,
	CHUNKID_OVERALL_STATE_MACHINE,
	CHUNKID_RAVESHAW_STATE_MACHINE,
	CHUNKID_ENGORGED_STATE_MACHINE,
	CHUNKID_MOVE_STATE_MACHINE,
	CHUNKID_STEALTH_SOLDIER_STATE_MACHINE,
	CHUNKID_THROWN_OBJ_STATE_MACHINE,
	CHUNKID_JUMP_STATE_MACHINE,
	CHUNKID_LIGHTNING_ROD_STATE_MACHINE,
	CHUNKID_STEALTH_SOLDIER,
	CHUNKID_HAVOC_STATE_MACHINE,

	XXX_VARID_STEALTH_SOLIDER_PTR					= 0,
	VARID_THROWN_OBJECT_PTR,
	VARID_IS_TIBERIUM_EFFECT_APPLIED,
	VARID_CURRENT_DEST_POS,
	VARID_OVERALL_STATE_TIMER,
	VARID_ENGORGED_STATE_TIMER,
	VARID_MOVE_STATE_TIMER,
	VARID_BODY_SLAM_TIMER,
	VARID_RAVESHAW_STATE_TIMER,
	VARID_STEALTHSOLDIER_STATE_TIMER,
	VARID_LIGHTNINGROD_STATE_TIMER,
	VARID_START_TIMER,
	VARID_LAST_MELEE_ANIM_FRAME,
	VARID_HAS_MELEE_ATTACK_HIT,
	VARID_FLYING_OBJECT_VECTOR,
	VARID_FLYING_OBJECT_DEST,
	VARID_FLYING_DIST,
	VARID_RELOBJ_TM,
	VARID_ARC_OBJ_PTR,
	VARID_CURR_JUMP_POS,
	VARID_CAMERA_BONE_PTR,
	VARID_CAMERA_BONE_TM,
	VARID_RESTORE_FIRST_PERSON,
};


//////////////////////////////////////////////////////////////////////////
//	Factories
//////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<RaveshawBossGameObjDefClass,	CHUNKID_GAME_OBJECT_DEF_RAVESHAW_BOSS>						_RaveshawBossGameObjDefPersistFactory;
SimplePersistFactoryClass<RaveshawBossGameObjClass,		CHUNKID_GAME_OBJECT_RAVESHAW_BOSS>							_RaveshawBossGameObjPersistFactory;
DECLARE_DEFINITION_FACTORY(RaveshawBossGameObjDefClass,	CLASSID_GAME_OBJECT_DEF_RAVESHAW_BOSS, "Raveshaw Boss")	_RaveshawBossGameObjDefDefFactory;



//////////////////////////////////////////////////////////////////////////
//
//	RaveshawBossGameObjDefClass
//
//////////////////////////////////////////////////////////////////////////
RaveshawBossGameObjDefClass::RaveshawBossGameObjDefClass (void)
{
	UseInnateBehavior = false;

	PARAM_SEPARATOR (RaveshawBossGameObjDefClass, "Boss Settings");
	PARAM_SEPARATOR (RaveshawBossGameObjDefClass, "");
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	~RaveshawBossGameObjDefClass
//
//////////////////////////////////////////////////////////////////////////
RaveshawBossGameObjDefClass::~RaveshawBossGameObjDefClass (void)
{	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Get_Class_ID
//
//////////////////////////////////////////////////////////////////////////
uint32
RaveshawBossGameObjDefClass::Get_Class_ID (void) const	
{ 
	return CLASSID_GAME_OBJECT_DEF_RAVESHAW_BOSS; 
}


//////////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////////
PersistClass *
RaveshawBossGameObjDefClass::Create (void) const
{
	RaveshawBossGameObjClass *obj = new RaveshawBossGameObjClass;
	obj->Init(*this);
	return obj;
}


//////////////////////////////////////////////////////////////////////////
//
//	Save
//
//////////////////////////////////////////////////////////////////////////
bool
RaveshawBossGameObjDefClass::Save (ChunkSaveClass &csave)
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
RaveshawBossGameObjDefClass::Load (ChunkLoadClass &cload)
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
				Debug_Say (("Unrecognized RaveshawBossGameObjDefClass chunkID\n"));
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
RaveshawBossGameObjDefClass::Save_Variables (ChunkSaveClass &csave)
{
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjDefClass::Load_Variables (ChunkLoadClass &cload)
{
	/*while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			default:
				Debug_Say (("Unrecognized RaveshawBossGameObjDefClass Variable chunkID %d\n", cload.Cur_Micro_Chunk_ID ()));
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
RaveshawBossGameObjDefClass::Get_Factory (void) const
{ 
	return _RaveshawBossGameObjDefPersistFactory; 
}


/*
**
**	Start of RaveshawBossGameObjClass
**
*/


///////////////////////////////////////////////////////////////////////////
//
//	RaveshawBossGameObjClass
//
///////////////////////////////////////////////////////////////////////////
RaveshawBossGameObjClass::RaveshawBossGameObjClass (void)	:
	RaveshawTauntTimeLeft (0),
	AvailableTaunts (0xFFFFFF),
	OverallState (this),
	RaveshawState (this),
	EngorgedState (this),
	MoveState (this),
	StealthSoldierState (this),
	HavocState (this),
	ThrownObjectState (this),
	JumpState (this),
	LightningRodState (this),
	OverallStateTimer (0),
	StealthSoldierStateTimer (0),
	LightningRodStateTimer (0),
	RaveshawStateTimer (0),
	EngorgedStateTimer (0),
	MoveStateTimer (0),
	BodySlamTimer (0),
	TiberiumEffect (NULL),
	IsTiberiumEffectApplied (false),
	StealthSoldier (NULL),
	StealthEffect (NULL),
	ThrownObject (NULL),
	RaveshawPos (0, 0, 0),
	StarPos (0, 0, 0),
	StartTimer (0),
	FlyingDist (0),
	FlyingObjectVector (0, 0, 0),
	FlyingObjectDest (0, 0, 0),
	LastMeleeAnimFrame (0),
	HasMeleeAttackHit (false),
	CameraBoneModel (NULL),
	RestoreFirstPerson (true)
{
	Shuffle_Taunt_List ();
	TauntList[0] = 1;//IDS_SAKURA_BOSS_TAUNT1;
	TauntList[1] = 2;//IDS_SAKURA_BOSS_TAUNT2;
	TauntList[2] = 3;//IDS_SAKURA_BOSS_TAUNT3;
	TauntList[3] = 4;//IDS_SAKURA_BOSS_TAUNT4;
	TauntList[4] = 5;//IDS_SAKURA_BOSS_TAUNT5;
	TauntList[5] = 6;//IDS_SAKURA_BOSS_TAUNT6;

	::memset (ArcObjects, 0, sizeof (ArcObjects));
	for (int index = 0; index < ARC_OBJ_COUNT; index ++) {
		ArcLifeRemaining[index] = 0;
	}

	//
	//	Register the Overall states with its state machine
	//	
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, OverallState, OVERALL_STATE_NOTHING);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, OverallState, OVERALL_STATE_HEALING);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, OverallState, OVERALL_STATE_THROWING_OBJECT);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, OverallState, OVERALL_STATE_THROWING_SOLDIER);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, OverallState, OVERALL_STATE_THROWING_STAR);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, OverallState, OVERALL_STATE_GRAB_STAR);	
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, OverallState, OVERALL_STATE_JUMP_TO_CATWALK);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, OverallState, OVERALL_STATE_ON_CATWALK);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, OverallState, OVERALL_STATE_BODYSLAM);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, OverallState, OVERALL_STATE_CHASE_STAR);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, OverallState, OVERALL_STATE_DAZED);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, OverallState, OVERALL_STATE_FLEE);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, OverallState, OVERALL_STATE_DEATH_SEQUENCE);

	//
	//	Register the Raveshaw states with its state machine
	//
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, RaveshawState, RAVESHAW_STATE_NOTHING);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, RaveshawState, RAVESHAW_STATE_ROAR);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, RaveshawState, RAVESHAW_STATE_GRAB_TIBERIUM);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, RaveshawState, RAVESHAW_STATE_GRAB_SOLDIER);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, RaveshawState, RAVESHAW_STATE_THROW_SOLDIER);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, RaveshawState, RAVESHAW_STATE_GRAB_OBJECT);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, RaveshawState, RAVESHAW_STATE_THROW_OBJECT);	
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, RaveshawState, RAVESHAW_STATE_GRAB_STAR);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, RaveshawState, RAVESHAW_STATE_BODYSLAM);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, RaveshawState, RAVESHAW_STATE_JUMP_DOWN);	
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, RaveshawState, RAVESHAW_STATE_STUMBLE);	
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, RaveshawState, RAVESHAW_STATE_LOOK_CONFUSED);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, RaveshawState, RAVESHAW_STATE_DYING);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, RaveshawState, RAVESHAW_STATE_FALL);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, RaveshawState, RAVESHAW_STATE_DEATH_LANDING);	
	
	//
	//	Register the Move states with its state machine
	//
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, MoveState, MOVE_STATE_STOP);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, MoveState, MOVE_STATE_GOTO_TIBERIUM);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, MoveState, MOVE_STATE_GOTO_CATWALK);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, MoveState, MOVE_STATE_JUMP_TO_CATWALK);	
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, MoveState, MOVE_STATE_CIRCLE_CATWALK);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, MoveState, MOVE_STATE_GOTO_THROW_OBJECT);		
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, MoveState, MOVE_STATE_JUMP_TO_STAR);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, MoveState, MOVE_STATE_FOLLOW_STAR);

	//
	//	Register the Engorged states with its state machine
	//
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, EngorgedState, ENGORGED_STATE_NONE);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, EngorgedState, ENGORGED_STATE_ABSORBING_TIBERIUM);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, EngorgedState, ENGORGED_STATE_FADING);

	//
	//	Register the Jump states with its state machine
	//
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, JumpState, JUMP_STATE_NONE);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, JumpState, JUMP_STATE_CROUCHING);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, JumpState, JUMP_STATE_JUMPING);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, JumpState, JUMP_STATE_LANDING);
	JumpState.Set_State (JUMP_STATE_NONE);
	
	//
	//	Register the Stealth Soldier states with its state machine
	//
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, StealthSoldierState, STEALTH_SOLDIER_STATE_NONE);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, StealthSoldierState, STEALTH_SOLDIER_STATE_DISPLAY);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, StealthSoldierState, STEALTH_SOLDIER_STATE_FLYING);	
	StealthSoldierState.Set_State (STEALTH_SOLDIER_STATE_NONE);

	//
	//	Register the Stealth Soldier states with its state machine
	//
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, HavocState, HAVOC_STATE_NONE);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, HavocState, HAVOC_STATE_GRABBED);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, HavocState, HAVOC_STATE_FLYING);	
	HavocState.Set_State (HAVOC_STATE_NONE);	

	//
	//	Register the Thrown object states with its state machine
	//
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, ThrownObjectState, THROWN_OBJECT_STATE_NONE);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, ThrownObjectState, THROWN_OBJECT_STATE_PICKUP);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, ThrownObjectState, THROWN_OBJECT_STATE_FLYING);	
	ThrownObjectState.Set_State (THROWN_OBJECT_STATE_NONE);

	//
	//	Register the Lightning Rod states with its state machine
	//
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, LightningRodState, LIGHTNING_ROD_STATE_NONE);
	ADD_STATE_TO_MACHINE (RaveshawBossGameObjClass, LightningRodState, LIGHTNING_ROD_STATE_ACTIVE);
	LightningRodState.Set_State (LIGHTNING_ROD_STATE_NONE);

	//
	//	Allocate a new stealth effect to use...
	//
	StealthEffect = NEW_REF (StealthEffectClass, ());

	//
	//	Allocate a new tiberium effect to use...
	//
	TiberiumEffect				= NEW_REF (ManualTransitionEffectClass, ());
	TextureClass *texture	= WW3DAssetManager::Get_Instance ()->Get_Texture ("bluetibeffect.tga");
	TiberiumEffect->Set_Texture (texture);
	REF_PTR_RELEASE (texture);

	//
	//	Create the camera bone model...
	//
	CameraBoneModel = WW3DAssetManager::Get_Instance ()->Create_Render_Obj ("CAMBONE");
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	~RaveshawBossGameObjClass
//
///////////////////////////////////////////////////////////////////////////
RaveshawBossGameObjClass::~RaveshawBossGameObjClass (void)
{
	REF_PTR_RELEASE (TiberiumEffect);
	REF_PTR_RELEASE (StealthEffect);
	REF_PTR_RELEASE (CameraBoneModel);

	for (int index = 0; index < ARC_OBJ_COUNT; index ++) {
		ArcObjects[index] = NULL;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
///////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
RaveshawBossGameObjClass::Get_Factory (void) const 
{
	return _RaveshawBossGameObjPersistFactory;
}


////////////////////////////////////////////////////////////////
//
//	Init
//
////////////////////////////////////////////////////////////////
void RaveshawBossGameObjClass::Init (void)
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
RaveshawBossGameObjClass::Init (const RaveshawBossGameObjDefClass &definition)
{
	SoldierGameObj::Init (definition);

	//
	//	Make Raveshaw big and beefy...
	//
	Peek_Model ()->Scale (1.5F);

	//
	//	Make a list of the lightning rods in the area
	//
	Collect_Lightning_Rods ();
	Create_Arc_Effects ();
	Prepare_Arc_Effect_Data ();

	//
	//	Start fresh...
	//
	Get_Action ()->Reset (100);
	StartTimer = 2.0F;	

	//
	//	Determine which state to do...
	//
	Determine_New_Overall_State ();
	Get_Human_State ()->Set_Human_Anim_Override ("Raveshaw Boss Override");
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Definition
//
///////////////////////////////////////////////////////////////////////////
const RaveshawBossGameObjDefClass &
RaveshawBossGameObjClass::Get_Definition (void) const
{
	return (const RaveshawBossGameObjDefClass &)BaseGameObj::Get_Definition ();
}




///////////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////////
bool
RaveshawBossGameObjClass::Save (ChunkSaveClass & csave)
{
	csave.Begin_Chunk (CHUNKID_PARENT);
		SoldierGameObj::Save (csave);
	csave.End_Chunk ();	

	//
	//	Save the tiberium effect to its own chunk
	//
	csave.Begin_Chunk (CHUNKID_STEALTH_SOLDIER);
		StealthSoldier.Save (csave);
	csave.End_Chunk ();
	
	//
	//	Save the tiberium effect to its own chunk
	//
	csave.Begin_Chunk (CHUNKID_TIBERIUM_EFFECT);
		TiberiumEffect->Save (csave);
	csave.End_Chunk ();

	//
	//	Save the state machines to their own chunk
	//
	csave.Begin_Chunk (CHUNKID_OVERALL_STATE_MACHINE);
		OverallState.Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_RAVESHAW_STATE_MACHINE);
		RaveshawState.Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_ENGORGED_STATE_MACHINE);
		EngorgedState.Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_MOVE_STATE_MACHINE);
		MoveState.Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_STEALTH_SOLDIER_STATE_MACHINE);
		StealthSoldierState.Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_HAVOC_STATE_MACHINE);
		HavocState.Save (csave);
	csave.End_Chunk ();	

	csave.Begin_Chunk (CHUNKID_THROWN_OBJ_STATE_MACHINE);
		ThrownObjectState.Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_JUMP_STATE_MACHINE);
		JumpState.Save (csave);		
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_LIGHTNING_ROD_STATE_MACHINE);
		LightningRodState.Save (csave);		
	csave.End_Chunk ();

	//
	//	Save the variables
	//	
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
RaveshawBossGameObjClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		switch(cload.Cur_Chunk_ID ()) {

			case CHUNKID_PARENT:
				SoldierGameObj::Load (cload);
				break;

			case CHUNKID_STEALTH_SOLDIER:
				StealthSoldier.Load (cload);
				break;

			case CHUNKID_TIBERIUM_EFFECT:
				TiberiumEffect->Load (cload);
				break;

			case CHUNKID_OVERALL_STATE_MACHINE:
				OverallState.Load (cload);
				break;

			case CHUNKID_RAVESHAW_STATE_MACHINE:
				RaveshawState.Load (cload);
				break;

			case CHUNKID_ENGORGED_STATE_MACHINE:
				EngorgedState.Load (cload);
				break;

			case CHUNKID_MOVE_STATE_MACHINE:
				MoveState.Load (cload);
				break;

			case CHUNKID_STEALTH_SOLDIER_STATE_MACHINE:
				StealthSoldierState.Load (cload);
				break;

			case CHUNKID_HAVOC_STATE_MACHINE:
				HavocState.Load (cload);
				break;

			case CHUNKID_THROWN_OBJ_STATE_MACHINE:
				ThrownObjectState.Load (cload);
				break;

			case CHUNKID_JUMP_STATE_MACHINE:
				JumpState.Load (cload);		
				break;

			case CHUNKID_LIGHTNING_ROD_STATE_MACHINE:
				LightningRodState.Load (cload);		
				break;

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;

			default:
				Debug_Say (("Unrecognized RaveshawBossGameObjClass chunk ID\n"));
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
RaveshawBossGameObjClass::On_Post_Load (void)
{	
	SoldierGameObj::On_Post_Load ();

	//
	//	Make Raveshaw big and beefy...
	//
	Peek_Model ()->Scale (1.5F);

	//
	//	Make a list of the lightning rods in the area
	//
	Collect_Lightning_Rods ();
	Prepare_Arc_Effect_Data ();

	//
	//	Override the walking animations for this guy
	//
	Get_Human_State ()->Set_Human_Anim_Override ("Raveshaw Boss Override");

	//
	//	Apply the blue tiberium effect as necessary
	//
	if (IsTiberiumEffectApplied) {
		Peek_Physical_Object ()->Add_Effect_To_Me (TiberiumEffect);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Save_Variables
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::Save_Variables (ChunkSaveClass &csave)
{
	//
	//	Save a pointer for each arc-object
	//
	for (int index = 0; index < ARC_OBJ_COUNT; index ++) {		
		WRITE_MICRO_CHUNK (csave, VARID_ARC_OBJ_PTR,	ArcObjects[index]);
	}

	Matrix3D cam_tm = CameraBoneModel->Get_Transform ();
	WRITE_MICRO_CHUNK (csave, VARID_CAMERA_BONE_PTR,					CameraBoneModel);
	WRITE_MICRO_CHUNK (csave, VARID_CAMERA_BONE_TM,						cam_tm);
	WRITE_MICRO_CHUNK (csave, VARID_RESTORE_FIRST_PERSON,				RestoreFirstPerson);	

	WRITE_MICRO_CHUNK (csave, VARID_THROWN_OBJECT_PTR,					ThrownObject);
	WRITE_MICRO_CHUNK (csave, VARID_IS_TIBERIUM_EFFECT_APPLIED,		IsTiberiumEffectApplied);

	WRITE_MICRO_CHUNK (csave, VARID_CURRENT_DEST_POS,					CurrentDestPos);
	WRITE_MICRO_CHUNK (csave, VARID_OVERALL_STATE_TIMER,				OverallStateTimer);
	WRITE_MICRO_CHUNK (csave, VARID_ENGORGED_STATE_TIMER,				EngorgedStateTimer);
	WRITE_MICRO_CHUNK (csave, VARID_MOVE_STATE_TIMER,					MoveStateTimer);
	WRITE_MICRO_CHUNK (csave, VARID_BODY_SLAM_TIMER,					BodySlamTimer);
	WRITE_MICRO_CHUNK (csave, VARID_RAVESHAW_STATE_TIMER,				RaveshawStateTimer);
	WRITE_MICRO_CHUNK (csave, VARID_STEALTHSOLDIER_STATE_TIMER,		StealthSoldierStateTimer);
	WRITE_MICRO_CHUNK (csave, VARID_LIGHTNINGROD_STATE_TIMER,		LightningRodStateTimer);
	WRITE_MICRO_CHUNK (csave, VARID_START_TIMER,							StartTimer);
	WRITE_MICRO_CHUNK (csave, VARID_LAST_MELEE_ANIM_FRAME,			LastMeleeAnimFrame);
	WRITE_MICRO_CHUNK (csave, VARID_HAS_MELEE_ATTACK_HIT,				HasMeleeAttackHit);
	WRITE_MICRO_CHUNK (csave, VARID_FLYING_OBJECT_VECTOR,				FlyingObjectVector);
	WRITE_MICRO_CHUNK (csave, VARID_FLYING_OBJECT_DEST,				FlyingObjectDest);
	WRITE_MICRO_CHUNK (csave, VARID_FLYING_DIST,							FlyingDist);
	WRITE_MICRO_CHUNK (csave, VARID_RELOBJ_TM,							RelObjTM);
	WRITE_MICRO_CHUNK (csave, VARID_CURR_JUMP_POS,						CurrentJumpToPos);	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::Load_Variables (ChunkLoadClass &cload)
{
	RenderObjClass *old_camera_bone_ptr = NULL;
	Matrix3D cam_tm (1);
	int arc_obj_index = 0;

	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			case VARID_ARC_OBJ_PTR:
				LOAD_MICRO_CHUNK (cload, ArcObjects[arc_obj_index++]);
				break;

			READ_MICRO_CHUNK (cload, VARID_CAMERA_BONE_PTR,					old_camera_bone_ptr);
			READ_MICRO_CHUNK (cload, VARID_CAMERA_BONE_TM,					cam_tm);
			READ_MICRO_CHUNK (cload, VARID_RESTORE_FIRST_PERSON,			RestoreFirstPerson);	

			READ_MICRO_CHUNK (cload, VARID_THROWN_OBJECT_PTR,				ThrownObject);
			READ_MICRO_CHUNK (cload, VARID_IS_TIBERIUM_EFFECT_APPLIED,	IsTiberiumEffectApplied);

			READ_MICRO_CHUNK (cload, VARID_CURRENT_DEST_POS,				CurrentDestPos);
			READ_MICRO_CHUNK (cload, VARID_OVERALL_STATE_TIMER,			OverallStateTimer);
			READ_MICRO_CHUNK (cload, VARID_ENGORGED_STATE_TIMER,			EngorgedStateTimer);
			READ_MICRO_CHUNK (cload, VARID_MOVE_STATE_TIMER,				MoveStateTimer);
			READ_MICRO_CHUNK (cload, VARID_BODY_SLAM_TIMER,					BodySlamTimer);
			READ_MICRO_CHUNK (cload, VARID_RAVESHAW_STATE_TIMER,			RaveshawStateTimer);
			READ_MICRO_CHUNK (cload, VARID_STEALTHSOLDIER_STATE_TIMER,	StealthSoldierStateTimer);
			READ_MICRO_CHUNK (cload, VARID_LIGHTNINGROD_STATE_TIMER,		LightningRodStateTimer);
			READ_MICRO_CHUNK (cload, VARID_START_TIMER,						StartTimer);
			READ_MICRO_CHUNK (cload, VARID_LAST_MELEE_ANIM_FRAME,			LastMeleeAnimFrame);
			READ_MICRO_CHUNK (cload, VARID_HAS_MELEE_ATTACK_HIT,			HasMeleeAttackHit);
			READ_MICRO_CHUNK (cload, VARID_FLYING_OBJECT_VECTOR,			FlyingObjectVector);
			READ_MICRO_CHUNK (cload, VARID_FLYING_OBJECT_DEST,				FlyingObjectDest);
			READ_MICRO_CHUNK (cload, VARID_FLYING_DIST,						FlyingDist);
			READ_MICRO_CHUNK (cload, VARID_RELOBJ_TM,							RelObjTM);
			READ_MICRO_CHUNK (cload, VARID_CURR_JUMP_POS,					CurrentJumpToPos);	

			default:
				Debug_Say (("Unrecognized RaveshawBossGameObjClass Variable chunkID %d\n", cload.Cur_Micro_Chunk_ID ()));
				break;
		}

		cload.Close_Micro_Chunk ();
	}

	//
	//	Remap the arc-object pointers
	//
	for (int index = 0; index < arc_obj_index; index ++) {
		REQUEST_POINTER_REMAP ((void **)&ArcObjects[index]);
	}

	if (ThrownObject != NULL) {
		REQUEST_POINTER_REMAP ((void **)&ThrownObject);
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
RaveshawBossGameObjClass::Apply_Control (void)
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
RaveshawBossGameObjClass::Think (void)
{	
	WWPROFILE ("Raveshaw Think");	

	bool ok_to_think = true;

//
//	Don't do this in the editor
//
#ifdef PARAM_EDITING_ON
	ok_to_think = false;
#endif

	if (COMBAT_STAR == NULL || COMBAT_STAR->Is_Dead () || COMBAT_STAR->Is_Destroyed ()) {
		ok_to_think = false;
	}

	if (ok_to_think) {

		Verify_Stealth_Soldier ();

		//
		//	Get the position of both Raveshaw and the star (this information
		// is used throughout the AI)
		//
		Get_Position (&RaveshawPos);
		COMBAT_STAR->Get_Position (&StarPos);

		//
		//	Check to see if its OK to kick off the boss logic
		//
		if (StartTimer > UNINITIALIZED_TIMER) {
			StartTimer -= TimeManager::Get_Frame_Seconds ();
			if (StartTimer <= 0) {
				StartTimer = UNINITIALIZED_TIMER;
				OverallState.Set_State (OVERALL_STATE_CHASE_STAR);
			}
		} else {

			//
			//	Let the state machines think
			//
			OverallState.Think ();
			RaveshawState.Think ();
			MoveState.Think ();
			EngorgedState.Think ();
			StealthSoldierState.Think ();
			HavocState.Think ();
			ThrownObjectState.Think ();
			JumpState.Think ();
			LightningRodState.Think ();
		}
	}

	//
	//	Give Raveshaw 100 extra health points so he doesn't get "killed" due to any
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
	DefenseObject.Set_Health (max (curr_health, 1.0F));
	DefenseObject.Set_Health_Max (old_health_max);	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Verify_Stealth_Soldier
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::Verify_Stealth_Soldier (void)
{
	if (StealthSoldierState.Get_State () == STEALTH_SOLDIER_STATE_NONE) {
		return ;
	}

	//
	//	Check to see if the stealth soldier has been destroyed...
	//
	SoldierGameObj *stealth_soldier = Peek_Stealth_Soldier ();
	if (	stealth_soldier != NULL &&
			(stealth_soldier->Is_Dead () || stealth_soldier->Is_Destroyed ()))
	{
		//
		//	Get rid of the stealth soldier
		//
		stealth_soldier->Peek_Physical_Object ()->Remove_Effect_From_Me (StealthEffect);
		stealth_soldier->Get_Human_State ()->Stop_Scripted_Animation ();
		stealth_soldier->Set_Delete_Pending ();
		StealthSoldier = NULL;		

		OverallState.Set_State (OVERALL_STATE_NOTHING);
		RaveshawState.Set_State (RAVESHAW_STATE_ROAR);
		MoveState.Set_State (MOVE_STATE_STOP);
		StealthSoldierState.Set_State (STEALTH_SOLDIER_STATE_NONE);
		Determine_New_Overall_State ();
		return ;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Shuffle_Taunt_List
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::Shuffle_Taunt_List (void)
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
RaveshawBossGameObjClass::Apply_Damage_Extended
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
	//	Apply the damage
	//
	DefenseObject.Set_Can_Object_Die (false);
	SoldierGameObj::Apply_Damage_Extended (damager, scale, direction, collision_box_name);

	//
	//	Get the boss's current health
	//
	float curr_health = DefenseObject.Get_Health ();

	if (FreeRandom.Get_Int (7) == 1) {
		//
		//	Play a hurt sound
		//
		WWAudioClass::Get_Instance ()->Create_Instant_Sound ("Rav_Hurt_Twidder", Get_Transform ());
	}

	//
	//	If he's circling the catwalk, and badly damaged, then
	// proceed to his death
	//
	if (MoveState.Get_State () == MOVE_STATE_CIRCLE_CATWALK && curr_health <= 20.0F) {
		if (FreeRandom.Get_Int (5) == 1)	{
			OverallState.Set_State (OVERALL_STATE_DEATH_SEQUENCE);
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Attach_Sound
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::Attach_Sound (const char *sound_name, const char *bone_name)
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
//	STATE_IMPL_BEGIN(OVERALL_STATE_HEALING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(OVERALL_STATE_HEALING) (void)
{
	//
	//	Pull the strings to get the mutant to run to the blue tiberium
	//
	RaveshawState.Set_State (RAVESHAW_STATE_NOTHING);
	MoveState.Set_State (MOVE_STATE_GOTO_TIBERIUM);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(OVERALL_STATE_THROWING_OBJECT)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(OVERALL_STATE_THROWING_OBJECT) (void)
{
	ThrownObject = Find_Object_To_Throw ();
	if (ThrownObject != NULL) {

		//
		//	Pull the strings to get the mutant to grab a stealth soldier...
		//
		RaveshawState.Set_State (RAVESHAW_STATE_NOTHING);
		MoveState.Set_State (MOVE_STATE_GOTO_THROW_OBJECT);
	} else {
		OverallState.Set_State (OVERALL_STATE_CHASE_STAR);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(OVERALL_STATE_THROWING_SOLDIER)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(OVERALL_STATE_THROWING_SOLDIER) (void)
{
	//
	//	Pull the strings to get the mutant to grab a stealth soldier...
	//
	RaveshawState.Set_State (RAVESHAW_STATE_GRAB_SOLDIER);
	MoveState.Set_State (MOVE_STATE_STOP);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(OVERALL_STATE_GRAB_STAR)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(OVERALL_STATE_GRAB_STAR) (void)
{
	//
	//	Pull the strings to get the mutant to grab the player
	//
	RaveshawState.Set_State (RAVESHAW_STATE_GRAB_STAR);
	MoveState.Set_State (MOVE_STATE_STOP);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(OVERALL_STATE_CHASE_STAR)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(OVERALL_STATE_CHASE_STAR) (void)
{
	//
	//	Pull the strings to get the mutant to run to the blue tiberium
	//
	RaveshawState.Set_State (RAVESHAW_STATE_NOTHING);
	MoveState.Set_State (MOVE_STATE_FOLLOW_STAR);

	//
	//	Chase the player for a random amount of time
	//
	OverallStateTimer = WWMath::Random_Float (6.0F, 20.0F);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(OVERALL_STATE_CHASE_STAR)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(OVERALL_STATE_CHASE_STAR) (void)
{
	const float ARMS_REACH	= 3.8F;

	//
	//	Check to see if Raveshaw is close enough to the player to do some damage.
	//
	float dist2 = (RaveshawPos - StarPos).Length2 ();
	if (dist2 <= ARMS_REACH && JumpState.Get_State () == JUMP_STATE_NONE) {
		
		//
		//	Grab for the star
		//
		OverallState.Set_State (OVERALL_STATE_GRAB_STAR);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(OVERALL_STATE_JUMP_TO_CATWALK)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(OVERALL_STATE_JUMP_TO_CATWALK) (void)
{
	//
	//	Pull the strings to get the mutant to run to a spot where he
	// can jump to the catwalk
	//
	RaveshawState.Set_State (RAVESHAW_STATE_NOTHING);
	MoveState.Set_State (MOVE_STATE_GOTO_CATWALK);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(OVERALL_STATE_ON_CATWALK)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(OVERALL_STATE_ON_CATWALK) (void)
{
	//
	//	Get the mutant to run around the catwalk for a bit...
	//
	RaveshawState.Set_State (RAVESHAW_STATE_NOTHING);
	MoveState.Set_State (MOVE_STATE_CIRCLE_CATWALK);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(OVERALL_STATE_BODYSLAM)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(OVERALL_STATE_BODYSLAM) (void)
{
	//
	//	Get Raveshaw to do his bodyslam
	//
	RaveshawState.Set_State (RAVESHAW_STATE_BODYSLAM);
	MoveState.Set_State (MOVE_STATE_STOP);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(OVERALL_STATE_DEATH_SEQUENCE)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(OVERALL_STATE_DEATH_SEQUENCE) (void)
{
	RaveshawState.Set_State (RAVESHAW_STATE_DYING);
	MoveState.Set_State (MOVE_STATE_STOP);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(MOVE_STATE_STOP)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(MOVE_STATE_STOP) (void)
{
	Get_Action ()->Reset (100);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(MOVE_STATE_GOTO_TIBERIUM)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(MOVE_STATE_GOTO_TIBERIUM) (void)
{
	//
	//	Find the "closest" position around the tiberium for Raveshaw to run to
	//
	Vector3 dir_vector = (RaveshawPos - TIBERIUM_POS);
	dir_vector.Normalize ();
	CurrentDestPos = TIBERIUM_POS + (TIBERIUM_RADIUS * dir_vector);

	//
	//	Make the mutant move to that position
	//
	ActionParamsStruct params;
	params.Set_Basic ((long)0, 100, 777);
	params.Set_Movement (CurrentDestPos, 0.6F, 0.5F);
	Get_Action ()->Goto (params);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(MOVE_STATE_GOTO_TIBERIUM)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(MOVE_STATE_GOTO_TIBERIUM) (void)
{
	//
	//	Put Raveshaw in the "healing" state
	//
	if (Get_Action ()->Is_Active () == false) {
		RaveshawState.Set_State (RAVESHAW_STATE_GRAB_TIBERIUM);
		MoveState.Set_State (MOVE_STATE_STOP);		
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(MOVE_STATE_GOTO_CATWALK)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(MOVE_STATE_GOTO_CATWALK) (void)
{
	//const float MIN_JUMP_DIST	= 4.0F;
	//const float MAX_JUMP_DIST	= 10.0F;

	//
	//	Find the closest catwlk position to our current position
	//
	Find_Closest_Catwalk_Pos (RaveshawPos, &CurrentJumpToPos);

	//
	//	Detemine what the "most" aligned ground position would be...
	//
	Vector3 dir_vector	= (CurrentJumpToPos - TIBERIUM_POS);
	dir_vector.Normalize ();
	CurrentDestPos.X = CurrentJumpToPos.X + (dir_vector.X * 8.0F);
	CurrentDestPos.Y = CurrentJumpToPos.Y + (dir_vector.Y * 8.0F);
	CurrentDestPos.Z = TIBERIUM_POS.Z;

	//
	//	Make the mutant move into position
	//
	ActionParamsStruct params;
	params.Set_Basic ((long)0, 100, 777);
	params.Set_Movement (CurrentDestPos, 0.6F, 0.5F);
	Get_Action ()->Goto (params);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Find_Closest_Catwalk_Pos
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::Find_Closest_Catwalk_Pos (const Vector3 &curr_pos, Vector3 *catwalk_pos)
{
	float best_dist2 = 99999.0F;

	//
	//	Get the catwalk waypath
	//
	WaypathClass *waypath = PathfindClass::Get_Instance ()->Find_Waypath (CATWALK_WAYPATH_ID);
	WWASSERT (waypath != NULL);

	//
	//	Now find the closest point which is in-between the waypath points
	//
	for (int index = 0; index < waypath->Get_Point_Count (); index ++) {
		
		//
		//	Determine what the current and next waypath points are
		//
		int curr_index = index;
		int next_index = index + 1;
		if (index + 1 >= waypath->Get_Point_Count ()) {
			next_index = 0;
		}

		//
		//	Get the position of the current and next waypoint
		//
		Vector3 curr_waypath_pos = waypath->Get_Point (curr_index)->Get_Position ();
		Vector3 next_waypath_pos = waypath->Get_Point (next_index)->Get_Position ();
		
		//
		//	Evaluate this jump to position (is it the closest?)
		//
		Vector3 jump_to_pos	= curr_waypath_pos + ((next_waypath_pos - curr_waypath_pos) * 0.5F);
		Vector3 dir_vector	= (curr_pos - jump_to_pos);
		dir_vector.Z			= 0;
		float dist2				= dir_vector.Length2 ();
		if (dist2 < best_dist2) {
			best_dist2			= dist2;
			(*catwalk_pos)		= jump_to_pos;
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(MOVE_STATE_GOTO_CATWALK)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(MOVE_STATE_GOTO_CATWALK) (void)
{
	//
	//	Face the tiberium as necessary
	//
	float dist2 = (RaveshawPos - CurrentDestPos).Length2 ();
	if (dist2 < 4.0F) {
		Get_Action ()->Get_Parameters ().IgnoreFacing = true;
		Set_Targeting (TIBERIUM_POS);
	}

	//
	//	Put Raveshaw in the jump to position stage
	//
	if (Get_Action ()->Is_Active () == false) {
		RaveshawState.Set_State (RAVESHAW_STATE_NOTHING);
		MoveState.Set_State (MOVE_STATE_JUMP_TO_CATWALK);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(MOVE_STATE_JUMP_TO_CATWALK)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(MOVE_STATE_JUMP_TO_CATWALK) (void)
{	
	//
	//	Simply cause the physics to jump to the specified point
	//
	Jump_To_Point (CurrentJumpToPos);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(MOVE_STATE_JUMP_TO_CATWALK)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(MOVE_STATE_JUMP_TO_CATWALK) (void)
{	
	//
	//	The state ends when the mutant lands
	//
	if (JumpState.Get_State () == JUMP_STATE_NONE) {
		OverallState.Set_State (OVERALL_STATE_ON_CATWALK);
	}

	Set_Targeting (TIBERIUM_POS);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(MOVE_STATE_CIRCLE_CATWALK)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(MOVE_STATE_CIRCLE_CATWALK) (void)
{
	MoveStateTimer = 0.0F;
	BodySlamTimer	= WWMath::Random_Float (1.0F, 5.0F);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(MOVE_STATE_CIRCLE_CATWALK)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(MOVE_STATE_CIRCLE_CATWALK) (void)
{
	//
	//	Always try to face the star
	//
	Set_Targeting (StarPos);

	//
	//	Calculate Raveshaw's remaining health
	//
	float health				= DefenseObject.Get_Health ();
	float health_percent		= health / DefenseObject.Get_Health_Max ();

	//
	//	Is it time to bodyslam?
	//
	BodySlamTimer -= TimeManager::Get_Frame_Seconds ();
	if (health_percent > 0.05F && BodySlamTimer <= 0) {
		OverallState.Set_State (OVERALL_STATE_BODYSLAM);
	} else {

		//
		//	Is it time to choose a new spot on the catwalk?
		//
		MoveStateTimer -= TimeManager::Get_Frame_Seconds ();
		if (MoveStateTimer <= 0) {
			MoveStateTimer = 0.5F;

			//
			//	Find the closest catwalk position to our current position
			//
			Find_Closest_Catwalk_Pos (StarPos, &CurrentDestPos);

			//
			//	Now, simply make the mutant move to this position
			//
			ActionParamsStruct params;
			params.Set_Basic ((long)0, 100, 777);
			params.Set_Movement (CurrentDestPos, 0.5F, 0.5F);
			params.IgnoreFacing = true;
			Get_Action ()->Goto (params);		
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(MOVE_STATE_GOTO_THROW_OBJECT)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(MOVE_STATE_GOTO_THROW_OBJECT) (void)
{
	//
	//	Walk to the position of the throw object
	//
	ActionParamsStruct params;
	params.Set_Basic ((long)0, 100, 777);
	params.Set_Movement (ThrownObject, 0.5F, 1.25F);
	Get_Action ()->Goto (params);		
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(MOVE_STATE_GOTO_THROW_OBJECT)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(MOVE_STATE_GOTO_THROW_OBJECT) (void)
{
	Vector3 obj_pos;
	ThrownObject->Get_Position (&obj_pos);
	if ((RaveshawPos - obj_pos).Length2 () < 3.0F || Get_Action ()->Is_Active () == false) {
	//}

	//
	//	Stop and pick up the object once we get there
	//
	//if (Get_Action ()->Is_Active () == false) {
		MoveState.Set_State (MOVE_STATE_STOP);
		RaveshawState.Set_State (RAVESHAW_STATE_GRAB_OBJECT);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(MOVE_STATE_JUMP_TO_STAR)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(MOVE_STATE_JUMP_TO_STAR) (void)
{
	//
	//	Stop any current action and jump to the star's position
	//
	Get_Action ()->Reset (100);	
	Jump_To_Point (StarPos);
	//Peek_Human_Phys ()->Jump_To_Point (StarPos);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(MOVE_STATE_JUMP_TO_STAR)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(MOVE_STATE_JUMP_TO_STAR) (void)
{
	//
	//	Face the star
	//
	Set_Targeting (StarPos);

	//
	//	The state ends when the mutant lands
	//
	if (JumpState.Get_State () == JUMP_STATE_NONE) {
		MoveState.Set_State (MOVE_STATE_FOLLOW_STAR);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(MOVE_STATE_FOLLOW_STAR)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(MOVE_STATE_FOLLOW_STAR) (void)
{
	MoveStateTimer = 0.0F;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(MOVE_STATE_FOLLOW_STAR)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_END(MOVE_STATE_FOLLOW_STAR) (void)
{
	//
	//	Simply stop any action
	//
	Get_Action ()->Reset (100);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(MOVE_STATE_FOLLOW_STAR)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(MOVE_STATE_FOLLOW_STAR) (void)
{
	//
	//	Is it time to track down the player again?
	//
	MoveStateTimer	-= TimeManager::Get_Frame_Seconds ();
	if (MoveStateTimer <= 0) {
		MoveStateTimer = 0.75F;

		//
		//	Should we jump to the player's position?
		//
		float dist2 = (RaveshawPos - StarPos).Length2 ();
		if ((dist2 > 64.0F) && FreeRandom.Get_Int (5) == 1) {
			MoveState.Set_State (MOVE_STATE_JUMP_TO_STAR);
		} else {

			//
			//	Track down the player
			//
			ActionParamsStruct params;
			params.Set_Basic ((long)0, 100, 777);
			params.Set_Movement (COMBAT_STAR, 1.0F, 1.0F);
			params.MoveFollow		= true;
			Get_Action ()->Goto (params);
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(RAVESHAW_STATE_NOTHING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(RAVESHAW_STATE_NOTHING) (void)
{
	//
	//	Choose a random time to do something a little different (i.e. roar, etc)
	//
	RaveshawStateTimer = WWMath::Random_Float (5.0F, 15.0F);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(RAVESHAW_STATE_NOTHING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(RAVESHAW_STATE_NOTHING) (void)
{
	RaveshawStateTimer -= TimeManager::Get_Frame_Seconds ();
	if (RaveshawStateTimer <= 0) {

		//
		//	Just throw your head back and roar
		//
		RaveshawState.Set_State (RAVESHAW_STATE_ROAR);
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(RAVESHAW_STATE_ROAR)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(RAVESHAW_STATE_ROAR) (void)
{
	//
	//	Play the roar animation and sound
	//
	Set_Blended_Animation ("S_C_HUMAN.H_C_A0A0_L07", false);
	Attach_Sound ("Rav_Long_Yell_Twiddler", "C HEAD");	

	//
	//	Stop whatever action he's doing (temporarily)
	//
	Get_Action ()->Pause (true);
	JumpState.Halt_State ();
	MoveState.Halt_State ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(RAVESHAW_STATE_ROAR)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_END(RAVESHAW_STATE_ROAR) (void)
{
	//
	//	Release our lock on the animation state
	//
	Get_Human_State ()->Stop_Scripted_Animation ();

	//
	//	Resume the action
	//
	MoveState.Resume_State ();
	JumpState.Resume_State ();
	Get_Action ()->Pause (false);	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(RAVESHAW_STATE_ROAR)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(RAVESHAW_STATE_ROAR) (void)
{
	//
	//	Reset the state when complete
	//
	if (Get_Anim_Control ()->Is_Complete ()) {
		RaveshawState.Set_State (RAVESHAW_STATE_NOTHING);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(RAVESHAW_STATE_GRAB_TIBERIUM)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(RAVESHAW_STATE_GRAB_TIBERIUM) (void)
{
	//
	//	Play the grab the tiberium animation
	//
	Set_Blended_Animation ("S_A_HUMAN.RAV_HEAL", false);
	Get_Human_State ()->Set_State (HumanStateClass::LOCKED_ANIMATION);

	//
	//	Stop the animation at frame 34
	//
	Get_Anim_Control ()->Set_Mode (ANIM_MODE_TARGET, 0);
	Get_Anim_Control ()->Set_Target_Frame (34);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(RAVESHAW_STATE_GRAB_TIBERIUM)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_END(RAVESHAW_STATE_GRAB_TIBERIUM) (void)
{
	//
	//	Release our lock on the animation state
	//
	Get_Human_State ()->Stop_Scripted_Animation ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(RAVESHAW_STATE_GRAB_TIBERIUM)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(RAVESHAW_STATE_GRAB_TIBERIUM) (void)
{
	//
	//	Face the tiberium
	//
	Set_Targeting (TIBERIUM_POS);	

	//
	//	Handy macro
	//
	float curr_frame = Get_Anim_Control ()->Get_Current_Frame ();
	#define PASSED_FRAME(frame) (LastMeleeAnimFrame < frame && curr_frame >= frame)
	if (PASSED_FRAME (9.0F)) {
		
		//
		//	Start healing
		//
		EngorgedState.Set_State (ENGORGED_STATE_ABSORBING_TIBERIUM);
	}

	//
	//	Loop the animation between frames 12 and 34
	//
	if (Get_Anim_Control ()->Is_Complete ()) {
		Get_Anim_Control ()->Set_Mode (ANIM_MODE_TARGET, 12);
		Get_Anim_Control ()->Set_Target_Frame (34);
	}

	//
	//	Cache the last frame number...
	//
	LastMeleeAnimFrame = Get_Anim_Control ()->Get_Current_Frame ();
	return ;
}



///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(RAVESHAW_STATE_GRAB_SOLDIER)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(RAVESHAW_STATE_GRAB_SOLDIER) (void)
{
	//
	//	Play the grab the soldier animation
	//
	Set_Blended_Animation ("S_A_HUMAN.RAV_GRABTHROW", false);
	Get_Human_State ()->Set_State (HumanStateClass::LOCKED_ANIMATION);

	//
	//	Stop the animation at frame 27
	//
	Get_Anim_Control ()->Set_Mode (ANIM_MODE_TARGET, 0);
	Get_Anim_Control ()->Set_Target_Frame (27);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(RAVESHAW_STATE_GRAB_SOLDIER)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_END(RAVESHAW_STATE_GRAB_SOLDIER) (void)
{
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(RAVESHAW_STATE_GRAB_SOLDIER)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(RAVESHAW_STATE_GRAB_SOLDIER) (void)
{
	//
	//	Is it time to show the soldier?
	//
	if (	StealthSoldierState.Get_State () == STEALTH_SOLDIER_STATE_NONE &&
			Get_Anim_Control ()->Is_Complete ())
	{
		StealthSoldierState.Set_State (STEALTH_SOLDIER_STATE_DISPLAY);
	}

	//
	//	Make sure we face the star...
	//
	if (StealthSoldierState.Get_State () != STEALTH_SOLDIER_STATE_NONE) {
		Set_Targeting (StarPos);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(RAVESHAW_STATE_THROW_SOLDIER)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(RAVESHAW_STATE_THROW_SOLDIER) (void)
{
	//
	//	Play the throw the soldier animation
	//
	/*Set_Blended_Animation ("S_A_HUMAN.GRABTHROW", false);
	Get_Human_State ()->Set_State (HumanStateClass::LOCKED_ANIMATION);

	//
	//	Start the animation at frame 27
	//
	Get_Anim_Control ()->Set_Mode (ANIM_MODE_ONCE, 27);*/
	
	//
	//	Advance to frame 49
	//
	Get_Anim_Control ()->Set_Target_Frame (49);

	//
	//	Play an exertion sound
	//
	WWAudioClass::Get_Instance ()->Create_Instant_Sound ("Rav_Exert_01", Get_Transform ());
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(RAVESHAW_STATE_THROW_SOLDIER)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_END(RAVESHAW_STATE_THROW_SOLDIER) (void)
{
	//
	//	Release our lock on the animation state
	//
	Get_Human_State ()->Stop_Scripted_Animation ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(RAVESHAW_STATE_THROW_SOLDIER)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(RAVESHAW_STATE_THROW_SOLDIER) (void)
{
	//
	//	Handy macro
	//
	float curr_frame = Get_Anim_Control ()->Get_Current_Frame ();
	#define PASSED_FRAME(frame) (LastMeleeAnimFrame < frame && curr_frame >= frame)
	if (PASSED_FRAME (36.0F)) {

		//
		//	Let the soldier fly!
		//
		StealthSoldierState.Set_State (STEALTH_SOLDIER_STATE_FLYING);
	}

	//
	//	Decide on what to do when finished
	//
	if (Get_Anim_Control ()->Is_Complete ()) {		
		Determine_New_Overall_State ();
	}

	//
	//	Cache the last frame number...
	//
	LastMeleeAnimFrame = Get_Anim_Control ()->Get_Current_Frame ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(RAVESHAW_STATE_GRAB_OBJECT)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(RAVESHAW_STATE_GRAB_OBJECT) (void)
{
	//
	//	Play the pickup the object animation
	//
	Set_Blended_Animation ("S_A_HUMAN.RAV_THROW", false);
	Get_Human_State ()->Set_State (HumanStateClass::LOCKED_ANIMATION);

	//
	//	Stop the animation at frame 22
	//
	Get_Anim_Control ()->Set_Mode (ANIM_MODE_TARGET, 0);
	Get_Anim_Control ()->Set_Target_Frame (22);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(RAVESHAW_STATE_GRAB_OBJECT)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(RAVESHAW_STATE_GRAB_OBJECT) (void)
{
	//
	//	Throw the object
	//
	if (Get_Anim_Control ()->Is_Complete ()) {
		RaveshawState.Set_State (RAVESHAW_STATE_THROW_OBJECT);
	}

	//
	//	Handy macro
	//
	float curr_frame = Get_Anim_Control ()->Get_Current_Frame ();
	#define PASSED_FRAME(frame) (LastMeleeAnimFrame < frame && curr_frame >= frame)
	if (PASSED_FRAME (9.0F)) {
	
		//
		//	Pickup the object
		//
		ThrownObjectState.Set_State (THROWN_OBJECT_STATE_PICKUP);
	}

	//
	//	Cache the last frame number...
	//
	LastMeleeAnimFrame = Get_Anim_Control ()->Get_Current_Frame ();
	return ;
}



///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(RAVESHAW_STATE_THROW_OBJECT)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(RAVESHAW_STATE_THROW_OBJECT) (void)
{
	//
	//	Play the throw the object animation
	//
	//Set_Blended_Animation ("S_A_HUMAN.A_HOST_L3A", false);
	//Get_Human_State ()->Set_State (HumanStateClass::LOCKED_ANIMATION);

	//
	//	Stop the animation at frame 29
	//
	//Get_Anim_Control ()->Set_Mode (ANIM_MODE_TARGET, 0);
	//Get_Anim_Control ()->Set_Target_Frame (29);

	RaveshawStateTimer = 1.0F;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(RAVESHAW_STATE_THROW_OBJECT)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_END(RAVESHAW_STATE_THROW_OBJECT) (void)
{
	//
	//	Release our lock on the animation state
	//
	Get_Human_State ()->Stop_Scripted_Animation ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(RAVESHAW_STATE_THROW_OBJECT)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(RAVESHAW_STATE_THROW_OBJECT) (void)
{
	if (RaveshawStateTimer != UNINITIALIZED_TIMER) {
		RaveshawStateTimer -= TimeManager::Get_Frame_Seconds ();
		if (RaveshawStateTimer <= 0) {
			RaveshawStateTimer = UNINITIALIZED_TIMER;

			//
			//	Finish the animation
			//
			Get_Anim_Control ()->Set_Target_Frame (58);
		}

	} else {

		//
		//	Handy macro
		//
		float curr_frame = Get_Anim_Control ()->Get_Current_Frame ();
		#define PASSED_FRAME(frame) (LastMeleeAnimFrame < frame && curr_frame >= frame)
		if (PASSED_FRAME (36.0F)) {
		
			//
			//	Let the object fly!
			//
			ThrownObjectState.Set_State (THROWN_OBJECT_STATE_FLYING);
		}

		if (Get_Anim_Control ()->Is_Complete ()) {

			//
			//	Decide on what to do next...
			//
			Determine_New_Overall_State ();
		}
	}

	//
	//	Make sure to face the star
	//
	Set_Targeting (StarPos);

	//
	//	Cache the last frame number...
	//
	LastMeleeAnimFrame = Get_Anim_Control ()->Get_Current_Frame ();
	return ;
}



///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(RAVESHAW_STATE_GRAB_STAR)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(RAVESHAW_STATE_GRAB_STAR) (void)
{
	HasMeleeAttackHit		= false;
	LastMeleeAnimFrame	= 0.0F;

	//
	//	Play the grab the star animation
	//
	//Set_Blended_Animation ("S_A_HUMAN.RAV_HIT", false);
	//Set_Blended_Animation ("S_C_HUMAN.H_C_A0A0_L02", false);
	//
	//	Play the grab the soldier animation
	//
	Set_Blended_Animation ("S_A_HUMAN.RAV_GRABTHROW", false, 14.0F);
	Get_Human_State ()->Set_State (HumanStateClass::LOCKED_ANIMATION);
	((HumanAnimControlClass *)Get_Anim_Control ())->Set_Anim_Speed_Scale (0.5F);

	//
	//	Grab the player
	//
	HavocState.Set_State (HAVOC_STATE_GRABBED);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(RAVESHAW_STATE_GRAB_STAR)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_END(RAVESHAW_STATE_GRAB_STAR) (void)
{
	//
	//	Release our lock on the animation state
	//
	Get_Human_State ()->Stop_Scripted_Animation ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(RAVESHAW_STATE_GRAB_STAR)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(RAVESHAW_STATE_GRAB_STAR) (void)
{
	//
	//	Decide on something new to do if finished
	//
	if (Get_Anim_Control ()->Is_Complete ()) {
		RaveshawState.Set_State (RAVESHAW_STATE_NOTHING);
		Determine_New_Overall_State ();
	} else {

		float curr_frame = Get_Anim_Control ()->Get_Current_Frame ();

		//
		//	Handy macro
		//
		#define PASSED_FRAME(frame) (LastMeleeAnimFrame < frame && curr_frame >= frame)
		if (PASSED_FRAME (37.0F)) {
			HavocState.Set_State (HAVOC_STATE_FLYING);
		} else if (PASSED_FRAME (25.0F)) {
			((HumanAnimControlClass *)Get_Anim_Control ())->Set_Anim_Speed_Scale (1.0F);
		}

		LastMeleeAnimFrame = curr_frame;
	}

	//
	//	Always try to face the star
	//
	//Set_Targeting (StarPos);

	/*float curr_frame = Get_Anim_Control ()->Get_Current_Frame ();
	if (curr_frame >= 7.0F) {

		//
		//	Decide on something new to do if finished
		//
		if (Get_Anim_Control ()->Is_Complete ()) {
			RaveshawState.Set_State (RAVESHAW_STATE_NOTHING);
			Determine_New_Overall_State ();
		}
		
		//
		//	Apply hefty damage if we connect
		//
		if (HasMeleeAttackHit == false) {
			HasMeleeAttackHit |= Apply_Bone_Collision_Damage (15.0F, "C R HAND");
			HasMeleeAttackHit |= Apply_Bone_Collision_Damage (15.0F, "C L HAND");
		}
		
		//
		//	Cache the last frame number...
		//
		LastMeleeAnimFrame = Get_Anim_Control ()->Get_Current_Frame ();	
	}*/

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(RAVESHAW_STATE_BODYSLAM)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(RAVESHAW_STATE_BODYSLAM) (void)
{
	//
	//	Find the "closest" position around the tiberium for Raveshaw to run to
	//
	Vector3 star_vector_from_tib = (StarPos - TIBERIUM_POS);
	star_vector_from_tib.Z = 0;

	//
	//	Check to see if the player is in the right spot for a body slam
	//
	if (star_vector_from_tib.Length2 () < (SAFE_JUMP_RADIUS * SAFE_JUMP_RADIUS)) {
		RaveshawState.Set_State (RAVESHAW_STATE_JUMP_DOWN);
	} else {
		HasMeleeAttackHit		= false;
		LastMeleeAnimFrame	= 0.0F;

		//
		//	Play the bodyslam animation
		//
		Set_Blended_Animation ("S_A_HUMAN.H_A_BODYSLAM", false);
		Get_Human_State ()->Set_State (HumanStateClass::LOCKED_ANIMATION);

		//
		//	Stop the animation at frame 69
		//
		Get_Anim_Control ()->Set_Mode (ANIM_MODE_TARGET, 0);
		Get_Anim_Control ()->Set_Target_Frame (69);

		//
		//	Play a yell sound.
		//
		WWAudioClass::Get_Instance ()->Create_Instant_Sound ("Rav_Body_Slam_Yell", Get_Transform ());
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(RAVESHAW_STATE_BODYSLAM)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_END(RAVESHAW_STATE_BODYSLAM) (void)
{
	//
	//	Release our lock on the animation state
	//
	Get_Human_State ()->Stop_Scripted_Animation ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(RAVESHAW_STATE_BODYSLAM)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(RAVESHAW_STATE_BODYSLAM) (void)
{
	//
	//	Have we finished the body slam?
	//
	float curr_frame = Get_Anim_Control ()->Get_Current_Frame ();
	if (curr_frame >= 174.0F) {
		
		//
		//	Determine which state to goto next...
		//
		Determine_New_Overall_State ();

	} else {

		//
		//	Handy macro
		//
		#define PASSED_FRAME(frame) (LastMeleeAnimFrame < frame && curr_frame >= frame)

		if (PASSED_FRAME (37.0F)) {

			//
			//	Get the mutant to jump to the star's position
			//
			Peek_Human_Phys ()->Jump_To_Point (StarPos);
		} else if (PASSED_FRAME (90.0F)) {

			//
			//	Create an explision when he lands
			//
			DefinitionClass *definition = DefinitionMgrClass::Find_Named_Definition ("Explosion_Raveshaw_Bodyslam");
			if (definition != NULL) {

				//
				//	Position the explosion at the mutants pelvis
				//
				const Matrix3D &pelvis_tm = Peek_Model ()->Get_Bone_Transform ("C PELVIS");
				ExplosionManager::Create_Explosion_At (definition->Get_ID (), pelvis_tm.Get_Translation (), NULL);
			}
		}
		
		//
		//	Do we need to check for the ground?
		//
		if (Get_Anim_Control ()->Get_Target_Frame () != 174.0F) {
		
			//
			//	Now determine if the mutant is within a few feet of the ground
			//
			float distance = Get_Distance_From_Ground ();
			if (distance < 1.0F) {
				Get_Anim_Control ()->Set_Target_Frame (174.0F);
			}		
		}

		//
		//	Apply massive damage if we connect
		//
		Apply_Bone_Collision_Damage (500.0F, "C PELVIS");
		
		//
		//	Cache the last frame number...
		//
		LastMeleeAnimFrame = Get_Anim_Control ()->Get_Current_Frame ();	
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(RAVESHAW_STATE_JUMP_DOWN)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(RAVESHAW_STATE_JUMP_DOWN) (void)
{
	//
	//	Find the "closest" position to the star that Raveshaw can jump to
	//
	Vector3 dir_vector = (StarPos - TIBERIUM_POS);
	dir_vector.Normalize ();
	CurrentDestPos = TIBERIUM_POS + (SAFE_JUMP_RADIUS * dir_vector);

	//
	//	Now, jump to this position
	//
	Jump_To_Point (CurrentDestPos);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(RAVESHAW_STATE_JUMP_DOWN)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(RAVESHAW_STATE_JUMP_DOWN) (void)
{
	//
	//	Apply massive damage if we connect
	//
	Apply_Bone_Collision_Damage (500.0F, "C L FOOT");
	Apply_Bone_Collision_Damage (500.0F, "C R FOOT");

	//
	//	The state ends when the mutant lands
	//
	if (JumpState.Get_State () == JUMP_STATE_NONE) {
		RaveshawState.Set_State (RAVESHAW_STATE_NOTHING);
		OverallState.Set_State (OVERALL_STATE_CHASE_STAR);
	}

	return ;
}





///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(RAVESHAW_STATE_DYING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(RAVESHAW_STATE_DYING) (void)
{
	//
	//	Play the crouching animation
	//
	Set_Blended_Animation ("S_A_HUMAN.RAV_DEATH", false);
	Get_Human_State ()->Set_State (HumanStateClass::LOCKED_ANIMATION);

	//
	//	Stop the animation at frame 49
	//
	Get_Anim_Control ()->Set_Mode (ANIM_MODE_TARGET, 0);
	Get_Anim_Control ()->Set_Target_Frame (49);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(RAVESHAW_STATE_DYING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(RAVESHAW_STATE_DYING) (void)
{
	/*Vector3 delta_vector = RaveshawPos - TIBERIUM_POS;

	Vector3 dir_vector = (RaveshawPos - TIBERIUM_POS);
	dir_vector.Normalize ();
	Vector3 target_pos = TIBERIUM_POS + (8.0F * dir_vector);*/

	Vector3 target_pos (0, 0, 0);
	Find_Death_Facing_Pos (&target_pos);
	Set_Targeting (target_pos);

	//
	//	Handy macro
	//
	float curr_frame = Get_Anim_Control ()->Get_Current_Frame ();
	#define PASSED_FRAME(frame) (LastMeleeAnimFrame < frame && curr_frame >= frame)
	if (PASSED_FRAME (43.0F)) {
		
	//
	//	Start falling when the animation gets to the correct frame
	//
		RaveshawState.Set_State (RAVESHAW_STATE_FALL);
	}

	//
	//	Cache the last frame number...
	//
	LastMeleeAnimFrame = Get_Anim_Control ()->Get_Current_Frame ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(RAVESHAW_STATE_FALL)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(RAVESHAW_STATE_FALL) (void)
{
	RaveshawStateTimer = 0.125F;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(RAVESHAW_STATE_FALL)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(RAVESHAW_STATE_FALL) (void)
{
	RaveshawStateTimer += TimeManager::Get_Frame_Seconds ();
	float velocity = (22.0F * RaveshawStateTimer);

	Vector3 new_pos = RaveshawPos;
	new_pos.Z -= velocity * TimeManager::Get_Frame_Seconds ();

	//
	//	Simply stop falling when he reaches the "ground".  (We hard-code the ground's
	// Z-position for simplicity.)
	//
	const float FLOOR_POS1 = -189.1F;
	const float FLOOR_POS2 = -191.0F;
	
	if (	(new_pos.X >= -133.1F && new_pos.Z <= FLOOR_POS1) ||
			(new_pos.X < -133.1F && new_pos.Z <= FLOOR_POS2))
	{
		if (new_pos.X >= -133.1F) {
			new_pos.Z = FLOOR_POS1;
		} else {
			new_pos.Z = FLOOR_POS2;
		}

		//
		//	Move onto the "landing" state
		//
		RaveshawState.Set_State (RAVESHAW_STATE_DEATH_LANDING);
	}

	//
	//	Move Raveshaw
	//
	Set_Position (new_pos);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(RAVESHAW_STATE_DEATH_LANDING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(RAVESHAW_STATE_DEATH_LANDING) (void)
{
	//
	//	Play the rest of the animation
	//
	Get_Anim_Control ()->Set_Target_Frame (63);
	RaveshawStateTimer = UNINITIALIZED_TIMER;

	//
	//	Play a landing sound.
	//
	WWAudioClass::Get_Instance ()->Create_Instant_Sound ("Rav_Death_Fall", Get_Transform ());
	COMBAT_SCENE->Add_Camera_Shake (RaveshawPos, 40.0, 0.75F, 0.125F);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(RAVESHAW_STATE_DEATH_LANDING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(RAVESHAW_STATE_DEATH_LANDING) (void)
{
	if (RaveshawStateTimer != UNINITIALIZED_TIMER) {
		RaveshawStateTimer -= TimeManager::Get_Frame_Seconds ();
		if (RaveshawStateTimer <= 0) {
			//
			//	Trigger mission success
			//
			CombatManager::Mission_Complete (true);
		}
	} else {

		//
		//	When the animation's complete, sit and wait for
		// a couple of seconds
		//
		if (Get_Anim_Control ()->Is_Complete ()) {
			RaveshawStateTimer = 2.0F;
		}
	}

	return ;
}



///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(ENGORGED_STATE_ABSORBING_TIBERIUM)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(ENGORGED_STATE_ABSORBING_TIBERIUM) (void)
{
	EngorgedStateTimer = HEAL_TIME;
	LightningRodState.Set_State (LIGHTNING_ROD_STATE_ACTIVE);

	//
	//	Configure the effect
	//
	TiberiumEffect->Set_Intensity (0.0F);
	Peek_Physical_Object ()->Add_Effect_To_Me (TiberiumEffect);
	IsTiberiumEffectApplied = true;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(ENGORGED_STATE_ABSORBING_TIBERIUM)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(ENGORGED_STATE_ABSORBING_TIBERIUM) (void)
{
	const float HEAL_RATE = 50.0F;

	//
	//	If he's finished absorbing the tiberium, then start chasing the player
	//
	EngorgedStateTimer -= TimeManager::Get_Frame_Seconds ();
	if (EngorgedStateTimer <= 0) {
		EngorgedState.Set_State (ENGORGED_STATE_FADING);
		OverallState.Set_State (OVERALL_STATE_CHASE_STAR);
	} else {
		
		//
		//	Configure the intensity
		//
		float percent = 1.0F - WWMath::Clamp ((EngorgedStateTimer / HEAL_TIME), 0.0F, 1.0F);
		TiberiumEffect->Set_Intensity (percent * EFFECT_INTENSITY);

		//
		//	Heal the mutant a little bit
		//
		float health = DefenseObject.Get_Health ();
		health			+= HEAL_RATE * TimeManager::Get_Frame_Seconds ();
		DefenseObject.Set_Health (health);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(ENGORGED_STATE_FADING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(ENGORGED_STATE_FADING) (void)
{
	LightningRodState.Set_State (ENGORGED_STATE_NONE);
	EngorgedStateTimer = TiberiumEffect->Get_Intensity () * EFFECT_FADE_TIME;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(ENGORGED_STATE_FADING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_END(ENGORGED_STATE_FADING) (void)
{
	//
	//	Remove the blue shimmering effect
	//
	Peek_Physical_Object ()->Remove_Effect_From_Me (TiberiumEffect);
	IsTiberiumEffectApplied = false;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(ENGORGED_STATE_FADING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(ENGORGED_STATE_FADING) (void)
{
	//
	//	If he's finished absorbing the tiberium, then start chasing the player
	//
	EngorgedStateTimer -= TimeManager::Get_Frame_Seconds ();
	if (EngorgedStateTimer <= 0) {
		EngorgedState.Set_State (ENGORGED_STATE_NONE);
	} else {
		
		//
		//	Configure the intensity
		//
		float percent = WWMath::Clamp ((EngorgedStateTimer / EFFECT_FADE_TIME), 0.0F, 1.0F);
		TiberiumEffect->Set_Intensity (percent * EFFECT_INTENSITY);
	}

	return ;
}



///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(JUMP_STATE_CROUCHING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(JUMP_STATE_CROUCHING) (void)
{
	//
	//	Play the crouching animation
	//
	Set_Blended_Animation ("S_A_HUMAN.RAV_JUMP", false);
	Get_Human_State ()->Set_State (HumanStateClass::LOCKED_ANIMATION);

	//
	//	Stop the animation at frame 10
	//
	Get_Anim_Control ()->Set_Mode (ANIM_MODE_TARGET, 0);
	Get_Anim_Control ()->Set_Target_Frame (10);

	//
	//	Stop any current action
	//
	Get_Action ()->Pause (true);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(JUMP_STATE_CROUCHING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(JUMP_STATE_CROUCHING) (void)
{
	if (Get_Anim_Control ()->Is_Complete ()) {
		JumpState.Set_State (JUMP_STATE_JUMPING);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(JUMP_STATE_JUMPING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(JUMP_STATE_JUMPING) (void)
{
	//
	//	Play the jumping animation
	//
	Set_Blended_Animation ("S_A_HUMAN.H_A_A0D0", false);
	//Get_Anim_Control ()->Set_Target_Frame (11);
	Get_Human_State ()->Set_State (HumanStateClass::LOCKED_ANIMATION);

	//
	//	Jump to the given position
	//
	Peek_Human_Phys ()->Jump_To_Point (CurrentJumpToPos);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(JUMP_STATE_JUMPING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(JUMP_STATE_JUMPING) (void)
{
	Vector3 velocity;
	Peek_Human_Phys ()->Get_Velocity (&velocity);

	//
	//	Is Raveshaw close to the ground?
	//
	if (velocity.Z < 0 && Get_Distance_From_Ground () < 2.0F) {
		JumpState.Set_State (JUMP_STATE_LANDING);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(JUMP_STATE_LANDING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(JUMP_STATE_LANDING) (void)
{
	//
	//	Play the landing animation
	//
	//Set_Blended_Animation ("S_A_HUMAN.H_A_A0L0", false, 0.0F, true);
	//Get_Human_State ()->Set_State (HumanStateClass::LOCKED_ANIMATION);

	Set_Blended_Animation ("S_A_HUMAN.RAV_JUMP", false);
	Get_Human_State ()->Set_State (HumanStateClass::LOCKED_ANIMATION);

	Get_Anim_Control ()->Set_Mode (ANIM_MODE_TARGET, 11);
	Get_Anim_Control ()->Set_Target_Frame (31);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(JUMP_STATE_LANDING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_END(JUMP_STATE_LANDING) (void)
{
	//
	//	Release our lock on the animation state
	//
	Get_Human_State ()->Stop_Scripted_Animation ();

	//
	//	Resume the current action
	//
	Get_Action ()->Pause (false);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(JUMP_STATE_LANDING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(JUMP_STATE_LANDING) (void)
{
	//
	//	Handy macro
	//
	float curr_frame = Get_Anim_Control ()->Get_Current_Frame ();
	#define PASSED_FRAME(frame) (LastMeleeAnimFrame < frame && curr_frame >= frame)
	if (PASSED_FRAME (20.0F)) {
		
		//
		//	Play the landing sound
		//
		WWAudioClass::Get_Instance ()->Create_Instant_Sound ("Rav_Land_On_Metal", Get_Transform ());
		COMBAT_SCENE->Add_Camera_Shake (RaveshawPos, 40.0, 0.75F, 0.125F);
	}

	LastMeleeAnimFrame = curr_frame;

	//
	//	End the state when the animation is complete
	//
	if (Get_Anim_Control ()->Is_Complete ()) {
		JumpState.Set_State (JUMP_STATE_NONE);
	}

	return ;
}





///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(HAVOC_STATE_GRABBED)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(HAVOC_STATE_GRABBED) (void)
{
	if (COMBAT_STAR != NULL) {
		COMBAT_STAR->Peek_Physical_Object ()->Enable_Objects_Simulation (false);
		COMBAT_STAR->Control_Enable (false);
	}

	//
	//	Now snap to 3rd person so we are rendered
	//
	RestoreFirstPerson = CombatManager::Is_First_Person ();
	CombatManager::Set_First_Person (false);

	//
	//	Lock Havoc into a struggling state
	//
	COMBAT_STAR->Set_Blended_Animation ("S_A_HUMAN.STL_STRUGGLE", true);
	COMBAT_STAR->Get_Human_State ()->Set_State (HumanStateClass::LOCKED_ANIMATION);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(HAVOC_STATE_GRABBED)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(HAVOC_STATE_GRABBED) (void)
{
	Link_Player_To_Hands ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(HAVOC_STATE_FLYING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(HAVOC_STATE_FLYING) (void)
{
	Vector3 dir_vector = Get_Transform ().Get_X_Vector ();
	dir_vector.Z = 0.2F;
	dir_vector.Normalize ();

	//
	//	Calculate the direction that the soldier will fly through the air
	//	
	FlyingObjectVector	= dir_vector * 15.0F;	
	FlyingDist				= FlyingObjectVector.Length ();
	FlyingObjectDest		= StarPos + FlyingObjectVector;
	FlyingObjectVector.Normalize ();

	//
	//	Restore control to the player
	//
	COMBAT_STAR->Control_Enable (true);
	CombatManager::Set_First_Person (RestoreFirstPerson);

	//
	//	Release Havoc from his struggles
	//
	COMBAT_STAR->Get_Human_State ()->Stop_Scripted_Animation ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(HAVOC_STATE_FLYING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(HAVOC_STATE_FLYING) (void)
{
	const float RATE		= 15.0F;

	float percent			= RATE * TimeManager::Get_Frame_Seconds ();
	Vector3 move_vector	= FlyingObjectVector * percent;

	//
	//	Simulate gravity
	//
	Vector3 delta			= StarPos - FlyingObjectDest;
	delta.Z					= 0;
	float dist				= delta.Length ();
	float dist_percent	= 1.0F - WWMath::Clamp ((dist / FlyingDist), 0.0F, 1.0F);

	if (dist_percent > 0.3F) {
		move_vector.Z	-= (dist_percent - 0.3F) * (8.0F * TimeManager::Get_Frame_Seconds ());
	}

	//
	//	Do the move and check to see if we hit anything
	//
	if (Fly_Move (COMBAT_STAR, move_vector)) {

		//
		//	Restore physics simulation
		//
		COMBAT_STAR->Peek_Physical_Object ()->Enable_Objects_Simulation (true);		

		//
		//	Apply some damage to the player
		//
		OffenseObjectClass offense_obj (20.0F, 1);
		COMBAT_STAR->Apply_Damage_Extended (offense_obj, 1.0F);		

		//
		//	Restore Havoc's state
		//
		HavocState.Set_State (HAVOC_STATE_NONE);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(STEALTH_SOLDIER_STATE_DISPLAY)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(STEALTH_SOLDIER_STATE_DISPLAY) (void)
{
	//
	//	Create the stealth solider and position its neck at Raveshaw's hand
	//
	Create_Stealth_Soldier (Matrix3D (1));
	Link_Stealth_Soldier_To_Hand ();

	//
	//	Check to see if the stealth soldier has been destroyed...
	//
	SoldierGameObj *stealth_soldier = Peek_Stealth_Soldier ();
	if (stealth_soldier == NULL) {
		OverallState.Set_State (OVERALL_STATE_NOTHING);
		RaveshawState.Set_State (RAVESHAW_STATE_ROAR);
		MoveState.Set_State (MOVE_STATE_STOP);
		StealthSoldierState.Set_State (STEALTH_SOLDIER_STATE_NONE);
		Determine_New_Overall_State ();
		return ;
	}

	//
	//	Play a looping animation on the soldier
	//
	stealth_soldier->Set_Blended_Animation ("S_A_HUMAN.STL_STRUGGLE", true);
	stealth_soldier->Get_Human_State ()->Set_State (HumanStateClass::LOCKED_ANIMATION);

	StealthSoldierStateTimer = 3.0F;

	//
	//	Play the stealth broken sound and the gurgle sound
	//
	WWAudioClass::Get_Instance ()->Create_Instant_Sound ("Rav_Stealth_Broken", Get_Transform ());
	WWAudioClass::Get_Instance ()->Create_Instant_Sound ("Rav_Throat_Grab_Twiddler", Get_Transform ());
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(STEALTH_SOLDIER_STATE_DISPLAY)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(STEALTH_SOLDIER_STATE_DISPLAY) (void)
{
	//
	//	Move the soldier with Ravesahw
	//
	Link_Stealth_Soldier_To_Hand ();

	//
	//	Is it time for the soldier to be thrown yet?
	//
	StealthSoldierStateTimer -= TimeManager::Get_Frame_Seconds ();
	if (StealthSoldierStateTimer <= 0) {
		RaveshawState.Set_State (RAVESHAW_STATE_THROW_SOLDIER);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(STEALTH_SOLDIER_STATE_FLYING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(STEALTH_SOLDIER_STATE_FLYING) (void)
{
	//
	//	Check to see if the stealth soldier has been destroyed...
	//
	SoldierGameObj *stealth_soldier = Peek_Stealth_Soldier ();
	if (stealth_soldier == NULL) {
		OverallState.Set_State (OVERALL_STATE_NOTHING);
		RaveshawState.Set_State (RAVESHAW_STATE_ROAR);
		MoveState.Set_State (MOVE_STATE_STOP);
		StealthSoldierState.Set_State (STEALTH_SOLDIER_STATE_NONE);
		Determine_New_Overall_State ();
		return ;
	}

	Vector3 solider_pos;
	stealth_soldier->Get_Position (&solider_pos);

	//
	//	Calculate the direction that the soldier will fly through the air
	//
	FlyingObjectDest		= StarPos;
	FlyingObjectVector	= (StarPos - solider_pos);
	FlyingDist				= FlyingObjectVector.Length ();
	FlyingObjectVector.Normalize ();

	//
	//	Check to see which animation we should be play.
	//		(Fly forwards, fly backwards, fly right, fly left)
	//
	Vector3 relative_vector = stealth_soldier->Get_Transform ().Inverse_Rotate_Vector (FlyingObjectVector);
	if (WWMath::Fabs (relative_vector.X) > WWMath::Fabs (relative_vector.Y)) {
		if (relative_vector.X > 0) {
			stealth_soldier->Set_Blended_Animation ("S_A_HUMAN.H_A_FLY1", true);
			stealth_soldier->Get_Human_State ()->Set_State (HumanStateClass::LOCKED_ANIMATION);
		} else {
			stealth_soldier->Set_Blended_Animation ("S_A_HUMAN.H_A_FLY2", true);
			stealth_soldier->Get_Human_State ()->Set_State (HumanStateClass::LOCKED_ANIMATION);		
		}
	} else {
		if (relative_vector.Y > 0) {
			stealth_soldier->Set_Blended_Animation ("S_A_HUMAN.H_A_FLY4", true);
			stealth_soldier->Get_Human_State ()->Set_State (HumanStateClass::LOCKED_ANIMATION);
		} else {
			stealth_soldier->Set_Blended_Animation ("S_A_HUMAN.H_A_FLY3", true);
			stealth_soldier->Get_Human_State ()->Set_State (HumanStateClass::LOCKED_ANIMATION);		
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(STEALTH_SOLDIER_STATE_FLYING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(STEALTH_SOLDIER_STATE_FLYING) (void)
{
	//
	//	Check to see if the stealth soldier has been destroyed...
	//
	SoldierGameObj *stealth_soldier = Peek_Stealth_Soldier ();
	if (stealth_soldier == NULL) {
		OverallState.Set_State (OVERALL_STATE_NOTHING);
		RaveshawState.Set_State (RAVESHAW_STATE_ROAR);
		MoveState.Set_State (MOVE_STATE_STOP);
		StealthSoldierState.Set_State (STEALTH_SOLDIER_STATE_NONE);
		Determine_New_Overall_State ();
		return ;
	}

	const float RATE		= 15.0F;

	Vector3 soldier_pos;
	stealth_soldier->Get_Position (&soldier_pos);
	
	float dist = (soldier_pos - FlyingObjectDest).Length ();
	float dist_percent = (dist / FlyingDist);

	float percent			= RATE * TimeManager::Get_Frame_Seconds ();
	Vector3 move_vector	= FlyingObjectVector * percent;

	move_vector.Z += (dist_percent - 0.5F) * (4.0F * TimeManager::Get_Frame_Seconds ());

	//
	//	Do the move and check to see if we hit anything
	//
	if (Fly_Move (stealth_soldier, move_vector)) {

		//
		//	Kill the soldier (which should create a very large explosion)
		//
		OffenseObjectClass offense_obj (10000.0F, 1);
		stealth_soldier->Apply_Damage_Extended (offense_obj, 1.0F);
		
		//
		//	Restore our state
		//
		StealthSoldierState.Set_State (STEALTH_SOLDIER_STATE_NONE);
		stealth_soldier->Peek_Physical_Object ()->Remove_Effect_From_Me (StealthEffect);
		StealthSoldier = NULL;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(THROWN_OBJECT_STATE_PICKUP)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(THROWN_OBJECT_STATE_PICKUP) (void)
{
	if (ThrownObject == NULL) {
		return ;
	}

	//
	//	Get the transform of Raveshaw's right hand
	//
	const Matrix3D &r_hand_tm	= Peek_Model ()->Get_Bone_Transform ("C R HAND");

	//
	//	Now, calculate the object's transform relative to Raveshaw's right hand
	//
	Matrix3D world_to_hand_tm;
	r_hand_tm.Get_Orthogonal_Inverse (world_to_hand_tm);
	
	const Matrix3D &obj_tm = ThrownObject->Get_Transform ();
	RelObjTM = world_to_hand_tm * obj_tm;

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(THROWN_OBJECT_STATE_PICKUP)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(THROWN_OBJECT_STATE_PICKUP) (void)
{
	//
	//	Move the object with Raveshaw
	//
	Link_Thrown_Object_To_Hands ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(THROWN_OBJECT_STATE_FLYING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(THROWN_OBJECT_STATE_FLYING) (void)
{
	Vector3 object_pos;
	ThrownObject->Get_Position (&object_pos);

	//
	//	Calculate the direction that the object will fly through the air
	//
	FlyingObjectDest		= StarPos;
	FlyingObjectVector	= (StarPos - object_pos);
	FlyingDist				= FlyingObjectVector.Length ();
	FlyingObjectVector.Normalize ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(THROWN_OBJECT_STATE_FLYING)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(THROWN_OBJECT_STATE_FLYING) (void)
{
	const float RATE		= 15.0F;

	Vector3 object_pos;
	ThrownObject->Get_Position (&object_pos);
	
	float dist				= (object_pos - FlyingObjectDest).Length ();
	float dist_percent	= (dist / FlyingDist);

	float percent			= RATE * TimeManager::Get_Frame_Seconds ();
	Vector3 move_vector	= FlyingObjectVector * percent;

	move_vector.Z += (dist_percent - 0.5F) * (4.0F * TimeManager::Get_Frame_Seconds ());

	//
	//	Do the move and check to see if we hit anything
	//
	if (Fly_Move (ThrownObject, move_vector)) {

		//
		//	Fake-destroy the object
		//
		OffenseObjectClass offense_obj (10000.0F, 1);
		ThrownObject->Completely_Damaged (offense_obj);
		ThrownObject->Set_Delete_Pending ();
		ThrownObject = NULL;

		//
		//	Now, revert back to the nothing state
		//
		ThrownObjectState.Set_State (THROWN_OBJECT_STATE_NONE);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_BEGIN(LIGHTNING_ROD_STATE_ACTIVE)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_BEGIN(LIGHTNING_ROD_STATE_ACTIVE) (void)
{
	LightningRodStateTimer = 0;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_END(LIGHTNING_ROD_STATE_ACTIVE)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_END(LIGHTNING_ROD_STATE_ACTIVE) (void)
{
	//
	//	Hide all the arc effects
	//
	for (int index = 0; index < ARC_OBJ_COUNT; index ++) {
		PhysClass *phys_obj = ArcObjects[index]->Peek_Physical_Object ();
		phys_obj->Peek_Model ()->Set_Hidden (true);
		ArcLifeRemaining[index] = 0;
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	STATE_IMPL_THINK(LIGHTNING_ROD_STATE_ACTIVE)
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::STATE_IMPL_THINK(LIGHTNING_ROD_STATE_ACTIVE) (void)
{
	LightningRodStateTimer -= TimeManager::Get_Frame_Seconds ();
	if (LightningRodStateTimer <= 0) {
		LightningRodStateTimer = 0.125F;
		
		int count	= LightningRodList.Count ();
		int index0	= FreeRandom.Get_Int (count - 1);
		int index1	= index0 + 1;

		const Matrix3D &start_tm	= LightningRodList[index0]->Peek_Model ()->Get_Bone_Transform ("BBZZZT");
		const Matrix3D &end_tm		= LightningRodList[index1]->Peek_Model ()->Get_Bone_Transform ("BBZZZT");

		Add_Lightning_Arc (start_tm.Get_Translation (), end_tm.Get_Translation ());

		//
		//	Randomly strike the mutant
		//
		if (FreeRandom.Get_Int (6) == 1) {
			Add_Lightning_Arc (start_tm.Get_Translation (), RaveshawPos + Vector3 (0, 0, 1.0F));

			//
			//	Play an electric sound.
			//
			WWAudioClass::Get_Instance ()->Create_Instant_Sound ("Rav_Elec_Twiddler", Get_Transform ());
		}

		//
		//	Randomly strike the player
		//
		int star_dist = (StarPos - TIBERIUM_POS).Length ();
		if (FreeRandom.Get_Int (star_dist) == 1) {

			//
			//	Apply 5 points of "steel" damage to the player
			//
			OffenseObjectClass offense_obj (5.0F, 1);
			COMBAT_STAR->Apply_Damage_Extended (offense_obj, 1.0F, Vector3 (-1, 0, 0));
			Add_Lightning_Arc (end_tm.Get_Translation (), StarPos + Vector3 (0, 0, 1.0F));

			//
			//	Play an electric sound.
			//
			WWAudioClass::Get_Instance ()->Create_Instant_Sound ("Rav_Elec_Twiddler", Matrix3D (StarPos));
		}
	}

	//
	//	Hide any arc-effects that have finished
	//
	for (int index = 0; index < ARC_OBJ_COUNT; index ++) {
		if (ArcLifeRemaining[index] > 0) {
			ArcLifeRemaining[index]	-= TimeManager::Get_Frame_Seconds ();
			if (ArcLifeRemaining[index] <= 0) {
				PhysClass *phys_obj = ArcObjects[index]->Peek_Physical_Object ();
				phys_obj->Peek_Model ()->Set_Hidden (true);
			}
		}
	}

	return ;
}



///////////////////////////////////////////////////////////////////////////
//
//	Apply_Bone_Collision_Damage
//
///////////////////////////////////////////////////////////////////////////
bool
RaveshawBossGameObjClass::Apply_Bone_Collision_Damage (float damage_scale, const char *bone_name)
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
		OffenseObjectClass offense_obj (damage_scale, 1);
		COMBAT_STAR->Apply_Damage_Extended (offense_obj, 1.0F, Vector3 (-1, 0, 0), obj_name);

		//
		//	Knock the player back a little bit
		//
		Vector3 delta_vector = move_vector;
		delta_vector.Normalize ();
		COMBAT_STAR->Peek_Physical_Object ()->As_Phys3Class ()->Collide (delta_vector * 0.25F);
		
		if (delta_vector.Z > 0.0F) {
			delta_vector.Z = 0.5F;
			delta_vector.Normalize ();
			COMBAT_STAR->Peek_Physical_Object ()->As_Phys3Class ()->Apply_Impulse (delta_vector * 2.0F);
		}

		//
		//	Play the hit sound...
		//
		Attach_Sound ("Fight Impact Sound Twiddler", "ROOTTRANSFORM");
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Distance_From_Ground
//
///////////////////////////////////////////////////////////////////////////
float
RaveshawBossGameObjClass::Get_Distance_From_Ground (void)
{	
	const float MAX_DISTANCE	= 100.0F;
	float distance					= 0.0F;

	//
	//	Determine the start and end positions of the ray
	//
	Vector3 start;
	Get_Position (&start);

	Vector3 end	= start - Vector3 (0, 0, MAX_DISTANCE);
	LineSegClass ray (start, end);

	//
	//	Cast the ray into the world
	//
	CastResultStruct res;
	PhysRayCollisionTestClass raytest (ray, &res, DEFAULT_COLLISION_GROUP, COLLISION_TYPE_PHYSICAL);

	Peek_Physical_Object ()->Inc_Ignore_Counter ();
	COMBAT_SCENE->Cast_Ray (raytest);
	Peek_Physical_Object ()->Dec_Ignore_Counter ();

	//
	//	Calculate where in the world this would hit
	//
	if (res.StartBad == false) {
		distance = (res.Fraction * MAX_DISTANCE);
	}

	return distance;
}


///////////////////////////////////////////////////////////////////////////
//
//	Create_Stealth_Soldier
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::Create_Stealth_Soldier (const Matrix3D &tm)
{
	//
	//	Create the stealth soldier
	//
	PhysicalGameObj *phys_game_obj = ObjectLibraryManager::Create_Object ("Raveshaw Boss Fodder");
	WWASSERT (phys_game_obj != NULL);
	StealthSoldier = phys_game_obj;

	//
	//	Position the stealth soldier accordingly
	//
	SoldierGameObj *soldier = phys_game_obj->As_SoldierGameObj ();
	soldier->Set_Transform (tm);

	//
	//	Configure the stealth effect to be "on" (stealthed)
	//
	StealthEffect->Set_Current_State (1.0F);
	StealthEffect->Set_Target_State (0.0F);
	
	//
	//	Add the effect to the object
	//
	soldier->Peek_Physical_Object ()->Add_Effect_To_Me (StealthEffect);
	
	//
	//	Disable physics for this soldier
	//
	soldier->Peek_Physical_Object ()->Enable_Objects_Simulation (false);	
	soldier->Start_Observers ();
	return ;	
}


///////////////////////////////////////////////////////////////////////////
//
//	Link_Thrown_Object_To_Hands
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::Link_Thrown_Object_To_Hands (void)
{
	if (ThrownObject == NULL) {
		return ;
	}

	//
	//	Get the transforms of Raveshaw's hands
	//
	const Matrix3D &r_hand_tm	= Peek_Model ()->Get_Bone_Transform ("C R HAND");
	const Matrix3D &l_hand_tm	= Peek_Model ()->Get_Bone_Transform ("C L HAND");

	//Vector3 z_axis = (l_hand_tm.Get_Translation () - r_hand_tm.Get_Translation ());
	//z_axis.Normalize ();

	Vector3 r_hand_pos		= r_hand_tm.Get_Translation ();
	Vector3 l_hand_pos		= l_hand_tm.Get_Translation ();
	Vector3 delta_vector		= l_hand_pos - r_hand_pos;

	Matrix3D obj_tm;
	//obj_tm.Look_At (r_hand_pos, r_hand_pos - delta_vector, 0);
	Matrix3D::Multiply (r_hand_tm, RelObjTM, &obj_tm);

	ThrownObject->Set_Transform (obj_tm);
	return ;	
}


///////////////////////////////////////////////////////////////////////////
//
//	Link_Player_To_Hands
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::Link_Player_To_Hands (void)
{
	if (COMBAT_STAR == NULL) {
		return ;
	}

	//
	//	Get the transform of Raveshaw's hand
	//
	const Matrix3D &hand_tm		= Peek_Model ()->Get_Bone_Transform ("C L HAND");

	const Matrix3D &neck_tm		= COMBAT_STAR->Peek_Model ()->Get_Bone_Transform ("C NECK");
	const Matrix3D &soldier_tm	= COMBAT_STAR->Get_Transform ();

	//
	//	Determine where to position the stealth soldier so he's embedded in Raveshaw's hand
	//
	Vector3 hand_pos	= hand_tm.Get_Translation ();
	Vector3 delta_pos = neck_tm.Get_Translation () - soldier_tm.Get_Translation ();
	Vector3 new_pos	= hand_pos - delta_pos;	

	//
	//	Disable collision on Raveshaw
	//
	Peek_Physical_Object ()->Inc_Ignore_Counter ();
	COMBAT_STAR->Peek_Physical_Object ()->Inc_Ignore_Counter ();

	//
	//	Get the player's collision box
	//
	AABoxClass collision_box;
	collision_box				= COMBAT_STAR->Peek_Physical_Object ()->As_HumanPhysClass ()->Get_Collision_Box ();
	collision_box.Center		+= StarPos;

	//
	//	Calculate what vector we'll be teleporting the player through
	//
	Vector3 vector = (new_pos - StarPos);

	//
	//	Check to see if we would hit something if we do this
	//
	CastResultStruct result;
	PhysAABoxCollisionTestClass col_test (collision_box, vector, &result, DEFAULT_COLLISION_GROUP, COLLISION_TYPE_PHYSICAL);
	bool retval = COMBAT_SCENE->Cast_AABox (col_test);
	if (retval == false && result.StartBad == false && result.Fraction == 1.0F) {

		//
		//	New position should be fine, so allow the position change
		//
		Matrix3D new_soldier_tm (new_pos);
		COMBAT_STAR->Set_Transform (new_soldier_tm);
	}

	//
	//	Restore collision on Raveshaw
	//
	Peek_Physical_Object ()->Dec_Ignore_Counter ();
	COMBAT_STAR->Peek_Physical_Object ()->Dec_Ignore_Counter ();
	

	//
	//	Get the target relative to the head
	//
	//Vector3 rav_head_pos = Peek_Model ()->Get_Bone_Transform ("C HEAD").Get_Translation ();
	
	//COMBAT_STAR->Set_Targeting (rav_head_pos);
	
	/*Vector3 rav_head_rel;
	Matrix3D::Inverse_Transform_Vector (new_soldier_tm, rav_head_pos, &rav_head_rel);

	float angle = ::atan2 (rav_head_rel.Y, rav_head_rel.X);
	new_soldier_tm.Rotate_Z (angle - DEG_TO_RADF (90.0F));*/

	//
	//	Choose a random orientation for the soldier
	//
	//new_soldier_tm.Rotate_Z (WWMath::Random_Float (0, DEG_TO_RADF (319.0F)));
	

	/*Matrix3D world_to_soldier_tm;
	soldier_tm.Get_Orthogonal_Inverse (world_to_soldier_tm);

	Matrix3D neck_rel_to_sol_tm = neck_tm * world_to_soldier_tm;

	Matrix3D sol_rel_to_neck_tm;
	neck_rel_to_sol_tm.Get_Orthogonal_Inverse (sol_rel_to_neck_tm);

	Matrix3D new_soldier_tm (hand_tm * sol_rel_to_neck_tm);*/	
	
	//Matrix3D test_tm = hand_tm * sol_rel_to_neck_tm;
	//Matrix3D new_soldier_tm (test_tm.Get_Translation ());
	//new_soldier_tm = hand_tm * sol_rel_to_neck_tm;

	
	/*Matrix3D soldier_to_neck_tm;
	neck_to_soldier_tm.Get_Orthogonal_Inverse (soldier_to_neck_tm);


	Matrix3D world_to_neck_tm;
	neck_tm.Get_Orthogonal_Inverse (world_to_neck_tm);

	Matrix3D neck_to_soldier_tm = soldier_tm * world_to_neck_tm;

	Matrix3D 
	Matrix3D new_soldier_tm = (hand_tm * neck_to_soldier_tm);*/

	//COMBAT_STAR->Set_Transform (new_soldier_tm);
	return ;	
}


///////////////////////////////////////////////////////////////////////////
//
//	Link_Stealth_Soldier_To_Hand
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::Link_Stealth_Soldier_To_Hand (void)
{
	//
	//	Check to see if the stealth soldier has been destroyed...
	//
	SoldierGameObj *stealth_soldier = Peek_Stealth_Soldier ();
	if (stealth_soldier == NULL) {
		return ;
	}

	//
	//	Get the transform of Raveshaw's hand
	//
	const Matrix3D &hand_tm		= Peek_Model ()->Get_Bone_Transform ("C L HAND");

	const Matrix3D &neck_tm		= stealth_soldier->Peek_Model ()->Get_Bone_Transform ("C NECK");
	const Matrix3D &soldier_tm	= stealth_soldier->Get_Transform ();

	//
	//	Determine where to position the stealth soldier so he's embedded in Raveshaw's hand
	//
	Vector3 hand_pos	= hand_tm.Get_Translation ();
	Vector3 delta_pos = neck_tm.Get_Translation () - soldier_tm.Get_Translation ();
	Vector3 new_pos	= hand_pos - delta_pos;

	Matrix3D new_soldier_tm (new_pos);

	//
	//	Choose a random orientation for the soldier
	//
	//new_soldier_tm.Rotate_Z (WWMath::Random_Float (0, DEG_TO_RADF (319.0F)));
	

	/*Matrix3D world_to_soldier_tm;
	soldier_tm.Get_Orthogonal_Inverse (world_to_soldier_tm);

	Matrix3D neck_rel_to_sol_tm = neck_tm * world_to_soldier_tm;

	Matrix3D sol_rel_to_neck_tm;
	neck_rel_to_sol_tm.Get_Orthogonal_Inverse (sol_rel_to_neck_tm);

	Matrix3D new_soldier_tm (hand_tm * sol_rel_to_neck_tm);*/	
	
	//Matrix3D test_tm = hand_tm * sol_rel_to_neck_tm;
	//Matrix3D new_soldier_tm (test_tm.Get_Translation ());
	//new_soldier_tm = hand_tm * sol_rel_to_neck_tm;

	
	/*Matrix3D soldier_to_neck_tm;
	neck_to_soldier_tm.Get_Orthogonal_Inverse (soldier_to_neck_tm);


	Matrix3D world_to_neck_tm;
	neck_tm.Get_Orthogonal_Inverse (world_to_neck_tm);

	Matrix3D neck_to_soldier_tm = soldier_tm * world_to_neck_tm;

	Matrix3D 
	Matrix3D new_soldier_tm = (hand_tm * neck_to_soldier_tm);*/

	stealth_soldier->Set_Transform (new_soldier_tm);
	return ;	
}


///////////////////////////////////////////////////////////////////////////
//
//	Fly_Move
//
///////////////////////////////////////////////////////////////////////////
bool
RaveshawBossGameObjClass::Fly_Move (PhysicalGameObj *game_obj, const Vector3 &vector)
{
	Vector3 curr_pos;
	game_obj->Get_Position (&curr_pos);

	//
	//	Get the object's collision box
	//
	AABoxClass collision_box;
	
	if (game_obj->As_SoldierGameObj () != NULL) {
		collision_box				= game_obj->Peek_Physical_Object ()->As_HumanPhysClass ()->Get_Collision_Box ();
		collision_box.Center		+= curr_pos;
	} else {
		collision_box = game_obj->Peek_Model ()->Get_Bounding_Box ();
	}	

	if (game_obj != COMBAT_STAR) {
		collision_box.Extent.Z -= 0.25F;
	}
	
	//
	//	Disable collision on Raveshaw
	//
	Peek_Physical_Object ()->Inc_Ignore_Counter ();
	game_obj->Peek_Physical_Object ()->Inc_Ignore_Counter ();

	//
	//	Check to see if we've hit something
	//
	CastResultStruct result;
	PhysAABoxCollisionTestClass col_test (collision_box, vector, &result, DEFAULT_COLLISION_GROUP, COLLISION_TYPE_PHYSICAL);
	
	//
	//	Find where we hit
	//
	bool retval = COMBAT_SCENE->Cast_AABox (col_test);
	if (result.StartBad == false) {
		
		//
		//	Calculate where to move to...
		//
		Vector3 new_pos = curr_pos + (vector * (result.Fraction * 0.99F));

		//
		//	Move the object
		//
		game_obj->Set_Position (new_pos);
	}

	//
	//	Restore collision to Raveshaw
	//
	Peek_Physical_Object ()->Dec_Ignore_Counter ();
	game_obj->Peek_Physical_Object ()->Dec_Ignore_Counter ();
	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Find_Object_To_Throw
//
///////////////////////////////////////////////////////////////////////////
SimpleGameObj *
RaveshawBossGameObjClass::Find_Object_To_Throw (void)
{
	if (COMBAT_STAR == NULL) {
		return NULL;
	}

	SimpleGameObj *best_object	= NULL;
	float best_object_rating	= 100.0F;

	//
	//	Loop over all the game objects in the world (looking for simple game objects)
	//
	SLNode<BaseGameObj> *obj_node = NULL;
	for (obj_node = GameObjManager::Get_Game_Obj_List ()->Head (); obj_node; obj_node = obj_node->Next ()) {
		
		//
		//	Is this a simple game object?
		//
		PhysicalGameObj *phys_game_obj = obj_node->Data ()->As_PhysicalGameObj ();
		if (phys_game_obj != NULL && phys_game_obj->As_SimpleGameObj () != NULL) {
			SimpleGameObj *object = phys_game_obj->As_SimpleGameObj ();
			if (object != NULL) {
				
				//
				//	Is this one of Raveshaw's throwable objects?
				//
				const StringClass &name = object->Get_Definition ().Get_Name ();
				if (::strstr (name, "(Raveshaw Ammo)") != NULL) {

					//
					//	Get the position of the object
					//
					Vector3 object_pos;
					object->Get_Position (&object_pos);
					
					//
					//	Is this object in the boss area?
					//
					object_pos.Z = 0;

					//
					//	Calculate how far this object is from Raveshaw and the player
					//
					float dist_to_me		= (object_pos - RaveshawPos).Length () / 10.0F;
					float dist_to_star	= (object_pos - StarPos).Length () / 10.0F;
					dist_to_star			= WWMath::Clamp (1.0F - dist_to_star, 0.0F, 1.0F);

					//
					//	Is this the object that's closest to Raveshaw and farthest from the player?
					//
					float rating = (dist_to_me * 0.5F) + (dist_to_star * 0.5F);
					if (rating < best_object_rating) {
						best_object				= object;
						best_object_rating	= rating;
					}
				}
			}
		}
	}

	return best_object;
}


///////////////////////////////////////////////////////////////////////////
//
//	Determine_New_Overall_State
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::Determine_New_Overall_State (void)
{
	float health				= DefenseObject.Get_Health ();
	float health_percent		= health / DefenseObject.Get_Health_Max ();

	if (health_percent <= 0.05F) {
		
		//
		//	Retreat to the catwalk if we're mostly damaged
		//
		if (	OverallState.Get_State () != OVERALL_STATE_JUMP_TO_CATWALK ||
				OverallState.Get_State () != OVERALL_STATE_ON_CATWALK ||
				OverallState.Get_State () != OVERALL_STATE_DEATH_SEQUENCE)
		{
			OverallState.Set_State (OVERALL_STATE_JUMP_TO_CATWALK);
		}

	} else {
		
		//
		//	Should he heal himself?
		//
		int possibility = (10.0F * health_percent) + 0.5F;
		bool go_heal = (health_percent <= 0.75F && possibility > 0) && (FreeRandom.Get_Int (possibility) == 0);
		if (go_heal) {
			OverallState.Set_State (OVERALL_STATE_HEALING);
		} else {

			//
			//	How far away is the player?
			//
			float dist2 = (RaveshawPos - StarPos).Length2 ();

			//
			//	Roll a random die to determine which action to take
			//
			int choice = FreeRandom.Get_Int (100);
			if (choice < 35 && dist2 > 16.0F) {
				OverallState.Set_State (OVERALL_STATE_THROWING_SOLDIER);
			} else if (choice < 60 && (ThrownObject == NULL)) {
				OverallState.Set_State (OVERALL_STATE_THROWING_OBJECT);
			} else if (choice < 80) {
				OverallState.Set_State (OVERALL_STATE_CHASE_STAR);
			} else {
				OverallState.Set_State (OVERALL_STATE_JUMP_TO_CATWALK);
			}
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Jump_To_Point
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::Jump_To_Point (const Vector3 &pos)
{
	//
	//	Begin the jump state (if necessary)
	//
	if (JumpState.Get_State () == JUMP_STATE_NONE) {
		CurrentJumpToPos = pos;
		JumpState.Set_State (JUMP_STATE_CROUCHING);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Collect_Lightning_Rods
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::Collect_Lightning_Rods (void)
{
	LightningRodList.Delete_All ();
	
	//
	//	Compute the box where the lightning rods in question should live
	//
	AABoxClass box;
	box.Center = TIBERIUM_POS;
	box.Extent.Set (40.0F, 40.0F, 20.0F);

	//
	//	Collect all the static objects in this box
	//
	NonRefPhysListClass obj_list;
	COMBAT_SCENE->Collect_Objects (box, true, false, &obj_list);

	//
	//	Loop over all the objects
	//
	NonRefPhysListIterator it (&obj_list);
	for (it.First (); !it.Is_Done (); it.Next ()) {
		DamageableStaticPhysClass *phys_obj = it.Peek_Obj ()->As_DamageableStaticPhysClass ();
		
		//
		//	Add this object to our list
		//
		if (phys_obj != NULL) {
			if (phys_obj->Peek_Model ()->Get_Bone_Index ("BBZZZT") > 0) {
				LightningRodList.Add (phys_obj);
			}
		}
	}

	//
	//	Sort the lightning rods based on the angle they make with the tiberium
	//
	int rod_count = LightningRodList.Count ();
	if (rod_count > 0) {
		::qsort (&LightningRodList[0], rod_count, sizeof (DamageableStaticPhysClass *), fnSortLightningRodsCallback);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Create_Arc_Effects
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::Create_Arc_Effects (void)
{
	//
	//	Create the simple game object's that we'll use to display the lightning effect
	//
	for (int index = 0; index < ARC_OBJ_COUNT; index ++) {
		ArcObjects[index]	= (SimpleGameObj *)ObjectLibraryManager::Create_Object ("Arc Effect");
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Prepare_Arc_Effect_Data
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::Prepare_Arc_Effect_Data (void)
{
	//
	//	Create the simple game object's that we'll use to display the lightning effect
	//
	for (int index = 0; index < ARC_OBJ_COUNT; index ++) {
		if (	ArcObjects[index]->Peek_Physical_Object () != NULL &&
				ArcObjects[index]->Peek_Physical_Object ()->Peek_Model () != NULL)
		{
			ArcObjects[index]->Peek_Physical_Object ()->Peek_Model ()->Set_Hidden (true);
		}
		
		ArcLifeRemaining[index]	= 0;
	}

	//
	//	Get information about the model
	//
	SimpleGameObj *temp_obj = (SimpleGameObj *)ObjectLibraryManager::Create_Object ("Arc Effect");
	if (temp_obj != NULL && temp_obj->Peek_Model () != NULL) {
		RenderObjClass *model = temp_obj->Peek_Model ();

		//
		//	Store the original transforms of each bone
		//
		for (int index = 0; index < BONE_COUNT; index ++) {
			Bones[index] = model->Get_Bone_Transform (ARC_BONE_NAMES[index]);
		}

		//
		//	Lookup the original transform of the end-bone
		//
		int end_index		= model->Get_Bone_Index ("bone_end");		
		EndTM					= model->Get_Bone_Transform (end_index);
		
		//
		//	Get rid of the temporary object
		//
		temp_obj->Peek_Model ()->Set_Hidden (true);
		temp_obj->Set_Delete_Pending ();
	}

	return ;
}


//BBZZZT

///////////////////////////////////////////////////////////////////////////
//
//	Add_Lightning_Arc
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::Add_Lightning_Arc (const Vector3 &start_point, const Vector3 &end_point)
{
	//
	//	Try to find an available welding-arc
	//
	for (int index = 0; index < ARC_OBJ_COUNT; index ++) {
		if ((ArcLifeRemaining[index] <= 0)) {

			//
			//	Get the model of the object we'll be displaying
			//
			RenderObjClass *model = ArcObjects[index]->Peek_Physical_Object ()->Peek_Model ();

			//
			//	Make the object "look" at its endpoint
			//
			Matrix3D start_tm;
			start_tm.Obj_Look_At (start_point, end_point, 0);
			ArcObjects[index]->Peek_Physical_Object ()->Set_Transform (start_tm);

			//
			//	Now scale the bone's that control the length of the arc so it
			// will fit perfectly between the start and endpoints.
			//
			for (int bone_index = 0; bone_index < BONE_COUNT; bone_index ++) {
				
				float percent		= WWMath::Fabs (Bones[bone_index].Get_Translation ().X / EndTM.Get_Translation ().X);
				Vector3 new_pos	= start_point + (end_point - start_point) * percent;

				//
				//	Calculate the world space position of the bone
				//
				Vector3 world_space_pos		= start_tm * Bones[bone_index].Get_Translation ();
				Vector3 world_space_offset	= new_pos - world_space_pos;
				Matrix3D bone_tm (world_space_offset);
				
				//
				//	Control the bone
				//
				int bone_id = model->Get_Bone_Index (ARC_BONE_NAMES[bone_index]);
				model->Capture_Bone (bone_id);
				model->Control_Bone (bone_id, bone_tm, true);
			}

			//
			//	Calculate the world space position of the endpoint
			//
			Vector3 world_space_end_pos	= start_tm * EndTM.Get_Translation ();
			Vector3 world_space_offset		= end_point - world_space_end_pos;
			Matrix3D end_tm (world_space_offset);

			//
			//	Control the ending bone
			//
			int end_index = model->Get_Bone_Index ("bone_end");
			model->Capture_Bone (end_index);
			model->Control_Bone (end_index, end_tm, true);

			//
			//	Start the animation
			//
			ArcLifeRemaining[index] = 3.0F;
			HAnimClass *anim = model->Peek_Animation ();
			if (anim != NULL) {
				model->Set_Animation (anim, 0, RenderObjClass::ANIM_MODE_ONCE);
				ArcLifeRemaining[index] = anim->Get_Total_Time ();
			}

			//
			//	Show the model
			//
			model->Set_Hidden (false);
			break;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	fnSortLightningRodsCallback
//
////////////////////////////////////////////////////////////////
int __cdecl
RaveshawBossGameObjClass::fnSortLightningRodsCallback
(
	const void *elem1,
	const void *elem2
)
{
   WWASSERT (elem1 != NULL);
   WWASSERT (elem2 != NULL);
   DamageableStaticPhysClass *rod1 = *((DamageableStaticPhysClass **)elem1);
   DamageableStaticPhysClass *rod2 = *((DamageableStaticPhysClass **)elem2);

	//
	//	Sort the rods based on the angle they make around the tiberium
	//
	int result = 0;
	if (rod1 != rod2) {

		Vector3 rod1_pos;
		Vector3 rod2_pos;
		rod1->Get_Position (&rod1_pos);
		rod2->Get_Position (&rod2_pos);
		
		rod1_pos -= TIBERIUM_POS;
		rod2_pos -= TIBERIUM_POS;

		//
		//	Compute the angle assuming the tiberium to be the center of a circle
		//
		float angle1 = WWMath::Atan2 (rod1_pos.Y, rod1_pos.X);
		float angle2 = WWMath::Atan2 (rod2_pos.Y, rod2_pos.X);		
		angle1 = WWMath::Wrap (angle1 + DEG_TO_RADF (90.0F), 0.0F, DEG_TO_RADF (360.0F));
		angle2 = WWMath::Wrap (angle2 + DEG_TO_RADF (90.0F), 0.0F, DEG_TO_RADF (360.0F));

		//
		//	Sort based on this angle
		//
		if (angle1 < angle2) {
			result = -1;
		} else if (angle1 > angle2) {
			result = 1;
		} else {
			result = 0;
		}
	}

   return result;
}




///////////////////////////////////////////////////////////////////////////
//
//	Find_Death_Facing_Pos
//
///////////////////////////////////////////////////////////////////////////
void
RaveshawBossGameObjClass::Find_Death_Facing_Pos (Vector3 *facing_pos)
{
	float best_dist2 = 99999.0F;

	//
	//	Get the catwalk waypath
	//
	WaypathClass *waypath = PathfindClass::Get_Instance ()->Find_Waypath (CATWALK_WAYPATH_ID);
	WWASSERT (waypath != NULL);

	//
	//	Now find the closest point which is in-between the waypath points
	//
	for (int index = 0; index < waypath->Get_Point_Count (); index ++) {
		
		//
		//	Determine what the current and next waypath points are
		//
		int curr_index = index;
		int next_index = index + 1;
		if (index + 1 >= waypath->Get_Point_Count ()) {
			next_index = 0;
		}

		//
		//	Get the position of the current and next waypoint
		//
		Vector3 curr_waypath_pos = waypath->Get_Point (curr_index)->Get_Position ();
		Vector3 next_waypath_pos = waypath->Get_Point (next_index)->Get_Position ();
		
		//
		//	Evaluate this jump to position (is it the closest?)
		//
		Vector3 jump_to_pos	= curr_waypath_pos + ((next_waypath_pos - curr_waypath_pos) * 0.5F);
		Vector3 dir_vector	= (RaveshawPos - jump_to_pos);
		dir_vector.Z			= 0;
		float dist2				= dir_vector.Length2 ();
		if (dist2 < best_dist2) {
			best_dist2			= dist2;
			(*facing_pos)		= curr_waypath_pos;
		}
	}

	return ;
}
