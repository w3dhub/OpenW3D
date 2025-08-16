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
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/SoundEnvironment.cpp  $*
 *                                                                                             *
 *                       Author:: Ian Leslie		                                               *
 *                                                                                             *
 *                     $Modtime:: 5/21/01 2:16p                                     $*
 *                                                                                             *
 *                    $Revision:: 7                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "SoundEnvironment.h"
#include "camera.h"
#include "combat.h"
#include "gameobjmanager.h"
#include "phys.h"
#include "physcoltest.h"
#include "pscene.h"


/***********************************************************************************************
 * SoundEnvironmentClass::SoundEnvironmentClass --															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/16/01    IML : Created.                                                                * 
 *=============================================================================================*/
SoundEnvironmentClass::SoundEnvironmentClass()
	: UserCount (0)
{
	// Initialize the amplitude mixing buffer.
	AmplitudeBuffer = new float [AMPLITUDE_BUFFER_SIZE];
	WWASSERT (AmplitudeBuffer != NULL);
	Reset();
}


/***********************************************************************************************
 * SoundEnvironmentClass::Reset --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/16/01    IML : Created.                                                                * 
 *=============================================================================================*/
void SoundEnvironmentClass::Reset()
{
	// Clear the amplitude buffer.
	AmplitudeIndex = 0;
	AmplitudeSum	= 0.0f;
	for (unsigned i = 0; i < AMPLITUDE_BUFFER_SIZE; i++) {
		AmplitudeBuffer [i] = 0.0f;
	}
}


/***********************************************************************************************
 * SoundEnvironmentClass::~SoundEnvironmentClass --														  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/20/01    PDS : Created.                                                                * 
 *=============================================================================================*/
SoundEnvironmentClass::~SoundEnvironmentClass()
{
	delete [] AmplitudeBuffer;
}


/***********************************************************************************************
 * SoundEnvironmentClass::Update --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/16/01    IML : Created.                                                                * 
 *=============================================================================================*/
void SoundEnvironmentClass::Update (PhysicsSceneClass *scene, CameraClass *camera)
{
	// Optimization: only update if this object is being used.
	if (UserCount > 0) {

		Vector3 cameraposition;
		Vector3 scenemin, scenemax;
		float	  amplitude;

		cameraposition = camera->Get_Position();

		// Get the highest point in the scene.
		scene->Get_Level_Extents (scenemin, scenemax);

		// Expand the bounding box by a small amount so that we can distinguish between collisions with geometry and
		// collisions with the bounding box.
		scenemin.Z -= 1.0f;
		scenemax.Z += 1.0f;

		// Cast a ray from the camera position upwards.
		{
			Vector3						  raycastendpoint (cameraposition.X, cameraposition.Y, scenemax.Z);
			LineSegClass				  raycast (cameraposition, raycastendpoint);
			CastResultStruct			  rayresult;
			PhysRayCollisionTestClass raytest (raycast, &rayresult, TERRAIN_ONLY_COLLISION_GROUP, COLLISION_TYPE_PROJECTILE);
			
			scene->Cast_Ray (raytest);
			
			// Did the ray hit an object?
			if (raytest.Result->Fraction < 1.0f) {
				
				// Is the camera in an environment zone (in which case environmental sounds should not be heard)?
				if (GameObjManager::Is_In_Environment_Zone (cameraposition)) {
					amplitude = 0.0f;
				} else {
					amplitude = 0.5f;
				}
			} else {
				amplitude = 1.0f;
			}
		}

		AmplitudeSum -= AmplitudeBuffer [AmplitudeIndex];
		AmplitudeBuffer [AmplitudeIndex] = amplitude;
		AmplitudeSum += amplitude;
		AmplitudeIndex++;
		if (AmplitudeIndex >= AMPLITUDE_BUFFER_SIZE) AmplitudeIndex = 0;
	}
}
