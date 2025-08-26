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
 *                     $Archive:: /Commando/Code/Combat/surfaceeffects.cpp                    $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 6/14/02 10:48a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 56                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "surfaceeffects.h"
#include "w3d_file.h"
#include "part_emt.h"
#include "assets.h"
#include "pscene.h"
#include "debug.h"
#include "wwstring.h"
#include "crandom.h"
#include "physcon.h"
#include "WWAudio.h"
#include "rndstrng.h"
#include "Sound3D.h"
#include "phys.h"
#include "wwprofile.h"
#include "gameobjref.h"
#include "physicalgameobj.h"
#include "combat.h"
#include "ccamera.h"
#include "effectrecycler.h"
#include "timeddecophys.h"

#define RECYCLE_EMITTERS	1

const float _MAX_SURFACE_EFFECT_DISTANCE2	= 50.0f * 50.0f;	// no surface effects created farther than this from camera
const float _SURFACE_EFFECT_NEAR_DISTANCE2 = 10.0f * 10.0f;	// force surface effects within this distance

#define	SECTION_BULLET_STOPPING		"Bullet_Stopping"
#define	SECTION_DAMAGE_WARHEAD		"Damage_Warhead"
#define	SECTION_DAMAGE_RATE			"Damage_Rate"

SurfaceEffectsManager::MODE	SurfaceEffectsManager::Mode	= SurfaceEffectsManager::MODE_FULL;
int									SurfaceEffectsManager::OverrideSurfaceType = -1;
bool									_IsSurfaceEffectsInitted = false;

// If there exist more than this many particle buffers, don't make emitters
#define	PARTICLE_BUFFER_LIMIT			5

/*
** These are the names for each Hitter Type.  If you add a hitter type, add the name here
*/
static const char * _HitterTypeName[ SurfaceEffectsManager::NUM_HITTER_TYPES ] = {
		"None",					// HITTER_TYPE_NONE,							// used to shut off persistant effects
		"Generic Object",		// HITTER_TYPE_GENERIC_OBJECT,			// generic dynamic object

		"Footstep Run",		// HITTER_TYPE_FOOTSTEP_RUN,
		"Footstep Walk",		// HITTER_TYPE_FOOTSTEP_WALK,
		"Footstep Crouched",	// HITTER_TYPE_FOOTSTEP_CROUCHED,
		"Footstep Jump",		// HITTER_TYPE_FOOTSTEP_JUMP,
		"Footstep Land",		// HITTER_TYPE_FOOTSTEP_LAND,

		"Bullet",				// HITTER_TYPE_BULLET,
		"Bullet Fire",			// HITTER_TYPE_BULLET_FIRE,
		"Bullet Grenade",		// HITTER_TYPE_BULLET_GRENADE,
		"Bullet Chem",			// HITTER_TYPE_BULLET_CHEM,
		"Bullet Electric",	// HITTER_TYPE_BULLET_ELECTRIC,
		"Bullet Laser",		// HITTER_TYPE_BULLET_LASER,
		"Bullet Squish",		// HITTER_TYPE_BULLET_SQUISH,
		"Bullet Tib Spray",	// HITTER_TYPE_BULLET_TIB_SPRAY,
		"Bullet Tib",			// HITTER_TYPE_BULLET_TIB_BULLET,
		"Bullet Shotgun",		// HITTER_TYPE_BULLET_SHOTGUN,
		"Bullet Silenced",	// HITTER_TYPE_BULLET_SILENCED,
		"Bullet Sniper",		// HITTER_TYPE_BULLET_SNIPER,
		"Bullet Water",		// HITTER_TYPE_BULLET_WATER,

		"Eject Casing",		// HITTER_TYPE_EJECT_CASING,				// casings ejected from weapons (all are assumed to make the same sounds)
		"Shell Shotgun",		// HITTER_TYPE_SHELL_SHOTGUN,

		"Tire Rolling",		// HITTER_TYPE_TIRE_ROLLING,				// wheels on humvees, etc...
		"Tire Sliding",		// HITTER_TYPE_TIRE_SLIDING,
		"Track Rolling",		// HITTER_TYPE_TRACK_ROLLING,				// tank tracks...
		"Track Sliding",		// HITTER_TYPE_TRACK_SLIDING,

		"Footstep Ladder",	// HITTER_TYPE_FOOTSTEP_LADDER,

};


