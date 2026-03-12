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
 *                     $Archive:: /Commando/Code/Combat/damageablegameobj.cpp                 $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 12/18/01 3:02p                                              $*
 *                                                                                             *
 *                    $Revision:: 19                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "damageablegameobj.h"
#include "debug.h"
#include "armedgameobj.h"
#include "playertype.h"
#include "colors.h"

/*
** DamageableGameObjDef - Defintion class for a DamageableGameObj
*/
DamageableGameObjDef::DamageableGameObjDef( void ) :
	TranslatedNameID( 0 ),
	EncyclopediaType( EncyclopediaMgrClass::TYPE_UNKNOWN ),
	EncyclopediaID( 0 ),
	NotTargetable( false ),
	DefaultPlayerType( PLAYERTYPE_NEUTRAL )
{
	DEFENSEOBJECTDEF_EDITABLE_PARAMS( DamageableGameObjDef, DefenseObjectDef );
	EDITABLE_PARAM( DamageableGameObjDef, ParameterClass::TYPE_STRINGSDB_ID,	TranslatedNameID );
	FILENAME_PARAM( DamageableGameObjDef, InfoIconTextureFilename, "InfoIconTextureFilename", ".TGA" );

#ifdef	PARAM_EDITING_ON
	EnumParameterClass *param = new EnumParameterClass( (int *)&EncyclopediaType );
	param->Set_Name( "Encyclopedia Type" );
	param->Add_Value( "<NA>",			0 );
	param->Add_Value( "Character",	EncyclopediaMgrClass::TYPE_CHARACTER );
	param->Add_Value( "Weapon",		EncyclopediaMgrClass::TYPE_WEAPON );
	param->Add_Value( "Vehicle",		EncyclopediaMgrClass::TYPE_VEHICLE );
	param->Add_Value( "Building",		EncyclopediaMgrClass::TYPE_BUILDING );
	GENERIC_EDITABLE_PARAM( DamageableGameObjDef, param )

	param = new EnumParameterClass( &DefaultPlayerType );
	param->Set_Name ("PlayerType");
	param->Add_Value ( "Mutant",		PLAYERTYPE_MUTANT );
	param->Add_Value ( "Unteamed",	PLAYERTYPE_NEUTRAL );
	param->Add_Value ( "Renegade",	PLAYERTYPE_RENEGADE );
	param->Add_Value ( "Nod",			PLAYERTYPE_NOD );
	param->Add_Value ( "GDI",			PLAYERTYPE_GDI );
	GENERIC_EDITABLE_PARAM(DamageableGameObjDef,param)
#endif

	EDITABLE_PARAM( DamageableGameObjDef, ParameterClass::TYPE_INT,	EncyclopediaID );
	EDITABLE_PARAM( DamageableGameObjDef, ParameterClass::TYPE_BOOL,	NotTargetable );
	return ;
}

enum	{
	CHUNKID_DEF_PARENT								=	207011205,
	CHUNKID_DEF_VARIABLES,
	CHUNKID_DEF_DEFENSEOBJECTDEF,

	MICROCHUNKID_DEF_TRANSLATED_NAME_ID			=	1,
	MICROCHUNKID_DEF_INFO_ICON_TEXTURE_FILENAME,
	MICROCHUNKID_DEF_ENCY_TYPE,
	MICROCHUNKID_DEF_ENCY_ID,
	MICROCHUNKID_DEF_NOT_TARGETABLE,
	MICROCHUNKID_DEF_DEFAULT_PLAYER_TYPE,
};

bool	DamageableGameObjDef::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_DEF_PARENT );
		ScriptableGameObjDef::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_DEF_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_TRANSLATED_NAME_ID, TranslatedNameID );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_INFO_ICON_TEXTURE_FILENAME, InfoIconTextureFilename );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_ENCY_TYPE, EncyclopediaType );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_ENCY_ID,	EncyclopediaID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_NOT_TARGETABLE,	NotTargetable );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_DEFAULT_PLAYER_TYPE, DefaultPlayerType );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_DEF_DEFENSEOBJECTDEF );
		DefenseObjectDef.Save(csave);
	csave.End_Chunk();

	return true;
}

