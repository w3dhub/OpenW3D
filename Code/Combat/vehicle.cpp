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
 *                     $Archive:: /Commando/Code/Combat/vehicle.cpp                           $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/29/02 4:08p                                               $*
 *                                                                                             *
 *                    $Revision:: 266                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


/*
**	Includes
*/
#include "vehicle.h"
#include "animcontrol.h"
#include "debug.h"
#include "combat.h"
#include "pscene.h"
#include "assets.h"
#include "rendobj.h"
#include "soldier.h"
#include "weapons.h"
#include "damage.h"
#include "wwpacket.h"
#include "gameobjmanager.h"
#include <stdio.h>
#include "WWAudio.H"
#include "Sound3D.H"
#include "vehiclephys.h"
#include "motorcycle.h"
#include "trackedvehicle.h"
#include "vtolvehicle.h"
#include "wheelvehicle.h"
#include "wheel.h"
#include "phys3.h"
#include "humanphys.h"
#include "persistfactory.h"
#include "combatchunkid.h"
#include "simpledefinitionfactory.h"
#include "wwhack.h"
#include "explosion.h"
#include "surfaceeffects.h"
#include "bitpackids.h"
#include "wwprofile.h"
#include "playertype.h"
#include "playerdata.h"
#include "translatedb.h"
#include "apppackettypes.h"
#include "stealtheffect.h"
#include "gametype.h"
#include "globalsettings.h"
#include "basecontroller.h"
#include "string_ids.h"
#include "specialbuilds.h"

/*
** Local prototypes
*/
static void	Set_Subobject_Visibility (RenderObjClass *model, int bone_index, bool show);

/*
** Defines
*/
#define	NORMALIZE_RADIANS( v ) 	while ( v > (float)DEG_TO_RAD(  180.0f ) ) v -= (float)DEG_TO_RAD( 360.0f ); \
											while ( v < (float)DEG_TO_RAD( -180.0f ) ) v += (float)DEG_TO_RAD( 360.0f );

//
// Class statics
//
bool	VehicleGameObj::DefaultDriverIsGunner	= false;
bool	VehicleGameObj::CameraLockedToTurret	= false;


/*
** Target Steering Option.
** (gth) 07/11/2001 - making this default to true
** (gth) 08/21/2001 - new default is false, user can turn it on with a console command...
*/
bool	_Use_Target_Steering = false;

bool	VehicleGameObj::Toggle_Target_Steering( void )
{
	return _Use_Target_Steering = !_Use_Target_Steering;
}

void	VehicleGameObj::Set_Target_Steering( bool onoff )
{
	_Use_Target_Steering = onoff;
	return ;
}

bool	VehicleGameObj::Is_Target_Steering( void )
{
	return _Use_Target_Steering;
}

/*
** VehicleGameObjDef
*/
DECLARE_FORCE_LINK( Vehicle )

SimplePersistFactoryClass<VehicleGameObjDef, CHUNKID_GAME_OBJECT_DEF_VEHICLE>	_VehicleGameObjDefPersistFactory;

DECLARE_DEFINITION_FACTORY(VehicleGameObjDef, CLASSID_GAME_OBJECT_DEF_VEHICLE, "Vehicle") _VehicleGameObjDefDefFactory;

VehicleGameObjDef::VehicleGameObjDef( void ) :
	Type( VEHICLE_TYPE_CAR ),
	TurnRadius( 10.0f ),
	OccupantsVisible( true ),
	EngineSoundMaxPitchFactor( 2.0F ),
	SightDownMuzzle( false ),
	Aim2D( true ),
	SquishVelocity( 1.5f ),
	VehicleNameID( 0 ),
	NumSeats( 2 ),
	GDIDamageReportID(0),
	NodDamageReportID(0),
	GDIDestroyReportID(0),
	NodDestroyReportID(0)
{
	// initialize all engine sound defs to zero
	for (int i=0; i<MAX_ENGINE_SOUND_STATES; i++) {
		EngineSound[i] = 0;
	}

	MODEL_DEF_PARAM( VehicleGameObjDef, PhysDefID, "DynamicPhysDef" );

	EDITABLE_PARAM( VehicleGameObjDef, ParameterClass::TYPE_STRING, 	TypeName );
	EDITABLE_PARAM( VehicleGameObjDef, ParameterClass::TYPE_STRING, 	Fire0Anim );
	EDITABLE_PARAM( VehicleGameObjDef, ParameterClass::TYPE_STRING, 	Fire1Anim );
	EDITABLE_PARAM( VehicleGameObjDef, ParameterClass::TYPE_STRING, 	Profile );

	EDITABLE_PARAM( VehicleGameObjDef, ParameterClass::TYPE_FLOAT, 	TurnRadius );
	EDITABLE_PARAM( VehicleGameObjDef, ParameterClass::TYPE_FLOAT, 	SquishVelocity );
	EDITABLE_PARAM( VehicleGameObjDef, ParameterClass::TYPE_BOOL,		Aim2D );

#ifdef	PARAM_EDITING_ON
	EnumParameterClass *param;
	param = new EnumParameterClass( (int*)&Type );
	param->Set_Name ("Type");
	param->Add_Value ( "Car",		VEHICLE_TYPE_CAR );
	param->Add_Value ( "Tank", 	VEHICLE_TYPE_TANK );
	param->Add_Value ( "Bike", 	VEHICLE_TYPE_BIKE );
	param->Add_Value ( "Flying", 	VEHICLE_TYPE_FLYING );
	param->Add_Value ( "Turret", 	VEHICLE_TYPE_TURRET );

	GENERIC_EDITABLE_PARAM(VehicleGameObjDef,param)
#endif

	EDITABLE_PARAM( VehicleGameObjDef, ParameterClass::TYPE_BOOL, 	OccupantsVisible );

	// engine sounds
	FLOAT_EDITABLE_PARAM(VehicleGameObjDef, EngineSoundMaxPitchFactor, 1.0F, 10.0F);
	NAMED_EDITABLE_PARAM(VehicleGameObjDef, ParameterClass::TYPE_SOUNDDEFINITIONID, EngineSound[ENGINE_SOUND_STATE_STARTING],"Engine Start Sound");
	NAMED_EDITABLE_PARAM(VehicleGameObjDef, ParameterClass::TYPE_SOUNDDEFINITIONID, EngineSound[ENGINE_SOUND_STATE_RUNNING],"Engine Running Loop");
	NAMED_EDITABLE_PARAM(VehicleGameObjDef, ParameterClass::TYPE_SOUNDDEFINITIONID, EngineSound[ENGINE_SOUND_STATE_STOPPING],"Engine Stop Sound");
//	NAMED_EDITABLE_PARAM(VehicleGameObjDef, ParameterClass::TYPE_SOUNDDEFINITIONID, EngineSound[ENGINE_SOUND_STATE_OFF],"Engine Off Sound");

	EDITABLE_PARAM( VehicleGameObjDef, ParameterClass::TYPE_BOOL,		SightDownMuzzle );

	EDITABLE_PARAM( VehicleGameObjDef, ParameterClass::TYPE_STRINGSDB_ID,		VehicleNameID );

	EDITABLE_PARAM( VehicleGameObjDef, ParameterClass::TYPE_INT,		NumSeats );

	EDITABLE_PARAM(VehicleGameObjDef, ParameterClass::TYPE_STRINGSDB_ID, GDIDamageReportID);
	EDITABLE_PARAM(VehicleGameObjDef, ParameterClass::TYPE_STRINGSDB_ID, NodDamageReportID);
	EDITABLE_PARAM(VehicleGameObjDef, ParameterClass::TYPE_STRINGSDB_ID, GDIDestroyReportID);
	EDITABLE_PARAM(VehicleGameObjDef, ParameterClass::TYPE_STRINGSDB_ID, NodDestroyReportID);
}

VehicleGameObjDef::~VehicleGameObjDef( void )
{
	Free_Transition_List();
}

uint32	VehicleGameObjDef::Get_Class_ID (void) const
{
	return CLASSID_GAME_OBJECT_DEF_VEHICLE;
}

PersistClass *	VehicleGameObjDef::Create( void ) const
{
	VehicleGameObj * obj = new VehicleGameObj;
	obj->Init( *this );
	return obj;
}

enum	{
	CHUNKID_DEF_PARENT							=	930991656,
	CHUNKID_DEF_VARIABLES,
	CHUNKID_DEF_TRANSITION,

	MICROCHUNKID_TYPE								=	1,
	MICROCHUNKID_TYPE_NAME,
	MICROCHUNKID_FIRE0ANIM,
	MICROCHUNKID_FIRE1ANIM,
	MICROCHUNKID_PROFILE,
	MICROCHUNKID_WEAPON_TURN_TRANS,

	XXXMICROCHUNKID_SOUND,
	MICROCHUNKID_EMITER_NAME,
	MICROCHUNKID_EMITER_OFFSET,
	MICROCHUNKID_EMITER2_NAME,
	MICROCHUNKID_EMITER2_OFFSET,
	XXX_MICROCHUNKID_MASS,
	XXX_MICROCHUNKID_MAX_ENGINE_TORQUE,
	XXX_MICROCHUNKID_STEERING_ANGLE,
	XXX_MICROCHUNKID_SPRING_CONSTANT,
	XXX_MICROCHUNKID_DAMPING_COEFFIENT,
	XXX_MICROCHUNKID_SPRING_LENGTH,
	MICROCHUNKID_PHYS_ID,
	MICROCHUNKID_TURN_RADIUS,
	MICROCHUNKID_OCCUPANTS_VISIBLE,

	XXX_MICROCHUNKID_ENGINE_SOUND_RPM_SCALE_MIN,
	XXX_MICROCHUNKID_ENGINE_SOUND_RPM_SCALE_MAX,
	MICROCHUNKID_ENGINE_START_SOUND,
	MICROCHUNKID_ENGINE_RUN_SOUND,
	MICROCHUNKID_ENGINE_STOP_SOUND,
	MICROCHUNKID_ENGINE_OFF_SOUND,

	MICROCHUNKID_DEF_SIGHT_DOWN_MUZZLE,
	MICROCHUNKID_DEF_AIM_2D,

	MICROCHUNKID_DEF_SQUISH_VELOCITY,
	MICROCHUNKID_ENGINE_SOUND_MAX_PITCH_FACTOR,
	MICROCHUNKID_DEF_VEHICLE_NAME_ID,
	MICROCHUNKID_DEF_NUM_SEATS,
	MICROCHUNKID_DEF_GDI_DAMAGE_REPORT_ID,
	MICROCHUNKID_DEF_NOD_DAMAGE_REPORT_ID,
	MICROCHUNKID_DEF_GDI_DESTROY_REPORT_ID,
	MICROCHUNKID_DEF_NOD_DESTROY_REPORT_ID,
};