/*
** Recycler for all "fire-and-forget" surface effect emitters
*/
#if (RECYCLE_EMITTERS)
static EffectRecyclerClass _EmitterRecycler;
#endif


/*
** Persistant Surface Effects
*/
class PersistantSurfaceEffectClass
{
public:
	PersistantSurfaceEffectClass( void );
	~PersistantSurfaceEffectClass( void );

	int							CurrentSurfaceType;
	int							CurrentHitterType;

	friend SurfaceEffectsManager;
};

class	PersistantSurfaceSoundClass : public PersistantSurfaceEffectClass
{
public:
	PersistantSurfaceSoundClass( void );
	~PersistantSurfaceSoundClass( void );

	void							Set_Sound( const char * name );

	AudibleSoundClass *		Sound;

	friend	SurfaceEffectsManager;
};

class	PersistantSurfaceEmitterClass : public PersistantSurfaceEffectClass
{
private:
	PersistantSurfaceEmitterClass( void );
	~PersistantSurfaceEmitterClass( void );

	void							Set_Emitter( const char * name );

	ParticleEmitterClass *	Emitter;

	friend	SurfaceEffectsManager;
};

/*
** PersistantSurfaceEffectClass
*/
PersistantSurfaceEffectClass::PersistantSurfaceEffectClass(void) :
	CurrentSurfaceType( -1 ),
	CurrentHitterType( -1 )
{
}

PersistantSurfaceEffectClass::~PersistantSurfaceEffectClass(void)
{
}

/*
**	PersistantSurfaceSoundClass
*/
PersistantSurfaceSoundClass::PersistantSurfaceSoundClass( void ) :
	Sound( NULL )
{
}

PersistantSurfaceSoundClass::~PersistantSurfaceSoundClass( void )
{
	Set_Sound( NULL );
}

void	PersistantSurfaceSoundClass::Set_Sound( const char * name )
{
	// Stop Old Sound
	if ( Sound != NULL ) {
		Sound->Stop();
		Sound->Remove_From_Scene();
		Sound->Release_Ref();
		Sound = NULL;
	}

	// Start new Sound
	if ( name != NULL ) {
		Sound = WWAudioClass::Get_Instance()->Create_Continuous_Sound( name );
		if ( Sound != NULL ) {
			Sound->Add_To_Scene( true );
		}
	}
}


/*
**	PersistantSurfaceEmitterClass
*/
PersistantSurfaceEmitterClass::PersistantSurfaceEmitterClass( void ) :
	Emitter( NULL )
{
}

PersistantSurfaceEmitterClass::~PersistantSurfaceEmitterClass( void )
{
	Set_Emitter( NULL );
}

void	PersistantSurfaceEmitterClass::Set_Emitter( const char * name )
{
	// Stop Old Emitter
	if ( Emitter != NULL ) {
		// Check if it is in the scene, it may ahve already been remove by completion
		if ( Emitter->Peek_Scene() != NULL ) {
			PhysicsSceneClass::Get_Instance()->Remove_Render_Object( Emitter );
		}
		Emitter->Stop();
		Emitter->Release_Ref();
		Emitter = NULL;
	}

	// Start new Emitter
	if ( name != NULL ) {
		Emitter = (ParticleEmitterClass *)WW3DAssetManager::Get_Instance()->Create_Render_Obj( name );
		if ( Emitter ) {
			SET_REF_OWNER( Emitter );
//			Emitter->Set_Remove_On_Complete( false );
			Emitter->Start();
			PhysicsSceneClass::Get_Instance()->Add_Render_Object( Emitter );
		}
	}
}

