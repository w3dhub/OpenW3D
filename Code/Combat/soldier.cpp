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
 *                     $Archive:: /Commando/Code/Combat/soldier.cpp                           $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 3/29/02 4:58p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 570                                                         $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "soldier.h"
#include "debug.h"
#include "pscene.h"
#include "combat.h"
#include "weapons.h"
#include "damage.h"
#include "humanphys.h"
#include "animcontrol.h"
#include "rendobj.h"
#include "wwpacket.h"
#include "assets.h"
#include "gameobjmanager.h"
#include "vehicle.h"
#include "c4.h"
#include "objlibrary.h"
#include "physicalgameobj.h"
#include "persistfactory.h"
#include "combatchunkid.h"
#include "definition.h"
#include "simpledefinitionfactory.h"
#include "wwhack.h"
#include "surfaceeffects.h"
#include "weaponmanager.h"
#include "weaponbag.h"
#include "physcoltest.h"
#include "htree.h"
#include "animobj.h"
#include "WWAudio.h"
#include "soldierobserver.h"
#include "playertype.h"
#include "crandom.h"
#include "ccamera.h"
#include "bitpackids.h"
#include "translatedb.h"
#include "translateobj.h"
#include "hlod.h"
#include "AudibleSound.h"
#include "wwprofile.h"
#include "globalsettings.h"
#include "colors.h"
#include "input.h"
#include "gametype.h"
#include "messagewindow.h"
#include "conversationmgr.h"
#include "activeconversation.h"
#include "bones.h"
#include "dynamicspeechanim.h"
#include "hudinfo.h"
#include "powerup.h"
#include "diaglog.h"
#include "vistable.h"
#include "crandom.h"
#include "transitioneffect.h"
#include "combatmaterialeffectmanager.h"
#include "explosion.h"
#include "playerdata.h"
#include "encyclopediamgr.h"
#include "apppackettypes.h"
#include "hud.h"
#include "unitcoordinationzonemgr.h"
#include "specialbuilds.h"
#include "weaponview.h"
#include "ffactory.h"
#include "realcrc.h"
#include "colmathaabox.h" // Agressive inlining causes linker issues if this isn't here.
#include <algorithm>

/*
**
*/
#define		GUN_BONE_NAME				"GUNBONE"
#define		BACK_GUN_BONE_NAME		"BACKGUNBONE"

#define		POKE_ANGLE					DEG_TO_RAD( 22 )

#define		AMBUSH_DAMAGE_SCALE		1

const float EMOT_ICON_HEIGHT = 2.0F;

/*
** SoldierGameObjDef
*/
DECLARE_FORCE_LINK( Soldier )

SimplePersistFactoryClass<SoldierGameObjDef, CHUNKID_GAME_OBJECT_DEF_SOLDIER>	_SoldierGameObjDefPersistFactory;

DECLARE_DEFINITION_FACTORY(SoldierGameObjDef, CLASSID_GAME_OBJECT_DEF_SOLDIER, "Soldier") _SoldierGameObjDefDefFactory;

SoldierGameObjDef::SoldierGameObjDef( void ) :
	TurnRate( DEG_TO_RADF( 360.0f ) ),
	JumpVelocity( 2 ),
	SkeletonHeight( 0 ),
	SkeletonWidth( 0 ),
	UseInnateBehavior( true ),
	InnateAggressiveness( 0.5f ),
	InnateTakeCoverProbability( 0.5f ),
	InnateIsStationary( false ),
	HumanAnimOverrideDefID( 0 ),
	HumanLoiterCollectionDefID( 0 ),
	DeathSoundPresetID( 0 )
{
	EDITABLE_PARAM( SoldierGameObjDef, ParameterClass::TYPE_ANGLE, TurnRate );
	EDITABLE_PARAM( SoldierGameObjDef, ParameterClass::TYPE_FLOAT, JumpVelocity );
	EDITABLE_PARAM( SoldierGameObjDef, ParameterClass::TYPE_FLOAT, SkeletonHeight );
	EDITABLE_PARAM( SoldierGameObjDef, ParameterClass::TYPE_FLOAT, SkeletonWidth );
	EDITABLE_PARAM( SoldierGameObjDef, ParameterClass::TYPE_BOOL, UseInnateBehavior );
	EDITABLE_PARAM( SoldierGameObjDef, ParameterClass::TYPE_FLOAT, InnateAggressiveness );
	EDITABLE_PARAM( SoldierGameObjDef, ParameterClass::TYPE_FLOAT, InnateTakeCoverProbability );
	EDITABLE_PARAM( SoldierGameObjDef, ParameterClass::TYPE_BOOL, InnateIsStationary );
	EDITABLE_PARAM( SoldierGameObjDef, ParameterClass::TYPE_FILENAME,	FirstPersonHands );
	GENERIC_DEFID_PARAM( SoldierGameObjDef, HumanAnimOverrideDefID, CLASSID_GLOBAL_SETTINGS_DEF_HUMAN_ANIM_OVERRIDE );
	GENERIC_DEFID_PARAM( SoldierGameObjDef, HumanLoiterCollectionDefID, CLASSID_GLOBAL_SETTINGS_DEF_HUMAN_LOITER );
	GENERIC_DEFID_PARAM( SoldierGameObjDef, DeathSoundPresetID, CLASSID_SOUND );	

	MODEL_DEF_PARAM( SoldierGameObjDef, PhysDefID, "HumanPhysDef" );

	//
	//	We want soldiers to use innate conversations by default
	//
	AllowInnateConversations = true;
	return ;
}

uint32	SoldierGameObjDef::Get_Class_ID (void) const	
{ 
	return CLASSID_GAME_OBJECT_DEF_SOLDIER; 
}

PersistClass *	SoldierGameObjDef::Create( void ) const 
{
	SoldierGameObj * obj = new SoldierGameObj;
	obj->Init( *this );
	return obj;
}

enum	{
	CHUNKID_DEF_PARENT							=	909991656,
	CHUNKID_DEF_VARIABLES,
	CHUNKID_DEF_DIALOG_ENTRY,

	MICROCHUNKID_DEF_TURN_RATE					=	1,
	MICROCHUNKID_DEF_JUMP_VELOCITY,
	MICROCHUNKID_DEF_SKELETON_HEIGHT,
	MICROCHUNKID_DEF_SKELETON_WIDTH,
	MICROCHUNKID_DEF_USE_INNATE_BEHAVIOR,
	MICROCHUNKID_DEF_INNATE_AGGRESSIVENESS, 
	MICROCHUNKID_DEF_INNATE_TAKE_COVER_PROB, 
	XXXMICROCHUNKID_DEF_INNATE_ESCORT_ID, 
	XXXMICROCHUNKID_DEF_INNATE_ESCORT_RANGE, 
	MICROCHUNKID_DEF_FIRST_PERSON_HANDS, 
	XXXMICROCHUNKID_DEF_CORPSE_PERSIST_TIME,
	MICROCHUNKID_DEF_USE_INNATE_CONVERSATIONS,
	MICROCHUNKID_DEF_INNATE_IS_STATIONARY,
	MICROCHUNKID_DEF_ORATOR_TYPE,
	MICROCHUNKID_DEF_HUMAN_ANIM_OVERRIDE_DEF_ID,
	MICROCHUNKID_DEF_DEATH_SOUND_PRESET,
	MICROCHUNKID_DEF_HUMAN_LOITER_COLLECTION_DEF_ID,
};

bool	SoldierGameObjDef::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_DEF_PARENT );
		SmartGameObjDef::Save( csave );
	csave.End_Chunk();

	//
	//	Save the dialog entries
	//
	for (int index = 0; index < DIALOG_MAX; index ++) {
		csave.Begin_Chunk( CHUNKID_DEF_DIALOG_ENTRY );
			DialogList[index].Save (csave);
		csave.End_Chunk();	
	}	

	csave.Begin_Chunk( CHUNKID_DEF_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_TURN_RATE, TurnRate );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_JUMP_VELOCITY, JumpVelocity );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_SKELETON_HEIGHT, SkeletonHeight );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_SKELETON_WIDTH, SkeletonWidth );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_USE_INNATE_BEHAVIOR, UseInnateBehavior );
		//WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_USE_INNATE_CONVERSATIONS, UseInnateConversations );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_INNATE_AGGRESSIVENESS, InnateAggressiveness );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_INNATE_TAKE_COVER_PROB, InnateTakeCoverProbability );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_INNATE_IS_STATIONARY, InnateIsStationary );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_FIRST_PERSON_HANDS, FirstPersonHands );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_HUMAN_ANIM_OVERRIDE_DEF_ID, HumanAnimOverrideDefID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_HUMAN_LOITER_COLLECTION_DEF_ID, HumanLoiterCollectionDefID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_DEATH_SOUND_PRESET, DeathSoundPresetID );		
		
	csave.End_Chunk();

	return true;
}

bool	SoldierGameObjDef::Load( ChunkLoadClass &cload )
{
	int dialog_index = 0;

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_DEF_PARENT:
				SmartGameObjDef::Load( cload );
				break;

			case CHUNKID_DEF_DIALOG_ENTRY:
				if (dialog_index < DIALOG_MAX) {
					DialogList[dialog_index++].Load (cload);
				}
				break;
								
			case CHUNKID_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_TURN_RATE, TurnRate );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_JUMP_VELOCITY, JumpVelocity );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_SKELETON_HEIGHT, SkeletonHeight );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_SKELETON_WIDTH, SkeletonWidth );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_USE_INNATE_BEHAVIOR, UseInnateBehavior );
						//READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_USE_INNATE_CONVERSATIONS, UseInnateConversations );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_INNATE_AGGRESSIVENESS, InnateAggressiveness );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_INNATE_TAKE_COVER_PROB, InnateTakeCoverProbability );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_INNATE_IS_STATIONARY, InnateIsStationary );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_DEF_FIRST_PERSON_HANDS, FirstPersonHands );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_ORATOR_TYPE, OratorType );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_HUMAN_ANIM_OVERRIDE_DEF_ID, HumanAnimOverrideDefID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_HUMAN_LOITER_COLLECTION_DEF_ID, HumanLoiterCollectionDefID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_DEATH_SOUND_PRESET, DeathSoundPresetID );		

						default:
							//Debug_Say(( "Unrecognized SoldierDef Variable chunkID\n" ));
							break;

					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized SoldierDef chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

const PersistFactoryClass & SoldierGameObjDef::Get_Factory (void) const 
{ 
	return _SoldierGameObjDefPersistFactory; 
}


/*
** SolderGameObj
*/
SimplePersistFactoryClass<SoldierGameObj, CHUNKID_GAME_OBJECT_SOLDIER>	_SoldierGameObjPersistFactory;

const PersistFactoryClass & SoldierGameObj::Get_Factory (void) const 
{
	return _SoldierGameObjPersistFactory;
}

//------------------------------------------------------------------------------------
bool	SoldierGameObj::DisplayDebugBoxForGhostCollision = false;

//------------------------------------------------------------------------------------
SoldierGameObj::SoldierGameObj() :
	WeaponRenderModel( NULL ),
	BackWeaponRenderModel( NULL ),
	BackFlagRenderModel( NULL ),
	WeaponAnimControl( NULL ),
	TransitionCompletionData( NULL ),
	Vehicle( NULL ),
	LegFacing( 0 ),
	SyncLegs( false ),
	LastLegMode( 0 ),
	HeadLookDuration( 0 ),
	HeadRotation( 0,0,0 ),
	HeadLookTarget( 0,0,0 ),
	HeadLookAngle( 0,0,0 ),
	HeadLookAngleTimer( 0 ),
	InnateEnableBits( 0xFFFFFFFF ),
	InnateObserver( NULL ),
//	FlameTimer( 0 ),
	SpecialDamageMode(ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_NONE),
	SpecialDamageTimer( 0 ),
	GenerateIdleFacialAnimTimer( 0 ),
	KeyRing( 0 ),	
	InFlyMode( false ),
	IsVisible( true ),
	CurrentSpeech( NULL ),
	AIState( AI_STATE_IDLE ),
	SpeechAnim( NULL ),
	HeadModel( NULL ),
	EmotIconModel( NULL ),
	EmotIconTimer( 0 ),
	LadderUpMask( false ),
	LadderDownMask( false ),
	SpecialDamageEffect( NULL ),
	HealingEffect( NULL ),
	ReloadingTilt(0),
	WaterWake(NULL),
	WeaponChanged( false )
{
	// All Humans need a HuamnAnimControl
	Set_Anim_Control( new HumanAnimControlClass );
	Set_App_Packet_Type(APPPACKETTYPE_SOLDIER);

	// create a water wake object
	WaterWake = SurfaceEffectsManager::Create_Persistant_Emitter();	
}

//------------------------------------------------------------------------------------
SoldierGameObj::~SoldierGameObj()
{
	if ( HealingEffect != NULL ) {
		Peek_Human_Phys()->Remove_Effect_From_Me( HealingEffect );
		REF_PTR_RELEASE( HealingEffect );
	}

	Set_Special_Damage_Mode( ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_NONE );
	Set_Emot_Icon (NULL, 0);

	REF_PTR_RELEASE( HeadModel );
	REF_PTR_RELEASE( SpeechAnim );
	REF_PTR_RELEASE( CurrentSpeech );

	if (CombatManager::I_Am_Server()) {
		CombatManager::On_Soldier_Death(this);
	}

	Set_Weapon_Model( NULL );

	if ( BackWeaponRenderModel != NULL ) {
		Peek_Model()->Remove_Sub_Object(BackWeaponRenderModel);
		BackWeaponRenderModel->Release_Ref();
		BackWeaponRenderModel = NULL;
	}

	if ( BackFlagRenderModel != NULL ) {
		Peek_Model()->Remove_Sub_Object(BackFlagRenderModel);
		BackFlagRenderModel->Release_Ref();
		BackFlagRenderModel = NULL;
	}

	if ( WeaponAnimControl != NULL ) {
		delete WeaponAnimControl;
		WeaponAnimControl = NULL;
	}

	if (Vehicle != NULL) {
      // Remove myself from the vehicle !
		Vehicle->Remove_Occupant(this);
	}	

	COMBAT_SCENE->Remove_Object( Peek_Physical_Object() );

	if ( Is_Human_Controlled() ) {
		GameObjManager::Remove_Star( this );
	}

	Reset_RenderObjs();

	if (WaterWake != NULL) {
		SurfaceEffectsManager::Destroy_Persistant_Emitter( WaterWake );
		WaterWake = NULL;
	}

	return ;
}

//------------------------------------------------------------------------------------
void	SoldierGameObj::Init( void )
{
	Re_Init( Get_Definition() );
	return ;
}

void	SoldierGameObj::Init( const SoldierGameObjDef & definition )
{
	SmartGameObj::Init( definition );
	Copy_Settings( definition );
	return ;
}

void	SoldierGameObj::Copy_Settings( const SoldierGameObjDef & definition )
{
	HumanState.Init( Peek_Human_Phys() );
	HumanState.Set_Anim_Control( (HumanAnimControlClass *)Get_Anim_Control() );  // Must set the anim control after the phys object
//HumanState.Set_Human_Anim_Override( "HAO Test" );
	if ( Get_Definition().HumanAnimOverrideDefID != 0 ) {
		HumanState.Set_Human_Anim_Override( Get_Definition().HumanAnimOverrideDefID );
	}

	if ( Get_Definition().HumanLoiterCollectionDefID != 0 ) {
		HumanState.Set_Human_Loiter_Collection( Get_Definition().HumanLoiterCollectionDefID );
	}

	Adjust_Skeleton( definition.SkeletonHeight, definition.SkeletonWidth );

	// All characters force their heads and hands to use the same LOD level as their body.
	RenderObjClass * model = Peek_Human_Phys()->Peek_Model();
	if (model != NULL) {
		model->Set_Sub_Objects_Match_LOD(true);
	}

	if (	InnateObserver == NULL && 
			Get_Definition().UseInnateBehavior && 
			Is_Controlled_By_Me () == false ) {
		InnateObserver = new SoldierObserverClass;
		Insert_Observer( InnateObserver );
	}

	//
	//	Copy the dialog entries from the definition
	//
	for( int index = 0; index < DIALOG_MAX; index ++ ) {
		DialogList[index] = definition.DialogList[index];
	}

	//
	//	Put the soldier in its own collision group 
	//
	Peek_Physical_Object ()->Set_Collision_Group( SOLDIER_COLLISION_GROUP );

	Prepare_Speech_Framework ();
	return ;
}


void	SoldierGameObj::Prepare_Speech_Framework( void )
{
	//
	//	Make sure we have an allocated animation to use for speech
	//
	if ( SpeechAnim == NULL) {		
		HeadModel = Find_Head_Model();

		//
		//	Dig out the skeleton name for the head model
		//
		if (HeadModel != NULL) {
			const HTreeClass *htree = HeadModel->Get_HTree ();
			if (htree != NULL) {
				StringClass skeleton_name = htree->Get_Name ();

				//
				//	Generate the animation
				//
				SpeechAnim = new DynamicSpeechAnimClass (skeleton_name);
				GenerateIdleFacialAnimTimer = 0;
			}
		}
	}

	return ;
}


void	SoldierGameObj::Re_Init( const SoldierGameObjDef & definition )
{
	if ( this == COMBAT_STAR ) {
		HUDClass::Force_Weapon_Chart_Update();
		WeaponViewClass::Reset();
	}

	//
	//	Remove the object from the world (just to be safe)
	//
	COMBAT_SCENE->Remove_Object( Peek_Physical_Object() );

	//
	//	Reset the weapon model
	//
	Set_Weapon_Model( NULL );

	if ( BackWeaponRenderModel != NULL ) {
		Peek_Model()->Remove_Sub_Object(BackWeaponRenderModel);
		BackWeaponRenderModel->Release_Ref();
		BackWeaponRenderModel = NULL;
	}

	if ( BackFlagRenderModel != NULL ) {
		Peek_Model()->Remove_Sub_Object(BackFlagRenderModel);
		BackFlagRenderModel->Release_Ref();
		BackFlagRenderModel = NULL;
	}

	if ( WeaponAnimControl != NULL ) {
		delete WeaponAnimControl;
		WeaponAnimControl = NULL;
	}

	//
	//	Re-initialize the base class
	//
	SmartGameObj::Re_Init( definition );

	//
	//	Free some of the data we will be re-initializing
	//
	REF_PTR_RELEASE( HeadModel );
	REF_PTR_RELEASE( SpeechAnim );
	REF_PTR_RELEASE( CurrentSpeech );	

	HumanState.Reset();

	//
	//	Copy any internal settings from the definition
	//
	Copy_Settings( definition );

	//
	//	"Dirty" the object for networking
	//
	Set_Object_Dirty_Bit( NetworkObjectClass::BIT_RARE, true );

	// When class changes, update for new weapon
	if ( this == COMBAT_STAR ) {
		HUDClass::Reset();
	}

	return ;
}


//------------------------------------------------------------------------------------
const SoldierGameObjDef & SoldierGameObj::Get_Definition( void ) const
{
	return (const SoldierGameObjDef &)BaseGameObj::Get_Definition();
}

void SoldierGameObj::Set_Control_Owner(int control_owner)		
{ 
	if ( Is_Human_Controlled() ) {
		GameObjManager::Remove_Star( this );
	}
	SmartGameObj::Set_Control_Owner(control_owner);
	if ( Is_Human_Controlled() ) {
		GameObjManager::Add_Star( this );
	}
}



/*
** Soldier Save and Load
*/

enum	{
	CHUNKID_PARENT							=	909991656,
	CHUNKID_VARIABLES,
	CHUNKID_WEAPON_ANIM,
	CHUNKID_HUMAN_STATE,
	XXXCHUNKID_C4_TIMER,
	XXXCHUNKID_WEAPON_MODEL,
	CHUNKID_TRANSITION_COMPLETION_DATA,
	CHUNKID_DIALOG_ENTRY,
	CHUNKID_RENDER_OBJS,
	CHUNKID_SPECIAL_DAMAGE_DAMAGER, 

	MICROCHUNKID_DETONATE_C4			=	1,
	MICROCHUNKID_LEG_FACING,
	MICROCHUNKID_SYNC_LEGS,
	MICROCHUNKID_ANIMATION_NAME,
	MICROCHUNKID_VEHICLE,
	XXX_MICROCHUNKID_TRANSITION,
	XXX_MICROCHUNKID_FORCE_FACING,
	MICROCHUNKID_INNATE_ENABLE_BITS,
	MICROCHUNKID_INNATE_OBSERVER_PTR,
	MICROCHUNKID_LAST_LEG_MODE,
	MICROCHUNKID_HEAD_LOOK_DURATION,
	MICROCHUNKID_HEAD_ROTATION,
	MICROCHUNKID_LOOK_TARGET,
	XXXMICROCHUNKID_FLAME_TIMER,
	MICROCHUNKID_KEY_RING,
	MICROCHUNKID_AI_STATE,
	XXX_MICROCHUNKID_IN_CONVERSATION,
	MICROCHUNKID_LOOK_ANGLE,
	MICROCHUNKID_LOOK_ANGLE_TIMER,
	XXX_MICROCHUNKID_ACTIVE_CONVERSATION,
	MICROCHUNKID_WEAPON_MODEL,
	MICROCHUNKID_SPECIAL_DAMAGE_MODE, 
	MICROCHUNKID_SPECIAL_DAMAGE_TIMER,
	MICROCHUNKID_IS_USING_GHOST_COLLISION
};

//------------------------------------------------------------------------------------
bool	SoldierGameObj::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_PARENT );
	SmartGameObj::Save( csave );
	csave.End_Chunk();

	//
	//	Save the dialog entries
	//
	for (int index = 0; index < DIALOG_MAX; index ++) {
		csave.Begin_Chunk( CHUNKID_DIALOG_ENTRY );
			DialogList[index].Save (csave);
		csave.End_Chunk();	
	}	

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DETONATE_C4, DetonateC4 );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_LEG_FACING, LegFacing );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_SYNC_LEGS, SyncLegs );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_KEY_RING, KeyRing );		
//		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_FORCE_FACING, ForceFacing );
		if ( Vehicle != NULL ) {
			WRITE_MICRO_CHUNK_PTR( csave, MICROCHUNKID_VEHICLE, Vehicle );
		}
		csave.Begin_Micro_Chunk( MICROCHUNKID_ANIMATION_NAME );
		char anim_string[80];
		strcpy( anim_string, AnimationName );
		csave.Write( anim_string, strlen( anim_string ) + 1);
		csave.End_Micro_Chunk();
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_INNATE_ENABLE_BITS, InnateEnableBits );
		WRITE_MICRO_CHUNK_PTR( csave, MICROCHUNKID_INNATE_OBSERVER_PTR, InnateObserver );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_LAST_LEG_MODE, LastLegMode );		
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_HEAD_LOOK_DURATION, HeadLookDuration );		
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_HEAD_ROTATION, HeadRotation );		
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_LOOK_TARGET, HeadLookTarget );		
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_LOOK_ANGLE, HeadLookAngle );		
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_LOOK_ANGLE_TIMER, HeadLookAngleTimer );		
//		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_FLAME_TIMER, FlameTimer );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_SPECIAL_DAMAGE_MODE, SpecialDamageMode );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_SPECIAL_DAMAGE_TIMER, SpecialDamageTimer );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_AI_STATE, AIState );
		WRITE_MICRO_CHUNK_PTR( csave, MICROCHUNKID_WEAPON_MODEL, WeaponRenderModel );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_IS_USING_GHOST_COLLISION, IsUsingGhostCollision );
				
	csave.End_Chunk();

	if ( WeaponAnimControl ) {
		csave.Begin_Chunk( CHUNKID_WEAPON_ANIM );
		WeaponAnimControl->Save( csave );
		csave.End_Chunk();
	}

	csave.Begin_Chunk( CHUNKID_HUMAN_STATE );
	HumanState.Save( csave );
	csave.End_Chunk();

	if ( TransitionCompletionData ) {
		csave.Begin_Chunk( CHUNKID_TRANSITION_COMPLETION_DATA );
		TransitionCompletionData->Save( csave );
		csave.End_Chunk();
	}

	for ( int i = 0; i < RenderObjList.Count(); i++ ) {
		csave.Begin_Chunk( CHUNKID_RENDER_OBJS );
			csave.Begin_Chunk(RenderObjList[i]->Get_Factory().Chunk_ID());
				RenderObjList[i]->Get_Factory().Save( csave, RenderObjList[i] );
			csave.End_Chunk();
		csave.End_Chunk();
	}

	if ( SpecialDamageDamager.Get_Ptr() != NULL ) {
		csave.Begin_Chunk( CHUNKID_SPECIAL_DAMAGE_DAMAGER );
		SpecialDamageDamager.Save( csave );
		csave.End_Chunk();
	}

	return true;
}

