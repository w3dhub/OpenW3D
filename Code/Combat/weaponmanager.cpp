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
 *                     $Archive:: /Commando/Code/Combat/weaponmanager.cpp                     $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/02/02 3:05p                                               $*
 *                                                                                             *
 *                    $Revision:: 106                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "weaponmanager.h"
#include "weapons.h"
#include "debug.h"
#include "damage.h"
#include "combatchunkid.h"
#include "simpledefinitionfactory.h"
#include "persistfactory.h"
#include "assets.h"
#include "surfaceeffects.h"

/*
** Weapon Type Names
*/
const char *WeaponStyleNames[ NUM_WEAPON_HOLD_STYLES ] = {
	"C4",					// WEAPON_HOLD_STYLE_C4					= 0,
	"---",				// WEAPON_HOLD_STYLE_NOT_USED,
	"Shoulder",			// WEAPON_HOLD_STYLE_AT_SHOULDER,	// 2
	"Hip",				// WEAPON_HOLD_STYLE_AT_HIP,
	"Launcher",			// WEAPON_HOLD_STYLE_LAUNCHER,
	"Handgun",			// WEAPON_HOLD_STYLE_HANDGUN,
	"Beacon",			// WEAPON_HOLD_STYLE_BEACON
	"Empty Hands",		// WEAPON_HOLD_STYLE_EMPTY_HANDS,
	"At Chest",			// WEAPON_HOLD_STYLE_AT_CHEST,
	"Hands Down",		// WEAPON_HOLD_STYLE_HANDS_DOWN,
};

/*
** WeaponDefinitionClass
*/
SimplePersistFactoryClass<WeaponDefinitionClass, CHUNKID_WEAPON_DEF>	_WeaponDefPersistFactory;

DECLARE_DEFINITION_FACTORY(WeaponDefinitionClass, CLASSID_DEF_WEAPON, "Weapon") _WeaponDefDefFactory;

uint32	WeaponDefinitionClass::Get_Class_ID (void) const	{ return CLASSID_DEF_WEAPON; }
const PersistFactoryClass & WeaponDefinitionClass::Get_Factory (void) const { return _WeaponDefPersistFactory; }

WeaponDefinitionClass::WeaponDefinitionClass( void ) :
	Style( 1 ),
	SwitchTime( 0 ),
	ReloadTime( 0 ),
	KeyNumber( 0 ),
	CanSnipe( false ),
	CanReceiveGenericCnCAmmo( true ),
//	LegacyFireSoundDefID( 0 ),
	EjectPhysDefID( 0 ),
	MuzzleFlashPhysDefID( 0 ),
	Rating( 0.1f ),
	FirstPersonOffset( 0,0,0 ),
	RecoilImpulse( 0.0f ),
	ReloadSoundDefID( 0 ),
	EmptySoundDefID( 0 ),
	PrimaryAmmoDefID( 0 ),
	SecondaryAmmoDefID( 0 ),
	ClipSize( 0 ),
	RecoilTime( 0.1f ),
	RecoilScale( 1.0f ),
	AGiveWeaponsWeapon( false ),
	MaxInventoryRounds( 100 ),
	IconNameID( 0 ),
	IconTextureUV( 0,0,0,0 ),
	IconOffset( 0,0 )
{
#ifdef PARAM_EDITING_ON
	int i;
//	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_INT,			Style);
	EnumParameterClass *param;
	param = new EnumParameterClass( &Style );
	param->Set_Name ( "Style" );
	for ( i = 0; i <= WEAPON_HOLD_STYLE_HANDGUN; i++ ) {
		param->Add_Value ( WeaponStyleNames[i], i );
	}
	param->Add_Value ( WeaponStyleNames[WEAPON_HOLD_STYLE_BEACON], WEAPON_HOLD_STYLE_BEACON );
	GENERIC_EDITABLE_PARAM(WeaponDefinitionClass,param)

	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_FILENAME,	Model);
	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_FILENAME,	IdleAnim);
	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_FILENAME,	FireAnim);
	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_FILENAME,	BackModel);
	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_FLOAT,		SwitchTime);
	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_FLOAT,		ReloadTime);
	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_FLOAT,		KeyNumber);

	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_BOOL,			CanSnipe);
	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_BOOL,			CanReceiveGenericCnCAmmo);	
	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_FLOAT,		Rating);

	PHYS_DEF_PARAM( WeaponDefinitionClass, EjectPhysDefID,			"ProjectileDef" );
	PHYS_DEF_PARAM( WeaponDefinitionClass, MuzzleFlashPhysDefID,	"TimedDecorationPhysDef" );

	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_FILENAME,	FirstPersonModel );
	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_VECTOR3,		FirstPersonOffset );
	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_FLOAT,		RecoilImpulse);

	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_FILENAME,	HUDIconTextureName );
	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_SOUNDDEFINITIONID,	ReloadSoundDefID);
	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_SOUNDDEFINITIONID,	EmptySoundDefID);

	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_AMMOOBJDEFINITIONID,	PrimaryAmmoDefID);
	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_AMMOOBJDEFINITIONID,	SecondaryAmmoDefID);

	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_INT,			ClipSize);
	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_INT,			MaxInventoryRounds);

	FLOAT_UNITS_PARAM( WeaponDefinitionClass, RecoilTime, 0.0f, 10.0f, "seconds" );
	FLOAT_EDITABLE_PARAM( WeaponDefinitionClass, RecoilScale, 0.0f, 10.0f );

	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_BOOL,			AGiveWeaponsWeapon);

	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_STRINGSDB_ID,		IconNameID );
	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_FILENAME,		IconTextureName );
	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_RECT,			IconTextureUV );
	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_VECTOR2,		IconOffset );

	EDITABLE_PARAM( WeaponDefinitionClass, ParameterClass::TYPE_STRING,		HumanFiringAnimation);

