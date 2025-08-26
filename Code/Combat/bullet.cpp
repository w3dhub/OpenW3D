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
 *                     $Archive:: /Commando/Code/Combat/bullet.cpp                            $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/13/02 5:22p                                               $*
 *                                                                                             *
 *                    $Revision:: 164                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "bullet.h"
#include "projectile.h"
#include "combat.h"	  // for collision groups (should be damage.h?)
#include "pscene.h"
#include "weaponmanager.h"
#include "assets.h"
#include "debug.h"
#include "armedgameobj.h"
#include "crandom.h"
#include "surfaceeffects.h"
#include "mesh.h"
#include "WWAudio.h"
#include "explosion.h"
#include "building.h"
#include "persistfactory.h"
#include "wwphysids.h"
#include "damageablestaticphys.h"
#include "wwprofile.h"
#include "part_emt.h"
#include "realcrc.h"
#include "soldier.h"
#include "segline.h"
#include "timeddecophys.h"
#include "simplegameobj.h"
#include "c4.h"


/*
** Constants for this module
*/
const float _INSTANT_BULLET_THRESHHOLD	= 400;  // If speed is >= _INSTANT_BULLET_THRESHHOLD m/s, treat it an an instant bullet


/**
** BeamEffectManagerClass
** This is a static object that simply manages the active beam effect.  Its job is to cache un-used beams
** so that we aren't allocating them at run time and to add new beam effects to the scene when needed.
*/
class BeamEffectManagerClass : public CombatPhysObserverClass
{
public:
	void		Init( void )		{}
	void		Shutdown( void );

	/*
	** Create a beam effect and put it in the scene.  Whenever possible, this function
	** tries to recycle a previously created beam effect
	*/
	void								Create_Beam_Effect(const AmmoDefinitionClass * ammo_def,const Vector3 & p0,const Vector3 & p1);


	/*
	** PhysObserver Interface - whenever a beam expires, we put it into a list so we can
	** recycle it next time we need a beam effect
	*/
	virtual void					Object_Removed_From_Scene(PhysClass * observed_obj);


private:

	void								Internal_Get_New_Beam(SegmentedLineClass ** beam_model,TimedDecorationPhysClass ** beam_wrapper);

	RefMultiListClass<TimedDecorationPhysClass>	UnusedBeamList;
};

static BeamEffectManagerClass _TheBeamEffectManager;


void	BeamEffectManagerClass::Shutdown( void )
{
	while (UnusedBeamList.Peek_Head()) {
		UnusedBeamList.Release_Head();
	}
}

void BeamEffectManagerClass::Create_Beam_Effect(const AmmoDefinitionClass * ammo_def,const Vector3 & p0,const Vector3 & p1)
{
	SegmentedLineClass * beam_model = NULL;
	TimedDecorationPhysClass * beam_wrapper = NULL;

	Internal_Get_New_Beam(&beam_model,&beam_wrapper);

	// line effect from "start" to "data.Position"
	static Vector3 points[2];
	points[0] = p0;
	points[1] = p1;

	const int BEAM_SUBDIVISION				= 4;
	const float BASE_AMPLITUDE				= 0.3f;
	const float AMPLITUDE_PER_METER		= 0.04f;

	/*
	** User controlled settings
	*/
	TextureClass * beam_texture = Get_Texture_From_Filename(ammo_def->BeamTexture);
	beam_model->Set_Texture(beam_texture);
	REF_PTR_RELEASE(beam_texture);

	beam_model->Set_Points(2,points);
	beam_model->Set_Width(ammo_def->BeamWidth);
	beam_model->Set_Color(ammo_def->BeamColor);

	beam_model->Set_UV_Offset_Rate(Vector2(0,1.0f / ammo_def->BeamTime));

	if (ammo_def->BeamSubdivisionEnabled) {
		beam_model->Set_Noise_Amplitude((BASE_AMPLITUDE + AMPLITUDE_PER_METER * (p1-p0).Quick_Length()) * ammo_def->BeamSubdivisionScale);
		beam_model->Set_Subdivision_Levels(BEAM_SUBDIVISION);
	} else {
		beam_model->Set_Noise_Amplitude(0.0f);
		beam_model->Set_Subdivision_Levels(0);
	}

	/*
	** Constants; I don't let the user control these for now
	*/
	beam_model->Set_Texture_Tile_Factor(1.0f); // Only relevant for TILED_TEXTURE_MAP mode
	beam_model->Set_Texture_Mapping_Mode(SegLineRendererClass::UNIFORM_WIDTH_TEXTURE_MAP);
	beam_model->Set_Opacity(1.0f);
	beam_model->Set_Shader(ShaderClass::_PresetAdditiveShader);
	beam_model->Set_Merge_Abort_Factor(1.0f);
	beam_model->Set_Merge_Intersections(false);
	beam_model->Set_Disable_Sorting(false);
	beam_model->Set_Freeze_Random(ammo_def->BeamSubdivisionFrozen);
	beam_model->Set_End_Caps(ammo_def->BeamEndCaps);
	beam_model->Reset_Line();

	beam_wrapper->Set_Lifetime(ammo_def->BeamTime);
	beam_wrapper->Set_Transform(Matrix3D(1));
	beam_wrapper->Enable_Is_Pre_Lit(true);

	COMBAT_SCENE->Add_Dynamic_Object(beam_wrapper);

	REF_PTR_RELEASE(beam_wrapper);
	REF_PTR_RELEASE(beam_model);
	REF_PTR_RELEASE(beam_texture);
}