//------------------------------------------------------------------------------------
bool	SoldierGameObj::Load( ChunkLoadClass &cload )
{
	char anim_string[80];
	int dialog_index = 0;

	WWASSERT( Vehicle == NULL );

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PARENT:
				SmartGameObj::Load( cload );
				break;

			case CHUNKID_DIALOG_ENTRY:
				if (dialog_index < DIALOG_MAX) {
					DialogList[dialog_index++].Load (cload);
				}
				break;
								
			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DETONATE_C4, DetonateC4 );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_LEG_FACING, LegFacing );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_SYNC_LEGS, SyncLegs );
						READ_MICRO_CHUNK_PTR( cload, MICROCHUNKID_VEHICLE, Vehicle );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_KEY_RING, KeyRing );
//						READ_MICRO_CHUNK( cload, MICROCHUNKID_FORCE_FACING, ForceFacing );

						case MICROCHUNKID_ANIMATION_NAME:
							cload.Read( anim_string, cload.Cur_Micro_Chunk_Length() );
							AnimationName = anim_string;
							break;

						case MICROCHUNKID_INNATE_OBSERVER_PTR:
							cload.Read( &InnateObserver, sizeof (InnateObserver) );
							if ( InnateObserver != NULL ) {
								REQUEST_POINTER_REMAP( (void **)&InnateObserver );
							}
							break;

						READ_MICRO_CHUNK( cload, MICROCHUNKID_INNATE_ENABLE_BITS, InnateEnableBits );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_LAST_LEG_MODE, LastLegMode );		
						READ_MICRO_CHUNK( cload, MICROCHUNKID_HEAD_LOOK_DURATION, HeadLookDuration );		
						READ_MICRO_CHUNK( cload, MICROCHUNKID_HEAD_ROTATION, HeadRotation );		
						READ_MICRO_CHUNK( cload, MICROCHUNKID_LOOK_TARGET, HeadLookTarget );		
						READ_MICRO_CHUNK( cload, MICROCHUNKID_LOOK_ANGLE, HeadLookAngle );		
						READ_MICRO_CHUNK( cload, MICROCHUNKID_LOOK_ANGLE_TIMER, HeadLookAngleTimer );		
//						READ_MICRO_CHUNK( cload, MICROCHUNKID_FLAME_TIMER, FlameTimer );		
						READ_MICRO_CHUNK( cload, MICROCHUNKID_SPECIAL_DAMAGE_MODE, SpecialDamageMode );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_SPECIAL_DAMAGE_TIMER, SpecialDamageTimer );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_AI_STATE, AIState );
						READ_MICRO_CHUNK_PTR( cload, MICROCHUNKID_WEAPON_MODEL, WeaponRenderModel );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_IS_USING_GHOST_COLLISION, IsUsingGhostCollision );

						default:
							Debug_Say(( "Unrecognized Soldier Variable chunkID\n" ));
							break;

					}
					cload.Close_Micro_Chunk();
				}
				if ( Vehicle != NULL ) {
					REQUEST_POINTER_REMAP( (void **)&Vehicle );
				}
				break;

			case CHUNKID_WEAPON_ANIM:
				Set_Weapon_Animation( NULL );	// Get the anim control built
				WeaponAnimControl->Load( cload );
				break;

			case CHUNKID_HUMAN_STATE:
				HumanState.Load( cload );
				break;

			case CHUNKID_RENDER_OBJS:
				cload.Open_Chunk();
				PersistFactoryClass * factory;
				factory = SaveLoadSystemClass::Find_Persist_Factory(cload.Cur_Chunk_ID());
				WWASSERT(factory != NULL);
				if (factory != NULL) {
					RenderObjClass * robj = (RenderObjClass *)factory->Load(cload);
					Add_RenderObj( robj );
					robj->Release_Ref();
				}
				cload.Close_Chunk();
				break;

			case CHUNKID_TRANSITION_COMPLETION_DATA:
				WWASSERT( TransitionCompletionData == NULL );
				TransitionCompletionData = new TransitionCompletionDataStruct();
				TransitionCompletionData->Load( cload );
				break;

			case CHUNKID_SPECIAL_DAMAGE_DAMAGER:
				SpecialDamageDamager.Load( cload );
				break;

			default:
				Debug_Say(( "Unrecognized SoldierGameObj chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	if ( WeaponRenderModel != NULL ) {
		REQUEST_REF_COUNTED_POINTER_REMAP( (RefCountClass **)&WeaponRenderModel );
	}


	SaveLoadSystemClass::Register_Post_Load_Callback(this);

	return true;
}

//------------------------------------------------------------------------------------
void	SoldierGameObj::On_Post_Load( void )
{
	HumanState.Set_Anim_Control( (HumanAnimControlClass *)Get_Anim_Control() );  // Must set the anim control after the phys object

	SmartGameObj::On_Post_Load();

	if ( Peek_Model() && (WeaponRenderModel != NULL) ) {
		Peek_Model()->Add_Sub_Object_To_Bone( WeaponRenderModel, GUN_BONE_NAME );
	}

	Adjust_Skeleton( Get_Definition().SkeletonHeight, Get_Definition().SkeletonWidth );

	// Fixup BackWeaponRenderModel
	Update_Back_Gun();

	// Hide if in vehicle
	if ( Is_In_Vehicle() && Peek_Model() != NULL ) {
		Peek_Model()->Set_Hidden( true );
	}

	//
	//	Make sure we have an allocated animation to use for speech
	//
	Prepare_Speech_Framework ();
	return ;
}

//-----------------------------------------------------------------------------

CollisionReactionType SoldierGameObj::Collision_Occurred(const CollisionEventClass & event)
{
//	Debug_Say(( "Soldier Collision %p %p\n", this, event.OtherObj ));

	// Detect squishing...
	if ( event.OtherObj != NULL && event.OtherObj->Get_Observer() != NULL ) {
		PhysicalGameObj * obj = ((CombatPhysObserverClass *)event.OtherObj->Get_Observer())->As_PhysicalGameObj();
		if ( obj && obj->As_VehicleGameObj() ) {
//			if ( !Is_Teammate( obj ) ) {		// never squish teammates
			if ( Is_Enemy( obj ) ) {		// only squish enemies
				Vector3 vel;
				if ( event.OtherObj->As_MoveablePhysClass() != NULL ) {
					event.OtherObj->As_MoveablePhysClass()->Get_Velocity( &vel );

					Vector3 my_pos;
					Vector3 vehicle_pos;
					Get_Position(&my_pos);
					obj->Get_Position(&vehicle_pos);

					// only squish if velocity is high enough, and velocity is towards the soldier
					if (	(vel.Length() > obj->As_VehicleGameObj()->Get_Squish_Velocity() ) &&
							(Vector3::Dot_Product(vel,my_pos - vehicle_pos) > 0.0f) )
					{	
						if ( HumanState.Get_State() != HumanStateClass::DEATH ) {
							// We need to pick a better warhead to damage with
							SmartGameObj * damager = obj->As_VehicleGameObj()->Get_Driver();
							if ( damager == NULL ) {
								damager = obj->As_VehicleGameObj();
							}
							OffenseObjectClass offense( 10000, 1, damager );
							Apply_Damage_Extended( offense, 10000.0f, vel, NULL ); 
							// Play the squish sound

							// Stats
							if (( obj->As_VehicleGameObj()->Get_Driver() != NULL ) && 
								( obj->As_VehicleGameObj()->Get_Driver()->Get_Player_Data() != NULL ) ) {
								obj->As_VehicleGameObj()->Get_Driver()->Get_Player_Data()->Stats_Add_Squish();
							}
						}
					}
				}
			}
		}
	}
	return COLLISION_REACTION_DEFAULT;
}

/*
**
*/
void	SoldierGameObj::Export_Creation( BitStreamClass &packet )
{
	//TSS091001
	//WWDEBUG_SAY((">>>>> SoldierGameObj::Export_Creation id %d\n", Get_ID()));

	SmartGameObj::Export_Creation( packet );
	return ;
}


/*
**
*/
void	SoldierGameObj::Import_Creation( BitStreamClass &packet )
{
	SmartGameObj::Import_Creation( packet );

	//TSS091001
	//WWDEBUG_SAY((">>>>> SoldierGameObj::Import_Creation id %d\n", Get_ID()));

	/*
	//
	//	Setup the speed settings for this soldier
	//
	float speed_factor = CombatManager::Get_Max_Speed_Pc( this ) / 100.0f;
	Set_Max_Speed( speed_factor * Get_Max_Speed() );
	*/
	return ;
}


/*
**
*/
void	SoldierGameObj::Export_Rare( BitStreamClass &packet )
{
	SmartGameObj::Export_Rare( packet );

	//
	//	Add our definition ID to the packet
	//
	uint32 definition_id = Get_Definition ().Get_ID ();
	packet.Add( definition_id );
}


/*
**
*/
void	SoldierGameObj::Import_Rare( BitStreamClass &packet )
{
	Set_Weapon_Model (NULL);
	// If we are changing to the same def, the above line made us lose our weapon model, and we won't get it back.
	// The next line makes us get it back.
 	Get_Weapon_Bag()->Force_Changed();

	SmartGameObj::Import_Rare( packet );

	//
	//	Read the definition ID from the packet
	//
	uint32 definition_id	= 0;
	packet.Get( definition_id );

	//
	//	Did our definition change?
	//
	if (definition_id != Get_Definition ().Get_ID ()) {
		DefinitionClass *definition = DefinitionMgrClass::Find_Definition (definition_id);

		//
		//	Did we find the right definition?
		//		
		if (definition != NULL && definition->Get_Class_ID() == CLASSID_GAME_OBJECT_DEF_SOLDIER) {
			SoldierGameObjDef *soldier_def = reinterpret_cast<SoldierGameObjDef *> (definition);
			
			//
			//	Re-initialize ourselves
			//
			Re_Init (*soldier_def);
		}		
	}

	return ;
}

/*
**
*/
void	SoldierGameObj::Export_Occasional( BitStreamClass &packet )
{
	SmartGameObj::Export_Occasional( packet );

	//
	// What weapon is being held?
	//
#if 0 //(gth) moving this back to Frequent to fix the game, re-optimize later...
	WeaponClass * p_weapon = Get_Weapon();
   bool has_weapon = (p_weapon != NULL);
   packet.Add(has_weapon);
   if (has_weapon) {
		packet.Add(p_weapon->Get_ID());
		packet.Add(p_weapon->Get_Total_Rounds());
   }
#endif

	WWASSERT(WeaponBag != NULL);
	WeaponBag->Export_Weapon_List(packet);

	//packet.Add(CtfTeamFlag, BITPACK_CTF_TEAM_FLAG);
}


/*
**
*/
void	SoldierGameObj::Import_Occasional( BitStreamClass &packet )
{
   WWASSERT(CombatManager::I_Am_Only_Client());

	SmartGameObj::Import_Occasional( packet );

	//
	// Held weapon
	//
#if 0 // (gth) moving back to "Frequent" to fix the game, re-optimize later?
	bool has_weapon = packet.Get(has_weapon);
   if (has_weapon) {
      WWASSERT(!packet.Is_Flushed());
	
		int weapon_id = packet.Get(weapon_id);
		int rounds = packet.Get(rounds);
		if ((Get_Weapon() == NULL) || (weapon_id != Get_Weapon()->Get_ID())) {
			WeaponBag->Select_Weapon_ID(weapon_id);
		}

		if (Get_Weapon() != NULL) {
			// If this weapon is currently being fired, ignore the server rounds count packet
			// This should help the jittery rounds count
			if ( !Get_Weapon()->Is_Triggered() ) {
				Get_Weapon()->Set_Total_Rounds( rounds );
			}
		}

   } else {
		if (Get_Weapon() != NULL) {
			Get_Weapon_Bag()->Deselect();
		}
	}
#endif

	//
	// Weapon list
	//
	WWASSERT(WeaponBag != NULL);
	WeaponBag->Import_Weapon_List(packet);


	/*
	//
	// Flag pickup/drop
	//
	int ctf_team_flag = packet.Get(ctf_team_flag, BITPACK_CTF_TEAM_FLAG);
	if (CtfTeamFlag != ctf_team_flag) {
		CombatManager::Change_Flag_Status(this, ctf_team_flag);
	}
	*/
}


/*
**
*/
void SoldierGameObj::Export_Frequent(BitStreamClass & packet)
{
	/**/
	//TSS101601
	bool in_vehicle = (Get_State() == HumanStateClass::IN_VEHICLE);
	packet.Add(in_vehicle);
	if (in_vehicle)
	{
		// Just do the control info
		SmartGameObj::Export_Frequent(packet);
		return;
	}
	/**/

	//
	// What weapon is being held?
	// (gth) moved this back into frequent to fix the game, re-optimize later
	//
	WeaponClass * p_weapon = Get_Weapon();
   bool has_weapon = (p_weapon != NULL);
   packet.Add(has_weapon);
   if (has_weapon) {
		packet.Add(p_weapon->Get_ID());
		packet.Add(p_weapon->Get_Total_Rounds());
	}

	//
	// Export Position and state
	//
	Vector3 position;
	Get_Position(&position);


#ifdef MULTIPLAYERDEMO
	//
	// Mix up the packet order to make demo/non-demo code more incompatible.
	//
	packet.Add(position.Y, BITPACK_WORLD_POSITION_Y);
	packet.Add(position.Z, BITPACK_WORLD_POSITION_Z);
	packet.Add(position.X, BITPACK_WORLD_POSITION_X);
#else
	packet.Add(position.X, BITPACK_WORLD_POSITION_X);
	packet.Add(position.Y, BITPACK_WORLD_POSITION_Y);
	packet.Add(position.Z, BITPACK_WORLD_POSITION_Z);
#endif

	
	packet.Add((int) HumanState.Get_State(), BITPACK_HUMAN_STATE);
	packet.Add(HumanState.Get_Sub_State(), BITPACK_HUMAN_SUB_STATE);


	if (HumanState.Get_State() == HumanStateClass::AIRBORNE) {
		// velocity is only needed for jumping
		Vector3 velocity;
		Get_Velocity(velocity);
		packet.Add(velocity.X);
		packet.Add(velocity.Y);
		packet.Add(velocity.Z);
	}

	if ((Get_State() == HumanStateClass::TRANSITION) || 
		 (Get_State() == HumanStateClass::ANIMATION) || 
		 (Get_State() == HumanStateClass::IN_VEHICLE)) {
      packet.Add_Terminated_String(AnimationName);
//		Debug_Say(( "In Transition %s\n", TransitionName ));
	}

	bool	is_special_damage = SpecialDamageMode != ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_NONE;
	packet.Add(is_special_damage);
	if ( is_special_damage ) {
		packet.Add( SpecialDamageMode );
	}

   SmartGameObj::Export_Frequent(packet);
}

/*
**
*/
void	SoldierGameObj::Import_Frequent( BitStreamClass & packet )
{
   WWASSERT(CombatManager::I_Am_Only_Client());

	/**/
	//TSS101601
	bool in_vehicle = packet.Get(in_vehicle);
	if (in_vehicle)
	{
		// Just get control info
		SmartGameObj::Import_Frequent(packet);
		return;
	}
	/**/

	//
	// What weapon is being held? 
	// (gth) moved this back into frequent to fix the game, re-optimize later
	//
	bool has_weapon = packet.Get(has_weapon);
   if (has_weapon) {
      WWASSERT(!packet.Is_Flushed());
	
		int weapon_id = packet.Get(weapon_id);
		int rounds = packet.Get(rounds);
		if ((Get_Weapon() == NULL) || (weapon_id != Get_Weapon()->Get_ID())) {
			WeaponBag->Select_Weapon_ID(weapon_id);
		}

		if (Get_Weapon() != NULL) {
			// If this weapon is currently being fired, ignore the server rounds count packet
			// This should help the jittery rounds count
			if ( !Get_Weapon()->Is_Triggered() ) {
				Get_Weapon()->Set_Total_Rounds( rounds );
			}
		}

   } else {
		if (Get_Weapon() != NULL) {
			Get_Weapon_Bag()->Deselect();
		}
	}

	//
	// Position
	//
	Vector3 sc_position;


#ifdef MULTIPLAYERDEMO
	//
	// Mix up the packet order to make demo/non-demo code more incompatible.
	//
	packet.Get(sc_position.Y, BITPACK_WORLD_POSITION_Y);
	packet.Get(sc_position.Z, BITPACK_WORLD_POSITION_Z);
	packet.Get(sc_position.X, BITPACK_WORLD_POSITION_X);
#else
	packet.Get(sc_position.X, BITPACK_WORLD_POSITION_X);
	packet.Get(sc_position.Y, BITPACK_WORLD_POSITION_Y);
	packet.Get(sc_position.Z, BITPACK_WORLD_POSITION_Z);
#endif


	// Bump Z up to the top of the possible values due to packing
	// we assume the max error is half of the resolution
	float max_error = cEncoderList::Get_Encoder_Type_Entry( BITPACK_WORLD_POSITION_Z ).Get_Resolution() / 2.0f;
	sc_position.Z += max_error;

	Interpret_Sc_Position_Data(sc_position);

	//
	// State and substate
	//
	int h_state = packet.Get(h_state, BITPACK_HUMAN_STATE);
	HumanStateClass::HumanStateType state = 
		(HumanStateClass::HumanStateType) h_state;
	int sub_state = packet.Get(sub_state, BITPACK_HUMAN_SUB_STATE);

	//
	// Velocity (if airborne)
	//
	Vector3 velocity;
	if (state == HumanStateClass::AIRBORNE) {
		packet.Get(velocity.X);
		packet.Get(velocity.Y);
		packet.Get(velocity.Z);
	}

	if (HumanState.Is_Locked()) {
      packet.Flush();
		return;
	}

#if 0
	if ( Get_State() == HumanStateClass::TRANSITION ) {
		Debug_Say(( "Ignoring Transitioning!\n" ));
      packet.Flush();
		return;
	}
#endif

	char trans_name[80] = "";
	if ( ( state == HumanStateClass::TRANSITION ) || 
		  ( state == HumanStateClass::ANIMATION ) || 
		  ( state == HumanStateClass::IN_VEHICLE ) ) {
      packet.Get_Terminated_String(trans_name, sizeof(trans_name));
	}

	Interpret_Sc_State_Data(state, sub_state, trans_name, velocity, sc_position);

	bool	is_special_damage;
	packet.Get(is_special_damage);
	int mode = ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_NONE;
	if ( is_special_damage ) {
		packet.Get(mode);
	}
	if ( mode != SpecialDamageMode ) {
		Set_Special_Damage_Mode( (ArmorWarheadManager::SpecialDamageType)mode );
	}

   if (Get_State() == HumanStateClass::DIVE)	{
      packet.Flush();
	} else {
		SmartGameObj::Import_Frequent(packet);
	}

   WWASSERT(packet.Is_Flushed());
}

//-----------------------------------------------------------------------------
void SoldierGameObj::Import_State_Cs(BitStreamClass & packet)
{
	bool	is_sniping = packet.Get( is_sniping );
	if ( is_sniping != Is_Sniping() ) {
		// Take sniping state from client
//		Debug_Say(( "Fixing Sniper State\n" ));
		HumanState.Toggle_State_Flag( HumanStateClass::SNIPING_FLAG );
	}

	bool	checking = packet.Get( checking );
	if ( checking ) {
		int check = packet.Get( check );
		if ( check != Check() ) {
			// React to cheating
			if ( Get_Player_Data() ) {
				Get_Player_Data()->Inc_Punish_Timer( TimeManager::Get_Frame_Seconds() )	;
			}
		}
	}

	SmartGameObj::Import_State_Cs(packet);
}

//-----------------------------------------------------------------------------
void SoldierGameObj::Export_State_Cs(BitStreamClass & packet)
{
	bool	is_sniping = Is_Sniping();
	packet.Add( is_sniping );

	bool checking = Control.Get_Boolean( ControlClass::BOOLEAN_ACTION );
	packet.Add( checking );
	if ( checking ) {
		int check = Check();
		packet.Add( check );
	}

	SmartGameObj::Export_State_Cs(packet);
}

