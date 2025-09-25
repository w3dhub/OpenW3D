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
 *                     $Archive:: /Commando/Code/Combat/doors.cpp                             $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 1/15/02 3:07p                                               $*
 *                                                                                             *
 *                    $Revision:: 47                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "doors.h"
#include "simpledefinitionfactory.h"
#include "persistfactory.h"
#include "wwphysids.h"
#include "debug.h"
#include "hanim.h"
#include "wwhack.h"
#include "combat.h"
#include "smartgameobj.h"
#include "gameobjmanager.h"
#include "soldier.h"
#include "WWAudio.h"
#include "wwprofile.h"
#include "bitpackids.h"
#include "diaglog.h"
#include "vehicle.h"


DECLARE_FORCE_LINK( doorphys );

/*
** DoorPhysDefClass
*/
SimplePersistFactoryClass<DoorPhysDefClass, PHYSICS_CHUNKID_DOORPHYSDEF>	_DoorPhysDefPersistFactory;

DECLARE_DEFINITION_FACTORY(DoorPhysDefClass, CLASSID_DOORPHYSDEF, "DoorPhys") _DoorPhysDefDefFactory;

DoorPhysDefClass::DoorPhysDefClass( void ) :
	AccessiblePhysDefClass(),
	CloseDelay( 2 ),
	TriggerZone1( Vector3( 0,0,0), Vector3( 1,1,1 ) ),
	TriggerZone2( Vector3( 0,0,0), Vector3( 1,1,1 ) ),
	OpenSoundDefID( 0 ),
	CloseSoundDefID( 0 ),
	UnlockSoundDefID( 0 ),
	AccessDeniedSoundDefID( 0 ),
	DoorOpensForVehicles( false )
{
	ZONE_PARAM(DoorPhysDefClass, TriggerZone1, "TriggerZone1" );
	ZONE_PARAM(DoorPhysDefClass, TriggerZone2, "TriggerZone2" );
	EDITABLE_PARAM(DoorPhysDefClass, ParameterClass::TYPE_FLOAT, CloseDelay );
	EDITABLE_PARAM(DoorPhysDefClass, ParameterClass::TYPE_SOUNDDEFINITIONID,	OpenSoundDefID);
	EDITABLE_PARAM(DoorPhysDefClass, ParameterClass::TYPE_SOUNDDEFINITIONID,	CloseSoundDefID);
	EDITABLE_PARAM(DoorPhysDefClass, ParameterClass::TYPE_SOUNDDEFINITIONID,	UnlockSoundDefID);
	EDITABLE_PARAM(DoorPhysDefClass, ParameterClass::TYPE_SOUNDDEFINITIONID,	AccessDeniedSoundDefID);
	EDITABLE_PARAM(DoorPhysDefClass, ParameterClass::TYPE_BOOL,	DoorOpensForVehicles);
}

uint32	DoorPhysDefClass::Get_Class_ID (void) const
{
	return CLASSID_DOORPHYSDEF;
}

bool DoorPhysDefClass::Is_Type(const char * type_name)
{
	if (openw3d::string_compare(type_name,DoorPhysDefClass::Get_Type_Name()) == 0) {
		return true;
	} else {
		return AccessiblePhysDefClass::Is_Type(type_name);
	}
}

PersistClass *	DoorPhysDefClass::Create( void ) const
{
	DoorPhysClass * door = new DoorPhysClass;
	door->Init( *this );
	return door;
}

enum	{
	CHUNKID_DEF_OLD_PARENT						=	320001902,
	CHUNKID_DEF_VARIABLES,
	CHUNKID_DEF_PARENT,

	XXXMICROCHUNKID_DEF_TRIGGER_RADIUS		=	1,
	MICROCHUNKID_DEF_CLOSE_DELAY,
	MICROCHUNKID_DEF_TRIGGER_ZONE1,
	MICROCHUNKID_DEF_OLD_LOCK_CODE,
	MICROCHUNKID_DEF_OPEN_SOUND_DEF_ID,
	MICROCHUNKID_DEF_CLOSE_SOUND_DEF_ID,
	MICROCHUNKID_DEF_UNLOCK_SOUND_DEF_ID,
	MICROCHUNKID_DEF_ACCESS_DENIED_SOUND_DEF_ID,
	MICROCHUNKID_DEF_TRIGGER_ZONE2,
	MICROCHUNKID_DEF_DOOR_OPENS_FOR_VEHICLES,
};

