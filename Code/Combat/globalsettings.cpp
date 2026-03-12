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
 *                     $Archive:: /Commando/Code/Combat/globalsettings.cpp                    $*
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/16/02 5:22p                                               $*
 *                                                                                             *
 *                    $Revision:: 40                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "globalsettings.h"
#include "wwhack.h"
#include "persistfactory.h"
#include "combatchunkid.h"
#include "simpledefinitionfactory.h"
#include "debug.h"
#include "definitionmgr.h"
#include "crandom.h"
#include "damage.h"

DECLARE_FORCE_LINK( GlobalSettings )

/*
** GlobalSettingsDef
*/

SimplePersistFactoryClass<GlobalSettingsDef, CHUNKID_GLOBAL_SETTINGS_DEF_GENERAL>	_GlobalSettingsDefPersistFactory;

DECLARE_DEFINITION_FACTORY(GlobalSettingsDef, CLASSID_GLOBAL_SETTINGS_DEF_GENERAL, "General") _GlobalSettingsDefDefFactory;

/*
** Static member initialization
*/
GlobalSettingsDef *GlobalSettingsDef::GlobalSettings = NULL;


GlobalSettingsDef::GlobalSettingsDef( void ) :
	DeathSoundID( 0 ),
	EVAObjectivesSoundID( 0 ),
	HUDHelpTextSoundID( 0 ),
	MaxConversationDist( 10.0F ),
	MaxCombatConversationDist( 10.0F ),
	SoldierWalkSpeed( 0.25 ),
	SoldierCrouchSpeed( 0.25 ),
	EncyclopediaEventStringID( 0 ),
	FallingDamageMinDistance( 5 ),
	FallingDamageMaxDistance( 20 ),
	FallingDamageWarhead( 15 ),		// Default to Earth
	StealthDistanceHuman( 15.0f ),
	StealthDistanceVehicle( 25.0f ),
	MPStealthDistanceHuman( 15.0f ),
	MPStealthDistanceVehicle( 25.0f )

{
	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_SOUNDDEFINITIONID, DeathSoundID);
	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_SOUNDDEFINITIONID, EVAObjectivesSoundID);
	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_SOUNDDEFINITIONID, HUDHelpTextSoundID);
	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FLOAT, MaxConversationDist);
	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FLOAT, MaxCombatConversationDist);
	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FLOAT, SoldierWalkSpeed );
	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FLOAT, SoldierCrouchSpeed );
	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FLOAT, FallingDamageMinDistance );
	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FLOAT, FallingDamageMaxDistance );

//	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_INT, FallingDamageWarhead );
#ifdef PARAM_EDITING_ON
	int i;
	EnumParameterClass *param;
	param = new EnumParameterClass( &FallingDamageWarhead );
	param->Set_Name ( "Falling Damage Warhead" );
	for ( i = 0; i < ArmorWarheadManager::Get_Num_Warhead_Types(); i++ ) {
		param->Add_Value ( ArmorWarheadManager::Get_Warhead_Name( i ), i );
	}
	GENERIC_EDITABLE_PARAM(GlobalSettingsDef,param)
#endif //PARAM_EDITING_ON

	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FILENAME, PurchaseGDICharactersTexture );
	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FILENAME, PurchaseGDIVehiclesTexture );
	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FILENAME, PurchaseGDIEquipmentTexture );

	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FILENAME, PurchaseNODCharactersTexture );
	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FILENAME, PurchaseNODVehiclesTexture );
	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FILENAME, PurchaseNODEquipmentTexture );

	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FILENAME, PurchaseGDIMUTCharactersTexture );
	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FILENAME, PurchaseGDIMUTVehiclesTexture );
	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FILENAME, PurchaseGDIMUTEquipmentTexture );

	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FILENAME, PurchaseNODMUTCharactersTexture );
	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FILENAME, PurchaseNODMUTVehiclesTexture );
	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FILENAME, PurchaseNODMUTEquipmentTexture );

	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, EncyclopediaEventStringID );

	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FLOAT, StealthDistanceHuman );
	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FLOAT, StealthDistanceVehicle );
	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FLOAT, MPStealthDistanceHuman );
	EDITABLE_PARAM(GlobalSettingsDef, ParameterClass::TYPE_FLOAT, MPStealthDistanceVehicle );

	GlobalSettings = this;
}


GlobalSettingsDef::~GlobalSettingsDef( void )
{
	GlobalSettings = NULL;
	return ;
}


uint32	GlobalSettingsDef::Get_Class_ID (void) const
{
	return CLASSID_GLOBAL_SETTINGS_DEF_GENERAL;
}

const PersistFactoryClass & GlobalSettingsDef::Get_Factory (void) const
{
	return _GlobalSettingsDefPersistFactory;
}

PersistClass *	GlobalSettingsDef::Create( void ) const
{
	WWASSERT( 0 );
	return NULL;
}

enum	{
	CHUNKID_DEF_PARENT							=	803001812,
	CHUNKID_DEF_VARIABLES,

	MICROCHUNKID_DEF_XXX							=	1,
	MICROCHUNKID_DEF_DEATH_SOUND,
	MICROCHUNKID_DEF_EVA_MO_SOUND,
	MICROCHUNKID_DEF_MAX_CONV_DIST,
	MICROCHUNKID_DEF_MAX_COMBAT_CONV_DIST,
	MICROCHUNKID_DEF_SOLDIER_WALK_SPEED,
	MICROCHUNKID_DEF_SOLDIER_CROUCH_SPEED,
	MICROCHUNKID_DEF_FALLING_DAMAGE_MIN_DISTANCE,
	MICROCHUNKID_DEF_FALLING_DAMAGE_MAX_DISTANCE,
	MICROCHUNKID_DEF_FALLING_DAMAGE_WARHEAD,

	MICROCHUNKID_DEF_PURCHASE_GDI_CHARS_TEXTURE,
	MICROCHUNKID_DEF_PURCHASE_GDI_VEHICLES_TEXTURE,
	MICROCHUNKID_DEF_PURCHASE_GDI_EQUIP_TEXTURE,
	XXXMICROCHUNKID_DEF_PURCHASE_ADV_EQUIP_TEXTURE,

	MICROCHUNKID_DEF_PURCHASE_NOD_CHARS_TEXTURE,
	MICROCHUNKID_DEF_PURCHASE_NOD_VEHICLES_TEXTURE,
	MICROCHUNKID_DEF_PURCHASE_NOD_EQUIP_TEXTURE,

	MICROCHUNKID_DEF_PURCHASE_GDI_MUT_CHARS_TEXTURE,
	MICROCHUNKID_DEF_PURCHASE_GDI_MUT_VEHICLES_TEXTURE,
	MICROCHUNKID_DEF_PURCHASE_GDI_MUT_EQUIP_TEXTURE,

	MICROCHUNKID_DEF_PURCHASE_NOD_MUT_CHARS_TEXTURE,
	MICROCHUNKID_DEF_PURCHASE_NOD_MUT_VEHICLES_TEXTURE,
	MICROCHUNKID_DEF_PURCHASE_NOD_MUT_EQUIP_TEXTURE,

	MICROCHUNKID_DEF_ENCY_EVENT_STRING_ID,
	MICROCHUNKID_DEF_HELP_TXT_SOUND,

	MICROCHUNKID_DEF_STEALTH_DISTANCE_HUMAN,
	MICROCHUNKID_DEF_STEALTH_DISTANCE_VEHICLE,
	MICROCHUNKID_DEF_MP_STEALTH_DISTANCE_HUMAN,
	MICROCHUNKID_DEF_MP_STEALTH_DISTANCE_VEHICLE,
};

