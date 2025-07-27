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
 *                     $Archive:: /Commando/Code/Combat/beacongameobj.cpp                     $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/14/02 1:59p                                               $*
 *                                                                                             *
 *                    $Revision:: 41                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "beacongameobj.h"
#include "debug.h"
#include "phys.h"
#include "combat.h"
#include "soldier.h"
#include "persistfactory.h"
#include "combatchunkid.h"
#include "weaponmanager.h"
#include "simpledefinitionfactory.h"
#include "wwhack.h"
#include "decophys.h"
#include "assets.h"
#include "gameobjmanager.h"
#include "wwaudio.h"
#include "wwprofile.h"
#include "cinematicgameobj.h"
#include "basecontroller.h"
#include "playertype.h"
#include "colmath.h"
#include "wwaudio.h"
#include "audiblesound.h"
#include "translateobj.h"
#include "translatedb.h"
#include "messagewindow.h"
#include "weaponbag.h"
#include "objlibrary.h"
#include "hudinfo.h"
#include "explosion.h"
#include "backgroundmgr.h"
#include "weathermgr.h"
#include "persistentgameobjobserver.h"
#include "apppackettypes.h"
#include "weapons.h"


////////////////////////////////////////////////////////////////
//	Hacks
////////////////////////////////////////////////////////////////
DECLARE_FORCE_LINK (Beacon)

////////////////////////////////////////////////////////////////
//	Editable and persist factories
////////////////////////////////////////////////////////////////
SimplePersistFactoryClass	<BeaconGameObjDef,	CHUNKID_GAME_OBJECT_DEF_BEACON>				_BeaconGameObjDefPersistFactory;
SimplePersistFactoryClass	<BeaconGameObj,		CHUNKID_GAME_OBJECT_BEACON>					_BeaconGameObjPersistFactory;
DECLARE_DEFINITION_FACTORY (BeaconGameObjDef,	CLASSID_GAME_OBJECT_DEF_BEACON, "Beacon")	_BeaconGameObjDefDefFactory;


////////////////////////////////////////////////////////////////
//	Save/Load constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_MONITOR_PARENT				=	0x07250256,
};

enum
{
	CHUNKID_DEF_PARENT					=	0x02190435,
	CHUNKID_DEF_VARIABLES,

	MICROCHUNKID_DEF_BROADCAST_TIME	= 1,
	MICROCHUNKID_DEF_ARM_TIME,
	MICROCHUNKID_DEF_DISARM_TIME,
	MICROCHUNKID_DEF_DETONATE_TIME,
	MICROCHUNKID_DEF_ARMED_SOUNDID,
	MICROCHUNKID_DEF_DISARMING_TEXTID,
	MICROCHUNKID_DEF_DISARMED_TEXTID,
	MICROCHUNKID_DEF_ARMING_TEXTID,
	MICROCHUNKID_DEF_POST_CINEMATIC_DEFID,
	MICROCHUNKID_DEF_ARM_INTERRUPT_TEXTID,
	MICROCHUNKID_DEF_DISARM_INTERRUPT_TEXTID,
	MICROCHUNKID_DEF_ARMING_ANIM_NAME,
	MICROCHUNKID_DEF_PRE_CINEMATIC_DEFID,
	MICROCHUNKID_DEF_EXPLOSION_DEFID,
	MICROCHUNKID_DEF_POST_DETONATE_TIME,
	MICROCHUNKID_DEF_PRE_DETONATE_CINEMATIC_DELAY,
	MICROCHUNKID_DEF_IS_NUKE,
};

enum
{
	CHUNKID_PARENT							=	0x0219043,
	CHUNKID_VARIABLES,
	CHUNKID_OWNER,
	CHUNKID_CINEMATIC,

	MICROCHUNKID_STATE					= 1,
	MICROCHUNKID_STATE_TIMER,
	MICROCHUNKID_DETONATE_TIMER,
	MICROCHUNKID_PRE_DETONATE_TIMER,
	MICROCHUNKID_IS_ARMED,
};


