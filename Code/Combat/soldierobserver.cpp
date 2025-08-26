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
 *                     $Archive:: /Commando/Code/Combat/soldierobserver.cpp                   $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 12/17/01 3:06p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 112                                                         $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "soldierobserver.h"
#include "soldier.h"
#include "action.h"
#include "debug.h"
#include "crandom.h"
#include "cover.h"
#include "WWAudio.h"
#include "persistfactory.h"
#include "combatchunkid.h"
#include "combat.h"
#include "gameobjobserver.h"
#include "gameobjmanager.h"
#include "weapons.h"
#include "conversationmgr.h"
#include "Pathfind.h"
#include "timemgr.h"
#include "weaponbag.h"
#include "wwprofile.h"
#include "playertype.h"


#define	THINK_RATE 1
#define	THINK_ID 100123

#define	INNATE_ACTION_ID	9000000

/*
**
*/
const char * SoldierAIStateNames[SoldierObserverClass::NUM_SOLDIER_AI_STATES] = 
{
	"Relaxed Idle",		//SOLDIER_AI_RELAXED_IDLE = 0,
	"Alert Idle",			//SOLDIER_AI_ALERT_IDLE,	    
	"Footsteps Heard",	//SOLDIER_AI_FOOTSTEPS_HEARD, 
	"Bullet Heard",		//SOLDIER_AI_BULLET_HEARD,    
	"Gunshot Heard",		//SOLDIER_AI_GUNSHOT_HEARD,   
	"Enemy Seen"			//SOLDIER_AI_ENEMY_SEEN,      
};

int	StatePriorities[ SoldierObserverClass::NUM_SOLDIER_AI_STATES ] = 
{
	10,	//	SOLDIER_AI_RELAXED_IDLE = 0,	
	20,	//	SOLDIER_AI_ALERTED_IDLE,	
	50,	// SOLDIER_AI_FOOTSTEPS_HEARD,
	50,	// SOLDIER_AI_BULLET_HEARD,
	70,	//	SOLDIER_AI_GUNSHOT_HEARD,
	90,	// SOLDIER_AI_ENEMY_SEEN,
};
 

typedef struct {
	float	StateDuration;		// How long we should stay in this state
	int	NextState;			// State to enter when we complete this state
} SoldierAIStateData;

SoldierAIStateData	StateData[SoldierObserverClass::NUM_SOLDIER_AI_STATES] = {
	{9999,	SoldierObserverClass::SOLDIER_AI_RELAXED_IDLE},
	{180,		SoldierObserverClass::SOLDIER_AI_RELAXED_IDLE},
	{6,		SoldierObserverClass::SOLDIER_AI_CONDITIONAL_IDLE},
	{20,		SoldierObserverClass::SOLDIER_AI_ALERT_IDLE},
	{60,		SoldierObserverClass::SOLDIER_AI_ALERT_IDLE},	//	{60,		SoldierObserverClass::SOLDIER_AI_BULLET_HEARD},
	{60,		SoldierObserverClass::SOLDIER_AI_BULLET_HEARD},
};

/*
** Speech
*/
const char* _SpeakEnemySeen[] = 
{
	"ThereHeIs02",
	"KillHim02",
	"GetHim02",
	"ForKane02",
};
const int	_SpeakEnemySeenCount = (sizeof(_SpeakEnemySeen) / sizeof(_SpeakEnemySeen[0])) ;

const char* Pick_Speak_EnemySeen(void)
{
	int index = FreeRandom.Get_Int(_SpeakEnemySeenCount);
	return _SpeakEnemySeen[index];
}

Vector3	Random_Vector(float size) 
{
	Vector3 vect(	FreeRandom.Get_Float( -size, size ),
						FreeRandom.Get_Float( -size, size ), 0 );
	return vect;
}

/*
**
*/
SoldierObserverClass::SoldierObserverClass( void ) : 
	State( SOLDIER_AI_RELAXED_IDLE ),
	StateTimer( 0 ),
	HomeRadius( 9999999 ),
	ActionTimer( 0 ),
	CoverPosition( NULL ),
	CoveredAttack( false ),
	LastEvent( SOLDIER_AI_RELAXED_IDLE ),
	ConversationTimer( WWMath::Random_Float( 1.0F, 10.0F ) ),
	IsAlerted( false ),
	Aggressiveness( 0.5f ),
	TakeCoverProbability( 0.5f ),
	IsStationary( false ),
	LastWeaponIndex( 0 )
{
}


SoldierObserverClass::~SoldierObserverClass( void )
{
	Release_Cover_Position();
}

/*
**
*/
void	SoldierObserverClass::Release_Cover_Position( void )
{
	if ( CoverPosition != NULL ) {
		CoverManager::Release_Cover( CoverPosition );
		CoverPosition = NULL;
	}
}

/*
**
*/
SimplePersistFactoryClass<SoldierObserverClass, CHUNKID_SOLDIER_OBSERVER>	_SoldierObserverPersistFactory;

const PersistFactoryClass &	SoldierObserverClass::Get_Factory (void) const
{
	return _SoldierObserverPersistFactory;
}


enum	{
	CHUNKID_PARENT							=	 410001836,
	CHUNKID_VARIABLES,
	CHUNKID_ENEMY_OBJ_REF,

	MICROCHUNKID_STATE							=	1,
	MICROCHUNKID_STATE_TIMER,
	MICROCHUNKID_HOME_LOCATION,
	MICROCHUNKID_ALERT_POSITION,
	MICROCHUNKID_ACTION_TIMER,
	XXXMICROCHUNKID_ANIMATING,
	XXXMICROCHUNKID_MOVING,
	MICROCHUNKID_COVER_POSITION,
	MICROCHUNKID_COVERED_ATTACK,
	MICROCHUNKID_HOME_RADIUS,
	XXXMICROCHUNKID_ESCORT_OFFSET,
	MICROCHUNKID_CONVERSATION_TIMER,
	MICROCHUNKID_IS_ALERTED,
	MICROCHUNKID_AGGRESSIVENESS,
	MICROCHUNKID_TAKE_COVER_PROBABILITY,
	MICROCHUNKID_IS_STATIONARY,
	MICROCHUNKID_LAST_WEAPON_INDEX,
};