bool	GlobalSettingsDef::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_DEF_PARENT );
		DefinitionClass::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_DEF_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_DEATH_SOUND, DeathSoundID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_EVA_MO_SOUND, EVAObjectivesSoundID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_HELP_TXT_SOUND, HUDHelpTextSoundID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_MAX_CONV_DIST, MaxConversationDist );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_MAX_COMBAT_CONV_DIST, MaxCombatConversationDist );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_SOLDIER_WALK_SPEED, SoldierWalkSpeed );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_SOLDIER_CROUCH_SPEED, SoldierCrouchSpeed );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_FALLING_DAMAGE_MIN_DISTANCE, FallingDamageMinDistance );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_FALLING_DAMAGE_MAX_DISTANCE, FallingDamageMaxDistance );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_FALLING_DAMAGE_WARHEAD, FallingDamageWarhead );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_ENCY_EVENT_STRING_ID, EncyclopediaEventStringID );

		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_PURCHASE_GDI_CHARS_TEXTURE,		PurchaseGDICharactersTexture );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_PURCHASE_GDI_VEHICLES_TEXTURE,	PurchaseGDIVehiclesTexture );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_PURCHASE_GDI_EQUIP_TEXTURE,		PurchaseGDIEquipmentTexture );

		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_PURCHASE_NOD_CHARS_TEXTURE,		PurchaseNODCharactersTexture );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_PURCHASE_NOD_VEHICLES_TEXTURE,	PurchaseNODVehiclesTexture );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_PURCHASE_NOD_EQUIP_TEXTURE,		PurchaseNODEquipmentTexture );

		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_PURCHASE_GDI_MUT_CHARS_TEXTURE,		PurchaseGDIMUTCharactersTexture );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_PURCHASE_GDI_MUT_VEHICLES_TEXTURE,	PurchaseGDIMUTVehiclesTexture );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_PURCHASE_GDI_MUT_EQUIP_TEXTURE,		PurchaseGDIMUTEquipmentTexture );

		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_PURCHASE_NOD_MUT_CHARS_TEXTURE,		PurchaseNODMUTCharactersTexture );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_PURCHASE_NOD_MUT_VEHICLES_TEXTURE,	PurchaseNODMUTVehiclesTexture );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_PURCHASE_NOD_MUT_EQUIP_TEXTURE,		PurchaseNODMUTEquipmentTexture );

		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_STEALTH_DISTANCE_HUMAN, StealthDistanceHuman );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_STEALTH_DISTANCE_VEHICLE, StealthDistanceVehicle );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_MP_STEALTH_DISTANCE_HUMAN, MPStealthDistanceHuman );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_MP_STEALTH_DISTANCE_VEHICLE, MPStealthDistanceVehicle );
	csave.End_Chunk();

	return true;
}

bool	GlobalSettingsDef::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_DEF_PARENT:
				DefinitionClass::Load( cload );
				break;

			case CHUNKID_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_DEATH_SOUND, DeathSoundID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_EVA_MO_SOUND, EVAObjectivesSoundID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_HELP_TXT_SOUND, HUDHelpTextSoundID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_MAX_CONV_DIST, MaxConversationDist );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_MAX_COMBAT_CONV_DIST, MaxCombatConversationDist );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_SOLDIER_WALK_SPEED, SoldierWalkSpeed );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_SOLDIER_CROUCH_SPEED, SoldierCrouchSpeed );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_FALLING_DAMAGE_MIN_DISTANCE, FallingDamageMinDistance );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_FALLING_DAMAGE_MAX_DISTANCE, FallingDamageMaxDistance );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_FALLING_DAMAGE_WARHEAD, FallingDamageWarhead );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_ENCY_EVENT_STRING_ID, EncyclopediaEventStringID );

						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_DEF_PURCHASE_GDI_CHARS_TEXTURE,		PurchaseGDICharactersTexture );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_DEF_PURCHASE_GDI_VEHICLES_TEXTURE,	PurchaseGDIVehiclesTexture );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_DEF_PURCHASE_GDI_EQUIP_TEXTURE,		PurchaseGDIEquipmentTexture );

						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_DEF_PURCHASE_NOD_CHARS_TEXTURE,		PurchaseNODCharactersTexture );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_DEF_PURCHASE_NOD_VEHICLES_TEXTURE,	PurchaseNODVehiclesTexture );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_DEF_PURCHASE_NOD_EQUIP_TEXTURE,		PurchaseNODEquipmentTexture );

						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_DEF_PURCHASE_GDI_MUT_CHARS_TEXTURE,		PurchaseGDIMUTCharactersTexture );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_DEF_PURCHASE_GDI_MUT_VEHICLES_TEXTURE,	PurchaseGDIMUTVehiclesTexture );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_DEF_PURCHASE_GDI_MUT_EQUIP_TEXTURE,		PurchaseGDIMUTEquipmentTexture );

						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_DEF_PURCHASE_NOD_MUT_CHARS_TEXTURE,		PurchaseNODMUTCharactersTexture );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_DEF_PURCHASE_NOD_MUT_VEHICLES_TEXTURE,	PurchaseNODMUTVehiclesTexture );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_DEF_PURCHASE_NOD_MUT_EQUIP_TEXTURE,		PurchaseNODMUTEquipmentTexture );

						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_STEALTH_DISTANCE_HUMAN, StealthDistanceHuman );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_STEALTH_DISTANCE_VEHICLE, StealthDistanceVehicle );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_MP_STEALTH_DISTANCE_HUMAN, MPStealthDistanceHuman );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_MP_STEALTH_DISTANCE_VEHICLE, MPStealthDistanceVehicle );

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


/*
** HumanLoiterGlobalSettingsDef
*/

SimplePersistFactoryClass<HumanLoiterGlobalSettingsDef, CHUNKID_GLOBAL_SETTINGS_DEF_HUMAN_LOITER>	_HumanLoiterGlobalSettingsDefPersistFactory;

DECLARE_DEFINITION_FACTORY(HumanLoiterGlobalSettingsDef, CLASSID_GLOBAL_SETTINGS_DEF_HUMAN_LOITER, "HumanLoiter") _HumanLoiterGlobalSettingsDefDefFactory;

HumanLoiterGlobalSettingsDef * HumanLoiterGlobalSettingsDef::DefaultLoiters		= NULL;
HumanLoiterGlobalSettingsDef * HumanLoiterGlobalSettingsDef::WeaponLoiters			= NULL;
HumanLoiterGlobalSettingsDef * HumanLoiterGlobalSettingsDef::WeaponlessLoiters	= NULL;

HumanLoiterGlobalSettingsDef::HumanLoiterGlobalSettingsDef( void ) :
	ActivationDelay( 20 ),
	LoiterFrequency( 10 )
{
	EDITABLE_PARAM( HumanLoiterGlobalSettingsDef, ParameterClass::TYPE_FLOAT, ActivationDelay );
	EDITABLE_PARAM( HumanLoiterGlobalSettingsDef, ParameterClass::TYPE_FLOAT, LoiterFrequency );
	EDITABLE_PARAM( HumanLoiterGlobalSettingsDef, ParameterClass::TYPE_FILENAMELIST, LoiterAnimList);
}

HumanLoiterGlobalSettingsDef::~HumanLoiterGlobalSettingsDef( void )
{
	if ( this == DefaultLoiters ) {
		DefaultLoiters = NULL;
	}
	if ( this == WeaponLoiters ) {
		WeaponLoiters = NULL;
	}
	if ( this == WeaponlessLoiters ) {
		WeaponlessLoiters = NULL;
	}
}

uint32	HumanLoiterGlobalSettingsDef::Get_Class_ID (void) const
{
	return CLASSID_GLOBAL_SETTINGS_DEF_HUMAN_LOITER;
}

const PersistFactoryClass & HumanLoiterGlobalSettingsDef::Get_Factory (void) const
{
	return _HumanLoiterGlobalSettingsDefPersistFactory;
}

PersistClass *	HumanLoiterGlobalSettingsDef::Create( void ) const
{
	WWASSERT( 0 );
	return NULL;
}

enum	{
	CHUNKID_HL_DEF_PARENT							=	803001812,
	CHUNKID_HL_DEF_VARIABLES,

	MICROCHUNKID_HL_DEF_ACTIVATION_DELAY		=	1,
	MICROCHUNKID_HL_DEF_LOITER_FREQUENCY,
	MICROCHUNKID_HL_DEF_LOITER_ANIM_LIST_ENTRY,
};

bool	HumanLoiterGlobalSettingsDef::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_HL_DEF_PARENT );
		DefinitionClass::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_HL_DEF_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_HL_DEF_ACTIVATION_DELAY, ActivationDelay );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_HL_DEF_LOITER_FREQUENCY, LoiterFrequency );

		for ( int i = 0; i < LoiterAnimList.Count(); i++ ) {
			WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_HL_DEF_LOITER_ANIM_LIST_ENTRY, LoiterAnimList[i] );
		}
	csave.End_Chunk();

	return true;
}

