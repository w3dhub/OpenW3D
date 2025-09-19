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
 *                     $Archive:: /Commando/Code/Combat/weaponview.cpp                        $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 1/16/02 11:46a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 56                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "weaponview.h"
#include "assets.h"
#include "hanim.h"
#include "combat.h"
#include "pscene.h"
#include "ccamera.h"
#include "texture.h"
#include "physicalgameobj.h"
#include "debug.h"
#include "animobj.h"
#include "soldier.h"
#include "wwprofile.h"
#include "weapons.h"
#include "decophys.h"
#include "stealtheffect.h"
#include "animcontrol.h"


/*
**
*/
enum {	
	WEAPON_STATE_IDLE,
	WEAPON_STATE_FIRE,
	WEAPON_STATE_RELOAD,
	WEAPON_STATE_ENTER,
	WEAPON_STATE_EXIT,
//	WEAPON_STATE_SPIN_DOWN,
	NUM_WEAPON_STATES,
};

int					WeaponViewEnabled;
int					WeaponState;
bool				LastMuzzleFlash;

DecorationPhysClass*HandsPhysObj;
RenderObjClass*		WeaponModel;
RenderObjClass*		ClipModel;

Vector3				HandsOffset;

HAnimClass	*		HandsAnims[ NUM_WEAPON_STATES ];
HAnimClass	*		WeaponAnims[ NUM_WEAPON_STATES ];

// Anim controls that provide blending
SimpleAnimControlClass	HandAnimControl;
SimpleAnimControlClass	WeaponAnimControl;

// Hands Bob
enum {
	BOB_NONE,
	BOB_IDLE,
	BOB_WALK,
	BOB_RUN,
};
int					BobState;
HTreeClass		*	BobHTree;
HAnimClass		*	BobHAnim;
float					BobFrame;
float					BobRecoil;


static	void		Set_Bob( int bob_state );
static	void		Set_Bob_Recoil( float amount );

static	void		Aquire_Hands_Assets( void );
static	void		Release_Hands_Assets( void );
static	void		Aquire_Weapon_Assets( const WeaponClass * weapon );
static	void		Release_Weapon_Assets( void );

/*
**
*/
void 	WeaponViewClass::Init()
{
	WeaponModel = NULL;
	HandsPhysObj = NULL;
	HandAnimControl.Set_Model( NULL );
	WeaponAnimControl.Set_Model( NULL );

	ClipModel = NULL;
	
	for ( int i = 0; i < NUM_WEAPON_STATES; i++ ) {
		WeaponAnims[i] = NULL;
		HandsAnims[i] = NULL;
	}

	WeaponState = -1;

	BobState = BOB_NONE;
	BobHTree = NULL;
	BobHAnim = NULL;
	BobFrame = 0;
	BobRecoil = 0;

	WeaponViewEnabled = false;
}

/*
**
*/
void 	WeaponViewClass::Shutdown()
{
	Set_Bob( BOB_NONE );

	Release_Weapon_Assets();
	Release_Hands_Assets();
}

/*
**
*/
void 	WeaponViewClass::Reset()
{
	Set_Bob( BOB_NONE );

	Release_Weapon_Assets();
	Release_Hands_Assets();
}


enum	{
	CHUNKID_VARIABLES						=	730011054,

	MICROCHUNKID_HANDS_PHYS_OBJ		=	1,
	MICROCHUNKID_ENABLED,
};

bool	WeaponViewClass::Save( ChunkSaveClass &csave )
{
	csave.Begin_Chunk( CHUNKID_VARIABLES );
		// If the scene has our hands, we must save and swizzle them
		if ( HandsPhysObj != NULL && COMBAT_SCENE->Contains( HandsPhysObj ) ) {
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_HANDS_PHYS_OBJ, HandsPhysObj );
		}
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ENABLED, WeaponViewEnabled );
	csave.End_Chunk();
	return true;
}

bool	WeaponViewClass::Load( ChunkLoadClass &cload )
{
	Release_Weapon_Assets();
	Release_Hands_Assets();

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_VARIABLES:
			{
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_HANDS_PHYS_OBJ, HandsPhysObj );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ENABLED, WeaponViewEnabled );

						default:
							Debug_Say(("Unhandled Micro Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
							break;
					}
					cload.Close_Micro_Chunk();
				}

				break;
			}

			default:
				Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}
		cload.Close_Chunk();
	}

	if ( HandsPhysObj != NULL ) {
		REQUEST_REF_COUNTED_POINTER_REMAP ((RefCountClass **)&HandsPhysObj);
	}

	return true;
}

