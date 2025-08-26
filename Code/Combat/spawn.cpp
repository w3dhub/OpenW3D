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
 *                     $Archive:: /Commando/Code/Combat/spawn.cpp                             $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 11/19/01 10:25a                                             $*
 *                                                                                             *
 *                    $Revision:: 107                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "spawn.h"
#include "wwhack.h"
#include "persistfactory.h"
#include "combatchunkid.h"
#include "simpledefinitionfactory.h"
#include "debug.h"
#include "definitionmgr.h"
#include "soldier.h"
#include "combat.h"
#include "playertype.h"
#include "gameobjmanager.h"
#include "soldierobserver.h"
#include "crandom.h"
#include "WWAudio.h"
#include "assets.h"
#include "objlibrary.h"
#include "movephys.h"
#include "scripts.h"
#include "wwmemlog.h"
#include "gametype.h"
#include "combatmaterialeffectmanager.h"
#include "transitioneffect.h"
#include "humanphys.h"


//
// Class statics
//
bool	_Allow_Killing_Hibernating_Spawn = true;

#define	AUTO_SPAWN_CHECK_DELAY	2.0f


/*
** SpawnDefClass
*/
DECLARE_FORCE_LINK( Spawner )

SimplePersistFactoryClass<SpawnerDefClass, CHUNKID_SPAWNER_DEF>	_SpawnerDefPersistFactory;

DECLARE_DEFINITION_FACTORY(SpawnerDefClass, CLASSID_SPAWNER_DEF, "Spawner") _SpawnerDefDefFactory;


SpawnerDefClass::SpawnerDefClass( void ) :
	PlayerType( PLAYERTYPE_NEUTRAL ),
	SpawnMax( -1 ),		// unlimited
	SpawnDelay( 10 ),
	SpawnDelayVariation( 0 ),
	IsPrimary( false ),
	IsSoldierStartup( false ),
	PostVisualSpawnDelay( 0 ),
	SpecialEffectsObjID( 0 ),
	GotoSpawnerPos( false ),
	GotoSpawnerPosPriority( 30 ),
	TeleportFirstSpawn( true ),
	StartsDisabled( false ),
	KillHibernatingSpawn( true ),
	ApplySpawnMaterialEffect( true ),
	IsMultiplayWeaponSpawner( false )
{
	DEFIDLIST_PARAM( SpawnerDefClass, SpawnDefinitionIDList, CLASSID_GAME_OBJECTS );

#ifdef	PARAM_EDITING_ON
	EnumParameterClass *param;
	param = new EnumParameterClass( (int*)&PlayerType );
	param->Set_Name ("PlayerType");
	param->Add_Value ( "Unteamed",	PLAYERTYPE_RENEGADE );
	param->Add_Value ( "Nod",			PLAYERTYPE_NOD );
	param->Add_Value ( "GDI",			PLAYERTYPE_GDI );

	GENERIC_EDITABLE_PARAM(SpawnerDefClass,param)
#endif

	EDITABLE_PARAM( SpawnerDefClass, ParameterClass::TYPE_INT, SpawnMax );
	EDITABLE_PARAM( SpawnerDefClass, ParameterClass::TYPE_FLOAT, SpawnDelay );
	EDITABLE_PARAM( SpawnerDefClass, ParameterClass::TYPE_FLOAT, SpawnDelayVariation );
	EDITABLE_PARAM( SpawnerDefClass, ParameterClass::TYPE_BOOL, IsPrimary );
	EDITABLE_PARAM( SpawnerDefClass, ParameterClass::TYPE_BOOL, IsSoldierStartup );
	EDITABLE_PARAM( SpawnerDefClass, ParameterClass::TYPE_BOOL, GotoSpawnerPos );
	EDITABLE_PARAM( SpawnerDefClass, ParameterClass::TYPE_FLOAT, GotoSpawnerPosPriority );
	EDITABLE_PARAM( SpawnerDefClass, ParameterClass::TYPE_BOOL, TeleportFirstSpawn );
	EDITABLE_PARAM( SpawnerDefClass, ParameterClass::TYPE_BOOL, StartsDisabled );
	EDITABLE_PARAM( SpawnerDefClass, ParameterClass::TYPE_BOOL, KillHibernatingSpawn );
	EDITABLE_PARAM( SpawnerDefClass, ParameterClass::TYPE_BOOL, ApplySpawnMaterialEffect );
	EDITABLE_PARAM( SpawnerDefClass, ParameterClass::TYPE_BOOL, IsMultiplayWeaponSpawner );

	GENERIC_DEFID_PARAM( SpawnerDefClass, SpecialEffectsObjID, CLASSID_GAME_OBJECT_DEF_SPECIAL_EFFECTS);
	EDITABLE_PARAM( SpawnerDefClass, ParameterClass::TYPE_FLOAT, PostVisualSpawnDelay );

	SCRIPTLIST_PARAM (SpawnerDefClass, "Scripts", ScriptNameList, ScriptParameterList);
}

