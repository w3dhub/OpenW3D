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
 *                     $Archive:: /Commando/Code/Combat/weapons.cpp                           $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/27/02 4:43p                                               $*
 *                                                                                             *
 *                    $Revision:: 270                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "weapons.h"
#include "weaponmanager.h"
#include "debug.h"
#include "soldier.h"
#include "combat.h"
#include "gameobjmanager.h"
#include "line3d.h"
#include "physcoltest.h"
#include "pscene.h"
#include "rendobj.h"
#include "quat.h"
#include "combatsound.h"
#include "slnode.h"
#include "scriptman.h"
#include "WWAudio.h"
#include "Sound3D.h"
#include "chunkio.h"
#include "assets.h"
#include "bullet.h"
#include "crandom.h"
#include "damage.h"
#include "ccamera.h"
#include "rbody.h"
#include "timeddecophys.h"
#include "wwphysids.h"
#include "part_emt.h"
#include "vehicle.h"
#include "c4.h"
#include "objlibrary.h"
#include "wwprofile.h"
#include "hudinfo.h"
#include "projectile.h"
#include "combatphysobserver.h"
#include "surfaceeffects.h"
#include "beacongameobj.h"
#include "weaponview.h"
#include "diaglog.h"
#include "playerdata.h"
#include "cheatmgr.h"
#include <algorithm>


/*
**
*/
#define	RANDOM_VECTOR( spread )		Vector3( FreeRandom.Get_Float( -(spread), (spread) ),	\
															FreeRandom.Get_Float( -(spread), (spread) ),	\
															FreeRandom.Get_Float( -(spread), (spread) ) )


/**
** EjectCasingObserverClass
** This is a class which observes all eject casings and creates sound effects when they
** collide.  Since the task of this observer is so simple, I am creating a single static
** instance of it and pluging it into all eject casings.
*/
class EjectCasingObserverClass : public CombatPhysObserverClass
{
public:
	virtual CollisionReactionType		Collision_Occurred(const CollisionEventClass & /* event */) override
	{
/*
		SurfaceEffectsManager::Apply_Effect(event.CollisionResult->SurfaceType,
														SurfaceEffectsManager::HITTER_TYPE_EJECT_CASING,
														Matrix3D(event.CollisionResult->ContactPoint),
														event.OtherObj, NULL, false);
*/
		return COLLISION_REACTION_DEFAULT;
	}
};


static EjectCasingObserverClass _TheEjectCasingObserver;






/*
**
*/
WeaponClass::WeaponClass( const WeaponDefinitionClass *def ) :
	Definition( NULL ),
	PrimaryAmmoDefinition( NULL ),
	SecondaryAmmoDefinition( NULL ),
	Model( NULL ),
	State( STATE_IDLE ),
	StateTimer( 0.0f ),
	UpdateModel( WEAPON_MODEL_UPDATE_IS_NEEDED ),
	NextAnimState( WEAPON_ANIM_NOT_FIRING ),
	CurrentAnimState( WEAPON_ANIM_NOT_FIRING ),
	LastFrameIsPrimaryTriggered( false ),
	LastFrameIsSecondaryTriggered( false ),
	IsPrimaryTriggered( false ),
	IsSecondaryTriggered( false ),
	TotalRoundsFired( 0 ),
	ClipRounds( 0 ),
	InventoryRounds( 0 ),
	BurstDelayTimer( 0 ),
	BurstCount( 0 ),
	BulletBumpTime( 0 ),
	DidFire( false ),
	ContinuousEmitters( ),
	ContinuousSound( NULL ),
	C4DetonationMode( 1 ),
	Target( 0, 0, 0 ),
	FiringSound( NULL ),
	FiringSoundDefID( 0 ),
	WeaponExists( true ),
	SafetySet( false ),
	LockTriggers( false ),
	EmptySoundTimer( 0 )
{
	if ( def != NULL ) {
		Init( def );
	}
}

WeaponClass::~WeaponClass( void )
{
	if (FiringSound != NULL) {
		FiringSound->Remove_From_Scene ();
		FiringSoundDefID = 0;
	}

	REF_PTR_RELEASE( Model );
	REF_PTR_RELEASE( FiringSound );

	Do_Continuous_Effects( false );
}

void	WeaponClass::Init( const WeaponDefinitionClass *weapon_def )
{
	WWASSERT( Definition == NULL );
	Definition = weapon_def;
	WWASSERT( Definition != NULL );

	// Setup the ammo defs
	WWASSERT( Definition->PrimaryAmmoDefID != 0 );
	PrimaryAmmoDefinition = WeaponManager::Find_Ammo_Definition( Definition->PrimaryAmmoDefID );

	WWASSERT( Definition->SecondaryAmmoDefID != 0 );
	SecondaryAmmoDefinition = WeaponManager::Find_Ammo_Definition( Definition->SecondaryAmmoDefID );
}

/*
** Weapon Save and Load
*/

enum	{
	CHUNKID_VARIABLES						=	910991544,
	XXX_CHUNKID_AMMO,
	CHUNKID_OWNER_REF,
	XXXCHUNKID_TARGET_REF,
	XXXCHUNKID_DESIRED_TARGET_REF,
	CHUNKID_TARGET_OBJECT,

	MICROCHUNKID_IN_FLY_MODE			=	1,
	MICROCHUNKID_STATE,
	MICROCHUNKID_STATE_TIMER,
	MICROCHUNKID_IS_PRIMARY_TRIGGERED,
	XXXMICROCHUNKID_IS_SNIPING,
	MICROCHUNKID_ROUNDS_FIRED,
	MICROCHUNKID_NEXT_ANIM_STATE,
	MICROCHUNKID_CURR_ANIM_STATE,
	MICROCHUNKID_UPDATE_MODEM,
	XXXXMICROCHUNKID_LAST_MUZZLE,
	XXXMICROCHUNKID_PENDING_TIMER,
	XXXMICROCHUNKID_TARGETING_MUZZLE,
	XXXMICROCHUNKID_TARGET_POSITION,
	XXXMICROCHUNKID_WEAPON_PARAMS_NAME,
	XXXMICROCHUNKID_MODEL_NAME,
	MICROCHUNKID_MODEL_PTR,
	XXXMICROCHUNKID_DYNAMIC_ERROR_ANGLE,
	MICROCHUNKID_CLIP_ROUNDS,
	MICROCHUNKID_INVENTORY_ROUNDS,
	MICROCHUNKID_BURST_TIMER,
	MICROCHUNKID_BURST_COUNT,
	XXX_MICROCHUNKID_AMMO_PARAMS_NAME,
	XXXMICROCHUNKID_AMMO_DEFINITION_NAME,
	XXXMICROCHUNKID_DEFINITION_NAME,
	XXXMICROCHUNKID_AMMO_DEFINITION_ID,
	MICROCHUNKID_DEFINITION_ID,
	MICROCHUNKID_IS_SECONDARY_TRIGGERED,
	MICROCHUNKID_C4_DETONATION_MODE,
	MICROCHUNKID_TARGET_LOCATION,
	MICROCHUNKID_WEAPON_EXISTS,
	MICROCHUNKID_SAFETY_SET,
	XXXMICROCHUNKID_OWNER_ERROR_ANGLE,
};

bool	WeaponClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_VARIABLES );

		if ( Model ) {
#if 0
			csave.Begin_Micro_Chunk( MICROCHUNKID_MODEL_NAME );
			const char * model_name = Model->Get_Name();
			csave.Write( model_name, strlen( model_name ) + 1 );
			csave.End_Micro_Chunk();
#endif
			WRITE_MICRO_CHUNK_PTR( csave, MICROCHUNKID_MODEL_PTR, Model );
		}
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STATE, State );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STATE_TIMER, StateTimer );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_IS_PRIMARY_TRIGGERED, IsPrimaryTriggered );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_IS_SECONDARY_TRIGGERED, IsSecondaryTriggered );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ROUNDS_FIRED, TotalRoundsFired );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_NEXT_ANIM_STATE, NextAnimState );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_CURR_ANIM_STATE, CurrentAnimState );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_UPDATE_MODEM, UpdateModel );
		WRITE_SAFE_MICRO_CHUNK( csave, MICROCHUNKID_CLIP_ROUNDS, ClipRounds, int );
		WRITE_SAFE_MICRO_CHUNK( csave, MICROCHUNKID_INVENTORY_ROUNDS, InventoryRounds, int );
		WRITE_SAFE_MICRO_CHUNK( csave, MICROCHUNKID_BURST_TIMER, BurstDelayTimer, float );
		WRITE_SAFE_MICRO_CHUNK( csave, MICROCHUNKID_BURST_COUNT, BurstCount, int );
		int def_id = Definition->Get_ID();
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEFINITION_ID, def_id );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_C4_DETONATION_MODE, C4DetonationMode );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TARGET_LOCATION, Target );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_WEAPON_EXISTS, WeaponExists );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_SAFETY_SET, SafetySet );

	csave.End_Chunk();

	if ( Owner != NULL ) {
		csave.Begin_Chunk( CHUNKID_OWNER_REF );
		Owner.Save( csave );
		csave.End_Chunk();
	}

	if ( TargetObject != NULL ) {
		csave.Begin_Chunk( CHUNKID_TARGET_OBJECT );
		TargetObject.Save( csave );
		csave.End_Chunk();
	}


	// Don't need to save DidFire
	// Don't need to save ContinuousEmitters;
	// Don't need to save ContinuousSound;
	// Don't need to save FiringSound;
	// Dont need to save BulletBumpTime

	return true;
}

