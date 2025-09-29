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
 *                     $Archive:: /Commando/Code/Combat/damage.cpp                            $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/21/02 2:42p                                               $*
 *                                                                                             *
 *                    $Revision:: 114                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
**	Includes
*/
#include "damage.h"
#include "assets.h"
#include "debug.h"
#include "smartgameobj.h"
#include "combat.h"
#include "slist.h"
#include "wwpacket.h"
#include "playerdata.h"
#include "soldier.h"
#include "weapons.h"
#include "diaglog.h"
#include "playertype.h"
#include <string.h>
#include <stdio.h>

#include "wwmath.h"
#include "encoderlist.h"
#include "bitpackids.h"
#include "gametype.h"
#include "csdamageevent.h"

#ifdef WWDEBUG
bool	DefenseObjectClass::OneShotKills	= false;
#endif // _WWDEBUG

ArmorWarheadManager	ArmorManagerObject;

DynamicVectorClass<StringClass>  	ArmorNames;
DynamicVectorClass<StringClass>	  	WarheadNames;
DynamicVectorClass<int>				  	ArmorSaveIDs;
DynamicVectorClass<int>				  	WarheadSaveIDs;
DynamicVectorClass<bool>				SoftArmorTable;

// For each Warhead
DynamicVectorClass<int>					SpecialDamageTypes;
DynamicVectorClass<float>				SpecialDamageProbability;
DynamicVectorClass<float>				VisceroidProbability;


// For each special damage type
int	SpecialDamageWarhead[ArmorWarheadManager::NUM_SPECIAL_DAMAGE_TYPES];
float SpecialDamageDuration[ArmorWarheadManager::NUM_SPECIAL_DAMAGE_TYPES];
float SpecialDamageScale[ArmorWarheadManager::NUM_SPECIAL_DAMAGE_TYPES];
StringClass SpecialDamageExplosion[ArmorWarheadManager::NUM_SPECIAL_DAMAGE_TYPES];

// Skins that are imprevious to special damage
DynamicVectorClass<ArmorType>		ImperviousSkins[ArmorWarheadManager::NUM_SPECIAL_DAMAGE_TYPES];

safe_float		*	ArmorWarheadManager::Multipliers = NULL;
safe_float		*	ArmorWarheadManager::Absorbsion = NULL;

#define	ARMOR_INI_FILENAME		"armor.ini"

#define	SECTION_SOFT_ARMOR_TYPES				"Soft_Armor_Types"
#define	SECTION_HARD_ARMOR_TYPES				"Hard_Armor_Types"
#define	SECTION_ARMOR_TYPES						"Armor_Types"
#define	SECTION_WARHEAD_TYPES					"Warhead_Types"
#define	SECTION_SCALE								"Scale_%s"
#define	SECTION_SHIELD								"Shield_%s"
#define	SECTION_ARMOR_SAVE_IDS					"Armor_Save_IDs"
#define	SECTION_WARHEAD_SAVE_IDS				"Warhead_Save_IDs"
#define	SECTION_SOFT_ARMOR						"Soft_Armor"
#define	SECTION_SPECIAL_DAMAGE_TYPE			"Special_Damage_Type"
#define	SECTION_SPECIAL_DAMAGE_PROBABILITY	"Special_Damage_Probability"
#define	SECTION_VISCEROID_PROBABILITY			"Visceroid_Probability"

#define	ENTRY_WARHEAD								"Warhead"
#define	ENTRY_DURATION								"Duration"
#define	ENTRY_SCALE									"Scale"
#define	ENTRY_EXPLOSION							"Explosion"