void BeamEffectManagerClass::Object_Removed_From_Scene(PhysClass * observed_obj)
{
	/*
	** One of the "beams" that we are observing expired and was removed from the scene so
	** lets put it in the list for future re-use
	*/
	UnusedBeamList.Add((TimedDecorationPhysClass *)observed_obj);
}

void BeamEffectManagerClass::Internal_Get_New_Beam(SegmentedLineClass ** set_beam_model,TimedDecorationPhysClass ** set_beam_wrapper)
{
	if (UnusedBeamList.Is_Empty()) {
		SegmentedLineClass * beam_model = NEW_REF(SegmentedLineClass,());

		TimedDecorationPhysClass * beam_wrapper = NEW_REF(TimedDecorationPhysClass,());
		beam_wrapper->Enable_Dont_Save(true);
		beam_wrapper->Set_Collision_Group(UNCOLLIDEABLE_GROUP);
		beam_wrapper->Set_Model(beam_model);
		beam_wrapper->Set_Observer(this);
		*set_beam_model = beam_model;
		*set_beam_wrapper = beam_wrapper;	// refs returned to the caller

	} else {

		*set_beam_wrapper = UnusedBeamList.Remove_Head(); // refs returned to the caller
		*set_beam_model = (SegmentedLineClass *)((*set_beam_wrapper)->Get_Model());

	}
}



/*
** Core Bullet Data for simulation
*/
class BulletDataClass {
public:
	BulletDataClass(  const AmmoDefinitionClass * def = NULL, const ArmedGameObj * owner = NULL,
							const Vector3 & position = Vector3(0,0,0), const Vector3 & velocity = Vector3(0,0,0) ) :
		AmmoDefinition( def ),
		Owner( owner ),
		Position( position ),
		Velocity( velocity ),
		SoftPierceCount( 0 ),
		Destroy( false ),
		DidExplode( false ),
		LastHitID( 0 ),
		GrenadeSafetyTimer( 0 )
	{}

	ArmedGameObj * Get_Owner( void ) const 	{ return (ArmedGameObj *)Owner.Get_Ptr(); }

	CollisionReactionType	Bullet_Collision_Occurred( const CollisionEventClass & event );
	ExpirationReactionType	Bullet_Expired();

	const AmmoDefinitionClass	*AmmoDefinition;
	GameObjReference				Owner;
	Vector3							Position;
	Vector3							Velocity;
	int								SoftPierceCount;
	bool							Destroy;
	bool							DidExplode;
	int								LastHitID;
	float							GrenadeSafetyTimer;
};