bool	HumanLoiterGlobalSettingsDef::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_HL_DEF_PARENT:
				DefinitionClass::Load( cload );
				break;

			case CHUNKID_HL_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_HL_DEF_ACTIVATION_DELAY, ActivationDelay );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_HL_DEF_LOITER_FREQUENCY, LoiterFrequency );

						case MICROCHUNKID_HL_DEF_LOITER_ANIM_LIST_ENTRY:
						{
							StringClass str;
							LOAD_MICRO_CHUNK_WWSTRING(cload,str);
							LoiterAnimList.Add( str );
							break;
						}

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

HumanLoiterGlobalSettingsDef * HumanLoiterGlobalSettingsDef::Get_Default_Loiters( void )
{
	if ( DefaultLoiters == NULL ) {
		DefaultLoiters = (HumanLoiterGlobalSettingsDef *)DefinitionMgrClass::Find_Typed_Definition( "Loiter", CLASSID_GLOBAL_SETTINGS_DEF_HUMAN_LOITER );
	}
	if ( DefaultLoiters == NULL ) {
		Debug_Say(( "Failed to load Default Loiter\n" ));
	}
	return DefaultLoiters;
}

HumanLoiterGlobalSettingsDef * HumanLoiterGlobalSettingsDef::Get_Weapon_Loiters( void )
{
	if ( WeaponLoiters == NULL ) {
		WeaponLoiters = (HumanLoiterGlobalSettingsDef *)DefinitionMgrClass::Find_Typed_Definition( "Weapon Loiters", CLASSID_GLOBAL_SETTINGS_DEF_HUMAN_LOITER );
	}
	if ( WeaponLoiters == NULL ) {
		Debug_Say(( "Failed to Weapons Loiter\n" ));
	}
	return WeaponLoiters;
}

HumanLoiterGlobalSettingsDef * HumanLoiterGlobalSettingsDef::Get_Weaponless_Loiters( void )
{
	if ( WeaponlessLoiters == NULL ) {
		WeaponlessLoiters = (HumanLoiterGlobalSettingsDef *)DefinitionMgrClass::Find_Typed_Definition( "Weaponless Loiters", CLASSID_GLOBAL_SETTINGS_DEF_HUMAN_LOITER );
	}
	if ( WeaponlessLoiters == NULL ) {
		Debug_Say(( "Failed to load weaponless Loiter\n" ));
	}
	return WeaponlessLoiters;
}

const char * HumanLoiterGlobalSettingsDef::Pick_Animation( void )
{
	if ( LoiterAnimList.Count() == 0 ) {
		return "";
	}
	return LoiterAnimList[ FreeRandom.Get_Int( LoiterAnimList.Count() ) ];
}


/*
** HUDGlobalSettingsDef
*/
HUDGlobalSettingsDef *	HUDGlobalSettingsDef::Instance	= NULL;

SimplePersistFactoryClass<HUDGlobalSettingsDef, CHUNKID_GLOBAL_SETTINGS_DEF_HUD>	_HUDGlobalSettingsDefPersistFactory;

DECLARE_DEFINITION_FACTORY(HUDGlobalSettingsDef, CLASSID_GLOBAL_SETTINGS_DEF_HUD, "HUD") _HUDGlobalSettingsDefDefFactory;

// The old define settings are now the defaults for the global setting
#define	INFO_TEXTURE_SIZE				128
#define	STAR_BRACKET_SIZE				Vector2( 72, 56 )
#define	STAR_BRACKET_OFFSET			Vector2( -70, -58 )
#define	STAR_BRACKET_UV				RectClass( 55.0f, 1.0f, 127.0f, 57.0f )
#define	STAR_BRACKET_TOP_SIZE		Vector2( 38, 16 )
#define	STAR_BRACKET_TOP_OFFSET		((STAR_BRACKET_OFFSET) + Vector2( 25, -16 ))
#define	STAR_BRACKET_TOP_UV			RectClass( 2.0f, 81.0f, 40.0f, 97.0f )
#define	STAR_BRACKET_TOP_ARMED_SIZE		Vector2( 38, 16 )
#define	STAR_BRACKET_TOP_ARMED_OFFSET		((STAR_BRACKET_OFFSET) + Vector2( 25, -16 ))
#define	STAR_BRACKET_TOP_ARMED_UV			RectClass( 40.0f, 81.0f, 2.0f, 97.0f )
#define	STAR_BAR_LENGTH				130
#define	STAR_BAR_SIZE					Vector2( STAR_BAR_LENGTH, 18 )
#define	STAR_BAR_OFFSET				((STAR_BRACKET_OFFSET) + Vector2( -STAR_BAR_LENGTH + 2, 31 ))
#define	STAR_BAR_UV						RectClass( 55.0f, 60.0f, 79.0f, 78.0f )
#define	STAR_BAR_END_SIZE				Vector2( 32, 24 )
#define	STAR_BAR_END_OFFSET			((STAR_BAR_OFFSET) + Vector2( -26, -2 ))
#define	STAR_BAR_END_UV				RectClass( 79.0f, 57.0f, 111.0f, 81.0f )
#define	STAR_HEALTH_SIZE				Vector2( STAR_BAR_LENGTH, 6 )
#define	STAR_HEALTH_OFFSET			(STAR_BAR_OFFSET + Vector2( 0, 3 ))
#define	STAR_HEALTH_UV					RectClass( 112.0f, 66.0f, 126.0f, 72.0f )
#define	STAR_SHIELD_SIZE				Vector2( STAR_BAR_LENGTH + 4, 6 )
#define	STAR_SHIELD_OFFSET			(STAR_BAR_OFFSET + Vector2( -4, 10 ))
#define	STAR_SHIELD_UV					RectClass( 112.0f, 66.0f, 126.0f, 72.0f )
#define	STAR_WEAPON_ICON_SIZE		Vector2( 64, 64 )
#define	STAR_WEAPON_ICON_OFFSET		((STAR_BRACKET_OFFSET) + Vector2( 0, -70 ))
#define	TARGET_BRACKET_SIZE			Vector2( 21, 52 )
#define	TARGET_BRACKET_OFFSET		Vector2( 68, -60 )
#define	TARGET_BRACKET_UV				RectClass( 2.0f, 1.0f, 23.0f, 53.0f )
#define	TARGET_ICON_SIZE				Vector2( 64, 64 )
#define	TARGET_ICON_OFFSET			Vector2( 2, -68 )
#define	TARGET_NAME_BAR_SIZE			Vector2( 120, 20 )
#define	TARGET_NAME_BAR_OFFSET		(TARGET_BRACKET_OFFSET + Vector2( 19, 2 ))
#define	TARGET_NAME_BAR_UV			RectClass( 1.0f, 59.0f, 31.0f, 79.0f )
#define	TARGET_NAME_OFFSET			(TARGET_BRACKET_OFFSET + Vector2( 24, 8 ))
#define	TARGET_BAR_LENGTH				130
#define	TARGET_BAR_SIZE				Vector2( TARGET_BAR_LENGTH, 18 )
#define	TARGET_BAR_OFFSET				(TARGET_BRACKET_OFFSET + Vector2( 19 , 33 ))
#define	TARGET_BAR_UV					RectClass( 55.0f, 60.0f, 79.0f, 78.0f )
#define	TARGET_BAR_END_SIZE			Vector2( 32, 24 )
#define	TARGET_BAR_END_OFFSET		(TARGET_BAR_OFFSET + Vector2( TARGET_BAR_LENGTH - 6, -2 ))
#define	TARGET_BAR_END_UV				RectClass( 111.0f, 57.0f, 79.0f, 81.0f )
#define	TARGET_HEALTH_SIZE			Vector2( TARGET_BAR_LENGTH, 6 )
#define	TARGET_HEALTH_OFFSET			(TARGET_BAR_OFFSET + Vector2( 0, 3 ))
#define	TARGET_HEALTH_UV				RectClass( 126.0f, 66.0f, 112.0f, 72.0f )
#define	TARGET_SHIELD_SIZE			Vector2( TARGET_BAR_LENGTH + 4, 6 )
#define	TARGET_SHIELD_OFFSET			(TARGET_BAR_OFFSET + Vector2( 0, 10 ))
#define	TARGET_SHIELD_UV				RectClass( 126.0f, 66.0f, 112.0f, 72.0f )
#define	RADAR_TEXTURE_SIZE			128
#define	RADAR_OFFSET					Vector2( 82, -124 )
#define	RADAR_RADIUS					64
#define	RADAR_FRAME_SIZE				Vector2( 112, 128 )
#define	RADAR_FRAME_UV					RectClass( 0, 0, 112.0f, 128.0f )
#define	RADAR_COMPASS_OFFSET			Vector2( -7, 54 )
#define	RADAR_COMPASS_SIZE			Vector2( 16, 8 )
#define	RADAR_COMPASS_BASE_UV		RectClass( 112.0f, 64.0f, 128.0f, 72.0f )
#define	RADAR_COMPASS_UV_OFFSET		Vector2( 0, 8 )
#define	RADAR_HUMAN_BLIP_UV			RectClass(	112.0f, 0.0f, 120.0f,	8.0f )
#define	RADAR_VEHICLE_BLIP_UV 		RectClass(	120.0f, 0.0f, 128.0f,	8.0f )
#define	RADAR_STATIONARY_BLIP_UV 	RectClass(	112.0f, 8.0f, 120.0f,  16.0f )
#define	RADAR_OBJECTIVE_BLIP_UV 	RectClass(	120.0f,	8.0f, 128.0f,  16.0f )
#define	RADAR_BLIP_BRACKET_UV 		RectClass(	112.0f,16.0f, 120.0f,  24.0f )
#define	RADAR_SWEEP_UV					RectClass(	121.0f,24.0f, 127.0f,  32.0f )
#define	SNIPER_TEXTURE_SIZE			256
#define	SNIPER_VIEW						RectClass( 0.2f, 0.12f, 0.8f, 0.88f )
#define	SNIPER_VIEW_UV					RectClass( 0.0f, 0.0f, 240.0f, 227.0f )
#define	SNIPER_SCAN_LINE_UV			RectClass( 0.01f, 0.01f, 0.05f, 0.05f )
#define	SNIPER_BLACK_COVER_UV		RectClass( 0.01f, 0.01f, 0.05f, 0.05f )
#define	SNIPER_TILT_BAR				RectClass( 0.20f, 0.25f, 0.225f, 0.75f )
#define	SNIPER_TILT_BAR_RATE			1
#define	SNIPER_TILT_BAR_UV			RectClass( 245.0f, 3.0f, 250.0f, 208.0f )
#define	SNIPER_TURN_BAR				RectClass( 0.35f, 0.25f, 0.65f, 0.275f )
#define	SNIPER_TURN_BAR_RATE			1
#define	SNIPER_TURN_BAR_UV			RectClass( 1.0f, 244.0f, 109.0f, 253.0f )
#define	SNIPER_DISTANCE_GRAPH		RectClass( 0.175f, 0.3f, 0.2f, 0.65f )
#define	SNIPER_DISTANCE_GRAPH_UV	RectClass( 1.0f, 231.0f, 87.0f, 239.0f )
#define	SNIPER_DISTANCE_GRAPH_MAX	200
#define	SNIPER_ZOOM_GRAPH				RectClass( 0.72f, 0.18f, 0.8f, 0.22f )
#define	SNIPER_ZOOM_GRAPH_UV			RectClass( 218.0f, 247.0f, 254.0f, 254.0f )
#define	DAMAGE_INDICATOR_UV			RectClass( 31.0f, 1.0f, 51.0f, 59.0f )
#define	DAMAGE_DIAG_INDICATOR_UV	RectClass( 1.0f, 60.0f, 47.0f, 106.0f )

