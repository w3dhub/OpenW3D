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
 *                     $Archive:: /Commando/Code/Combat/physicalgameobj.cpp                   $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 6/14/02 5:58p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 223                                                         $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "physicalgameobj.h"
#include "damage.h"
#include "scriptman.h"
#include "debug.h"
#include "explosion.h"
#include "assets.h"
#include "combatsound.h"
#include "matrix3d.h"
#include "phys.h"
#include "smartgameobj.h"
#include "soldier.h"
#include "animcontrol.h"
#include "chunkio.h"
#include "saveload.h"
#include "combat.h"
#include "persistfactory.h"
#include "combatchunkid.h"
#include "parameter.h"
#include "radar.h"
#include "playertype.h"
#include "matinfo.h"
#include "gameobjmanager.h"
#include "pscene.h"
#include "wwprofile.h"
#include "rbody.h"
#include "bitpackids.h"
#include "activeconversation.h"
#include "oratortypes.h"
#include "vistable.h"
#include "encyclopediamgr.h"
#include "combatmaterialeffectmanager.h"
#include "phys.h"
#include "transitioneffect.h"
#include "phys3.h"
#include "surfaceeffects.h"


// Hibernate after 30 seconds
#define	HIBERNATION_DELAY		30
bool	_DisplayHibernating	= false;

/*
** PhysicalGameObjDef
*/
PhysicalGameObjDef::PhysicalGameObjDef( void ) :
	Type( 0 ),
	BullseyeOffsetZ( 0.0f ),
	RadarBlipType( 0 ),
	PhysDefID( 0 ),
	KilledExplosion( 0 ),
	OratorType( ORATOR_TYPE_START - 1 ),
	DefaultHibernationEnable( true ),
	AllowInnateConversations( false ),
	UseCreationEffect( false )
{
#ifdef	PARAM_EDITING_ON
	int i;
	EnumParameterClass *param;

	EDITABLE_PARAM( PhysicalGameObjDef, ParameterClass::TYPE_FLOAT,	BullseyeOffsetZ );

	param = new EnumParameterClass( &RadarBlipType );
	param->Set_Name("Radar Blip Type");
	for ( i = 0; i < RadarManager::Get_Num_Blip_Shape_Types(); i++ ) {
		param->Add_Value ( RadarManager::Get_Blip_Shape_Type_Name( i ), i );
	}
	GENERIC_EDITABLE_PARAM(PhysicalGameObjDef,param)

	EDITABLE_PARAM( PhysicalGameObjDef, ParameterClass::TYPE_STRING,	Animation );

	EDITABLE_PARAM( PhysicalGameObjDef, ParameterClass::TYPE_EXPLOSIONDEFINITIONID,	KilledExplosion);

	EDITABLE_PARAM( PhysicalGameObjDef, ParameterClass::TYPE_BOOL,	DefaultHibernationEnable );

	EDITABLE_PARAM( PhysicalGameObjDef, ParameterClass::TYPE_BOOL,	AllowInnateConversations );	

	EDITABLE_PARAM( PhysicalGameObjDef, ParameterClass::TYPE_BOOL,	UseCreationEffect );

	//
	//	Configure the orator types parameter
	//
	EnumParameterClass *orator_type_param = new EnumParameterClass (&OratorType);
	orator_type_param->Set_Name ("Orator Type");
	
	//
	//	Add all the orator types to the list
	//
	int count = OratorTypeClass::Get_Count ();
	for (int index = 0; index < count; index ++) {
		orator_type_param->Add_Value (OratorTypeClass::Get_Description (index),
									OratorTypeClass::Get_ID (index));
	}
																												
	GENERIC_EDITABLE_PARAM( PhysicalGameObjDef, orator_type_param );


#endif
}

enum	{
	CHUNKID_DEF_VARIABLES						=	909991657,
	LEGACY_CHUNKID_DEF_PARENT_OLD,
	XXXCHUNKID_DEF_PARENT_OLD_OLD,
	LEGACY_CHUNKID_DEF_DEFENSEOBJECTDEF,
	CHUNKID_DEF_PARENT,

	MICROCHUNKID_DEF_TYPE						=	1,
	MICROCHUNKID_DEF_BULLSEYE_OFFSET_Z,
	XXXMICROCHUNKID_DEF_DEFAULT_GANG,
	MICROCHUNKID_DEF_BLIP_TYPE,
	XXXMICROCHUNKID_DEF_MODEL_NAME,
	XXXMICROCHUNKID_DEF_HEALTH,
	XXXMICROCHUNKID_DEF_HEALTH_MAX,
	XXXMICROCHUNKID_DEF_SKIN,
	XXXMICROCHUNKID_DEF_SHIELD_STRENGTH,
	XXXMICROCHUNKID_DEF_SHIELD_STRENGTH_MAX,
	XXXMICROCHUNKID_DEF_SHIELD_TYPE,
	XXXMICROCHUNKID_DEF_LISTEN_RANGE,
	XXX_MICROCHUNKID_DEF_SCRIPT_NAME,
	XXX_MICROCHUNKID_DEF_SCRIPT_PARAMETERS,
	XXXMICROCHUNKID_DEF_POSITION,	// ???
	MICROCHUNKID_DEF_FACING,
	MICROCHUNKID_DEF_ANIMATION,
	MICROCHUNKID_DEF_PHYS_ID,
	LEGACY_MICROCHUNKID_DEF_DEFAULT_PLAYER_TYPE,
	MICROCHUNKID_DEF_KILLED_EXPLOSION,
	LEGACY_MICROCHUNKID_DEF_TRANSLATED_NAME_ID,
	MICROCHUNKID_DEF_DEFAULT_HIBERNATION_ENABLE,
	MICROCHUNKID_DEF_ALLOW_INNATE_CONVERSATIONS,
	MICROCHUNKID_DEF_ORATOR_TYPE,
	MICROCHUNKID_DEF_USE_CREATION_EFFECT, 
};