/*
** Bullet_Collision_Occured
*/
CollisionReactionType BulletDataClass::Bullet_Collision_Occurred( const CollisionEventClass & event )
{
	WWPROFILE( "Bullet Collision Occurred" );

	WWASSERT(event.OtherObj);
	WWASSERT(event.CollisionResult != NULL);
//	WWDEBUG_SAY(( "Bullet Collision\n" ));

	//
	// Pre-calculate some useful variables
	//
	CollisionReactionType return_value = COLLISION_REACTION_DEFAULT;
	OffenseObjectClass offense( AmmoDefinition->Damage, (int)AmmoDefinition->Warhead, Get_Owner() );
	offense.EnableClientDamage = true;	// Only bullets apply to client damage;

	const Vector3 & collision_point = event.CollisionResult->ContactPoint;
	const Vector3 & collision_normal = event.CollisionResult->Normal;

	PhysicalGameObj * other = NULL;
	BuildingGameObj * building = NULL;
	if ( event.OtherObj->Get_Observer() != NULL ) {
		other = ((CombatPhysObserverClass *)event.OtherObj->Get_Observer())->As_PhysicalGameObj();
		building = ((CombatPhysObserverClass *)event.OtherObj->Get_Observer())->As_BuildingGameObj();
	}

	//
	// If the bullet hits its owner or its owner's vehicle, allow it to continue
	// Also, if it is a simple "Hidden Object", allow the bullet to continue
	//
	if ( other != NULL && Get_Owner() != NULL ) {

		SimpleGameObj * simple = other->As_SimpleGameObj();
		VehicleGameObj * vehicle = other->As_VehicleGameObj();
		SoldierGameObj * soldier = Get_Owner()->As_SoldierGameObj();

		if ( vehicle != NULL && soldier != NULL && vehicle == soldier->Get_Vehicle() ) {
//			Debug_Say(( "Bullet Hitting its owner's vehicle\n" ));
			return COLLISION_REACTION_NO_BOUNCE;
		}
		if ( other == Get_Owner() ) {
//			Debug_Say(( "Bullet Hitting its owner\n" ));
			return COLLISION_REACTION_NO_BOUNCE;
		}
		if ((simple != NULL) && (simple->Is_Hidden_Object())) {
			return COLLISION_REACTION_NO_BOUNCE;
		}
	}

	//
	// Apply Graphical Effects.
	// If the mesh is shatterable it is shattered,  Apply a surface effect depending on the surface type
	// that was collided with.  In the case that the mesh was shattered, we disable surface effect decals.
	//
	bool shattered = false;
	if ((event.CollidedRenderObj != NULL) && (event.CollidedRenderObj->Class_ID() == RenderObjClass::CLASSID_MESH)) {
		MeshClass * mesh = (MeshClass *)event.CollidedRenderObj;

		if ((mesh->Get_W3D_Flags() & W3D_MESH_FLAG_SHATTERABLE) && (mesh->Is_Not_Hidden_At_All())) {

			PhysicsSceneClass::Get_Instance()->Shatter_Mesh(	mesh,
																				collision_point,
																				collision_normal,
																				Velocity	);

			// remeber that we shattered the mesh so that we don't create decals later
			shattered = true;

			// Bullets always pass through shattered objects because we sometimes have two-layered
			// windows that have different materials on the two sides.  We don't want bullets to ever
			// shatter only one "side" of a window!
			return_value = COLLISION_REACTION_NO_BOUNCE;
		}
	}

	// Ignore backfaces (after shattering for windows)
	float dot = Vector3::Dot_Product( event.CollisionResult->Normal, Velocity );
	if ( dot > 0 ) {
		return COLLISION_REACTION_NO_BOUNCE;
	}

	//
	// Apply Damage
	// There are several types of objects that can be damaged: normal dynamic game objects, damageable static
	// physics objects, and buildings.  A building is damaged whenever any of its meshes or aggregates are
	// hit by a bullet.
	// Also, we only apply damage if the mesh wasn't shattered.  Bullets pretend like nothing happened
	// if they shatter something.
	//
	if (!shattered) {

		const char * collided_obj_name = NULL;
		if ( event.CollidedRenderObj != NULL ) {
			collided_obj_name = event.CollidedRenderObj->Get_Name();
		}

		if ( other ) {	// if hitting a game object,

			if ( other->Get_ID() == LastHitID ) {
//				Debug_Say(( "Double Hit\n" ));
				return COLLISION_REACTION_NO_BOUNCE;
			}

			if ( GrenadeSafetyTimer > 0 ) {
				// only if the other is not an enemy
				if ( other && Get_Owner() && !other->Is_Enemy( Get_Owner() ) ) {
					return COLLISION_REACTION_DEFAULT;		// Just bounce
				}
			}

			LastHitID = other->Get_ID();

			// Apply Bullet Damage to a game object
			// Allow it to happen for clients, so we can get the repair effect
//			if ( CombatManager::I_Am_Server() ) {
				other->Apply_Damage_Extended( offense, 1.0f, Velocity, collided_obj_name );

				// If the bullet is hitting C4, also apply damage to the object the C4 is
				// attached to.  This is a fix for the "C4 armor" exploit.
				if (other->As_C4GameObj() != NULL) {
					ScriptableGameObj * host = other->As_C4GameObj()->Get_Stuck_Object();
					if ((host != NULL) && (host->As_PhysicalGameObj() != NULL)) {
						host->As_PhysicalGameObj()->Apply_Damage_Extended( offense, 1.0f, Velocity, NULL );
					}
				}
//			}

			if ( ( !other->Is_Soft() ) ||
				  ( ++SoftPierceCount >= AmmoDefinition->SoftPierceLimit ) ) {

				if ( AmmoDefinition->ExplosionDefID ) {
					ExplosionManager::Create_Explosion_At( AmmoDefinition->ExplosionDefID, collision_point, Get_Owner(), -collision_normal, other );
				}

				Destroy = true;
				return_value = COLLISION_REACTION_STOP_MOTION;
			} else {
				return_value = COLLISION_REACTION_NO_BOUNCE;
			}

		} else {		// if hitting terain

			if ( GrenadeSafetyTimer > 0 ) {
				Debug_Say(( "Safety\n" ));
				return COLLISION_REACTION_DEFAULT;		// Just bounce
			}

			// If hitting a building, apply damage to it
			if ( building != NULL ) { //	&& CombatManager::I_Am_Server() ) {
				building->Apply_Damage_Building( offense, event.OtherObj->As_StaticPhysClass() );
			}

			// Check for damage to a DamageableStaticPhys object
			// For now I'm using the persist factory chunk-ID for RTTI... If a better solution
			// turns up we should change this.
			if (	event.OtherObj->Get_Factory().Chunk_ID() == PHYSICS_CHUNKID_DAMAGEABLESTATICPHYS 
//					&& CombatManager::I_Am_Server() 
				) 
			{
				DamageableStaticPhysClass * damphys = (DamageableStaticPhysClass *)event.OtherObj;
				OffenseObjectClass offense( AmmoDefinition->Damage, (int)AmmoDefinition->Warhead, Get_Owner() );
				offense.EnableClientDamage = true;	// Only bullets apply to client damage;
				damphys->Apply_Damage_Static( offense );
			}

			// check for a non-stopping surface
			if ( !SurfaceEffectsManager::Does_Surface_Stop_Bullets( event.CollisionResult->SurfaceType ) ) {
				return_value = COLLISION_REACTION_NO_BOUNCE;
			} else {

				// If the ammo explodes when it hits terrain, then create an explosion and kill the bullet
				if ( AmmoDefinition->TerrainActivated ) {

					if ( AmmoDefinition->ExplosionDefID ) {
						ExplosionManager::Create_Explosion_At( AmmoDefinition->ExplosionDefID, collision_point, Get_Owner(), -collision_normal, building );
						DidExplode = true;
					}

					Destroy = true;
					return_value = COLLISION_REACTION_STOP_MOTION;
				}
			}
		}
	}

	// Only make a bullet hit if it doesn't make an explosion
	if ( !DidExplode ) {
		Matrix3D surface_tm;
		surface_tm.Look_At(collision_point,collision_point-collision_normal,FreeRandom.Get_Float(0,DEG_TO_RADF(360)));
		int my_type = AmmoDefinition->HitterType;
		bool allow_decals = (shattered == false);
		if ( (float)AmmoDefinition->Damage < 0.0f ) {
			allow_decals = false;
		}
		SurfaceEffectsManager::Apply_Effect( event.CollisionResult->SurfaceType, my_type, surface_tm, event.OtherObj, Get_Owner(), allow_decals );
	}

	return return_value;
}