HUDGlobalSettingsDef::HUDGlobalSettingsDef( void ) :
	NodColor( 1,0,0 ),
	GDIColor( 1,1,0 ),
	NeutralColor( 1,1,1 ),
	MutantColor( 0,1,0 ),
	RenegadeColor( 0,0,1 ),
	PrimaryObjectiveColor( 0,1,0 ),
	SecondaryObjectiveColor( 0,0,1 ),
	TertiaryObjectiveColor( 1,0,1 ),
	HealthHighColor( 0,1,0 ),
	HealthMedColor( 1,1,0 ),
	HealthLowColor( 1,0,0 ),
	EnemyColor( 1,0,0 ),
	FriendlyColor( 0,1,0 ),
	NoRelationColor( 1,1,1 ),
	InfoTextureSize(INFO_TEXTURE_SIZE),
	StarBracketSize(STAR_BRACKET_SIZE),
	StarBracketOffset(STAR_BRACKET_OFFSET),
	StarBracketUV(STAR_BRACKET_UV),
	StarBracketTopSize(STAR_BRACKET_TOP_SIZE),
	StarBracketTopOffset(STAR_BRACKET_TOP_OFFSET),
	StarBracketTopUV(STAR_BRACKET_TOP_UV),
	StarBracketTopArmedSize(STAR_BRACKET_TOP_ARMED_SIZE),
	StarBracketTopArmedOffset(STAR_BRACKET_TOP_ARMED_OFFSET),
	StarBracketTopArmedUV(STAR_BRACKET_TOP_ARMED_UV),
	StarBarSize(STAR_BAR_SIZE),
	StarBarOffset(STAR_BAR_OFFSET),
	StarBarUV(STAR_BAR_UV),
	StarBarEndSize(STAR_BAR_END_SIZE),
	StarBarEndOffset(STAR_BAR_END_OFFSET),
	StarBarEndUV(STAR_BAR_END_UV),
	StarHealthSize(STAR_HEALTH_SIZE),
	StarHealthOffset(STAR_HEALTH_OFFSET),
	StarHealthUV(STAR_HEALTH_UV),
	StarShieldSize(STAR_SHIELD_SIZE),
	StarShieldOffset(STAR_SHIELD_OFFSET),
	StarShieldUV(STAR_SHIELD_UV),
	StarWeaponIconSize(STAR_WEAPON_ICON_SIZE),
	StarWeaponIconOffset(STAR_WEAPON_ICON_OFFSET),
	TargetBracketSize(TARGET_BRACKET_SIZE),
	TargetBracketOffset(TARGET_BRACKET_OFFSET),
	TargetBracketUV(TARGET_BRACKET_UV),
	TargetIconSize(TARGET_ICON_SIZE),
	TargetIconOffset(TARGET_ICON_OFFSET),
	TargetNameBarSize(TARGET_NAME_BAR_SIZE),
	TargetNameBarOffset(TARGET_NAME_BAR_OFFSET),
	TargetNameBarUV(TARGET_NAME_BAR_UV),
	TargetNameOffset(TARGET_NAME_OFFSET),
	TargetBarSize(TARGET_BAR_SIZE),
	TargetBarOffset(TARGET_BAR_OFFSET),
	TargetBarUV(TARGET_BAR_UV),
	TargetBarEndSize(TARGET_BAR_END_SIZE),
	TargetBarEndOffset(TARGET_BAR_END_OFFSET),
	TargetBarEndUV(TARGET_BAR_END_UV),
	TargetHealthSize(TARGET_HEALTH_SIZE),
	TargetHealthOffset(TARGET_HEALTH_OFFSET),
	TargetHealthUV(TARGET_HEALTH_UV),
	TargetShieldSize(TARGET_SHIELD_SIZE),
	TargetShieldOffset(TARGET_SHIELD_OFFSET),
	TargetShieldUV(TARGET_SHIELD_UV),
	RadarTextureSize(RADAR_TEXTURE_SIZE),
	RadarOffset(RADAR_OFFSET),
	RadarRadius(RADAR_RADIUS),
	RadarFrameSize(RADAR_FRAME_SIZE),
	RadarFrameUV(RADAR_FRAME_UV),
	RadarCompassOffset(RADAR_COMPASS_OFFSET),
	RadarCompassSize(RADAR_COMPASS_SIZE),
	RadarCompassBaseUV(RADAR_COMPASS_BASE_UV),
	RadarCompassUVOffset(RADAR_COMPASS_UV_OFFSET),
	RadarHumanBlipUV(RADAR_HUMAN_BLIP_UV),
	RadarVehicleBlipUV(RADAR_VEHICLE_BLIP_UV),
	RadarStationaryBlipUV(RADAR_STATIONARY_BLIP_UV),
	RadarObjectiveBlipUV(RADAR_OBJECTIVE_BLIP_UV),
	RadarBlipBracketUV(RADAR_BLIP_BRACKET_UV),
	RadarSweepUV(RADAR_SWEEP_UV),
	RadarOnSoundID(0),
	RadarOffSoundID(0),
	SniperTextureSize(SNIPER_TEXTURE_SIZE),
	SniperView(SNIPER_VIEW),
	SniperViewUV(SNIPER_VIEW_UV),
	SniperScanLineUV(SNIPER_SCAN_LINE_UV),
	SniperBlackCoverUV(SNIPER_BLACK_COVER_UV),
	SniperTiltBar(SNIPER_TILT_BAR),
	SniperTiltBarRate(SNIPER_TILT_BAR_RATE),
	SniperTiltBarUV(SNIPER_TILT_BAR_UV),
	SniperTurnBar(SNIPER_TURN_BAR),
	SniperTurnBarRate(SNIPER_TURN_BAR_RATE),
	SniperTurnBarUV(SNIPER_TURN_BAR_UV),
	SniperDistanceGraph(SNIPER_DISTANCE_GRAPH),
	SniperDistanceGraphUV(SNIPER_DISTANCE_GRAPH_UV),
	SniperDistanceGraphMax(SNIPER_DISTANCE_GRAPH_MAX),
	SniperZoomGraph(SNIPER_ZOOM_GRAPH),
	SniperZoomGraphUV(SNIPER_ZOOM_GRAPH_UV),
	DamageIndicatorUV(DAMAGE_INDICATOR_UV),
	DamageDiagIndicatorUV(DAMAGE_DIAG_INDICATOR_UV)
{
	WWASSERT( Instance == NULL );	 // Pat, you can remove this if needed
	Instance = this;

	PARAM_SEPARATOR( HUDGlobalSettingsDef, "Colors" );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_COLOR, NodColor );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_COLOR, GDIColor );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_COLOR, NeutralColor );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_COLOR, MutantColor );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_COLOR, RenegadeColor );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_COLOR, PrimaryObjectiveColor );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_COLOR, SecondaryObjectiveColor );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_COLOR, TertiaryObjectiveColor );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_COLOR, HealthHighColor );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_COLOR, HealthMedColor );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_COLOR, HealthLowColor );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_COLOR, EnemyColor );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_COLOR, FriendlyColor );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_COLOR, NoRelationColor );

	PARAM_SEPARATOR( HUDGlobalSettingsDef, "Star Info" );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_FLOAT,		InfoTextureSize	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, StarBracketSize	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, StarBracketOffset	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  StarBracketUV	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, StarBracketTopSize	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, StarBracketTopOffset	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  StarBracketTopUV	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, StarBracketTopArmedSize	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, StarBracketTopArmedOffset	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  StarBracketTopArmedUV	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, StarBarSize	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, StarBarOffset	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  StarBarUV	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, StarBarEndSize	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, StarBarEndOffset	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  StarBarEndUV	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, StarHealthSize	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, StarHealthOffset	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  StarHealthUV	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, StarShieldSize	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, StarShieldOffset	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  StarShieldUV	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, StarWeaponIconSize );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, StarWeaponIconOffset );

	PARAM_SEPARATOR( HUDGlobalSettingsDef, "Target Info" );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, TargetBracketSize	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, TargetBracketOffset	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  TargetBracketUV	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, TargetIconSize	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, TargetIconOffset	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, TargetNameBarSize	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, TargetNameBarOffset	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  TargetNameBarUV	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, TargetNameOffset	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, TargetBarSize	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, TargetBarOffset	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  TargetBarUV	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, TargetBarEndSize	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, TargetBarEndOffset	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  TargetBarEndUV	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, TargetHealthSize	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, TargetHealthOffset	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  TargetHealthUV	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, TargetShieldSize	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, TargetShieldOffset	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  TargetShieldUV	);

	PARAM_SEPARATOR( HUDGlobalSettingsDef, "Damage Indicators" );
   EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  DamageIndicatorUV );
   EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  DamageDiagIndicatorUV );

	PARAM_SEPARATOR( HUDGlobalSettingsDef, "Radar" );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_FLOAT,		RadarTextureSize	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, RadarOffset	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_FLOAT,	  RadarRadius	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, RadarFrameSize	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  RadarFrameUV	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, RadarCompassOffset	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, RadarCompassSize	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  RadarCompassBaseUV	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_VECTOR2, RadarCompassUVOffset	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  RadarHumanBlipUV	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  RadarVehicleBlipUV	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  RadarStationaryBlipUV	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  RadarObjectiveBlipUV	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  RadarBlipBracketUV	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  RadarSweepUV	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_SOUNDDEFINITIONID,  RadarOnSoundID );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_SOUNDDEFINITIONID,  RadarOffSoundID );

	PARAM_SEPARATOR( HUDGlobalSettingsDef, "Sniper" );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_FLOAT,		SniperTextureSize	);
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  SniperView );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  SniperViewUV );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  SniperScanLineUV );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  SniperBlackCoverUV );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  SniperTiltBar );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_FLOAT,	  SniperTiltBarRate );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  SniperTiltBarUV );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  SniperTurnBar );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_FLOAT,	  SniperTurnBarRate );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  SniperTurnBarUV );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  SniperDistanceGraph );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  SniperDistanceGraphUV );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_FLOAT,	  SniperDistanceGraphMax );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  SniperZoomGraph );
	EDITABLE_PARAM( HUDGlobalSettingsDef, ParameterClass::TYPE_RECT,	  SniperZoomGraphUV );

}