bool	VehicleGameObjDef::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_DEF_PARENT );
		SmartGameObjDef::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_DEF_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TYPE, Type );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_TYPE_NAME, TypeName );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_FIRE0ANIM, Fire0Anim );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_FIRE1ANIM, Fire1Anim );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_PROFILE, Profile );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TURN_RADIUS, TurnRadius );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_OCCUPANTS_VISIBLE, OccupantsVisible );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ENGINE_SOUND_MAX_PITCH_FACTOR, EngineSoundMaxPitchFactor );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ENGINE_START_SOUND, EngineSound[ENGINE_SOUND_STATE_STARTING] );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ENGINE_RUN_SOUND, EngineSound[ENGINE_SOUND_STATE_RUNNING] );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ENGINE_STOP_SOUND, EngineSound[ENGINE_SOUND_STATE_STOPPING] );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ENGINE_OFF_SOUND, EngineSound[ENGINE_SOUND_STATE_OFF] );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_SIGHT_DOWN_MUZZLE, SightDownMuzzle );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_AIM_2D, Aim2D );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_SQUISH_VELOCITY, SquishVelocity );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_VEHICLE_NAME_ID,	VehicleNameID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_NUM_SEATS, NumSeats );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_GDI_DAMAGE_REPORT_ID, GDIDamageReportID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_NOD_DAMAGE_REPORT_ID, NodDamageReportID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_GDI_DESTROY_REPORT_ID, GDIDestroyReportID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_NOD_DESTROY_REPORT_ID, NodDestroyReportID );
	csave.End_Chunk();

	//	Save each of the transition 'definitions' in our list.
	for( int index = 0; index < Transitions.Count (); index ++ ) {
		TransitionDataClass *transition = Transitions[index];
		if( transition != NULL ) {

			//	Save this transition 'defintion' to its own chunk.
			csave.Begin_Chunk( CHUNKID_DEF_TRANSITION );
			transition->Save( csave );
			csave.End_Chunk();
		}
	}

	return true;
}

bool	VehicleGameObjDef::Load( ChunkLoadClass &cload )
{
	Free_Transition_List ();

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_DEF_PARENT:
				SmartGameObjDef::Load( cload );
				break;

			case CHUNKID_DEF_TRANSITION:
			{
				TransitionDataClass *transition = new TransitionDataClass;
				transition->Load( cload );
				Transitions.Add( transition );
			}
			break;

			case CHUNKID_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TYPE, Type );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_TYPE_NAME, TypeName );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_FIRE0ANIM, Fire0Anim );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_FIRE1ANIM, Fire1Anim );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_PROFILE, Profile );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_PHYS_ID, PhysDefID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TURN_RADIUS, TurnRadius );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_OCCUPANTS_VISIBLE, OccupantsVisible );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ENGINE_SOUND_MAX_PITCH_FACTOR, EngineSoundMaxPitchFactor);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ENGINE_START_SOUND, EngineSound[ENGINE_SOUND_STATE_STARTING] );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ENGINE_RUN_SOUND, EngineSound[ENGINE_SOUND_STATE_RUNNING] );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ENGINE_STOP_SOUND, EngineSound[ENGINE_SOUND_STATE_STOPPING] );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ENGINE_OFF_SOUND, EngineSound[ENGINE_SOUND_STATE_OFF] );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_SIGHT_DOWN_MUZZLE, SightDownMuzzle );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_AIM_2D, Aim2D );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_SQUISH_VELOCITY, SquishVelocity );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_VEHICLE_NAME_ID,	VehicleNameID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_NUM_SEATS, NumSeats );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_GDI_DAMAGE_REPORT_ID, GDIDamageReportID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_NOD_DAMAGE_REPORT_ID, NodDamageReportID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_GDI_DESTROY_REPORT_ID, GDIDestroyReportID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_NOD_DESTROY_REPORT_ID, NodDestroyReportID );
						default:
							Debug_Say(( "Unrecognized VehicleDef Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized VehicleDef chunkID\n" ));
				break;
		}
		cload.Close_Chunk();
	}

	return true;
}

/*
**
*/
void VehicleGameObjDef::Free_Transition_List( void )
{
	//	Delete each of the transition 'definitions' in our list.
	for( int index = 0; index < Transitions.Count (); index ++ ) {
		TransitionDataClass *transition = Transitions[index];
		if( transition != NULL ) {
			delete transition;
		}
	}
	Transitions.Delete_All();
	return ;
}

const PersistFactoryClass & VehicleGameObjDef::Get_Factory (void) const
{
	return _VehicleGameObjDefPersistFactory;
}


int VehicleGameObjDef::Get_Damage_Report(int team) const
{
	if (PLAYERTYPE_GDI == team) {
		return GDIDamageReportID;
	} else if (PLAYERTYPE_NOD == team) {
		return NodDamageReportID;
	}

	return 0;
}


int VehicleGameObjDef::Get_Destroy_Report(int team) const
{
	if (PLAYERTYPE_GDI == team) {
		return GDIDestroyReportID;
	} else if (PLAYERTYPE_NOD == team) {
		return NodDestroyReportID;
	}

	return 0;
}

/*
** VehicleGameObj
*/
SimplePersistFactoryClass<VehicleGameObj, CHUNKID_GAME_OBJECT_VEHICLE>	_VehicleGameObjPersistFactory;

const PersistFactoryClass & VehicleGameObj::Get_Factory (void) const
{
	return _VehicleGameObjPersistFactory;
}


VehicleGameObj::VehicleGameObj()	:
	TurretBone( 0 ),
	BarrelBone( 0 ),
	TurretTurn( 0 ),
	BarrelTilt( 0 ),
	BarrelOffset( 0 ),
	Sound( NULL ),
	EngineSoundState( ENGINE_SOUND_STATE_OFF ),
	CachedEngineSound( NULL ),
	WheelSurfaceSound(NULL),
	TransitionsEnabled( true ),
	OccupiedSeats( 0 ),
	HasEnterTransitions( false ),
	HasExitTransitions( false ),
	VehicleDelivered(false),
	LockTimer( 0 )
{
	DriverIsGunner = DefaultDriverIsGunner;
	Set_App_Packet_Type(APPPACKETTYPE_VEHICLE);
}

VehicleGameObj::~VehicleGameObj()
{
	if ( Peek_Physical_Object() != NULL && COMBAT_SCENE != NULL) {

		// Make sure the exiters don't hit me
		Peek_Physical_Object()->Set_Collision_Group( UNCOLLIDEABLE_GROUP );

		// Eject all the occupants around the vehicle so they don't intersect
		Vector3 vehicle_pos;
		Peek_Physical_Object()->Get_Position( &vehicle_pos );
		for ( int i = 0; i < SeatOccupants.Length(); i++ ) {
			if ( SeatOccupants[i] != NULL ) {
				SeatOccupants[i]->Exit_Destroyed_Vehicle( i, vehicle_pos );
			}
			SeatOccupants[i] = NULL;
		}
	} else {
		for ( int i = 0; i < SeatOccupants.Length(); i++ ) {
			if ( SeatOccupants[i] != NULL ) {
				SeatOccupants[i]->Exit_Vehicle();
			}
			SeatOccupants[i] = NULL;
		}
	}

	Destroy_Transitions();

	if (Sound) {
		Sound->Remove_From_Scene ();
		Sound->Release_Ref ();
		Sound = NULL;
	}

	if (CachedEngineSound) {
		CachedEngineSound->Remove_From_Scene();
		REF_PTR_RELEASE(CachedEngineSound);
	}

	Release_Turret_Bones();

	Shutdown_Wheel_Effects();
}

/*
**
*/
void VehicleGameObj::Init( void )
{
	DriverIsGunner = DefaultDriverIsGunner;
	// Vehicles are too complex to re-init
//	Init( Get_Definition() );
}

void	VehicleGameObj::Init( const VehicleGameObjDef & definition )
{
	DriverIsGunner = DefaultDriverIsGunner;
	SmartGameObj::Init( definition );

	SeatOccupants.Resize(definition.NumSeats);
	for ( int i = 0; i < SeatOccupants.Length(); i++ ) {
		SeatOccupants[i] = NULL;
	}

	Aquire_Turret_Bones();
	Init_Wheel_Effects();
	Create_And_Destroy_Transitions();
	Update_Damage_Meshes();

	//
	// Refine the app packet type if this is a turret
	//
	if (Get_Definition().Type == VEHICLE_TYPE_TURRET)
	{
		Set_App_Packet_Type(APPPACKETTYPE_TURRET);
	}

	//WWDEBUG_SAY((">> VehicleGameObj::Init, Type = %d\n", Get_Definition().Type));
}

const VehicleGameObjDef & VehicleGameObj::Get_Definition( void ) const
{
	return (const VehicleGameObjDef &)BaseGameObj::Get_Definition();
}


/*
** VehicleGameObj Save and Load
*/
enum	{
	CHUNKID_PARENT							=	923991632,
	CHUNKID_VARIABLES,
	XXXCHUNKID_ANIM_CONTROL,
	XXXCHUNKID_TRANSITION_LIST_OLD,
	CHUNKID_SEAT_LIST,

	MICROCHUNKID_TURRET_BONE			=	1,
	MICROCHUNKID_BARREL_BONE,
	XXXMICROCHUNKID_PHYSICAL_OBJECT,
	MICROCHUNKID_TRANSITION_LIST_ENTRY_OLD,
	MICROCHUNKID_TURRET_TURN,
	MICROCHUNKID_BARREL_TILT,
	MICROCHUNKID_TRANSITIONS_ENABLED,
	MICROCHUNKID_OCCUPIED_SEATS,
	MICROCHUNKID_NUM_SEATS,
};

bool	VehicleGameObj::Save( ChunkSaveClass & csave )
{
	// Destroy the transitions, because we don't save them, and don't
	// want to have the vehicle have a un-matched reference to them.
	// Re-create at the end of save
	Destroy_Transitions();

	csave.Begin_Chunk( CHUNKID_PARENT );
	SmartGameObj::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TURRET_TURN, TurretTurn );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_BARREL_TILT, BarrelTilt );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TRANSITIONS_ENABLED, TransitionsEnabled );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_OCCUPIED_SEATS, OccupiedSeats );
		int num_seats = SeatOccupants.Length();
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_NUM_SEATS, num_seats );
	csave.End_Chunk();

	if ( num_seats != 0 ) {
		csave.Begin_Chunk( CHUNKID_SEAT_LIST );
		csave.Write( &SeatOccupants[0], num_seats * sizeof( SeatOccupants[0] ) );
		csave.End_Chunk();
	}

	Create_And_Destroy_Transitions();

	return true;
}

