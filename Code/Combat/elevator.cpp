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
 *                     $Archive:: /Commando/Code/Combat/elevator.cpp                          $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 9/13/01 12:03p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 25                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "elevator.h"
#include "simpledefinitionfactory.h"
#include "persistfactory.h"
#include "wwphysids.h"
#include "bitstream.h"
#include "debug.h"
#include "hanim.h"
#include "wwhack.h"
#include "combat.h"
#include "smartgameobj.h"
#include "gameobjmanager.h"
#include "soldier.h"
#include "wwprofile.h"
#include "AudibleSound.h"
#include "WWAudio.h"
#include "bitpackids.h"


DECLARE_FORCE_LINK( elevatorphys );

/*
** Local constants
*/
enum
{
	EFFECT_DOOR_OPENING,
	EFFECT_DOOR_CLOSING,	
	EFFECT_DOOR_UNLOCKED,
	EFFECT_ACCESS_DENIED,
	EFFECT_ELEVATOR_MOVING,
	EFFECT_ELEVATOR_STOPPED_MOVING,
};


/*
** ElevatorPhysDefClass
*/
SimplePersistFactoryClass<ElevatorPhysDefClass, PHYSICS_CHUNKID_ELEVATORPHYSDEF>	_ElevatorPhysDefPersistFactory;

DECLARE_DEFINITION_FACTORY(ElevatorPhysDefClass, CLASSID_ELEVATORPHYSDEF, "ElevatorPhys") _ElevatorPhysDefDefFactory;

ElevatorPhysDefClass::ElevatorPhysDefClass( void ) : 
	AccessiblePhysDefClass(),
	CloseDelay( 2 ),
	/*LowerCallZone( Vector3( 0,0,0), Vector3( 1,1,1 ) ),
	LowerInsideZone( Vector3( 0,0,0), Vector3( 1,1,1 ) ),
	UpperCallZone( Vector3( 0,0,0), Vector3( 1,1,1 ) ),
	UpperInsideZone( Vector3( 0,0,0), Vector3( 1,1,1 ) ),*/
	DoorClosedTop_FrameNum( -1 ),
	DoorOpeningBottom_FrameNum( -1 ),
	ElevatorStartTop_FrameNum( -1 ),
	ElevatorStoppedBottom_FrameNum( -1 ),
	DoorOpenSoundDefID( 0 ),
	DoorCloseSoundDefID( 0 ),
	DoorUnlockSoundDefID( 0 ),
	DoorAccessDeniedSoundDefID( 0 ),
	ElevatorMovingSoundDefID( 0 )
{
	for (int index = 0; index < ZONE_MAX; index ++) {
		CallZones[index].Center	= Vector3( 0, 0, 0 );
		CallZones[index].Extent	= Vector3( 1, 1, 1 );
	}

	ZONE_PARAM(ElevatorPhysDefClass, CallZones[ZONE_LOWER_CALL],	"LowerCallZone" );
	ZONE_PARAM(ElevatorPhysDefClass, CallZones[ZONE_LOWER_INSIDE], "LowerInsideZone" );
	ZONE_PARAM(ElevatorPhysDefClass, CallZones[ZONE_UPPER_CALL],	"UpperCallZone" );
	ZONE_PARAM(ElevatorPhysDefClass, CallZones[ZONE_UPPER_INSIDE],	"UpperInsideZone" );
	EDITABLE_PARAM(ElevatorPhysDefClass, ParameterClass::TYPE_FLOAT, CloseDelay );

	PARAM_SEPARATOR(ElevatorPhysDefClass, "Frame Numbers");
	EDITABLE_PARAM(ElevatorPhysDefClass, ParameterClass::TYPE_FLOAT,					DoorClosedTop_FrameNum );
	EDITABLE_PARAM(ElevatorPhysDefClass, ParameterClass::TYPE_FLOAT,					DoorOpeningBottom_FrameNum );
	EDITABLE_PARAM(ElevatorPhysDefClass, ParameterClass::TYPE_FLOAT,					ElevatorStartTop_FrameNum );
	EDITABLE_PARAM(ElevatorPhysDefClass, ParameterClass::TYPE_FLOAT,					ElevatorStoppedBottom_FrameNum );
	
	PARAM_SEPARATOR(ElevatorPhysDefClass, "Sounds");
	EDITABLE_PARAM(ElevatorPhysDefClass, ParameterClass::TYPE_SOUNDDEFINITIONID,	DoorOpenSoundDefID );	
	EDITABLE_PARAM(ElevatorPhysDefClass, ParameterClass::TYPE_SOUNDDEFINITIONID,	DoorCloseSoundDefID );
	EDITABLE_PARAM(ElevatorPhysDefClass, ParameterClass::TYPE_SOUNDDEFINITIONID,	DoorUnlockSoundDefID );
	EDITABLE_PARAM(ElevatorPhysDefClass, ParameterClass::TYPE_SOUNDDEFINITIONID,	DoorAccessDeniedSoundDefID );
	EDITABLE_PARAM(ElevatorPhysDefClass, ParameterClass::TYPE_SOUNDDEFINITIONID,	ElevatorMovingSoundDefID );
}

