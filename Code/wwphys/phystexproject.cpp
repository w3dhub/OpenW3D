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
 *                     $Archive:: /Commando/Code/wwphys/phystexproject.cpp                    $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/11/02 7:06p                                               $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   PhysTexProjectClass::PhysTexProjectClass -- Constructor                                   *
 *   PhysTexProjectClass::~PhysTexProjectClass -- Destructor                                   *
 *   PhysTexProjectClass::Compute_Perspective_Projection -- compute projection of a Phys objec *
 *   PhysTexProjectClass::Compute_Ortho_Projection -- Compute projection of a Phys object      *
 *   PhysTexProjectClass::Compute_Texture -- Render the given Phys object to texture           *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "phystexproject.h"
#include "pscene.h"
#include "phys.h"
#include "rinfo.h"
#include "vertmaterial.h"
#include "lightenvironment.h"

/***********************************************************************************************
 * PhysTexProjectClass::PhysTexProjectClass -- Constructor                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/2000  gth : Created.                                                                 *
 *=============================================================================================*/
PhysTexProjectClass::PhysTexProjectClass(void)
{
}


/***********************************************************************************************
 * PhysTexProjectClass::~PhysTexProjectClass -- Destructor                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/2000  gth : Created.                                                                 *
 *=============================================================================================*/
PhysTexProjectClass::~PhysTexProjectClass(void)
{
}


/***********************************************************************************************
 * PhysTexProjectClass::Compute_Perspective_Projection -- compute projection of a Phys object  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool PhysTexProjectClass::Compute_Perspective_Projection
(
	PhysClass * obj,
	const Vector3 & lightpos,
	float nearz,
	float farz
)
{
	RenderObjClass * model = obj->Peek_Model();
	if (model == NULL) {
		WWDEBUG_SAY(("Attempting to generate projection for a NULL model\r\n"));
		return false;
	}

	AABoxClass box;
	model->Get_Obj_Space_Bounding_Box(box);
	const Matrix3D & tm = model->Get_Transform();

	return TexProjectClass::Compute_Perspective_Projection(box,tm,lightpos,nearz,farz);
}


/***********************************************************************************************
 * PhysTexProjectClass::Compute_Ortho_Projection -- Compute projection of a Phys object        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool PhysTexProjectClass::Compute_Ortho_Projection
(
	PhysClass * obj,
	const Vector3 & lightdir,
	float nearz,
	float farz
)
{
	RenderObjClass * model = obj->Peek_Model();
	if (model == NULL) {
		WWDEBUG_SAY(("Attempting to generate projection for a NULL model\r\n"));
		return false;
	}

	AABoxClass box;
	model->Get_Obj_Space_Bounding_Box(box);
	const Matrix3D & tm = model->Get_Transform();

	return TexProjectClass::Compute_Ortho_Projection(box,tm,lightdir,nearz,farz);
}


/***********************************************************************************************
 * PhysTexProjectClass::Compute_Texture -- Render the given Phys object to texture             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool PhysTexProjectClass::Compute_Texture(PhysClass * obj,bool additive_projection)
{
	/*
	** Grab the shadow render context and the render object
	*/
	RenderObjClass * model = obj->Peek_Model();
	if (model == NULL) {
		return false;
	}

	int tex_size = Get_Texture_Size();
	SpecialRenderInfoClass * context = PhysicsSceneClass::Get_Instance()->Get_Shadow_Render_Context(tex_size,tex_size);
	if (context == NULL) {
		return false;
	}

	/*
	** Set up the render context
	*/
	bool pushed_mtls = false;
	LightEnvironmentClass lenv;
	if (!additive_projection) {
		lenv.Reset(Vector3(0,0,0),Vector3(0,0,0));		// light env that makes everything black

		MaterialPassClass * shadow_mtl = PhysicsSceneClass::Get_Instance()->Get_Shadow_Material_Pass();
		if (shadow_mtl) {
			pushed_mtls = true;
			context->Push_Material_Pass(shadow_mtl);
			context->Push_Override_Flags((RenderInfoClass::RINFO_OVERRIDE_FLAGS)(RenderInfoClass::RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY | RenderInfoClass::RINFO_OVERRIDE_SHADOW_RENDERING));
			REF_PTR_RELEASE(shadow_mtl);
		}

	} else {
		lenv.Reset(Vector3(0,0,0),Vector3(1,1,1));		// light env of pure ambient
	}
	context->light_environment = &lenv;

	/*
	** Pass on to our parent class function
	*/
	bool success = TexProjectClass::Compute_Texture(model,context);

	/*
	** Cleanup
	*/
	if (pushed_mtls) {
		context->Pop_Override_Flags();
		context->Pop_Material_Pass();
	}

	return success;
}