/*
** Surface Effect Database
*/
class SurfaceEffectClass {

public:
	RandomStringClass	Sound;
	RandomStringClass	Emitter;
	RandomStringClass	Decal;
	float					DecalSize;
	float					DecalSizeRandom;
};

SurfaceEffectClass * SurfaceEffectsDatabase[SURFACE_TYPE_MAX][SurfaceEffectsManager::NUM_HITTER_TYPES];
bool						SurfaceStopsBullets[SURFACE_TYPE_MAX];
int						SurfaceDamageWarhead[SURFACE_TYPE_MAX];
float						SurfaceDamageRate[SURFACE_TYPE_MAX];

/*
** These are the physics object types for each Hitter Type.  These are only used to pass
** the friction and drag constants on to the physics system.  If your hitter type does not
** correspond to a physics object type, put a -1 for its entry here.
*/
const int HitterPhysicsTypes[SurfaceEffectsManager::NUM_HITTER_TYPES] =
{
	-1,														//"Bullet",
	-1,														//"Eject Casing"
	-1,														//"Footstep Run",
	-1,														//"Footstep Walk",
	-1,														//"Footstep Crouch",
	PhysicsConstants::DYNAMIC_OBJ_TYPE_TIRE,		//"Tire Rolling",
	-1,														//"Tire Sliding",
	PhysicsConstants::DYNAMIC_OBJ_TYPE_TRACK,		//"Track Rolling",
	-1,														//"Track Sliding",
	PhysicsConstants::DYNAMIC_OBJ_TYPE_GENERIC,	//"Generic Object"
	-1,
};


#define	SURFACE_EFFECTS_INI_FILENAME		"surfaceeffects.ini"

bool SurfaceEffectsManagerDebug = false;

void SurfaceEffectsManager::Toggle_Debug( void )
{
	SurfaceEffectsManagerDebug = !SurfaceEffectsManagerDebug;
}

