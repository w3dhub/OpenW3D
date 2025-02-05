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
 *                     $Archive:: /Commando/Code/Combat/scriptzone.cpp                        $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 1/18/02 10:44a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 18                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "scriptzone.h"
#include "gameobjmanager.h"
#include "scripts.h"
#include "colmath.h"
#include "colmathinlines.h"
#include "debug.h"
#include "persistfactory.h"
#include "combatchunkid.h"
#include "wwhack.h"
#include "simpledefinitionfactory.h"
#include "combat.h"
#include "smartgameobj.h"
#include "playertype.h"
#include "wwprofile.h"
#include "pscene.h"
#include "phys.h"
#include "soldier.h"

/*
** ScriptZoneGameObjDef
*/
DECLARE_FORCE_LINK( Zone )

/*
** Namespaces
*/
using namespace ZoneConstants;

/*
** Constants
*/
static const char *	ZONE_TYPE_NAMES[TYPE_COUNT] =
{
	"Default",
	"Capture The Flag",
	"Vehicle Construction",
	"Vehicle Repair",
	"Tiberium Field",
	"Beacon",
	"GDI Tiberium Field",
	"NOD Tiberium Field",
};

/*
** Editable and Save/Load system stuff
*/
SimplePersistFactoryClass<ScriptZoneGameObjDef, CHUNKID_GAME_OBJECT_DEF_SCRIPT_ZONE>	_ScriptZoneGameObjDefPersistFactory;

DECLARE_DEFINITION_FACTORY(ScriptZoneGameObjDef, CLASSID_GAME_OBJECT_DEF_SCRIPT_ZONE, "Script Zone") _ScriptZoneGameObjDefDefFactory;

ScriptZoneGameObjDef::ScriptZoneGameObjDef( void ) :
	Color( 0, 0.7F, 0 ),
	CheckStarsOnly( true ),
	ZoneType( TYPE_DEFAULT ),
	IsEnvironmentZone( false )
{
	EDITABLE_PARAM( ScriptZoneGameObjDef, ParameterClass::TYPE_COLOR, Color );
	EDITABLE_PARAM( ScriptZoneGameObjDef, ParameterClass::TYPE_BOOL, CheckStarsOnly );
	EDITABLE_PARAM( ScriptZoneGameObjDef, ParameterClass::TYPE_BOOL, IsEnvironmentZone );

	//
	//	Configure the zone type parameter
	//
#ifdef PARAM_EDITING_ON
	EnumParameterClass *zone_type_param = new EnumParameterClass( (int*)&ZoneType );
	zone_type_param->Set_Name( "Zone Type" );
	for ( int index = 0; index < TYPE_COUNT; index++ ) {																			
		zone_type_param->Add_Value( ZONE_TYPE_NAMES[index], index );
	}																																									
	GENERIC_EDITABLE_PARAM(ScriptZoneGameObjDef,zone_type_param);
#endif

}

uint32	ScriptZoneGameObjDef::Get_Class_ID (void) const	
{ 
	return CLASSID_GAME_OBJECT_DEF_SCRIPT_ZONE; 
}

PersistClass *	ScriptZoneGameObjDef::Create( void ) const 
{
	ScriptZoneGameObj * obj = new ScriptZoneGameObj;
	obj->Init( *this );
	return obj;
}

enum	{
	XXXCHUNKID_DEF_PARENT_OLD							= 1111991132,
	CHUNKID_DEF_VARIABLES,
	CHUNKID_DEF_PARENT,

	MICROCHUNKID_DEF_IS_CTF_ZONE						=	1,
	MICROCHUNKID_DEF_ZONE_COLOR,
	MICROCHUNKID_DEF_CHECK_STARS_ONLY,
	MICROCHUNKID_DEF_ZONE_TYPE,
	MICROCHUNKID_DEF_IS_ENVIRONMENT_ZONE,

};

bool	ScriptZoneGameObjDef::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_DEF_PARENT );
		ScriptableGameObjDef::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_DEF_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_ZONE_COLOR, Color );		
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_CHECK_STARS_ONLY, CheckStarsOnly );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_ZONE_TYPE, ZoneType );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_IS_ENVIRONMENT_ZONE, IsEnvironmentZone );
	csave.End_Chunk();

	return true;
}

bool	ScriptZoneGameObjDef::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_DEF_PARENT:
				ScriptableGameObjDef::Load( cload );
				break;
								
			case CHUNKID_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_ZONE_COLOR, Color );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_CHECK_STARS_ONLY, CheckStarsOnly );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_ZONE_TYPE, ZoneType );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_IS_ENVIRONMENT_ZONE, IsEnvironmentZone );

						default:
							Debug_Say(( "Unrecognized ZoneDef Variable chunkID\n" ));
							break;

					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized ZoneDef chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}


const PersistFactoryClass & ScriptZoneGameObjDef::Get_Factory (void) const 
{ 
	return _ScriptZoneGameObjDefPersistFactory; 
}


/*
** ScriptZoneGameObj
*/
SimplePersistFactoryClass<ScriptZoneGameObj, CHUNKID_GAME_OBJECT_SCRIPT_ZONE>	_ScriptZoneGameObjPersistFactory;