uint32	SpawnerDefClass::Get_Class_ID( void ) const
{
	return CLASSID_SPAWNER_DEF;
}


PersistClass *	SpawnerDefClass::Create( void ) const
{
	SpawnerClass * obj = new SpawnerClass;
	obj->Init( *this );
	return obj;
}

enum	{
	CHUNKID_DEF_PARENT							=	1013991542,
	CHUNKID_DEF_VARIABLES,

	MICROCHUNKID_DEF_DEFINITION_ID			=	1,
	XXXMICROCHUNKID_DEF_IS_COMMANDO_STARTING_POINT,
	MICROCHUNKID_DEF_PLAYER_TYPE,
	MICROCHUNKID_DEF_SPAWN_MAX,
	MICROCHUNKID_DEF_SPAWN_DELAY,
	XXXMICROCHUNKID_DEF_AUTO_SPAWN_RADIUS,
	XXXMICROCHUNKID_DEF_ONE_AT_A_TIME,
	MICROCHUNKID_DEF_SPAWN_DELAY_VARIATION,
	MICROCHUNKID_DEF_IS_PRIMARY,
	XXXMICROCHUNKID_DEF_EFFECT_MODEL_NAME,
	XXXMICROCHUNKID_DEF_EFFECT_ANIMATION_NAME,
	XXXMICROCHUNKID_DEF_SOUND_ID,
	MICROCHUNKID_DEF_POST_EFFECT_SPAWN_DELAY,
	MICROCHUNKID_DEF_SPECIAL_EFFECTS_OBJ_ID,
	MICROCHUNKID_DEF_IS_SOLDIER_STARTUP,
	MICROCHUNKID_DEF_GOTO_SPAWNER_POS,
	MICROCHUNKID_DEF_TELEPORT_FIRST_SPAWN,
	MICROCHUNKID_DEF_SCRIPT_NAME,
	MICROCHUNKID_DEF_SCRIPT_PARAMETERS,
	MICROCHUNKID_DEF_STARTS_DISABLED,
	MICROCHUNKID_DEF_KILL_HIBERNATING_SPAWN,
	MICROCHUNKID_DEF_GOTO_SPAWNER_POS_PRIORITY,
	MICROCHUNKID_DEF_APPLY_SPAWN_MATERIAL_EFFECT,
	MICROCHUNKID_DEF_IS_MULTIPLAY_WEAPON_SPAWNER
};

bool	SpawnerDefClass::Save( ChunkSaveClass &csave )
{
	int i;
	csave.Begin_Chunk( CHUNKID_DEF_PARENT );
		DefinitionClass::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_DEF_VARIABLES );
		for ( i = 0; i < SpawnDefinitionIDList.Count(); i++ ) {
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_DEFINITION_ID,	SpawnDefinitionIDList[i] );
		}
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_PLAYER_TYPE,			PlayerType );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_SPAWN_MAX,				SpawnMax );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_SPAWN_DELAY,			SpawnDelay );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_SPAWN_DELAY_VARIATION,	SpawnDelayVariation );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_IS_PRIMARY,			IsPrimary );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_IS_SOLDIER_STARTUP,	IsSoldierStartup );

		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_SPECIAL_EFFECTS_OBJ_ID,	SpecialEffectsObjID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_POST_EFFECT_SPAWN_DELAY,	PostVisualSpawnDelay );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_GOTO_SPAWNER_POS,			GotoSpawnerPos );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_GOTO_SPAWNER_POS_PRIORITY,GotoSpawnerPosPriority );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_TELEPORT_FIRST_SPAWN,		TeleportFirstSpawn );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_STARTS_DISABLED,				StartsDisabled );

		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_KILL_HIBERNATING_SPAWN,	KillHibernatingSpawn );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_APPLY_SPAWN_MATERIAL_EFFECT,	ApplySpawnMaterialEffect );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_IS_MULTIPLAY_WEAPON_SPAWNER,	IsMultiplayWeaponSpawner );

		WWASSERT( ScriptNameList.Count() == ScriptParameterList.Count() );
		for ( i = 0; i < ScriptNameList.Count(); i++ ) {
			WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_SCRIPT_NAME, ScriptNameList[i] );
			WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_SCRIPT_PARAMETERS, ScriptParameterList[i] );
		}

	csave.End_Chunk();

	return true;
}