bool	PhysicalGameObjDef::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_DEF_PARENT );
		DamageableGameObjDef::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_DEF_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_TYPE, Type );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_BULLSEYE_OFFSET_Z, BullseyeOffsetZ );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_BLIP_TYPE, RadarBlipType );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_ANIMATION, Animation );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_PHYS_ID, PhysDefID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_KILLED_EXPLOSION, KilledExplosion );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_DEFAULT_HIBERNATION_ENABLE, DefaultHibernationEnable );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_ALLOW_INNATE_CONVERSATIONS, AllowInnateConversations );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_ORATOR_TYPE, OratorType );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_USE_CREATION_EFFECT, UseCreationEffect );

	csave.End_Chunk();

	return true;
}

bool	PhysicalGameObjDef::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case LEGACY_CHUNKID_DEF_PARENT_OLD:
				ScriptableGameObjDef::Load( cload );
				break;
								
			case CHUNKID_DEF_PARENT:
				DamageableGameObjDef::Load( cload );
				break;
								
			case CHUNKID_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_TYPE, Type );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_BULLSEYE_OFFSET_Z, BullseyeOffsetZ );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_BLIP_TYPE, RadarBlipType );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_DEF_ANIMATION, Animation );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_PHYS_ID, PhysDefID );
						READ_MICRO_CHUNK( cload, LEGACY_MICROCHUNKID_DEF_DEFAULT_PLAYER_TYPE, DefaultPlayerType );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_KILLED_EXPLOSION, KilledExplosion );
						READ_MICRO_CHUNK( cload, LEGACY_MICROCHUNKID_DEF_TRANSLATED_NAME_ID, TranslatedNameID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_DEFAULT_HIBERNATION_ENABLE, DefaultHibernationEnable );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_ALLOW_INNATE_CONVERSATIONS, AllowInnateConversations );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_ORATOR_TYPE, OratorType );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_USE_CREATION_EFFECT, UseCreationEffect );

						default:
							Debug_Say(( "Unrecognized PhysicalDef Variable chunkID %d\n", cload.Cur_Micro_Chunk_ID() ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;
			
			case LEGACY_CHUNKID_DEF_DEFENSEOBJECTDEF:
				DefenseObjectDef.Load(cload);
				break;

			default:
				Debug_Say(( "Unrecognized PhysicalGameObjDef chunkID %d\n", cload.Cur_Chunk_ID() ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

bool	PhysicalGameObjDef::Is_Valid_Config (StringClass &message)
{
	bool retval = false;

	DefinitionClass *phys_def = DefinitionMgrClass::Find_Definition (PhysDefID);
	if (phys_def != NULL) {
		retval = phys_def->Is_Valid_Config (message);
	} else {
		message += "Can't find physics object definition.\n";
	}

	return retval;
}


/*
** PhysicalGameObj
*/
PhysicalGameObj::PhysicalGameObj( void ) :
	PhysObj( NULL ),
	AnimControl( NULL ),
	//TintColor(1, 1, 1),
	HibernationTimer( 0 ),		// Start alseep
	HibernationEnable( true ),
	HostGameObjBone( 0 ),
	RadarBlipShapeType( 0 ),
	RadarBlipColorType( 0 ),
	RadarBlipIntensity( 0 ),
	ActiveConversation( NULL ),
	PendingHostObjID( 0 ),
	HUDPokableIndicatorEnabled( false ),
	IsInnateConversationsEnabled( true )
{
   Reset_Server_Skips(255);
	return ;
}

PhysicalGameObj::~PhysicalGameObj( void )
{
	if ( AnimControl != NULL ) {
		delete AnimControl;
		AnimControl = NULL;
	}

	if ( PhysObj != NULL ) {
		COMBAT_SCENE->Remove_Object( PhysObj );
		PhysObj->Release_Ref();
		PhysObj = NULL;
	}

	REF_PTR_RELEASE( ActiveConversation );
}

/*
**
*/
void	PhysicalGameObj::Init( const PhysicalGameObjDef & definition )
{
	DamageableGameObj::Init( definition );
	Copy_Settings( definition );

	Hide_Muzzle_Flashes();

	/*
	** If the definition calls for it, add a material effect to the object
	*/
	if ( definition.UseCreationEffect ) {
		PhysClass * physobj = Peek_Physical_Object();
		if (physobj != NULL) {
			TransitionEffectClass * effect = CombatMaterialEffectManager::Get_Spawn_Effect();
			physobj->Add_Effect_To_Me(effect);
			REF_PTR_RELEASE(effect);
		}
	}


	return ;
}

/*
**
*/
void	PhysicalGameObj::Copy_Settings( const PhysicalGameObjDef & definition )
{

	//
	//	Release our hold on the physics object
	//
	if ( PhysObj != NULL ) {
		COMBAT_SCENE->Remove_Object( PhysObj );
		PhysObj->Release_Ref();
		PhysObj = NULL;
	}

	// Set the Physical Object
	WWASSERT( PhysObj == NULL );
	DefinitionClass * podef = DefinitionMgrClass::Find_Definition( definition.PhysDefID );
	WWASSERT( SuperClassID_From_ClassID( podef->Get_Class_ID() ) == CLASSID_PHYSICS );
	PhysObj = (PhysClass *)podef->Create();
	WWASSERT( PhysObj != NULL );

	PhysObj->Set_Collision_Group( DEFAULT_COLLISION_GROUP );
	PhysObj->Set_Observer( this );
	COMBAT_SCENE->Add_Dynamic_Object( PhysObj );

	// Do we still use this?????
	if ( !definition.Animation.Is_Empty() ) {
		Set_Animation( definition.Animation );
	}

	Enable_Hibernation( definition.DefaultHibernationEnable );

	Reset_Radar_Blip_Shape_Type();
	return ;
}

/*
**
*/
void	PhysicalGameObj::Re_Init( const PhysicalGameObjDef & definition )
{
	Matrix3D tm				= Get_Transform ();

	//
	//	Re-initialize the base class
	//
	DamageableGameObj::Re_Init( definition );

	//
	//	Copy any internal settings from the definition
	//
	Copy_Settings( definition );

	//
	//	Restore the necessary settings
	//
	Set_Transform( tm );
	return ;
}


const PhysicalGameObjDef & PhysicalGameObj::Get_Definition( void ) const 
{
	return (const PhysicalGameObjDef &)BaseGameObj::Get_Definition();
}


/*
** PhysicalGameObj Save and Load
*/

enum	{
	XXXCHUNKID_PARENT_OLD_OLD					=	910991145,
	CHUNKID_VARIABLES,
	XXX_CHUNKID_SCRIPTS,
	LEGACY_CHUNKID_DEFENSE,
	XXXCHUNKID_LISTENER,
	XXXCHUNKID_REFERENCEABLE,
	XXXCHUNKID_OBSER_XXX_VER,
	LEGACY_CHUNKID_PARENT_OLD,
	CHUNKID_ANIM_CONTROL,
	CHUNKID_HOST_GAME_OBJ,
	CHUNKID_PARENT,

	XXXMICROCHUNKID_ID						=	1,
	XXXMICROCHUNKID_GANG,
	MICROCHUNKID_PHYS_OBSERVER_PTR,
	XXXMICROCHUNKID_REFERENCEABLE_PTR,
   XXXMICROCHUNKID_DISTANCE_PRIORITY,
	XXXMICROCHUNKID_TIME_PRIORITY,
	XXXMICROCHUNKID_PRIORITY,
	XXXMICROCHUNKID_GAME_OBJ_OBSERVER_PTR,
	LEGACY_MICROCHUNKID_PLAYER_TYPE,
	MICROCHUNKID_PHYSICAL_OBJECT,
	MICROCHUNKID_HIBERNATION_TIMER,
	MICROCHUNKID_HIBERNATION_ENABLE,
	MICROCHUNKID_HOST_GAME_OBJ_BONE,

	MICROCHUNKID_RADAR_BLIP_SHAPE_TYPE, 
	MICROCHUNKID_RADAR_BLIP_COLOR_TYPE, 
	MICROCHUNKID_RADAR_BLIP_INTENSITY,
	MICROCHUNKID_ACTIVE_CONVERSATION,
	MICROCHUNKID_HUD_POKABLE_INDICATOR,
	MICROCHUNKID_IS_INNATE_CONVERSATIONS_ENABLED,
};

bool	PhysicalGameObj::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_PARENT );
		DamageableGameObj::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		CombatPhysObserverClass * phys_observer_ptr = (CombatPhysObserverClass *)this;
		WRITE_MICRO_CHUNK_PTR( csave, MICROCHUNKID_PHYS_OBSERVER_PTR, phys_observer_ptr );
		WRITE_MICRO_CHUNK_PTR( csave, MICROCHUNKID_PHYSICAL_OBJECT, PhysObj );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_HIBERNATION_TIMER, HibernationTimer );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_HIBERNATION_ENABLE, HibernationEnable );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_HOST_GAME_OBJ_BONE, HostGameObjBone );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_RADAR_BLIP_SHAPE_TYPE, RadarBlipShapeType );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_RADAR_BLIP_COLOR_TYPE, RadarBlipColorType );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_RADAR_BLIP_INTENSITY, RadarBlipIntensity );
		WRITE_MICRO_CHUNK_PTR( csave, MICROCHUNKID_ACTIVE_CONVERSATION, ActiveConversation );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_HUD_POKABLE_INDICATOR, HUDPokableIndicatorEnabled );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_IS_INNATE_CONVERSATIONS_ENABLED, IsInnateConversationsEnabled );

	csave.End_Chunk();

	if ( AnimControl ) {
		csave.Begin_Chunk( CHUNKID_ANIM_CONTROL );
		AnimControl->Save( csave );
		csave.End_Chunk();
	}

   if ( HostGameObj.Get_Ptr() != NULL ) {
		csave.Begin_Chunk( CHUNKID_HOST_GAME_OBJ );
		HostGameObj.Save( csave );
		csave.End_Chunk();
	}