uint32	ElevatorPhysDefClass::Get_Class_ID (void) const	
{ 
	return CLASSID_ELEVATORPHYSDEF; 
}

bool ElevatorPhysDefClass::Is_Type(const char * type_name)
{
	if (stricmp(type_name,ElevatorPhysDefClass::Get_Type_Name()) == 0) {
		return true;
	} else {
		return AccessiblePhysDefClass::Is_Type(type_name);
	}
}

PersistClass *	ElevatorPhysDefClass::Create( void ) const 
{
	ElevatorPhysClass * elevator = new ElevatorPhysClass;
	elevator->Init( *this );
	return elevator;
}

enum	{
	CHUNKID_DEF_OLD_PARENT					=	714001418,
	CHUNKID_DEF_VARIABLES,
	CHUNKID_DEF_PARENT,

	MICROCHUNKID_DEF_CLOSE_DELAY			= 1,
	MICROCHUNKID_DEF_LOCK_CODE,
	MICROCHUNKID_DEF_UPPER_CALL_ZONE,
	MICROCHUNKID_DEF_UPPER_INSIDE_ZONE,
	MICROCHUNKID_DEF_LOWER_CALL_ZONE,
	MICROCHUNKID_DEF_LOWER_INSIDE_ZONE,

	MICROCHUNKID_DEF_DOORCLOSED_FRAMENUM,
	MICROCHUNKID_DEF_DOOROPENING_FRAMENUM,
	MICROCHUNKID_DEF_ELEVATOR_START_FRAMENUM,
	MICROCHUNKID_DEF_ELEVATOR_STOPPED_FRAMENUM,
	MICROCHUNKID_DEF_DOOR_OPEN_SOUNDID,
	MICROCHUNKID_DEF_DOOR_CLOSE_SOUNDID,
	MICROCHUNKID_DEF_DOOR_UNLOCK_SOUNDID,
	MICROCHUNKID_DEF_DOOR_ACCESS_DENIED_SOUNDID,
	MICROCHUNKID_DEF_ELEVATOR_MOVING_SOUNDID
};

bool	ElevatorPhysDefClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_DEF_PARENT );
		AccessiblePhysDefClass::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_DEF_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_CLOSE_DELAY, CloseDelay );
		//WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_LOCK_CODE, LockCode );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_LOWER_CALL_ZONE,		CallZones[ZONE_LOWER_CALL] );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_LOWER_INSIDE_ZONE,	CallZones[ZONE_LOWER_INSIDE] );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_UPPER_CALL_ZONE,		CallZones[ZONE_UPPER_CALL] );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_UPPER_INSIDE_ZONE,	CallZones[ZONE_UPPER_INSIDE] );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_DOORCLOSED_FRAMENUM,			DoorClosedTop_FrameNum );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_DOOROPENING_FRAMENUM,			DoorOpeningBottom_FrameNum );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_ELEVATOR_START_FRAMENUM,		ElevatorStartTop_FrameNum );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_ELEVATOR_STOPPED_FRAMENUM,	ElevatorStoppedBottom_FrameNum );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_DOOR_OPEN_SOUNDID,				DoorOpenSoundDefID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_DOOR_CLOSE_SOUNDID,				DoorCloseSoundDefID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_DOOR_UNLOCK_SOUNDID,			DoorUnlockSoundDefID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_DOOR_ACCESS_DENIED_SOUNDID,	DoorAccessDeniedSoundDefID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_ELEVATOR_MOVING_SOUNDID,		ElevatorMovingSoundDefID );

	csave.End_Chunk();

	return true;
}