bool	SpawnerDefClass::Load( ChunkLoadClass &cload )
{
	WWASSERT( ScriptNameList.Count() == ScriptParameterList.Count() );
	WWASSERT( SpawnDefinitionIDList.Count() == 0 );
	StringClass str;

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_DEF_PARENT:
				DefinitionClass::Load( cload );
				break;

			case CHUNKID_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						case MICROCHUNKID_DEF_DEFINITION_ID:
							{
								int def_id;
								cload.Read(&def_id,sizeof(def_id));
								SpawnDefinitionIDList.Add( def_id );
							}
							break;
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_PLAYER_TYPE,					PlayerType );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_SPAWN_MAX,						SpawnMax );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_SPAWN_DELAY,					SpawnDelay );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_SPAWN_DELAY_VARIATION,		SpawnDelayVariation );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_IS_PRIMARY,						IsPrimary );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_IS_SOLDIER_STARTUP,			IsSoldierStartup );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_POST_EFFECT_SPAWN_DELAY,	PostVisualSpawnDelay );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_SPECIAL_EFFECTS_OBJ_ID,		SpecialEffectsObjID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_GOTO_SPAWNER_POS,				GotoSpawnerPos );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_GOTO_SPAWNER_POS_PRIORITY,	GotoSpawnerPosPriority );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_TELEPORT_FIRST_SPAWN,		TeleportFirstSpawn );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_STARTS_DISABLED,				StartsDisabled );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_KILL_HIBERNATING_SPAWN,	KillHibernatingSpawn );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_APPLY_SPAWN_MATERIAL_EFFECT,	ApplySpawnMaterialEffect );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_IS_MULTIPLAY_WEAPON_SPAWNER,	IsMultiplayWeaponSpawner );

						case MICROCHUNKID_DEF_SCRIPT_NAME:
							LOAD_MICRO_CHUNK_WWSTRING( cload, str );
							ScriptNameList.Add( str );
							break;

						case MICROCHUNKID_DEF_SCRIPT_PARAMETERS:
							LOAD_MICRO_CHUNK_WWSTRING( cload, str );
							ScriptParameterList.Add( str );
							break;

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

	WWASSERT( ScriptNameList.Count() == ScriptParameterList.Count() );
	return true;
}

const PersistFactoryClass &	SpawnerDefClass::Get_Factory( void ) const
{
	return _SpawnerDefPersistFactory;
}


/*
** SpawnClass
*/
SimplePersistFactoryClass<SpawnerClass, CHUNKID_SPAWNER>	_SpawnerPersistFactory;

const PersistFactoryClass & SpawnerClass::Get_Factory (void) const
{
	return _SpawnerPersistFactory;
}

SpawnerClass::SpawnerClass( void ) :
	ID( 0 ),
	TM( 1 ),
	SpawnTM( 1 ),
	Definition( NULL ),
	SpawnCount( 0 ),
	SpawnDelayTimer( 0 ),
	Enabled( true )
{
	SpawnManager::Add_Spawner( this );
}

SpawnerClass::~SpawnerClass( void )
{
	SpawnManager::Remove_Spawner( this );
}

void	SpawnerClass::Init( const SpawnerDefClass & definition )
{
	Definition = &definition;
	Enabled = !definition.StartsDisabled;
}

enum	{
	CHUNKID_PARENT							=	1014991053,
	CHUNKID_VARIABLES,
	CHUNKID_LAST_SPAWN,

	MICROCHUNKID_ID						=	1,
	MICROCHUNKID_TM,
	MICROCHUNKID_DEFINITION_ID,
	MICROCHUNKID_SPAWN_COUNT,
	MICROCHUNKID_SPAWN_DELAY_TIMER,
	MICROCHUNKID_ENABLED,
	MICROCHUNKID_SPAWN_POINT_ENTRY,
	XXXMICROCHUNKID_REAL_SPAWN_TIMER,
	MICROCHUNKID_SPAWN_TM,
	MICROCHUNKID_SCRIPT_NAME,
	MICROCHUNKID_SCRIPT_PARAMETERS,
};