/*
**
*/
void	ArmorWarheadManager::Init( void )
{
	Shutdown();

	INIClass	* armorINI = Get_INI( ARMOR_INI_FILENAME );
	if (armorINI != NULL) {

		WWASSERT( armorINI && armorINI->Section_Count() > 0 );

		int count, entry;

		// Load Armor Types
		count =  armorINI->Entry_Count( SECTION_ARMOR_TYPES );
		for ( entry = 0; entry < count; entry++ )	{
			char	entry_name[80];
			armorINI->Get_String(	SECTION_ARMOR_TYPES,
											armorINI->Get_Entry( SECTION_ARMOR_TYPES, entry),
											"",	entry_name,		sizeof( entry_name ) );
			ArmorNames.Add( entry_name );
			ArmorSaveIDs.Add( entry );
			SoftArmorTable.Add( false );
		}

		// Load Warhead Types
		count =  armorINI->Entry_Count( SECTION_WARHEAD_TYPES );
		for ( entry = 0; entry < count; entry++ )	{
			char	entry_name[80];
			armorINI->Get_String(	SECTION_WARHEAD_TYPES,
											armorINI->Get_Entry( SECTION_WARHEAD_TYPES, entry),
											"",	entry_name,		sizeof( entry_name ) );
			WarheadNames.Add( entry_name );
			WarheadSaveIDs.Add( entry );
		}

		int armor_num;

		// Load Armor Save IDs
		for ( armor_num = 0; armor_num < ArmorNames.Count(); armor_num++ ) {
			int id = armorINI->Get_Int(	SECTION_ARMOR_SAVE_IDS, ArmorNames[armor_num], -100 );
			if ( id == -100 ) {
				Debug_Say(( "Missing Armor_Save_ID for %s\n", (const char *)ArmorNames[armor_num] ));
			}
			ArmorSaveIDs[ armor_num ] = id;
		}

		// Load Warhead Save IDs
		int warhead_num;
		for ( warhead_num = 0; warhead_num < WarheadNames.Count(); warhead_num++ ) {
			int id = armorINI->Get_Int(	SECTION_WARHEAD_SAVE_IDS, WarheadNames[warhead_num], -100 );
			if ( id == -100 ) {
				Debug_Say(( "Missing Warhead_Save_ID for %s\n", (const char *)WarheadNames[warhead_num] ));
			}
			WarheadSaveIDs[ warhead_num ] = id;
		}

		// Load Soft Armor Table
		count =  armorINI->Entry_Count( SECTION_SOFT_ARMOR );
		for ( entry = 0; entry < count; entry++ )	{
			char	entry_name[80];
			armorINI->Get_String(	SECTION_SOFT_ARMOR,
											armorINI->Get_Entry( SECTION_SOFT_ARMOR, entry),
											"",	entry_name,		sizeof( entry_name ) );

			ArmorType type = Get_Armor_Type( entry_name );
			SoftArmorTable[ type ] = true;
		}


		// Load Multipliers
		Multipliers = new safe_float[ Get_Num_Armor_Types() * Get_Num_Warhead_Types() ];
		for ( armor_num = 0; armor_num < ArmorNames.Count(); armor_num++ ) {
			char section_name[80];
			sprintf( section_name, SECTION_SCALE, (const char *)ArmorNames[armor_num] );
			for ( int warhead_num = 0; warhead_num < WarheadNames.Count(); warhead_num++ ) {
				Multipliers[ armor_num * Get_Num_Warhead_Types() + warhead_num ]  =
					armorINI->Get_Float( section_name, WarheadNames[warhead_num], 1.0f );
			}
		}

		// Load Shield Absorbsion
		Absorbsion = new safe_float[ Get_Num_Armor_Types() * Get_Num_Warhead_Types() ];
		for ( armor_num = 0; armor_num < ArmorNames.Count(); armor_num++ ) {
			char section_name[80];
			sprintf( section_name, SECTION_SHIELD, (const char *)ArmorNames[armor_num] );
			for ( int warhead_num = 0; warhead_num < WarheadNames.Count(); warhead_num++ ) {
				Absorbsion[ armor_num * Get_Num_Warhead_Types() + warhead_num ]  =
					armorINI->Get_Float( section_name, WarheadNames[warhead_num], 0.0f );
			}
		}

		// Load Warhead Special Damage
		for ( warhead_num = 0; warhead_num < WarheadNames.Count(); warhead_num++ ) {

			SpecialDamageTypes.Add( SPECIAL_DAMAGE_TYPE_NONE );
			SpecialDamageProbability.Add( 0 );
			VisceroidProbability.Add( 0 );

			StringClass	special_damage_type(0,true);
			armorINI->Get_String(special_damage_type,	SECTION_SPECIAL_DAMAGE_TYPE, WarheadNames[warhead_num] );
			if ( !special_damage_type.Is_Empty() ) {
				if ( !stricmp( special_damage_type, "FIRE" ) ) {
					SpecialDamageTypes[warhead_num] = SPECIAL_DAMAGE_TYPE_FIRE;
				}
				if ( !stricmp( special_damage_type, "CHEM" ) ) {
					SpecialDamageTypes[warhead_num] = SPECIAL_DAMAGE_TYPE_CHEM;
				}
				if ( !stricmp( special_damage_type, "CNC_FIRE" ) ) {
					SpecialDamageTypes[warhead_num] = SPECIAL_DAMAGE_TYPE_CNC_FIRE;
				}
				if ( !stricmp( special_damage_type, "CNC_CHEM" ) ) {
					SpecialDamageTypes[warhead_num] = SPECIAL_DAMAGE_TYPE_CNC_CHEM;
				}
				if ( !stricmp( special_damage_type, "ELECTRIC" ) ) {
					SpecialDamageTypes[warhead_num] = SPECIAL_DAMAGE_TYPE_ELECTRIC;
				}
			}

			SpecialDamageProbability[warhead_num] = 
				armorINI->Get_Float(	SECTION_SPECIAL_DAMAGE_PROBABILITY, WarheadNames[warhead_num], 0.0f );

			VisceroidProbability[warhead_num] = 
				armorINI->Get_Float(	SECTION_VISCEROID_PROBABILITY, WarheadNames[warhead_num], 0.0f );

//			Debug_Say(( "Warhead %s %d %f\n", WarheadNames[warhead_num], 
//				SpecialDamageTypes[warhead_num], SpecialDamageProbability[warhead_num] ));
		}

		int i;
		for ( i = 0; i < NUM_SPECIAL_DAMAGE_TYPES; i++ ) {
			SpecialDamageWarhead[i] = 0;
			SpecialDamageDuration[i] = 0;
			SpecialDamageScale[i] = 0;
			SpecialDamageExplosion[i] = "";
		}

		StringClass temp_string(0,true);
		for ( i = SPECIAL_DAMAGE_TYPE_FIRE; i < NUM_SPECIAL_DAMAGE_TYPES; i++ ) {
			const char * SpecialDamageSectionNames[NUM_SPECIAL_DAMAGE_TYPES] = {
				"Special_Damage_None",
				"Special_Damage_Fire",
				"Special_Damage_Chem",
				"Special_Damage_Electric",
				"Special_Damage_CNC_Fire",
				"Special_Damage_CNC_Chem",
			};
			StringClass section(SpecialDamageSectionNames[i],true);
			SpecialDamageWarhead[i] = Get_Warhead_Type( armorINI->Get_String( temp_string, section, ENTRY_WARHEAD ) );
			SpecialDamageDuration[i] = armorINI->Get_Float(	section, ENTRY_DURATION, 0 );
			SpecialDamageScale[i] = armorINI->Get_Float(	section, ENTRY_SCALE, 1 );
			SpecialDamageExplosion[i] = armorINI->Get_String( temp_string, section, ENTRY_EXPLOSION );
		}

		//
		//	Copy fire into super fire
		//
		SpecialDamageWarhead[SPECIAL_DAMAGE_TYPE_SUPER_FIRE]		= SpecialDamageWarhead[SPECIAL_DAMAGE_TYPE_FIRE];
		SpecialDamageDuration[SPECIAL_DAMAGE_TYPE_SUPER_FIRE]		= SpecialDamageDuration[SPECIAL_DAMAGE_TYPE_FIRE];
		SpecialDamageScale[SPECIAL_DAMAGE_TYPE_SUPER_FIRE]			= SpecialDamageScale[SPECIAL_DAMAGE_TYPE_FIRE];
		SpecialDamageExplosion[SPECIAL_DAMAGE_TYPE_SUPER_FIRE]	= SpecialDamageExplosion[SPECIAL_DAMAGE_TYPE_FIRE];

		// Load Impervious skin
		for ( int damage = SPECIAL_DAMAGE_TYPE_FIRE; damage < NUM_SPECIAL_DAMAGE_TYPES; damage++ ) {
			const char * ImperviousSectionNames[NUM_SPECIAL_DAMAGE_TYPES] = {
				"Impervious_None",
				"Impervious_Fire",
				"Impervious_Chem",
				"Impervious_Electric",
				"Impervious_CNC_Fire",
				"Impervious_CNC_Chem",
			};
			StringClass section = ImperviousSectionNames[damage];

			int count =  armorINI->Entry_Count( section );
			for ( entry = 0; entry < count; entry++ )	{
				StringClass	entry_name = armorINI->Get_String( temp_string, section, armorINI->Get_Entry( section, entry) );
				if ( !entry_name.Is_Empty() ) {
					ImperviousSkins[damage].Add( Get_Armor_Type( entry_name ) );
				}
			}

//			for ( int i = 0; i < ImperviousSkins[damage].Count(); i++ ) {
//				Debug_Say(( "Loaded %s %d\n", (const char *)section, ImperviousSkins[damage][i] ));
//			}
		}


		Release_INI( armorINI );

	} else {
		Debug_Say(("ArmorWarheadManager::Init - Unable to load %s\n", ARMOR_INI_FILENAME));
	}
}