bool	SoldierObserverClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk( CHUNKID_PARENT );
		PersistentGameObjObserverClass::Save( csave );
	csave.End_Chunk();

	if ( CombatManager::Are_Observers_Active() ) {
		csave.Begin_Chunk( CHUNKID_VARIABLES );
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STATE, State );
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STATE_TIMER, StateTimer );
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_HOME_LOCATION, HomeLocation );
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_HOME_RADIUS, HomeRadius );
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ALERT_POSITION, AlertPosition );
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ACTION_TIMER, ActionTimer );
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_COVER_POSITION, CoverPosition );
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_COVERED_ATTACK, CoveredAttack );
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_CONVERSATION_TIMER, ConversationTimer );
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_IS_ALERTED, IsAlerted );
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_AGGRESSIVENESS, Aggressiveness );
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TAKE_COVER_PROBABILITY, TakeCoverProbability );
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_IS_STATIONARY, IsStationary );
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_LAST_WEAPON_INDEX, LastWeaponIndex );
		csave.End_Chunk();

		if ( EnemyObject != NULL ) {
			csave.Begin_Chunk( CHUNKID_ENEMY_OBJ_REF );
			EnemyObject.Save( csave );
			csave.End_Chunk();
		}
	}

	// Don't need to save SubStateString, or LastEvent

	return true;
}


bool	SoldierObserverClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PARENT:
				PersistentGameObjObserverClass::Load( cload );
				break;
								
			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STATE, State );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STATE_TIMER, StateTimer );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_HOME_LOCATION, HomeLocation );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_HOME_RADIUS, HomeRadius );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ALERT_POSITION, AlertPosition );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ACTION_TIMER, ActionTimer );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_COVER_POSITION, CoverPosition );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_COVERED_ATTACK, CoveredAttack );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_CONVERSATION_TIMER, ConversationTimer );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_IS_ALERTED, IsAlerted );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_AGGRESSIVENESS, Aggressiveness );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TAKE_COVER_PROBABILITY, TakeCoverProbability );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_IS_STATIONARY, IsStationary );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_LAST_WEAPON_INDEX, LastWeaponIndex );

						default:
							Debug_Say(("Unhandled Variable Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			case CHUNKID_ENEMY_OBJ_REF:
				EnemyObject.Load( cload );
				break;

			default:
				Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}
		cload.Close_Chunk();
	}

	if ( CoverPosition ) {
		REQUEST_POINTER_REMAP( (void **)&CoverPosition );
	}

	return true;
}


/*
**
*/
void	SoldierObserverClass::Attach( GameObject * obj )
{
	// Warning, Attach may not be called on loaded scripts
}


void	SoldierObserverClass::Detach( GameObject * obj )
{
	//
	//	Clear the soldier's internal innate observer pointer
	//
	SmartGameObj *smart_game_obj = obj->As_SmartGameObj();
	if ( smart_game_obj != NULL ) {
		SoldierGameObj *soldier = smart_game_obj->As_SoldierGameObj();
		if ( soldier != NULL ) {
			if ( soldier->Get_Innate_Observer () == this ) {
				soldier->Clear_Innate_Observer();
			}
		}
	}

	GameObjObserverManager::Delete_Register( this );
}


void SoldierObserverClass::Created( GameObject* obj )
{
	SmartGameObj* smart = obj->As_SmartGameObj();

	WWASSERT( smart != NULL );

//	Debug_Say(("Innate soldier [%d] created\n", obj->Get_ID()));

	smart->Get_Position( &HomeLocation );

	// Enable enemy sensory
	smart->Set_Enemy_Seen_Enabled(true);

	obj->Start_Observer_Timer( Get_ID(), THINK_RATE, THINK_ID );

	Aggressiveness = smart->As_SoldierGameObj()->Get_Definition().InnateAggressiveness;

	// Modify aggressiveness based on difficulty
	switch ( CombatManager::Get_Difficulty_Level() ) {
		case 0:	
			if ( Aggressiveness > 0.25f ) {
				Aggressiveness -= 0.25f;	
			}
			break;
		case 1:	
			// no change
			break;
		case 2:	
			Aggressiveness += 0.25f;
			if ( Aggressiveness > 1 ) {
				Aggressiveness = 1;
			}
			break;
	};

	TakeCoverProbability = smart->As_SoldierGameObj()->Get_Definition().InnateTakeCoverProbability;
	IsStationary = smart->As_SoldierGameObj()->Get_Definition().InnateIsStationary;

}


void SoldierObserverClass::Destroyed(GameObject* obj)
{
//	Debug_Say(("Innate soldier [%d] destroyed\n", obj->Get_ID()));

	Release_Cover_Position();
}


void SoldierObserverClass::Timer_Expired(GameObject* obj, int timer_id)
{
	SmartGameObj* smart = obj->As_SmartGameObj();
	WWASSERT( smart != NULL );
	SoldierGameObj* soldier = smart->As_SoldierGameObj();
	WWASSERT(soldier != NULL);

	if (timer_id == THINK_ID) {
		obj->Start_Observer_Timer( Get_ID(), THINK_RATE, THINK_ID );
		Think(soldier);
	}
}


void SoldierObserverClass::Killed(GameObject* obj, GameObject* killer)
{
//	Debug_Say(("Innate soldier [%d] killed by [%d]\n", obj->Get_ID(), killer ? killer->Get_ID() : -1));
}


void SoldierObserverClass::Damaged(GameObject* obj, GameObject* damager, float amount)
{
//	Debug_Say(("Innate soldier [%d] damaged by [%d]\n", obj->Get_ID(), damager ? damager->Get_ID() : -1));
	if ( obj->Is_Hibernating() ) {
		return;
	}

	SmartGameObj* smart = obj->As_SmartGameObj();
	WWASSERT( smart != NULL );
	SoldierGameObj* soldier = smart->As_SoldierGameObj();
	WWASSERT(soldier != NULL);

	if (soldier->Is_Innate_Enabled(SOLDIER_INNATE_EVENT_BULLET_HEARD)) {
//Debug_Say(( "Damaged\n" ));
		Vector3 pos;
		soldier->Get_Position( &pos );

		if ( damager != NULL ) {
			if ( FreeRandom.Get_Float() < 0.8 ) {		// 80% chance of knowing the source
				damager->Get_Position( &pos );
			}
		}

		Set_State(soldier, SOLDIER_AI_BULLET_HEARD, pos);
	}

	// If I am attacking an object, and I am damaged by a closer object, switch targets
	if ( State == SOLDIER_AI_ENEMY_SEEN ) {
		GameObject * enemy = EnemyObject;
		if ( damager != NULL && enemy != damager ) {

			Vector3 my_pos;
			soldier->Get_Position( &my_pos );
			Vector3 pos;
			float target_range = 1000000;
			if ( enemy ) {
				enemy->Get_Position( &pos );
				pos -= my_pos;
				target_range = pos.Length();
			}

			damager->Get_Position( &pos );
			pos -= my_pos;
			if ( pos.Length() < target_range ) {
				PhysicalGameObj * pdamager = damager->As_PhysicalGameObj();
				if ( pdamager && pdamager->Is_Enemy( soldier ) ) {
					Debug_Say(( "Switching to nearer target\n" ));
					EnemyObject = damager;
					ActionTimer = 0;  // Act on it now
				}
			}
		}
	}
}