bool	ElevatorPhysDefClass::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_DEF_OLD_PARENT:
				StaticAnimPhysDefClass::Load( cload );
				break;

			case CHUNKID_DEF_PARENT:
				AccessiblePhysDefClass::Load( cload );
				break;
  
			case CHUNKID_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_CLOSE_DELAY, CloseDelay );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_LOCK_CODE, LockCode );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_LOWER_CALL_ZONE,					CallZones[ZONE_LOWER_CALL] );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_LOWER_INSIDE_ZONE,				CallZones[ZONE_LOWER_INSIDE] );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_UPPER_CALL_ZONE,					CallZones[ZONE_UPPER_CALL] );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_UPPER_INSIDE_ZONE,				CallZones[ZONE_UPPER_INSIDE] );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_DOORCLOSED_FRAMENUM,				DoorClosedTop_FrameNum );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_DOOROPENING_FRAMENUM,			DoorOpeningBottom_FrameNum );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_ELEVATOR_START_FRAMENUM,		ElevatorStartTop_FrameNum );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_ELEVATOR_STOPPED_FRAMENUM,		ElevatorStoppedBottom_FrameNum );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_DOOR_OPEN_SOUNDID,				DoorOpenSoundDefID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_DOOR_CLOSE_SOUNDID,				DoorCloseSoundDefID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_DOOR_UNLOCK_SOUNDID,				DoorUnlockSoundDefID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_DOOR_ACCESS_DENIED_SOUNDID,	DoorAccessDeniedSoundDefID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_ELEVATOR_MOVING_SOUNDID,		ElevatorMovingSoundDefID );

						default:
							Debug_Say(( "Unrecognized ElevatorPhys Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized ElevatorPhys chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

const PersistFactoryClass & ElevatorPhysDefClass::Get_Factory (void) const 
{ 
	return _ElevatorPhysDefPersistFactory; 
}

/*
**
*/
SimplePersistFactoryClass<ElevatorPhysClass, PHYSICS_CHUNKID_ELEVATORPHYS>	_ElevatorPhysPersistFactory;

const PersistFactoryClass & ElevatorPhysClass::Get_Factory (void) const 
{
	return _ElevatorPhysPersistFactory;
}

/*
**
*/
ElevatorPhysClass::ElevatorPhysClass( void ) :
	CheckTimer( 0 ),
	State( STATE_UP ),
	MovingSoundObj( NULL ),
	PrevFrame( 0 ),
	IsCallTimerSet( false ),
	CallTimer( 0 ),
	TriggerRequest( 0 )
{
	DoorStates[TOP]		= DOOR_STATE_NORMAL;
	DoorStates[BOTTOM]	= DOOR_STATE_NORMAL;
}

ElevatorPhysClass::~ElevatorPhysClass( void )
{
	if ( MovingSoundObj != NULL ) {
		MovingSoundObj->Stop();
		MovingSoundObj->Remove_From_Scene();
		REF_PTR_RELEASE( MovingSoundObj );
	}
}

void ElevatorPhysClass::Init( const ElevatorPhysDefClass & def )
{
	AccessiblePhysClass::Init(def);
	AnimManager.Set_Animation_Mode( AnimCollisionManagerClass::ANIMATE_TARGET );
	AnimManager.Set_Target_Frame( 0 );
}

/*
** Save and Load
*/
enum	{
	CHUNKID_PARENT							=	714001438,
	CHUNKID_VARIABLES,
	CHUNKID_RIDER_OBJECT,

	MICROCHUNKID_							=	1,
	MICROCHUNKID_STATE,
	MICROCHUNKID_TOP_DOOR_STATE,
	MICROCHUNKID_BOTTOM_DOOR_STATE
};

bool	ElevatorPhysClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_PARENT );
		AccessiblePhysClass::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STATE,					State );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TOP_DOOR_STATE,		DoorStates[TOP] );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_BOTTOM_DOOR_STATE,	DoorStates[BOTTOM] );
	csave.End_Chunk();

	if ( CurrentAIRider != NULL ) {
		csave.Begin_Chunk( CHUNKID_RIDER_OBJECT );
		CurrentAIRider.Save( csave );
		csave.End_Chunk();
	}

	return true;
}

