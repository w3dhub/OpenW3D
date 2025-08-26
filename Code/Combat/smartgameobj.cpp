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
 *                     $Archive:: /Commando/Code/Combat/smartgameobj.cpp                      $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 1/05/02 4:00p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 246                                                         $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


/*
**	Includes
*/
#include "smartgameobj.h"
#include "gameobjmanager.h"
#include "weapons.h"
#include "bittype.h"
#include "combat.h"
#include "movephys.h"
#include "damage.h"
#include "wwpacket.h"
#include "assets.h"
#include "debug.h"
#include "scripts.h"
#include "pscene.h"
#include "physcoltest.h"
#include "chunkio.h"
#include "weaponbag.h"
#include "matinfo.h"
#include "vehicle.h"
#include "ccamera.h"
#include "soldier.h"
#include "parameter.h"
#include "crandom.h"
#include "playertype.h"
#include "WWAudio.h"
#include "LogicalListener.h"
#include "combatsound.h"
#include "LogicalSound.h"
#include "wwprofile.h"
#include "playerdata.h"
#include "persistfactory.h"
#include "persistfactory.h"
#include "diaglog.h"
#include "clientcontrol.h"
#include "stealtheffect.h"
#include "hud.h"


const float STEALTH_FIRING_TIME = 5.0f;  // amount of time an object stays un-stealthed after firing


/*
** SmartGameObjDef
*/
SmartGameObjDef::SmartGameObjDef( void ) :
	SightRange( 0 ),
	SightArc( DEG_TO_RADF( 0 ) ),
	ListenerScale( 1 )
{
	EDITABLE_PARAM( SmartGameObjDef, ParameterClass::TYPE_FLOAT,	SightRange );
	EDITABLE_PARAM( SmartGameObjDef, ParameterClass::TYPE_ANGLE,	SightArc );
	EDITABLE_PARAM( SmartGameObjDef, ParameterClass::TYPE_FLOAT,	ListenerScale );
	EDITABLE_PARAM( SmartGameObjDef, ParameterClass::TYPE_BOOL,		IsStealthUnit );
}

enum	{
	XXX_CHUNKID_DEF_PHYSICALGAMEOBJ_PARENT				=	909991656,
	CHUNKID_DEF_VARIABLES,
	CHUNKID_DEF_ARMEDGAMEOBJ_PARENT,

	MICROCHUNKID_DEF_SIGHT_RANGE							=	9,
	MICROCHUNKID_DEF_SIGHT_ARC,
	MICROCHUNKID_DEF_LISTENER_SCALE						= 17,
	LEGACY_MICROCHUNKID_DEF_INFO_ICON_TEXTURE_FILENAME,
	MICROCHUNKID_DEF_IS_STEALTH_UNIT,
};

bool	SmartGameObjDef::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_DEF_ARMEDGAMEOBJ_PARENT );
		ArmedGameObjDef::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_DEF_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_SIGHT_RANGE,      SightRange );				
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_SIGHT_ARC,        SightArc );				
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_LISTENER_SCALE,   ListenerScale );				
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_IS_STEALTH_UNIT,  IsStealthUnit );				
	csave.End_Chunk();

	return true;
}

bool	SmartGameObjDef::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_DEF_ARMEDGAMEOBJ_PARENT:
				ArmedGameObjDef::Load( cload );
				break;
								
			case CHUNKID_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_SIGHT_RANGE,      SightRange );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_SIGHT_ARC,        SightArc );				
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_LISTENER_SCALE,   ListenerScale );				
						READ_MICRO_CHUNK_WWSTRING( cload, LEGACY_MICROCHUNKID_DEF_INFO_ICON_TEXTURE_FILENAME, InfoIconTextureFilename );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_IS_STEALTH_UNIT,  IsStealthUnit );				

						default:
							Debug_Say(( "Unrecognized SmartDef Variable chunkID %d\n", cload.Cur_Micro_Chunk_ID() ));
							break;

					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized SmartDef chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}