void ArmorWarheadManager::Shutdown( void )
{
	if ( Multipliers != NULL ) {
		delete [] Multipliers;
		Multipliers = NULL;
	}

	if ( Absorbsion != NULL ) {
		delete [] Absorbsion;
		Absorbsion = NULL;
	}

	ArmorNames.Delete_All();
	WarheadNames.Delete_All();

	ArmorSaveIDs.Delete_All();
	WarheadSaveIDs.Delete_All();
	SoftArmorTable.Delete_All();

	SpecialDamageTypes.Delete_All();
	SpecialDamageProbability.Delete_All();
	VisceroidProbability.Delete_All();

	for (int i=0; i<ArmorWarheadManager::NUM_SPECIAL_DAMAGE_TYPES; i++) {
		ImperviousSkins[i].Delete_All();
	}
}

int			 	ArmorWarheadManager::Get_Num_Armor_Types( void )
{
 	return ArmorNames.Count();
}

int			 	ArmorWarheadManager::Get_Num_Warhead_Types( void )
{
	return WarheadNames.Count();
}

ArmorType		ArmorWarheadManager::Get_Armor_Type( const char *name )
{
	for (	int index = 0; index < ArmorNames.Count(); index++ ) {
		if ( !stricmp( ArmorNames[index], name ) ) {
			return index;
		}
	}
	Debug_Say(( "Armor %s Not Found\n", name ));
//	WWASSERT( 0 );	//Armor type not found!
	return 0;
}

WarheadType		ArmorWarheadManager::Get_Warhead_Type( const char *name )
{
	for (	int index = 0; index < WarheadNames.Count(); index++ ) {
		if ( !stricmp( WarheadNames[index], name ) ) {
			return index;
		}
	}
	Debug_Say(( "Warhead %s Not Found\n", name ));
//	WWASSERT( 0 );	//Warhead type not found!
	return 0;
}

const char * ArmorWarheadManager::Get_Armor_Name( ArmorType type )
{
	WWASSERT( (int)type >= 0 );
	WWASSERT( (int)type < Get_Num_Armor_Types() );
	return ArmorNames[type];
}

const char *	ArmorWarheadManager::Get_Warhead_Name( WarheadType type )
{
	WWASSERT( (int)type >= 0 );
	WWASSERT( (int)type < Get_Num_Warhead_Types() );
	return WarheadNames[type];
}


float	ArmorWarheadManager::Get_Damage_Multiplier( ArmorType armor, WarheadType warhead )
{
	WWASSERT( (int)armor >= 0 );
	WWASSERT( (int)armor < Get_Num_Armor_Types() );
	WWASSERT( (int)warhead >= 0 );
	WWASSERT( (int)warhead < Get_Num_Warhead_Types() );
	return( Multipliers[ (unsigned int)armor * Get_Num_Warhead_Types() + (unsigned int)warhead ] );

	/*
	int index = armor * Get_Num_Warhead_Types() + warhead;
	float multiplier = Multipliers[index];
	WWASSERT(WWMath::Is_Valid_Float(multiplier));

	Debug_Say(("index = %d, armor = %d, warhead = %d, multiplier = %5.2f\n",
		index, armor, warhead, multiplier));

	return multiplier;
	*/
}

float	ArmorWarheadManager::Get_Shield_Absorbsion( ArmorType armor, WarheadType warhead )
{
	WWASSERT( (int)armor >= 0 );
	WWASSERT( (int)armor < Get_Num_Armor_Types() );
	WWASSERT( (int)warhead >= 0 );
	WWASSERT( (int)warhead < Get_Num_Warhead_Types() );
	return( Absorbsion[ (unsigned int)armor * Get_Num_Warhead_Types() + (unsigned int)warhead ] );
}


bool				ArmorWarheadManager::Is_Armor_Soft( ArmorType armor )
{
	return SoftArmorTable[armor];
}

int			 	ArmorWarheadManager::Get_Armor_Save_ID( ArmorType type )
{
	return	ArmorSaveIDs[ type ];
}

ArmorType	 	ArmorWarheadManager::Find_Armor_Save_ID( int id )
{
	for (	int index = 0; index < ArmorSaveIDs.Count(); index++ ) {
		if ( ArmorSaveIDs[ index ] == id ) {
			return index;
		}
	}
	//Debug_Say(( "FAILED to find ARMOR_SAVE_ID for %d\n", id ));
	return 0;
}

int			 	ArmorWarheadManager::Get_Warhead_Save_ID( WarheadType type )
{
	return	WarheadSaveIDs[ type ];
}

ArmorType	 	ArmorWarheadManager::Find_Warhead_Save_ID( int id )
{
	for (	int index = 0; index < WarheadSaveIDs.Count(); index++ ) {
		if ( WarheadSaveIDs[ index ] == id ) {
			return index;
		}
	}
	Debug_Say(( "FAILED to find WARHEAD_SAVE_ID for %d\n", id ));
	return 0;
}

ArmorWarheadManager::SpecialDamageType	ArmorWarheadManager::Get_Special_Damage_Type( WarheadType type )
{
	WWASSERT( (int)type >= 0 );
	WWASSERT( (int)type < Get_Num_Warhead_Types() );
	return (SpecialDamageType)SpecialDamageTypes[type];
}

float	ArmorWarheadManager::Get_Special_Damage_Probability( WarheadType type )
{
	WWASSERT( (int)type >= 0 );
	WWASSERT( (int)type < Get_Num_Warhead_Types() );
	return SpecialDamageProbability[type];
}

WarheadType ArmorWarheadManager::Get_Special_Damage_Warhead( SpecialDamageType type )
{
	WWASSERT( (int)type >= 0 );
	WWASSERT( (int)type < NUM_SPECIAL_DAMAGE_TYPES );
	return SpecialDamageWarhead[type];
}

float	ArmorWarheadManager::Get_Special_Damage_Duration( SpecialDamageType type )
{
	WWASSERT( (int)type >= 0 );
	WWASSERT( (int)type < NUM_SPECIAL_DAMAGE_TYPES );
	return SpecialDamageDuration[type];
}

float	ArmorWarheadManager::Get_Special_Damage_Scale( SpecialDamageType type )
{
	WWASSERT( (int)type >= 0 );
	WWASSERT( (int)type < NUM_SPECIAL_DAMAGE_TYPES );
	return SpecialDamageScale[type];
}

