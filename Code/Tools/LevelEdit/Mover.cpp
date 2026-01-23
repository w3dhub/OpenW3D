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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/Mover.cpp                    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/15/01 10:15a                                              $*
 *                                                                                             *
 *                    $Revision:: 15                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "mover.h"
#include "Utils.h"
#include "cameramgr.h"
#include "leveleditdoc.h"
#include "LevelEditView.h"
#include "node.h"
#include "phys3.h"
#include <algorithm>


///////////////////////////////////////////////////////////////////////////
//
//	Clip_Point
//
///////////////////////////////////////////////////////////////////////////
static inline bool
Clip_Point (Vector3 *point, const AABoxClass &box)
{
	Vector3 temp_point = *point;

	//
	//	Clip the temporary point
	//
	temp_point.X = std::clamp (temp_point.X, box.Center.X - box.Extent.X, box.Center.X + box.Extent.X);
	temp_point.Y = std::clamp (temp_point.Y, box.Center.Y - box.Extent.Y, box.Center.Y + box.Extent.Y);
	temp_point.Z = std::clamp (temp_point.Z, box.Center.Z - box.Extent.Z, box.Center.Z + box.Extent.Z);

	//
	//	Did the clip change the point?
	//
	bool retval = (point->X != temp_point.X);
	retval		|= (point->Y != temp_point.Y);
	retval		|= (point->Z != temp_point.Z);
	
	//
	//	Pass the new point back to the caller
	//
	(*point) = temp_point;
	
	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Mouse_Ray
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Get_Mouse_Ray
(
	const POINT &	mouse_pos, 
	float				length,
	Vector3 &		start,
	Vector3 &		end
)
{
	// The start of the ray is simply the camera's position
	start = ::Get_Camera_Mgr ()->Get_Camera ()->Get_Transform ().Get_Translation ();

	//
	// Ensure the 'point' is correct for this mode (fullscreen/windowed)
	//
	float xpos = mouse_pos.x;
	float ypos = mouse_pos.y;
	::Constrain_Point_To_Aspect_Ratio (xpos, ypos);
	
	// The 'end' of the ray is the world coordinates of the supplied point
	::Get_Camera_Mgr ()->Get_Camera ()->Device_To_World_Space (Vector2 (xpos, ypos), &end);

	//
	// Recalculate the length of the ray
	//
	end = start + ((end - start) * length);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Mouse_Ray
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Get_Mouse_Ray
(
	float		length,
	Vector3 &start,
	Vector3 &end
)
{
	//
	// Ensure the 'point' is correct for this mode (fullscreen/windowed)
	//
	CPoint point;
	::GetCursorPos (&point);
	::Get_Main_View ()->ScreenToClient (&point);

	Get_Mouse_Ray (point, length, start, end);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_LOS_Ray
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Get_LOS_Ray
(
	const POINT &	/* mouse_pos */, 
	float				length,
	Vector3 &		start,
	Vector3 &		end
)
{
	// Get the transformation matrix for the camera
	Matrix3D camera_tm = ::Get_Camera_Mgr ()->Get_Camera ()->Get_Transform ();
	
	//
	//	The start and end's of the ray are the camera's position
	// and 'length' down the camera's -z axis.
	//
	start = camera_tm.Get_Translation ();
	end	= camera_tm * Vector3 (0, 0, -length);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_LOS_Ray
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Get_LOS_Ray
(
	float		length,
	Vector3 &start,
	Vector3 &end
)
{
	//
	// Ensure the 'point' is correct for this mode (fullscreen/windowed)
	//
	CPoint point;
	::GetCursorPos (&point);
	::Get_Main_View ()->ScreenToClient (&point);

	Get_LOS_Ray (point, length, start, end);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Position_Node
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Position_Node (NodeClass *node)
{	
	float distance			= 1000.0F;

	//
	//	Determine what ray to cast the node along
	//
	Vector3 ray_start;
	Vector3 ray_end;
	Get_LOS_Ray (distance, ray_start, ray_end);

	Position_Node_Along_Ray (node, ray_start, ray_end);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Position_Node_Along_Ray
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Position_Node_Along_Ray (NodeClass *node)
{	
	//
	//	Determine what ray to cast the node along
	//
	Vector3 ray_start;
	Vector3 ray_end;
	Get_Mouse_Ray (350.00F, ray_start, ray_end);

	Position_Node_Along_Ray (node, ray_start, ray_end);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Position_Nodes_Along_Ray
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Position_Nodes_Along_Ray (void)
{
	//
	//	Determine what ray to cast the node along
	//
	Vector3 ray_start;
	Vector3 ray_end;
	Get_Mouse_Ray (350.00F, ray_start, ray_end);

	Position_Nodes_Along_Ray (ray_start, ray_end);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Position_Nodes_Along_Ray
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Position_Nodes_Along_Ray (const Vector3 &start, const Vector3 &end)
{
	NODE_LIST list;
	Vector3 center = ::Get_Scene_Editor ()->Build_Node_List (list);

	if (list.Count () == 1) {
		Position_Node_Along_Ray (list[0], start, end);
	} else {
		Position_Nodes_Along_Ray (list, center, start, end);
	}
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Position_Nodes_Along_Ray
//
///////////////////////////////////////////////////////////////////////
Vector3
MoverClass::Position_Nodes_Along_Ray
(
	NODE_LIST &		list,
	const	Vector3 &tracking_pt,
	const Vector3 &start,
	const Vector3 &end
)
{
	if (list.Count () == 1) {
		return Position_Node_Along_Ray (list[0], start, end);
	}

	Vector3 position (tracking_pt);

	//
	//	Turn off collision detection
	//
	int index;
	for (index = 0; index < list.Count (); index ++) {
		PhysClass *phys_obj = list[index]->Peek_Physics_Obj ();
		if (phys_obj != NULL) {			
			phys_obj->Inc_Ignore_Counter();
		}
	}	

	//
	// Cast the ray into the world so we can stop the objects at whatever it hits
	//
	CastResultStruct res;							
	if (Cast_Ray (res, start, end, GAME_COLLISION_GROUP) != NULL) {
		position = start + (end - start) * res.Fraction;
	}

	// Calculate a delta to move each object
	Vector3 delta = position - tracking_pt;

	//
	//	Re-position the nodes
	//
	for (index = 0; index < list.Count (); index ++) {
		NodeClass *node = list[index];

		PhysClass *phys_obj = node->Peek_Physics_Obj ();
		if (phys_obj != NULL) {			
			
			//
			//	Move the node and turn collision back on
			//
			node->Translate (delta);
			phys_obj->Dec_Ignore_Counter();
		}
	}

	::Set_Modified ();
	return position;
}


///////////////////////////////////////////////////////////////////////
//
//	Position_Node_Along_Ray
//
///////////////////////////////////////////////////////////////////////
Vector3
MoverClass::Position_Node_Along_Ray
(
	NodeClass *		node,
	const Vector3 &start,
	const Vector3 &end
)
{
	Vector3 position (0, 0, 0);

	PhysClass *phys_obj = node->Peek_Physics_Obj ();
	if (phys_obj != NULL) {

		position = node->Get_Transform ().Get_Translation ();

		// Don't include this node in the collision test
		phys_obj->Inc_Ignore_Counter();

		//
		// Cast the ray into the world so we can stop the object at whatever it hits
		//
		CastResultStruct res;							
		if (Cast_Ray (res, start, end, GAME_COLLISION_GROUP) != NULL) {

			//
			// Determine the new position based on whether or not the
			// ray-cast hit anything.
			//			
			position = start + (end - start) * res.Fraction;
			
			//
			//	Get the collision box for this render obj
			//
			AABoxClass box;
			RenderObjClass *model = phys_obj->Peek_Model ();		
			RenderObjClass *world_box = model->Get_Sub_Object_By_Name ("WORLDBOX");
			if (world_box != NULL) {
				world_box->Get_Obj_Space_Bounding_Box (box);
				MEMBER_RELEASE (world_box);
			} else {
				model->Get_Obj_Space_Bounding_Box (box);
			}
			box.Extent.Z = WWMATH_EPSILON;
			
			//
			//	Move the object so its not embedded in the surface
			//
			Vector3 temp_pt = box.Center - res.Normal;
			::Clip_Point (&temp_pt, box);
			float len	= (temp_pt - box.Center).Length () + WWMATH_EPSILON;
			position		+= (res.Normal * len);

			//
			//	Move the node
			//
			node->Translate (position - node->Get_Transform ().Get_Translation ());
		}

		phys_obj->Dec_Ignore_Counter();
		::Set_Modified ();
	}

	return position;
}


///////////////////////////////////////////////////////////////////////
//
//	Calc_Ray_Intersection_XY
//
///////////////////////////////////////////////////////////////////////
bool
MoverClass::Calc_Ray_Intersection_XY
(
	float				plane,
	const Vector3 &ray_start,
	const Vector3 &ray_end,
	Vector3 &		result
)
{
	// Assume no intersection
	bool intersected = false;

	if ((ray_end.Z - ray_start.Z) != 0.00F) {

		// Calculate the fraction of the distance along the ray where the
		// Z value of the ray is the same as the Z value of the plane.
		// This simulates an intersection of the ray with the x-y plane at depth 'z'.
		double fraction = double(plane - ray_start.Z) / double(ray_end.Z - ray_start.Z);

		// If the fraction is between 0 and 1, then the ray intersects the plane
		if ((fraction >= 0) && (fraction <= 1.0F)) {

			// Now calcuate the intersection point based on this fraction.
			// To do this we use the parameterized form of a line equation:
			// P = P1 + (P2 - P1) * t
			// (Where t is a percentage between 0 and 1)
			result = ray_start + ((ray_end - ray_start) * fraction);

			// Success!
			intersected = true;
		}
	}

	// Return the true/false result code
	return intersected;
}


///////////////////////////////////////////////////////////////////////
//
//	Calc_Ray_Intersection_XZ
//
///////////////////////////////////////////////////////////////////////
bool
MoverClass::Calc_Ray_Intersection_XZ
(
	float				plane,
	const Vector3 &ray_start,
	const Vector3 &ray_end,
	Vector3 &		result
)
{
	// Assume no intersection
	bool intersected = false;

	if ((ray_end.Y - ray_start.Y) != 0.00F) {

		// Calculate the fraction of the distance along the ray where the
		// Y value of the ray is the same as the Y value of the plane.
		// This simulates an intersection of the ray with the x-z plane at depth 'y'.
		double fraction = double(plane - ray_start.Y) / double(ray_end.Y - ray_start.Y);

		// If the fraction is between 0 and 1, then the ray intersects the plane
		if ((fraction >= 0) && (fraction <= 1.0F)) {

			// Now calcuate the intersection point based on this fraction.
			// To do this we use the parameterized form of a line equation:
			// P = P1 + (P2 - P1) * t
			// (Where t is a percentage between 0 and 1)
			result = ray_start + ((ray_end - ray_start) * fraction);

			// Success!
			intersected = true;
		}
	}

	// Return the true/false result code
	return intersected;
}


///////////////////////////////////////////////////////////////////////
//
//	Calc_Ray_Intersection_YZ
//
///////////////////////////////////////////////////////////////////////
bool
MoverClass::Calc_Ray_Intersection_YZ
(
	float				plane,
	const Vector3 &ray_start,
	const Vector3 &ray_end,
	Vector3 &		result
)
{
	// Assume no intersection
	bool intersected = false;

	if ((ray_end.X - ray_start.X) != 0.00F) {

		// Calculate the fraction of the distance along the ray where the
		// X value of the ray is the same as the X value of the plane.
		// This simulates an intersection of the ray with the y-z plane at depth 'x'.
		double fraction = double(plane - ray_start.X) / double(ray_end.X - ray_start.X);

		// If the fraction is between 0 and 1, then the ray intersects the plane
		if ((fraction >= 0) && (fraction <= 1.0F)) {

			// Now calcuate the intersection point based on this fraction.
			// To do this we use the parameterized form of a line equation:
			// P = P1 + (P2 - P1) * t
			// (Where t is a percentage between 0 and 1)
			result = ray_start + ((ray_end - ray_start) * fraction);

			// Success!
			intersected = true;
		}
	}

	// Return the true/false result code
	return intersected;
}


///////////////////////////////////////////////////////////////////////
//
//	Transform_Node
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Transform_Node
(
	NodeClass *			node,
	const Matrix3D &	transform
)
{
	//
	// Move this node by the translation delta
	//	
	Matrix3D curr_transform = node->Get_Transform ();
	Move_Node (node, transform.Get_Translation () - curr_transform.Get_Translation ());
	
	//
	// Determine a rotation matrix that is relative to the node's current transform
	//
	Matrix3D relative_transform;
	curr_transform.Get_Orthogonal_Inverse (relative_transform);
	Matrix3D rotation_matrix = relative_transform * transform;
	rotation_matrix.Set_Translation (Vector3 (0, 0, 0));
	
	//
	// Rotate the node by this matrix, centered about the new position.
	//
	Matrix3D coord_system (transform.Get_Translation ());
	Rotate_Node (node, rotation_matrix, coord_system);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Transform_Nodes
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Transform_Nodes
(
	NodeClass *			node,
	const Matrix3D &	transform
)
{
	NODE_LIST list;
	::Get_Scene_Editor ()->Build_Node_List (list);
	Transform_Nodes (list, node, transform);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Transform_Nodes
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Transform_Nodes
(
	NODE_LIST &			list,
	NodeClass *			node,
	const Matrix3D &	transform
)
{
	Matrix3D curr_transform = node->Get_Transform ();

	//
	// Move this list of nodes by the translation delta
	//
	Move_Nodes (list, transform.Get_Translation () - curr_transform.Get_Translation ());

	//
	// Determine a rotation matrix that is relative to the node's current transform
	//
	Matrix3D relative_transform;
	curr_transform.Get_Orthogonal_Inverse (relative_transform);
	Matrix3D rotation_matrix = relative_transform * transform;
	rotation_matrix.Set_Translation (Vector3 (0, 0, 0));
	
	//
	// Rotate the list of nodes by this matrix, centered about the new position.
	//
	Rotate_Nodes (list, rotation_matrix, transform.Get_Translation ());
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Move_Node
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Move_Node (NodeClass *node, const Vector3 &translation)
{
	node->Translate (translation);
	::Set_Modified ();
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Move_Nodes
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Simple_Move_Nodes (NODE_LIST &list, const Vector3 &translation)
{
	for (int index = 0; index < list.Count (); index ++) {
		NodeClass *node = list[index];
		node->Translate (translation);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Move_Nodes
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Move_Nodes (const Vector3 &translation)
{
	NODE_LIST list;
	::Get_Scene_Editor ()->Build_Node_List (list);

	Simple_Move_Nodes (list, translation);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Move_Nodes
//
///////////////////////////////////////////////////////////////////////
Vector3
MoverClass::Move_Nodes (NODE_LIST &list, const Vector3 &tracking_point)
{
	//
	// Calculate the new position for the group
	//
	Matrix3D coord_system (1);		
	Vector3 new_pos = Calc_New_Position (coord_system, tracking_point);

	//
	// Loop through all the nodes and move them by the calculated delta
	//
	for (int index = 0; index < list.Count (); index ++) {
		NodeClass *node = list[index];
		
		//
		// Move the node
		//
		if (node != NULL) {
			node->Translate (new_pos - tracking_point);
		}
	}

	Set_Modified ();
	return new_pos;
}


///////////////////////////////////////////////////////////////////////
//
//	Calc_New_Position
//
///////////////////////////////////////////////////////////////////////
Vector3
MoverClass::Calc_New_Position
(
	const Matrix3D &	coord_system,
	const Vector3 &	start_in_world_coords,
	LPPOINT				mouse_pos
)
{
	Vector3 return_point	= start_in_world_coords;
	Vector3 start_point	= start_in_world_coords;

	//
	// The start of the ray is simply the camera's position
	//
	Vector3 ray_start = ::Get_Camera_Mgr ()->Get_Camera ()->Get_Transform ().Get_Translation ();

	//
	// Ensure the 'mouse-point' is correct for this mode (fullscreen/windowed)
	//
	CPoint point;

	if (mouse_pos == NULL) {		
		::GetCursorPos (&point);
		::Get_Main_View()->ScreenToClient (&point);
	} else {
		point = (*mouse_pos);
	}
	
	float xpos = point.x;
	float ypos = point.y;
	::Constrain_Point_To_Aspect_Ratio (xpos, ypos);
	
	//
	// The 'end' of the ray is the world coordinates of the mouse point
	//
	Vector3 ray_end;				
	::Get_Camera_Mgr ()->Get_Camera ()->Device_To_World_Space (Vector2 (xpos, ypos), &ray_end);

	//
	// However, now we must transform all our coords into the coordinate system we were passed.
	//
	Matrix3D coord_system_inv;
	coord_system.Get_Orthogonal_Inverse (coord_system_inv);
	ray_start			= coord_system_inv * ray_start;
	ray_end				= coord_system_inv * ray_end;
	start_point			= coord_system_inv * start_point;
	return_point		= coord_system_inv * return_point;
	
	//
	// Move the ray's endpoint along the line 1000 meters
	//
	ray_end = ray_start + ((ray_end - ray_start) * 1000.00F);

	//
	// Should we restrict movement to the z-axis?
	//
	if ((::GetKeyState (VK_SHIFT) < 0) ||
	    (::Get_Current_Document ()->Get_Axis_Restriction () == CLevelEditDoc::RESTRICT_Z)) {

		// Determine which plane (y-z or x-z) to base the movement on
		float deltax = ::fabs (ray_end.X - ray_start.X);
		float deltay = ::fabs (ray_end.Y - ray_start.Y);
		
		// Determine where the ray intersects this plane
		if (deltax > deltay) {

			// Determine where the ray intersects the YZ plane
			Vector3 result;
			if (Calc_Ray_Intersection_YZ (start_point.X, ray_start, ray_end, result)) {
				return_point.Z = result.Z;
			}
			
		} else {

			// Determine where the ray intersects the XZ plane
			Vector3 result;
			if (Calc_Ray_Intersection_XZ (start_point.Y, ray_start, ray_end, result)) {
				return_point.Z = result.Z;
			}
		}	

	} else {
		
		// Determine where the ray intersects the XY plane
		Vector3 result;
		if (Calc_Ray_Intersection_XY (start_point.Z, ray_start, ray_end, result)) {
			
			//
			// Now apply the axis restrictions
			//
			if (::Get_Current_Document ()->Get_Axis_Restriction () == CLevelEditDoc::RESTRICT_X) {
				return_point.X = result.X;
			} else if (::Get_Current_Document ()->Get_Axis_Restriction () == CLevelEditDoc::RESTRICT_Y) {
				return_point.Y = result.Y;
			} else {
				return_point = result;
			}
		}
	}	
	
	//
	// Now convert the return value to world coords
	//
	return_point = coord_system * return_point;

	// Return the new point
	return return_point;
}


///////////////////////////////////////////////////////////////////////
//
//	Rotate_Nodes
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Rotate_Nodes
(
	float				deltax,
	float				deltay,
	const Quaternion &	rotation
)
{
	NODE_LIST list;
	Vector3 center = ::Get_Scene_Editor ()->Build_Node_List (list);

	Rotate_Nodes (list, center, deltax, deltay, rotation);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Rotate_Nodes
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Rotate_Nodes
(
	NODE_LIST &			list,
	const Vector3 &		center,
	float				deltax,
	float				deltay,
	const Quaternion &	rotation
)
{
	Matrix3D rotation_matrix (1);

	//
	// Are we restricting movement to a specific axis?
	//
	switch (::Get_Current_Document ()->Get_Axis_Restriction ()) {
	
		case CLevelEditDoc::RESTRICT_X:
			rotation_matrix.Rotate_X (deltax * 3.0F);
			break;

		case CLevelEditDoc::RESTRICT_Y:
			rotation_matrix.Rotate_Y (deltay * 3.0F);
			break;

		case CLevelEditDoc::RESTRICT_Z:
			rotation_matrix.Rotate_Z (deltay * 3.0F);
			break;

		case CLevelEditDoc::RESTRICT_NONE:
			rotation_matrix.Set_Rotation (rotation);
			break;
	}

	Rotate_Nodes (list, rotation_matrix, center);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Rotate_Nodes
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Rotate_Nodes
(
	NODE_LIST &			list,
	const Matrix3D &	rotation_matrix,
	const Vector3 &		center
)
{
	//
	//	Setup the rotation
	//
	Matrix3D coord_system (1);
	coord_system.Set_Translation (center);

	//
	// Determine if we should restrict rotation or not
	//
	bool restrict_rotation = false;
	int index;
	for (index = 0; index < list.Count (); index ++) {
		restrict_rotation |= list[index]->Is_Rotation_Restricted ();
	}

	//
	// Loop through all the nodes and rotate them
	//
	for (index = 0; index < list.Count (); index ++) {
		NodeClass *node = list[index];

		//
		// Rotate the node
		//
		if (restrict_rotation == false) {
			Rotate_Node_Freely (node, rotation_matrix, coord_system);
		} else {
			Rotate_Node_Z90 (node, rotation_matrix, coord_system);
		}
	}

	Set_Modified ();
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Rotate_Nodes_Z
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Rotate_Nodes_Z (float multiplier)
{
	NODE_LIST list;
	Vector3 center = ::Get_Scene_Editor ()->Build_Node_List (list);

	Rotate_Nodes_Z (list, center, multiplier);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Rotate_Nodes_Z
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Rotate_Nodes_Z
(
	NODE_LIST &		list,
	const Vector3 &	center,
	float			multiplier
)
{
	Matrix3D coord_system (1);	
	coord_system.Set_Translation (center);

	//
	// Determine if we should restrict rotation or not
	//
	bool restrict_rotation = false;
	int index;
	for (index = 0; index < list.Count (); index ++) {
		restrict_rotation |= list[index]->Is_Rotation_Restricted ();
	}

	//
	// Loop through all the nodes and rotate them
	//
	for (index = 0; index < list.Count (); index ++) {
		NodeClass *node = list[index];

		//
		// Determine which rotation matrix to use (Z90 lock, or normal)
		//
		Matrix3D rotation_matrix (1);
		if (restrict_rotation == false) {
			rotation_matrix.Rotate_Z (DEG_TO_RAD (5.0F) * multiplier);
		} else if (multiplier > 0.0F) {
			rotation_matrix.Rotate_Z ((float)DEG_TO_RAD (-90));
		} else {
			rotation_matrix.Rotate_Z ((float)DEG_TO_RAD (90));
		}

		// Rotate the node
		node->Rotate (rotation_matrix, coord_system);
	}

	Set_Modified ();		
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Rotate_Node_Z90
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Rotate_Node_Z90
(
	NodeClass *			node,
	const Matrix3D &	rotation_matrix,
	const Matrix3D &	coord_system
)
{
	if (node->Is_Locked () == false) {

		//
		// Get the transformation matrix
		//
		Matrix3D transform		= node->Get_Transform ();
		Vector3 translation		= transform.Get_Translation ();
		bool allow_translation	= bool(coord_system.Get_Translation () != translation);

		Quaternion orientation = node->Get_Orientation ();
		transform.Set_Rotation (orientation);

		//
		// Rotate the transformation matrix in the provided coordinate system
		//
		::Rotate_Matrix (transform, rotation_matrix, coord_system, &transform);

		//
		// Remember the orientation of the new matrix
		//
		orientation = ::Build_Quaternion (transform);
		node->Set_Orientation (orientation);

		double new_z_rot = transform.Get_Z_Rotation ();
		if (new_z_rot < 0) {
			new_z_rot += DEG_TO_RAD (360);
		}

		if (allow_translation) {
			translation = transform.Get_Translation ();
		}

		// Restrict translation if necessary
		transform.Make_Identity ();

		// Based on the z oomponent of the rotation, determine which
		// direction the object should be facing
		if ((new_z_rot > DEG_TO_RAD (45)) && (new_z_rot <= DEG_TO_RAD (135))) {
			transform = Matrix3D::RotateZ90;
		} else if ((new_z_rot > DEG_TO_RAD (135)) && (new_z_rot <= DEG_TO_RAD (225))) {
			transform = Matrix3D::RotateZ180;
		} else if ((new_z_rot > DEG_TO_RAD (225)) && (new_z_rot <= DEG_TO_RAD (315))) {
			transform = Matrix3D::RotateZ270;
		}

		//
		// Translate the matrix as necessary
		//
		transform.Set_Translation (translation);	

		//
		// Pass the new transformation matrix onto the object
		//
		node->Set_Transform (transform);
		node->On_Rotate ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Rotate_Node_Freely
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Rotate_Node_Freely
(
	NodeClass *			node,
	const Matrix3D &	rotation_matrix,
	const Matrix3D &	coord_system
)
{
	if (node->Is_Locked () == false) {

		Matrix3D tm = node->Get_Transform ();

		//
		// Translate this node in the specified coordinate system
		//
		Matrix3D coord_inv;
		Matrix3D coord_to_obj;
		
		coord_system.Get_Orthogonal_Inverse (coord_inv);
		Matrix3D::Multiply (coord_inv, tm, &coord_to_obj);

		Matrix3D::Multiply (coord_system, rotation_matrix, &tm);
		Matrix3D::Multiply (tm, coord_to_obj, &tm);

		if (!tm.Is_Orthogonal ()) {
			tm.Re_Orthogonalize ();
			TRACE ("Matrix wasn't orthogonal.\n");
		}

		//
		// Rotate the object and do post processing
		//
		node->Set_Transform (tm);
		node->On_Rotate ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Rotate_Node
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Rotate_Node
(
	NodeClass *			node,
	const Matrix3D &	rotation_matrix,
	const Matrix3D &	coord_system
)
{
	//
	//	Determine whether the node can be rotated free-form or
	// is restricted to 90 deg rotations about the Z axis.
	//
	if (node->Is_Rotation_Restricted () == false) {
		Rotate_Node_Freely (node, rotation_matrix, coord_system);
	} else {
		Rotate_Node_Z90 (node, rotation_matrix, coord_system);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Translate_Node
//
///////////////////////////////////////////////////////////////////////
void
MoverClass::Translate_Node (NodeClass *node, const Vector3 &vector)
{
	if (node->Is_Locked () == false) {
		
		//
		//	Move the physics object by the requested amount
		//
		Matrix3D tm = node->Get_Transform ();
		tm.Set_Translation (tm.Get_Translation () + vector);
		node->Set_Transform (tm);
		//phys_obj->Set_Position (tm.Get_Translation () + vector);

		/*Phys3Class *phys3_obj = phys_obj->As_Phys3Class ();
		if (phys3_obj != NULL) {
			phys3_obj->Set_Velocity (vector);
		}*/

		// Notify the node
		node->On_Translate ();
	}

	return ;
}