////////////////////////////////////////////////////////////////
//
//	BeaconGameObjDef
//
////////////////////////////////////////////////////////////////
BeaconGameObjDef::BeaconGameObjDef (void)	:
	BroadcastToAllTime (5.0F),
	ArmTime (10.0F),
	DisarmTime (10.0F),
	PreDetonateCinematicDelay(0),
	DetonateTime (30.0F),
	PostDetonateTime (10.0F),
	ArmedSoundDefID (0),
	DisarmingTextID (0),
	DisarmedTextID (0),
	ArmingTextID (0),
	ArmingInterruptedTextID (0),
	DisarmingInterruptedTextID (0),
	PreDetonateCinematicDefID (0),
	PostDetonateCinematicDefID (0),
	ExplosionDefID (0),
	IsNuke( true )
{
	//
	//	Editable support
	//
	EDITABLE_PARAM (BeaconGameObjDef, ParameterClass::TYPE_STRING,					ArmingAnimationName);
	EDITABLE_PARAM (BeaconGameObjDef, ParameterClass::TYPE_FLOAT,					BroadcastToAllTime);
	EDITABLE_PARAM (BeaconGameObjDef, ParameterClass::TYPE_FLOAT,					ArmTime);
	EDITABLE_PARAM (BeaconGameObjDef, ParameterClass::TYPE_FLOAT,					DisarmTime);
	EDITABLE_PARAM (BeaconGameObjDef, ParameterClass::TYPE_FLOAT,					PreDetonateCinematicDelay);
	EDITABLE_PARAM (BeaconGameObjDef, ParameterClass::TYPE_FLOAT,					DetonateTime);
	EDITABLE_PARAM (BeaconGameObjDef, ParameterClass::TYPE_FLOAT,					PostDetonateTime);
	EDITABLE_PARAM (BeaconGameObjDef, ParameterClass::TYPE_SOUNDDEFINITIONID,	ArmedSoundDefID);
	EDITABLE_PARAM (BeaconGameObjDef, ParameterClass::TYPE_STRINGSDB_ID,			ArmingTextID);
	EDITABLE_PARAM (BeaconGameObjDef, ParameterClass::TYPE_STRINGSDB_ID,			ArmingInterruptedTextID);
	EDITABLE_PARAM (BeaconGameObjDef, ParameterClass::TYPE_STRINGSDB_ID,			DisarmingTextID);
	EDITABLE_PARAM (BeaconGameObjDef, ParameterClass::TYPE_STRINGSDB_ID,			DisarmingInterruptedTextID);
	EDITABLE_PARAM (BeaconGameObjDef, ParameterClass::TYPE_STRINGSDB_ID,			DisarmedTextID);
	EDITABLE_PARAM (BeaconGameObjDef, ParameterClass::TYPE_BOOL,					IsNuke);

	#ifdef PARAM_EDITING_ON
		GenericDefParameterClass *param = new GenericDefParameterClass (&PreDetonateCinematicDefID);
		param->Set_Class_ID (CLASSID_GAME_OBJECT_DEF_CINEMATIC);
		param->Set_Name ("Pre-Detonate Cinematic Obj");
		GENERIC_EDITABLE_PARAM (BeaconGameObjDef, param)

		param = new GenericDefParameterClass (&PostDetonateCinematicDefID);
		param->Set_Class_ID (CLASSID_GAME_OBJECT_DEF_CINEMATIC);
		param->Set_Name ("Post-Detonate Cinematic Obj");
		GENERIC_EDITABLE_PARAM (BeaconGameObjDef, param)

		param = new GenericDefParameterClass (&ExplosionDefID);
		param->Set_Class_ID (CLASSID_DEF_EXPLOSION);
		param->Set_Name ("Explosion Obj");
		GENERIC_EDITABLE_PARAM (BeaconGameObjDef, param)

	#endif //PARAM_EDITING_ON

	return ;
}