bool	WeaponClass::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_VARIABLES:
			{
				int def_id = 0;

				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STATE, State );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STATE_TIMER, StateTimer );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_IS_PRIMARY_TRIGGERED, IsPrimaryTriggered );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_IS_SECONDARY_TRIGGERED, IsSecondaryTriggered );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ROUNDS_FIRED, TotalRoundsFired );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_NEXT_ANIM_STATE, NextAnimState );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_CURR_ANIM_STATE, CurrentAnimState );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_UPDATE_MODEM, UpdateModel );
						READ_SAFE_MICRO_CHUNK( cload, MICROCHUNKID_CLIP_ROUNDS, ClipRounds, int );
						READ_SAFE_MICRO_CHUNK( cload, MICROCHUNKID_INVENTORY_ROUNDS, InventoryRounds, int );
						READ_SAFE_MICRO_CHUNK( cload, MICROCHUNKID_BURST_TIMER, BurstDelayTimer, float );
						READ_SAFE_MICRO_CHUNK( cload, MICROCHUNKID_BURST_COUNT, BurstCount, int );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEFINITION_ID, def_id );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_C4_DETONATION_MODE, C4DetonationMode );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TARGET_LOCATION, Target );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_WEAPON_EXISTS, WeaponExists );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_SAFETY_SET, SafetySet );

#if 0
						case XXX_MICROCHUNKID_MODEL_NAME:
						{
							WWASSERT( Model == NULL );
							char	model_name[80];
							cload.Read( model_name, cload.Cur_Micro_Chunk_Length() );
							RenderObjClass * robj = WW3DAssetManager::Get_Instance()->Create_Render_Obj( model_name );

							Set_Model( robj );

							cload.Close_Micro_Chunk();
							cload.Open_Micro_Chunk();
							WWASSERT( cload.Cur_Micro_Chunk_ID() == MICROCHUNKID_MODEL_PTR );
							void * old_ptr = NULL;
							cload.Read( &old_ptr, sizeof( uint32 ) );
							SaveLoadSystemClass::Register_Pointer( old_ptr, robj );
							REF_PTR_RELEASE( robj );
							break;
						}
#endif

						READ_MICRO_CHUNK_PTR( cload, MICROCHUNKID_MODEL_PTR, Model );

						default:
							Debug_Say(("Unhandled Micro Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
							break;
					}
					cload.Close_Micro_Chunk();
				}

				Init( WeaponManager::Find_Weapon_Definition( def_id ) );

				break;
			}

			case CHUNKID_OWNER_REF:
				Owner.Load( cload );
				break;

			case CHUNKID_TARGET_OBJECT:
				TargetObject.Load( cload );
				break;

			default:
				Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}
		cload.Close_Chunk();
	}

	if (Model != NULL) {
		REQUEST_REF_COUNTED_POINTER_REMAP ((RefCountClass **)&Model);
	}

	// Legacy
	if ( ClipRounds == 0 && InventoryRounds < 0 ) {
		ClipRounds = Definition->ClipSize;
	}

	return true;
}


void	WeaponClass::Set_Owner( ArmedGameObj *owner )
{
	Owner = owner;
}

void	WeaponClass::Set_Target_Object( PhysicalGameObj * obj )
{
	TargetObject = obj;
}

PhysicalGameObj *	WeaponClass::Get_Target_Object( PhysicalGameObj * /* obj */ )
{
	return ( PhysicalGameObj * )TargetObject.Get_Ptr();
}

void	WeaponClass::Set_Model( RenderObjClass *model )
{
	if ( Model != NULL ) {
		Model->Release_Ref();
	}
	Model = model;
	if ( Model != NULL ) {
		Model->Add_Ref();
		Init_Muzzle_Flash( Model );
	}
}

void	WeaponClass::Select( void )
{
//	Debug_Say(( "Weapon Selected\n" ));
	Set_State( STATE_START_SWITCH );
	UpdateModel = WEAPON_MODEL_UPDATE_WILL_BE_NEEDED;
}

void	WeaponClass::Deselect( void )
{
	Do_Continuous_Effects( false );
	IsPrimaryTriggered = false;
	IsSecondaryTriggered = false;
	LastFrameIsPrimaryTriggered = false;
	LastFrameIsSecondaryTriggered = false;
}

void	WeaponClass::Next_C4_Detonation_Mode( void )
{
#define	LAST_C4_DETONATION_MODE	3
	if ( ++C4DetonationMode > LAST_C4_DETONATION_MODE ) {
		C4DetonationMode = 1;
	}
	Debug_Say(( "Weapon Next C4 Detonation Mode %d\n", C4DetonationMode ));
}


/*
**
*/
void	WeaponClass::Set_Primary_Triggered( bool triggered )
{
	if ( !LockTriggers ) {
		IsPrimaryTriggered = triggered;
	}
}

void	WeaponClass::Set_Secondary_Triggered( bool triggered )
{
	if ( !LockTriggers ) {
		IsSecondaryTriggered = triggered;
	}
}

/*
** AMMO
*/
void	WeaponClass::Add_Rounds( int num )
{
	// If we have an empty clip ( first time )  load directly
	if ( ClipRounds == 0 ) {
		if ( num < 0 ) {
			ClipRounds = Definition->ClipSize;
		} else if ( InventoryRounds >= 0 )  {
			int count = MIN( num, (int)Definition->ClipSize );
			num -= count;
			ClipRounds = count;
		}
	}

	if (	CheatMgrClass::Get_Instance () != NULL &&
			CheatMgrClass::Get_Instance ()->Is_Cheat_Enabled( CheatMgrClass::CHEAT_INFINITE_AMMO ) && Owner == COMBAT_STAR )
	{
		InventoryRounds = Definition->MaxInventoryRounds ;
	} else if ( num < 0 ) {
		InventoryRounds = -1;
	} else if ( InventoryRounds >= 0 )  {
		InventoryRounds += num;
	}

	if ( (InventoryRounds != -1) && (InventoryRounds > (int)Definition->MaxInventoryRounds) ) {
		InventoryRounds = Definition->MaxInventoryRounds ;
	}

}

int	WeaponClass::Get_Total_Rounds( void )
{
	if ( InventoryRounds < 0 ) {
		return -1;
	}

	//
	//	Check to see if the infinite ammo cheat is enabled
	//
	if (	CheatMgrClass::Get_Instance () != NULL &&
			CheatMgrClass::Get_Instance ()->Is_Cheat_Enabled( CheatMgrClass::CHEAT_INFINITE_AMMO ) && Owner == COMBAT_STAR )
	{
		return -1;
	}

	return ClipRounds + InventoryRounds;
}

void	WeaponClass::Set_Total_Rounds( int num )
{
	if (	CheatMgrClass::Get_Instance () != NULL &&
			CheatMgrClass::Get_Instance ()->Is_Cheat_Enabled( CheatMgrClass::CHEAT_INFINITE_AMMO ) && Owner == COMBAT_STAR )
	{
		InventoryRounds = Definition->MaxInventoryRounds;
	} else if ( num == -1 ) {
		InventoryRounds = -1;
	} else {
		// Assume what you have in inventory is correct
		num -= (int)InventoryRounds;
		// And the rest goes in the clip
		ClipRounds = num;

		// Fix a negative clip
		if ( ClipRounds < 0 ) {
			InventoryRounds += ClipRounds;
			ClipRounds = 0;
		}

		// Fix overloaded clip
		if ( ClipRounds > Definition->ClipSize ) {
			InventoryRounds += ClipRounds - Definition->ClipSize;
			ClipRounds -= ClipRounds - Definition->ClipSize;
		}

		// Fix Overloaded inventory
		if ( InventoryRounds > (int)Definition->MaxInventoryRounds ) {
			InventoryRounds = Definition->MaxInventoryRounds;
		}
	}
}

