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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/Mover.h                      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/15/01 10:16a                                              $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __MOVER_H
#define __MOVER_H

#include "listtypes.h"
#include "bittype.h"
#include "collisiongroups.h"
#include "coltest.h"
#include "physcoltest.h"
#include "Utils.h"
#include "sceneeditor.h"


// Forward declarations
class NodeClass;
class Quaternion;
class Vector3;
class Matrix3D;
struct CastResultStruct;


///////////////////////////////////////////////////////////////////////
//
//	MoverClass
//
///////////////////////////////////////////////////////////////////////
class MoverClass
{
public:

	////////////////////////////////////////////////////////////////////
	//	Static methods
	////////////////////////////////////////////////////////////////////
	
	//
	//	Node 'casting' functions.
	//	These position a node at the interesction point of a ray cast into the world.
	//
	static void			Position_Node (NodeClass *node);
	static void			Position_Node_Along_Ray (NodeClass *node);
	static Vector3		Position_Node_Along_Ray (NodeClass *node, const Vector3 &start, const Vector3 &end);
	static void			Position_Nodes_Along_Ray (void);
	static void			Position_Nodes_Along_Ray (const Vector3 &start, const Vector3 &end);
	static Vector3		Position_Nodes_Along_Ray (NODE_LIST &list, const Vector3 &tracking_pt, const Vector3 &start, const Vector3 &end);
	
	static Vector3		Calc_New_Position (const Matrix3D &coord_system, const Vector3 &start_point_in_world_coords, LPPOINT mouse_pos = NULL);

	//
	//	Translation functions
	//
	static void			Move_Node (NodeClass *node, const Vector3 &translation);
	static void			Move_Nodes (const Vector3 &translation);
	static Vector3		Move_Nodes (NODE_LIST &list, const Vector3 &tracking_point);
	static void			Simple_Move_Nodes (NODE_LIST &list, const Vector3 &translation);
	
	//
	//	Rotation functions
	//
	static void			Rotate_Nodes (float deltax, float deltay, const Quaternion &rotation);
	static void			Rotate_Nodes (NODE_LIST &list, const Vector3 &center, float deltax, float deltay, const Quaternion &rotation);
	static void			Rotate_Nodes (NODE_LIST &list, const Matrix3D &rotation_matrix, const Vector3 &center);
	static void			Rotate_Nodes_Z (float multiplier);
	static void			Rotate_Nodes_Z (NODE_LIST &list, const Vector3 &center, float multiplier);

	//
	//	Low-level rotate and translate functions on a per-node basis.
	//	These are called by the other functions once the rotatation or translation params are calculated.
	//
	static void			Rotate_Node (NodeClass *node, const Matrix3D &rotation_matrix, const Matrix3D &coord_system);
	static void			Rotate_Node_Freely (NodeClass *node, const Matrix3D &rotation_matrix, const Matrix3D &coord_system);
	static void			Rotate_Node_Z90 (NodeClass *node, const Matrix3D &rotation_matrix, const Matrix3D &coord_system);
	static void			Translate_Node (NodeClass *node, const Vector3 &vector);

	static void			Transform_Node (NodeClass *node, const Matrix3D &transform);
	static void			Transform_Nodes (NodeClass *node, const Matrix3D &transform);
	static void			Transform_Nodes (NODE_LIST &list, NodeClass *node, const Matrix3D &transform);

	//
	//	Ray-Cast helper functions
	//	These are used to simplify ray casting and line-of-sight casting
	//
	static void			Get_LOS_Ray (const POINT &mouse_pos, float length, Vector3 &start, Vector3 &end);
	static void			Get_LOS_Ray (float length, Vector3 &start, Vector3 &end);
	static void			Get_Mouse_Ray (const POINT &mouse_pos, float length, Vector3 &start, Vector3 &end);
	static void			Get_Mouse_Ray (float length, Vector3 &start, Vector3 &end);
	static PhysClass *Cast_Ray (CastResultStruct &result, const Vector3 &start, const Vector3 &end, uint32 group = DEF_COLLISION_GROUP, uint32 type = COLLISION_TYPE_PHYSICAL | COLLISION_TYPE_PROJECTILE | COLLISION_TYPE_6);
	static PhysClass *Cast_Box (CastResultStruct &result, const AABoxClass &box, const Vector3 &move, uint32 group = DEF_COLLISION_GROUP, uint32 type = COLLISION_TYPE_PHYSICAL | COLLISION_TYPE_PROJECTILE | COLLISION_TYPE_6);

protected:
	
	////////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////////
	static bool			Calc_Ray_Intersection_XY (float plane, const Vector3 &start, const Vector3 &end, Vector3 &result);	
	static bool			Calc_Ray_Intersection_XZ (float plane, const Vector3 &start, const Vector3 &end, Vector3 &result);
	static bool			Calc_Ray_Intersection_YZ (float plane, const Vector3 &start, const Vector3 &end, Vector3 &result);	

private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
};


////////////////////////////////////////////////////////////////////
//	Cast_Ray
////////////////////////////////////////////////////////////////////
inline PhysClass *
MoverClass::Cast_Ray
(
	CastResultStruct &	result,
	const Vector3 &		start,
	const Vector3 &		end,
	uint32				group,
	uint32				type
)
{	
	LineSegClass ray (start, end);
	PhysRayCollisionTestClass raytest (ray, &result, group, type);
	::Get_Scene_Editor ()->Cast_Ray (raytest);

	//
	//	Return a pointer to the collided physics object if the
	// cast hit something
	//
	PhysClass *collided_obj = NULL;
	if (result.Fraction < 1.0F) {
		collided_obj = raytest.CollidedPhysObj;
	}

	return collided_obj;
}


////////////////////////////////////////////////////////////////////
//	Cast_Box
////////////////////////////////////////////////////////////////////
inline PhysClass *
MoverClass::Cast_Box
(
	CastResultStruct &	result,
	const AABoxClass &	box,
	const Vector3 &		move,
	uint32					group,
	uint32					type
)
{
	PhysAABoxCollisionTestClass boxtest (box, move, &result, group, type);
	PhysicsSceneClass::Get_Instance()->Cast_AABox (boxtest);

	//
	//	Return a pointer to the collided physics object if the
	// cast hit something
	//
	PhysClass *collided_obj = NULL;
	if (result.Fraction < 1.0F) {
		collided_obj = boxtest.CollidedPhysObj;
	}

	return collided_obj;
}


#endif //__MOVER_H


