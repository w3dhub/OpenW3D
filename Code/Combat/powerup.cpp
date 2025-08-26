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
 *                     $Archive:: /Commando/Code/Combat/powerup.cpp                           $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 12/17/01 10:57a                                             $*
 *                                                                                             *
 *                    $Revision:: 93                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "powerup.h"
#include "debug.h"
#include "combat.h"
#include "phys.h"
#include "persistfactory.h"
#include "combatchunkid.h"
#include "simpledefinitionfactory.h"
#include "wwhack.h"
#include "weaponbag.h"
#include "damage.h"
#include "gameobjmanager.h"
#include "colmath.h"
#include "movephys.h"
#include "WWAudio.h"
#include "AudibleSound.h"
#include "hanim.h"
#include "timemgr.h"
#include "assetmgr.h"
#include "animcontrol.h"
#include "soldier.h"
#include "wwprofile.h"
#include "objlibrary.h"
#include "gameobjobserver.h"
#include "diaglog.h"
#include "weaponmanager.h"
#include "weapons.h"
#include "hud.h"
#include "playerdata.h"
#include "physinttest.h"
#include "encyclopediamgr.h"
#include "vehicle.h"
#include "combatmaterialeffectmanager.h"
#include "transitioneffect.h"
#include "apppackettypes.h"
#include "hudinfo.h"
#include "string_ids.h"
#include "translatedb.h"

/*
** PowerUpGameObjDef
*/
DECLARE_FORCE_LINK( PowerUp )

SimplePersistFactoryClass<PowerUpGameObjDef, CHUNKID_GAME_OBJECT_DEF_POWERUP>	_PowerUpGameObjDefPersistFactory;

DECLARE_DEFINITION_FACTORY(PowerUpGameObjDef, CLASSID_GAME_OBJECT_DEF_POWERUP, "PowerUp") _PowerUpGameObjDefDefFactory;

PowerUpGameObjDef::PowerUpGameObjDef( void ) :
	GrantShieldType( 0 ),
	GrantShieldStrength( 0 ),
	GrantShieldStrengthMax( 0 ),
	GrantHealth( 0 ),
	GrantHealthMax( 0 ),
	GrantWeaponID( 0 ),
	GrantWeapon( true ),
	GrantWeaponClips( false ),
	GrantWeaponRounds( 0 ),
	Persistent( false ),
	//IsCaptureTheFlag( false ),
	GrantKey( 0 ),
	GrantSoundID( 0 ),
	IdleSoundID( 0 ),
	AlwaysAllowGrant( false )
{
#ifdef PARAM_EDITING_ON
//	EDITABLE_PARAM( PowerUpGameObjDef, ParameterClass::TYPE_INT,	GrantShieldType );
	EnumParameterClass *param;
	param = new EnumParameterClass( &GrantShieldType );
	param->Set_Name ( "GrantShieldType" );
	for ( int param_counter = 0; param_counter < ArmorWarheadManager::Get_Num_Armor_Types(); param_counter++ ) {
		param->Add_Value(ArmorWarheadManager::Get_Armor_Name(param_counter), param_counter);
	}
	GENERIC_EDITABLE_PARAM( PowerUpGameObjDef, param )

	EDITABLE_PARAM( PowerUpGameObjDef, ParameterClass::TYPE_FLOAT, GrantShieldStrength );
	EDITABLE_PARAM( PowerUpGameObjDef, ParameterClass::TYPE_FLOAT, GrantShieldStrengthMax );
	EDITABLE_PARAM( PowerUpGameObjDef, ParameterClass::TYPE_FLOAT, GrantHealth );
	EDITABLE_PARAM( PowerUpGameObjDef, ParameterClass::TYPE_FLOAT, GrantHealthMax );
	EDITABLE_PARAM( PowerUpGameObjDef, ParameterClass::TYPE_WEAPONOBJDEFINITIONID,	GrantWeaponID );
	EDITABLE_PARAM( PowerUpGameObjDef, ParameterClass::TYPE_BOOL,	GrantWeapon );
	EDITABLE_PARAM( PowerUpGameObjDef, ParameterClass::TYPE_BOOL,	GrantWeaponClips );	
	EDITABLE_PARAM( PowerUpGameObjDef, ParameterClass::TYPE_INT,	GrantWeaponRounds );
	EDITABLE_PARAM( PowerUpGameObjDef, ParameterClass::TYPE_BOOL,	Persistent );
	//EDITABLE_PARAM( PowerUpGameObjDef, ParameterClass::TYPE_BOOL,	false );//IsCaptureTheFlag );
	EDITABLE_PARAM( PowerUpGameObjDef, ParameterClass::TYPE_INT,	GrantKey );
	EDITABLE_PARAM( PowerUpGameObjDef, ParameterClass::TYPE_BOOL,	AlwaysAllowGrant );

	EDITABLE_PARAM( PowerUpGameObjDef, ParameterClass::TYPE_STRING, GrantAnimationName );
	EDITABLE_PARAM( PowerUpGameObjDef, ParameterClass::TYPE_STRING, IdleAnimationName );
	EDITABLE_PARAM( PowerUpGameObjDef, ParameterClass::TYPE_SOUNDDEFINITIONID, GrantSoundID );
	EDITABLE_PARAM( PowerUpGameObjDef, ParameterClass::TYPE_SOUNDDEFINITIONID, IdleSoundID );
#endif	//PARAM_EDITING_ON
}

