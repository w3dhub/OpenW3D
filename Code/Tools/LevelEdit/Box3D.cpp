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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/Box3D.cpp                    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/28/01 3:11p                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */



//
//
//	Following is a diagram specifying which entries in the m_Verticies array
// correspond to which verticies in the box (in object space).
// I don't know how well this diagram is going to hold up to different editors/font sizes, etc
// so I apologize in advance if its unreadable... ;>
//
//
//			  2 --------------- 3
//				/|				  /|
//			  / |				 / |
//			 /	 |				/  |
//		 6 --------------- 7 |
//			|	 |			  |	|
//			|	 |			  |	|
//			|	1|_________|___| 0
//			|	/			  |  /
//			| /			  | /						
//			|/				  |/
//		 5 --------------- 4
//
//
//
//


#include "StdAfx.h"
#include "box3d.h"
#include "coltest.h"
#include "tri.h"
#include "leveleditdoc.h"
#include "cameramgr.h"
#include "camera.h"
#include <algorithm>


/////////////////////////////////////////////////////////////////////////////////////////
//	Local constants and typedefs
/////////////////////////////////////////////////////////////////////////////////////////
typedef enum
{
	FACE_FRONT = 0,
	FACE_BACK,
	FACE_TOP,
	FACE_BOTTOM,
	FACE_RIGHT,
	FACE_LEFT,
	FACE_COUNT
} FACE_INDEX;

const int FACE_VERTICIES[6][4] = {
	{ 5, 4, 6, 7 }, { 0, 1, 3, 2 },
	{ 3, 2, 7, 6 }, { 4, 5, 0, 1 },
	{ 1, 5, 2, 6 }, { 4, 0, 7, 3 }
};

const float	MIN_SIZE				= 0.01F;

/////////////////////////////////////////////////////////////////////////////////////////
//	Local prototypes
/////////////////////////////////////////////////////////////////////////////////////////
static bool Find_Intersection_Point (const AABoxClass &box, const Vector3 &p0, const Vector3 &p1, float *percent, Vector3 *intersection_point);