/*
**
*/
void 	WeaponViewClass::Enable( bool enable )
{
	WeaponViewEnabled = enable;
}

/*
**	called each time through the main loop
*/
void 	WeaponViewClass::Think()
{
	WWPROFILE( "WeaponView Think" );

	bool bail = false;

	if ( COMBAT_CAMERA && COMBAT_CAMERA->Is_Star_Sniping() ) {
		bail = true;
	}

	if ( COMBAT_CAMERA && COMBAT_CAMERA->Is_Using_Host_Model() ) {
		bail = true;
	}

	if ( COMBAT_STAR == NULL ) {
		bail = true;
	}

	if ( !WeaponViewEnabled ) {
		bail = true;
	}

	// No first person weapon in vehicles
	if ( COMBAT_STAR && COMBAT_STAR->Get_Vehicle() != NULL ) {
		bail = true;
	}

	// not if in trans (camera lerping )
	if ( COMBAT_CAMERA && COMBAT_CAMERA->Is_Lerping() ) {
		bail = true;
	}


	WeaponClass * weapon = NULL;

	if ( COMBAT_STAR != NULL ) {
		weapon = COMBAT_STAR->Get_Weapon();
	}

	if ( weapon == NULL && WeaponModel == NULL ) {
		bail = true;
	}

	if ( weapon != NULL ) {
		if ( ( weapon->Get_Style() == WEAPON_HOLD_STYLE_C4 ) || 
			  ( weapon->Get_Style() == WEAPON_HOLD_STYLE_BEACON ) ) {
			if ( weapon->Get_Total_Rounds() == 0 ) {
				if ( WeaponModel == NULL || WeaponState == WEAPON_STATE_IDLE ) {
					bail = true;
				}
			}
		}
	}

	// If not in first person mode, hide the model and bail
	if ( bail ) {
		if ( HandsPhysObj != NULL && COMBAT_SCENE->Contains( HandsPhysObj ) ) {
			COMBAT_SCENE->Remove_Object( HandsPhysObj );
		}

		// To force an enter when we come back
		WeaponState = WEAPON_STATE_EXIT;

		Release_Weapon_Assets();
		return;
	}

	// Setup camera bob
	if ( COMBAT_STAR ) {
		Vector3 vel;
		COMBAT_STAR->Get_Velocity( vel );
		if ( vel.Length() > WWMATH_EPSILON ) {
			if ( COMBAT_STAR->Is_Slow() ) {
				Set_Bob( BOB_WALK );
			} else {
				Set_Bob( BOB_RUN );
			}
		} else {
			Set_Bob( BOB_IDLE );
		}

		// Setup weapon State
		if ( weapon != NULL && weapon->Is_Firing() ) {
			Set_Bob_Recoil( 0.15f );
		}
	}

	// is the current (non-looping) animation completed
	bool	is_current_complete = true;
#if 0
	if ( HandsPhysObj != NULL && (WeaponState != WEAPON_STATE_IDLE) ) {
		RenderObjClass * hands_model = HandsPhysObj->Peek_Model();
		if ( hands_model != NULL && ( hands_model->Peek_Animation() != NULL ) &&
			  ((Animatable3DObjClass*)hands_model)->Is_Animation_Complete() == false ) {
			is_current_complete = false;
		}
	}
#else
	if ( HandsPhysObj != NULL && (WeaponState != WEAPON_STATE_IDLE) ) {
		is_current_complete = HandAnimControl.Is_Complete();
	}
#endif

	bool	muzzle_flash_on = false;

	static bool	ForceFireLoop = false;

	// What state are we going to?
	int new_weapon_state = WEAPON_STATE_IDLE;
	if ( COMBAT_STAR ) {
		WeaponClass	* star_weapon = COMBAT_STAR->Get_Weapon();
		if ( star_weapon != NULL ) {
			if ( star_weapon->Is_Reloading() ) {
				new_weapon_state = WEAPON_STATE_RELOAD;
			} else if ( star_weapon->Is_Firing() ) {
				if ( WeaponState == WEAPON_STATE_FIRE ) {
					ForceFireLoop = true;
				}
				new_weapon_state = WEAPON_STATE_FIRE;

				if ( WeaponState == WEAPON_STATE_RELOAD ) {
					is_current_complete = true;	// Force a fire during reload
				}

				muzzle_flash_on = true;

				if ( WeaponModel != NULL ) {
					// if this gun has an eject bone, eject a shell.
					int eject_index = WeaponModel->Get_Bone_Index( "eject" );
					if ( eject_index > 0 ) {
						star_weapon->Make_Shell_Eject( WeaponModel->Get_Bone_Transform( eject_index ) );
					}
				}
			}

		}
	}

	const char* cur_weapon_model_name="";

	if ( WeaponModel ) {
		cur_weapon_model_name = WeaponModel->Get_Name();
	}

	StringClass new_weapon_model_name(true);

	if ( weapon ) {
		Get_Render_Obj_Name_From_Filename( new_weapon_model_name, weapon->Get_First_Person_Model_Name() );
	}

	// If we need to change the assets,
	if ( openw3d::string_compare( cur_weapon_model_name,new_weapon_model_name)!=0) {
//	if ( cur_weapon_model_name.Compare_No_Case( new_weapon_model_name ) != 0 ) {

		// If we don't have a weapon, switch!
		if ( WeaponModel == NULL ) {
			WeaponState = WEAPON_STATE_EXIT;
			is_current_complete = true;
		} 
			
		if ( WeaponState != WEAPON_STATE_EXIT ) {	// If we have a current weapon, exit it
			new_weapon_state = WEAPON_STATE_EXIT;
		} else {
			if ( is_current_complete ) {
				// If we are done exiting, switch and enter
				Release_Weapon_Assets();
				Aquire_Weapon_Assets( COMBAT_STAR->Get_Weapon() );
				new_weapon_state = WEAPON_STATE_ENTER;
			}
		}
	}

	if ( WeaponModel ) {
		// Update the muzzle flash
		RenderObjClass * model = WeaponModel;
		if ( model && (LastMuzzleFlash != muzzle_flash_on)) {
			for (int i=0; i<model->Get_Num_Sub_Objects(); i++) {
				RenderObjClass * robj = model->Get_Sub_Object(i);
				if (strstr(robj->Get_Name(),"MUZZLEFLASH") || strstr(robj->Get_Name(),"MZ")) {
					robj->Set_Hidden( !muzzle_flash_on );
				}
				robj->Release_Ref();
			}
			LastMuzzleFlash = muzzle_flash_on;
		}
	}


	if ( HandsPhysObj != NULL && WeaponModel != NULL ) {

		// make sure the "hands" object has the stealth effect
		if (COMBAT_STAR->Peek_Stealth_Effect() != NULL) {
			HandsPhysObj->Add_Effect_To_Me(COMBAT_STAR->Peek_Stealth_Effect());
		}

		// Add all rendobjs from the scene
		if ( !COMBAT_SCENE->Contains( HandsPhysObj ) ) {
			COMBAT_SCENE->Add_Dynamic_Object( HandsPhysObj );

			if ( new_weapon_state == WEAPON_STATE_ENTER ) {
				new_weapon_state = WEAPON_STATE_IDLE;
				is_current_complete = true;
			}
		}

		// When stopping, go to spin down
		if ( is_current_complete && 
				WeaponState == WEAPON_STATE_FIRE && 
				new_weapon_state == WEAPON_STATE_IDLE ) {
			// if the anims exist...
//			new_weapon_state = WEAPON_STATE_SPIN_DOWN;
		}

		if ( WeaponState != WEAPON_STATE_IDLE ) {
			if ( is_current_complete ) {
				// If done playing the current non-looping anim, go to next state

				if ( WeaponState == WEAPON_STATE_FIRE && ForceFireLoop ) {
					ForceFireLoop = false;	// Clear any force fire
					// Keep it by restarting the fire anim

					RenderObjClass * hands_model = HandsPhysObj->Peek_Model();
					if ( hands_model != NULL ) {										
//						hands_model->Set_Animation( HandsAnims[ WeaponState ], 0, RenderObjClass::ANIM_MODE_ONCE );
						HandAnimControl.Set_Animation( HandsAnims[ WeaponState ] );
						HandAnimControl.Set_Mode( ANIM_MODE_ONCE, 0 );
						HandAnimControl.Update(0);
					}
//					WeaponModel->Set_Animation( WeaponAnims[ WeaponState ], 0, RenderObjClass::ANIM_MODE_ONCE );
					WeaponAnimControl.Set_Animation( WeaponAnims[ WeaponState ] );
					WeaponAnimControl.Set_Mode( ANIM_MODE_ONCE, 0 );
					WeaponAnimControl.Update(0);
				} else {
					WeaponState = -1;
				}
			}
		}

		if ( new_weapon_state == WEAPON_STATE_EXIT ) {
			WeaponState = -1; // force a change
		}

		// If playing a looping anim, switch if needed
		if ( (WeaponState == -1) || ( WeaponState == WEAPON_STATE_IDLE ) ) {
			if ( new_weapon_state != WeaponState ) {
				WeaponState = new_weapon_state;


				float anim_blend_time = 0.15f;
				if ( new_weapon_state == WEAPON_STATE_FIRE ) {
					anim_blend_time = 0;	// No blend when firing
				}

				int mode = ( WeaponState == WEAPON_STATE_IDLE ) ? RenderObjClass::ANIM_MODE_LOOP : RenderObjClass::ANIM_MODE_ONCE;
				RenderObjClass * hands_model = HandsPhysObj->Peek_Model();
				if ( hands_model != NULL ) {
					mode = ( WeaponState == WEAPON_STATE_IDLE ) ? ANIM_MODE_LOOP : ANIM_MODE_ONCE;
//					hands_model->Set_Animation( HandsAnims[ WeaponState ], 0, mode );
					HandAnimControl.Set_Animation( HandsAnims[ WeaponState ], anim_blend_time );
					HandAnimControl.Set_Mode( (AnimMode)mode );
				}
//				WeaponModel->Set_Animation( WeaponAnims[ WeaponState ], 0, mode );
  				WeaponAnimControl.Set_Animation( WeaponAnims[ WeaponState ], anim_blend_time );
				mode = ( WeaponState == WEAPON_STATE_IDLE ) ? ANIM_MODE_LOOP : ANIM_MODE_ONCE;
  				WeaponAnimControl.Set_Mode( (AnimMode)mode );

//				if ( HandsAnims[ WeaponState ] != NULL ) {
//					Debug_Say(( "Playing %s\n", HandsAnims[ WeaponState ]->Get_Name() ));
//				}
			}
		}


		// Lets put the gun relative to the camera
		Matrix3D tm = COMBAT_CAMERA->Get_Transform();

		// Add bobing position
		if ( BobHTree != NULL && BobHAnim != NULL ) {
			BobFrame += TimeManager::Get_Frame_Seconds() * BobHAnim->Get_Frame_Rate();
			if ( BobFrame > BobHAnim->Get_Num_Frames()-1 ) {
				BobFrame -= BobHAnim->Get_Num_Frames()-1;
			}
#if 0 // no more programatic recoil
			tm.Translate( 0, 0, BobRecoil );

			if ( BobRecoil > 0 ) {
				BobRecoil -= TimeManager::Get_Frame_Real_Seconds() * 2;
				if ( BobRecoil < 0 ) {
					BobRecoil = 0;
				}
			}
#endif

			BobHTree->Anim_Update( tm, BobHAnim, BobFrame );

			// Now get the camera bone
//			Debug_Say(( "%f %f %f\n", tm.Get_Translation().X, tm.Get_Translation().Y, tm.Get_Translation().Z ));
			tm = BobHTree->Get_Transform( BobHTree->Get_Bone_Index( "CAMERA" ) );
//			Debug_Say(( "%f %f %f\n", tm.Get_Translation().X, tm.Get_Translation().Y, tm.Get_Translation().Z ));
		}

		// Convert from camera to world convention
		tm.Rotate_Z( DEG_TO_RADF(90.0) );
		tm.Rotate_Y( DEG_TO_RADF(90.0) );

		Vector3	fp_offset = HandsOffset + COMBAT_CAMERA->Get_First_Person_Offset_Tweak();
		tm.Translate( fp_offset );

		//WWASSERT(COMBAT_STAR != NULL);//TSS
		if (COMBAT_STAR != NULL) {
			//
			// COMBAT_STAR may be NULL during MP game intermission
			//
			Matrix3D obj_convention_camera = COMBAT_CAMERA->Get_Transform();
			obj_convention_camera.Rotate_Z( DEG_TO_RADF(90.0) );
			obj_convention_camera.Rotate_Y( DEG_TO_RADF(90.0) );

			Vector3	camera_space_target;
			Vector3	camera_space_source;
			Vector3	target_pos = COMBAT_STAR->Get_Targeting_Pos();
			
			// Always look at a point a constant distance ahead, to avoid the gun poping around
			target_pos = COMBAT_CAMERA->Get_Transform() * Vector3( 0,0,-100 );
			Matrix3D::Inverse_Transform_Vector( obj_convention_camera, target_pos, &camera_space_target );
			Matrix3D::Inverse_Transform_Vector( obj_convention_camera, tm.Get_Translation(), &camera_space_source );
			Matrix3D camera_space_aim;
			camera_space_aim.Obj_Look_At( camera_space_source, camera_space_target, 0 );
			Matrix3D::Multiply( obj_convention_camera, camera_space_aim, &tm );

		}

		HandsPhysObj->Set_Transform( tm );

		// Now, pull the hands backwards enough to clear any walls
		if ( WeaponModel && COMBAT_CAMERA) {

			const Matrix3D & muzzle_tm = WeaponModel->Get_Bone_Transform( "muzzlea0" );
			Vector3 ray_end = muzzle_tm * Vector3( 0.1f, 0, 0 );

			Vector3 ray_start = COMBAT_CAMERA->Get_Transform().Get_Translation();
			ray_start.Z = ray_end.Z;

			LineSegClass ray;
			ray.Set( ray_start, ray_end );

			CastResultStruct result;
			PhysRayCollisionTestClass raytest(ray, &result, BULLET_COLLISION_GROUP, COLLISION_TYPE_PROJECTILE);

			{ 
				if (COMBAT_STAR != NULL && COMBAT_STAR->Peek_Physical_Object()!=NULL) {
					COMBAT_STAR->Peek_Physical_Object()->Inc_Ignore_Counter();
				}
				WWPROFILE( "Cast Ray" );
				WWASSERT(COMBAT_SCENE != NULL);
				COMBAT_SCENE->Cast_Ray( raytest );
				if (COMBAT_STAR != NULL && COMBAT_STAR->Peek_Physical_Object()!=NULL) {
					COMBAT_STAR->Peek_Physical_Object()->Dec_Ignore_Counter();
				}
			}

/*			if ( raytest.CollidedPhysObj != NULL && raytest.CollidedPhysObj->Get_Observer() != NULL ) {
				DamageableGameObj * obj = ((CombatPhysObserverClass *)raytest.CollidedPhysObj->Get_Observer())->As_DamageableGameObj();
				if ( obj ) {
					Debug_Say(( "Hit %s\n", obj->Get_Definition().Get_Name() ));
				}
			}*/

			// Determine the Camera Target Point and object
			Vector3	collision_pt;
			ray.Compute_Point( raytest.Result->Fraction, &collision_pt );
			collision_pt -= ray_end;
			float pullback = collision_pt.Length();
//			Debug_Say(( "Pullback = %f\n", pullback ));
			tm.Translate( Vector3( -pullback, 0, 0 ) );

			HandsPhysObj->Set_Transform( tm );
		}

	}

	if ( HandsPhysObj && HandsPhysObj->Peek_Model() ) {
		HandAnimControl.Update( TimeManager::Get_Frame_Seconds() );
	}
	if ( WeaponModel ) {
		WeaponAnimControl.Update( TimeManager::Get_Frame_Seconds() );
	}
}

