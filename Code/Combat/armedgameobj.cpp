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
 *                     $Archive:: /Commando/Code/Combat/armedgameobj.cpp                      $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 12/20/01 7:10p                                              $*
 *                                                                                             *
 *                    $Revision:: 38                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


/*
**	Includes
*/
#include "armedgameobj.h"
#include "debug.h"
#include "weaponbag.h"
#include "weapons.h"
#include "rendobj.h"
#include "bitpackids.h"
#include "wwprofile.h"
#include "smartgameobj.h"

/*
** ArmedGameObjDef
*/
ArmedGameObjDef::ArmedGameObjDef( void ) :
	WeaponTiltRate( 1.0f ),
	WeaponTiltMin(	-10000.0f ),
	WeaponTiltMax(	 10000.0f ),
	WeaponTurnRate( 1.0f ),
	WeaponTurnMin(	-10000.0f ),
	WeaponTurnMax(	 10000.0f ),
	WeaponError( 0 ),
	WeaponDefID( 0 ),
	SecondaryWeaponDefID( 0 ),
	WeaponRounds( -1 )
{
	EDITABLE_PARAM( ArmedGameObjDef, ParameterClass::TYPE_ANGLE,	WeaponTiltRate );
	EDITABLE_PARAM( ArmedGameObjDef, ParameterClass::TYPE_ANGLE,	WeaponTiltMin );
	EDITABLE_PARAM( ArmedGameObjDef, ParameterClass::TYPE_ANGLE,	WeaponTiltMax );
	EDITABLE_PARAM( ArmedGameObjDef, ParameterClass::TYPE_ANGLE,	WeaponTurnRate );
	EDITABLE_PARAM( ArmedGameObjDef, ParameterClass::TYPE_ANGLE,	WeaponTurnMin );
	EDITABLE_PARAM( ArmedGameObjDef, ParameterClass::TYPE_ANGLE,	WeaponTurnMax );
	EDITABLE_PARAM( ArmedGameObjDef, ParameterClass::TYPE_ANGLE,	WeaponError );
	EDITABLE_PARAM( ArmedGameObjDef, ParameterClass::TYPE_WEAPONOBJDEFINITIONID,	WeaponDefID );
	EDITABLE_PARAM( ArmedGameObjDef, ParameterClass::TYPE_INT,		WeaponRounds );
	EDITABLE_PARAM( ArmedGameObjDef, ParameterClass::TYPE_WEAPONOBJDEFINITIONID,	SecondaryWeaponDefID );
}

enum	{
	CHUNKID_DEF_PARENT							=	418001829,
	CHUNKID_DEF_VARIABLES,

	MICROCHUNKID_DEF_WEAPON_TILT_RATE		=	1,
	MICROCHUNKID_DEF_WEAPON_TILT_MIN,
	MICROCHUNKID_DEF_WEAPON_TILT_MAX,
	MICROCHUNKID_DEF_WEAPON_TURN_RATE,
	MICROCHUNKID_DEF_WEAPON_TURN_MIN,
	MICROCHUNKID_DEF_WEAPON_TURN_MAX,
	XXXMICROCHUNKID_DEF_PRIMARY_ROUNDS,
	XXXMICROCHUNKID_DEF_PRIMARY_AMMO_WEAPON_DEF_ID,
	XXXMICROCHUNKID_DEF_SECONDARY_AMMO_WEAPON_DEF_ID,
	XXXMICROCHUNKID_DEF_SECONDARY_ROUNDS,
	MICROCHUNKID_DEF_WEAPON_DEF_ID,
	MICROCHUNKID_DEF_WEAPON_ROUNDS,
	MICROCHUNKID_DEF_WEAPON_ERROR,
	MICROCHUNKID_DEF_SECONDARY_WEAPON_DEF_ID,
};

