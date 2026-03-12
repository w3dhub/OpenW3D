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

/* $Header: /Commando/Code/wwphys/physcon.cpp 13    9/20/01 5:12p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/physcon.cpp                           $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 9/19/01 7:56p                                               $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "physcon.h"
#include "chunkio.h"
#include "wwdebug.h"
#include "w3d_file.h"


/*
** Static instance of PhysicsConstants so that the constructor gets called and all arrays are filled
*/
static PhysicsConstants _PhysicsConstantsObject;


/*
** Physics Constants - see the header file for explanation
*/

Vector3		PhysicsConstants::GravityAcceleration(0.0f,0.0f,-9.8f);

float			PhysicsConstants::LinearDamping = 0.01f;

float			PhysicsConstants::AngularDamping = 0.05f;

float			PhysicsConstants::RestingContactVelocity = 0.1f;

float			PhysicsConstants::MinFrictionVelocity = 0.05f;

float			PhysicsConstants::DefaultContactFriction = 0.5f;

float			PhysicsConstants::DefaultContactDrag = 0.0f;

int			PhysicsConstants::SurfaceTypeOverride = -1;
float			PhysicsConstants::OverrideDrag = 0.0f;
float			PhysicsConstants::OverrideFriction = 0.0f;

/*
** these are all constants computed from the above constants
*/
float			PhysicsConstants::MinFrictionVelocity2 = (PhysicsConstants::MinFrictionVelocity * PhysicsConstants::MinFrictionVelocity);

/*
** These are constants which are not exposed in the header file
** Note that SURFACE_TYPE_MAX comes from w3d_file.h
*/
float			ContactFrictionCoefficient[PhysicsConstants::DYNAMIC_OBJ_TYPE_MAX][SURFACE_TYPE_MAX];
float			ContactDragCoefficient[PhysicsConstants::DYNAMIC_OBJ_TYPE_MAX][SURFACE_TYPE_MAX];

float test[2][2] = { 1,2,3,4 };




/*
** PhysicsConstants Member functions
*/
void PhysicsConstants::Init(void)
{
	for (int obj_type=0; obj_type < DYNAMIC_OBJ_TYPE_MAX; obj_type++) {
		for (int surf_type=0; surf_type < SURFACE_TYPE_MAX; surf_type++) {
			ContactFrictionCoefficient[obj_type][surf_type] = DefaultContactFriction;
			ContactDragCoefficient[obj_type][surf_type] = DefaultContactDrag;
		}
	}
}

void PhysicsConstants::Set_Contact_Friction_Coefficient(int obj_type,int surface_type,float friction)
{
	if ((obj_type < 0) || (obj_type > DYNAMIC_OBJ_TYPE_MAX)) {
		WWDEBUG_SAY(("PhysicsConstants -> Out of range dynamic object type!\n"));
		return;
	}
	if ((surface_type < 0) || (surface_type > SURFACE_TYPE_MAX)) {
		WWDEBUG_SAY(("PhysicsConstants -> Out of range surface type!\n"));
		return;
	}
	ContactFrictionCoefficient[obj_type][surface_type] = friction;
}

float PhysicsConstants::Get_Contact_Friction_Coefficient(int obj_type,int surface_type)
{
	if (SurfaceTypeOverride != -1) surface_type = SurfaceTypeOverride;
	if (OverrideFriction != 0.0f) return OverrideFriction;

	if ((obj_type < 0) || (obj_type > DYNAMIC_OBJ_TYPE_MAX)) {
		WWDEBUG_SAY(("PhysicsConstants -> Out of range dynamic object type!\n"));
		return ContactFrictionCoefficient[0][0];
	}
	if ((surface_type < 0) || (surface_type > SURFACE_TYPE_MAX)) {
		WWDEBUG_SAY(("PhysicsConstants -> Out of range surface type!\n"));
		return ContactFrictionCoefficient[0][0];
	}
	return ContactFrictionCoefficient[obj_type][surface_type];
}

void	PhysicsConstants::Set_Contact_Drag_Coefficient(int obj_type,int surface_type,float drag)
{
	if ((obj_type < 0) || (obj_type > DYNAMIC_OBJ_TYPE_MAX)) {
		WWDEBUG_SAY(("PhysicsConstants -> Out of range dynamic object type!\n"));
		return;
	}
	if ((surface_type < 0) || (surface_type > SURFACE_TYPE_MAX)) {
		WWDEBUG_SAY(("PhysicsConstants -> Out of range surface type!\n"));
		return;
	}
	ContactDragCoefficient[obj_type][surface_type] = drag;
}