bool	DamageableGameObjDef::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_DEF_PARENT:
				ScriptableGameObjDef::Load( cload );
				break;

			case CHUNKID_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_TRANSLATED_NAME_ID, TranslatedNameID );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_DEF_INFO_ICON_TEXTURE_FILENAME, InfoIconTextureFilename );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_ENCY_TYPE,	EncyclopediaType );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_ENCY_ID,		EncyclopediaID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_NOT_TARGETABLE,	NotTargetable );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_DEFAULT_PLAYER_TYPE, DefaultPlayerType );

						default:
							Debug_Say(( "Unhandled MicroChunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
							break;

					}
					cload.Close_Micro_Chunk();
				}
				break;

			case CHUNKID_DEF_DEFENSEOBJECTDEF:
				DefenseObjectDef.Load(cload);
				break;

			default:
				Debug_Say(( "Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}
		cload.Close_Chunk();
	}
	return true;
}

/*
** DamageableGameObj
*/
DamageableGameObj::DamageableGameObj( void ) :
	IsHealthBarDisplayed( true )
{
	Set_Player_Type(PLAYERTYPE_NEUTRAL);
}

DamageableGameObj::~DamageableGameObj( void )
{
	Remove_All_Observers();
}



/*
**
*/
void	DamageableGameObj::Init( const DamageableGameObjDef & definition )
{
	ScriptableGameObj::Init( definition );
	Copy_Settings( definition );
	return ;
}

/*
**
*/
void	DamageableGameObj::Copy_Settings( const DamageableGameObjDef & definition )
{
	Set_Player_Type(definition.DefaultPlayerType);
	DefenseObject.Init(definition.DefenseObjectDef, this );
	return ;
}

/*
**
*/
void	DamageableGameObj::Re_Init( const DamageableGameObjDef & definition )
{
	int old_player_type	= PlayerType;

	//
	//	Record the health and shield percent so we can restore the
	// appropriate amount of health and shield after we've re-initialized
	//
	//float health_percent	= DefenseObject.Get_Health() / DefenseObject.Get_Health_Max();
	//float shield_percent	= DefenseObject.Get_Shield_Strength() / DefenseObject.Get_Shield_Strength_Max();

	//
	//	Re-initialize the base class
	//
	ScriptableGameObj::Re_Init( definition );

	//
	//	Copy any internal settings from the definition
	//
	Copy_Settings( definition );

	//
	//	Reset the health and shield to appropriate values
	//
	//DefenseObject.Set_Health (DefenseObject.Get_Health_Max () * health_percent);
	//DefenseObject.Set_Shield_Strength (DefenseObject.Get_Shield_Strength_Max () * shield_percent);
	Set_Player_Type( old_player_type );
	return ;
}

const DamageableGameObjDef & DamageableGameObj::Get_Definition( void ) const
{
	return (const DamageableGameObjDef &)BaseGameObj::Get_Definition();
}

/*
** DamageableGameObj Save and Load
*/
enum	{
	CHUNKID_PARENT							=	207011212,
	CHUNKID_DEFENSEOBJECT,
	CHUNKID_VARIABLES,

	MICROCHUNKID_PLAYER_TYPE				=	1,
	MICROCHUNKID_IS_HEALTH_BAR_DISPLAYED,
};

bool	DamageableGameObj::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_PARENT );
		ScriptableGameObj::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_PLAYER_TYPE, PlayerType );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_IS_HEALTH_BAR_DISPLAYED, IsHealthBarDisplayed );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_DEFENSEOBJECT );
		DefenseObject.Save(csave);
	csave.End_Chunk();

	return true;
}

