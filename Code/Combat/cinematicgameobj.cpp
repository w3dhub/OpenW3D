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
 *                     $Archive:: /Commando/Code/Combat/cinematicgameobj.cpp                  $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 10/10/01 11:37a                                             $* 
 *                                                                                             * 
 *                    $Revision:: 32                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
**	Includes
*/
#include "cinematicgameobj.h"
#include "debug.h"
#include "animcontrol.h"
#include "Sound3D.h"
#include "combat.h"
#include "pscene.h"
#include "persistfactory.h"
#include "combatchunkid.h"
#include "simpledefinitionfactory.h"
#include "wwhack.h"
#include "weapons.h"
#include "gameobjmanager.h"
#include "assets.h"
#include "ccamera.h"
#include "explosion.h"
#include "damage.h"
#include "dynamicanimphys.h"
#include "wwprofile.h"
#include "apppackettypes.h"


/*
** CinematicGameObjDef
*/
DECLARE_FORCE_LINK( Cinematic )

SimplePersistFactoryClass<CinematicGameObjDef, CHUNKID_GAME_OBJECT_DEF_CINEMATIC>	_CinematicGameObjDefPersistFactory;

DECLARE_DEFINITION_FACTORY(CinematicGameObjDef, CLASSID_GAME_OBJECT_DEF_CINEMATIC, "Cinematic") _CinematicGameObjDefDefFactory;

CinematicGameObjDef::CinematicGameObjDef( void ) :
	SoundDefID( 0 ),
	AutoFireWeapon( false ),
	DestroyAfterAnimation( true ),
	CameraRelative( false )
{
	MODEL_DEF_PARAM( CinematicGameObjDef, PhysDefID, "DynamicAnimPhysDef" );
	EDITABLE_PARAM( CinematicGameObjDef, ParameterClass::TYPE_SOUNDDEFINITIONID,	SoundDefID );
	EDITABLE_PARAM( CinematicGameObjDef, ParameterClass::TYPE_STRING,	SoundBoneName );
	FILENAME_PARAM( CinematicGameObjDef, AnimationName, "Animation", ".W3D" );
	EDITABLE_PARAM( CinematicGameObjDef, ParameterClass::TYPE_BOOL,	AutoFireWeapon );
	EDITABLE_PARAM( CinematicGameObjDef, ParameterClass::TYPE_BOOL,	DestroyAfterAnimation );
	EDITABLE_PARAM( CinematicGameObjDef, ParameterClass::TYPE_BOOL,	CameraRelative );
}

uint32	CinematicGameObjDef::Get_Class_ID (void) const	
{ 
	return CLASSID_GAME_OBJECT_DEF_CINEMATIC; 
}

const PersistFactoryClass & CinematicGameObjDef::Get_Factory (void) const 
{ 
	return _CinematicGameObjDefPersistFactory; 
}

PersistClass *	CinematicGameObjDef::Create( void ) const 
{
	CinematicGameObj * obj = new CinematicGameObj;
	obj->Init( *this );
	return obj;
}

enum	{
	CHUNKID_DEF_PARENT							=	418001957,
	CHUNKID_DEF_VARIABLES,

	MICROCHUNKID_DEF_SOUND_DEF_ID				=	1,
	MICROCHUNKID_DEF_SOUND_BONE_NAME,
	XXX_MICROCHUNKID_DEF_ANIMATION_NAME,
	MICROCHUNKID_DEF_AUTO_FIRE_WEAPON,
	MICROCHUNKID_DEF_DESTROY_AFTER_ANIMATION,
	MICROCHUNKID_DEF_CAMERA_RELATIVE,
};

bool	CinematicGameObjDef::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_DEF_PARENT );
		ArmedGameObjDef::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_DEF_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_SOUND_DEF_ID,	SoundDefID );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_SOUND_BONE_NAME,	SoundBoneName );
		WRITE_MICRO_CHUNK_WWSTRING( csave, XXX_MICROCHUNKID_DEF_ANIMATION_NAME,	AnimationName );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_AUTO_FIRE_WEAPON,AutoFireWeapon );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_DESTROY_AFTER_ANIMATION, DestroyAfterAnimation );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_CAMERA_RELATIVE, CameraRelative );
	csave.End_Chunk();

	return true;
}

bool	CinematicGameObjDef::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_DEF_PARENT:
				ArmedGameObjDef::Load( cload );
				break;

			case CHUNKID_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_SOUND_DEF_ID,	SoundDefID );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_DEF_SOUND_BONE_NAME,	SoundBoneName );
						READ_MICRO_CHUNK_WWSTRING( cload, XXX_MICROCHUNKID_DEF_ANIMATION_NAME,	AnimationName );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_AUTO_FIRE_WEAPON,AutoFireWeapon );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_DESTROY_AFTER_ANIMATION, DestroyAfterAnimation );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_CAMERA_RELATIVE, CameraRelative );
						default:
							Debug_Say(( "Unrecognized CinematicDef Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized CinematicDef chunkID\n" ));
				break;
		}
		cload.Close_Chunk();
	}

	return true;
}