float	PhysicsConstants::Get_Contact_Drag_Coefficient(int obj_type,int surface_type)
{
	if (SurfaceTypeOverride != -1) surface_type = SurfaceTypeOverride;
	if (OverrideDrag != 0.0f) return OverrideDrag;

	if ((obj_type < 0) || (obj_type > DYNAMIC_OBJ_TYPE_MAX)) {
		WWDEBUG_SAY(("PhysicsConstants -> Out of range dynamic object type!\n"));
		return ContactDragCoefficient[0][0];
	}
	if ((surface_type < 0) || (surface_type > SURFACE_TYPE_MAX)) {
		WWDEBUG_SAY(("PhysicsConstants -> Out of range surface type!\n"));
		return ContactDragCoefficient[0][0];
	}
	return ContactDragCoefficient[obj_type][surface_type];
}

void PhysicsConstants::Set_Override_Surface_Type(int type)
{
	if ((type <0) || (type > SURFACE_TYPE_MAX)) {
		SurfaceTypeOverride = -1;
	} else {
		SurfaceTypeOverride = type;
	}
}

void PhysicsConstants::Set_Override_Surface_Friction(float friction)
{
	OverrideFriction = WWMath::Clamp(friction);
}

void PhysicsConstants::Set_Override_Surface_Drag(float drag)
{
	OverrideDrag = drag;
}

/*
** Save-Load Support
*/
enum
{
	PHYSCONSTANTS_CHUNK_VARIABLES					= 0x00044005,

	PHYSCONSTANT_GRAVITYACCELERATION				= 0x00,
	PHYSCONSTANT_LINEARDAMPING,
	PHYSCONSTANT_ANGULARDAMPING,
	PHYSCONSTANT_RESTINGCONTACTVELOCITY,
	PHYSCONSTANT_MINFRICTIONVELOCITY,
	PHYSCONSTANT_DEFAULTCONTACTFRICTION,
	PHYSCONSTANT_MINFRICTIONVELOCITY2
};


void PhysicsConstants::Save(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(PHYSCONSTANTS_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,PHYSCONSTANT_GRAVITYACCELERATION,GravityAcceleration);
	WRITE_MICRO_CHUNK(csave,PHYSCONSTANT_LINEARDAMPING,LinearDamping);
	WRITE_MICRO_CHUNK(csave,PHYSCONSTANT_ANGULARDAMPING,AngularDamping);
	WRITE_MICRO_CHUNK(csave,PHYSCONSTANT_RESTINGCONTACTVELOCITY,RestingContactVelocity);
	WRITE_MICRO_CHUNK(csave,PHYSCONSTANT_MINFRICTIONVELOCITY,MinFrictionVelocity);
	WRITE_MICRO_CHUNK(csave,PHYSCONSTANT_DEFAULTCONTACTFRICTION,DefaultContactFriction);
	WRITE_MICRO_CHUNK(csave,PHYSCONSTANT_MINFRICTIONVELOCITY2,MinFrictionVelocity2);
	csave.End_Chunk();
}

void PhysicsConstants::Load(ChunkLoadClass & cload)
{

	while (cload.Open_Chunk()) {

		switch(cload.Cur_Chunk_ID())
		{
			case PHYSCONSTANTS_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,PHYSCONSTANT_GRAVITYACCELERATION,GravityAcceleration);
						READ_MICRO_CHUNK(cload,PHYSCONSTANT_LINEARDAMPING,LinearDamping);
						READ_MICRO_CHUNK(cload,PHYSCONSTANT_ANGULARDAMPING,AngularDamping);
						READ_MICRO_CHUNK(cload,PHYSCONSTANT_RESTINGCONTACTVELOCITY,RestingContactVelocity);
						READ_MICRO_CHUNK(cload,PHYSCONSTANT_MINFRICTIONVELOCITY,MinFrictionVelocity);
						READ_MICRO_CHUNK(cload,PHYSCONSTANT_DEFAULTCONTACTFRICTION,DefaultContactFriction);
						READ_MICRO_CHUNK(cload,PHYSCONSTANT_MINFRICTIONVELOCITY2,MinFrictionVelocity2);
					}
					cload.Close_Micro_Chunk();
				}
				break;


			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
		}

		cload.Close_Chunk();
	}
}