bool	DoorPhysDefClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_DEF_PARENT );
		AccessiblePhysDefClass::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_DEF_VARIABLES );
//		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_TRIGGER_RADIUS, TriggerRadius );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_CLOSE_DELAY, CloseDelay );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_TRIGGER_ZONE1, TriggerZone1 );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_TRIGGER_ZONE2, TriggerZone2 );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_OPEN_SOUND_DEF_ID, OpenSoundDefID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_CLOSE_SOUND_DEF_ID, CloseSoundDefID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_UNLOCK_SOUND_DEF_ID, UnlockSoundDefID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_ACCESS_DENIED_SOUND_DEF_ID, AccessDeniedSoundDefID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_DOOR_OPENS_FOR_VEHICLES, DoorOpensForVehicles );
	csave.End_Chunk();

	return true;
}

bool	DoorPhysDefClass::Load( ChunkLoadClass &cload )
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
//						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_TRIGGER_RADIUS, TriggerRadius );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_CLOSE_DELAY, CloseDelay );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_TRIGGER_ZONE1, TriggerZone1 );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_TRIGGER_ZONE2, TriggerZone2 );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_OLD_LOCK_CODE, LockCode );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_OPEN_SOUND_DEF_ID,				OpenSoundDefID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_CLOSE_SOUND_DEF_ID,				CloseSoundDefID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_UNLOCK_SOUND_DEF_ID,				UnlockSoundDefID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_ACCESS_DENIED_SOUND_DEF_ID,	AccessDeniedSoundDefID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_DOOR_OPENS_FOR_VEHICLES, DoorOpensForVehicles );

						default:
							Debug_Say(( "Unrecognized DoorPhys Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized DoorPhys chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

const PersistFactoryClass & DoorPhysDefClass::Get_Factory (void) const
{
	return _DoorPhysDefPersistFactory;
}

/*
**
*/
SimplePersistFactoryClass<DoorPhysClass, PHYSICS_CHUNKID_DOORPHYS>	_DoorPhysPersistFactory;

const PersistFactoryClass & DoorPhysClass::Get_Factory (void) const
{
	return _DoorPhysPersistFactory;
}


/*
**
*/
DoorPhysClass::DoorPhysClass( void ) :
	Timer( 0 ),
	CheckTimer( 0 ),
	State( STATE_CLOSED_DOOR ),
	LockState( false ),
	OpenRequestPending( false )
{
}

DoorPhysClass::~DoorPhysClass( void )
{
}

void DoorPhysClass::Init( const DoorPhysDefClass & def )
{
	AccessiblePhysClass::Init(def);
	AnimManager.Set_Animation_Mode( AnimCollisionManagerClass::ANIMATE_TARGET );
	AnimManager.Set_Target_Frame( 0 );
}

/*
** Save and Load
*/
enum	{
	CHUNKID_OLD_PARENT						=	320002202,
	CHUNKID_VARIABLES,
	CHUNKID_PARENT,

	XXXMICROCHUNKID_							=	1,
	MICROCHUNKID_STATE						=	2,
	MICROCHUNKID_TIMER						=	3,
	MICROCHUNKID_CHECK_TIMER				=	4,
	MICROCHUNKID_OPEN_REQUEST_PENDING	=	5,
};

bool	DoorPhysClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_PARENT );
		AccessiblePhysClass::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STATE, State );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TIMER, Timer );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_CHECK_TIMER, CheckTimer );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_OPEN_REQUEST_PENDING, OpenRequestPending );
	csave.End_Chunk();

	return true;
}

bool	DoorPhysClass::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_OLD_PARENT:
				StaticAnimPhysClass::Load( cload );
				break;

			case CHUNKID_PARENT:
				AccessiblePhysClass::Load( cload );
				break;

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STATE, State );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TIMER, Timer );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_CHECK_TIMER, CheckTimer );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_OPEN_REQUEST_PENDING, OpenRequestPending );
						default:
							Debug_Say(( "Unrecognized DoorPhys Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized DoorPhys chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	AnimManager.Set_Animation_Mode( AnimCollisionManagerClass::ANIMATE_TARGET );
	AnimManager.Set_Target_Frame( 0 );
	return true;
}

void	DoorPhysClass::Save_State( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_PARENT );
		AccessiblePhysClass::Save_State( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STATE, State );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TIMER, Timer );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_CHECK_TIMER, CheckTimer );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_OPEN_REQUEST_PENDING, OpenRequestPending );
	csave.End_Chunk();
}