#endif	//PARAM_EDITING_ON
}

/*
**
*/
enum	{
	CHUNKID_WEAPON_DEF_VARIABLES							=	1205091654,
	CHUNKID_WEAPON_DEF_PARENT,

	MICROCHUNKID_WEAPON_DEF_STYLE							=	1,
	MICROCHUNKID_WEAPON_DEF_MODEL,
	MICROCHUNKID_WEAPON_DEF_IDLE_ANIM,
	MICROCHUNKID_WEAPON_DEF_FIRE_ANIM,
	MICROCHUNKID_WEAPON_DEF_BACK_MODEL,
	MICROCHUNKID_WEAPON_DEF_XXXXX,
	XXXMICROCHUNKID_WEAPON_DEF_RATE_OF_FIRE,
	MICROCHUNKID_WEAPON_DEF_SWITCH_TIME,
	MICROCHUNKID_WEAPON_DEF_RELOAD_TIME,
	XXXMICROCHUNKID_WEAPON_DEF_HELP_ANGLE,
	XXXMICROCHUNKID_WEAPON_DEF_MUZZLE_FLASH_MODEL,
	XXXMICROCHUNKID_WEAPON_DEF_MUZZLE_FLASH_DURATION,
	XXXMICROCHUNKID_WEAPON_DEF_EJECT_EMITTER,
	XXXMICROCHUNKID_WEAPON_DEF_TILT,
	XXXMICROCHUNKID_WEAPON_DEF_KEY_NUMBER,
	MICROCHUNKID_WEAPON_DEF_CAN_SNIPE,
	XXXMICROCHUNKID_WEAPON_DEF_SOUND_RADIUS,
	LEGACY_MICROCHUNKID_WEAPON_DEF_FIRE_SOUND_DEFID,
	MICROCHUNKID_WEAPON_DEF_EJECT_PHYS_DEF_ID,
	MICROCHUNKID_WEAPON_DEF_MUZZLE_FLASH_PHYS_DEF_ID,
	MICROCHUNKID_WEAPON_DEF_RATING,
	MICROCHUNKID_WEAPON_DEF_FIRST_PERSON_MODEL,
	MICROCHUNKID_WEAPON_DEF_FIRST_PERSON_OFFSET,
	MICROCHUNKID_WEAPON_DEF_RECOIL_IMPULSE,
	XXX_MICROCHUNKID_WEAPON_DEF_HUD_MODEL,
	MICROCHUNKID_WEAPON_DEF_RELOAD_SOUND_DEFID,
	MICROCHUNKID_WEAPON_DEF_PRIMARY_AMMO_DEF_ID,
	MICROCHUNKID_WEAPON_DEF_SECONDARY_AMMO_DEF_ID,
	MICROCHUNKID_WEAPON_DEF_CLIP_SIZE,
	MICROCHUNKID_WEAPON_DEF_RECOIL_TIME,
	MICROCHUNKID_WEAPON_DEF_RECOIL_SCALE,
	MICROCHUNKID_WEAPON_DEF_A_GIVE_WEAPONS_WEAPON,
	XXXMICROCHUNKID_WEAPON_DEF_FIRST_PERSON_TYPE,
	MICROCHUNKID_WEAPON_DEF_HUD_ICON_TEXTURE_NAME,
	MICROCHUNKID_WEAPON_DEF_MAX_INVENTORY_ROUNDS,
	XXXMICROCHUNKID_WEAPON_DEF_FIRST_PERSON_TYPE_NAME,
	MICROCHUNKID_WEAPON_DEF_KEY_NUMBER,
	MICROCHUNKID_WEAPON_ICON_NAME_ID,
	MICROCHUNKID_WEAPON_ICON_TEXTURE_NAME,
	MICROCHUNKID_WEAPON_ICON_TEXTURE_UV,
	MICROCHUNKID_WEAPON_ICON_OFFSET,
	MICROCHUNKID_HUMAN_FIRING_ANIMATION,
	MICROCHUNKID_WEAPON_DEF_EMPTY_SOUND_DEFID,
	MICROCHUNKID_WEAPON_DEF_GENERIC_AMMO_OK
};