bool	ElevatorPhysClass::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PARENT:
				AccessiblePhysClass::Load( cload );
				break;

			case CHUNKID_RIDER_OBJECT:
				CurrentAIRider.Load( cload );
				break;

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {

						READ_MICRO_CHUNK( cload, MICROCHUNKID_STATE,					State );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TOP_DOOR_STATE,		DoorStates[TOP] );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_BOTTOM_DOOR_STATE,	DoorStates[BOTTOM] );

						default:
							Debug_Say(( "Unrecognized ElevatorPhys Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized ElevatorPhys chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	AnimManager.Set_Animation_Mode( AnimCollisionManagerClass::ANIMATE_TARGET );
	AnimManager.Set_Target_Frame( 0 );
	return true;
}

void	ElevatorPhysClass::Save_State( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_PARENT );
		AccessiblePhysClass::Save_State( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STATE,					State );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TOP_DOOR_STATE,		DoorStates[TOP] );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_BOTTOM_DOOR_STATE,	DoorStates[BOTTOM] );
	csave.End_Chunk();

	if ( CurrentAIRider != NULL ) {
		csave.Begin_Chunk( CHUNKID_RIDER_OBJECT );
		CurrentAIRider.Save( csave );
		csave.End_Chunk();
	}
}

#define	LEGACY_STATICANIMPHYS_CHUNK_ANIMMANAGER		 0x01170012