ExpirationReactionType	BulletDataClass::Bullet_Expired( void )
{

	Destroy = true;

	if ( AmmoDefinition->TimeActivated && AmmoDefinition->ExplosionDefID && !DidExplode ) {
		ExplosionManager::Create_Explosion_At( AmmoDefinition->ExplosionDefID, Position, Get_Owner() );
	}

	return EXPIRATION_DENIED; // Don't let it die, I'll pull it from the scene later
}


/*
** BulletClass (for non-instant bullets)
*/
class	BulletClass : public CombatPhysObserverClass, public MultiListObjectClass, public PostLoadableClass {

public:
	~BulletClass( void );

	void		Init( const BulletDataClass & data, float progress_time, const Vector3 & target, DamageableGameObj * target_object = NULL );
	void		Shutdown( void );
	void		Think( void );

	// Save / Load
	virtual	bool	Save( ChunkSaveClass & csave );
	virtual	bool	Load( ChunkLoadClass & cload );
	virtual	void	On_Post_Load(void);
	bool	Is_Valid( void )	{ return BulletData.AmmoDefinition != NULL; }

	// Collision
	virtual CollisionReactionType		Collision_Occurred( const CollisionEventClass & event );
	virtual ExpirationReactionType	Object_Expired( PhysClass * observed_obj );

private:
	BulletDataClass				BulletData;
	ProjectileClass *				Projectile;			//	for physics
	Vector3							TargetVector;
	GameObjReference				TargetObject;
	float								TrackingErrorTimer;
	Vector3							TrackingError;
	long								ModelNameCRC;

	BulletClass( void );

	friend	class		BulletManager;

};


BulletClass::BulletClass( void ) :
	Projectile( NULL ),
	TargetVector( 0,0,0 ),
	TrackingErrorTimer( 0 ),
	TrackingError( 0,0,0 ),
	ModelNameCRC( 0 )
{
	WWASSERT( Projectile == NULL );
	Projectile = NEW_REF( ProjectileClass, () );
   Projectile->Set_Collision_Group( BULLET_COLLISION_GROUP );
}