//-----------------------------------------------------------------------------
int SoldierGameObj::Check(void)
{
Matrix3D tm(1);
#define	UNINITIALLIZED_CRC	0x78629140
	static int crc = UNINITIALLIZED_CRC;
tm.Rotate_X( 1.4f );
	if ( crc == UNINITIALLIZED_CRC ) {
		const char * filelist[] = {
		"laif`wp-gga",								//"objects.ddb",              
		"bqnlq-jmj",								//"armor.ini",                
		"almfp-jmj",								//"bones.ini",                
		"pvqeb`ffeef`wp-jmj",					//"surfaceeffects.ini",       
		"`bnfqbp-jmj",								//"cameras.ini",              
		"`\\mlg\\nd\\o3-t0g",						//"c_nod_mg_l0.w3d",          
		"`\\mlg\\qh\\o3-t0g",						//"c_nod_rk_l0.w3d",          
		"`\\mlg\\eo\\o3-t0g",						//"c_nod_fl_l0.w3d",          
		"`\\mlg\\fm\\o3-t0g",						//"c_nod_en_l0.w3d",          
		"`\\mlg\\ndl\\o3-t0g",						//"c_nod_mgo_l0.w3d",         
		"`\\mlg\\qhl\\o3-t0g",						//"c_nod_rko_l0.w3d",         
		"`\\mlg\\`kfnw\\o3-t0g",					//"c_nod_chemt_l0.w3d",       
		"`\\mlg\\pmjsfq\\o3-t0g",					//"c_nod_sniper_l0.w3d",      
		"`\\mlg\\qplog\\o3-t0g",					//"c_nod_rsold_l0.w3d",       
		"`\\mlg\\pwowk\\o3-t0g",					//"c_nod_stlth_l0.w3d",       
		"`\\mlg\\pbhv\\o3-t0g",						//"c_nod_saku_l0.w3d",        
		"`\\mlg\\pbhv1\\o3-t0g",					//"c_nod_saku2_l0.w3d",       
		"`\\mlg\\qbu\\o3-t0g",						//"c_nod_rav_l0.w3d",         
		"`\\mlg\\nqbu\\o3-t0g",						//"c_nod_mrav_l0.w3d",        
		"`\\mlg\\ngy\\o3-t0g",						//"c_nod_mdz_l0.w3d",         
		"`\\mlg\\ngy1\\o3-t0g",						//"c_nod_mdz2_l0.w3d",        
		"`\\mlg\\w`\\o3-t0g",						//"c_nod_tc_l0.w3d",          
		"`\\mlg\\nvwbmw\\o3-t0g",					//"c_nod_mutant_l0.w3d",      
		"`\\mlg\\npog\\o3-t0g",						//"c_nod_msld_l0.w3d",        
		"`\\mlg\\pplog\\o3-t0g",					//"c_nod_ssold_l0.w3d",       
		"`\\mlg\\sfwn\\o3-t0g",						//"c_nod_petm_l0.w3d",        
		"`\\mlg\\hbmf\\o3-t0g",						//"c_nod_kane_l0.w3d",        
		"`\\dgj\\nd\\o3-t0g",						//"c_gdi_mg_l0.w3d",          
		"`\\dgj\\qh\\o3-t0g",						//"c_gdi_rk_l0.w3d",          
		"`\\dgj\\dq\\o3-t0g",						//"c_gdi_gr_l0.w3d",          
		"`\\dgj\\fm\\o3-t0g",						//"c_gdi_en_l0.w3d",          
		"`\\dgj\\ndl\\o3-t0g",						//"c_gdi_mgo_l0.w3d",         
		"`\\dgj\\qhl\\o3-t0g",						//"c_gdi_rko_l0.w3d",         
		"`\\dgj\\pzg\\o3-t0g",						//"c_gdi_syd_l0.w3d",         
		"`\\dgj\\gfbg\\o3-t0g",						//"c_gdi_dead_l0.w3d",        
		"`\\dgj\\dvm\\o3-t0g",						//"c_gdi_gun_l0.w3d",         
		"`\\dgj\\sw`k\\o3-t0g",						//"c_gdi_ptch_l0.w3d",        
		"`\\kbul`\\o3-t0g",							//"c_havoc_l0.w3d",           
		"`\\kbul`m\\o3-t0g",						//"c_havocn_l0.w3d",          
		"`\\kbul`t\\o3-t0g",						//"c_havocw_l0.w3d",          
		"`\\kbul`g\\o3-t0g",						//"c_havocd_l0.w3d",          
		"`\\dgj\\pzg\\o3-t0g",						//"c_gdi_syd_l0.w3d",         
		"`\\dgj\\pzg1\\o3-t0g",						//"c_gdi_syd2_l0.w3d",        
		"`\\dgj\\nlaj\\o3-t0g",						//"c_gdi_mobi_l0.w3d",        
		"`\\dgj\\klwt\\o3-t0g",						//"c_gdi_hotw_l0.w3d",        
		"`\\dgj\\ow\\o3-t0g",						//"c_gdi_lt_l0.w3d",          
		"`\\mlg\\sfwq\\o3-t0g",						//"c_nod_petr_l0.w3d",        
		"`\\oldbm\\o3-t0g",							//"c_logan_l0.w3d",           
		"`\\dgj\\ol`hf\\o3-t0g",					//"c_gdi_locke_l0.w3d",       
		"u\\mlg\\avddz-t0g",						//"v_nod_buggy.w3d",          
		"u\\mlg\\bs`\\n-t0g",						//"v_nod_apc_m.w3d",          
		"u\\mlg\\bqwoqz-t0g",						//"v_nod_artlry.w3d",         
		"u\\mlg\\eobnf-t0g",						//"v_nod_flame.w3d",          
		"u\\mlg\\owbmh-t0g",						//"v_nod_ltank.w3d",          
		"u\\mlg\\pwowk-t0g",						//"v_nod_stlth.w3d",          
		"u\\mlg\\wqmpsw\\n-t0g",					//"v_nod_trnspt_m.w3d",       
		"u\\mlg\\bsb`kf\\n-t0g",					//"v_nod_apache_m.w3d",       
		"u\\`kbnfoflm-t0g",						//"v_chameleon.w3d",          
		"u\\dgj\\kvnuff-t0g",						//"v_gdi_humvee.w3d",         
		"u\\dgj\\bs`\\n-t0g",						//"v_gdi_apc_m.w3d",          
		"u\\dgj\\nqop-t0g",							//"v_gdi_mrls.w3d",           
		"u\\dgj\\nfgwmh-t0g",						//"v_gdi_medtnk.w3d",         
		"u\\dgj\\nbnnwk-t0g",						//"v_gdi_mammth.w3d",         
		"u\\sj`hvs32-t0g",							//"v_pickup01.w3d",           
		"u\\pfgbm32-t0g",							//"v_sedan01.w3d",            
		"u\\dgj\\lq`b\\n-t0g",						//"v_gdi_orca_m.w3d",         
		"u\\dgj\\wqmpsw\\n-t0g",					//"v_gdi_trnspt_m.w3d",       

		};
#define	NUM_CRC_FILES	(sizeof(	filelist ) / sizeof( filelist[0] ) )
		crc = 0;
tm.Rotate_Y( 2.8f );
		for ( int i = 0; i < NUM_CRC_FILES; i++ ) {
			StringClass name = filelist[i];
			// Obfuscate name
			char * n = &name[0];
			while ( *n ) *n++ ^= 0x3;
tm.Rotate_Z( 3.6f );
//			Debug_Say(( "		\"%s\",\n", name ));
			FileClass * file = _TheFileFactory->Get_File( name );
tm.Pre_Rotate_X( 0.3f );
			if ( file && file->Is_Available() ) {
				size_t size = file->Size();
tm.Pre_Rotate_Y( 0.4f );
				file->Open();
tm.Pre_Rotate_Z( 0.5f );
				while ( size > 0 ) {
					unsigned char buffer[ 4096 ];
					const size_t chunk = std::min<size_t>(size, sizeof(buffer));
tm.Translate_X( 3.1f );
					const int amount = file->Read(buffer, static_cast<int>(chunk));
tm.Translate_Y( 4.6f );
			if (amount <= 0) {
				break;
					}
					crc = CRC_Memory( buffer, amount, crc );
tm.Translate_Z( 8.2f );
					size -= static_cast<size_t>(amount);
				}
				file->Close();
			} else {
//				Debug_Say(( "***%s not found\n", name ));
			}

tm.Rotate_X( 1.4f );
		}
	}
	return crc;
}
//-----------------------------------------------------------------------------


/*
**
*/
bool	_UseLatencyInterpret = true;

//-----------------------------------------------------------------------------
void SoldierGameObj::Interpret_Sc_Position_Data( const Vector3 & sc_position)
{
	Vector3 position = sc_position;

	// Only use the latency code for the star
	if ( _UseLatencyInterpret && (this == COMBAT_STAR) ) {
		Peek_Human_Phys()->Network_Latency_State_Update(sc_position,Vector3(0,0,0));
		return;
	}

   WWASSERT(CombatManager::I_Am_Only_Client());

	if (Get_State() == HumanStateClass::TRANSITION) {
		//
		// Pop
		//
//		Set_Position(sc_position);
		Peek_Human_Phys()->Network_State_Update(sc_position,Vector3(0,0,0));
	} else {
		Vector3 current_position;
		Get_Position(&current_position);
		Vector3 pos_error = current_position - sc_position;

		//TSS081501
		if (Is_In_Elevator()) {
			position.Z = current_position.Z;
		}

//		Set_Position(sc_position);
		Peek_Human_Phys()->Network_State_Update(position,Vector3(0,0,0));
	}
}

//-----------------------------------------------------------------------------
void SoldierGameObj::Interpret_Sc_State_Data(
	HumanStateClass::HumanStateType state, int sub_state, 
	LPCSTR trans_name, const Vector3 & velocity, const Vector3 & sc_position)
{
   WWASSERT(CombatManager::I_Am_Only_Client());

	if ( ( Get_State() != state ) || (	Get_Sub_State() != sub_state ) ){
		if ( ( Get_State() == HumanStateClass::TRANSITION ) &&		// If in TRANSITION, don't switch to TRANSITION_COMPLETE
			  ( state == HumanStateClass::TRANSITION_COMPLETE ) ) {
			// do nothing
		} else if ( ( Get_State() == HumanStateClass::UPRIGHT ) && // If UPRIGHT, don't switch to LAND
						( state == HumanStateClass::LAND ) ) {	
			// do nothing
		} else {
			if (Is_Controlled_By_Me() &&
				 ((( Get_State() == HumanStateClass::UPRIGHT ) || ( Get_State() == HumanStateClass::AIRBORNE ) || ( Get_State() == HumanStateClass::LAND )) &&
				  (( state       == HumanStateClass::UPRIGHT ) || ( state       == HumanStateClass::AIRBORNE ) || ( state       == HumanStateClass::LAND )) )) {
				// do nothing
//			} else if (Is_Controlled_By_Me() == false) {
			} else {
				// take the new state
				if (state == HumanStateClass::TRANSITION) {
					Start_Transition_Animation( trans_name, NULL );
					AnimationName = trans_name;
				} else if (state == HumanStateClass::ANIMATION) {
					AnimationName = trans_name;
					HumanState.Start_Scripted_Animation( AnimationName, true, false );
				} else if (state == HumanStateClass::IN_VEHICLE) {
					HumanState.Force_Animation( trans_name, false );
					AnimationName = trans_name;
				}

				//TSS2001
//				if (state != HumanStateClass::IN_VEHICLE) {
					HumanState.Set_State( state, sub_state );
//				}
			}
		}
	}
		
	if (( state == HumanStateClass::AIRBORNE ) && ( this != COMBAT_STAR ) ) {
		Peek_Human_Phys()->Set_In_Contact( false );
		Peek_Human_Phys()->Set_Velocity( velocity );
		Peek_Human_Phys()->Set_Position( sc_position );
	}
}

//-----------------------------------------------------------------------------
int SoldierGameObj::Tally_Vis_Visible_Soldiers( void )
{
	int retcode = -1;

	Vector3 position;
	Get_Position(&position);
	position.Z += 1.5f;

	WWASSERT(COMBAT_SCENE != NULL);
	VisTableClass * pvs = COMBAT_SCENE->Get_Vis_Table(position);
	
	if (pvs != NULL) {

		retcode = 0;

		for (
			SLNode<SmartGameObj> * smart_objnode = GameObjManager::Get_Smart_Game_Obj_List()->Head(); 
			smart_objnode != NULL; 
			smart_objnode = smart_objnode->Next()) {

			SoldierGameObj * p_soldier = smart_objnode->Data()->As_SoldierGameObj();
			if (p_soldier != NULL) {

				PhysClass * phys_obj	= p_soldier->Peek_Physical_Object();
				if (phys_obj != NULL && pvs->Get_Bit(phys_obj->Get_Vis_Object_ID())) {
					retcode++;
				}
			}
		}
	}

	return retcode;
}

//-----------------------------------------------------------------------------
bool SoldierGameObj::Is_In_Elevator( void )
{
	HumanPhysClass	* p_human_phys = Peek_Human_Phys();

	return		
		p_human_phys != NULL && 
		p_human_phys->Peek_Carrier_Object() != NULL && 
		p_human_phys->Peek_Carrier_Object()->As_ElevatorPhysClass() != NULL;
}

//-----------------------------------------------------------------------------
float SoldierGameObj::Get_Max_Speed( void )
{
	return Peek_Human_Phys()->Get_Normalized_Speed();
}

//------------------------------------------------------------------------------------
void	SoldierGameObj::Set_Max_Speed( float speed )
{
	Peek_Human_Phys()->Set_Normalized_Speed( speed );
}

//------------------------------------------------------------------------------------
float	SoldierGameObj::Get_Turn_Rate( void )
{
	return Get_Definition().TurnRate;
}

//------------------------------------------------------------------------------------
void	SoldierGameObj::Generate_Control( void )
{
	switch( Get_State() ) {
		case HumanStateClass::DEATH:
//		case HumanStateClass::WOUNDED:
//		case HumanStateClass::DESTROY:
			Clear_Control();
			break;

		default:
			SmartGameObj::Generate_Control();
			break;
	}
}

//------------------------------------------------------------------------------------
void SoldierGameObj::Apply_Control( void )
{
	// if gameplay not permitted, skip
	if ( !CombatManager::Is_Gameplay_Permitted() ) {
		Clear_Control();
		Controller.Reset();
		return;
	}

#ifdef WWDEBUG
	// Change the control before applying
	if ( IS_SOLOPLAY && Is_Human_Controlled() && Input::Get_State( INPUT_FUNCTION_DEBUG_RAPID_MOVE ) ) {
		if ( InFlyMode ) {
			Control.Set_Analog( ControlClass::ANALOG_MOVE_UP,
				Control.Get_Analog( ControlClass::ANALOG_MOVE_FORWARD ) );
			Control.Set_Analog( ControlClass::ANALOG_MOVE_FORWARD, 0 );
		} else {
			Control.Set_Analog( ControlClass::ANALOG_MOVE_FORWARD,
				Control.Get_Analog( ControlClass::ANALOG_MOVE_FORWARD ) * 10 );
		}
	}
#endif

	if ( Get_State() == HumanStateClass::IN_VEHICLE ) {

		// Handle Vehicle Gunner Toggle
		if ( Get_Vehicle() && Get_Vehicle()->Get_Driver() == this ) {
			if ( Control.Get_Boolean( ControlClass::BOOLEAN_VEHICLE_TOGGLE_GUNNER ) ) {
				Get_Vehicle()->Toggle_Driver_Is_Gunner();
			}
		}		

		if (	CombatManager::I_Am_Server() )		// Server authoritative
//		if (	CombatManager::I_Am_Server() ||
//				(Is_Controlled_By_Me() && TransitionCompletionData == NULL) )
		{
			if ( Control.Get_Boolean( ControlClass::BOOLEAN_ACTION ) ) {
				TransitionManager::Check( this, true );
			}
		}
		return;
	}

	if ( ( Get_State() == HumanStateClass::TRANSITION ) || 
		  ( Get_State() == HumanStateClass::ANIMATION ) ||
		  ( Get_State() == HumanStateClass::DESTROY ) ||
		  ( Get_State() == HumanStateClass::DEATH ) ||
		  ( Get_State() == HumanStateClass::ON_FIRE ) ||
		  ( Get_State() == HumanStateClass::ON_CHEM ) ||
		  ( Get_State() == HumanStateClass::ON_CNC_FIRE ) ||
		  ( Get_State() == HumanStateClass::ON_CNC_CHEM ) ||
		  ( Get_State() == HumanStateClass::ON_ELECTRIC ) ) {

		// Force exit of the corpse mode	
		if ( Get_State() == HumanStateClass::DESTROY && Is_Human_Controlled() ) {
			if ( Control.Get_Boolean( ControlClass::BOOLEAN_WEAPON_FIRE_PRIMARY ) ) {
				HumanState.Set_State_Timer( 1000 );	// Done!
			}
		}

//		Debug_Say(( "No Control in transition\n" ));
		Clear_Control();
//		Controller.Reset();
//		return;
	}

	if ( Get_State() == HumanStateClass::DIVE ) {

		Clear_Control();

		SmartGameObj::Apply_Control();

		Vector3	forced_move(0,0,0);
		int sub_state = HumanState.Get_Sub_State();
		if ( sub_state & HumanStateClass::SUB_STATE_FORWARD )	forced_move = Vector3( 0.5f,0,0 );
		if ( sub_state & HumanStateClass::SUB_STATE_BACKWARD )	forced_move = Vector3( -0.5f,0,0 );
		if ( sub_state & HumanStateClass::SUB_STATE_LEFT )		forced_move = Vector3( 0,0.5f,0 );
		if ( sub_state & HumanStateClass::SUB_STATE_RIGHT )		forced_move = Vector3( 0,-0.5f,0 );

		Controller.Set_Move_Forward(	forced_move.X );
		Controller.Set_Move_Left(		forced_move.Y );
		Controller.Set_Move_Up(			forced_move.Z );
		return;
	}

#define	CLIMB_SCALE			0.3f

	// Skip all this if control is clear?!?!?!
	if ( Get_State() == HumanStateClass::LADDER ) {
		// Convert Forward motion to Up motion

		float up_down = CLIMB_SCALE * Control.Get_Analog( ControlClass::ANALOG_MOVE_FORWARD );

		if ( Control.Get_Analog( ControlClass::ANALOG_MOVE_FORWARD ) == 0 ) {
			LadderUpMask = false;
			LadderDownMask = false;
		}

		if ( LadderUpMask ) {
			up_down = MIN( up_down, 0 );
		}
		if ( LadderDownMask ) {
			up_down = MAX( up_down, 0 );
		}

		// when on ladder, clear all velocity
		Control.Set_Analog( ControlClass::ANALOG_MOVE_UP, up_down );


		// Can't clear this, or we wont be able to climb next net frame!!!
//		Control.Set_Analog( ControlClass::ANALOG_MOVE_FORWARD, 0 );
		Control.Set_Analog( ControlClass::ANALOG_MOVE_LEFT, 0 );
		Control.Set_Analog( ControlClass::ANALOG_TURN_LEFT, 0 );
		Control.Set_Boolean( ControlClass::BOOLEAN_JUMP, 0 );
	} else {
		LadderUpMask = false;
		LadderDownMask = false;

		// no move up

		if ( !InFlyMode ) {
			Control.Set_Analog( ControlClass::ANALOG_MOVE_UP, 0 );
		}
	}

#if 0
	if ( Control.Get_Boolean( ControlClass::BOOLEAN_CROUCH ) ) {
		if ( HumanState.Is_Sub_State_Adjustable() ) {
//			HumanState.Set_Sub_State( Get_Sub_State() ^ HumanStateClass::CROUCHED );
			HumanState.Toggle_State_Flag( HumanStateClass::CROUCHED_FLAG );

			if ( this == COMBAT_STAR ) {
				Vector3 pos;
				Get_Position( &pos );
				if ( Is_Crouched() ) {
					DIAG_LOG(( "COEN", "%1.2f; %1.2f; %1.2f", pos.X, pos.Y, pos.Z ));
				} else {
					DIAG_LOG(( "COEX", "%1.2f; %1.2f; %1.2f", pos.X, pos.Y, pos.Z ));
				}
			}
		}
	}
#else
	if ( HumanState.Is_Sub_State_Adjustable() ) {

		bool new_state = Control.Get_Boolean( ControlClass::BOOLEAN_CROUCH );
		bool old_state = HumanState.Get_State_Flag( HumanStateClass::CROUCHED_FLAG );

		if ( new_state != old_state ) {
	
			HumanState.Toggle_State_Flag( HumanStateClass::CROUCHED_FLAG );

			if ( this == COMBAT_STAR ) {
				Vector3 pos;
				Get_Position( &pos );
				if ( Is_Crouched() ) {
					DIAG_LOG(( "COEN", "%1.2f; %1.2f; %1.2f", pos.X, pos.Y, pos.Z ));
				} else {
					DIAG_LOG(( "COEX", "%1.2f; %1.2f; %1.2f", pos.X, pos.Y, pos.Z ));
				}
			}
		}
	}
#endif

	/*
	if (	CombatManager::I_Am_Server() && 
			Control.Get_Boolean(ControlClass::BOOLEAN_DROP_FLAG)	) {
		CombatManager::Soldier_Drops_Flag(this);
	}
	*/

	DetonateC4 = false;
	if ( Get_Weapon() && Control.Get_Boolean( ControlClass::BOOLEAN_WEAPON_USE ) ) {
		if ( Get_Weapon()->Get_Style() == WEAPON_HOLD_STYLE_C4 ) {
			DetonateC4 = true;
		}

		if ( Get_Weapon()->Get_Can_Snipe() ) {
			if ( HumanState.Is_Sub_State_Adjustable() ) {
				HumanState.Toggle_State_Flag( HumanStateClass::SNIPING_FLAG );
			}
		}
	}

#if 0
	if ( Is_Sniping() ) {

		// No motion for snipers (I don't know if this should be in Soldier.cpp)
		// (But then, Sniper mode means nothing for plain soldiers, only for players)
		//	Don't clear the move forward, just don't allow the controller to have
		//	any move forward.  This was not alowing sniper zoom
//		Control.Set_Analog( ControlClass::ANALOG_MOVE_FORWARD, 0 );	 
		Control.Set_Analog( ControlClass::ANALOG_MOVE_LEFT, 0 );
		Control.Set_Boolean( ControlClass::BOOLEAN_JUMP, 0 );
		Control.Set_Boolean( ControlClass::BOOLEAN_CROUCH, 0 );
		Control.Set_Boolean( ControlClass::BOOLEAN_DIVE_FORWARD, 0 );
		Control.Set_Boolean( ControlClass::BOOLEAN_DIVE_BACKWARD, 0 );
		Control.Set_Boolean( ControlClass::BOOLEAN_DIVE_LEFT, 0 );
		Control.Set_Boolean( ControlClass::BOOLEAN_DIVE_RIGHT, 0 );
		Control.Set_Boolean( ControlClass::BOOLEAN_ACTION, 0 );		// stop ladder
	}
#endif

	if ( Is_Sniping() || Is_On_Ladder() ) {
		// No diving for snipers or on ladders
		Control.Set_Boolean( ControlClass::BOOLEAN_DIVE_FORWARD, 0 );
		Control.Set_Boolean( ControlClass::BOOLEAN_DIVE_BACKWARD, 0 );
		Control.Set_Boolean( ControlClass::BOOLEAN_DIVE_LEFT, 0 );
		Control.Set_Boolean( ControlClass::BOOLEAN_DIVE_RIGHT, 0 );
	}

	bool dove = false;

	// Can only dive if upright
	if ( Get_State() == HumanStateClass::UPRIGHT ) {
		if ( Control.Get_Boolean( ControlClass::BOOLEAN_DIVE_FORWARD ) ) {
			HumanState.Set_State( HumanStateClass::DIVE, HumanStateClass::SUB_STATE_FORWARD );
			dove = true;
		}

		if ( Control.Get_Boolean( ControlClass::BOOLEAN_DIVE_BACKWARD ) ) {
			HumanState.Set_State( HumanStateClass::DIVE, HumanStateClass::SUB_STATE_BACKWARD );
			dove = true;
		}

		if ( Control.Get_Boolean( ControlClass::BOOLEAN_DIVE_LEFT ) ) {
			HumanState.Set_State( HumanStateClass::DIVE, HumanStateClass::SUB_STATE_LEFT );
			dove = true;
		}

		if ( Control.Get_Boolean( ControlClass::BOOLEAN_DIVE_RIGHT ) ) {
			HumanState.Set_State( HumanStateClass::DIVE, HumanStateClass::SUB_STATE_RIGHT );
			dove = true;
		}

		if ( dove && this == COMBAT_STAR ) {
			Vector3 pos;
			Get_Position( &pos );
			DefenseObjectClass * defense = Get_Defense_Object();
			const char * weapon_name = "";
			int ammo = 0;
			if ( Get_Weapon() != NULL )  {
				weapon_name = Get_Weapon()->Get_Definition()->Get_Name();
				ammo = Get_Weapon()->Get_Total_Rounds();
			}
			DIAG_LOG(( "ROUS", "%1.2f; %1.2f; %1.2f; %1.2f; %1.2f; %s; %d", pos.X, pos.Y, pos.Z, defense->Get_Shield_Strength(), defense->Get_Health(), weapon_name, ammo ));
		}
	}

	// Handle facing 
	float amount = Control.Get_Analog( ControlClass::ANALOG_TURN_LEFT );
	if ( COMBAT_STAR == this ) {	// Star gets turning from the camera
		amount = 0;
	}
	if ( amount != 0 ) {
		float heading = Peek_Human_Phys()->Get_Heading();
		heading += amount * Get_Turn_Rate() * TimeManager::Get_Frame_Seconds();
		heading = WWMath::Wrap( heading, DEG_TO_RADF( -180.0f ), DEG_TO_RADF( 180.0f ) );
		Peek_Human_Phys()->Set_Heading(heading);
		Control.Set_Analog(ControlClass::ANALOG_TURN_LEFT,0.0f);
		if ( !Is_Human_Controlled() ) {	// human players don't use turn anims
  			HumanState.Set_Turn_Velocity( amount );	// play the leg turning anim
		}
	}

	// Let parent class handle the rest
	SmartGameObj::Apply_Control();

#if 01
	// Have to Move_Up after after Apply control, but BOOLEAN_JUMP is cleared
	if ( Control.Get_Boolean( ControlClass::BOOLEAN_JUMP ) ) {
		if ( Get_State() != HumanStateClass::AIRBORNE ) {
			Controller.Set_Move_Up( Get_Definition().JumpVelocity );
		}

		if ( this == COMBAT_STAR ) {
			Vector3 pos;
			Get_Position( &pos );
			DefenseObjectClass * defense = Get_Defense_Object();
			const char * weapon_name = "";
			int ammo = 0;
			if ( Get_Weapon() != NULL )  {
				weapon_name = Get_Weapon()->Get_Definition()->Get_Name();
				ammo = Get_Weapon()->Get_Total_Rounds();
			}
			DIAG_LOG(( "JUUS", "%1.2f; %1.2f; %1.2f; %1.2f; %1.2f; %s; %d", pos.X, pos.Y, pos.Z, defense->Get_Shield_Strength(), defense->Get_Health(), weapon_name, ammo ));
		}
	}
#else
	static float jump_timer = 0;

	// Have to Move_Up after after Apply control, but BOOLEAN_JUMP is cleared
	if ( Control.Get_Boolean( ControlClass::BOOLEAN_JUMP ) ) {
		if ( Get_State() != HumanStateClass::AIRBORNE ) {
			HumanState.Set_State( HumanStateClass::AIRBORNE, HumanState.Get_Sub_State() );
			jump_timer = 0.2f;
		}
	}

	if ( jump_timer > 0 ) {
		jump_timer -= TimeManager::Get_Frame_Seconds();
		if ( jump_timer <= 0 ) {
			Controller.Set_Move_Up( Get_Definition().JumpVelocity );
		}
	}

#endif

#if 0
	if ( Is_Sniping() ) {
		// Dont allow move forward.  It may have been requested by the
		// Control, because we can't clear it if it must persist for
		// network sniper zoom.  So just clear it here.
		Controller.Set_Move_Forward( 0 );	
	}
#else
	if ( Is_Sniping() ) {
		// Dont allow move forward.  It may have been requested by the
		// Control, because we can't clear it if it must persist for
		// network sniper zoom.  So just clear it here.
		Controller.Set_Move_Forward( WWMath::Clamp( Controller.Get_Move_Forward(), -0.25f, 0.25f ) );	
		Controller.Set_Move_Left( WWMath::Clamp( Controller.Get_Move_Left(), -0.25f, 0.25f ) );	
	}
#endif

	if ( Get_State() == HumanStateClass::LADDER ) {
		// When on ladder, you can't move forward
		// Same rules as above wrt the network persistence
		Controller.Set_Move_Forward( 0 );	
	}

	if ( ( Get_State() == HumanStateClass::UPRIGHT ) && Is_Crouched() ) {
		float crouch_speed = GlobalSettingsDef::Get_Global_Settings ()->Get_Soldier_Crouch_Speed();
		Controller.Set_Move_Forward( Controller.Get_Move_Forward() * crouch_speed );	
		Controller.Set_Move_Left( Controller.Get_Move_Left() * crouch_speed );	
	} else if ( Control.Get_Boolean( ControlClass::BOOLEAN_WALK ) ) {
		float walk_speed = GlobalSettingsDef::Get_Global_Settings ()->Get_Soldier_Walk_Speed();
		Controller.Set_Move_Forward( Controller.Get_Move_Forward() * walk_speed );	
		Controller.Set_Move_Left( Controller.Get_Move_Left() * walk_speed );	
	}

	if ( this == COMBAT_STAR ) {
		static bool was_walking = false;
		if ( Control.Get_Boolean( ControlClass::BOOLEAN_WALK )  != was_walking ) {
			Vector3 pos;
			Get_Position( &pos );
			if ( !was_walking ) {
				DIAG_LOG(( "WAEN", "%1.2f; %1.2f; %1.2f", pos.X, pos.Y, pos.Z ));
			} else {
				DIAG_LOG(( "WAEX", "%1.2f; %1.2f; %1.2f", pos.X, pos.Y, pos.Z ));
			}
			was_walking = Control.Get_Boolean( ControlClass::BOOLEAN_WALK );
		}

	}

	if ( this == COMBAT_STAR && Control.Get_Boolean( ControlClass::BOOLEAN_ACTION ) ) {
		Vector3 pos;
		COMBAT_STAR->Get_Position( &pos );
		DIAG_LOG(( "ACPR", "%1.2f; %1.2f; %1.2f", pos.X, pos.Y, pos.Z ));
	}


	if (	CombatManager::I_Am_Server() ||
			(Is_Controlled_By_Me() && TransitionCompletionData == NULL) )
	{
		bool action_triggered = Control.Get_Boolean( ControlClass::BOOLEAN_ACTION );

		/// Transitions only trigger on server
		bool transition_triggered = false;
		if ( CombatManager::I_Am_Server() ) {
			transition_triggered = TransitionManager::Check( this, action_triggered );
		}

		if ( !transition_triggered ) {
			if ( action_triggered ) {

				//
				//	Lookup the current target
				//
//				DamageableGameObj *damageable_target = HUDInfo::Get_Weapon_Target_Object();
				DamageableGameObj *damageable_target = HUDInfo::Get_Info_Object();
				if ( damageable_target != NULL ) {
					PhysicalGameObj *physical_target = damageable_target->As_PhysicalGameObj();
					if ( physical_target != NULL ) {
						Vector3 target_pos;
						physical_target->Get_Position( &target_pos );
						Vector3 my_pos;
						Get_Position( &my_pos );

						//
						//	Check to see if the target is within the "poke" range
						//
						if ( (target_pos - my_pos).Length() <= 2 ) {

							//
							//	Notify all the observers
							//
							if ( CombatManager::I_Am_Server () ) {
								const GameObjObserverList & observer_list = physical_target->Get_Observers();
								for( int index = 0; index < observer_list.Count(); index++ ) {
									observer_list[ index ]->Poked( physical_target, this );
								}
							}

							//
							//	Was the player the soldier who did the poking?
							//
							if ( COMBAT_STAR == this ) {

								//
								//	Display this object in the encyclopedia
								//
								EncyclopediaMgrClass::Reveal_Object( damageable_target );
								
								//
								//	Check for player terminal access
								//
								if ( physical_target->As_SimpleGameObj () != NULL ) {
									
									//
									//	Is this a player terminal?
									//
									PlayerTerminalClass::TYPE type = PlayerTerminalClass::TYPE_NONE;
									type = (physical_target->As_SimpleGameObj())->Get_Definition().Get_Player_Terminal_Type();
									if (type != PlayerTerminalClass::TYPE_NONE) {

										//
										//	Show the player terminal
										//
										PlayerTerminalClass::Get_Instance()->Display_Terminal( this, type );
									}
								}
							}
						}
					}
				}
			}
		} else {
			// We started a transition, clear the control
			Controller.Reset();
		}
	}

	// Override the following controls
	if ( IS_SOLOPLAY && Is_Human_Controlled() && Input::Get_State( INPUT_FUNCTION_DEBUG_RAPID_MOVE ) ) {
		if ( Control.Get_Boolean( ControlClass::BOOLEAN_JUMP ) ) {
			Controller.Set_Move_Up( Get_Definition().JumpVelocity );
		}
	}

}

