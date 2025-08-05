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
 *                     $Archive:: /Commando/Code/Combat/samsite.cpp                           $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 9/17/01 4:18p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 36                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "samsite.h"
#include "decophys.h"
#include "combat.h"
#include "animcontrol.h"
#include "wwpacket.h"
#include "debug.h"
#include "gameobjmanager.h"
#include "vehicle.h"
#include "persistfactory.h"
#include "combatchunkid.h"
#include "simpledefinitionfactory.h"
#include "wwhack.h"
#include "wwprofile.h"


/*
** SAMSiteGameObjDef
*/
DECLARE_FORCE_LINK( SAMSite )

SimplePersistFactoryClass<SAMSiteGameObjDef, CHUNKID_GAME_OBJECT_DEF_SAMSITE>	_SAMSiteGameObjDefPersistFactory;

DECLARE_DEFINITION_FACTORY(SAMSiteGameObjDef, CLASSID_GAME_OBJECT_DEF_SAMSITE, "SAMSite") _SAMSiteGameObjDefDefFactory;

SAMSiteGameObjDef::SAMSiteGameObjDef( void )
{
	MODEL_DEF_PARAM( SAMSiteGameObjDef, PhysDefID, "DecorationPhysDef" );
}

uint32	SAMSiteGameObjDef::Get_Class_ID (void) const	
{ 
	return CLASSID_GAME_OBJECT_DEF_SAMSITE; 
}

PersistClass *	SAMSiteGameObjDef::Create( void ) const 
{
	SAMSiteGameObj * obj = new SAMSiteGameObj;
	obj->Init( *this );
	return obj;
}

enum	{
	CHUNKID_DEF_PARENT							=	930991800,
};

bool	SAMSiteGameObjDef::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_DEF_PARENT );
		SmartGameObjDef::Save( csave );
	csave.End_Chunk();

	return true;
}

bool	SAMSiteGameObjDef::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_DEF_PARENT:
				SmartGameObjDef::Load( cload );
				break;
	  
			default:
				Debug_Say(( "Unrecognized SimpleDef chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

const PersistFactoryClass & SAMSiteGameObjDef::Get_Factory (void) const 
{ 
	return _SAMSiteGameObjDefPersistFactory; 
}


/*
**
*/
SimplePersistFactoryClass<SAMSiteGameObj, CHUNKID_GAME_OBJECT_SAMSITE>	_SAMSiteGameObjPersistFactory;

const PersistFactoryClass & SAMSiteGameObj::Get_Factory (void) const 
{
	return _SAMSiteGameObjPersistFactory;
}


enum {
	SAMSITE_STATE_DOWN,
	SAMSITE_STATE_UP,
	SAMSITE_STATE_RISING,
	SAMSITE_STATE_DROPPING,
};

/*
** SAMSiteGameObject
*/
SAMSiteGameObj::SAMSiteGameObj()	:
	TurretBone( -1 ),
	BarrelBone( -1 ),
	State( SAMSITE_STATE_DOWN )
{
}

SAMSiteGameObj::~SAMSiteGameObj()
{
	if ( Peek_Model() ) {
		if ( TurretBone != -1 ) {
	 		if ( Peek_Model()->Is_Bone_Captured( TurretBone ) ) {
				Peek_Model()->Release_Bone( TurretBone );
				TurretBone = -1;
			}
		}

		if ( BarrelBone != -1 ) {
	 		if ( Peek_Model()->Is_Bone_Captured( BarrelBone ) ) {
				Peek_Model()->Release_Bone( BarrelBone );
				BarrelBone = -1;
			}
		}
	}
}

/*
**
*/
void SAMSiteGameObj::Init( void )
{
	Init( Get_Definition() );
}

void	SAMSiteGameObj::Init( const SAMSiteGameObjDef & definition )
{
	SmartGameObj::Init( definition );
}

const SAMSiteGameObjDef & SAMSiteGameObj::Get_Definition( void ) const
{
	return (const SAMSiteGameObjDef &)BaseGameObj::Get_Definition();
}


/*
** SAMSiteGameObj Save and Load
*/
enum	{
	CHUNKID_PARENT							=	922991750,
	CHUNKID_VARIABLES,
	XXXCHUNKID_SAM_TIMER,
	XXXCHUNKID_ANIM_CONTROL,

	XXXMICROCHUNKID_PHYSOBJ					=	1,
	MICROCHUNKID_STATE,
	MICROCHUNKID_TIMER,
};

bool	SAMSiteGameObj::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_PARENT );
	SmartGameObj::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STATE,	State );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TIMER,	Timer );
	csave.End_Chunk();

	return true;
}