bool	ArmedGameObjDef::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_DEF_PARENT );
		PhysicalGameObjDef::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_DEF_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_WEAPON_TILT_RATE, WeaponTiltRate );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_WEAPON_TILT_MIN,  WeaponTiltMin );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_WEAPON_TILT_MAX,  WeaponTiltMax );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_WEAPON_TURN_RATE, WeaponTurnRate );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_WEAPON_TURN_MIN,  WeaponTurnMin );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_WEAPON_TURN_MAX,  WeaponTurnMax );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_WEAPON_DEF_ID,		WeaponDefID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_SECONDARY_WEAPON_DEF_ID,		SecondaryWeaponDefID );
		WRITE_SAFE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_WEAPON_ROUNDS,	WeaponRounds, int );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_WEAPON_ERROR,		WeaponError );
	csave.End_Chunk();

	return true;
}

bool	ArmedGameObjDef::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_DEF_PARENT:
				PhysicalGameObjDef::Load( cload );
				break;

			case CHUNKID_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_WEAPON_TILT_RATE, WeaponTiltRate );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_WEAPON_TILT_MIN,  WeaponTiltMin );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_WEAPON_TILT_MAX,  WeaponTiltMax );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_WEAPON_TURN_RATE, WeaponTurnRate );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_WEAPON_TURN_MIN,  WeaponTurnMin );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_WEAPON_TURN_MAX,  WeaponTurnMax );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_WEAPON_DEF_ID,		WeaponDefID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_SECONDARY_WEAPON_DEF_ID,		SecondaryWeaponDefID );
						READ_SAFE_MICRO_CHUNK( cload, MICROCHUNKID_DEF_WEAPON_ROUNDS,		WeaponRounds, int );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_WEAPON_ERROR,		WeaponError );

						default:
							Debug_Say(( "Unrecognized ArmedDef Variable chunkID %d\n", cload.Cur_Micro_Chunk_ID() ));
							break;

					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized ArmedDef chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

/*
** ArmedGameObj
*/
ArmedGameObj::ArmedGameObj( void ) :
	MuzzleA0Bone( 0 ),
	MuzzleA1Bone( 0 ),
	MuzzleB0Bone( 0 ),
	MuzzleB1Bone( 0 ),
	TargetingPos(0,0,0)
{
	WeaponBag = new WeaponBagClass( this );
}


ArmedGameObj::~ArmedGameObj( void )
{
	delete WeaponBag;
}

/*
**
*/
void	ArmedGameObj::Init( const ArmedGameObjDef & definition )
{
	PhysicalGameObj::Init( definition );
	Copy_Settings( definition );
	return ;
}

/*
**
*/
void	ArmedGameObj::Copy_Settings( const ArmedGameObjDef & definition )
{
	WeaponClass * weapon = NULL;
	if ( definition.WeaponDefID != 0 ) {
		weapon = WeaponBag->Add_Weapon( definition.WeaponDefID, definition.WeaponRounds );
	}

	if ( definition.SecondaryWeaponDefID != 0 ) {
		WeaponClass * s_weapon = WeaponBag->Add_Weapon( definition.SecondaryWeaponDefID, definition.WeaponRounds );
		if ( weapon == NULL ) {
			weapon = s_weapon;
		}
	}

	if ( weapon != NULL ) {
		WeaponBag->Select_Weapon( weapon );
	}

	Init_Muzzle_Bones();
	return ;
}

/*
**
*/
void	ArmedGameObj::Re_Init( const ArmedGameObjDef & definition )
{
	PhysicalGameObj::Re_Init( definition );

	//
	//	Remove all non-beacon entries from the weapon bag...
	//
	WeaponBagClass *old_bag = WeaponBag;
	if ( WeaponBag != NULL ) {

		//
		//	Loop over all the weapons in the bag
		//
		int weapon_index = WeaponBag->Get_Count();
		while (weapon_index --) {
			WeaponClass *weapon = WeaponBag->Peek_Weapon( weapon_index );
			
			//
			//	If this isn't a beacon, then remove it
			//
			if (weapon != NULL && weapon->Get_Definition ()->Style != WEAPON_HOLD_STYLE_BEACON) {
				WeaponBag->Remove_Weapon( weapon_index );
			}
		}

		WeaponBag = NULL;
	}

	//
	//	Re-initialize the weapon bag
	//
	WeaponBag = new WeaponBagClass( this );

	//
	//	Copy any internal settings from the definition
	//
	Copy_Settings( definition );

	//
	//	Now add any beacons back into the weapon bag
	//
	if ( old_bag != NULL ) {
		WeaponBag->Move_Contents( old_bag );
		delete old_bag;
	}

	return ;
}