//------------------------------------------------------------------------------------
void	SoldierGameObj::Handle_Legs( void )
{
#if 0
	if ( Get_State() != HumanStateClass::UPRIGHT ) {
		LegFacing = Get_Facing();
		SyncLegs = false;
	}

	// If Moving, clear LegFacing
	if ((Control.Get_Analog( ControlClass::ANALOG_MOVE_FORWARD ) != 0.0f ) ||
		 (Control.Get_Analog( ControlClass::ANALOG_MOVE_LEFT ) != 0.0f )) {

		LegFacing = Get_Facing();
		SyncLegs = false;

	} else {

#define	CLAMP_DEGREES	60
#define	SYNC_DEGREES	35
#define	SYNC_RATE		75
#define	FLIP_RATE		720

		// find the leg difference;
		LegFacing = WWMath::Wrap( LegFacing, DEG_TO_RADF( -180 ), DEG_TO_RADF( 180 ) );

		float	legs_rotation = Get_Facing() - LegFacing;

		legs_rotation = WWMath::Wrap( legs_rotation, DEG_TO_RADF( -180 ), DEG_TO_RADF( 180 ) );

		// Clamp to with 90
		legs_rotation = WWMath::Clamp( legs_rotation, DEG_TO_RADF( -CLAMP_DEGREES ), DEG_TO_RADF( CLAMP_DEGREES ) );
		LegFacing = Get_Facing() - legs_rotation; 		
		LegFacing = WWMath::Wrap( LegFacing, DEG_TO_RADF( -180 ), DEG_TO_RADF( 180 ) );

		// if legs are more than 30 degrees off, start correcting
		if ( WWMath::Fabs( legs_rotation ) > DEG_TO_RAD( SYNC_DEGREES ) ) {
			SyncLegs = true;
		}

SyncLegs = true;

		// if syncing, start moving legs to match rotation
		if ( SyncLegs ) {
			float move = DEG_TO_RAD( SYNC_RATE ) * TimeManager::Get_Frame_Seconds() * 
								WWMath::Sign( legs_rotation );
			if ( WWMath::Fabs( move ) >= WWMath::Fabs( legs_rotation ) ) {
				move = legs_rotation;	// Complete syncing
				SyncLegs = false;
			} else {
				LegFacing += move;
				if ( !Is_Human_Controlled() ) {	// human players don't use turn anims
					HumanState.Set_Turn_Velocity( move );	// Also, play the leg turning anim
				}
			}
		}
	}

	if ( !SyncLegs ) {
		HumanState.Set_Turn_Velocity( 0 );
	}

	// I'm making this staic for now, because all human
	// skeletons have the bone at the same index
	static int  root_bone = -1;
	if ( root_bone == -1 ) {			// Get root bone index
		root_bone = Peek_Model()->Get_Bone_Index( "root" );
	}

	static int  torso_bone = -1;
	if ( torso_bone == -1 ) {			// Get torso bone index
		torso_bone = Peek_Model()->Get_Bone_Index( "thorax" );
	}
	// Update the model
	float	legs_rotation = Get_Facing() - LegFacing;
	if ( legs_rotation ) {

		WWASSERT( root_bone != -1 );
		WWASSERT( torso_bone != -1 );

		if ( !Peek_Model()->Is_Bone_Captured( root_bone ) ) {
			Peek_Model()->Capture_Bone( root_bone );
		}
		if ( !Peek_Model()->Is_Bone_Captured( torso_bone ) ) {
			Peek_Model()->Capture_Bone( torso_bone );
		}

		// LOOK INTO RELATIVE_CONTROL_BONE
		Matrix3D	root_adjust(1);				// adjust it
		root_adjust.Rotate_Z( -legs_rotation );
		Peek_Model()->Control_Bone( root_bone, root_adjust );

		Matrix3D	legs_adjust(1);				// adjust it
		legs_adjust.Rotate_Z( legs_rotation );
		Peek_Model()->Control_Bone( torso_bone, legs_adjust );
	} else {	// no adjustment, release

	 	if ( Peek_Model()->Is_Bone_Captured( root_bone ) ) {
			Peek_Model()->Release_Bone( root_bone );
		}
	 	if ( Peek_Model()->Is_Bone_Captured( torso_bone ) ) {
			Peek_Model()->Release_Bone( torso_bone );
		}
	}

#else

#if 0
	float	legs_rotation = 0;

	// Compare the facing to the motion, set leg_racing to the difference
	Vector3 move( Control.Get_Analog( ControlClass::ANALOG_MOVE_FORWARD ), Control.Get_Analog( ControlClass::ANALOG_MOVE_LEFT ), 0 );
	if ( move.Length() > 0 ) {
		float move_direction = ::WWMath::Atan2( -move.Y, move.X );
		float diff = move_direction;
		diff += 2*DEG_TO_RADF( 360 ) + DEG_TO_RADF( 45 );
		diff -= WWMath::Floor( diff / DEG_TO_RADF( 90 ) ) * DEG_TO_RADF( 90 );
		diff -= DEG_TO_RADF( 45 );
		legs_rotation = diff;
		Debug_Say(( "Move (%1.1f)  %1.1f %1.1f %1.1f   %1.1f\n", RAD_TO_DEG(diff), move.X, move.Y, move.Z, RAD_TO_DEG( move_direction ) ));
	}


	// I'm making this staic for now, because all human
	// skeletons have the bone at the same index
	static int  root_bone = -1;
	if ( root_bone == -1 ) {			// Get root bone index
		root_bone = Peek_Model()->Get_Bone_Index( "c spine" );
	}

	static int  torso_bone = -1;
	if ( torso_bone == -1 ) {			// Get torso bone index
		torso_bone = Peek_Model()->Get_Bone_Index( "c spine1" );
	}

	if ( legs_rotation ) {

		WWASSERT( root_bone != -1 );
		WWASSERT( torso_bone != -1 );

		if ( !Peek_Model()->Is_Bone_Captured( root_bone ) ) {
			Peek_Model()->Capture_Bone( root_bone );
		}
		if ( !Peek_Model()->Is_Bone_Captured( torso_bone ) ) {
			Peek_Model()->Capture_Bone( torso_bone );
		}

		Matrix3D	root_adjust(1);				// adjust it
		root_adjust.Rotate_X( legs_rotation );
		Peek_Model()->Control_Bone( root_bone, root_adjust );

		Matrix3D	legs_adjust(1);				// adjust it
		legs_adjust.Rotate_X( -legs_rotation );
		Peek_Model()->Control_Bone( torso_bone, legs_adjust );
	} else {	// no adjustment, release

	 	if ( Peek_Model()->Is_Bone_Captured( root_bone ) ) {
			Peek_Model()->Release_Bone( root_bone );
		}
	 	if ( Peek_Model()->Is_Bone_Captured( torso_bone ) ) {
			Peek_Model()->Release_Bone( torso_bone );
		}
	}

#endif

#endif

	bool do_steps = false;
	if ( Is_On_Ladder() ) {
		do_steps = (Control.Get_Analog( ControlClass::ANALOG_MOVE_FORWARD ) != 0.0f );
	} else {
		do_steps = ((Control.Get_Analog( ControlClass::ANALOG_MOVE_FORWARD ) != 0.0f ) ||
			 (Control.Get_Analog( ControlClass::ANALOG_MOVE_LEFT ) != 0.0f ));
	}

	// Footsteps
	if (!Is_Sniping() && !InFlyMode && CombatManager::Is_Gameplay_Permitted() && do_steps && Is_Control_Enabled() ) {

		bool leg_mode = HumanState.Get_Leg_Mode();

		// Watch for the legs to reach certain percentage completions

		if ( leg_mode != LastLegMode ) {

			int my_type = SurfaceEffectsManager::HITTER_TYPE_FOOTSTEP_RUN;
			if ( Is_Crouched() ) {
				my_type = SurfaceEffectsManager::HITTER_TYPE_FOOTSTEP_CROUCHED;
			} else if ( HumanState.Get_Sub_State() & HumanStateClass::SUB_STATE_SLOW ) {
				my_type = SurfaceEffectsManager::HITTER_TYPE_FOOTSTEP_WALK;
			}

			int ground_type = Peek_Human_Phys()->Get_Contact_Surface_Type();

			// Try effects on ladders again BMG
			/**/if ( Is_On_Ladder() ) {
				my_type = SurfaceEffectsManager::HITTER_TYPE_FOOTSTEP_LADDER;
				ground_type = SURFACE_TYPE_LIGHT_METAL;
			}/**/

			Matrix3D tm;
			tm = Get_Transform();
			if ( Control.Get_Analog( ControlClass::ANALOG_MOVE_FORWARD ) != 0.0f )  {
				tm.Rotate_Z( DEG_TO_RAD( -90 ) );
			}
			float offset = leg_mode ? -0.15f : 0.1f;
			tm.Translate( Vector3( offset, 0.3f, 0 ) );
			
			bool allow_emitters = !Is_Stealthed();
			SurfaceEffectsManager::Apply_Effect( ground_type, my_type, tm, NULL, this, true, allow_emitters);
			LastLegMode = leg_mode;
		}

	}
}

static float	_shake_delay = 0;
static float	_cry_delay = 0;

static const char * _profile_name = "Soldier Think";

//------------------------------------------------------------------------------------
void	SoldierGameObj::Think( void )
{
	{	WWPROFILE( _profile_name );

		if ( this == COMBAT_STAR ) {
			_shake_delay -= TimeManager::Get_Frame_Seconds();
			_cry_delay -= TimeManager::Get_Frame_Seconds();
		}
		/*
		ActionClass * p_action = Get_Action();
		WWASSERT(p_action != NULL);
		*/
		
		//
		//	Simply check to see if this soldier has entered a coordination zone (which bound
		// the entrances/exits of ladders and elevators).  If so, then disable
		// collision between them and all other soldiers.
		//
		Vector3 position;
		{
			WWPROFILE("Coordination Zone");
			Get_Position( &position );
			if ( UnitCoordinationZoneMgr::Is_Unit_In_Zone( position ) ) {
				Enable_Ghost_Collision( true );
			} else if ( Is_Safe_To_Disable_Ghost_Collision( position ) ) {
				Enable_Ghost_Collision( false );
			}
		}

		//
		//	Display a debug box to show the ghosted collision as ncessary
		//
		if (	DisplayDebugBoxForGhostCollision &&
				(Peek_Physical_Object ()->Get_Collision_Group() == SOLDIER_GHOST_COLLISION_GROUP))
		{
			WWPROFILE("Add_Debug_AABox");
			AABoxClass soldier_box;
			soldier_box.Center = position + Vector3( 0, 0, 1.0F );
			soldier_box.Extent.Set( 0.3F, 0.3F, 1.0F );
			PhysicsSceneClass::Get_Instance ()->Add_Debug_AABox( soldier_box, Vector3( 1.0F, 0.0F, 0.25F ) );
		}

		// Stats
		if ( Get_Player_Data() != NULL ) {
			WWPROFILE("Stats");
			Get_Player_Data()->Stats_Add_Game_Time( TimeManager::Get_Frame_Seconds() );
			Get_Player_Data()->Stats_Set_Final_Health( Get_Defense_Object()->Get_Health() );
			if ( Get_Vehicle() != NULL ) {
				Get_Player_Data()->Stats_Add_Vehicle_Time( TimeManager::Get_Frame_Seconds() );
			}
		}

		//
		//	Update the soldier's facing
		//
		{
			WWPROFILE("Update_Locked_Facing");
			Update_Locked_Facing();
		}

		/*
		//
		// In CTF a flag-bearer may not be permitted to use weapons
		// This code will deselect the weapon if that is the case.
		// 
		if (CombatManager::I_Am_Server() && Get_Weapon() != NULL &&
			!CombatManager::Is_Firing_Enabled(this)) {
			Get_Weapon_Bag()->Deselect();
		}
		*/

		// Handle_Legs moved form Apply_Control because clients don't run it for server objects
		{
			WWPROFILE("Handle_Legs");
			Handle_Legs();
		}

		/*	
		if (CombatManager::I_Am_Server()) {
			TransitionManager::Check( this );		
		}*/
	}

	{
		WWPROFILE( "Embedded smart think in soldier" );
		SmartGameObj::Think(); 	// Perform smart object thinking	( apply controls )
	}

{
	WWPROFILE( _profile_name );

	if ( CombatManager::I_Am_Server() ) {
		WWPROFILE("Handle C4");
		// Handle C4 in a special way.  When C4 is fired, the human plays
		// a crouching animation, and starts a c4 placement timer.
		if (  Get_Weapon() && 
			  (Get_Weapon()->Get_Style() == WEAPON_HOLD_STYLE_C4) && 
			  (Get_Weapon()->Is_Firing()) && 
			  (Get_State() == HumanStateClass::UPRIGHT) ) {

			AnimationName = "s_a_human.h_a_j12c";
			HumanState.Start_Scripted_Animation( AnimationName, true, false );
		}
	}

	if ( Get_State() != HumanStateClass::IN_VEHICLE ) {
		WWPROFILE("Vehicle");
		// This may try to change controls, so can't be in Post_Think
		HumanState.Update_Weapon( Get_Weapon(), WeaponBag->Is_Changed() );

		if ( WeaponBag->Is_Changed() ) {
			WeaponChanged = true;
			WeaponBag->Reset_Changed();
		}

//		HumanState.Update_Aiming( Get_Weapon_Tilt(), Get_Weapon_Turn() );
		HumanState.Update_State(); 

		// Remove C4 from hand if empty
		if ( Get_Weapon() && WeaponRenderModel ) {
			if ( Get_Weapon()->Get_Style() == WEAPON_HOLD_STYLE_C4 || Get_Weapon()->Get_Style() == WEAPON_HOLD_STYLE_BEACON ) {
				WeaponRenderModel->Set_Hidden( !Get_Weapon()->Is_Loaded() );
			} else {
				WeaponRenderModel->Set_Hidden( false );
			}
		}

	} else {
//		Debug_Say(( "No state update in vehicle\n" ));
	}

	/*
	**  Special Death
	*/
//	ArmorWarheadManager::SpecialDamageType			SpecialDamageMode;
//	float						SpecialDamageTimer;
	if ( SpecialDamageMode != ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_NONE && 
			SpecialDamageTimer > 0 ) {
		WWPROFILE("Special Damage");

		float previous_timer = SpecialDamageTimer;

		SpecialDamageTimer -= TimeManager::Get_Frame_Seconds();

		// Once a second, make an explosion
		if ( (int)SpecialDamageTimer != (int)previous_timer ) {
			StringClass explosion_name = ArmorWarheadManager::Get_Special_Damage_Explosion( SpecialDamageMode );
			if ( !explosion_name.Is_Empty() ) {
				ExplosionDefinitionClass * def = (ExplosionDefinitionClass *)DefinitionMgrClass::Find_Typed_Definition( explosion_name, CLASSID_DEF_EXPLOSION );
				if ( def != NULL) {
					Vector3 pos = Get_Bullseye_Position();
					ExplosionManager::Create_Explosion_At( def->Get_ID(), pos, NULL );
				}
			}
		}

		// Fix to allow taking fire damage in CNC
//		if ( Get_Defense_Object()->Get_Health() > 0 && Allow_Special_Damage_State_Lock () ) {
		if ( Get_Defense_Object()->Get_Health() > 0 && 
				(Allow_Special_Damage_State_Lock () || Is_Human_Controlled() ) ) {
			// do damage
			WarheadType warhead = ArmorWarheadManager::Get_Special_Damage_Warhead( SpecialDamageMode );
			float damage = ArmorWarheadManager::Get_Special_Damage_Scale( SpecialDamageMode ) * 
									TimeManager::Get_Frame_Seconds();
			OffenseObjectClass off( damage, warhead, (ArmedGameObj*)SpecialDamageDamager.Get_Ptr()	);
//			Apply_Damage( off, 1 );
			Apply_Damage_Extended( off, 1 );
		}

		if ( HumanState.Get_State() != HumanStateClass::DEATH ) {

			// if dead, play death
			if ( Get_Defense_Object()->Get_Health() <= 0 ) {
				HumanState.Set_State( HumanStateClass::DEATH, 
					SpecialDamageMode - ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_FIRE + HumanStateClass::OUCH_FIRE );
			}

			if ( SpecialDamageTimer <= 0 ) {		// Stop the flame
//				HumanState.Set_State( HumanStateClass::UPRIGHT );
				Set_Special_Damage_Mode( ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_NONE );
			}
		}
	}

	if (Get_State() == HumanStateClass::DESTROY) {
		if ( HumanState.Get_State_Timer() > CORPSE_PERSIST_TIME ) {
			// If Human Controlled, drop a backpack
			if ( Is_Human_Controlled() ) {	
//				PowerUpGameObj::Create_Backpack( this );	// No more backpacks
			}

			Set_Delete_Pending();
		}
	}

	// ClientOnlys doen't handle TRANSITION_COMPLETE
	if ( !CombatManager::I_Am_Only_Client() || Is_Controlled_By_Me()) {
		if ( Get_State() == HumanStateClass::TRANSITION_COMPLETE ) {
			if ( TransitionCompletionData != NULL ) {
	//			Debug_Say(( "Ended Transition %p\n", Transition ));
				// We just completed a transition, notify
				TransitionInstanceClass::End( this, TransitionCompletionData );
				TransitionCompletionData = NULL;
			} else {
	//			Debug_Say(( "Ended Unknown Transition %p\n", Transition ));
				HumanState.Set_State( HumanStateClass::UPRIGHT );
			}
		}
	}

	//
	//	Remove the emot icon (if necessary)
	//
	{
		WWPROFILE("EmotIcon");
		EmotIconTimer -= TimeManager::Get_Frame_Seconds();
		if ( EmotIconTimer <= 0 ) {

			//
			//	Remove the emoticon
			//
			if ( EmotIconModel != NULL && EmotIconModel->Is_In_Scene() ) {
				EmotIconModel->Remove();
				REF_PTR_RELEASE( EmotIconModel );
			}

		} else if ( EmotIconModel != NULL  ) {

			//
			//	Put the icon over the soldier's head
			//
			Matrix3D tm = Get_Transform();
			tm.Set_Translation (tm.Get_Translation() + Vector3 (0, 0, EMOT_ICON_HEIGHT));
			EmotIconModel->Set_Transform( tm );
		}
	}

	//
	//	Should we change the idle facial animation?
	//
	GenerateIdleFacialAnimTimer -= TimeManager::Get_Frame_Seconds();
	if ( GenerateIdleFacialAnimTimer <= 0 ) {
		WWPROFILE("FacialAnims");
		
		//
		//	Change the facial animation
		//
		if ( HeadModel != NULL ) {
			// Don't stop a playing script animation
			if ( ( HeadModel->Peek_Animation() == NULL ) || 
				  (HeadModel->Peek_Animation() == SpeechAnim ) ) {
				SpeechAnim->Generate_Idle_Animation( 20, 0.5F );
				HeadModel->Set_Animation( SpeechAnim, 0, RenderObjClass::ANIM_MODE_LOOP );
			}
		}

		//
		//	Reset the timer
		//
		GenerateIdleFacialAnimTimer = 100.0F;
	}

	/*
	if (!Is_Destroy()) {
	   CombatManager::Set_Soldier_Tint(this);
	}
	*/

	// Apply Surface damage
	{
		WWPROFILE("Apply_Damage");
		if ( Peek_Human_Phys() ) {
			SurfaceEffectsManager::Apply_Damage( Peek_Human_Phys()->Get_Contact_Surface_Type(), this );
		}
	}

	// Update the water wake 
	// - If our feet are not on "underwater dirt" we don't even check for water
	// - Otherwise, do a ray cast to find the water surface, if it hits
	{
		WWPROFILE("Water");
		bool in_water = false;
		Vector3 p0 = Get_Transform().Get_Translation();
		Vector3 p1 = Get_Bullseye_Position();

		int hit_type = SurfaceEffectsManager::HITTER_TYPE_FOOTSTEP_RUN;
		if ( Is_Crouched() ) {
			hit_type = SurfaceEffectsManager::HITTER_TYPE_FOOTSTEP_CROUCHED;
		} else if ( HumanState.Get_Sub_State() & HumanStateClass::SUB_STATE_SLOW ) {
			hit_type = SurfaceEffectsManager::HITTER_TYPE_FOOTSTEP_WALK;
		}

		Vector3 vel;
		Peek_Human_Phys()->Get_Velocity(&vel);

		if ((Peek_Human_Phys()->Get_Contact_Surface_Type() == SURFACE_TYPE_UNDERWATER_DIRT) &&
			 (vel.Length2() > 0.1f))
		{
			
			// Raycast to find water surface
			CastResultStruct res;
			LineSegClass ray(p0,p1);
			PhysRayCollisionTestClass raytest(ray,&res,BULLET_COLLISION_GROUP,COLLISION_TYPE_PROJECTILE);
			
			Peek_Human_Phys()->Inc_Ignore_Counter();
			COMBAT_SCENE->Cast_Ray(raytest);
			Peek_Human_Phys()->Dec_Ignore_Counter();

			// if found update emitter
			if ((res.Fraction < 1.0f) && (res.SurfaceType == SURFACE_TYPE_WATER_PERMEABLE)) {

				Vector3 point;
				ray.Compute_Point(res.Fraction,&point);

				SurfaceEffectsManager::Update_Persistant_Emitter(	WaterWake,
																					res.SurfaceType, 
																					hit_type, 
																					Matrix3D(point));
				in_water = true;
			}
		}
		
		if (!in_water) {
				SurfaceEffectsManager::Update_Persistant_Emitter(	WaterWake,
																					SURFACE_TYPE_DEFAULT, 
																					hit_type, 
																					Matrix3D(p0));
		}
	}


	// Punish update
	if ( Get_Player_Data() && Get_Player_Data()->Get_Punish_Timer() > 0 ) {
		Get_Player_Data()->Inc_Punish_Timer( TimeManager::Get_Frame_Seconds() )	;
	}

}
}