bool	SpawnerClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_PARENT );
		PersistClass::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ID,  ID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TM,  TM );
		int id = Get_Definition().Get_ID();
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEFINITION_ID, id );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_SPAWN_COUNT, SpawnCount );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_SPAWN_DELAY_TIMER, SpawnDelayTimer );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ENABLED, Enabled );
		int i;
		for ( i = 0; i < SpawnPointList.Count(); i++ ) {
			Matrix3D tm = SpawnPointList[i];
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_SPAWN_POINT_ENTRY, tm );
		}
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_SPAWN_TM, SpawnTM );

		WWASSERT( ScriptNameList.Count() == ScriptParameterList.Count() );
		for ( i = 0; i < ScriptNameList.Count(); i++ ) {
			WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_SCRIPT_NAME, ScriptNameList[i] );
			WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_SCRIPT_PARAMETERS, ScriptParameterList[i] );
		}

	csave.End_Chunk();

	if ( LastSpawn.Get_Ptr() != NULL ) {
		csave.Begin_Chunk( CHUNKID_LAST_SPAWN );
			LastSpawn.Save( csave );
		csave.End_Chunk();
	}


	return true;
}

bool	SpawnerClass::Load( ChunkLoadClass & cload )
{
	WWASSERT( ScriptNameList.Count() == ScriptParameterList.Count() );
	StringClass str;

	Matrix3D tm;
	WWASSERT( SpawnPointList.Length() == 0 );

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PARENT:
				PersistClass::Load( cload );
				break;

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ID,  ID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TM,  TM );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_SPAWN_COUNT, SpawnCount );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_SPAWN_DELAY_TIMER, SpawnDelayTimer );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ENABLED, Enabled );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_SPAWN_TM, SpawnTM );

						case	MICROCHUNKID_DEFINITION_ID:
							int definition_id;
							LOAD_MICRO_CHUNK( cload, definition_id );
							WWASSERT( Definition == NULL );
							Definition = (const SpawnerDefClass *)DefinitionMgrClass::Find_Definition( definition_id );
							WWASSERT( Definition != NULL );
							break;

						case	MICROCHUNKID_SPAWN_POINT_ENTRY:
							LOAD_MICRO_CHUNK( cload, tm );
							SpawnPointList.Add( tm );
							break;

						case MICROCHUNKID_SCRIPT_NAME:
							LOAD_MICRO_CHUNK_WWSTRING( cload, str );
							ScriptNameList.Add( str );
							break;

						case MICROCHUNKID_SCRIPT_PARAMETERS:
							LOAD_MICRO_CHUNK_WWSTRING( cload, str );
							ScriptParameterList.Add( str );
							break;

						default:
							Debug_Say(("Unhandled Micro Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
							break;

					}
					cload.Close_Micro_Chunk();
				}
				break;

			case CHUNKID_LAST_SPAWN:
				LastSpawn.Load( cload );
				break;

			default:
				Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}
		cload.Close_Chunk();
	}

	WWASSERT( ScriptNameList.Count() == ScriptParameterList.Count() );
	return true;
}


bool SpawnerClass::Determine_Spawn_TM( PhysicalGameObj * obj )
{
	bool spawnable = false;

	MoveablePhysClass * phys_obj = NULL;
	if ( obj->Peek_Physical_Object() ) {
		phys_obj = obj->Peek_Physical_Object()->As_MoveablePhysClass();
	}

	// If we have spawn points, try and find one where the object can be placed
	if ( SpawnPointList.Count() > 0 ) {

		// Find human player nearest spawner
		Vector3 nearest_human_player_pos( 0,0,0 );
		float commando_range = 10000000;
		SLNode<SmartGameObj> *objnode;
		for (	objnode = GameObjManager::Get_Smart_Game_Obj_List()->Head(); objnode; objnode = objnode->Next()) {
			SoldierGameObj * soldier = objnode->Data()->As_SoldierGameObj();
			if ( soldier && soldier->Is_Human_Controlled() ) {
				Vector3 pos;
				soldier->Get_Position( &pos );
				pos -= TM.Get_Translation();
				float range = pos.Length();
				if ( range < commando_range ) {
					soldier->Get_Position( &nearest_human_player_pos );
					commando_range = range;
				}
			}
		}

		// Find a safe spawnable point furthest from the commando
		float best_range = 0;

		// For every spawn point
		for ( int index = 0; index < SpawnPointList.Count(); index++ ) {

			bool is_safe = true;		// Assume safe
			Matrix3D safe_spot = SpawnPointList[index];
			if ( phys_obj ) {
				is_safe = phys_obj->Can_Teleport_And_Stand( SpawnPointList[index], &safe_spot );
			}

			if ( is_safe ) {
				// check range for furthest from commando
				Vector3 diff = safe_spot.Get_Translation() - nearest_human_player_pos;
				float range = diff.Length();
				if ( range > best_range ) {
					best_range = range;
					SpawnTM = safe_spot;
					spawnable = true;
				}
			}
		}

	} else {
		// Lets see if we can spawn at TM..

		// Only check teleport for armed objs
		if ( phys_obj && obj->As_ArmedGameObj() != NULL ) {
			Matrix3D safe_spot = TM;
			if ( phys_obj->Can_Teleport_And_Stand( TM, &safe_spot ) ) {
				SpawnTM = safe_spot;
				spawnable = true;
			}
		} else {
			SpawnTM = TM;
			spawnable = true;
		}
	}

	if ( spawnable ) {
		obj->Set_Transform( SpawnTM );
	}

	return spawnable;
}