float		SmartGameObj::GlobalSightRangeScale = 1.0f;

/*
** SmartGameObj
*/
SmartGameObj::SmartGameObj( void ) :
	Action( this ),
   ControlOwner( SERVER_CONTROL_OWNER ),
   ControlEnabled( true ),
	IsEnemySeenEnabled( false ),
	MovingSoundTimer( 0 ),
	PlayerData( NULL ),
	StealthEnabled( false ),
	StealthPowerupTimer( 0.0f ),
	StealthFiringTimer( 0.0f ),
	StealthEffect( NULL )
{
	GameObjManager::Add_Smart( this );
	Listener = WWAudioClass::Get_Instance()->Create_Logical_Listener();
	SET_REF_OWNER( Listener );

	Listener->Register_Callback( AudioCallbackClass::EVENT_LOGICAL_HEARD, this );
}


SmartGameObj::~SmartGameObj( void )
{
	GameObjManager::Remove_Smart( this );
	Listener->Remove_From_Scene();
	Listener->Release_Ref();
	REF_PTR_RELEASE(StealthEffect);
}

/*
**
*/
void	SmartGameObj::Init( const SmartGameObjDef & definition )
{
	ArmedGameObj::Init( definition );
	Copy_Settings( definition );
	return ;
}

/*
**
*/
void	SmartGameObj::Copy_Settings( const SmartGameObjDef & definition )
{
	WWASSERT( Peek_Physical_Object() );
	MoveablePhysClass * moveable = Peek_Physical_Object()->As_MoveablePhysClass();
	if (moveable != NULL) {
		Peek_Physical_Object()->As_MoveablePhysClass()->Set_Controller( &Controller );
	}
	Register_Listener();

	if (definition.IsStealthUnit) {
		Enable_Stealth(true);
	}

	return ;
}

/*
**
*/
void	SmartGameObj::Re_Init( const SmartGameObjDef & definition )
{
	ArmedGameObj::Re_Init( definition );

	//
	//	Remove the listener from the scene
	//
	if ( Listener != NULL ) {
		Listener->Remove_From_Scene();
	}

	//
	//	Free the stealth effect as necessary
	//
	if (StealthEffect != NULL) {
		REF_PTR_RELEASE (StealthEffect);
		StealthEnabled			= false;
		StealthPowerupTimer	= 0.0F;
		StealthFiringTimer	= 0.0F;
	}

	//
	//	Copy any internal settings from the definition
	//
	Copy_Settings( definition );
	return ;
}

const SmartGameObjDef & SmartGameObj::Get_Definition( void ) const
{
	return (const SmartGameObjDef &)BaseGameObj::Get_Definition();
}


/*
** SmartGameObj Save and Load
*/

enum	{
	OLD_CHUNKID_PHYSICALGAMEOBJ_PARENT							=	910991113,
	CHUNKID_VARIABLES,
	CHUNKID_CONTROL,
	CHUNKID_CONTROLLER,
	CHUNKID_ACTION,
	XXXCHUNKID_WEAPONBAG,
	CHUNKID_ARMEDGAMEOBJ_PARENT,
	XXXCHUNKID_PLAYER_DATA,
	CHUNKID_STEALTH_EFFECT,

	MICROCHUNKID_CONTROL_ENABLED		=	1,
	XXXMICROCHUNKID_WEAPON_TILT,
	XXXMICROCHUNKID_WEAPON_TURN,
	MICROCHUNKID_CONTROL_OWNER,
	XXX_MICROCHUNKID_IS_GHOST,
	MICROCHUNKID_IMPORT_STATE_COUNT,
	MICROCHUNKID_TINT_COLOR,

	MICROCHUNKID_CONTROLLER_PTR,
	MICROCHUNKID_IS_ENEMY_SEEN_ENABLED,
	XXXMICROCHUNKID_TARGETING_POS,
	MICROCHUNKID_MOVING_SOUND_TIMER,
	MICROCHUNKID_PLAYER_DATA,

