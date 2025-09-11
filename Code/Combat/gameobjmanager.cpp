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
 *                     $Archive:: /Commando/Code/Combat/gameobjmanager.cpp                    $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 12/21/01 3:43p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 76                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "gameobjmanager.h"
#include "wwdebug.h"
#include "smartgameobj.h"
#include "combatchunkid.h"
#include "saveloadsubsystem.h"
#include "persistfactory.h"
#include "debug.h"
#include "combat.h"
#include "scripts.h"
#include "soldier.h"
#include "building.h"
#include "wwphysids.h"
#include "scriptzone.h"
#include "wwprofile.h"
#include "networkobjectmgr.h"
#include "networkobjectmgr.h"
#include "vehicle.h"
#include "persistentgameobjobserver.h"
#include "weapons.h"

/*
** Create an instance of the game object manager list.  Since all
** memeber functions are static, no one needs to see it
*/
SList<BaseGameObj>	  	GameObjManager::GameObjList;
SList<SmartGameObj>	  	GameObjManager::SmartGameObjList;	// list of all render game objs
SList<SoldierGameObj>	GameObjManager::StarGameObjList;
SList<BuildingGameObj>	GameObjManager::BuildingGameObjList;
bool							GameObjManager::CinematicFreezeActive;

/*
**
*/
void GameObjManager::Init(void)
{
	Destroy_All();

	CinematicFreezeActive = false;
}

void GameObjManager::Shutdown(void)
{
	Destroy_All();

	PersistentGameObjObserverManager::Reset();
}


/*
**
*/
enum	{
	CHUNKID_OBJECTS							=	916991653,
	CHUNKID_VARIABLES,
	
	MICROCHUNKID_GENERATED_ID				= 1,
	MICROCHUNKID_GLOBAL_SIGHT_RANGE_SCALE,
	MICROCHUNKID_CINEMATIC_FREEZE,
};

bool	GameObjManager::Save( ChunkSaveClass &csave )
{
	csave.Begin_Chunk( CHUNKID_OBJECTS );
	// Allow each object in the master list to save
	SLNode<BaseGameObj> *objnode;
	for (	objnode = GameObjManager::Get_Game_Obj_List()->Head(); objnode; objnode = objnode->Next()) {
		BaseGameObj * obj = objnode->Data();
		csave.Begin_Chunk( obj->Get_Factory().Chunk_ID() );
		obj->Get_Factory().Save( csave, obj );
		csave.End_Chunk();
	}
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		int next_id = NetworkObjectMgrClass::Get_Current_Dynamic_ID ();
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_GENERATED_ID, next_id );

		float	scale = SmartGameObj::Get_Global_Sight_Range_Scale();
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_GLOBAL_SIGHT_RANGE_SCALE, scale );

		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_CINEMATIC_FREEZE, CinematicFreezeActive );

	csave.End_Chunk();

	return true;
}

bool	GameObjManager::Load( ChunkLoadClass &cload )
{
	float sight_scale = 1.0f;

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_OBJECTS:
				while (cload.Open_Chunk()) {
					PersistFactoryClass * factory = SaveLoadSystemClass::Find_Persist_Factory( cload.Cur_Chunk_ID() );
					if ( factory ) {
						factory->Load( cload );
					}
					cload.Close_Chunk();
				}
				break;
								
			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						
						READ_MICRO_CHUNK( cload, MICROCHUNKID_GLOBAL_SIGHT_RANGE_SCALE, sight_scale );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_CINEMATIC_FREEZE, CinematicFreezeActive );
						case MICROCHUNKID_GENERATED_ID:
						{
							/*TSS091001*/
							int next_id = NETID_DYNAMIC_OBJECT_MIN;
							LOAD_MICRO_CHUNK( cload, next_id )
							NetworkObjectMgrClass::Set_New_Dynamic_ID( next_id );
							Debug_Say(( "NetworkObjectMgrClass::Set_New_Dynamic_ID to %d\n", next_id ));
							/**/
							break;
						}

						default:
							Debug_Say(( "Unrecognized GameObjManager Variable chunkID %d\n", cload.Cur_Micro_Chunk_ID() ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized GameObjManager chunkID %d\n", cload.Cur_Chunk_ID() ));
				break;

		}
		cload.Close_Chunk();
	}

	SmartGameObj::Set_Global_Sight_Range_Scale( sight_scale );

	return true;
}