/*
**
*/
void	SurfaceEffectsManager::Init( void )
{
	if (_IsSurfaceEffectsInitted) {
		Shutdown();
	}
	
	for ( int i = 0; i < SURFACE_TYPE_MAX; i++ ) {
		SurfaceStopsBullets[i] = true;
		SurfaceDamageWarhead[i] = 0;
		SurfaceDamageRate[i] = 0;
	}

	// Read Database INI file
	INIClass	* ini = Get_INI( SURFACE_EFFECTS_INI_FILENAME );

	if ( ini != NULL ) {
		WWASSERT( ini && ini->Section_Count() > 0 );

		int surface;

		// Read Bullet Stopping
		for ( surface = 0; surface < SURFACE_TYPE_MAX; surface++ ) {
			SurfaceStopsBullets[ surface ] = ini->Get_Bool(	SECTION_BULLET_STOPPING, SURFACE_TYPE_STRINGS[surface], true );
		}

		// Read Surface Damage Data
		for ( surface = 0; surface < SURFACE_TYPE_MAX; surface++ ) {
			StringClass	warhead(0,true);
			ini->Get_String(warhead,SECTION_DAMAGE_WARHEAD, SURFACE_TYPE_STRINGS[surface] );
			if ( !warhead.Is_Empty() ) {
				SurfaceDamageWarhead[surface] = ArmorWarheadManager::Get_Warhead_Type( warhead );
			}
			SurfaceDamageRate[surface] = ini->Get_Float( SECTION_DAMAGE_RATE, SURFACE_TYPE_STRINGS[surface], 0 );

			if ( SurfaceDamageRate[surface] != 0 ) {
//				Debug_Say(( "Surface %s Rate %f Warhead %s\n", SURFACE_TYPE_STRINGS[surface],
//					SurfaceDamageRate[surface], ArmorWarheadManager::Get_Warhead_Name( SurfaceDamageWarhead[surface] ) ));
			}
		}

		for ( surface = 0; surface < SURFACE_TYPE_MAX; surface++ ) {
			for ( int hitter = 0; hitter < NUM_HITTER_TYPES; hitter++ ) {
				StringClass	section_name;
				section_name.Format( "%s_%s", SURFACE_TYPE_STRINGS[surface], _HitterTypeName[hitter] );

				if ( ini->Entry_Count( section_name ) == 0 ) {
					SurfaceEffectsDatabase[ surface ][ hitter ] = NULL;
					continue;
				}

	//			Debug_Say(( "Surface Effect Section %s\n", section_name ));

				// create the effect
				SurfaceEffectClass * surface_effect = new SurfaceEffectClass;

				// Read sounds
				for ( int sound = 0; ; sound++ ) {
					StringClass	sound_entry(0,true);
					sound_entry.Format( "Sound%d", sound );
					StringClass	sound_name(0,true);
					ini->Get_String(sound_name, section_name, sound_entry );
					if ( sound_name.Is_Empty() ) {
						break;
					}
	//				Debug_Say(( "Add Sound %s\n", sound_name ));
					surface_effect->Sound.Add_String( sound_name );
				}

				// Read emitters
				for ( int emitter = 0; ; emitter++ ) {
					StringClass	emitter_entry(0,true);
					emitter_entry.Format( "Emitter%d", emitter );
					StringClass	emitter_name(0,true);
					ini->Get_String(emitter_name, section_name, emitter_entry );
					if ( emitter_name.Is_Empty() ) {
						break;
					}
	//				Debug_Say(( "Add Emitter %s\n", emitter_name ));
					surface_effect->Emitter.Add_String( emitter_name );
				}

				// Read decals
				for ( int decal = 0; ; decal++ ) {
					StringClass	decal_entry(0,true);
					decal_entry.Format( "Decal%d", decal );
					StringClass	decal_name(0,true);
					ini->Get_String(decal_name, section_name, decal_entry );
					if ( decal_name.Is_Empty() ) {
						break;
					}
	//				Debug_Say(( "Add Decal %s\n", decal_name ));
					surface_effect->Decal.Add_String( decal_name );
				}
				surface_effect->DecalSize = ini->Get_Float( section_name, "DecalSize", 1 );
				surface_effect->DecalSizeRandom = ini->Get_Float( section_name, "DecalSizeRandom", 0 );


				// save the effect
				SurfaceEffectsDatabase[ surface ][ hitter ] = surface_effect;

				// (gth) if this is one of the physics "hitter types" copy the
				// constants into the physics engine
				if (HitterPhysicsTypes[hitter] != -1) {

					float friction = ini->Get_Float( section_name,
																"Friction",
																PhysicsConstants::DefaultContactFriction );

					float drag = ini->Get_Float(		section_name,
																"Drag",
																PhysicsConstants::DefaultContactDrag );


					PhysicsConstants::Set_Contact_Friction_Coefficient(	HitterPhysicsTypes[hitter],
																							surface,
																							friction  );

					PhysicsConstants::Set_Contact_Drag_Coefficient(			HitterPhysicsTypes[hitter],
																							surface,
																							drag  );
				}
			}
		}

		Release_INI( ini );
		ini = NULL;
	} else {
		Debug_Say(("SurfaceEffectsManager::Init - Unable to load %s\n", SURFACE_EFFECTS_INI_FILENAME ));
	}

	WWASSERT(ini == NULL);
   _IsSurfaceEffectsInitted = true;
}

void	SurfaceEffectsManager::Shutdown( void )
{
#if (RECYCLE_EMITTERS)
	_EmitterRecycler.Reset();
#endif

	for ( int surface = 0; surface < SURFACE_TYPE_MAX; surface++ ) {
		for ( int hitter = 0; hitter < NUM_HITTER_TYPES; hitter++ ) {
			if ( SurfaceEffectsDatabase[ surface ][ hitter ] ) {
				delete SurfaceEffectsDatabase[ surface ][ hitter ];
				SurfaceEffectsDatabase[ surface ][ hitter ] = NULL;
			}
		}
	}
	_IsSurfaceEffectsInitted = false;
}