bool	SAMSiteGameObj::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PARENT:
				SmartGameObj::Load( cload );
				break;
								
			case CHUNKID_VARIABLES:
			{
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STATE,		State );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TIMER,		Timer );
						default:
							Debug_Say(( "Unrecognized SimpleGameObj Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}

				break;
			}

			default:
				Debug_Say(( "Unrecognized SimpleGameObj chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

/*
**
*/
void	SAMSiteGameObj::Import_Frequent( BitStreamClass & packet )
{
   WWASSERT(CombatManager::I_Am_Only_Client());

   SmartGameObj::Import_Frequent( packet );
   WWASSERT(packet.Is_Flushed());
}

void	SAMSiteGameObj::Export_Frequent( BitStreamClass & packet )
{
	SmartGameObj::Export_Frequent( packet );
}

void	SAMSiteGameObj::Update_Turret( float weapon_turn, float weapon_tilt )
{
   if ( TurretBone == -1 ) {
		TurretBone = Peek_Model()->Get_Bone_Index( "turret" );
		if ( TurretBone != -1 ) {
			Peek_Model()->Capture_Bone( TurretBone );
			if ( !Peek_Model()->Is_Bone_Captured( TurretBone ) ) {
				Debug_Say(( "SAMSiteGameObj : Turret Bone not captured\n" ));
				TurretBone = -1;
			}
		} else {
			Debug_Say(( "SAMSiteGameObj : Turret Bone not found\n" ));
		}
	}

	if ( BarrelBone == -1 ) {
		BarrelBone = Peek_Model()->Get_Bone_Index( "barrel" );
		if ( BarrelBone != -1 ) {
			Peek_Model()->Capture_Bone( BarrelBone );
			if ( !Peek_Model()->Is_Bone_Captured( BarrelBone ) ) {
				BarrelBone = -1;
				Debug_Say(( "Barrel Bone not captured\n" ));
			}

		} else {
			Debug_Say(( "Barrel Bone not found\n" ));
		}
	}

	if ( TurretBone != -1 ) {

		Matrix3D	facing(1);
		facing.Rotate_Z( weapon_turn );

		if ( BarrelBone == -1 ) {		// if no barrel bone
			facing.Rotate_Y( -weapon_tilt );  // neg rotate y tilts up
		}

		Peek_Model()->Control_Bone( TurretBone, facing );
	}

	if ( BarrelBone != -1 ) {
		Matrix3D	facing(1);
		facing.Rotate_Y( -weapon_tilt );  // neg rotate y tilts up
		Peek_Model()->Control_Bone( BarrelBone, facing );
	}

}

bool	SAMSiteGameObj::Set_Targeting( const Vector3 & target_pos, bool do_tilt )
{
	// Should add slow turning
	SmartGameObj::Set_Targeting( target_pos );

	Vector3 muzzle_pos = Peek_Model()->Get_Bone_Transform( "muzzle" ).Get_Translation();
	Matrix3D tm;
	tm.Obj_Look_At( muzzle_pos, target_pos, 0 );

	float weapon_tilt = -tm.Get_Y_Rotation();
	tm.Rotate_Y( weapon_tilt );
	float weapon_turn = tm.Get_Z_Rotation();

	if ( do_tilt == false ) {
		weapon_tilt = 0;
	}

	Update_Turret( weapon_turn, weapon_tilt );

	return true;
}


void SAMSiteGameObj::Generate_Control( void )
{
	if ( State == SAMSITE_STATE_UP ) {
		SmartGameObj::Generate_Control();
	}
}

static const char * _profile_name = "SAMSite Think";
void	SAMSiteGameObj::Think( void ) 
{
	SmartGameObj *target = NULL;
{	WWPROFILE( _profile_name );

	// Find Nearest Emeny
	Vector3	my_pos;
	Get_Position( &my_pos );
	SLNode<SmartGameObj> *node;
	for (	node = GameObjManager::Get_Smart_Game_Obj_List()->Head(); node; node = node->Next()) {
		VehicleGameObj *obj = node->Data()->As_VehicleGameObj();
		if ( !obj || !obj->Is_Aircraft() ) continue;
		Vector3	vehicle_pos;
		obj->Get_Position( &vehicle_pos );
		vehicle_pos -= my_pos;
		target = obj;
//		Debug_Say(( "Vehicle is %f away\n", distance ));
	}
	if ( target ) {
		ActionParamsStruct parameters;
		parameters.Priority = 100;
		parameters.AttackObject = target;
		parameters.AttackRange = 40;
		parameters.MoveLocation = my_pos;
		parameters.MoveArrivedDistance = 10000;
		Get_Action()->Attack( parameters );
	} else {
		Get_Action()->Reset( 100 );
	}
}
	SmartGameObj::Think(); 									  
{	WWPROFILE( _profile_name );

	if ( State == SAMSITE_STATE_DOWN ) {
		if ( target ) {
			Set_Animation( "B_samsiteL1.M_samrise" );
			Get_Anim_Control()->Set_Mode( ANIM_MODE_ONCE, 0 );
			State = SAMSITE_STATE_RISING;
		}
	} else if ( State == SAMSITE_STATE_UP ) {
		Timer -= TimeManager::Get_Frame_Seconds();
		if ( Timer <= 0 ) {
			Set_Animation( "B_samsitel1.M_samdrop" );
			Get_Anim_Control()->Set_Mode( ANIM_MODE_ONCE, 0 );
			State = SAMSITE_STATE_DROPPING;
		}
	} else {
		// If rising of dropping, check for completion, then set timer and state
		if( Get_Anim_Control()->Is_Complete() ) {
			State = ( State == SAMSITE_STATE_RISING ) ? SAMSITE_STATE_UP : SAMSITE_STATE_DOWN;
			Timer = 5;
		}
	}
}
}