void	GameObjManager::Add( BaseGameObj *obj ) 
{ 
	// Make sure we have no duplicate IDs
	PhysicalGameObj *pobj = obj->As_PhysicalGameObj();
	if ( pobj ) {
		WWASSERT( Find_PhysicalGameObj(pobj->Get_ID()) == NULL ); 
	}

	// Cinematic scripts wanted objects not to progress on the frame they were created, 
	// and wanted the ability to set a frame number without haveing it bumped.
	// So, make new things at the head of the list, so the oldest thinks last.
//	GameObjList.Add_Tail( obj ); 
	GameObjList.Add_Head( obj ); 
}

void GameObjManager::Init_All()
{
#if 0
	SLNode<BaseGameObj> *objnode;
	for (	objnode = GameObjList.Head(); objnode; objnode = objnode->Next()) {
		objnode->Data()->Init();
	}
#endif
}


/*
**	GameObjectManager::Destroy_All
** This static routine destroys all objects
*/
void GameObjManager::Destroy_All()		// Destroy each object in the list
{
	ScriptManager::Enable_Script_Creation( false );	// Disable new scripts

	SLNode<BaseGameObj> *objnode;
	for (	objnode = GameObjList.Head(); objnode; objnode = objnode->Next()) {
		objnode->Data()->Set_Delete_Pending();
	}

// FIXME Disabling Think and Post_Think in GameObjManager::Destroy_All()
#if 0
	Think();	
	Post_Think();
#endif
	
	NetworkObjectMgrClass::Delete_Pending ();	

													// we do it twice in case any new objects got created
	for (	objnode = GameObjList.Head(); objnode; objnode = objnode->Next()) {
		objnode->Data()->Set_Delete_Pending();
	}
#if 0 	
	Think();	
	Post_Think();
#endif

	NetworkObjectMgrClass::Delete_Pending ();	

	WWASSERT( GameObjList.Head() == NULL );
	WWASSERT( SmartGameObjList.Head() == NULL );
	WWASSERT( StarGameObjList.Head() == NULL );
	WWASSERT( BuildingGameObjList.Head() == NULL );

	ScriptManager::Enable_Script_Creation( true );	// turn it back on
}


/*
**	GameObjectManager::Update_Control()
** This routine allows each SmartGameObject to generate input controls
*/
int	GameObjManager::Generate_Control()
{
	SLNode<SmartGameObj> *objnode;
	for (	objnode = SmartGameObjList.Head(); objnode; objnode = objnode->Next()) {

		// Don't genreate_control when cinematic frozen
		if ( Is_Cinematic_Freeze_Active() && objnode->Data()->Is_Cinematic_Freeze_Enabled() ) {
			continue;
		}

		if ( !objnode->Data()->Is_Hibernating() ) {
			objnode->Data()->Generate_Control();
		}
	}

	return 0;
}


int _AwakeSoldiers = 0;
int _HibernatingSoldiers = 0;


/*
** This static routine allows each GameObject to think
*/
int	GameObjManager::Think()
{
	// Allow each object in the master list to think
	SLNode<BaseGameObj> *objnode;
	for (	objnode = GameObjList.Head(); objnode; objnode = objnode->Next()) {

		// Don't think when cinematic frozen
		if ( Is_Cinematic_Freeze_Active() && objnode->Data()->Is_Cinematic_Freeze_Enabled() ) {
			// Stop the physics motion
			if ( objnode->Data()->As_SmartGameObj() != NULL ) {
				objnode->Data()->As_SmartGameObj()->Reset_Controller();

				// And the weapon (flames, etc)
				if ( objnode->Data()->As_SmartGameObj()->Get_Weapon() != NULL ) {
					objnode->Data()->As_SmartGameObj()->Get_Weapon()->Deselect();
				}

			}
			continue;
		}

		if ( !objnode->Data()->Is_Hibernating() ) {
			objnode->Data()->Think();
		}

		if ( objnode->Data()->As_SmartGameObj() ) {
			if ( objnode->Data()->As_SmartGameObj()->As_SoldierGameObj() ) {
				if ( objnode->Data()->Is_Hibernating() ) {
					_HibernatingSoldiers++;
				} else {
					_AwakeSoldiers++;
				}
			}
		}
	}

	return 0;
}