/*
** CinematicGameObj
*/
SimplePersistFactoryClass<CinematicGameObj, CHUNKID_GAME_OBJECT_CINEMATIC>	_CinematicGameObjPersistFactory;

const PersistFactoryClass & CinematicGameObj::Get_Factory (void) const 
{
	return _CinematicGameObjPersistFactory;
}


CinematicGameObj::CinematicGameObj()	:
	Sound( NULL )
{
	Set_App_Packet_Type(APPPACKETTYPE_CINEMATIC);
}

CinematicGameObj::~CinematicGameObj()
{
	Set_Sound( 0 );

	COMBAT_SCENE->Remove_From_Dirty_Cull_List( Peek_Physical_Object() );
}

/*
**
*/
void CinematicGameObj::Init( void )
{
	Init( Get_Definition() );
}

/*
**
*/
void	CinematicGameObj::Init( const CinematicGameObjDef & definition )
{
	ArmedGameObj::Init( definition );

	Cinematic_Init();
}

void	CinematicGameObj::Cinematic_Init( void )
{
	/*
	** (gth) cinematic game objects behave like animated terrain so they are in the 
	** terrain collision group 
	*/
	Peek_Physical_Object()->Set_Collision_Group( PhysicsSceneClass::COLLISION_GROUP_WORLD );
//	COMBAT_SCENE->Add_To_Dirty_Cull_List( Peek_Physical_Object() );

	Set_Sound( Get_Definition().SoundDefID, Get_Definition().SoundBoneName );
}

const CinematicGameObjDef & CinematicGameObj::Get_Definition( void ) const
{
	return (const CinematicGameObjDef &)BaseGameObj::Get_Definition();
}


/*
** CinematicGameObj Save and Load
*/
enum	{
	CHUNKID_PARENT							=	418002008,
	XXXCHUNKID_VARIABLES,
	XXXCHUNKID_ANIM_CONTROL,

	XXXMICROCHUNKID_PHYSICAL_OBJECT		=	1,
};

bool	CinematicGameObj::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_PARENT );
	ArmedGameObj::Save( csave );
	csave.End_Chunk();

	// We don't need to save the sound

	return true;
}