const char * SurfaceEffectsManager::Hitter_Type_Name( int hitter )
{
	return _HitterTypeName[ hitter ];
}

void	SurfaceEffectsManager::Apply_Effect
(
	int surface_type,
	int hitter_type,
	const Matrix3D & tm,
	PhysClass * hit_obj,
	PhysicalGameObj * creator,
	bool allow_decals,
	bool allow_emitters
)
{
	WWPROFILE( "Apply Surface Effect" );

	bool ok = (	(surface_type >= 0) && 
					(surface_type < SURFACE_TYPE_MAX) &&
					(hitter_type >=0 ) &&
					(hitter_type < NUM_HITTER_TYPES) );
	if (!ok) {
		WWDEBUG_SAY(("Invalid surface params. surface_type: %d hitter_type: %d\r\n",surface_type,hitter_type));
		return;
	}

	// Return if all surface effects are currently disabled
	if ( Mode == MODE_OFF ) {
		return;
	}
	
	// If the user has specified an override type, use it.
	if (OverrideSurfaceType != -1) { surface_type = OverrideSurfaceType; }

	// Return if we don't find any surface effect settings for this combination of surface_type, hitter_type
	SurfaceEffectClass * surface_effect = SurfaceEffectsDatabase[ surface_type ][ hitter_type ];
	if ( surface_effect == NULL ) {
		if ( SurfaceEffectsManagerDebug ) {
			Debug_Say(( "No Surface Effect for %s and %s\n", SURFACE_TYPE_STRINGS[surface_type], _HitterTypeName[hitter_type] ));
		}
		return;
	}

	const char * sound_name = surface_effect->Sound.Get_String();
	const char * emitter_name = surface_effect->Emitter.Get_String();
	const char * decal_name = surface_effect->Decal.Get_String();

	if ( SurfaceEffectsManagerDebug ) {
		Debug_Say(( "Applying Surface Effect for %s and %s\n--Sound:%s Emitter:%s Decal:%s\n",
			SURFACE_TYPE_STRINGS[surface_type],
			_HitterTypeName[hitter_type],
			sound_name ? sound_name : "none",
			emitter_name ? emitter_name : "none",
			decal_name ? decal_name : "none" ));
	}

	// Create the sound
	if ( sound_name ) {

		WWPROFILE( "Sound" );

		// I may need to plug an owner in here
		RefCountedGameObjReference *owner_ref = new RefCountedGameObjReference;
		owner_ref->Set_Ptr( creator );
		WWAudioClass::Get_Instance()->Create_Instant_Sound( sound_name, tm, owner_ref, 0, CLASSID_PSEUDO3D );
		REF_PTR_RELEASE( owner_ref );
	}

	// Return if this effect is far from the camera or not within the view frustum
	// But, do this after the sound is created!!!
	Vector3 effect_point;
	tm.Get_Translation(&effect_point);
	float dist2 = (effect_point - COMBAT_CAMERA->Get_Position()).Length2();

	if (	(dist2 > _SURFACE_EFFECT_NEAR_DISTANCE2) ) {
		if (	(dist2 > _MAX_SURFACE_EFFECT_DISTANCE2) ||
			(CollisionMath::Overlap_Test(COMBAT_CAMERA->Get_Frustum(),effect_point) == CollisionMath::OUTSIDE) )
		{
			return;
		}
	}

	// Create the emitter
	if ((allow_emitters) && (emitter_name) && (Mode != MODE_NO_EMITTERS)) {
		WWPROFILE( "Emitter" );

#if (RECYCLE_EMITTERS)
		_EmitterRecycler.Spawn_Effect(emitter_name,tm);
#else
		ParticleEmitterClass * emitter = (ParticleEmitterClass *)WW3DAssetManager::Get_Instance()->Create_Render_Obj( emitter_name );
		if ( emitter ) {
			SET_REF_OWNER( emitter );
			emitter->Set_Transform( tm );
			emitter->Start();
			emitter->Enable_Remove_On_Complete(true);
			PhysicsSceneClass::Get_Instance()->Add_Render_Object( emitter );
			emitter->Release_Ref();
		}
#endif
	}

	// Create decal, (will only apply to static physics objects)
	if ((allow_decals) && (decal_name)) {

		WWPROFILE( "Decal" );

		float size = surface_effect->DecalSize;
		size += FreeRandom.Get_Float( -surface_effect->DecalSizeRandom, surface_effect->DecalSizeRandom );
		StringClass	new_name( true );
		Strip_Path_From_Filename( new_name, decal_name );


		// If this is a glass decal, we enable "apply_to_translucent_meshes" and we
		// also pass in the object that was hit so the decal is *only* applied to that
		// object.  Otherwise, we use normal decal processing which collects the meshes
		// that overlap the volume and applies the decal to all of them
		if ((surface_type == SURFACE_TYPE_GLASS) || (surface_type == SURFACE_TYPE_GLASS_PERMEABLE)) {
			PhysicsSceneClass::Get_Instance()->Create_Decal( tm, new_name, size, false, true, hit_obj );
		} else {
			PhysicsSceneClass::Get_Instance()->Create_Decal( tm, new_name, size );
		}
	}
}