/*
**	GameObjectManager::PostThink()
** This static routine allows each GameObject to think after the rest
*/
int	GameObjManager::Post_Think()
{
	// Allow each object in the master list to think
	SLNode<BaseGameObj> *objnode;
	for (	objnode = GameObjList.Head(); objnode; objnode = objnode->Next()) {

		// Don't post_think when cinematic frozen
		if ( Is_Cinematic_Freeze_Active() && objnode->Data()->Is_Cinematic_Freeze_Enabled() ) {
			continue;
		}

		if ( !objnode->Data()->Is_Hibernating() && objnode->Data()->Is_Post_Think_Allowed() ) {
			objnode->Data()->Post_Think();
		}
	}

	//Destroy_Pending();

	GameObjObserverManager::Delete_Pending();

	ScriptManager::Destroy_Pending();

	return 0;
}

/*
** searches the commando with this client id
*/
SoldierGameObj * GameObjManager::Find_Soldier_Of_Client_ID(int client_id)
{
	{
		WWPROFILE( "FSOC id" );
		for (
			SLNode<SmartGameObj> * objnode = Get_Smart_Game_Obj_List()->Head(); 
			objnode; 
			objnode = objnode->Next()) {

			SoldierGameObj * p_soldier = objnode->Data()->As_SoldierGameObj();

			if (p_soldier != NULL && 
				 !p_soldier->Is_Delete_Pending() && 
				 p_soldier->Get_Control_Owner() == client_id) {

				return p_soldier;
			}
		}
	}

   return NULL;
}

/*
** searches for a player commando other than my own
*/
SoldierGameObj * GameObjManager::Find_Different_Player_Soldier(int my_id)
{
	for (
		SLNode<SmartGameObj> * objnode = Get_Smart_Game_Obj_List()->Head(); 
		objnode; 
		objnode = objnode->Next()) {

		SoldierGameObj * p_soldier = objnode->Data()->As_SoldierGameObj();

		if (p_soldier != NULL && 
			 !p_soldier->Is_Delete_Pending() && 
			 p_soldier->Is_Human_Controlled() &&
			 p_soldier->Get_Control_Owner() != my_id) {

			return p_soldier;
		}
	}

   return NULL;
}

SoldierGameObj	* GameObjManager::Find_Soldier_Of_Player_Type(int player_type)
{
   for (
		SLNode<SmartGameObj> * objnode = Get_Smart_Game_Obj_List()->Head(); 
		objnode; 
		objnode = objnode->Next()) {

		SoldierGameObj * p_soldier = objnode->Data()->As_SoldierGameObj();

      if (p_soldier != NULL && 
			!p_soldier->Is_Delete_Pending() && 
			 p_soldier->Get_Player_Type() == player_type) {

         return p_soldier;
      }
   }

   return NULL;
}

/*
** searches the game object list for the object with this id
*/
PhysicalGameObj * GameObjManager::Find_PhysicalGameObj( int id )
{
	SLNode<BaseGameObj> * objnode;
	for (	objnode = GameObjList.Head(); objnode; objnode = objnode->Next()) {
		PhysicalGameObj *obj = objnode->Data()->As_PhysicalGameObj();
		if ( obj && (obj->Get_ID() == id) ) {
			return obj;		// found it
		}
	}

	return NULL;	// Not found
}


/*
** searches the game object list for the object with this id
*/
ScriptableGameObj * GameObjManager::Find_ScriptableGameObj( int id )
{
	SLNode<BaseGameObj> * objnode;

	for (	objnode = GameObjList.Head(); objnode; objnode = objnode->Next()) {
		ScriptableGameObj *obj = objnode->Data()->As_ScriptableGameObj();
		if ( obj && (obj->Get_ID() == id) ) {
			return obj;		// found it
		}
	}

	return NULL;	// Not found
}


/*
** searches the game object list for a vehicle occupied by the given soldier.
*/
VehicleGameObj * GameObjManager::Find_Vehicle_Occupied_By( SoldierGameObj * p_soldier )
{
	WWASSERT(p_soldier != NULL);

	SLNode<BaseGameObj> * objnode;

	for (	objnode = GameObjList.Head(); objnode; objnode = objnode->Next()) {
		VehicleGameObj *obj = objnode->Data()->As_VehicleGameObj();
		//if ( obj && (obj->Get_Driver() == p_soldier) ) {
		if ( obj && obj->Contains_Occupant( p_soldier) ) {
			return obj;		// found it
		}
	}

	return NULL;	// Not found
}


/*
** searches the smart game object list for the object with this id
*/
SmartGameObj * GameObjManager::Find_SmartGameObj( int id )
{
	SLNode<SmartGameObj> * objnode;

	for (	objnode = SmartGameObjList.Head(); objnode; objnode = objnode->Next()) {
		SmartGameObj *obj = objnode->Data();
		if ( obj->Is_Delete_Pending() ) continue;		// Perhaps not find things that will be dieing?
		if ( obj->Get_ID() == id ) {
			return obj;		// found it
		}
	}

	return NULL;	// Not found
}


