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
 *                     $Archive:: /Commando/Code/wwphys/dyntexproject.h                       $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/02/00 6:30p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef DYNTEXPROJECT_H
#define DYNTEXPROJECT_H

#include "always.h"
#include "phystexproject.h"

/**
** DynTexProjectClass
** Dynamic projected textures.  This class is simply a projected texture that 
** can keep track of the object it is supposed to be projecting and refresh 
** its texture only when needed.  In addition, it stores the 'ID' of the
** light source which generated it so that we can re-use the same DynTexProjectClass's
** when the same object is being projected by the same light source (see the
** Update_Texture call in MovePhysClass...)
** NOTE: the pointer back to the projected object is not ref-counted because
** that would create a circular reference.
*/
class DynTexProjectClass : public PhysTexProjectClass
{
public:

	DynTexProjectClass(PhysClass * shadow_generator);
	virtual ~DynTexProjectClass(void);

	/*
	** TexProjectClass interface
	*/
	virtual void		Pre_Render_Update(const Matrix3D & camera) override;

	/*
	** DynTexProjectClass interface
	** - stores the object and light source parameters which generate this projector
	** - regenerates the texture in the Pre_Render_Update call
	** In order to properly set up a dynamic projector, be sure you set all of
	** the following: the projection object, the light source ID (pointer cast to an int),
	** the light vector, the perspective flag (indicates whether the vector is a direction
	** or a light position)
	*/
	void					Set_Projection_Object(PhysClass * obj);
	PhysClass *			Peek_Projection_Object(void) const;
	virtual void *		Get_Projection_Object_ID(void) const override			{ return ProjectionObject; }

	void					Set_Light_Source_ID(uint32 id);
	uint32				Get_Light_Source_ID(void);

	void					Set_Light_Vector(const Vector3 & vector);
	void					Get_Light_Vector(Vector3 * set_vector);
	
	void					Enable_Perspective(bool onoff);
	bool					Is_Perspective_Enabled(void);

	void					Set_Texture_Dirty(bool onoff = true);
	bool					Is_Texture_Dirty(void);

	void					Update_Projection(const AABoxClass & objbox,const Matrix3D & objtm,float znear = -1.0f,float zfar = -1.0f);
	
protected:

	PhysClass *			ProjectionObject;			// Object to be projected, not ref-counted!
	uint32				LightSourceID;				// ID of the light source (just its pointer,used for matching)
	Vector3				LightVector;				//	copy of the light vector (postion or direction)
};



#endif