void	ElevatorPhysClass::Load_State( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PARENT:
				AccessiblePhysClass::Load_State( cload );
				break;

			case CHUNKID_RIDER_OBJECT:
				CurrentAIRider.Load( cload );
				break;

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {

						READ_MICRO_CHUNK( cload, MICROCHUNKID_STATE,					State );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TOP_DOOR_STATE,		DoorStates[TOP] );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_BOTTOM_DOOR_STATE,	DoorStates[BOTTOM] );

						default:
							Debug_Say(( "Unrecognized ElevatorPhys Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			case LEGACY_STATICANIMPHYS_CHUNK_ANIMMANAGER:
				AnimManager.Load(cload);
				break;

			default:
				Debug_Say(( "Unrecognized ElevatorPhys chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

}

void	ElevatorPhysClass::Timestep( float dt )
{
	AccessiblePhysClass::Timestep( dt );

	/*const ElevatorPhysDefClass *def = Get_ElevatorPhysDef();

	for (int index = 0; index < ZONE_MAX; index ++) {
		OBBoxClass world_box;
		OBBoxClass::Transform( Get_Transform(), def->CallZones[index], &world_box );

		PhysicsSceneClass::Get_Instance ()->Add_Debug_OBBox (world_box, Vector3 (1, 0, 0));
	}*/
	

	{
		WWPROFILE("ElevatorPhys::Timestep");
		// Delayed Open/close
		if ( AnimManager.Peek_Animation() != NULL ) {
			
			//
			//	Make sure the door sounds play at the appropriate times
			//
			Update_Sound_Effects();

			if ( IsCallTimerSet ) {
				CallTimer -= dt;
			}

			//
			//	Should we evaluate the state of the elevator?
			//
			CheckTimer -= dt;
			if ( CheckTimer <= 0 ) {
				CheckTimer = 0.5F;
				Update_State();				
			}
		}
	}

	return ;
}

void	ElevatorPhysClass::Set_Door_State( int new_state, int door_id  )
{
	if ( new_state != DoorStates[door_id] ) {
		
		switch ( new_state )
		{
			case DOOR_STATE_NORMAL:
				break;

			case DOOR_STATE_UNLOCKED:
				Play_Effect( EFFECT_DOOR_UNLOCKED, door_id == TOP );
				break;

			case DOOR_STATE_ACCESS_DENIED:
				Play_Effect( EFFECT_ACCESS_DENIED, door_id == TOP );
				break;
		}
		
		DoorStates[door_id] = new_state;
	}

	return ;
}

void	ElevatorPhysClass::Set_State( int new_state )
{
	if ( new_state != State ) {
		
		switch ( new_state )
		{
			case STATE_DOWN:
			case STATE_UP:

				//
				//	Leave the elevator in this state for at least 4 seconds (before it checks
				// to see if it should move to the next level).
				//
				//Timer = 4;
				break;

			case STATE_MOVING_UP:
				AnimManager.Set_Animation_Mode( AnimCollisionManagerClass::ANIMATE_TARGET );
				AnimManager.Set_Target_Frame( 0 );
				break;

			case STATE_MOVING_DOWN:
				AnimManager.Set_Animation_Mode( AnimCollisionManagerClass::ANIMATE_TARGET );
				AnimManager.Set_Target_Frame( AnimManager.Peek_Animation()->Get_Num_Frames() - 1 );
				break;
		}
		
		State = new_state;
	}

	return ;
}

void	ElevatorPhysClass::Update_State(void)
{
	switch ( State ) {

		case STATE_MOVING_UP:
			if ( AnimManager.Get_Current_Frame() == 0 ) {
				Set_State( STATE_UP );
			}
			break;

		case STATE_MOVING_DOWN:
			if ( AnimManager.Get_Current_Frame() == AnimManager.Peek_Animation()->Get_Num_Frames() - 1 ) {
				Set_State( STATE_DOWN );
			}
			break;

		case STATE_UP:
			if ( CombatManager::I_Am_Server() ) {
								
				if ( IsCallTimerSet && CallTimer <= 0 ) {
					IsCallTimerSet = false;

					if ( Triggered( ZONE_LOWER_CALL ) || Triggered( ZONE_UPPER_INSIDE ) ) {
						Set_State( STATE_MOVING_DOWN );
						TriggerRequest = 0;
					}

				} else  if ( IsCallTimerSet == false ) {

					bool in_call	= Triggered( ZONE_LOWER_CALL );
					bool in_inside	= Triggered( ZONE_UPPER_INSIDE );
					
					if ( in_call && !in_inside ) {
						IsCallTimerSet	= true;
						CallTimer		= 1.0F;
					} else if ( in_call || in_inside ) {
						IsCallTimerSet	= true;
						CallTimer		= 1.0F;
					}
				}
			}
			break;

		case STATE_DOWN:
			if ( CombatManager::I_Am_Server() ) {
				
				if ( IsCallTimerSet && CallTimer <= 0 ) {
					IsCallTimerSet = false;

					if ( Triggered( ZONE_LOWER_INSIDE ) || Triggered( ZONE_UPPER_CALL ) ) {
						Set_State( STATE_MOVING_UP );	
						TriggerRequest = 0;
					}

				} else if ( IsCallTimerSet == false ) {
					
					bool in_call	= Triggered( ZONE_UPPER_CALL );
					bool in_inside	= Triggered( ZONE_LOWER_INSIDE );

					if ( in_call && !in_inside ) {
						IsCallTimerSet	= true;
						CallTimer		= 1.0F;
					} else if ( in_call || in_inside ) {
						IsCallTimerSet	= true;
						CallTimer		= 1.0F;
					}
				}
			}
			break;
	}

	return ;
}

inline bool Is_Top_Zone ( int zone_id )
{
	return ((zone_id > ZONE_LOWER_INSIDE) && (zone_id <= ZONE_UPPER_INSIDE));
}

inline bool Is_Call_Zone ( int zone_id )
{
	return (zone_id == ZONE_LOWER_CALL) || (zone_id == ZONE_UPPER_CALL);
}

bool	ElevatorPhysClass::Triggered( int zone_id )
{	
	const ElevatorPhysDefClass *def = Get_ElevatorPhysDef();

	OBBoxClass world_box;
	OBBoxClass::Transform( Get_Transform(), def->CallZones[zone_id], &world_box );

	//
	//	Get information about the zone we are checking
	//
	bool is_top		= ::Is_Top_Zone( zone_id );
	bool is_call	= ::Is_Call_Zone( zone_id );

	//
	// Quick return if an AI is requesting the elevator
	//
	if (TriggerRequest & (1 << zone_id)) {
		return true;
	}

	//
	//	Get the list of all the players in the world
	//
	SList<SoldierGameObj> *player_list = GameObjManager::Get_Star_Game_Obj_List();

	//
	//	Loop over each player
	//
	SLNode<SoldierGameObj> *objnode;
	for (	objnode = player_list->Head(); objnode; objnode = objnode->Next()) {
		SoldierGameObj *soldier = objnode->Data();
		if ( soldier != NULL ) {

			//
			//	Is this player inside the trigger zone?
			//
			Vector3 pos;
			soldier->Get_Position( &pos );
			if ( CollisionMath::Overlap_Test(world_box, pos) == CollisionMath::INSIDE ) {

				//
				//	Is the door locked?
				//
				if ( Get_ElevatorPhysDef()->LockCode != 0 ) {

					bool open_door = true;
					if ( is_call ) {
						if ( soldier->Has_Key( Get_ElevatorPhysDef()->LockCode ) ) {
							
							//
							//	The player just unlocked this door...
							//
							Set_Door_State( DOOR_STATE_UNLOCKED, is_top ? TOP : BOTTOM );

						} else {

							//
							//	Someone tried to enter this door without unlocking it...
							//
							Set_Door_State( DOOR_STATE_ACCESS_DENIED, is_top ? TOP : BOTTOM );
							open_door = false;
						}
					}

					if ( open_door ) {
						return true;
					}

				} else {
					
					//
					//	Reset the state of the door to 'normal'
					//
					if ( is_call ) {
						Set_Door_State( DOOR_STATE_NORMAL, is_top ? TOP : BOTTOM );
					}

					return true;
				}
			}
		}
	}

	//
	//	Reset the state of the door to 'normal'
	//
	if ( is_call ) {
		Set_Door_State( DOOR_STATE_NORMAL, is_top ? TOP : BOTTOM );
	}
	return false;
}


inline bool Has_Frame_Occured (float prev_frame, float current_frame, float target_frame)
{
	bool retval = false;

	//
	//	Check to see if the animation passed the target frame in either direction
	//
	if ( target_frame >= 0 ) {
		retval =		((prev_frame <= target_frame) && (current_frame >= target_frame));
		retval |=	((prev_frame >= target_frame) && (current_frame <= target_frame));
	}

	return retval;
}


void	ElevatorPhysClass::Update_Sound_Effects( void )
{
	const ElevatorPhysDefClass *def = Get_ElevatorPhysDef();

	float curr_frame	= AnimManager.Get_Current_Frame();
	float prev_frame	= PrevFrame;

	if ( State == STATE_MOVING_DOWN ) {

		//
		//	Check to see if we should play the opening or closing sounds on the door
		//
		if ( Has_Frame_Occured( prev_frame, curr_frame, 0 ) ) {
			Play_Effect( EFFECT_DOOR_CLOSING,  true );
		} else if ( Has_Frame_Occured( prev_frame, curr_frame, def->DoorOpeningBottom_FrameNum ) ) {
			Play_Effect( EFFECT_DOOR_OPENING,  false );
		}
		
		//
		//	Check to see if we should start or stop the elevator moving sound
		//
		if ( Has_Frame_Occured( prev_frame, curr_frame, def->ElevatorStartTop_FrameNum ) ) {
			Play_Effect( EFFECT_ELEVATOR_MOVING,  true );
		} else if ( Has_Frame_Occured( prev_frame, curr_frame, def->ElevatorStoppedBottom_FrameNum ) ) {
			Play_Effect( EFFECT_ELEVATOR_STOPPED_MOVING,  false );
		}
		
	} else if ( State == STATE_MOVING_UP ) {

		//
		//	Check to see if we should play the opening or closing sounds on the door
		//
		if ( Has_Frame_Occured( prev_frame, curr_frame, AnimManager.Peek_Animation()->Get_Num_Frames()-1 ) ) {
			Play_Effect( EFFECT_DOOR_CLOSING,  false );
		} else if ( Has_Frame_Occured( prev_frame, curr_frame, def->DoorClosedTop_FrameNum ) ) {
			Play_Effect( EFFECT_DOOR_OPENING,  true );
		}
		
		//
		//	Check to see if we should start or stop the elevator moving sound
		//
		if ( Has_Frame_Occured( prev_frame, curr_frame, def->ElevatorStoppedBottom_FrameNum ) ) {
			Play_Effect( EFFECT_ELEVATOR_MOVING,  false );
		} else if ( Has_Frame_Occured( prev_frame, curr_frame, def->ElevatorStartTop_FrameNum ) ) {
			Play_Effect( EFFECT_ELEVATOR_STOPPED_MOVING, true );
		}
	}
	
	PrevFrame = curr_frame;
	return ;
}


void	ElevatorPhysClass::Play_Effect( int effect_id, bool is_top )
{
	const ElevatorPhysDefClass *def = Get_ElevatorPhysDef();

	switch ( effect_id )
	{
		case EFFECT_DOOR_OPENING:
		{
			Matrix3D tm(1);
			Get_Door_Transform( is_top, tm );
			WWAudioClass::Get_Instance()->Create_Instant_Sound( def->DoorOpenSoundDefID, tm );
		}
		break;

		case EFFECT_DOOR_CLOSING:
		{
			Matrix3D tm(1);
			Get_Door_Transform( is_top, tm );
			WWAudioClass::Get_Instance()->Create_Instant_Sound( def->DoorCloseSoundDefID, tm );
		}
		break;

		case EFFECT_DOOR_UNLOCKED:
		{
			Matrix3D tm(1);
			Get_Door_Transform( is_top, tm );
			WWAudioClass::Get_Instance()->Create_Instant_Sound( def->DoorUnlockSoundDefID, tm );
		}
		break;

		case EFFECT_ACCESS_DENIED:
		{
			Matrix3D tm(1);
			Get_Door_Transform( is_top, tm );
			WWAudioClass::Get_Instance()->Create_Instant_Sound( def->DoorAccessDeniedSoundDefID, tm );
		}
		break;

		case EFFECT_ELEVATOR_MOVING:
		{
			if ( def->ElevatorMovingSoundDefID != 0 ) {

				RenderObjClass *model = Peek_Model ();
				if (model != NULL) {
				
					//
					//	Lookup the bone we need to attach this sound to
					//
					int bone_index = model->Get_Bone_Index( "ELESOUND" );
					if ( bone_index > 0) {

						//
						//	Create the sound object (if necessary)
						//
						if ( MovingSoundObj == NULL ) {
							MovingSoundObj = WWAudioClass::Get_Instance ()->Create_Continuous_Sound( def->ElevatorMovingSoundDefID );
						}

						//
						//	Attach the sound to the elevator car and insert it into the scene
						//
						if ( MovingSoundObj != NULL ) {
							MovingSoundObj->Attach_To_Object( model, bone_index );
							MovingSoundObj->Add_To_Scene( true );
						}
					}
				}
			}
		}
		break;

		case EFFECT_ELEVATOR_STOPPED_MOVING:
		{
			if ( MovingSoundObj != NULL ) {
				MovingSoundObj->Stop();
				MovingSoundObj->Remove_From_Scene();
			}
		}
		break;
	}

	return ;
}


void	ElevatorPhysClass::Get_Door_Transform( bool is_top, Matrix3D &tm )
{
	const ElevatorPhysDefClass *def = Get_ElevatorPhysDef();
	tm.Make_Identity();

	Matrix3D curr_tm = Get_Transform();

	//
	//	We assume the door is 'about' halfway between the call and inside zones.
	//
	if ( is_top ) {
		Vector3 zone_vector	= def->CallZones[ZONE_UPPER_CALL].Center - def->CallZones[ZONE_UPPER_INSIDE].Center;
		Vector3 center_point	= def->CallZones[ZONE_UPPER_INSIDE].Center + (zone_vector * 0.5F);
		tm.Set_Translation( center_point + curr_tm.Get_Translation () );
	} else {
		Vector3 zone_vector	= def->CallZones[ZONE_LOWER_CALL].Center - def->CallZones[ZONE_LOWER_INSIDE].Center;
		Vector3 center_point	= def->CallZones[ZONE_LOWER_INSIDE].Center + (zone_vector * 0.5F);
		tm.Set_Translation( center_point + curr_tm.Get_Translation () );
	}

	return ;
}

void ElevatorPhysClass::Set_Precision(void)
{
	cEncoderList::Set_Precision( BITPACK_ELEVATOR_STATE,					0, (int)ElevatorPhysClass::STATE_MAX );
	cEncoderList::Set_Precision( BITPACK_ELEVATOR_TOP_DOOR_STATE,		0, (int)ElevatorPhysClass::DOOR_STATE_MAX );
	cEncoderList::Set_Precision( BITPACK_ELEVATOR_BOTTOM_DOOR_STATE,	0, (int)ElevatorPhysClass::DOOR_STATE_MAX );
	return ;
}

bool ElevatorPhysClass::Can_Object_Enter (SmartGameObj *game_obj)
{
	bool retval = false;

	WWASSERT( game_obj != NULL && game_obj->Peek_Physical_Object () != NULL );
	if( game_obj == NULL || game_obj->Peek_Physical_Object () == NULL ) {
		return false;
	}

	//
	//	Don't let other objects on if we've already got an AI rider
	//
	if (CurrentAIRider != NULL) {
		return false;
	}

	//
	//	Get the object's bounding box
	//
	RenderObjClass *model		= game_obj->Peek_Physical_Object()->Peek_Model ();
	const AABoxClass &obj_box	= model->Get_Bounding_Box ();

	//
	//	Get the bottom and top call zones
	//
	const ElevatorPhysDefClass *def = Get_ElevatorPhysDef();
	OBBoxClass bottom_box;
	OBBoxClass top_box;

	OBBoxClass::Transform( Get_Transform(), def->CallZones[STATE_DOWN], &bottom_box );
	OBBoxClass::Transform( Get_Transform(), def->CallZones[STATE_UP], &top_box );

	//
	//	Which floor is the object waiting on?
	//
	if( CollisionMath::Intersection_Test( bottom_box, obj_box ) ) {
		retval = (State == STATE_DOWN);
	} else if( CollisionMath::Intersection_Test( top_box, obj_box ) ) {
		retval = (State == STATE_UP);
	}
	
	return retval;
}

bool ElevatorPhysClass::Can_Object_Exit (SmartGameObj * /*game_obj*/)
{
	bool retval = false;

	//
	//	If the elevator is at rest, then the object can exit
	//
	if (State == STATE_DOWN || State == STATE_UP) {
		retval = true;
	}

	return retval;
}

void ElevatorPhysClass::Request_Elevator (SmartGameObj *game_obj)
{
	WWASSERT( game_obj != NULL && game_obj->Peek_Physical_Object () != NULL );
	if( game_obj == NULL || game_obj->Peek_Physical_Object () == NULL ) {
		return ;
	}

	//
	//	Get the object's bounding box
	//
	RenderObjClass *model		= game_obj->Peek_Physical_Object()->Peek_Model ();
	const AABoxClass &obj_box	= model->Get_Bounding_Box ();

	//
	//	Get the bottom and top call zones
	//
	const ElevatorPhysDefClass *def = Get_ElevatorPhysDef();
	
	//
	//	See what trigger zone the object is in...
	//
	for ( int index = 0; index < ZONE_MAX; index ++) {
		
		//
		//	Transform the trigger zone into world space
		//
		OBBoxClass world_box;
		OBBoxClass::Transform( Get_Transform(), def->CallZones[index], &world_box );

		//
		//	Is this the trigger zone the object is standing in?
		//
		if( CollisionMath::Intersection_Test( world_box, obj_box ) ) {
			TriggerRequest |= (1 << index);
		}
	}
	
	return ;
}


int ElevatorPhysClass::Get_Floor (void)
{
	 int floor = 13;

	 //
	 // Return 0 if the elevator is currently sitting on the bottom floor
	 // or return 1 if the elevator is sitting at the top floor
	 //
	 if (State == STATE_DOWN) {
		floor = 0;
	 } else if (State == STATE_UP) {
		floor = 1;
	 }

	 return floor;
}