/*
** Buildings
*/
void	GameObjManager::Init_Buildings( void )
{
	/*
	** Ask each building to build its list of aggregates, meshes, and lights
	*/
	SLNode<BuildingGameObj> *objnode = NULL;
	for ( objnode = BuildingGameObjList.Head(); objnode; objnode = objnode->Next() ) {
		BuildingGameObj *obj = objnode->Data()->As_BuildingGameObj();
		if (obj != NULL) {
			obj->Collect_Building_Components();
		}
	}

	return ;
}


/*
** Update_Building_Collection_Spheres
*/
void	GameObjManager::Update_Building_Collection_Spheres( void )
{
	SLNode<BuildingGameObj> *objnode = NULL;
	for (	objnode = BuildingGameObjList.Head(); objnode; objnode = objnode->Next()) {
		BuildingGameObj *obj = objnode->Data()->As_BuildingGameObj();
		if (obj != NULL) {

			//
			//	Get some information about the current building
			//
			const char *prefix	= obj->Get_Name_Prefix ();
			float max_radius		= 50.0F;
			Vector3 position;
			obj->Get_Position (&position);

			//
			//	Test this building with all other buildings that have the same prefix
			//
			SLNode<BuildingGameObj> *test_node = NULL;
			for (	test_node = BuildingGameObjList.Head(); test_node; test_node = test_node->Next()) {
				BuildingGameObj *test_obj = test_node->Data()->As_BuildingGameObj();

				//
				//	Is this a building with the same prefix?
				//
				if (	test_obj != NULL && test_obj != obj &&
						::openw3d::stricmp (test_obj->Get_Name_Prefix (), prefix) == 0)
				{
					//
					//	Get the test object's position
					//
					Vector3 test_position;
					test_obj->Get_Position (&test_position);

					
					//
					//	Minimize the collection radius for this building (if necessary)
					//
					float distance = (position - test_position).Length ();
					max_radius = min (distance, max_radius);
				}
			}

			//
			//	Set this object's collection radius
			//
			obj->CollectionSphere.Radius = max_radius;
		}
	}

	return ;
}


/*
** Debug_Set_All_Building_States
*/
void	GameObjManager::Debug_Set_All_Building_States(float health_percentage,bool power_on)
{
	SLNode<BuildingGameObj> * objnode;
	for (	objnode = BuildingGameObjList.Head(); objnode; objnode = objnode->Next()) {
		BuildingGameObj *building = objnode->Data()->As_BuildingGameObj();
		building->Enable_Power(power_on);
		building->Set_Normalized_Health(health_percentage / 100.0f);
	}
}


/*
** Environment Zones
*/
bool	GameObjManager::Is_In_Environment_Zone( Vector3 & pos )
{
	WWPROFILE( "Is_In_Environment_Zone" );

	// Allow each object in the master list to think
	SLNode<BaseGameObj> *objnode;
	for (	objnode = GameObjList.Head(); objnode; objnode = objnode->Next()) {
		ScriptableGameObj * scriptable = objnode->Data()->As_ScriptableGameObj();
		if ( scriptable ) {
			ScriptZoneGameObj * script_zone = scriptable->As_ScriptZoneGameObj();
			if ( script_zone && script_zone->Is_Environment_Zone() ) {
				if ( CollisionMath::Overlap_Test( script_zone->Get_Bounding_Box(), pos ) == CollisionMath::INSIDE ) {
					return true;
				}
			}
		}
	}
	return false;
}



























/*
**	GameObjectManager::Destroy_Pending
** This static routine destroys all objects with destroy bit set
*/
/*void GameObjManager::Destroy_Pending()
{
	// Remove the wishing to be dead ones
	SLNode<BaseGameObj> *objnode;
	for (	objnode = GameObjList.Head(); objnode; ) {
      WWASSERT(objnode != NULL);
		BaseGameObj *obj = objnode->Data();
		objnode = objnode->Next();
      WWASSERT(obj != NULL);

		if ( obj->Is_Destroy() ) {
			// If I'm a server, notify all others to destroy this object if this is a physical game obj!
			if ( CombatManager::I_Am_Server() ) {
				CombatManager::Object_Destroyed(obj);
			}

			delete obj;
		}
	}
}*/


