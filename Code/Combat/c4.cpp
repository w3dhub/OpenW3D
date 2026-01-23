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
 *                     $Archive:: /Commando/Code/Combat/c4.cpp                                $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 2/12/02 10:25a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 72                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "c4.h"
#include "debug.h"
#include "phys.h"
#include "combat.h"
#include "explosion.h"
#include "soldier.h"
#include "persistfactory.h"
#include "combatchunkid.h"
#include "weaponmanager.h"
#include "simpledefinitionfactory.h"
#include "wwhack.h"
#include "decophys.h"
#include "assets.h"
#include "gameobjmanager.h"
#include "WWAudio.h"
#include "wwprofile.h"
#include "projectile.h"
#include "wwphysids.h"
#include "buildingaggregate.h"
#include "rendobj.h"
#include "definitionmgr.h"
#include "apppackettypes.h"
#include "bitpackids.h"
#include "surfaceeffects.h"
#include "gametype.h"

/*
** C4GameObjDef
*/
DECLARE_FORCE_LINK( C4 )

SimplePersistFactoryClass<C4GameObjDef, CHUNKID_GAME_OBJECT_DEF_C4>	_C4GameObjDefPersistFactory;

DECLARE_DEFINITION_FACTORY(C4GameObjDef, CLASSID_GAME_OBJECT_DEF_C4, "C4") _C4GameObjDefDefFactory;

C4GameObjDef::C4GameObjDef( void ) :
	ThrowVelocity( 5 )
{
	EDITABLE_PARAM (C4GameObjDef, ParameterClass::TYPE_FLOAT,	ThrowVelocity);
}

uint32	C4GameObjDef::Get_Class_ID (void) const	
{ 
	return CLASSID_GAME_OBJECT_DEF_C4; 
}

PersistClass *	C4GameObjDef::Create( void ) const 
{
	C4GameObj * obj = new C4GameObj;
	obj->Init( *this );
	return obj;
}

enum	{
	CHUNKID_DEF_PARENT							=	930991700,
	CHUNKID_DEF_VARIABLES,

	MICROCHUNKID_DEF_THROW_VELOCITY			= 1,

};

bool	C4GameObjDef::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_DEF_PARENT );
		PhysicalGameObjDef::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_DEF_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_THROW_VELOCITY, ThrowVelocity );
	csave.End_Chunk();

	return true;
}