////////////////////////////////////////////////////////////////
//
//	~BeaconGameObjDef
//
////////////////////////////////////////////////////////////////
BeaconGameObjDef::~BeaconGameObjDef (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Class_ID
//
////////////////////////////////////////////////////////////////
uint32
BeaconGameObjDef::Get_Class_ID (void) const
{
	return CLASSID_GAME_OBJECT_DEF_BEACON;
}


////////////////////////////////////////////////////////////////
//
//	Create
//
////////////////////////////////////////////////////////////////
PersistClass *
BeaconGameObjDef::Create (void) const
{
	BeaconGameObj *beacon = new BeaconGameObj;
	beacon->Init (*this);

	return beacon;
}


////////////////////////////////////////////////////////////////
//
//	Create
//
////////////////////////////////////////////////////////////////
bool
BeaconGameObjDef::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_DEF_PARENT);
		SimpleGameObjDef::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_DEF_VARIABLES);

		WRITE_MICRO_CHUNK_WWSTRING (csave, MICROCHUNKID_DEF_ARMING_ANIM_NAME,	ArmingAnimationName);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_BROADCAST_TIME,					BroadcastToAllTime);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_ARM_TIME,							ArmTime);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_DISARM_TIME,						DisarmTime);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_PRE_DETONATE_CINEMATIC_DELAY,PreDetonateCinematicDelay);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_DETONATE_TIME,					DetonateTime);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_POST_DETONATE_TIME,				PostDetonateTime);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_ARMED_SOUNDID,					ArmedSoundDefID);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_DISARMING_TEXTID,				DisarmingTextID);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_DISARMED_TEXTID,					DisarmedTextID);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_ARMING_TEXTID,					ArmingTextID);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_ARM_INTERRUPT_TEXTID,			ArmingInterruptedTextID);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_DISARM_INTERRUPT_TEXTID,		DisarmingInterruptedTextID);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_PRE_CINEMATIC_DEFID,			PreDetonateCinematicDefID);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_POST_CINEMATIC_DEFID,			PostDetonateCinematicDefID);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_EXPLOSION_DEFID,					ExplosionDefID);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_IS_NUKE,							IsNuke);

	csave.End_Chunk ();

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////
bool
BeaconGameObjDef::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ())
	{
		switch (cload.Cur_Chunk_ID ())
		{
			case CHUNKID_DEF_PARENT:
				SimpleGameObjDef::Load (cload);
				break;

			case CHUNKID_DEF_VARIABLES:
				Load_Variables (cload);
				break;

			default:
				Debug_Say (("Unrecognized Beacon Def chunkID\n"));
				break;

		}
		cload.Close_Chunk ();
	}

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
////////////////////////////////////////////////////////////////
void
BeaconGameObjDef::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ())
		{
			READ_MICRO_CHUNK_WWSTRING (cload, MICROCHUNKID_DEF_ARMING_ANIM_NAME,		ArmingAnimationName);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_BROADCAST_TIME,					BroadcastToAllTime);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_ARM_TIME,							ArmTime);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_DISARM_TIME,						DisarmTime);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_PRE_DETONATE_CINEMATIC_DELAY,	PreDetonateCinematicDelay);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_DETONATE_TIME,						DetonateTime);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_POST_DETONATE_TIME,				PostDetonateTime);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_ARMED_SOUNDID,						ArmedSoundDefID);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_DISARMING_TEXTID,					DisarmingTextID);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_DISARMED_TEXTID,					DisarmedTextID);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_ARMING_TEXTID,						ArmingTextID);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_ARM_INTERRUPT_TEXTID,			ArmingInterruptedTextID);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_DISARM_INTERRUPT_TEXTID,		DisarmingInterruptedTextID);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_PRE_CINEMATIC_DEFID,				PreDetonateCinematicDefID);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_POST_CINEMATIC_DEFID,			PostDetonateCinematicDefID);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_EXPLOSION_DEFID,					ExplosionDefID);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_IS_NUKE,							IsNuke);

			default:
				Debug_Say (("Unrecognized Beacon Def Variable chunkID\n"));
				break;
		}

		cload.Close_Micro_Chunk();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
BeaconGameObjDef::Get_Factory (void) const
{
	return _BeaconGameObjDefPersistFactory;
}


////////////////////////////////////////////////////////////////
//
//	BeaconGameObj
//
////////////////////////////////////////////////////////////////
BeaconGameObj::BeaconGameObj (void)	:
	StateTimer (0),
	PreDetonateTimer (0),
	DetonateTimer (0),
	WarningTimer(0.0f),
	State (0),
	IsArmed (false),
	WeaponDefinition (NULL),
	MessageSound (NULL),
	ArmedSound (NULL),
	OwnerBackup( NULL )
{
	Set_App_Packet_Type(APPPACKETTYPE_BEACON);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~BeaconGameObj
//
////////////////////////////////////////////////////////////////
BeaconGameObj::~BeaconGameObj (void)
{
	Stop_Current_Message_Sound ();
	Stop_Armed_Sound ();
	Stop_Owner_Animation ();

	CinematicObject = NULL;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
BeaconGameObj::Get_Factory (void) const
{
	return _BeaconGameObjPersistFactory;
}


////////////////////////////////////////////////////////////////
//
//	Init
//
////////////////////////////////////////////////////////////////
void BeaconGameObj::Init( void )
{
	Init( Get_Definition() );
}

////////////////////////////////////////////////////////////////
//
//	Init
//
////////////////////////////////////////////////////////////////
void
BeaconGameObj::Init (const BeaconGameObjDef &definition)
{
	SimpleGameObj::Init (definition);

	//
	//	Make the object so its targettable and it collides with the terrain
	//
	Peek_Physical_Object()->Set_Collision_Group (TERRAIN_AND_BULLET_COLLISION_GROUP);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Definition
//
////////////////////////////////////////////////////////////////
const BeaconGameObjDef &
BeaconGameObj::Get_Definition (void) const
{
	return (const BeaconGameObjDef &)BaseGameObj::Get_Definition ();
}


////////////////////////////////////////////////////////////////
//
//	Init_Beacon
//
////////////////////////////////////////////////////////////////
void
BeaconGameObj::Init_Beacon
(
	const WeaponDefinitionClass *	definition,
	SoldierGameObj *					owner,
	const Vector3 &					position
)
{
	WeaponDefinition	= definition;
	Owner					= owner;
	Set_Position (position);

	if ( owner ) {
		OwnerBackup = owner->Get_Player_Data();
	}

	//
	//	Become part of the same team as the player who is
	// dropping us
	//
	if (Owner != NULL) {
		Set_Player_Type (Owner.Get_Ptr ()->As_SmartGameObj ()->Get_Player_Type ());
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////
bool
BeaconGameObj::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_PARENT);
		SimpleGameObj::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_STATE,				State);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_STATE_TIMER,		StateTimer);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DETONATE_TIMER,	DetonateTimer);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_PRE_DETONATE_TIMER,	PreDetonateTimer);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_IS_ARMED,		IsArmed);
	csave.End_Chunk ();

	//
	//	Save the owner (if necessary)
	//
	if (Owner != NULL) {
		csave.Begin_Chunk (CHUNKID_OWNER);
			Owner.Save (csave);
		csave.End_Chunk ();
	}

	if (CinematicObject != NULL) {
		csave.Begin_Chunk (CHUNKID_CINEMATIC);
			CinematicObject.Save (csave);
		csave.End_Chunk ();
	}

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////
bool
BeaconGameObj::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_PARENT:
				SimpleGameObj::Load (cload);
				break;

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;

			case CHUNKID_OWNER:
				Owner.Load (cload);
				break;

			case CHUNKID_CINEMATIC:
				CinematicObject.Load (cload);
				break;

			default:
				Debug_Say (("Unrecognized Beacon chunkID\n"));
				break;
		}

		cload.Close_Chunk();
	}

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
////////////////////////////////////////////////////////////////
void
BeaconGameObj::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {

		switch (cload.Cur_Micro_Chunk_ID ())
		{
			READ_MICRO_CHUNK (cload, MICROCHUNKID_STATE,				State);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_STATE_TIMER,		StateTimer);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DETONATE_TIMER,	DetonateTimer);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_PRE_DETONATE_TIMER,	PreDetonateTimer);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_IS_ARMED,		IsArmed);

			default:
				Debug_Say (("Unrecognized Beacon Variable chunkID\n"));
				break;
		}

		cload.Close_Micro_Chunk();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Think