	MICROCHUNKID_STEALTH_ENABLED,
	MICROCHUNKID_STEALTH_POWERUP_TIMER,
	MICROCHUNKID_STEALTH_FIRING_TIMER,

};

bool	SmartGameObj::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_ARMEDGAMEOBJ_PARENT );
	ArmedGameObj::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );

		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_CONTROL_ENABLED, ControlEnabled );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_CONTROL_OWNER, ControlOwner );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_IS_ENEMY_SEEN_ENABLED, IsEnemySeenEnabled );
		void * ptr = &Controller;
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_CONTROLLER_PTR, ptr );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_MOVING_SOUND_TIMER, MovingSoundTimer ); 
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_PLAYER_DATA, PlayerData ); 

		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STEALTH_ENABLED, StealthEnabled ); 
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STEALTH_POWERUP_TIMER, StealthPowerupTimer ); 
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STEALTH_FIRING_TIMER, StealthFiringTimer ); 

	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_CONTROL );
	Control.Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_CONTROLLER );
	Controller.Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_ACTION );
	Action.Save( csave );
	csave.End_Chunk();

	if (StealthEffect != NULL) {
		csave.Begin_Chunk(CHUNKID_STEALTH_EFFECT);
		StealthEffect->Save(csave);
		csave.End_Chunk();
	}

//	Don't need to save Listener

	return true;
}

bool	SmartGameObj::Load( ChunkLoadClass &cload )
{
	WWASSERT( PlayerData == NULL );

	int new_control_owner = 0;

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case OLD_CHUNKID_PHYSICALGAMEOBJ_PARENT:
				Debug_Say(( "Loading old SmartGameObj format\n" ));
				PhysicalGameObj::Load( cload );
				break;
								
			case CHUNKID_ARMEDGAMEOBJ_PARENT:
				ArmedGameObj::Load( cload );
				break;
								
			case CHUNKID_VARIABLES:
				void * old_controller_ptr;
				old_controller_ptr = NULL;

				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_CONTROL_ENABLED, ControlEnabled );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_CONTROL_OWNER, new_control_owner );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_IS_ENEMY_SEEN_ENABLED, IsEnemySeenEnabled );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_CONTROLLER_PTR,        old_controller_ptr );				
						READ_MICRO_CHUNK( cload, MICROCHUNKID_MOVING_SOUND_TIMER, MovingSoundTimer ); 
						READ_MICRO_CHUNK( cload, MICROCHUNKID_PLAYER_DATA, PlayerData ); 

						READ_MICRO_CHUNK( cload, MICROCHUNKID_STEALTH_ENABLED, StealthEnabled ); 
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STEALTH_POWERUP_TIMER, StealthPowerupTimer ); 
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STEALTH_FIRING_TIMER, StealthFiringTimer ); 
					
					default:
							Debug_Say(( "Unrecognized SmartGameObj Variable chunkID\n" ));
							break;

					}
					cload.Close_Micro_Chunk();
				}

				if ( old_controller_ptr ) {
					SaveLoadSystemClass::Register_Pointer(old_controller_ptr, &Controller);
				}

				break;

			case CHUNKID_CONTROL:
				Control.Load( cload );
				break;
								
			case CHUNKID_CONTROLLER:
				Controller.Load( cload );
				break;
								
			case CHUNKID_ACTION:
				Action.Load( cload );
				break;
								
			case CHUNKID_STEALTH_EFFECT:
				Alloc_Stealth_Effect();
				StealthEffect->Load(cload);
				break;
			
			default:
				Debug_Say(( "Unrecognized SoldierGameObj chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	if ( PlayerData != NULL ) {
		REQUEST_POINTER_REMAP ((void **)&PlayerData);
	}

	Set_Control_Owner( new_control_owner );	// Be sure soldier virtual function calls

	SaveLoadSystemClass::Register_Post_Load_Callback(this);
	return true;
}