bool	WeaponDefinitionClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_WEAPON_DEF_PARENT );
		DefinitionClass::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_WEAPON_DEF_VARIABLES );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_WEAPON_DEF_STYLE,					Style );
		WRITE_MICRO_CHUNK_WWSTRING( csave, 	MICROCHUNKID_WEAPON_DEF_MODEL,					Model );
		WRITE_MICRO_CHUNK_WWSTRING( csave, 	MICROCHUNKID_WEAPON_DEF_IDLE_ANIM,		  		IdleAnim );
		WRITE_MICRO_CHUNK_WWSTRING( csave, 	MICROCHUNKID_WEAPON_DEF_FIRE_ANIM,		  		FireAnim );
		WRITE_MICRO_CHUNK_WWSTRING( csave, 	MICROCHUNKID_WEAPON_DEF_BACK_MODEL,				BackModel );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_WEAPON_DEF_SWITCH_TIME,			SwitchTime );
		WRITE_SAFE_MICRO_CHUNK(		 csave, 	MICROCHUNKID_WEAPON_DEF_RELOAD_TIME,			ReloadTime, float );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_WEAPON_DEF_KEY_NUMBER,				KeyNumber );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_WEAPON_DEF_CAN_SNIPE,				CanSnipe );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_WEAPON_DEF_GENERIC_AMMO_OK,		CanReceiveGenericCnCAmmo );		
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_WEAPON_DEF_EJECT_PHYS_DEF_ID, 	EjectPhysDefID );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_WEAPON_DEF_MUZZLE_FLASH_PHYS_DEF_ID, 	MuzzleFlashPhysDefID );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_WEAPON_DEF_RATING,					Rating );
		WRITE_MICRO_CHUNK_WWSTRING( csave, 	MICROCHUNKID_WEAPON_DEF_FIRST_PERSON_MODEL,	FirstPersonModel );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_WEAPON_DEF_FIRST_PERSON_OFFSET,	FirstPersonOffset );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_WEAPON_DEF_RECOIL_IMPULSE,		RecoilImpulse );
		WRITE_MICRO_CHUNK_WWSTRING( csave, 	MICROCHUNKID_WEAPON_DEF_HUD_ICON_TEXTURE_NAME,	HUDIconTextureName );
		WRITE_MICRO_CHUNK			  ( csave, 	MICROCHUNKID_WEAPON_DEF_RELOAD_SOUND_DEFID,	ReloadSoundDefID );
		WRITE_MICRO_CHUNK			  ( csave, 	MICROCHUNKID_WEAPON_DEF_EMPTY_SOUND_DEFID,	EmptySoundDefID);
		WRITE_MICRO_CHUNK			  ( csave, 	MICROCHUNKID_WEAPON_DEF_PRIMARY_AMMO_DEF_ID,	PrimaryAmmoDefID );
		WRITE_MICRO_CHUNK			  ( csave, 	MICROCHUNKID_WEAPON_DEF_SECONDARY_AMMO_DEF_ID,	SecondaryAmmoDefID );
		WRITE_SAFE_MICRO_CHUNK(		 csave, 	MICROCHUNKID_WEAPON_DEF_CLIP_SIZE,				ClipSize, int );
		WRITE_MICRO_CHUNK			  ( csave, 	MICROCHUNKID_WEAPON_DEF_RECOIL_TIME,			RecoilTime );
		WRITE_MICRO_CHUNK			  ( csave, 	MICROCHUNKID_WEAPON_DEF_RECOIL_SCALE,			RecoilScale );
		WRITE_MICRO_CHUNK			  ( csave, 	MICROCHUNKID_WEAPON_DEF_A_GIVE_WEAPONS_WEAPON, AGiveWeaponsWeapon );
		WRITE_SAFE_MICRO_CHUNK(		 csave, 	MICROCHUNKID_WEAPON_DEF_MAX_INVENTORY_ROUNDS,MaxInventoryRounds, int );
		WRITE_MICRO_CHUNK			  ( csave, 	MICROCHUNKID_WEAPON_ICON_NAME_ID,				IconNameID );
		WRITE_MICRO_CHUNK_WWSTRING( csave, 	MICROCHUNKID_WEAPON_ICON_TEXTURE_NAME,			IconTextureName );
		WRITE_MICRO_CHUNK			  ( csave, 	MICROCHUNKID_WEAPON_ICON_TEXTURE_UV,			IconTextureUV );
		WRITE_MICRO_CHUNK			  ( csave, 	MICROCHUNKID_WEAPON_ICON_OFFSET,				IconOffset );
		WRITE_MICRO_CHUNK_WWSTRING( csave, 	MICROCHUNKID_HUMAN_FIRING_ANIMATION,			HumanFiringAnimation );
	csave.End_Chunk();

	return true;
}

bool	WeaponDefinitionClass::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_WEAPON_DEF_PARENT:
				DefinitionClass::Load( cload );
				break;

			case CHUNKID_WEAPON_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {

						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_WEAPON_DEF_STYLE,					Style );
						READ_MICRO_CHUNK_WWSTRING( cload, 	MICROCHUNKID_WEAPON_DEF_MODEL,					Model );
						READ_MICRO_CHUNK_WWSTRING( cload, 	MICROCHUNKID_WEAPON_DEF_IDLE_ANIM,		  		IdleAnim );
						READ_MICRO_CHUNK_WWSTRING( cload, 	MICROCHUNKID_WEAPON_DEF_FIRE_ANIM,		  		FireAnim );
						READ_MICRO_CHUNK_WWSTRING( cload, 	MICROCHUNKID_WEAPON_DEF_BACK_MODEL,				BackModel );