bool	VehicleGameObj::Load( ChunkLoadClass &cload )
{
	int num_seats = 0;

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PARENT:
				SmartGameObj::Load( cload );
				break;

			case CHUNKID_VARIABLES:
			{
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TURRET_TURN, TurretTurn );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_BARREL_TILT, BarrelTilt );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TRANSITIONS_ENABLED, TransitionsEnabled );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_OCCUPIED_SEATS, OccupiedSeats );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_NUM_SEATS, num_seats );

						default:
							Debug_Say(( "Unrecognized Vehicle Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}

				break;
			}

			case	CHUNKID_SEAT_LIST:
			{
				if ( (num_seats == 0) || (num_seats != Get_Definition().NumSeats)) {
					SeatOccupants.Resize(Get_Definition().NumSeats);
					for ( int i = 0; i < SeatOccupants.Length(); i++ ) {
						SeatOccupants[i] = NULL;
					}
					break;		// May be legacy
				}

				SeatOccupants.Resize( num_seats );
				cload.Read( &SeatOccupants[0], num_seats * sizeof( SeatOccupants[0] ) );
				for ( int i = 0; i < num_seats; i++ ) {
					if ( SeatOccupants[i] != NULL ) {
						REQUEST_POINTER_REMAP( (void **)&SeatOccupants[i] );
					}
				}
				break;
			}

			default:
				Debug_Say(( "Unrecognized Vehicle chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	SaveLoadSystemClass::Register_Post_Load_Callback(this);
	return true;
}

void	VehicleGameObj::On_Post_Load( void )
{
	DriverIsGunner = DefaultDriverIsGunner;

	Init_Wheel_Effects();
	Aquire_Turret_Bones();
	SmartGameObj::On_Post_Load();
	Create_And_Destroy_Transitions();
	Update_Damage_Meshes();

	// For some reason???  some vehicles come in with an anim control, but no model in the anim control.
	if ( Get_Anim_Control() != NULL && Get_Anim_Control()->Peek_Model() == NULL ) {
		Get_Anim_Control()->Set_Model( Peek_Model() );
	}


	//
	// Refine the app packet type if this is a turret
	//
	if (Get_Definition().Type == VEHICLE_TYPE_TURRET)
	{
		Set_App_Packet_Type(APPPACKETTYPE_TURRET);
	}
}


#if 0
void	VehicleGameObj::Create_Transitions( void )
{
	Destroy_Transitions();

	WWASSERT( TransitionInstances.Count() == 0 );

	const TRANSITION_DATA_LIST & trans_data_list = Get_Definition().Get_Transition_List();

	if ( trans_data_list.Count() != 0 ) {		// new style

		if ( Seats[0] == NULL ) {
			Create_New_Transitions( TransitionDataClass::VEHICLE_ENTER_0 );
		} else {
			Create_New_Transitions( TransitionDataClass::VEHICLE_EXIT_0 );
		}

		if ( Seats[1] == NULL ) {
			Create_New_Transitions( TransitionDataClass::VEHICLE_ENTER_1 );
		} else {
			Create_New_Transitions( TransitionDataClass::VEHICLE_EXIT_1 );
		}
	}
}
#endif

void	VehicleGameObj::Destroy_Transitions( void )
{
	HasEnterTransitions = false;
	HasExitTransitions = false;
	while ( TransitionInstances.Count() ) {
  		TransitionManager::Destroy( TransitionInstances[0] );
  		TransitionInstances.Delete( 0 );
	}

	// They get put in a list, so flush the list.
	TransitionManager::Destroy_Pending();
}

void	VehicleGameObj::Create_And_Destroy_Transitions( void )
{
	// Manage the enter transitions
	bool	should_have_enter_transitions = (OccupiedSeats < Get_Definition().NumSeats) && TransitionsEnabled;
	if ( should_have_enter_transitions != HasEnterTransitions ) {
		if ( should_have_enter_transitions ) {
			Create_New_Transitions( TransitionDataClass::VEHICLE_ENTER );
			HasEnterTransitions = true;
		} else {
			Remove_Transitions( TransitionDataClass::VEHICLE_ENTER );
			HasEnterTransitions = false;
		}
	}

	// Manage the exit transitions
	bool	should_have_exit_transitions = (OccupiedSeats > 0) && TransitionsEnabled;
	if ( should_have_exit_transitions != HasExitTransitions ) {
		if ( should_have_exit_transitions ) {
			Create_New_Transitions( TransitionDataClass::VEHICLE_EXIT );
			HasExitTransitions = true;
		} else {
			Remove_Transitions( TransitionDataClass::VEHICLE_EXIT );
			HasExitTransitions = false;
		}
	}

}


/*
**
*/
void VehicleGameObj::Aquire_Turret_Bones( void )
{
   if ( TurretBone == 0 ) {
		TurretBone = Peek_Model()->Get_Bone_Index( "turret" );
		if ( TurretBone != 0 ) {
			Peek_Model()->Capture_Bone( TurretBone );
			if ( !Peek_Model()->Is_Bone_Captured( TurretBone ) ) {
				Debug_Say(( "VehicleGameObj::Aquire_Bones() : Turret Bone not captured for model %s\n",
					Peek_Model()->Get_Name()));
				TurretBone = 0;
			}
		}
	}

	BarrelOffset = 0;

	if ( BarrelBone == 0 ) {
		BarrelBone = Peek_Model()->Get_Bone_Index( "barrel" );
		if ( BarrelBone != 0 ) {
			Peek_Model()->Capture_Bone( BarrelBone );
			if ( !Peek_Model()->Is_Bone_Captured( BarrelBone ) ) {
				BarrelBone = 0;
				Debug_Say(( "VehicleGameObj::Aquire_Bones() : Barrel Bone not captured for model %s\n",
					Peek_Model()->Get_Name()));
			}
		}

		// find the barrel in turret space
	   if ( TurretBone != 0 ) {
			Matrix3D	turret_base = Peek_Model()->Get_Bone_Transform( TurretBone );
			Vector3	barrel_pos = Peek_Model()->Get_Bone_Transform( BarrelBone ).Get_Translation();
			Vector3	turret_space_barrel;
			Matrix3D::Inverse_Transform_Vector( turret_base, barrel_pos, &turret_space_barrel );
			BarrelOffset = turret_space_barrel.Y;
			if ( WWMath::Fabs( BarrelOffset ) < 0.1f  ) {
				BarrelOffset = 0;
			}
		}
	}

}

void VehicleGameObj::Release_Turret_Bones( void )
{
	if ( Peek_Model() ) {
		if ( TurretBone != 0 ) {
	 		if ( Peek_Model()->Is_Bone_Captured( TurretBone ) ) {
				Peek_Model()->Release_Bone( TurretBone );
				TurretBone = 0;
			}
		}

		if ( BarrelBone != 0 ) {
	 		if ( Peek_Model()->Is_Bone_Captured( BarrelBone ) ) {
				Peek_Model()->Release_Bone( BarrelBone );
				BarrelBone = 0;
			}
		}
	}

	BarrelOffset = 0;
}

void	VehicleGameObj::Export_Creation( BitStreamClass &packet )
{
	SmartGameObj::Export_Creation( packet );

	//
	//	Send the lock status to the client
	//
	int lock_owner_id = 0;
	if (LockOwner != NULL) {
		lock_owner_id = LockOwner.Get_Ptr ()->Get_ID ();
	}
	packet.Add(lock_owner_id);

	if (lock_owner_id != 0) {
		packet.Add(LockTimer,BITPACK_VEHICLE_LOCK_TIMER);
	}

	return ;
}


void	VehicleGameObj::Import_Creation( BitStreamClass &packet )
{
	SmartGameObj::Import_Creation( packet );

	//
	//	Get the lock status from the server
	//
	int lock_owner_id;
	packet.Get(lock_owner_id);
	if (lock_owner_id != 0) {
		LockOwner = GameObjManager::Find_PhysicalGameObj( lock_owner_id );
		packet.Get(LockTimer,BITPACK_VEHICLE_LOCK_TIMER);
	}

	return ;
}


void VehicleGameObj::Export_Rare( BitStreamClass &packet )
{
	SmartGameObj::Export_Rare( packet );

	//
	// Export the seat occupants
	//
	for (int i = 0; i < SeatOccupants.Length(); i++) {
		if (SeatOccupants[i] == NULL) {
			const int NO_OCCUPANTS = -1;
			packet.Add(NO_OCCUPANTS);
		} else {
			packet.Add(SeatOccupants[i]->Get_ID());
		}
	}

	packet.Add(VehicleDelivered);
}

void VehicleGameObj::Import_Rare( BitStreamClass &packet )
{
   WWASSERT(CombatManager::I_Am_Only_Client());
	SmartGameObj::Import_Rare( packet );

	//
	// Update the seat occupants
	//
	for (int i = 0; i < SeatOccupants.Length(); i++) {
		int occupant = packet.Get(occupant);
		if (occupant == -1) {

			//
			//	Remove the occupant from the seat
			//
			if (SeatOccupants[i] != NULL) {
				Remove_Occupant (SeatOccupants[i]);
			}

		} else {
			if ((SeatOccupants[i] == NULL) || (SeatOccupants[i]->Get_ID() != occupant)) {
				SmartGameObj * obj = GameObjManager::Find_SmartGameObj(occupant);

				if (SeatOccupants[i] != NULL) {
					Remove_Occupant (SeatOccupants[i]);
				}

				if (obj != NULL) {

					//
					//	Add the occupant to the seat
					//
					if (SeatOccupants[i] == NULL) {
						Add_Occupant (obj->As_SoldierGameObj(), i);
					}
				}
			}
		}
	}

	bool wasDelivered = VehicleDelivered;

	packet.Get(VehicleDelivered);

	if (!wasDelivered && VehicleDelivered) {
		BaseControllerClass* base = BaseControllerClass::Find_Base(Get_Player_Type());

		if (base) {
			base->On_Vehicle_Delivered(this);
		}
	}
}


//-----------------------------------------------------------------------------
/*
**
*/
void VehicleGameObj::Import_Frequent(BitStreamClass & packet)
{
	/*
	if (Get_Definition().Type == VEHICLE_TYPE_TURRET) {
		WWDEBUG_SAY(("VEHICLE_TYPE_TURRET::Import_Frequent: %s, %d, %d\n",
			Get_Definition().Get_Name(),
			Get_Definition().NumSeats,
			SeatOccupants.Length()));
	}
	*/

   WWASSERT(CombatManager::I_Am_Only_Client());

	int rounds = packet.Get(rounds);
	if (Get_Weapon() != NULL) {
		Get_Weapon()->Set_Total_Rounds(rounds);
	}

	switch(Get_Definition().Type) {

		case VEHICLE_TYPE_BIKE:
		case VEHICLE_TYPE_CAR:
		case VEHICLE_TYPE_TANK:
		case VEHICLE_TYPE_FLYING:
		{
			VehiclePhysClass * p_obj = Peek_Vehicle_Phys();

			if (p_obj != NULL) {


				Vector3 sc_position;
				Quaternion q;
				Vector3 vel;
				Vector3 ang_vel;
				bool is_engine_on;

				packet.Get(is_engine_on);
				packet.Get(sc_position.X, BITPACK_WORLD_POSITION_X);
				packet.Get(sc_position.Y, BITPACK_WORLD_POSITION_Y);
				packet.Get(sc_position.Z, BITPACK_WORLD_POSITION_Z);

				packet.Get(q.X, BITPACK_VEHICLE_QUATERNION);
				packet.Get(q.Y, BITPACK_VEHICLE_QUATERNION);
				packet.Get(q.Z, BITPACK_VEHICLE_QUATERNION);
				packet.Get(q.W, BITPACK_VEHICLE_QUATERNION);
				q.Normalize();


#ifdef MULTIPLAYERDEMO
				//
				// Mix up the packet order to make demo/non-demo code more incompatible.
				//
				packet.Get(vel.Y, BITPACK_VEHICLE_VELOCITY);
				packet.Get(vel.Z, BITPACK_VEHICLE_VELOCITY);
				packet.Get(vel.X, BITPACK_VEHICLE_VELOCITY);
#else
				packet.Get(vel.X, BITPACK_VEHICLE_VELOCITY);
				packet.Get(vel.Y, BITPACK_VEHICLE_VELOCITY);
				packet.Get(vel.Z, BITPACK_VEHICLE_VELOCITY);
#endif
				
				packet.Get(ang_vel.X, BITPACK_VEHICLE_ANGULAR_VELOCITY);
				packet.Get(ang_vel.Y, BITPACK_VEHICLE_ANGULAR_VELOCITY);
				packet.Get(ang_vel.Z, BITPACK_VEHICLE_ANGULAR_VELOCITY);

				if (COMBAT_STAR && (COMBAT_STAR->Get_Vehicle() == this)) {
					p_obj->Network_Latency_State_Update(		sc_position,
																			q,
																			vel,
																			ang_vel);
				} else {
					p_obj->Network_Interpolate_State_Update(	sc_position,
																			q,
																			vel,
																			ang_vel,
																			0.1f );
				}
				p_obj->Enable_Engine(is_engine_on);
			}
			break;
		}

		case VEHICLE_TYPE_TURRET:
			//WWDEBUG_SAY(("turret read %d\n", packet.Get_Bit_Write_Position()));
			break;

		default:
			WWASSERT( 0 );
			break;
	}

	packet.Get( DriverIsGunner );

   SmartGameObj::Import_Frequent(packet);

   WWASSERT(packet.Is_Flushed());
}

//-----------------------------------------------------------------------------
void VehicleGameObj::Export_Frequent(BitStreamClass & packet)
{
	Apply_Control();	// Make sure we have our drivers controls
	/*
	if (Get_Definition().Type == VEHICLE_TYPE_TURRET) {
		WWDEBUG_SAY(("VEHICLE_TYPE_TURRET::Export_Frequent: %s, %d, %d\n",
			Get_Definition().Get_Name(),
			Get_Definition().NumSeats,
			SeatOccupants.Length()));
	}
	*/

	//
	// Vehicles never change their weapon type
	//
	int total_rounds = 0;
	if (Get_Weapon() != NULL) {
		total_rounds = Get_Weapon()->Get_Total_Rounds();
	}
	packet.Add(total_rounds);

	switch(Get_Definition().Type) {

		case VEHICLE_TYPE_BIKE:
		case VEHICLE_TYPE_TANK:
		case VEHICLE_TYPE_CAR:
		case VEHICLE_TYPE_FLYING:
		{
			VehiclePhysClass * p_obj = Peek_Vehicle_Phys();
			if (p_obj != NULL) {

				Vector3 pos;
				Quaternion q;
				Vector3 vel;
				Vector3 ang_vel;
				bool is_engine_on;

				p_obj->Get_Position(&pos);
				p_obj->Get_Orientation(&q);
				p_obj->Get_Velocity(&vel);
				p_obj->Get_Angular_Velocity(&ang_vel);
				is_engine_on = p_obj->Is_Engine_Enabled();

				packet.Add(is_engine_on);
				packet.Add(pos.X, BITPACK_WORLD_POSITION_X);
				packet.Add(pos.Y, BITPACK_WORLD_POSITION_Y);
				packet.Add(pos.Z, BITPACK_WORLD_POSITION_Z);

				packet.Add(q.X, BITPACK_VEHICLE_QUATERNION);
				packet.Add(q.Y, BITPACK_VEHICLE_QUATERNION);
				packet.Add(q.Z, BITPACK_VEHICLE_QUATERNION);
				packet.Add(q.W, BITPACK_VEHICLE_QUATERNION);


#ifdef MULTIPLAYERDEMO
				//
				// Mix up the packet order to make demo/non-demo code more incompatible.
				//
				packet.Add(vel.Y, BITPACK_VEHICLE_VELOCITY);
				packet.Add(vel.Z, BITPACK_VEHICLE_VELOCITY);
				packet.Add(vel.X, BITPACK_VEHICLE_VELOCITY);
#else
				packet.Add(vel.X, BITPACK_VEHICLE_VELOCITY);
				packet.Add(vel.Y, BITPACK_VEHICLE_VELOCITY);
				packet.Add(vel.Z, BITPACK_VEHICLE_VELOCITY);
#endif


				packet.Add(ang_vel.X, BITPACK_VEHICLE_ANGULAR_VELOCITY);
				packet.Add(ang_vel.Y, BITPACK_VEHICLE_ANGULAR_VELOCITY);
				packet.Add(ang_vel.Z, BITPACK_VEHICLE_ANGULAR_VELOCITY);
			}

         break;
		}

		case VEHICLE_TYPE_TURRET:
			//WWDEBUG_SAY(("turret write %d\n", packet.Get_Bit_Write_Position()));
			break;

		default:
			WWASSERT(0);
			break;
	}

	packet.Add( DriverIsGunner );

	SmartGameObj::Export_Frequent(packet);

}

//-----------------------------------------------------------------------------
void VehicleGameObj::Import_State_Cs(BitStreamClass & packet)
{
   SmartGameObj::Import_State_Cs(packet);
}

void VehicleGameObj::Export_State_Cs(BitStreamClass & packet)
{
   SmartGameObj::Export_State_Cs(packet);
}

void VehicleGameObj::Get_Velocity(Vector3 & vel)
{
	VehiclePhysClass * vp = Peek_Vehicle_Phys();
	if (vp != NULL) {
		vp->Get_Velocity(&vel);
	} else {
		vel.Set(0,0,0);
	}
}

void VehicleGameObj::Set_Velocity(Vector3 & vel)
{
	VehiclePhysClass * vehicle = Peek_Vehicle_Phys();
	if (vehicle != NULL) {
		vehicle->Set_Velocity(vel);
	}
}

void	VehicleGameObj::Startup( void )
{
	// Setup all Seats for entry
	Create_And_Destroy_Transitions();
}

void	VehicleGameObj::Update_Turret( float weapon_turn, float weapon_tilt )
{
	if ( TurretBone != 0 ) {

		Matrix3D	facing(1);
		facing.Rotate_Z( weapon_turn );

		if ( BarrelBone == 0 ) {		// if no barrel bone
			facing.Rotate_Y( -weapon_tilt );  // neg rotate y tilts up
		}

		Peek_Model()->Control_Bone( TurretBone, facing );
	}

	if ( BarrelBone != 0 ) {
		Matrix3D	facing(1);
		facing.Rotate_Y( -weapon_tilt );  // neg rotate y tilts up
		Peek_Model()->Control_Bone( BarrelBone, facing );
	}

}

/*
** Vehicle Weapons
*/
bool	VehicleGameObj::Set_Targeting( const Vector3 & target_pos, bool do_tilt )
{
	bool	ready = true;

//Debug_Say(( "Weapon set targeting %f %f %f\n", target_pos.X, target_pos.Y, target_pos.Z ));

	SmartGameObj::Set_Targeting( target_pos );

	// Find the desired turret tilt and turn
	float relative_turn = 0;
	float relative_tilt = 0;

	if ( TurretBone != 0 ) {

		// find the target pos in turret space
		Matrix3D	turret_base = Peek_Model()->Get_Bone_Transform( TurretBone );
		Vector3	turret_space_target;
		Matrix3D::Inverse_Transform_Vector( turret_base, target_pos, &turret_space_target );

		// Set the tilt and turn
		relative_turn = WWMath::Atan2( turret_space_target.Y, turret_space_target.X );

		if ( BarrelOffset ) {
			turret_space_target.Z = 0;
			float barrel_offset_angle = WWMath::Atan2( BarrelOffset, turret_space_target.Length() );
//			Debug_Say(( "barrel angle %f %f\n", RAD_TO_DEG( barrel_offset_angle ), RAD_TO_DEG( relative_turn + barrel_offset_angle ) ));
			relative_turn -= barrel_offset_angle;
		}

	}

	if ( WWMath::Fabs( relative_turn ) < DEG_TO_RAD( 80 ) ) {

		if ( BarrelBone != 0 ) {
			// find the target pos in barrel space
			Matrix3D	barrel_base = Peek_Model()->Get_Bone_Transform( BarrelBone );
			Vector3	barrel_space_target;
			Matrix3D::Inverse_Transform_Vector( barrel_base, target_pos, &barrel_space_target );

			float dist = barrel_space_target.Length();
			if ( dist ) {
			// Only tilt when the turn is within 80 deg
				relative_tilt = WWMath::Fast_Asin( barrel_space_target.Z / dist );
			}
		}
	}


	// Move the tilt and turn towards the desired, following rates and limits
	float max_move;
	max_move = Get_Definition().WeaponTurnRate * TimeManager::Get_Frame_Seconds();
	if ( Get_Definition().WeaponTurnRate < DEG_TO_RAD(1000) ) {
		TurretTurn += WWMath::Clamp( relative_turn, -max_move, max_move );
		if ( WWMath::Fabs( relative_turn ) > WWMath::Fabs( max_move ) ) {
			ready = false;
		}
	} else {
		TurretTurn += relative_turn;
	}
	TurretTurn = WWMath::Clamp( TurretTurn, Get_Definition().WeaponTurnMin, Get_Definition().WeaponTurnMax );
	max_move = Get_Definition().WeaponTiltRate * TimeManager::Get_Frame_Seconds();
	if ( do_tilt ) {
		if ( Get_Definition().WeaponTiltRate < DEG_TO_RAD(1000) ) {
			BarrelTilt += WWMath::Clamp( relative_tilt, -max_move, max_move );
			if ( WWMath::Fabs( relative_tilt ) > WWMath::Fabs( max_move ) ) {
				ready = false;
			}
		} else {
			BarrelTilt += relative_tilt;
		}
	}
	BarrelTilt = WWMath::Clamp( BarrelTilt, Get_Definition().WeaponTiltMin, Get_Definition().WeaponTiltMax );

	// Apply the turn and tilt to the bones
	Update_Turret( TurretTurn, BarrelTilt );

	// if a fast turner and had to turn, do it again,just to make sure (trying to fix obelisk)
	if ( Get_Definition().WeaponTurnRate > DEG_TO_RAD(1000) &&
		( WWMath::Fabs(relative_turn) >= DEG_TO_RAD( 2 ) || WWMath::Fabs(relative_tilt) > DEG_TO_RAD( 2 ) ) ) {
		static int calls = 0;
		if ( calls < 3 ) {
			calls++;
			Set_Targeting( target_pos, do_tilt );		// recurse
			calls--;
		}
		ready = true;
	}

	return ready;
}


int	VehicleGameObj::Get_Weapon_Control_Owner(void)
{
	SoldierGameObj * gunner = Get_Gunner();
	if ( gunner && !Get_Driver_Is_Gunner() ) {
		return gunner->Get_Control_Owner();
	}
	SoldierGameObj * driver = Get_Driver();
	if ( driver ) {
		return driver->Get_Control_Owner();
	}
	return Get_Control_Owner();
}


/*
void VehicleGameObj::Apply_Control( void )
{
	SoldierGameObj * driver = Seats[DRIVER_SEAT];
	SoldierGameObj * gunner = Seats[GUNNER_SEAT];
	if ( gunner == NULL ) {
		gunner = driver;
	}

	bool target_steering = false;
	// If I have a driver, use his control
	if ( driver && !driver->In_Transition() ) {
		//Debug_Say(( "Vehicle copy driver's control\n" ));	//copy his control???

		Control = driver->Get_Control();
		target_steering = true;

	} else if (	!Get_Action()->Is_Acting() ) {
		Control.Clear_Control();	// Bad for network????
	}

	// If i have a gunner, use him for the gun control
	if ( gunner && (gunner != driver) && !gunner->In_Transition() ) {
		Control.Set_Boolean( ControlClass::BOOLEAN_WEAPON_FIRE_PRIMARY,		gunner->Get_Control().Get_Boolean( ControlClass::BOOLEAN_WEAPON_FIRE_PRIMARY ) );
		Control.Set_Boolean( ControlClass::BOOLEAN_WEAPON_FIRE_SECONDARY,		gunner->Get_Control().Get_Boolean( ControlClass::BOOLEAN_WEAPON_FIRE_SECONDARY ) );
		Control.Set_Boolean( ControlClass::BOOLEAN_WEAPON_NEXT,		gunner->Get_Control().Get_Boolean( ControlClass::BOOLEAN_WEAPON_NEXT ) );
		Control.Set_Boolean( ControlClass::BOOLEAN_WEAPON_PREV,		gunner->Get_Control().Get_Boolean( ControlClass::BOOLEAN_WEAPON_PREV ) );
		Control.Set_Boolean( ControlClass::BOOLEAN_WEAPON_RELOAD,	gunner->Get_Control().Get_Boolean( ControlClass::BOOLEAN_WEAPON_RELOAD ) );
		Control.Set_Boolean( ControlClass::BOOLEAN_WEAPON_USE,		gunner->Get_Control().Get_Boolean( ControlClass::BOOLEAN_WEAPON_USE ) );
	}

	if ( target_steering ) {
		// Lets make turns come from the targeting
		Vector3 target_pos = Get_Targeting_Pos();
		Vector3 obj_space_target;
		Matrix3D::Inverse_Transform_Vector( Get_Transform(), target_pos, &obj_space_target );
		float target_direction = WWMath::Atan2( obj_space_target.Y, obj_space_target.X );
		float turn_amount = target_direction / DEG_TO_RAD( 60 );
		Control.Set_Analog( ControlClass::ANALOG_TURN_LEFT, turn_amount );
	}

	// Clamp turning
	Control.Set_Analog( ControlClass::ANALOG_TURN_LEFT,
		WWMath::Clamp( Control.Get_Analog( ControlClass::ANALOG_TURN_LEFT ), -1.0F, 1.0F ) );

	SmartGameObj::Apply_Control();
}
*/

void VehicleGameObj::Apply_Control( void )
{
	SoldierGameObj * driver = Get_Driver();
	SoldierGameObj * gunner = Get_Gunner();

	if ( gunner == NULL || Get_Driver_Is_Gunner() ) {
		gunner = driver;
	}

//	if ( gunner || driver ) {
		if ( !CombatManager::Is_Gameplay_Permitted() ) {
			Clear_Control();
			Controller.Reset();
			if ( Peek_Vehicle_Phys() != NULL) {
				Peek_Vehicle_Phys()->Set_Velocity( Vector3(0,0,0) );
			}
			return;
		}
//	}

	bool target_steering = false;
	// If I have a driver, use his control

	if ( driver && !driver->In_Transition() && driver->Is_Human_Controlled () == true) {

		/*
		bool use_control = CombatManager::I_Am_Server() ||
			(CombatManager::Get_Client_Interpolation_Model() !=
				CLIENT_INTERPOLATION_PATHFIND) ||
			(CombatManager::I_Am_Client() &&
			 driver->Get_Control_Owner() == CombatManager::Get_My_Id());

		//Debug_Say(( "Vehicle copy driver's control\n" ));	//copy his control???

		if (use_control) {
			Control = driver->Get_Control();
			target_steering = true;
		}
		*/

		//
		// This could be a little dodgy... does client do this?
		//
		Control = driver->Get_Control();
		target_steering = true;
	}

	// If i have a gunner, use him for the gun control
	if ( gunner && (gunner != driver) && !gunner->In_Transition() && !Get_Driver_Is_Gunner() ) {
		Control.Set_Boolean( ControlClass::BOOLEAN_WEAPON_FIRE_PRIMARY,		gunner->Get_Control().Get_Boolean( ControlClass::BOOLEAN_WEAPON_FIRE_PRIMARY ) );
		Control.Set_Boolean( ControlClass::BOOLEAN_WEAPON_FIRE_SECONDARY,		gunner->Get_Control().Get_Boolean( ControlClass::BOOLEAN_WEAPON_FIRE_SECONDARY ) );
		Control.Set_Boolean( ControlClass::BOOLEAN_WEAPON_RELOAD,	gunner->Get_Control().Get_Boolean( ControlClass::BOOLEAN_WEAPON_RELOAD ) );
	}

	// Vehicles never change weapons
	Control.Set_Boolean( ControlClass::BOOLEAN_WEAPON_NEXT,		false );
	Control.Set_Boolean( ControlClass::BOOLEAN_WEAPON_PREV,		false );


	if ( target_steering && _Use_Target_Steering ) {

		// Target Steering kicks in if the user is pressing the accelerator key.
		if (Control.Get_Analog( ControlClass::ANALOG_MOVE_FORWARD ) >= 0.0f) {

			// Lets make turns come from the targeting
			Vector3 target_pos = Get_Targeting_Pos();
			Vector3 obj_space_target;
			Matrix3D::Inverse_Transform_Vector( Get_Transform(), target_pos, &obj_space_target );
			float target_direction = WWMath::Atan2( obj_space_target.Y, obj_space_target.X );

			// Offset the target direction if the user is turning while driving forward
			const float TARGET_STRAFE_ANGLE = DEG_TO_RADF(50.0f);
			target_direction += TARGET_STRAFE_ANGLE * Control.Get_Analog( ControlClass::ANALOG_TURN_LEFT );

			// Set the final steering angle
			float turn_amount = target_direction / DEG_TO_RAD( 60 ) * Control.Get_Analog( ControlClass::ANALOG_MOVE_FORWARD );
			Control.Set_Analog( ControlClass::ANALOG_TURN_LEFT, turn_amount );

		} else {

			// When over-riding the target steering with keyboard steering,
			// scale the turn rate down if we're going forward.
			if (Control.Get_Analog( ControlClass::ANALOG_MOVE_FORWARD ) > 0.0f) {
				Control.Set_Analog( ControlClass::ANALOG_TURN_LEFT, 0.5f * Control.Get_Analog(ControlClass::ANALOG_TURN_LEFT) );
			}
		}
	}

	// Clamp turning
	Control.Set_Analog( ControlClass::ANALOG_TURN_LEFT,
		WWMath::Clamp( Control.Get_Analog( ControlClass::ANALOG_TURN_LEFT ), -1.0F, 1.0F ) );

	SmartGameObj::Apply_Control();
}

static char * _profile_name = "Vehicle Think";

void	VehicleGameObj::Think( void )
{
{	WWPROFILE( _profile_name );

	Apply_Control();	// ????

	Update_Transitions();
}
	SmartGameObj::Think(); 										// Perform smart object thinking
{	WWPROFILE( _profile_name );

	Update_Sound_Effects();

	Update_Wheel_Effects();

	// Update the lock status
	if (LockTimer > 0.0f) {
		LockTimer -= TimeManager::Get_Frame_Seconds();
	} else {
		LockOwner = NULL;
	}

	// UnStealth if we don't have any occupants, and we aren't in single play
	if (StealthEffect != NULL) {
		if ((Get_Occupant_Count() == 0 ) && ( !IS_MISSION )) {
			StealthEffect->Enable_Stealth(false);
		}
	}
}
}

static char * _post_profile_name = "Vehicle PostThink";

void	VehicleGameObj::Post_Think( void )
{
{	WWPROFILE( _post_profile_name );
	RenderObjClass * model = Peek_Model();
	WWASSERT( model );

	for ( int i = 0; i < SeatOccupants.Length(); i++ ) {
		if ( SeatOccupants[i] != NULL ) {
			char	bone_name[80];
			sprintf( bone_name, "SEAT%d", i );
			int seat_bone_index = model->Get_Bone_Index( bone_name );
			if ( seat_bone_index != -1 ) {
				Matrix3D seat = model->Get_Bone_Transform( seat_bone_index );
				if ( !SeatOccupants[i]->In_Transition() ) {
					SeatOccupants[i]->Set_Transform( seat );
				}
			} else {
				Debug_Say(( "No Seat bone %s\n", bone_name ));
			}
		}
	}
}
	SmartGameObj::Post_Think();

{	WWPROFILE( _post_profile_name );
	if ( Get_Weapon() && !Get_Definition().Fire0Anim.Is_Empty() ) {
		Get_Weapon()->Reset_Anim_Update();
		int state = Get_Weapon()->Get_Anim_State();
		if ( state == WEAPON_ANIM_NOT_FIRING ) {
			Set_Animation( NULL );
		} else if ( state == WEAPON_ANIM_FIRING_0 ) {
			Set_Animation( Get_Definition().Fire0Anim );
		} else if ( state == WEAPON_ANIM_FIRING_1 ) {
			Set_Animation( !Get_Definition().Fire1Anim.Is_Empty() ? Get_Definition().Fire1Anim : Get_Definition().Fire0Anim );
		} else {
			Debug_Say(( "Bad Weapon Anim State\n" ));
		}
	}
}
}


/*
**
*/
int VehicleGameObj::Get_Player_Type(void) const
{
	// If they have a driver, they are his team.
	for (int i = 0; i < SeatOccupants.Length(); i++) {
		if (SeatOccupants[i] != NULL) {
			return SeatOccupants[i]->Get_Player_Type();
		}
	}

#if 0	// Do this at exit time
	// in MP, empty vehicles are neutral
	if ( !IS_MISSION ) {
		return PLAYERTYPE_NEUTRAL;
	}
#endif

	return SmartGameObj::Get_Player_Type();
}

/*
**
*/
void	VehicleGameObj::Init_Wheel_Effects( void )
{
	// Create an array of persistant surface emitters, one for each "real" wheel
	WWASSERT( WheelSurfaceEmitters.Length() == 0 );
	if (	(Peek_Vehicle_Phys() != NULL) &&
			(Peek_Vehicle_Phys()->Get_VehiclePhysDef()->Is_Fake() == false) )
	{
		int real_wheel_count = Peek_Vehicle_Phys()->Get_Real_Wheel_Count();
		WheelSurfaceEmitters.Resize(real_wheel_count);
		for (int i=0; i<real_wheel_count; i++) {
			WheelSurfaceEmitters[i] = SurfaceEffectsManager::Create_Persistant_Emitter();
		}
	}

	// Create a single persistant surface effect sound for tire squealing
	WWASSERT(WheelSurfaceSound == NULL);
	WheelSurfaceSound = SurfaceEffectsManager::Create_Persistant_Sound();
}

void	VehicleGameObj::Shutdown_Wheel_Effects( void )
{
	// Release the persistant surface emitters
	for (int i=0; i<WheelSurfaceEmitters.Length(); i++) {
		SurfaceEffectsManager::Destroy_Persistant_Emitter( WheelSurfaceEmitters[i] );
		WheelSurfaceEmitters[i] = NULL;
	}
	WheelSurfaceEmitters.Clear();

	// Release the persistant surface sound effect
	SurfaceEffectsManager::Destroy_Persistant_Sound(WheelSurfaceSound);
	WheelSurfaceSound = NULL;
}

void	VehicleGameObj::Update_Wheel_Effects( void )
{
	if (	(Peek_Vehicle_Phys() == NULL) ||
			(Peek_Vehicle_Phys()->Get_VehiclePhysDef()->Is_Fake()) )
	{
		return;
	}

	int roll_hitter_type = SurfaceEffectsManager::HITTER_TYPE_TIRE_ROLLING;
	int slide_hitter_type = SurfaceEffectsManager::HITTER_TYPE_TIRE_SLIDING;

	if ( Peek_Vehicle_Phys()->As_TrackedVehicleClass() != NULL) {
		roll_hitter_type = SurfaceEffectsManager::HITTER_TYPE_TRACK_ROLLING;
		slide_hitter_type = SurfaceEffectsManager::HITTER_TYPE_TRACK_SLIDING;
	}

	Vector3 vel;
	Peek_Vehicle_Phys()->Get_Velocity(&vel);
	float vlen2 = vel.Length2();

	// Update the emitters and sound
	const float EMITTER_SLIDE_THRESHOLD = 1.0f;
	const float EMITTER_ROLL_THRESHOLD2 = 2.0f * 2.0f;
	const float EMITTER_CINEMATIC_ROLL_AVEL = DEG_TO_RADF(1.0f);

	bool sound_done = false;

	if (WheelSurfaceEmitters.Length() > 0) {

		if ( Peek_Vehicle_Phys() != NULL) {
			int emitter_index = 0;
			int wheel_count = Peek_Vehicle_Phys()->Get_Wheel_Count();
			for (int i=0; i<wheel_count; i++) {

				SuspensionElementClass * wheel = Peek_Vehicle_Phys()->Peek_Wheel(i);
				if (wheel->Get_Flag(SuspensionElementClass::FAKE) == false) {

					// Look up the surface and hitter types
					int surface_type = wheel->Get_Contact_Surface();
					int hitter_type = SurfaceEffectsManager::HITTER_TYPE_NONE;

					// If stealthed, no emitters.
					if (Is_Stealthed()) {
						surface_type = SURFACE_TYPE_DEFAULT;
					}

					if (wheel->Get_Flag( SuspensionElementClass::INCONTACT ) ) {
						if (wheel->Get_Slip_Factor() > EMITTER_SLIDE_THRESHOLD) {

							hitter_type = slide_hitter_type;

						} else {

							// normally, trigger roll emitters by the velocity of the vehicle
							if (vlen2 > EMITTER_ROLL_THRESHOLD2) {
								hitter_type = roll_hitter_type;
							}

							// when attached to a cinematic, we deduce whether the wheels are rolling
							// by using the rotation delta feature of wheels
							if (Is_Attached_To_An_Object()) {
								float wheel_avel = WWMath::Fabs(wheel->Get_Rotation_Delta() / TimeManager::Get_Frame_Seconds());
								if (wheel_avel > EMITTER_CINEMATIC_ROLL_AVEL) {
									hitter_type = roll_hitter_type;
								}
							}
						}
					}

	   			// Update the emitter and move to the next emitter
					SurfaceEffectsManager::Update_Persistant_Emitter(	WheelSurfaceEmitters[emitter_index],
																						surface_type,
																						hitter_type,
																						wheel->Get_Wheel_Transform());

					emitter_index++;


					// If this is the sound generating wheel, update the sound
					// I'm just using the first engine wheel as the one that generates the tire sound effects.
					if (!sound_done && wheel->Get_Flag(SuspensionElementClass::ENGINE)) {
						SurfaceEffectsManager::Update_Persistant_Sound( WheelSurfaceSound,
																						surface_type,
																						hitter_type,
																						Get_Transform());

						sound_done = true;
					}
				}
			}
		}
	}
}

void VehicleGameObj::Update_Sound_Effects(void)
{
	if (Peek_Vehicle_Phys() == NULL) {
		return;
	}

	switch (EngineSoundState)
	{

	case ENGINE_SOUND_STATE_STARTING:
		{
			// play our ENGINE_STARTING sound, if it is finished or not found, destroy and go to ENGINE_RUNNING
			if ((CachedEngineSound == NULL) || ((CachedEngineSound != NULL) && (CachedEngineSound->Get_State()==AudibleSoundClass::STATE_STOPPED))) {
				Change_Engine_Sound_State(ENGINE_SOUND_STATE_RUNNING);
			}
		}
		break;

	case ENGINE_SOUND_STATE_RUNNING:
		{
			if (CachedEngineSound != NULL) {
				Update_Engine_Sound_Pitch ();
			}
			if (Peek_Vehicle_Phys()->Is_Engine_Enabled() == false) {
				Change_Engine_Sound_State(ENGINE_SOUND_STATE_STOPPING);
			}
		}
		break;

	case ENGINE_SOUND_STATE_STOPPING:
		{
			// play the ENGINE_STOPPING sound, if it is finished or not found, destroy and go to ENGINE_OFF
			if ((CachedEngineSound == NULL) || ((CachedEngineSound != NULL) && (CachedEngineSound->Get_State()==AudibleSoundClass::STATE_STOPPED))) {
				Change_Engine_Sound_State(ENGINE_SOUND_STATE_OFF);
			}
		}
		break;
	case ENGINE_SOUND_STATE_OFF:
		{
			// if the engine is on, go to ENGINE_STARTING
			if (Peek_Vehicle_Phys()->Is_Engine_Enabled() == true) {
				Change_Engine_Sound_State(ENGINE_SOUND_STATE_STARTING);
			}
		}
		break;

	default:
		WWDEBUG_SAY(("Error: Invalid Engine Sound State %d! file: %s line: %d\n",EngineSoundState,__FILE__,__LINE__));
		break;
	}

}

void VehicleGameObj::Change_Engine_Sound_State(int new_state)
{
	if (CachedEngineSound != NULL) {
		CachedEngineSound->Remove_From_Scene();
		CachedEngineSound->Release_Ref();
		CachedEngineSound = NULL;
	}

	EngineSoundState = new_state;

	DefinitionClass * sound_def = DefinitionMgrClass::Find_Definition(Get_Definition().EngineSound[new_state]);
	if (sound_def != NULL) {
		CachedEngineSound = (AudibleSoundClass *)sound_def->Create();
	}

	if (CachedEngineSound != NULL) {
		CachedEngineSound->Add_To_Scene();
		CachedEngineSound->Attach_To_Object(Peek_Model());
	}

	return ;
}

void	VehicleGameObj::Update_Engine_Sound_Pitch(void)
{
	if (CachedEngineSound == NULL) {
		return ;
	}

	float pitch_factor = 1.0F;

	if (Peek_Vehicle_Phys() != NULL) {

		MotorVehicleClass * motophys = Peek_Physical_Object()->As_MotorVehicleClass();
		TrackedVehicleClass * trackphys = Peek_Physical_Object()->As_TrackedVehicleClass();
		VTOLVehicleClass * vtolphys = Peek_Physical_Object()->As_VTOLVehicleClass();

		float max_factor = Get_Definition().EngineSoundMaxPitchFactor;

		if (motophys != NULL) {

			// Wheeled vehicle engine pitch depends on their rpms
			float norm_rpm = motophys->Get_Normalized_Engine_RPM();
			if (norm_rpm < 0) {
				norm_rpm = -norm_rpm;
			}
			norm_rpm = WWMath::Clamp(norm_rpm,0.0f,1.0f);

			pitch_factor = 1.0F + ((max_factor - 1.0F) * norm_rpm);

		} else if (trackphys != NULL) {

			// Tracked vehicle engine pitch depends on their overall speed
			const float MAX_TRACKED_SPEED = 10.0f;
			Vector3 vel;
			trackphys->Get_Velocity(&vel);

			float norm_speed = WWMath::Clamp(vel.Length() / MAX_TRACKED_SPEED,0.0f,1.0f);

			pitch_factor = 1.0F + ((max_factor - 1.0F) * norm_speed);

		} else if (vtolphys != NULL) {

			// VTOL vehicle engine pitch depends on absolute acceleration
			const float MAX_VTOL_SPEED = 20.0f;
			Vector3 vel;
			vtolphys->Get_Velocity(&vel);

			float norm_speed = WWMath::Clamp(vel.Length() / MAX_VTOL_SPEED,0.0f,1.0f);

			pitch_factor = 1.0F + ((max_factor - 1.0F) * norm_speed);
		}
	}

	CachedEngineSound->Set_Pitch_Factor (pitch_factor);
	return ;
}

VehiclePhysClass	*VehicleGameObj::Peek_Vehicle_Phys( void ) const
{
	WWASSERT( Peek_Physical_Object() );
	return Peek_Physical_Object()->As_VehiclePhysClass();		// NOTE: sometimes will return NULL!
}


/*
** Occupants ( drivers, gunners, passengers )
*/
void	VehicleGameObj::Add_Occupant( SoldierGameObj * occupant )
{
	// Add the the lowest empty seat
	for ( int i = 0; i < Get_Definition().NumSeats; i++ ) {
		if ( SeatOccupants[i] == NULL ) {
			Add_Occupant( occupant, i );
			break;;
		}
	}
}

void	VehicleGameObj::Add_Occupant( SoldierGameObj * occupant, int seat_id )
{
	WWASSERT( occupant );

	if ( SeatOccupants[seat_id] != NULL ) {
		Debug_Say(( "Vehicle already has an occupant in that seat\n" ));
		return;
	}

	if ( Find_Seat( occupant ) != -1 ) {
		Debug_Say(( "Soldier already in vehicle\n" ));
	}


	if ( seat_id == 0 ) {
		DriverIsGunner = DefaultDriverIsGunner;
	}

	Debug_Say(( "Soldier %p added to seat %d\n", occupant, seat_id ));

	if ( seat_id > Get_Definition().NumSeats-1 ) {
		Debug_Say(( "Adding to extra seat %d\n", seat_id ));
	}

	SeatOccupants[seat_id] = occupant;
	OccupiedSeats++;
	Set_Object_Dirty_Bit( NetworkObjectClass::BIT_RARE, true );

	const char * anim_name = NULL;
	if ( Get_Definition().Type == VEHICLE_TYPE_BIKE ) {
		anim_name = "S_A_HUMAN.H_A_V20A";
	} else {
		anim_name = "S_A_HUMAN.H_A_V10A";
	}
	occupant->Enter_Vehicle( this, anim_name );

//	Debug_Say(( "Vehicle has a occupant in seat %d\n", seat_id ));

	// possibily delete transitions
	Create_And_Destroy_Transitions();

	Enable_Engine(true);

	if ( Get_Definition().OccupantsVisible == false ) {
		occupant->Peek_Model()->Set_Hidden( true );
	}

	// Notify the observers that someone entered
	const GameObjObserverList & observer_list = Get_Observers();
	for( int index = 0; index < observer_list.Count(); index++ ) {
		observer_list[ index ]->Custom( this, CUSTOM_EVENT_VEHICLE_ENTERED, seat_id, occupant );
	}

	//
	//	Reveal this vehicle to the player if he was the one to enter the vehicle
	//
	if ( occupant == COMBAT_STAR ) {
		EncyclopediaMgrClass::Reveal_Object( this );
	}

	//
	// Reset any action the vehicle was performing
	//
	if (Get_Action() != NULL) {
		Get_Action()->Reset(1);
	}

	//
	// Unlock the vehicle!
	//
	LockTimer = 0.0f;
	LockOwner = NULL;
}

int	VehicleGameObj::Find_Seat( SoldierGameObj * occupant )
{
	for ( int i = 0; i < SeatOccupants.Length(); i++ ) {
		if ( SeatOccupants[i] == occupant ) {
			return i;
		}
	}
	return -1;
}

void	VehicleGameObj::Remove_Occupant( SoldierGameObj * occupant )
{
	WWASSERT( occupant );

	int	seat_num = Find_Seat( occupant );

	if ( seat_num == -1 ) {
		Debug_Say(( "Object is not in this vehicle\n" ));
		return;
	}

	// Do this anyway, just to make sure it happens.
	if ( Get_Definition().OccupantsVisible == false ) {
		if ( SeatOccupants[seat_num] && SeatOccupants[seat_num]->Peek_Model() ) {
			SeatOccupants[seat_num]->Peek_Model()->Set_Hidden( false );
		}
	}

	Debug_Say(( "Soldier %p removed from seat %d\n", occupant, seat_num ));

	SeatOccupants[seat_num]->Exit_Vehicle();
	SeatOccupants[seat_num] = NULL;
	OccupiedSeats--;

	// in MP, empty vehicles are neutral
	if ( !IS_MISSION && OccupiedSeats == 0 ) {
		Set_Player_Type( PLAYERTYPE_NEUTRAL );
	}

	Set_Object_Dirty_Bit( NetworkObjectClass::BIT_RARE, true );

	// possibily re-enable transitions
	Create_And_Destroy_Transitions();

	if ( seat_num == 0 ) {
		Enable_Engine(false);
	}

	// Notify the observers that someone left
	const GameObjObserverList & observer_list = Get_Observers();
	for( int index = 0; index < observer_list.Count(); index++ ) {
		observer_list[ index ]->Custom( this, CUSTOM_EVENT_VEHICLE_EXITED, seat_num, occupant );
	}
}

bool	VehicleGameObj::Contains_Occupant( SoldierGameObj * occupant )
{
	WWASSERT( occupant );

	for ( int i = 0; i < SeatOccupants.Length(); i++ ) {
		if ( SeatOccupants[i] == occupant ) {
			return true;
		}
	}

	return false;
}

int VehicleGameObj::Get_Occupant_Count(void)
{
	int count = 0;
	for ( int i = 0; i < SeatOccupants.Length(); i++ ) {
		if ( SeatOccupants[i] != NULL ) {
			count++;
		}
	}
	return count;
}

bool	VehicleGameObj::Is_Entry_Permitted( SoldierGameObj * p_soldier )
{
	if (!CombatManager::Is_Gameplay_Permitted()) {
		//
		// You can't enter a vehicle if there are no opponents, etc
		//
		return false;
	}

	//
	// Do not permit a soldier to enter a vehicle if there is already
	// another soldier of a different player type in the vehicle.
	// Actually, only allow 2 Nods or 2 GDI's to share a vehicle...
	//

	WWASSERT(p_soldier != NULL);

	bool is_permitted = true;

	int player_type = p_soldier->Get_Player_Type();

	int i;
	for (i = 0; i < SeatOccupants.Length(); i++) {

		if (SeatOccupants[i] != NULL) {
			int seated_pt = SeatOccupants[i]->Get_Player_Type();

			if (seated_pt != player_type ||
				(seated_pt != PLAYERTYPE_NOD && seated_pt != PLAYERTYPE_GDI)) {

				is_permitted = false;
				break;
			}
		}
	}

	// If there are no entry transitions, entry is not possible
	bool entry_exists = false;
	for ( i = 0; i < TransitionInstances.Count(); i++ ) {
//		if ( ( TransitionInstances[i]->Get_Type() == TransitionDataClass::LEGACY_VEHICLE_ENTER_0 ) ||
//			  ( TransitionInstances[i]->Get_Type() == TransitionDataClass::LEGACY_VEHICLE_ENTER_1 ) ) {
		if ( TransitionInstances[i]->Get_Type() == TransitionDataClass::VEHICLE_ENTER ) {
			entry_exists = true;
		}
	}
	if ( !entry_exists ) {
		is_permitted = false;
	}

	// If the vehicle is locked and you are not the lock owner, entry is not permitted
	if (Is_Locked() && (p_soldier != LockOwner.Get_Ptr())) {
		is_permitted = false;
	}

	return is_permitted;
}

void	VehicleGameObj::Remove_Transitions( TransitionDataClass::StyleType transition_type )
{
	for ( int i = 0; i < TransitionInstances.Count(); i++ ) {
		if ( TransitionInstances[i]->Get_Type() == transition_type ) {
			TransitionManager::Destroy( TransitionInstances[i] );
			TransitionInstances.Delete( i );
			i--;
		}
	}
}

void	VehicleGameObj::Create_New_Transitions( TransitionDataClass::StyleType transition_type )
{
	const TRANSITION_DATA_LIST & trans_data_list = Get_Definition().Get_Transition_List();

	for ( int i = 0; i < trans_data_list.Count(); i++ ) {
		if ( trans_data_list[i]->Get_Type() == transition_type ) {
			// make new instance
			TransitionInstanceClass * trans = new TransitionInstanceClass( *trans_data_list[i] );
			// setup
			trans->Set_Vehicle( this );
			// add to our list
			TransitionInstances.Add( trans );
			// add to master list
			TransitionManager::Add( trans );
		}
	}
}

void	VehicleGameObj::Update_Transitions( void )
{
	Matrix3D tm = Get_Transform();

	// Update all transitions
	for ( int i = 0; i < TransitionInstances.Count(); i++ ) {
		TransitionInstances[i]->Set_Parent_Transform( tm );
	}
}


void	VehicleGameObj::Passenger_Entering( void )
{
	// Play the passenger entering animation
	char string[80];
	sprintf( string, "V_%sL1.M_%sCL", (const char *)Get_Definition().TypeName, (const char *)Get_Definition().TypeName );
	Set_Animation( string );
	Get_Anim_Control()->Set_Mode( ANIM_MODE_ONCE );
	//Debug_Say(("VehicleGameObj::Passenger_Entering\n"));
}

void	VehicleGameObj::Passenger_Exiting( void )
{
	// Play the passenger leaving animation
	char string[80];
	sprintf( string, "V_%sL1.M_%sOP", (const char *)Get_Definition().TypeName, (const char *)Get_Definition().TypeName );
	Set_Animation( string );
	Get_Anim_Control()->Set_Mode( ANIM_MODE_ONCE );
	Control.Clear_Control();
}

//void VehicleGameObj::Get_Extended_Information(StringClass & description)
void VehicleGameObj::Get_Description(StringClass & description)
{
	StringClass line;

   line.Format("ID:    %d\n", Get_ID());
	description += line;

   line.Format("NAME:  %s\n", Get_Definition().Get_Name());
	description += line;

   line.Format("TEAM:  %d\n", Get_Player_Type());
	description += line;

   line.Format("CONTR: %d\n", Get_Control_Owner());
	description += line;

	Vector3 position;
	Get_Position(&position);
	line.Format("POS:   %-5.2f, %-5.2f, %-5.2f\n", position.X, position.Y, position.Z);
	description += line;

	Vector3 target = Get_Targeting_Pos();
	line.Format("TGT:   %-5.2f, %-5.2f, %-5.2f\n", target.X, target.Y, target.Z);
	description += line;

	Vector3 velocity;
	Get_Velocity(velocity);
	line.Format("VEL:   %-5.2f, %-5.2f, %-5.2f\n", velocity.X, velocity.Y, velocity.Z);
	description += line;

	line.Format("SPD:   %-5.2f\n", velocity.Length());
	description += line;

	WeaponClass	* p_weapon = Get_Weapon();
	if (p_weapon != NULL) {
		line.Format("WEAP:  %s\n", p_weapon->Get_Name());
		description += line;

		line.Format("RNDS:  %d\n", p_weapon->Get_Total_Rounds());
		description += line;
   }

	if (Get_Defense_Object() != NULL) {
		line.Format("HLTH:  %-5.2f\n", Get_Defense_Object()->Get_Health());
		description += line;
	}

   if (Get_Driver() != NULL) {
		line.Format("DRVR:  %d\n", Get_Driver()->Get_ID());
		description += line;
   }

   if (Get_Gunner() != NULL) {
		line.Format("GUNR:   %d\n", Get_Gunner()->Get_ID());
		description += line;
   }

	line.Format("HIB:   %d\n", Is_Hibernating());
	description += line;

	ControlClass & control = Get_Control();
	line.Format("CTRL:  %d, %d, %5.2f, %5.2f, %5.2f, %5.2f\n",
		control.Get_One_Time_Boolean_Bits(),
		control.Get_Continuous_Boolean_Bits(),
		control.Get_Analog(ControlClass::ANALOG_MOVE_FORWARD),
		control.Get_Analog(ControlClass::ANALOG_MOVE_LEFT),
		control.Get_Analog(ControlClass::ANALOG_MOVE_UP),
		control.Get_Analog(ControlClass::ANALOG_TURN_LEFT));
	description += line;

	if (Get_Action() != NULL) {

		line.Format("ACTCT: %d\n", Get_Action()->Get_Act_Count());
		description += line;

		Vector3 move_loc = Get_Action()->Get_Parameters().MoveLocation;
		line.Format("MVLOC: (%5.2f, %5.2f, %5.2f)\n", move_loc.X, move_loc.Y, move_loc.Z);
		description += line;
	}

	line.Format ("ATTCH: %d\n", Is_Attached_To_An_Object ());
	description += line;

	line.Format("stlth: %d\n", Is_Stealth_Enabled());
	description += line;

	line.Format("   on: %d\n", Is_Stealthed());
	description += line;

	line.Format("ISC:   %d\n", Get_Import_State_Count());
	description += line;
}

bool	VehicleGameObj::Is_Engine_Enabled (void) const
{
	if (Peek_Vehicle_Phys() != NULL) {
		return Peek_Vehicle_Phys()->Is_Engine_Enabled();
	} else {
		return false;
	}
}

void	VehicleGameObj::Enable_Engine (bool onoff)
{
	if (Peek_Vehicle_Phys() != NULL) {
		Peek_Vehicle_Phys()->Enable_Engine(onoff);
	}
}

void VehicleGameObj::Set_Precision(void)
{
	cEncoderList::Set_Precision(BITPACK_VEHICLE_VELOCITY,
		-90.0f, 90.0f, 0.01f); // 200 mph = approx. 90 m/s
	cEncoderList::Set_Precision(BITPACK_VEHICLE_ANGULAR_VELOCITY,
		-20.0f, 20.0f, 0.01f); // to approx 6 PI
	cEncoderList::Set_Precision(BITPACK_VEHICLE_QUATERNION,
		-1.0f, 1.0f, 0.0005f); // -1 to 1
	cEncoderList::Set_Precision(BITPACK_VEHICLE_LOCK_TIMER,
		0.0f, 16.0f, 0.25f);
}

void	VehicleGameObj::Apply_Damage( const OffenseObjectClass & damager, float scale, int alternate_skin )
{
	float starting_health = Get_Defense_Object()->Get_Health();

	//
	//	Let the base class actually handle the call
	//
	SmartGameObj::Apply_Damage( damager, scale, alternate_skin );

	// Stats
	if ( starting_health > 0 && Get_Defense_Object()->Get_Health() <= 0 ) {
		if ( damager.Get_Owner() && damager.Get_Owner()->As_SoldierGameObj() != NULL ) {
			if ( damager.Get_Owner()->As_SoldierGameObj()->Get_Player_Data() != NULL ) {
				damager.Get_Owner()->As_SoldierGameObj()->Get_Player_Data()->Stats_Add_Vehicle_Destroyed();
			}
		}
	}

	//
	//	Now make sure the model is in a state to match its damage level
	//
	Update_Damage_Meshes();
}

void	VehicleGameObj::Update_Damage_Meshes( void )
{
	//
	//	Calculate what percent health we have after this damage
	//
	DefenseObjectClass *defense_object = Get_Defense_Object();
	if ( defense_object != NULL ) {
		float ending_health = defense_object->Get_Health() / defense_object->Get_Health_Max();

		//
		//	Make sure we have a render object to lookup bones on
		//
		RenderObjClass *model = Peek_Model ();
		if (model != NULL) {
			bool show_damage25 = false;
			bool show_damage50 = false;
			bool show_damage75 = false;

			//
			//	Determine what the visibility state of these bones should be
			//
			if (ending_health < 0.25F) {
				show_damage25 = true;
				show_damage50 = true;
				show_damage75 = true;
			} else if (ending_health < 0.5F) {
				show_damage25 = true;
				show_damage50 = true;
			} else if (ending_health < 0.75F) {
				show_damage25 = true;
			}

			static const char *	DAMAGE25_BONE_NAME		= "DAMAGE25";
			static const char *	DAMAGE50_BONE_NAME		= "DAMAGE50";
			static const char *	DAMAGE75_BONE_NAME		= "DAMAGE75";
			static int				DAMAGE25_BONE_NAME_LEN	= ::strlen( DAMAGE25_BONE_NAME );
			static int				DAMAGE50_BONE_NAME_LEN	= ::strlen( DAMAGE50_BONE_NAME );
			static int				DAMAGE75_BONE_NAME_LEN	= ::strlen( DAMAGE75_BONE_NAME );

			//
			//	Loop over all the bones in the model, showing and hiding any that represent
			// damage levels
			//
			int bone_count = model->Get_Num_Bones();
			for (	int index = 0; index < bone_count; index ++) {
				const char *bone_name = model->Get_Bone_Name( index );

				if ( ::strnicmp( bone_name, DAMAGE25_BONE_NAME, DAMAGE25_BONE_NAME_LEN ) == 0 ) {

					//
					//	Show/hide the subobjects associated with 25% damage
					//
					::Set_Subobject_Visibility( model, index, show_damage25 );
				} else if ( ::strnicmp( bone_name, DAMAGE50_BONE_NAME, DAMAGE50_BONE_NAME_LEN ) == 0 ) {

					//
					//	Show/hide the subobjects associated with 50% damage
					//
					::Set_Subobject_Visibility( model, index, show_damage50 );
				} else if ( ::strnicmp( bone_name, DAMAGE75_BONE_NAME, DAMAGE75_BONE_NAME_LEN ) == 0 ) {

					//
					//	Show/hide the subobjects associated with 75% damage
					//
					::Set_Subobject_Visibility( model, index, show_damage75 );
				}
			}
		}
	}

	return ;
}


void	Set_Subobject_Visibility (RenderObjClass *model, int bone_index, bool show)
{
	WWASSERT( model != NULL );

	//
	//	Loop over all the subobjects that are attached to this bone
	//
	int count = model->Get_Num_Sub_Objects_On_Bone( bone_index );
	for (int index = 0; index < count; index ++) {
		RenderObjClass *sub_obj = model->Get_Sub_Object_On_Bone(index, bone_index);
		if (sub_obj != NULL) {

			//
			//	Show or hide this subobject
			//
			sub_obj->Set_Hidden (!show);
			sub_obj->Release_Ref();
		}
	}

	return ;
}

Matrix3D VehicleGameObj::Get_Look_Transform(void)
{
	if ( Get_Definition().SightDownMuzzle ) {
		return Get_Muzzle();
	}
	return SmartGameObj::Get_Look_Transform();
}

const WCHAR *	VehicleGameObj::Get_Vehicle_Name( void )
{
	if ( Get_Definition().VehicleNameID != 0 ) {
		return TranslateDBClass::Get_String( Get_Definition().VehicleNameID );
	}
	return TRANSLATE(IDS_HUD_VEHICLE_NAME);
}

// Vehicles should explode when expired / flipped
ExpirationReactionType	VehicleGameObj::Object_Expired( PhysClass * observed_obj )
{
	// Only on the server
	if ( CombatManager::I_Am_Server() ) {

		if ( Get_Definition().KilledExplosion != 0 && !Is_Delete_Pending() ) {
			Vector3 pos;
			Get_Position(&pos);
			WWASSERT(pos.Is_Valid());// most likely candidate for explosion damage bug....?
			ExplosionManager::Server_Explode( Get_Definition().KilledExplosion,pos,0 );
		}

		if ( !Is_Delete_Pending() ) {
			// notify the observers of killed
			const GameObjObserverList & observer_list = Get_Observers();
			for( int index = 0; index < observer_list.Count(); index++ ) {
				observer_list[ index ]->Killed( this, NULL );
			}
		}

		return SmartGameObj::Object_Expired( observed_obj );

	} else {
		return EXPIRATION_DENIED;
	}
}

/*
**
*/
SoldierGameObj * VehicleGameObj::Get_Driver(void)
{
	if ( SeatOccupants.Length() > DRIVER_SEAT ) {
		return SeatOccupants[DRIVER_SEAT];
	}
	return NULL;
}

SoldierGameObj * VehicleGameObj::Get_Gunner(void)
{
	if ( SeatOccupants.Length() > GUNNER_SEAT ) {
		return SeatOccupants[GUNNER_SEAT];
	}
	return NULL;
}

SoldierGameObj * VehicleGameObj::Get_Actual_Gunner(void)
{
	SoldierGameObj * gunner = Get_Gunner();
	if ( gunner && !Get_Driver_Is_Gunner() ) {
		return gunner;
	}
	return Get_Driver();
}

float VehicleGameObj::Get_Stealth_Fade_Distance(void) const
{
	if (IS_MISSION) {
		return GlobalSettingsDef::Get_Global_Settings()->Get_Stealth_Distance_Vehicle();
	} else {
		return GlobalSettingsDef::Get_Global_Settings()->Get_MP_Stealth_Distance_Vehicle();
	}
}


void VehicleGameObj::Set_Vehicle_Delivered(void)
{
	VehicleDelivered = true;
	Set_Object_Dirty_Bit( NetworkObjectClass::BIT_RARE, true );
}



float VehicleGameObj::Get_Filter_Distance(void) const
{
	if (Get_Definition().Type == VEHICLE_TYPE_TURRET) {
		WeaponClass *weapon = ((VehicleGameObj*)this)->Get_Weapon();
		float range = Get_Definition().SightRange;
		if (weapon != NULL) {
			range = min(weapon->Get_Range(), range);
		}
		return(range);
	}
	return(SmartGameObj::Get_Filter_Distance());
}



void	VehicleGameObj::Ignore_Occupants( void )
{
	for (int i = 0; i < SeatOccupants.Length(); i++) {
		if (SeatOccupants[i] != NULL) {
			if ( SeatOccupants[i]->Peek_Physical_Object() != NULL ) {
				SeatOccupants[i]->Peek_Physical_Object()->Inc_Ignore_Counter();
			}
		}
	}
}

void	VehicleGameObj::Unignore_Occupants( void )
{
	for (int i = 0; i < SeatOccupants.Length(); i++) {
		if (SeatOccupants[i] != NULL) {
			if ( SeatOccupants[i]->Peek_Physical_Object() != NULL ) {
				SeatOccupants[i]->Peek_Physical_Object()->Dec_Ignore_Counter();
			}
		}
	}
}




				/*
				Vector3 current_position;
				Get_Position(&current_position);
				Vector3 pos_error = current_position - sc_position;

				if (pos_error.Length() > 2.0f && Is_Attached_To_An_Object () == false) {
					Matrix3D tm(q, sc_position);
					p_obj->Set_Transform(tm);
					p_obj->Set_Velocity(vel);
					p_obj->Set_Angular_Velocity(ang_vel);
				} else if (Is_Attached_To_An_Object ()) {
					Debug_Say(( "Skipping vehicle position update - attached to bone.\n" ));
				}
				*/

				/*
				// New temporary scheme : constantly correct to server position by linearly
				// interpolating a fraction of the error
				//
				Matrix3D dest_tm(q, sc_position);
				Matrix3D tm = ::Lerp(Get_Transform (), dest_tm, 0.1F);
				p_obj->Set_Transform(tm);
				p_obj->Set_Velocity(vel);
				p_obj->Set_Angular_Velocity(ang_vel);
				*/

				/*
				packet.Add_Vector3(pos);
				packet.Add_Quaternion(q);
				packet.Add_Vector3(vel);
				packet.Add_Vector3(ang_vel);
				*/

















































/*
void VehicleGameObj::Import_State_Sc(BitStreamClass & packet)
{
   WWASSERT(CombatManager::I_Am_Only_Client());

	if (Get_Weapon() != NULL) {
		int rounds = packet.Get(rounds);
		Get_Weapon()->Set_Total_Rounds(rounds);
	}

	for (int i = 0; i < NUM_SEATS; i++) {
		int occupant = packet.Get(occupant);
		if (occupant == -1) {
			Seats[i] = NULL;
		} else {
			if ((Seats[i] == NULL) || (Seats[i]->Get_ID() != occupant)) {
				SmartGameObj * obj = GameObjManager::Find_SmartGameObj(occupant);
				Seats[i] = NULL;
				if (obj != NULL) {
					Seats[i] = obj->As_SoldierGameObj();
				}
			}
		}
	}

	switch(Get_Definition().Type) {

		case VEHICLE_TYPE_BIKE:
		case VEHICLE_TYPE_CAR:
		case VEHICLE_TYPE_TANK:
		{
			VehiclePhysClass * p_obj = Peek_Vehicle_Phys();
			WWASSERT(p_obj != NULL);

			Vector3 sc_position;
			Quaternion q;
			Vector3 vel;
			Vector3 ang_vel;

			//packet.Get_Vector3(pos);
			//packet.Get_Quaternion(q);
			//packet.Get_Vector3(vel);
			//packet.Get_Vector3(ang_vel);

			packet.Get(sc_position.X, BITPACK_WORLD_POSITION_X);
			packet.Get(sc_position.Y, BITPACK_WORLD_POSITION_Y);
			packet.Get(sc_position.Z, BITPACK_WORLD_POSITION_Z);

			packet.Get(q.X, BITPACK_VEHICLE_QUATERNION);
			packet.Get(q.Y, BITPACK_VEHICLE_QUATERNION);
			packet.Get(q.Z, BITPACK_VEHICLE_QUATERNION);
			packet.Get(q.W, BITPACK_VEHICLE_QUATERNION);

			packet.Get(vel.X, BITPACK_VEHICLE_VELOCITY);
			packet.Get(vel.Y, BITPACK_VEHICLE_VELOCITY);
			packet.Get(vel.Z, BITPACK_VEHICLE_VELOCITY);

			packet.Get(ang_vel.X, BITPACK_VEHICLE_ANGULAR_VELOCITY);
			packet.Get(ang_vel.Y, BITPACK_VEHICLE_ANGULAR_VELOCITY);
			packet.Get(ang_vel.Z, BITPACK_VEHICLE_ANGULAR_VELOCITY);

			switch (CombatManager::Get_Client_Interpolation_Model()) {

				case CLIENT_INTERPOLATION_SERVER_AUTHORITATIVE:
				case CLIENT_INTERPOLATION_SERVER_AUTHORITATIVE_WITH_BLENDING:
				{
					//Matrix3D tm(q, sc_position);
					//p_obj->Set_Transform(tm);
					//p_obj->Set_Velocity(vel);
					//p_obj->Set_Angular_Velocity(ang_vel);

					Vector3 current_position;
					Get_Position(&current_position);
					Vector3 pos_error = current_position - sc_position;
					if (pos_error.Length() > 2.0f) {
						Matrix3D tm(q, sc_position);
						p_obj->Set_Transform(tm);
						p_obj->Set_Velocity(vel);
						p_obj->Set_Angular_Velocity(ang_vel);
					}
					break;
				}

				case CLIENT_INTERPOLATION_PATHFIND: {

					SoldierGameObj * p_driver = Seats[DRIVER_SEAT];
					if (p_driver != NULL && p_driver->Get_Control_Owner() == CombatManager::Get_My_Id()) {

						Matrix3D tm(q, sc_position);
						p_obj->Set_Transform(tm);
						p_obj->Set_Velocity(vel);
						p_obj->Set_Angular_Velocity(ang_vel);

					} else {

						ActionParamsStruct parameters;
						parameters.Priority = 1;
						parameters.MoveLocation = sc_position;
						parameters.MoveArrivedDistance = 0.1f;
						WWASSERT(Get_Action() != NULL);
						Get_Action()->Goto(parameters);
					}
					break;
				}

				default:
					WWASSERT(0);
			}
			break;
		}

		case VEHICLE_TYPE_FLYING:
			break;

		case VEHICLE_TYPE_TURRET:
			break;

		default:
			WWASSERT( 0 );
			break;
	}

   SmartGameObj::Import_State_Sc(packet);

   WWASSERT(packet.Is_Flushed());
}
*/