bool	C4GameObjDef::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_DEF_PARENT:
				PhysicalGameObjDef::Load( cload );
				break;

			case CHUNKID_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_THROW_VELOCITY, ThrowVelocity );
						default:
							Debug_Say(("Unhandled Micro Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;
				
			default:
				Debug_Say(( "Unrecognized SimpleDef chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

const PersistFactoryClass & C4GameObjDef::Get_Factory (void) const 
{ 
	return _C4GameObjDefPersistFactory; 
}


/*
**
*/
SimplePersistFactoryClass<C4GameObj, CHUNKID_GAME_OBJECT_C4>	_C4GameObjPersistFactory;

const PersistFactoryClass & C4GameObj::Get_Factory (void) const 
{
	return _C4GameObjPersistFactory;
}


/*
**
*/
C4GameObj::C4GameObj( void ) :
	AmmoDefinition( NULL ),
	Stuck( false ),
	StuckMCT( false ),
	StuckToObject( false ),
	StuckBone( 0 ),
	StuckStaticAnimObj(NULL),
	OwnerBackup( NULL ),
	Age( 0 )
{
	Set_App_Packet_Type(APPPACKETTYPE_C4);

}

C4GameObj::~C4GameObj()
{
	REF_PTR_RELEASE(StuckStaticAnimObj);
}

/*
**
*/
void C4GameObj::Init( void )
{
	Init( Get_Definition() );
}

/*
**
*/
void	C4GameObj::Init( const C4GameObjDef & definition )
{
	SimpleGameObj::Init( definition );
	Peek_Physical_Object()->Set_Collision_Group( TERRAIN_AND_BULLET_COLLISION_GROUP );
	Peek_Physical_Object()->Set_Collision_Group( DEFAULT_COLLISION_GROUP );
}

const C4GameObjDef & C4GameObj::Get_Definition( void ) const
{
	return (const C4GameObjDef &)BaseGameObj::Get_Definition();
}

void	C4GameObj::Init_C4( const AmmoDefinitionClass * def, SoldierGameObj *owner, int detonation_mode, const Matrix3D & tm )
{
	WWASSERT( AmmoDefinition == NULL );
	AmmoDefinition = def;

	if ( !def->ModelName.Is_Empty() ) {
		Peek_Physical_Object()->Set_Model_By_Name( def->ModelName ) ;
	}

	Owner = owner;
	DetonationMode = detonation_mode;
	Set_Transform( tm );
	Stuck = false;
	StuckMCT = false;
	StuckToObject = false;
	Peek_Physical_Object()->Set_Collision_Group( DEFAULT_COLLISION_GROUP );
	OwnerBackup = NULL;

	if ( owner ) {
		Set_Player_Type( owner->Get_Player_Type() );
		OwnerBackup = owner->Get_Player_Data();

		if ( CombatManager::I_Am_Server() && !IS_MISSION ) {
			Maintain_C4_Limit( Get_Player_Type() );
		}
	}

	int type = AmmoDefinition->AmmoType;

	if ( type != AmmoDefinitionClass::AMMO_TYPE_C4_REMOTE ) {
		// Setup Arming Timer
		float	time = def->C4TriggerTime1;
		if ( detonation_mode == 2 ) 		time = def->C4TriggerTime2;
		if ( detonation_mode == 3 )		time = def->C4TriggerTime3;
		Timer = time;
	}

	float	sound_id = def->C4TimingSound1ID;
	if ( detonation_mode == 2 ) 		sound_id = def->C4TimingSound1ID;
	if ( detonation_mode == 3 )		sound_id = def->C4TimingSound1ID;

	if ( sound_id ) {
		RefCountedGameObjReference *owner_ref = new RefCountedGameObjReference(Owner);
		WWAudioClass::Get_Instance()->Create_Instant_Sound( sound_id, Get_Transform(), owner_ref);
		REF_PTR_RELEASE(owner_ref);
	}

	ProjectileClass * po = Peek_Physical_Object()->As_ProjectileClass();
	if ( po ) {
		po->Set_Velocity( tm.Get_X_Vector() * Get_Definition().ThrowVelocity );
	}

	//
	//	"Dirty" the object for networking
	//
	Set_Object_Dirty_Bit( NetworkObjectClass::BIT_RARE, true );
}

CollisionReactionType	C4GameObj::Collision_Occurred( const CollisionEventClass & event )
{
	Debug_Say(( "C4 collision\n" ));

//	if ( ( !Stuck ) && (CombatManager::I_Am_Server()) ) 
	if (!Stuck)
	{
		// Figure out who/what/where we hit
		PhysicalGameObj * other = NULL;
		BuildingGameObj * building = NULL;
		bool hit_projectile = event.OtherObj->As_ProjectileClass() != NULL;

		if ( event.OtherObj->Get_Observer() != NULL ) {
			other = ((CombatPhysObserverClass *)event.OtherObj->Get_Observer())->As_PhysicalGameObj();
			building = ((CombatPhysObserverClass *)event.OtherObj->Get_Observer())->As_BuildingGameObj();
		}

		Restore_Owner();

		// Ignore my owner and my owners vehicle
		if ( other != NULL && Get_Owner() != NULL ) {
			VehicleGameObj * vehicle = other->As_VehicleGameObj();
			SoldierGameObj * soldier = Get_Owner()->As_SoldierGameObj();
			if ( vehicle != NULL && vehicle == soldier->Get_Vehicle() ) {
				return COLLISION_REACTION_NO_BOUNCE;
			}
			if ( other == Get_Owner() ) {
				return COLLISION_REACTION_NO_BOUNCE;
			}

//			if ( !other->Is_Teammate( Get_Owner() ) ) 
			{
				Debug_Say(( "Sticking to game object %p (%p)\n", other, Get_Owner() ));
				Stuck = true;
				StuckToObject = true;
				StuckObject = other;

				StuckBone = 0;
				RenderObjClass * parent_model = other->Peek_Model();
				if ( parent_model ) {
					StuckBone = parent_model->Get_Sub_Object_Bone_Index( event.CollidedRenderObj );
				}

				Vector3 my_pos;
				Get_Position(&my_pos);
				Matrix3D::Inverse_Transform_Vector( parent_model->Get_Bone_Transform( StuckBone ), my_pos, &StuckOffset );
				Peek_Physical_Object()->Enable_User_Control( true );
				
				if (CombatManager::I_Am_Server()) {
					Set_Object_Dirty_Bit(NetworkObjectClass::BIT_RARE, true);
				}
				return COLLISION_REACTION_STOP_MOTION;
			}
		} else if ( building != NULL ) {

			// Stick to the building
			Stuck = true;
			StuckObject = (ScriptableGameObj*)building;
			StuckMCT = false;

			// Check for MCT collision
			if (event.OtherObj->Get_Factory().Chunk_ID() == PHYSICS_CHUNKID_BUILDINGAGGREGATE) {
				if (((BuildingAggregateClass *)event.OtherObj)->Is_MCT()) {
					StuckMCT = true;
				}
			}

			Peek_Physical_Object()->Enable_User_Control( true );

			if (CombatManager::I_Am_Server()) {
				Set_Object_Dirty_Bit(NetworkObjectClass::BIT_RARE, true);
			}
			return COLLISION_REACTION_STOP_MOTION;

		} else if ( other == NULL && !hit_projectile ) {
			
			// if this is a static anim, then try to stick to it
			if (event.OtherObj->As_StaticAnimPhysClass() != NULL) {

				Debug_Say(( "Sticking to static anim object %p (%p)\n", event.OtherObj ));
				REF_PTR_SET(StuckStaticAnimObj,(StaticAnimPhysClass *)event.OtherObj);
				
				StuckBone = 0;
				if ( StuckStaticAnimObj->Peek_Model() ) {
					StuckBone = StuckStaticAnimObj->Peek_Model()->Get_Sub_Object_Bone_Index( event.CollidedRenderObj );
					Vector3 my_pos;
					Get_Position(&my_pos);
					Matrix3D::Inverse_Transform_Vector( StuckStaticAnimObj->Peek_Model()->Get_Bone_Transform( StuckBone ), my_pos, &StuckOffset );
				}
			} 

			// If we hit permiable, pass through
			if ( event.CollisionResult != NULL && 
				SurfaceEffectsManager::Is_Surface_Permeable( event.CollisionResult->SurfaceType ) ) {
				Debug_Say(( "C4 passes through permeable\n" ));
				return COLLISION_REACTION_NO_BOUNCE;
			}

			// We are hitting a static terrain, just stick
			Debug_Say(( "Sticking to terrain\n" ));
			Peek_Physical_Object()->Enable_User_Control( true );
			Stuck = true;

			if (CombatManager::I_Am_Server()) {
				Set_Object_Dirty_Bit(NetworkObjectClass::BIT_RARE, true);
			}
			return COLLISION_REACTION_STOP_MOTION;
		}

	}

	return COLLISION_REACTION_NO_BOUNCE;
}


/*
** C4GameObj Save and Load
*/
enum	{
	CHUNKID_PARENT							=	922991750,
	CHUNKID_VARIABLES,
	XXXCHUNKID_C4_TIMER,
	CHUNKID_OWNER,
	CHUNKID_STUCK_OBJECT,

	XXXXMICROCHUNKID_PARAMS_NAME			=	1,
	XXXXXMICROCHUNKID_PARAMS_NAME,
	MICROCHUNKID_AMMO_DEF_ID,
	MICROCHUNKID_DETONATION_MODE,
	MICROCHUNKID_TIMER,
	MICROCHUNKID_STUCK,
	MICROCHUNKID_STUCK_OFFSET,
	MICROCHUNKID_STUCK_MCT,
	MICROCHUNKID_STUCK_BONE,
	MICROCHUNKID_STUCK_STATIC_ANIM_OBJ_ID,
	MICROCHUNKID_STUCK_TO_OBJECT,
	MICROCHUNKID_AGE,
};

bool	C4GameObj::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_PARENT );
	SimpleGameObj::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		int ammo_def_id = AmmoDefinition->Get_ID();
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_AMMO_DEF_ID, ammo_def_id );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DETONATION_MODE,	DetonationMode );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TIMER, Timer );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STUCK, Stuck );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STUCK_OFFSET, StuckOffset );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STUCK_MCT, StuckMCT );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STUCK_BONE, StuckBone );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STUCK_TO_OBJECT, StuckToObject );
		if (StuckStaticAnimObj != NULL) {
			uint32 id = StuckStaticAnimObj->Get_ID();
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STUCK_STATIC_ANIM_OBJ_ID, id);
		}
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_AGE, Age );
	csave.End_Chunk();

	if ( Owner != NULL ) {
		csave.Begin_Chunk( CHUNKID_OWNER );
		Owner.Save( csave );
		csave.End_Chunk();
	}

	if ( StuckObject != NULL ) {
		csave.Begin_Chunk( CHUNKID_STUCK_OBJECT );
		StuckObject.Save( csave );
		csave.End_Chunk();
	}

	return true;
}