bool	WeaponClass::Is_Ammo_Maxed( void )
{
	if (InventoryRounds == -1) return true;

	// Special case for C4
	if (((int)Definition->MaxInventoryRounds == 0) && (ClipRounds == 0)) return false;

	return (InventoryRounds == (int)Definition->MaxInventoryRounds);
}

void	WeaponClass::Decrement_Rounds( int rounds )
{
	if ( ClipRounds != -1 ) {
		ClipRounds -= rounds;
		if ( ClipRounds <= 0 ) {
			ClipRounds = 0;
		}
	}
}

void	WeaponClass::Do_Reload( void )
{
	int added = (int)Definition->ClipSize - (int)ClipRounds;

	//
	//	Check to see if the infinite ammo cheat is enabled
	//
	bool apply_cheat = false;
	if (	CheatMgrClass::Get_Instance () != NULL &&
			CheatMgrClass::Get_Instance ()->Is_Cheat_Enabled( CheatMgrClass::CHEAT_INFINITE_AMMO ) && Owner == COMBAT_STAR )
	{
		apply_cheat = true;
	}

	if ( apply_cheat == false && InventoryRounds >= 0 ) {

		if ( InventoryRounds < added ) {
			added = InventoryRounds;
		}

		InventoryRounds -= added;
	}

	ClipRounds += added;
}

float	WeaponClass::Get_Range( void )
{
	return ( PrimaryAmmoDefinition != NULL ) ? static_cast<float>(PrimaryAmmoDefinition->Range) : 0.0f;
}


/*
**	Fire_C4
*/
void	WeaponClass::Fire_C4( const AmmoDefinitionClass *ammo_def )
{
	#define		C4_OBJECT_NAME				"Tossed C4"

	// C4 special case
	C4GameObj *c4 = (C4GameObj *)ObjectLibraryManager::Create_Object( C4_OBJECT_NAME );
	if ( c4 ) {
		c4->Init_C4( ammo_def, Get_Owner()->As_SoldierGameObj(), C4DetonationMode, Get_Muzzle() );
	}

	return ;
}


/*
**	Fire_Beacon
*/
bool	WeaponClass::Fire_Beacon( const AmmoDefinitionClass *ammo_def )
{
	bool retval = false;

	// only for server
	if ( CombatManager::I_Am_Server() ) {
		//
		//	Create the beacon
		//
		BeaconGameObj *beacon = (BeaconGameObj *)ObjectLibraryManager::Create_Object( ammo_def->BeaconDefID );
		if ( beacon != NULL ) {

			//
			//	Get the position of the owner
			//
			Vector3 pos;
			Get_Owner()->Get_Position( &pos );
			beacon->Init_Beacon( Definition, Get_Owner()->As_SoldierGameObj(), pos );
			beacon->Start_Observers ();

			//
			//	Begin arming the beacon
			//
			if ( beacon->Can_Place_Here ( pos ) ) {
				beacon->Begin_Arming();
				retval = true;
			} else {
				beacon->Set_Delete_Pending ();
			}
		}
	}

	return retval;
}


/*
**
*/
void	WeaponClass::Do_Fire( bool primary )
{
	WWPROFILE( "Do Fire" );

	// Stats
	if (	Get_Owner() && 
			Get_Owner()->As_SoldierGameObj() && 
			Get_Owner()->As_SoldierGameObj()->Get_Player_Data() ) {
		Get_Owner()->As_SoldierGameObj()->Get_Player_Data()->Stats_Add_Shot_Fired();

		Get_Owner()->As_SoldierGameObj()->Get_Player_Data()->Stats_Add_Weapon_Fired( 
			this->Get_Definition()->Get_ID() );

	}

	//	Debug_Say(( "Fire at %f\n", (float)WW3D::Get_Sync_Time() ));

	const AmmoDefinitionClass *ammo_def;
	if ( primary ) {
		ammo_def = PrimaryAmmoDefinition;
	} else {
		ammo_def = SecondaryAmmoDefinition;
	}

//	Debug_Say(( "Firing from %f %f %f\n", muzzle_pos.X, muzzle_pos.Y, muzzle_pos.Z ));
//	Debug_Say(( "Firing at %f %f %f\n", target_pos.X, target_pos.Y, target_pos.Z ));
//	Debug_Say(( " %d Clips  %d in Clip\n", NumClips, ClipContents ));

	//
	//	Handle each weapon type differently
	//
	if ( Get_Style() == WEAPON_HOLD_STYLE_BEACON ) {

		//
		//	Try to fire the beacon
		//
		if ( Fire_Beacon( ammo_def ) ) {

			//
			//	Decrement the rounds
			//
//			WWASSERT( BurstCount != 0 );
			BurstCount--;
//			WWASSERT( ClipRounds != 0 );
			Decrement_Rounds( ammo_def->SprayBulletCost );
		}

	} else {

		//
		//	Decrement the rounds
		//
//		WWASSERT( BurstCount != 0 );
		BurstCount--;
//		WWASSERT( ClipRounds != 0 );
		Decrement_Rounds( ammo_def->SprayBulletCost );

		if ( Get_Style() == WEAPON_HOLD_STYLE_C4 ) {
			if ( CombatManager::I_Am_Server() ) {	// Only on servers
				Fire_C4( ammo_def );
			}
		} else {
			Fire_Bullet( ammo_def, primary );
		}
	}

	return ;
}

