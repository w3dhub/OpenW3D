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

/* $Header: /Commando/Code/wwphys/physcon.h 11    9/20/01 5:12p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/wwphys/physcon.h                             $* 
 *                                                                                             * 
 *                       Author:: Greg_h                                                       * 
 *                                                                                             * 
 *                     $Modtime:: 9/19/01 7:49p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 11                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PHYSCON_H
#define PHYSCON_H

#include "always.h"
#include "vector3.h"


class ChunkLoadClass;
class ChunkSaveClass;

/**
** PhysicsConstants
** This is just a collection of global variables used by the physics simulation
*/
class PhysicsConstants
{

public:

	/*
	** One-Time initialization.  This is called from WWPhys::Init automatically.  Some of the
	** constants are filled in with default values in this function.
	*/
	static void		Init(void);

	/*
	** In this engine friction usually occurs between a moving object and 
	** polygons in the static terrain.
	*/
	enum 
	{
		DYNAMIC_OBJ_TYPE_TIRE			= 0,			// (tires on humvees, etc)
		DYNAMIC_OBJ_TYPE_TRACK,							// (tracks on tanks, APCs, etc)
		DYNAMIC_OBJ_TYPE_GENERIC,						// (generic dynamic objects, may need to expand this one)
		
		DYNAMIC_OBJ_TYPE_MAX,
	};

	/*
	** Friction coefficients for various objects on various surface types
	*/
	static void		Set_Contact_Friction_Coefficient(int obj_type,int surface_type,float friction);
	static float	Get_Contact_Friction_Coefficient(int obj_type,int surface_type);
	static void		Set_Contact_Drag_Coefficient(int obj_type,int surface_type,float drag);
	static float	Get_Contact_Drag_Coefficient(int obj_type,int surface_type);

	static void		Set_Override_Surface_Type(int type);
	static void		Set_Override_Surface_Friction(float friction);
	static void		Set_Override_Surface_Drag(float drag);

	/*
	** Save/Load support.
	*/
	static void		Save(ChunkSaveClass & csave);
	static void		Load(ChunkLoadClass & cload);

public:

	/*
	** Accleration of gravity
	*/
	static Vector3	GravityAcceleration;

	/*
	** Linear Damping factor.  
	*/
	static float	LinearDamping;
	
	/*
	** Angular Damping factor.  
	*/
	static float	AngularDamping;
	
	/*
	** epsilon for detecting resting contact.  If the
	** velocity is within this epsilon of zero, a
	** point is considered to be in resting contact.
	*/
	static float	RestingContactVelocity;

	/*
	** Square of the minimum velocity a point must
	** have before applying a dynamic friction to it.
	*/
	static float	MinFrictionVelocity;
	static float	MinFrictionVelocity2;

	/*
	** Default values for contact friction and drag
	*/
	static float	DefaultContactFriction;
	static float	DefaultContactDrag;

	/*
	** For debugging you can globally override the surface type
	*/
	static int		SurfaceTypeOverride;
	static float	OverrideFriction;
	static float	OverrideDrag;
};


#endif /*PHYSCON_H*/