//						READ_MICRO_CHUNK(				cload, 	LEGACY_MICROCHUNKID_WEAPON_DEF_FIRE_SOUND_DEFID,	LegacyFireSoundDefID );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_WEAPON_DEF_SWITCH_TIME,			SwitchTime );
						READ_SAFE_MICRO_CHUNK(		cload, 	MICROCHUNKID_WEAPON_DEF_RELOAD_TIME,			ReloadTime, float);
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_WEAPON_DEF_KEY_NUMBER,				KeyNumber );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_WEAPON_DEF_CAN_SNIPE,				CanSnipe );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_WEAPON_DEF_GENERIC_AMMO_OK,		CanReceiveGenericCnCAmmo );		
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_WEAPON_DEF_EJECT_PHYS_DEF_ID, 	EjectPhysDefID );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_WEAPON_DEF_MUZZLE_FLASH_PHYS_DEF_ID, 	MuzzleFlashPhysDefID );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_WEAPON_DEF_RATING,					Rating );
						READ_MICRO_CHUNK_WWSTRING( cload, 	MICROCHUNKID_WEAPON_DEF_FIRST_PERSON_MODEL,	FirstPersonModel );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_WEAPON_DEF_FIRST_PERSON_OFFSET,	FirstPersonOffset );
						READ_MICRO_CHUNK(			   cload, 	MICROCHUNKID_WEAPON_DEF_RECOIL_IMPULSE,		RecoilImpulse );
						READ_MICRO_CHUNK_WWSTRING( cload, 	MICROCHUNKID_WEAPON_DEF_HUD_ICON_TEXTURE_NAME,	HUDIconTextureName );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_WEAPON_DEF_RELOAD_SOUND_DEFID,	ReloadSoundDefID );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_WEAPON_DEF_EMPTY_SOUND_DEFID,	EmptySoundDefID);
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_WEAPON_DEF_PRIMARY_AMMO_DEF_ID,	PrimaryAmmoDefID );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_WEAPON_DEF_SECONDARY_AMMO_DEF_ID,	SecondaryAmmoDefID );
						READ_SAFE_MICRO_CHUNK(		cload, 	MICROCHUNKID_WEAPON_DEF_CLIP_SIZE,				ClipSize, int );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_WEAPON_DEF_RECOIL_TIME,			RecoilTime );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_WEAPON_DEF_RECOIL_SCALE,			RecoilScale );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_WEAPON_DEF_A_GIVE_WEAPONS_WEAPON, AGiveWeaponsWeapon );
						READ_SAFE_MICRO_CHUNK(		cload, 	MICROCHUNKID_WEAPON_DEF_MAX_INVENTORY_ROUNDS,MaxInventoryRounds, int );
						READ_MICRO_CHUNK			  ( cload, 	MICROCHUNKID_WEAPON_ICON_NAME_ID,				IconNameID );
						READ_MICRO_CHUNK_WWSTRING ( cload, 	MICROCHUNKID_WEAPON_ICON_TEXTURE_NAME,			IconTextureName );
						READ_MICRO_CHUNK			  ( cload, 	MICROCHUNKID_WEAPON_ICON_TEXTURE_UV,			IconTextureUV );
						READ_MICRO_CHUNK			  ( cload, 	MICROCHUNKID_WEAPON_ICON_OFFSET,				IconOffset );
						READ_MICRO_CHUNK_WWSTRING ( cload, 	MICROCHUNKID_HUMAN_FIRING_ANIMATION,			HumanFiringAnimation );

						default:
							Debug_Say(( "Unrecognized WeaponDef Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized WeaponDef chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

/*
** AmmoDefinitionClass
*/
SimplePersistFactoryClass<AmmoDefinitionClass, CHUNKID_AMMO_DEF>	_AmmoDefPersistFactory;

DECLARE_DEFINITION_FACTORY(AmmoDefinitionClass, CLASSID_DEF_AMMO, "Ammo") _AmmoDefDefFactory;

uint32	AmmoDefinitionClass::Get_Class_ID (void) const	{ return CLASSID_DEF_AMMO; }
const PersistFactoryClass & AmmoDefinitionClass::Get_Factory (void) const { return _AmmoDefPersistFactory; }

AmmoDefinitionClass::AmmoDefinitionClass( void ) :
	Warhead( 0 ),
	Damage( 1 ),
	Range( 10 ),
	EffectiveRange( 10 ),
	Velocity( 1 ),
	Gravity( 0 ),
	Elasticity( 1.0f ),
	RateOfFire( 1 ),
	SprayAngle( 0 ),
	SprayCount( 1 ),
	AquireTime( 0 ),
	BurstDelayTime( 0 ),
	BurstMax( 0 ),
	SoftPierceLimit( 0 ),
	TurnRate( 0 ),
	TimeActivated( false ),
	TerrainActivated( false ),
	IsTracking( false ),
//	WeaponDefID( 0 ),
	ExplosionDefID( 0 ),
	RandomTrackingScale( 0 ),
	DisplayLaser( false ),
	ChargeTime( 0 ),
	ContinuousSoundDefID( 0 ),
	FireSoundDefID( 0 ),
	MaxBounces( 0 ),
	SprayBulletCost( 1 ),
	AmmoType( AMMO_TYPE_NORMAL ),
	BeaconDefID( 0 ),
	C4TriggerTime1( 0 ),
	C4TriggerTime2( 0 ),
	C4TriggerTime3( 0 ),
	C4TriggerRange1( 0 ),
	C4TriggerRange2( 0 ),
	C4TriggerRange3( 0 ),
	C4TimingSound1ID( 0 ),
	C4TimingSound2ID( 0 ),
	C4TimingSound3ID( 0 ),
	AliasedSpeed( 0 ),
	HitterType( SurfaceEffectsManager::HITTER_TYPE_BULLET ),
	BeamEnabled(false),
	BeamColor(1,1,1),
	BeamTime(0.3f),
	BeamWidth(0.25f),
	BeamEndCaps(true),
	BeamSubdivisionEnabled(false),
	BeamSubdivisionScale(1.0f),
	BeamSubdivisionFrozen(false),
	IconNameID( 0 ),
	IconTextureUV( 0,0,0,0 ),
	IconOffset( 0,0 ),
	GrenadeSafetyTime( 0 )
{
#ifdef	PARAM_EDITING_ON
	int i;
	EnumParameterClass *param;
	param = new EnumParameterClass( &AmmoType );
	param->Set_Name ( "Ammo Type" );
	param->Add_Value ( "Normal",			AmmoDefinitionClass::AMMO_TYPE_NORMAL );
	param->Add_Value ( "Remote C4",		AmmoDefinitionClass::AMMO_TYPE_C4_REMOTE );
	param->Add_Value ( "Timed C4",		AmmoDefinitionClass::AMMO_TYPE_C4_TIMED );
	param->Add_Value ( "Proximity C4",	AmmoDefinitionClass::AMMO_TYPE_C4_PROXIMITY );
	GENERIC_EDITABLE_PARAM(AmmoDefinitionClass,param)

//	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_WEAPONOBJDEFINITIONID,	WeaponDefID);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FILENAME,	ModelFilename);

//	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_INT,			Warhead);
	param = new EnumParameterClass( &Warhead );
	param->Set_Name ( "Warhead" );
	for ( i = 0; i < ArmorWarheadManager::Get_Num_Warhead_Types(); i++ ) {
		param->Add_Value ( ArmorWarheadManager::Get_Warhead_Name( i ), i );
	}
	GENERIC_EDITABLE_PARAM(AmmoDefinitionClass,param)

	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FLOAT,		Damage);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FLOAT,		Range);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FLOAT,		EffectiveRange);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FLOAT,		Velocity);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FLOAT,		Gravity);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FLOAT,		Elasticity);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FLOAT,		RateOfFire);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_ANGLE,		SprayAngle);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_INT,			SprayCount);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_INT,			SprayBulletCost);