PhysicalGameObj * SpawnerClass::Create_Spawned_Object( int obj_id )
{
	WWMEMLOG(MEM_GAMEDATA);
	
	int spawn_id = obj_id;
	if ( obj_id == -1 ) {
		
		/*
		** Randomly select an object to spawn
		*/
		int spawn_count = Get_Definition().SpawnDefinitionIDList.Count();		
		if ( spawn_count > 0 ) {
			int spawn_index = FreeRandom.Get_Int( spawn_count );
			spawn_id = Get_Definition().SpawnDefinitionIDList[ spawn_index ];
		}
	}

	/*
	** Create the object
	*/
	PhysicalGameObj * obj = NULL;
	if ( spawn_id != -1 ) {
		PhysicalGameObjDef * def = (PhysicalGameObjDef *)DefinitionMgrClass::Find_Definition( spawn_id );
		if ( def == NULL ) {
			Debug_Say(( "Spawner %s failed to create a spawn id %d\n", Get_Definition().Get_Name(), spawn_id ));
		}
		WWASSERT( def );
		if ( def != NULL ) {
			obj = (PhysicalGameObj *)def->Create();

			if ( obj == NULL ) {
				Debug_Say(( "Spawner Failed to create %s\n", def->Get_Name() ));
			}
		}
	}

	/*
	** If the definition calls for it, add a material effect to the object
	*/
	if ((obj != NULL) && (Get_Definition().ApplySpawnMaterialEffect)) {
		PhysClass * physobj = obj->Peek_Physical_Object();
		if (physobj != NULL) {
			TransitionEffectClass * effect = CombatMaterialEffectManager::Get_Spawn_Effect();
			physobj->Add_Effect_To_Me(effect);
			REF_PTR_RELEASE(effect);
		}
	}

	return obj;
}

void	SpawnerClass::Check_Auto_Spawn( float dtime )
{
	if ( LastSpawn.Get_Ptr() == NULL ) {	// If our last spawn is gone

		bool spawn_ok = false;

		if ( Get_Definition().SpawnMax < 0 ||
			  SpawnCount < Get_Definition().SpawnMax ) {
			spawn_ok = true;
		}

		if ( Enabled && spawn_ok ) {
			if ( SpawnDelayTimer > 0 ) {
				SpawnDelayTimer -= dtime;
			}
			if ( SpawnDelayTimer <= 0 ) {
				Spawn();
			}
		}
	} else if ( Get_Definition().KillHibernatingSpawn && _Allow_Killing_Hibernating_Spawn ) {
		 // If our last spawn is alive, but hibernating
		PhysicalGameObj * spawn = (PhysicalGameObj *)LastSpawn.Get_Ptr();
		if ( spawn->Is_Hibernating() ) {
			spawn->Set_Delete_Pending();	// Kill em
			SpawnCount--;
			SpawnDelayTimer = 0;		// no delay to bring him back

#if 0
			if ( Enabled ) {
				Debug_Say(( "Killing Hibernating Spawn from an Enabled spawner (id %d)\n", Get_ID() ));
			} else {
				Debug_Say(( "Killing Hibernating Spawn (spawner id %d)\n", Get_ID() ));
			}
#endif
		}
	}
}

PhysicalGameObj * SpawnerClass::Spawn_Object( int obj_def_id )
{	
	return Spawn( obj_def_id );
}

bool	SpawnerClass::Can_Spawn_Object( int obj_def_id )
{
	bool retval = false;

	//
	//	Loop over all the objects this spawner can create
	//
	const DynamicVectorClass<int>	&def_list = Definition->Get_Spawn_Definition_ID_List ();
	for (int index = 0; index < def_list.Count (); index ++) {
		
		//
		//	Is this the object we're looking for?
		//
		if ( def_list[index] == obj_def_id ) {
			retval = true;
			break;
		}
	}

	return retval;
}