uint32	PowerUpGameObjDef::Get_Class_ID (void) const
{
	return CLASSID_GAME_OBJECT_DEF_POWERUP;
}

PersistClass *	PowerUpGameObjDef::Create( void ) const
{
	PowerUpGameObj * obj = new PowerUpGameObj;
	obj->Init( *this );
	return obj;
}

enum	{
	CHUNKID_DEF_PARENT							=	909991656,
	CHUNKID_DEF_VARIABLES,

	XXXMICROCHUNKID_DEF_PARAMETERS				=	1,
	MICROCHUNKID_DEF_PERSISTENT,
	MICROCHUNKID_DEF_GRANT_SHIELD_TYPE,
	MICROCHUNKID_DEF_GRANT_SHIELD_STRENGTH,
	XXXMICROCHUNKID_DEF_GRANT_SHIELD_STRENGTH_MAX,
	MICROCHUNKID_DEF_GRANT_HEALTH,
	XXXMICROCHUNKID_DEF_GRANT_HEALTH_MAX,
	MICROCHUNKID_DEF_GRANT_WEAPON_ID,
	MICROCHUNKID_DEF_GRANT_WEAPON,
	MICROCHUNKID_DEF_GRANT_WEAPON_ROUNDS,
	XXXMICROCHUNKID_DEF_IS_CAPTURE_THE_FLAG,
	XXXMICROCHUNKID_DEF_GRANT_KEY_MASK,

	MICROCHUNKID_DEF_GRANT_ANIMATION_NAME,
	MICROCHUNKID_DEF_GRANT_SOUNDID,
	MICROCHUNKID_DEF_IDLE_ANIMATION_NAME,
	MICROCHUNKID_DEF_IDLE_SOUNDID,
	MICROCHUNKID_DEF_GRANT_KEY,
	MICROCHUNKID_DEF_ALWAYS_ALLOW_GRANT,
	MICROCHUNKID_DEF_GRANT_WEAPON_CLIPS,

	MICROCHUNKID_DEF_GRANT_SHIELD_STRENGTH_MAX,
	MICROCHUNKID_DEF_GRANT_HEALTH_MAX,

};

bool	PowerUpGameObjDef::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_DEF_PARENT );
		SimpleGameObjDef::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_DEF_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_GRANT_SHIELD_TYPE,			GrantShieldType );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_GRANT_SHIELD_STRENGTH,		GrantShieldStrength );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_GRANT_SHIELD_STRENGTH_MAX,GrantShieldStrengthMax );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_GRANT_HEALTH,					GrantHealth );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_GRANT_HEALTH_MAX,			GrantHealthMax );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_GRANT_WEAPON_ID,				GrantWeaponID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_GRANT_WEAPON,					GrantWeapon );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_GRANT_WEAPON_CLIPS,			GrantWeaponClips );		
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_GRANT_WEAPON_ROUNDS,		GrantWeaponRounds );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_PERSISTENT,					Persistent );
		//WRITE_MICRO_CHUNK( csave, XXXMICROCHUNKID_DEF_IS_CAPTURE_THE_FLAG,		IsCaptureTheFlag );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_GRANT_KEY,						GrantKey );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_GRANT_SOUNDID,				GrantSoundID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_IDLE_SOUNDID,					IdleSoundID );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_GRANT_ANIMATION_NAME,		GrantAnimationName );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_IDLE_ANIMATION_NAME,		IdleAnimationName );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_ALWAYS_ALLOW_GRANT,			AlwaysAllowGrant );
	csave.End_Chunk();

	return true;
}