#define	LEGACY_STATICANIMPHYS_CHUNK_ANIMMANAGER		 0x01170012

void	DoorPhysClass::Load_State( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_OLD_PARENT:
				StaticAnimPhysClass::Load_State( cload );
				break;

			case CHUNKID_PARENT:
				AccessiblePhysClass::Load_State( cload );
				break;

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STATE, State );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TIMER, Timer );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_CHECK_TIMER, CheckTimer );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_OPEN_REQUEST_PENDING, OpenRequestPending );
						default:
							Debug_Say(( "Unrecognized DoorPhys Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			case LEGACY_STATICANIMPHYS_CHUNK_ANIMMANAGER:
				AnimManager.Load(cload);
				break;

			default:
				Debug_Say(( "Unrecognized DoorPhys chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

}

void	DoorPhysClass::Timestep( float dt )
{
	AccessiblePhysClass::Timestep( dt );

	{
		WWPROFILE("DoorPhys::Timestep");

		//
		//	If we are configured correctly with an animation,
		// then evaluate our state and determine if we need to
		// do anything.
		//
		if ( AnimManager.Peek_Animation() != NULL ) {
			Update_State( dt );
		}

		//
		//	Reset the open request flag
		//
		OpenRequestPending = false;
	}

	return ;
}

bool DoorPhysClass::Set_State( int new_state )
{
	bool retval = false;

	if ( State != new_state ) {

		switch ( new_state )
		{
			case STATE_OPENING_DOOR:
			{
				//
				//	Did someone just unlock the door?
				//
				if ( Get_DoorPhysDef()->LockCode != 0 ) {
					WWAudioClass::Get_Instance()->Create_Instant_Sound( Get_DoorPhysDef()->UnlockSoundDefID, Get_Transform() );
				}

				//
				//	Play the 'door-opening' sound
				//
				WWAudioClass::Get_Instance()->Create_Instant_Sound( Get_DoorPhysDef()->OpenSoundDefID, Get_Transform() );

				//
				//	Open the door
				//
				AnimManager.Set_Animation_Mode( AnimCollisionManagerClass::ANIMATE_TARGET );
				AnimManager.Set_Target_Frame( AnimManager.Peek_Animation()->Get_Num_Frames() - 1	);
			}
			break;

			case STATE_CLOSING_DOOR:
			{
				//
				//	Play the 'door-closing' sound
				//
				WWAudioClass::Get_Instance()->Create_Instant_Sound( Get_DoorPhysDef()->CloseSoundDefID, Get_Transform() );

				//
				//	Close the door
				//
				AnimManager.Set_Animation_Mode( AnimCollisionManagerClass::ANIMATE_TARGET );
				AnimManager.Set_Target_Frame( 0 );
			}
			break;

			case STATE_OPENED_DOOR:
			{
				//
				//	Set a timer that we can use to close the door
				//
				Timer = Get_DoorPhysDef()->CloseDelay;

				//
				//	Make sure the door is open (net games, may not see the "opening" state)
				//
				// Clients *must* see the opening - it's a guaranteed packet. ST - 1/15/2002 12:59PM
				//
				AnimManager.Set_Animation_Mode( AnimCollisionManagerClass::ANIMATE_TARGET );
				AnimManager.Set_Target_Frame( AnimManager.Peek_Animation()->Get_Num_Frames() - 1	);
			}
			break;

			case STATE_CLOSED_DOOR:

				//
				//	Make sure the door is closed (in net games, we may not get
				// the "closing" state)
				//
				AnimManager.Set_Animation_Mode( AnimCollisionManagerClass::ANIMATE_TARGET );
				AnimManager.Set_Target_Frame( 0 );

				break;

			case STATE_ACCESS_DENIED:
			{
				//
				//	Play the 'access denied' sound
				//
				WWAudioClass::Get_Instance()->Create_Instant_Sound( Get_DoorPhysDef()->AccessDeniedSoundDefID, Get_Transform() );
			}
			break;
		}

		State = new_state;
		retval = true;
	}

	return retval;
}


enum
{
	DOOR_OPEN_OK			= 0,
	DOOR_OPEN_LOCKED,
	DOOR_OPEN_NOONE
};


void	DoorPhysClass::Update_State( float dt )
{
	switch ( State )
	{
		case STATE_OPENING_DOOR:
		{
			//
			//	Has the door finished opening?
			//
			if (AnimManager.Get_Current_Frame() == AnimManager.Peek_Animation()->Get_Num_Frames() - 1) {
				Set_State( STATE_OPENED_DOOR );
			}
		}
		break;

		case STATE_OPENED_DOOR:
		{
			//
			//	Has the door been in 'opened' state long enough?
			//
			// Client shouldn't be able to close his own door unless he hears from the server. ST - 1/15/2002 12:58PM
			//
			if ( CombatManager::I_Am_Server() ) {
				Timer -= dt;
				if (	Timer < 0 &&
						LockState == false &&
						OpenRequestPending == false &&
						Can_Open_Door() != DOOR_OPEN_OK)
				{
					//
					//	Switch to closing state
					//
					Set_State( STATE_CLOSING_DOOR );
				}
			}
		}
		break;

		case STATE_CLOSING_DOOR:
		{
			//
			//	Has the door finished closing?
			//
			if ( AnimManager.Get_Current_Frame() == 0 ) {
				Set_State( STATE_CLOSED_DOOR );
			}
		}

		case STATE_CLOSED_DOOR:
		case STATE_ACCESS_DENIED:
		{
			//
			//	We only do this if we are the server
			//
			if ( CombatManager::I_Am_Server() ) {

				//
				//	Should we check to see if someone wants to open the door?
				//
				CheckTimer -= dt;
				if ( CheckTimer <= 0 ) {
					CheckTimer = 0.3f;		// Check every 1/3 second

					//
					//	Check to see if we should open the door
					//
					if ( LockState == false ) {
						bool update_clients	= false;
						int result				= Can_Open_Door();
						if ( OpenRequestPending || result == DOOR_OPEN_OK ) {
							update_clients	= Set_State( STATE_OPENING_DOOR );
						} else if ( result == DOOR_OPEN_LOCKED ) {
							update_clients	= Set_State( STATE_ACCESS_DENIED );
						} else if ( State == STATE_ACCESS_DENIED ) {
							update_clients	= Set_State( STATE_CLOSED_DOOR );
						}

						//
						//	If the server has decided on a new state, then
						// notify the clients
						//
						if ( update_clients ) {
							Enable_Is_State_Dirty( true );
						}
					}
				}
			}
		}
		break;
	}

	return ;
}


void DoorPhysClass::Lock_Door_Open( bool onoff )
{
	if ( CombatManager::I_Am_Server() ) {

		//
		//	Force the door open...
		//
		if ( onoff && Set_State( STATE_OPENING_DOOR ) ) {
			Enable_Is_State_Dirty( true );
		}

		//
		//	Lock the state
		//
		LockState = onoff;
	}

	return ;
}


int DoorPhysClass::Can_Open_Door( void )
{
	//
	//	Transform both trigger zones into world space
	//
	OBBoxClass trigger_zone1;
	OBBoxClass trigger_zone2;
	OBBoxClass::Transform( Get_Transform(), Get_DoorPhysDef()->TriggerZone1, &trigger_zone1 );
	OBBoxClass::Transform( Get_Transform(), Get_DoorPhysDef()->TriggerZone2, &trigger_zone2 );

	//
	//	Check the zone's to see if we should open the door
	//
	int result1 = Check_Door_Trigger( trigger_zone1 );
	int result2 = DOOR_OPEN_NOONE;
	if (result1 != DOOR_OPEN_OK) {
		result2 = Check_Door_Trigger( trigger_zone2 );
	}

	//
	//	Now determine what information to return about the trigger check
	//
	int result = DOOR_OPEN_NOONE;
	if ( result1 == DOOR_OPEN_OK || result2 == DOOR_OPEN_OK ) {
		result = DOOR_OPEN_OK;
	} else if ( result1 == DOOR_OPEN_LOCKED || result2 == DOOR_OPEN_LOCKED ) {
		result = DOOR_OPEN_LOCKED;
	}

	return result;
}

int DoorPhysClass::Check_Door_Trigger( const OBBoxClass &trigger_zone )
{
	int result = DOOR_OPEN_NOONE;


#if 0
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
			if ( CollisionMath::Overlap_Test( trigger_zone, pos ) == CollisionMath::INSIDE ) {

				//
				//	Is the door locked?
				//
				if ( Get_DoorPhysDef()->LockCode != 0 ) {

					if ( soldier == COMBAT_STAR ) {
						Vector3 pos;
						soldier->Get_Position( &pos );
						const char * desc = soldier->Has_Key( Get_DoorPhysDef()->LockCode ) ? "KEUS" : "KEDE";
						DIAG_LOG(( desc, "%1.2f; %1.2f; %1.2f; %d; %d", pos.X, pos.Y, pos.Z, Get_ID(), Get_DoorPhysDef()->LockCode ));
					}

					//
					//	Can the soldier unlock this door?
					//
					if ( soldier->Has_Key( Get_DoorPhysDef()->LockCode ) ) {
						result = DOOR_OPEN_OK;
						break;
					} else {
						result = DOOR_OPEN_LOCKED;
					}
				} else {
					result = DOOR_OPEN_OK;
					break;
				}
			}
		}
	}

#else

	NonRefPhysListClass obj_list;
	WWASSERT(PhysicsSceneClass::Get_Instance() != NULL);
	PhysicsSceneClass::Get_Instance()->Collect_Objects(trigger_zone,false,true,&obj_list);
	NonRefPhysListIterator it(&obj_list);

	while (!it.Is_Done() && result == DOOR_OPEN_NOONE) {

		CombatPhysObserverClass * observer = (CombatPhysObserverClass *)it.Peek_Obj()->Get_Observer();
		//WWASSERT(observer != NULL);
		//if (observer->As_PhysicalGameObj()) {
		if (observer != NULL && observer->As_PhysicalGameObj()) {
			SoldierGameObj * soldier = observer->As_PhysicalGameObj()->As_SoldierGameObj();
			VehicleGameObj * vehicle = observer->As_PhysicalGameObj()->As_VehicleGameObj();

			//
			// Check if this is a soldier who can open the door
			//
			if ((soldier != NULL) && soldier->Is_Human_Controlled()) {

				WWASSERT(Get_DoorPhysDef() != NULL);
				if ( Get_DoorPhysDef()->LockCode == 0 ) {

					result = DOOR_OPEN_OK;

				} else {

					if (Can_Unlock_Me(soldier)) {
						result = DOOR_OPEN_OK;
						break;
					} else {
						result = DOOR_OPEN_LOCKED;
					}

				}
			}

			//
			// Check if this is a vehicle that is allowed to open the door
			//
			WWASSERT(Get_DoorPhysDef() != NULL);
			if ((vehicle != NULL) && (Get_DoorPhysDef()->DoorOpensForVehicles)) {

				if ( Get_DoorPhysDef()->LockCode == 0 ) {

					result = DOOR_OPEN_OK;

				} else {

					SoldierGameObj * driver = vehicle->Get_Driver();
					if ((driver != NULL) && (Can_Unlock_Me(driver))) {
						result = DOOR_OPEN_OK;
						break;
					} else {
						result = DOOR_OPEN_LOCKED;
					}
				}
			}

		}

		it.Next();
	}

#endif

	return result;
}

bool DoorPhysClass::Can_Unlock_Me(SoldierGameObj * soldier) const
{
	if ( soldier == COMBAT_STAR ) {
		Vector3 pos;
		soldier->Get_Position( &pos );
		const char * desc = soldier->Has_Key( Get_DoorPhysDef()->LockCode ) ? "KEUS" : "KEDE";
		DIAG_LOG(( desc, "%1.2f; %1.2f; %1.2f; %d; %d", pos.X, pos.Y, pos.Z, Get_ID(), Get_DoorPhysDef()->LockCode ));
	}

	//
	//	Can the soldier unlock this door?
	//
	return (Get_DoorPhysDef()->LockCode == 0) || (soldier->Has_Key( Get_DoorPhysDef()->LockCode ));
}


void DoorPhysClass::Set_Precision(void)
{
	cEncoderList::Set_Precision( BITPACK_DOOR_STATE, 0, (int)STATE_MAX );
	return ;
}


bool DoorPhysClass::Is_Door_Open( void ) const
{
	return (State == STATE_OPENED_DOOR);
}
