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
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/dynamicshadowmanager.cpp              $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 12/07/01 4:39p                                              $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "dynamicshadowmanager.h"
#include "chunkio.h"
#include "phys.h"
#include "dyntexproject.h"
#include "pscene.h"
#include "physcoltest.h"
#include "light.h"
#include "texture.h"
#include "physresourcemgr.h"


#define SINGLE_SHADOW_CODE 1

DynamicShadowManagerClass::DynamicShadowManagerClass(PhysClass & parent) :
	Parent(parent),
	Shadow(NULL),
	ShadowNearZ(-1.0f),
	ShadowFarZ(-1.0f),
	ForceUseBlobBox(false),
	BlobBoxProjectionScale(1,1,1)
{
}

DynamicShadowManagerClass::~DynamicShadowManagerClass(void)
{
	Release_Shadow();
}


void DynamicShadowManagerClass::Update_Shadow(void)
{
#if SINGLE_SHADOW_CODE	
	/*
	** Shadow Update
	** - if shadows are off, release projector and RETURN
	** - find dominant light source (or multiple sources?)
	** - if no light sources, set projector intensity to 0.0 and RETURN
	** - for each light source
	**   - re-use projector from previous frame or allocate a new projector 
	**   - initialize the projection parameters, depending on: blob/real,point/directional
	**   - set the shadow's intensity by attenuating it with distance from lightsource
	*/
	PhysicsSceneClass * scene = PhysicsSceneClass::Get_Instance();
	PhysicsSceneClass::ShadowEnum shadow_mode = scene->Get_Shadow_Mode();
	
	float near_atten;
	float far_atten;
	scene->Get_Shadow_Attenuation(&near_atten,&far_atten);

	RenderObjClass * model = Parent.Peek_Model();

	/*
	** Grab the bounding box info based on whether we are using blobs or real projections
	** use blobs  IF(the mode is BLOBS or (the mode is BLOBS_PLUS and we're not the 'star'))
	*/
	AABoxClass objbox;

	bool use_blob = (shadow_mode == PhysicsSceneClass::SHADOW_MODE_BLOBS) ||
						 ((shadow_mode == PhysicsSceneClass::SHADOW_MODE_BLOBS_PLUS) && (!Parent.Is_Force_Projection_Shadow_Enabled()));

	if (use_blob) {
		Parent.Get_Shadow_Blob_Box(&objbox);
	} else {
		if (ForceUseBlobBox) {
			Parent.Get_Shadow_Blob_Box(&objbox);
			objbox.Extent.Scale(BlobBoxProjectionScale);
		} else {
			model->Get_Obj_Space_Bounding_Box(objbox);
		}
	}

	/*
	** Check if the center of the bounding box is well beyond the shadow fading
	** distance.  If so, we will exit early
	*/
	Vector3 position;
	Matrix3D::Transform_Vector(Parent.Get_Transform(),objbox.Center,&position);
	float shadow_shutoff2 = far_atten * 1.3f * far_atten * 1.3f;
	float shadow_dist2 = (position - scene->Get_Last_Camera_Position()).Length2();

	/*
	** If shadow generation is disabled or we are farther than 1.2x the shadow attenuation distance,
	** release any projector that we may have and return
	*/
	if (	(Parent.Do_Any_Effects_Suppress_Shadows()) ||
			(Parent.Is_Shadow_Generation_Enabled() == false) || 
			(shadow_mode == PhysicsSceneClass::SHADOW_MODE_NONE) ||
			(shadow_dist2 > shadow_shutoff2) || 
			(model == NULL) ||
			(model->Is_Hidden()) ||
			(objbox.Extent.Length2() < 0.1f) )
	{
		if (Shadow != NULL) {
			scene->Remove_Dynamic_Texture_Projector(Shadow);
			Shadow->Release_Ref();
			Shadow = NULL;
		}
		return;
	}

	/*
	** Find dominant light source.  First check if sun is available, then find closest
	** static light.
	*/
	bool found_light = false;
	Vector3 sunlight;
	scene->Get_Sun_Light_Vector(&sunlight);
	
	if (Parent.Is_In_The_Sun()) {

		/*
		** We can see the sunlight so set our projector up for it.
		** Our shadow is attenuated only by distance from the camera so
		** set the intensity to 'normal' and set the flag which causes
		** the physics scene to attenuate it with distance.
		*/
		Allocate_Shadow();

		/*
		** Save parameters in the projector so it can continue to
		** update later if it needs to fade out
		*/
		LightClass * sun = scene->Get_Sun_Light();
		Shadow->Enable_Perspective(false);
		Shadow->Set_Light_Source_ID((uint32)sun);
		Shadow->Set_Light_Vector(sunlight);
		sun->Release_Ref();
		found_light = true;

	} else {

// FIXME (gth) Disabling local shadows
#if 0
		/*
		** We couldn't use the sunlight so now we look for the nearest
		** local light source which casts shadows.  If we find one, initialize
		** our shadow projector with it.
		*/
		NonRefPhysListClass lightlist;
		scene->Collect_Lights(position,true,false,&lightlist);
	
		if (!lightlist.Is_Empty()) {

			/*
			** Ensure that a shadow is allocated!
			*/
			Allocate_Shadow();

			LightClass * best_light = NULL;
			NonRefPhysListIterator it(&lightlist);
			for (it.First(); !it.Is_Done(); it.Next()) {
				best_light = (LightClass *)(it.Peek_Obj()->Peek_Model());
				break;				
			}

			if (best_light) {

#if TRUE_PERSPECTIVE_SHADOWS		// This code uses true perspective projection for local light sources
				Shadow->Enable_Perspective(true);
				Shadow->Set_Light_Source_ID((uint32)best_light);
				Shadow->Set_Light_Vector(best_light->Get_Position());

#else			// This code uses an orthographic approximation 
				Shadow->Enable_Perspective(false);
				Shadow->Set_Light_Source_ID((uint32)best_light);
				
				Vector3 direction;
				Get_Position(&direction);
				direction -= best_light->Get_Position();
				direction.Normalize();
				Shadow->Set_Light_Vector(direction);
#endif
				found_light = true;

				DEBUG_RENDER_VECTOR(position,best_light->Get_Position()-position,Vector3(1,1,1));
			}
		} 
#endif //0
	}

	if (found_light) {
		
		/*
		** We have a shadow and a light so update the projection parameters.
		** If we are using blob shadows, just plug in the blob texture and 
		** clear the dirty flag.  Otherwise, mark the texture dirty so it will
		** be re-generated if the shadow actually gets projected onto something
		*/
		Shadow->Update_Projection(objbox,Parent.Get_Transform(),ShadowNearZ,ShadowFarZ);
		Shadow->Set_Intensity(scene->Get_Shadow_Normal_Intensity());

		if (use_blob) {
			TextureClass * shadow_texture = PhysResourceMgrClass::Get_Shadow_Blob_Texture();
			WWASSERT(shadow_texture != NULL);
			Shadow->Set_Texture(shadow_texture);
			shadow_texture->Release_Ref();
			Shadow->Set_Texture_Dirty(false);
		} else {
			Shadow->Set_Texture_Dirty(true);
		}

	} else {
		
		/*
		** If we have a shadow but we don't want it any more, wait until
		** it fades out before we release it
		*/
		if (Shadow) {
			Shadow->Set_Intensity(0.0f);
			if (!Shadow->Is_Intensity_Zero()) {
				Shadow->Update_Projection(objbox,Parent.Get_Transform(),ShadowNearZ,ShadowFarZ);
			}
		}
	
	}

#else

	/*
	** - Collect all lights that want to and can cast a shadow with this object (raytest, etc)
	** - Reduce number of lights in list to MaxShadowsPerObject
	** - Create new empty shadow object list
	** - For each light
	**   - Try to find shadow which used this light from prev-frame's shadow list, if found
	**     remove it from prev-frame's list and put in current frame's shadow list
	**     Else create a new shadow object, immediately set its intensity to zero (since it is turning on)
	**   - Initialize projector with light parameters
	** - For each shadow still in prev-frame's list
	**   - If intensity is zero, destroy it.
	**     Else
	**     - Set target intensity to zero
	**     - Update projection parameters, move into this frame's shadow list
	*/
	

#endif
}


void DynamicShadowManagerClass::Allocate_Shadow(void)
{
	if (Shadow == NULL) {
		PhysicsSceneClass * scene = PhysicsSceneClass::Get_Instance();
		
		Shadow = NEW_REF(DynTexProjectClass,(&Parent));
		Shadow->Enable_Attenuation(true);
		Shadow->Enable_Depth_Gradient(true);
		Shadow->Enable_Affect_Dynamic_Objects(false);
		Shadow->Set_Intensity(scene->Get_Shadow_Normal_Intensity(),true);
		Shadow->Peek_Material_Pass()->Enable_On_Translucent_Meshes(false);

		scene->Add_Dynamic_Texture_Projector(Shadow);
	}
}


void DynamicShadowManagerClass::Release_Shadow(void)
{
	if (Shadow) {
		if (PhysicsSceneClass::Get_Instance()->Contains(Shadow)) {
			PhysicsSceneClass::Get_Instance()->Remove_Dynamic_Texture_Projector(Shadow);
		}
		Shadow->Release_Ref();
		Shadow = NULL;
	}
}