Vector3	WeaponViewClass::Get_Muzzle_Pos()
{
	if ( WeaponModel ) {
		return WeaponModel->Get_Bone_Transform( "muzzlea0" ).Get_Translation();
	} 

	return COMBAT_CAMERA->Get_Transform().Get_Translation();
}

const char * WeaponActionNames[ NUM_WEAPON_STATES ] = {
	"IDLE",		//WEAPON_STATE_IDLE,
	"FIRE",		//WEAPON_STATE_FIRE,
	"RELOD",		//WEAPON_STATE_RELOAD,
	"ENTER",		//WEAPON_STATE_ENTER,
	"EXIT",		//WEAPON_STATE_EXIT,
//	"EFIRE",		//WEAPON_STATE_SPIN_DOWN,
};

/*
** Get and release the Weapon specific assets
** Each weapon has a name ie: PIST
** The weapon model is in the form F_GM_<NAME> ie: F_GM_PIST
** The clip model is in the form F_CM_<NAME> ie: F_CM_PIST
** The weapon states are IDLE, FIRE, RELOAD, ENTER, EXIT
** For each weapon state, except ENTER and EXIT, we have a weapon anim tied to the weapon model (use IDLE anim for ENTER/EXIT )
** The weapon anims are in the form F_GA_<NAME>_<STATE> ie: F_GA_PIST_IDLE, F_GA_PIST_FIRE
** For each weapon state, we have a hand anim tied to the F_SKELETON
** The hand anims are in the form F_GA_<NAME>_<STATE> ie: F_GA_PIST_IDLE, F_GA_PIST_FIRE
** Weapon anims should be the same duration (number of frames) and the corresponding hand anim (except looping +IDLES)
*/
static void	Aquire_Weapon_Assets( const WeaponClass * weapon )
{
	if ( HandsPhysObj == NULL ) {
		Aquire_Hands_Assets();
	} else {
		HandAnimControl.Set_Model( HandsPhysObj->Peek_Model() );
		HandAnimControl.Set_Animation( (HAnimClass*)NULL );
	}


	if ( HandsPhysObj == NULL || HandsPhysObj->Peek_Model() == NULL ) {
		return;
	}

	RenderObjClass * hands_model = HandsPhysObj->Peek_Model();

	StringClass weapon_name;

	if ( weapon ) {
//		Debug_Say(( "Loading Weapon from %s\n", weapon->Get_First_Person_Model_Name() ));
		WeaponModel = ::Create_Render_Obj_From_Filename( weapon->Get_First_Person_Model_Name() );
		LastMuzzleFlash = true;	// force it off
//		Debug_Say(( "Loaded %s\n", WeaponModel ? WeaponModel->Get_Name() : "NONE" ));

		// Use model name
		if ( WeaponModel != NULL ) {
			weapon_name = WeaponModel->Get_Name() + 5;		// Get the weapon name from the model
			Debug_Say(( "weapon_name is %s\n", (const char *)weapon_name ));

			StringClass clip_name;
			clip_name = WeaponModel->Get_Name();
			if ( clip_name.Get_Length() > 4 ) {
				clip_name[2] = 'C';
				clip_name[3] = 'M';
			}
			ClipModel = WW3DAssetManager::Get_Instance()->Create_Render_Obj( clip_name );
			if ( ClipModel != NULL ) {
				hands_model->Add_Sub_Object_To_Bone( ClipModel, "CLIPBONE" );
			}
		}
	}

	WeaponAnimControl.Set_Model( WeaponModel );
	WeaponAnimControl.Set_Animation( (HAnimClass*)NULL );

	if ( WeaponModel != NULL ) {

		// Add the model
		hands_model->Add_Sub_Object_To_Bone( WeaponModel, "GUNBONE" );

		HandsOffset = weapon->Get_First_Person_Model_Offset();

		StringClass weapon_htree_name;
		if ( WeaponModel->Get_HTree() != NULL ) {
			weapon_htree_name = WeaponModel->Get_HTree()->Get_Name();
		}

		SET_REF_OWNER( WeaponModel );

		for ( int i = 0; i < NUM_WEAPON_STATES; i++ ) {

			StringClass anim_name;

			// Get Weapon Anims, use the idle anim for enter and exit
			int state = ( i >= WEAPON_STATE_ENTER ) ? WEAPON_STATE_IDLE : i;
			anim_name.Format( "%s.F_GA_%s_%s", weapon_htree_name, weapon_name, WeaponActionNames[state] );
//			Debug_Say(( "Loading Weapon Anim %s\n", anim_name ));
			WeaponAnims[i] = WW3DAssetManager::Get_Instance()->Get_HAnim( anim_name );
			if ( WeaponAnims[i] == NULL ) {
				Debug_Say(( "Missing Weapon Anim %s\n", anim_name ));
			}

			// Get Hands Anims
			anim_name.Format( "F_SKELETON.F_HA_%s_%s", weapon_name, WeaponActionNames[i] );
//			Debug_Say(( "Loading Hands Anim %s\n", anim_name ));
			HandsAnims[i] = WW3DAssetManager::Get_Instance()->Get_HAnim( anim_name );
			if ( HandsAnims[i] == NULL ) {
				Debug_Say(( "Missing Hands Anim %s\n", anim_name ));
				HandsAnims[i] = WW3DAssetManager::Get_Instance()->Get_HAnim( "F_SKELETON.F_HA_PIST_IDLE" );
			}
		}
	}
}

