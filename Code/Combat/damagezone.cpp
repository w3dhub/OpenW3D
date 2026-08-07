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
 *                     $Archive:: /Commando/Code/Combat/damagezone.cpp                        $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/16/01 12:14p                                              $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "damagezone.h"
#include "persistfactory.h"
#include "combatchunkid.h"
#include "simpledefinitionfactory.h"
#include "debug.h"
#include "wwhack.h"
#include "gameobjmanager.h"
#include "smartgameobj.h"
#include "colmath.h"
#include "weapons.h"
#include "damage.h"
#include "wwprofile.h"
#include "combat.h"
#include "crandom.h"

DECLARE_FORCE_LINK( DamageZone )

SimplePersistFactoryClass<DamageZoneGameObjDef, CHUNKID_GAME_OBJECT_DEF_DAMAGE_ZONE>	_DamageZoneGameObjDefPersistFactory;
DECLARE_DEFINITION_FACTORY(DamageZoneGameObjDef, CLASSID_GAME_OBJECT_DEF_DAMAGE_ZONE, "Damage Zone") _DamageZoneGameObjDefDefFactory;

DamageZoneGameObjDef::DamageZoneGameObjDef( void ) :
	DamageRate( 10 ),
	DamageWarhead( 1 ),
	Color( 0.7F, 0, 0 )
{
#ifdef PARAM_EDITING_ON
	EDITABLE_PARAM( DamageZoneGameObjDef, ParameterClass::TYPE_FLOAT, DamageRate );

	EnumParameterClass *param;
	param = new EnumParameterClass( &DamageWarhead );
	param->Set_Name ( "Damage Warhead" );
	for ( int i = 0; i < ArmorWarheadManager::Get_Num_Warhead_Types(); i++ ) {
		param->Add_Value ( ArmorWarheadManager::Get_Warhead_Name( i ), i );
	}
	GENERIC_EDITABLE_PARAM(DamageZoneGameObjDef,param)

	EDITABLE_PARAM( DamageZoneGameObjDef, ParameterClass::TYPE_COLOR, Color );
#endif	//PARAM_EDITING_ON
}

uint32	DamageZoneGameObjDef::Get_Class_ID (void) const
{
	return CLASSID_GAME_OBJECT_DEF_DAMAGE_ZONE;
}

PersistClass *	DamageZoneGameObjDef::Create( void ) const
{
	DamageZoneGameObj * zone = new DamageZoneGameObj;
	zone->Init( *this );
	return zone;
}

enum	{
	CHUNKID_DEF_PARENT							=	626000947,
	CHUNKID_DEF_VARIABLES,

	XXXMICROCHUNKID_DEF_DAMAGE_TYPE				=	1,
	MICROCHUNKID_DEF_ZONE_COLOR,
	MICROCHUNKID_DEF_DAMAGE_RATE,
	MICROCHUNKID_DEF_DAMAGE_WARHEAD,
};

bool	DamageZoneGameObjDef::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_DEF_PARENT );
		BaseGameObjDef::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_DEF_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_ZONE_COLOR, Color );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_DAMAGE_RATE, DamageRate );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_DAMAGE_WARHEAD, DamageWarhead );
	csave.End_Chunk();

	return true;
}

bool	DamageZoneGameObjDef::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_DEF_PARENT:
				BaseGameObjDef::Load( cload );
				break;

			case CHUNKID_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_ZONE_COLOR, Color );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_DAMAGE_RATE, DamageRate );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_DAMAGE_WARHEAD, DamageWarhead );
						default:
							Debug_Say(("Unhandled Variable Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
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

const PersistFactoryClass & DamageZoneGameObjDef::Get_Factory (void) const
{
	return _DamageZoneGameObjDefPersistFactory;
}

/*
**
*/
DamageZoneGameObj::DamageZoneGameObj( void ) :
	DamageTimer( 0 )
{
}

DamageZoneGameObj::~DamageZoneGameObj( void )
{
}

void DamageZoneGameObj::Init( void )
{
	Init( Get_Definition() );
}

void	DamageZoneGameObj::Init( const DamageZoneGameObjDef & definition )
{
	BaseGameObj::Init( definition );
}

const DamageZoneGameObjDef & DamageZoneGameObj::Get_Definition( void ) const
{
	return (const DamageZoneGameObjDef &)BaseGameObj::Get_Definition();
}

enum	{
	CHUNKID_PARENT							=	626000947,

	CHUNKID_VARIABLES,
	MICROCHUNKID_BOUNDING_BOX			=	1,
	MICROCHUNKID_DAMAGE_TIMER,
};

bool	DamageZoneGameObj::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_PARENT );
		BaseGameObj::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_BOUNDING_BOX, BoundingBox );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DAMAGE_TIMER, DamageTimer );
	csave.End_Chunk();

	return true;
}

bool	DamageZoneGameObj::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PARENT:
				BaseGameObj::Load( cload );
				break;

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_BOUNDING_BOX, BoundingBox );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DAMAGE_TIMER, DamageTimer );
						default:
							Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
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

SimplePersistFactoryClass<DamageZoneGameObj, CHUNKID_GAME_OBJECT_DAMAGE_ZONE>	_DamageZonePersistFactory;

const PersistFactoryClass & DamageZoneGameObj::Get_Factory( void ) const
{
	return _DamageZonePersistFactory;
}

void	DamageZoneGameObj::Think( void )
{
	WWPROFILE( "DamageZone Think" );

	if ( !CombatManager::I_Am_Server() ) {
		return;
	}

	// Only apply damage every second...
	DamageTimer += TimeManager::Get_Frame_Seconds();

	// Scramble it a bit so all don't hit at the same time...
	float trigger = 1.0f;
	trigger = FreeRandom.Get_Float( 1.0f, 1.5f );
	if ( DamageTimer >= trigger ) {
//		Debug_Say(( "Zone Damage %f\n", DamageTimer ));
		OffenseObjectClass offense( Get_Definition().DamageRate, Get_Definition().DamageWarhead );

		SLNode<SmartGameObj> * smart_objnode;
		for (smart_objnode = GameObjManager::Get_Smart_Game_Obj_List()->Head(); smart_objnode; smart_objnode = smart_objnode->Next()) {
			SmartGameObj * obj = smart_objnode->Data();
			WWASSERT( obj != nullptr );

			Vector3 pos;
			obj->Get_Position(&pos);
			if ( CollisionMath::Overlap_Test(BoundingBox,pos) == CollisionMath::INSIDE ) {
				obj->Apply_Damage_Extended( offense, DamageTimer );
			}
  		}

		DamageTimer = 0;
	}

}