//------------------------------------------------------------------------------------
void	SoldierGameObj::Post_Think( void )
{
	if ( Get_State() == HumanStateClass::IN_VEHICLE ) {
		SmartGameObj::Post_Think();
		return;
	}

{	WWPROFILE( "Soldier PostThink" );

	HumanState.Post_Think(); 

	bool update_weapon = false;

	if ( WeaponChanged ) {
		WeaponChanged = false;
		Update_Back_Gun();
		update_weapon = true;
	}

	if ( ( Get_Weapon() != NULL ) && ( Get_Weapon()->Is_Model_Update_Needed() ) ) {
		update_weapon = true;
		Get_Weapon()->Reset_Model_Update();			// reset model update needed
	}


	if ( update_weapon ) {
		if ( Get_Weapon() != NULL ) {
			Set_Weapon_Model( Get_Weapon()->Get_Model_Name() );
		} else {
			Set_Weapon_Model( NULL );
		}
	}

//			if ( Get_Weapon()->Is_Anim_Update_Needed() ) {
//				Set_Weapon_Animation( Get_Weapon()->Get_Anim_Name() );
//				Get_Weapon()->Reset_Anim_Update();		// reset model anim update needed
//			}

	if (WeaponAnimControl) {
		WeaponAnimControl->Update( TimeManager::Get_Frame_Seconds() );			// update the animation control
	}


	Handle_Head_look();

	if ( Get_Weapon() != NULL && Get_State() == HumanStateClass::ON_FIRE ) {
		Get_Weapon()->Set_Primary_Triggered( false );
		Get_Weapon()->Set_Secondary_Triggered( false );
	}

}
	SmartGameObj::Post_Think();

	Update_Healing_Effect();

}

//------------------------------------------------------------------------------------
const Matrix3D & SoldierGameObj::Get_Muzzle( int /* index */ )
{
	static Matrix3D _muzzle(1);
	if ( WeaponRenderModel != NULL ) {
		Matrix3D true_muzzle = WeaponRenderModel->Get_Bone_Transform( "muzzlea0" );
		Vector3 muzzle_pos = true_muzzle.Get_Translation();
		_muzzle.Obj_Look_At( muzzle_pos, Get_Targeting_Pos(), 0 );

		if ( !Is_Human_Controlled() ) {
			// If the bullet is not close to going down the muzzle, force it to be
			Vector3	to_target = _muzzle.Get_X_Vector();
			Vector3	down_muzzle = true_muzzle.Get_X_Vector();
			float cos = Vector3::Dot_Product( to_target, down_muzzle );
			if ( cos < WWMath::Cos( DEG_TO_RADF( 20 ) ) ) {
				_muzzle = true_muzzle;
			}
		}

	}
	return _muzzle;
}

static int head_bone = -1;
static int neck_bone = -1;

//------------------------------------------------------------------------------------
Matrix3D SoldierGameObj::Get_Look_Transform(void)  
{
	if ( head_bone != -1 ) {
		// Convert from CS head convention back to normal
		Matrix3D tm = Peek_Model()->Get_Bone_Transform( head_bone );
		tm.Rotate_Z( DEG_TO_RAD( 90 ) );
		tm.Rotate_X( DEG_TO_RAD( 90 ) );
		return tm;
	}
	return Get_Transform();
}

//------------------------------------------------------------------------------------
#define	HEAD_TURN_LIMIT	DEG_TO_RADF( 70 )
#define	HEAD_TILT_LIMIT	DEG_TO_RADF( 20 )

void	SoldierGameObj::Look_Random( float time )
{
	if ( time == 0 && HeadLookDuration != 0 ) {
		HeadLookDuration = 0.0001f;		// maybe done next time...
	} else {
		HeadLookDuration = time;
	}
	HeadLookAngleTimer = 0;
	HeadLookAngle = Vector3( 1,1,1 );
}

////////////////////////////////////////////////////////////////
//	Clamp_Angle
////////////////////////////////////////////////////////////////
static inline float
Clamp_Angle (float angle, float min_angle, float max_angle)
{
	//
	//	Make sure all the parameters are in the same range
	//
	angle			= WWMath::Wrap (angle,		0, DEG_TO_RADF (360));
	min_angle	= WWMath::Wrap (min_angle,	0, DEG_TO_RADF (360));
	max_angle	= WWMath::Wrap (max_angle,	0, DEG_TO_RADF (360));

	float result = angle;

	if (min_angle <= max_angle) {
		
		//
		//	Handle the 'typical' case where there is no 360-mark wrap-around
		//
		if (angle < min_angle) {
			result = min_angle;
		} else if (angle > max_angle) {		
			result = max_angle;
		}

	} else {
		
		//
		//	Handle the 360-mark wrap-around case
		//
		if (angle < min_angle && angle > max_angle) {
			float min_delta = min_angle - angle;
			float max_delta = angle - max_angle;

			//
			//	Which edge is the angle closer to?
			//
			if (min_delta < max_delta) {
				result = min_angle;
			} else {
				result = max_angle;
			}
		}
	}

	return result;
}


void	SoldierGameObj::Handle_Head_look( void )
{
	if ( Peek_Model ()->Get_HTree () == NULL ) {
		return ;
	}

	//
	//	Get the head bone
	//
	if ( head_bone == -1 || neck_bone == -1) {
		head_bone = Peek_Model()->Get_Bone_Index( "C HEAD" );
		neck_bone = Peek_Model()->Get_Bone_Index( "C NECK" );
		WWASSERT( head_bone != -1 );
		WWASSERT( neck_bone != -1 );
	}

	if ( HeadLookDuration > 0 ) {

		HeadLookDuration -= TimeManager::Get_Frame_Seconds();

		// Should we be returning to look ahead?
		bool returning = HeadLookDuration < 0;

		//Matrix3D	head(1);

		Vector3	desired_head_rotation( 0,0,0 );
		if ( !returning ) {

			if ( HeadLookAngle.Length() > 0.001f ) {
				HeadLookAngleTimer -= TimeManager::Get_Frame_Seconds();
				if ( HeadLookAngleTimer < 0 ) {
					HeadLookAngle = Vector3(
						FreeRandom.Get_Float( -HEAD_TURN_LIMIT, HEAD_TURN_LIMIT ),
						FreeRandom.Get_Float( -HEAD_TILT_LIMIT, HEAD_TILT_LIMIT )	,0); 
//					Debug_Say(( "New Look Turn %f Tilt %f\n", RAD_TO_DEG(HeadLookAngle.X), RAD_TO_DEG(HeadLookAngle.Y) ));
					HeadLookAngleTimer = FreeRandom.Get_Float( 2, 5 );
				}
				desired_head_rotation = HeadLookAngle;

			} else {

				//
				//	Release the captured bone, this will have the effect of causing the Get_Bone_Transform ()
				// methods to return the un-modified (due to being controlled) transform of the bone.
				//
				/*if ( Peek_Model()->Is_Bone_Captured( head_bone ) ) {
					Peek_Model()->Release_Bone( head_bone );
				}*/

				///
				//	Get the transform that has been used to modify the head bone...
				//
				const HTreeClass *htree = Peek_Model()->Get_HTree();
				WWASSERT( htree != NULL );

				Matrix3D bone_control_tm( 1 );
				htree->Get_Bone_Control( head_bone, bone_control_tm );

				//
				//	Get the inverse of the head-bone transform 
				//
				Matrix3D inv_bone_control_tm;
				bone_control_tm.Get_Orthogonal_Inverse (inv_bone_control_tm);

				//
				//	Get the head to world and neck to world transforms
				//
				Matrix3D cur_head = Peek_Model()->Get_Bone_Transform( head_bone );
				Matrix3D cur_neck = Peek_Model()->Get_Bone_Transform( neck_bone );

				//
				//	Strip off the control transform from last frame
				//
				cur_head = cur_head * inv_bone_control_tm;

				//
				//	Get the world to neck transform
				//
				Matrix3D world_to_neck_tm;
				cur_neck.Get_Orthogonal_Inverse (world_to_neck_tm);

				//
				//	Build a head to neck transform
				//
				Matrix3D head_to_neck_tm = world_to_neck_tm * cur_head;				

				//
				//	Get the target relative to the head
				//
				Vector3 relative_head_target;
				Matrix3D::Inverse_Transform_Vector( cur_head, HeadLookTarget, &relative_head_target );

				//
				//	Determine the 'twist' and lookup/down angles.
				// Note:  Currently in the head bone coordinate system, the X axis is the same
				// as the Z axis in object space, the Y axis is the same as the X axis in object space,
				// and the Z axis is the same as the Y axis in object space.
				//
				desired_head_rotation.X = WWMath::Atan2( relative_head_target.Z, relative_head_target.Y );
				desired_head_rotation.Z = -WWMath::Fast_Asin( relative_head_target.X / relative_head_target.Length() );
				desired_head_rotation.Y = 0;

				//
				//	Determine how far to allow the character to turn and tilt his/her head.
				// These boundaries are based on the "absolute" amount the person can turn
				// their head, this has to take into consideration the amount that the current
				// animation is turning the head and the amount we need to turn to look at the target.
				//
				Vector3 temp_vec = head_to_neck_tm.Get_Y_Vector ();
				float curr_rot_x  = ::atan2 (temp_vec.Z, temp_vec.Y);
				float curr_rot_z  = ::atan2 (temp_vec.X, temp_vec.Y);
				
				float min_twist = ((-HEAD_TURN_LIMIT) - curr_rot_x);
				float max_twist = (HEAD_TURN_LIMIT - curr_rot_x);

				float min_tilt = ((-HEAD_TILT_LIMIT) - curr_rot_z);
				float max_tilt = (HEAD_TILT_LIMIT - curr_rot_z);

				//
				//	Clamp the rotations
				//
				desired_head_rotation.X = WWMath::Clamp( desired_head_rotation.X, min_twist, max_twist);
				desired_head_rotation.Z = WWMath::Clamp( desired_head_rotation.Z, min_tilt, max_tilt);
			}
		}

#define	HEAD_TURN_RATE		(DEG_TO_RAD( 360 )/2)
#define	HEAD_TILT_RATE		(DEG_TO_RAD( 180 )/2)
		float max_turn = HEAD_TURN_RATE * TimeManager::Get_Frame_Seconds();
		float max_tilt = HEAD_TILT_RATE * TimeManager::Get_Frame_Seconds();
		HeadRotation.X += WWMath::Clamp( (desired_head_rotation.X - HeadRotation.X), -max_turn, max_turn );
		HeadRotation.Z += WWMath::Clamp( (desired_head_rotation.Z - HeadRotation.Z), -max_tilt, max_tilt );

		Matrix3D	head(1);		
		head.Rotate_X( HeadRotation.X );
		head.Rotate_Z( HeadRotation.Z );
		if ( !Peek_Model()->Is_Bone_Captured( head_bone ) ) {
			Peek_Model()->Capture_Bone( head_bone );
		}
		WWASSERT( Peek_Model()->Is_Bone_Captured( head_bone ) );
		if ( Peek_Model()->Is_Bone_Captured( head_bone ) ) {
			Peek_Model()->Control_Bone( head_bone, head );
		}

		HeadRotation.Z = 0;
		if ( returning && HeadRotation.Length() > 0.001f ) {
			HeadLookDuration = 0.0001f;		// maybe done next time...
		}
	}
}

//------------------------------------------------------------------------------------
void	SoldierGameObj::Set_Blended_Animation( const char *animation_name, bool looping, float frame_offset, bool play_backwards )
{
	if ( animation_name == NULL ) {
		HumanState.Stop_Scripted_Animation();
		return;
	}

	// Humans ignore the start_frame parameter for now

	AnimationName = animation_name;
	HumanState.Start_Scripted_Animation( animation_name, true, looping );
	Get_Anim_Control()->Set_Mode( looping ? ANIM_MODE_LOOP : ANIM_MODE_ONCE, frame_offset );

	if ( play_backwards ) {
		HAnimClass *anim = Get_Anim_Control()->Peek_Animation();
		if ( anim != NULL ) {
			int frame_count = anim->Get_Num_Frames();
			Get_Anim_Control()->Set_Mode( ANIM_MODE_TARGET, frame_count - 1 );
			Get_Anim_Control()->Set_Target_Frame( 0 );
		}
	}

	return ;
}


//------------------------------------------------------------------------------------
void	SoldierGameObj::Set_Animation( const char *animation_name, bool looping, float /* start_frame */ )
{
	if ( animation_name == NULL ) {
//		Debug_Say(( "Stoping Scripted Human Animation\n" ));
		HumanState.Stop_Scripted_Animation();
// FIX		AnimControl->UnLock_Animation();
		return;
	}

	// Humans ignore the start_frame parameter for now

//	Debug_Say(( "Starting Scripted Human Animation %s\n", animation_name ));
	AnimationName = animation_name;
	HumanState.Start_Scripted_Animation( animation_name, false, looping );
}

//------------------------------------------------------------------------------------
void	SoldierGameObj::Start_Transition_Animation( const char * anim_name, TransitionCompletionDataStruct *data )
{
//	Debug_Say(( "Starting Human Transition Animation %s\n", anim_name ));
	//WWASSERT( TransitionCompletionData == NULL );
	TransitionCompletionData = data;
	AnimationName = anim_name;
	HumanState.Start_Transition_Animation( anim_name, false );
}

//------------------------------------------------------------------------------------
void SoldierGameObj::Set_Weapon_Model( const char *model_name )
{
	if ( WeaponRenderModel != NULL ) {		// remove old gun model
		if ( Peek_Model() != NULL ) {
			Peek_Model()->Remove_Sub_Object(WeaponRenderModel);		// Clean the bone
		}
		WeaponRenderModel->Release_Ref();
		WeaponRenderModel = NULL;
	}

	if ( (model_name != NULL) && (*model_name != 0) ) {
		//Debug_Say(( "Updating soldier Weapon model to %s!!!\n", model_name ));

		StringClass stripped_name(true);
		Get_Render_Obj_Name_From_Filename( stripped_name, model_name );
		WeaponRenderModel = SoldierGameObj::Find_RenderObj( stripped_name );
		if ( WeaponRenderModel == NULL ) {
			WeaponRenderModel = Create_Render_Obj_From_Filename( model_name );
			WWASSERT( WeaponRenderModel );
			SET_REF_OWNER( WeaponRenderModel );
			Add_RenderObj( WeaponRenderModel );
			WeaponRenderModel->Release_Ref();
		}
		WeaponRenderModel->Add_Ref();

		// Create and add new gun model

		if ( Peek_Model() != NULL ) {
			if ( WeaponRenderModel->Get_Container() != Peek_Model() ) {
				Peek_Model()->Add_Sub_Object_To_Bone( WeaponRenderModel, GUN_BONE_NAME );
			}
		}

		assert( Get_Weapon() );
		Get_Weapon()->Set_Model( WeaponRenderModel );	// let the weapon know who's firing him
	}
}

//------------------------------------------------------------------------------------
void SoldierGameObj::Set_Weapon_Animation( const char *anim_name )
{
	if ( WeaponAnimControl == NULL ) {
		WeaponAnimControl = new SimpleAnimControlClass;
		if (WeaponRenderModel != NULL ) {
			// Update Anim Control
			WeaponAnimControl->Set_Model( WeaponRenderModel );
		}
	}
	if ( WeaponAnimControl != NULL ) {
//		Debug_Say(( "Updating soldier gun anim to \"%s\"!!!\n", anim_name ));
		if ( ( anim_name != NULL ) && ( *anim_name != 0 ) ) {
			WeaponAnimControl->Set_Animation( anim_name );
		}
	}
}

void SoldierGameObj::Set_Back_Weapon_Model( const char *model_name )
{
	if ( BackWeaponRenderModel != NULL ) {		// remove old gun model
		if (Peek_Model() != NULL ) {
			Peek_Model()->Remove_Sub_Object(BackWeaponRenderModel);
		}
		BackWeaponRenderModel->Release_Ref();
		BackWeaponRenderModel = NULL;
	}

	if ( (model_name != NULL) && (*model_name != 0) ) {
		
		// Create and add back gun model
		StringClass stripped_name(true);
		Get_Render_Obj_Name_From_Filename( stripped_name, model_name );
		BackWeaponRenderModel = SoldierGameObj::Find_RenderObj( stripped_name );
		if ( BackWeaponRenderModel == NULL ) {
			BackWeaponRenderModel = Create_Render_Obj_From_Filename( model_name );
//			WWASSERT( BackWeaponRenderModel );
			if ( BackWeaponRenderModel != NULL ) {
				SET_REF_OWNER( BackWeaponRenderModel );
				Add_RenderObj( BackWeaponRenderModel );
				BackWeaponRenderModel->Release_Ref();
			}
		}

		if ( BackWeaponRenderModel != NULL ) {
			BackWeaponRenderModel->Add_Ref();

			if (Peek_Model() != NULL ) {
				Peek_Model()->Add_Sub_Object_To_Bone( BackWeaponRenderModel, BACK_GUN_BONE_NAME );
			}
		} else {
			Debug_Say(( "Missing Back Model %s", model_name ));
		}
	}
}