PhysicalGameObj * SpawnerClass::Spawn( int obj_id )
{
	WWASSERT( CombatManager::I_Am_Server() );

	// Pick what will be spawned
	PhysicalGameObj * obj = Create_Spawned_Object( obj_id );
	WWASSERT( obj );

	//	Choose a spawn location
	bool spawnable = Determine_Spawn_TM( obj );

	if ( !spawnable ) {
		Debug_Say(( "Couldn't find a place to spawn this object ID %d\n", Get_ID() ));
		obj->Set_Delete_Pending();
		return NULL;
	}

	obj->Start_Observers();

	if ( Get_Definition().GotoSpawnerPos ) {

		// Teleport the first spawn
		if ( Get_Definition().TeleportFirstSpawn && SpawnCount == 0 ) {
			obj->Set_Transform( TM );
		} else {
			// Give initial goto command / don't let him hibernate
			SoldierGameObj * soldier = obj->As_SoldierGameObj();
			if ( soldier != NULL ) {
				ActionParamsStruct parameters;
				parameters.Priority = Get_Definition().GotoSpawnerPosPriority;
				parameters.ObserverID = 0;
				parameters.MoveLocation = TM.Get_Translation();
				parameters.MoveSpeed = 1;
				parameters.MoveCrouched = false;
				parameters.MoveArrivedDistance = 1;
				soldier->Get_Action()->Goto( parameters );
				soldier->Reset_Hibernating();
			}
		}
	}

	// Keep track of the last spawned object
	LastSpawn = obj;

	//	Create the special effects game object that will be responsible for playing
	// an animation and sound
	if ( Get_Definition().SpecialEffectsObjID != 0 ) {
		PhysicalGameObj *effect_obj = ObjectLibraryManager::Create_Object( Get_Definition().SpecialEffectsObjID );
		if ( effect_obj != NULL ) {
			effect_obj->Set_Transform( SpawnTM );
		}
	}

	//	Setup the spawning variables
	SpawnCount++;

	float variation = Get_Definition().SpawnDelayVariation/2;
	SpawnDelayTimer = Get_Definition().SpawnDelay + FreeRandom.Get_Float( -variation, variation );

	int i;

	// Start Def Scripts
	WWASSERT( Get_Definition().ScriptNameList.Count() == Get_Definition().ScriptParameterList.Count() );
	for ( i = 0; i < Get_Definition().ScriptNameList.Count(); i++ ) {
		ScriptClass* script = ScriptManager::Create_Script( Get_Definition().ScriptNameList[i] );
		if (script) {
			script->Set_Parameters_String( Get_Definition().ScriptParameterList[i] );
			obj->Add_Observer(script);
		}
	}

	// Start Instance Scripts
	WWASSERT( ScriptNameList.Count() == ScriptParameterList.Count() );
	for ( i = 0; i < ScriptNameList.Count(); i++ ) {
		ScriptClass* script = ScriptManager::Create_Script( ScriptNameList[i] );
		if (script) {
			script->Set_Parameters_String( ScriptParameterList[i] );
			obj->Add_Observer(script);
		}
	}

	return obj;
}

void	SpawnerClass::Add_Script( const char * script_name, const char * script_parameter )
{
	WWASSERT( ScriptNameList.Count() == ScriptParameterList.Count() );
	ScriptNameList.Add( script_name );
	ScriptParameterList.Add( script_parameter );
}

/*
** SpawnManager
*/
DynamicVectorClass<SpawnerClass*>	SpawnManager::SpawnerList;
float											SpawnManager::AutoSpawnTimer = AUTO_SPAWN_CHECK_DELAY;

/*
** Instance of SpawnManager that exists just to free memory at game end when it gets destructed.
*/
static SpawnManager JustForLeaks;


void	SpawnManager::Add_Spawner( SpawnerClass * spawner )
{
	WWASSERT(spawner != NULL);
	SpawnerList.Add( spawner );
}

void	SpawnManager::Remove_Spawner( SpawnerClass * spawner )
{
	SpawnerList.Delete( spawner );
}

void	SpawnManager::Remove_All_Spawners( void )
{
	while ( SpawnerList.Count() ) {
		delete SpawnerList[0];
	}
}


/*
**
*/
enum	{
	CHUNKID_SPAWNER_DATA					=	1014991133,
	CHUNKID_SPAWNER_VARIABLES,