bool	PowerUpGameObjDef::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_DEF_PARENT:
				SimpleGameObjDef::Load( cload );
				break;

			case CHUNKID_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_GRANT_SHIELD_TYPE,			GrantShieldType );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_GRANT_SHIELD_STRENGTH,		GrantShieldStrength );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_GRANT_SHIELD_STRENGTH_MAX,	GrantShieldStrengthMax );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_GRANT_HEALTH,					GrantHealth );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_GRANT_HEALTH_MAX,				GrantHealthMax );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_GRANT_WEAPON_ID,				GrantWeaponID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_GRANT_WEAPON,					GrantWeapon );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_GRANT_WEAPON_CLIPS,			GrantWeaponClips );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_GRANT_WEAPON_ROUNDS,			GrantWeaponRounds );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_PERSISTENT,						Persistent );
						//READ_MICRO_CHUNK( cload, XXXMICROCHUNKID_DEF_IS_CAPTURE_THE_FLAG,			IsCaptureTheFlag );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_GRANT_KEY,						GrantKey );

						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_GRANT_SOUNDID,					GrantSoundID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_IDLE_SOUNDID,					IdleSoundID );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_DEF_GRANT_ANIMATION_NAME,	GrantAnimationName );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_DEF_IDLE_ANIMATION_NAME,		IdleAnimationName );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_ALWAYS_ALLOW_GRANT,			AlwaysAllowGrant );

						default:
							Debug_Say(("Unhandled Micro Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
							break;

					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

const PersistFactoryClass & PowerUpGameObjDef::Get_Factory (void) const
{
	return _PowerUpGameObjDefPersistFactory;
}

bool	PowerUpGameObjDef::Grant( SmartGameObj * obj, PowerUpGameObj * p_powerup, bool hud_display ) const
{
	int no_grant_message = 0;

	bool granted = false;

	WWASSERT(CombatManager::I_Am_Server());

	DefenseObjectClass * defense = obj->Get_Defense_Object();
	// Grant the shield
	if ( GrantShieldType != 0 ) {
		if ( GrantShieldType > (int)defense->Get_Shield_Type() ) {
			defense->Set_Shield_Type( GrantShieldType );
			granted = true;
		} else {
			no_grant_message = IDS_M00DSGN_DSGN1015I1DSGN_TXT; //"You are already at full shield."
		}
	}

	if ( GrantShieldStrengthMax != 0 ) {
		float add = GrantShieldStrengthMax * (float)obj->Get_Definition().Get_DefenseObjectDef().ShieldStrengthMax;

		switch ( CombatManager::Get_Difficulty_Level() ) {
			case 0:	add *= 2.0f;	break;
			case 2:	add *= 0.75f;	break;
		};

		// Round up to next int
		add = (int)(add + 0.95f);

		defense->Set_Shield_Strength_Max( defense->Get_Shield_Strength_Max() + add );
		granted = true;

		if ( hud_display && obj == COMBAT_STAR ) {
			HUDClass::Add_Shield_Upgrade_Grant( add );
		}

	}

	if ( GrantShieldStrength != 0 ) {
		if ( (defense->Get_Shield_Strength() < defense->Get_Shield_Strength_Max()) ) {
			defense->Add_Shield_Strength( GrantShieldStrength );
			granted = true;

			if ( obj == COMBAT_STAR ) {
				Vector3 pos;
				obj->Get_Position( &pos );
				DIAG_LOG(( "AMPU", "%1.2f; %1.2f; %1.2f; %1.2f; %1.2f", pos.X, pos.Y, pos.Z, defense->Get_Shield_Strength(), defense->Get_Health() ));
			}
		} else {
			no_grant_message = IDS_M00DSGN_DSGN1015I1DSGN_TXT; //"You are already at full shield."
		}
	}

	if ( granted && hud_display ) {
		if ( obj == COMBAT_STAR ) {
			if ( GrantShieldStrengthMax == 0 ) {
				HUDClass::Add_Shield_Grant( GrantShieldStrength );
			}
		}
	}

	// Grant the Health
	if ( GrantHealthMax != 0 ) {
		float add = GrantHealthMax * (float)obj->Get_Definition().Get_DefenseObjectDef().HealthMax;

		switch ( CombatManager::Get_Difficulty_Level() ) {
			case 0:	add *= 2.0f;	break;
			case 2:	add *= 0.75f;	break;
		};

		// Round up to next int
		add = (int)(add + 0.95f);

		defense->Set_Health_Max( defense->Get_Health_Max() + add );
		granted = true;

		if ( hud_display && obj == COMBAT_STAR ) {
			HUDClass::Add_Health_Upgrade_Grant( add );
		}
	}

	if ( GrantHealth != 0 ) {
		if ( defense->Get_Health() < defense->Get_Health_Max() ) {
			defense->Add_Health( GrantHealth );
			granted = true;

			if ( obj == COMBAT_STAR && hud_display  ) {
				if ( GrantHealthMax == 0 ) {
					HUDClass::Add_Health_Grant( GrantHealth );
				}
			}

			if ( obj == COMBAT_STAR ) {
				Vector3 pos;
				obj->Get_Position( &pos );
				DIAG_LOG(( "HEPU", "%1.2f; %1.2f; %1.2f; %1.2f; %1.2f", pos.X, pos.Y, pos.Z, defense->Get_Shield_Strength(), defense->Get_Health() ));
			}
		} else {
			no_grant_message = IDS_M00DSGN_DSGN1014I1DSGN_TXT; //"You are already at full health."
		}
	}

	// Grant the Weapon
	if ( GrantWeaponID != 0 ) {

		if ( ( GrantWeapon && !obj->Get_Weapon_Bag()->Is_Weapon_Owned( GrantWeaponID ) ) ||
			  ( !obj->Get_Weapon_Bag()->Is_Ammo_Full( GrantWeaponID ) ) ) {

			if ( obj == COMBAT_STAR && hud_display ) {
				if ( GrantWeapon && !obj->Get_Weapon_Bag()->Is_Weapon_Owned( GrantWeaponID ) )  {
					HUDClass::Add_Powerup_Weapon( GrantWeaponID, GrantWeaponRounds );
				} else {
					if ( !obj->Get_Weapon_Bag()->Is_Ammo_Full( GrantWeaponID ) ) {
						HUDClass::Add_Powerup_Ammo( GrantWeaponID, GrantWeaponRounds );
					}
				}
			}

			obj->Get_Weapon_Bag()->Add_Weapon( GrantWeaponID, GrantWeaponRounds, GrantWeapon );
			granted = true;

			if ( obj == COMBAT_STAR ) {
				Vector3 pos;
				obj->Get_Position( &pos );
				const char * grant_name = "";
				const WeaponDefinitionClass * def = WeaponManager::Find_Weapon_Definition( GrantWeaponID );
				if ( def ) {
					grant_name = def->Get_Name();
				}
				const char * weapon_name = "";
				int ammo = 0;
				if ( obj->Get_Weapon() ) {
					weapon_name = obj->Get_Weapon()->Get_Definition()->Get_Name();
					ammo = obj->Get_Weapon()->Get_Total_Rounds();
				}
				DIAG_LOG(( "WEPU", "%1.2f; %1.2f; %1.2f; %s; %d; %s; %d", pos.X, pos.Y, pos.Z, grant_name, GrantWeaponRounds, weapon_name, ammo ));
			}
		} else {
			no_grant_message = IDS_M00DSGN_DSGN1016I1DSGN_TXT; //"Your weapon is full."
		}

	} else if ( GrantWeaponClips ) {
		
		//
		//	Loop over all the weapons in the owner's bag
		//
		WeaponBagClass *weapon_bag = obj->Get_Weapon_Bag();
		for ( int weapon_index = 0; weapon_index < weapon_bag->Get_Count(); weapon_index ++ ) {
			WeaponClass	*weapon = weapon_bag->Peek_Weapon( weapon_index );
			if( weapon != NULL && weapon->Get_Definition ()->CanReceiveGenericCnCAmmo ) {

				//
				//	Grant "x" number of clips to the weapon
				//
				int clip_rounds = weapon->Get_Definition()->ClipSize;
				weapon->Add_Rounds( clip_rounds * GrantWeaponRounds );
			}
		}
	}


	// Grant the key
	if ( GrantKey != 0 ) {
		SoldierGameObj * soldier = obj->As_SoldierGameObj();
		if ( soldier && soldier->Is_Human_Controlled() ) {
			if ( !soldier->Has_Key( GrantKey ) ) {
				soldier->Give_Key( GrantKey );
				granted = true;
			}
		}
		if ( obj == COMBAT_STAR ) {
			Vector3 pos;
			obj->Get_Position( &pos );
			DIAG_LOG(( "KEPU", "%1.2f; %1.2f; %1.2f; %d", pos.X, pos.Y, pos.Z, GrantKey ));
		}

		if ( granted && hud_display && obj == COMBAT_STAR ) {
			HUDClass::Add_Key_Grant( GrantKey );
		}
	}

	/*
	// Handle Capture the Flag
	if ( IsCaptureTheFlag && p_powerup != NULL && obj->As_SoldierGameObj() != NULL ) {
		CombatManager::Soldier_Contacts_Flag( obj->As_SoldierGameObj(), p_powerup );
		granted = true;
	}
	*/

	if ( AlwaysAllowGrant ) {
		granted = true;
	}

	if ( granted && p_powerup != NULL ) {
		p_powerup->Set_State( PowerUpGameObj::STATE_GRANTING );

		//
		//	Reveal this object to the player
		//
		if ( COMBAT_STAR == obj ) {
			EncyclopediaMgrClass::Reveal_Object( p_powerup );
		}
	}

	// Stats
	if ( granted && obj->Get_Player_Data() ) {
		obj->Get_Player_Data()->Stats_Add_Powerup();
	}

	if ( !granted && ( COMBAT_STAR == obj ) && no_grant_message != 0 ) {
		HUDInfo::Set_HUD_Help_Text( TRANSLATE( no_grant_message ), Vector3( 0,1,0 ) );

	}

	return granted;
}


/*
** PowerUpGameObj
*/
SimplePersistFactoryClass<PowerUpGameObj, CHUNKID_GAME_OBJECT_POWERUP>	_PowerUpGameObjPersistFactory;

PowerUpGameObj::PowerUpGameObj (void)	:
	IdleSoundObj( NULL ),
	State( STATE_BECOMING_IDLE ),
	WeaponBag( NULL )
{
	Set_App_Packet_Type(APPPACKETTYPE_POWERUP);
}

PowerUpGameObj::~PowerUpGameObj (void)
{
	//
	//	Cleanup the idle sound
	//
	if ( IdleSoundObj != NULL ) {
		IdleSoundObj->Remove_From_Scene();
		IdleSoundObj->Release_Ref();
		IdleSoundObj = NULL;
	}

	if ( WeaponBag != NULL ) {
		delete WeaponBag;
		WeaponBag = NULL;
	}

	return ;
}

const PersistFactoryClass & PowerUpGameObj::Get_Factory (void) const
{
	return _PowerUpGameObjPersistFactory;
}

/*
**
*/
void PowerUpGameObj::Init( void )
{
	Init( Get_Definition() );
}

/*
**
*/
void	PowerUpGameObj::Init( const PowerUpGameObjDef & definition )
{
	SimpleGameObj::Init( definition );

	// Only collide with terrain!
	Peek_Physical_Object()->Set_Collision_Group( TERRAIN_ONLY_COLLISION_GROUP );
}

const PowerUpGameObjDef & PowerUpGameObj::Get_Definition( void ) const
{
	return (const PowerUpGameObjDef &)BaseGameObj::Get_Definition();
}


/*
** PowerUpGameObj Save and Load
*/
enum	{
	CHUNKID_PARENT							=	927991635,
	CHUNKID_VARIABLES,
	CHUNKID_WEAPONBAG,

	MICROCHUNKID_STATE					=	1,
	MICROCHUNKID_STATE_END_TIMER,
};

bool	PowerUpGameObj::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_PARENT );
		SimpleGameObj::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STATE,				State );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STATE_END_TIMER,	StateEndTimer );
	csave.End_Chunk();

	if ( WeaponBag != NULL ) {
		csave.Begin_Chunk( CHUNKID_WEAPONBAG );
		WeaponBag->Save( csave );
		csave.End_Chunk();
	}