/*
**	Fire_Bullet
*/
void	WeaponClass::Fire_Bullet( const AmmoDefinitionClass *ammo_def, bool primary )
{
	WWPROFILE( "Fire Bullet" );

	if ( Get_Owner() == COMBAT_STAR ) {
		Vector3 pos;
		COMBAT_STAR->Get_Position( &pos );
	   DIAG_LOG(( "WFRD", "%1.2f; %1.2f; %1.2f; %s; %d; %d", pos.X, pos.Y, pos.Z, Get_Definition()->Get_Name(), !primary, Get_Total_Rounds() ));
	}

	int spray_count = ammo_def->SprayCount;
	if ( spray_count < 1 ) {
		spray_count = 1;
	}

	int muzzle_index = (Get_Total_Rounds_Fired() & 1) + ( primary ? 0 : 2 );

	if ( !Get_Owner()->Muzzle_Exists( muzzle_index ) ) {
		// if the second muzzle doesn't exist, use the first
		if ( muzzle_index & 1 ) {
			muzzle_index--;
		}
	}

	Matrix3D muzzle = Get_Muzzle( muzzle_index );

	// Special test for first person
	if ( CombatManager::Is_First_Person() && Get_Owner() == COMBAT_STAR ) {
		Vector3	fp_muzzle_pos = WeaponViewClass::Get_Muzzle_Pos();

		// cast to see what we hit
		Vector3 start = fp_muzzle_pos;
		Vector3 end = Get_Owner()->Get_Targeting_Pos();
		LineSegClass ray( start, end );
		CastResultStruct res;
		PhysRayCollisionTestClass raytest(ray,&res,BULLET_COLLISION_GROUP,COLLISION_TYPE_PROJECTILE);

		Ignore_Owner();

{		WWPROFILE( "Cast_Ray" );
		COMBAT_SCENE->Cast_Ray( raytest );
}
		Unignore_Owner();

		// If the hit point is near the targeting_pos, use the first person muzzle
		if ( raytest.Result->Fraction >= 0.99F ) {
			muzzle.Obj_Look_At( fp_muzzle_pos, Get_Owner()->Get_Targeting_Pos(), 0 );
		}
	}

	Vector3 bullet_target(0,0,0);
	DamageableGameObj * target_obj = NULL;

	if ( ammo_def->IsTracking ) {
		// determine the target
		Vector3 start = muzzle.Get_Translation();

		// (gth) this modification makes bikes able to target anything even though their muzzle doesn't rotate...
		Vector3 end = Get_Owner()->Get_Targeting_Pos(); //muzzle * Vector3( ammo_def->Range, 0, 0 );

		LineSegClass ray( start, end );
		CastResultStruct res;
		PhysRayCollisionTestClass raytest(ray,&res,BULLET_COLLISION_GROUP,COLLISION_TYPE_PROJECTILE);

		Ignore_Owner();

{ WWPROFILE( "Cast Ray" );
		COMBAT_SCENE->Cast_Ray( raytest );
}

		Unignore_Owner();

		ray.Compute_Point( raytest.Result->Fraction, &end );
		bullet_target = end;

		if ( raytest.CollidedPhysObj != NULL && raytest.CollidedPhysObj->Get_Observer() != NULL ) {
			target_obj = ((CombatPhysObserverClass *)raytest.CollidedPhysObj->Get_Observer())->As_DamageableGameObj();
			// No homing on buildings
			if ( target_obj->As_BuildingGameObj() != NULL ) {
				target_obj = NULL;
			}
		}
	}

	for ( int count = 0; count < spray_count; count++ ) {

		Vector3 velocity = muzzle.Get_X_Vector() * ammo_def->Velocity;


#if 0
		if ( ammo_def->SprayAngle ) {
			// Use Spray Angle
			Vector3	spray_adjust = RANDOM_VECTOR( 1.0 );
			spray_adjust.Normalize();
			spray_adjust = spray_adjust * ammo_def->Velocity;
			spray_adjust *= ammo_def->SprayAngle / (float)(2*WWMATH_PI);

			velocity += spray_adjust;
			velocity.Normalize();
			velocity = velocity * ammo_def->Velocity;
		}

		if ( DynamicErrorAngle ) {
			velocity += RANDOM_VECTOR( DynamicErrorAngle / 2 );
			velocity.Normalize();
			velocity = velocity * ammo_def->Velocity;
		}
#else
		Matrix3D	VelTM;
		// Look at backwards with random roll
		VelTM.Look_At( velocity, Vector3( 0,0,0 ), FreeRandom.Get_Float( DEG_TO_RADF( 360.0f ) ) );
#define	SPRAY_CHEAT	0
#if SPRAY_CHEAT
		float error_angle = 0;
#else
		float error_angle = ammo_def->SprayAngle;
#endif
		float rand_angle = FreeRandom.Get_Float() * error_angle;
		VelTM.Rotate_Y( rand_angle );
		velocity = VelTM.Get_Z_Vector();
		velocity.Normalize();
		velocity *= ammo_def->Velocity;
#endif

		//Vector3 position = muzzle.Get_Translation();
		Vector3 position;
		Compute_Bullet_Start_Point(muzzle,&position);

		BulletBumpTime += ammo_def->AliasedSpeed * TimeManager::Get_Frame_Seconds() * 1.3f;
		BulletBumpTime = WWMath::Wrap( BulletBumpTime, 0, TimeManager::Get_Frame_Seconds() );

		float bump_time = BulletBumpTime;

		if ( (float)ammo_def->Velocity != 0 ) {
			bump_time += 1 / (float)ammo_def->Velocity;  // Skip the first meter
		}

		//Debug_Say(( "%f %f\n", BulletBumpTime,bump_time ));

		if ( Target.Length2() != 0 || !ammo_def->IsTracking ) {
			bullet_target = Target;
		}

		// Bullets from vehicles belong to the gunner
		ArmedGameObj * bullet_owner = Get_Owner();
		if ( bullet_owner != NULL && bullet_owner->As_SmartGameObj() ) {
			VehicleGameObj * vehicle = bullet_owner->As_SmartGameObj()->As_VehicleGameObj();
			if ( ( vehicle ) && ( vehicle->Get_Actual_Gunner() != NULL ) ) {
				bullet_owner = vehicle->Get_Actual_Gunner();
			}
		}

		// Create a bullet
		BulletManager::Create_Bullet( ammo_def, position, velocity, bullet_owner, bump_time, bullet_target, target_obj );
	}

	if ( ammo_def->DisplayLaser ) {
		TimedDecorationPhysClass * laser_obj = NEW_REF( TimedDecorationPhysClass, () );
		if ( laser_obj ) {

			// Find where the laser should stop
			Vector3 start = muzzle.Get_Translation();
			Vector3 end = muzzle * Vector3( ammo_def->Range, 0, 0 );
			LineSegClass ray( start, end );
			CastResultStruct res;
			PhysRayCollisionTestClass raytest(ray,&res,BULLET_COLLISION_GROUP,COLLISION_TYPE_PROJECTILE);

			Ignore_Owner();

{ WWPROFILE( "Cast Ray" );
			COMBAT_SCENE->Cast_Ray( raytest );
}

			Unignore_Owner();

			ray.Compute_Point( raytest.Result->Fraction, &end );

			// Create a laser
			RenderObjClass * model = NEW_REF( Line3DClass, ( start, end, 0.1f,  0.8f,0,0,  0.8f ) );
			laser_obj->Set_Model( model );
			model->Release_Ref();
			laser_obj->Set_Lifetime( 0.6f );
			laser_obj->Update_Cull_Box();	// htis may be needed due to a bug in Line3DClass
			COMBAT_SCENE->Add_Dynamic_Object( laser_obj );
			laser_obj->Release_Ref();
		}
	}

	// Initiate recoil effects on our parent object.
	Get_Owner()->Start_Recoil(muzzle_index,Get_Recoil_Scale(),Get_Recoil_Time());

	TotalRoundsFired++;
}


bool	WeaponClass::Is_Muzzle_Clear()
{
	int primary = 1;
	int muzzle_index = Get_Total_Rounds_Fired() & 1 + ( primary ? 0 : 2 );
	Matrix3D muzzle = Get_Muzzle( muzzle_index );

	Vector3 start_pt;
	Vector3 end_pt;
	muzzle.Get_Translation( &end_pt );

	if ( Get_Owner() == NULL ) {
		return true;
	}

	if ((Get_Owner() == COMBAT_STAR) && CombatManager::Is_First_Person()) {
		return true;
	}

	if (Get_Owner()->As_SoldierGameObj() != NULL) {
	
		start_pt = Get_Owner()->Get_Bullseye_Position();

	} else {

		// Estimate the distance we need to sweep back along the muzzle in order to be inside the vehicle.
		Vector3 owner_pos;
		Get_Owner()->Get_Position( &owner_pos );
		owner_pos -= end_pt;
		owner_pos.Z = 0.0f;
		float dist = owner_pos.Quick_Length();

		// Compute the starting point for a ray to test if the muzzle is inside anything.
		Vector3 offset;
		Matrix3D::Rotate_Vector(muzzle,Vector3(-dist,0.0f,0.0f),&offset);
		start_pt = end_pt + offset;

	}

	// Cast the ray
	LineSegClass ray( start_pt, end_pt );
	CastResultStruct res;
	PhysRayCollisionTestClass raytest(ray,&res,BULLET_COLLISION_GROUP,COLLISION_TYPE_PROJECTILE);
	raytest.CheckDynamicObjs = false;

	{ 
		WWPROFILE( "Cast Ray" );
		COMBAT_SCENE->Cast_Ray( raytest );
	}

	return raytest.Result->Fraction == 1.0f;
}

void WeaponClass::Compute_Bullet_Start_Point(const Matrix3D & muzzle,Vector3 * set_start_point)
{
	//
	// The default behavior is to start the bullet at the position of the muzzle.
	//
	muzzle.Get_Translation(set_start_point);

	// 
	// If this weapon is owned by a vehicle, try to ensure the bullet starts in front
	// of any dynamic objects that may be between the muzzle point and the body of the vehicle
	//
	if (Get_Owner()->As_VehicleGameObj() != NULL) {
		Vector3 ray_start;
		Vector3 ray_end;
		muzzle.Get_Translation( &ray_end );

		// Estimate the distance we need to sweep back along the muzzle in order to be inside the vehicle.
		Vector3 owner_pos;
		Get_Owner()->Get_Position( &owner_pos );
		owner_pos -= ray_end;
		owner_pos.Z = 0.0f;
		float dist = owner_pos.Quick_Length();

		// Compute the starting point for a ray to test if the muzzle is inside anything.
		Vector3 offset;
		Matrix3D::Rotate_Vector(muzzle,Vector3(-dist,0.0f,0.0f),&offset);
		ray_start = ray_end + offset;

		// Cast the ray
		LineSegClass ray( ray_start, ray_end );
		CastResultStruct res;
		PhysRayCollisionTestClass raytest(ray,&res,BULLET_COLLISION_GROUP,COLLISION_TYPE_PROJECTILE);
		raytest.CheckStaticObjs = false;

		{ 
			Ignore_Owner();

			WWPROFILE( "Cast Ray" );
			COMBAT_SCENE->Cast_Ray( raytest );

			Unignore_Owner();
		}

		if (res.Fraction < 1.0f) {
			ray.Compute_Point(res.Fraction,set_start_point);
		}
	}
}

