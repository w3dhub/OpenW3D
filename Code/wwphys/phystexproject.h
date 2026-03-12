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
 *                     $Archive:: /Commando/Code/wwphys/phystexproject.h                      $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/23/01 7:23p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PHYSTEXPROJECT_H
#define PHYSTEXPROJECT_H

#include "always.h"
#include "texproject.h"

class PhysClass;

/**
** PhysTexProjectClass
** This derived TexProjectClass simply adds interfaces for handling PhysClasses direction.
** It also handles getting the shadow render context from the physics scene.
** TODO: should we "standardize" the shadow render context stuff and move it to WW3D?
*/
class PhysTexProjectClass : public TexProjectClass
{
public:

	PhysTexProjectClass(void);
	virtual ~PhysTexProjectClass(void);

	/*
	** Automatic initialization of a TexProjectClass.
	** First set up your projection parameters with one of the Compute_xxx_Projection functinos,
	** Then call Compute_Texture.
	*/
	bool						Compute_Perspective_Projection(PhysClass * obj,const Vector3 & lightpos,float nearz=-1.0f,float farz=-1.0f);
	bool						Compute_Ortho_Projection(PhysClass * obj,const Vector3 & lightdir,float nearz=-1.0f,float farz=-1.0f);
	bool						Compute_Texture(PhysClass * obj,bool additive_projection = false);


};


#endif