	MICROCHUNKID_SPAWNER_AUTO_SPAWN_TIMER	=	1,
};

bool	SpawnManager::Save( ChunkSaveClass &csave )
{
	csave.Begin_Chunk( CHUNKID_SPAWNER_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_SPAWNER_AUTO_SPAWN_TIMER, AutoSpawnTimer );
	csave.End_Chunk();

	for ( int i = 0; i < SpawnerList.Count(); i++ ) {
		csave.Begin_Chunk( CHUNKID_SPAWNER_DATA );
			SpawnerList[i]->Save( csave );
		csave.End_Chunk();
	}
	return true;
}

bool	SpawnManager::Load( ChunkLoadClass &cload )
{
   Remove_All_Spawners();

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_SPAWNER_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_SPAWNER_AUTO_SPAWN_TIMER, AutoSpawnTimer );

						default:
							Debug_Say(("Unhandled Micro Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
							break;

					}
					cload.Close_Micro_Chunk();
				}
				break;

			case CHUNKID_SPAWNER_DATA:
			{
				SpawnerClass * spawner = new SpawnerClass();
				spawner->Load( cload );
				break;
			}

			default:
				Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}
		cload.Close_Chunk();
	}

   return true;
}

/*
**
*/
bool		SpawnManager::Spawner_Exists( int player_type )
{
	for ( int i = 0; i < SpawnerList.Count(); i++ ) {
		if ( SpawnerList[i]->Get_Definition().PlayerType == player_type ) {
			return true;
		}
	}
	return false;
}

Matrix3D SpawnManager::Get_Primary_Spawn_Location( void )
{
	int index = -1;
	for (int i = 0; i < SpawnerList.Count(); i++) {
		if ( SpawnerList[i]->Get_Definition().IsPrimary &&
			  SpawnerList[i]->Get_Definition().IsSoldierStartup ) {

			index = i;
			break;
		}
	}

	if (index == -1) {
		Debug_Say(("Get_Primary_Spawn_Location: failed to find suitable spawner, return origin.\n"));
		return Matrix3D(1);
	} else {
		return SpawnerList[index]->Get_TM();
	}
}

Matrix3D SpawnManager::Get_Multiplayer_Spawn_Location( int player_type, SoldierGameObj * soldier )
{
	if (player_type == PLAYERTYPE_NEUTRAL) {		
		//
		// Use renegade spawner
		//
		player_type = PLAYERTYPE_RENEGADE;
	}

	//
	// Find out how many suitable spawners there are
	//
	int suitable_spawners = 0;
	for ( int i = 0; i < SpawnerList.Count(); i++ ) {
		if ( !SpawnerList[i]->Get_Definition().IsPrimary &&
			  SpawnerList[i]->Get_Definition().IsSoldierStartup &&
			  SpawnerList[i]->Get_Definition().PlayerType == player_type ) {

			suitable_spawners++;
		}
	}

	Matrix3D tm;

	if (suitable_spawners == 0) {
		Debug_Say(("Get_Multiplayer_Spawn_Location: failed to find suitable spawner, return origin.\n"));
		tm =  Matrix3D(1);
	} else {

		//
		// Randomly choose one of the suitable spawners
		//
		int i;
		int selected_spawner = rand() % suitable_spawners;
		int start_index = 0;
		int count = 0;

		for ( i = 0; i < SpawnerList.Count(); i++ ) 
		{
			if ( !SpawnerList[i]->Get_Definition().IsPrimary &&
				  SpawnerList[i]->Get_Definition().IsSoldierStartup &&
				  SpawnerList[i]->Get_Definition().PlayerType == player_type ) 
			{
				if (count == selected_spawner) 
				{
					// As a fallback, we will always return the first transform
					// that was selected.
					tm = SpawnerList[i]->Get_TM();
					start_index = i;					
					break;
				}

				count++;
			}
		}
		
		//
		// Loop through the entire list of spawners, starting with the one
		// we just selected, until we find a clear spawn point for this soldier
		//
		Phys3Class * phys_obj = soldier->Peek_Human_Phys();
		if (phys_obj != NULL) {
			for ( i = 0; i < SpawnerList.Count(); i++) {
				
				// Wrap around the list if needed:
				int index = (i + start_index) % SpawnerList.Count();
				
				if ( !SpawnerList[index]->Get_Definition().IsPrimary &&
					  SpawnerList[index]->Get_Definition().IsSoldierStartup &&
					  SpawnerList[index]->Get_Definition().PlayerType == player_type ) 
				{
					if (phys_obj->Can_Teleport(SpawnerList[index]->Get_TM(),true)) {
						
						// Return the first good spawn point we find!
						return SpawnerList[index]->Get_TM();
					}
				}
			}
			// If we fall through to here, no clear spawn points were found, need more spawners!
			WWDEBUG_SAY(("Failed to find clear multiplayer spawn point for object: %s\r\n",phys_obj->Peek_Model()->Get_Name()));
		}
	}

	// Fallback - return the spawn point we randomly chose.
	return tm;
}