PersistantSurfaceSoundClass *	SurfaceEffectsManager::Create_Persistant_Sound( void )
{
	return new PersistantSurfaceSoundClass;
}

void	SurfaceEffectsManager::Destroy_Persistant_Sound( PersistantSurfaceSoundClass * effect )
{
	delete effect;
}

void	SurfaceEffectsManager::Update_Persistant_Sound( PersistantSurfaceSoundClass * effect,
																int surface_type, int hitter_type, const Matrix3D & tm )
{
	WWASSERT( surface_type >= 0 );
	WWASSERT( surface_type < SURFACE_TYPE_MAX );
	WWASSERT( hitter_type >= 0 );
	WWASSERT( hitter_type < NUM_HITTER_TYPES );

	if ( Mode == MODE_OFF ) {
		return;
	}

	// If the user has specified an override type, use it.
	if (OverrideSurfaceType != -1) { surface_type = OverrideSurfaceType; }

	// If something changed, re-lookup the sound
	if ( ( surface_type != effect->CurrentSurfaceType ) || ( hitter_type != effect->CurrentHitterType ) ) {

		effect->CurrentSurfaceType = surface_type;
		effect->CurrentHitterType = hitter_type;

		SurfaceEffectClass * surface_effect = SurfaceEffectsDatabase[ surface_type ][ hitter_type ];

		if ( surface_effect == NULL ) {
			if ( SurfaceEffectsManagerDebug ) {
				Debug_Say(( "No Surface Effect for %s and %s\n", SURFACE_TYPE_STRINGS[surface_type], _HitterTypeName[hitter_type] ));
			}
			effect->Set_Sound( NULL );

		} else {

			const char * sound_name = surface_effect->Sound.Get_String();
			if ( SurfaceEffectsManagerDebug ) {
				Debug_Say(( "Applying Persistant Surface Sound for %s and %s Sound:%s\n",
					SURFACE_TYPE_STRINGS[surface_type],
					_HitterTypeName[hitter_type],
					sound_name ? sound_name : "none" ));
			}
			effect->Set_Sound( sound_name );
		}
	}

	// Update Position
	if ( effect->Sound != NULL ) {
		effect->Sound->Set_Transform( tm );
	}
}


PersistantSurfaceEmitterClass *	SurfaceEffectsManager::Create_Persistant_Emitter( void )
{
	return new PersistantSurfaceEmitterClass;
}

void	SurfaceEffectsManager::Destroy_Persistant_Emitter( PersistantSurfaceEmitterClass * effect )
{
	delete effect;
}