bool	CinematicGameObj::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PARENT:
				ArmedGameObj::Load( cload );
				break;

			default:
				Debug_Say(( "Unrecognized Cinematic chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	SaveLoadSystemClass::Register_Post_Load_Callback(this);
	return true;
}

void	CinematicGameObj::On_Post_Load( void )
{
	ArmedGameObj::On_Post_Load();

	Cinematic_Init();
}


/*
**
*/
void CinematicGameObj::Set_Sound( int sound_def_id, const char * bone_name )
{
	// Stop Old Sound
	if ( Sound != NULL ) {
		Sound->Stop();
		Sound->Attach_To_Object( NULL );
		Sound->Remove_From_Scene();
		Sound->Release_Ref();
		Sound = NULL;
	}

	// Start new Sound
	if ( sound_def_id != 0 ) {
		Sound = WWAudioClass::Get_Instance()->Create_Continuous_Sound( sound_def_id );
		if ( Sound != NULL ) {
			RenderObjClass * model = Peek_Model();
			WWASSERT( model );
			int bone_index = model->Get_Bone_Index( bone_name );
			Sound->Attach_To_Object( Peek_Model(),	bone_index );
			Sound->Add_To_Scene( true );
		}
	}
}

void	CinematicGameObj::Think( void )
{
{	WWPROFILE( "Cinematic Think" );

	// If auto fire weapon
	if ( Get_Definition().AutoFireWeapon ) {

		PhysicalGameObj * enemy = NULL;
		Vector3	my_pos;
		Get_Position( &my_pos );

		// if any enemies can be found in range
		SLNode<BaseGameObj> *objnode;
		for (	objnode = GameObjManager::Get_Game_Obj_List()->Head(); objnode; objnode = objnode->Next()) {
			PhysicalGameObj *obj = objnode->Data()->As_PhysicalGameObj();
			if ( obj && obj->Peek_Physical_Object() ) {	// zones have no phy obj  CHANGE THIS
				if ( obj == this ) {
					continue;
				}
				if ( Is_Teammate( obj ) ) {
					continue;
				}

				Vector3 v;
				obj->Get_Position(&v);
				v -= my_pos;
				if ( v.Length() < Get_Weapon()->Get_Range() ) {
					enemy = obj;
				}
			}
		}

		if ( enemy != NULL ) {
			Vector3 enemy_pos;
			enemy->Get_Position( &enemy_pos );
			enemy_pos.Z += 1;

			if ( Set_Targeting( enemy_pos ) == false ) {
				Get_Weapon()->Set_Primary_Triggered( false );
			} else {
				Get_Weapon()->Set_Primary_Triggered( true );		// Fire Primary
			}
		} else {
			Get_Weapon()->Set_Primary_Triggered( false );
		}

	}

	if ( Get_Definition().CameraRelative && COMBAT_CAMERA != NULL ) {
		Matrix3D tm = COMBAT_CAMERA->Get_Transform();
		tm.Rotate_Z( DEG_TO_RADF(90.0) );
		tm.Rotate_Y( DEG_TO_RADF(90.0) );
		Set_Transform( tm );
	}
}
	ArmedGameObj::Think();

}

void	CinematicGameObj::Post_Think( void )
{
	ArmedGameObj::Post_Think();

	WWPROFILE( "Cinematic PostThink" );

	// Animation is handled by the DynamicAnimPhysClass for this class
	WWASSERT(Get_Anim_Control() == NULL);

	if (Get_Definition().DestroyAfterAnimation) {
		PhysClass * pobj = Peek_Physical_Object();
		if (pobj != NULL) {
			DynamicAnimPhysClass * dpobj = pobj->As_DynamicAnimPhysClass();
			if ((dpobj != NULL) && 
				 (dpobj->Get_Animation_Manager().Peek_Animation() != NULL) && 
				 (dpobj->Get_Animation_Manager().Is_At_Target()) )
			{
				Set_Delete_Pending();
			}
		}
	}
}


void CinematicGameObj::Completely_Damaged( const OffenseObjectClass & damager ) 
{
	if ( Get_Definition().KilledExplosion != 0 ) {
		Vector3 pos;
		Get_Position(&pos);
		WWASSERT(pos.Is_Valid());// most likely candidate for explosion damage bug....?

		// If the object has a moving bounding box, use its center point for the explosion
		RenderObjClass * model = Peek_Model();
		if (model != NULL) {
			RenderObjClass * bbox = model->Get_Sub_Object_By_Name("BoundingBox");
			if (bbox != NULL) {
				Matrix3D	bbox_tm = bbox->Get_Transform();
				bbox_tm.Get_Translation(&pos);
				REF_PTR_RELEASE(bbox);
			}
		}

		ExplosionManager::Create_Explosion_At( Get_Definition().KilledExplosion, pos, damager.Get_Owner() );	 // no one gets credit for this
	}
	Set_Delete_Pending();
}


float	CinematicGameObj::Get_Animation_Length( void )
{
	float length = 0;

	//
	//	Try to get the dynamic anim phys object from the physics object
	//
	PhysClass *phys_obj = Peek_Physical_Object();
	if (phys_obj != NULL) {
		DynamicAnimPhysClass * dynamic_anim_phys = phys_obj->As_DynamicAnimPhysClass();
		if (dynamic_anim_phys != NULL) {

			//
			//	Peek at this object's animation
			//
			AnimCollisionManagerClass &anim_mgr = dynamic_anim_phys->Get_Animation_Manager();
			HAnimClass *anim = anim_mgr.Peek_Animation();
			if (anim != NULL) {

				//
				//	Return the length of the animation to the caller
				//
				length = anim->Get_Total_Time();
			}
		}
	}

	return length;
}



/*
void
CinematicGameObj::Import_Creation( BitStreamClass &packet )
{
	ArmedGameObj::Import_Creation (packet);
	return ;
}
*/


void	CinematicGameObj::Export_Rare( BitStreamClass &packet )
{
	ArmedGameObj::Export_Rare( packet );

	StringClass animation_name;
	AnimCollisionManagerClass::AnimModeType anim_mode = AnimCollisionManagerClass::ANIMATE_TARGET;

	//
	//	Dig the animation data out of the physics object
	//
	DynamicAnimPhysClass *dynanim = Peek_Physical_Object()->As_DynamicAnimPhysClass();
	if (dynanim != NULL) {		
		AnimCollisionManagerClass &anim_mgr = dynanim->Get_Animation_Manager();

		//
		//	Get the animation name
		//
		HAnimClass *anim = anim_mgr.Peek_Animation();
		if (anim != NULL) {
			animation_name = anim->Get_Name();
		}

		//
		//	Get the animation mode
		//
		anim_mode = anim_mgr.Get_Animation_Mode ();
	}

	//
	//	Send the animation data to the client
	//
	packet.Add_Terminated_String( (const char *)animation_name, true );
	packet.Add( anim_mode );
	return ;
}


void	CinematicGameObj::Import_Rare( BitStreamClass &packet )
{
	ArmedGameObj::Import_Rare( packet );

	//
	//	Get information about the animation
	//
	StringClass animation_name;
	int anim_mode = AnimCollisionManagerClass::ANIMATE_TARGET;
	packet.Get_Terminated_String( animation_name.Get_Buffer( 256 ), 256, true );
	packet.Get( anim_mode );

	//
	//	Pass the animation information onto the controller
	//
	DynamicAnimPhysClass *dynanim = Peek_Physical_Object()->As_DynamicAnimPhysClass();
	if (dynanim != NULL) {		
		AnimCollisionManagerClass &anim_mgr = dynanim->Get_Animation_Manager();
		anim_mgr.Set_Animation( animation_name );
		anim_mgr.Set_Animation_Mode( (AnimCollisionManagerClass::AnimModeType)anim_mode );
	}

	return ;
}