HUDGlobalSettingsDef::~HUDGlobalSettingsDef( void )
{
	WWASSERT( Instance == this );
	Instance = NULL;
}

uint32	HUDGlobalSettingsDef::Get_Class_ID (void) const
{
	return CLASSID_GLOBAL_SETTINGS_DEF_HUD;
}

const PersistFactoryClass & HUDGlobalSettingsDef::Get_Factory (void) const
{
	return _HUDGlobalSettingsDefPersistFactory;
}

PersistClass *	HUDGlobalSettingsDef::Create( void ) const
{
	WWASSERT( 0 );
	return NULL;
}

enum	{
	CHUNKID_HUD_DEF_PARENT							=	803001812,
	CHUNKID_HUD_DEF_VARIABLES,

	MICROCHUNKID_HUD_DEF_NOD_COLOR				=	1,
	MICROCHUNKID_HUD_DEF_GDI_COLOR,
	MICROCHUNKID_HUD_DEF_NEUTRAL_COLOR,
	MICROCHUNKID_HUD_DEF_PRIMARY_OBJECTIVE_COLOR,
	MICROCHUNKID_HUD_DEF_SECONDARY_OBJECTIVE_COLOR,
	MICROCHUNKID_HUD_DEF_TERTIARY_OBJECTIVE_COLOR,