const char *	ArmorWarheadManager::Get_Special_Damage_Explosion( SpecialDamageType type )
{
	WWASSERT( (int)type >= 0 );
	WWASSERT( (int)type < NUM_SPECIAL_DAMAGE_TYPES );
	return SpecialDamageExplosion[type];
}

float	ArmorWarheadManager::Get_Visceroid_Probability( WarheadType type )
{
	WWASSERT( (int)type >= 0 );
	WWASSERT( (int)type < Get_Num_Warhead_Types() );
	return VisceroidProbability[type];
}

bool	ArmorWarheadManager::Is_Skin_Impervious( SpecialDamageType type, ArmorType skin )
{
	WWASSERT( (int)type >= 0 );
	WWASSERT( (int)type < NUM_SPECIAL_DAMAGE_TYPES );
	for ( int i = 0; i < ImperviousSkins[type].Count(); i++ ) {
		if ( ImperviousSkins[type][i] == skin ) {
			return true;
		}
	}
	return false;
}

/*
**
*/
OffenseObjectClass::OffenseObjectClass( const OffenseObjectClass & base ) :
	Damage( base.Damage ),
	Warhead( base.Warhead )
{
	Set_Owner( base.Get_Owner() );
}


/*
** Save and Load
*/
enum	{
	CHUNKID_VARIABLES						=	914991020,
	CHUNKID_OWNER,

	MICROCHUNKID_WARHEAD					=  1,
	MICROCHUNKID_DAMAGE,
	MICROCHUNKID_HEALTH,
	MICROCHUNKID_HEALTH_MAX,
	MICROCHUNKID_SKIN,
	MICROCHUNKID_SHIELD_STRENGTH,
	MICROCHUNKID_SHIELD_STRENGTH_MAX,
	MICROCHUNKID_SHIELD_TYPE,
	XXXMICROCHUNKID_LAST_HEALTH,
	XXXMICROCHUNKID_LAST_SKIN,
	XXXMICROCHUNKID_LAST_SHIELD_STRENGTH,
	XXXMICROCHUNKID_LAST_SHIELD_TYPE,
	MICROCHUNKID_DAMAGE_POINTS,
	MICROCHUNKID_DEATH_POINTS,
};

bool	OffenseObjectClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_VARIABLES );
		int save_id = ArmorWarheadManager::Get_Warhead_Save_ID( Warhead );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_WARHEAD, save_id );
		WRITE_SAFE_MICRO_CHUNK( csave, MICROCHUNKID_DAMAGE, Damage, float );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_OWNER );
	Owner.Save( csave );
	csave.End_Chunk();

	return true;
}

bool	OffenseObjectClass::Load( ChunkLoadClass &cload )
{
	int save_id = -2;
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_WARHEAD, save_id );
						READ_SAFE_MICRO_CHUNK( cload, MICROCHUNKID_DAMAGE, Damage, float);

						default:
							Debug_Say(( "Unrecognized Offense Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			case CHUNKID_OWNER:
				Owner.Load( cload );
				break;

			default:
				Debug_Say(( "Unrecognized Offense chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	Warhead = ArmorWarheadManager::Find_Warhead_Save_ID( save_id );
	return true;
}

void DefenseObjectClass::Init( const DefenseObjectDefClass & def, DamageableGameObj * owner )
{
	Health = def.Health;
	HealthMax = def.HealthMax;
	Skin = def.Skin;
	ShieldStrength = def.ShieldStrength;
	ShieldStrengthMax = def.ShieldStrengthMax;
	ShieldType = def.ShieldType;
	DamagePoints = def.DamagePoints;
	DeathPoints = def.DeathPoints;

	Set_Owner( owner );
}


bool	DefenseObjectClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_SAFE_MICRO_CHUNK( csave, MICROCHUNKID_HEALTH, Health, float );
		WRITE_SAFE_MICRO_CHUNK( csave, MICROCHUNKID_HEALTH_MAX, HealthMax, float );
		int skin_save_id = ArmorWarheadManager::Get_Armor_Save_ID( Skin );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_SKIN, skin_save_id );
		WRITE_SAFE_MICRO_CHUNK( csave, MICROCHUNKID_SHIELD_STRENGTH, ShieldStrength, float );
		WRITE_SAFE_MICRO_CHUNK( csave, MICROCHUNKID_SHIELD_STRENGTH_MAX, ShieldStrengthMax, float );
		int shield_save_id = ArmorWarheadManager::Get_Armor_Save_ID( ShieldType );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_SHIELD_TYPE, shield_save_id );
		WRITE_SAFE_MICRO_CHUNK( csave, MICROCHUNKID_DAMAGE_POINTS, DamagePoints, float );
		WRITE_SAFE_MICRO_CHUNK( csave, MICROCHUNKID_DEATH_POINTS, DeathPoints, float );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_OWNER );
	Owner.Save( csave );
	csave.End_Chunk();

	return true;
}