//
////////////////////////////////////////////////////////////////
void
BeaconGameObj::Think (void)
{
	WWPROFILE ("Beacon Think");

	Restore_Owner();


	//
	//	Let the state think a little
	//
	Update_State ();

	//
	//	Check to see if we need to break out of the
	// arming or disarming state
	//
	if ( CombatManager::I_Am_Server() && State == STATE_ARMING )
	{
		//
		//	Was the owner interrupted while performing his action?
		//
		if (Was_Owner_Interrupted ()) {
			Stop_Owner_Animation ();

			//
			//	Let the player know that the arming operation was cancelled
			//
			Display_Message (Get_Definition ().ArmingInterruptedTextID);

			//
			//	If the arming state was interrupted, then
			// return the ammo that created the beacon to its owner.
			//
			SoldierGameObj *soldier = Get_Owner ();
			if (soldier != NULL) {
				WeaponBagClass	*weapon_bag = soldier->Get_Weapon_Bag ();
				weapon_bag->Add_Weapon (WeaponDefinition, 1, false);

				//
				//	If the user is still holding the beacon weapon then stop its firing
				// sound.
				//
				WeaponClass	*curr_weapon = weapon_bag->Get_Weapon ();
				if (curr_weapon != NULL && curr_weapon->Get_ID () == WeaponDefinition->Get_ID ()) {
					curr_weapon->Stop_Firing_Sound ();
				}
			}

			//
			//	Now destroy ourselves
			//
			Set_Delete_Pending ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Information
//
////////////////////////////////////////////////////////////////
void
BeaconGameObj::Get_Information (StringClass &string)
{
	SimpleGameObj::Get_Information( string );
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Start_Cinematic
//
////////////////////////////////////////////////////////////////
void
BeaconGameObj::Start_Cinematic ( int id )
{
	//
	//	Create the cinematic controller
	//
	if ( id != 0 ) {
		PhysicalGameObj *game_obj = ObjectLibraryManager::Create_Object ( id );
		if (game_obj != NULL) {
			game_obj->Start_Observers ();

			//
			//	Position the cinematic controller in the world
			//
			Vector3 position;
			Get_Position (&position);
			game_obj->Set_Position (position);
	
			CinematicObject = game_obj;

			Debug_Say(( "Beacon Cinematic Started\n" ));
		}
	}
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Stop_Armed_Sound
//
////////////////////////////////////////////////////////////////
void
BeaconGameObj::Stop_Armed_Sound (void)
{
	if (ArmedSound != NULL) {
		ArmedSound->Remove_From_Scene ();
		REF_PTR_RELEASE (ArmedSound);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_State
//
////////////////////////////////////////////////////////////////
void
BeaconGameObj::Set_State (int state)
{
	if (State == state) {
		return ;
	}

	Restore_Owner();

	if ( CombatManager::I_Am_Server() ) {
		//
		//	"Dirty" the object for networking
		//
		Set_Object_Dirty_Bit( NetworkObjectClass::BIT_RARE, true );
	}

	bool is_nuke = !!Get_Definition().IsNuke;

	switch (state) {
		case STATE_NULL:
			Stop_Armed_Sound ();
			Display_Message (Get_Definition ().ArmingInterruptedTextID);
			break;

		case STATE_ARMING:
			StateTimer = Get_Definition ().ArmTime;
			Display_Message (Get_Definition ().ArmingTextID);
			Start_Owner_Animation ();
			break;

		case STATE_ARMED:
			if (IsArmed == false) {
				Stop_Owner_Animation ();

				//
				//	Set the detonation timer
				//
				DetonateTimer = Get_Definition ().DetonateTime;
				WarningTimer = Get_Definition().BroadcastToAllTime;

				//
				//	Create the "armed" sound
				//
				ArmedSound = WWAudioClass::Get_Instance ()->Create_Continuous_Sound (Get_Definition ().ArmedSoundDefID);
				if (ArmedSound != NULL) {

					//
					//	Insert the sound object into the world
					//
					ArmedSound->Set_Transform (Get_Transform ());
					ArmedSound->Add_To_Scene ();
				}

				// Only on Server
				if ( CombatManager::I_Am_Server() ) {
					// switch to pre detonate weather
					Debug_Say(( "Pre-Detonate weather override\n" ));
					if ( is_nuke ) {
						BackgroundMgrClass::Override_Sky_Tint ( 0.8f, DetonateTimer/2 );
						WeatherMgrClass::Override_Wind (0, 3, 1, DetonateTimer/2 );
					} else {
						BackgroundMgrClass::Override_Clouds(1.0f, 1.0f, DetonateTimer/2);
						BackgroundMgrClass::Override_Lightning( 0.8f, 0.2f, 0.8f, 0, 1.0f, DetonateTimer/2);
						WeatherMgrClass::Override_Precipitation (WeatherMgrClass::PRECIPITATION_RAIN, 2.0f, DetonateTimer/2);
					}
				}

				// Notify base controllers the beacon is armed
				BaseControllerClass* base = BaseControllerClass::Find_Base(Get_Player_Type());

				if (base) {
					base->On_Beacon_Armed(this);
				}

				PreDetonateTimer = max( Get_Definition ().PreDetonateCinematicDelay, 0.001f );

				IsArmed = true;
			}
			break;

		case STATE_DISARMED: {
			Display_Message (Get_Definition ().DisarmedTextID);
			Stop_Armed_Sound ();
			Stop_Owner_Animation ();

			// Only on Server
			if ( CombatManager::I_Am_Server() ) {
				// cancel weather override
				Debug_Say(( "Cancelling weather override\n" ));
				if ( is_nuke ) {
					BackgroundMgrClass::Restore_Sky_Tint ( 5 );
					WeatherMgrClass::Restore_Wind( 5 );
				} else {
					BackgroundMgrClass::Restore_Clouds( 5 );
					BackgroundMgrClass::Restore_Lightning( 5 );
					WeatherMgrClass::Restore_Precipitation( 5 );
				}
			}

			// Notify base that the beacon is disarmed
			BaseControllerClass* base = BaseControllerClass::Find_Base(Get_Player_Type());

			if (base) {
				base->On_Beacon_Disarmed(this);
			}

			// Stop cinematic
			if ( CinematicObject.Get_Ptr() != NULL ) {
				CinematicObject.Get_Ptr()->Set_Delete_Pending();
				CinematicObject = NULL;
			}

			Set_Delete_Pending ();
			break;
		}

		case STATE_DETONATING:
			Stop_Armed_Sound ();

			Debug_Say(( "Detonate!\n" ));

			IsArmed = false;

			StateTimer = Get_Definition ().PostDetonateTime;

			// Only on Server
			if ( CombatManager::I_Am_Server() ) {

				//	Create the cinematic controller
				Start_Cinematic( Get_Definition ().PostDetonateCinematicDefID );

				// switch to post detonate weather
				Debug_Say(( "Post-Detonate weather override\n" ));

				if ( is_nuke ) {
					WeatherMgrClass::Override_Precipitation (WeatherMgrClass::PRECIPITATION_ASH, 0.3f);
				}

				/*
				// Create the explosion, if not in the enemy base
				if ( !Is_In_Enemy_Base() && Get_Definition ().ExplosionDefID != 0 ) {
					Create_Explosion ();
				}
				*/
			}

			// Create the explosion, if not in the enemy base
//			if ( !Is_In_Enemy_Base() && Get_Definition ().ExplosionDefID != 0 ) {
			if ( Get_Definition ().ExplosionDefID != 0 ) {
				Create_Explosion ();
			}

			// Hide our model
			if ( Peek_Model() ) {
				Peek_Model()->Set_Hidden( true );
			}
			break;
	}

	State = state;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_State
//
////////////////////////////////////////////////////////////////
void
BeaconGameObj::Update_State (void)
{
	StateTimer -= TimeManager::Get_Frame_Seconds ();

	if (IsArmed) {
		//
		//	Tweak the pitch of the armed sound
		//
		if (ArmedSound != NULL) {
			float percent = (1.0F - DetonateTimer / Get_Definition ().DetonateTime);
			ArmedSound->Set_Pitch_Factor (1.0F + (percent * 5.0F));
		}

		//
		//	Check to see if we've exploded
		//
		DetonateTimer -= TimeManager::Get_Frame_Seconds ();

		if (DetonateTimer <= 0) {
			Set_State (STATE_DETONATING);
		}

		if ( PreDetonateTimer != 0 ) {
			PreDetonateTimer -= TimeManager::Get_Frame_Seconds ();

			if ( PreDetonateTimer <= 0 ) {
				PreDetonateTimer = 0;

				// Only on Server
				if ( CombatManager::I_Am_Server() ) {
					Start_Cinematic( Get_Definition ().PreDetonateCinematicDefID );
				}
			}
		}
	}

	//
	//	Handle each state independently
	//
	switch (State) {
		case STATE_NULL:
			break;

		case STATE_ARMING: {
			//
			//	Update the action bar in the HUD
			//
			float percent = (1.0F - StateTimer / Get_Definition ().ArmTime);
			HUDInfo::Set_Action_Status_Value (percent);

			//
			//	Did the player successfully arm the beacon?
			//
			if (StateTimer <= 0) {
				Set_State (STATE_ARMED);
			}
		}
		break;

		case STATE_ARMED:
			if (WarningTimer != 0.0f) {
				WarningTimer -= TimeManager::Get_Frame_Seconds();

				if (WarningTimer <= 0.0f) {
					WarningTimer = 0.0f;

					// Notify base controller beacon is disarmed
					BaseControllerClass* base = BaseControllerClass::Find_Base(Get_Player_Type());

					if (base) {
						base->On_Beacon_Warning(this);
					}
				}
			}
			break;

		case STATE_DISARMED:
			break;

		case STATE_DETONATING:
			// Wait for post-detonate timer
			if (StateTimer <= 0) {
				// Only on Server
				if ( CombatManager::I_Am_Server() ) {
					if ( CombatManager::Does_Beacon_Placement_Ends_Game() && Is_In_Enemy_Base() ) {
						BaseControllerClass *base = Get_Enemy_Base ();

						if (base != NULL) {
							//
							//	Destroy the enemy base
							//
							base->Destroy_Base ();
							base->Set_Beacon_Destroyed_Base(true);
						}
					}

					// cancel weather override
					Debug_Say(( "restore weather override\n" ));

					bool is_nuke = !!Get_Definition().IsNuke;
					if ( is_nuke ) {
						BackgroundMgrClass::Restore_Sky_Tint ( 5 );
						WeatherMgrClass::Restore_Wind( 5 );
						WeatherMgrClass::Restore_Precipitation( 5 );
					} else {
						BackgroundMgrClass::Restore_Clouds( 5 );
						BackgroundMgrClass::Restore_Lightning( 5 );
						WeatherMgrClass::Restore_Precipitation( 5 );
					}
				}

				Set_Delete_Pending();
			}
			break;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Enemy_Base
//
////////////////////////////////////////////////////////////////
BaseControllerClass *
BeaconGameObj::Get_Enemy_Base (void)
{
	//
	//	Lookup the team that this object is associated with
	//
	int player_type = PLAYERTYPE_GDI;
	if (Get_Player_Type () == PLAYERTYPE_GDI) {
		player_type = PLAYERTYPE_NOD;
	}

	return BaseControllerClass::Find_Base (player_type);
}


////////////////////////////////////////////////////////////////
//
//	Can_Place_Here
//
////////////////////////////////////////////////////////////////
bool
BeaconGameObj::Can_Place_Here (const Vector3 &position)
{
	// always return true;
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Is_In_Enemy_Base (CnC mode only)
//
////////////////////////////////////////////////////////////////
bool
BeaconGameObj::Is_In_Enemy_Base( void )
{
	bool retval = false;

	//
	//	Lookup the enemy's base
	//
	BaseControllerClass *base = Get_Enemy_Base ();
	if (base != NULL) {

		//
		//	Check to see if the point is inside the beacon's zone
		//
		Vector3 position;
		Get_Position (&position);
		const OBBoxClass &zone = base->Get_Beacon_Zone ();
		if (CollisionMath::Overlap_Test (zone, position) != CollisionMath::OUTSIDE) {
			retval = true;
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Stop_Current_Message_Sound
//
////////////////////////////////////////////////////////////////
void
BeaconGameObj::Stop_Current_Message_Sound (void)
{
	if (MessageSound != NULL) {
		MessageSound->Remove_From_Scene ();
		REF_PTR_RELEASE (MessageSound);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Display_Message
//
////////////////////////////////////////////////////////////////
void
BeaconGameObj::Display_Message (int text_id)
{
	Stop_Current_Message_Sound ();

	//
	//	Lookup the translation object from the strings database
	//
	TDBObjClass *translate_obj = TranslateDBClass::Find_Object (text_id);
	if (translate_obj != NULL) {

		const WCHAR *string		= translate_obj->Get_String ();
		int sound_def_id			= (int)translate_obj->Get_Sound_ID ();
		float duration				= 2.0F;

		//
		//	Play the sound effect
		//
		bool display_text = true;
		if (sound_def_id > 0) {

			//
			//	Create the sound object
			//
			MessageSound = WWAudioClass::Get_Instance ()->Create_Sound (sound_def_id);
			if (MessageSound != NULL) {
				duration = (MessageSound->Get_Duration () / 1000.0F);

				//
				//	Play the sound effect at the lcoation of the beacon
				//
				MessageSound->Set_Transform (Get_Transform ());
				MessageSound->Add_To_Scene ();
				display_text = (MessageSound->Is_Sound_Culled () == false);
			}
		}

		//
		//	Display the text on the screen
		//
		if (display_text && string != NULL) {
			float message_duration = max (duration, 5.0F);
			CombatManager::Get_Message_Window ()->Add_Message (string, Vector3 (1, 1, 1),
																NULL, message_duration);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Begin_Arming
//
////////////////////////////////////////////////////////////////
void
BeaconGameObj::Begin_Arming (void)
{
	//
	//	Switch states
	//
	Set_State (STATE_ARMING);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Start_Owner_Animation
//
////////////////////////////////////////////////////////////////
void
BeaconGameObj::Start_Owner_Animation (void)
{
	Restore_Owner();

	//
	//	Release control of the human's animation (if possible)
	//
	SoldierGameObj *soldier = Get_Owner ();
	if (soldier != NULL) {

		//
		//	Play the animation looped until we detect its time to stop
		//
		soldier->Set_Animation (Get_Definition ().ArmingAnimationName, true, 0);
	}

	// Only show the HUD if the owner if the star
	if ( soldier == COMBAT_STAR ) {
		HUDInfo::Display_Action_Status_Bar (true);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Stop_Owner_Animation
//
////////////////////////////////////////////////////////////////
void
BeaconGameObj::Stop_Owner_Animation (void)
{
	Restore_Owner();

	//
	//	Release control of the human's animation (if possible)
	//
	SoldierGameObj *soldier = Get_Owner ();
	if (soldier != NULL) {

		//
		//	Stop the animation
		//
		soldier->Set_Animation (NULL);
	}

	// Only show the HUD if the owner if the star
	if ( soldier == COMBAT_STAR ) {
		HUDInfo::Display_Action_Status_Bar (false);
	}
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Was_Owner_Interrupted
//
////////////////////////////////////////////////////////////////
bool
BeaconGameObj::Was_Owner_Interrupted (void)
{
	Restore_Owner();

	bool retval = false;

	//
	//	The owner is interrupted if he's dead
	//
	SoldierGameObj *soldier = Get_Owner ();
	if (soldier == NULL || soldier->Is_Dead ()) {
		retval = true;
	} else {

		//
		//	The owner is also interrupted if he moves
		//

		//
		//	Check each movement control
		//
		ControlClass &control = soldier->Get_Control ();
		// Ignore up/down and turning
//		for (int index = 0; index < ControlClass::ANALOG_CONTROL_COUNT; index ++) {
		int index;
		for (index = 0; index < ControlClass::ANALOG_MOVE_LEFT+1; index ++) {
			//
			//	Check this input to see if the soldier is moving
			//
			if (control.Get_Analog ((ControlClass::AnalogControl)index) != 0) {
				retval = true;
				break;
			}
		}

		//
		//	Check each "boolean" control (jumping, etc)
		//
		if (retval == false) {
			for (	index = ControlClass::BOOLEAN_ONE_TIME_FIRST;
					index <= ControlClass::BOOLEAN_DROP_FLAG;
					index ++)
			{
				//
				//	Check this input to see if the soldier is moving
				//
				if (control.Get_Boolean ((ControlClass::BooleanControl)index)) {
					retval = true;
					break;
				}
			}
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Get_Owner
//
////////////////////////////////////////////////////////////////
SoldierGameObj *
BeaconGameObj::Get_Owner (void)
{
	SoldierGameObj *soldier = NULL;

	//
	//	Dig the soldier pointer out of the referencer object
	//
	if (Owner != NULL) {
		PhysicalGameObj *physical_obj = Owner.Get_Ptr ()->As_PhysicalGameObj ();
		if (physical_obj != NULL) {
			soldier = physical_obj->As_SoldierGameObj ();
		}
	}

	return soldier;
}


////////////////////////////////////////////////////////////////
//
//	Completely_Damaged
//
////////////////////////////////////////////////////////////////
void
BeaconGameObj::Completely_Damaged (const OffenseObjectClass &damager)
{
	if (!Is_Delete_Pending()) {
		Set_State (STATE_DISARMED);
		Set_Delete_Pending ();
	}
}


////////////////////////////////////////////////////////////////
//
//	Create_Explosion
//
////////////////////////////////////////////////////////////////
void
BeaconGameObj::Create_Explosion (void)
{
	if (CombatManager::I_Am_Server())
	{
		Restore_Owner();
	}

	Debug_Say (("Create Explosion\n"));

	//
	//	Create the explosion...
	//
	Vector3 position;
	Get_Position (&position);
	
	//
	// (gth) don't explode if the owner is gone
	//
	if (Get_Owner() != NULL) {
	
		ExplosionManager::Create_Explosion_At (Get_Definition ().ExplosionDefID, Get_Transform (), Get_Owner ());

		if (CombatManager::I_Am_Server())
		{
			//
			//	Look-up the explosion that this beacon will create
			//
			ExplosionDefinitionClass *explosion_def = (ExplosionDefinitionClass *)DefinitionMgrClass::Find_Definition (Get_Definition ().ExplosionDefID);
			if (explosion_def != NULL) {
				float outter_radius	= explosion_def->DamageRadius;
				float outter_radius2 = outter_radius * outter_radius;

				//
				//	Loop over all the buildings in the level
				//
				SLNode<BuildingGameObj> *obj_node = NULL;
				for (	obj_node = GameObjManager::Get_Building_Game_Obj_List()->Head ();
						obj_node != NULL;
						obj_node = obj_node->Next ())
				{
					WWASSERT (obj_node->Data () != NULL);
					BuildingGameObj *building = obj_node->Data ();
					if (building != NULL) {

						//
						//	Check to see if any part of the building is inside our damage sphere
						//
						float dist2 = 0;
						building->Find_Closest_Poly (position, &dist2);
						if (dist2 <= outter_radius2) {

							//
							//	Determine the strength of the explosion at this location
							//
							float percent	= (WWMath::Sqrt (dist2) / outter_radius);
							percent			= 1.0F - WWMath::Clamp (percent, 0.0F, 1.0F);
							float strength = percent * explosion_def->DamageStrength;

							//
							//	Apply the damage to this building
							//
							OffenseObjectClass offense (strength, explosion_def->DamageWarhead, Get_Owner ());
							building->Apply_Damage_Building (offense, true);
						}
					}
				}
			}
		}
	}

	return ;
}

/*
**
*/
void	BeaconGameObj::Export_Rare( BitStreamClass &packet )
{
	Restore_Owner();

	SimpleGameObj::Export_Rare( packet );

	packet.Add( State );

	int owner_id = 0;
	if ( Get_Owner() ) {
		owner_id = Get_Owner()->Get_ID();
	}
	packet.Add( owner_id );

}


/*
**
*/
void	BeaconGameObj::Import_Rare( BitStreamClass &packet )
{
	SimpleGameObj::Import_Rare( packet );

	int state;
	packet.Get( state );

	int owner_id;
	packet.Get( owner_id );

	if ( owner_id != 0 ) {
		Owner = GameObjManager::Find_SmartGameObj( owner_id );
	}
	Set_State( state );

	return ;
}

void	BeaconGameObj::Restore_Owner( void )
{
	if ( Get_Owner() == NULL && OwnerBackup != NULL ) {
		// Try and find a smart game obj with the same playerdata

		SLNode<SmartGameObj> * smart_objnode;
		for (smart_objnode = GameObjManager::Get_Smart_Game_Obj_List()->Head(); smart_objnode; smart_objnode = smart_objnode->Next()) {
			SmartGameObj * obj = smart_objnode->Data();
			if ( obj->Get_Player_Data() == OwnerBackup ) {
				Owner = obj;
				Debug_Say(( "Found Beacon owner\n" ));
			}
		}

		if ( Get_Owner() == NULL ) {
			Debug_Say(( "Didn't find Beacon owner\n" ));
		}
	}
}