	MICROCHUNKID_HUD_DEF_STAR_BRACKET_SIZE,
	MICROCHUNKID_HUD_DEF_STAR_BRACKET_OFFSET,
	MICROCHUNKID_HUD_DEF_STAR_BRACKET_UV,
	MICROCHUNKID_HUD_DEF_STAR_BRACKET_TOP_SIZE,
	MICROCHUNKID_HUD_DEF_STAR_BRACKET_TOP_OFFSET,
	MICROCHUNKID_HUD_DEF_STAR_BRACKET_TOP_UV,
	MICROCHUNKID_HUD_DEF_STAR_BAR_SIZE,
	MICROCHUNKID_HUD_DEF_STAR_BAR_OFFSET,
	MICROCHUNKID_HUD_DEF_STAR_BAR_UV,
	MICROCHUNKID_HUD_DEF_STAR_BAR_END_SIZE,
	MICROCHUNKID_HUD_DEF_STAR_BAR_END_OFFSET,
	MICROCHUNKID_HUD_DEF_STAR_BAR_END_UV,
	MICROCHUNKID_HUD_DEF_STAR_HEALTH_SIZE,
	MICROCHUNKID_HUD_DEF_STAR_HEALTH_OFFSET,
	MICROCHUNKID_HUD_DEF_STAR_HEALTH_UV,
	MICROCHUNKID_HUD_DEF_STAR_SHIELD_SIZE,
	MICROCHUNKID_HUD_DEF_STAR_SHIELD_OFFSET,
	MICROCHUNKID_HUD_DEF_STAR_SHIELD_UV,
	MICROCHUNKID_HUD_DEF_TARGET_BRACKET_SIZE,
	MICROCHUNKID_HUD_DEF_TARGET_BRACKET_OFFSET,
	MICROCHUNKID_HUD_DEF_TARGET_BRACKET_UV,
	MICROCHUNKID_HUD_DEF_TARGET_ICON_SIZE,
	MICROCHUNKID_HUD_DEF_TARGET_ICON_OFFSET,
	MICROCHUNKID_HUD_DEF_TARGET_NAME_BAR_SIZE,
	MICROCHUNKID_HUD_DEF_TARGET_NAME_BAR_OFFSET,
	MICROCHUNKID_HUD_DEF_TARGET_NAME_BAR_UV,
	MICROCHUNKID_HUD_DEF_TARGET_NAME_OFFSET,
	MICROCHUNKID_HUD_DEF_TARGET_BAR_SIZE,
	MICROCHUNKID_HUD_DEF_TARGET_BAR_OFFSET,
	MICROCHUNKID_HUD_DEF_TARGET_BAR_UV,
	MICROCHUNKID_HUD_DEF_TARGET_BAR_END_SIZE,
	MICROCHUNKID_HUD_DEF_TARGET_BAR_END_OFFSET,
	MICROCHUNKID_HUD_DEF_TARGET_BAR_END_UV,
	MICROCHUNKID_HUD_DEF_TARGET_HEALTH_SIZE,
	MICROCHUNKID_HUD_DEF_TARGET_HEALTH_OFFSET,
	MICROCHUNKID_HUD_DEF_TARGET_HEALTH_UV,
	MICROCHUNKID_HUD_DEF_TARGET_SHIELD_SIZE,
	MICROCHUNKID_HUD_DEF_TARGET_SHIELD_OFFSET,
	MICROCHUNKID_HUD_DEF_TARGET_SHIELD_UV,
	MICROCHUNKID_HUD_DEF_RADAR_OFFSET,
	MICROCHUNKID_HUD_DEF_RADAR_RADIUS,
	MICROCHUNKID_HUD_DEF_RADAR_FRAME_SIZE,
	MICROCHUNKID_HUD_DEF_RADAR_FRAME_UV,
	MICROCHUNKID_HUD_DEF_RADAR_COMPASS_OFFSET,
	MICROCHUNKID_HUD_DEF_RADAR_COMPASS_SIZE,
	MICROCHUNKID_HUD_DEF_RADAR_COMPASS_BASE_UV,
	MICROCHUNKID_HUD_DEF_RADAR_COMPASS_UV_OFFSET,
	MICROCHUNKID_HUD_DEF_RADAR_HUMAN_BLIP_UV,
	MICROCHUNKID_HUD_DEF_RADAR_VEHICLE_BLIP_UV,
	MICROCHUNKID_HUD_DEF_RADAR_STATIONARY_BLIP_UV,
	MICROCHUNKID_HUD_DEF_RADAR_OBJECTIVE_BLIP_UV,
	MICROCHUNKID_HUD_DEF_RADAR_BLIP_BRACKET_UV,
	MICROCHUNKID_HUD_DEF_RADAR_SWEEP_UV,
	MICROCHUNKID_HUD_DEF_STAR_BRACKET_TOP_ARMED_SIZE,
	MICROCHUNKID_HUD_DEF_STAR_BRACKET_TOP_ARMED_OFFSET,
	MICROCHUNKID_HUD_DEF_STAR_BRACKET_TOP_ARMED_UV,
	MICROCHUNKID_HUD_DEF_STAR_WEAPON_ICON_SIZE,
	MICROCHUNKID_HUD_DEF_STAR_WEAPON_ICON_OFFSET,
	MICROCHUNKID_HUD_DEF_SNIPER_VIEW,
	MICROCHUNKID_HUD_DEF_SNIPER_VIEW_UV,
	MICROCHUNKID_HUD_DEF_SNIPER_BLACK_COVER_UV,
	MICROCHUNKID_HUD_DEF_SNIPER_TILT_BAR,
	MICROCHUNKID_HUD_DEF_SNIPER_TILT_BAR_RATE,
	MICROCHUNKID_HUD_DEF_SNIPER_TILT_BAR_UV,
	MICROCHUNKID_HUD_DEF_SNIPER_TURN_BAR,
	MICROCHUNKID_HUD_DEF_SNIPER_TURN_BAR_RATE,
	MICROCHUNKID_HUD_DEF_SNIPER_TURN_BAR_UV,
	MICROCHUNKID_HUD_DEF_SNIPER_DISTANCE_GRAPH,
	MICROCHUNKID_HUD_DEF_SNIPER_DISTANCE_GRAPH_UV,
	MICROCHUNKID_HUD_DEF_SNIPER_DISTANCE_GRAPH_MAX,
	MICROCHUNKID_HUD_DEF_SNIPER_SCAN_LINE_UV,
	MICROCHUNKID_HUD_DEF_INFO_TEXTURE_SIZE,
	MICROCHUNKID_HUD_DEF_RADAR_TEXTURE_SIZE,
	MICROCHUNKID_HUD_DEF_SNIPER_TEXTURE_SIZE,
	MICROCHUNKID_HUD_DEF_DAMAGE_INDICATOR_UV,
	MICROCHUNKID_HUD_DEF_DAMAGE_DIAG_INDICATOR_UV,
	MICROCHUNKID_HUD_DEF_SNIPER_ZOOM_GRAPH,
	MICROCHUNKID_HUD_DEF_SNIPER_ZOOM_GRAPH_UV,
	MICROCHUNKID_HUD_DEF_RADAR_ON_SOUND_ID,
	MICROCHUNKID_HUD_DEF_RADAR_OFF_SOUND_ID,
	MICROCHUNKID_HUD_DEF_MUTANT_COLOR,
	MICROCHUNKID_HUD_DEF_RENEGADE_COLOR,
	MICROCHUNKID_HUD_DEF_HEALTH_HIGH_COLOR,
	MICROCHUNKID_HUD_DEF_HEALTH_MED_COLOR,
	MICROCHUNKID_HUD_DEF_HEALTH_LOW_COLOR,
	MICROCHUNKID_HUD_DEF_ENEMY_COLOR,
	MICROCHUNKID_HUD_DEF_FRIENDLY_COLOR,
	MICROCHUNKID_HUD_DEF_NO_RELATION_COLOR,

};

