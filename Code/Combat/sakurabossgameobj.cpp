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
 *                     $Archive:: /Commando/Code/Combat/sakurabossgameobj.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/16/02 6:01p                                               $*
 *                                                                                             *
 *                    $Revision:: 35                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "sakurabossgameobj.h"
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
#include "rbody.h"
#include "pscene.h"
#include "soldier.h"
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


DECLARE_FORCE_LINK (SakuraBoss)

//////////////////////////////////////////////////////////////////////////
//	General constants
//////////////////////////////////////////////////////////////////////////
static const char *	LEFT_ROCKET_BONE			= "BN_MISSILE_WNGL";
static const char *	RIGHT_ROCKET_BONE			= "BN_MISSILE_WNGR";
static const char *	ROCKET_DOOR_ANIMATION	= "V_COMMANCHE.A_COMM_MISL";
static const char *	GATLING_MUZZLE				= "MUZZLEA0";
static const char *	LEFT_ROCKET_MUZZLE		= "MUZZLEB01";
static const char *	RIGHT_ROCKET_MUZZLE		= "MUZZLEB0";
static const char *	LEFT_ROCKET_MESH			= "V_COM_MISSILEL";
static const char *	RIGHT_ROCKET_MESH			= "V_COM_MISSILER";
static float			ROCKETS_IN_FRAME_NUM		= 0.0F;
static float			ROCKETS_OUT_FRAME_NUM	= 10.0F;


//////////////////////////////////////////////////////////////////////////
//	Waypath IDs
//////////////////////////////////////////////////////////////////////////
const int WID_LONG_STRAFING_RUN		= 1700009;
const int WID_LONG_STRAFING_RUN2		= 1700016;

const int WID_TO_LIFT_AREA_SOUTH		= 1700018;
const int WID_TO_LIFT_AREA_NORTH		= 1700019;

const int WID_TIBFIELD_TO_VSTRAFE	= 1700004;
const int WID_LSTRAF2_TO_VSTRAFE		= 1700017;

const int WID_VSTRAFE_TO_TIBFIELD	= 1700006;
const int WID_LSTRAFE_TO_TIBFIELD	= 1700002;
const int WID_REFINERY_TO_TIBFIELD	= 1700012;

const int WID_LSTRAFE_TO_REFINERY	= 1700010;
const int WID_VSTRAFE_TO_REFINERY	= 1700014;
const int WID_PPLANT_TO_REFINERY		= 1700007;

const int WID_VSTRAFE_TO_PPLANT		= 1700015;
const int WID_REFINERY_TO_PPLANT		= 1700008;

const int WID_REFINERY_TO_LSTRAFE2	= 1700003;
const int WID_VSTRAFE_TO_LSTRAFE2	= 1700005;

const int WID_REFINERY_TO_LSTRAFE	= 1700011;
const int WID_VSTRAFE_TO_LSTRAFE		= 1700013;


//////////////////////////////////////////////////////////////////////////
//	Weapon and state enumerations
//////////////////////////////////////////////////////////////////////////
enum
{
	WEAPON_GATLING_GUN		= 0x01,
	WEAPON_ROCKETS_LEFT		= 0x02,
	WEAPON_ROCKETS_RIGHT		= 0x04,
	WEAPON_ALL					= WEAPON_GATLING_GUN | WEAPON_ROCKETS_LEFT | WEAPON_ROCKETS_RIGHT,
};

enum
{
	ROCKET_STATE_OPENING					= 1,
	ROCKET_STATE_OPEN,
	ROCKET_STATE_CLOSING,
	ROCKET_STATE_CLOSED,
	ROCKET_STATE_FIRING
};

enum
{
	GATLING_STATE_NORMAL					= 1,
	GATLING_STATE_REVVING_UP,
	GATLING_STATE_FIRING
};

enum
{
	SAKURA_STATE_NORMAL					= 1,
	SAKURA_STATE_TAUNTING,
};

enum
{
	VEHICLE_STATE_HOVERING				= 1,
	VEHICLE_STATE_MOVING,
};

enum
{
	STATE_CIRCLE_POWER_PLANT			= 1,
	STATE_CIRCLE_REFINERY,
	STATE_ATTACK_LIFT_AREA,
	STATE_LONG_STRAFING_RUN,
	STATE_LAND_IN_TIBERIUM_FIELD,
	STATE_VALLEY_STRAFE,
	STATE_LONG_STRAFING_RUN2,
	STATE_IN_TRANSITION
};

//////////////////////////////////////////////////////////////////////////
//	Weapon constants
//////////////////////////////////////////////////////////////////////////
enum
{
	ROCKET_RIGHT	= 0,
	ROCKET_LEFT		= 1
};


//////////////////////////////////////////////////////////////////////////
//	Taunt constants
//////////////////////////////////////////////////////////////////////////

const char *TAUNT_IDS[6] =
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
	CHUNKID_DEF_PARENT								=	0x09070458,
	CHUNKID_DEF_VARIABLES,
	CHUNKID_DEF_ROCKET_DEFENSEOBJ_DEF,

	VARID_DEF_GATLING_DEF_ID						= 1,
	VARID_DEF_ROCKET_DEF_ID,
	VARID_DEF_GATLING_REV_SOUND_ID,
	VARID_DEF_ROCKET_DOOR_SOUND_ID,
	VARID_DEF_ROCKET_DESTROYED_EXPLOSION_ID
};

enum
{
	CHUNKID_PARENT										=	0x09070459,
	CHUNKID_VARIABLES,
	CHUNKID_ROCKETL_DEFENSE_OBJ,
	CHUNKID_ROCKETR_DEFENSE_OBJ,
	CHUNKID_LAST_DAMAGER,
	CHUNKID_CURR_TARGET,
	CHUNKID_PILOT,
	CHUNKID_PATH,

	VARID_AVAILABLE_WEAPONS							= 1,
	VARID_OVERALL_STATE,							
	VARID_NEXT_OVERALL_STATE,					
	VARID_FACE_TARGET_IN_TRANSITION,			
	VARID_ROCKET_LAUNCHER_STATE,				
	VARID_GATTLING_GUN_STATE,					
	VARID_SAKURA_STATE,							
	VARID_VEHICLE_STATE,							
	VARID_MOVE_TO_LOCATION,						
	VARID_GATTLING_GUN_STATE_TIME_LEFT,		
	VARID_SAKURA_TAUNT_TIME_LEFT,				
	VARID_ROCKET_LAUNCHER_STATE_TIME_LEFT,	
	VARID_OVERALL_STATE_TIME_LEFT,			
	VARID_TARGET_TIME_LEFT,						
	VARID_CURRENT_HEALTH,						
	VARID_TARGET_ANGLE,							
	VARID_TARGET_POS,								
	VARID_IS_ATTACKING,							
	VARID_CURRENT_MUZZLE_TM,					
	VARID_CURRENT_MUZZLE_INDEX,				
	VARID_TILT_ANGLE,
	VARID_CHOPPER_TILT_BONE_INDEX
};

//////////////////////////////////////////////////////////////////////////
//	Factories
//////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<SakuraBossGameObjDef,		CHUNKID_GAME_OBJECT_DEF_SAKURA_BOSS>						_SakuraBossGameObjDefPersistFactory;
SimplePersistFactoryClass<SakuraBossGameObj,			CHUNKID_GAME_OBJECT_SAKURA_BOSS>								_SakuraBossGameObjPersistFactory;
DECLARE_DEFINITION_FACTORY(SakuraBossGameObjDef,	CLASSID_GAME_OBJECT_DEF_SAKURA_BOSS, "Sakura Boss")	_SakuraBossGameObjDefDefFactory;