void SmartGameObj::On_Post_Load(void)
{
	ArmedGameObj::On_Post_Load();
	if ( StealthEffect != NULL ) {
		Peek_Physical_Object()->Add_Effect_To_Me(StealthEffect);
	}
	Register_Listener();

	if ( PlayerData ) {
		Set_Player_Data( PlayerData );	// make sure we re map each other
	}

}

void SmartGameObj::Set_Player_Data( PlayerDataClass * player_data )
{
	if ( PlayerData ) {
		PlayerData->Set_GameObj( NULL );
	}
	PlayerData = player_data;
	if ( PlayerData ) {
		PlayerData->Set_GameObj( this );
	}
}


//-----------------------------------------------------------------------------
void SmartGameObj::Import_Frequent(BitStreamClass & packet)
{
   WWASSERT(CombatManager::I_Am_Only_Client());
	
	/*TSS091301
	//
	//	Update the player data structure from the server
	//
	if (PlayerData != NULL) {
		PlayerData->Import_Frequent( packet );
	}
	*/

	//
	//	Import all data from the base classes
	//
	ArmedGameObj::Import_Frequent(packet);

   //
	//	Don't import the controller if the player is controlling
	// this object
	//
	if (Is_Controlled_By_Me()) {
      packet.Flush();
   } else {
		Import_Control_Sc(packet);
   }

   WWASSERT(packet.Is_Flushed());
	return ;
}


//-----------------------------------------------------------------------------
void SmartGameObj::Export_Frequent(BitStreamClass & packet)
{
	/*TSS091301
	//
	//	Send the player data to the client
	//
	if (PlayerData != NULL) {
		PlayerData->Export_Frequent( packet );
	}
	*/

	//
	//	Send data from the base class to the client
	//
	ArmedGameObj::Export_Frequent(packet);
	Export_Control_Sc(packet);
	return ;
}

//-----------------------------------------------------------------------------
void SmartGameObj::Export_State_Cs(BitStreamClass & packet)
{
	ArmedGameObj::Export_State_Cs(packet);
}

//-----------------------------------------------------------------------------
void SmartGameObj::Import_State_Cs(BitStreamClass & packet)
{
	ArmedGameObj::Import_State_Cs(packet);
}

//-----------------------------------------------------------------------------
void SmartGameObj::Generate_Control(void)
{
	if (CombatManager::I_Am_Server() && 
		 (ControlOwner == SERVER_CONTROL_OWNER || !Is_Human_Controlled())) {
		
		Action.Act(); 
	}

	if (CombatManager::I_Am_Client() && 
		 ControlOwner == CombatManager::Get_My_Id()) {
		
		Action.Act(); 
	
		// 
		// Notify server 
		//
		/*
		if (PClientControl != NULL && !Is_Delete_Pending()) {
			PClientControl->Set_Update_Flag(Get_ID());
		}
		*/

		//TSS092101
		if (PClientControl != NULL) {
			if (Is_Delete_Pending()) {
				PClientControl->Set_Update_Flag(-1);
			} else {
				PClientControl->Set_Update_Flag(Get_ID());
			}
		}
	}
}

//-----------------------------------------------------------------------------
bool SmartGameObj::Is_Control_Data_Dirty(cPacket & packet)
{	
	//
	// future optimization
	//

   return true;
}

bool SmartGameObj::Has_Player(void)
{
	// There is a cPlayer object for this smart object
   return (ControlOwner != SERVER_CONTROL_OWNER);
}

bool SmartGameObj::Is_Human_Controlled(void)
{
	// There is a human cPlayer object for this smart object
   //return ControlOwner >= 0 && !CombatManager::Player_Is_Afk(ControlOwner);
   return ControlOwner >= 0;
}