void SoldierGameObj::Set_Back_Flag_Model(const char *model_name,
	const Vector3 & /* tint */)
{
	if ( BackFlagRenderModel != NULL ) {		// remove old gun model
		if (Peek_Model() != NULL ) {
			Peek_Model()->Remove_Sub_Object(BackFlagRenderModel);
		}
		BackFlagRenderModel->Release_Ref();
		BackFlagRenderModel = NULL;
	}

	if ( (model_name != NULL) && (*model_name != 0) ) {
		
		// Create and add back gun model
		BackFlagRenderModel = Create_Render_Obj_From_Filename( model_name );
		if ( BackFlagRenderModel != NULL ) {
			SET_REF_OWNER( BackFlagRenderModel );

			/*
			extern void Tint(RenderObjClass *robj, const Vector3 & color);
			Tint(BackFlagRenderModel, tint);
			*/

			if (Peek_Model() != NULL ) {
				Peek_Model()->Add_Sub_Object_To_Bone( BackFlagRenderModel, BACK_GUN_BONE_NAME );
			}
		} else {
			Debug_Say(( "Missing Back Model %s", model_name ));
		}
	}
}

void	SoldierGameObj::Update_Back_Gun( void )
{
	//Debug_Say(("CommandoGameObj::Update_Back_Gun\n"));

	WeaponClass	*next_weapon = WeaponBag->Get_Next_Weapon();
	if ( next_weapon && ( next_weapon != Get_Weapon() ) ) {
		Set_Back_Weapon_Model( next_weapon->Get_Back_Model_Name() );
	} else {
		Set_Back_Weapon_Model( NULL );
	}
}

//------------------------------------------------------------------------------------
float		SoldierGameObj::Get_Weapon_Height( void )
{
	float height = 1.62f;		// set to be at about eye level
	if ( Is_Crouched() ) {
		height -= 0.56f;
	}
	return height;		// verticle offset, move to weapon
}

//------------------------------------------------------------------------------------
float		SoldierGameObj::Get_Weapon_Length( void )
{
	return 0.8f;		// forward offset, move to weapon
}


//------------------------------------------------------------------------------------
bool	SoldierGameObj::Internal_Set_Targeting( const Vector3 & target_pos, bool do_tilt )
{
	WWPROFILE( "Soldier Set Targeting" );

	if ( CombatManager::Is_Skeleton_Slider_Demo_Enabled() ) {
		return false;
	}

	SmartGameObj::Set_Targeting( target_pos );

	if ( ( Get_State() == HumanStateClass::DEATH ) ||
		  ( Get_State() == HumanStateClass::DESTROY ) ||
		  ( Get_State() == HumanStateClass::TRANSITION ) ||
		  ( Get_State() == HumanStateClass::LADDER ) ) {
//		Debug_Say(( "Ignore Set Target in Transition \n" ));
		return false;
	}

	if ( Get_State() == HumanStateClass::IN_VEHICLE ) {
		if ( Vehicle != NULL ) {
			if ( Vehicle->Get_Driver_Is_Gunner() ) {
				if ( Vehicle->Get_Driver() == this ) {
					Vehicle->Set_Targeting( target_pos );
				}
			} else {
				if ( Vehicle->Get_Driver() == this && Vehicle->Get_Gunner() == NULL ) {
					Vehicle->Set_Targeting( target_pos );
				} else if ( Vehicle->Get_Gunner() == this ) {
					Vehicle->Set_Targeting( target_pos );
				}
			}
		}
		return false;
	}

	Vector3 muzzle_pos;
	Get_Position( &muzzle_pos );
	muzzle_pos.Z += Get_Weapon_Height();
	Vector3	rel_target_pos = target_pos - muzzle_pos;
	
	// Set Tilt
	float dist = rel_target_pos.Length();
	float	tilt = 0;
	if ( dist && do_tilt ) {
		tilt = WWMath::Fast_Asin( rel_target_pos.Z / dist );
	}

//	tilt = 0;
//	Debug_Say(( "Tilt %f", tilt ));

	bool	is_complete = true;

	// Set Facing
	float cur_facing = Peek_Human_Phys()->Get_Heading();
	float facing = WWMath::Atan2( rel_target_pos.Y, rel_target_pos.X );
	float facing_dif = facing - cur_facing;
	if ( WWMath::Fabs(facing_dif) > 0.001f ) {
		WWPROFILE( "Facing" );
		facing_dif = WWMath::Wrap( facing_dif, DEG_TO_RADF( -180.0f ), DEG_TO_RADF( 180.0f ) );
//		Debug_Say(( "Must change %1.3f, from facing %1.3f to facing %1.3f\n", RAD_TO_DEG( facing_dif ), RAD_TO_DEG( cur_facing ), RAD_TO_DEG( facing ) ));

		float change = facing_dif;
		if ( !Is_Human_Controlled() ) {	// human players don't use turn limits
			float max_change = Get_Turn_Rate() * TimeManager::Get_Frame_Seconds();

			// Turn more slowly when turning a small amount
			if ( WWMath::Fabs( change ) < DEG_TO_RAD( 20 ) ) {
				max_change *= 0.3f;
			}

			change = WWMath::Clamp( facing_dif, -max_change, max_change );
		}

		if ( !Is_Human_Controlled() ) {	// human players don't use turn anims
  			HumanState.Set_Turn_Velocity( change );	// play the leg turning anim
		}

		is_complete = (change == facing_dif);	// are we facing?

		facing = cur_facing + change;
		facing = WWMath::Wrap( facing, DEG_TO_RADF( -180.0f ), DEG_TO_RADF( 180.0f ) );
		Peek_Human_Phys()->Set_Heading( facing );
	}

	if ( Is_Human_Controlled() && Get_State() != HumanStateClass::IN_VEHICLE ) {

#define		TILT_DOWN_SPEED		4.0
		float direction = -1;
		if ( Get_Weapon() && Get_Weapon()->Is_Reloading() ) {
			direction = 1;
		}

		ReloadingTilt += direction * TimeManager::Get_Frame_Seconds() * TILT_DOWN_SPEED;
		ReloadingTilt = WWMath::Clamp( ReloadingTilt, 0, 1 );

		if ( ReloadingTilt > 0 ) {
			tilt = WWMath::Lerp(	(float)tilt, DEG_TO_RADF( -90 ), ReloadingTilt );
		}
	}

	HumanState.Update_Aiming( tilt, 0 );	// no turn

//	facing = HumanState.Peek_Physical_Object()->Get_Heading();
//	Debug_Say(( "Soldier Facing %f Tilt %f\n", RAD_TO_DEG( facing ), RAD_TO_DEG( WeaponTilt ) ));
//	Debug_Say(( "rel_target_pos %f %f %f\n", rel_target_pos.X, rel_target_pos.Y, rel_target_pos.Z ));

	return is_complete;
}


//------------------------------------------------------------------------------------
bool	SoldierGameObj::Set_Targeting( const Vector3 & target_pos, bool do_tilt )
{
	bool retval = false;

	//
	//	Don't do the targetting if we are locked on an object
	//
	if ( FacingObject == NULL ) {
		retval = Internal_Set_Targeting( target_pos, do_tilt );
	}

	return retval;
}

//------------------------------------------------------------------------------------
RenderObjClass *	SoldierGameObj::Find_Head_Model( void )
{
	RenderObjClass *head_model = NULL;

	//
	//	Get a pointer to the HLod this object is using
	//
	RenderObjClass *model = Peek_Model ();
	if ( model != NULL && model->Class_ID () == RenderObjClass::CLASSID_HLOD ) {
		HLodClass *lod = reinterpret_cast<HLodClass *> ( model );
		
		//
		//	Try to find the head-bone
		//
		int bone_index	= lod->Get_Bone_Index ( "C HEAD" );
		int obj_count	= lod->Get_Num_Sub_Objects_On_Bone(bone_index);

		//
		//	Try to find a model on the head bone that uses the human head skeleton
		//
		for (int index = 0; index < obj_count; index ++) {
			RenderObjClass *render_obj = lod->Get_Sub_Object_On_Bone ( index, bone_index );
			if (render_obj != NULL) {
				const HTreeClass *htree = render_obj->Get_HTree();
				if ( htree != NULL ) {

					//
					//	Is this really a head model?
					//
					if ( ::stricmp (htree->Get_Name(), "S_A_HEAD") == 0 ||
						  ::stricmp (htree->Get_Name(), "S_B_HEAD") == 0)
					{
						head_model = render_obj;
						break;
					}
				}
				render_obj->Release_Ref ();
			}
		}
	}

	return head_model;
}


//------------------------------------------------------------------------------------
void	SoldierGameObj::Set_Emot_Icon ( const char *model_name, float duration )
{
	//
	//	Free the old emot icon
	//
	if ( EmotIconModel != NULL ) {

		if ( EmotIconModel->Is_In_Scene() ) {
			EmotIconModel->Remove ();
		}

		REF_PTR_RELEASE( EmotIconModel );
	}
	
	//
	//	Try to load the new emot icon model
	//
	if ( model_name != NULL ) {

		//
		//	Create the new model
		//
		EmotIconModel = ::Create_Render_Obj_From_Filename( model_name );
		if ( EmotIconModel != NULL ) {

			//
			//	Determine what the animation name for the model is
			//
			const char *name = EmotIconModel->Get_Name ();
			StringClass anim_name;
			anim_name.Format ("%s.%s", name, name);

			//
			//	Animate the model
			//
			HAnimClass *anim = WW3DAssetManager::Get_Instance ()->Get_HAnim( anim_name );
			if ( anim != NULL ) {
				EmotIconModel->Set_Animation( anim, 0.0F, RenderObjClass::ANIM_MODE_LOOP );
				REF_PTR_RELEASE( anim );
			}
			
			//
			//	Put the icon a few meters above the soldier's head
			//
			Matrix3D tm = Get_Transform();
			tm.Set_Translation (tm.Get_Translation() + Vector3 (0, 0, EMOT_ICON_HEIGHT));
			EmotIconModel->Set_Transform( tm );
			COMBAT_SCENE->Add_Render_Object( EmotIconModel );
			EmotIconTimer = duration;
		}
	}
	
	return ;
}


//------------------------------------------------------------------------------------
void	SoldierGameObj::Say_Dialogue( int dialog_id )
{
	WWASSERT( dialog_id >= 0 && dialog_id < DIALOG_MAX );

	//
	//	Lookup the conversation we are starting
	//	
	int conversation_id = DialogList[dialog_id].Get_Conversation();
	if ( conversation_id > 0 ) {

		//
		//	Start the conversation
		//
		ActiveConversationClass *conversation = ConversationMgrClass::Start_Conversation( this, conversation_id, true );
		REF_PTR_RELEASE (conversation);
	}
	
	return ;
}

//------------------------------------------------------------------------------------
float SoldierGameObj::Say_Dynamic_Dialogue
(
	int						text_id,
	SoldierGameObj *		speaker,
	AudibleSoundClass **	sound_obj_to_return 
)
{
	float duration	= 2.0F;
		
	//
	//	Lookup the translation object from the strings database
	//
	TDBObjClass *translate_obj = TranslateDBClass::Find_Object( text_id );
	if (translate_obj != NULL) {
		
		const unichar_t *string		= translate_obj->Get_String ();
		int sound_def_id			= (int)translate_obj->Get_Sound_ID ();
		const char *anim_name	= translate_obj->Get_Animation_Name ();

		//
		//	If this soldier was still talking from a previos call, then
		// kill the old sound
		//
		if ( speaker != NULL ) {
			speaker->Stop_Current_Speech();
		}

		//
		//	Play the sound effect
		//
		bool display_text = true;
		if ( sound_def_id > 0 ) {
			
			//
			//	Create the sound object
			//
			AudibleSoundClass *speech = WWAudioClass::Get_Instance ()->Create_Sound( sound_def_id );
			if ( speech != NULL ) {
				duration = (speech->Get_Duration() / 1000.0F);

				//
				//	Return the sound object to the caller as necessary
				//
				if ( sound_obj_to_return != NULL ) {
					(*sound_obj_to_return) = speech;
					(*sound_obj_to_return)->Add_Ref();
				}

				//
				//	Either play the sound at the speaker's location
				// or just play it as a 2D sample.
				//
				if ( speaker != NULL ) {
					speech->Set_Transform( speaker->Get_Transform() );
					speech->Add_To_Scene();
					display_text = (speech->Is_Sound_Culled() == false);
				} else {
					
					//
					//	Play the sound
					//
					speech->Play();
				}
			}

			//
			//	Update the speaker's internal sound object pointer
			//
			if ( speaker != NULL ) {
				speaker->CurrentSpeech = speech;
			} else {
				REF_PTR_RELEASE (speech);
			}
		}

		//
		//	Check to see if this string is commented out.
		//
		if( string != NULL && string[0] == U_CHAR('/') && string[1] == U_CHAR('/') ) {
			string			+= 2;
			display_text	= false;
		}

		//
		//	Display the text on the screen
		//
		if ( display_text && string != NULL && string[0] != 0 ) {
			float message_duration = std::max( duration, 5.0F );
			CombatManager::Get_Message_Window ()->Add_Message( string, Vector3 (1, 1, 1),
																speaker, message_duration );
		}
		
		//
		//	Play the facial animation (if one exists)
		//
		if ( speaker != NULL && speaker->HeadModel != NULL) {
			HAnimClass *head_anim = NULL;

			//
			//	Dig out the skeleton name for the head model
			//
			StringClass anim_root_name;
			const HTreeClass *htree = speaker->HeadModel->Get_HTree ();
			if (htree != NULL) {
				anim_root_name = htree->Get_Name ();
				anim_root_name += ".";
			}
			
			//
			//	Determine if we should use a pre-canned animation, or
			// if we should build one dynamically
			//
			if ( anim_name == NULL || anim_name[0] == 0 ) {
				
				//
				//	Build an animation dynamically
				//
				const char *english_string = translate_obj->Get_English_String();
				if ( english_string != NULL && english_string[0] != 0 ) {
					if (speaker->SpeechAnim->Generate_Animation( english_string, duration )) {
						head_anim = speaker->SpeechAnim;
					}
				}
				
			} else {
				
				//
				//	Build a string containing the fully qualified animation name
				//
				StringClass full_anim_name;
				if ( ::strchr( anim_name, '.' ) == 0 ) {
					full_anim_name	= anim_root_name;
					full_anim_name	+= anim_name;
				} else {
					full_anim_name = anim_name;
				}

				head_anim = WW3DAssetManager::Get_Instance()->Get_HAnim( full_anim_name );
			}

			//
			//	Recreate the facial animation when the speech is complete
			//
			speaker->GenerateIdleFacialAnimTimer = duration + 1.0F;
			
			//
			//	Play the animation on the head model (if one exists)
			//
			if ( speaker->HeadModel != NULL && head_anim != NULL ) {
				speaker->HeadModel->Set_Animation( head_anim, 0, RenderObjClass::ANIM_MODE_ONCE );
			}			
		}
	}

	return duration;
}


//------------------------------------------------------------------------------------
void	SoldierGameObj::Stop_Current_Speech( void )
{
	//
	//	Kill the current sound we are making (speech, scream, grunt, etc)
	//
	if ( CurrentSpeech != NULL ) {
		GenerateIdleFacialAnimTimer = 0;
		CurrentSpeech->Stop();
		CurrentSpeech->Remove_From_Scene();
		CurrentSpeech->Release_Ref();
		CurrentSpeech = NULL;
	}
	
	return ;
}


//------------------------------------------------------------------------------------
void	SoldierGameObj::Apply_Damage( const OffenseObjectClass & damager, float scale, int alternate_skin )
{
	if ( !Is_In_Vehicle() ) {
		SmartGameObj::Apply_Damage( damager, scale, alternate_skin );
	}
}

//------------------------------------------------------------------------------------
void	SoldierGameObj::Apply_Damage_Extended( const OffenseObjectClass & damager, float scale,
			const	Vector3 & direction, const char * collision_box_name )
{
	// If negative damage, just apply
	if ( Get_Defense_Object()->Is_Repair( damager ) ) {
//	if ( damager.Get_Damage() * scale < 0 ) {
		SmartGameObj::Apply_Damage_Extended( damager, scale, direction, collision_box_name );

		// Add electric effect
		if ( HealingEffect == NULL ) {
			HealingEffect = CombatMaterialEffectManager::Get_Health_Effect();
			if ( HealingEffect != NULL ) {
				Peek_Human_Phys()->Add_Effect_To_Me( HealingEffect );
			}
		}

		if ( HealingEffect != NULL ) {
			HealingEffect->Set_Target_Parameter(0.495f);	// go almost halfway
		}

		return;
	}

	// Do this after the repair effect
/*	if ( !CombatManager::I_Am_Server() ) {		Clients can apply damage
		return;
	}*/

	if ( Is_In_Vehicle() ) {
		return;
	}

	bool ambushed = false;
	// Ambush Kill
	if ( ( Get_AI_State() == AI_STATE_IDLE ) || ( Get_AI_State() == AI_STATE_SECONDARY_IDLE ) ) {
		// Only ambush if there is an owner to the damage, and the target is human controlled
		if ( damager.Get_Owner() != NULL && !Is_Human_Controlled() ) {
			scale *= AMBUSH_DAMAGE_SCALE;
			ambushed = true;
//			Debug_Say(( "Ambush Damage!!\n" ));
		}
	}

	bool anim_ok = true;

	Reset_Hibernating();		// Wake up when damaged

	int ouch_type = Get_Ouch_Type( direction, collision_box_name );

	// Stats
	PlayerDataClass * damager_data = NULL;
	if ( damager.Get_Owner() && damager.Get_Owner()->As_SoldierGameObj() ) {
		damager_data = damager.Get_Owner()->As_SoldierGameObj()->Get_Player_Data();
	}

	// Dont add stats for "non-directional" damage.  E.g. standing in a tiberium
	// field should not count as thousands of hits.
	if (direction.Length2() > WWMATH_EPSILON) {
		switch( ouch_type ) {
			case HumanStateClass::HEAD_FROM_BEHIND:
			case HumanStateClass::HEAD_FROM_FRONT:
				if ( Get_Player_Data() != NULL ) {
					Get_Player_Data()->Stats_Add_Head_Hit();
				}
				if ( damager_data != NULL ) {
					damager_data->Stats_Add_Head_Shot();
				}
				break;

			case HumanStateClass::TORSO_FROM_BEHIND:
			case HumanStateClass::TORSO_FROM_FRONT:
				if ( Get_Player_Data() != NULL ) {
					Get_Player_Data()->Stats_Add_Torso_Hit();
				}
				if ( damager_data != NULL ) {
					damager_data->Stats_Add_Torso_Shot();
				}
				break;

			case HumanStateClass::LEFT_ARM_FROM_BEHIND:
			case HumanStateClass::LEFT_ARM_FROM_FRONT:
			case HumanStateClass::RIGHT_ARM_FROM_BEHIND:
			case HumanStateClass::RIGHT_ARM_FROM_FRONT:
				if ( Get_Player_Data() != NULL ) {
					Get_Player_Data()->Stats_Add_Arm_Hit();
				}
				if ( damager_data != NULL ) {
					damager_data->Stats_Add_Arm_Shot();
				}
				break;

			case HumanStateClass::LEFT_LEG_FROM_BEHIND:
			case HumanStateClass::LEFT_LEG_FROM_FRONT:
			case HumanStateClass::RIGHT_LEG_FROM_BEHIND:
			case HumanStateClass::RIGHT_LEG_FROM_FRONT:
				if ( Get_Player_Data() != NULL ) {
					Get_Player_Data()->Stats_Add_Leg_Hit();
				}
				if ( damager_data != NULL ) {
					damager_data->Stats_Add_Leg_Shot();
				}
				break;

			case HumanStateClass::GROIN:
				if ( Get_Player_Data() != NULL ) {
					Get_Player_Data()->Stats_Add_Crotch_Hit();
				}
				if ( damager_data != NULL ) {
					damager_data->Stats_Add_Crotch_Shot();
				}
				break;
		}
	}

	//	Determine what (if any) dialogue to say
	int dialogue_id = -1;
	ArmedGameObj *damage_owner = damager.Get_Owner();
	if( damage_owner != NULL ) {
		
		//	Were we injured by a friend or foe?
		if ( Is_Enemy( damage_owner ) ) {
			dialogue_id = DIALOG_ON_TAKE_DAMAGE_FROM_ENEMY;
		} else {
			dialogue_id = DIALOG_ON_TAKE_DAMAGE_FROM_FRIEND;
		}		
	}

	float health_before = Get_Defense_Object()->Get_Health();
	float armor_before = Get_Defense_Object()->Get_Shield_Strength();

	if ( collision_box_name != NULL ) {
		const char * start = ::strchr( collision_box_name, '.' );
		if ( start != NULL ) {
			start++;
			float bone_scale = BonesManager::Get_Bone_Damage_Scale( start );
			scale *= bone_scale;
		}
	}

	// Update damage indicators
	if ( this == COMBAT_STAR ) {
		if ( Get_Defense_Object()->Would_Damage( damager ) && !damager.ForceServerDamage ) {
			if ( direction.Length() == 0 ) {
				for ( int i = 0; i < 8; i++ ) {
					CombatManager::Show_Star_Damage_Direction( i );
				}
			// Set all
			} else {
				Vector3 relative_direction = Get_Transform().Inverse_Rotate_Vector( direction );
				// Convert direction into 0 .. 7
				float angle = ::WWMath::Atan2 ( relative_direction.Y, -relative_direction.X);
				int dir = (int)(8.0f * angle / DEG_TO_RAD( 360 ) + 8.5f);
				CombatManager::Show_Star_Damage_Direction( dir & 7 );
			}
		}
	}


	SmartGameObj::Apply_Damage_Extended( damager, scale, direction, collision_box_name );
	float health = Get_Defense_Object()->Get_Health();

	if ( this == COMBAT_STAR ) {
		Vector3 pos;
		COMBAT_STAR->Get_Position( &pos );
		int hitter_id = 0;
		const char * weapon_name = "";
		if ( damager.Get_Owner() ) {
			hitter_id = damager.Get_Owner()->Get_ID();
			if ( damager.Get_Owner()->Get_Weapon() ) {
				weapon_name = damager.Get_Owner()->Get_Weapon()->Get_Definition()->Get_Name();
			}
		}
		const char * body_part = ( collision_box_name != NULL ) ? collision_box_name : "";
		float armor = Get_Defense_Object()->Get_Shield_Strength();
	   DIAG_LOG(( "DRCE", "%s; %d; %s; %1.2f; %1.2f; %1.2f; %1.2f; %1.2f", weapon_name, hitter_id, body_part, armor, health, pos.X, pos.Y, pos.Z ));
	}

	if (( damager.Get_Owner() == COMBAT_STAR ) && ( COMBAT_STAR != NULL )) {
		Vector3 pos;
		COMBAT_STAR->Get_Position( &pos );
		const char * weapon_name = "";
		int ammo = 0;
		if ( COMBAT_STAR->Get_Weapon() ) {
			weapon_name = COMBAT_STAR->Get_Weapon()->Get_Definition()->Get_Name();
			ammo = COMBAT_STAR->Get_Weapon()->Get_Total_Rounds();
		}
		int hittee_id = Get_ID();
		const char * body_part = ( collision_box_name != NULL ) ? collision_box_name : "";
		float armor = Get_Defense_Object()->Get_Shield_Strength();
		Vector3 victim_pos;
		Get_Position( &victim_pos );
		const char * team_name = Player_Type_Name( Get_Player_Type() );
	   DIAG_LOG(( "DEFE", "%1.2f; %1.2f; %1.2f; %s; %d; %d; %s; %1.2f; %1.2f; %1.2f; %1.2f; %1.2f; %s ", pos.X, pos.Y, pos.Z, weapon_name, ammo, hittee_id, body_part, armor, health, victim_pos.X, victim_pos.Y, victim_pos.Z, team_name ));
	}

	if ( ambushed ) {
		int victim_id = Get_ID();
		Vector3 victim_pos;
		Get_Position( &victim_pos );
		Vector3 shooter_pos;
		damager.Get_Owner()->Get_Position( &shooter_pos );
		float armor = Get_Defense_Object()->Get_Shield_Strength();
		DIAG_LOG(( "AMGE", "%1.2f; %1.2f; %1.2f; %d; %1.2f; %1.2f; %1.2f; %1.2f; %1.2f", victim_pos.X, victim_pos.Y, victim_pos.Z, victim_id, shooter_pos.X, shooter_pos.Y, shooter_pos.Z, armor, health ));
	}


	if (health == health_before && Get_Defense_Object()->Get_Shield_Strength() == armor_before) {
		//
		// No damage was applied. 
		// This is a hack. 
		//

		// Handle zero health bug
		if ( health > 0 || 
				HumanState.Get_State() == HumanStateClass::DEATH || 
				HumanState.Get_State() == HumanStateClass::DESTROY ) {
			return;
		} else {
			Debug_Say(( "Solving Zero health Bug %f %d\n", health, HumanState.Get_State() ));
		}
	}

	// Can't start special damage unless damage was done.

	// Check for special damage
	int warhead = damager.Get_Warhead();
	ArmorWarheadManager::SpecialDamageType special_damage = ArmorWarheadManager::Get_Special_Damage_Type( warhead );
	if ( special_damage != 0 ) {
		float probability = ArmorWarheadManager::Get_Special_Damage_Probability( warhead );
		if (health <= 0) {
			probability = 1;
		}
		if ( FreeRandom.Get_Float() < probability ) {
			// if skin is not impervious to the damage
			int skin = Get_Defense_Object()->Get_Skin();
			ArmorWarheadManager::SpecialDamageType special_damage = ArmorWarheadManager::Get_Special_Damage_Type( warhead );
			if ( !ArmorWarheadManager::Is_Skin_Impervious( special_damage, skin ) ) {
				Set_Special_Damage_Mode( special_damage, damager.Get_Owner() );
			}
		}
	}

	if ( this == COMBAT_STAR ) {
		if ( 	_shake_delay < 0 ) {
			_shake_delay = 0.6f;		// don't shake/sound too fast

			// And Shake the camera
			Vector3	pos = COMBAT_CAMERA->Get_Transform().Get_Translation();
			COMBAT_SCENE->Add_Camera_Shake(	pos,	1, 0.5f, 0.05f );
		}

		anim_ok = false;
		if ( 	_cry_delay < 0 ) {
			_cry_delay = 2;		// don't shake/sound too fast
			anim_ok = true;

			// Play ouch sound
			AudibleSoundClass * sound = WWAudioClass::Get_Instance()->Create_Sound( "Take_Damage_Sound" );
			if ( sound ) {
				sound->Play();
				sound->Release_Ref();
			}
		}

	}


	if ( health <= 0 ) {

		// Check for creating visceroids
		if ( IS_MISSION ) {
			int warhead = damager.Get_Warhead();
			float visceroid_probability = ArmorWarheadManager::Get_Visceroid_Probability( warhead );
			if ( !Is_Human_Controlled() && visceroid_probability != 0 ) {
				if ( FreeRandom.Get_Float() < visceroid_probability ) {

					#define	VISCEROID_NAME			"Visceroid"
					// Not from other visceroids
					if ( ::stricmp( Get_Definition().Get_Name(), VISCEROID_NAME ) != 0 ) {
						// Create a visceroid
						PhysicalGameObj * vis = ObjectLibraryManager::Create_Object( VISCEROID_NAME );
						if ( vis != NULL ) {
							vis->Set_Transform( Get_Transform() );
							vis->Start_Observers ();
							Set_Delete_Pending();		// Kill me
							Debug_Say(( "Made Visceroid\n" ));
						}
					}
				}
			}
		}

		Enable_Hibernation( false );

		// Stats
		SoldierGameObj * damager_owner = NULL;
		if ( damager.Get_Owner() && damager.Get_Owner()->As_SoldierGameObj() ) {
			damager_owner = damager.Get_Owner()->As_SoldierGameObj();
		}
		if ( damager_owner && damager_owner->Get_Player_Data() ) {

			if ( Is_Teammate( damager_owner ) ) {
				damager_owner->Get_Player_Data()->Stats_Add_Ally_Killed();
			}

			if ( Is_Enemy( damager_owner ) ) {
				damager_owner->Get_Player_Data()->Stats_Add_Enemy_Killed();
			}

			if ( damager_owner->Get_Vehicle() ) {
				damager_owner->Get_Player_Data()->Stats_Add_Kill_From_Vehicle();
			}
		}

		if ( HumanState.Get_State() != HumanStateClass::DEATH ) {

			if ( this == COMBAT_STAR ) {
				CombatManager::Register_Star_Killer( damager.Get_Owner() );
			}

			// Play death animation
			HumanState.Set_State( HumanStateClass::DEATH, ouch_type );

			//
			//	Stop any sound we WERE making
			//
			Stop_Current_Speech ();

			int death_sound_id = Get_Definition ().DeathSoundPresetID;
			if (death_sound_id == 0) {

				//
				//	Lookup the global death sound
				//
				GlobalSettingsDef *global_settings = GlobalSettingsDef::Get_Global_Settings ();
				if (global_settings != NULL) {
					death_sound_id = global_settings->Get_Death_Sound_ID();
				}				
			}

			//
			//	Reveal this object to the player's encyclopedia
			//
			if ( damager.Get_Owner () == COMBAT_STAR ) {
				EncyclopediaMgrClass::Reveal_Object( this );
			}

			//
			//	Play the death sound
			//
			if (death_sound_id != 0) {
				Matrix3D tm = Get_Transform();
				RefCountedGameObjReference *owner_ref = new RefCountedGameObjReference(this);
				WWAudioClass::Get_Instance()->Create_Instant_Sound( death_sound_id, tm, owner_ref );
				REF_PTR_RELEASE(owner_ref);
			}


			// Play death explosion, since we don't call Completely_Damaged
			if ( Get_Definition().KilledExplosion != 0 ) {
				Vector3 pos;
				Get_Position(&pos);
				WWASSERT(pos.Is_Valid());// most likely candidate for explosion damage bug....?
				ExplosionManager::Create_Explosion_At( Get_Definition().KilledExplosion, pos, damager.Get_Owner() );	 // no one gets credit for this
			}

			//
			//	Queue the guy who killed us...
			//
			if (	damage_owner != NULL &&
					damage_owner != this &&
					damage_owner->As_SoldierGameObj () != NULL )
			{
				SoldierGameObj *our_killer = damage_owner->As_SoldierGameObj  ();
				
				//
				//	Were we killed by a friend or foe?
				//
				if ( Is_Enemy( damage_owner ) ) {			
					our_killer->Say_Dialogue( DIALOG_ON_KILLED_ENEMY );
				} else {
					our_killer->Say_Dialogue( DIALOG_ON_KILLED_FRIEND );
				}		
			}

		}

	} else {
		// Play wound animation
		// If state is interruptable.....
		if ( Get_State() == HumanStateClass::UPRIGHT && anim_ok ) {
			HumanState.Set_State( HumanStateClass::WOUNDED, ouch_type );
		}
	}

	if( CurrentSpeech == NULL && dialogue_id >= 0 ) {
		Say_Dialogue( dialogue_id );
	}
}