//	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FILENAME,	TrailEmitter);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FLOAT,		AquireTime);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FLOAT,		BurstDelayTime);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_INT,			BurstMax);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_INT,			SoftPierceLimit);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_ANGLE,		TurnRate);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_BOOL,		TimeActivated);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_BOOL,		TerrainActivated);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_BOOL,		IsTracking);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_EXPLOSIONDEFINITIONID,		ExplosionDefID);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FLOAT,		RandomTrackingScale);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_BOOL,		DisplayLaser);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FLOAT,		ChargeTime);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_SOUNDDEFINITIONID,	ContinuousSoundDefID);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_SOUNDDEFINITIONID,	FireSoundDefID);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FILENAME,	ContinuousEmitterName );
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_INT,			MaxBounces);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FLOAT,		C4TriggerTime1 );
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FLOAT,		C4TriggerTime2 );
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FLOAT,		C4TriggerTime3 );
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FLOAT,		C4TriggerRange1 );
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FLOAT,		C4TriggerRange2 );
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FLOAT,		C4TriggerRange3 );
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_SOUNDDEFINITIONID,	C4TimingSound1ID );
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_SOUNDDEFINITIONID,	C4TimingSound2ID );
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_SOUNDDEFINITIONID,	C4TimingSound3ID );
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FLOAT,		AliasedSpeed );

	param = new EnumParameterClass( &HitterType );
	param->Set_Name ( "HitterType" );
	for ( i = 0; i < SurfaceEffectsManager::Num_Hitter_Types(); i++ ) {
		param->Add_Value ( SurfaceEffectsManager::Hitter_Type_Name( i ), i );
	}
	GENERIC_EDITABLE_PARAM(AmmoDefinitionClass,param)

	GenericDefParameterClass *beacon_param = new GenericDefParameterClass (&BeaconDefID);
	beacon_param->Set_Class_ID (CLASSID_GAME_OBJECT_DEF_BEACON);
	beacon_param->Set_Name ("Beacon Object");
	GENERIC_EDITABLE_PARAM (AmmoDefinitionClass, beacon_param);

   /*
	** Beam effect parameters
	*/
	PARAM_SEPARATOR( AmmoDefinitionClass, "Instant Bullet Beam Effects");

	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_BOOL, BeamEnabled);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_VECTOR3, BeamColor);
	FLOAT_UNITS_PARAM( AmmoDefinitionClass, BeamTime, 0.01f, 10.0f, "seconds");
	FLOAT_UNITS_PARAM( AmmoDefinitionClass, BeamWidth, 0.01f, 10.0f, "meters");
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_BOOL, BeamEndCaps);
	FILENAME_PARAM( AmmoDefinitionClass, BeamTexture, "Texture filename",".tga");
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_BOOL, BeamSubdivisionEnabled);
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_BOOL, BeamSubdivisionFrozen);
	FLOAT_EDITABLE_PARAM( AmmoDefinitionClass, BeamSubdivisionScale, 0.01f,10.0f);

	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_STRINGSDB_ID,		IconNameID );
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FILENAME,		IconTextureName );
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_RECT,			IconTextureUV );
	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_VECTOR2,		IconOffset );

	EDITABLE_PARAM( AmmoDefinitionClass, ParameterClass::TYPE_FLOAT,		GrenadeSafetyTime );

#endif
}

/*
**
*/
enum	{
	CHUNKID_AMMO_DEF_VARIABLES							=	1206091429,
	CHUNKID_AMMO_DEF_PARENT,