bool SmartGameObj::Is_Controlled_By_Me(void)
{
//	WWASSERT(CombatManager::I_Am_Client());
	if (!CombatManager::I_Am_Client()) {
		return false;
	}

	SmartGameObj *game_obj = this;

	//
	//	If this is a vehicle, then passthru to the driver
	//
	VehicleGameObj *vehicle = As_VehicleGameObj();
	if (vehicle != NULL) {
		SoldierGameObj *driver = vehicle->Get_Driver();
		if (driver != NULL) {
			game_obj = driver;
		}
	}
   
	return game_obj->Is_Human_Controlled() && (game_obj->ControlOwner == CombatManager::Get_My_Id());
}

void SmartGameObj::Apply_Control( void )
{
	bool switched = false;

	const char * change_type = NULL;

	if	( Control.Get_Boolean( ControlClass::BOOLEAN_WEAPON_NEXT ) ) {

		if ( COMBAT_CAMERA != NULL ) {
			COMBAT_CAMERA->Reset_First_Person_Offset_Tweak();
		}

		WeaponBag->Select_Next();
		switched = true;
		change_type = "Next";

	}

	if	( Control.Get_Boolean( ControlClass::BOOLEAN_WEAPON_PREV ) ) {

		if ( COMBAT_CAMERA != NULL ) {
			COMBAT_CAMERA->Reset_First_Person_Offset_Tweak();
		}

		WeaponBag->Select_Prev();
		switched = true;
		change_type = "Prev";
	}

	// -1 is no weapon
	for ( int weapon_key_num = -1; weapon_key_num <= 9; weapon_key_num++ ) {
		if	( Control.Get_Boolean( (ControlClass::BooleanControl)((int)ControlClass::BOOLEAN_SELECT_WEAPON_0 + weapon_key_num) ) ) {

			if ( this == COMBAT_STAR ) {
				HUDClass::Force_Weapon_Chart_Display();
			}

			if ( COMBAT_CAMERA != NULL ) {
				COMBAT_CAMERA->Reset_First_Person_Offset_Tweak();
			}

			WeaponBag->Select_Key_Number( weapon_key_num );	
			switched = true;
			change_type = "Key";

		}		
	}

	if ( this == COMBAT_STAR && change_type != NULL ) {
		Vector3	pos;
		Get_Position( &pos );
		const char * weapon_name = "";
		int ammo = 0;
		if ( Get_Weapon() ) {
			weapon_name = Get_Weapon()->Get_Definition()->Get_Name();
			ammo = Get_Weapon()->Get_Total_Rounds();
		}
		DIAG_LOG(( "WCHN", "%1.2f; %1.2f; %1.2f; %s; %d; %s", pos.X, pos.Y, pos.Z, weapon_name,  ammo, change_type ));
	}

	// This is kinda ugly....
	SoldierGameObj * p_soldier = As_SoldierGameObj();
	if (switched && p_soldier != NULL && p_soldier->Is_Sniping()) {
		// If sniping, and you cannot snipe for the current weapon, exit sniping
		if ( Get_Weapon() == NULL || !Get_Weapon()->Get_Can_Snipe() ) {
			Debug_Say(( "Force Sniper exit\n" ));
			p_soldier->Get_Human_State()->Toggle_State_Flag( HumanStateClass::SNIPING_FLAG );
		}
	}

	Controller.Reset();

	Controller.Set_Turn_Left(	Control.Get_Analog( ControlClass::ANALOG_TURN_LEFT )		);
	Controller.Set_Move_Up(		Control.Get_Analog( ControlClass::ANALOG_MOVE_UP )			);
	Controller.Set_Move_Forward(Control.Get_Analog( ControlClass::ANALOG_MOVE_FORWARD )	);
	Controller.Set_Move_Left(	Control.Get_Analog( ControlClass::ANALOG_MOVE_LEFT )		);

   if ( ControlEnabled ) {
		if ( ( Get_Weapon() != NULL ) && ( !Is_Delete_Pending() ) ) {

			Get_Weapon()->Set_Primary_Triggered( Control.Get_Boolean( ControlClass::BOOLEAN_WEAPON_FIRE_PRIMARY ) );
			Get_Weapon()->Set_Secondary_Triggered( Control.Get_Boolean( ControlClass::BOOLEAN_WEAPON_FIRE_SECONDARY ) );

			if ( Control.Get_Boolean( ControlClass::BOOLEAN_WEAPON_USE ) ) {
				Get_Weapon()->Next_C4_Detonation_Mode();
			}
			if ( Control.Get_Boolean( ControlClass::BOOLEAN_WEAPON_RELOAD ) ) {
				Get_Weapon()->Force_Reload();
			}

			// If we fire a weapon, we de-cloak for a certain amount of time
			if (	Control.Get_Boolean( ControlClass::BOOLEAN_WEAPON_FIRE_PRIMARY ) ||
					Control.Get_Boolean( ControlClass::BOOLEAN_WEAPON_FIRE_SECONDARY )) 
			{
				StealthFiringTimer = STEALTH_FIRING_TIME;
			}
		}
	} else {
		Get_Weapon()->Set_Primary_Triggered( false );
		Get_Weapon()->Set_Secondary_Triggered( false );
	}
}

