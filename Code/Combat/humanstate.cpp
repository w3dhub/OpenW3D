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
 *                     $Archive:: /Commando/Code/Combat/humanstate.cpp                        $* 
 *                                                                                             * 
 *                      $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 2/26/02 11:47a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 169                                                         $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
**	Includes
*/
#include "humanstate.h"
#include "debug.h"
#include "animcontrol.h"
#include "humanphys.h"
#include "weapons.h"
#include "assets.h"
#include "combat.h"
#include "combat.h"
#include "physcoltest.h"
#include "pscene.h"
#include <stdio.h>
#include "timemgr.h"
#include "definitionclassids.h"
#include "Pathfind.h"
#include "encoderlist.h"
#include "bitpackids.h"
#include "humanrecoil.h"
#include "globalsettings.h"
#include "combatchunkid.h"
#include "wwprofile.h"
#include "surfaceeffects.h"
#include "crandom.h"
#include "gametype.h"
#include "soldier.h"
#include "damage.h"
#include "transitioneffect.h"
#include "combatmaterialeffectmanager.h"

/*
** Static instance of HumanRecoilClass for recoil calculations
*/
static HumanRecoilClass _TheRecoiler;



/******************************************************************************************
**
** HumanStateClass Implementation
**
******************************************************************************************/

HumanStateClass::HumanStateClass( void ) :
	State( UPRIGHT ),
	StateFlags( 0 ),
	StateTimer( 0 ),
	SubState( 0 ),
	StateLocked( false ),
	AnimControl( NULL ),
	WeaponHoldStyle(WEAPON_HOLD_STYLE_EMPTY_HANDS),
	HumanPhys( NULL ),
	TurnVelocity( 0 ),
	AimingTilt( 0 ),
	AimingTurn( 0 ),
	RecoilTimer( 0.0f ),
	RecoilScale( 1.0f ),
	LoiterDelay( 0 ),
	LoitersAllowed( true ),
	LegRotation( 0 ),
	WeaponHoldTimer( 0 ),
	NoAnimBlend( false ),
	HumanAnimOverride( NULL ),
	HumanLoiterCollection( NULL ),
	WeaponFired( false )
{
	Reset_Loiter_Delay();
}

HumanStateClass::~HumanStateClass( void ) 
{
	if ( HumanPhys != NULL ) {
		HumanPhys->Release_Ref();
		HumanPhys = NULL;
	}
}

void	HumanStateClass::Init( HumanPhysClass	* human_phys )
{
	WWASSERT( HumanPhys == NULL );
	WWASSERT( human_phys != NULL );

	HumanPhys = human_phys;
	HumanPhys->Add_Ref();
}

void	HumanStateClass::Reset( void )
{
	REF_PTR_RELEASE( HumanPhys );

	// Clear the sniping flag
	if ( Get_State_Flag( SNIPING_FLAG ) ) {
		Toggle_State_Flag( SNIPING_FLAG );
	}
}

void	HumanStateClass::Set_Anim_Control( HumanAnimControlClass * anim_control )
{
	WWASSERT( AnimControl == NULL || AnimControl == anim_control );
	WWASSERT( anim_control != NULL );
	AnimControl = anim_control;
	AnimControl->Set_Model( HumanPhys->Peek_Model() );
}

void	HumanStateClass::Set_Human_Anim_Override( const char * name )
{
	HumanAnimOverride = (HumanAnimOverrideDef *)DefinitionMgrClass::Find_Typed_Definition( name, 
															CLASSID_GLOBAL_SETTINGS_DEF_HUMAN_ANIM_OVERRIDE );
}

void	HumanStateClass::Set_Human_Anim_Override( int def_id )
{
	HumanAnimOverride = (HumanAnimOverrideDef *)DefinitionMgrClass::Find_Definition( def_id );
}

void	HumanStateClass::Set_Human_Loiter_Collection( int def_id )
{
	HumanLoiterCollection = (HumanLoiterGlobalSettingsDef *)DefinitionMgrClass::Find_Definition( def_id );
}

/*
** CommandoHumanState Save and Load
*/
enum	{
	CHUNKID_VARIABLES						=	915991207,
	XXX_CHUNKID_ANIM_CONTROL,

	MICROCHUNKID_STATE					=	1,
	MICROCHUNKID_SUB_STATE,
	MICROCHUNKID_STATE_LOCKED,
	MICROCHUNKID_WEAPON_HOLD_STYLE,
	XXXMICROCHUNKID_WEAPON_STATE,
	MICROCHUNKID_AIMING_TILT,
	MICROCHUNKID_AIMING_TURN,
	MICROCHUNKID_TURN_VELOCITY,
	MICROCHUNKID_PHYSOBJ,
	MICROCHUNKID_LOITER_DELAY,
	MICROCHUNKID_STATE_FLAGS,
	MICROCHUNKID_JUMP_TM,
	MICROCHUNKID_STATE_TIMER,
	MICROCHUNKID_LOITERS_ALLOWED,
	MICROCHUNKID_WEAPON_HOLD_TIMER,
	MICROCHUNKID_HUMAN_ANIM_OVERRIDE_DEF_ID,
	MICROCHUNKID_HUMAN_LOITER_COLLECTION_DEF_ID,
};

bool	HumanStateClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STATE, State );         		
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STATE_FLAGS, StateFlags );         		
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_SUB_STATE, SubState );      
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STATE_LOCKED, StateLocked );   
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_WEAPON_HOLD_STYLE, WeaponHoldStyle );   
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_AIMING_TILT, AimingTilt );    
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_AIMING_TURN, AimingTurn );    
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TURN_VELOCITY, TurnVelocity );  
		WRITE_MICRO_CHUNK_PTR( csave, MICROCHUNKID_PHYSOBJ,	HumanPhys );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_LOITER_DELAY,	LoiterDelay );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_LOITERS_ALLOWED,	LoitersAllowed );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_JUMP_TM,	JumpTM );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STATE_TIMER, StateTimer );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_WEAPON_HOLD_TIMER, WeaponHoldTimer );

		if ( HumanAnimOverride != NULL ) {
			int id = HumanAnimOverride->Get_ID();
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_HUMAN_ANIM_OVERRIDE_DEF_ID, id );
		}

		if ( HumanLoiterCollection != NULL ) {
			int id = HumanLoiterCollection->Get_ID();
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_HUMAN_LOITER_COLLECTION_DEF_ID, id );
		}

	csave.End_Chunk();

//	Don't need to save AnimControl, it gets set externally
// We don't save recoil states
	return true;
}