	XXXMICROCHUNKID_AMMO_DEF_WEAPON_DEF_ID				=	1,
	MICROCHUNKID_AMMO_DEF_MODEL,
	MICROCHUNKID_AMMO_DEF_WARHEAD,
	MICROCHUNKID_AMMO_DEF_DAMAGE,
	MICROCHUNKID_AMMO_DEF_RANGE,
	MICROCHUNKID_AMMO_DEF_VELOCITY,
	MICROCHUNKID_AMMO_DEF_GRAVITY,
	XXXMICROCHUNKID_AMMO_DEF_ELASTICITY,
	XXXMICROCHUNKID_AMMO_DEF_CLIP_SIZE,
	MICROCHUNKID_AMMO_DEF_SPRAY_ANGLE,
	MICROCHUNKID_AMMO_DEF_SPRAY_COUNT,
	MICROCHUNKID_AMMO_DEF_TRAIL_EMITTER,
	MICROCHUNKID_AMMO_DEF_AQUIRE_TIME,
	MICROCHUNKID_AMMO_DEF_BURST_DELAY_TIME,
	MICROCHUNKID_AMMO_DEF_BURST_MAX,
	XXXXMICROCHUNKID_AMMO_DEF_EXPLOSION,
	MICROCHUNKID_AMMO_DEF_SOFT_PIERCE_LIMIT,
	MICROCHUNKID_AMMO_DEF_TURN_RATE,
	MICROCHUNKID_AMMO_DEF_TIME_ACTIVATED,
	MICROCHUNKID_AMMO_DEF_TERRAIN_ACTIVATED,
	MICROCHUNKID_AMMO_DEF_IS_TRACKING,
	XXXMICROCHUNKID_AMMO_DEF_ACTIVATE_SOUND_RADIUS,
	MICROCHUNKID_AMMO_DEF_EFFECTIVE_RANGE,
	MICROCHUNKID_AMMO_DEF_EXPLOSION_DEF_ID,
	MICROCHUNKID_AMMO_DEF_RANDOM_TRACKING_SCALE,
	MICROCHUNKID_AMMO_DEF_DISPLAY_LASER,
	MICROCHUNKID_AMMO_DEF_CHARGE_TIME,
	MICROCHUNKID_AMMO_DEF_CONTINUOUS_SOUND_DEF_ID,
	MICROCHUNKID_AMMO_DEF_CONTINUOUS_EMITTER_NAME,
	MICROCHUNKID_AMMO_DEF_MAX_BOUNCES,
	MICROCHUNKID_AMMO_DEF_SPRAY_BULLET_COST,
	MICROCHUNKID_AMMO_DEF_AMMO_TYPE,
	XXXMICROCHUNKID_AMMO_DEF_C4_DETONATE_SOUND_ID,
	XXXMICROCHUNKID_AMMO_DEF_C4_TIMING_SOUND_ID,
	MICROCHUNKID_AMMO_DEF_C4_TRIGGER_TIME_1,
	MICROCHUNKID_AMMO_DEF_C4_TRIGGER_TIME_2,
	MICROCHUNKID_AMMO_DEF_C4_TRIGGER_TIME_3,
	MICROCHUNKID_AMMO_DEF_C4_TRIGGER_RANGE_1,
	MICROCHUNKID_AMMO_DEF_C4_TRIGGER_RANGE_2,
	MICROCHUNKID_AMMO_DEF_C4_TRIGGER_RANGE_3,
	MICROCHUNKID_AMMO_DEF_C4_TIMING_SOUND_1_ID,
	MICROCHUNKID_AMMO_DEF_C4_TIMING_SOUND_2_ID,
	MICROCHUNKID_AMMO_DEF_C4_TIMING_SOUND_3_ID,
	MICROCHUNKID_AMMO_DEF_ELASTICITY,
	MICROCHUNKID_AMMO_DEF_ALIASED_SPEED,
	MICROCHUNKID_AMMO_DEF_HITTER_TYPE,
	MICROCHUNKID_AMMO_DEF_BEACON_DEFID,
	MICROCHUNKID_AMMO_DEF_RATE_OF_FIRE,

	MICROCHUNKID_AMMO_DEF_BEAM_ENABLED,
	MICROCHUNKID_AMMO_DEF_BEAM_COLOR,
	MICROCHUNKID_AMMO_DEF_BEAM_TIME,
	MICROCHUNKID_AMMO_DEF_BEAM_WIDTH,
	MICROCHUNKID_AMMO_DEF_BEAM_TEXTURE,
	MICROCHUNKID_AMMO_DEF_BEAM_SUBDIVISION_ENABLED,
	MICROCHUNKID_AMMO_DEF_BEAM_SUBDIVISION_SCALE,
	MICROCHUNKID_AMMO_DEF_BEAM_SUBDIVISION_FROZEN,
	MICROCHUNKID_AMMO_DEF_BEAM_END_CAPS,
	MICROCHUNKID_AMMO_ICON_NAME_ID,
	MICROCHUNKID_AMMO_ICON_TEXTURE_NAME,
	MICROCHUNKID_AMMO_ICON_TEXTURE_UV,
	MICROCHUNKID_AMMO_ICON_OFFSET,
 	MICROCHUNKID_AMMO_DEF_FIRE_SOUND_DEFID,
 	MICROCHUNKID_AMMO_GRENADE_SAFETY_TIME,
};

bool	AmmoDefinitionClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_AMMO_DEF_PARENT );
		DefinitionClass::Save( csave );
	csave.End_Chunk( );

	csave.Begin_Chunk( CHUNKID_AMMO_DEF_VARIABLES );
		WRITE_MICRO_CHUNK_WWSTRING( csave, 	MICROCHUNKID_AMMO_DEF_MODEL,						ModelFilename );