/////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Model
//
/////////////////////////////////////////////////////////////////////////////////////////
void
Box3DClass::Create_Model (void)
{
	//
	// Assign a default vertex material for the model
	//	
	VertexMaterialClass *vmat = new VertexMaterialClass ();
	vmat->Set_Lighting (false);
	vmat->Set_Ambient(0,0,0);
	vmat->Set_Diffuse(0,0,0);
	vmat->Set_Specular(0,0,0);
	vmat->Set_Emissive(1,1,1);
	vmat->Set_Opacity(0.5F);
	vmat->Set_Shininess(0.0f);

	//vmat->Set_Opacity (0.5F);
	Set_Vertex_Material (vmat);
	MEMBER_RELEASE (vmat);

	//
	// Use an alpha shader so the box will be transparent
	//
	Set_Shader (ShaderClass::_PresetAlphaSolidShader);
	Enable_Sort ();
	
	float half_width = m_Dimensions.Y / 2.0F;
	float half_height = m_Dimensions.Z / 2.0F;
	float half_depth = m_Dimensions.X / 2.0F;

	// Determine the object space coords of our 8 verticies
	m_Verticies[0].X = -half_depth;
	m_Verticies[0].Y = half_width;
	m_Verticies[0].Z = -half_height;

	m_Verticies[1].X = -half_depth;
	m_Verticies[1].Y = -half_width;
	m_Verticies[1].Z = -half_height;

	m_Verticies[2].X = -half_depth;
	m_Verticies[2].Y = -half_width;
	m_Verticies[2].Z = half_height;

	m_Verticies[3].X = -half_depth;
	m_Verticies[3].Y = half_width;
	m_Verticies[3].Z = half_height;

	m_Verticies[4].X = half_depth;
	m_Verticies[4].Y = half_width;
	m_Verticies[4].Z = -half_height;

	m_Verticies[5].X = half_depth;
	m_Verticies[5].Y = -half_width;
	m_Verticies[5].Z = -half_height;

	m_Verticies[6].X = half_depth;
	m_Verticies[6].Y = -half_width;
	m_Verticies[6].Z = half_height;

	m_Verticies[7].X = half_depth;
	m_Verticies[7].Y = half_width;
	m_Verticies[7].Z = half_height;


	//Set_Vertex_Color (Vector3 (0.0F, 0.0F, ((float)(iface)) * 0.15F));
	Set_Vertex_Color (Vector4 (0.0F, 0.5F, 0.0F, 0.5f));
	//Set_Vertex_Alpha (0.5F);
	
	// Loop through all the faces and create them from triangle strips.
	for (int iface = 0; iface < FACE_COUNT; iface ++) {
		
		Begin_Tri_Strip ();
		Easy_Vertex (m_Verticies[FACE_VERTICIES[iface][0]]);
		Easy_Vertex (m_Verticies[FACE_VERTICIES[iface][1]]);
		Easy_Vertex (m_Verticies[FACE_VERTICIES[iface][2]]);
		Easy_Vertex (m_Verticies[FACE_VERTICIES[iface][3]]);
		End_Tri_Strip ();
	}
		
	Set_Collision_Type (COLLISION_TYPE_0);
	return ;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Dimensions
//
/////////////////////////////////////////////////////////////////////////////////////////
void
Box3DClass::Set_Dimensions (const Vector3 &dimensions)
{
	Set_Width (dimensions.Y);
	Set_Height (dimensions.Z);
	Set_Depth (dimensions.X);
	Update_Verticies ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Width
//
/////////////////////////////////////////////////////////////////////////////////////////
void
Box3DClass::Set_Width (float width)
{
	width = ::fabs (width);

	// Determine if the widht actually changed or not
	float curr_width = m_Dimensions.Y;
	if (curr_width != width) {
		
		// Recalc the obj-space positions of our 8 verticies
		float half_width = width / 2.0F;
		m_Verticies[0].Y = half_width;
		m_Verticies[1].Y = -half_width;
		m_Verticies[2].Y = -half_width;
		m_Verticies[3].Y = half_width;
		m_Verticies[4].Y = half_width;
		m_Verticies[5].Y = -half_width;
		m_Verticies[6].Y = -half_width;
		m_Verticies[7].Y = half_width;

		// Keep a flag around that will tell us we need
		// to update the verticies in the model when we're ready
		m_bDirty = true;
		m_Dimensions.Y = width;
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Height
//
/////////////////////////////////////////////////////////////////////////////////////////
void
Box3DClass::Set_Height (float height)
{
	height = ::fabs (height);

	// Determine if the height actually changed or not
	float curr_height = m_Dimensions.Z;
	if (curr_height != height) {
		
		// Recalc the obj-space positions of our 8 verticies
		float half_height = height / 2.0F;
		m_Verticies[0].Z = -half_height;
		m_Verticies[1].Z = -half_height;
		m_Verticies[2].Z = half_height;
		m_Verticies[3].Z = half_height;
		m_Verticies[4].Z = -half_height;
		m_Verticies[5].Z = -half_height;
		m_Verticies[6].Z = half_height;
		m_Verticies[7].Z = half_height;

		// Keep a flag around that will tell us we need
		// to update the verticies in the model when we're ready
		m_bDirty = true;
		m_Dimensions.Z = height;
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Depth
//
/////////////////////////////////////////////////////////////////////////////////////////
void
Box3DClass::Set_Depth (float depth)
{
	depth = ::fabs (depth);

	// Determine if the depth actually changed or not
	float curr_depth = m_Dimensions.X;
	if (curr_depth != depth) {
		
		// Recalc the obj-space positions of our 8 verticies
		float half_depth = depth / 2.0F;
		m_Verticies[0].X = -half_depth;
		m_Verticies[1].X = -half_depth;
		m_Verticies[2].X = -half_depth;
		m_Verticies[3].X = -half_depth;
		m_Verticies[4].X = half_depth;
		m_Verticies[5].X = half_depth;
		m_Verticies[6].X = half_depth;
		m_Verticies[7].X = half_depth;

		// Keep a flag around that will tell us we need
		// to update the verticies in the model when we're ready
		m_bDirty = true;
		m_Dimensions.X = depth;
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//	Update_Verticies
//
/////////////////////////////////////////////////////////////////////////////////////////
void
Box3DClass::Update_Verticies (void)
{
	if (m_bDirty) {

		// Loop through all 24 verticies in the model and
		// update their obj-space positions
		int ivertex = 0;
		for (int iface = 0; iface < FACE_COUNT; iface ++) {
			
			Easy_Move_Vertex (ivertex++, m_Verticies[FACE_VERTICIES[iface][0]]);
			Easy_Move_Vertex (ivertex++, m_Verticies[FACE_VERTICIES[iface][1]]);
			Easy_Move_Vertex (ivertex++, m_Verticies[FACE_VERTICIES[iface][2]]);
			Easy_Move_Vertex (ivertex++, m_Verticies[FACE_VERTICIES[iface][3]]);
		}

		// We're no longer dirty
		m_bDirty = false;
		Set_Dirty_Bounds ();
		Set_Dirty ();
		Invalidate_Cached_Bounding_Volumes ();
		Update_Cached_Bounding_Volumes ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Vertex_Lock_Position
//
/////////////////////////////////////////////////////////////////////////////////////////
Vector3
Box3DClass::Get_Vertex_Lock_Position (int vertex)
{
	Vector3 position (0, 0, 0);

	// Params OK?
	if ((vertex >= 0) && (vertex < 8)) {

		switch (vertex) {
			case 0:
				position = m_Verticies[6];
				break;

			case 1:
				position = m_Verticies[7];
				break;

			case 2:
				position = m_Verticies[4];
				break;

			case 3:
				position = m_Verticies[5];
				break;

			case 4:
				position = m_Verticies[2];
				break;

			case 5:
				position = m_Verticies[3];
				break;

			case 6:
				position = m_Verticies[0];
				break;

			case 7:
				position = m_Verticies[1];
				break;
		}

		// Convert the vertex position from obj space to world space
		//position += Get_Transform ().Get_Translation ();
		position = Get_Transform () * position;
	}

	// Return the vertex position
	return position;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//	Position_Vertex
//
/////////////////////////////////////////////////////////////////////////////////////////
void
Box3DClass::Position_Vertex
(
	int vertex,
	const Vector3 &new_position
)
{
	// Params OK?
	if ((vertex >= 0) && (vertex < 8)) {
				
		// Make a box from the vertex that we are 'locking' and
		// the new position
		Make_Box (Get_Vertex_Lock_Position (vertex), new_position);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//	Translate_Vertex
//
/////////////////////////////////////////////////////////////////////////////////////////
void
Box3DClass::Translate_Vertex
(
	int vertex,
	const Vector3 &translation
)
{
	// Params OK?
	if ((vertex >= 0) && (vertex < 8)) {
		
		float new_width = m_Dimensions.Y;
		float new_height = m_Dimensions.Z;
		float new_depth = m_Dimensions.X;

		// Modifiy the dimensions of the box
		new_width += translation.Y * 2.0F;
		new_height += translation.Z * 2.0F;
		new_depth += translation.X * 2.0F;
			
		// Modify the dimensions of the box if they are all valid
		if ((new_width >= MIN_SIZE) &&
		    (new_height >= MIN_SIZE) &&
			 (new_depth >= MIN_SIZE)) {
			Set_Dimensions (Vector3 (new_depth, new_width, new_height));
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//	Make_Box
//
/////////////////////////////////////////////////////////////////////////////////////////
void
Box3DClass::Make_Box
(
	const Vector3 &point1,
	const Vector3 &point2
)
{
	// Calculate the new center point for the box
	Vector3 delta = point2 - point1;
	Vector3 center = point1 + (delta / 2.0F);
	
	// Recalc the box's position to be centered around the 2 points
	Matrix3D transform = Get_Transform ();
	transform.Set_Translation (center);
	Set_Transform (transform);

	// Convert the 2 points to object space
	Matrix3D transform_inv;
	transform.Get_Orthogonal_Inverse (transform_inv);
	Vector3 obj_point1 = transform_inv * point1;
	Vector3 obj_point2 = transform_inv * point2;
	//Vector3 obj_point1 = point1 - center;
	//Vector3 obj_point2 = point2 - center;

	// Use these 2 points to determine the bottom left
	// and upper right points of the box
	Vector3 bottom_left;
	Vector3 upper_right;
	bottom_left.X = std::max (obj_point1.X, obj_point2.X);
	bottom_left.Y = std::min (obj_point1.Y, obj_point2.Y);
	bottom_left.Z = std::min (obj_point1.Z, obj_point2.Z);
	upper_right.X = std::min (obj_point1.X, obj_point2.X);
	upper_right.Y = std::max (obj_point1.Y, obj_point2.Y);
	upper_right.Z = std::max (obj_point1.Z, obj_point2.Z);

	// Given the bottom left and upper right coords, determine
	// what each of the 8 verticies are...
	m_Verticies[0].X = upper_right.X;
	m_Verticies[0].Y = upper_right.Y;
	m_Verticies[0].Z = bottom_left.Z;

	m_Verticies[1].X = upper_right.X;
	m_Verticies[1].Y = bottom_left.Y;
	m_Verticies[1].Z = bottom_left.Z;

	m_Verticies[2].X = upper_right.X;
	m_Verticies[2].Y = bottom_left.Y;
	m_Verticies[2].Z = upper_right.Z;

	m_Verticies[3].X = upper_right.X;
	m_Verticies[3].Y = upper_right.Y;
	m_Verticies[3].Z = upper_right.Z;

	m_Verticies[4].X = bottom_left.X;
	m_Verticies[4].Y = upper_right.Y;
	m_Verticies[4].Z = bottom_left.Z;

	m_Verticies[5].X = bottom_left.X;
	m_Verticies[5].Y = bottom_left.Y;
	m_Verticies[5].Z = bottom_left.Z;

	m_Verticies[6].X = bottom_left.X;
	m_Verticies[6].Y = bottom_left.Y;
	m_Verticies[6].Z = upper_right.Z;

	m_Verticies[7].X = bottom_left.X;
	m_Verticies[7].Y = upper_right.Y;
	m_Verticies[7].Z = upper_right.Z;

	// Recalc the box's dimensions and update its mesh
	m_Dimensions = upper_right - bottom_left;
	m_Dimensions.X = fabs (m_Dimensions.X);
	m_Dimensions.Y = fabs (m_Dimensions.Y);
	m_Dimensions.Z = fabs (m_Dimensions.Z);
	m_bDirty = true;
	Update_Verticies ();	
	return ;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//	Cast_Ray
//
/////////////////////////////////////////////////////////////////////////////////////////
bool 
Box3DClass::Cast_Ray (RayCollisionTestClass & raytest)
{
	if ((Get_Collision_Type() & raytest.CollisionType) == 0) return false;

	Matrix3D world_to_obj;
	Get_Transform().Get_Orthogonal_Inverse(world_to_obj);
	RayCollisionTestClass objray(raytest,world_to_obj);

	AABoxClass box;
	box.Center.Set (0, 0, 0);
	box.Extent = m_Dimensions * 0.5F;
	bool hit = CollisionMath::Collide (objray.Ray, box, raytest.Result);

	/*OBBoxClass obbox;
	obbox.Center = Get_Transform ().Get_Translation ();
	obbox.Extent = m_Dimensions * 0.5F;
	obbox.Basis = Matrix3 (Get_Transform ());
	
	CastResultStruct test_result = *raytest.Result;
	bool obb_hit = CollisionMath::Collide (raytest.Ray, obbox, &test_result);

	WWASSERT (hit == obb_hit);*/
	
	//
	// Transform result back into original coordinate system
	//
	if (hit) {
		raytest.CollidedRenderObj = this;
		if (raytest.Result->ComputeContactPoint) {
			Matrix3D::Transform_Vector (Get_Transform (), raytest.Result->ContactPoint, &(raytest.Result->ContactPoint));
		}
	}

	return hit;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Obj_Space_Bounding_Box
//
/////////////////////////////////////////////////////////////////////////////////////////
void
Box3DClass::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	box.Center = Vector3 (0,0,0);
	box.Extent = m_Dimensions * 0.5F;
   return ;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Color
//
/////////////////////////////////////////////////////////////////////////////////////////
void
Box3DClass::Set_Color (const Vector3 &color)
{
	for (int vertex = 0; vertex < VertCount; vertex ++) {
		Change_Vertex_Color (vertex, Vector4 (color.X, color.Y, color.Z, 0.5F), 0);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//	Render
//
/////////////////////////////////////////////////////////////////////////////////////////
void
Box3DClass::Render (RenderInfoClass &rinfo)
{
	// Only render if we if flagged visible
	if (Is_Not_Hidden_At_All ()) {
		DynamicMeshClass::Render (rinfo);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Vertex_Position
//
/////////////////////////////////////////////////////////////////////////////////////////
Vector3
Box3DClass::Get_Vertex_Position (int vertex)
{
	// The vertex position is simply the box's world position + the vertex offset
	//Vector3 position = Get_Transform ().Get_Translation ();
	//position += m_Verticies[vertex];

	Vector3 position = Get_Transform () * m_Verticies[vertex];

	// Return the vertex position
	return position;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//	Drag_VertexXY
//
/////////////////////////////////////////////////////////////////////////////////////////
void
Box3DClass::Drag_VertexXY (int vertex_index, POINT point, const Vector3 &locked_vertex)
{
	//
	// The start of the ray is simply the camera's position
	//
	Vector3 ray_start = ::Get_Camera_Mgr ()->Get_Camera ()->Get_Transform ().Get_Translation ();

	//
	// Ensure the 'point' is correct for this mode (fullscreen/windowed)
	//
	float xpos = point.x;
	float ypos = point.y;
	::Constrain_Point_To_Aspect_Ratio (xpos, ypos);
	
	//
	// The 'end' of the ray is the world coordinates of the supplied point
	//
	Vector3 ray_end;
	::Get_Camera_Mgr ()->Get_Camera ()->Device_To_World_Space (Vector2 (xpos, ypos), &ray_end);
	ray_end = ray_start + ((ray_end-ray_start) * 1000.00F);

	//
	// If the ray isn't parallel to the z-axis, then move the corner vertex
	//
	if (fabs(ray_end.Z - ray_start.Z) > 1.0F) {

		Vector3 vertex_pos = Get_Vertex_Position (vertex_index);

		// Calculate the fraction of the distance along the ray where the
		// Z value of the ray is the same as the Z value of the vertex we are moving.
		// This simulates an intersection of the ray with the x-y plane at height 'z'.
		double fraction = double(vertex_pos.Z - ray_start.Z) / double(ray_end.Z - ray_start.Z);

		// Now calcuate the 2nd point based on this fraction.
		// To do this we use the parameterized form of a line equation:
		// P = P1 + (P2 - P1) * t
		// (Where t is a percentage between 0 and 1)
		Vector3 new_point = ray_start + ((ray_end - ray_start) * fraction);
		Make_Box (locked_vertex, new_point);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//	Drag_VertexZ
//
/////////////////////////////////////////////////////////////////////////////////////////
void
Box3DClass::Drag_VertexZ (int vertex_index, POINT point, const Vector3 &locked_vertex)
{
	//
	// The start of the ray is simply the camera's position
	//
	Vector3 ray_start = ::Get_Camera_Mgr ()->Get_Camera ()->Get_Transform ().Get_Translation ();

	//
	// Ensure the 'point' is correct for this mode (fullscreen/windowed)
	//
	float xpos = point.x;
	float ypos = point.y;
	::Constrain_Point_To_Aspect_Ratio (xpos, ypos);
	
	//
	// The 'end' of the ray is the world coordinates of the supplied point
	//
	Vector3 ray_end;
	::Get_Camera_Mgr ()->Get_Camera ()->Device_To_World_Space (Vector2 (xpos, ypos), &ray_end);
	ray_end = ray_start + ((ray_end-ray_start) * 1000.00F);

	//
	// Determine which plane (y-z or x-z) to base the movement on
	//
	float deltax = ::fabs (ray_end.X - ray_start.X);
	float deltay = ::fabs (ray_end.Y - ray_start.Y);
	
	//
	// Determine where the ray intersects this plane
	//
	double fraction = 0;
	Vector3 vertex_pos = Get_Vertex_Position (vertex_index);
	if ((deltax > deltay) && (deltax != 0.0F)) {
						
		// Calculate the fraction of the distance along the ray where the
		// X value of the ray is the same as the X value of the vertex.
		// This simulates an intersection of the ray with the y-z plane at depth 'x'.
		fraction = double(vertex_pos.X - ray_start.X) / double(ray_end.X - ray_start.X);
		
	} else if ((deltay > deltax) && (deltay != 0.0F)) {

		// Calculate the fraction of the distance along the ray where the
		// X value of the ray is the same as the X value of the vertex.
		// This simulates an intersection of the ray with the x-z plane at depth 'y'.
		fraction = double(vertex_pos.Y - ray_start.Y) / double(ray_end.Y - ray_start.Y);			
	}

	// If we calculated a valid fraction, then use it
	// to determine the new vertex position.
	if (std::min (deltax, deltay) != 0.0F) {

		// Now calcuate the 2nd point based on this fraction.
		// To do this we use the parameterized form of a line equation:
		// P = P1 + (P2 - P1) * t
		// (Where t is a percentage between 0 and 1)
		Vector3 new_point = vertex_pos;
		new_point.Z = ray_start.Z + ((ray_end.Z - ray_start.Z) * fraction);
		Make_Box (locked_vertex, new_point);
	}

	return ;
}



//////////////////////////////////////////////////////////////////////////////////
//
//	Find_Intersection_Point
//
//////////////////////////////////////////////////////////////////////////////////
bool
Find_Intersection_Point
(
	const AABoxClass &	box,
	const Vector3 &		p0,
	const Vector3 &		p1,
	float *					percent,
	Vector3 *				intersection_point
)
{
	bool retval = false;

	//
	//	Find the locations of each of the 6 "planes"
	// we will be testing against
	//
	float x1 = box.Center.X - box.Extent.X;
	float x2 = box.Center.X + box.Extent.X;
	float y1 = box.Center.Y - box.Extent.Y;
	float y2 = box.Center.Y + box.Extent.Y;
	float z1 = box.Center.Z - box.Extent.Z;
	float z2 = box.Center.Z + box.Extent.Z;

	float t_values[6] = { -1, -1, -1, -1, -1, -1 };

	Vector3 delta = p1 - p0;

	//
	//	Find the "t" values where the line intersects the
	// 2 "Y-Z" planes
	//
	if (delta.X != 0) {
		t_values[0] = (x1 - p0.X) / delta.X;
		t_values[1] = (x2 - p0.X) / delta.X;
	}

	//
	//	Find the "t" values where the line intersects the
	// 2 "X-Z" planes
	//
	if (delta.Y != 0) {
		t_values[2] = (y1 - p0.Y) / delta.Y;
		t_values[3] = (y2 - p0.Y) / delta.Y;
	}

	//
	//	Find the "t" values where the line intersects the
	// 2 "X-Y" planes
	//
	if (delta.Z != 0) {
		t_values[4] = (z1 - p0.Z) / delta.Z;
		t_values[5] = (z2 - p0.Z) / delta.Z;
	}

	//
	//	Loop over all the "t" values we've calculated
	//
	(*percent) = 2.0F;
	for (int index = 0; index < 6; index ++) {		

		//
		// Is this "t" value the smallest in-range value we've
		// found yet?
		//
		if (	t_values[index] >= 0 &&
				t_values[index] <= 1.0F &&
				t_values[index] < (*percent))
		{
			//
			//	Find the point which exists at this "t" value along the line segment
			//
			Vector3 point = p0 + delta * t_values[index];

			//
			//	If this point isn't outside the box, then we'll consider it good enough
			//
			if (	(WWMath::Fabs (point.X - box.Center.X) <= (box.Extent.X + WWMATH_EPSILON)) &&
					(WWMath::Fabs (point.Y - box.Center.Y) <= (box.Extent.Y + WWMATH_EPSILON)) &&
					(WWMath::Fabs (point.Z - box.Center.Z) <= (box.Extent.Z + WWMATH_EPSILON)))
			{
				(*percent)					= t_values[index];
				(*intersection_point)	= point;
				retval						= true;
			}
		}
	}

	return retval;
}