void SoldierObserverClass::Sound_Heard(GameObject* obj, const CombatSound& sound)
{
	if ( obj->Is_Hibernating() ) {
		return;
	}

	SmartGameObj* smart = obj->As_SmartGameObj();
	WWASSERT( smart != NULL );
	SoldierGameObj* soldier = smart->As_SoldierGameObj();
	WWASSERT(soldier != NULL);

	PhysicalGameObj * creator = NULL;
	if ( sound.Creator != NULL ) {
		creator = sound.Creator->As_PhysicalGameObj();
	}

	bool state_changed = false;

	switch (sound.Type) {
		case SOUND_TYPE_BULLET_HIT: 
		{
			if (soldier->Is_Innate_Enabled(SOLDIER_INNATE_EVENT_BULLET_HEARD)) {
				Vector3 pos;
				pos = sound.Position;

				if ( creator && FreeRandom.Get_Float() < 0.7 ) {		// 70% chance of knowing the source
					creator->Get_Position( &pos );
				}

				state_changed = Set_State(soldier, SOLDIER_AI_BULLET_HEARD, pos);
			}
		}
		break;

		case SOUND_TYPE_GUNSHOT:
			// Don't hear friendly
			if ( (creator == NULL) || !soldier->Is_Teammate( creator ) ) {
				if (soldier->Is_Innate_Enabled(SOLDIER_INNATE_EVENT_GUNSHOT_HEARD)) {
					state_changed = Set_State(soldier, SOLDIER_AI_GUNSHOT_HEARD, sound.Position);
				}
			}
		break;

		case SOUND_TYPE_FOOTSTEPS:
			if ( (creator == NULL) || !soldier->Is_Teammate( creator ) ) {
				if (soldier->Is_Innate_Enabled(SOLDIER_INNATE_EVENT_FOOTSTEP_HEARD)) {
//Debug_Say(( "Heard Footsteps\n" ));
					state_changed = Set_State(soldier, SOLDIER_AI_FOOTSTEPS_HEARD, sound.Position);
				}
			}
		break;

		default:
//			Debug_Say(("Innate soldier [%d] unrecognized sound!\n", obj->Get_ID()));

		break;
	}

	if ( state_changed ) {
		Notify_Neighbors_Sound( soldier, sound );
	}
}


void SoldierObserverClass::Enemy_Seen(GameObject* obj, GameObject* enemy)
{
	if ( obj->Is_Hibernating() ) {
		return;
	}

//	WWASSERT( obj != enemy );

	// only see the enemy if it has health
	WWASSERT( enemy );
	PhysicalGameObj * p_enemy = enemy->As_PhysicalGameObj();
	if ( p_enemy == NULL || p_enemy->Get_Defense_Object()->Get_Health() <= 0 ) {
//		Debug_Say(( "I see dead people\n" ));
		return;
	}

	GameObject * curr_enemy = EnemyObject;
	if ( curr_enemy != NULL && curr_enemy != enemy ) {
		// take the closer enemy
		Vector3	old_pos;
		curr_enemy->Get_Position( &old_pos );
		Vector3	new_pos;
		enemy->Get_Position( &new_pos );
		Vector3	my_pos;
		obj->Get_Position( &my_pos );
		old_pos -= my_pos;
		new_pos -= my_pos;
		if ( new_pos.Length2() > old_pos.Length2() ) {
//			Debug_Say(( "New guy is too far away!\n" ));
			return;
		}

//		Debug_Say(( "Found New guy!!\n" ));
	}

	SmartGameObj* smart = obj->As_SmartGameObj();
	WWASSERT( smart != NULL );
	SoldierGameObj* soldier = smart->As_SoldierGameObj();
	WWASSERT(soldier != NULL);

//	Debug_Say(("Innate soldier [%d] seen enemy [%d]\n", obj->Get_ID(), enemy->Get_ID()));

	bool	state_changed = false;

	if (soldier->Is_Innate_Enabled(SOLDIER_INNATE_EVENT_ENEMY_SEEN)) {
//Debug_Say(( "Seen Enemy\n" ));
		state_changed = Set_State(soldier, SOLDIER_AI_ENEMY_SEEN, Vector3(0,0,0), enemy);
	}

	if ( state_changed ) {
		Notify_Neighbors_Enemy( soldier, enemy );
	}
}


void	SoldierObserverClass::Custom( GameObject * obj, int type, int param, GameObject * sender )
{
	if ( type == CUSTOM_EVENT_ATTACK_ARRIVED ) {
//		Debug_Say(( "Attack Arrvied event %d\n", param ));
	}
}

void SoldierObserverClass::Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason )
{
	if ( action_id == INNATE_ACTION_ID ) {
		SubStateString += "\nComplete";
	//	ActionTimer = MIN( ActionTimer, 10 );		// When he completes, only stay in the action state for 5 more seconds

		// if I had an enemy target, who was just killed...
		PhysicalGameObj* enemy = (PhysicalGameObj*)EnemyObject.Get_Ptr();
		if ( enemy && State == SOLDIER_AI_ENEMY_SEEN ) {
			if ( enemy->Get_Defense_Object()->Get_Health() <= 0 ) {
	//			Debug_Say(( "Target dead, moving on\n" ));
				StateTimer = 100000;	// Leave current state
				EnemyObject = NULL;
			}
		}

		if (	complete_reason == ACTION_COMPLETE_NORMAL ||
				complete_reason == ACTION_COMPLETE_PATH_BAD_START ||
				complete_reason == ACTION_COMPLETE_PATH_BAD_DEST )
		{
			if ( complete_reason == ACTION_COMPLETE_PATH_BAD_DEST ) {
				// Pick a new action within 1 to 2 seconds
				ActionTimer = MIN( ActionTimer, WWMath::Random_Float( 1.0F, 2.0F ) ); 
			} else {
				// Pick a new action within 2 to 3 seconds
				ActionTimer = MIN( ActionTimer, WWMath::Random_Float( 2.0F, 3.0F ) ); 
			}

			if ( State > SOLDIER_AI_ALERT_IDLE ) {
				StateTimer = 100000;	// Leave current state
			}
		}
	}
}