//		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_WEAPON_DEF_ID,			WeaponDefID );
		WRITE_SAFE_MICRO_CHUNK(		 csave, 	MICROCHUNKID_AMMO_DEF_WARHEAD,					Warhead, int );
		WRITE_SAFE_MICRO_CHUNK(		 csave, 	MICROCHUNKID_AMMO_DEF_DAMAGE,						Damage, float );
		WRITE_SAFE_MICRO_CHUNK(		 csave, 	MICROCHUNKID_AMMO_DEF_RANGE,						Range, float );
		WRITE_SAFE_MICRO_CHUNK(		 csave, 	MICROCHUNKID_AMMO_DEF_VELOCITY,					Velocity, float );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_GRAVITY,					Gravity );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_ELASTICITY,				Elasticity );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_RATE_OF_FIRE,	  			RateOfFire );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_SPRAY_ANGLE,				SprayAngle );
		WRITE_SAFE_MICRO_CHUNK(		 csave, 	MICROCHUNKID_AMMO_DEF_SPRAY_COUNT,				SprayCount, int );
		WRITE_MICRO_CHUNK_WWSTRING( csave, 	MICROCHUNKID_AMMO_DEF_TRAIL_EMITTER,			TrailEmitter );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_AQUIRE_TIME,				AquireTime );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_BURST_DELAY_TIME,		BurstDelayTime );
		WRITE_SAFE_MICRO_CHUNK(		 csave, 	MICROCHUNKID_AMMO_DEF_BURST_MAX,					BurstMax, int );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_SOFT_PIERCE_LIMIT,		SoftPierceLimit );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_TURN_RATE,					TurnRate );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_TIME_ACTIVATED,			TimeActivated );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_TERRAIN_ACTIVATED,		TerrainActivated );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_IS_TRACKING,				IsTracking );
		WRITE_SAFE_MICRO_CHUNK(		 csave, 	MICROCHUNKID_AMMO_DEF_EFFECTIVE_RANGE,			EffectiveRange, float );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_EXPLOSION_DEF_ID,		ExplosionDefID );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_RANDOM_TRACKING_SCALE,	RandomTrackingScale );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_DISPLAY_LASER,			DisplayLaser );
		WRITE_SAFE_MICRO_CHUNK(		 csave, 	MICROCHUNKID_AMMO_DEF_CHARGE_TIME,				ChargeTime, float );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_CONTINUOUS_SOUND_DEF_ID,	ContinuousSoundDefID );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_FIRE_SOUND_DEFID,		FireSoundDefID );
		WRITE_MICRO_CHUNK_WWSTRING( csave, 	MICROCHUNKID_AMMO_DEF_CONTINUOUS_EMITTER_NAME,	ContinuousEmitterName );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_MAX_BOUNCES,				MaxBounces );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_SPRAY_BULLET_COST,		SprayBulletCost );
		WRITE_SAFE_MICRO_CHUNK(		 csave, 	MICROCHUNKID_AMMO_DEF_AMMO_TYPE,					AmmoType, int );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_C4_TRIGGER_TIME_1,		C4TriggerTime1 );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_C4_TRIGGER_TIME_2,		C4TriggerTime2 );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_C4_TRIGGER_TIME_3,		C4TriggerTime3 );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_C4_TRIGGER_RANGE_1,		C4TriggerRange1 );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_C4_TRIGGER_RANGE_2,		C4TriggerRange2 );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_C4_TRIGGER_RANGE_3,		C4TriggerRange3 );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_C4_TIMING_SOUND_1_ID,	C4TimingSound1ID );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_C4_TIMING_SOUND_2_ID,	C4TimingSound2ID );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_C4_TIMING_SOUND_3_ID,	C4TimingSound3ID );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_ALIASED_SPEED,			AliasedSpeed );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_HITTER_TYPE,				HitterType );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_BEACON_DEFID,				BeaconDefID );

		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_BEAM_ENABLED,				BeamEnabled );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_BEAM_COLOR,				BeamColor );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_BEAM_TIME,					BeamTime );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_BEAM_WIDTH,				BeamWidth );
		WRITE_MICRO_CHUNK_WWSTRING( csave, 	MICROCHUNKID_AMMO_DEF_BEAM_TEXTURE,				BeamTexture );
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_BEAM_SUBDIVISION_ENABLED, BeamSubdivisionEnabled);
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_BEAM_SUBDIVISION_SCALE, BeamSubdivisionScale);
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_BEAM_SUBDIVISION_FROZEN, BeamSubdivisionFrozen);
		WRITE_MICRO_CHUNK(			 csave, 	MICROCHUNKID_AMMO_DEF_BEAM_END_CAPS,			BeamEndCaps);
		WRITE_MICRO_CHUNK			  ( csave, 	MICROCHUNKID_AMMO_ICON_NAME_ID,				IconNameID );
		WRITE_MICRO_CHUNK_WWSTRING( csave, 	MICROCHUNKID_AMMO_ICON_TEXTURE_NAME,			IconTextureName );
		WRITE_MICRO_CHUNK			  ( csave, 	MICROCHUNKID_AMMO_ICON_TEXTURE_UV,			IconTextureUV );
		WRITE_MICRO_CHUNK			  ( csave, 	MICROCHUNKID_AMMO_ICON_OFFSET,				IconOffset );
		WRITE_MICRO_CHUNK			  ( csave, 	MICROCHUNKID_AMMO_GRENADE_SAFETY_TIME, 		GrenadeSafetyTime );

	csave.End_Chunk();

	return true;
}

