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
 *                     $Archive:: /Commando/Code/wwphys/pscene_lighting.cpp                   $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 2/28/02 1:41p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   PhysicsSceneClass::Is_Sun_Light_Enabled -- Returns true if the sun-light is enabled       *
 *   PhysicsSceneClass::Enable_Sun_Light -- Enable/Disable the sun-light                       *
 *   PhysicsSceneClass::Get_Sun_Light -- Returns pointer to the sun-light object               *
 *   PhysicsSceneClass::Set_Sun_Light_Orientation -- Set the orientation of the sun-ligth      *
 *   PhysicsSceneClass::Get_Sun_Light_Orientation -- returns the sun-light orientation         *
 *   PhysicsSceneClass::Get_Sun_Light_Vector -- returns the sun-light vector                   *
 *   PhysicsSceneClass::Reset_Sun_Light -- Resets the sun-light to default settings            *
 *   PhysicsSceneClass::Set_Lighting_LOD_Cutoff -- Sets the LOD cutoff for lighting            *
 *   PhysicsSceneClass::Get_Lighting_LOD_Cutoff -- returns the LOD cutoff for lighting         *
 *   PhysicsSceneClass::Compute_Static_Lighting -- Compute the static lighting approximation   *
 *   PhysicsSceneClass::Invalidate_Lighting_Caches -- invalidate lighting caches in the given  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "pscene.h"
#include "rendobj.h"
#include "phys.h"
#include "staticphys.h"
#include "dx8renderer.h"
#include "simplevec.h"
#include "vp.h"
#include "lightphys.h"
#include "light.h"
#include "lightcull.h"


/***********************************************************************************************
 * PhysicsSceneClass::Set_Lighting_LOD_Cutoff -- Sets the LOD cutoff for lighting              *
 *                                                                                             *
 *    All lights below this intensity become purely ambient                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/10/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Set_Lighting_LOD_Cutoff(float intensity)
{
	LightEnvironmentClass::Set_Lighting_LOD_Cutoff(intensity);
}


/***********************************************************************************************
 * PhysicsSceneClass::Get_Lighting_LOD_Cutoff -- returns the LOD cutoff for lighting           *
 *                                                                                             *
 *    All lights below the returned intensity are being converted into pure ambient            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/10/2000  gth : Created.                                                                 *
 *=============================================================================================*/
float PhysicsSceneClass::Get_Lighting_LOD_Cutoff(void)
{
	return LightEnvironmentClass::Get_Lighting_LOD_Cutoff();
}


/***********************************************************************************************
 * PhysicsSceneClass::Is_Sun_Light_Enabled -- Returns true if the sun-light is enabled         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
bool PhysicsSceneClass::Is_Sun_Light_Enabled(void)
{
	return UseSun;
}


/***********************************************************************************************
 * PhysicsSceneClass::Enable_Sun_Light -- Enable/Disable the sun-light                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Enable_Sun_Light(bool onoff)
{
	UseSun = onoff;
}


/***********************************************************************************************
 * PhysicsSceneClass::Get_Sun_Light -- Returns pointer to the sun-light object                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
LightClass * PhysicsSceneClass::Get_Sun_Light(void)
{
	WWASSERT(SunLight);
	SunLight->Add_Ref();
	return SunLight;
}


/***********************************************************************************************
 * PhysicsSceneClass::Set_Sun_Light_Orientation -- Set the orientation of the sun-ligth        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Set_Sun_Light_Orientation(float yaw,float pitch)
{
	SunYaw = yaw;
	SunPitch = pitch;

	Matrix3D tm(1);
	tm.Rotate_Z(yaw);
	tm.Rotate_Y(DEG_TO_RADF(90.0f) - pitch);
	tm.Rotate_X(DEG_TO_RADF(180.0f));

	SunLight->Set_Transform(tm);
}


/***********************************************************************************************
 * PhysicsSceneClass::Get_Sun_Light_Orientation -- returns the sun-light orientation           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Get_Sun_Light_Orientation(float * set_yaw,float * set_pitch)
{
	*set_yaw = SunYaw;
	*set_pitch = SunPitch;
}


/***********************************************************************************************
 * PhysicsSceneClass::Get_Sun_Light_Vector -- returns the sun-light vector                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Get_Sun_Light_Vector(Vector3 * set_vector)
{
	WWASSERT(set_vector != NULL);
	const Matrix3D & tm = SunLight->Get_Transform();
	*set_vector = tm * Vector3(0,0,1);
}


/***********************************************************************************************
 * PhysicsSceneClass::Reset_Sun_Light -- Resets the sun-light to default settings              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Reset_Sun_Light(void)
{
	UseSun = false;
	SunLight->Set_Transform(Matrix3D(1));
	SunLight->Set_Ambient(Vector3(0,0,0));
	SunLight->Set_Diffuse(Vector3(1,1,1));
	SunLight->Set_Specular(Vector3(0,0,0));
	SunLight->Set_Flag(LightClass::NEAR_ATTENUATION,false);
	SunLight->Set_Flag(LightClass::FAR_ATTENUATION,false);
	Set_Sun_Light_Orientation(DEG_TO_RADF(0.0f),DEG_TO_RADF(70.0f));
}


/***********************************************************************************************
 * PhysicsSceneClass::Compute_Static_Lighting -- Compute the static lighting approximation     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/25/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Compute_Static_Lighting
(
	LightEnvironmentClass * light_env,
	const Vector3 & obj_center,
	bool use_sun,
	int vis_object_id
)
{
	WWASSERT(light_env != NULL);
	light_env->Reset(obj_center,Get_Ambient_Light());

	/*
	** Add in the sun
	*/
	if (use_sun) {
		light_env->Add_Light(*SunLight);
	}

	/*
	** Add in the static lights affecting this object
	*/
	StaticLightingSystem->Reset_Collection();
	StaticLightingSystem->Collect_Objects(obj_center);
	LightPhysClass * light = StaticLightingSystem->Get_First_Collected_Object();
	while (light != NULL) {

		if ((light->Is_Disabled() == false) && (light->Is_Vis_Object_Visible(vis_object_id))) {

			LightClass * light_obj = (LightClass *)light->Peek_Model();
			light_env->Add_Light(*light_obj);
		}

		light = StaticLightingSystem->Get_Next_Collected_Object(light);
	}
}


/***********************************************************************************************
 * PhysicsSceneClass::Invalidate_Lighting_Caches -- invalidate lighting caches in the given bo *
 *                                                                                             *
 *    This function finds all physics objects that contain cached lighting information within  *
 *    the given bounds and invalidates their caches.  This is used when a light source in the  *
 *    level changes state (e.g. when the buildings lose power, etc)                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 * box - all objects overlapping the specified box will have thier lighting cache reset        *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/26/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Invalidate_Lighting_Caches(const AABoxClass & box)
{
	NonRefPhysListClass list;
	Collect_Objects(box,true,true,&list);

	NonRefPhysListIterator it(&list);
	for (it.First(); !it.Is_Done(); it.Next()) {
		PhysClass * obj = it.Peek_Obj();
		if (!obj->Is_Pre_Lit()) {
			obj->Invalidate_Static_Lighting_Cache();
		}
	}
}