/*
**
*/
void SoldierObserverClass::State_Changed( SoldierGameObj * soldier ) 
{
	switch ( State ) {
		case SOLDIER_AI_RELAXED_IDLE:
					IsAlerted = false;
					break;
		case SOLDIER_AI_FOOTSTEPS_HEARD:
					// Leave it was it was
					break;
		case SOLDIER_AI_ALERT_IDLE:
		case SOLDIER_AI_BULLET_HEARD:
		case SOLDIER_AI_GUNSHOT_HEARD:
		case SOLDIER_AI_ENEMY_SEEN:
					IsAlerted = true;
					break;
	}

	// Remeber the last selected weapon
	if ( soldier->Get_Weapon_Bag()->Get_Index() != 0 ) {
		LastWeaponIndex = soldier->Get_Weapon_Bag()->Get_Index();
	}

	if ( IsAlerted ) {
		soldier->Get_Weapon_Bag()->Select_Index( LastWeaponIndex );
	} else {
		soldier->Get_Weapon_Bag()->Select_Index( 0 );
	}

	if ( State == SOLDIER_AI_ALERT_IDLE ) {
		soldier->Get_Human_State()->Drop_Weapon();
	}

	ActionTimer = 0;
}

/*
**
*/
bool SoldierObserverClass::Set_State( SoldierGameObj * soldier, int state, const Vector3& location, GameObject* enemy) 
{
//	WWASSERT( soldier != enemy );

	LastEvent = state;

	int old_state = State;

	// If this is a more extreme state, act surprised!

	if (StatePriorities[State] < StatePriorities[state]) {
		// If we have seen the enemy then make a comment.
		if (state == SOLDIER_AI_ENEMY_SEEN) {
//			Speak(obj, Pick_Speak_EnemySeen());
		} else if (state != SOLDIER_AI_FOOTSTEPS_HEARD) {
			// Duck
//			Action_Play_Animation(obj, "s_a_human.h_a_j21c");
//			Speak(obj, "Q_Huh02");
		}

//		Debug_Say(("Switching to State %d from %d\n", state, State));
		AlertPosition = location;
		if ( State != state ) {
			State = state;
			State_Changed( soldier );
		}

//		Debug_Say(( "New Innate State!\n" ));
	}

	// If this is not a less extreme state, reset state timer

	if (StatePriorities[State] <= StatePriorities[state]) {

		if ( State != state ) {
			State = state;
			State_Changed( soldier );
		}

		StateTimer = 0;

		if ( ( state == SOLDIER_AI_BULLET_HEARD ) ||
			  ( state == SOLDIER_AI_GUNSHOT_HEARD ) ) {
			if ( ActionTimer > 1 ) {
				ActionTimer = 1;
			}
		}

//		ActionTimer = 0;  no

#if 01
		// Only take the new pos if closer
		Vector3 my_pos;
		soldier->Get_Position(&my_pos);
		Vector3 old_pos = AlertPosition - my_pos;
		Vector3 new_pos = location - my_pos;
		if ( old_pos.Length2() > new_pos.Length2() ) 
#endif
		{
			AlertPosition = location;
		}

		if ( enemy != NULL ) {
			EnemyObject = enemy->As_PhysicalGameObj();
		} else {
			EnemyObject = NULL;
		}
		Think(soldier, (State != old_state) );
	}	

	//
	//	Reset the conversation timer if the state has changed
	//
	if ( old_state != State ) {		
		if ( State == SOLDIER_AI_ENEMY_SEEN ) {
			ConversationTimer = WWMath::Random_Float( 5.0F, 30.0F );
		} else {
			ConversationTimer = WWMath::Random_Float( 20.0F, 60.0F );
		}		
	}

	//
	//	Decide which (if any) dialogue the soldier should say
	//
	if ( ( old_state == SOLDIER_AI_RELAXED_IDLE ) || ( old_state == SOLDIER_AI_ALERT_IDLE ) ) {
		
		if (	State == SOLDIER_AI_FOOTSTEPS_HEARD ||
				State == SOLDIER_AI_BULLET_HEARD ||
				State == SOLDIER_AI_GUNSHOT_HEARD)
		{
			soldier->Say_Dialogue( DIALOG_STATE_FROM_IDLE_TO_SEARCH );
		} else if ( State == SOLDIER_AI_ENEMY_SEEN ) {
			soldier->Say_Dialogue( DIALOG_STATE_FROM_IDLE_TO_COMBAT );
		}

	} else if ( old_state == SOLDIER_AI_ENEMY_SEEN ) {
		
		if (	State == SOLDIER_AI_FOOTSTEPS_HEARD ||
				State == SOLDIER_AI_BULLET_HEARD ||
				State == SOLDIER_AI_GUNSHOT_HEARD)
		{
			soldier->Say_Dialogue( DIALOG_STATE_FROM_COMBAT_TO_SEARCH );
		} else if ( ( old_state == SOLDIER_AI_RELAXED_IDLE ) || ( old_state == SOLDIER_AI_ALERT_IDLE ) ) {
			soldier->Say_Dialogue( DIALOG_STATE_FROM_COMBAT_TO_IDLE );
		}

	} else {

		if ( State == SOLDIER_AI_ENEMY_SEEN ) {
			soldier->Say_Dialogue( DIALOG_STATE_FROM_SEARCH_TO_COMBAT );
		} else if ( ( old_state == SOLDIER_AI_RELAXED_IDLE ) || ( old_state == SOLDIER_AI_ALERT_IDLE ) ) {
			soldier->Say_Dialogue( DIALOG_STATE_FROM_SEARCH_TO_IDLE );
		}		
	}

	return (old_state != State);
}