ScriptZoneGameObj::ScriptZoneGameObj( void ) :
	PlayerType( PLAYERTYPE_NEUTRAL )
{
}

ScriptZoneGameObj::~ScriptZoneGameObj( void )
{
	GameObjReference * ref;
	while (	(ref = InsideList.Remove_Head() ) != NULL ) {
		*ref = NULL;
		delete ref;
	}
}

const PersistFactoryClass & ScriptZoneGameObj::Get_Factory (void) const 
{
	return _ScriptZoneGameObjPersistFactory;
}

/*
**
*/
void ScriptZoneGameObj::Init( void )
{
	Init( Get_Definition() );
}

void	ScriptZoneGameObj::Init( const ScriptZoneGameObjDef & definition )
{
	ScriptableGameObj::Init( definition );
}

const ScriptZoneGameObjDef & ScriptZoneGameObj::Get_Definition( void ) const
{
	return (const ScriptZoneGameObjDef &)BaseGameObj::Get_Definition();
}

/*
** ScriptZoneGameObj Save and Load
*/
enum	{
	CHUNKID_PARENT_OLD						=	922991806,
	CHUNKID_VARIABLES,
	CHUNKID_INSIDE_LIST,
	CHUNKID_INSIDE_LIST_ENTRY,
	CHUNKID_PARENT,

	MICROCHUNKID_BOUNDING_BOX			=	1,
	MICROCHUNKID_PLAYER_TYPE,
};

bool	ScriptZoneGameObj::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_PARENT );
		ScriptableGameObj::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_BOUNDING_BOX, BoundingBox );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_PLAYER_TYPE, PlayerType );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_INSIDE_LIST );
	SLNode<GameObjReference> *pobjrefnode;
	for (	pobjrefnode = InsideList.Head(); pobjrefnode; pobjrefnode = pobjrefnode->Next() ) {
		GameObjReference *ref = pobjrefnode->Data();
		if ( *ref != NULL ) {
			csave.Begin_Chunk( CHUNKID_INSIDE_LIST_ENTRY );
			ref->Save( csave );
			csave.End_Chunk();
		}
	}

	csave.End_Chunk();

	return true;
}