//------------------------------------------------------------------------------------
void	SoldierGameObj::Exit_Ladder( void ) 
{
	HumanState.Set_State( HumanStateClass::UPRIGHT );
}

//------------------------------------------------------------------------------------
void	SoldierGameObj::Enter_Ladder( bool top ) 
{
	HumanState.Set_State( HumanStateClass::LADDER );

	if ( Is_Human_Controlled() ) {
		if ( top ) {
			LadderUpMask = true;
		} else {
			LadderDownMask = true;
		}
	}
}

//------------------------------------------------------------------------------------
HumanPhysClass	*SoldierGameObj::Peek_Human_Phys( void ) const 
{ 
	WWASSERT( Peek_Physical_Object() ); 
	WWASSERT( Peek_Physical_Object()->As_HumanPhysClass() ); 
	return Peek_Physical_Object()->As_HumanPhysClass(); 
}

//------------------------------------------------------------------------------------
/*
** Vehicles
*/
void	SoldierGameObj::Enter_Vehicle( VehicleGameObj * vehicle, const char * anim_name )
{
//	WWASSERT( Get_State() != HumanStateClass::IN_VEHICLE );

	Vehicle = vehicle;
	HumanState.Set_State( HumanStateClass::IN_VEHICLE );

	AnimationName = anim_name;
	HumanState.Force_Animation( anim_name, false );

	if ( this == COMBAT_STAR && vehicle != NULL ) {
		Vector3 pos;
		Vehicle->Get_Position( &pos );
		int ammo = 0;
		if ( vehicle->Get_Weapon() ) {
			ammo = vehicle->Get_Weapon()->Get_Total_Rounds();
		}
		DefenseObjectClass * defense = vehicle->Get_Defense_Object();
		DIAG_LOG(( "ENVE", "%1.2f; %1.2f; %1.2f; %1.2f; %1.2f; %d", pos.X, pos.Y, pos.Z, defense->Get_Shield_Strength(), defense->Get_Health(), ammo ));
	}
}

//------------------------------------------------------------------------------------
void	SoldierGameObj::Exit_Vehicle( void ) 
{
	if ( this == COMBAT_STAR && Vehicle  != NULL ) {
		Vector3 pos;
		Vehicle->Get_Position( &pos );
		int ammo = 0;
		if ( Vehicle->Get_Weapon() ) {
			ammo = Vehicle->Get_Weapon()->Get_Total_Rounds();
		}
		DefenseObjectClass * defense = Vehicle->Get_Defense_Object();
		DIAG_LOG(( "EXVE", "%1.2f; %1.2f; %1.2f; %1.2f; %1.2f; %d", pos.X, pos.Y, pos.Z, defense->Get_Shield_Strength(), defense->Get_Health(), ammo ));
	}

	Vehicle = NULL;
	HumanState.Set_State( HumanStateClass::UPRIGHT );
}

void	SoldierGameObj::Exit_Destroyed_Vehicle( int seat_num, const Vector3 & vehicle_pos )
{
	Vehicle = NULL;
	HumanState.Set_State( HumanStateClass::UPRIGHT );

	Vector3 extent = Peek_Human_Phys()->Get_Collision_Box().Extent;
	float width = MAX( extent.X, extent.Y ) * 1.1f;

	if ( Peek_Model() ) {
		Peek_Model()->Set_Hidden( false );
	}

	Vector3 offsets[4] = {
		Vector3( -1.0f, -1.0f, 0 ),
		Vector3( -1.0f,  1.0f, 0 ),
		Vector3(  1.0f, -1.0f, 0 ),
		Vector3(  1.0f,  1.0f, 0 ),
	};

	Vector3 pos = vehicle_pos + offsets[ seat_num & 3 ] * width;

	Matrix3D tm( pos );
	if ( Peek_Human_Phys()->Can_Teleport( tm ) ) {
		Peek_Human_Phys()->Set_Position( pos );
	} else {
		Vector3 new_pos = pos;
		Peek_Human_Phys()->Find_Teleport_Location( pos, 4, &new_pos );
		Peek_Human_Phys()->Set_Position( new_pos );
	}

}

//------------------------------------------------------------------------------------
bool SoldierGameObj::Is_Permitted_To_Enter_Vehicle(void)
{
	if (!CombatManager::Is_Gameplay_Permitted()) {
		//
		// You can't enter a vehicle if there are no opponents, etc
		//
		return false;
	}

	/*
	//
	// A soldier cannot enter a vehicle if he is carrying a flag
	// 	
	if (CtfTeamFlag == NO_FLAG) {
		return true;
	} else {
		return false;
	}
	*/

	return true;
}

//------------------------------------------------------------------------------------
void SoldierGameObj::Get_Velocity(Vector3 & vel)
{
	Peek_Human_Phys()->Get_Velocity(&vel);
}

//------------------------------------------------------------------------------------
void SoldierGameObj::Set_Velocity(Vector3 & vel)
{
	Peek_Human_Phys()->Set_Velocity(vel);
}

//------------------------------------------------------------------------------------
void SoldierGameObj::Give_All_Weapons(void)
{
   //WWASSERT(CombatManager::I_Am_Server());
//	WWASSERT(PlayerType != PLAYERTYPE_NEUTRAL);

	// For all weapon defs with the AGiveWeaponsWeapon checked, Give It!
	for ( WeaponDefinitionClass *weapon_def = (WeaponDefinitionClass *)DefinitionMgrClass::Get_First( CLASSID_DEF_WEAPON );
			weapon_def != NULL;
			weapon_def = (WeaponDefinitionClass *)DefinitionMgrClass::Get_Next( weapon_def, CLASSID_DEF_WEAPON ) ) {
		if ( weapon_def->AGiveWeaponsWeapon ) {
			WeaponBag->Add_Weapon( weapon_def->Get_Name(), -1 );
		}
	}
}

//------------------------------------------------------------------------------------
VehicleGameObj	*	SoldierGameObj::Get_Profile_Vehicle( void )
{
	if (( Get_State() == HumanStateClass::IN_VEHICLE ) && Vehicle ) {
		return Vehicle;
	}

	if (( Get_State() == HumanStateClass::TRANSITION ) && 
		( TransitionCompletionData != NULL )) {
		if ( TransitionCompletionData->Type == TransitionDataClass::VEHICLE_ENTER ) {
			return (VehicleGameObj *)TransitionCompletionData->Vehicle.Get_Ptr();
		}
	}

	return NULL;
}