void SoldierObserverClass::Notify_Neighbors_Sound( SoldierGameObj * soldier, const CombatSound & sound )
{
	// Notify nearby neighbors of my knowledge
	// for all physicalgameobjs
	Vector3 my_pos;
	soldier->Get_Position( &my_pos );
	SLNode<SmartGameObj> *objnode;
	for (	objnode = GameObjManager::Get_Smart_Game_Obj_List()->Head(); objnode; objnode = objnode->Next()) {
		SmartGameObj *obj = objnode->Data()->As_SmartGameObj();
		if ( obj ) {
			if ( obj == soldier )	continue;
			if ( obj->Get_Player_Type() != soldier->Get_Player_Type() ) continue;

			Vector3 obj_pos;
			obj->Get_Position( &obj_pos );

			obj_pos -= my_pos;
			if ( obj_pos.Length() <= 5 ) {
				// Notify him of my info
				const GameObjObserverList & observer_list = obj->Get_Observers();
				for( int index = 0; index < observer_list.Count(); index++ ) {
					observer_list[ index ]->Sound_Heard( obj, sound );
				}		
			}
		}
	}
}

void SoldierObserverClass::Notify_Neighbors_Enemy( SoldierGameObj * soldier, GameObject * enemy )
{
	// Notify nearby neighbors of my knowledge
	// for all physicalgameobjs
	Vector3 my_pos;
	soldier->Get_Position( &my_pos );
	SLNode<SmartGameObj> *objnode;
	for (	objnode = GameObjManager::Get_Smart_Game_Obj_List()->Head(); objnode; objnode = objnode->Next()) {
		SmartGameObj *obj = objnode->Data()->As_SmartGameObj();
		if ( obj ) {
			if ( obj == soldier )	continue;
			if ( obj->Get_Player_Type() != soldier->Get_Player_Type() ) continue;

			Vector3 obj_pos;
			obj->Get_Position( &obj_pos );

			obj_pos -= my_pos;
			if ( obj_pos.Length() <= 5 ) {
				// Notify him of my info
				const GameObjObserverList & observer_list = obj->Get_Observers();
				for( int index = 0; index < observer_list.Count(); index++ ) {
					observer_list[ index ]->Enemy_Seen( obj, enemy );
				}		
			}
		}
	}
}



void SoldierObserverClass::Poked( GameObject * obj, GameObject * poker )
{
	if ( obj != NULL && obj->As_PhysicalGameObj() != NULL ) {
		PhysicalGameObj *physical_obj = obj->As_PhysicalGameObj();
		if ( physical_obj->As_SoldierGameObj() != NULL ) {

			bool face_poker = false;
			
			//
			//	Get the soldier to say one of its pre-canned remarks
			//
			SoldierGameObj *soldier = physical_obj->As_SoldierGameObj();
			float duration = 2.0F;
			switch ( State )
			{
				case SOLDIER_AI_RELAXED_IDLE:
				case SOLDIER_AI_ALERT_IDLE:
					soldier->Say_Dialogue( DIALOG_ON_POKE_IDLE );
					face_poker = true;
					break;

				case SOLDIER_AI_FOOTSTEPS_HEARD:
				case SOLDIER_AI_BULLET_HEARD:
				case SOLDIER_AI_GUNSHOT_HEARD:
					soldier->Say_Dialogue( DIALOG_ON_POKE_SEARCH );
					face_poker = true;
					break;

				case SOLDIER_AI_ENEMY_SEEN:
					soldier->Say_Dialogue( DIALOG_ON_POKE_COMBAT );
					break;
			}

			//
			//	Increment the duration by a small random amount
			//
			duration += WWMath::Random_Float( 0.5F, 2.0F );

			PhysicalGameObj *physical_poker_obj = poker->As_PhysicalGameObj();
			if ( physical_poker_obj != NULL ) {

				//
				//	Get the position of the player who poked us
				//
				Vector3 poker_pos = physical_poker_obj->Get_Bullseye_Position();

				//
				//	Make the soldier turn his body to face the speaker
				//
				if ( face_poker ) {
					ActionParamsStruct parameters;
					parameters.Priority		= 20;
					parameters.ObserverID	= Get_ID();
					parameters.Set_Face_Location( poker_pos, duration );
					soldier->Get_Action()->Face_Location( parameters );
				}
					
				//
				//	Turn the soldier's head to look at the speaker
				//
				soldier->Look_At( poker_pos, duration );
			}
		}
	}

	return ;
}

void SoldierObserverClass::Think( SoldierGameObj * soldier, bool is_new_state )
{
	WWPROFILE( "SoldierObserver Think" );
	WWASSERT( soldier != NULL );

	//
	//	We can't switch states if the soldier is "busy".  Busy usually
	// means he's going up an elevator, climbing a ladder, or opening a door.
	//
	if ( soldier->Get_Action()->Is_Busy() ) {
		return ;
	}


	// Check how long we have been in this state, and if we should switch
	StateTimer += 1;
	if (StateTimer >= StateData[State].StateDuration) {
//		Debug_Say(("Soldier relaxing from state %d to %d\n", State, StateData[State].NextState));
		
		Action_Reset( soldier );		// reset the old state
		State = StateData[State].NextState;

		if ( State == SOLDIER_AI_CONDITIONAL_IDLE ) {
			State = ( IsAlerted ) ? SOLDIER_AI_ALERT_IDLE : SOLDIER_AI_RELAXED_IDLE;
		}

		State_Changed( soldier );

		StateTimer = 0;
	}

	if ( ( State != SOLDIER_AI_RELAXED_IDLE ) && ( State != SOLDIER_AI_ALERT_IDLE ) ) {
		soldier->Reset_Loiter_Delay();
	}

	// Check if time for another state action
	if (soldier->Is_Innate_Enabled(SOLDIER_INNATE_ACTIONS)) {
		ActionTimer -= 1;
		if (ActionTimer <= 0) {
			State_Act(soldier, is_new_state);
		}
	}

	//
	//	Allow this unit to start a conversation regardless of the state he's in
	//
	if ( soldier->Is_Human_Controlled () == false && soldier->Is_In_Conversation () == false ) {

		//
		//	Is it time to start a new conversation?
		//
		ConversationTimer -= 1.0F;
		if (ConversationTimer <= 0) {
			
			//
			//	Don't start an innate conversation if there are already
			// conversations taking place
			//
#ifdef ENABLE_INNATE_CONVERSATIONS
			if ( ConversationMgrClass::Get_Active_Conversation_Count () == 0 ) {
				ConversationMgrClass::Start_Conversation( soldier );
			}
#endif //ENABLE_INNATE_CONVERSATIONS

			Reset_Conversation_Timer();
		}
	}

}