void	WeaponClass::Do_Firing_Effects( void )
{
	if ( TimeManager::Get_Frame_Seconds() == 0 ) {
		return;	// No sounds when time stops
	}

	WWPROFILE( "Firing Effects" );

	Matrix3D	muzzle = Get_Muzzle();

	// Don't do these two if fired by the star in First Person Mode
	if ( (Get_Owner() != COMBAT_STAR) || !CombatManager::Is_First_Person() ) {

#if 01
		// create muzzle flash
		if ( Definition->MuzzleFlashPhysDefID != 0 ) {

			DefinitionClass * def = DefinitionMgrClass::Find_Definition( Definition->MuzzleFlashPhysDefID );
			if ( def != NULL ) {
				WWASSERT( ((PhysDefClass *)def)->Is_Type( "TimedDecorationPhysDef" ) );
				TimedDecorationPhysClass * muzzle_flash = (TimedDecorationPhysClass *)def->Create();
				if ( muzzle_flash ) {
					RenderObjClass * model = muzzle_flash->Peek_Model();
					if ( model != NULL ) {
//						muzzle.Rotate_X( FreeRandom.Get_Float( DEG_TO_RAD( 360 ) ) );
						muzzle_flash->Set_Transform( muzzle );
						COMBAT_SCENE->Add_Dynamic_Object( muzzle_flash );

					// If this is an emitter, start it
						if ( model->Class_ID() == RenderObjClass::CLASSID_PARTICLEEMITTER ) {
							ParticleEmitterClass * pe = (ParticleEmitterClass *)model;
							pe->Start();
						}
					} else {
						Debug_Say(( "Missing muzzle flash for %s\n", Definition->Get_Name() ));
					}

					muzzle_flash->Release_Ref();
				}
			} else {
				//Debug_Say(( "Couldn't find muzzle flash def for %s\n", Definition->Get_Name() ));
			}
		}
#endif

		// if this gun is fired by the STAR and they have an eject bone, eject a shell.
		if ((Get_Owner() == COMBAT_STAR) && (Model != NULL)) {
			WWPROFILE( "Eject" );
			int eject_index = Model->Get_Bone_Index( "eject" );
			if ( eject_index > 0 ) {
				Make_Shell_Eject( Model->Get_Bone_Transform( eject_index ) );
			}
		}
	}

	// Make the Gunshot sound

	//
	//	Determine which sound to play, the primary or secondary firing sound?
	//
	int sound_id = 0;
	if ( IsPrimaryTriggered && PrimaryAmmoDefinition != NULL ) {
		sound_id = PrimaryAmmoDefinition->FireSoundDefID;
	} else if ( IsSecondaryTriggered && SecondaryAmmoDefinition != NULL ) {
		sound_id = SecondaryAmmoDefinition->FireSoundDefID;
	}

	//
	//	Release the old firing sound (if necessary)
	//
	bool release_curr_sound = false;
	if ( sound_id != FiringSoundDefID && FiringSound != NULL ) {
		release_curr_sound = true;
	}

	//
	//	For first-person we make the weapon sounds "2D", so check to see
	// if we need to re-create the sound...
	//
	if ( FiringSound != NULL ) {
		if ( Get_Owner() == COMBAT_STAR && CombatManager::Is_First_Person() ) {
			
			//
			//	In first person, we need the sound to be "2D"
			//
			if ( FiringSound->Get_Class_ID () != CLASSID_2D ) {
				release_curr_sound = true;
			}
		} else {

			//
			//	In third person, we need the sound to be "3D"
			//
			if ( FiringSound->Get_Class_ID () == CLASSID_2D ) {
				release_curr_sound = true;
			}
		}
	}

	//
	//	Release the currently playing sound as necessary
	//	
	if ( release_curr_sound ) {		
		FiringSound->Remove_From_Scene ();
		REF_PTR_RELEASE (FiringSound);
		FiringSoundDefID = 0;
	}

	//
	//	Do we need to recreate the firing sound object, or can we just
	// reuse the one we already have?
	//
	if ( FiringSound != NULL ) {

		//
		// Stop the current sound
		//
   	//FiringSound->Set_Transform( muzzle );
		//FiringSound->Stop();		

		if ( FiringSound->Get_Class_ID () == CLASSID_3D ) {
			FiringSound->Stop();
		} else {
			FiringSound->Seek( 0 );
		} 
		
		
		//
		//	Force the sound to play
		//
		if ( Get_Owner() != COMBAT_STAR || CombatManager::Is_First_Person() == false ) {
			FiringSound->Add_To_Scene( true );
			FiringSound->Play();
		} else {			
			FiringSound->Play();
		}		

		//
		//	Play the sound (or add it to the scene)
		//
		/*if ( Get_Owner() == COMBAT_STAR && CombatManager::Is_First_Person() ) {
			FiringSound->Play();
		} else {
			FiringSound->Remove_From_Scene();
			FiringSound->Add_To_Scene( true );
		}*/

	} else {

		//
		//	Create a pseudo-3d sound effect for this firing sound and
		// add it to the sound scene.
		//

		if ( sound_id != 0 ) {
			
			//
			//	Determine whether to play the sound as 2D or 3D
			//
			int classid_hint = CLASSID_3D;
			if ( Get_Owner() == COMBAT_STAR && CombatManager::Is_First_Person() ) {
				classid_hint = CLASSID_2D;
			}

			//
			//	Create the sound
			//
			RefCountedGameObjReference *owner_ref = new RefCountedGameObjReference;
			owner_ref->Set_Ptr( Get_Owner() );
			FiringSound = WWAudioClass::Get_Instance()->Create_Sound( sound_id, owner_ref, 0, classid_hint );
			REF_PTR_RELEASE( owner_ref );
		}

		if ( FiringSound != NULL ) {

			FiringSoundDefID = sound_id;
			FiringSound->Set_Transform( muzzle );
			FiringSound->Attach_To_Object( Model );
			FiringSound->Add_To_Scene( true );
		}
	}

	// (gth) Apply an Impulse to tanks when they fires a bullet
	// Byon, this can be done with any RigidBody derived physics object (all vehicles)
	// I suggest making it a parameter of the weapon and having the strength be a
	// multiple of the mass of the vehicle.
	ArmedGameObj * owner = Get_Owner();
	VehicleGameObj * vehicle_game_obj = owner->As_VehicleGameObj();
	if (vehicle_game_obj != NULL) {
		PhysClass * vehicle = vehicle_game_obj->Peek_Physical_Object();
		if (vehicle->As_TrackedVehicleClass() != NULL) {

			RigidBodyClass * rbody = (RigidBodyClass *)vehicle;

			Vector3 impulse_pos;
			muzzle.Get_Translation(&impulse_pos);
			Vector3 impulse;
			muzzle.Get_X_Vector(&impulse);
			impulse *= -Definition->RecoilImpulse * rbody->Get_Mass();

			rbody->Apply_Impulse(impulse,impulse_pos);
		}
	}

	// Play an anim on the human owner
	if ( owner != NULL && owner->As_SoldierGameObj() ) {
		if ( !Definition->HumanFiringAnimation.Is_Empty() ) {
			owner->As_SoldierGameObj()->Set_Animation( Definition->HumanFiringAnimation, 0, 0 );
		}
	}
}

void	WeaponClass::Make_Shell_Eject( const Matrix3D & tm )
{
	DefinitionClass * def = DefinitionMgrClass::Find_Definition( Definition->EjectPhysDefID );

	if ((def != NULL) && ((PhysDefClass *)def)->Is_Type( "ProjectileDef" )) {

		// Create the object
		ProjectileClass * PhysicalObject = (ProjectileClass *)def->Create();

		// Set its transform, collision group, observer, and initial velocity
		PhysicalObject->Set_Transform( tm );
		PhysicalObject->Set_Observer(&_TheEjectCasingObserver);
		PhysicalObject->Set_Collision_Group( DEFAULT_COLLISION_GROUP );
		PhysicalObject->Set_Velocity( tm.Rotate_Vector( Vector3( 2,0,0 ) ) );

		// Add it to the scene
		COMBAT_SCENE->Add_Dynamic_Object( PhysicalObject );
		PhysicalObject->Release_Ref();
	}
}


