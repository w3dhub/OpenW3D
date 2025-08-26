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
 *                     $Archive:: /Commando/Code/Combat/explosion.cpp                         $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 1/15/02 2:07p                                               $*
 *                                                                                             *
 *                    $Revision:: 69                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "explosion.h"
#include "debug.h"
#include "damage.h"
#include "combat.h"
#include "Sound3D.h"
#include "combatchunkid.h"
#include "simpledefinitionfactory.h"
#include "persistfactory.h"
#include "timeddecophys.h"
#include "WWAudio.h"
#include "gameobjmanager.h"
#include "physicalgameobj.h"
#include "crandom.h"
#include "assets.h"
#include "htree.h"
#include "hanim.h"
#include "smartgameobj.h"
#include "building.h"
#include "scexplosionevent.h"

/*
** ExplosionDefinitionClass
*/
SimplePersistFactoryClass<ExplosionDefinitionClass, CHUNKID_EXPLOSION_DEF>	_ExplosionDefPersistFactory;

DECLARE_DEFINITION_FACTORY(ExplosionDefinitionClass, CLASSID_DEF_EXPLOSION, "Explosion") _ExplosionDefDefFactory;

uint32	ExplosionDefinitionClass::Get_Class_ID (void) const	{ return CLASSID_DEF_EXPLOSION; }
const PersistFactoryClass & ExplosionDefinitionClass::Get_Factory (void) const { return _ExplosionDefPersistFactory; }

ExplosionDefinitionClass::ExplosionDefinitionClass( void ) :
	PhysDefID( 0 ),
	SoundDefID( 0 ),
	DamageRadius( 0 ),
	DamageStrength( 0 ),
	DamageWarhead( 0 ),
	DamageIsScaled( true ),
	DecalSize( 10 ),
	AnimatedExplosion( true ),
	CameraShakeIntensity( 0.0f ),
	CameraShakeRadius( 25.0f ),
	CameraShakeDuration( 1.5f )
{
#ifdef PARAM_EDITING_ON
	MODEL_DEF_PARAM( ExplosionDefinitionClass, PhysDefID, "TimedDecorationPhysDef" );
	EDITABLE_PARAM( ExplosionDefinitionClass, ParameterClass::TYPE_SOUNDDEFINITIONID,	SoundDefID );
	EDITABLE_PARAM( ExplosionDefinitionClass, ParameterClass::TYPE_FLOAT,					DamageRadius );
	EDITABLE_PARAM( ExplosionDefinitionClass, ParameterClass::TYPE_FLOAT,					DamageStrength );

//	EDITABLE_PARAM( ExplosionDefinitionClass, ParameterClass::TYPE_INT,						DamageWarhead );
	EnumParameterClass *param;
	param = new EnumParameterClass( &DamageWarhead );
	param->Set_Name ( "Warhead" );
	for ( int i = 0; i < ArmorWarheadManager::Get_Num_Warhead_Types(); i++ ) {
		param->Add_Value ( ArmorWarheadManager::Get_Warhead_Name( i ), i );
	}
	GENERIC_EDITABLE_PARAM(ExplosionDefinitionClass,param)

	EDITABLE_PARAM( ExplosionDefinitionClass, ParameterClass::TYPE_BOOL,						DamageIsScaled );
	EDITABLE_PARAM( ExplosionDefinitionClass, ParameterClass::TYPE_FILENAME, 				DecalFilename );
	EDITABLE_PARAM( ExplosionDefinitionClass, ParameterClass::TYPE_FLOAT,					DecalSize );
	EDITABLE_PARAM( ExplosionDefinitionClass, ParameterClass::TYPE_BOOL, 					AnimatedExplosion );

	FLOAT_EDITABLE_PARAM( ExplosionDefinitionClass, CameraShakeIntensity, 0.0f, 1.0f);
	FLOAT_UNITS_PARAM( ExplosionDefinitionClass, CameraShakeRadius, 0.01f, 1000.0f, "meters");
	FLOAT_UNITS_PARAM( ExplosionDefinitionClass, CameraShakeDuration, 0.01f, 60.0f, "seconds");
#endif	//PARAM_EDITING_ON
}