const ArmedGameObjDef & ArmedGameObj::Get_Definition( void ) const
{
	return (const ArmedGameObjDef &)BaseGameObj::Get_Definition();
}


/*
** ArmedGameObj Save and Load
*/
enum	{
	CHUNKID_PARENT							=	418001841,
	CHUNKID_VARIABLES,
	CHUNKID_WEAPONBAG,

	MICROCHUNKID_TARGETING_POS			=	1,
};

bool	ArmedGameObj::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_PARENT );
	PhysicalGameObj::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TARGETING_POS, TargetingPos );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_WEAPONBAG );
	WeaponBag->Save( csave );
	csave.End_Chunk();

	return true;
}

bool	ArmedGameObj::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PARENT:
				PhysicalGameObj::Load( cload );
				break;

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TARGETING_POS, TargetingPos );
						default:
							Debug_Say(( "Unrecognized ArmedGameObj Variable chunkID\n" ));
							break;

					}
					cload.Close_Micro_Chunk();
				}
				break;

			case CHUNKID_WEAPONBAG:
			 	WeaponBag->Load( cload );
				break;

			default:
				Debug_Say(( "Unrecognized ArmedGameObj chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	SaveLoadSystemClass::Register_Post_Load_Callback(this);

	return true;
}

void	ArmedGameObj::On_Post_Load( void )
{
	PhysicalGameObj::On_Post_Load();
	Init_Muzzle_Bones();
}


//-----------------------------------------------------------------------------
void ArmedGameObj::Import_Frequent(BitStreamClass & packet)
{
	PhysicalGameObj::Import_Frequent( packet );

	Vector3 targeting_pos;
	packet.Get(targeting_pos.X, BITPACK_WORLD_POSITION_X);
	packet.Get(targeting_pos.Y, BITPACK_WORLD_POSITION_Y);
	packet.Get(targeting_pos.Z, BITPACK_WORLD_POSITION_Z);

	//
	//	Don't force the targetting if the object is controlled
	// by this player
	//
	SmartGameObj *smart_game_obj = As_SmartGameObj ();
	if (smart_game_obj == NULL || smart_game_obj->Is_Controlled_By_Me() == false) {
		Set_Targeting(targeting_pos);
	}

	return ;
}

//-----------------------------------------------------------------------------
void ArmedGameObj::Export_Frequent(BitStreamClass & packet)
{
	PhysicalGameObj::Export_Frequent( packet );

	packet.Add(TargetingPos.X, BITPACK_WORLD_POSITION_X);
	packet.Add(TargetingPos.Y, BITPACK_WORLD_POSITION_Y);
	packet.Add(TargetingPos.Z, BITPACK_WORLD_POSITION_Z);
}

//-----------------------------------------------------------------------------
void ArmedGameObj::Export_State_Cs(BitStreamClass & packet)
{
#if 1
	Vector3	my_pos;
	Get_Position( &my_pos );
	Vector3 rel_target = TargetingPos - my_pos;
	packet.Add(rel_target.X, BITPACK_WORLD_POSITION_X);
	packet.Add(rel_target.Y, BITPACK_WORLD_POSITION_Y);
	packet.Add(rel_target.Z, BITPACK_WORLD_POSITION_Z);
#else
	packet.Add(TargetingPos.X, BITPACK_WORLD_POSITION_X);
	packet.Add(TargetingPos.Y, BITPACK_WORLD_POSITION_Y);
	packet.Add(TargetingPos.Z, BITPACK_WORLD_POSITION_Z);
#endif
}

//-----------------------------------------------------------------------------
void ArmedGameObj::Import_State_Cs(BitStreamClass & packet)
{
#if 1
	Vector3 rel_target;
	packet.Get(rel_target.X, BITPACK_WORLD_POSITION_X);
	packet.Get(rel_target.Y, BITPACK_WORLD_POSITION_Y);
	packet.Get(rel_target.Z, BITPACK_WORLD_POSITION_Z);
	Vector3	my_pos;
	Get_Position( &my_pos );
	Vector3 targeting_pos = rel_target + my_pos;

	Set_Targeting(targeting_pos);
#else
	Vector3 targeting_pos;
	packet.Get(targeting_pos.X, BITPACK_WORLD_POSITION_X);
	packet.Get(targeting_pos.Y, BITPACK_WORLD_POSITION_Y);
	packet.Get(targeting_pos.Z, BITPACK_WORLD_POSITION_Z);

	Set_Targeting(targeting_pos);
#endif
}

void	ArmedGameObj::Post_Think( void )
{
	PhysicalGameObj::Post_Think();

	WWPROFILE( "Armed PostThink" );

	if ( Is_Delete_Pending() ) {				// don't update if destroying... (so we don't create a new laser!)
		return;
	}

	if ( Get_Weapon() != NULL ) {			// Update the weapon after the commands and update_human_animation
		Get_Weapon()->Update();
	}

	// allow any recoil animation to progress
	if (Peek_Model() != NULL) {
		for (int i=0; i<MAX_MUZZLES; i++) {
			MuzzleRecoilController[i].Update(Peek_Model());
		}
	}
}

/*
** Weapons
*/
WeaponClass	*	ArmedGameObj::Get_Weapon( void )
{
	return WeaponBag->Get_Weapon();
}

void	ArmedGameObj::Init_Muzzle_Bones( void )
{
	MuzzleA0Bone = Peek_Model()->Get_Bone_Index( "muzzlea0" );
	MuzzleA1Bone = Peek_Model()->Get_Bone_Index( "muzzlea1" );
	MuzzleB0Bone = Peek_Model()->Get_Bone_Index( "muzzleb0" );
	MuzzleB1Bone = Peek_Model()->Get_Bone_Index( "muzzleb1" );

	if (MuzzleA1Bone == 0) {
		MuzzleA1Bone = MuzzleA0Bone;
	}
	if (MuzzleB0Bone == 0) {
		MuzzleB0Bone = MuzzleA0Bone;
	}
	if (MuzzleB1Bone == 0) {
		MuzzleB1Bone = MuzzleB0Bone;
	}

	MuzzleRecoilController[0].Init(MuzzleA0Bone);
	MuzzleRecoilController[1].Init(MuzzleA1Bone);
	MuzzleRecoilController[2].Init(MuzzleB0Bone);
	MuzzleRecoilController[3].Init(MuzzleB1Bone);

	// Let the weapon learn about muzzle flashes
	if ( Get_Weapon() ) {
		Get_Weapon()->Set_Model( Peek_Model() );
	}
}

bool	ArmedGameObj::Set_Targeting( const Vector3 & pos, bool /* do_tilt */ )
{
	TargetingPos = pos;
	// Move the turret to match the target
	return true;
}

const Matrix3D & ArmedGameObj::Get_Muzzle( int index )
{
	RenderObjClass * model = Peek_Model();

	if ( index == 3 && MuzzleB1Bone != 0 ) {
		return model->Get_Bone_Transform( MuzzleB1Bone );
	}

	if ( index >= 2 && MuzzleB0Bone != 0 ) {
		return model->Get_Bone_Transform( MuzzleB0Bone );
	}

	if ( index == 1 && MuzzleA1Bone != 0 ) {
		return model->Get_Bone_Transform( MuzzleA1Bone );
	}

	if ( MuzzleA0Bone != 0 ) {
		return model->Get_Bone_Transform( MuzzleA0Bone );
	}

	return Get_Transform();
}

bool	ArmedGameObj::Muzzle_Exists( int index )
{
	if ( index == 0 ) {
		return MuzzleA0Bone != 0;
	}
	if ( index == 1 ) {
		return MuzzleA1Bone != 0;
	}
	if ( index == 2 ) {
		return MuzzleB0Bone != 0;
	}
	if ( index == 3 ) {
		return MuzzleB1Bone != 0;
	}
	return false;
}

void ArmedGameObj::Start_Recoil( int muzzle_index,float recoil_scale,float recoil_time )
{
	WWASSERT(muzzle_index >= 0);
	WWASSERT(muzzle_index < MAX_MUZZLES);
	MuzzleRecoilController[muzzle_index].Start_Recoil(recoil_scale,recoil_time);
}