BulletClass::~BulletClass( void )
{
	if ( Projectile != NULL ) {
		Projectile->Set_Observer(NULL);
		Projectile->Release_Ref();
		Projectile = NULL;
	}
}

void BulletClass::Init( const BulletDataClass & data, float progress_time, const Vector3 & target, DamageableGameObj * target_object )
{
	WWPROFILE( "Bullet Init" );

	BulletData.AmmoDefinition = data.AmmoDefinition;
	BulletData.Owner = data.Get_Owner();
	BulletData.SoftPierceCount = data.SoftPierceCount;
	BulletData.Destroy = data.Destroy;
	BulletData.LastHitID = 0;			// We haven't hit anyone yet.
	BulletData.DidExplode = false;
	BulletData.GrenadeSafetyTimer = data.GrenadeSafetyTimer;

	if ( data.AmmoDefinition ) {
		BulletData.GrenadeSafetyTimer = data.AmmoDefinition->GrenadeSafetyTime;
	}

	TargetVector = target;
	TargetObject = target_object;
	TrackingErrorTimer = 0;
	TrackingError = Vector3( 0,0,0 );

	Projectile->Set_Observer( this );
	Projectile->Set_Bounce_Count( BulletData.AmmoDefinition->MaxBounces );
	if ( BulletData.AmmoDefinition ) {
		Projectile->Set_Gravity_Multiplier( BulletData.AmmoDefinition->Gravity );
		Projectile->Set_Elasticity( BulletData.AmmoDefinition->Elasticity );
	}
	Projectile->Enable_Is_Pre_Lit(true);	// bullets don't get lighting.

	if ( Projectile->Peek_Model() != NULL &&
		  Projectile->Peek_Model()->Get_Name() != NULL &&
		  BulletData.AmmoDefinition->ModelName.Compare_No_Case( Projectile->Peek_Model()->Get_Name() ) == 0 ) {
		// We don't need to do anything
	} else {
		RenderObjClass * model = NULL;
		model = WW3DAssetManager::Get_Instance ()->Create_Render_Obj( BulletData.AmmoDefinition->ModelName );

		// If no name is given, lets create the NULL render obj
		if ( model == NULL ) {
			Debug_Say(( "Bullet Not Found \"%s\" \n", BulletData.AmmoDefinition->ModelName ));
			model = WW3DAssetManager::Get_Instance ()->Create_Render_Obj( "NULL" );

		}

		Projectile->Set_Model( model );

		if (model) {
			if ( BulletData.AmmoDefinition->ModelName.Compare_No_Case( model->Get_Name() ) != 0 ) {
				Debug_Say(( "Possible bullet twiddler!!  %s %s\n", BulletData.AmmoDefinition->ModelName, model->Get_Name() ));
			}
//			ModelNameCRC = CRC_Stringi( model->Get_Name() );
			ModelNameCRC = CRC_Stringi( BulletData.AmmoDefinition->ModelName );
			model->Release_Ref();
		}
		else {
			ModelNameCRC = 0xFFFFFFFF;
		}

	}

	if (Projectile->Get_Gravity_Multiplier() < 0.1f) {
		Projectile->Set_Orientation_Mode_Aligned_Fixed();
	} else {
		Projectile->Set_Orientation_Mode_Aligned();
	}

	COMBAT_SCENE->Add_Dynamic_Object( Projectile );

	float duration = (float) BulletData.AmmoDefinition->Range / (float) BulletData.AmmoDefinition->Velocity;
	if (duration <= 0.0) {
		Debug_Say(( "NULL DURATION\n" ));
	}
	Projectile->Set_Lifetime( duration * 1.2f );
	Projectile->Set_Position( data.Position );
	WWASSERT(data.Velocity.Is_Valid());
	Projectile->Set_Velocity( data.Velocity );

	// If there are any emitters in this model, reset them for our new position
	RenderObjClass * model = Projectile->Peek_Model();
	model->Restart();

	// Timestep last, incase it gets shutdown during this call
	if ( data.Get_Owner() != NULL ) {
		data.Get_Owner()->Peek_Physical_Object()->Inc_Ignore_Counter();
	}
	Projectile->Timestep( progress_time );
	if ( data.Get_Owner() != NULL ) {
		data.Get_Owner()->Peek_Physical_Object()->Dec_Ignore_Counter();
	}

}

void	BulletClass::Shutdown( void )
{
	if ( Projectile != NULL ) {
		COMBAT_SCENE->Remove_Object( Projectile );
		Projectile->Set_Observer( NULL );
	}

	BulletData.AmmoDefinition = NULL;
	BulletData.Owner = NULL;
}

/*
** BulletClass Save and Load
*/
enum	{
	CHUNKID_VARIABLES						=	910991544,
	CHUNKID_OWNER,
	CHUNKID_TARGET_OBJECT,