Matrix3D SpawnManager::Get_Ctf_Spawn_Location(int team)
{
	WWASSERT(team == PLAYERTYPE_NOD || team == PLAYERTYPE_GDI);

	StringClass spawner_name;
	if (team == PLAYERTYPE_NOD) {
		spawner_name = "Ctf_Pedestal_Nod";
	} else {
		spawner_name = "Ctf_Pedestal_GDI";
	}

	int index = -1;
	
	for (int i = 0; i < SpawnerList.Count(); i++) {
		if ( !spawner_name.Compare(SpawnerList[i]->Get_Definition().Get_Name()) ) {
			index = i;
			break;
		}
	}

	//WWASSERT(index != -1);
	if (index == -1)
	{
		Debug_Say(("*** Fatal Media Error: CTF spawner(s) missing from level.\n"));
		DIE;
	}

	return SpawnerList[index]->Get_TM();
}

SpawnerClass * SpawnManager::Get_Primary_Spawner( void )
{
	int index = -1;
	for (int i = 0; i < SpawnerList.Count(); i++) {
		if ( SpawnerList[i]->Get_Definition().IsPrimary &&
			  SpawnerList[i]->Get_Definition().IsSoldierStartup ) {

			index = i;
			break;
		}
	}

	if (index == -1) {
		Debug_Say(("Get_Primary_Spawner: failed to find suitable spawner, returning NULL.\n"));
		return NULL;
	} else {
		return SpawnerList[index];
	}
}

void	SpawnManager::Update( void )
{
	AutoSpawnTimer += TimeManager::Get_Frame_Seconds();

	if ( CombatManager::I_Am_Server() ) {

		if (AutoSpawnTimer >= AUTO_SPAWN_CHECK_DELAY ) {

			// Let each Spawner Check for Auto Spwan
			for ( int i = 0; i < SpawnerList.Count(); i++ ) {
				//
				// Soldiers are only spawned in missions (SP, coop)
				//
				if (!SpawnerList[i]->Get_Definition().IsSoldierStartup ||
					//CombatManager::Is_Mission()) {
					IS_SOLOPLAY) {
					SpawnerList[i]->Check_Auto_Spawn( AutoSpawnTimer );
				}
			}
			AutoSpawnTimer = 0;
		}
	}
}

PhysicalGameObj * SpawnManager::Spawner_Trigger( int id )
{
	for ( int i = 0; i < SpawnerList.Count(); i++ ) {
		if ( SpawnerList[i]->Get_ID() == id ) {
			return SpawnerList[i]->Spawn();
		}
	}
	return NULL;
}

void SpawnManager::Spawner_Enable( int id, bool enable )
{
	for ( int i = 0; i < SpawnerList.Count(); i++ ) {
		if ( SpawnerList[i]->Get_ID() == id ) {
			if ( enable ) {
				// force an immeadiate spawn check
				AutoSpawnTimer = AUTO_SPAWN_CHECK_DELAY;
			}
			SpawnerList[i]->Enable( enable );
		}
	}
}

void SpawnManager::Display_Unused_Spawners( void )
{
	Debug_Say(( "Unused Spawner Display....\n" ));
	for ( int i = 0; i < SpawnerList.Count(); i++ ) {
		if ( SpawnerList[i]->SpawnCount == 0 ) {
			Debug_Say(( "Spawner %d never Spawned\n", SpawnerList[i]->Get_ID() ));
		}
	}
}

bool	SpawnManager::Toggle_Allow_Killing_Hibernating_Spawn( void )
{
	_Allow_Killing_Hibernating_Spawn = !_Allow_Killing_Hibernating_Spawn;
	return _Allow_Killing_Hibernating_Spawn;
}




/***********************************************************************************************
 * SpawnManager::~SpawnManager -- Class destructor                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/14/2001 2:27PM ST : Created                                                             *
 *=============================================================================================*/
SpawnManager::~SpawnManager(void)
{
	Remove_All_Spawners();
}