void	SoldierObserverClass::Get_Information( StringClass & string )
{
	StringClass temp;
	string += "-----------\n";
	temp.Format( "State: %s\n", SoldierAIStateNames[ State ] );
	string += temp;
	temp.Format( "Sub: %s\n", SubStateString );
	string += temp;
	if ( IsStationary ) {
		string += "Stationary\n";
	}
	if ( State == SOLDIER_AI_ENEMY_SEEN ) {
		temp.Format( "Attacking: %d\n", EnemyObject.Get_Ptr() ? EnemyObject.Get_Ptr()->Get_ID() : 0 );
		string += temp;
	}
	temp.Format( "Last: %s\n", SoldierAIStateNames[ LastEvent ] );
	string += temp;
	temp.Format( "Act Time: %f\n", ActionTimer );
	string += temp;
	if ( CoverPosition != NULL ) {
		temp.Format( "Has Cover Pos\n" );
		string += temp;
	}
}

void	SoldierObserverClass::Reset_Conversation_Timer( void )
{
	ConversationTimer = WWMath::Random_Float( 20.0F, 60.0F );
}


/*********************************************************************************************** 
**													Innate AI State Actions
***********************************************************************************************/
void SoldierObserverClass::State_Act( SoldierGameObj * soldier, bool is_new_state )
{
	WWASSERT( soldier != NULL );

	switch (State) {

		case SOLDIER_AI_RELAXED_IDLE:
			State_Act_Idle( soldier );
			break;

		case SOLDIER_AI_FOOTSTEPS_HEARD: 
			State_Act_Footsteps_Heard( soldier );
			break;

		case SOLDIER_AI_ALERT_IDLE: 
		case SOLDIER_AI_BULLET_HEARD:
			State_Act_Bullet_Heard( soldier, is_new_state );
			break;

		case SOLDIER_AI_GUNSHOT_HEARD:
			State_Act_Gunshot_Heard( soldier );
			break;

		case SOLDIER_AI_ENEMY_SEEN: 
			State_Act_Attack( soldier );
			break;
	}
}

#define	IDLE_WALK_DISTANCE		6
#define	IDLE_WALK_SPEED			0.3f	// was 0.2f

#define	ALERTED_RUN_DISTANCE		6	
#define	ALERTED_RUN_SPEED			0.9f
#define	ALERTED_RUN_RANGE			2

/*
**
*/
void SoldierObserverClass::State_Act_Idle( SoldierGameObj * soldier )
{
	ActionTimer = 5;	// Will probably be overridden

	Vector3	position;
	soldier->Get_Position(&position);
	Release_Cover_Position();

	if ( FreeRandom.Get_Float() < 0.5f ) {		// 50% chance of facing a random direction
		SubStateString = "Facing Random";
		position += Random_Vector(1);
		if ( COMBAT_STAR ) {
			SubStateString = "Facing Star";
			COMBAT_STAR->Get_Position( &position );
		}
		Action_Face_Location( soldier, position, AI_STATE_IDLE );
		ActionTimer = FreeRandom.Get_Float(4, 6);	//Delay 4 to 6 seconds
	} else if ( !IsStationary ) {					// 50% chance of walking to a random point
		SubStateString = "Random Walk";
		PathfindClass *pathfind = PathfindClass::Get_Instance();		//	Lookup a safe random position to walk to
		if ( pathfind->Find_Random_Spot( position, IDLE_WALK_DISTANCE, &position ) ) {
			Action_Goto_Location( soldier, position, AI_STATE_IDLE, IDLE_WALK_SPEED, 0.3f );
		}
		ActionTimer = FreeRandom.Get_Float(7, 10);	//Delay 7 to 10 seconds
	}

	Look_Random( soldier, ActionTimer );
}


/*
**
*/
void SoldierObserverClass::State_Act_Footsteps_Heard( SoldierGameObj * soldier )
{
	ActionTimer = FreeRandom.Get_Float(3, 4);	//Delay 7 to 10 seconds
	Release_Cover_Position();

	bool walking = false;
	if ( FreeRandom.Get_Float() < Aggressiveness && !IsStationary ) {
		SubStateString = "Walk To Footsteps";
		Vector3	position;
		PathfindClass *pathfind = PathfindClass::Get_Instance();
		if ( pathfind->Find_Random_Spot( AlertPosition, 2, &position ) ) {
			Action_Goto_Location( soldier, position, AI_STATE_IDLE, IDLE_WALK_SPEED, 0.5f );
			walking = true;
		}					
	}
	if ( !walking ) {
		SubStateString = "Face Footsteps";
		Action_Face_Location( soldier, AlertPosition, AI_STATE_SEARCH );
	}

	Look_Random( soldier, 0 );		// No random looking!
}


/*
**
*/
void SoldierObserverClass::State_Act_Bullet_Heard( SoldierGameObj * soldier, bool is_new_state )
{
	ActionTimer = FreeRandom.Get_Float(5,10);
	ActionTimer = FreeRandom.Get_Float(2,4);
	soldier->Get_Human_State()->Raise_Weapon();		// Keep the gun up

	Vector3	current_position;
	soldier->Get_Position(&current_position);

	Vector3	walk_position;

	#define	RANGE		10		// Dive with probability tied to bullet distance; 100% at 10m, 0 percent at 20m
	float range = (current_position - AlertPosition).Length();
	float percent = WWMath::Clamp( (1 - ((range-RANGE)/RANGE)), 0, 1 ); 
	bool dive = is_new_state && (FreeRandom.Get_Float() < percent) && !IsStationary;

	if ( dive ) {
		SubStateString = "Dive Away";					// Dive away
		ActionTimer = FreeRandom.Get_Float(2,4);
		Vector3 other_dir = 2*current_position - AlertPosition;
		Action_Dive( soldier, other_dir );
	} else {
		bool done = false;
		if ( !IsStationary ) {
			done = Take_Cover( soldier );
		}
		if ( !done ) {
			if ( FreeRandom.Get_Float() < TakeCoverProbability || IsStationary ) {
				SubStateString = "Face Random Crouched"; 	// Face and crouch
				walk_position = current_position + Random_Vector(1);
				if ( COMBAT_STAR ) {
					SubStateString = "Face Star Crouched";
					COMBAT_STAR->Get_Position( &walk_position );
				}

				Action_Face_Location( soldier, walk_position, AI_STATE_IDLE, true );
			} else {
				SubStateString = "Run To";
				//	Lookup a safe random position to run to
				PathfindClass *pathfind = PathfindClass::Get_Instance();
				if ( pathfind->Find_Random_Spot( AlertPosition, ALERTED_RUN_DISTANCE, &walk_position ) ) {
					Action_Goto_Location( soldier, walk_position, AI_STATE_SEARCH, ALERTED_RUN_SPEED, ALERTED_RUN_RANGE, true );
				}
			}
		}
	}

	Look_Random( soldier, ActionTimer );
}