bool	DefenseObjectClass::Load( ChunkLoadClass &cload )
{
	int skin_save_id = -2;
	int shield_save_id = -2;
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_SAFE_MICRO_CHUNK( cload, MICROCHUNKID_HEALTH, Health, float );
						READ_SAFE_MICRO_CHUNK( cload, MICROCHUNKID_HEALTH_MAX, HealthMax, float );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_SKIN, skin_save_id );
						READ_SAFE_MICRO_CHUNK( cload, MICROCHUNKID_SHIELD_STRENGTH, ShieldStrength, float );
						READ_SAFE_MICRO_CHUNK( cload, MICROCHUNKID_SHIELD_STRENGTH_MAX, ShieldStrengthMax, float );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_SHIELD_TYPE, shield_save_id );
						READ_SAFE_MICRO_CHUNK( cload, MICROCHUNKID_DAMAGE_POINTS, DamagePoints, float );
						READ_SAFE_MICRO_CHUNK( cload, MICROCHUNKID_DEATH_POINTS, DeathPoints, float );

						default:
							Debug_Say(( "Unrecognized Defense Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			case CHUNKID_OWNER:
				Owner.Load( cload );
				break;

			default:
				Debug_Say(( "Unrecognized Defense chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	Skin = ArmorWarheadManager::Find_Armor_Save_ID( skin_save_id );
	ShieldType = ArmorWarheadManager::Find_Armor_Save_ID( shield_save_id );

	return true;
}




bool	DefenseObjectClass::Is_Soft( void )
{
	if ( !ArmorWarheadManager::Is_Armor_Soft( Skin ) ) {
		return false;
	}

	if ( ( (float)ShieldStrength > 0 ) && ( !ArmorWarheadManager::Is_Armor_Soft( ShieldType ) ) ) {
		return false;
	}

	return true;
}


/*
**
*/
float	DefenseObjectClass::Apply_Damage( const OffenseObjectClass & offense, float scale, int alternate_skin )
{
#if 1
	// If singleplay or non trusted clients, just apply damage normally, if server
	bool normal = IS_SOLOPLAY || !cCsDamageEvent::Get_Are_Clients_Trusted();
	DamageableGameObj * owner = Get_Owner();
	// Buildings have no owner
	if ( owner == NULL || owner->As_BuildingGameObj() ) {
		normal = true;
	}
	if ( offense.ForceServerDamage || !offense.EnableClientDamage ) {
		normal = true;
	}
	if ( normal ) {
		if ( CombatManager::I_Am_Server() ) {
			return Do_Damage( offense, scale, alternate_skin );
		} else {
			return Health;
		}
	}

	// Client Damage
	// If I am a client, and damager is my guy, tell the server that damage should apply
	// If I am a server, and the damager has a client, ignore the damage
	bool has_client = false;
	bool my_client = false;
	if ( offense.Get_Owner() != NULL && offense.Get_Owner()->As_SmartGameObj() ) {
		int control_owner = offense.Get_Owner()->As_SmartGameObj()->Get_Control_Owner();
		has_client = control_owner > 0;
		my_client = control_owner == CombatManager::Get_My_Id();
	}

	if ( CombatManager::I_Am_Server() ) {
		if ( my_client || !has_client ) {
			return Do_Damage( offense, scale, alternate_skin );
		} else {
			return Health;	// assume we will be notified by the owner Don't apply the damage
		}
	}

	// I am a client.  If this is my object, request damage
	if ( my_client ) {
		Request_Damage( offense, scale );
	}
	return Health;	// Don't apply the damage
#else
	// Client Authorative damage is no longer done here.
	if ( CombatManager::I_Am_Server() ) {
		return Do_Damage( offense, scale, alternate_skin );
	} else {
		return Health;
	}
#endif
}


/*
**
*/
void	DefenseObjectClass::Request_Damage( const OffenseObjectClass & offense, float scale )
{
	WWASSERT(CombatManager::I_Am_Only_Client());
	WWASSERT(cCsDamageEvent::Get_Are_Clients_Trusted() == true);

	cCsDamageEvent * event = new cCsDamageEvent();
	if ( event ) {
		int damager_id = 0;
		if ( offense.Get_Owner() ) {
			damager_id = offense.Get_Owner()->Get_ID();
		}
		int damagee_id = 0;
		if ( Get_Owner() ) {
			damagee_id = Get_Owner()->Get_ID();
		}
		int warhead = offense.Get_Warhead();
		float damage = offense.Get_Damage() * scale;
		event->Init( damager_id, damagee_id, damage, warhead );
//		Debug_Say(( "Requesting damage of %f from %d to %d\n", damage, damager_id, damagee_id ));
	}
}


/*
**
*/
float	DefenseObjectClass::Do_Damage( const OffenseObjectClass & offense, float scale, int alternate_skin )
{
	SmartGameObj * smart = NULL;

	if ( offense.Get_Owner() != NULL ) {
		smart = offense.Get_Owner()->As_SmartGameObj();
	}

	//TSS102201
	//
	// Keep track of who is attempting to damage who. This is used as a server
	// heuristic used in network priority calculations. It doesn't matter whether
	// or not damage is actually done.
	//
	int damager_id = -1;
	int damagee_id = -1;

	if (offense.Get_Owner() != NULL) {
		damager_id = offense.Get_Owner()->Get_Network_ID();
	}
	
	if (Get_Owner() != NULL) {
		damagee_id = Get_Owner()->Get_Network_ID();
	}

	if (offense.Get_Owner() != NULL && damagee_id != -1) {
		offense.Get_Owner()->Set_Last_Object_Id_I_Damaged(damagee_id);
	}

	if (Get_Owner() != NULL && damager_id != -1) {
		Get_Owner()->Set_Last_Object_Id_I_Got_Damaged_By(damager_id);
	}


	float damage = offense.Get_Damage() * scale;

	// Scale damage by difficulty if star is applying
	if ( IS_MISSION && offense.Get_Owner() == COMBAT_STAR ) {
		switch ( CombatManager::Get_Difficulty_Level() ) {
			case 0:	damage *= 2.0f;		break;
			case 1:	damage *= 1.333f;	break;
			case 2:	damage *= 1.0f;		break;
		};
	}

	float	shield_damage = 0;

	float damage_scale = 1;
	float shield_damage_scale = 1;
	if (alternate_skin != -1) {
		damage_scale = ArmorWarheadManager::Get_Damage_Multiplier( alternate_skin, offense.Get_Warhead() );
		shield_damage_scale = 0;
	} else {
		damage_scale = ArmorWarheadManager::Get_Damage_Multiplier( Skin, offense.Get_Warhead() );
		shield_damage_scale = ArmorWarheadManager::Get_Damage_Multiplier( ShieldType, offense.Get_Warhead() );
	}

	// check for punish = no more damage
	if ( smart != NULL && smart->Get_Player_Data() ) {
#define	PUNISH_DELAY	60
		if ( smart->Get_Player_Data()->Get_Punish_Timer() > PUNISH_DELAY ) {
			damage_scale = 0;
			shield_damage_scale = 0;
			smart->Get_Player_Data()->Set_Money( 0 );
			smart->Get_Player_Data()->Set_Score( 0 );
		}
	}


	bool	is_repair = false;

	// check for repair on either health of shield
	// Note: humans don't repair health, but vehicles do.
	if ( (damage * damage_scale < 0) || (damage * shield_damage_scale < 0) ) {	// We are repairing

		is_repair = true;

		Mark_Owner_Dirty();

		// Apply first to health, then to shield
		if ( Health < HealthMax && (damage_scale != 0) ) {
			damage *= damage_scale;
			float min_damage = (float)Health - (float)HealthMax;
			damage = WWMath::Clamp( damage, min_damage, 0 );
			//Debug_Say(( "Repairing %f health\n", damage ));
			Health = (float)Health - damage;
   	} else {
			damage *= shield_damage_scale;
			shield_damage = damage;
			damage = 0;
			float min_shield_damage = (float)ShieldStrength - (float)ShieldStrengthMax;
			shield_damage = WWMath::Clamp( shield_damage, min_shield_damage, 0 );
			//Debug_Say(( "Repairing %f shield\n", shield_damage ));
			ShieldStrength = (float)ShieldStrength - shield_damage;
		}

	} else {

		if (	smart && Get_Owner() && 
				(smart != Get_Owner() )  && 
				smart->Is_Teammate( Get_Owner() ) ) {
			// This is friendly fire!!
			if ( !CombatManager::Is_Friendly_Fire_Permitted() ) {
//				damage = 0;
				return Health;
			}
		}

		if ( damage != 0 ) {
			Mark_Owner_Dirty();
		}

		// if we have a shield, redirect a fraction of our damage;
		// If alternate skin (MCT) ignore sheild damage;
		if ( (float)ShieldStrength > 0.0f && alternate_skin == -1 ) {
			shield_damage = damage * ArmorWarheadManager::Get_Shield_Absorbsion( ShieldType, offense.Get_Warhead() );
			damage -= shield_damage;

			shield_damage *= shield_damage_scale;
			float shield_damage_to_apply = shield_damage;  // how much scaled damage to apply

			if ( shield_damage > (float)ShieldStrength ) {
				shield_damage = ShieldStrength;
			}
			ShieldStrength = (float)ShieldStrength - shield_damage;
			if ( shield_damage_scale != 0 ) {
				shield_damage /= shield_damage_scale;		// how much un scaled damage did we apply
				shield_damage_to_apply /= shield_damage_scale;  // how much scaled damage did we try to apply
 			}

			damage += shield_damage_to_apply - shield_damage;

			// Clamp Shield Strength
			ShieldStrength = WWMath::Clamp( (float)ShieldStrength, 0, (float)ShieldStrengthMax );
		}

		// scale the (remaining) damage
		// (gth) added alternate_skin feature which is used by buildings when their MCT is being damaged.
		damage *= damage_scale;

		if ( (float)Health < damage ) {
			damage = Health;
		}

	#ifdef WWDEBUG
		//
		// TSS090601
		//
		if (OneShotKills)	{
			damage = Health;
		}
	#endif // _WWDEBUG

		Health = (float)Health - damage;
		
		//
		//	Don't allow this object to die (if necessary)
		//
		if ( CanObjectDie == false ) {
			Health = max ( (float)Health, 1.0F );
		}
	}

	if ( COMBAT_STAR != NULL &&			// dedicated server test
		  Get_Owner() == COMBAT_STAR ) {
		Vector3 pos;
		Get_Owner()->Get_Position( &pos );
		int hitter_id = 0;
		const char * weapon_name = "";
		if ( offense.Get_Owner() ) {
			hitter_id = offense.Get_Owner()->Get_ID();
			if ( offense.Get_Owner()->Get_Weapon() ) {
				weapon_name = offense.Get_Owner()->Get_Weapon()->Get_Definition()->Get_Name();
			}
		}
		float points = damage;
		float armor = Get_Shield_Strength();
	   DIAG_LOG(( "DRCV", "%s; %d; %1.2f; %1.2f; %1.2f; %1.2f; %1.2f; %1.2f", weapon_name, hitter_id, points, armor, static_cast<float>(Health), pos.X, pos.Y, pos.Z ));
	}

	if (( smart == COMBAT_STAR ) && ( smart != NULL )) {
		Vector3 pos;
		smart->Get_Position( &pos );
		const char * weapon_name = "";
		int ammo = 0;
		if ( smart->Get_Weapon() ) {
			weapon_name = smart->Get_Weapon()->Get_Definition()->Get_Name();
			ammo = smart->Get_Weapon()->Get_Total_Rounds();
		}
		int hittee_id = 0;
		Vector3 victim_pos(0,0,0);
		const char * team_name = "";
		if ( Get_Owner() ) {
			hittee_id = Get_Owner()->Get_ID();
			Get_Owner()->Get_Position( &victim_pos );
			team_name = Player_Type_Name( Get_Owner()->Get_Player_Type() );
		}
		float points = damage;
		float armor = Get_Shield_Strength();
	   DIAG_LOG(( "DEFC", "%1.2f; %1.2f; %1.2f; %s; %d; %d; %1.2f; %1.2f; %1.2f; %1.2f; %1.2f; %1.2f; %s ", pos.X, pos.Y, pos.Z, weapon_name, ammo, hittee_id, points, armor, static_cast<float>(Health), victim_pos.X, victim_pos.Y, victim_pos.Z, team_name ));
	}

	// Clamp Health to Max
	Health = WWMath::Clamp( (float)Health, 0, (float)HealthMax );

	if ( damage > 0 && (float)Health <= 0 ) {
		int victim_id = 0;
		Vector3 victim_pos(0,0,0);
		if ( Get_Owner() != NULL ) {
			victim_id = Get_Owner()->Get_ID();
			Get_Owner()->Get_Position( &victim_pos );
		}
		int killer_id = 0;
		Vector3 killer_pos(0,0,0);
		const char * weapon_name = "";
		if ( smart != NULL ) {
			killer_id = smart->Get_ID();
			smart->Get_Position( &killer_pos );
			if ( smart->Get_Weapon() ) {
				weapon_name = smart->Get_Weapon()->Get_Definition()->Get_Name();
			}
		}
		DIAG_LOG(( "OBDE", "%d; %1.2f; %1.2f; %1.2f; %d; %1.2f; %1.2f; %1.2f; %s", victim_id, victim_pos.X, victim_pos.Y, victim_pos.Z, killer_id, killer_pos.X, killer_pos.Y, killer_pos.Z, weapon_name ));
		if ( Get_Owner() == COMBAT_STAR ) {
			DIAG_LOG(( "STDE", "%1.2f; %1.2f; %1.2f; %d; %1.2f; %1.2f; %1.2f; %s", victim_pos.X, victim_pos.Y, victim_pos.Z, killer_id, killer_pos.X, killer_pos.Y, killer_pos.Z, weapon_name ));
		}
	}

	if (damage > 0 && 
		 (float) Health <= 0 &&
		 smart != NULL && 
		 smart->As_SoldierGameObj() != NULL && 
		 Get_Owner() != NULL && 
		 Get_Owner()->As_SoldierGameObj() != NULL) {

		CombatManager::On_Soldier_Kill(smart->As_SoldierGameObj(), Get_Owner()->As_SoldierGameObj());
	}

	// Apply Points for damage/death
	if ( smart != NULL && smart->Get_Player_Data() ) {

		float points_dir = 1;

		// If same team, make the points negative

		if ( Get_Owner() ) {
			if ( smart->Is_Teammate( Get_Owner() ) ) {
				points_dir = -1;
			} else if ( !smart->Is_Enemy( Get_Owner() ) ) {
				// No points for neutrals
				points_dir = 0;
			}
		}

		if (offense.Get_Owner() != Get_Owner()) {
			float points = (float)DamagePoints;
			if ( is_repair ) {	// half points for repair
				points *= 0.5f;
			}

			smart->Get_Player_Data()->Apply_Damage_Points( points_dir * (damage + shield_damage) * points, Get_Owner() );
		}

		if ( damage > 0 && (float)Health <= 0 ) {
			
			if (offense.Get_Owner() != Get_Owner()) {
				smart->Get_Player_Data()->Apply_Death_Points( points_dir * (float)DeathPoints, Get_Owner() );
			}

		}
	}

	return Health;
}

bool DefenseObjectClass::Is_Repair( const OffenseObjectClass	& offense, float scale )
{
	float damage = offense.Get_Damage() * scale;
	float damage_scale = ArmorWarheadManager::Get_Damage_Multiplier( Skin, offense.Get_Warhead() );
	float shield_damage_scale = ArmorWarheadManager::Get_Damage_Multiplier( ShieldType, offense.Get_Warhead() );

	// check for repair on either health of shield
	return ( (damage * damage_scale < 0) || (damage * shield_damage_scale < 0) );
}

bool DefenseObjectClass::Would_Damage( const OffenseObjectClass	& offense, float scale )
{
	float damage = offense.Get_Damage() * scale;
	float damage_scale = ArmorWarheadManager::Get_Damage_Multiplier( Skin, offense.Get_Warhead() );
	float shield_damage_scale = ArmorWarheadManager::Get_Damage_Multiplier( ShieldType, offense.Get_Warhead() );

	SmartGameObj * smart = NULL;
	if ( offense.Get_Owner() != NULL ) {
		smart = offense.Get_Owner()->As_SmartGameObj();
	}
	if (	smart && Get_Owner() && 
			smart->Is_Teammate( Get_Owner() ) && 
			(smart != Get_Owner() ) ) {
		// This is friendly fire!!
		if ( !CombatManager::Is_Friendly_Fire_Permitted() ) {
   			return false;
   		}
   	}

	if ( damage * damage_scale > 0 && (float)Health > 0 ) {
		return true;
	}

	if ( damage * shield_damage_scale > 0 && (float)ShieldStrength > 0 ) {
		return true;
	}

	return false;
}

void DefenseObjectClass::Import(BitStreamClass & packet)
{
	bool is_health_zero = packet.Get(is_health_zero);

	int health				= packet.Get(health, BITPACK_HEALTH);
	int shield_strength	= packet.Get(shield_strength, BITPACK_SHIELD_STRENGTH);
	unsigned int shield_type;
	packet.Get(shield_type, BITPACK_SHIELD_TYPE);

	ShieldType			= shield_type;
	Health				= (float)health;
	ShieldStrength		= (float)shield_strength;

	if (is_health_zero) {
		Health = 0;
	} else {
		Health = WWMath::Max(Health, 0.01f);
	}

	//WWASSERT(WWMath::Is_Valid_Float(ShieldStrength));
   //WWASSERT(packet.Is_Flushed());
}

void DefenseObjectClass::Export(BitStreamClass & packet)
{
	//
	// N.B. The funky syntax is to prevent datasafe asserts
	//
	int health				= cMathUtil::Round((double) (float)Health);
	int shield_strength	= cMathUtil::Round((double) (float)ShieldStrength);

	//TSS012202
	//packet.Add((bool)(health == 0));
	packet.Add((bool)(((float)Health) == 0));

	packet.Add(health,							BITPACK_HEALTH);
	packet.Add(shield_strength,				BITPACK_SHIELD_STRENGTH);
	packet.Add((unsigned long)ShieldType,	BITPACK_SHIELD_TYPE);

   //LastSentHealth = Health;
   //LastSentSkin = Skin;
   //LastSentShieldStrength = ShieldStrength;
   //LastSentShieldType = ShieldType;
}

/*
//
// This was only valid with 1 client. We would need dirty tests for every client.
//
bool DefenseObjectClass::Is_Defense_State_Dirty(void)
{
   return (
      fabs(Health - LastSentHealth) > MISCUTIL_EPSILON ||
      Skin != LastSentSkin ||
      fabs(ShieldStrength - LastSentShieldStrength) > MISCUTIL_EPSILON ||
      ShieldType != LastSentShieldType);
}
*/


/*****************************************************************************************
**
** DefenseObjectDefClass Implementation
**
*****************************************************************************************/

enum
{
	DEFENSEOBJECTDEF_CHUNK_VARIABLES			= 7311607,

	DEFENSEOBJECTDEF_VARIABLE_HEALTH			= 0x00,
	DEFENSEOBJECTDEF_VARIABLE_HEALTHMAX,
	DEFENSEOBJECTDEF_VARIABLE_SKIN,
	DEFENSEOBJECTDEF_VARIABLE_SHIELDSTRENGTH,
	DEFENSEOBJECTDEF_VARIABLE_SHIELDSTRENGTHMAX,
	DEFENSEOBJECTDEF_VARIABLE_SHIELDTYPE,
	DEFENSEOBJECTDEF_VARIABLE_DAMAGE_POINTS,
	DEFENSEOBJECTDEF_VARIABLE_DEATH_POINTS,
};


DefenseObjectDefClass::DefenseObjectDefClass(void) :
	Health( 100.0f ),
	HealthMax( 100.0f ),
	Skin( 0 ),
 	ShieldStrength( 0 ),
	ShieldStrengthMax( 0 ),
	ShieldType( 0 ),
	DamagePoints( 0 ),
	DeathPoints( 0 )
{
}

DefenseObjectDefClass::~DefenseObjectDefClass(void)
{
}

bool DefenseObjectDefClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(DEFENSEOBJECTDEF_CHUNK_VARIABLES);
	WRITE_SAFE_MICRO_CHUNK(csave,DEFENSEOBJECTDEF_VARIABLE_HEALTH, Health, float);
	WRITE_SAFE_MICRO_CHUNK(csave,DEFENSEOBJECTDEF_VARIABLE_HEALTHMAX,HealthMax, float);
  	int skin_save_id = ArmorWarheadManager::Get_Armor_Save_ID( Skin );
	WRITE_MICRO_CHUNK(csave,DEFENSEOBJECTDEF_VARIABLE_SKIN,skin_save_id);
	WRITE_SAFE_MICRO_CHUNK(csave,DEFENSEOBJECTDEF_VARIABLE_SHIELDSTRENGTH, ShieldStrength, float);
	WRITE_SAFE_MICRO_CHUNK(csave,DEFENSEOBJECTDEF_VARIABLE_SHIELDSTRENGTHMAX, ShieldStrengthMax, float);
  	int shield_save_id = ArmorWarheadManager::Get_Armor_Save_ID( ShieldType );
	WRITE_MICRO_CHUNK(csave,DEFENSEOBJECTDEF_VARIABLE_SHIELDTYPE,shield_save_id);
	WRITE_SAFE_MICRO_CHUNK(csave,DEFENSEOBJECTDEF_VARIABLE_DAMAGE_POINTS,DamagePoints, float);
	WRITE_SAFE_MICRO_CHUNK(csave,DEFENSEOBJECTDEF_VARIABLE_DEATH_POINTS,DeathPoints, float);
	csave.End_Chunk();
	return true;
}

bool DefenseObjectDefClass::Load(ChunkLoadClass &cload)
{
	int skin_save_id = -2;
	int shield_save_id = -2;
	while (cload.Open_Chunk()) {

		switch(cload.Cur_Chunk_ID())
		{
			case DEFENSEOBJECTDEF_CHUNK_VARIABLES:

				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_SAFE_MICRO_CHUNK(cload,DEFENSEOBJECTDEF_VARIABLE_HEALTH,Health,float);
						READ_SAFE_MICRO_CHUNK(cload,DEFENSEOBJECTDEF_VARIABLE_HEALTHMAX,HealthMax,float);
						READ_MICRO_CHUNK(cload,DEFENSEOBJECTDEF_VARIABLE_SKIN,skin_save_id);
						READ_SAFE_MICRO_CHUNK(cload,DEFENSEOBJECTDEF_VARIABLE_SHIELDSTRENGTH,ShieldStrength,float);
						READ_SAFE_MICRO_CHUNK(cload,DEFENSEOBJECTDEF_VARIABLE_SHIELDSTRENGTHMAX,ShieldStrengthMax,float);
						READ_MICRO_CHUNK(cload,DEFENSEOBJECTDEF_VARIABLE_SHIELDTYPE,shield_save_id);
						READ_SAFE_MICRO_CHUNK(cload,DEFENSEOBJECTDEF_VARIABLE_DAMAGE_POINTS,DamagePoints,float);
						READ_SAFE_MICRO_CHUNK(cload,DEFENSEOBJECTDEF_VARIABLE_DEATH_POINTS,DeathPoints,float);
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
		};

		cload.Close_Chunk();
	}
	Skin = ArmorWarheadManager::Find_Armor_Save_ID( skin_save_id );
	ShieldType = ArmorWarheadManager::Find_Armor_Save_ID( shield_save_id );
	return true;
}



void DefenseObjectClass::Set_Health(float health)
{
	float old_health = Health;
	Health = WWMath::Clamp(health, 0, HealthMax);
	if ( old_health != (float)Health ) {
		Mark_Owner_Dirty();
	}
}

void DefenseObjectClass::Add_Health(float add_health)
{
	Set_Health( WWMath::Clamp((float)Health + add_health, 0, HealthMax) );
}

float DefenseObjectClass::Get_Health(void) const
{
	return Health;
}

void DefenseObjectClass::Set_Health_Max(float health)
{
	HealthMax = WWMath::Clamp(health, 0, MAX_MAX_HEALTH);
	Mark_Owner_Dirty();
}

float DefenseObjectClass::Get_Health_Max(void) const
{
	return HealthMax;
}



void DefenseObjectClass::Set_Shield_Strength(float str)
{
	float old = ShieldStrength;
	ShieldStrength = WWMath::Clamp(str, 0, ShieldStrengthMax);
	if ( old != (float)ShieldStrength ) {
		Mark_Owner_Dirty();
	}
}

void DefenseObjectClass::Add_Shield_Strength(float str)
{
	Set_Shield_Strength( WWMath::Clamp((float)ShieldStrength + str, 0, ShieldStrengthMax) );
}

float DefenseObjectClass::Get_Shield_Strength(void) const
{
	return ShieldStrength;
}

void DefenseObjectClass::Set_Shield_Strength_Max(float str)
{
	ShieldStrengthMax = WWMath::Clamp(str, 0, MAX_MAX_SHIELD_STRENGTH);
	Mark_Owner_Dirty();
}

float DefenseObjectClass::Get_Shield_Strength_Max(void) const
{
	return ShieldStrengthMax;
}

void DefenseObjectClass::Set_Precision(void)
{
	//
	// This static function needs to be called after ArmorWarheadManager::Init
	// has done its work.
	//

	cEncoderList::Set_Precision(BITPACK_HEALTH, 0, (int) MAX_MAX_HEALTH);
	cEncoderList::Set_Precision(BITPACK_SHIELD_STRENGTH, 0, (int) MAX_MAX_SHIELD_STRENGTH);
	cEncoderList::Set_Precision(BITPACK_SHIELD_TYPE, 0,
		//ArmorWarheadManager::Get_Num_Armor_Types(), 1);
		ArmorWarheadManager::Get_Num_Armor_Types());
}

/*
**
*/
void	DefenseObjectClass::Mark_Owner_Dirty( void ) 
{
	if ( Get_Owner() != NULL ) {
		Get_Owner()->Set_Object_Dirty_Bit( NetworkObjectClass::BIT_OCCASIONAL, true );
	}
}


/*
**
*/
void	DefenseObjectClass::Set_Shield_Type( ArmorType type )	
{ 
	ShieldType = type; 
	Mark_Owner_Dirty();
}


























			//
			// Do extra stuff when somebody kills somebody
			//
			/*
			if ( Get_Owner() != NULL &&
				  Get_Owner()->As_SmartGameObj() &&
				  Get_Owner()->As_SmartGameObj()->Has_Player() ) {

				int victim_id = Get_Owner()->As_SmartGameObj()->Get_Control_Owner();

				smart->Get_Player_Data()->On_Kill(victim_id);
			}
			*/
			/*
			if ( Get_Owner() != NULL &&
				  Get_Owner()->As_DamageableGameObj() != NULL && 
				  Get_Owner()->As_DamageableGameObj()->As_PhysicalGameObj() != NULL && 
				  Get_Owner()->As_DamageableGameObj()->As_PhysicalGameObj()->As_SoldierGameObj() != NULL &&
				  Get_Owner()->As_SmartGameObj() != smart) {

				int victim_id = Get_Owner()->As_SmartGameObj()->Get_Control_Owner();
				int victim_team = Get_Owner()->As_SmartGameObj()->Get_Player_Type();

				smart->Get_Player_Data()->On_Kill(victim_id, victim_team);
			}
			*/
			/*
			if ( Get_Owner() != NULL &&
				  Get_Owner()->As_SoldierGameObj != NULL && 
				  Get_Owner()->As_SmartGameObj() != smart) {

				WWASSERT(Get_Owner()->As_SmartGameObj() != NULL);
				int victim_id = Get_Owner()->As_SmartGameObj()->Get_Control_Owner();
				int victim_team = Get_Owner()->As_SmartGameObj()->Get_Player_Type();

				smart->Get_Player_Data()->On_Kill(victim_id, victim_team);
			}
			*/