bool	DamageableGameObj::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PARENT:
				ScriptableGameObj::Load( cload );
				break;

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_PLAYER_TYPE, PlayerType );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_IS_HEALTH_BAR_DISPLAYED, IsHealthBarDisplayed );
						default:
							Debug_Say(( "Unhandled MicroChunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
							break;

					}
					cload.Close_Micro_Chunk();
				}
				break;

			case CHUNKID_DEFENSEOBJECT:
				DefenseObject.Load( cload );
				break;

			default:
				Debug_Say(( "Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

void	DamageableGameObj::Apply_Damage( const OffenseObjectClass & damager, float scale, int alternate_skin )
{
	if ( DefenseObject.Get_Health() <= 0 ) {
		return;
	}

	if (Is_Delete_Pending()) {
		return;
	}

	float old_health = DefenseObject.Get_Health();
	float old_shield = DefenseObject.Get_Shield_Strength();
	DefenseObject.Apply_Damage( damager, scale, alternate_skin );
	float new_health = DefenseObject.Get_Health();
	float new_shield = DefenseObject.Get_Shield_Strength();

	float diff = old_health + old_shield - new_health - new_shield;

	// Notify the observers
	const GameObjObserverList & observer_list = Get_Observers();

	for( int index = 0; index < observer_list.Count(); index++ ) {
		observer_list[ index ]->Damaged( this, damager.Get_Owner(), diff );
	}

	if ( DefenseObject.Get_Health() <= 0 ) {
		// notify the observers
		for( int index = 0; index < observer_list.Count(); index++ ) {
			observer_list[ index ]->Killed( this, damager.Get_Owner() );
		}

		Completely_Damaged( damager );
	}
}

//------------------------------------------------------------------------------------
/*void	DamageableGameObj::Get_Information( StringClass & string )
{
	// If we just came from the editor, call created on all out observers
	const GameObjObserverList & observer_list = Get_Observers();
	for( int index = 0; index < observer_list.Count(); index++ ) {
		StringClass temp;
		temp.Format( "%s\n", observer_list[ index ]->Get_Name() );
		string += temp;
	}
} */


/*
**
*/
void	DamageableGameObj::Export_Occasional( BitStreamClass &packet )
{
	ScriptableGameObj::Export_Occasional( packet );

	//
	//	Export the defense object's state
	//
	DefenseObject.Export (packet);
}


/*
**
*/
void	DamageableGameObj::Import_Occasional( BitStreamClass &packet )
{
	ScriptableGameObj::Import_Occasional( packet );

	//
	//	Update the defense object's state
	//
	float old_health = DefenseObject.Get_Health();
	DefenseObject.Import (packet);
	float new_health = DefenseObject.Get_Health();

	/*
	//
	// Hack !
	//
	if ( Is_Delete_Pending() ) {
		if ( new_health != 0 ) {
			DefenseObject.Set_Health( 0 );
			new_health = DefenseObject.Get_Health();
		}
	} else {
		if ( new_health == 0 ) {
			DefenseObject.Set_Health( 0.01f );
			new_health = DefenseObject.Get_Health();
		}
	}
	*/

	if (old_health > 0 && old_health > new_health) {
		// Notify the observers that we are damaged
		const GameObjObserverList& observer_list = Get_Observers();
		int count = observer_list.Count();

		for (int index = 0; index < count; ++index) {
			observer_list[index]->Damaged(this, NULL, old_health - new_health);
		}
	}

	//
	//	Check to see if the object is completely damaged
	//
	if ( old_health > 0 && new_health <= 0 ) {
		// Notify the observers that the building has been destroyed
		const GameObjObserverList& observer_list = Get_Observers();
		int count = observer_list.Count();

		for (int index = 0; index < count; ++index) {
			observer_list[index]->Killed(this, NULL);
		}

		OffenseObjectClass dummy_offense_obj;
		Completely_Damaged( dummy_offense_obj );
	}
}


//-----------------------------------------------------------------------------
bool DamageableGameObj::Is_Team_Player(void)
{
	return PlayerType == PLAYERTYPE_NOD || PlayerType == PLAYERTYPE_GDI;
}

//-----------------------------------------------------------------------------
Vector3 DamageableGameObj::Get_Team_Color(void)
{
	return Get_Color_For_Team(PlayerType);
}

//-----------------------------------------------------------------------------
void DamageableGameObj::Set_Player_Type(int id)
{
	PlayerType = id;

	Set_Object_Dirty_Bit( NetworkObjectClass::BIT_RARE, true );
}

//-----------------------------------------------------------------------------
bool DamageableGameObj::Is_Teammate(DamageableGameObj * p_obj)
{
	WWASSERT(p_obj != NULL);

   return ((p_obj == this) ||
			  (Is_Team_Player() && Get_Player_Type() == p_obj->Get_Player_Type()));
}

bool DamageableGameObj::Is_Enemy(DamageableGameObj * p_obj)
{
	WWASSERT(p_obj != NULL);
   return ( (p_obj != this) && Player_Types_Are_Enemies( Get_Player_Type(), p_obj->Get_Player_Type() ) );
}