void	SurfaceEffectsManager::Update_Persistant_Emitter( PersistantSurfaceEmitterClass * effect,
																int surface_type, int hitter_type, const Matrix3D & tm )
{
	WWASSERT( surface_type >= 0 );
	WWASSERT( surface_type < SURFACE_TYPE_MAX );
	WWASSERT( hitter_type >= 0 );
	WWASSERT( hitter_type < NUM_HITTER_TYPES );

	if ( Mode != MODE_FULL ) {
		return;
	}

	// If the user has specified an override type, use it.
	if (OverrideSurfaceType != -1) { surface_type = OverrideSurfaceType; }

	// If something changed, re-lookup the emitter
	if ( ( surface_type != effect->CurrentSurfaceType ) || ( hitter_type != effect->CurrentHitterType ) ) {

		effect->CurrentSurfaceType = surface_type;
		effect->CurrentHitterType = hitter_type;
		SurfaceEffectClass * surface_effect = SurfaceEffectsDatabase[ surface_type ][ hitter_type ];

		if ( surface_effect == NULL ) {
			if ( SurfaceEffectsManagerDebug ) {
				Debug_Say(( "No Surface Effect for %s and %s\n", SURFACE_TYPE_STRINGS[surface_type], _HitterTypeName[hitter_type] ));
			}
			effect->Set_Emitter( NULL );

		} else {

			const char * emitter_name = surface_effect->Emitter.Get_String();
			if ( SurfaceEffectsManagerDebug ) {
				Debug_Say(( "Applying Persistant Surface Emitter for %s and %s Emitter:%s\n",
					SURFACE_TYPE_STRINGS[surface_type],
					_HitterTypeName[hitter_type],
					emitter_name ? emitter_name : "none" ));
			}
			effect->Set_Emitter( emitter_name );
		}
	}

	// Update the transform
	if ( effect->Emitter != NULL ) {
		effect->Emitter->Set_Transform( tm );
	}
}

bool	SurfaceEffectsManager::Does_Surface_Stop_Bullets( int surface_type )
{
	WWASSERT( surface_type >= 0 );
	WWASSERT( surface_type < SURFACE_TYPE_MAX );
	return SurfaceStopsBullets[ surface_type ];
}

void	SurfaceEffectsManager::Apply_Damage( int surface_type, PhysicalGameObj * obj )
{

#ifndef PARAM_EDITING_ON

	// If the user has specified an override type, use it.
	if (OverrideSurfaceType != -1) { surface_type = OverrideSurfaceType; }

	if ( SurfaceDamageRate[surface_type] > 0 ) {
		OffenseObjectClass offense_obj( SurfaceDamageRate[surface_type], SurfaceDamageWarhead[surface_type] );
		obj->Apply_Damage_Extended( offense_obj, TimeManager::Get_Frame_Seconds() );
	}

#endif //PARAM_EDITING_ON

}

bool	SurfaceEffectsManager::Is_Surface_Permeable( int surface )
{
	switch (surface) {
		case SURFACE_TYPE_FOLIAGE_PERMEABLE:
		case SURFACE_TYPE_GLASS_PERMEABLE:
		case SURFACE_TYPE_ICE_PERMEABLE:
		case SURFACE_TYPE_CLOTH_PERMEABLE:
		case SURFACE_TYPE_ELECTRICAL_PERMEABLE:
		case SURFACE_TYPE_FLAMMABLE_PERMEABLE:
		case SURFACE_TYPE_STEAM_PERMEABLE:
		case SURFACE_TYPE_WATER_PERMEABLE:
		case SURFACE_TYPE_TIBERIUM_WATER_PERMEABLE:
			return true;
	}
	return false;
}

void SurfaceEffectsManager::Set_Override_Surface_Type(int type)
{
	if ((type < 0) || (type >= SURFACE_TYPE_MAX)) {
		OverrideSurfaceType = -1;
	} else {
		OverrideSurfaceType = type;
	}

	PhysicsConstants::Set_Override_Surface_Type(OverrideSurfaceType);
}