/*
**
*/
void SoldierObserverClass::State_Act_Gunshot_Heard( SoldierGameObj * soldier )
{
	soldier->Get_Human_State()->Raise_Weapon();		// Keep the gun up

	Look_Random( soldier, 0 );

	ActionTimer = FreeRandom.Get_Float( 5, 10 );

	if ( !IsStationary ) {
		bool done = Take_Cover( soldier, true, AlertPosition );
		if ( !done ) {
			SubStateString = "Run To Random";
			PathfindClass *pathfind = PathfindClass::Get_Instance();
			
			//
			//	Lookup a safe random position to run to
			//
			Vector3 position = AlertPosition;			
			if (pathfind->Find_Random_Spot( AlertPosition, ALERTED_RUN_DISTANCE, &position )) {
				Action_Goto_Location_Facing(soldier, position, AI_STATE_SEARCH, AlertPosition, ALERTED_RUN_SPEED, ALERTED_RUN_RANGE);
			} else {
				
				//
				//	Wasn't possible to run to a random location near the alert position,
				// so simply move around your local area a little bit and look at the alert position
				//
				Vector3 curr_pos;
				soldier->Get_Position (&curr_pos);
				if (pathfind->Find_Random_Spot( curr_pos, IDLE_WALK_DISTANCE, &position )) {
					Action_Goto_Location_Facing( soldier, position, AI_STATE_SEARCH, AlertPosition, IDLE_WALK_SPEED, 0.5F );
				}
				//Release_Cover_Position();
				//SubStateString = "Face gunshot";
				//Action_Face_Location( soldier, AlertPosition, AI_STATE_SEARCH );
			}

		}
	} else {
		Release_Cover_Position();
		SubStateString = "Face gunshot";
		Action_Face_Location( soldier, AlertPosition, AI_STATE_SEARCH );
	}

}

/*
**
*/
void	SoldierObserverClass::State_Act_Attack( SoldierGameObj * soldier ) 
{
	WWASSERT( soldier != NULL );

	soldier->Get_Human_State()->Raise_Weapon();		// Keep the gun up

	ActionTimer = FreeRandom.Get_Float( 5, 8 );

	Look_Random( soldier, 0 );

#define	RUN_SPEED	0.9f
#define	WALK_SPEED	0.2f

	bool done = false;

	// OK, here's the attack plan....
	// Periodically (every 5 seconds?), pick a new attack action. 4 Attack options are:
	// If in a attack point, jump back to the cover
	// If in a cover spot, possibily jump out and attack (goto attack point)
	// If there is a new cover spot available, goto it
	// Pick a random point and goto it

	PhysicalGameObj* enemy = (PhysicalGameObj*)EnemyObject.Get_Ptr();
//	WWASSERT( soldier != enemy );

	if ( enemy == NULL ) {
		StateTimer = 100000;	// Leave this state
		return;
	}
	Vector3 enemy_pos;
	enemy->Get_Position(&enemy_pos);

	float weapon_range = 60.0f;
	if ( soldier->Get_Weapon() ) {
		weapon_range = soldier->Get_Weapon()->Get_Range();
	}

	Vector3 current_position;
	soldier->Get_Position(&current_position);

	if ( CoverPosition != NULL ) {
		if ( CoveredAttack == true ) {
			SubStateString = "Return to Cover";
			Vector3 cover_position = CoverPosition->Get_Transform().Get_Translation();
			Action_Attack_Object( soldier, enemy, weapon_range, CoverPosition->Get_Crouch(), cover_position, 0.5f );
			CoveredAttack = false;
			ActionTimer = 5;	// for 3 seconds
			done = true;
		} else if ( FreeRandom.Get_Float() < Aggressiveness ) {

			if ( soldier->Get_Weapon() != NULL ) {
				/*
				** Start a covered attack.
				** Pick an attack point, and go there, attacking at the target
				*/
				SubStateString = "Covered Attack";
				Vector3 attack_point = CoverPosition->Get_Attack_Position( enemy_pos );
				Action_Attack_Object( soldier, enemy, weapon_range, false, attack_point, 0.5f  );
				CoveredAttack = true;
				ActionTimer = 5;	// for 3 seconds
				done = true;
			}
		}
	}

	if ( CoverPosition != NULL ) {
		if ( !CoverManager::Is_Cover_Safe( CoverPosition, enemy_pos ) ) {
			Release_Cover_Position();
			CoveredAttack = false;
			done = false;
		}
	}

	float effective_range = 20;
	if ( soldier->Get_Weapon() ) {
		effective_range = soldier->Get_Weapon()->Get_Effective_Range();
	}

	if ( !done && ( FreeRandom.Get_Float() < TakeCoverProbability ) && !IsStationary ) {		// 25% chance of not finding cover
		Release_Cover_Position();		// Give us the option of selecting our current spot
		CoverEntryClass * cover = CoverManager::Request_Cover(current_position, enemy_pos, effective_range);
		if (cover != NULL) {					// Yes, take it
			SubStateString = "Take Cover";
			CoverPosition = cover;
			Vector3 cover_position = cover->Get_Transform().Get_Translation();
			Action_Attack_Object( soldier, enemy, weapon_range, CoverPosition->Get_Crouch(), cover_position, 0.5f );
			ActionTimer = 15;
			done = true;
		}
	}

	if ( FreeRandom.Get_Float() < Aggressiveness && !IsStationary ) {	// Charge Attack
		Vector3	pos;
		enemy->Get_Position( &pos );
		SubStateString = "Charge Attack";
		ActionTimer = 10;
		bool kneel = ( FreeRandom.Get_Float() < (1-Aggressiveness) );
		Action_Attack_Object( soldier, enemy, weapon_range, kneel, pos, 5 );
	 	done = true;
	}

	if ( !done ) {
		Vector3	best_pos = current_position;
		float best_distance = 100000;

		for (int i = 0; i < 3; i++) {
			//	Lookup a safe random position to run to
			Vector3 pos = current_position;
			PathfindClass *pathfind = PathfindClass::Get_Instance();
			if ( pathfind->Find_Random_Spot( current_position, effective_range*2, &pos ) ) {

				float distance = (pos - enemy_pos).Length();

				if (WWMath::Fabs(distance - effective_range) < best_distance) {
					best_distance = WWMath::Fabs(distance - effective_range);
					best_pos = pos;
				}
			}
		}

		if ( IsStationary ) {
			best_pos = current_position;
		}

		SubStateString = "Random Run Attack";
		ActionTimer = 10;
		bool kneel = ( FreeRandom.Get_Float() < (1-Aggressiveness) );
		Action_Attack_Object( soldier, enemy, weapon_range, kneel, best_pos, 0.5f );
	}
}