void SmartGameObj::Think()
{
	{	
		WWPROFILE( "Smart Think" );

		// React to the controls
		{
			WWPROFILE("Controls");
			if ( ControlEnabled ) {
				Apply_Control();
			} else {
				Controller.Reset();
				if ( Get_Weapon() != NULL ) {
					Get_Weapon()->Set_Primary_Triggered( false );
					Get_Weapon()->Set_Secondary_Triggered( false );
				}
			}
		}

		MovingSoundTimer -= TimeManager::Get_Frame_Seconds();
		if ( MovingSoundTimer < 0 ) {
			WWPROFILE("See");
//			MovingSoundTimer += FreeRandom.Get_Float( 1, 2 );	// sound every 1-2 seconds
			MovingSoundTimer += FreeRandom.Get_Float( 0.5f, 1 );	// sound every 0.5 - 1 seconds

			// if I have sight, see who I see
			if ( Is_Enemy_Seen_Enabled() ) {
				// for all physicalgameobjs
				SLNode<BaseGameObj> *objnode;
				for (	objnode = GameObjManager::Get_Game_Obj_List()->Head(); objnode; objnode = objnode->Next()) {
					SmartGameObj *obj = objnode->Data()->As_SmartGameObj();
					if ( obj ) {
						if ( obj == this )	continue;
						if ( !Is_Enemy( obj ) ) continue;
						if ( !obj->Is_Visible() ) continue;
						// Don't see hidden models
						if ( obj != COMBAT_STAR && obj->Peek_Model() && obj->Peek_Model()->Is_Hidden() ) {
							continue;
						}
						if ( Is_Obj_Visible( obj ) ) {
							const GameObjObserverList & observer_list = Get_Observers();
							for( int index = 0; index < observer_list.Count(); index++ ) {
								observer_list[ index ]->Enemy_Seen( this, obj );
							}		
						}
					}
				}
			}
		}

		// Stealth logic
		if (StealthPowerupTimer > 0.0f) {
			StealthPowerupTimer -= TimeManager::Get_Frame_Seconds();
		}

		if (StealthFiringTimer > 0.0f) {
			StealthFiringTimer -= TimeManager::Get_Frame_Seconds();
		}

		if ( (StealthEffect != NULL ) && COMBAT_STAR) {
			StealthEffect->Set_Friendly( Is_Teammate( COMBAT_STAR ) );
		}

		if (((StealthPowerupTimer > 0.0f) || (StealthEnabled)) && (StealthFiringTimer <= 0.0f)) {
			WWPROFILE("Stealh");
			
			Alloc_Stealth_Effect();
			WWASSERT(StealthEffect != NULL);
			Peek_Physical_Object()->Add_Effect_To_Me(StealthEffect);

			StealthEffect->Enable_Stealth(true);

			const float STEALTH_BROKEN_FRACTION = 0.25f;
			DefenseObjectClass * defobj = Get_Defense_Object();
			if (defobj != NULL) {
				StealthEffect->Set_Broken((defobj->Get_Health() / defobj->Get_Health_Max()) < STEALTH_BROKEN_FRACTION);
			}
			
		} else {
			if (StealthEffect != NULL) {
				StealthEffect->Enable_Stealth(false);
			}
		}
	}
	{
	WWPROFILE("Embedded Armed think in smart think");
	ArmedGameObj::Think();
	}
}