// We don't save IdleSoundObj

	return true;
}

bool	PowerUpGameObj::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PARENT:
				SimpleGameObj::Load( cload );
				break;

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STATE,					State );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STATE_END_TIMER,	StateEndTimer );

						default:
							Debug_Say(( "Unrecognized PowerupGameObj Variable chunkID\n" ));
							break;

					}
					cload.Close_Micro_Chunk();
				}

				break;

			case CHUNKID_WEAPONBAG:
				WWASSERT( WeaponBag == NULL );
				WeaponBag = new WeaponBagClass( NULL );
			 	WeaponBag->Load( cload );
				break;

			default:
				Debug_Say(( "Unrecognized PowerUpGameObj chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	SaveLoadSystemClass::Register_Post_Load_Callback(this);		// MOVED
	return true;
}

void	PowerUpGameObj::On_Post_Load( void )
{
	SimpleGameObj::On_Post_Load();
	Peek_Physical_Object()->Set_Collision_Group( UNCOLLIDEABLE_GROUP );	// MOVED
	if ( Peek_Physical_Object()->As_MoveablePhysClass() != NULL ) {
		Peek_Physical_Object()->As_MoveablePhysClass()->Set_Gravity_Multiplier( 0 );
	}

	// This allows the idle sound and animation to start after loading
	if ( State == STATE_IDLING ) {
		State = STATE_BECOMING_IDLE;
	}

}

void	PowerUpGameObj::Set_State( int state )
{
	if (state != State) {
		State				= state;
		StateEndTimer	= 0;

		if ( State == STATE_GRANTING ) {

			//
			//	Stop the idling sound (if necessary)
			//
			if ( IdleSoundObj != NULL ) {
				IdleSoundObj->Remove_From_Scene();
				IdleSoundObj->Stop();
			}

			//
			// Play the grant sound (if exists)
			//
			if ( Get_Definition ().GrantSoundID != 0 ) {
				WWAudioClass::Get_Instance()->Create_Instant_Sound( Get_Definition ().GrantSoundID, Get_Transform () );
			}

			//
			// Play the grant animation (if exists)
			//
			if ( Get_Definition ().GrantAnimationName.Get_Length () > 0 ) {
				Set_Animation( Get_Definition ().GrantAnimationName, false );
				HAnimClass *animation =  WW3DAssetManager::Get_Instance ()->Get_HAnim( Get_Anim_Control ()->Get_Animation_Name () );
				if ( animation != NULL ) {
					StateEndTimer = animation->Get_Total_Time();
					REF_PTR_RELEASE( animation );
				}
			}
		}
	}

	return ;
}

void	PowerUpGameObj::Update_State( void )
{
	switch ( State )
	{
		case STATE_IDLING:
			break;

		case STATE_BECOMING_IDLE:

			//
			//	Start playing the idle sound
			//
			if ( Get_Definition ().IdleSoundID != 0 ) {
				if ( IdleSoundObj == NULL ) {
					IdleSoundObj = WWAudioClass::Get_Instance ()->Create_Continuous_Sound( Get_Definition ().IdleSoundID );
				}
				if ( IdleSoundObj != NULL ) {
					IdleSoundObj->Set_Transform( Get_Transform () );
					IdleSoundObj->Add_To_Scene( true );
				}
			}

			//
			//	Start playing the idle animation
			//
			if ( Get_Definition ().IdleAnimationName.Get_Length () > 0 ) {
				Set_Animation( Get_Definition ().IdleAnimationName, true );
			}

			State = STATE_IDLING;
			break;

		case STATE_GRANTING:

			//
			//	If the granting animation has finished, then change to another state (or remove
			// the powerup from the world).
			//
			StateEndTimer -= TimeManager::Get_Frame_Seconds();
			if ( StateEndTimer <= 0 ) {
				//if ( Get_Definition().IsCaptureTheFlag || Get_Definition().Persistent ) {
				if ( Get_Definition().Persistent ) {
					Set_State ( STATE_BECOMING_IDLE );
				} else {
					Set_Delete_Pending();
				}
			}
			break;

		case STATE_EXPIRING:

			//
			//	If the granting animation has finished, then change to another state (or remove
			// the powerup from the world).
			//
			StateEndTimer -= TimeManager::Get_Frame_Seconds();
			if ( StateEndTimer <= 0 ) {
				Set_Delete_Pending();
			}
			break;
	}

	return ;
}

void	PowerUpGameObj::Grant( SmartGameObj * obj )
{
	WWASSERT ( State != STATE_GRANTING );

	WWASSERT( obj );

	// Grant Def
	Get_Definition().Grant( obj, this );

	// If we have a weapon bag, move it
	if ( WeaponBag != NULL ) {
		WWASSERT( obj->Get_Weapon_Bag() );
		if ( obj->Get_Weapon_Bag()->Move_Contents( WeaponBag ) ) {
			Set_State( PowerUpGameObj::STATE_GRANTING );
		}
	}

	if ( State == STATE_GRANTING ) {
		const GameObjObserverList & observer_list = Get_Observers();
		for( int index = 0; index < observer_list.Count(); index++ ) {
			observer_list[ index ]->Custom( this, CUSTOM_EVENT_POWERUP_GRANTED, 0, obj );
		}
	}
}


void	PowerUpGameObj::Think( void )
{
	SimpleGameObj::Think();

	WWPROFILE( "PowerUp Think" );

	//
	//	Make sure the powerup is playing its correct animation and sound
	//
	Update_State();

	//
	//	If this powerup isn't currently granting itself to a player, then check
	// to see if it should!
	//
	// The server grants the prize, but allow the client to destroy
	// the powerup before being instructed to do so,
	// so that this doesn't lag
	//
	if ( CombatManager::I_Am_Server() && State != STATE_GRANTING ) {

		// Check my bounding box for collisions with Soldiers
		AABoxClass box = Peek_Model()->Get_Bounding_Box();

		SLNode<SmartGameObj> * smart_objnode;
		for (smart_objnode = GameObjManager::Get_Smart_Game_Obj_List()->Head(); smart_objnode; smart_objnode = smart_objnode->Next()) {
			SmartGameObj * obj = smart_objnode->Data();
			WWASSERT( obj != NULL );

			SoldierGameObj * soldier = obj->As_SoldierGameObj();

			if ( obj->As_VehicleGameObj() ) {
				soldier = obj->As_VehicleGameObj()->Get_Driver();
			}

			if ( soldier && soldier->Wants_Powerups() ) {

				PhysAABoxIntersectionTestClass test( box, DEFAULT_COLLISION_GROUP, COLLISION_TYPE_PHYSICAL );
				bool result = obj->Peek_Physical_Object()->Intersection_Test(test);
				if ( result ) {
					Grant( soldier );		// Don't grant any more
					break;
				}
			}
  		}
	}

	return ;
}

PowerUpGameObj *	PowerUpGameObj::Create_Backpack( ArmedGameObj * provider )
{
	WWASSERT( provider );

//	Debug_Say(( "Creating a Backpack\n" ));

	PowerUpGameObj * backpack = (PowerUpGameObj *)ObjectLibraryManager::Create_Object( "Backpack" );

	if ( backpack != NULL ) {
		Matrix3D tm(1);
		tm.Set_Translation( provider->Get_Bullseye_Position() );
		backpack->Set_Transform(tm);
		backpack->WeaponBag = new WeaponBagClass( NULL );
		backpack->WeaponBag->Move_Contents( provider->Get_Weapon_Bag() );
		backpack->Start_Observers();
	}

	return backpack;
}


//void	PowerUpGameObj::Get_Extended_Information( StringClass & description ) 
void	PowerUpGameObj::Get_Description( StringClass & description ) 
{
	//
	// Construct a diagnostic string
	//

	StringClass line;

   line.Format("ID:    %d\n", Get_ID());
	description += line;

   line.Format("Name:  %s\n", Get_Definition().Get_Name());
	description += line;

	Vector3 position;
	Get_Position(&position);
	line.Format("POS:   %-5.2f, %-5.2f, %-5.2f\n", position.X, position.Y, position.Z);
	description += line;
	
	if (Get_Defense_Object() != NULL) {
		line.Format("HLTH:  %-5.2f\n", Get_Defense_Object()->Get_Health());
		description += line;
	}

	line.Format("HIB:   %d\n", Is_Hibernating());
	description += line;

	line.Format("ISC:   %d\n", Get_Import_State_Count());
	description += line;
}

void	PowerUpGameObj::Expire( void )
{
#define		EXPIRE_TIME		2

	if ( State != STATE_EXPIRING ) {
		/*
		** If the definition calls for it, add a material effect to the object
		*/
		PhysClass * physobj = Peek_Physical_Object();
		if (physobj != NULL) {
			TransitionEffectClass * effect = CombatMaterialEffectManager::Get_Death_Effect();
			effect->Set_Transition_Time( EXPIRE_TIME );
			physobj->Add_Effect_To_Me(effect);
			REF_PTR_RELEASE(effect);
		}

		State = STATE_EXPIRING;
		StateEndTimer = EXPIRE_TIME;
	}
}