/*********************************************************************************************** 
**													Innate AI Actions
***********************************************************************************************/
void SoldierObserverClass::Action_Reset( SoldierGameObj * soldier )
{
	WWASSERT( soldier != NULL );
	soldier->Get_Action()->Reset( StatePriorities[ State ] );
}

void SoldierObserverClass::Action_Face_Location( SoldierGameObj * soldier, const Vector3 & location, SoldierAIState ai_state, bool crouched )
{
	WWASSERT( soldier != NULL );
	ActionParamsStruct parameters;
	parameters.Set_Basic( Get_ID(), StatePriorities[ State ], INNATE_ACTION_ID, ai_state );
	parameters.Set_Face_Location( location, 2 );
	parameters.MoveCrouched = crouched;
   soldier->Get_Action()->Face_Location( parameters );
}

void SoldierObserverClass::Action_Goto_Location( SoldierGameObj * soldier, const Vector3 & location, SoldierAIState ai_state, float speed, float distance, bool crouched )
{
	WWASSERT( soldier != NULL );
	Vector3	move_position = location;
	Stay_Within_Home( soldier, &move_position, &distance );
	ActionParamsStruct parameters;
	parameters.Set_Basic( Get_ID(), StatePriorities[ State ], INNATE_ACTION_ID, ai_state );
	parameters.Set_Movement( move_position, speed, distance, crouched );
   soldier->Get_Action()->Goto( parameters );
}

void SoldierObserverClass::Action_Goto_Location_Facing( SoldierGameObj * soldier, const Vector3 & location, SoldierAIState ai_state, const Vector3 & facing_pos, float speed, float distance, bool crouched )
{
	WWASSERT( soldier != NULL );
	Vector3	move_position = location;
	Stay_Within_Home( soldier, &move_position, &distance );
	ActionParamsStruct parameters;
	parameters.Set_Basic( Get_ID(), StatePriorities[ State ], INNATE_ACTION_ID, ai_state );
	parameters.Set_Movement( move_position, speed, distance, crouched );
	parameters.ForceFacing = true;
	parameters.FaceLocation = facing_pos;
   soldier->Get_Action()->Goto( parameters );
}

void SoldierObserverClass::Action_Attack_Object( SoldierGameObj * soldier, PhysicalGameObj * enemy, float range, 
																	bool kneel, const Vector3 & move_location, float arrived_distance )
{
	WWASSERT( soldier != NULL );
	Vector3	move_position = move_location;
	Stay_Within_Home( soldier, &move_position, &arrived_distance );

	if ( IsStationary ) {					// Don't move if stationary
		arrived_distance = 10000;
	}

	if ( enemy != NULL && soldier->Get_Weapon() != NULL ) {
		ActionParamsStruct parameters;
		parameters.Set_Basic( Get_ID(), StatePriorities[ State ], INNATE_ACTION_ID, AI_STATE_COMBAT );
		parameters.Set_Movement( move_position, RUN_SPEED, arrived_distance, kneel );
		parameters.Set_Attack( enemy, range, FreeRandom.Get_Float(2), true );
		parameters.AttackCrouched = kneel;
		parameters.AttackWanderAllowed = !IsStationary;

		// 50% chance of looking where you are running, for a bit
		if ( FreeRandom.Get_Float() < 0.5 ) {
			parameters.LookLocation = move_position + Vector3( 0,0,1 );
			parameters.LookDuration = FreeRandom.Get_Float( 0.3f, 1 );
		}

		soldier->Get_Action()->Attack( parameters );
	}
}

void SoldierObserverClass::Action_Dive( SoldierGameObj * soldier, const Vector3 & location )
{
	WWASSERT( soldier != NULL );
	ActionParamsStruct parameters;
	parameters.Set_Basic( Get_ID(), StatePriorities[ State ], INNATE_ACTION_ID, AI_STATE_SEARCH );
	parameters.MoveLocation = location;
	soldier->Get_Action()->Dive( parameters );
}

void SoldierObserverClass::Stay_Within_Home( SoldierGameObj * soldier, Vector3 * location, float * distance )
{
	// Whenever we move, if we are outside our home radius, override the move to move back inside
	Vector3 home_diff;
	soldier->Get_Position( &home_diff );
	home_diff -= HomeLocation;
	home_diff.Z = 0;	// don't consider Z
	if ( home_diff.Length() > HomeRadius ) {
		*location = HomeLocation;
		*distance = HomeRadius;
	}

}

bool SoldierObserverClass::Take_Cover( SoldierGameObj * soldier, bool force_face, const Vector3 & face_pos )
{
	if ( CoverPosition != NULL ) {
		if ( !CoverManager::Is_Cover_Safe( CoverPosition, AlertPosition ) ) {
			Release_Cover_Position();
		}
	}

	// Possibily take a cover spot
	if ( FreeRandom.Get_Float() < TakeCoverProbability ) { 
		Release_Cover_Position();		// Give us the option of selecting our current spot
		float cover_range = 20;
		Vector3 current_position;
		soldier->Get_Position(&current_position);
		CoverEntryClass * cover = CoverManager::Request_Cover(current_position, AlertPosition, cover_range);	// Can we find a cover spot?
		if (cover != NULL) {					// Yes, take it
			SubStateString = "Take Cover";
			CoverPosition = cover;
			Vector3 cover_position = cover->Get_Transform().Get_Translation();

			if ( force_face ) {
				Action_Goto_Location_Facing(soldier, cover_position, AI_STATE_SEARCH, face_pos, ALERTED_RUN_SPEED, 0.5, cover->Get_Crouch() );
			} else {
				Action_Goto_Location(soldier, cover_position, AI_STATE_SEARCH, ALERTED_RUN_SPEED, 0.5, cover->Get_Crouch() );
			}

			return true;
		}
	}
	return false;
}

void	SoldierObserverClass::Look_Random( SoldierGameObj * soldier, float duration ) 
{
	soldier->Look_Random( duration );
}