/*
#pragma message( "Tom, do these need to be saved?" )
	int						ImportStateCount;
   float						DistancePriority;
	float						TimePriority;
	float						VolatilityPriority;
	float						Priority;
	BYTE						ClientUpdateSkips[255];
	BYTE						ServerUpdateSkips;
	cPacket					StatePacket;
	int						StatePacketUnchangedCount;
	Vector3					TintColor;
*/

	return true;
}

bool	PhysicalGameObj::Load( ChunkLoadClass &cload )
{
	WWASSERT( PhysObj == NULL );		// May need to change to release???
	CombatPhysObserverClass * phys_observer_ptr = NULL;

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case LEGACY_CHUNKID_PARENT_OLD:
				ScriptableGameObj::Load( cload );
				break;

			case CHUNKID_PARENT:
				DamageableGameObj::Load( cload );
				break;

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK_PTR( cload, MICROCHUNKID_PHYS_OBSERVER_PTR, phys_observer_ptr );
						READ_MICRO_CHUNK( cload, LEGACY_MICROCHUNKID_PLAYER_TYPE, PlayerType );
						READ_MICRO_CHUNK_PTR( cload, MICROCHUNKID_PHYSICAL_OBJECT, PhysObj );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_HIBERNATION_TIMER, HibernationTimer );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_HIBERNATION_ENABLE, HibernationEnable );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_HOST_GAME_OBJ_BONE, HostGameObjBone );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_RADAR_BLIP_SHAPE_TYPE, RadarBlipShapeType );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_RADAR_BLIP_COLOR_TYPE, RadarBlipColorType );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_RADAR_BLIP_INTENSITY, RadarBlipIntensity );
						READ_MICRO_CHUNK_PTR( cload, MICROCHUNKID_ACTIVE_CONVERSATION, ActiveConversation );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_HUD_POKABLE_INDICATOR, HUDPokableIndicatorEnabled );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_IS_INNATE_CONVERSATIONS_ENABLED, IsInnateConversationsEnabled );


						default:
							Debug_Say(( "Unrecognized PhysicalGameObj Variable chunkID\n" ));
							break;

					}
					cload.Close_Micro_Chunk();
				}

				break;
								
			case LEGACY_CHUNKID_DEFENSE:
				DefenseObject.Load( cload );
				break;
								
			case CHUNKID_ANIM_CONTROL:
				Set_Animation( NULL );  // Build AnimControl
				AnimControl->Load( cload );
				break;

			case CHUNKID_HOST_GAME_OBJ:
				HostGameObj.Load( cload );
				break;

			default:
				Debug_Say(( "Unrecognized PhysicalGameObj chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	WWASSERT( PhysObj != NULL );		
	REQUEST_REF_COUNTED_POINTER_REMAP( (RefCountClass **)&PhysObj );

	if ( ActiveConversation != NULL ) {
		REQUEST_REF_COUNTED_POINTER_REMAP( (RefCountClass **)&ActiveConversation );
	}

	// Register the multiple-inheritance versions of our this pointer.
	WWASSERT(phys_observer_ptr != NULL);
	if (phys_observer_ptr != NULL) {
		SaveLoadSystemClass::Register_Pointer(phys_observer_ptr, (CombatPhysObserverClass *)this);
	}

	SaveLoadSystemClass::Register_Post_Load_Callback(this);

	return true;
}

void PhysicalGameObj::On_Post_Load (void)					
{ 
	// Plug ourselves back into the physics object as an observer
	WWASSERT(PhysObj != NULL);
	PhysObj->Set_Observer(this);

	Hide_Muzzle_Flashes();
	DamageableGameObj::On_Post_Load ();
}


AnimControlClass *	PhysicalGameObj::Get_Anim_Control( void ) 
{
	return AnimControl;
}

void	PhysicalGameObj::Set_Anim_Control( AnimControlClass * anim_control )
{
	WWASSERT( AnimControl == NULL );
	AnimControl = anim_control;
	WWASSERT( AnimControl != NULL );
}

bool	PhysicalGameObj::Is_Soft( void )
{ 
	return DefenseObject.Is_Soft(); 
}

Vector3	PhysicalGameObj::Get_Bullseye_Position( void )
{
	Vector3 pos;
	Get_Position(&pos);
//	pos.Z += Get_Bullseye_Offset_Z();
	return pos;
}

void	PhysicalGameObj::Apply_Damage( const OffenseObjectClass & damager, float scale, int /* alternate_skin */ )
{
	// If this damage is allowed
	if ( !CombatManager::Can_Damage( damager.Get_Owner(), this)) {
		return;
	}

#ifdef WWDEBUG
	//
	// Tone it down for VIP's
	//
	scale *= CombatManager::Get_Damage_Factor(damager.Get_Owner(), this);
#endif // WWDEBUG

	DamageableGameObj::Apply_Damage( damager, scale );
}


void	PhysicalGameObj::Apply_Damage_Extended( const OffenseObjectClass & damager, float scale,
			const	Vector3 & /* direction */, const char * /* collision_box_name */ )
{
//	if ( CombatManager::I_Am_Server() ) 	Clients can apply damage now
	{
		Apply_Damage( damager, scale );
	}
}


void PhysicalGameObj::Completely_Damaged( const OffenseObjectClass & damager ) 
{
	if ( Get_Definition().KilledExplosion != 0 ) {
		
		Vector3 pos;
		Get_Position(&pos);
		WWASSERT(pos.Is_Valid());// most likely candidate for explosion damage bug....?

		// Build a transform with the same heading as the object
		float z_rot = Get_Transform().Get_Z_Rotation();
		Matrix3D tm(pos);
		tm.Rotate_Z(z_rot);

		// Create the explosion
		ExplosionManager::Create_Explosion_At( Get_Definition().KilledExplosion, tm/*pos*/, damager.Get_Owner() );	 // no one gets credit for this

		//
		//	Reveal this object to the player's encyclopedia
		//
		if ( damager.Get_Owner () == COMBAT_STAR ) {
			EncyclopediaMgrClass::Reveal_Object( this );
		}
	}
	Set_Delete_Pending();
}

void PhysicalGameObj::Teleport_To_Host_Bone( void )
{
//	Debug_Say(( "Teleporting %d to host\n", Get_ID() ));

	if ( HostGameObj.Get_Ptr() ) {
		RenderObjClass * model = ((PhysicalGameObj * )(HostGameObj.Get_Ptr()))->Peek_Model();
		if ( model != NULL ) {

			/*
			** Calculate the bone's transform and try to go there
			*/
			bool ok = false;
			Matrix3D new_transform = model->Get_Bone_Transform( HostGameObjBone );


			/*
			** If we are some kind of moveable object see if we can teleport to the desired position.
			** Also, clear our velocity whenever being controlled by an animation.
			*/
			if (Peek_Physical_Object()->As_MoveablePhysClass()) {

				/*
				** Don't collide with our host
				*/
				PhysClass * host_phys = ((PhysicalGameObj * )HostGameObj.Get_Ptr())->Peek_Physical_Object();
				if (host_phys != NULL) {
					host_phys->Inc_Ignore_Counter();
				}

				/*
				** Try to teleport to the new location while pushing any blocking dynamic
				** objects out of the way
				*/
				MoveablePhysClass * movephys = Peek_Physical_Object()->As_MoveablePhysClass();			
				movephys->Set_Velocity(Vector3(0,0,0));
				movephys->Cinematic_Move_To(new_transform);

				/*
				** Re-enable collision for our host object
				*/
				if (host_phys != NULL) {
					host_phys->Dec_Ignore_Counter();
				}
			}

			/*
			** If we are not a moveable object, just set the transform.
			*/
			if (Peek_Physical_Object()->As_MoveablePhysClass() == NULL) {
				Set_Transform(new_transform);
				ok = true;			
			}

			/*
			** Always reset the hibernation state
			*/
			Reset_Hibernating();
		}
	}
}

void PhysicalGameObj::Post_Think( void )
{
	if ( AnimControl != NULL ) {
		if ( AnimControl->Peek_Model() != Peek_Model() ) {
			Debug_Say(( "Anim control doesn't match Model\n" ));

			// For some reason???  some vehicles come in with an anim control, but no model in the anim control.
			if ( Get_Anim_Control() != NULL && Get_Anim_Control()->Peek_Model() == NULL ) {
				Get_Anim_Control()->Set_Model( Peek_Model() );
			}
		}
//		WWASSERT( AnimControl->Peek_Model() == Peek_Model() );
	}

	// Handle Pending Host
	if ( PendingHostObjID != 0 ) {
		Reset_Hibernating();
		HostGameObj = GameObjManager::Find_PhysicalGameObj( PendingHostObjID );
		Set_Object_Dirty_Bit( NetworkObjectClass::BIT_RARE, true );
		if ( HostGameObj.Get_Ptr() != NULL ) {
//			Debug_Say(( "Found Pending Host\n" ));
			PendingHostObjID = 0;		// Fond em
		}
	}

	// If host bone controlled
	if ( HostGameObj.Get_Ptr() ) {
		Teleport_To_Host_Bone();
	}

	DamageableGameObj::Post_Think();		

	WWPROFILE( "Physical PostThink" );

	if ( HibernationEnable && HibernationTimer > 0 ) {
		HibernationTimer -= TimeManager::Get_Frame_Seconds();

		if ( HibernationTimer <= 0 ) {
			Begin_Hibernation();
//			Debug_Say(( "Hibernate!!!\n" ));
		}
	}

	if ( AnimControl != NULL ) {
		bool	anim_complete = AnimControl->Is_Complete();

		AnimControl->Update( TimeManager::Get_Frame_Seconds() );	// update the animation control

		if ( !anim_complete && AnimControl->Is_Complete() ) {
			// we just completed.  Return Animation_Complete IF this is not a smart obj with animation action
			if ( As_SmartGameObj() == NULL || !As_SmartGameObj()->Get_Action()->Is_Animating() ) {
				const GameObjObserverList & observer_list = Get_Observers();
				for( int index = 0; index < observer_list.Count(); index++ ) {
					observer_list[ index ]->Animation_Complete( this, AnimControl->Get_Animation_Name() );
				}
			}
		}
	}

// FIXME Going to hell on a client is problematic.
#ifndef PARAM_EDITING_ON  //(gth) don't go to hell in the editor cause it will cause a crash!
	if (CombatManager::I_Am_Only_Client () == false && COMBAT_SCENE != NULL) {
		Vector3 pos;
		Get_Position(&pos);
		Vector3 min;
		Vector3 max;
		COMBAT_SCENE->Get_Level_Extents(min, max);
		if ( pos.Z < min.Z - 20.0f ) {
			Debug_Say(( "Object %d is going to hell at (%1.1f, %1.1f, %1.1f).  Die!\n", Get_ID(), pos.X, pos.Y, pos.Z ));
			Set_Delete_Pending();
		}
	}
#endif

}

void PhysicalGameObj::Set_Collision_Group( int group )		
{	
	Peek_Physical_Object()->Set_Collision_Group( group ); 
}

void	PhysicalGameObj::Attach_To_Object_Bone( PhysicalGameObj * host, const char * bone_name )
{
	// Make sure we get teleported immeadiately!
	Teleport_To_Host_Bone();

	//
	//	Zero the velocity of the object if we are detaching it from 
	// the bone...  (This makes physics behave better)
	//
	if (HostGameObj != host && host == NULL) {
		RigidBodyClass *rigid_body = Peek_Physical_Object()->As_RigidBodyClass();
		if (rigid_body != NULL) {			
			
			Vector3 velocity;
			rigid_body->Get_Velocity (&velocity);

			Matrix3D curr_tm = rigid_body->Get_Transform ();
			float heading = curr_tm.Get_Z_Rotation ();
			Matrix3D new_tm (1);
			new_tm.Rotate_Z (heading);
			new_tm.Set_Translation (curr_tm.Get_Translation ());			
			rigid_body->Set_Transform (new_tm);

			velocity.X = 0;
			velocity.Y = 0;
			velocity.Z = 0;
			rigid_body->Set_Velocity (velocity);
			rigid_body->Set_Angular_Velocity(velocity);
		}
	}

	HostGameObj = host;
	if ( host != NULL ) {
		WWASSERT( host->Peek_Model() );
		HostGameObjBone = host->Peek_Model()->Get_Bone_Index( bone_name );

		// Make sure we get teleported immeadiately!
		Teleport_To_Host_Bone();
	}

	//
	//	"Dirty" the object for networking
	//
	Set_Object_Dirty_Bit( NetworkObjectClass::BIT_RARE, true );
	return ;
}

void PhysicalGameObj::Reset_Server_Skips(BYTE value) 
{
	ServerUpdateSkips = value;
}

void PhysicalGameObj::Increment_Server_Skips(void) 
{
	if (ServerUpdateSkips < 254) {
		ServerUpdateSkips++;
	}
}


//-----------------------------------------------------------------------------
void	PhysicalGameObj::Reset_Radar_Blip_Color_Type( void )
{
	switch( Get_Player_Type() ) {
		case PLAYERTYPE_NOD:	RadarBlipColorType = RadarManager::BLIP_COLOR_TYPE_NOD; break;
		case PLAYERTYPE_GDI:	RadarBlipColorType = RadarManager::BLIP_COLOR_TYPE_GDI; break;
		case PLAYERTYPE_MUTANT:	RadarBlipColorType = RadarManager::BLIP_COLOR_TYPE_MUTANT; break;
		case PLAYERTYPE_RENEGADE:RadarBlipColorType = RadarManager::BLIP_COLOR_TYPE_RENEGADE; break;
		default:				RadarBlipColorType = RadarManager::BLIP_COLOR_TYPE_NEUTRAL; break;
	}
}

//-----------------------------------------------------------------------------

/*
**
*/
void	PhysicalGameObj::Set_Animation( const char *animation_name, bool looping, float frame_offset )
{
	if ( AnimControl == NULL ) {
		Set_Anim_Control( new SimpleAnimControlClass );		// be sure we have a anim control
	}

	StringClass	anim_name(animation_name,true);
	if ( !anim_name.Is_Empty() ) {

		// make sure it lead with model name
		if ( ::strchr( anim_name, '.' ) == NULL ) {
			Create_Animation_Name( anim_name, animation_name, Peek_Model()->Get_Name() );
		}

		AnimControl->Set_Model( Peek_Model() );

		AnimControl->Set_Animation( anim_name, 0, frame_offset );
		AnimControl->Set_Mode( looping ? ANIM_MODE_LOOP : ANIM_MODE_ONCE );

		// Force the object to start using the anim
		AnimControl->Update( 0 );

		//
		//	"Dirty" the object for networking
		//
		Set_Object_Dirty_Bit( NetworkObjectClass::BIT_RARE, true );
	}
}

/*
**
*/
void	PhysicalGameObj::Set_Animation_Frame ( const char *animation_name, int frame )
{
	if ( AnimControl == NULL ) {
		Set_Anim_Control( new SimpleAnimControlClass );		// be sure we have a anim control
	}

 	StringClass	anim_name(animation_name,true);
	if ( !anim_name.Is_Empty() ) {

		// make sure it lead with model name
		if ( ::strchr( anim_name, '.' ) == NULL ) {
			Create_Animation_Name( anim_name, animation_name, Peek_Model()->Get_Name() );
		}

		AnimControl->Set_Model( Peek_Model() );

		AnimControl->Set_Animation( anim_name, 0 );
		AnimControl->Set_Mode( ANIM_MODE_STOP, frame );

		//
		//	"Dirty" the object for networking
		//
		Set_Object_Dirty_Bit( NetworkObjectClass::BIT_RARE, true );
	}
}

/*
**
*/
void	PhysicalGameObj::Set_Transform(const Matrix3D & tm) 
{ 
	WWASSERT(Peek_Physical_Object() != NULL);
	Peek_Physical_Object()->Set_Transform(tm); 
}

const Matrix3D &	PhysicalGameObj::Get_Transform(void) const 
{ 
	WWASSERT(Peek_Physical_Object() != NULL);
	return Peek_Physical_Object()->Get_Transform(); 
}

void	PhysicalGameObj::Get_Position(Vector3 * set_pos) const 
{ 
	WWASSERT(Peek_Physical_Object() != NULL);
	Peek_Physical_Object()->Get_Position(set_pos); 
}

void	PhysicalGameObj::Set_Position(const Vector3 & pos) 
{ 
	WWASSERT(Peek_Physical_Object() != NULL);
	Peek_Physical_Object()->Set_Position(pos); 
}

float	PhysicalGameObj::Get_Facing(void) const
{ 
	WWASSERT(Peek_Physical_Object() != NULL);
	return Peek_Physical_Object()->Get_Facing(); 
}

void	PhysicalGameObj::Reset_Hibernating( void )
{
	//
	//	Notify the object that is has just finished hiibernating
	//
	if (Is_Hibernating ()) {
		End_Hibernation ();
	}

	HibernationTimer = MIN( HIBERNATION_DELAY, HibernationTimer + TimeManager::Get_Frame_Seconds() * 2 );
}

void	PhysicalGameObj::Begin_Hibernation( void )		
{
	if (_DisplayHibernating) {
		Debug_Say(( "Object %d Hibernating\n", Get_ID() ));
	}
}

void	PhysicalGameObj::End_Hibernation( void )			
{
	if (_DisplayHibernating) {
		Debug_Say(( "Object %d De-Hibernating\n", Get_ID() ));
	}
}


void	PhysicalGameObj::Get_Information( StringClass & string )
{
	StringClass temp;

	temp.Format( "%s\n", Get_Definition().Get_Name() );
	string += temp;

	temp.Format( "ID: %d\n", Get_ID() );
	string += temp;

	temp.Format( "Health: %d\n", (int)DefenseObject.Get_Health() );
	string += temp;

	if ( Is_Hibernating() ) {
		string += "HIBERNATING\n";
	}

	DamageableGameObj::Get_Information( string );
}


void	PhysicalGameObj::Export_Creation( BitStreamClass &packet )
{
	DamageableGameObj::Export_Creation( packet );

	//
	//	Get the object's position and facing
	//
	Vector3 position (0, 0, 0);
	float facing = 0;
	Get_Position (&position);
	facing = Get_Facing ();

	//
	//	Send the object's position
	//
	packet.Add( position.X, BITPACK_WORLD_POSITION_X );
	packet.Add( position.Y, BITPACK_WORLD_POSITION_Y );
	packet.Add( position.Z, BITPACK_WORLD_POSITION_Z );

	//
	//	Send the object's facing
	//
	packet.Add( facing );

	/*
	//
	//	Send the player type
	//
	int player_type = Get_Player_Type();
	packet.Add( player_type );
	*/
	return ;
}


void	PhysicalGameObj::Import_Creation( BitStreamClass &packet )
{
	DamageableGameObj::Import_Creation( packet );
 
	//
	//	Read the object's position
	//
	Vector3 position (0, 0, 0);
	packet.Get( position.X, BITPACK_WORLD_POSITION_X );
	packet.Get( position.Y, BITPACK_WORLD_POSITION_Y );
	packet.Get( position.Z, BITPACK_WORLD_POSITION_Z );

	//
	//	Read the object's facing
	//
	float facing = 0;
	packet.Get( facing );

	//
	//	Build a matrix from the position and facing, then set it
	//	
	Matrix3D tm (1);
	tm.Translate (position);
	tm.Rotate_Z (facing);
	Set_Transform (tm);

	/*
	//
	//	Get the player type from the packet
	//
	int player_type = 0;
	packet.Get( player_type );
	Set_Player_Type( player_type );
	*/

	return ;
}


void	PhysicalGameObj::Export_Rare( BitStreamClass &packet )
{
	DamageableGameObj::Export_Rare( packet );

	//
	//	Pass the model name across
	//

	WWASSERT(Peek_Physical_Object() != NULL);
	WWASSERT(Peek_Physical_Object()->Peek_Model() != NULL);
	//WWASSERT(Peek_Physical_Object()->Peek_Model()->Get_Name());
	
	const char *model_name = Peek_Physical_Object()->Peek_Model()->Get_Name();
	WWASSERT(model_name != NULL);
	WWASSERT(::strlen(model_name) < 256);
	packet.Add_Terminated_String( model_name, true );

	//
	//	Get information about the animation
	//
	StringClass animation_name;
	int target_frame		= 0;
	int curr_frame			= 0;
	AnimMode anim_mode	= ANIM_MODE_TARGET;
	if (AnimControl != NULL) {
		animation_name	= AnimControl->Get_Animation_Name();
		target_frame	= AnimControl->Get_Target_Frame ();
		curr_frame		= AnimControl->Get_Current_Frame ();
		anim_mode		= AnimControl->Get_Mode ();
	}

	//
	//	Send the information to the client
	//
	packet.Add_Terminated_String( (const char *)animation_name, true );
	packet.Add( curr_frame );
	packet.Add( target_frame );
	packet.Add( anim_mode );

	//
	//	Lookup the id of the host object
	//
	int host_model_id = 0;
	if (HostGameObj != NULL) {
		host_model_id = HostGameObj.Get_Ptr ()->Get_ID ();
	}

	//
	//	Pass the host information across
	//
	packet.Add( host_model_id );
	packet.Add( HostGameObjBone );
	
	//
	//	Send the player type
	//
	int player_type = Get_Player_Type();
	packet.Add( player_type );

	packet.Add( HUDPokableIndicatorEnabled );

	// We want to copy the hidden status for cinematics, (specefically, airstrip drops of vehicles)
	// but, we only want to change vehicles visibility
	if ( As_VehicleGameObj() != NULL ) {
		// Send hidden
		bool hidden = false;
		if ( Peek_Model() ) {
			hidden = !!(Peek_Model()->Is_Hidden());
		}		
		packet.Add( hidden );
	}

	return ;
}


void	PhysicalGameObj::Import_Rare( BitStreamClass &packet )
{
	DamageableGameObj::Import_Rare( packet );

	//
	//	Get the model name
	//
	StringClass model_name;
	packet.Get_Terminated_String( model_name.Get_Buffer( 256 ), 256, true );

	//
	//	Set the new model (if necessary)
	//
	const char *old_model_name = Peek_Physical_Object()->Peek_Model()->Get_Name();
	if ( model_name.Compare_No_Case (old_model_name) != 0 ) {
		Peek_Physical_Object()->Set_Model_By_Name( model_name );
	}

	//
	//	Get information about the animation
	//
	StringClass animation_name;
	int target_frame	= 0;
	int curr_frame		= 0;
	int anim_mode		= ANIM_MODE_TARGET;
	packet.Get_Terminated_String( animation_name.Get_Buffer( 256 ), 256, true );
	packet.Get( curr_frame );
	packet.Get( target_frame );
	packet.Get( anim_mode );

	//
	//	Pass the animation information onto the controller
	//
	if (AnimControl != NULL) {
		AnimControl->Set_Animation( animation_name, 0, curr_frame );
		AnimControl->Set_Target_Frame( target_frame );
		AnimControl->Set_Mode( (AnimMode)anim_mode );
	}

	//
	//	Get the host information
	//
	int host_model_id = 0;
	packet.Get( host_model_id );
	packet.Get( HostGameObjBone );

	//
	//	Change the host object
	//
	PendingHostObjID = 0;	// Assume no pending
	if ( host_model_id != 0 ) {
		HostGameObj = GameObjManager::Find_PhysicalGameObj( host_model_id );

		if ( HostGameObj.Get_Ptr() == NULL ) {
			PendingHostObjID = host_model_id;	// Pending Host
			Reset_Hibernating();
		}
	} else {
		HostGameObj = NULL;
	}
	
	//
	//	Get the player type from the packet
	//
	int player_type = packet.Get( player_type );
	Set_Player_Type( player_type );

	HUDPokableIndicatorEnabled = packet.Get( HUDPokableIndicatorEnabled );


	if ( As_VehicleGameObj() != NULL ) {
		// Get Hidden
		bool hidden = packet.Get( hidden );
		if ( Peek_Model() ) {
			Peek_Model()->Set_Hidden( hidden );
		}
	}

	return ;
}

void PhysicalGameObj::Export_Frequent( BitStreamClass &packet )
{
	bool on_host_bone = false;

	if ( HostGameObj.Get_Ptr() ) {
		RenderObjClass * model = ((PhysicalGameObj * )(HostGameObj.Get_Ptr()))->Peek_Model();
		on_host_bone = ( model != NULL );
	}

	packet.Add(on_host_bone);
}

void PhysicalGameObj::Import_Frequent( BitStreamClass &packet )
{
	bool on_host_bone;
	packet.Get(on_host_bone);

	if (	(on_host_bone) && 
			(Peek_Physical_Object()) && 
			(Peek_Physical_Object()->As_MoveablePhysClass()) ) 
	{
		MoveablePhysClass * movephys = Peek_Physical_Object()->As_MoveablePhysClass();
		movephys->Set_Velocity(Vector3(0,0,0));
	}
}

/*
**
*/
void	PhysicalGameObj::Set_Conversation( ActiveConversationClass *conversation )
{
	REF_PTR_SET (ActiveConversation, conversation);
	return ;
}

/*
**
*/
void	PhysicalGameObj::Hide_Muzzle_Flashes( bool hide )
{
	RenderObjClass * model = Peek_Model();
	if ( model ) {
		for (int i=0; i<model->Get_Num_Sub_Objects(); i++) {
			RenderObjClass * robj = model->Get_Sub_Object(i);
			if (strstr(robj->Get_Name(),"MUZZLEFLASH") || strstr(robj->Get_Name(),"MZ")) {
				robj->Set_Hidden( hide );
			}
			robj->Release_Ref();
		}
	}
}


/*
**  Get_Vis_ID - return the vis id for this object
*/
int	PhysicalGameObj::Get_Vis_ID ()
{
	PhysClass *phys_obj	= Peek_Physical_Object ();

	//
	//	Do we have a physics object we can use?
	//
	if (phys_obj != NULL) {

		return phys_obj->Get_Vis_Object_ID();

	}

	return -1;
}


void PhysicalGameObj::Set_Player_Type(int id) 
{
	DamageableGameObj::Set_Player_Type(id);

	Reset_Radar_Blip_Color_Type();
}



















	/*
	if (Is_Team_Player()) {
		
		//
		// Handle tinting for soldiers, pedestals, flags
		//
		if (	stricmp( Get_Definition().Get_Name(), "CtfFlag" ) == 0 ||
				stricmp( Get_Definition().Get_Name(), "CtfPedestal" ) == 0)
		{
			Set_Tint( Get_Team_Color() );
		}
	}
	*/

//-----------------------------------------------------------------------------
/*
void Tint(RenderObjClass *robj, const Vector3 & color) 
{
//	Debug_Say(( "%s has %d sub objs\n", robj->Get_Name(), robj->Get_Num_Sub_Objects() ));
	for( int so = 0; so < robj->Get_Num_Sub_Objects(); so++ ) {
		RenderObjClass *child = robj->Get_Sub_Object( so );
		Tint(child, color);
		child->Release_Ref();
	}

	// Tint the commando body
	MaterialInfoClass * matinfo = robj->Get_Material_Info();
	if ( matinfo ) {
		for ( int vm = 0; vm < matinfo->Vertex_Material_Count(); vm++ ) {
			VertexMaterialClass *vertmat = matinfo->Peek_Vertex_Material(vm);
//			Debug_Say(( "VM %p %s\n", vertmat, vertmat->Get_Name() ));
//			if (1) {// !strnicmp( vertmat->Get_Name(), "tintt", 5 ) ) {
				vertmat->Set_Diffuse( color[0], color[1], color[2] );
				vertmat->Set_Ambient( color[0], color[1], color[2] );
//			}
		}
		matinfo->Release_Ref();
	}
}

//-----------------------------------------------------------------------------
void PhysicalGameObj::Set_Tint(Vector3 color)	
{ 
   if (TintColor != color && Peek_Model() != NULL) {
		Tint(Peek_Model(), color);
		TintColor = color; 
   }
}
*/


void	PhysicalGameObj::Enable_HUD_Pokable_Indicator( bool enable )		
{ 
	HUDPokableIndicatorEnabled = enable; 
	Set_Object_Dirty_Bit( NetworkObjectClass::BIT_RARE, true );
}


void PhysicalGameObj::Object_Shattered_Something
(
	PhysClass * observed_obj, 
	PhysClass * /* shattered_obj */, 
	int surface_type
)
{
	const Matrix3D & tm = observed_obj->Get_Transform();

	SurfaceEffectsManager::Apply_Effect(	surface_type, 
														SurfaceEffectsManager::HITTER_TYPE_BULLET, 
														tm, 
														NULL,		
														NULL,
														false,	// no decals
														false		// no emitter
														);
}