static void	Release_Weapon_Assets( void )
{
	WeaponAnimControl.Set_Model( NULL );
	WeaponAnimControl.Set_Animation( (HAnimClass*)NULL );

	if ( WeaponModel != NULL ) {
		// Remove
		WWASSERT( HandsPhysObj != NULL );
		RenderObjClass * hands_model = HandsPhysObj->Peek_Model();
		WWASSERT( hands_model != NULL );
		hands_model->Remove_Sub_Objects_From_Bone( "GUNBONE" );

		WeaponModel->Release_Ref();
		WeaponModel = NULL;
	}

	if ( ClipModel != NULL ) {
		// Remove
		WWASSERT( HandsPhysObj != NULL );
		RenderObjClass * hands_model = HandsPhysObj->Peek_Model();
		WWASSERT( hands_model != NULL );
		hands_model->Remove_Sub_Objects_From_Bone( "CLIPBONE" );

		ClipModel->Release_Ref();
		ClipModel = NULL;
	}

	for ( int i = 0; i < NUM_WEAPON_STATES; i++ ) {
		if ( WeaponAnims[i] != NULL ) {
			WeaponAnims[i]->Release_Ref();
			WeaponAnims[i] = NULL;
		}

		if ( HandsAnims[i] != NULL ) {
			HandsAnims[i]->Release_Ref();
			HandsAnims[i] = NULL;
		}
	}
}