	MICROCHUNKID_AMMO_DEFINITION_ID	=	1,
	MICROCHUNKID_PROJECTILE,
	MICROCHUNKID_SOFT_PIERCE_COUNT,
	MICROCHUNKID_DESTROY,
	MICROCHUNKID_TARGET_VECTOR,
	MICROCHUNKID_TRACKING_ERROR_TIMER,
	MICROCHUNKID_TRACKING_ERROR,
	MICROCHUNKID_MODEL_NAME_CRC,
};

bool	BulletClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WWASSERT( BulletData.AmmoDefinition != NULL );
		int def_id = BulletData.AmmoDefinition->Get_ID();
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_AMMO_DEFINITION_ID, def_id );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_PROJECTILE, Projectile );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_SOFT_PIERCE_COUNT, BulletData.SoftPierceCount );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DESTROY, BulletData.Destroy );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TARGET_VECTOR, TargetVector );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TRACKING_ERROR_TIMER, TrackingErrorTimer );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TRACKING_ERROR, TrackingError );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_MODEL_NAME_CRC, ModelNameCRC );
	csave.End_Chunk();

	if ( BulletData.Get_Owner() != NULL ) {
		csave.Begin_Chunk( CHUNKID_OWNER );
		BulletData.Owner.Save( csave );
		csave.End_Chunk();
	}

	if ( TargetObject.Get_Ptr() != NULL ) {
		csave.Begin_Chunk( CHUNKID_TARGET_OBJECT );
		TargetObject.Save( csave );
		csave.End_Chunk();
	}

	return true;
}

bool	BulletClass::Load( ChunkLoadClass & cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_VARIABLES:
			{
				int def_id = 0;
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_AMMO_DEFINITION_ID, def_id );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_PROJECTILE, Projectile );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_SOFT_PIERCE_COUNT, BulletData.SoftPierceCount );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DESTROY, BulletData.Destroy );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TARGET_VECTOR, TargetVector );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TRACKING_ERROR_TIMER, TrackingErrorTimer );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TRACKING_ERROR, TrackingError );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_MODEL_NAME_CRC, ModelNameCRC );

						default:
							Debug_Say(("Unhandled Micro Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
							break;
					}
					cload.Close_Micro_Chunk();
				}

				WWASSERT( BulletData.AmmoDefinition == NULL );
				BulletData.AmmoDefinition = WeaponManager::Find_Ammo_Definition( def_id );
				WWASSERT( BulletData.AmmoDefinition != NULL );

				WWASSERT( Projectile != NULL );
				if ( Projectile != NULL ) {
					REQUEST_REF_COUNTED_POINTER_REMAP( (RefCountClass **)&Projectile );
				}

				break;
			}

			case CHUNKID_OWNER:
				BulletData.Owner.Load( cload );
				break;

			case CHUNKID_TARGET_OBJECT:
				TargetObject.Load( cload );
				break;

			default:
				Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}
		cload.Close_Chunk();
	}

	SaveLoadSystemClass::Register_Post_Load_Callback(this);

	return true;
}

void BulletClass::On_Post_Load (void)
{
	// Plug ourselves back into the physics object as an observer
	WWASSERT(Projectile != NULL);
	Projectile->Set_Observer(this);
	return ;
}


CollisionReactionType BulletClass::Collision_Occurred( const CollisionEventClass & event )
{
	// Copy the data from the Projectile
	Projectile->Get_Velocity( &BulletData.Velocity );
	Projectile->Get_Position( &BulletData.Position );
	CollisionReactionType result = BulletData.Bullet_Collision_Occurred( event );
	if ( BulletData.Destroy ) {
		Shutdown();
	}
	return result;
}

ExpirationReactionType	BulletClass::Object_Expired(PhysClass * observed_obj)
{
	// Copy the data from the Projectile
	Projectile->Get_Velocity( &BulletData.Velocity );
	Projectile->Get_Position( &BulletData.Position );
	ExpirationReactionType result = BulletData.Bullet_Expired();
	if ( BulletData.Destroy ) {
		Shutdown();
	}
	return result;
}


#define	RANDOM_VECTOR( spread )		Vector3( FreeRandom.Get_Float( -(spread), (spread) ),	\
															FreeRandom.Get_Float( -(spread), (spread) ),	\
															FreeRandom.Get_Float( 0, (spread) ) )