/*
**
*/
enum	{
	CHUNKID_EXPLOSION_DEF_VARIABLES						=	0317001525,
	CHUNKID_EXPLOSION_DEF_PARENT,

	MICROCHUNKID_EXPLOSION_DEF_PHYS_DEF_ID				=	1,
	MICROCHUNKID_EXPLOSION_DEF_SOUND_DEF_ID,
	MICROCHUNKID_EXPLOSION_DEF_DAMAGE_RADIUS,
	MICROCHUNKID_EXPLOSION_DEF_DAMAGE_STRENGTH,
	MICROCHUNKID_EXPLOSION_DEF_DAMAGE_WARHEAD,
	MICROCHUNKID_EXPLOSION_DEF_DAMAGE_IS_SCALED,
	MICROCHUNKID_EXPLOSION_DEF_DECAL_FILENAME,
	MICROCHUNKID_EXPLOSION_DEF_DECAL_SIZE,
	XXXMICROCHUNKID_EXPLOSION_DEF_ANIMATION,
	MICROCHUNKID_EXPLOSION_DEF_ANIMATED_EXPLOLSION,
	MICROCHUNKID_EXPLOSION_DEF_CAMERASHAKEINTENSITY,
	MICROCHUNKID_EXPLOSION_DEF_CAMERASHAKERADIUS,
	MICROCHUNKID_EXPLOSION_DEF_CAMERASHAKEDURATION,
};

bool	ExplosionDefinitionClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_EXPLOSION_DEF_PARENT );
		DefinitionClass::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_EXPLOSION_DEF_VARIABLES );
		WRITE_MICRO_CHUNK( csave,	MICROCHUNKID_EXPLOSION_DEF_PHYS_DEF_ID,		PhysDefID );
		WRITE_MICRO_CHUNK( csave,	MICROCHUNKID_EXPLOSION_DEF_SOUND_DEF_ID,		SoundDefID );
		WRITE_MICRO_CHUNK( csave,	MICROCHUNKID_EXPLOSION_DEF_DAMAGE_RADIUS,		DamageRadius )
		WRITE_MICRO_CHUNK( csave,	MICROCHUNKID_EXPLOSION_DEF_DAMAGE_STRENGTH,	DamageStrength );
		WRITE_MICRO_CHUNK( csave,	MICROCHUNKID_EXPLOSION_DEF_DAMAGE_WARHEAD,	DamageWarhead );
		WRITE_MICRO_CHUNK( csave,	MICROCHUNKID_EXPLOSION_DEF_DAMAGE_IS_SCALED,	DamageIsScaled );
		WRITE_MICRO_CHUNK_WWSTRING( csave,	MICROCHUNKID_EXPLOSION_DEF_DECAL_FILENAME,	DecalFilename );
		WRITE_MICRO_CHUNK( csave,	MICROCHUNKID_EXPLOSION_DEF_DECAL_SIZE,			DecalSize );
		WRITE_MICRO_CHUNK( csave,	MICROCHUNKID_EXPLOSION_DEF_ANIMATED_EXPLOLSION,		AnimatedExplosion );
		WRITE_MICRO_CHUNK( csave,	MICROCHUNKID_EXPLOSION_DEF_CAMERASHAKEINTENSITY,	CameraShakeIntensity);
		WRITE_MICRO_CHUNK( csave,	MICROCHUNKID_EXPLOSION_DEF_CAMERASHAKERADIUS,		CameraShakeRadius);
		WRITE_MICRO_CHUNK( csave,	MICROCHUNKID_EXPLOSION_DEF_CAMERASHAKEDURATION,		CameraShakeDuration);
	csave.End_Chunk();

	return true;
}