bool	SoldierGameObj::Use_Ladder_View( void )
{
	if ( Get_State() == HumanStateClass::LADDER ) {
			return true;
	}

	if (( Get_State() == HumanStateClass::TRANSITION ) && 
		( TransitionCompletionData != NULL )) {
		if (( TransitionCompletionData->Type == TransitionDataClass::LADDER_ENTER_TOP ) ||
			 ( TransitionCompletionData->Type == TransitionDataClass::LADDER_ENTER_BOTTOM )) {
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------------
void SoldierGameObj::Set_Model(const char *model_name)
{
	Peek_Physical_Object()->Set_Model_By_Name(model_name);
	HumanState.Set_Anim_Control( (HumanAnimControlClass *)Get_Anim_Control() );  // Must set the anim control after the phys object
}

//------------------------------------------------------------------------------------

Vector3	SoldierGameObj::Get_Bullseye_Position( void )
{
	if ( Get_Vehicle() != NULL ) {
		return Get_Vehicle()->Get_Bullseye_Position();
	}

	Vector3 pos;
	Get_Position(&pos);
	if ( Is_Crouched() ) {
		pos.Z += 0.5f;
	} else {
		pos.Z += Get_Bullseye_Offset_Z();
	}
//	pos.Z += 1.2f;
	return pos;
}

//------------------------------------------------------------------------------------

bool SoldierGameObj::Can_See(SoldierGameObj * p_soldier)
{
	WWASSERT(p_soldier != NULL);

	Vector3 ray_start = Get_Bullseye_Position();
	Vector3 ray_end = p_soldier->Get_Bullseye_Position();	// This may a big high

	Vector3 path = ray_end - ray_start;
	if (path.Length() > 1) {
		Vector3 offset = path;
		offset.Normalize();
		ray_start += offset;
	}

	LineSegClass ray(ray_start, ray_end);
	CastResultStruct result;
	PhysRayCollisionTestClass raytest(ray, &result, 
		BULLET_COLLISION_GROUP, COLLISION_TYPE_PHYSICAL);
	WWASSERT(COMBAT_SCENE != NULL);
{ WWPROFILE( "Cast Ray" );
	COMBAT_SCENE->Cast_Ray(raytest);
}

	bool can_see = false;
	
	if (raytest.CollidedPhysObj != NULL) {
		SmartGameObj * p_blocker = NULL;

		if ( raytest.CollidedPhysObj->Get_Observer() != NULL ) {
			PhysicalGameObj * p_obj = ((CombatPhysObserverClass *)raytest.CollidedPhysObj->Get_Observer())->As_PhysicalGameObj();
			if ( p_obj != NULL ) {
				p_blocker = p_obj->As_SmartGameObj();
			}
		}


		if (p_blocker != NULL && p_blocker->Get_Control_Owner() == 
			p_soldier->Get_Control_Owner()) {
			can_see = true;
		}
	}

	return can_see;
}

//------------------------------------------------------------------------------------
void	SoldierGameObj::Adjust_Skeleton( float height, float width )
{
//	Debug_Say(( "Height %f, width %f\n", height, width ));

	// Only adjust male skeletons
	Animatable3DObjClass * robj = (Animatable3DObjClass *)Peek_Model();
	if ( !robj || !robj->Get_HTree() || robj->Get_HTree()->Get_Name()[2] != 'A' ) {
		return;
	}

	HTreeClass	* tree_base = NULL;
	HTreeClass	* tree_tall = NULL;
	HTreeClass	* tree_wide = NULL;

	if ( tree_base == NULL ) {
		tree_base = WW3DAssetManager::Get_Instance()->Get_HTree( "s_a_human" );
		tree_tall = WW3DAssetManager::Get_Instance()->Get_HTree( "s_a_tall" );
		tree_wide = WW3DAssetManager::Get_Instance()->Get_HTree( "s_a_wide" );
	}

	if ( ( tree_base != NULL ) && ( tree_tall != NULL ) && ( tree_wide != NULL ) ) {

		HTreeClass *tree = HTreeClass::Create_Interpolated( tree_base, tree_tall, tree_wide,
																			 height, width );
		if ( tree ) {
			robj->Set_HTree( tree );
			delete tree;
		}
	}
}

/*
//------------------------------------------------------------------------------------
void SoldierGameObj::Set_Ctf_Team_Flag(int team) 
{
	WWASSERT(
		team == NO_FLAG			|| 
		team == PLAYERTYPE_NOD	||
		team == PLAYERTYPE_GDI);

	CtfTeamFlag = team;

	Set_Object_Dirty_Bit( NetworkObjectClass::BIT_OCCASIONAL, true );

	if (team == NO_FLAG) {
		Set_Back_Flag_Model(NULL);
	} else {
		//Set_Back_Flag_Model("O_Flag.w3d", Get_Team_Color());
		Set_Back_Flag_Model("O_Flag.w3d", Get_Color_For_Team(team));
	}

}
*/

/*
//------------------------------------------------------------------------------------
void SoldierGameObj::Set_Precision(void)
{
	cEncoderList::Set_Precision(BITPACK_CTF_TEAM_FLAG,	NO_FLAG, (int) PLAYERTYPE_GDI);
}
*/

//------------------------------------------------------------------------------------
SoldierObserverClass	*	SoldierGameObj::Get_Innate_Controller( void )
{
	if ( Get_Definition().UseInnateBehavior ) {
		const GameObjObserverList & observer_list = Get_Observers();
		for( int index = 0; index < observer_list.Count(); index++ ) {
			if ( !strcmp( observer_list[ index ]->Get_Name(), "Innate Soldier" )  ) {
				return (SoldierObserverClass	*)observer_list[ index ];
			}
		}
	}

	return NULL;
}

//------------------------------------------------------------------------------------
const char * SoldierGameObj::Get_First_Person_Hands_Model_Name( void )
{
	return Get_Definition().FirstPersonHands;
}

//------------------------------------------------------------------------------------
void	SoldierGameObj::Get_Information( StringClass & string )
{
	SmartGameObj::Get_Information( string );

	HumanState.Get_Information( string );

	SoldierObserverClass	*	innate = Get_Innate_Controller();
	if ( innate ) {
		innate->Get_Information( string );

		StringClass disabled;
		if (!Is_Innate_Enabled(SOLDIER_INNATE_EVENT_BULLET_HEARD)) {
			disabled += "B";
		}
		if (!Is_Innate_Enabled(SOLDIER_INNATE_EVENT_GUNSHOT_HEARD)) {
			disabled += "G";
		}
		if (!Is_Innate_Enabled(SOLDIER_INNATE_EVENT_FOOTSTEP_HEARD)) {
			disabled += "F";
		}
		if (!Is_Innate_Enabled(SOLDIER_INNATE_EVENT_ENEMY_SEEN)) {
			disabled += "S";
		}
		if (!Is_Innate_Enabled(SOLDIER_INNATE_ACTIONS)) {
			disabled += "A";
		}

		if ( !disabled.Is_Empty() ) {
			string += "Disabled:";
			string += disabled;
			string += "\n";
		}
	}
}

//------------------------------------------------------------------------------------
//void SoldierGameObj::Get_Extended_Information(StringClass & description)
void SoldierGameObj::Get_Description(StringClass & description)
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

	WeaponClass	* p_weapon = Get_Weapon();
	if (p_weapon != NULL) {
		line.Format("WEAP:  %s\n", p_weapon->Get_Name());
		description += line;

		line.Format("TRNDS: %d\n", p_weapon->Get_Total_Rounds());
		description += line;
   }

	line.Format("HLTH:  %-5.2f\n", DefenseObject.Get_Health());
	description += line;

	line.Format("HMAX:  %-5.2f\n", DefenseObject.Get_Health_Max());
	description += line;

	HumanPhysClass * p_human_phys = Peek_Human_Phys();
	if (p_human_phys != NULL) {
		line.Format("FACE:  %-5.2f\n", p_human_phys->Get_Facing());
		description += line;
	}

	line.Format("STATE: %s", Get_State_Name());
	if (Is_Crouched()) {
		line += " (Crouched)";
	}
	line += "\n";
	description += line;

	line.Format("ANIM:  %s\n", AnimationName);
	description += line;

   if (Vehicle != NULL) {
		line.Format("VEH:   %d\n", Vehicle->Get_ID());
		description += line;
   }

	const GameObjObserverList & observer_list = Get_Observers();
	line.Format("#OBSV: %d\n", observer_list.Count());
	description += line;

	for (int index = 0; index < observer_list.Count(); index++) {
		line.Format("       %s\n", observer_list[index]->Get_Name());
		description += line;
	}
	
	line.Format("INNAT: %d\n", Is_Innate_Enabled());
	description += line;

	line.Format("HIB:   %d\n", Is_Hibernating());
	description += line;

	line.Format("CTRL:  %d, %d, %5.2f, %5.2f, %5.2f, %5.2f\n", 
		Control.Get_One_Time_Boolean_Bits(), 
		Control.Get_Continuous_Boolean_Bits(),
		Control.Get_Analog(ControlClass::ANALOG_MOVE_FORWARD),
		Control.Get_Analog(ControlClass::ANALOG_MOVE_LEFT),
		Control.Get_Analog(ControlClass::ANALOG_MOVE_UP),
		Control.Get_Analog(ControlClass::ANALOG_TURN_LEFT));
	description += line;

	ActionClass * p_action = Get_Action();
	if (p_action != NULL)
	{
		line.Format("ACTCT: %d\n", p_action->Get_Act_Count());
		description += line;

		Vector3 move_loc = p_action->Get_Parameters().MoveLocation;
		line.Format("MVLOC: (%5.2f, %5.2f, %5.2f)\n", move_loc.X, move_loc.Y, move_loc.Z);
		description += line;
	}

	line.Format("stlth: %d\n", Is_Stealth_Enabled());
	description += line;

	line.Format("   on: %d\n", Is_Stealthed());
	description += line;

	line.Format("ISC:   %d\n", Get_Import_State_Count());
	description += line;
}

//------------------------------------------------------------------------------------

struct {
	ArmorWarheadManager::SpecialDamageType Mode;
	const char * EmitterName;
	const char * BoneName;
} _SpecialDamageEmitters[] = {
	{ ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_FIRE,	"AG_FLAME01",		"C SPINE" },

	{ ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_SUPER_FIRE,	"AG_FLAME01",		"C SPINE" },
	{ ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_SUPER_FIRE,	"AG_FLAME01",		"C L UPPERARM" },
	{ ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_SUPER_FIRE,	"AG_FLAME01",		"C R UPPERARM" },
	{ ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_SUPER_FIRE,	"AG_FLAME01",		"C L CALF" },
	{ ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_SUPER_FIRE,	"AG_FLAME01",		"C R CALF" },
	{ ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_SUPER_FIRE,	"AG_FLAME01",		"C NECK" },
//	{ ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_FIRE,	"E_FLAME01",		"C L UPPERARM" },
//	{ ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_FIRE,	"E_FLAME01",		"C R UPPERARM" },
	{ ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_CHEM,	"e_tib_dump",		"C SPINE" },
//	{ ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_CHEM,	"e_tib_dump",		"C L UPPERARM" },
//	{ ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_CHEM,	"e_tib_dump",		"C R UPPERARM" },
};
#define NUM_SPECIAL_DAMAGE_EMITTERS	(sizeof(_SpecialDamageEmitters)/sizeof(_SpecialDamageEmitters[0]))

void	SoldierGameObj::Set_Special_Damage_Mode( ArmorWarheadManager::SpecialDamageType mode, ArmedGameObj * damager )
{
	// Check for turning off
	if ( mode == ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_NONE ) {

		// remove all subobjs from the bones used
		for ( int emitter = 0; emitter < NUM_SPECIAL_DAMAGE_EMITTERS; emitter++ ) {
			if ( _SpecialDamageEmitters[emitter].Mode == SpecialDamageMode ) {
				int bone = Peek_Model()->Get_Bone_Index( _SpecialDamageEmitters[emitter].BoneName );
				int sub_count = Peek_Model()->Get_Num_Sub_Objects_On_Bone( bone );
				while ( --sub_count >= 0 ) {
					RenderObjClass * sub = Peek_Model()->Get_Sub_Object_On_Bone( sub_count, bone );
					if ( ::stricmp(sub->Get_Name(), _SpecialDamageEmitters[emitter].EmitterName ) == 0 ) {
						Peek_Model()->Remove_Sub_Object( sub );
					}
					sub->Release_Ref();
				}
			}
		}

		if ( SpecialDamageEffect != NULL ) {
			SpecialDamageEffect->Set_Target_Parameter(0);
			SpecialDamageEffect->Enable_Remove_On_Complete(true);
			REF_PTR_RELEASE( SpecialDamageEffect );
		}

		if ( !Is_Human_Controlled() && IS_MISSION ) {
			if ( HumanState.Get_State() >= HumanStateClass::ON_FIRE && 
				HumanState.Get_State() <= HumanStateClass::ON_CNC_CHEM ) {
				HumanState.Set_State( HumanStateClass::UPRIGHT );
			}
		}

		SpecialDamageDamager = NULL;

	} else {

		if ( mode != SpecialDamageMode ) {

			// Remove old mode
			Set_Special_Damage_Mode( ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_NONE );

			if ( Allow_Special_Damage_State_Lock() && IS_MISSION && mode != ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_SUPER_FIRE ) {
				HumanState.Set_State( (HumanStateClass::HumanStateType)( mode - 
							ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_FIRE + HumanStateClass::ON_FIRE ) );
			}

			// Add new emitters
			for ( int emitter = 0; emitter < NUM_SPECIAL_DAMAGE_EMITTERS; emitter++ ) {
				if ( _SpecialDamageEmitters[emitter].Mode == mode ) {
					RenderObjClass * ro = WW3DAssetManager::Get_Instance()->Create_Render_Obj( 
																		_SpecialDamageEmitters[emitter].EmitterName );
					if ( ro != NULL ) {
						Peek_Model()->Add_Sub_Object_To_Bone( ro, _SpecialDamageEmitters[emitter].BoneName );
						ro->Release_Ref();
					}
				}
			}

			// Add electric effect
			if ( mode == ArmorWarheadManager::SPECIAL_DAMAGE_TYPE_ELECTRIC ) {
				WWASSERT( SpecialDamageEffect == NULL );
				SpecialDamageEffect = CombatMaterialEffectManager::Get_Electrocution_Effect();
				if ( SpecialDamageEffect != NULL ) {
					SpecialDamageEffect->Set_Target_Parameter(0.49f);	// go almost halfway
					Peek_Human_Phys()->Add_Effect_To_Me( SpecialDamageEffect );
				}
			}

			// Save Damager
			SpecialDamageDamager = damager;
		}

	}

	SpecialDamageMode = mode;
	SpecialDamageTimer = ArmorWarheadManager::Get_Special_Damage_Duration( SpecialDamageMode );

}

//-----------------------------------------------------------------------------
void SoldierGameObj::Perturb_Position(float max_perturb)
{
	//
	// Jiggle the soldier to a non-intersecting position
	//
	Vector3 initial_position;
	Get_Position(&initial_position);
	Vector3 new_position;
	WWASSERT(Peek_Human_Phys() != NULL);
	bool succeeded = Peek_Human_Phys()->Find_Teleport_Location(
		initial_position, max_perturb, &new_position);
	if (succeeded) {
		Set_Position(new_position);
	}
}

/*
**
*/
void	SoldierGameObj::Toggle_Fly_Mode( void )
{
	InFlyMode = !InFlyMode;

	if ( InFlyMode ) {
		HumanState.Set_State( HumanStateClass::DEBUG_FLY );
		Peek_Physical_Object()->Set_Collision_Group( UNCOLLIDEABLE_GROUP );
	} else {
		HumanState.Set_State( HumanStateClass::UPRIGHT );
		Peek_Physical_Object()->Set_Collision_Group( SOLDIER_COLLISION_GROUP );
	}
}


/*
**
*/
void	SoldierGameObj::Set_AI_State( SoldierAIState state )
{
	//
	//	Is the soldier changing states?
	//
	if ( AIState != state ) {		

		//
		//	Kill the conversation (if necessary)
		//
		if (	state >= AI_STATE_SEARCH && state > AIState &&
				ActiveConversation != NULL &&
				ActiveConversation->Is_Interruptable ())
		{
			ActiveConversation->Stop_Conversation (ACTION_COMPLETE_CONVERSATION_INTERRUPTED);
		}

		AIState = state;
	}

	return ;
}


/*
** Soldiers maintain all RenderObjs they have created for later use and saving
** This is used for weapon models
*/
void	SoldierGameObj::Add_RenderObj( RenderObjClass * obj )
{
	RenderObjList.Add( obj );
	obj->Add_Ref();
}

RenderObjClass *	SoldierGameObj::Find_RenderObj( const char * name )
{
	for ( int i = 0; i < RenderObjList.Count(); i++ ) {
		if ( !stricmp( RenderObjList[i]->Get_Name(), name ) ) {
			return RenderObjList[i];
		}
	}
	return NULL;
}

void	SoldierGameObj::Reset_RenderObjs( void )
{
	while ( RenderObjList.Count() ) {
		RenderObjList[0]->Release_Ref();
		RenderObjList.Delete( 0 );
	}
}






























































/*
//-----------------------------------------------------------------------------
void SoldierGameObj::Change_Flag_Status(int ctf_team_flag)
{
   WWASSERT(CombatManager::I_Am_Only_Client());

	if (ctf_team_flag == NO_FLAG) {
		//
		// We dropped a flag
		//
		WWASSERT(WWAudioClass::Get_Instance() != NULL);
		WWAudioClass::Get_Instance()->Create_Instant_Sound("Drop_Flag", Get_Transform());

		float speed_factor = 100 / (float) CombatManager::Get_Max_Speed_Pc(this);
		Set_Max_Speed(speed_factor * Get_Max_Speed());

		Set_Ctf_Team_Flag(ctf_team_flag);

	} else {
		//
		// We picked up a flag
		//
		WWASSERT(WWAudioClass::Get_Instance() != NULL);
		WWAudioClass::Get_Instance()->Create_Instant_Sound("Pick_Up_Flag", Get_Transform());

		Set_Ctf_Team_Flag(ctf_team_flag);

		float speed_factor = CombatManager::Get_Max_Speed_Pc(this) / 100.0f;
		Set_Max_Speed(speed_factor * Get_Max_Speed());
	}
}
*/


/*
void SoldierGameObj::Interpret_Sc_Position_Data(Vector3 & sc_position)
{
   WWASSERT(CombatManager::I_Am_Only_Client());

	switch (CombatManager::Get_Client_Interpolation_Model()) {

		case CLIENT_INTERPOLATION_SERVER_AUTHORITATIVE:
			//
			// Simplest case - pop to location given by server
			//
			//Set_Position(sc_position);

			if (Get_State() == HumanStateClass::TRANSITION) {
				//
				// Pop
				//
				Set_Position(sc_position);
				if (Get_Control_Owner() == CombatManager::Get_My_Id()) {
					Debug_Say(("Popping own soldier at time %s\n", cMiscUtil::Get_Text_Time()));
				}
			} else {
				Vector3 current_position;
				Get_Position(&current_position);
				Vector3 pos_error = current_position - sc_position;
				if (pos_error.Length() > 2.0f) {
					Set_Position(sc_position);
					if (Get_Control_Owner() == CombatManager::Get_My_Id()) {
						Debug_Say(("Popping own soldier at time %s\n", cMiscUtil::Get_Text_Time()));
					}
				}
			}

			break;

		case CLIENT_INTERPOLATION_SERVER_AUTHORITATIVE_WITH_BLENDING:
			if (Get_State() == HumanStateClass::TRANSITION) {
				//
				// Pop
				//
				Set_Position(sc_position);
			} else {
				//
				// Blend to position given by server
				//
				Vector3 current_position;
				Get_Position(&current_position);
				Set_Position(0.50 * current_position + 0.50 * sc_position);
			}
			break;

		case CLIENT_INTERPOLATION_PATHFIND: {

			Vector3 current_position;
			Get_Position(&current_position);
			Vector3 pos_error = current_position - sc_position;

			if (pos_error.Length() > 0.02f) {

				if (Get_Control_Owner() == CombatManager::Get_My_Id()) {
					if (pos_error.Length() > 2) {
						//
						// Client is authoritative over his own guy, unless error
						// becomes significant. In this case, pop.
						//
						Set_Position(sc_position);
					}
				} else {

					//if (pos_error.Length() > 20) {
					//	//
					//	// Pop
					//	//
					//	Set_Position(sc_position);
					//} else {
      
						//
						// For NPC's the server could transmit the final destination as well as
						// the proper location. Pop if appropriate to get as close to 
						// proper dest as possible, but pathfind instead to the final dest.
						//
						ActionParamsStruct parameters;
						parameters.Priority = 1;
						parameters.MoveLocation = sc_position;
						parameters.MoveArrivedDistance = 0.1f;
						WWASSERT(Get_Action() != NULL);
						Get_Action()->Goto(parameters);
					//}
				}
			}
			break;
		}

		default:
			WWASSERT(0);
	}
}
*/

/*
void	SoldierGameObj::Think_Pathfind( void )
{
	// CLIENT_PATHFINDING

   //
   // Pop pathfinding client-side soldiers at the earliest opportunity - 
   // as soon as we find them out of the camera fulcrum.
   //
   if (CombatManager::I_Am_Only_Client() && 
      Get_Control_Owner() != CombatManager::Get_My_Id()) {
		   
      Vector3 current_position;
		Get_Position(&current_position);

		ActionClass * p_action = Get_Action();
		WWASSERT(p_action != NULL);

		const Vector3 height_offset(0, 0, 0.9f);

		Vector3 from_position = current_position + height_offset;
		Vector3 to_position = p_action->MovementLocation + height_offset;

		//
		// We can only pop if he is not presently in the camera fulcrum.
		//
		if (p_action->MovementAction != NULL &&
         !CombatManager::Is_In_Camera_Frustrum(from_position)) {

			if (!CombatManager::Is_In_Camera_Frustrum(to_position)) {
				//
				// If the correct position is also out of the fulcrum, pop
				// all the way there.
				//
				Set_Position(p_action->MovementLocation);
				p_action->Set_Movement(NULL);
			} else {

				//
				// We are crossing into the fulcrum. Pop as close to the edge
				// of the fulcrum as possible and pathfind from there.
				//
				
				//Debug_Say(("FRAME\n"));
				bool is_finished = false;
				do {

					Vector3 next_pos;
					p_action->Movement_Act(next_pos);
					
					//if (CombatManager::Is_In_Camera_Frustrum(next_pos + height_offset)) {
					//	is_finished = true;
					//	
					//	//
					//	// Logically it would be better if the next line could be omitted.
					//	// However this seems to screw things up.
					//	//
					//	Set_Position(next_pos);
					//
					//	//Debug_Say(("Finished skipping."));
					//} else {
					//	Set_Position(next_pos);
					//
					//	//Debug_Say(("Skipping to (%5.2f, %5.2f, %5.2f)\n", 
					//	//	next_pos.X, next_pos.Y, next_pos.Z));
					//}
					
					Set_Position(next_pos);

					Vector3 adjusted_next_pos = next_pos + height_offset;
					if (CombatManager::Is_In_Camera_Frustrum(adjusted_next_pos)) {
						is_finished = true;
					}

				} while ((p_action->MovementAction != NULL) && !is_finished);			
			}
      }	
	}
}

void SoldierGameObj::Think_Pathfind(void)
{
   //
   // Pop pathfinding client-side soldiers at the earliest opportunity - 
   // as soon as we find them out of the camera fulcrum.
   //

   if ((CombatManager::Get_Client_Interpolation_Model() == CLIENT_INTERPOLATION_PATHFIND) &&
		CombatManager::I_Am_Only_Client() && 
      Get_Control_Owner() != CombatManager::Get_My_Id()) {
		   
      Vector3 current_position;
		Get_Position(&current_position);

		ActionClass * p_action = Get_Action();
		WWASSERT(p_action != NULL);

		ActionParamsStruct parameters = p_action->Get_Parameters();

		const Vector3 height_offset(0, 0, 0.9f);

		Vector3 from_position = current_position + height_offset;
		Vector3 to_position = parameters.MoveLocation + height_offset;

		//
		// We can only pop if he is not presently in the camera fulcrum.
		//
		if (!CombatManager::Is_In_Camera_Frustrum(from_position)) {

			if (!CombatManager::Is_In_Camera_Frustrum(to_position)) {
				//
				// If the correct position is also out of the fulcrum, pop
				// all the way there.
				//
				Set_Position(to_position);

			} else {

				//
				// Keep popping until we are almost in the fulcrum.
				//
				Vector3 current_pos;
				Get_Position(&current_pos);
				Vector3 last_pos;
				Vector3 test_pos;
				do {

					last_pos = current_pos;
					p_action->Act();
					Get_Position(&current_pos);
					test_pos = current_pos + height_offset;

				} while ((current_pos != last_pos) &&
					!CombatManager::Is_In_Camera_Frustrum(test_pos));

				Set_Position(last_pos);
			}
      }	
	}
}
*/

void	SoldierGameObj::Update_Healing_Effect( void )
{
	if ( HealingEffect != NULL ) {

		if (( HealingEffect->Get_Target_Parameter() >= 0.49f ) && 
			( HealingEffect->Get_Parameter() >= 0.49f ) ) {
			HealingEffect->Set_Target_Parameter( 0 );
		}

		if (( HealingEffect->Get_Target_Parameter() == 0 ) && 
			( HealingEffect->Get_Parameter() == 0 ) ) {
			Peek_Human_Phys()->Remove_Effect_From_Me( HealingEffect );
			REF_PTR_RELEASE( HealingEffect );
		}
	}
}


void	SoldierGameObj::Enable_Ghost_Collision( bool onoff )
{
	bool is_using_ghost_collision = (Peek_Physical_Object ()->Get_Collision_Group() == SOLDIER_GHOST_COLLISION_GROUP);
	if ( onoff == is_using_ghost_collision ) {
		return ;
	}

	//
	//	Change the collision group as necessary
	//
	if ( onoff ) {
		Peek_Physical_Object ()->Set_Collision_Group( SOLDIER_GHOST_COLLISION_GROUP );
	} else {
		Peek_Physical_Object ()->Set_Collision_Group( SOLDIER_COLLISION_GROUP );
	}

	return ;
}


bool	SoldierGameObj::Is_Safe_To_Disable_Ghost_Collision( const Vector3 &curr_pos )
{
	const Vector3 PERSONAL_SPACE_BOX_SIZE (1.5F, 1.5F, 1.0F);
	const float HUMAN_HALF_HEIGHT = 1.0F;
	bool retval = true;

	//
	//	Build a box that represents the "personal" space around the soldier
	//
	Vector3 box_pos = curr_pos + Vector3 (0, 0, HUMAN_HALF_HEIGHT);
	AABoxClass box (box_pos, PERSONAL_SPACE_BOX_SIZE);

	//
	//	Collect all the dynamic objects in this box
	//
	NonRefPhysListClass obj_list;
	PhysicsSceneClass::Get_Instance ()->Collect_Objects (box, false, true, &obj_list);

	//
	//	Loop over all the collected objects
	//
	NonRefPhysListIterator it (&obj_list);
	for (it.First(); !it.Is_Done(); it.Next()) {
		PhysClass *phys_obj = it.Peek_Obj ();
		PhysicalGameObj *game_obj = NULL;

		if ( phys_obj->As_HumanPhysClass () && phys_obj->Get_Observer() != NULL ) {
			game_obj = ((CombatPhysObserverClass *)phys_obj->Get_Observer())->As_PhysicalGameObj();
		}

		//
		//	Is this a living soldier?
		//
		if (	game_obj != NULL && game_obj != this &&
				game_obj->As_SoldierGameObj() != NULL &&
				game_obj->As_SoldierGameObj()->Is_Destroyed () == false)
		{
			Vector3 block_pos;
			game_obj->Get_Position (&block_pos);

			//
			//	Lets make sure the boxes really intersect
			//
			AABoxClass block_box (block_pos + Vector3 (0, 0, 1.0F), Vector3 (0.3F, 0.3F, 1.0F));
			if (CollisionMath::Overlap_Test (box, block_box) != CollisionMath::OUTSIDE) {

				//
				//	Is the boxes intersect, then it is NOT SAFE to disable ghost collision
				// mode.
				//
				retval = false;
				break;
			}
		}
	}

	return retval;
}


bool	SoldierGameObj::Is_Soldier_Blocked( const Vector3 &curr_pos )
{
	//
	//	Only do this for soldiers who meet our criteria
	//
	if (	(Get_Action () != NULL && Get_Action ()->Is_Busy ()) ||
			Is_Destroyed () || (IS_SOLOPLAY == false))
	{
		return false;
	}

	const Vector3 PERSONAL_SPACE_BOX_SIZE (1.5F, 1.5F, 1.0F);
	const float HUMAN_HALF_HEIGHT = 1.0F;

	//
	//	Build a box that represents the "personal" space around the soldier
	//
	Vector3 box_pos = curr_pos + Vector3 (0, 0, HUMAN_HALF_HEIGHT);
	AABoxClass box (box_pos, PERSONAL_SPACE_BOX_SIZE);

	//
	//	Collect all the dynamic objects in this box
	//
	NonRefPhysListClass obj_list;
	PhysicsSceneClass::Get_Instance ()->Collect_Objects (box, false, true, &obj_list);

	uint32 my_id			= Get_ID ();
	uint32 smallest_id	= my_id;

	//
	//	Loop over all the collected objects
	//
	NonRefPhysListIterator it (&obj_list);
	for (it.First(); !it.Is_Done(); it.Next()) {
		PhysClass *phys_obj = it.Peek_Obj ();
		PhysicalGameObj *game_obj = NULL;

		if ( phys_obj->As_HumanPhysClass () && phys_obj->Get_Observer() != NULL ) {
			game_obj = ((CombatPhysObserverClass *)phys_obj->Get_Observer())->As_PhysicalGameObj();
		}

		//
		//	Is this a living soldier?
		//
		if (	game_obj != NULL && game_obj != this &&
				game_obj->As_SoldierGameObj() != NULL &&
				game_obj->As_SoldierGameObj()->Is_Destroyed () == false)
		{
			Vector3 block_pos;
			game_obj->Get_Position (&block_pos);

			//
			//	Lets make sure the boxes really intersect
			//
			AABoxClass block_box (block_pos + Vector3 (0, 0, 1.0F), Vector3 (0.3F, 0.3F, 1.0F));
			if (CollisionMath::Overlap_Test (box, block_box) != CollisionMath::OUTSIDE) {

				if (game_obj->As_SmartGameObj ()->Is_Human_Controlled () == false) {
					
					//
					//	Is this soldier moving?
					//
					ActionClass *curr_action = game_obj->As_SmartGameObj ()->Get_Action ();
					Vector3 dest_pos	= curr_action->Get_Parameters ().MoveLocation;
					float distance		= (block_pos - dest_pos).Length ();
					if (distance > curr_action->Get_Parameters ().MoveArrivedDistance) {					

						//
						//	Is this the smallest ID so far?
						//
						uint32 curr_id = game_obj->Get_ID ();
						if (curr_id < smallest_id) {
							smallest_id = curr_id;
						}

						//
						//	Push the other guy out of the way (a little)
						//
						/*if (Is_Human_Controlled () || curr_id > my_id) {
							Vector3 delta_vector = block_pos - curr_pos;
							delta_vector.Normalize ();
							SoldierGameObj *soldier = game_obj->As_SoldierGameObj ();
							if (soldier->Peek_Human_Phys () != NULL) {
								soldier->Peek_Human_Phys ()->Collide (delta_vector * TimeManager::Get_Frame_Seconds ());
							}
						}*/
					}
				}
			}
		}
	}

	return (smallest_id != my_id);
}



bool		SoldierGameObj::Is_Targetable( void ) const					
{ 
	return (!(((SoldierGameObj*)this)->Is_In_Vehicle())) && 
		SmartGameObj::Is_Targetable(); 
}


float		SoldierGameObj::Get_Stealth_Fade_Distance(void) const
{
	if (IS_MISSION) {
		return GlobalSettingsDef::Get_Global_Settings()->Get_Stealth_Distance_Human();
	} else {
		return GlobalSettingsDef::Get_Global_Settings()->Get_MP_Stealth_Distance_Human();
	}


}


void	SoldierGameObj::Lock_Facing( PhysicalGameObj * game_obj, bool turn_body )
{
	FacingObject			= game_obj;
	FacingAllowBodyTurn	= turn_body;

	//
	//	Stop facing if there's nothing to look at
	//
	if ( game_obj == NULL ) {
		Cancel_Look_At();
	}

	return ;
}


void	SoldierGameObj::Update_Locked_Facing( void )
{
	if ( FacingObject != NULL ) {

		//
		//	Get the position of the object we're "looking" at.
		//
		Vector3 pos;
		FacingObject.Get_Ptr ()->Get_Position( &pos );

		//
		//	If the object is a soldier, then look at his head
		//
		if ( FacingObject.Get_Ptr ()->As_PhysicalGameObj ()->As_SoldierGameObj () != NULL ) {
			const float SOLDIER_HEIGHT = 1.7F;
			pos.Z += SOLDIER_HEIGHT;
		}

		//
		//	Look at the object
		//
		Look_At( pos, 100.0F );
		
		//
		//	If we can turn to face the object, do so...
		//
		if ( FacingAllowBodyTurn ) {
			Internal_Set_Targeting( pos, false );
		}
	}

	return ;
}






























	/*
	if (CombatManager::I_Am_Server()) {
		CombatManager::Soldier_Dies(this);
	}
	*/