void	BulletClass::Think( void )
{
	WWPROFILE( "Bullet Think" );

	// Count down safety timer
	BulletData.GrenadeSafetyTimer -= TimeManager::Get_Frame_Seconds();
	if ( BulletData.GrenadeSafetyTimer < 0 ) {
		BulletData.GrenadeSafetyTimer = 0;
	}

	// Handle Homing Bullets
	if ( BulletData.AmmoDefinition->IsTracking ) {

		// Track your target object
		DamageableGameObj * target_obj = (DamageableGameObj *)TargetObject.Get_Ptr();
		if ( target_obj != NULL ) {
			if ( target_obj->As_PhysicalGameObj() ) {
				TargetVector = target_obj->As_PhysicalGameObj()->Get_Bullseye_Position();
			} else {
				target_obj->Get_Position( &TargetVector );
			}
		}

		Vector3	target_vector = TargetVector;

		Vector3	obj_pos;
		Projectile->Get_Transform().Get_Translation( &obj_pos );
//			Debug_Say(( "Bullet Tracking %f %f %f\n", obj_pos.X, obj_pos.Y, obj_pos.Z ));
		target_vector -= obj_pos;

		// Find distance to target
		float target_distance = target_vector.Length();

		TrackingErrorTimer -= TimeManager::Get_Frame_Seconds();
		if ( TrackingErrorTimer < 0 ) {
			TrackingErrorTimer = FreeRandom.Get_Float( 0.1f, 0.4f );
			TrackingError = RANDOM_VECTOR( BulletData.AmmoDefinition->RandomTrackingScale * target_distance / 2 );
		}
		target_vector += TrackingError;
		target_vector.Normalize();

		Vector3	current_vector;
		Projectile->Get_Velocity( &current_vector );
		current_vector.Normalize();

		Vector3 cross = Vector3::Cross_Product( current_vector, target_vector );

		// only re-aim if cross product not near zero
		if ( cross.Length() > WWMATH_EPSILON ) {

			float dot = Vector3::Dot_Product( target_vector, current_vector );

			if ( WWMath::Fabs(dot) >= 1.0f ) {
				// No turning needed
			} else {

				float angle = WWMath::Fast_Acos( dot );
				float	allowed_turn = BulletData.AmmoDefinition->TurnRate * TimeManager::Get_Frame_Seconds();

				if ( angle > allowed_turn ) {
					angle = allowed_turn;
				}

				cross.Normalize();
				Matrix3D tm(cross,angle);
				Vector3	current_vector;
				Projectile->Get_Velocity( &current_vector );
				WWASSERT(current_vector.Is_Valid());
				current_vector = tm.Rotate_Vector( current_vector );
				WWASSERT(current_vector.Is_Valid());
				Projectile->Set_Velocity( current_vector );
			}
		}
	}
}

/*
** Instant Bullet Code
*/
void	Simulate_Instant_Bullet( BulletDataClass & data, float progress_time )
{
	WWPROFILE("Simulate_Instant_Bullet");
//	WWASSERT(data.Position.Is_Valid());
//	WWASSERT(data.Velocity.Is_Valid());

	Vector3	start = data.Position;
	Vector3	end = data.Velocity;
	end.Normalize();
	end = start + (end * data.AmmoDefinition->Range);

	// This is that last object we choose to ignore
	PhysClass	*blocker = NULL;

	// Always ignore the owner
	if ( data.Get_Owner() != NULL ) {
		if ( data.Get_Owner()->Peek_Physical_Object() != NULL ) {
			data.Get_Owner()->Peek_Physical_Object()->Inc_Ignore_Counter();
		}
	}

	bool	done = false;
	int hit_counter = 0;
	const int MAX_HITS = 5;

	while ( !done && (hit_counter < MAX_HITS)) {

		hit_counter++;

		// Check for collisions in the path of the object
		CastResultStruct res;
		LineSegClass ray( data.Position, end );
		PhysRayCollisionTestClass raytest(ray,&res,BULLET_COLLISION_GROUP,COLLISION_TYPE_PROJECTILE);
{WWPROFILE("Cast_Ray");
		PhysicsSceneClass::Get_Instance()->Cast_Ray(raytest);
}

		// If the result was a "startbad", we are done
		if (raytest.Result->StartBad) {
//			Debug_Say(( "Simulate_Instant_Bullet Startbad\n" ));
			done = true;
		} else {

			if (raytest.Result->Fraction < 1.0f) {
				// If there was a collision, set us at the point of collision
				// Also, fill in the cast result struct.
				ray.Compute_Point(raytest.Result->Fraction,&(raytest.Result->ContactPoint));
				data.Position = raytest.Result->ContactPoint;

				// Notify the parties involved
				WWASSERT(raytest.CollidedPhysObj != NULL);

				CollisionReactionType reaction = COLLISION_REACTION_DEFAULT;

				CollisionEventClass event;
				event.CollisionResult = raytest.Result;
				event.CollidedRenderObj = raytest.CollidedRenderObj;

				event.OtherObj = raytest.CollidedPhysObj;
				reaction |= data.Bullet_Collision_Occurred(event);

				if ( reaction & COLLISION_REACTION_NO_BOUNCE ) {
					// We were requested to fly through.  Mark the current blocker as ignore
					// so we collide with him no more this pass
					if ( blocker ) {	 // Stop ignoring the last blocker
						blocker->Dec_Ignore_Counter();
					}
					blocker = raytest.CollidedPhysObj;
					if ( blocker ) {	// Start ignoring this blocker
						blocker->Inc_Ignore_Counter();
					}
				} else {
					done = true;
				}
			} else {
				done = true;
				data.Position = end;
			}
		}
	}

	if ( blocker ) {	// Stop ignoring the last blocker
		blocker->Dec_Ignore_Counter();
	}

	// stop ignoring the owner
	if ( data.Get_Owner() != NULL ) {
		if ( data.Get_Owner()->Peek_Physical_Object() != NULL ) {
			data.Get_Owner()->Peek_Physical_Object()->Dec_Ignore_Counter();
		}
	}

	// If not destroyed, Expire..
	if ( !data.Destroy ) {
		data.Bullet_Expired();
	}

	// If the definition requests a beam effect, launch it
	if (data.AmmoDefinition->BeamEnabled) {
		_TheBeamEffectManager.Create_Beam_Effect(data.AmmoDefinition,start,data.Position);
	}
}