//////////////////////////////////////////////////////////////////////////
//
//	SakuraBossGameObjDef
//
//////////////////////////////////////////////////////////////////////////
SakuraBossGameObjDef::SakuraBossGameObjDef (void)	:
	GattlingGunDefID (0),
	RocketLauncherDefID (0),
	GattlingGunRevSoundDefID (0),
	RocketDestroyedExplosionID (0),
	RocketDoorOpenSoundID (0)
{	
	PARAM_SEPARATOR (SakuraBossGameObjDef, "Rocket Launcher Defense Settings");
	DEFENSEOBJECTDEF_EDITABLE_PARAMS (SakuraBossGameObjDef, RocketsDefense);
	PARAM_SEPARATOR (SakuraBossGameObjDef, "");

	PARAM_SEPARATOR (SakuraBossGameObjDef, "Weapons");	
	EDITABLE_PARAM (SakuraBossGameObjDef, ParameterClass::TYPE_WEAPONOBJDEFINITIONID,	RocketLauncherDefID);
	EDITABLE_PARAM (SakuraBossGameObjDef, ParameterClass::TYPE_WEAPONOBJDEFINITIONID,	GattlingGunDefID);
	EDITABLE_PARAM (SakuraBossGameObjDef, ParameterClass::TYPE_SOUNDDEFINITIONID,			GattlingGunRevSoundDefID);	
	EDITABLE_PARAM (SakuraBossGameObjDef, ParameterClass::TYPE_SOUNDDEFINITIONID,			RocketDoorOpenSoundID);		
	EDITABLE_PARAM (SakuraBossGameObjDef, ParameterClass::TYPE_EXPLOSIONDEFINITIONID,	RocketDestroyedExplosionID);
	PARAM_SEPARATOR (SakuraBossGameObjDef, "");
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	~SakuraBossGameObjDef
//
//////////////////////////////////////////////////////////////////////////
SakuraBossGameObjDef::~SakuraBossGameObjDef (void)
{	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Get_Class_ID
//
//////////////////////////////////////////////////////////////////////////
uint32
SakuraBossGameObjDef::Get_Class_ID (void) const	
{ 
	return CLASSID_GAME_OBJECT_DEF_SAKURA_BOSS; 
}


//////////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////////
PersistClass *
SakuraBossGameObjDef::Create (void) const
{
	SakuraBossGameObj *obj = new SakuraBossGameObj;
	obj->Init(*this);
	return obj;
}


//////////////////////////////////////////////////////////////////////////
//
//	Save
//
//////////////////////////////////////////////////////////////////////////
bool
SakuraBossGameObjDef::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_DEF_PARENT);
		VehicleGameObjDef::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_DEF_VARIABLES);
		Save_Variables (csave);
	csave.End_Chunk();

	csave.Begin_Chunk (CHUNKID_DEF_ROCKET_DEFENSEOBJ_DEF);
		RocketsDefense.Save (csave);
	csave.End_Chunk();

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	Load
//
//////////////////////////////////////////////////////////////////////////
bool
SakuraBossGameObjDef::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		switch(cload.Cur_Chunk_ID ()) {

			case CHUNKID_DEF_PARENT:
				VehicleGameObjDef::Load (cload);
				break;

			case CHUNKID_DEF_VARIABLES:
				Load_Variables (cload);
				break;

			case CHUNKID_DEF_ROCKET_DEFENSEOBJ_DEF:
				RocketsDefense.Load (cload);
				break;
								
			default:
				Debug_Say (("Unrecognized SakuraBossGameObjDef chunkID\n"));
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
SakuraBossGameObjDef::Save_Variables (ChunkSaveClass &csave)
{
	WRITE_MICRO_CHUNK (csave, VARID_DEF_GATLING_DEF_ID,						GattlingGunDefID);
	WRITE_MICRO_CHUNK (csave, VARID_DEF_ROCKET_DEF_ID,							RocketLauncherDefID);
	WRITE_MICRO_CHUNK (csave, VARID_DEF_GATLING_REV_SOUND_ID,				GattlingGunRevSoundDefID);
	WRITE_MICRO_CHUNK (csave, VARID_DEF_ROCKET_DOOR_SOUND_ID,				RocketDoorOpenSoundID);	
	WRITE_MICRO_CHUNK (csave, VARID_DEF_ROCKET_DESTROYED_EXPLOSION_ID,	RocketDestroyedExplosionID);	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObjDef::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK (cload, VARID_DEF_GATLING_DEF_ID,						GattlingGunDefID);
			READ_MICRO_CHUNK (cload, VARID_DEF_ROCKET_DEF_ID,						RocketLauncherDefID);
			READ_MICRO_CHUNK (cload, VARID_DEF_GATLING_REV_SOUND_ID,				GattlingGunRevSoundDefID);
			READ_MICRO_CHUNK (cload, VARID_DEF_ROCKET_DOOR_SOUND_ID,				RocketDoorOpenSoundID);	
			READ_MICRO_CHUNK (cload, VARID_DEF_ROCKET_DESTROYED_EXPLOSION_ID,	RocketDestroyedExplosionID);

			default:
				Debug_Say (("Unrecognized SakuraBossGameObjDef Variable chunkID %d\n", cload.Cur_Micro_Chunk_ID ()));
				break;
		}

		cload.Close_Micro_Chunk ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
///////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
SakuraBossGameObjDef::Get_Factory (void) const
{ 
	return _SakuraBossGameObjDefPersistFactory; 
}


/*
**
**	Start of SakuraBossGameObj
**
*/


///////////////////////////////////////////////////////////////////////////
//
//	SakuraBossGameObj
//
///////////////////////////////////////////////////////////////////////////
SakuraBossGameObj::SakuraBossGameObj (void)	:
	GattlingGun (NULL),
	RockerLauncherLeft (NULL),
	RockerLauncherRight (NULL),
	AvailableWeapons (WEAPON_ALL),
	OverallState (STATE_ATTACK_LIFT_AREA),
	NextOverallState (STATE_ATTACK_LIFT_AREA),
	RocketLauncherState (ROCKET_STATE_CLOSED),
	GattlingGunState (GATLING_STATE_NORMAL),
	SakuraState (SAKURA_STATE_NORMAL),
	VehicleState (VEHICLE_STATE_HOVERING),
	GattlingGunStateTimeLeft (0),
	SakuraTauntTimeLeft (0),
	CurrentMuzzleIndex (0),
	RocketLauncherStateTimeLeft (0),
	OverallStateTimeLeft (10.0F),
	TargetTimeLeft (0),
	FaceTargetInTransition (false),
	CurrentHealth (1.0F),
	TargetAngle (0),
	IsAttacking (false),
	Path (NULL),
	ChopperTiltBoneIndex (0),
	TiltAngle (0),
	AvailableTaunts (0xFFFFFF)
{
	Path = new PathClass;

	Shuffle_Taunt_List ();
	TauntList[0] = 1;//IDS_SAKURA_BOSS_TAUNT1;
	TauntList[1] = 2;//IDS_SAKURA_BOSS_TAUNT2;
	TauntList[2] = 3;//IDS_SAKURA_BOSS_TAUNT3;
	TauntList[3] = 4;//IDS_SAKURA_BOSS_TAUNT4;
	TauntList[4] = 5;//IDS_SAKURA_BOSS_TAUNT5;
	TauntList[5] = 6;//IDS_SAKURA_BOSS_TAUNT6;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	~SakuraBossGameObj
//
///////////////////////////////////////////////////////////////////////////
SakuraBossGameObj::~SakuraBossGameObj (void)
{
	REF_PTR_RELEASE (Path);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
///////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
SakuraBossGameObj::Get_Factory (void) const 
{
	return _SakuraBossGameObjPersistFactory;
}


////////////////////////////////////////////////////////////////
//
//	Init
//
////////////////////////////////////////////////////////////////
void SakuraBossGameObj::Init( void )
{
	Init( Get_Definition() );
}

///////////////////////////////////////////////////////////////////////////
//
//	Init
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Init (const SakuraBossGameObjDef &definition)
{
	VehicleGameObj::Init (definition);

	//
	//	Initialize the defense for the left and right rocket launchers
	//
	// Pass a NULL for the owner so we don't think we have killed Sakura too early
	LeftRocketDefenseObject.Init (definition.RocketsDefense, NULL);
	RightRocketDefenseObject.Init (definition.RocketsDefense, NULL);

	//
	//	Lookup the weapon definitions
	//
	const WeaponDefinitionClass *gatling_gun_def			= WeaponManager::Find_Weapon_Definition (definition.GattlingGunDefID);
	const WeaponDefinitionClass *rocket_launcher_def	= WeaponManager::Find_Weapon_Definition (definition.RocketLauncherDefID);
	
	//
	//	Create the gatling gun
	//
	if (gatling_gun_def != NULL) {
		GattlingGun = new WeaponClass (gatling_gun_def);
		GattlingGun->Set_Owner (this);
		GattlingGun->Init_Muzzle_Flash (Peek_Physical_Object ()->Peek_Model ());
	}

	//
	//	Create the rocket launchers
	//
	if (rocket_launcher_def != NULL) {
		RockerLauncherLeft	= new WeaponClass (rocket_launcher_def);
		RockerLauncherRight	= new WeaponClass (rocket_launcher_def);
		RockerLauncherLeft->Set_Owner (this);
		RockerLauncherRight->Set_Owner (this);
		RockerLauncherLeft->Init_Muzzle_Flash (Peek_Physical_Object ()->Peek_Model ());
		RockerLauncherRight->Init_Muzzle_Flash (Peek_Physical_Object ()->Peek_Model ());
	}

	//
	//	Make sure we have an animation controller
	//
	if (Get_Anim_Control () == NULL) {
		Set_Anim_Control (new SimpleAnimControlClass);
		Get_Anim_Control ()->Set_Model (Peek_Model ());
	}
	
	//
	//	Make sure the engine is on
	//
	Enable_Engine (true);

	Pilot.Initialize (this);
	Pilot.Set_Arrived_Dist (1.0F);
	Pilot.Set_Aggressivness (1.0F);
	Pilot.Set_Is_Exact_Z_Important (true);

	//
	//	Lookup the chopper-tilting bone index
	//
	ChopperTiltBoneIndex = Peek_Model ()->Get_Bone_Index ("V_CHOPPER");
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Definition
//
///////////////////////////////////////////////////////////////////////////
const SakuraBossGameObjDef &
SakuraBossGameObj::Get_Definition (void) const
{
	return (const SakuraBossGameObjDef &)BaseGameObj::Get_Definition ();
}


///////////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////////
bool
SakuraBossGameObj::Save (ChunkSaveClass & csave)
{
	csave.Begin_Chunk (CHUNKID_PARENT);
		VehicleGameObj::Save (csave);
	csave.End_Chunk ();	

	csave.Begin_Chunk (CHUNKID_ROCKETL_DEFENSE_OBJ);
		LeftRocketDefenseObject.Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_ROCKETR_DEFENSE_OBJ);
		RightRocketDefenseObject.Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_LAST_DAMAGER);
		LastDamager.Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_CURR_TARGET);
		CurrentTarget.Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_PILOT);
		Pilot.Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);
		Save_Variables (csave);
	csave.End_Chunk ();

	if (Path != NULL) {
		csave.Begin_Chunk (CHUNKID_PATH);
			Path->Save (csave);
		csave.End_Chunk();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load
//
///////////////////////////////////////////////////////////////////////////
bool
SakuraBossGameObj::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		switch(cload.Cur_Chunk_ID ()) {

			case CHUNKID_PARENT:
				VehicleGameObj::Load (cload);
				break;

			case CHUNKID_ROCKETL_DEFENSE_OBJ:
				LeftRocketDefenseObject.Load (cload);
				break;

			case CHUNKID_ROCKETR_DEFENSE_OBJ:
				RightRocketDefenseObject.Load (cload);
				break;

			case CHUNKID_LAST_DAMAGER:
				LastDamager.Load (cload);
				break;

			case CHUNKID_CURR_TARGET:
				CurrentTarget.Load (cload);
				break;

			case CHUNKID_PILOT:
				Pilot.Load (cload);
				break;

			case CHUNKID_PATH:
			{
				Path = new PathClass;
				Path->Load (cload);
			}
			break;

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;

			default:
				Debug_Say (("Unrecognized SakuraBossGameObj chunk ID\n"));
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
SakuraBossGameObj::On_Post_Load (void)
{
	VehicleGameObj::On_Post_Load ();

	//
	//	Initialize the defense for the left and right rocket launchers
	//
	// Pass a NULL for the owner so we don't think we have killed Sakura too early
	//LeftRocketDefenseObject.Init (Get_Definition ().RocketsDefense, NULL);
	//RightRocketDefenseObject.Init (Get_Definition ().RocketsDefense, NULL);

	//
	//	Lookup the weapon definitions
	//
	const WeaponDefinitionClass *gatling_gun_def			= WeaponManager::Find_Weapon_Definition (Get_Definition ().GattlingGunDefID);
	const WeaponDefinitionClass *rocket_launcher_def	= WeaponManager::Find_Weapon_Definition (Get_Definition ().RocketLauncherDefID);
	
	//
	//	Create the gatling gun
	//
	if (gatling_gun_def != NULL) {
		GattlingGun = new WeaponClass (gatling_gun_def);
		GattlingGun->Set_Owner (this);
		GattlingGun->Init_Muzzle_Flash (Peek_Physical_Object ()->Peek_Model ());
	}

	//
	//	Create the rocket launchers
	//
	if (rocket_launcher_def != NULL) {
		RockerLauncherLeft	= new WeaponClass (rocket_launcher_def);
		RockerLauncherRight	= new WeaponClass (rocket_launcher_def);
		RockerLauncherLeft->Set_Owner (this);
		RockerLauncherRight->Set_Owner (this);
		RockerLauncherLeft->Init_Muzzle_Flash (Peek_Physical_Object ()->Peek_Model ());
		RockerLauncherRight->Init_Muzzle_Flash (Peek_Physical_Object ()->Peek_Model ());
	}

	//
	//	Make sure the engine is on
	//
	//Enable_Engine (true);

	//Pilot.Initialize (this);
	//Pilot.Set_Arrived_Dist (1.0F);
	//Pilot.Set_Aggressivness (1.0F);

	//
	//	Lookup the chopper-tilting bone index
	//
	ChopperTiltBoneIndex = Peek_Model ()->Get_Bone_Index ("V_CHOPPER");
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Save_Variables
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Save_Variables (ChunkSaveClass &csave)
{
	WRITE_MICRO_CHUNK (csave, VARID_AVAILABLE_WEAPONS,						AvailableWeapons);
	WRITE_MICRO_CHUNK (csave, VARID_OVERALL_STATE,							OverallState);
	WRITE_MICRO_CHUNK (csave, VARID_NEXT_OVERALL_STATE,					NextOverallState);
	WRITE_MICRO_CHUNK (csave, VARID_FACE_TARGET_IN_TRANSITION,			FaceTargetInTransition);
	WRITE_MICRO_CHUNK (csave, VARID_ROCKET_LAUNCHER_STATE,				RocketLauncherState);
	WRITE_MICRO_CHUNK (csave, VARID_GATTLING_GUN_STATE,					GattlingGunState);
	WRITE_MICRO_CHUNK (csave, VARID_SAKURA_STATE,							SakuraState);
	WRITE_MICRO_CHUNK (csave, VARID_VEHICLE_STATE,							VehicleState);
	WRITE_MICRO_CHUNK (csave, VARID_MOVE_TO_LOCATION,						MoveToLocation);
	WRITE_MICRO_CHUNK (csave, VARID_GATTLING_GUN_STATE_TIME_LEFT,		GattlingGunStateTimeLeft);
	WRITE_MICRO_CHUNK (csave, VARID_SAKURA_TAUNT_TIME_LEFT,				SakuraTauntTimeLeft);
	WRITE_MICRO_CHUNK (csave, VARID_ROCKET_LAUNCHER_STATE_TIME_LEFT,	RocketLauncherStateTimeLeft);
	WRITE_MICRO_CHUNK (csave, VARID_OVERALL_STATE_TIME_LEFT,				OverallStateTimeLeft);
	WRITE_MICRO_CHUNK (csave, VARID_TARGET_TIME_LEFT,						TargetTimeLeft);
	WRITE_MICRO_CHUNK (csave, VARID_CURRENT_HEALTH,							CurrentHealth);
	WRITE_MICRO_CHUNK (csave, VARID_TARGET_ANGLE,							TargetAngle);
	WRITE_MICRO_CHUNK (csave, VARID_TARGET_POS,								TargetPos);
	WRITE_MICRO_CHUNK (csave, VARID_IS_ATTACKING,							IsAttacking);
	WRITE_MICRO_CHUNK (csave, VARID_CURRENT_MUZZLE_TM,						CurrentMuzzleTM);
	WRITE_MICRO_CHUNK (csave, VARID_CURRENT_MUZZLE_INDEX,					CurrentMuzzleIndex);
	WRITE_MICRO_CHUNK (csave, VARID_TILT_ANGLE,								TiltAngle);
	WRITE_MICRO_CHUNK (csave, VARID_CHOPPER_TILT_BONE_INDEX,				ChopperTiltBoneIndex);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK (cload, VARID_AVAILABLE_WEAPONS,						AvailableWeapons);
			READ_MICRO_CHUNK (cload, VARID_OVERALL_STATE,							OverallState);
			READ_MICRO_CHUNK (cload, VARID_NEXT_OVERALL_STATE,						NextOverallState);
			READ_MICRO_CHUNK (cload, VARID_FACE_TARGET_IN_TRANSITION,			FaceTargetInTransition);
			READ_MICRO_CHUNK (cload, VARID_ROCKET_LAUNCHER_STATE,					RocketLauncherState);
			READ_MICRO_CHUNK (cload, VARID_GATTLING_GUN_STATE,						GattlingGunState);
			READ_MICRO_CHUNK (cload, VARID_SAKURA_STATE,								SakuraState);
			READ_MICRO_CHUNK (cload, VARID_VEHICLE_STATE,							VehicleState);
			READ_MICRO_CHUNK (cload, VARID_MOVE_TO_LOCATION,						MoveToLocation);
			READ_MICRO_CHUNK (cload, VARID_GATTLING_GUN_STATE_TIME_LEFT,		GattlingGunStateTimeLeft);
			READ_MICRO_CHUNK (cload, VARID_SAKURA_TAUNT_TIME_LEFT,				SakuraTauntTimeLeft);
			READ_MICRO_CHUNK (cload, VARID_ROCKET_LAUNCHER_STATE_TIME_LEFT,	RocketLauncherStateTimeLeft);
			READ_MICRO_CHUNK (cload, VARID_OVERALL_STATE_TIME_LEFT,				OverallStateTimeLeft);
			READ_MICRO_CHUNK (cload, VARID_TARGET_TIME_LEFT,						TargetTimeLeft);
			READ_MICRO_CHUNK (cload, VARID_CURRENT_HEALTH,							CurrentHealth);
			READ_MICRO_CHUNK (cload, VARID_TARGET_ANGLE,								TargetAngle);
			READ_MICRO_CHUNK (cload, VARID_TARGET_POS,								TargetPos);
			READ_MICRO_CHUNK (cload, VARID_IS_ATTACKING,								IsAttacking);
			READ_MICRO_CHUNK (cload, VARID_CURRENT_MUZZLE_TM,						CurrentMuzzleTM);
			READ_MICRO_CHUNK (cload, VARID_CURRENT_MUZZLE_INDEX,					CurrentMuzzleIndex);
			READ_MICRO_CHUNK (cload, VARID_TILT_ANGLE,								TiltAngle);
			READ_MICRO_CHUNK (cload, VARID_CHOPPER_TILT_BONE_INDEX,				ChopperTiltBoneIndex);

			default:
				Debug_Say (("Unrecognized SakuraBossGameObj Variable chunkID %d\n", cload.Cur_Micro_Chunk_ID ()));
				break;
		}

		cload.Close_Micro_Chunk ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Apply_Control
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Apply_Control (void)
{
	VehicleGameObj::Apply_Control ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Update_Decision_Data
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Update_Decision_Data (void)
{
	//
	//	Update our current health
	//
	DefenseObjectClass *defense_object = Get_Defense_Object ();
	if (defense_object != NULL) {		
		CurrentHealth = defense_object->Get_Health () / defense_object->Get_Health_Max ();
	}

	//
	//	Determine if any of our weapons are currently attacking the player
	//
	IsAttacking = false;
	if (	RocketLauncherState != ROCKET_STATE_CLOSED ||
			GattlingGunState != GATLING_STATE_NORMAL)
	{
		IsAttacking = true;
	}

	//
	//	Update information about our current target
	//
	TargetAngle = 0;
	TargetPos.Set (0, 0, 0);
	if (CurrentTarget != NULL) {

		//
		//	Get the target's position
		//
		CurrentTarget.Get_Ptr ()->Get_Position (&TargetPos);

		//
		//	Get the target's position relative to ours
		//
		Vector3 obj_space_target;
		Matrix3D::Inverse_Transform_Vector (Get_Transform (), TargetPos, &obj_space_target);

		//
		//	What angle is the target off from our current facing?
		//
		TargetAngle = WWMath::Atan2 (obj_space_target.Y, obj_space_target.X);
		TargetAngle	= WWMath::Wrap (TargetAngle, DEG_TO_RADF (-180), DEG_TO_RADF (180));
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Think
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Think (void)
{
	Pilot.Think ();
	VehicleGameObj::Think ();

	WWPROFILE( "Sakura Think" );

	//
	//	Find a target if necessary
	//
	Update_Target ();

	//
	//	Update our 'snapshot' of the world's (and our) state
	//
	Update_Decision_Data ();

	//
	//	Update the individual parts that make-up the boss
	//
	Update_Rocket_State ();
	Update_Gattling_Gun_State ();
	Update_Sakura_State ();
	Update_Vehicle_State ();
	Update_Tilt ();

	//
	//	Now, look at the bigger picture and determine
	// what each part should be doing
	//
	Decide_New_Overall_State ();
	Update_Overall_State ();

	//
	//	Update the weapons
	//
	if (RockerLauncherLeft != NULL) {
		CurrentMuzzleIndex = ROCKET_LEFT;
		RockerLauncherLeft->Update ();
		RockerLauncherLeft->Set_Total_Rounds (1000);
	}

	if (RockerLauncherRight != NULL) {
		CurrentMuzzleIndex = ROCKET_RIGHT;
		RockerLauncherRight->Update ();
		RockerLauncherRight->Set_Total_Rounds (1000);
	}

	if (GattlingGun != NULL) {
		GattlingGun->Update ();
		GattlingGun->Set_Total_Rounds (1000);
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Request_Overall_State
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Request_Overall_State (int new_state)
{
	if (OverallState != STATE_IN_TRANSITION && OverallState != new_state) {

		switch (new_state)
		{
			case STATE_CIRCLE_POWER_PLANT:

				if (	OverallState == STATE_CIRCLE_REFINERY ||
						OverallState == STATE_ATTACK_LIFT_AREA)
				{					
					Do_Waypath (WID_REFINERY_TO_PPLANT);
					Set_Overall_State (STATE_IN_TRANSITION);

				} else if (	OverallState == STATE_LONG_STRAFING_RUN || 
								OverallState == STATE_LAND_IN_TIBERIUM_FIELD ||
								OverallState == STATE_LONG_STRAFING_RUN2)
				{
					Set_Overall_State (STATE_CIRCLE_POWER_PLANT);

				} else if (OverallState == STATE_VALLEY_STRAFE) {

					Do_Waypath (WID_VSTRAFE_TO_PPLANT);
					Set_Overall_State (STATE_IN_TRANSITION);
				}
				
				break;

			case STATE_CIRCLE_REFINERY:

				if (	OverallState == STATE_CIRCLE_POWER_PLANT ||
						OverallState == STATE_LAND_IN_TIBERIUM_FIELD ||
						OverallState == STATE_LONG_STRAFING_RUN2)
				{
					Do_Waypath (WID_PPLANT_TO_REFINERY);
					Set_Overall_State (STATE_IN_TRANSITION);

				} else if (OverallState == STATE_ATTACK_LIFT_AREA) {
					
					Set_Overall_State (STATE_CIRCLE_REFINERY);
				
				} else if (OverallState == STATE_VALLEY_STRAFE) {
					
					Do_Waypath (WID_VSTRAFE_TO_REFINERY);
					Set_Overall_State (STATE_IN_TRANSITION);

				} else if (OverallState == STATE_LONG_STRAFING_RUN) {
					
					Do_Waypath (WID_LSTRAFE_TO_REFINERY);
					Set_Overall_State (STATE_IN_TRANSITION);
				}

				break;

			case STATE_LAND_IN_TIBERIUM_FIELD:

				if (	OverallState == STATE_CIRCLE_REFINERY ||
						OverallState == STATE_ATTACK_LIFT_AREA) {

					Do_Waypath (WID_REFINERY_TO_TIBFIELD);
					Set_Overall_State (STATE_IN_TRANSITION);

				} else if (	OverallState == STATE_CIRCLE_POWER_PLANT ||
								OverallState == STATE_LONG_STRAFING_RUN2)
				{
					Set_Overall_State (STATE_LAND_IN_TIBERIUM_FIELD);					

				} else if (OverallState == STATE_LONG_STRAFING_RUN) {

					Do_Waypath (WID_LSTRAFE_TO_TIBFIELD);
					Set_Overall_State (STATE_IN_TRANSITION);

				} else if (OverallState == STATE_VALLEY_STRAFE) {

					Do_Waypath (WID_VSTRAFE_TO_TIBFIELD);
					Set_Overall_State (STATE_IN_TRANSITION);
				}

				break;

			case STATE_VALLEY_STRAFE:

				if (	OverallState == STATE_CIRCLE_REFINERY ||
						OverallState == STATE_CIRCLE_POWER_PLANT ||
						OverallState == STATE_LONG_STRAFING_RUN)
				{
					Set_Overall_State (STATE_VALLEY_STRAFE);

				} else if (OverallState == STATE_ATTACK_LIFT_AREA) {

					Set_Overall_State (STATE_VALLEY_STRAFE);

				} else if (OverallState == STATE_LONG_STRAFING_RUN2) {

					Do_Waypath (WID_LSTRAF2_TO_VSTRAFE);
					Set_Overall_State (STATE_IN_TRANSITION);

				} else if (OverallState == STATE_LAND_IN_TIBERIUM_FIELD) {
										
					Do_Waypath (WID_TIBFIELD_TO_VSTRAFE);
					Set_Overall_State (STATE_IN_TRANSITION);
				}

				break;

			case STATE_ATTACK_LIFT_AREA:

				if (	OverallState == STATE_CIRCLE_REFINERY ||
						OverallState == STATE_LONG_STRAFING_RUN ||
						OverallState == STATE_VALLEY_STRAFE)
				{
					Do_Waypath (WID_TO_LIFT_AREA_SOUTH);
					Set_Overall_State (STATE_IN_TRANSITION);

				} else if (	OverallState == STATE_CIRCLE_POWER_PLANT ||
								OverallState == STATE_LONG_STRAFING_RUN2 ||
								OverallState == STATE_LAND_IN_TIBERIUM_FIELD)
				{										
					Do_Waypath (WID_TO_LIFT_AREA_NORTH);
					Set_Overall_State (STATE_IN_TRANSITION);
				}

				break;
			
			case STATE_LONG_STRAFING_RUN2:

				if (	OverallState == STATE_CIRCLE_REFINERY ||
						OverallState == STATE_ATTACK_LIFT_AREA) {
					
					Do_Waypath (WID_REFINERY_TO_LSTRAFE2);
					Set_Overall_State (STATE_IN_TRANSITION);
				
				} else if (OverallState == STATE_VALLEY_STRAFE) {

					Do_Waypath (WID_VSTRAFE_TO_LSTRAFE2);
					Set_Overall_State (STATE_IN_TRANSITION);

				} else if (	OverallState == STATE_CIRCLE_POWER_PLANT ||
								OverallState == STATE_LAND_IN_TIBERIUM_FIELD ||
								OverallState == STATE_LONG_STRAFING_RUN)
				{
					Set_Overall_State (STATE_LONG_STRAFING_RUN2);
				}

				break;

			case STATE_LONG_STRAFING_RUN:

				if (	OverallState == STATE_CIRCLE_REFINERY ||
						OverallState == STATE_ATTACK_LIFT_AREA)
				{					
					Do_Waypath (WID_REFINERY_TO_LSTRAFE);
					Set_Overall_State (STATE_IN_TRANSITION);

				} else if (OverallState == STATE_VALLEY_STRAFE) {

					Do_Waypath (WID_VSTRAFE_TO_LSTRAFE);
					Set_Overall_State (STATE_IN_TRANSITION);

				} else if (	OverallState == STATE_CIRCLE_POWER_PLANT ||
								OverallState == STATE_LAND_IN_TIBERIUM_FIELD ||
								OverallState == STATE_LONG_STRAFING_RUN2)
				{
					Set_Overall_State (STATE_LONG_STRAFING_RUN);
				}

				break;
		}
		
		NextOverallState = new_state;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Set_Overall_State
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Set_Overall_State (int new_state)
{
	if (OverallState != new_state) {

		switch (new_state)
		{
			case STATE_CIRCLE_POWER_PLANT:				
				Pilot.Set_Destination (Vector3 (-63.34F, 21.26F, 26.0F));
				Pilot.Set_Mode (PilotClass::MODE_CIRCLE_POINT);
				Pilot.Set_Circle_Bounds (DEG_TO_RADF (100), DEG_TO_RADF (225));
				Pilot.Set_Circle_Dist (30);				
				break;

			case STATE_CIRCLE_REFINERY:
			{
				Vector3 destination (-166.0F, 40.13F, 42.23F);
				Pilot.Set_Destination (destination);
				Pilot.Set_Mode (PilotClass::MODE_HOVER);
				destination += Vector3 (0, -10, 0);
				Pilot.Set_Target (&destination);
			}
			break;

			case STATE_ATTACK_LIFT_AREA:
			{
				Vector3 destination (-161.78F, 79.03F, 18.88F);
				Pilot.Set_Destination (destination);
				Pilot.Set_Mode (PilotClass::MODE_HOVER);

				destination.X += 10;
				Pilot.Set_Target (&destination);
			}
			break;

			case STATE_LONG_STRAFING_RUN2:
				Pilot.Set_Max_Speed (100.0F);
				Do_Waypath (WID_LONG_STRAFING_RUN2);
				break;

			case STATE_LONG_STRAFING_RUN:
				Pilot.Set_Max_Speed (100.0F);
				Do_Waypath (WID_LONG_STRAFING_RUN);
				break;

			case STATE_VALLEY_STRAFE:
			{
				Vector3 destination (-84.33F, 77.99F, 26.77F);
				Pilot.Set_Destination (destination);
				Pilot.Set_Mode (PilotClass::MODE_HOVER);
			}
			break;

				
			case STATE_LAND_IN_TIBERIUM_FIELD:
			{
				Vector3 destination (-107.0F, -10.0F, 15.0F);
				Pilot.Set_Destination (destination);
				Pilot.Set_Mode (PilotClass::MODE_HOVER);
			}
			break;

			case STATE_IN_TRANSITION:
				FaceTargetInTransition = false;
				break;
		}
		
		OverallState = new_state;

		//
		//	Force a new state change in a little while
		//
		OverallStateTimeLeft = WWMath::Random_Float (5.0F, 10.0F);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Should_Change_Overall_State
//
///////////////////////////////////////////////////////////////////////////
bool
SakuraBossGameObj::Should_Change_Overall_State (void)
{
	if (OverallState == STATE_IN_TRANSITION || VehicleState == VEHICLE_STATE_MOVING) {
		return false;
	}

	bool retval = false;

	//
	//	We need to change states if our timer has told us to...
	//
	OverallStateTimeLeft -= TimeManager::Get_Frame_Seconds ();
	if (OverallStateTimeLeft <= 0) {
		retval = true;
	} else if (CurrentTarget != NULL) {
		
		switch (OverallState)
		{
			case STATE_CIRCLE_POWER_PLANT:

				if (TargetPos.Z < 12.0F) {
					retval = true;
				}

				break;

			case STATE_CIRCLE_REFINERY:

				if (TargetPos.X > -91.0F || TargetPos.Y > 69.0F) {
					retval = true;
				}

				break;

			case STATE_ATTACK_LIFT_AREA:

				if (TargetPos.X > -100.0F || TargetPos.Y < 50.0F) {
					retval = true;
				}

				break;

			case STATE_LAND_IN_TIBERIUM_FIELD:

				if (TargetPos.X > -119.0F && TargetPos.X < -95.0F && TargetPos.Y < 0.0F) {
					retval = true;
				}

				break;
			
			case STATE_VALLEY_STRAFE:

				if (TargetPos.Y < 20.0F && (TargetPos.X < -138.0F || TargetPos.X > -78.0F)) {
					retval = true;
				}

				break;

			case STATE_LONG_STRAFING_RUN:

				if (TargetPos.X > -100.0F && TargetPos.Y > 35.0F) {
					retval = true;
				}

				break;

			case STATE_LONG_STRAFING_RUN2:
				
				if (TargetPos.X > -80.0F || TargetPos.Y < 10.0F) {
					retval = true;
				}
				
				break;
			
		}
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Decide_New_Overall_State
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Decide_New_Overall_State (void)
{
	//
	//	Do we need to change state now?
	//	
	if (Should_Change_Overall_State ()) {

		//
		//	If we are going to change states, then stop attacking the target
		//
		Stop_Attacking ();
		
		//
		//	If we've been injured enough, bump up the long strafing run possibility
		//
		if (CurrentHealth < 0.45F && FreeRandom.Get_Int (4) == 1) {

			//
			//	Flip a coin and do a strafing run
			//
			if (FreeRandom.Get_Int (2) == 0) {
				Request_Overall_State (STATE_LONG_STRAFING_RUN);
			} else {
				Request_Overall_State (STATE_LONG_STRAFING_RUN2);
			}

		} else {

			//
			//	Based on this position determine which state we want to go to...
			//
			if (	TargetPos.X < -104.0F &&
					TargetPos.Y > 69.0F && TargetPos.Y < 85.8F)
			{
				Request_Overall_State (STATE_ATTACK_LIFT_AREA);

			} else if (	OverallState != STATE_CIRCLE_REFINERY &&
							TargetPos.X < -125.0F && TargetPos.Y > -19.0F) {

				Request_Overall_State (STATE_CIRCLE_REFINERY);

			} else if (	OverallState != STATE_CIRCLE_POWER_PLANT &&
							TargetPos.X > -84.0F && TargetPos.Z > 11.0F) {

				Request_Overall_State (STATE_CIRCLE_POWER_PLANT);

			} else if (	OverallState != STATE_LAND_IN_TIBERIUM_FIELD &&
							TargetPos.Y < 39.0F) {

				Request_Overall_State (STATE_LAND_IN_TIBERIUM_FIELD);

			} else if (OverallState != STATE_VALLEY_STRAFE) {

				Request_Overall_State (STATE_VALLEY_STRAFE);

			} else if (FreeRandom.Get_Int (5) == 1) {
				
				//
				//	Flip a coin and do a strafing run
				//
				if (FreeRandom.Get_Int (2) == 0) {
					Request_Overall_State (STATE_LONG_STRAFING_RUN);
				} else {
					Request_Overall_State (STATE_LONG_STRAFING_RUN2);
				}
			}
		}

		OverallStateTimeLeft = WWMath::Random_Float (5.0F, 10.0F);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Update_Overall_State
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Update_Overall_State (void)
{

	switch (OverallState)
	{
		case STATE_CIRCLE_POWER_PLANT:

			if (CurrentTarget != NULL) {
				Pilot.Set_Circle_Pos (TargetPos);
			} else {
				Pilot.Set_Target (NULL);
			}

			//
			//	Use the gattling guns on the player
			//
			Attack_Target (0, 1);
			break;

		case STATE_CIRCLE_REFINERY:

			if (CurrentTarget != NULL) {
				Vector3 destination (-166.0F, 40.13F, 42.23F);

				bool target_player = true;

				//
				//	Should the commanche 'face' the player or look straight ahead
				//
				if (TargetPos.Y < 21.0F && TargetPos.X < -151.0F) {
					target_player = false;
				}

				//
				// Have the commanche strafe by the player
				//
				if (TargetPos.X > -140.0F && TargetPos.Y > 34.0F) {
					
					destination.X += 10.0F;
					destination.Y = TargetPos.Y + 1.0F;
					destination.Y = min (destination.Y, 78.0F);
					destination.Z -= 20.0F;

				} else {
					
					if (target_player || TargetPos.X > -151.0F) {					
						destination.X += 20.0F;
						destination.Z -= 20.0F;
					}

					//
					//	Let the commanche move backwards (if necessary)
					//					
					if (TargetPos.Y > -6.0F) {
						destination.Y = TargetPos.Y + 46.0F;
						destination.Y = min (destination.Y, 78.0F);
					}
				}

				//
				//	Don't let the commanche dip below the player's level
				//
				destination.Z = max (TargetPos.Z + 10.0F, destination.Z);

				Pilot.Set_Destination (destination);
				Pilot.Set_Mode (PilotClass::MODE_HOVER);
				
				if (target_player) {
					
					Pilot.Set_Target (&TargetPos);

				} else {
					
					Vector3 position;
					Get_Position (&position);
					position += Vector3 (0, -10, 0);
					Pilot.Set_Target (&position);
				}

				//
				//	Start attacking the player if we are within 4 meters of
				// our expected destination
				//				
				Vector3 curr_position;
				Get_Position (&curr_position);
				if ((destination - curr_position).Length2 () < 16) {
					
					//
					//	Use the rockets to attack the player
					//				
					Attack_Target (1, 0);
				}

			} else {
				Pilot.Set_Target (NULL);
			}

			break;

		case STATE_ATTACK_LIFT_AREA:
		{
			Vector3 destination (-161.78F, 79.03F, 18.88F);
			destination.Z = max (TargetPos.Z + 7.0F, 18.88F);
			Pilot.Set_Destination (destination);				

			destination.X += 10;
			Pilot.Set_Target (&destination);

			//
			//	Make sure we are attacking the player
			//
			if (IsAttacking == false) {

				//
				//	Attack with the gattling gun if the player is close,
				// otherwise attack with the rockets
				//
				if (TargetPos.X > -130.0F) {
					Attack_Target (1, 0);
				} else {
					Attack_Target (0, 1);
				}				
			}
		}
		break;

		case STATE_LONG_STRAFING_RUN2:
		case STATE_LONG_STRAFING_RUN:
		{
			bool should_fire = false;

			//
			//	Determine if we should start firing or not...
			// We should fire on the target if they are in front of us
			//
			if (CurrentTarget != NULL) {
				
				//
				//	Transform the target into our coordinate system.  Is the target in front of us?
				//
				if (TargetAngle > DEG_TO_RADF (-75) && TargetAngle < DEG_TO_RADF (75)) {

					Vector3 curr_position;
					Get_Position (&curr_position);

					//
					//	Are we inside the valley?  Note:  We don't want to fire if we have
					// no chance of hitting the target
					//
					if (	curr_position.Y >= -40.0F && curr_position.Y <= 140.0F &&
							curr_position.X >= -178.0F)
					{
						should_fire = true;
					}
				}

			} else {
				Pilot.Set_Target (NULL);
			}

			//
			//	Turn the gattling guns on/off
			//
			if (should_fire) {
				Set_Gattling_Gun_State (GATLING_STATE_FIRING);
			} else {
				Set_Gattling_Gun_State (GATLING_STATE_NORMAL);
			}

			//
			//	Restore the vehicle's speed when we've finished moving
			//
			if (VehicleState != VEHICLE_STATE_MOVING) {

				//
				//	Target the player
				//
				Pilot.Set_Target (&TargetPos);
				
				//
				//	Restore the speed
				//
				Pilot.Set_Max_Speed (20.0F);
			}
		}
		break;
		
		case STATE_VALLEY_STRAFE:

			if (CurrentTarget != NULL) {
				Vector3 destination (-84.33F, 77.99F, 26.77F);

				if (TargetPos.Y < 42.0F) {
					
					//
					//	Simply match the player's position along our strafe path
					//
					destination.X = WWMath::Clamp (TargetPos.X, -143.0F, -84.33F);				
				} else {
					
					//
					//	Try to stay targetted on the player by strafing along the path
					//
					if (TargetPos.X > -113.0F) {
						destination.X = WWMath::Clamp (TargetPos.X - 35.0F, -143.0F, -84.33F);
					} else {
						destination.X = WWMath::Clamp (TargetPos.X + 35.0F, -143.0F, -84.33F);
					}									
				}

				Pilot.Set_Destination (destination);
				Pilot.Set_Mode (PilotClass::MODE_HOVER);				
				Pilot.Set_Target (&TargetPos);

				//
				//	Start attacking the player if we are within 4 meters of
				// our expected destination
				//				
				Vector3 curr_position;
				Get_Position (&curr_position);
				if ((destination - curr_position).Length2 () < 16) {
					
					//
					//	Make sure we are attacking the player (with either weapon)
					//
					Attack_Target (1, 1);
				}

			} else {
				Pilot.Set_Target (NULL);
			}
			
			break;

		case STATE_LAND_IN_TIBERIUM_FIELD:
		{

			if (CurrentTarget != NULL) {			
				Pilot.Set_Target (&TargetPos);
			} else {
				Pilot.Set_Target (NULL);
			}


			//
			//	Start attacking the player if we are within 4 meters of
			// our expected destination
			//				
			Vector3 curr_position;
			Vector3 destination = Pilot.Get_Destination ();
			Get_Position (&curr_position);
			if ((destination - curr_position).Length2 () < 16) {
				
				//
				//	Attack the player with either weapon
				//
				Attack_Target (1, 1);
			}
		}
		break;

		case STATE_IN_TRANSITION:

			//
			//	If we've finished moving, then we are done with the transition
			// and are ready to move on to the new state.
			//
			if (VehicleState != VEHICLE_STATE_MOVING) {
				Set_Overall_State (NextOverallState);
			} else if (FaceTargetInTransition) {

				if (CurrentTarget != NULL) {					
					Pilot.Set_Target (&TargetPos);
				} else {
					Pilot.Set_Target (NULL);
				}				
			}

			break;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Update_Tilt
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Update_Tilt (void)
{
	const float TILT_RATE = DEG_TO_RADF (25.0F);

	//
	//	Lookup the model we are going to tilt
	//
	RenderObjClass *model = Peek_Model ();
	WWASSERT (model != NULL);

	if (ChopperTiltBoneIndex > 0) {
		
		//
		//	Determine what the maximum tilt allowed is...
		//
		float max_tilt = DEG_TO_RADF (35.0F);
		if (OverallState == STATE_LAND_IN_TIBERIUM_FIELD) {
			max_tilt = DEG_TO_RADF (10.0F);
		}

		//
		//	Decide if we should be in a tilted or normal state
		//
		if (OverallState != STATE_IN_TRANSITION) {

			//
			//	Determine how far we need to tile to look straight at the player
			//			
			float target_angle = DEG_TO_RADF (25.0F);
			if (CurrentTarget != NULL) {

				//
				//	Get the target's position relative to ours
				//
				Matrix3D untilted_tm (1);
				untilted_tm.Rotate_Z (Get_Transform ().Get_Z_Rotation ());
				untilted_tm.Set_Translation (Get_Transform ().Get_Translation ());
				Vector3 obj_space_target;
				Matrix3D::Inverse_Transform_Vector (untilted_tm, TargetPos, &obj_space_target);

				//
				//	How far do we need to tilt to be facing the player?
				//
				target_angle = -WWMath::Atan2 (obj_space_target.Z, obj_space_target.X);
				target_angle = WWMath::Clamp (target_angle, -max_tilt, max_tilt);
			}

			//
			//	Increate our tilt percent
			//
			if (TiltAngle < target_angle) {
				TiltAngle += TimeManager::Get_Frame_Seconds () * TILT_RATE;
				TiltAngle = min (TiltAngle, target_angle);
			} else {
				TiltAngle -= TimeManager::Get_Frame_Seconds () * TILT_RATE;
				TiltAngle = max (TiltAngle, target_angle);
			}

			//
			//	Check to see if we should start the tilting process
			//
			if (!model->Is_Bone_Captured (ChopperTiltBoneIndex)) {
				model->Capture_Bone (ChopperTiltBoneIndex);
				TiltAngle = 0;
			}

		} else {

			//
			//	Decrease our tilt percent
			//
			TiltAngle -= (TimeManager::Get_Frame_Seconds () * TILT_RATE);
			TiltAngle = max (TiltAngle, 0.0F);

			//
			//	Check to see if we should release our control of the bone
			//
			if (TiltAngle == 0 && model->Is_Bone_Captured (ChopperTiltBoneIndex)) {
				model->Release_Bone (ChopperTiltBoneIndex);
			}
		}

		//
		//	Make sure our percent is clamped between 0 and 1
		//
		//TiltPercent = WWMath::Clamp (TiltPercent, 0, 1.0F);

		//
		//	Do we need to tilt the bone?
		//
		if (model->Is_Bone_Captured (ChopperTiltBoneIndex)) {
			
			//
			//	Tilt the model up to 25 degrees
			//
			Matrix3D bone_tm (1);
			bone_tm.Rotate_Y (TiltAngle);
			model->Control_Bone (ChopperTiltBoneIndex, bone_tm);
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Attack_Target
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Attack_Target
(
	float rocket_priority,
	float gattling_priority
)
{
	//
	//	Don't attack if we don't have a target, or we are already attacking
	//
	if (CurrentTarget == NULL || IsAttacking || SakuraState != SAKURA_STATE_NORMAL) {
		return ;
	}

	//
	//	There's a 1 in 6 chance that Sakura will taunt the player
	// instead of attacking
	//
	if (FreeRandom.Get_Int (4) == 2) {
		Set_Sakura_State (SAKURA_STATE_TAUNTING);
	} else {

		//
		//	If both rockets are gone, then decrease its priority so
		// we won't choose them
		//
		if (	((AvailableWeapons & WEAPON_ROCKETS_LEFT) == 0) &&
				((AvailableWeapons & WEAPON_ROCKETS_RIGHT) == 0))
		{
			rocket_priority = gattling_priority - 1.0F;
		}

		//
		//	Decide which weapon to use
		//
		if (rocket_priority > gattling_priority) {		
			Set_Rocket_State (ROCKET_STATE_OPENING);
		} else if (gattling_priority > rocket_priority) {		
			Set_Gattling_Gun_State (GATLING_STATE_REVVING_UP);
		} else {
			
			//
			//	Priorities are equal, so toss a coin and pick a weapon
			//
			if (FreeRandom.Get_Int (2) == 1) {
				Set_Rocket_State (ROCKET_STATE_OPENING);
			} else {
				Set_Gattling_Gun_State (GATLING_STATE_REVVING_UP);
			}
		}
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Stop_Attacking
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Stop_Attacking (void)
{
	//
	//	Force the rockets closed
	//
	if (RocketLauncherState != ROCKET_STATE_CLOSED) {
		Set_Rocket_State (ROCKET_STATE_CLOSING);
	}

	//
	//	Force the gattling gun to stop
	//
	if (GattlingGunState != GATLING_STATE_NORMAL) {
		Set_Gattling_Gun_State (GATLING_STATE_NORMAL);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Set_Vehicle_State
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Set_Vehicle_State (int new_state)
{
	if (VehicleState != new_state) {
		
		switch (new_state)
		{
			case VEHICLE_STATE_HOVERING:
			case VEHICLE_STATE_MOVING:
				break;
		}

		VehicleState = new_state;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Update_Vehicle_State
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Update_Vehicle_State (void)
{
	//
	//	If we have a current target, then check to make sure
	// we are pointing all our weapons at that target
	//
	if (TargetAngle > DEG_TO_RADF (-90) && TargetAngle < DEG_TO_RADF (90)) {
		Set_Targeting (TargetPos, true);
	} else {
		Vector3 target_pos = Get_Transform () * Vector3 (100.0F, 0, 0) ;
		Set_Targeting (target_pos, true);
	}

	switch (VehicleState)
	{		
		case VEHICLE_STATE_HOVERING:
			break;

		case VEHICLE_STATE_MOVING:
		{
			//
			//	Get the next point on the path
			//			
			Vector3 curr_pos;
			Vector3 next_pos;
			Get_Position (&curr_pos);

			const AABoxClass &bounding_box = Peek_Model ()->Get_Bounding_Box ();
			Vector3 center_to_tip_vector;
			Matrix3D::Rotate_Vector (Get_Transform (), Vector3 (bounding_box.Extent.X / 2.0F, 0, 0), &center_to_tip_vector);
			Vector3 tip_curr_pos = curr_pos + center_to_tip_vector;
			tip_curr_pos.Z = curr_pos.Z;

			Path->Evaluate_Next_Point (tip_curr_pos, next_pos);
			PathClass::STATE_DESC result = Path->Get_State ();

			//
			//	Check to see if we need to continue along the path of
			// if we are finished
			//
			if (Pilot.Get_Mode () == PilotClass::MODE_HOVER) {
				Set_Vehicle_State (VEHICLE_STATE_HOVERING);
			} else if (result < PathClass::FIRST_ERROR) {
				Pilot.Set_Next_Point (next_pos);
			}			
		}
		break;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Set_Sakura_State
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Set_Sakura_State (int new_state)
{
	if (SakuraState != new_state) {
		
		switch (new_state)
		{
			case SAKURA_STATE_NORMAL:
				break;

			case SAKURA_STATE_TAUNTING:
			{
				//
				//	Find the first available taunt
				//
				int taunt_id = 0;
				int index;
				for (index = 0; index < MAX_TAUNTS; index ++) {
					if (TauntList[index] != 0) {
						taunt_id = TauntList[index] - 1;
						TauntList[index] = 0;
						break;
					}
				}

				//
				//	Reshuffle the taunts if necessary
				//
				if (index >= (MAX_TAUNTS - 1)) {
					Shuffle_Taunt_List ();
				}

				//
				//	Default to a 2 second taunt deault
				//
				SakuraTauntTimeLeft = 2.0F;
				
				//
				//	Lookup the conversation for this taunt
				//
				ConversationClass *conv = ConversationMgrClass::Find_Conversation (TAUNT_IDS[taunt_id]);
				if (conv != NULL) {
					ActiveConversationClass *taunt_conversation = ConversationMgrClass::Create_New_Conversation (conv);
					if (taunt_conversation != NULL) {
						
						//
						//	Add Sakura and the player to the list
						//
						taunt_conversation->Add_Orator (NULL);
						OratorClass *orator = taunt_conversation->Add_Orator (COMBAT_STAR);
						orator->Set_Flag (OratorClass::FLAG_DONT_MOVE,			true);
						orator->Set_Flag (OratorClass::FLAG_DONT_TURN_HEAD,	false);
						orator->Set_Flag (OratorClass::FLAG_DONT_FACE,			true);

						SakuraTauntTimeLeft = taunt_conversation->Get_Conversation_Time ();
						
						//
						//	Start the conversation
						//
						taunt_conversation->Start_Conversation ();
						REF_PTR_RELEASE (taunt_conversation);
					}

					REF_PTR_RELEASE (conv);
				}
			}
			break;
		}

		SakuraState = new_state;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Shuffle_Taunt_List
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Shuffle_Taunt_List (void)
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
			list_index = FreeRandom.Get_Int ( MAX_TAUNTS );
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
//	Update_Sakura_State
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Update_Sakura_State (void)
{
	switch (SakuraState)
	{		
		case SAKURA_STATE_NORMAL:
			break;

		case SAKURA_STATE_TAUNTING:
			SakuraTauntTimeLeft -= TimeManager::Get_Frame_Seconds ();
			if (SakuraTauntTimeLeft <= 0) {
				Set_Sakura_State (SAKURA_STATE_NORMAL);
			}
			break;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Set_Gattling_Gun_State
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Set_Gattling_Gun_State (int new_state)
{
	if (GattlingGunState != new_state) {
		
		switch (new_state)
		{
			case GATLING_STATE_NORMAL:
				GattlingGun->Set_Primary_Triggered (false);
				break;

			case GATLING_STATE_REVVING_UP:
			{
				AudibleSoundClass *sound = WWAudioClass::Get_Instance()->Create_Sound (Get_Definition().GattlingGunRevSoundDefID);
				if (sound != NULL) {

					//
					// We will continue to 'rev' the gatling gun until the sound ends
					//
					GattlingGunStateTimeLeft = (sound->Get_Duration () / 1000.0F);

					//
					//	Attach the sound to the gatling gun's muzzle, and add it to the world
					//
					sound->Attach_To_Object (Peek_Model (), GATLING_MUZZLE);
					sound->Add_To_Scene (true);
					REF_PTR_RELEASE (sound);					
				} else {
					GattlingGunStateTimeLeft = 1.0F;
				}				
			}				
			break;

			case GATLING_STATE_FIRING:
				
				//
				//	Pick a random amount of time to fire at the player
				//
				GattlingGunStateTimeLeft = WWMath::Random_Float (5.0F, 10.0F);
				if (GattlingGun != NULL) {

					//
					//	Start firing the gatling gun at the player
					//
					GattlingGun->Set_Primary_Triggered (true);
				}
				break;
		}

		GattlingGunState = new_state;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Update_Gattling_Gun_State
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Update_Gattling_Gun_State (void)
{
	switch ( GattlingGunState )
	{		
		case GATLING_STATE_REVVING_UP:
			GattlingGunStateTimeLeft -= TimeManager::Get_Frame_Seconds ();
			if (GattlingGunStateTimeLeft <= 0) {

				//
				//	We've finished revving up the gatling gun, so now
				// start firing at the player
				//
				Set_Gattling_Gun_State (GATLING_STATE_FIRING);
			}
			break;

		case GATLING_STATE_NORMAL:
			break;

		case GATLING_STATE_FIRING:

			//
			//	Target the player
			//
			ScriptableGameObj *target = CurrentTarget.Get_Ptr ();
			if (target != NULL) {
				Set_Targeting (TargetPos, true);
			}

			//
			//	Stop firing when we've shot at the player for a few seconds
			//							
			GattlingGunStateTimeLeft -= TimeManager::Get_Frame_Seconds ();
			if (GattlingGunStateTimeLeft <= 0) {				
				Set_Gattling_Gun_State (GATLING_STATE_NORMAL);
			}

			break;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Set_Rocket_State
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Set_Rocket_State (int new_state)
{
	if (RocketLauncherState != new_state) {
		
		switch (new_state)
		{
			case ROCKET_STATE_OPENING:
			{
				//
				//	Play the door open sound on the left rocket launcher
				//
				if (AvailableWeapons & WEAPON_ROCKETS_LEFT) {
					Matrix3D left_tm = Peek_Model ()->Get_Bone_Transform (LEFT_ROCKET_MUZZLE);
					WWAudioClass::Get_Instance()->Create_Instant_Sound (Get_Definition ().RocketDoorOpenSoundID, left_tm);
				}

				//
				//	Play the door open sound on the right rocket launcher
				//
				if (AvailableWeapons & WEAPON_ROCKETS_RIGHT) {
					Matrix3D right_tm = Peek_Model ()->Get_Bone_Transform (RIGHT_ROCKET_MUZZLE);
					WWAudioClass::Get_Instance()->Create_Instant_Sound (Get_Definition ().RocketDoorOpenSoundID, right_tm);
				}
				
				//
				//	Open the rocket doors
				//
				Open_Rocket_Launchers (true);
			}
			break;

			case ROCKET_STATE_CLOSING:
			{
				//
				//	Close the rocket doors
				//
				Open_Rocket_Launchers (false);

				//
				//	Stop firing
				//				
				if (RockerLauncherLeft != NULL && RockerLauncherRight != NULL) {
					RockerLauncherLeft->Set_Primary_Triggered (false);
					RockerLauncherRight->Set_Primary_Triggered (false);
				}
			}
			break;

			case ROCKET_STATE_FIRING:
			{
				//
				//	Pick a random amount of time to fire at the player
				//
				RocketLauncherStateTimeLeft = WWMath::Random_Float (2.0F, 6.0F);
				if (RockerLauncherLeft != NULL && RockerLauncherRight != NULL) {

					RockerLauncherLeft->Set_Target (TargetPos);
					RockerLauncherRight->Set_Target (TargetPos);

					//
					//	Start launching rockets at the player
					//
					if (AvailableWeapons & WEAPON_ROCKETS_LEFT) {
						RockerLauncherLeft->Set_Primary_Triggered (true);
					}

					if (AvailableWeapons & WEAPON_ROCKETS_RIGHT) {
						RockerLauncherRight->Set_Primary_Triggered (true);
					}
				}
			}
			break;

			case ROCKET_STATE_OPEN:
				RocketLauncherStateTimeLeft = 2.0F;
				break;

			case ROCKET_STATE_CLOSED:
				break;
		}

		RocketLauncherState = new_state;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Update_Rocket_State
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Update_Rocket_State (void)
{
	switch ( RocketLauncherState )
	{
		case ROCKET_STATE_OPENING:
		{
			//
			//	Are the rocket launcher doors completely open?
			//
			AnimControlClass *anim_control = Get_Anim_Control ();
			if (anim_control != NULL && anim_control->Get_Current_Frame () == ROCKETS_OUT_FRAME_NUM) {
				Set_Rocket_State (ROCKET_STATE_OPEN);
			}
		}	
		break;

		case ROCKET_STATE_CLOSING:
		{
			//
			//	Are the rocket launcher doors completely closed?
			//
			AnimControlClass *anim_control = Get_Anim_Control ();
			if (anim_control != NULL && anim_control->Get_Current_Frame () == ROCKETS_IN_FRAME_NUM) {
				Set_Rocket_State (ROCKET_STATE_CLOSED);
			}
		}	
		break;

		case ROCKET_STATE_FIRING:
		{
			RockerLauncherLeft->Set_Target (TargetPos);
			RockerLauncherRight->Set_Target (TargetPos);

			RocketLauncherStateTimeLeft -= TimeManager::Get_Frame_Seconds ();
			if (RocketLauncherStateTimeLeft <= 0) {
				
				//
				//	Stop firing
				//
				RockerLauncherLeft->Set_Primary_Triggered (false);
				RockerLauncherRight->Set_Primary_Triggered (false);

				//
				//	Close the rocket doors
				//
				Set_Rocket_State (ROCKET_STATE_CLOSING);
			}
		}
		break;

		case ROCKET_STATE_OPEN:
		{
			RocketLauncherStateTimeLeft -= TimeManager::Get_Frame_Seconds ();
			if (RocketLauncherStateTimeLeft <= 0) {
				
				//
				//	Commence firing
				//
				Set_Rocket_State (ROCKET_STATE_FIRING);
			}
		}
		break;

		case ROCKET_STATE_CLOSED:
			break;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Apply_Damage
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Apply_Damage_Extended
(
	const OffenseObjectClass &		damager,
	float									scale,
	const Vector3 &					direction,
	const char *						collision_box_name
)
{
	if ( !CombatManager::I_Am_Server() ) {
		return;
	}

	//
	//	If rockets are out, and the player shot the rocket, then
	// apply the damage to the rocket (not the whole vehicle)
	//
	bool damaged_rockets = false;
	if (RocketLauncherState != ROCKET_STATE_CLOSED && collision_box_name != NULL) {
				
		if (::strstr (collision_box_name, LEFT_ROCKET_MESH) != NULL) {
			damaged_rockets = true;
			
			//
			//	Apply the damage to the left rocket launcher
			//
			float health_left = LeftRocketDefenseObject.Apply_Damage (damager, scale);
			if (health_left <= 0) {
				Blow_Off_Weapon (WEAPON_ROCKETS_LEFT);
			}

		} else if (::strstr (collision_box_name, RIGHT_ROCKET_MESH) != NULL) {
			damaged_rockets = true;

			//
			//	Apply the damage to the right rocket launcher
			//
			float health_left = RightRocketDefenseObject.Apply_Damage (damager, scale);
			if (health_left <= 0) {
				Blow_Off_Weapon (WEAPON_ROCKETS_RIGHT);
			}
		}
	}
	
	//
	//	If the rockets weren't specifically targetted, then let the damage occur normally
	//		
	if (damaged_rockets == false) {
		VehicleGameObj::Apply_Damage_Extended (damager, scale, direction, collision_box_name);
	}

	//
	//	Remember who was last shooting at us
	//
	LastDamager		= damager.Get_Owner ();
	CurrentTarget	= LastDamager;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Blow_Off_Weapon
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Blow_Off_Weapon (int weapon_id)
{
	Matrix3D tm (1);

	//
	//	Determine where to create the explosion at
	//
	if (weapon_id == WEAPON_ROCKETS_LEFT) {
		tm = Peek_Model ()->Get_Bone_Transform (LEFT_ROCKET_MUZZLE);
	} else {
		tm = Peek_Model ()->Get_Bone_Transform (RIGHT_ROCKET_MUZZLE);
	}

	//
	//	Create an explosion
	//
	ExplosionManager::Create_Explosion_At (Get_Definition ().RocketDestroyedExplosionID, tm, NULL);

	//
	//	Find the rocket launcher model
	//
	RenderObjClass *weapon_model = NULL;
	if (weapon_id == WEAPON_ROCKETS_LEFT) {
		weapon_model = Peek_Model ()->Get_Sub_Object_By_Name (LEFT_ROCKET_MESH);
	} else {
		weapon_model = Peek_Model ()->Get_Sub_Object_By_Name (RIGHT_ROCKET_MESH);
	}

	//
	//	Remove the destroyed rocket launcher model from the apache
	//
	if (weapon_model != NULL) {
		Peek_Model ()->Remove_Sub_Object (weapon_model);
		REF_PTR_RELEASE (weapon_model);
	}

	//
	//	'Shake' the vehicle
	//
	PhysClass *phys_obj = Peek_Physical_Object ();
	if (phys_obj != NULL && phys_obj->As_RigidBodyClass () != NULL) {
		RigidBodyClass *rigid_body = phys_obj->As_RigidBodyClass ();
							
		Vector3 center = Get_Transform ().Get_Translation ();
		Vector3 y_axis = Get_Transform ().Get_Y_Vector ();

		//
		//	Calculate a position 1 meter to the left and right of the
		// apache's center of mass
		//
		Vector3 left_imp_pos		= center + y_axis;
		Vector3 right_imp_pos	= center - y_axis;

		//
		//	Determine how much impulse to apply on each side
		//
		Vector3 left_imp (0, 0, rigid_body->Get_Mass () * 10);
		Vector3 right_imp (0, 0, -rigid_body->Get_Mass () * 10);
		if (weapon_id == WEAPON_ROCKETS_RIGHT) {
			left_imp.Z	= -left_imp.Z;
			right_imp.Z	= -right_imp.Z;
		}

		//
		//	Make the vehicle 'react' to the explosion
		//
		rigid_body->Apply_Impulse (left_imp, left_imp_pos);
		rigid_body->Apply_Impulse (right_imp, right_imp_pos);
	}
	
	//
	//	Make sure the weapon stops firing
	//
	if (weapon_id == WEAPON_ROCKETS_LEFT) {
		RockerLauncherLeft->Set_Primary_Triggered (false);
	} else {
		RockerLauncherRight->Set_Primary_Triggered (false);
	}

	//
	//	Remove the weapon from the list of available weapons
	//
	AvailableWeapons &= ~weapon_id;	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Open_Rocket_Launchers
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Open_Rocket_Launchers (bool onoff)
{
	AnimControlClass *anim_control = Get_Anim_Control ();
	if (anim_control != NULL) {

		//
		//	Set the rocket-door animation
		//
		anim_control->Set_Animation (ROCKET_DOOR_ANIMATION);
		anim_control->Set_Mode (ANIM_MODE_TARGET);
		anim_control->Set_Target_Frame (onoff ? ROCKETS_OUT_FRAME_NUM : ROCKETS_IN_FRAME_NUM);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Are_Rocker_Launchers_Ready
//
///////////////////////////////////////////////////////////////////////////
bool
SakuraBossGameObj::Are_Rocker_Launchers_Ready (void)
{
	bool retval = false;

	//
	//	Rocket launchers are ready if the animation has been played to end
	//
	AnimControlClass *anim_control = Get_Anim_Control ();
	if (anim_control != NULL) {
		retval = (anim_control->Get_Current_Frame () == ROCKETS_OUT_FRAME_NUM);
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Move_To_Location
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Move_To_Location (const Vector3 &pos, float speed)
{	
	MoveToLocation = pos;
	Set_Vehicle_State (VEHICLE_STATE_MOVING);

	//
	//	Create an action that will be responsible for moving the vehicle
	// to this location.
	//
	Pilot.Set_Mode (PilotClass::MODE_FLY_TO_POINT);
	Pilot.Set_Destination (pos);	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Muzzle
//
///////////////////////////////////////////////////////////////////////////
const Matrix3D &
SakuraBossGameObj::Get_Muzzle (int /*index*/)
{
	CurrentMuzzleTM = Get_Transform ();

	RenderObjClass *model = Peek_Model ();
	if (model != NULL) {
				
		if (GattlingGunState == GATLING_STATE_FIRING) {

			//
			//	Simply return the muzzle bone of the gatling gun
			//
			CurrentMuzzleTM = model->Get_Bone_Transform (GATLING_MUZZLE);

		} else {

			
			//
			//	Toggle rocket launchers.
			//
			//		Note:  This is done because we use a separate
			// weapon for each rocket launcher.  However, the weapon asks
			// us for a muzzle without telling us which weapon it is, so
			// to simulate attacking from both sides simultaneously we alternate
			// between the two muzzles.
			//
			//CurrentMuzzleIndex = !CurrentMuzzleIndex;

			//
			//	Make sure we aren't attacking from a side that's been blown off.
			//
			if ((AvailableWeapons & WEAPON_ROCKETS_LEFT) == 0) {
				CurrentMuzzleIndex = ROCKET_RIGHT;
			} else if ((AvailableWeapons & WEAPON_ROCKETS_RIGHT) == 0) {
				CurrentMuzzleIndex = ROCKET_LEFT;
			}

			if (CurrentMuzzleIndex == ROCKET_RIGHT) {
				CurrentMuzzleTM = model->Get_Bone_Transform (RIGHT_ROCKET_MUZZLE);
			} else {
				CurrentMuzzleTM = model->Get_Bone_Transform (LEFT_ROCKET_MUZZLE);
			}
		}
	}
	
	return CurrentMuzzleTM;
}


///////////////////////////////////////////////////////////////////////////
//
//	Update_Target
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Update_Target (void)
{
	//
	//	Check to see if its time to choose a new target yet
	//
	TargetTimeLeft -= TimeManager::Get_Frame_Seconds ();
	if (TargetTimeLeft <= 0) {

		//
		//	Choose either the closest commando or the last person to damage us
		//
		if (FreeRandom.Get_Int (2) == 0 && LastDamager != NULL) {		
			CurrentTarget = LastDamager;	
		} else {			
			CurrentTarget.Set_Ptr (Find_Closest_Human_Player ());
		}

		//
		//	Wait a random amount of time before we choose a new target
		//
		TargetTimeLeft = WWMath::Random_Float (5.0F, 20.0F);
	} else if (CurrentTarget == NULL) {
		TargetTimeLeft = TargetTimeLeft * 0.25F;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Find_Closest_Human_Player
//
///////////////////////////////////////////////////////////////////////////
SoldierGameObj *
SakuraBossGameObj::Find_Closest_Human_Player (void)
{
	SoldierGameObj *closest_human_player	= NULL;
	float closest_distance					= 9999.0F;

	//
	//	Get our current position
	//
	Vector3 sakura_position;
	Get_Position (&sakura_position);

	AABoxClass valley_box;		
	valley_box.Center.Set (-105.0F, 35.0F, 20.0F);
	valley_box.Extent.Set (75.0F, 70.5F, 14.0F);

	//
	// Collect the dynamic physics objects in the valley
	//
	NonRefPhysListClass objs_in_valley;
	PhysicsSceneClass::Get_Instance ()->Collect_Objects (valley_box, false, true, &objs_in_valley);
	
	//
	//	Loop over all the objects
	//
	NonRefPhysListIterator it (&objs_in_valley);
	for (it.First (); !it.Is_Done (); it.Next ()) {
		PhysicalGameObj *gameobj = NULL;
		
		//
		//	Get the game object from this physics object
		//
		if (it.Peek_Obj()->Get_Observer () != NULL) {
			gameobj = ((CombatPhysObserverClass *)it.Peek_Obj()->Get_Observer())->As_PhysicalGameObj();
		}
		
		//
		//	Check to see if this is a commando game object
		//
		if (gameobj != NULL) {
			SoldierGameObj *soldier = gameobj->As_SoldierGameObj ();
			if (soldier != NULL && soldier->Is_Human_Controlled()) {
				
				Vector3 position;
				soldier->Get_Position (&position);

				//
				//	Is this the closest commando we've found so far?
				//
				float distance = (position - sakura_position).Length ();
				if (distance < closest_distance) {
					closest_human_player = soldier;
					closest_distance = distance;
				}
			}			
		}
	}

	return closest_human_player;
}


///////////////////////////////////////////////////////////////////////////
//
//	Do_Waypath
//
///////////////////////////////////////////////////////////////////////////
void
SakuraBossGameObj::Do_Waypath (int waypath_id, int start_id, int end_id)
{
	WaypathClass *waypath = PathfindClass::Get_Instance()->Find_Waypath (waypath_id);
	if (waypath != NULL) {

		//
		//	Initialize our pathfind object with the waypath
		//
		PathObjectClass path_obj;
		path_obj.Initialize (*Peek_Physical_Object ());
		path_obj.Set_Max_Speed (Get_Max_Speed ());
		path_obj.Set_Turn_Radius (Get_Turn_Radius ());
		path_obj.Set_Flag (PathObjectClass::IS_VEHICLE, true);								
		Path->Set_Path_Object (path_obj);
		Path->Set_Traversal_Type (PathClass::SPLINE);
		Path->Set_Movement_Radius (5.0F);
		Path->Initialize (waypath, start_id, end_id);
		Path->Set_Movement_Directions (PathClass::MOVE_X | PathClass::MOVE_Y | PathClass::MOVE_Z);

		//
		//	Find the starting and ending waypoints
		//
		int count = waypath->Get_Point_Count ();
		int start_index	= 0;
		int end_index		= count - 1;
		for (int index  = 0; index < count; index ++) {
			if (waypath->Get_Point (index)->Get_ID () == start_id) {
				start_index = index;
			}
			if (waypath->Get_Point (index)->Get_ID () == end_id) {
				end_index = index;
			}
		}
		
		//
		//	Get the world space locations of the starting and ending waypoints
		//
		Vector3 start_pos	= waypath->Get_Point (start_index)->Get_Position ();
		Vector3 end_pos	= waypath->Get_Point (end_index)->Get_Position ();

		//
		//	Initialize the pilot code with the new flight path
		//
		Pilot.Set_Mode (PilotClass::MODE_FLY_TO_POINT);
		Pilot.Set_Next_Point (start_pos);
		Pilot.Set_Destination (end_pos);
		Pilot.Set_Arrived_Dist (1.0F);
		Pilot.Set_Hover_Dist (10.0F);

		Set_Vehicle_State (VEHICLE_STATE_MOVING);
	}

	return ;
}