/*
** Get and release the Hand assets
** All hand models are built on the F_SKELETON HTree, which has a GUNBONE and CLIPBONE
** The hand models can be different for each player model, and are in the form F_HM_XXX ie: F_HM_HAVOC
** The hand anims are created from the particular weapon model loaded.
*/
static void	Aquire_Hands_Assets( void )
{
	StringClass name = COMBAT_STAR->Get_First_Person_Hands_Model_Name();
//	Debug_Say(( "Loading Hands from %s\n", name ));

	HandsPhysObj = new DecorationPhysClass();
	if ( HandsPhysObj != NULL ) {
		HandsPhysObj->Set_Collision_Group( UNCOLLIDEABLE_GROUP );

		RenderObjClass * model = ::Create_Render_Obj_From_Filename( name );
		if ( model != NULL ) {
			HandAnimControl.Set_Model( model );
			HandAnimControl.Set_Animation( (HAnimClass*)NULL );
			HandsPhysObj->Set_Model( model );
			model->Release_Ref();
		}
	}
}

static void	Release_Hands_Assets( void )
{
	HandAnimControl.Set_Model( NULL );
	HandAnimControl.Set_Animation( (HAnimClass*)NULL );

	if ( HandsPhysObj != NULL ) {
		if ( COMBAT_SCENE->Contains( HandsPhysObj ) ) {
			COMBAT_SCENE->Remove_Object( HandsPhysObj );
		}
		HandsPhysObj->Release_Ref();
		HandsPhysObj = NULL;
	}
}


/*
**
*/
static const char * BobNames[] = {
	"",				// BOB_NONE
	"F_CA_IDLE",	// BOB_IDLE
	"F_CA_WALK",	// BOB_WALK
	"F_CA_RUN",		// BOB_RUN
};

static void	Set_Bob( int bob_state )
{
	if ( BobState != bob_state ) {
		BobState = bob_state;

		// Release the old copys, if any
		BobHTree = NULL;
		REF_PTR_RELEASE( BobHAnim );

		StringClass name(BobNames[ BobState ],true);

//		Debug_Say(( "New Camera Bob State %d \"%s\"\n", BobState, name ));

		if ( !name.Is_Empty() ) {
			BobHTree = WW3DAssetManager::Get_Instance()->Get_HTree( name );
			StringClass anim(0,true);
			anim.Format( "%s.%s", name,name );
			BobHAnim = WW3DAssetManager::Get_Instance()->Get_HAnim( anim );
		}
	}
}


static void	Set_Bob_Recoil( float amount )		
{ 
	BobRecoil = amount; 
}