/*
** BulletManager
*/
MultiListClass<BulletClass>	LiveBulletList;
MultiListClass<BulletClass>	DeadBulletList;

void	BulletManager::Init( void )
{
	_TheBeamEffectManager.Init();
}

void	BulletManager::Shutdown( void )
{
	while ( !LiveBulletList.Is_Empty() ) {
		delete LiveBulletList.Remove_Head();
	}
	while ( !DeadBulletList.Is_Empty() ) {
		delete DeadBulletList.Remove_Head();
	}

	_TheBeamEffectManager.Shutdown();
}

void	BulletManager::Update( void )
{
	MultiListIterator<BulletClass> it( &LiveBulletList );;
	while (!it.Is_Done()) {
		BulletClass * bullet = it.Peek_Obj();
		if (bullet->BulletData.Destroy) {
			bullet->Shutdown();
			it.Remove_Current_Object();
			DeadBulletList.Add( bullet );
		} else {
			bullet->Think();
			it.Next();
		}
	}
}

#define	BULLET_SPEED_CHEAT	0

void	BulletManager::Create_Bullet( const AmmoDefinitionClass * def, const Vector3 & position,
	const Vector3 & velocity, const ArmedGameObj * owner, float progress_time, const Vector3 & target,
	DamageableGameObj * target_obj )
{
	WWASSERT(velocity.Is_Valid());

	BulletDataClass data( def, owner, position, velocity );

#if BULLET_SPEED_CHEAT 
	if ( (float)def->Velocity >= _INSTANT_BULLET_THRESHHOLD/3 ) {
#else
	if ( (float)def->Velocity >= _INSTANT_BULLET_THRESHHOLD ) {
#endif
		Simulate_Instant_Bullet( data, progress_time );
		return;
	}

	WWPROFILE( "Create Bullet" );
	BulletClass * bullet = NULL;

	// Find a bullet
	long crc = CRC_Stringi( def->ModelName );
	MultiListIterator<BulletClass> it( &DeadBulletList );;
	while ( !it.Is_Done() && bullet == NULL ) {
		BulletClass * test_bullet = it.Peek_Obj();
		if ( test_bullet->ModelNameCRC == crc ) {
			bullet = test_bullet;
			it.Remove_Current_Object();
		} else {
			it.Next();
		}
	}

	if ( bullet == NULL ) {
		bullet = new BulletClass();
	}

	if ( bullet != NULL ) {
		bullet->Init( data, progress_time, target, target_obj );
		LiveBulletList.Add( bullet );
	}
}

/*
** Save & Load
*/
enum	{
	CHUNKID_BULLET								=	916991653,
};

bool	BulletManager::Save( ChunkSaveClass &csave )
{
	MultiListIterator<BulletClass> it( &LiveBulletList );;
	while (!it.Is_Done()) {
		BulletClass * bullet = it.Peek_Obj();

		// Only save valid bullets
		if ( bullet->Is_Valid() ) {
			csave.Begin_Chunk( CHUNKID_BULLET );
			bullet->Save( csave );
			csave.End_Chunk();
		}
		it.Next();
	}
	return true;
}

bool	BulletManager::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_BULLET:
			{
				BulletClass * bullet = new BulletClass();
				bullet->Load( cload );
				LiveBulletList.Add( bullet );
				break;
			}

			default:
				Debug_Say(( "Unrecognized BulletManager chunkID %d\n", cload.Cur_Chunk_ID() ));
				break;
		}
		cload.Close_Chunk();
	}
	return true;
}