bool	HUDGlobalSettingsDef::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_HL_DEF_PARENT );
		DefinitionClass::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_HL_DEF_VARIABLES );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_NOD_COLOR,					NodColor );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_GDI_COLOR,					GDIColor );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_NEUTRAL_COLOR,				NeutralColor );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_MUTANT_COLOR,				MutantColor );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_RENEGADE_COLOR,			RenegadeColor );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_PRIMARY_OBJECTIVE_COLOR,	PrimaryObjectiveColor );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_SECONDARY_OBJECTIVE_COLOR,	SecondaryObjectiveColor );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TERTIARY_OBJECTIVE_COLOR,	TertiaryObjectiveColor );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_HEALTH_HIGH_COLOR,			HealthHighColor );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_HEALTH_MED_COLOR,			HealthMedColor );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_HEALTH_LOW_COLOR,			HealthLowColor );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_ENEMY_COLOR,				EnemyColor );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_FRIENDLY_COLOR,			FriendlyColor );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_NO_RELATION_COLOR,			NoRelationColor );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_INFO_TEXTURE_SIZE,			InfoTextureSize );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_BRACKET_SIZE,			StarBracketSize	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_BRACKET_OFFSET,			 StarBracketOffset	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_BRACKET_UV,				 StarBracketUV	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_BRACKET_TOP_SIZE,		 StarBracketTopSize	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_BRACKET_TOP_OFFSET,	StarBracketTopOffset	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_BRACKET_TOP_UV,			 StarBracketTopUV	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_BRACKET_TOP_ARMED_SIZE,	StarBracketTopArmedSize	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_BRACKET_TOP_ARMED_OFFSET,StarBracketTopArmedOffset	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_BRACKET_TOP_ARMED_UV,		StarBracketTopArmedUV	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_BAR_SIZE,					 StarBarSize	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_BAR_OFFSET,				 StarBarOffset	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_BAR_UV,					StarBarUV	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_BAR_END_SIZE,			StarBarEndSize	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_BAR_END_OFFSET,			 StarBarEndOffset	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_BAR_END_UV,				 StarBarEndUV	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_HEALTH_SIZE,				 StarHealthSize	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_HEALTH_OFFSET,			 StarHealthOffset	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_HEALTH_UV,				StarHealthUV	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_SHIELD_SIZE,				 StarShieldSize	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_SHIELD_OFFSET,			 StarShieldOffset	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_SHIELD_UV,				StarShieldUV	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_WEAPON_ICON_SIZE,		StarWeaponIconSize	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_STAR_WEAPON_ICON_OFFSET,	StarWeaponIconOffset	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TARGET_BRACKET_SIZE,			 TargetBracketSize	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TARGET_BRACKET_OFFSET,		 TargetBracketOffset	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TARGET_BRACKET_UV,			TargetBracketUV	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TARGET_ICON_SIZE,				 TargetIconSize	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TARGET_ICON_OFFSET,			 TargetIconOffset	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TARGET_NAME_BAR_SIZE,		TargetNameBarSize	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TARGET_NAME_BAR_OFFSET,		 TargetNameBarOffset	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TARGET_NAME_BAR_UV,			 TargetNameBarUV	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TARGET_NAME_OFFSET,			 TargetNameOffset	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TARGET_BAR_SIZE,				 TargetBarSize	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TARGET_BAR_OFFSET,			TargetBarOffset	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TARGET_BAR_UV,					 TargetBarUV	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TARGET_BAR_END_SIZE,			 TargetBarEndSize	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TARGET_BAR_END_OFFSET,		 TargetBarEndOffset	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TARGET_BAR_END_UV,			TargetBarEndUV	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TARGET_HEALTH_SIZE,			 TargetHealthSize	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TARGET_HEALTH_OFFSET,		TargetHealthOffset	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TARGET_HEALTH_UV,				 TargetHealthUV	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TARGET_SHIELD_SIZE,			 TargetShieldSize	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TARGET_SHIELD_OFFSET,		TargetShieldOffset	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_TARGET_SHIELD_UV,				 TargetShieldUV	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_RADAR_TEXTURE_SIZE,			RadarTextureSize );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_RADAR_OFFSET,						RadarOffset	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_RADAR_RADIUS,						RadarRadius	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_RADAR_FRAME_SIZE,					RadarFrameSize	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_RADAR_FRAME_UV,					RadarFrameUV	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_RADAR_COMPASS_OFFSET,			RadarCompassOffset	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_RADAR_COMPASS_SIZE,				RadarCompassSize	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_RADAR_COMPASS_BASE_UV,			RadarCompassBaseUV	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_RADAR_COMPASS_UV_OFFSET,		RadarCompassUVOffset	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_RADAR_HUMAN_BLIP_UV,				RadarHumanBlipUV	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_RADAR_VEHICLE_BLIP_UV,			RadarVehicleBlipUV	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_RADAR_STATIONARY_BLIP_UV,		RadarStationaryBlipUV	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_RADAR_OBJECTIVE_BLIP_UV,		RadarObjectiveBlipUV	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_RADAR_BLIP_BRACKET_UV,			RadarBlipBracketUV	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_RADAR_SWEEP_UV,					RadarSweepUV	);
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_RADAR_ON_SOUND_ID,				RadarOnSoundID );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_RADAR_OFF_SOUND_ID,				RadarOffSoundID );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_SNIPER_TEXTURE_SIZE,			SniperTextureSize );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_SNIPER_VIEW,						SniperView );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_SNIPER_VIEW_UV,					SniperViewUV );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_SNIPER_SCAN_LINE_UV,				SniperScanLineUV );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_SNIPER_BLACK_COVER_UV,			SniperBlackCoverUV );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_SNIPER_TILT_BAR,					SniperTiltBar );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_SNIPER_TILT_BAR_RATE,			SniperTiltBarRate );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_SNIPER_TILT_BAR_UV,				SniperTiltBarUV );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_SNIPER_TURN_BAR,					SniperTurnBar );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_SNIPER_TURN_BAR_RATE,			SniperTurnBarRate );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_SNIPER_TURN_BAR_UV,				SniperTurnBarUV );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_SNIPER_DISTANCE_GRAPH,			SniperDistanceGraph );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_SNIPER_DISTANCE_GRAPH_UV,		SniperDistanceGraphUV );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_SNIPER_DISTANCE_GRAPH_MAX,		SniperDistanceGraphMax );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_SNIPER_ZOOM_GRAPH,				SniperZoomGraph );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_SNIPER_ZOOM_GRAPH_UV,			SniperZoomGraphUV );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_DAMAGE_INDICATOR_UV,				DamageIndicatorUV );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_DEF_DAMAGE_DIAG_INDICATOR_UV,		DamageDiagIndicatorUV );

	csave.End_Chunk();

	return true;
}

