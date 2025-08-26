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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/PathObject.cpp                        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/21/01 2:33p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "PathObject.h"
#include "rendobj.h"
#include "phys.h"
#include "hlod.h"
#include "boxrobj.h"
#include <stdio.h>

//////////////////////////////////////////////////////////////////
//	Local constants
//////////////////////////////////////////////////////////////////
const float DEF_TURN_RADIUS	= 0;
const float DEF_SPEED			= 4.0F;
const int DEF_FLAGS				= PathObjectClass::CAN_USE_EQUIPMENT | PathObjectClass::IS_VEHICLE;


//////////////////////////////////////////////////////////////////
//	Local inlines
//////////////////////////////////////////////////////////////////
void Get_Collision_Box (RenderObjClass *render_obj, OBBoxClass &bounding_box);



//////////////////////////////////////////////////////////////////
//
//	PathObjectClass
//
//////////////////////////////////////////////////////////////////
PathObjectClass::PathObjectClass (void)
	:	m_TurnRadius (DEF_TURN_RADIUS),
		m_MaxSpeed (DEF_SPEED),
		m_CollisionBox (Vector3 (0, 0, 0), Vector3 (0.35F, 0.35F, 0.7F)),
		m_Flags (DEF_FLAGS),
		m_KeyRing (0)
{
	Init_Human ();
	return ;
}


//////////////////////////////////////////////////////////////////
//
//	~PathObjectClass
//
//////////////////////////////////////////////////////////////////
PathObjectClass::~PathObjectClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////
void
PathObjectClass::Initialize (PhysClass &phys_obj)
{
	RenderObjClass *render_obj = phys_obj.Peek_Model ();
	if (render_obj != NULL) {
		
		//
		//	Lookup the render object's collision box
		//
		::Get_Collision_Box (render_obj, m_CollisionBox);		
	}

	//
	//	Everything except for humans rotate their collision boxes
	//
	Set_Flag (PathObjectClass::CAN_BOX_ROTATE, (phys_obj.As_HumanPhysClass () == NULL));

	//
	//	By default we will assume the vehicles steering-wheels are located in the front...
	//
	if (phys_obj.As_HumanPhysClass () == NULL) {
		Set_Wheel_Offset (m_CollisionBox.Extent.X);
	}

	return ;
}


//////////////////////////////////////////////////////////////////
//
//	Init_Human
//
//////////////////////////////////////////////////////////////////
void
PathObjectClass::Init_Human (void)
{
	m_Flags			= CAN_USE_EQUIPMENT;
	m_TurnRadius	= 0;	
	m_WheelOffset	= 0;

	// Use some default numbers for the human...
	m_CollisionBox.Center.Set (0, 0, 0);
	m_CollisionBox.Extent.Set (0.35F, 0.35F, 0.7F);
	m_CollisionBox.Basis.Make_Identity ();

	// (4.75 meters per sec is approx a 6 min mile)
	m_MaxSpeed		= 4.75F;
	return ;
}


//////////////////////////////////////////////////////////////////
//
//	Get_Collision_Box
//
//////////////////////////////////////////////////////////////////
void
Get_Collision_Box (RenderObjClass *render_obj, OBBoxClass &bounding_box)
{
	if (render_obj != NULL) {

		//
		// Try to get the "WorldBox" from the model
		//
		RenderObjClass *world_box = render_obj->Get_Sub_Object_By_Name ("WorldBox");

		// If we didn't finde WorldBox, try to find the LOD named "WorldBox" 
		// The LOD code generates a unique name for the mesh by appending A,B,C, etc to the name.
		// A is the lowest LOD, B is the next, and so on.  Our worldbox is specified in the highest
		// LOD so we have to construct the name by appending 'A'+LodCount to the name... icky
		if ((world_box == NULL) && (render_obj->Class_ID () == RenderObjClass::CLASSID_HLOD)) {
			
			char namebuffer[64];
			sprintf(namebuffer,"WorldBox%c",'A' + ((HLodClass *)render_obj)->Get_Lod_Count() - 1);
			world_box = render_obj->Get_Sub_Object_By_Name (namebuffer);
		}

		if (world_box != NULL && world_box->Class_ID() == RenderObjClass::CLASSID_OBBOX) {
			bounding_box = ((OBBoxRenderObjClass *)world_box)->Get_Box ();
		}

		REF_PTR_RELEASE (world_box);
	}

	return ;
}