void	SmartGameObj::Post_Think( void ) 
{
	ArmedGameObj::Post_Think();

	WWPROFILE( "Smart PostThink" );

	if ( Is_Delete_Pending() ) {				// don't update if destroying... (so we don't create a new laser!)
		return;
	}

	// Reset the one time booleans
	Control.Clear_One_Time_Boolean();
}


void	SmartGameObj::Apply_Damage(const OffenseObjectClass & damager, float scale, int alternate_skin)
{
	float damage = damager.Get_Damage() * scale;
	if ((damage > 0) && (StealthEffect != NULL)) {
		StealthEffect->Damage_Occured();
	}

	PhysicalGameObj::Apply_Damage(damager,scale,alternate_skin);
}

bool	SmartGameObj::Is_Obj_Visible( PhysicalGameObj *obj ) 
{
	Vector3 diff = obj->Get_Bullseye_Position();

	Matrix3D	look_tm = Get_Look_Transform();
	Matrix3D::Inverse_Transform_Vector( look_tm, diff, &diff );

	float dist = diff.Length();
	if ( dist < Get_Definition().SightRange * GlobalSightRangeScale) {
		// find view angle
		diff.Z = 0;
		diff.Normalize();
		float angle = WWMath::Fast_Acos( diff.X );

		if ( WWMath::Fabs( angle ) < Get_Definition().SightArc/2 ) {

			// if it passes this test, do a raycast to see if we see it.
			Vector3	me = look_tm.Get_Translation();
			Vector3	him = obj->Get_Bullseye_Position();

			Peek_Physical_Object()->Inc_Ignore_Counter();

			CastResultStruct res;
			LineSegClass ray( me, him );
			PhysRayCollisionTestClass raytest(ray, &res, BULLET_COLLISION_GROUP);
{ WWPROFILE( "Cast Ray" );
			PhysicsSceneClass::Get_Instance()->Cast_Ray(raytest);
}

			Peek_Physical_Object()->Dec_Ignore_Counter();

#if 0
			if (raytest.Result->StartBad) {
//				Debug_Say(( "Is_Vis Start Bad\n" ));
			} else if ( raytest.CollidedPhysObj == obj->Peek_Physical_Object() ) {
				return true;
	  		}
#else
			return ((raytest.Result->Fraction == 1.0f ) ||
					 ( raytest.CollidedPhysObj == obj->Peek_Physical_Object() ));
#endif
		}
	}
	return false;
}

/*
**
*/
void	SmartGameObj::On_Logical_Heard (LogicalListenerClass *listener, LogicalSoundClass *sound_obj) 
{
	CombatSound sound;
	sound.Type = (CombatSoundType)sound_obj->Get_Type_Mask();
	sound.Position = sound_obj->Get_Position();

	//
	//	Dig the sound creator out from the logical sound object
	//
	RefCountedGameObjReference *creator = (RefCountedGameObjReference *)sound_obj->Peek_User_Obj();
	if (creator != NULL) {
		sound.Creator = creator->Get_Ptr ();
	} else {
		sound.Creator = NULL;
	}

	// Notify observers
	if (CombatManager::Are_Observers_Active()) {
		const GameObjObserverList & observer_list = Get_Observers();
		for( int index = 0; index < observer_list.Count(); index++ ) {
			observer_list[ index ]->Sound_Heard( this, sound );
		}
	}
}