/*
**
*/
void	WeaponClass::Do_Continuous_Effects( bool enable )
{
	if ( !enable ) {

		for ( int i = 0; i < ContinuousEmitters.Length(); i++ ) {
			if ( ContinuousEmitters[i] != NULL ) {
				// Check if it is in the scene, it may ahve already been remove by completion
				if ( ContinuousEmitters[i]->Peek_Scene() != NULL ) {
					PhysicsSceneClass::Get_Instance()->Remove_Render_Object( ContinuousEmitters[i] );
				}
				ContinuousEmitters[i]->Stop();
				ContinuousEmitters[i]->Release_Ref();
				ContinuousEmitters[i] = NULL;
			}
		}

		if ( ContinuousSound != NULL ) {
			ContinuousSound->Stop();
			ContinuousSound->Remove_From_Scene();
			ContinuousSound->Release_Ref();
			ContinuousSound = NULL;
		}
	}

	if ( enable ) {

		const AmmoDefinitionClass *ammo_def = PrimaryAmmoDefinition;
		if (IsPrimaryTriggered == false && IsSecondaryTriggered) {
			ammo_def = SecondaryAmmoDefinition;
		}

		int i;

		if ( ContinuousEmitters.Length() == 0 ) {

			int num_muzzles = 1;
			if ( Get_Muzzle( 0 ) != Get_Muzzle( 1 ) ) {
				num_muzzles = 2;
			}

			ContinuousEmitters.Resize( num_muzzles );
			for ( int i = 0; i < ContinuousEmitters.Length(); i++ ) {
				ContinuousEmitters[i] = NULL;
			}
		}

		for ( i = 0; i < ContinuousEmitters.Length(); i++ ) {
			if ( ContinuousEmitters[i] == NULL && !ammo_def->ContinuousEmitterName.Is_Empty() ) {
				RenderObjClass * renderobj = Create_Render_Obj_From_Filename( ammo_def->ContinuousEmitterName );
				if ( renderobj ) {
					WWASSERT( renderobj->Class_ID() == RenderObjClass::CLASSID_PARTICLEEMITTER );
					ContinuousEmitters[i] = (ParticleEmitterClass *)renderobj;
					ContinuousEmitters[i]->Set_Velocity_Inheritance_Factor( 1 );
				}
				if ( ContinuousEmitters[i] ) {
					SET_REF_OWNER( ContinuousEmitters[i] );
					ContinuousEmitters[i]->Start();
					PhysicsSceneClass::Get_Instance()->Add_Render_Object( ContinuousEmitters[i] );
				}
			}
		}

		if ( ContinuousSound == NULL && ammo_def->ContinuousSoundDefID != 0 ) {
			ContinuousSound = WWAudioClass::Get_Instance()->Create_Continuous_Sound( ammo_def->ContinuousSoundDefID );
			if ( ContinuousSound != NULL ) {
				ContinuousSound->Add_To_Scene( true );
			}
		}

		// if first person star, use first muzzle
		if ( (Get_Owner() == COMBAT_STAR) && CombatManager::Is_First_Person() ) {

			Vector3	fp_muzzle_pos = WeaponViewClass::Get_Muzzle_Pos();
			Matrix3D muzzle;
			muzzle.Obj_Look_At( fp_muzzle_pos, Get_Owner()->Get_Targeting_Pos(), 0 );

			for ( i = 0; i < ContinuousEmitters.Length(); i++ ) {
				if ( ContinuousEmitters[i] != NULL ) {
					ContinuousEmitters[i]->Set_Transform( muzzle );
				}
			}
			if ( ContinuousSound != NULL ) {
				ContinuousSound->Set_Transform( muzzle );
			}
		} else {
			for ( i = 0; i < ContinuousEmitters.Length(); i++ ) {
				if ( ContinuousEmitters[i] != NULL ) {
					ContinuousEmitters[i]->Set_Transform( Get_Muzzle( i ) );
				}
			}
			if ( ContinuousSound != NULL ) {
				ContinuousSound->Set_Transform( Get_Muzzle() );
			}
		}

	}

}

#define	WEAPON_RATE_CHEAT	0

/*
** Weapon State
*/
void	WeaponClass::Set_State( WeaponStateType new_state )
{
	#define	SWITCH_TIME		1.0f
	#define	RELOAD_TIME		Definition->ReloadTime
	#define	READY_TIME		25.0f

//	Debug_Say(( "Weapon switching from state %d to %d\n", State, new_state ));

	State = new_state;

	if ( State == STATE_START_SWITCH )	StateTimer = SWITCH_TIME/2.0f;
	if ( State == STATE_END_SWITCH )		StateTimer = SWITCH_TIME/2.0f;
	if ( State == STATE_RELOAD )			StateTimer = (float) RELOAD_TIME;
	if ( State == STATE_READY )			StateTimer = READY_TIME;	// delay til IDLE
	if ( State == STATE_FIRE_PRIMARY )	StateTimer = 1.0f / PrimaryAmmoDefinition->RateOfFire;
	if ( State == STATE_FIRE_SECONDARY )StateTimer = 1.0f / SecondaryAmmoDefinition->RateOfFire;
	if ( State == STATE_CHARGE )			StateTimer = PrimaryAmmoDefinition->ChargeTime;	// Charge Time

#if WEAPON_RATE_CHEAT
	if ( Get_Owner() == COMBAT_STAR ) {
		StateTimer *= 0.2f;
	}
#endif

}

void	WeaponClass::Update_State( float pending_time )
{
	LockTriggers = false;		// Unlock triggers

	WWPROFILE( "Update weapon state" );
	while ( pending_time > 0.0f ) {

		bool trigger_ok = true;
		// Only fire beacon if state is upright
		if ( Get_Style() == WEAPON_HOLD_STYLE_BEACON && 
				Get_Owner() && Get_Owner()->As_SoldierGameObj() && 
				!Get_Owner()->As_SoldierGameObj()->Is_Upright() ) {
			trigger_ok = false;
		}

		// If we can fire, start charging, it will fire and go back to READY
		if ( (State == STATE_READY) || (State == STATE_IDLE) ) {
			if ( (IsPrimaryTriggered || IsSecondaryTriggered) && (BurstCount != 0) && CombatManager::Is_Gameplay_Permitted() && trigger_ok ) {
				if ( SafetySet ) {
					LockTriggers = true;
				} else {
					if ( Is_Loaded() ) {
						Set_State( STATE_CHARGE );
					} else {
						// We are not loaded, play the empty sound
#if 0
						AudibleSoundClass * sound = WWAudioClass::Get_Instance()->Create_Sound( "Pistol_Empty_Click" );
						if ( sound ) {
							sound->Play();
							sound->Release_Ref();
						}
#else
						if ( Definition->EmptySoundDefID != 0 ) {
							EmptySoundTimer -= TimeManager::Get_Frame_Seconds();
							if ( EmptySoundTimer <= 0 ) {
								EmptySoundTimer = 0.3f;
								Matrix3D	muzzle = Get_Muzzle();

								RefCountedGameObjReference *owner_ref = new RefCountedGameObjReference;
								owner_ref->Set_Ptr( Get_Owner() );

								AudibleSoundClass * sound = WWAudioClass::Get_Instance()->Create_Sound( Definition->EmptySoundDefID, owner_ref );
								if ( sound != NULL ) {
									sound->Set_Transform( muzzle );
									sound->Attach_To_Object( Model );
									sound->Add_To_Scene( true );
									sound->Release_Ref();
								}

								REF_PTR_RELEASE( owner_ref );
							}
						}
#endif
					}
				}
			}

			// Force a reload when empty
			if ( Is_Reload_Needed() ) {
				Force_Reload();
			}
		}

		float	useable_time = std::min( StateTimer, pending_time );
		StateTimer -= useable_time;
		pending_time -= useable_time;

		if ( StateTimer <= 0 ) {	// Time for new state

			switch ( State ) {

				case STATE_IDLE:
					StateTimer = 0;
					pending_time = 0;
					break;

				case STATE_READY:
					Set_State( STATE_IDLE );
					break;

				case STATE_CHARGE:
					if ( IsPrimaryTriggered || !IsSecondaryTriggered ) {
						Set_State( STATE_FIRE_PRIMARY );
					} else {
						Set_State( STATE_FIRE_SECONDARY );
					}

					if ( IsPrimaryTriggered  || IsSecondaryTriggered ) {
						if ( Is_Muzzle_Clear() ) {
							Do_Fire( IsPrimaryTriggered );
							DidFire = true;
						}
					}
					break;

				case STATE_FIRE_PRIMARY:
				case STATE_FIRE_SECONDARY:
					Set_State( STATE_READY );
					break;

				case STATE_RELOAD:
					Do_Reload();
					Set_State( STATE_READY );
					break;

				case STATE_START_SWITCH:
					if ( UpdateModel == WEAPON_MODEL_UPDATE_WILL_BE_NEEDED ) {
						UpdateModel = WEAPON_MODEL_UPDATE_IS_NEEDED;
					}
					Set_State( STATE_END_SWITCH );
					break;

				case STATE_END_SWITCH:
					Set_State( STATE_IDLE );
					break;
			}
		}
	}
}