bool	C4GameObj::Load( ChunkLoadClass &cload )
{
	REF_PTR_RELEASE(StuckStaticAnimObj);
	uint32 static_anim_obj_id = 0xFFFFFFFF;

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PARENT:
				SimpleGameObj::Load( cload );
				break;
								
			case CHUNKID_VARIABLES:
			{
				int ammo_def_id = 0;

				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_AMMO_DEF_ID, ammo_def_id );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DETONATION_MODE,	DetonationMode );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TIMER,	Timer );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STUCK, Stuck );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STUCK_OFFSET, StuckOffset );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STUCK_MCT, StuckMCT );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STUCK_BONE, StuckBone );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STUCK_STATIC_ANIM_OBJ_ID, static_anim_obj_id);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STUCK_TO_OBJECT, StuckToObject );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_AGE, Age );

						default:
							Debug_Say(( "Unrecognized C4 Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}

				WWASSERT( AmmoDefinition == NULL );
				AmmoDefinition = WeaponManager::Find_Ammo_Definition( ammo_def_id );
				WWASSERT( AmmoDefinition != NULL );
				break;
			}	

			case CHUNKID_OWNER:
				Owner.Load( cload );
				break;

			case CHUNKID_STUCK_OBJECT:
				StuckObject.Load( cload );
				break;

			default:
				Debug_Say(( "Unrecognized C4 chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	if (static_anim_obj_id != 0xFFFFFFFF) {
		StaticPhysClass * pobj = PhysicsSceneClass::Get_Instance()->Get_Static_Object_By_ID(static_anim_obj_id);
		if (pobj && (pobj->As_StaticAnimPhysClass() != NULL)) {
			REF_PTR_SET(StuckStaticAnimObj,(StaticAnimPhysClass *)pobj);
		}
		REF_PTR_RELEASE(pobj);
	}

	return true;
}

void C4GameObj::Think( void )
{
	SimpleGameObj::Think();

	WWPROFILE( "C4 Think" );

	if ( !CombatManager::I_Am_Server() ) {
		return;
	}

	Age += TimeManager::Get_Frame_Seconds();

	int type = AmmoDefinition->AmmoType;

	WWASSERT( type >= AmmoDefinitionClass::AMMO_TYPE_C4_REMOTE );
	WWASSERT( type <= AmmoDefinitionClass::AMMO_TYPE_C4_PROXIMITY );

	Restore_Owner();

	if ( type == AmmoDefinitionClass::AMMO_TYPE_C4_REMOTE ) {
		if ( CombatManager::I_Am_Server() ) {
			// Check for owner to detonate
			SoldierGameObj * owner = Get_Owner();
			if ( owner == NULL ) {
				Defuse();
			} else if ( owner->Detonate_C4() ) {
				Detonate();
			}
		} else {
			Set_Delete_Pending();
		}

	}

	if ( type == AmmoDefinitionClass::AMMO_TYPE_C4_TIMED ) {
		// Check for time's up
		Timer -= TimeManager::Get_Frame_Seconds();
		if ( Timer <= 0 ) {
			Detonate();
		}
	}

	if ( type == AmmoDefinitionClass::AMMO_TYPE_C4_PROXIMITY ) {
		Timer -= TimeManager::Get_Frame_Seconds();
		if ( Timer <= 0 ) {
			// Reset Timer
//			Timer += 1;		// Check every second
			Timer += 0.25;		// Check every 1/4 second

			// Check for smart objs in proximity
			float trigger_range	= AmmoDefinition->C4TriggerRange1;
			if ( DetonationMode == 2 ) 	trigger_range	= AmmoDefinition->C4TriggerRange2;
			if ( DetonationMode == 3 )		trigger_range	= AmmoDefinition->C4TriggerRange3;

			Vector3 c4_pos;
			Get_Position( &c4_pos );
			SLNode<SmartGameObj> * smart_objnode;
			for (smart_objnode = GameObjManager::Get_Smart_Game_Obj_List()->Head(); smart_objnode; smart_objnode = smart_objnode->Next()) {
				SmartGameObj * obj = smart_objnode->Data();
				if ( obj && Is_Enemy( obj ) ) {
					Vector3 obj_pos;
					obj->Get_Position( &obj_pos );
					float range = (obj_pos - c4_pos).Length();
					if ( range <= trigger_range ) {
						Detonate();
					}
				}
			}
		}
	}

}

void C4GameObj::Post_Think( void )
{
	SimpleGameObj::Post_Think();

	WWPROFILE( "C4 Post_Think" );

	// Follow your stuck object
	if ( Stuck ) {
		if ( StuckObject.Get_Ptr() != NULL ) {
			PhysicalGameObj * obj = StuckObject.Get_Ptr()->As_PhysicalGameObj();
			if ( obj ) {
				RenderObjClass * parent_model = obj->Peek_Model();
				Vector3 pos;
				if ( parent_model ) {
					pos = parent_model->Get_Bone_Transform( StuckBone ) * StuckOffset;
				} else {
					pos = obj->Get_Transform() * StuckOffset;
				}
				Set_Position( pos );

				if (obj->As_SoldierGameObj()) {
					bool hide = (obj->As_SoldierGameObj()->Get_Vehicle() != NULL);
					if (Peek_Model()) {
						Peek_Model()->Set_Hidden(hide);
					}
				} 
			}
		
		} else if ( StuckStaticAnimObj != NULL) {
			
			Vector3 pos;
			pos = StuckStaticAnimObj->Peek_Model()->Get_Bone_Transform( StuckBone ) * StuckOffset;
			Set_Position(pos);
			
		} else {
			// Delete without exploding if my object is gone
			if ( StuckToObject ) {
				Set_Delete_Pending();
			}
		}
	}
}

void	C4GameObj::Detonate( void )
{
	if ( CombatManager::I_Am_Server() ) {
		Restore_Owner();

		if ( AmmoDefinition && AmmoDefinition->ExplosionDefID ) {
			int owner_id = 0;
			if ( Get_Owner() ) {
				owner_id = Get_Owner()->Get_ID();
			}
			DamageableGameObj * force_victim = NULL;
			if ( Stuck && StuckToObject ) {
				force_victim = (DamageableGameObj *)StuckObject.Get_Ptr();
			}
			ExplosionManager::Server_Explode( AmmoDefinition->ExplosionDefID, Get_Transform().Get_Translation(), owner_id, force_victim );
		}

		// If I am stuck to a building, apply damage to that building
		if ( Stuck ) {
			if ( StuckObject.Get_Ptr() != NULL ) {
				BuildingGameObj * building = StuckObject.Get_Ptr()->As_BuildingGameObj();
				if ( building ) {
					ExplosionManager::Explosion_Damage_Building( AmmoDefinition->ExplosionDefID, building, StuckMCT, Get_Owner() );
				}
			}
		}
	}

	Set_Delete_Pending();
}

void	C4GameObj::Completely_Damaged( const OffenseObjectClass & /* damager */ )
{
	Defuse();
}

/*
**
*/
void	C4GameObj::Get_Information( StringClass & string )
{
	SimpleGameObj::Get_Information( string );

	int type = AmmoDefinition->AmmoType;
	if ( type == AmmoDefinitionClass::AMMO_TYPE_C4_TIMED ) {
		StringClass temp(0,true);
		temp.Format( "Timer: %1.1f\n", Timer );
		string += temp;
	}
}

/*
**
*/
void	C4GameObj::Export_Rare( BitStreamClass &packet )
{
	SimpleGameObj::Export_Rare( packet );

	int ammo_def_id = 0;
	if ( AmmoDefinition != NULL ) {
		ammo_def_id = AmmoDefinition->Get_ID();
	}
	packet.Add( ammo_def_id );

	int owner_id = 0;
	if (Get_Owner()) {
		owner_id = Get_Owner()->Get_ID();
	}
	packet.Add(owner_id);

	Vector3 pos(0,0,0);
	Vector3 vel(0,0,0);
	ProjectileClass * po = Peek_Physical_Object()->As_ProjectileClass();
	if ( po ) {
		po->Get_Velocity(&vel);
		po->Get_Position(&pos);
	}

	packet.Add(vel.X, BITPACK_VEHICLE_VELOCITY);
	packet.Add(vel.Y, BITPACK_VEHICLE_VELOCITY);
	packet.Add(vel.Z, BITPACK_VEHICLE_VELOCITY);

	//
	// Synchronize the stuck state of C4
	//
	packet.Add(Stuck);
	if (Stuck) {
	
		packet.Add(pos.X, BITPACK_WORLD_POSITION_X);
		packet.Add(pos.Y, BITPACK_WORLD_POSITION_Y);
		packet.Add(pos.Z, BITPACK_WORLD_POSITION_Z);

		packet.Add(StuckMCT);
		packet.Add(StuckToObject);

		int stuck_object_id=0;
		if (StuckObject.Get_Ptr()) {
			stuck_object_id = StuckObject.Get_Ptr()->Get_ID();
		}
		packet.Add(stuck_object_id);
			
		if (StuckToObject) {

			packet.Add(StuckOffset.X, BITPACK_VEHICLE_VELOCITY);	// offset, using velocity packing...
			packet.Add(StuckOffset.Y, BITPACK_VEHICLE_VELOCITY);
			packet.Add(StuckOffset.Z, BITPACK_VEHICLE_VELOCITY);
			packet.Add(StuckBone);
		} 

		bool stuck_static_anim = (StuckStaticAnimObj != NULL);
		packet.Add(stuck_static_anim);
		if (stuck_static_anim) {
			packet.Add(StuckStaticAnimObj->Get_ID());
		}
	}
}


void	C4GameObj::Import_Rare( BitStreamClass &packet )
{
	SimpleGameObj::Import_Rare( packet );

	int ammo_def_id = packet.Get( ammo_def_id );
	if ( ammo_def_id != 0 ) {
		AmmoDefinition = (AmmoDefinitionClass *)DefinitionMgrClass::Find_Definition( ammo_def_id );
		if ( !AmmoDefinition->ModelName.Is_Empty() ) {
			Peek_Physical_Object()->Set_Model_By_Name( AmmoDefinition->ModelName ) ;
		}
	}

	int owner_id = packet.Get(owner_id);
	if (owner_id != 0) {
		Owner = GameObjManager::Find_SmartGameObj(owner_id);
	} else {
		Owner = NULL;
	}

	Vector3 vel;
	packet.Get(vel.X, BITPACK_VEHICLE_VELOCITY);
	packet.Get(vel.Y, BITPACK_VEHICLE_VELOCITY);
	packet.Get(vel.Z, BITPACK_VEHICLE_VELOCITY);

	ProjectileClass * po = Peek_Physical_Object()->As_ProjectileClass();
	if ( po ) {
		po->Set_Velocity(vel);
	}

	//
	// Synchronize the stuck state of C4
	//
	Stuck = packet.Get(Stuck);
	if (Stuck) {

		Peek_Physical_Object()->Enable_User_Control( true );

		// Update the position
		Vector3 pos;
		packet.Get(pos.X, BITPACK_WORLD_POSITION_X);
		packet.Get(pos.Y, BITPACK_WORLD_POSITION_Y);
		packet.Get(pos.Z, BITPACK_WORLD_POSITION_Z);
		ProjectileClass * po = Peek_Physical_Object()->As_ProjectileClass();
		if ( po )  {
			Vector3 local_pos;
			po->Get_Position(&local_pos);
			if ((local_pos - pos).Length2() > 0.5f * 0.5f) {
				po->Set_Position(pos);
			}
		}
		WWDEBUG_SAY(("C4 %p is now STUCK, pos= %f, %f, %f", this, pos.X,pos.Y,pos.Z));

		packet.Get(StuckMCT);
		packet.Get(StuckToObject);

		int stuck_object_id;
		packet.Get(stuck_object_id);
		StuckObject = GameObjManager::Find_ScriptableGameObj(stuck_object_id);

		if (StuckToObject) {
			packet.Get(StuckOffset.X, BITPACK_VEHICLE_VELOCITY);	// offset, using velocity packing...
			packet.Get(StuckOffset.Y, BITPACK_VEHICLE_VELOCITY);
			packet.Get(StuckOffset.Z, BITPACK_VEHICLE_VELOCITY);
			packet.Get(StuckBone);
		} 

		bool stuck_static_anim;
		packet.Get(stuck_static_anim);
		if (stuck_static_anim) {
			
			uint32 static_anim_obj_id = 0;
			packet.Get(static_anim_obj_id);

			if (static_anim_obj_id != 0xFFFFFFFF) {
				StaticPhysClass * pobj = PhysicsSceneClass::Get_Instance()->Get_Static_Object_By_ID(static_anim_obj_id);
				if (pobj && (pobj->As_StaticAnimPhysClass() != NULL)) {
					REF_PTR_SET(StuckStaticAnimObj,(StaticAnimPhysClass *)pobj);
				}
				REF_PTR_RELEASE(pobj);
			}
		}
	}

}


void	C4GameObj::Defuse( void )
{
	if ( CombatManager::I_Am_Server() ) {
		Restore_Owner();
		const AmmoDefinitionClass * disarmed_ammo = WeaponManager::Find_Ammo_Definition( "KilledC4" );
		if ( disarmed_ammo && disarmed_ammo->ExplosionDefID ) {
			int owner_id = 0;
			if ( Get_Owner() ) {
				owner_id = Get_Owner()->Get_ID();
			}
			ExplosionManager::Server_Explode( disarmed_ammo->ExplosionDefID, Get_Transform().Get_Translation(), owner_id );
		}
	}

	Set_Delete_Pending();
}

void	C4GameObj::Restore_Owner( void ) 
{
	if ( Get_Owner() == NULL && OwnerBackup != NULL && 
		AmmoDefinition && (int)AmmoDefinition->AmmoType != (int)AmmoDefinitionClass::AMMO_TYPE_C4_REMOTE ) {
		// Try and find a smart game obj with the same playerdata

		SLNode<SmartGameObj> * smart_objnode;
		for (smart_objnode = GameObjManager::Get_Smart_Game_Obj_List()->Head(); smart_objnode; smart_objnode = smart_objnode->Next()) {
			SmartGameObj * obj = smart_objnode->Data();
			if ( obj->Get_Player_Data() == OwnerBackup ) {
				Owner = obj;
				Debug_Say(( "Found C4 owner\n" ));
			}
		}

		if ( Get_Owner() == NULL ) {
			OwnerBackup = NULL;
			Defuse();
			Debug_Say(( "Didn't find C4 owner\n" ));
		}
	}
}

/*
**
*/

#define	C4_LIMIT			30

void	C4GameObj::Maintain_C4_Limit( int player_type )
{
	if ( !CombatManager::I_Am_Server() || IS_MISSION ) {
		return;
	}

	SLNode<BaseGameObj> *objnode;
	C4GameObj * oldest_c4 = NULL;
	int count = 0;

	for (	objnode = GameObjManager::Get_Game_Obj_List()->Head(); objnode; objnode = objnode->Next()) {
		PhysicalGameObj * phys = objnode->Data()->As_PhysicalGameObj();
		if ( phys ) {
			C4GameObj * c4 = phys->As_C4GameObj();
			if (	c4 && 
					c4->Get_Player_Type() == player_type && 
					c4->AmmoDefinition && 
					(int)c4->AmmoDefinition->AmmoType != (int)AmmoDefinitionClass::AMMO_TYPE_C4_TIMED ) {
				count++;

				if ( (oldest_c4 == NULL) || (c4->Age > oldest_c4->Age) ) {
					oldest_c4 = c4;
				}
			}
		}
	}

	if ( count > C4_LIMIT && oldest_c4 != NULL ) {
		oldest_c4->Defuse();
	}

}