bool	AmmoDefinitionClass::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_AMMO_DEF_PARENT:
				DefinitionClass::Load( cload );
				break;

			case CHUNKID_AMMO_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {

						READ_MICRO_CHUNK_WWSTRING( cload, 	MICROCHUNKID_AMMO_DEF_MODEL,						ModelFilename );
//						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_WEAPON_DEF_ID,			WeaponDefID );
						READ_SAFE_MICRO_CHUNK(		cload, 	MICROCHUNKID_AMMO_DEF_WARHEAD,					Warhead, int );
						READ_SAFE_MICRO_CHUNK(		cload, 	MICROCHUNKID_AMMO_DEF_DAMAGE,						Damage, float );
						READ_SAFE_MICRO_CHUNK(		cload, 	MICROCHUNKID_AMMO_DEF_RANGE,						Range, float );
						READ_SAFE_MICRO_CHUNK(		cload, 	MICROCHUNKID_AMMO_DEF_VELOCITY,					Velocity, float );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_GRAVITY,					Gravity );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_ELASTICITY,				Elasticity );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_RATE_OF_FIRE,	  			RateOfFire );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_SPRAY_ANGLE,				SprayAngle );
						READ_SAFE_MICRO_CHUNK(		cload, 	MICROCHUNKID_AMMO_DEF_SPRAY_COUNT,				SprayCount, int );
						READ_MICRO_CHUNK_WWSTRING( cload, 	MICROCHUNKID_AMMO_DEF_TRAIL_EMITTER,			TrailEmitter );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_AQUIRE_TIME,				AquireTime );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_BURST_DELAY_TIME,		BurstDelayTime );
						READ_SAFE_MICRO_CHUNK(		cload, 	MICROCHUNKID_AMMO_DEF_BURST_MAX,					BurstMax, int );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_SOFT_PIERCE_LIMIT,		SoftPierceLimit );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_TURN_RATE,					TurnRate );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_TIME_ACTIVATED,			TimeActivated );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_TERRAIN_ACTIVATED,		TerrainActivated );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_IS_TRACKING,				IsTracking );
						READ_SAFE_MICRO_CHUNK(		cload, 	MICROCHUNKID_AMMO_DEF_EFFECTIVE_RANGE,			EffectiveRange, float );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_EXPLOSION_DEF_ID,		ExplosionDefID );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_RANDOM_TRACKING_SCALE,	RandomTrackingScale );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_DISPLAY_LASER,			DisplayLaser );
						READ_SAFE_MICRO_CHUNK(		cload, 	MICROCHUNKID_AMMO_DEF_CHARGE_TIME,				ChargeTime, float );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_CONTINUOUS_SOUND_DEF_ID,	ContinuousSoundDefID );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_FIRE_SOUND_DEFID,		FireSoundDefID );
						READ_MICRO_CHUNK_WWSTRING( cload, 	MICROCHUNKID_AMMO_DEF_CONTINUOUS_EMITTER_NAME,	ContinuousEmitterName );
						READ_MICRO_CHUNK(			   cload, 	MICROCHUNKID_AMMO_DEF_MAX_BOUNCES,				MaxBounces );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_SPRAY_BULLET_COST,		SprayBulletCost );
						READ_SAFE_MICRO_CHUNK(		cload, 	MICROCHUNKID_AMMO_DEF_AMMO_TYPE,					AmmoType, int );
						READ_MICRO_CHUNK(			   cload, 	MICROCHUNKID_AMMO_DEF_C4_TRIGGER_TIME_1,		C4TriggerTime1 );
						READ_MICRO_CHUNK(			   cload, 	MICROCHUNKID_AMMO_DEF_C4_TRIGGER_TIME_2,		C4TriggerTime2 );
						READ_MICRO_CHUNK(			   cload, 	MICROCHUNKID_AMMO_DEF_C4_TRIGGER_TIME_3,		C4TriggerTime3 );
						READ_MICRO_CHUNK(			   cload, 	MICROCHUNKID_AMMO_DEF_C4_TRIGGER_RANGE_1,		C4TriggerRange1 );
						READ_MICRO_CHUNK(			   cload, 	MICROCHUNKID_AMMO_DEF_C4_TRIGGER_RANGE_2,		C4TriggerRange2 );
						READ_MICRO_CHUNK(			   cload, 	MICROCHUNKID_AMMO_DEF_C4_TRIGGER_RANGE_3,		C4TriggerRange3 );
						READ_MICRO_CHUNK(			   cload, 	MICROCHUNKID_AMMO_DEF_C4_TIMING_SOUND_1_ID,	C4TimingSound1ID );
						READ_MICRO_CHUNK(			   cload, 	MICROCHUNKID_AMMO_DEF_C4_TIMING_SOUND_2_ID,	C4TimingSound2ID );
						READ_MICRO_CHUNK(			   cload, 	MICROCHUNKID_AMMO_DEF_C4_TIMING_SOUND_3_ID,	C4TimingSound3ID );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_ALIASED_SPEED,			AliasedSpeed );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_HITTER_TYPE,				HitterType );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_BEACON_DEFID,				BeaconDefID );

						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_BEAM_ENABLED,				BeamEnabled );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_BEAM_COLOR,				BeamColor );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_BEAM_TIME,					BeamTime );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_BEAM_WIDTH,				BeamWidth );
						READ_MICRO_CHUNK_WWSTRING(	cload, 	MICROCHUNKID_AMMO_DEF_BEAM_TEXTURE,				BeamTexture );
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_BEAM_SUBDIVISION_ENABLED, BeamSubdivisionEnabled);
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_BEAM_SUBDIVISION_SCALE, BeamSubdivisionScale);
						READ_MICRO_CHUNK(			   cload, 	MICROCHUNKID_AMMO_DEF_BEAM_SUBDIVISION_FROZEN, BeamSubdivisionFrozen);
						READ_MICRO_CHUNK(				cload, 	MICROCHUNKID_AMMO_DEF_BEAM_END_CAPS,			BeamEndCaps);
						READ_MICRO_CHUNK			  ( cload, 	MICROCHUNKID_AMMO_ICON_NAME_ID,				IconNameID );
						READ_MICRO_CHUNK_WWSTRING ( cload, 	MICROCHUNKID_AMMO_ICON_TEXTURE_NAME,			IconTextureName );
						READ_MICRO_CHUNK			  ( cload, 	MICROCHUNKID_AMMO_ICON_TEXTURE_UV,			IconTextureUV );
						READ_MICRO_CHUNK			  ( cload, 	MICROCHUNKID_AMMO_ICON_OFFSET,				IconOffset );
						READ_MICRO_CHUNK			  ( cload, 	MICROCHUNKID_AMMO_GRENADE_SAFETY_TIME, 		GrenadeSafetyTime );


					default:
							Debug_Say(( "Unrecognized AmmoDef Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized AmmoDef chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	// Init the model name from the model filename
	::Get_Render_Obj_Name_From_Filename( ModelName, ModelFilename );

	if ( ModelName.Is_Empty() ) {
		ModelName = "NULL";
	}

	return true;
}

/*
**
*/
bool	WeaponManager::IsWeaponHelpDisabled = false;

const WeaponDefinitionClass *WeaponManager::Find_Weapon_Definition( const char *name )
{
	return (const WeaponDefinitionClass *)DefinitionMgrClass::Find_Typed_Definition( name, CLASSID_DEF_WEAPON );
}

const WeaponDefinitionClass *WeaponManager::Find_Weapon_Definition( int id )
{
	return (const WeaponDefinitionClass *)DefinitionMgrClass::Find_Definition( id );
}

const AmmoDefinitionClass *WeaponManager::Find_Ammo_Definition( const char *name )
{
	return (const AmmoDefinitionClass *)DefinitionMgrClass::Find_Typed_Definition( name, CLASSID_DEF_AMMO );
}

const AmmoDefinitionClass *WeaponManager::Find_Ammo_Definition( int id )
{
	return (const AmmoDefinitionClass *)DefinitionMgrClass::Find_Definition( id );
}