bool	ExplosionDefinitionClass::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_EXPLOSION_DEF_PARENT:
				DefinitionClass::Load( cload );
				break;

			case CHUNKID_EXPLOSION_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload,	MICROCHUNKID_EXPLOSION_DEF_PHYS_DEF_ID,		PhysDefID );
						READ_MICRO_CHUNK( cload,	MICROCHUNKID_EXPLOSION_DEF_SOUND_DEF_ID,		SoundDefID );
						READ_MICRO_CHUNK( cload,	MICROCHUNKID_EXPLOSION_DEF_DAMAGE_RADIUS,		DamageRadius )
						READ_MICRO_CHUNK( cload,	MICROCHUNKID_EXPLOSION_DEF_DAMAGE_STRENGTH,	DamageStrength );
						READ_MICRO_CHUNK( cload,	MICROCHUNKID_EXPLOSION_DEF_DAMAGE_WARHEAD,	DamageWarhead );
						READ_MICRO_CHUNK( cload,	MICROCHUNKID_EXPLOSION_DEF_DAMAGE_IS_SCALED,	DamageIsScaled );
						READ_MICRO_CHUNK_WWSTRING( cload,	MICROCHUNKID_EXPLOSION_DEF_DECAL_FILENAME,	DecalFilename );
						READ_MICRO_CHUNK( cload,	MICROCHUNKID_EXPLOSION_DEF_DECAL_SIZE,			DecalSize );
						READ_MICRO_CHUNK( cload,	MICROCHUNKID_EXPLOSION_DEF_ANIMATED_EXPLOLSION,		AnimatedExplosion );
						READ_MICRO_CHUNK( cload,	MICROCHUNKID_EXPLOSION_DEF_CAMERASHAKEINTENSITY,	CameraShakeIntensity);
						READ_MICRO_CHUNK( cload,	MICROCHUNKID_EXPLOSION_DEF_CAMERASHAKERADIUS,		CameraShakeRadius);
						READ_MICRO_CHUNK( cload,	MICROCHUNKID_EXPLOSION_DEF_CAMERASHAKEDURATION,		CameraShakeDuration);
						default:
							Debug_Say(( "Unrecognized ExplosionDef Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized ExplosionDef chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

/*
**
*/
void	ExplosionManager::Create_Explosion_At( int explosion_def_id, const Vector3 & pos, ArmedGameObj * damager, const Vector3 & blast_direction, DamageableGameObj * force_victim )
{
	Matrix3D up_tm(1);
	up_tm.Set_Translation( pos );

	Create_Explosion_At( explosion_def_id, up_tm, damager, blast_direction, force_victim );
}

void	ExplosionManager::Create_Explosion_At( int explosion_def_id, const Matrix3D & up_tm, ArmedGameObj * damager, const Vector3 & blast_direction, DamageableGameObj * force_victim )
{
	// Make a camera convention transform pointing in the blast direction
	Vector3 pos = up_tm.Get_Translation();

#ifdef WWDEBUG
	if (blast_direction.Length2() < WWMATH_EPSILON) {
		WWDEBUG_SAY(("ExplosionManager::Create_Explosion_At - Invalid Blast Direction! %f %f %f\r\n",blast_direction.X,blast_direction.Y,blast_direction.Z));
	}
#endif

	Matrix3D blast_tm;
	blast_tm.Look_At( pos, pos + blast_direction, FreeRandom.Get_Float( 0, DEG_TO_RADF( 360 ) ) );

	// Find Explosion Def
	ExplosionDefinitionClass * explosion_def = (ExplosionDefinitionClass *)DefinitionMgrClass::Find_Definition( explosion_def_id );
	if ( explosion_def == NULL ) {
		Debug_Say(( "Explosion Def %d not found\n", explosion_def_id ));
		return;
	}

	WWASSERT( explosion_def );

	// Make the Phys Object
	if ( explosion_def->PhysDefID != 0 ) {
	//if ( explosion_def->PhysDefID != 0 ) {
		PhysDefClass * phys_def = (PhysDefClass *)DefinitionMgrClass::Find_Definition( explosion_def->PhysDefID );
		if ( phys_def != NULL ) {
			WWASSERT( phys_def );
			WWASSERT( phys_def->Is_Type( "TimedDecorationPhysDef" ) );
			TimedDecorationPhysClass * explosion = (TimedDecorationPhysClass *)phys_def->Create();
			if ( explosion ) {
				RenderObjClass * model = explosion->Peek_Model();
				WWASSERT(model != NULL);
				if (model != NULL) {

					explosion->Set_Transform( up_tm );

					if (model->Get_HTree() != NULL && explosion_def->AnimatedExplosion) {

						// Auto play an explosion anim if we find it
						StringClass	exp_anim_name;
						exp_anim_name.Format( "%s.%s",
							model->Get_HTree()->Get_Name(),
							model->Get_HTree()->Get_Name() );
						WWASSERT(WW3DAssetManager::Get_Instance() != NULL);
						HAnimClass * anim = WW3DAssetManager::Get_Instance()->Get_HAnim( exp_anim_name );
						if ( anim != NULL ) {
							model->Set_Animation( anim, 0, RenderObjClass::ANIM_MODE_ONCE );
							anim->Release_Ref();
						}
					}

					WWASSERT(COMBAT_SCENE != NULL);
					COMBAT_SCENE->Add_Dynamic_Object( explosion );
				}
				explosion->Release_Ref();

			}
		}
	}

	// Make the decal
	if ( !explosion_def->DecalFilename.Is_Empty() ) {
		StringClass	new_name(true);
		::Strip_Path_From_Filename( new_name, explosion_def->DecalFilename );
		PhysicsSceneClass::Get_Instance()->Create_Decal( blast_tm, new_name,
																		explosion_def->DecalSize, false, NULL );
	}


	// Apply the damage
//	if ( CombatManager::I_Am_Server() )		 Clients can create damage now...
	{
		float radius = explosion_def->DamageRadius;
		WWASSERT(WWMath::Is_Valid_Float(radius));
		if (radius > 0.0f) {

			WWASSERT(pos.Is_Valid());

			// Create an offense object to carry the damage information
			OffenseObjectClass offense( explosion_def->DamageStrength, explosion_def->DamageWarhead, damager );

			// Loop over all game objects, appling damage to those close enough
			SLNode<BaseGameObj> *objnode;
			for (	objnode = GameObjManager::Get_Game_Obj_List()->Head(); objnode; objnode = objnode->Next()) {

				PhysicalGameObj *obj = objnode->Data()->As_PhysicalGameObj();

				// If this object is the force victim, give him it all!
				if ( objnode->Data() == force_victim ) {

					// physical objs take extended damage
					if ( obj != NULL ) {
						obj->Apply_Damage_Extended( offense );
					} else {
						force_victim->Apply_Damage( offense );
					}

				} else {

					if ( obj && obj->Peek_Physical_Object() ) {	// zones have no phy obj  CHANGE THIS

						if ( !obj->Takes_Explosion_Damage() ) {	// Cinematics shouldn't take explosion damage
							continue;										// Because that don't exist where they are drawn
						}

						Vector3 obj_pos,v;
						obj_pos = obj->Get_Bullseye_Position();
						WWASSERT(obj_pos.Is_Valid());
						v = obj_pos - pos;
						float dist = v.Length();

#ifdef WWDEBUG
if (!WWMath::Is_Valid_Float(dist)) {
	WWDEBUG_SAY(("Explosion Distance Bug!\r\n"));
	Vector3 obj_pos;
	obj->Get_Position(&obj_pos);
	WWDEBUG_SAY(("  explosion pos: %f, %f, %f  object pos: %f, %f, %f\r\n",pos.X,pos.Y,pos.Z,obj_pos.X,obj_pos.Y,obj_pos.Z));
	WWDEBUG_SAY(("  object definition name: %s\r\n", obj->Get_Definition().Get_Name()));
	WWDEBUG_SAY(("  explosion definition name; %s\r\n", explosion_def->Get_Name()));
}
#endif
						WWASSERT(WWMath::Is_Valid_Float(dist));
						if ( dist <= radius ) {
							float scale = 1.0f;
							if ( explosion_def->DamageIsScaled ) {
								WWASSERT(radius > WWMATH_EPSILON);
								scale = 1.0 - (dist / radius);
								WWASSERT(WWMath::Is_Valid_Float(scale));
							}

							// Check for collisions in the path of the object
							CastResultStruct res;
							LineSegClass ray(obj_pos,pos);
							PhysRayCollisionTestClass raytest(ray,&res,obj->Peek_Physical_Object()->Get_Collision_Group(),COLLISION_TYPE_PROJECTILE);
							raytest.CheckDynamicObjs = false;

							PhysicsSceneClass::Get_Instance()->Cast_Ray(raytest);

							if (res.Fraction < 1.0f - WWMATH_EPSILON) {
								scale *= 0.25f;
							}

							// Apply the damage
							obj->Apply_Damage_Extended( offense, scale, v );
						}
					}
				}
			}
		}
	}

	// Make the Sound
	if ( explosion_def->SoundDefID != 0 ) {
		RefCountedGameObjReference *owner_ref = new RefCountedGameObjReference;
		owner_ref->Set_Ptr( damager );
		WWAudioClass::Get_Instance()->Create_Instant_Sound( explosion_def->SoundDefID, up_tm, owner_ref );
		REF_PTR_RELEASE( owner_ref );
	}

	// Make the camera shake!
	if ( explosion_def->CameraShakeIntensity > 0.0f ) {
		COMBAT_SCENE->Add_Camera_Shake(	pos,
													explosion_def->CameraShakeRadius,
													explosion_def->CameraShakeDuration,
													explosion_def->CameraShakeIntensity	);

	}

}


void	ExplosionManager::Explosion_Damage_Building( int explosion_def_id, BuildingGameObj * building, bool mct_damage, ArmedGameObj * damager )
{
//	if ( CombatManager::I_Am_Server() )			Clients can make damage now
	{
		// Find Explosion Def
		ExplosionDefinitionClass * explosion_def = (ExplosionDefinitionClass *)DefinitionMgrClass::Find_Definition( explosion_def_id );
		WWASSERT( explosion_def );

		OffenseObjectClass offense( explosion_def->DamageStrength, explosion_def->DamageWarhead, damager );
		building->Apply_Damage_Building( offense, mct_damage );
	}
}


/*
** Broadcastes server explosion event
*/
void	ExplosionManager::Server_Explode( int explosion_def_id, const Vector3 & pos, int owner_id, DamageableGameObj * force_victim )
{
	WWASSERT( CombatManager::I_Am_Server() );

	cScExplosionEvent * event = new cScExplosionEvent();
	if ( event ) {
		int victim_id = 0;
		if ( force_victim ) {
			victim_id = force_victim->Get_ID();
		}
		event->Init( explosion_def_id, pos, owner_id, victim_id );
	}
}

void	ExplosionManager::Explode( int explosion_def_id, const Vector3 & pos, int owner_id, int victim_id )
{
	if ( explosion_def_id != 0 ) {
		ArmedGameObj * owner = NULL;
		if ( owner_id != 0 ) {
			owner = GameObjManager::Find_SmartGameObj( owner_id );
		}
		DamageableGameObj * force_victim = NULL;
		if ( victim_id != 0 ) {
			force_victim = GameObjManager::Find_PhysicalGameObj( victim_id );
		}
		Create_Explosion_At( explosion_def_id, pos, owner, Vector3( 0,0,-1), force_victim );
	}
}




/* 04/23/01 - reenabling - in the hopes that sr was to blame
#ifdef WWDEBUG
	char computer_name[200];
	DWORD size = sizeof(computer_name);
	GetComputerName(computer_name, &size);
	if (cMiscUtil::Is_String_Same(computer_name, "TOMSS2")) {
		//return;
		is_enabled = false;
	}
#endif // WWDEBUG
/**/