void SmartGameObj::Register_Listener(void)
{
	if (Listener != NULL) {
		const SmartGameObjDef& definition = Get_Definition();
		Listener->Set_Scale(definition.ListenerScale);
		Listener->Set_DropOff_Radius( 0.001f );
		Listener->Attach_To_Object(Peek_Model());
		Listener->Add_To_Scene();
	}
}

void	SmartGameObj::Begin_Hibernation( void )
{
	ArmedGameObj::Begin_Hibernation();

	// Reset our physics controller
	Controller.Reset();

	//
	//	Stop listening for sounds
	//
	if (Listener != NULL) {
		Listener->Remove_From_Scene ();
	}

	//
	//	Let the action prepare itself for hibernation
	//
	Action.Begin_Hibernation ();
	return ;
}

void	SmartGameObj::End_Hibernation( void )
{
	ArmedGameObj::End_Hibernation();

	//
	//	Make sure we are listening for logical sounds
	//
	if (Listener != NULL) {
		Listener->Add_To_Scene ();
	}

	//
	//	Let the action prepare itself for activation
	//
	Action.End_Hibernation ();
	return ;
}


//------------------------------------------------------------------------------------
void	SmartGameObj::Get_Information( StringClass & string )
{
	ArmedGameObj::Get_Information( string );

	StringClass temp;
	temp.Format( "Observer:%d\n", Action.Get_Parameters().ObserverID );
	string += temp;
	temp.Format( "Priority:%d\n", Action.Get_Parameters().Priority );
	string += temp;

	if ( Action.Get_Parameters().WaypathID ) {
		temp.Format( "Waypath:%d\n", Action.Get_Parameters().WaypathID );
		string += temp;
	}
}


/*
**
*/
void	SmartGameObj::Export_Creation( BitStreamClass &packet )
{
	ArmedGameObj::Export_Creation( packet );

	//
	//	Send the control owner to the client
	//
	int control_owner = Get_Control_Owner();
	packet.Add( control_owner );
	return ;
}


/*
**
*/
void	SmartGameObj::Import_Creation( BitStreamClass &packet )
{
	ArmedGameObj::Import_Creation( packet );

	//
	//	Get the control owner from the server
	//
	int control_owner = 0;
	packet.Get( control_owner );
	Set_Control_Owner( control_owner );

	//
	//	Hookup the control input for this player
	//
	if (control_owner  == CombatManager::Get_My_Id() ) {
		ActionParamsStruct parameters;
		Get_Action()->Follow_Input( parameters );
		CombatManager::Set_The_Star( As_SoldierGameObj() );
	}

	return ;
}


void SmartGameObj::Enable_Stealth(bool onoff)
{
	StealthEnabled = onoff;
	if (StealthEnabled) {
		Alloc_Stealth_Effect ();
	}

	return ;
}

void SmartGameObj::Toggle_Stealth(void)
{
	StealthEnabled = !StealthEnabled;
}

bool SmartGameObj::Is_Stealth_Enabled(void)
{
	return StealthEnabled;
}

bool SmartGameObj::Is_Stealthed(void) const
{
	if (StealthEffect != NULL) {
		return StealthEffect->Is_Stealthed();
	} else {
		return false;
	}
}

void SmartGameObj::Grant_Stealth_Powerup(float seconds)
{
	StealthPowerupTimer = seconds;
}

float SmartGameObj::Remaining_Stealth_Powerup_Time(void)
{
	return StealthPowerupTimer;
}

StealthEffectClass * SmartGameObj::Peek_Stealth_Effect(void)
{
	return StealthEffect;
}

void SmartGameObj::Alloc_Stealth_Effect(void)
{
	if (StealthEffect == NULL) {
		StealthEffect = NEW_REF(StealthEffectClass,());
		StealthEffect->Set_Fade_Distance( Get_Stealth_Fade_Distance() );
	}
}

void	SmartGameObj::Reset_Controller( void )
{
	Controller.Reset();
}

		/*
		CombatManager::Send_Control_Packet(this);
		CombatManager::Send_State_Packet(this);
		*/