bool	HUDGlobalSettingsDef::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_HL_DEF_PARENT:
				DefinitionClass::Load( cload );
				break;

			case CHUNKID_HL_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_NOD_COLOR,					NodColor );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_GDI_COLOR,					GDIColor );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_NEUTRAL_COLOR,				NeutralColor );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_MUTANT_COLOR,				MutantColor );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_RENEGADE_COLOR,			RenegadeColor );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_PRIMARY_OBJECTIVE_COLOR,	PrimaryObjectiveColor );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_SECONDARY_OBJECTIVE_COLOR,	SecondaryObjectiveColor );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TERTIARY_OBJECTIVE_COLOR,	TertiaryObjectiveColor );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_HEALTH_HIGH_COLOR,			HealthHighColor );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_HEALTH_MED_COLOR,			HealthMedColor );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_HEALTH_LOW_COLOR,			HealthLowColor );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_ENEMY_COLOR,				EnemyColor );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_FRIENDLY_COLOR,			FriendlyColor );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_NO_RELATION_COLOR,			NoRelationColor );

						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_INFO_TEXTURE_SIZE,			InfoTextureSize );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_BRACKET_SIZE,			StarBracketSize	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_BRACKET_OFFSET,			 StarBracketOffset	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_BRACKET_UV,				 StarBracketUV	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_BRACKET_TOP_SIZE,		 StarBracketTopSize	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_BRACKET_TOP_OFFSET,	StarBracketTopOffset	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_BRACKET_TOP_UV,			 StarBracketTopUV	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_BRACKET_TOP_ARMED_SIZE,	StarBracketTopArmedSize	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_BRACKET_TOP_ARMED_OFFSET,StarBracketTopArmedOffset	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_BRACKET_TOP_ARMED_UV,		StarBracketTopArmedUV	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_BAR_SIZE,					 StarBarSize	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_BAR_OFFSET,				 StarBarOffset	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_BAR_UV,					StarBarUV	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_BAR_END_SIZE,			StarBarEndSize	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_BAR_END_OFFSET,			 StarBarEndOffset	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_BAR_END_UV,				 StarBarEndUV	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_HEALTH_SIZE,				 StarHealthSize	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_HEALTH_OFFSET,			 StarHealthOffset	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_HEALTH_UV,				StarHealthUV	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_SHIELD_SIZE,				 StarShieldSize	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_SHIELD_OFFSET,			 StarShieldOffset	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_SHIELD_UV,				StarShieldUV	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_WEAPON_ICON_SIZE,		StarWeaponIconSize	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_STAR_WEAPON_ICON_OFFSET,	StarWeaponIconOffset	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TARGET_BRACKET_SIZE,			 TargetBracketSize	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TARGET_BRACKET_OFFSET,		 TargetBracketOffset	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TARGET_BRACKET_UV,			TargetBracketUV	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TARGET_ICON_SIZE,				 TargetIconSize	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TARGET_ICON_OFFSET,			 TargetIconOffset	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TARGET_NAME_BAR_SIZE,		TargetNameBarSize	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TARGET_NAME_BAR_OFFSET,		 TargetNameBarOffset	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TARGET_NAME_BAR_UV,			 TargetNameBarUV	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TARGET_NAME_OFFSET,			 TargetNameOffset	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TARGET_BAR_SIZE,				 TargetBarSize	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TARGET_BAR_OFFSET,			TargetBarOffset	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TARGET_BAR_UV,					 TargetBarUV	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TARGET_BAR_END_SIZE,			 TargetBarEndSize	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TARGET_BAR_END_OFFSET,		 TargetBarEndOffset	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TARGET_BAR_END_UV,			TargetBarEndUV	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TARGET_HEALTH_SIZE,			 TargetHealthSize	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TARGET_HEALTH_OFFSET,		TargetHealthOffset	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TARGET_HEALTH_UV,				 TargetHealthUV	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TARGET_SHIELD_SIZE,			 TargetShieldSize	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TARGET_SHIELD_OFFSET,		TargetShieldOffset	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_TARGET_SHIELD_UV,				 TargetShieldUV	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_RADAR_TEXTURE_SIZE,			RadarTextureSize );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_RADAR_OFFSET,						RadarOffset	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_RADAR_RADIUS,						RadarRadius	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_RADAR_FRAME_SIZE,					RadarFrameSize	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_RADAR_FRAME_UV,					RadarFrameUV	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_RADAR_COMPASS_OFFSET,			RadarCompassOffset	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_RADAR_COMPASS_SIZE,				RadarCompassSize	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_RADAR_COMPASS_BASE_UV,			RadarCompassBaseUV	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_RADAR_COMPASS_UV_OFFSET,		RadarCompassUVOffset	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_RADAR_HUMAN_BLIP_UV,				RadarHumanBlipUV	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_RADAR_VEHICLE_BLIP_UV,			RadarVehicleBlipUV	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_RADAR_STATIONARY_BLIP_UV,		RadarStationaryBlipUV	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_RADAR_OBJECTIVE_BLIP_UV,		RadarObjectiveBlipUV	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_RADAR_BLIP_BRACKET_UV,			RadarBlipBracketUV	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_RADAR_SWEEP_UV,					RadarSweepUV	);
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_RADAR_ON_SOUND_ID,				RadarOnSoundID );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_RADAR_OFF_SOUND_ID,				RadarOffSoundID );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_SNIPER_TEXTURE_SIZE,			SniperTextureSize );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_SNIPER_VIEW,						SniperView );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_SNIPER_VIEW_UV,					SniperViewUV );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_SNIPER_SCAN_LINE_UV,				SniperScanLineUV );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_SNIPER_BLACK_COVER_UV,			SniperBlackCoverUV );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_SNIPER_TILT_BAR,					SniperTiltBar );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_SNIPER_TILT_BAR_RATE,			SniperTiltBarRate );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_SNIPER_TILT_BAR_UV,				SniperTiltBarUV );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_SNIPER_TURN_BAR,					SniperTurnBar );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_SNIPER_TURN_BAR_RATE,			SniperTurnBarRate );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_SNIPER_TURN_BAR_UV,				SniperTurnBarUV );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_SNIPER_DISTANCE_GRAPH,			SniperDistanceGraph );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_SNIPER_DISTANCE_GRAPH_UV,		SniperDistanceGraphUV );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_SNIPER_DISTANCE_GRAPH_MAX,		SniperDistanceGraphMax );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_SNIPER_ZOOM_GRAPH,				SniperZoomGraph );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_SNIPER_ZOOM_GRAPH_UV,			SniperZoomGraphUV );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_DAMAGE_INDICATOR_UV,				DamageIndicatorUV );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_DEF_DAMAGE_DIAG_INDICATOR_UV,		DamageDiagIndicatorUV );



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

/*
** HumanAnimOverrides
*/
SimplePersistFactoryClass<HumanAnimOverrideDef, CHUNKID_GLOBAL_SETTINGS_DEF_HUMAN_ANIM_OVERRIDE>	_HumanAnimOverrideDefPersistFactory;

DECLARE_DEFINITION_FACTORY(HumanAnimOverrideDef, CLASSID_GLOBAL_SETTINGS_DEF_HUMAN_ANIM_OVERRIDE, "HUMAN_ANIM_OVERRIDE") _HumanAnimOverrideDefDefFactory;

HumanAnimOverrideDef::HumanAnimOverrideDef( void )
{
	EDITABLE_PARAM( HumanAnimOverrideDef, ParameterClass::TYPE_STRING, RunEmptyHands );
	EDITABLE_PARAM( HumanAnimOverrideDef, ParameterClass::TYPE_STRING, WalkEmptyHands );
	EDITABLE_PARAM( HumanAnimOverrideDef, ParameterClass::TYPE_STRING, RunAtChest );
	EDITABLE_PARAM( HumanAnimOverrideDef, ParameterClass::TYPE_STRING, WalkAtChest );
	EDITABLE_PARAM( HumanAnimOverrideDef, ParameterClass::TYPE_STRING, RunAtHip );
	EDITABLE_PARAM( HumanAnimOverrideDef, ParameterClass::TYPE_STRING, WalkAtHip );
}

uint32	HumanAnimOverrideDef::Get_Class_ID (void) const
{
	return CLASSID_GLOBAL_SETTINGS_DEF_HUMAN_ANIM_OVERRIDE;
}

const PersistFactoryClass & HumanAnimOverrideDef::Get_Factory (void) const
{
	return _HumanAnimOverrideDefPersistFactory;
}

PersistClass *	HumanAnimOverrideDef::Create( void ) const
{
	WWASSERT( 0 );
	return NULL;
}

enum	{
	CHUNKID_HAO_DEF_PARENT							=	726011912,
	CHUNKID_HAO_DEF_VARIABLES,

	MICROCHUNKID_HAO_DEF_RUN_EMPTY_HANDS		=	1,
	MICROCHUNKID_HAO_DEF_WALK_EMPTY_HANDS,
	MICROCHUNKID_HAO_DEF_RUN_AT_CHEST,
	MICROCHUNKID_HAO_DEF_WALK_AT_CHEST,
	MICROCHUNKID_HAO_DEF_RUN_AT_HIP,
	MICROCHUNKID_HAO_DEF_WALK_AT_HIP,


};

bool	HumanAnimOverrideDef::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_HAO_DEF_PARENT );
		DefinitionClass::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_HAO_DEF_VARIABLES );
		WRITE_MICRO_CHUNK_WWSTRING( csave, 	MICROCHUNKID_HAO_DEF_RUN_EMPTY_HANDS,	RunEmptyHands );
		WRITE_MICRO_CHUNK_WWSTRING( csave, 	MICROCHUNKID_HAO_DEF_WALK_EMPTY_HANDS,	WalkEmptyHands );
		WRITE_MICRO_CHUNK_WWSTRING( csave, 	MICROCHUNKID_HAO_DEF_RUN_AT_CHEST,		RunAtChest );
		WRITE_MICRO_CHUNK_WWSTRING( csave, 	MICROCHUNKID_HAO_DEF_WALK_AT_CHEST,		WalkAtChest );
		WRITE_MICRO_CHUNK_WWSTRING( csave, 	MICROCHUNKID_HAO_DEF_RUN_AT_HIP,			RunAtHip );
		WRITE_MICRO_CHUNK_WWSTRING( csave, 	MICROCHUNKID_HAO_DEF_WALK_AT_HIP,		WalkAtHip );
	csave.End_Chunk();

	return true;
}

bool	HumanAnimOverrideDef::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_HAO_DEF_PARENT:
				DefinitionClass::Load( cload );
				break;

			case CHUNKID_HAO_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK_WWSTRING( cload, 	MICROCHUNKID_HAO_DEF_RUN_EMPTY_HANDS,	RunEmptyHands );
						READ_MICRO_CHUNK_WWSTRING( cload, 	MICROCHUNKID_HAO_DEF_WALK_EMPTY_HANDS,	WalkEmptyHands );
						READ_MICRO_CHUNK_WWSTRING( cload, 	MICROCHUNKID_HAO_DEF_RUN_AT_CHEST,		RunAtChest );
						READ_MICRO_CHUNK_WWSTRING( cload, 	MICROCHUNKID_HAO_DEF_WALK_AT_CHEST,		WalkAtChest );
						READ_MICRO_CHUNK_WWSTRING( cload, 	MICROCHUNKID_HAO_DEF_RUN_AT_HIP,			RunAtHip );
						READ_MICRO_CHUNK_WWSTRING( cload, 	MICROCHUNKID_HAO_DEF_WALK_AT_HIP,		WalkAtHip );

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