void	WeaponClass::Force_Reload( void )
{
	// Stop reloading on last bullet to fire faster
//	if ( Is_Reload_OK() && State != STATE_RELOAD ) {
	if ( Is_Reload_OK() && State <= STATE_READY ) {
		Set_State( STATE_RELOAD );

		if ( Definition->ReloadSoundDefID != 0 ) {
			// Make the reload sound
			Matrix3D	muzzle = Get_Muzzle();

			RefCountedGameObjReference *owner_ref = new RefCountedGameObjReference;
			owner_ref->Set_Ptr( Get_Owner() );

			AudibleSoundClass * sound = WWAudioClass::Get_Instance()->Create_Sound( Definition->ReloadSoundDefID, owner_ref );
			if ( sound != NULL ) {
				sound->Set_Transform( muzzle );
				sound->Attach_To_Object( Model );
				sound->Add_To_Scene( true );
				sound->Release_Ref();
			}

			REF_PTR_RELEASE( owner_ref );
		}
	}
}

/*
**
*/
void	WeaponClass::Update( void )
{
	// Remove player weapons from vehicle drivers
	if (	Get_Owner() && 
			Get_Owner()->As_SoldierGameObj() && 
			Get_Owner()->As_SoldierGameObj()->Is_In_Vehicle() ) {
		IsPrimaryTriggered = false;
		IsSecondaryTriggered = false;
	}

	if (	Get_Style() == WEAPON_HOLD_STYLE_C4 ||
			Get_Style() == WEAPON_HOLD_STYLE_BEACON ||
			Get_Can_Snipe() )
	{
		IsSecondaryTriggered = false;
	}

	WWASSERT( PrimaryAmmoDefinition != NULL );

	// Update Burst logic
	if ( (int)PrimaryAmmoDefinition->BurstMax == 0 ) {	// if not using bursts
		BurstCount = -1;
	} else {
		BurstDelayTimer -= TimeManager::Get_Frame_Seconds();
		if ( (!IsPrimaryTriggered && !IsSecondaryTriggered) || ( BurstDelayTimer <= 0.0f ) ) {
			BurstDelayTimer = PrimaryAmmoDefinition->BurstDelayTime * FreeRandom.Get_Float( 0.50f, 1.0f );
			BurstCount = PrimaryAmmoDefinition->BurstMax;
		}
	}

	if ( TimeManager::Get_Frame_Seconds() != 0 ) {
		DidFire = false;	// reset the did fire flag for this frame
	}

	Update_State( TimeManager::Get_Frame_Seconds() );

	// If star in first person, force no muzzle flashes on 3rd person model
	if ( (Get_Owner() == COMBAT_STAR) && CombatManager::Is_First_Person() ) {
		Update_Muzzle_Flash( false, false );
	} else {
		Update_Muzzle_Flash( DidFire && (TotalRoundsFired & 1), DidFire && (~TotalRoundsFired & 1) );
	}

	if ( DidFire ) {
		Do_Firing_Effects();	// Create muzzle flash and sounds
		NextAnimState = (TotalRoundsFired & 1) ? WEAPON_ANIM_FIRING_0 : WEAPON_ANIM_FIRING_1;
	} else {
		NextAnimState = WEAPON_ANIM_NOT_FIRING;
	}

	//
	//	Remove any continuous effects if the trigger state has changed since last frame
	//
	if (	(IsPrimaryTriggered == false && LastFrameIsPrimaryTriggered == true) ||
			(IsSecondaryTriggered == false && LastFrameIsSecondaryTriggered == true) )
	{
		Do_Continuous_Effects( false );
	} else {

		//
		//	Determine whether or not to trigger the continuous effects (emitters) for
		// this weapon.
		//
		bool emitters_on = IsPrimaryTriggered || IsSecondaryTriggered;
		if ( State == STATE_START_SWITCH ||  State == STATE_END_SWITCH || 
			  State == STATE_RELOAD || Get_Clip_Rounds() == 0 ) {
			emitters_on = false;
		}
		Do_Continuous_Effects( emitters_on );
	}

	//
	//	Remember what our state last frame was...
	//
	LastFrameIsPrimaryTriggered	= IsPrimaryTriggered;
	LastFrameIsSecondaryTriggered	= IsSecondaryTriggered;
	return ;
}


/*
** Simulate firing straight down the muzzle and see where in the world we hit
*/
bool	WeaponClass::Cast_Weapon_Down_Muzzle( Vector3 & hit_pos )
{
	int muzzle_index	= Get_Total_Rounds_Fired() & 1;
	Matrix3D muzzle	= Get_Muzzle( muzzle_index );

	//
	//	Determine the start and end positions of the ray
	//
	Vector3 start	= muzzle.Get_Translation();
	Vector3 end		= muzzle * Vector3( Get_Range(), 0, 0 );
	LineSegClass ray( start, end );

	//
	//	Cast the ray into the world
	//
	CastResultStruct res;
	PhysRayCollisionTestClass raytest(ray,&res,BULLET_COLLISION_GROUP,COLLISION_TYPE_PROJECTILE);

	Ignore_Owner();
{ WWPROFILE( "Cast Ray" );
	COMBAT_SCENE->Cast_Ray( raytest );
}
	Unignore_Owner();

	//
	//	Calculate where in the world this would hit
	//
	bool retval = false;
	if ( res.StartBad == false ) {
		hit_pos = start + (end * res.Fraction);
		retval = true;
	}

	return retval;
}


/*
** Simulate firing from muzzle to target, see where and if we hit
*/
PhysicalGameObj * WeaponClass::Cast_Weapon( const Vector3 & target )
{
	int muzzle_index = Get_Total_Rounds_Fired() & 1;
	Matrix3D muzzle = Get_Muzzle( muzzle_index );

	Vector3 start = muzzle.Get_Translation();
//	Vector3 end = muzzle * Vector3( Get_Range(), 0, 0 );
	Vector3 end = target;
	LineSegClass ray( start, end );

	CastResultStruct res;
	PhysRayCollisionTestClass raytest(ray,&res,BULLET_COLLISION_GROUP,COLLISION_TYPE_PROJECTILE);

	Ignore_Owner();
{ WWPROFILE( "Cast Ray" );
	COMBAT_SCENE->Cast_Ray( raytest );
}
	Unignore_Owner();

//	Debug_Say(( "Weapon Focus %f %f %f\n", bullet_hit_position->X, bullet_hit_position->Y, bullet_hit_position->Z ));

	PhysicalGameObj * hit_obj = NULL;
	if ( raytest.CollidedPhysObj ) {
		if ( raytest.CollidedPhysObj->Get_Observer() != NULL ) {
			hit_obj = ((CombatPhysObserverClass *)raytest.CollidedPhysObj->Get_Observer())->As_PhysicalGameObj();
		}
	}

	return hit_obj;
}


/*
** Check to see if this weapon can reload
*/
bool
WeaponClass::Is_Reload_OK( void )
{
	bool retval = (InventoryRounds != 0);

	//
	//	Allow reloads if the infinite ammo cheat is enabled
	//
	if (	CheatMgrClass::Get_Instance () != NULL && 
			CheatMgrClass::Get_Instance ()->Is_Cheat_Enabled( CheatMgrClass::CHEAT_INFINITE_AMMO ) && Owner == COMBAT_STAR )
	{
		retval = true;
	}

	return retval;
}


void	WeaponClass::Stop_Firing_Sound( void )
{
	if ( FiringSound == NULL ) {
		return ;
	}

	//
	//	Stop the sound
	//
	FiringSound->Stop ();
	FiringSound->Remove_From_Scene ();
	REF_PTR_RELEASE (FiringSound);	
	return ;
}