bool	ScriptZoneGameObj::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PARENT:
				ScriptableGameObj::Load( cload );
				break;
								
			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_BOUNDING_BOX, BoundingBox );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_PLAYER_TYPE, PlayerType );
						default:
							Debug_Say(( "Unrecognized ScriptZoneGameObj Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;
	
			case CHUNKID_INSIDE_LIST:
				WWASSERT( InsideList.Get_Count()== 0 );
				while (cload.Open_Chunk()) {
					WWASSERT(  cload.Cur_Chunk_ID() == CHUNKID_INSIDE_LIST_ENTRY );
					GameObjReference * ref = new GameObjReference;
					ref->Load( cload );
					InsideList.Add_Head( ref );
					cload.Close_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized ScriptZoneGameObj chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	// Legacy Fix
	switch( PlayerType ) {
		case 2: PlayerType = PLAYERTYPE_NEUTRAL; break;		// Remap TEAM2
		case 3: PlayerType = PLAYERTYPE_NEUTRAL; break;		// Remap TEAM3
		case 4: PlayerType = PLAYERTYPE_NOD; break;		// Remap NOD
		case 5: PlayerType = PLAYERTYPE_GDI; break;		// Remap GDI
	}

	return true;
}

/*
**
*/
void	ScriptZoneGameObj::Think()
{
	ScriptableGameObj::Think();

	if ( Get_Observers().Count() == 0 && Get_Definition().ZoneType != TYPE_CTF ) {
		return;
	}

	WWPROFILE( "ScriptZone Think" );

	// check current objects for exiting
	SLNode<GameObjReference> *pobjrefnode;
	for (	pobjrefnode = InsideList.Head(); pobjrefnode; ) {
		SmartGameObj * obj = (SmartGameObj*)pobjrefnode->Data()->Get_Ptr();
		
		SLNode<GameObjReference> * next = pobjrefnode->Next();

		if ( obj == NULL ) {
			Debug_Say(( "Object died inside me\n" ));
			GameObjReference * ref = pobjrefnode->Data();
			InsideList.Remove( ref );
			*ref = NULL;
			delete ref;
		} else if ( !Inside_Me( obj ) ) {

			const GameObjObserverList & observer_list = Get_Observers();
			for( int index = 0; index < observer_list.Count(); index++ ) {
				observer_list[ index ]->Exited( this, (PhysicalGameObj*)obj );
			}

			GameObjReference * ref = pobjrefnode->Data();
			InsideList.Remove( ref );
			*ref = NULL;
			delete ref;
		}

		pobjrefnode = next;
	}


	// If only gathering stars...
	if (Get_Definition().CheckStarsOnly ) {

		WWPROFILE( "Star Enter" );
		// check all stars for entering
		SLNode<SoldierGameObj> *objnode;
		for (	objnode = GameObjManager::Get_Star_Game_Obj_List()->Head(); objnode; objnode = objnode->Next()) {
			SoldierGameObj * obj = objnode->Data();
			if ( obj && Inside_Me( obj ) && !In_List( obj ) ) {
				Entered( obj );
			}
		}
	} else {
		WWPROFILE( "All Enter" );
		// Collect the dynamic physics objects overlapping this zone
		NonRefPhysListClass objs_in_zone;
		PhysicsSceneClass::Get_Instance()->Collect_Objects( BoundingBox, false, true, &objs_in_zone );
		NonRefPhysListIterator it(&objs_in_zone);
		for (it.First(); !it.Is_Done(); it.Next()) {
			if ( it.Peek_Obj()->Get_Observer() != NULL ) {
				PhysicalGameObj * gameobj = ((CombatPhysObserverClass *)it.Peek_Obj()->Get_Observer())->As_PhysicalGameObj();
				if (gameobj != NULL) {
					SmartGameObj *obj = gameobj->As_SmartGameObj();
					if ( obj && Inside_Me( obj ) && !In_List( obj ) ) {
						Entered( obj );
					}
				}
			}
		}
	}

}

/*
**
*/
void		ScriptZoneGameObj::Entered( SmartGameObj * obj )
{
	const GameObjObserverList & observer_list = Get_Observers();
	for( int index = 0; index < observer_list.Count(); index++ ) {
		observer_list[ index ]->Entered( this, obj );
	}

	// Create a new reference and add it to our list
	GameObjReference * ref = new GameObjReference;
	*ref = obj;
	InsideList.Add_Head( ref );
}

/*
**
*/
bool	ScriptZoneGameObj::In_List( SmartGameObj * obj ) 
{
	WWASSERT( obj != NULL );
	SLNode<GameObjReference> *pobjrefnode;
	for (	pobjrefnode = InsideList.Head(); pobjrefnode; pobjrefnode = pobjrefnode->Next() ) {
		if ( obj == *pobjrefnode->Data() ) {
			return true;
		}
	}

	return false;
}

/*
**
*/
bool	ScriptZoneGameObj::Inside_Me( const SmartGameObj * obj ) 
{
	if ( obj && obj->Peek_Physical_Object() ) { // hack to hide zones
		Vector3 pos;
		obj->Get_Position(&pos);
		return (CollisionMath::Overlap_Test(BoundingBox,pos) == CollisionMath::INSIDE);
	}
	return false;
}

/*
**
*/
ScriptZoneGameObj *ScriptZoneGameObj::Find_Closest_Zone (const Vector3 &pos, ZoneConstants::ZoneType type)
{
	SList<BaseGameObj> *list = GameObjManager::Get_Game_Obj_List ();
	float closest_dist2					= 999999.0F;
	ScriptZoneGameObj *closest_zone	= NULL;

	//
	//	Loop over all the objects looking for zones
	//	
	SLNode<BaseGameObj> *node = NULL;
	for (node = list->Head (); node != NULL; node = node->Next ()) {
		BaseGameObj *game_obj = node->Data ();
		
		//
		//	Is this a building on our team?
		//		
		if (game_obj != NULL && game_obj->As_ScriptableGameObj () != NULL) {
			ScriptZoneGameObj *zone = game_obj->As_ScriptableGameObj ()->As_ScriptZoneGameObj ();
			if (zone != NULL) {
			
				//
				//	Is the type of zone we are looking for?
				//
				if (zone->Get_Definition ().Get_Type () == type) {
					float dist2 = (pos - zone->Get_Bounding_Box ().Center).Length2 ();
					
					//
					//	Is this the closest zone we've found yet?
					//
					if (dist2 < closest_dist2) {
						closest_dist2 = dist2;
						closest_zone = zone;
					}
				}
			}
		}
	}
	
	return closest_zone;
}

/*
**
*/
int ScriptZoneGameObj::Count_Team_Members_Inside( int player_type )
{
	SLNode<GameObjReference> *pobjrefnode;
	int count = 0;
	for (	pobjrefnode = InsideList.Head(); pobjrefnode; pobjrefnode = pobjrefnode->Next() ) {
		SmartGameObj * obj = (SmartGameObj*)pobjrefnode->Data()->Get_Ptr();
		if ( obj->Get_Player_Type() == player_type ) {
			count++;
		}
	}
	return count;
}




















/*
**
*/
/*
bool	ScriptZoneGameObj::Exists_On_Client( void ) const
{
	bool retval = false;
	
	//
	//	Allow the C&C mode zone's to exist on the client
	//
	switch (Get_Definition ().Get_Type ())
	{
		case TYPE_VEHICLE_CONSTRUCTION:
		case TYPE_VEHICLE_REPAIR:
		case TYPE_TIBERIUM_FIELD:
		case TYPE_BEACON:
			retval = false;
			break;
	}

	return retval;
}
*/	

/*
	if (Get_Definition().ZoneType == TYPE_CTF && obj->As_SoldierGameObj() != NULL) {
		CombatManager::Soldier_Enters_Pedestal(obj->As_SoldierGameObj(), Get_Player_Type());
	}
	*/