bool	HumanStateClass::Load( ChunkLoadClass &cload )
{
	int human_anim_override_def_id = 0;
	int human_loiter_collection_def_id = 0;

	WWASSERT( HumanPhys == NULL );
	
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_VARIABLES:

				WWASSERT( HumanPhys == NULL );

				while (cload.Open_Micro_Chunk()) {
					WWASSERT(SubState >= 0 && SubState <= HIGHEST_HUMAN_SUB_STATE);

					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STATE, State );         		
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STATE_FLAGS, StateFlags );         		
						READ_MICRO_CHUNK( cload, MICROCHUNKID_SUB_STATE, SubState );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STATE_LOCKED, StateLocked );   
						READ_MICRO_CHUNK( cload, MICROCHUNKID_WEAPON_HOLD_STYLE, WeaponHoldStyle );   
						READ_MICRO_CHUNK( cload, MICROCHUNKID_AIMING_TILT, AimingTilt );    
						READ_MICRO_CHUNK( cload, MICROCHUNKID_AIMING_TURN, AimingTurn );    
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TURN_VELOCITY, TurnVelocity );  
						READ_MICRO_CHUNK_PTR( cload, MICROCHUNKID_PHYSOBJ,	HumanPhys );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_LOITER_DELAY,	LoiterDelay );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_LOITERS_ALLOWED,	LoitersAllowed );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_JUMP_TM,	JumpTM );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STATE_TIMER, StateTimer );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_WEAPON_HOLD_TIMER, WeaponHoldTimer );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_HUMAN_ANIM_OVERRIDE_DEF_ID, human_anim_override_def_id );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_HUMAN_LOITER_COLLECTION_DEF_ID, human_loiter_collection_def_id );

						default:
							Debug_Say(( "Unrecognized Human Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}

				WWASSERT( HumanPhys != NULL );

				if ( HumanPhys != NULL ) {
					REQUEST_REF_COUNTED_POINTER_REMAP( (RefCountClass **)&HumanPhys );
				}

				break;

			default:
				Debug_Say(( "Unrecognized Human chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	if ( human_anim_override_def_id != 0 ) {
		Set_Human_Anim_Override( human_anim_override_def_id );
	}

	if ( human_loiter_collection_def_id != 0 ) {
		Set_Human_Loiter_Collection( human_loiter_collection_def_id );
	}

	return true;
}

/*
**
*/
void	HumanStateClass::Update_Weapon( WeaponClass * weapon, bool new_weapon )
{
	WWPROFILE( "Human Weapon" );

	Update_Recoil(weapon);

	int new_hold_style = WeaponHoldStyle;

	WeaponFired = false;

	if ( weapon ) {

		if ( weapon->Is_Reloading() ) {
			Raise_Weapon();
		}

		weapon->Set_Safety( WeaponHoldStyle > WEAPON_HOLD_STYLE_EMPTY_HANDS );

		// Only humans can cansider weapons firing in state
		if ( weapon->Get_Owner() && 
			  weapon->Get_Owner()->As_SoldierGameObj() && 
			  weapon->Get_Owner()->As_SoldierGameObj()->Is_Human_Controlled() ) {
			WeaponFired = weapon->Is_Firing();
		}


		if ( weapon->Is_Triggered() || new_weapon || Get_State_Flag( CROUCHED_FLAG ) ) {
			if ( weapon->Is_Safety_Set() ) {		// Don't blend anim when un-safetying
				NoAnimBlend = true;
			}
			Raise_Weapon();					// Reset the drop timer
			new_hold_style = weapon->Get_Style();		// Take the new style
		}

		if ( WeaponHoldTimer > 0 ) {
			if ( WeaponHoldStyle > WEAPON_HOLD_STYLE_EMPTY_HANDS ) {
				new_hold_style = weapon->Get_Style();		// Take the new style
			}
		}
	} else {
		new_hold_style = WEAPON_HOLD_STYLE_EMPTY_HANDS;
	}

	if ( new_hold_style == WeaponHoldStyle ) {
		return;
	}

	WeaponHoldStyle = new_hold_style;

	if ( !StateLocked ) {
		Update_Animation();
	}
}

void	HumanStateClass::Update_Aiming( float tilt, float turn )
{
	if ( (AimingTilt == tilt) && (AimingTurn == turn) ) {
		return;
	}

	AimingTilt = tilt;
	AimingTurn = turn;

	if ( !StateLocked ) {
		Update_Animation();
	}
}

void HumanStateClass::Update_Recoil(WeaponClass * weapon)
{
	// Programatic Recoil System.  This code needs to run once per frame.
	if ((weapon != NULL) && (weapon->Is_Firing())) {
		
		// Set our recoil timer and capture all of the necessary bones 
		// I'm copying all necessary data out of the weapon in case the weapon goes
		// away before our recoil is finished.
		RecoilTimer = weapon->Get_Recoil_Time();
		RecoilScale = weapon->Get_Recoil_Scale();
		if (RecoilTimer > 0.0f) {
			RecoilScale *= 1.0f / RecoilTimer; 
			_TheRecoiler.Capture_Bones(HumanPhys->Peek_Model());
		}
	}

	if (RecoilTimer > 0.0f) {

		// Apply the recoil effect.
		Matrix3D recoil_tm(1);
		recoil_tm.Rotate_Z( HumanPhys->Get_Facing() );
		recoil_tm.Rotate_Y( -AimingTilt );
		float recoil_scale = RecoilScale * RecoilTimer;
		_TheRecoiler.Apply_Recoil( recoil_tm,HumanPhys->Peek_Model(),recoil_scale);
		
		// Decrement the recoil timer and release the bones if it expires
		RecoilTimer -= TimeManager::Get_Frame_Seconds();
		if (RecoilTimer <= 0.0f) {
			RecoilTimer = 0.0f;
			_TheRecoiler.Release_Bones(HumanPhys->Peek_Model());
		}
	}
}

void	HumanStateClass::Set_State( HumanStateType state, int sub_state )
{
	// Special case for death
	if (( State == DEATH ) || ( State == DESTROY )) {
		if ( state != DESTROY ) {
			return;
		}
	}

	if ( ( State != DEATH ) && ( state == DEATH ) ) {
		StateLocked = false;
	}

	if ( StateLocked ) {
		if ( state != DEATH ) {
// Temp Test
//			Debug_Say(( "State is Locked.  Can't change from %d to %d\n", State, state ));
//			return;
		}

// FIXME StateLocked Hack
		if ( !IS_SOLOPLAY ) {		// E3 HACK
			StateLocked = false;
		}
	}

	if ( State == state && SubState == sub_state ) {
		if (CombatManager::I_Am_Server()) {
			Debug_Say(( "Already in this state\n" ));
		}
		return;
	}

//	Debug_Say(( "%p Set State %d, %x from %d\n", this, state, sub_state, State ));
	State = state;
	SubState = sub_state;
	StateTimer = 0;

	if (( State == LADDER ) || ( State == IN_VEHICLE ) || 
		 ( State == TRANSITION ) || ( State == TRANSITION_COMPLETE ) ||
		 ( State == DEBUG_FLY )						 ) {
		HumanPhys->Enable_User_Control( true );
	} else {
		HumanPhys->Enable_User_Control( false );
	}

	// Turn off shadows in vehicles
// FIXME (gth) shadow review hacking
#if 0
	if ( State == IN_VEHICLE )  {
		HumanPhys->Enable_Shadow_Generation( false );
	} else {
		HumanPhys->Enable_Shadow_Generation( true );
	}
#endif

	if ( ( State == IN_VEHICLE ) || ( State == TRANSITION ) || ( State == TRANSITION_COMPLETE ) ) {
		HumanPhys->Set_Collision_Group( BULLET_ONLY_COLLISION_GROUP );
		HumanPhys->Set_Immovable( true );
	} else if ( ( State == DESTROY ) || ( State == DEATH ) ) {
		HumanPhys->Set_Collision_Group( TERRAIN_ONLY_COLLISION_GROUP );
		HumanPhys->Set_Immovable( true );
	} else {
		HumanPhys->Set_Collision_Group( SOLDIER_COLLISION_GROUP );
		HumanPhys->Set_Immovable( false );
	}

	Update_Animation();
}


bool	HumanStateClass::Is_State_Interruptable( void )
{
	return (State == UPRIGHT) || (State == WOUNDED) || (State == LAND) || (State == LOITER) || (State == ANIMATION);
}


#define ADD_CASE(exp)  case exp: return #exp; break;  
const char *  HumanStateClass::Get_State_Name( void )
{
   switch (State) {
		ADD_CASE(UPRIGHT);
		ADD_CASE(LAND);
		ADD_CASE(ANIMATION);
		ADD_CASE(WOUNDED);
		ADD_CASE(LOITER);
		ADD_CASE(AIRBORNE);
		ADD_CASE(DIVE);
		ADD_CASE(DEATH);
		ADD_CASE(LADDER);
		ADD_CASE(IN_VEHICLE);
		ADD_CASE(TRANSITION);
		ADD_CASE(TRANSITION_COMPLETE);
		ADD_CASE(DESTROY);
		ADD_CASE(DEBUG_FLY);
		ADD_CASE(ON_FIRE);
		ADD_CASE(ON_CHEM);
		ADD_CASE(ON_ELECTRIC);
		ADD_CASE(ON_CNC_FIRE);
		ADD_CASE(ON_CNC_CHEM);
		ADD_CASE(LOCKED_ANIMATION);

		default: 
			WWASSERT(0);
			return ""; // to avoid compiler warning
	}
}

void	HumanStateClass::Set_Sub_State( int sub_state )
{
	if ( Is_Sub_State_Adjustable() ) {
		if ( SubState != sub_state ) {
			WWASSERT(sub_state >= 0 && sub_state <= HIGHEST_HUMAN_SUB_STATE);
			SubState = sub_state;
			Update_Animation();
		}
	} else {
		Debug_Say(( "Can't adjust state %s", Get_State_Name() ));
	}
}

bool	HumanStateClass::Is_Sub_State_Adjustable( void )
{
	return (State == UPRIGHT) || (State == LADDER);
}

void	HumanStateClass::Start_Transition_Animation( const char * anim_name, bool blend )
{
	if ( StateLocked ) {
		Debug_Say(( "State is Locked.  Can't Start Transition Anim %s\n", anim_name ));
		return;
	}

	//Debug_Say(("Start_Transition_Animation %s\n", anim_name));

	if (( Get_State() == DEATH ) || ( Get_State() == DESTROY ) ) {
		return;
	}


	Set_State( TRANSITION );

	float blend_time = blend ? 0.2 : 0;
	AnimControl->Set_Animation( anim_name, blend_time );
	AnimControl->Set_Mode( ANIM_MODE_ONCE );
	AnimControl->Update( 0 );	// update 
	StateLocked = true;
}

void	HumanStateClass::Start_Scripted_Animation( const char * anim_name, bool blend, bool looping )
{
#if 0
	if ( StateLocked ) {
		Debug_Say(( "State is Locked.  Can't Start Transition Anim %s\n", anim_name ));
		return;
	}
#endif

	//Debug_Say(("Start_Scripted_Animation %s\n", anim_name));

	// We used to not start a scripted anim when wounded, but then the scripts couldn't set
	// up custom events well.  So I am gonna try to remove the wounded check
	// 8/17/01  Byon
//	if (( Get_State() == DEATH ) || ( Get_State() == DESTROY ) || ( Get_State() == WOUNDED )) {
	if (( Get_State() == DEATH ) || ( Get_State() == DESTROY ) ) {
		return;
	}


	Set_State( ANIMATION );

	float blend_time = blend ? 0.2 : 0;
	AnimControl->Set_Animation( anim_name, blend_time );
	AnimControl->Set_Mode( looping ? ANIM_MODE_LOOP : ANIM_MODE_ONCE );
	AnimControl->Update( 0 );	// update 
	StateLocked = true;
}

void	HumanStateClass::Stop_Scripted_Animation( void )
{
	if ( State != ANIMATION && State != LOCKED_ANIMATION ) {
//		Debug_Say(( "Not in a Scripted Animation to stop\n" ));

		// Better clear the locked state.  This was keeping us in the place beacon mode
		StateLocked = false;
		return;
	}

	StateLocked = false;
	Set_State( UPRIGHT );
//	AnimControl->Lock_Animation();
}

void	HumanStateClass::Force_Animation( const char * anim_name, bool blend )
{
	//Debug_Say(( "Forcing Animation to %s\n", anim_name ));
	float blend_time = blend ? 0.2 : 0;
	AnimControl->Set_Animation( anim_name, blend_time );
	AnimControl->Update( 0 );	// update 
}

/*
**
*/
typedef enum {
	LEG_STYLE_STAND,						// A0
	LEG_STYLE_RUN_FORWARD,				// A1
	LEG_STYLE_RUN_BACKWARD,				// A2
	LEG_STYLE_RUN_LEFT,					// A3
	LEG_STYLE_RUN_RIGHT,					// A4
	LEG_STYLE_TURN_LEFT,					// A5
	LEG_STYLE_TURN_RIGHT,				// A6
	LEG_STYLE_WALK_FORWARD,				// B1
	LEG_STYLE_WALK_BACKWARD,			// B2
	LEG_STYLE_WALK_LEFT,					// B3
	LEG_STYLE_WALK_RIGHT,				// B4
	LEG_STYLE_CROUCH,						// C0
	LEG_STYLE_CROUCH_MOVE_FORWARD,	// C1
	LEG_STYLE_CROUCH_MOVE_BACKWARD,	// C2
	LEG_STYLE_CROUCH_MOVE_LEFT,		// C3
	LEG_STYLE_CROUCH_MOVE_RIGHT,		// C4
	LEG_STYLE_CROUCH_TURN_LEFT,		// C3
	LEG_STYLE_CROUCH_TURN_RIGHT,		// C4
	LEG_STYLE_JUMP_UP,					// D0
	LEG_STYLE_JUMP_FORWARD,				// D1
	LEG_STYLE_JUMP_BACKWARD,			// D2
	LEG_STYLE_JUMP_LEFT,					// D3
	LEG_STYLE_JUMP_RIGHT,				// D4
} HUMAN_ANIM_LEG_STYLE;

static const char * LegAnimNames[] = {
	"A0",	// LEG_STYLE_STAND,					
	"A1",	// LEG_STYLE_RUN_FORWARD,			
	"A2",	// LEG_STYLE_RUN_BACKWARD,			
	"A3",	// LEG_STYLE_RUN_LEFT,				
	"A4",	// LEG_STYLE_RUN_RIGHT,				
	"A5",	// LEG_STYLE_TURN_LEFT,				
	"A6",	// LEG_STYLE_TURN_RIGHT,				
	"B1",	// LEG_STYLE_WALK_FORWARD,			
	"B2",	// LEG_STYLE_WALK_BACKWARD,			
	"B3",	// LEG_STYLE_WALK_LEFT,				
	"B4",	// LEG_STYLE_WALK_RIGHT,				
	"C0",	// LEG_STYLE_CROUCH,					
	"C1",	// LEG_STYLE_CROUCH_MOVE_FORWARD,	
	"C2",	// LEG_STYLE_CROUCH_MOVE_BACKWARD,
	"C3",	// LEG_STYLE_CROUCH_MOVE_LEFT,		
	"C4",	// LEG_STYLE_CROUCH_MOVE_RIGHT,	
	"C5",	// LEG_STYLE_CROUCH_TURN_LEFT,		
	"C6",	// LEG_STYLE_CROUCH_TURN_RIGHT,	
	"J0",	// LEG_STYLE_JUMP_UP,		
	"J1",	// LEG_STYLE_JUMP_FORWARD,	
	"J2",	// LEG_STYLE_JUMP_BACKWARD,
	"J3",	// LEG_STYLE_JUMP_LEFT,		
	"J4",	// LEG_STYLE_JUMP_RIGHT,	
};

static const char * _weapon_style_names[ NUM_WEAPON_HOLD_STYLES ] = {
		"A0",		//WEAPON_HOLD_STYLE_C4					= 0,
		"A0",		//WEAPON_HOLD_STYLE_NOT_USED,
		"C2",		//WEAPON_HOLD_STYLE_AT_SHOULDER,	// 2
		"D2", 	//WEAPON_HOLD_STYLE_AT_HIP,
		"E2",		//WEAPON_HOLD_STYLE_LAUNCHER,
		"F2",		//WEAPON_HOLD_STYLE_HANDGUN,
		"A0",		//WEAPON_HOLD_STYLE_BEACON
		"A0",		//WEAPON_HOLD_STYLE_EMPTY_HANDS,
		"B0",		//WEAPON_HOLD_STYLE_AT_CHEST,
		"A0",		//WEAPON_HOLD_STYLE_HANDS_DOWN,		
};

static const char * _dive_anims[ 4 * 2 ] = {
	// Forwrd Anims
		"S_A_HUMAN.H_A_SLD1_01",
		"S_A_HUMAN.H_A_SLD1_02",

	// Backward anims
		"S_A_HUMAN.H_A_SLD2_01",
		"S_A_HUMAN.H_A_SLD2_02",

	// Left anims
		"S_A_HUMAN.H_A_SLD3_01",
		"S_A_HUMAN.H_A_SLD3_02",

	// Right Anims
		"S_A_HUMAN.H_A_SLD4_01",
		"S_A_HUMAN.H_A_SLD4_02",

};

// Weapons style, weapon action, recoil, blend, vehicle, mix/math, aiming tilt
void	HumanStateClass::Update_Animation( void )
{
	WWPROFILE( "Human Animation" );

	// no updates for visceroids
	if ( AnimControl->Get_Skeleton() == 'V' ) {
		StateLocked = true;
		return;
	}

	if ( StateLocked ) {
//		Debug_Say(( "ERROR: updating animation when locked\n" ));
		return;
		// if you change your animn when locked, death state may clear a scripted anim
	}

	int hold_style = WeaponHoldStyle;

	// Setup animation for state, substate, weapon, tilt, etc.
	if ( (State == UPRIGHT) || (State == AIRBORNE) ) {
		// determine leg style
		int	leg_style = LEG_STYLE_STAND;
		if ( State == AIRBORNE ) {

			leg_style = LEG_STYLE_JUMP_UP;
			if ( SubState & SUB_STATE_LEFT ) 				leg_style = LEG_STYLE_JUMP_LEFT;
			if ( SubState & SUB_STATE_RIGHT )				leg_style = LEG_STYLE_JUMP_RIGHT;
			if ( SubState & SUB_STATE_FORWARD ) 			leg_style = LEG_STYLE_JUMP_FORWARD;
			if ( SubState & SUB_STATE_BACKWARD )			leg_style = LEG_STYLE_JUMP_BACKWARD;

		} else {
			if ( SubState & SUB_STATE_TURN_LEFT ) 		leg_style = LEG_STYLE_TURN_LEFT;
			if ( SubState & SUB_STATE_TURN_RIGHT )		leg_style = LEG_STYLE_TURN_RIGHT;
			if ( SubState & SUB_STATE_LEFT ) 				leg_style = LEG_STYLE_RUN_LEFT;
			if ( SubState & SUB_STATE_RIGHT )				leg_style = LEG_STYLE_RUN_RIGHT;
			if ( SubState & SUB_STATE_FORWARD ) 			leg_style = LEG_STYLE_RUN_FORWARD;
			if ( SubState & SUB_STATE_BACKWARD )			leg_style = LEG_STYLE_RUN_BACKWARD;

			if ( Get_State_Flag( CROUCHED_FLAG ) ) {

				// Tend to hold at chest when crouched
				if (	( hold_style == WEAPON_HOLD_STYLE_HANDS_DOWN ) ||
						( hold_style == WEAPON_HOLD_STYLE_C4 ) ||
						( hold_style == WEAPON_HOLD_STYLE_BEACON ) ) {

					hold_style = WEAPON_HOLD_STYLE_AT_CHEST;
				}

				leg_style += LEG_STYLE_CROUCH - LEG_STYLE_STAND;
			} else if ( SubState & SUB_STATE_SLOW ) {	
				if ( ( leg_style >= LEG_STYLE_RUN_FORWARD ) && 
					  ( leg_style <= LEG_STYLE_RUN_RIGHT ) ) {
					leg_style += LEG_STYLE_WALK_FORWARD - LEG_STYLE_RUN_FORWARD;
				}
			}
		}

		const char * leg_anim_name = LegAnimNames[ leg_style ];
		const char * torso_anim_name = _weapon_style_names[hold_style];

		bool single_anim = true;

		float blend_time = 0.2f;
		if ( NoAnimBlend ) {
			blend_time = 0;
			NoAnimBlend = false;
		}


		if ( torso_anim_name[1] == '2' ) {
			// Lets try aiming
			StringClass	anim1_name(0,true);
			StringClass	anim2_name(0,true);
			StringClass	anim3_name(0,true);
			anim1_name.Format( "S_A_HUMAN.H_A_%c1%s", 'A' + hold_style, leg_anim_name );
			anim2_name.Format( "S_A_HUMAN.H_A_%c2%s", 'A' + hold_style, leg_anim_name );
			anim3_name.Format( "S_A_HUMAN.H_A_%c3%s", 'A' + hold_style, leg_anim_name );

			// See if we have the tilting data
			HAnimClass * anim = WW3DAssetManager::Get_Instance()->Get_HAnim( anim3_name );
			if ( anim != NULL ) {
				anim->Release_Ref();
				single_anim = false;

				float	tilt_blend = WWMath::Clamp( (AimingTilt / DEG_TO_RADF( 65 )), -1, 1 );
				float frame = AnimControl->Get_Frame();	// Maintain the frame number for moving
				if ( tilt_blend < 0 ) {
					AnimControl->Set_Animation( anim1_name, anim2_name, 1+tilt_blend, blend_time );
				} else {
					AnimControl->Set_Animation( anim3_name, anim2_name, 1-tilt_blend, blend_time );
				}

				AnimControl->Set_Mode( ANIM_MODE_LOOP, frame );
			}
		}

		if ( single_anim ) {

			StringClass	anim_name(0,true);
			anim_name.Format( "S_A_HUMAN.H_A_%s%s", torso_anim_name, leg_anim_name );

			// Human Anim Override
			if ( HumanAnimOverride != NULL ) {
				if ( hold_style == WEAPON_HOLD_STYLE_EMPTY_HANDS ) {
					if ( leg_style == LEG_STYLE_RUN_FORWARD ) {
						anim_name = HumanAnimOverride->RunEmptyHands;
					}
					if ( leg_style == LEG_STYLE_WALK_FORWARD ) {
						anim_name = HumanAnimOverride->WalkEmptyHands;
					}
				}
				if ( hold_style == WEAPON_HOLD_STYLE_AT_CHEST ) {
					if ( leg_style == LEG_STYLE_RUN_FORWARD ) {
						anim_name = HumanAnimOverride->RunAtChest;
					}
					if ( leg_style == LEG_STYLE_WALK_FORWARD ) {
						anim_name = HumanAnimOverride->WalkAtChest;
					}
				}
				if ( hold_style == WEAPON_HOLD_STYLE_AT_HIP ) {
					if ( leg_style == LEG_STYLE_RUN_FORWARD ) {
						anim_name = HumanAnimOverride->RunAtHip;
					}
					if ( leg_style == LEG_STYLE_WALK_FORWARD ) {
						anim_name = HumanAnimOverride->WalkAtHip;
					}
				}
			}

			// Saftey check anim
//			Debug_Say(( "Anim name %s\n", (const char *)anim_name ));

//			float frame = AnimControl->Get_Frame();			// Maintain the frame number for moving
			AnimControl->Set_Animation( anim_name, blend_time );
	//		AnimControl->Set_Mode( ANIM_MODE_LOOP, frame );
			AnimControl->Set_Mode( ANIM_MODE_LOOP );
		}

	} else if ( State == DIVE ) {


		const char * anim_name = NULL;
#if 0
		if ( SubState & SUB_STATE_LEFT ) 				anim_name = "S_A_HUMAN.H_A_DIV3";
		if ( SubState & SUB_STATE_RIGHT )				anim_name = "S_A_HUMAN.H_A_DIV4";
		if ( SubState & SUB_STATE_FORWARD ) 			anim_name = "S_A_HUMAN.H_A_DIV1";
		if ( SubState & SUB_STATE_BACKWARD )			anim_name = "S_A_HUMAN.H_A_DIV2";
#else
		int offset = FreeRandom.Get_Int( 2 );
		if ( !IS_SOLOPLAY ) {
			offset = 0;
		}
		if ( SubState & SUB_STATE_FORWARD ) 			offset += 0;
		if ( SubState & SUB_STATE_BACKWARD )			offset += 2;
		if ( SubState & SUB_STATE_LEFT ) 				offset += 4;
		if ( SubState & SUB_STATE_RIGHT )				offset += 6;
		anim_name = _dive_anims[offset];
#endif

		AnimControl->Set_Animation( anim_name, 0.2f );
		AnimControl->Set_Mode( ANIM_MODE_ONCE );
		StateLocked = true;

	} else if ( State == LAND ) {

		int dir = 0;
		if ( SubState & SUB_STATE_LEFT ) 				dir = 3;
		if ( SubState & SUB_STATE_RIGHT )				dir = 4;
		if ( SubState & SUB_STATE_FORWARD ) 			dir = 1;
		if ( SubState & SUB_STATE_BACKWARD )			dir = 2;

		StringClass anim_name(0,true);
		anim_name.Format( "S_A_HUMAN.H_A_A0L%d", dir );
		AnimControl->Set_Animation( anim_name, 0.2f );
		AnimControl->Set_Mode( ANIM_MODE_ONCE );

	} else if ( State == WOUNDED ) {

		AnimControl->Set_Animation( Get_Wound_Anim( SubState ), 0.2f );
		AnimControl->Set_Mode( ANIM_MODE_ONCE );

	} else if ( State == DEATH ) {

		AnimControl->Set_Animation( Get_Death_Anim( SubState ), 0.2f );
		AnimControl->Set_Mode( ANIM_MODE_ONCE );
		StateLocked = true;

	} else if ( State == LADDER ) {

		const char * anim_name = "S_A_HUMAN.H_A_412A";
		if ( SubState & SUB_STATE_UP ) 				anim_name = "S_A_HUMAN.H_A_422A";
		if ( SubState & SUB_STATE_DOWN ) 				anim_name = "S_A_HUMAN.H_A_432A";
		AnimControl->Set_Animation( anim_name, 0.2f );
		AnimControl->Set_Mode( ANIM_MODE_LOOP );

	} else if ( State == ANIMATION ) {
	} else if ( State == LOITER ) {
	} else if ( State == DESTROY ) {
	} else if ( State == TRANSITION ) {
	} else if ( State == TRANSITION_COMPLETE ) {
	} else if ( State == ON_FIRE ) {

		AnimControl->Set_Animation( "S_A_HUMAN.H_A_FLMA", 0.2f );
		AnimControl->Set_Mode( ANIM_MODE_LOOP );

	} else if ( State == ON_CHEM ) {

		AnimControl->Set_Animation( "S_A_HUMAN.h_a_6x01", 0.2f );
		AnimControl->Set_Mode( ANIM_MODE_LOOP );

	} else if ( State == ON_CNC_FIRE ) {

		AnimControl->Set_Animation( "S_A_HUMAN.H_A_FLMA", 0.2f );
		AnimControl->Set_Mode( ANIM_MODE_LOOP );

	} else if ( State == ON_CNC_CHEM ) {

		AnimControl->Set_Animation( "S_A_HUMAN.h_a_6x01", 0.2f );
		AnimControl->Set_Mode( ANIM_MODE_LOOP );

	} else if ( State == ON_ELECTRIC ) {

		AnimControl->Set_Animation( "S_A_HUMAN.h_a_6x05", 0.2f );
		AnimControl->Set_Mode( ANIM_MODE_LOOP );

	} else if ( State == DEBUG_FLY ) {

	} else {
		Debug_Say(( "Uncoded Human State %d\n", State ));
		AnimControl->Set_Animation( (const char *)NULL );
	}
}

#define	MOVING_THRESHHOLD			0.2
//#define	WALKING_THRESHHOLD		4.5
#define	WALKING_THRESHHOLD		3.21f

void	HumanStateClass::Reset_Loiter_Delay( void )									
{ 
	LoiterDelay = FreeRandom.Get_Float( 6 ) - 3;
}

// Must deal with LAND, ANIMATIONS, DEATH, WOUNDS, TRANSITIONS, JUMP, DEATH FALLS, LADDER
void	HumanStateClass::Update_State( void )
{
	WWPROFILE( "Human State" );

	StateTimer += TimeManager::Get_Frame_Seconds();

	if ( AnimControl && AnimControl->Get_Skeleton() == 'V' ) {
		LoitersAllowed = false;
	}

	if ( State == UPRIGHT && LoitersAllowed ) {

		// Don't loiter when crouched or moving
		if ( SubState != 0 ) {
			Reset_Loiter_Delay();
		}

		LoiterDelay += TimeManager::Get_Frame_Seconds();

		HumanLoiterGlobalSettingsDef * loiter_def = HumanLoiterCollection;

		if ( loiter_def == NULL ) {
			loiter_def = HumanLoiterGlobalSettingsDef::Get_Default_Loiters();
		}

#if 0
		// loiter based on holding a weapon
		if ( WeaponHoldStyle == WEAPON_HOLD_STYLE_EMPTY_HANDS ) {
			loiter_def = HumanLoiterGlobalSettingsDef::Get_Weaponless_Loiters();
		} else {
			loiter_def = HumanLoiterGlobalSettingsDef::Get_Weapon_Loiters();
		}
#endif

		if ( loiter_def != NULL ) {
			if ( LoiterDelay > loiter_def->Get_Activation_Delay() ) {

				Set_State( LOITER, Get_Sub_State() );

				StringClass anim(loiter_def->Pick_Animation(),true);
//				Debug_Say(( "Start loiter %s\n", anim ));

				if ( !anim.Is_Empty() ) {
					StringClass new_anim(true);
					::Strip_Path_From_Filename( new_anim, anim );
					// remove the .W3D
					if ( new_anim.Get_Length() >= 5 ) {
						new_anim.Erase( new_anim.Get_Length()-4, 4 );
					}
					AnimControl->Set_Animation( new_anim, 0.2f );
					AnimControl->Set_Mode( ANIM_MODE_ONCE );
					AnimControl->Update( 0 );	// update 
				}
			}
		} else {
			Debug_Say(( "Failed to find Loiter Def\n" ));
		}

	} else {
		Reset_Loiter_Delay();
	}

	// Get out of locked states
	if ( AnimControl->Is_Complete() ) {	// Time to unlock
//		if ( StateLocked && State != LOCKED_ANIMATION ) {
		if ( (StateLocked && State != LOCKED_ANIMATION) || (State == DEATH) ) {

			StateLocked = false;

			if ( State == DIVE ) {
				Set_State( UPRIGHT );
			} else if ( State == ANIMATION ) {
				Set_State( UPRIGHT );
			} else if ( State == DEATH ) {
				
				Set_State( DESTROY );
				TransitionEffectClass * effect = CombatMaterialEffectManager::Get_Death_Effect();
				this->HumanPhys->Add_Effect_To_Me(effect);
				REF_PTR_RELEASE(effect);

			} else if ( State == TRANSITION ) {
				Set_State( TRANSITION_COMPLETE );
			} else if ( State != LOCKED_ANIMATION ) {
				Debug_Say(( "Unsupported locked state %s\n", Get_State_Name() ));
			}
		} else {

//			Debug_Say(( "Anim Complete %s\n", Get_State_Name() ));

			if ( State == LOITER ) {
				Set_State( UPRIGHT );
			} else if ( State == LAND ) {
				Set_State( UPRIGHT, Get_Sub_State() );
			} else if ( State == WOUNDED ) {
				Set_State( UPRIGHT );
			}

		}
	}

	/* 
	** Handle Jump and landing
	*/
	if ( !HumanPhys->Is_In_Contact() ) {	// If I am not in contact with the ground
		if ( State == UPRIGHT ) {				// If I am currently UPRIGHT
			Begin_Jump();							// Begin a jump
		}
	} else if ( State == AIRBORNE ) {		// If I am in contact, and in the , I just landed
		Complete_Jump();							// So complete the jump
	}

	if ( WeaponHoldTimer > 0 && !Get_State_Flag( CROUCHED_FLAG ) ) {
		Reset_Loiter_Delay();
		WeaponHoldTimer -= TimeManager::Get_Frame_Seconds();
		if ( WeaponHoldTimer <= 0 ) {
			if (	WeaponHoldStyle == WEAPON_HOLD_STYLE_HANDGUN ||
					WeaponHoldStyle == WEAPON_HOLD_STYLE_C4 ||
					WeaponHoldStyle == WEAPON_HOLD_STYLE_BEACON )
			{
				WeaponHoldStyle = WEAPON_HOLD_STYLE_HANDS_DOWN;		// Lower the weapon
			} else {
				WeaponHoldStyle = WEAPON_HOLD_STYLE_AT_CHEST;		// Lower the weapon
			}
			Update_Animation();
		}
	}

}

/*
**
*/
void	HumanStateClass::Post_Think( void )
{
	// Update sub_state per movement
	// do it for upright, land, ladder, airborne,
	if ( Is_Sub_State_Adjustable() || Is_State_Interruptable() ) {

		// Update the SubState
		int new_sub_state = 0;

		// Get our current move vector
		Vector3	move_vector;
		HumanPhys->Get_Animation_Move( &move_vector );
		if ( TimeManager::Get_Frame_Seconds() > 0 ) {
			move_vector /= TimeManager::Get_Frame_Seconds();
		}

		move_vector = HumanPhys->Get_Transform().Inverse_Rotate_Vector( move_vector );

		// When walking running diagonally, use forward/backward legs.
		// Unless you are crouched, then use straffe legs
		float direction_ratio = 0.75f;
		if ( Get_State_Flag( CROUCHED_FLAG ) ) {		
			direction_ratio = 2;
		}

		// Convert to SubMode
		if ( WWMath::Fabs( move_vector[0] ) > direction_ratio * WWMath::Fabs( move_vector[1] ) ) {
				  if ( move_vector[0] > MOVING_THRESHHOLD ) 	new_sub_state |= SUB_STATE_FORWARD;
			else if ( move_vector[0] < -MOVING_THRESHHOLD )	new_sub_state |= SUB_STATE_BACKWARD;
		} else {
				  if ( move_vector[1] > MOVING_THRESHHOLD ) 	new_sub_state |= SUB_STATE_LEFT;
			else if ( move_vector[1] < -MOVING_THRESHHOLD )	new_sub_state |= SUB_STATE_RIGHT;
		}

		if ( new_sub_state == 0 ) {
				  if ( move_vector[2] > MOVING_THRESHHOLD ) 	new_sub_state |= SUB_STATE_UP;
			else if ( move_vector[2] < -MOVING_THRESHHOLD )	new_sub_state |= SUB_STATE_DOWN;
		}

		if ( new_sub_state != 0 ) {
			if ( move_vector.Length() < WALKING_THRESHHOLD ) new_sub_state |= SUB_STATE_SLOW;
			//Debug_Say(( "%f %f\n", move_vector.Length(), WALKING_THRESHHOLD ));
		}																		

#if 0	// No turn anims!!!
		// Get our current turn vector
		if ( TurnVelocity > 0 )				new_sub_state |= SUB_STATE_TURN_LEFT;
		else if ( TurnVelocity < 0 ) 		new_sub_state |= SUB_STATE_TURN_RIGHT;
		TurnVelocity = 0;
#endif

		// Get him out of WOUNDED, LAND, LOITER states if moving or shooting
		if ( Is_State_Interruptable() && Get_State() != UPRIGHT ) {
//			if ( new_sub_state != 0 || WeaponState > WeaponClass::STATE_READY ) {

			if ( new_sub_state != 0 || WeaponFired ) {
				if ( Get_State() == LAND && Get_Sub_State() == new_sub_state ) {
					// don't interrupt lands for the same direction
				} else {
//					Debug_Say(( "Interrupt State %s\n", Get_State_Name() ));
					Set_State( UPRIGHT );
				}
			}
		}
		
		if ( Is_Sub_State_Adjustable() ) {

			if ( new_sub_state != Get_Sub_State() ) {
				Set_Sub_State( (HumanSubStateType)new_sub_state );
			}


#if 0		// Disable all leg twisting

// ===================================================================			// LEG TWIST!!!!!

			// Don't leg twist for crouched
			if ( Get_State() == UPRIGHT && !Get_State_Flag( CROUCHED_FLAG ) ) {

				float	legs_rotation = 0;

				// Compare the facing to the motion, set leg_racing to the difference
				if ( move_vector.Length() > 0 ) {
					float move_direction = WWMath::Atan2( -move_vector.Y, move_vector.X );

					if ( new_sub_state & SUB_STATE_FORWARD ) {
						legs_rotation = -move_direction;
					} else if ( new_sub_state & SUB_STATE_BACKWARD ) {
						legs_rotation = -move_direction + DEG_TO_RAD( 180 );
					} else if ( new_sub_state & SUB_STATE_LEFT ) {
						legs_rotation = -move_direction + DEG_TO_RAD( 270 );
					} else if ( new_sub_state & SUB_STATE_RIGHT ) {
						legs_rotation = -move_direction + DEG_TO_RAD( 90 );
					}

					legs_rotation = WWMath::Wrap( legs_rotation, DEG_TO_RADF( -180 ), DEG_TO_RADF( 180 ) );
//					legs_rotation = WWMath::Clamp( legs_rotation, DEG_TO_RADF( -45 ), DEG_TO_RADF( 45 ) );
					legs_rotation = WWMath::Clamp( legs_rotation, DEG_TO_RADF( -30 ), DEG_TO_RADF( 30 ) );

					if ( WWMath::Fabs( legs_rotation ) < DEG_TO_RAD( 25 ) ) {
						legs_rotation = 0;
					}
				}

				// Move LegRotation toward leg_rotation
				float rot_diff = legs_rotation - LegRotation;
				float max_mov = DEG_TO_RAD( 180 ) * TimeManager::Get_Frame_Seconds();
//				float max_mov = DEG_TO_RAD( 90 ) * TimeManager::Get_Frame_Seconds();
				rot_diff = WWMath::Clamp( rot_diff, -max_mov, max_mov );
				LegRotation += rot_diff;
				legs_rotation = LegRotation;

				// I'm making this staic for now, because all human
				// skeletons have the bone at the same index
				static int  root_bone = -1;
				if ( root_bone == -1 ) {			// Get root bone index
					root_bone = HumanPhys->Peek_Model()->Get_Bone_Index( "c spine" );
				}

				static int  torso_bone = -1;
				if ( torso_bone == -1 ) {			// Get torso bone index
					torso_bone = HumanPhys->Peek_Model()->Get_Bone_Index( "c spine1" );
				}

				if ( legs_rotation != 0 ) {

					WWASSERT( root_bone != -1 );
					WWASSERT( torso_bone != -1 );

					if ( !HumanPhys->Peek_Model()->Is_Bone_Captured( root_bone ) ) {
						HumanPhys->Peek_Model()->Capture_Bone( root_bone );
					}
					if ( !HumanPhys->Peek_Model()->Is_Bone_Captured( torso_bone ) ) {
						HumanPhys->Peek_Model()->Capture_Bone( torso_bone );
					}

					Matrix3D	root_adjust(1);				// adjust it
					root_adjust.Rotate_X( legs_rotation );
					HumanPhys->Peek_Model()->Control_Bone( root_bone, root_adjust );

					Matrix3D	legs_adjust(1);				// adjust it
					legs_adjust.Rotate_X( -legs_rotation );
					HumanPhys->Peek_Model()->Control_Bone( torso_bone, legs_adjust );
				} else {	// no adjustment, release
	 				if ( HumanPhys->Peek_Model()->Is_Bone_Captured( root_bone ) ) {
						HumanPhys->Peek_Model()->Release_Bone( root_bone );
					}
	 				if ( HumanPhys->Peek_Model()->Is_Bone_Captured( torso_bone ) ) {
						HumanPhys->Peek_Model()->Release_Bone( torso_bone );
					}
				}
			}
#endif
		}

		// Scale animation speed
		float ideal_speed = 0;
		if ( !(new_sub_state & SUB_STATE_SLOW) ) {
			if ( new_sub_state & SUB_STATE_FORWARD )	ideal_speed = 5.5f;
			if ( new_sub_state & SUB_STATE_BACKWARD )	ideal_speed = 4.5f;
			if ( new_sub_state & SUB_STATE_LEFT )		ideal_speed = 4.5f;
			if ( new_sub_state & SUB_STATE_RIGHT )		ideal_speed = 5.5f;
		} else {
			if ( new_sub_state & SUB_STATE_FORWARD )	ideal_speed = 1.6f;
			if ( new_sub_state & SUB_STATE_BACKWARD )	ideal_speed = 1.5f;
			if ( new_sub_state & SUB_STATE_LEFT )		ideal_speed = 1.5f;
			if ( new_sub_state & SUB_STATE_RIGHT )		ideal_speed = 1.6f;
		}

		if ( State == LADDER ) {
			if ( new_sub_state & SUB_STATE_UP )			ideal_speed = 0.15f;
			if ( new_sub_state & SUB_STATE_DOWN )		ideal_speed = 0.15f;
		}

		// Turning is at speed 1
		bool turning = ( new_sub_state & (SUB_STATE_TURN_LEFT | SUB_STATE_TURN_RIGHT ) && 
						  !( new_sub_state & (SUB_STATE_FORWARD | SUB_STATE_BACKWARD )));

		if ( !turning && ideal_speed != 0 ) {
			// Get Anim_Speed_Scale
			Vector3	vel;
			HumanPhys->Get_Animation_Move( &vel );
			if ( TimeManager::Get_Frame_Seconds() > 0 ) {
				vel /= TimeManager::Get_Frame_Seconds();
			}
			float speed = WWMath::Clamp( vel.Length() / ideal_speed, 0.33f, 3 );
			AnimControl->Set_Anim_Speed_Scale( speed );
		} else {
			AnimControl->Set_Anim_Speed_Scale( 1 );
		}

		HumanPhys->Reset_Animation_Move();

	}
}

/*
**
*/
bool	HumanStateClass::Get_Leg_Mode( void )		
{ 
	return AnimControl->Get_Progress() > 0.5f; 
}

/*
**
*/
struct BoneToOuchType {
	const char * bone_name;
	int ouch_type;
};

BoneToOuchType	_BoneToOuchTypeList[] = {
	{ "K_HEAD", HumanStateClass::HEAD_FROM_BEHIND },
	{ "K_NECK", HumanStateClass::HEAD_FROM_BEHIND },
	{ "K_CHEST", HumanStateClass::HEAD_FROM_BEHIND },
	{ "K_ABDOMEN", HumanStateClass::TORSO_FROM_BEHIND },
	{ "K_PELVIS", HumanStateClass::GROIN },
	{ "K_L THIGH", HumanStateClass::LEFT_LEG_FROM_BEHIND },
	{ "K_L CALF", HumanStateClass::LEFT_LEG_FROM_BEHIND },
	{ "K_L FOOT", HumanStateClass::LEFT_LEG_FROM_BEHIND },
	{ "K_L HAND", HumanStateClass::LEFT_ARM_FROM_BEHIND },
	{ "K_L FOREARM", HumanStateClass::LEFT_ARM_FROM_BEHIND },
	{ "K_L UPPERARM", HumanStateClass::LEFT_ARM_FROM_BEHIND },
	{ "K_R THIGH", HumanStateClass::RIGHT_LEG_FROM_BEHIND },
	{ "K_R CALF", HumanStateClass::RIGHT_LEG_FROM_BEHIND },
	{ "K_R FOOT", HumanStateClass::RIGHT_LEG_FROM_BEHIND },
	{ "K_R HAND", HumanStateClass::RIGHT_ARM_FROM_BEHIND },
	{ "K_R FOREARM", HumanStateClass::RIGHT_ARM_FROM_BEHIND },
	{ "K_R UPPERARM", HumanStateClass::RIGHT_ARM_FROM_BEHIND },
};
#define	BONE_LIST_COUNT	( sizeof(_BoneToOuchTypeList) / sizeof(_BoneToOuchTypeList[0]) )


int	HumanStateClass::Get_Ouch_Type( const	Vector3 & direction, const char * collision_box_name )
{
	// Initialize ouch_type to a default value
	int ouch_type = TORSO_FROM_FRONT;
	
	const char * base_name = NULL;
	if ( collision_box_name != NULL ) {
		base_name = ::strchr( collision_box_name, '.' );
	} else {
		return ouch_type;
	}

	if ( base_name != NULL ) {
		base_name++;
		for ( int i = 0; i < BONE_LIST_COUNT; i++ ) {
			if ( ::strcmp( _BoneToOuchTypeList[i].bone_name, base_name ) == 0 ) {
				ouch_type = _BoneToOuchTypeList[i].ouch_type;
			}
		}
	}

	if ( ouch_type != -1 ) {
		// Set direction
		Vector3 relative_direction = HumanPhys->Get_Transform().Inverse_Rotate_Vector( direction );
		if ( ouch_type != HumanStateClass::GROIN ) {		// but not for the groin shot
			if ( relative_direction.X < 0 ) {
				ouch_type += 1;
			}
		}
	} else {
		Debug_Say(( "Bad human collision box name %s\n", collision_box_name ));
	}

	return ouch_type;
}

/*
**
*/
const char * _WoundAnims[] = 
{
	"S_A_HUMAN.H_A_811A",	// HEAD_FROM_BEHIND,       
	"S_A_HUMAN.H_A_812A",	// HEAD_FROM_FRONT,        
	"S_A_HUMAN.H_A_821A",	// TORSO_FROM_BEHIND,      
	"S_A_HUMAN.H_A_822A",	// TORSO_FROM_FRONT,       
	"S_A_HUMAN.H_A_831A",	// LEFT_ARM_FROM_BEHIND,   
	"S_A_HUMAN.H_A_832A",	// LEFT_ARM_FROM_FRONT,    
	"S_A_HUMAN.H_A_841A",	// RIGHT_ARM_FROM_BEHIND,  
	"S_A_HUMAN.H_A_842A",	// RIGHT_ARM_FROM_FRONT,   
	"S_A_HUMAN.H_A_851A",	// LEFT_LEG_FROM_BEHIND,   
	"S_A_HUMAN.H_A_852A",	// LEFT_LEG_FROM_FRONT,    
	"S_A_HUMAN.H_A_861A",	// RIGHT_LEG_FROM_BEHIND,  
	"S_A_HUMAN.H_A_862A",	// RIGHT_LEG_FROM_FRONT,   
	"S_A_HUMAN.H_A_871A",	// GROIN,                  
};

const char *	HumanStateClass::Get_Wound_Anim( int ouch_type )
{
	return _WoundAnims[ ouch_type ];
}

/*
**
*/
const char * _DeathAnims[] = 
{
	"S_A_HUMAN.H_A_622A",	// HEAD_FROM_BEHIND,       
	"S_A_HUMAN.H_A_635A",	// HEAD_FROM_FRONT,        
	"S_A_HUMAN.H_A_622A",	// TORSO_FROM_BEHIND,      
	"S_A_HUMAN.H_A_632A",	// TORSO_FROM_FRONT,       
	"S_A_HUMAN.H_A_623A",	// LEFT_ARM_FROM_BEHIND,   
	"S_A_HUMAN.H_A_634A",	// LEFT_ARM_FROM_FRONT,    
	"S_A_HUMAN.H_A_624A",	// RIGHT_ARM_FROM_BEHIND,  
	"S_A_HUMAN.H_A_633A",	// RIGHT_ARM_FROM_FRONT,   
	"S_A_HUMAN.H_A_623A",	// LEFT_LEG_FROM_BEHIND,   
	"S_A_HUMAN.H_A_634A",	// LEFT_LEG_FROM_FRONT,    
	"S_A_HUMAN.H_A_624A",	// RIGHT_LEG_FROM_BEHIND,  
	"S_A_HUMAN.H_A_633A",	// RIGHT_LEG_FROM_FRONT,   
	"S_A_HUMAN.H_A_612A",	// GROIN,                  
	"S_A_HUMAN.H_A_FLMB",	// ON_FIRE,                  
	"S_A_HUMAN.H_A_FLMB",	// ON_CHEM,                  
	"S_A_HUMAN.H_A_FLMB",	// ON_ELECTRIC,                  
	"S_A_HUMAN.H_A_FLMB",	// ON_CNC_FIRE,                  
	"S_A_HUMAN.H_A_FLMB",	// ON_CNC_CHEM,                  
};

const char *	HumanStateClass::Get_Death_Anim( int ouch_type )
{
	WWASSERT( ouch_type <= OUCH_SUPER_FIRE );
	return _DeathAnims[ ouch_type ];
}

/*
**
*/
void HumanStateClass::Set_Precision(void)
{
	cEncoderList::Set_Precision(BITPACK_HUMAN_STATE, 0, (int) HIGHEST_HUMAN_STATE);
	cEncoderList::Set_Precision(BITPACK_HUMAN_SUB_STATE, 0, (int) HIGHEST_HUMAN_SUB_STATE);
}

/*
**
*/
void	HumanStateClass::Get_Information( StringClass & string )
{
	StringClass temp(0,true);
	temp.Format( "%s\n", Get_State_Name() );
	string += temp;

	AnimControl->Get_Information( string );
}

/*
** Begin_Jump is called when we first become airborne
*/
void	HumanStateClass::Begin_Jump( void )
{
	//	Store some information about the jump
	JumpTM	= HumanPhys->Get_Transform();

	// Set my state to Jump, with a direction of my current relative velocity
	Vector3	move_vector;	
	HumanPhys->Get_Velocity( &move_vector );
	move_vector = HumanPhys->Get_Transform().Inverse_Rotate_Vector( move_vector );
	int sub_state = 0;
		  if ( move_vector[0] > MOVING_THRESHHOLD ) 	sub_state |= SUB_STATE_FORWARD;
	else if ( move_vector[0] < -MOVING_THRESHHOLD )	sub_state |= SUB_STATE_BACKWARD;
		  if ( move_vector[1] > MOVING_THRESHHOLD ) 	sub_state |= SUB_STATE_LEFT;
	else if ( move_vector[1] < -MOVING_THRESHHOLD )	sub_state |= SUB_STATE_RIGHT;

	Set_State( AIRBORNE, sub_state );

	// Whenever a jump starts, play a jump surface effect
	Matrix3D tm = HumanPhys->Get_Transform();
	int ground_type = HumanPhys->Get_Contact_Surface_Type();
	SurfaceEffectsManager::Apply_Effect( ground_type, SurfaceEffectsManager::HITTER_TYPE_FOOTSTEP_JUMP, tm );
}

/*
** Complete_Jump is called when we land
*/
void	HumanStateClass::Complete_Jump( void )
{
	Vector3 fall = JumpTM.Get_Translation() - HumanPhys->Get_Transform().Get_Translation();
	[[maybe_unused]] GlobalSettingsDef * settings = GlobalSettingsDef::Get_Global_Settings();
//	Debug_Say(( "Fall Distance %f\n", fall.Z ));

	[[maybe_unused]] bool add_portal = true;

//
//	Don't do this in the level editor
//
#ifndef PARAM_EDITING_ON

	if ( fall.Z > settings->Get_Falling_Damage_Min_Distance() ) {
		add_portal = false;
		float scale = (fall.Z - settings->Get_Falling_Damage_Min_Distance()) / 
			(settings->Get_Falling_Damage_Max_Distance() - settings->Get_Falling_Damage_Min_Distance());
		scale = WWMath::Clamp( scale, 0, 1 );
	
		// (gth) don't take damage when falling onto an elevator (because we have
		// super-fast elevators...)
		if (	(HumanPhys->Peek_Ground_Object() != NULL) && 
				(HumanPhys->Peek_Ground_Object()->As_ElevatorPhysClass() != NULL))
		{
			Debug_Say(( "Fell onto an elevator, no damage!\n", scale ));
			scale = 0.0f;
		}
		
		Debug_Say(( "Fall Damage Scale %f\n", scale ));

		SoldierGameObj * owner = (SoldierGameObj *)HumanPhys->Get_Observer();
		if ( owner != NULL ) {
			DefenseObjectClass * defense = owner->Get_Defense_Object();
			float damage = defense->Get_Health_Max();

			// AI's get notified
//			if ( !owner->Is_Human_Controlled() ) {
				const GameObjObserverList & observer_list = owner->Get_Observers();
				for( int index = 0; index < observer_list.Count(); index++ ) {
					observer_list[ index ]->Custom( owner, CUSTOM_EVENT_FALLING_DAMAGE, damage*scale, NULL );
				}
//			}

			// All get the damage
			OffenseObjectClass offense( damage, settings->Get_Falling_Damage_Warhead(), NULL );
			owner->Apply_Damage_Extended( offense, scale, fall, NULL );

		}
	}

#endif // PARAM_EDITING_ON

	//	Find the sector the human jumped from and the sector the human jumped to.
	/*Vector3 curr_pos							= HumanPhys->Get_Position ();
	PathfindSectorClass *start_sector	= PathfindClass::Get_Instance ()->Find_Sector (JumpTM.Get_Translation (), 2.0F);
	PathfindSectorClass *end_sector		= PathfindClass::Get_Instance ()->Find_Sector (curr_pos, 2.0F);

	//	Make a temporary pathfind connection between these sectors (if necessary)
	if (add_portal && start_sector != NULL && end_sector != NULL && start_sector != end_sector) {									
		PathfindClass::Get_Instance ()->Add_Temporary_Portal (start_sector, end_sector, JumpTM.Get_Translation (), curr_pos);
	}*/

	// Set me state to Land, with a direction of my current velocity
	Vector3	move_vector;
	HumanPhys->Get_Velocity( &move_vector );
	move_vector = HumanPhys->Get_Transform().Inverse_Rotate_Vector( move_vector );
	int sub_state = 0;
		  if ( move_vector[0] > MOVING_THRESHHOLD ) 	sub_state |= SUB_STATE_FORWARD;
	else if ( move_vector[0] < -MOVING_THRESHHOLD )	sub_state |= SUB_STATE_BACKWARD;
		  if ( move_vector[1] > MOVING_THRESHHOLD ) 	sub_state |= SUB_STATE_LEFT;
	else if ( move_vector[1] < -MOVING_THRESHHOLD )	sub_state |= SUB_STATE_RIGHT;
	Set_State( LAND, sub_state );

	// Whenever a jump ends, play a land surface effect
	Matrix3D tm = HumanPhys->Get_Transform();
	int ground_type = HumanPhys->Get_Contact_Surface_Type();
	SurfaceEffectsManager::Apply_Effect( ground_type, SurfaceEffectsManager::HITTER_TYPE_FOOTSTEP_LAND, tm );
}