// This function calcuates the weapon information that must be displayed for the star's
// weapon in the hud reticle & sniper view.  This should only be called for the weapon
// we want to show a reticle for.

// The goal is to find the HitPosition, and HitPositionHot

void	WeaponClass::Display_Targeting( void )
{
	WWPROFILE( "Cast_Star_Weapon" );
	int muzzle_index = Get_Total_Rounds_Fired() & 1;

	// vehicles that don't have a turret bone and do have homing weapons target from the camera
	Matrix3D muzzle;
	VehicleGameObj * vehicle = Get_Owner()->As_VehicleGameObj();;

	if (vehicle && (vehicle->Has_Turret() == false) && (PrimaryAmmoDefinition->IsTracking)) {
		muzzle = COMBAT_CAMERA->Get_Transform();
		muzzle.Rotate_Z( DEG_TO_RADF(90.0) );
		muzzle.Rotate_Y( DEG_TO_RADF(90.0) );
	} else {
		muzzle = Get_Muzzle( muzzle_index );
	}

	// Cast a ray to see what the weapon is pointing at
	Vector3 start = muzzle.Get_Translation();
	Vector3 end = muzzle * Vector3( Get_Range(), 0, 0 );
	LineSegClass ray( start, end );

	CastResultStruct res;
	PhysRayCollisionTestClass raytest(ray,&res,BULLET_COLLISION_GROUP,COLLISION_TYPE_PROJECTILE);

	Ignore_Owner();

	{
		WWPROFILE( "Cast_Ray" );
		COMBAT_SCENE->Cast_Ray( raytest );
	}

	Unignore_Owner();

	Vector3	pos;
	ray.Compute_Point( raytest.Result->Fraction, &pos );
	HUDInfo::Set_Weapon_Target_Position( pos );

//	Debug_Say(( "Weapon Focus %f %f %f\n", bullet_hit_position->X, bullet_hit_position->Y, bullet_hit_position->Z ));

	if ( raytest.CollidedPhysObj && raytest.CollidedPhysObj->Get_Observer() != NULL ) {
		DamageableGameObj * obj = ((CombatPhysObserverClass *)raytest.CollidedPhysObj->Get_Observer())->As_DamageableGameObj();

		if ( obj->Is_Targetable() == false ) {
			obj = NULL;
		}

		// if stealthed and enemy, it's not targetable
		if ( obj && obj->As_SmartGameObj() && obj->As_SmartGameObj()->Is_Stealthed() && 
				COMBAT_STAR && obj->Is_Enemy( COMBAT_STAR ) ) {
			obj = NULL;
		}

		HUDInfo::Set_Weapon_Target_Object( obj );

		// We have a problem here with a building here clearing the PT, so dont set
		// buildings here, assume they will be set from the ccamera
		// Mo, try pulling back the cast above
		if ( obj == NULL || obj->As_BuildingGameObj() == NULL ) {
			HUDInfo::Set_Info_Object( obj );
		}
	}

}

#if 0
	// Aquire
	if ( AmmoDefinition->AquireTime != 0.0f ) {
		SmartGameObj *target = weapon->Pointing_At_Target();

		if ( target ) {
			if ( AquireTarget != target ) {	// locking onto someone new
				AquireTarget = target;
				AquireTimer = 0.0f;
			}
			AquireTimer += TimeManager::Get_Frame_Seconds();		// Bump the timer forward in seconds
			if (AquireTimer >= AmmoDefinition->AquireTime) {
				AquireTimer = AmmoDefinition->AquireTime;
			}
		} else {
			AquireTimer -= TimeManager::Get_Frame_Seconds();		// Bump the timer backwards in seconds
			if ( AquireTimer < 0.0f ) {
				AquireTimer = 0.0f;
			}
		}

		IsAquired = (target != NULL) &&
						(AquireTimer >= AmmoDefinition->AquireTime);
	}
#endif

const Matrix3D & WeaponClass::Get_Muzzle( int index )
{
	WWASSERT( Get_Owner() != NULL );

	// Star's weapon in first person fires from the camera
	if ( Get_Owner() == COMBAT_STAR && CombatManager::Is_First_Person() ) {
		static Matrix3D _muzzle;
		_muzzle.Obj_Look_At( COMBAT_CAMERA->Get_Transform().Get_Translation(), Get_Owner()->Get_Targeting_Pos(), 0 );
		return _muzzle;
	}

	return Get_Owner()->Get_Muzzle( index );
}


void	WeaponClass::Ignore_Owner( void )
{
	if ( Get_Owner() != NULL ) {
		if ( Get_Owner()->Peek_Physical_Object() != NULL ) {
			Get_Owner()->Peek_Physical_Object()->Inc_Ignore_Counter();
		}

		// If Owner is a vehicle, inc all the occupants
		if ( Get_Owner()->As_VehicleGameObj() != NULL ) {
			Get_Owner()->As_VehicleGameObj()->Ignore_Occupants();
		}
	}
}

void	WeaponClass::Unignore_Owner( void )
{
	if ( Get_Owner() != NULL ) {
		if ( Get_Owner()->Peek_Physical_Object() != NULL ) {
			Get_Owner()->Peek_Physical_Object()->Dec_Ignore_Counter();
		}

		if ( Get_Owner()->As_VehicleGameObj() != NULL ) {
			Get_Owner()->As_VehicleGameObj()->Unignore_Occupants();
		}
	}
}


/*
**
*/
MuzzleFlashClass::MuzzleFlashClass( void ) :
	MuzzleA0Bone( 0 ),
	MuzzleA1Bone( 0 ),
	Rotation( 0 ),
	Model( 0 ),
	LastFlashA0( true ),
	LastFlashA1( true )
{
}

MuzzleFlashClass::~MuzzleFlashClass( void )
{
	REF_PTR_RELEASE( Model );
}

void	MuzzleFlashClass::Init( RenderObjClass * model )
{
	REF_PTR_SET( Model, model );

	if ( model != NULL ) {

		// Find the muzzle bone ( for rotation )
		MuzzleA0Bone = model->Get_Bone_Index( "muzzlea0" );
		MuzzleA1Bone = model->Get_Bone_Index( "muzzlea1" );
		if ( MuzzleA0Bone > 0 ) {
//			model->Capture_Bone( MuzzleA0Bone );
		}
		if ( MuzzleA1Bone > 0 ) {
//			model->Capture_Bone( MuzzleA1Bone );
		}

		Update( false, false );
	}

}

void	MuzzleFlashClass::Update( bool flashA0, bool flashA1 )
{
	if ( MuzzleA1Bone == 0 ) {
		flashA0 = flashA0 | flashA1;
		flashA1 = false;
	}

	if ( Model != NULL ) {
		if (( MuzzleA0Bone > 0 ) && (LastFlashA0 != flashA0)) {
			LastFlashA0 = flashA0;
	#if 0
			if ( flash ) {
				Rotation += TimeManager::Get_Frame_Seconds() * 10;
				Matrix3D tm(1);
				tm.Rotate_X( Rotation );
				Model->Control_Bone( MuzzleA0Bone, tm );
			}
	#endif

			for (int i=0; i<Model->Get_Num_Sub_Objects_On_Bone( MuzzleA0Bone ); i++) {
				RenderObjClass * robj = Model->Get_Sub_Object_On_Bone(i, MuzzleA0Bone );
					
				// hide all meshes named "muzzleflash"
				if (strstr(robj->Get_Name(),"MUZZLEFLASH") || strstr(robj->Get_Name(),"MZ"))  {
					robj->Set_Hidden( !flashA0 );
				}

				robj->Release_Ref();
			}
		}

		if (( MuzzleA1Bone > 0 ) && (LastFlashA1 != flashA1)) {
			
			LastFlashA1 = flashA1;
	#if 0
			if ( flash ) {
				Rotation += TimeManager::Get_Frame_Seconds() * 10;
				Matrix3D tm(1);
				tm.Rotate_X( Rotation );
				Model->Control_Bone( MuzzleA1Bone, tm );
			}
	#endif

			for (int i=0; i<Model->Get_Num_Sub_Objects_On_Bone( MuzzleA1Bone ); i++) {
				RenderObjClass * robj = Model->Get_Sub_Object_On_Bone(i, MuzzleA1Bone );
				// hide all meshes named "muzzleflash"
				if (strstr(robj->Get_Name(),"MUZZLEFLASH") || strstr(robj->Get_Name(),"MZ")) {
					robj->Set_Hidden( !flashA1 );
				}
				robj->Release_Ref();
			}
		}
	}
}
