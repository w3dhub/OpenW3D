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
 *                     $Archive:: /Commando/Code/wwphys/dyntexproject.cpp                     $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/02/00 6:30p                                               $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "dyntexproject.h"
#include "phys.h"


DynTexProjectClass::DynTexProjectClass(PhysClass * shadow_generator) : 
	ProjectionObject(shadow_generator),
	LightSourceID(0xFFFFFFFF),
	LightVector(0.0f,0.0f,-1.0f)
{
	Set_Flag(VOLATILE,true);
}

DynTexProjectClass::~DynTexProjectClass(void)
{
}

void DynTexProjectClass::Pre_Render_Update(const Matrix3D & camera)
{
	TexProjectClass::Pre_Render_Update(camera);

	if (Get_Flag(TEXTURE_DIRTY)) {
		Compute_Texture(ProjectionObject);
	}
}

void DynTexProjectClass::Set_Projection_Object(PhysClass * obj)
{
	ProjectionObject = obj;
}

PhysClass * DynTexProjectClass::Peek_Projection_Object(void) const
{
	return ProjectionObject;
}

void DynTexProjectClass::Set_Light_Source_ID(uintptr_t id)
{
	LightSourceID = id;
}

uintptr_t DynTexProjectClass::Get_Light_Source_ID(void)
{
	return LightSourceID;
}

void DynTexProjectClass::Set_Light_Vector(const Vector3 & vector)
{
	LightVector = vector;
}

void DynTexProjectClass::Get_Light_Vector(Vector3 * set_vector)
{
	WWASSERT(set_vector != NULL);
	*set_vector = LightVector;
}

void DynTexProjectClass::Enable_Perspective(bool onoff)
{
	Set_Flag(PERSPECTIVE,onoff);
}

bool DynTexProjectClass::Is_Perspective_Enabled(void)
{
	return Get_Flag(PERSPECTIVE);
}

void DynTexProjectClass::Set_Texture_Dirty(bool onoff)
{
	Set_Flag(TEXTURE_DIRTY,onoff);
}

bool DynTexProjectClass::Is_Texture_Dirty(void)
{
	return Get_Flag(TEXTURE_DIRTY);
}

void DynTexProjectClass::Update_Projection(const AABoxClass & objbox,const Matrix3D & objtm,float znear,float zfar)
{
	if (Get_Flag(PERSPECTIVE)) {
		TexProjectClass::Compute_Perspective_Projection(objbox,objtm,LightVector,znear,zfar);
	} else {
		TexProjectClass::Compute_Ortho_Projection(objbox,objtm,LightVector,znear,zfar);
	}
}
