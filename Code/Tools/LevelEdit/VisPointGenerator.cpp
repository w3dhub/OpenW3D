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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/VisPointGenerator.cpp                                                                                                                                                                                                                                                                                                                              $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 25                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "vispointgenerator.h"
#include "cameramgr.h"
#include "Utils.h"
#include "mesh.h"
#include "meshmdl.h"
#include "vector3.h"
#include "vector3i.h"
#include "collisiongroups.h"
#include "sceneeditor.h"
#include "physcoltest.h"
#include "hittestinfo.h"
#include "node.h"
#include "obbox.h"
#include "VisPointNode.h"
#include "nodemgr.h"
#include "staticphys.h"
#include <algorithm>



//////////////////////////////////////////////////////////////////////////
//
//	Constants
//
//////////////////////////////////////////////////////////////////////////

const float CEILING_CHECK_HEIGHT				= 250.0F;
const float FLOOR_CHECK_HEIGHT				= -250.0F;
const float MIN_CEILING_HEIGHT				= 2.0F;
const float PERCENT_FROM_COLLISION			= 0.75F;
const float DEF_POINT_RAISE_HEIGHT			= 6.5F;
const float DEF_INDOOR_POINT_RAISE_HEIGHT	= 2.0f;
const float FLAT_FACE_NORMAL_MIN				= 0.0F;
const float VIEWPLANE_BOX_HALF_DEPTH		= 0.0005F;
const float DEF_VIS_SAMPLE_HEIGHT			= 20.0F;		// we sample up to 20 meters above the vis sector
const float VERTICAL_GRANULARITY				= 5.0F;		// granularity of sampling in the Z axis
const float DEF_CAM_SIM_POINT_COUNT			= 32.0F;
const float CAMERA_SIM_RADIUS					= 10.0F;

const char * VIS_BIAS_STRING					= "visbias";


//////////////////////////////////////////////////////////////////////////
//
//	VisPointGeneratorClass
//
//////////////////////////////////////////////////////////////////////////
VisPointGeneratorClass::VisPointGeneratorClass (float granularity)
	:	m_Granularity (granularity),
		m_BaseGranularity (granularity),
		m_IgnoreBias (false),
		m_ViewPlaneExtent (0, 0, 0),
		m_WorldToGridTM (1),
		m_CurrentNode (NULL),
		m_StatWindow (NULL),
		m_pCameraSimOffsets (NULL),
		m_CurrentPointList (NULL),
		m_PolygonsProcessed (0),
		m_TotalPoints (0),
		m_CameraSimPointCount (DEF_CAM_SIM_POINT_COUNT),
		m_CurrentCeilingHeight (DEF_POINT_RAISE_HEIGHT),
		m_VisSampleHeight(DEF_VIS_SAMPLE_HEIGHT)
{
	Initialize_Camera_Sim ();

	double hfov = 0;
	double vfov = 0;
	float znear = 0;
	float zfar = 0;
	::Get_Scene_Editor ()->Get_Vis_Camera_FOV (hfov, vfov);
	::Get_Scene_Editor ()->Get_Vis_Camera_Clip_Planes (znear, zfar);

	//
	//	Calculate what the extents of a box would be if we
	// extruded the view plane by one millimeter.
	// Note: The extents are half-extents.
	//
	float y_dim = znear * ::tan (hfov / 2);
	float z_dim = znear * ::tan (vfov / 2);
	float max_dim = std::max (y_dim, z_dim);
	max_dim += 0.001F;
	
	m_ViewPlaneExtent.Set (max_dim, max_dim, max_dim);
	return ;
}

//////////////////////////////////////////////////////////////////////////
//
//	~VisPointGeneratorClass
//
//////////////////////////////////////////////////////////////////////////
VisPointGeneratorClass::~VisPointGeneratorClass (void)
{
	SAFE_DELETE_ARRAY (m_pCameraSimOffsets);

	//
	//	Free the memory associated with the per-node point list
	//
	for (int index = 0; index < m_PointList.Count (); index ++) {
		VisPointListClass *sub_point_list = m_PointList[index];
		SAFE_DELETE (sub_point_list);
	}

	m_PointList.Delete_All ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Is_Numerical
//
//////////////////////////////////////////////////////////////////////////
static inline bool
Is_Numerical (char ch)
{
	return ((ch >= '0' && ch <= '9') || ch == '-' || ch == '+' || ch == '.');
}


//////////////////////////////////////////////////////////////////////////
//
//	Read_Float_Param
//
//////////////////////////////////////////////////////////////////////////
static bool
Read_Float_Param (LPCTSTR text, LPCTSTR key, float *value)
{
	bool retval = false;

	//
	//	Attempt to find the 'key' in this text block
	//
	CString lowercase_text (text);
	lowercase_text.MakeLower ();
	LPCTSTR key_start = ::strstr (lowercase_text, key);
	if (key_start != NULL) {
		
		//
		//	Move past the key designator
		//
		LPCTSTR key_value = key_start + ::lstrlen (key);

		//
		//	Skip whitespace
		//
		while (key_value[0] != 0 && key_value[0] == ' ') {
			key_value ++;
		}

		if (key_value[0] == '=') {
			key_value ++;

			//
			//	Skip whitespace
			//
			while (key_value[0] != 0 && key_value[0] == ' ') {
				key_value ++;
			}

			//
			//	Determine how much of the string we should convert
			// to a number.
			//
			int index = 0;
			while (::Is_Numerical (key_value[index ++])) ;
			
			if (index > 1) {
				
				//
				//	Convert the string to a float and return the value
				// to the caller.
				//
				char *number_str = new char[index];
				::lstrcpyn (number_str, key_value, index);
				(*value) = ::atof (number_str);
				delete [] number_str;

				retval = true;
			}
		}
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////
//
//	Determine_Granularity
//
//////////////////////////////////////////////////////////////////////////
void
VisPointGeneratorClass::Determine_Granularity (MeshClass &mesh)
{
	//
	//	Default to the base granularity
	//
	m_Granularity = m_BaseGranularity;
	
	//
	//	Should we use the vis-bias settings from the mesh?
	//
	if (m_IgnoreBias == false) {

		//
		//	Check to see if this mesh uses a vis-bias parameter
		//	
		float vis_bias		= 1.0F;
		CString user_text	= mesh.Get_User_Text ();	
		if (::Read_Float_Param (user_text, VIS_BIAS_STRING, &vis_bias)) {
			
			//
			//	Adjust the granularity based on the value of the
			// VisBias setting.
			//
			m_Granularity	= m_Granularity / vis_bias;

			//
			//	Ensure the bias is inside an exceptable range
			//
			m_Granularity = std::clamp (m_Granularity, 0.1F, 100.0F);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Submit_Mesh
//
//////////////////////////////////////////////////////////////////////////
void
VisPointGeneratorClass::Submit_Mesh (MeshClass &mesh)
{
	//
	//	Make sure we use the right granularity for this mesh
	//
	Determine_Granularity (mesh);

	//
	//	Create a grid we can use to effeciently build a list
	// of vis points spaced 'granularity' meters apart.
	//	
	AABoxClass box;
	mesh.Get_Obj_Space_Bounding_Box (box);
	int cells_x = (int)fabs(((box.Extent.X * 2) / m_Granularity));
	int cells_y = (int)fabs(((box.Extent.Y * 2) / m_Granularity));
	int cells_z = (int)fabs((((box.Extent.Z * 2) + DEF_POINT_RAISE_HEIGHT) / m_Granularity));
	m_Grid.Create_Grid (Vector3 (cells_x + 1, cells_y + 1, cells_z + 1), 0);

	//
	//	Create a transformation matrix that can convert
	// a vis-point from world space to 'grid' space.
	//
	Matrix3D object_tm = mesh.Get_Transform ();
	object_tm.Get_Orthogonal_Inverse (m_WorldToGridTM);	
	
	Vector3 offset (	-(box.Center.X - box.Extent.X),
							-(box.Center.Y - box.Extent.Y),
							-(box.Center.Z - box.Extent.Z));
	
	Vector3 pos = m_WorldToGridTM.Get_Translation ();
	pos += offset;
	m_WorldToGridTM.Set_Translation (pos);

	//
	//	Get this mesh's polygon information
	//
	MeshModelClass *model = mesh.Get_Model ();
	if (model != NULL) {		
		const TriIndex *poly_array		= model->Get_Polygon_Array ();
		const Vector3 *vertex_array	= model->Get_Vertex_Array ();
		const Vector4 *plane_array		= model->Get_Plane_Array (true);

		//
		//	Loop through all the polygons in the mesh
		//
		int poly_count = model->Get_Polygon_Count ();
		for (int index = 0; index < poly_count; index ++) {				
			const TriIndex &poly_def = poly_array[index];

			// Is this poly mostly flat?
			if (plane_array[index].Z > FLAT_FACE_NORMAL_MIN) {

				//
				//	Get the verticies that make up the triangle
				//
				Vector3 point1 = vertex_array[poly_def.I];
				Vector3 point2 = vertex_array[poly_def.J];
				Vector3 point3 = vertex_array[poly_def.K];

				//
				//	Convert the points from obj-space to world space.
				//
				Matrix3D obj_tm = mesh.Get_Transform ();
				point1 = obj_tm * point1;
				point2 = obj_tm * point2;
				point3 = obj_tm * point3;

				// Determine what the center of the triangle is.
				Vector3 center = (point1 + point2 + point3) / 3;

				//
				//	Try to submit a vis point for vertex #1 of this triangle.
				//
				Find_Valid_Points (point1 + ((center-point1) * 0.1F), center, Matrix3(1), false);

				//
				//	Try to submit a vis point for vertex #2 of this triangle.
				//
				Find_Valid_Points (point2 + ((center-point2) * 0.1F), center, Matrix3(1), false);

				//
				//	Try to submit a vis point for vertex #3 of this triangle.
				//
				Find_Valid_Points (point3 + ((center-point3) * 0.1F), center, Matrix3(1), false);

				// 
				// Recursively subdivide and submit center points 
				//
				Subdivide_And_Submit_Centers (point1, point2, point3);
			}

			m_PolygonsProcessed ++;
		}
	}

	MEMBER_RELEASE (model);
	
	//DWORD before_collection = ::GetTickCount ();

	//
	//	Add all the vis points from the grid to the global list.
	//
	int cell_count = m_Grid.Get_Flat_Size ();
	for (int cell = 0; cell < cell_count; cell ++) {
					
		//
		//	Add the vis point from this cell to the list.
		//
		//Matrix3D *vis_point = m_Grid.Get_At_Flat (cell);
		VisPointInfo *vis_point = m_Grid.Get_At_Flat (cell);
		if (vis_point != NULL) {
			
			//
			//	Allocate a new point list for this point
			//
			m_CurrentPointList = new VisPointListClass;
			m_CurrentPointList->transform		= vis_point->m_Transform;
			m_CurrentPointList->sample_point = vis_point->m_Transform.Get_Translation ();
			m_PointList.Add (m_CurrentPointList);
			m_TotalPoints ++;

			//
			//	Add 'sub-points' to this point list
			//
			if (vis_point->m_DoCameraRing) {
				//Generate_Camera_Locations (vis_point->m_Transform);
			}

			//
			//	Convert the transform from an obj transform to a camera transform
			//
			Matrix3D world_to_cam_tm (Vector3 (0, -1, 0), Vector3 (0, 0, 1), Vector3 (-1, 0, 0), Vector3 (0, 0, 0));
			m_CurrentPointList->transform		= m_CurrentPointList->transform * world_to_cam_tm;
			m_CurrentPointList->sample_point	= m_CurrentPointList->transform.Get_Translation ();

			// Free the grid-vis point
			delete vis_point;
		}
	}

	m_Grid.Free_Grid ();
	return ;
}

/////////////////////////////////////////////////////////////////////////
//
//	Subdivide_And_Submit_Centers
// submits the center point of the given triangle and recursively 
// subdivides the triangle until it is smaller than the granularity
// value.
//
/////////////////////////////////////////////////////////////////////////
void
VisPointGeneratorClass::Subdivide_And_Submit_Centers
(
	const Vector3 & point1,
	const Vector3 & point2,
	const Vector3 & point3
)
{
	// submit the center point
	Vector3 center = (point1 + point2 + point3) / 3.0f;
	Find_Valid_Points (center, center, Matrix3(1), false);

	// recurse if the edge lengths are greater than 2*granularity
	float e0len = (point2 - point1).Length2();
	float e1len = (point3 - point2).Length2();
	float e2len = (point1 - point3).Length2();
	float elen = m_Granularity * m_Granularity * 2.0f;

	if ( (e0len > elen) || (e1len > elen) || (e2len > elen) ) {
		// subdivide into four tris 
		Vector3 p12 = (point1 + point2) * 0.5f;
		Vector3 p23 = (point2 + point3) * 0.5f;
		Vector3 p31 = (point3 + point1) * 0.5f;

		Subdivide_And_Submit_Centers(point1,p12,p31);
		Subdivide_And_Submit_Centers(p12,point2,p23);
		Subdivide_And_Submit_Centers(p23,point3,p31);
		Subdivide_And_Submit_Centers(p12,p23,p31);
	}	
}

/////////////////////////////////////////////////////////////////////////
//
//	Find_Valid_Point
//
/////////////////////////////////////////////////////////////////////////
bool
VisPointGeneratorClass::Find_Valid_Points
(
	const Vector3 &start,
	const Vector3 &end,
	const Matrix3 &/* orientation */,
	bool do_camera_ring
)
{
	bool retval = false;

	//
	//	Try a few different points along the line from [start, end].
	//
	const int MAX_ATTEMPTS = 5;
	
	// 
	// If the start and end point are very close together, only make
	// one attempt.
	//
	int max_attempts = 5;
	if ((end - start).Length() < 0.01f) {
		max_attempts = 1;
	}

	for (int attempt = 0; (attempt < max_attempts) && !retval; attempt ++) {
		
		//
		//	Build a transform to test along the provided line segment
		//
		float percent = ((float)attempt) / ((float)MAX_ATTEMPTS);
		Vector3 position = start + ((end - start) * percent);

		
#if 0
		//
		//	Check to make sure this vis-point isn't under a hill or mountain.
		// If the vis-point's ceiling checks out, then raise the point up
		// to the approximate camera height in the game.
		//
		float ceiling_dist = 0;
		if	(	Check_Ceiling (position, &ceiling_dist) &&
				(ceiling_dist > MIN_CEILING_HEIGHT))
		{
			bool retval = true;
			if (ceiling_dist < HEIGHT_SPREAD_DISTANCE) {
				
				//
				//	The ceiling is low enough where we should just submit one point
				//
				position.Z += DEF_INDOOR_POINT_RAISE_HEIGHT;
				retval &= Submit_Point (Matrix3D (position), do_camera_ring);
			
			} else {
				
				//
				//	The ceiling is high enough to try spreading points vertically
				//
				float delta_z = HEIGHT_SPREAD_DELTA;
				while (retval && delta_z <= HEIGHT_SPREAD_DISTANCE) {
					Vector3 point_pos = position;
					point_pos.Z += delta_z;
					retval &= Submit_Point (Matrix3D (point_pos), do_camera_ring);					
					delta_z += HEIGHT_SPREAD_DELTA;
				}
			}
		}
#else

		//
		// Search for the proper interval to sample above this sector.
		// We search upwards for "floor" and "roof" polygons.  We start above
		// the highest floor encountered and sample up until we hit the lowest
		// "roof" encountered.
		// If any floor is encountered that is itself another vis sector, we don't
		// generate sample points at this location for this vis sector.
		//
		bool done = false;
		bool ok_to_sample = false;
		
		Vector3 start_point(position.X,position.Y,position.Z + 0.001f);
		Vector3 end_point(position.X,position.Y,position.Z + m_VisSampleHeight);

		while (!done) {

			CastResultStruct res;	
			LineSegClass ray (start_point, end_point);
	
			PhysRayCollisionTestClass raytest (ray, &res, GAME_COLLISION_GROUP, COLLISION_TYPE_PHYSICAL | COLLISION_TYPE_VIS);
			raytest.CheckDynamicObjs = false;
			::Get_Scene_Editor ()->Cast_Ray (raytest);
			
			if (res.Fraction == 1.0f) {
				done = true;
			} else {
				
				Vector3 point;
				ray.Compute_Point(res.Fraction,&point);

				PhysClass *physobj = raytest.CollidedPhysObj;
				bool is_vis_sector = (physobj->As_StaticPhysClass() && physobj->As_StaticPhysClass()->Is_Vis_Sector());

				if ((is_vis_sector) || (res.Normal.Z < 0.0f)) {

					// 
					// If we hit a roof or another vis sector, snap the end point down and signal that we're done
					//
					end_point.Z = point.Z;
					ok_to_sample = true;
					done = true;
			
				} else {
				
					// 
					// If we hit a floor (or anything else), "ratchet" up past that point 
					//
					start_point.Z = point.Z + 0.001f;
				}

			}
		}

		//
		// If we didn't find a roof within the VIS_SAMPLE_HEIGHT (20 meters)
		// then we'll just do a sanity check to make sure we're not underneath a backface above
		// the 20 meter sampling distance.  The purpose of this is to try to avoid
		// generating sample points that will have to be rejected.  These types of points can
		// easily be generated when a vis sector protrudes underneath a hill or something else big.
		// If this check passes, then it is ok to sample.
		//
		if (!ok_to_sample) {
		
			float dist = 0.0f;
			ok_to_sample = Check_Ceiling(end_point,&dist);

		}

		// 
		// Now generate samples between start_point and end_point
		//
		if (ok_to_sample) {

			retval = true;
			Vector3 sample_point(start_point);
			
			if (DEF_INDOOR_POINT_RAISE_HEIGHT < end_point.Z - sample_point.Z) {
				sample_point.Z += DEF_INDOOR_POINT_RAISE_HEIGHT;
			} else {
				sample_point.Z = end_point.Z - 0.1f;
			}
			
			while (sample_point.Z < end_point.Z) {
				
				retval &= Submit_Point (Matrix3D(sample_point), do_camera_ring);					
				sample_point.Z += VERTICAL_GRANULARITY;

			}
		} 
#endif
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////
//
//	Check_Ceiling
//
//	This method casts a ray straight up from the candiate vis-point
// and checks to see if the object it hits (if any) is a valid
// vis 'ceiling'.  This makes sure we don't generate points that are
// inside of hills or mountains.
//
/////////////////////////////////////////////////////////////////////////
bool
VisPointGeneratorClass::Check_Ceiling (const Vector3 &position, float *ceiling_dist)
{
	bool retval = true;

	//
	//	Build a ray from the given position up 100 meters in the air.
	//
	Vector3 start_point = position + Vector3 (0, 0, 0.001F);
	Vector3 end_point = position + Vector3 (0, 0, CEILING_CHECK_HEIGHT);
	LineSegClass ray (start_point, end_point);
	
	//
	// Cast the ray into the world and see what it hits.
	//
	CastResultStruct res;	
	PhysRayCollisionTestClass raytest (ray, &res, GAME_COLLISION_GROUP, COLLISION_TYPE_PHYSICAL | COLLISION_TYPE_VIS);
	::Get_Scene_Editor ()->Cast_Ray (raytest);

	//
	//	Return how far above us the ceiling is.
	//
	if (ceiling_dist != NULL) {
		(*ceiling_dist) = (res.Fraction * (end_point.Z - start_point.Z));
	}

	//
	//	Did we hit anything?
	//
	if (res.Fraction < 1.0F) {

		// Get the physics object we hit
		PhysClass *physobj = raytest.CollidedPhysObj;
		if (physobj != NULL) {

			//
			//	Check to see if the node we hit is a static tile.  Otherwise
			// we don't care about it.
			//
			/*HITTESTINFO *hittest = (HITTESTINFO *)physobj->Peek_Model ()->Get_User_Data ();
			if (	(hittest != NULL) &&
					(hittest->Type == HITTESTINFO::Node) &&
					(hittest->node != NULL) &&
					(hittest->node->Is_Static ()))*/

			if (physobj->As_StaticPhysClass () != NULL) {
					
				// If this polygon is facing up, then make sure it satisfies
				// our 'ceiling' requirements for back-face polygons.
				if ((res.Normal.Z > 0) &&
					 Is_Object_Invalid_Roof (physobj->Peek_Model ()))
				{
					retval = false;
				}
			}
		}
	}	

	return retval;
}


/////////////////////////////////////////////////////////////////////////
//
//	Is_Grid_Cell_Empty
//
/////////////////////////////////////////////////////////////////////////
bool
VisPointGeneratorClass::Is_Grid_Cell_Empty (const Vector3 &position)
{
	Vector3 grid_pos = m_WorldToGridTM * position;
	grid_pos.X = grid_pos.X / m_Granularity;
	grid_pos.Y = grid_pos.Y / m_Granularity;
	grid_pos.Z = grid_pos.Z / m_Granularity;

	return (m_Grid.Get_At (grid_pos.X, grid_pos.Y, grid_pos.Z) == NULL);
}


/////////////////////////////////////////////////////////////////////////
//
//	Submit_Point
//
/////////////////////////////////////////////////////////////////////////
bool
VisPointGeneratorClass::Submit_Point
(
	const Matrix3D &	vis_transform,
	bool					do_camera_ring
)
{
	bool was_valid = true;

	//
	//	Convert the world-space position of the matrix to the
	// grid-space position.
	//
	Vector3 grid_pos = vis_transform.Get_Translation ();
	grid_pos = m_WorldToGridTM * grid_pos;
	grid_pos.X = grid_pos.X / m_Granularity;
	grid_pos.Y = grid_pos.Y / m_Granularity;
	grid_pos.Z = grid_pos.Z / m_Granularity;

	grid_pos.X = WWMath::Clamp (grid_pos.X, 0, (float)(m_Grid.Get_Cells_X () - 1));
	grid_pos.Y = WWMath::Clamp (grid_pos.Y, 0, (float)(m_Grid.Get_Cells_Y () - 1));
	grid_pos.Z = WWMath::Clamp (grid_pos.Z, 0, (float)(m_Grid.Get_Cells_Z () - 1));

	float half_gran = m_Granularity / 2;
	Vector3 cell_center ((grid_pos.X * m_Granularity) + half_gran,
								(grid_pos.Y * m_Granularity) + half_gran,
								(grid_pos.Z * m_Granularity) + half_gran);

	Matrix3D grid_to_world_tm;
	m_WorldToGridTM.Get_Orthogonal_Inverse (grid_to_world_tm);
	cell_center = grid_to_world_tm * cell_center;
	float new_point_dist = (vis_transform.Get_Translation () - cell_center).Length ();
	
	//
	//	Only insert the point into the grid if the grid cell is
	//	empty.
	//
	VisPointInfo *cell_contents = m_Grid.Get_At (grid_pos.X, grid_pos.Y, grid_pos.Z);
	if ((cell_contents == NULL) ||
		 ((cell_contents->m_Transform.Get_Translation () - cell_center).Length () > new_point_dist)) {

		//
		//	See if the camera would intersect a wall if a vis point
		// were positioned here.
		//
		was_valid = Do_View_Planes_Pass (vis_transform);
		if (1 || was_valid) {
			
			//
			//	Convert the world-space transform to a 
			// camera space transform.
			//
			//Matrix3D world_to_cam_tm (Vector3 (0, -1, 0), Vector3 (0, 0, 1), Vector3 (-1, 0, 0), Vector3 (0, 0, 0));
			VisPointInfo *vis_info		= new VisPointInfo;
			vis_info->m_Transform		= vis_transform;
			vis_info->m_DoCameraRing	= do_camera_ring;

			// Insert this point into the grid
			m_Grid.Set_At (grid_pos.X, grid_pos.Y, grid_pos.Z, vis_info);
			SAFE_DELETE (cell_contents);
		}
	}
	
	return was_valid;
}


/////////////////////////////////////////////////////////////////////////
//
//	Do_View_Planes_Pass
//
//	This method checks the view-plane in each of the six directions around
// the candidate point and determines if any of them intersect a 'wall'.
//
/////////////////////////////////////////////////////////////////////////
bool
VisPointGeneratorClass::Do_View_Planes_Pass (const Matrix3D &vis_transform)
{
	bool retval = true;

	Vector3 center = vis_transform.Get_Translation ();
	Matrix3 orig_basis (vis_transform);
	
	//
	//	Loop through and test each of the 6 orienations
	// of the view plane to make sure none of them intersect
	// a 'wall'.
	//	
	/*CastResultStruct result;
	for (int index = 0; (index < VIS_RENDER_DIRECTIONS) && retval; index ++) {

		//
		//	Build the orientation of the view-plane
		//
		Matrix3 basis = orig_basis;
		basis.Rotate_X (VIS_RENDER_TABLE[index].X);
		basis.Rotate_Y (VIS_RENDER_TABLE[index].Y);
		basis.Rotate_Z (VIS_RENDER_TABLE[index].Z);*/

		//
		//	Create a box representing the view plane
		//
		//Vector3
		AABoxClass box (center, m_ViewPlaneExtent);
		
		//
		// Check to see if this viewplane 'box' collides
		// with anything.
		//
		//result.Reset ();
		CastResultStruct result;
		PhysAABoxCollisionTestClass collision_test (box, Vector3(0,0,0), &result, GAME_COLLISION_GROUP, COLLISION_TYPE_ALL);
		SceneEditorClass *scene = ::Get_Scene_Editor ();
		scene->Cast_AABox (collision_test);
		retval = (result.StartBad == false) && !(result.Fraction < 1.0F);
	//}

	return retval;
}


/////////////////////////////////////////////////////////////////////////
//
//	Test_Camera_Sim_Point
//
/////////////////////////////////////////////////////////////////////////
bool
VisPointGeneratorClass::Test_Camera_Sim_Point
(
	const Vector3 &	start_point,
	const Vector3 &	end_point,
	Matrix3D *			transform_result,
	NodeClass **		node_result
)
{
	bool retval = false;

	AABoxClass box (start_point, m_ViewPlaneExtent);
	Vector3 sweep_vector = end_point - start_point;
	
	//
	// Check to see if the camera's 'box' collides with anything.
	//
	CastResultStruct result;
	PhysAABoxCollisionTestClass collision_test (box, sweep_vector, &result, GAME_COLLISION_GROUP, COLLISION_TYPE_ALL);
	SceneEditorClass *scene = ::Get_Scene_Editor ();
	scene->Cast_AABox (collision_test);

	//
	//	If we collided with an object, then move the sample point in a little...
	//
	Vector3 test_point = end_point;
	if (result.Fraction < 1.0F) {
		test_point = start_point + (sweep_vector * result.Fraction * PERCENT_FROM_COLLISION);
	}

	//
	//	Find the node this point lies over
	//
	NodeClass *node = Find_Floor_Node (test_point);
	
	//
	//	If the point doesn't lie over the current node, then
	// we've successfully found a new point.
	//
	if ((node != m_CurrentNode) && (node != NULL)) {
		(*node_result) = node;
		(*transform_result).Make_Identity ();
		(*transform_result).Obj_Look_At (test_point, start_point, 0);
		retval = true;
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////
//
//	Find_Floor_Node
//
/////////////////////////////////////////////////////////////////////////
NodeClass *
VisPointGeneratorClass::Find_Floor_Node (const Vector3 &start_point)
{
	NodeClass *node = NULL;

	//
	//	Build a ray from the given position down 1000 meters
	//
	Vector3 end_point = start_point + Vector3 (0, 0, FLOOR_CHECK_HEIGHT);
	LineSegClass ray (start_point, end_point);

	//
	// Cast the ray into the world and see what it hits.
	//
	CastResultStruct res;	
	PhysRayCollisionTestClass raytest (ray, &res, GAME_COLLISION_GROUP, COLLISION_TYPE_ALL);
	::Get_Scene_Editor ()->Cast_Ray (raytest);

	//
	//	Did we hit anything?
	//
	if (res.Fraction < 1.0F) {

		// Get the physics object we hit
		PhysClass *physobj = raytest.CollidedPhysObj;
		if (physobj != NULL) {

			//
			//	Check to see if the 'node' we hit is different then the node
			// we are currently processing.
			//
			HITTESTINFO *hittest = (HITTESTINFO *)physobj->Peek_Model ()->Get_User_Data ();
			if (	(hittest != NULL) &&
					(hittest->Type == HITTESTINFO::Node))
			{
				//
				//	Pass the node we found back to the caller.
				//
				node = hittest->node;
			}
		}
	}

	return node;
}


/////////////////////////////////////////////////////////////////////////
//
//	Is_Object_Invalid_Roof
//
//	This method determines if it is valid for a vis-point to be generated
// underneath a given object.
//
//	An object is considered 'invalid' for a vis-point's 'roof' if:
//
//		- The mesh's polygons are visible.
//		- The mesh's polygons are single-sided
//		- The mesh's polygons can be physically collideable.
//		- The mesh is marked for vis generation itself.
//
/////////////////////////////////////////////////////////////////////////
bool
VisPointGeneratorClass::Is_Object_Invalid_Roof (RenderObjClass *render_obj)
{
	bool retval = true;

	//
	// Loop through all the render object's sub-objects
	//
	int count = render_obj->Get_Num_Sub_Objects ();
	for (int index = 0; (index < count) && retval; index ++) {
		
		//
		// Check this subobject
		//
		RenderObjClass *sub_object = render_obj->Get_Sub_Object (index);
		if (sub_object != NULL) {			
			retval &= Is_Object_Invalid_Roof (sub_object);
			MEMBER_RELEASE (sub_object);
		}
	}	

	
	//
	// Is this render object a mesh?
	//
	if (render_obj->Class_ID () == RenderObjClass::CLASSID_MESH) {
		MeshModelClass *model = ((MeshClass *)render_obj)->Get_Model ();
		if (model != NULL) {

			//
			//	The mesh is invalid if:
			//
			//		a) The mesh's polys are single-sided AND
			//		b) The mesh is a vis-sector AND
			//		c) The mesh is physically collideable AND
			//		d) The mesh visible
			//
			retval &= (model->Get_Flag (MeshModelClass::TWO_SIDED) != MeshModelClass::TWO_SIDED);
			//retval &= ((render_obj->Get_Collision_Type () & COLLISION_TYPE_VIS) == COLLISION_TYPE_VIS);
			retval &= ((render_obj->Get_Collision_Type () & COLLISION_TYPE_PHYSICAL) == COLLISION_TYPE_PHYSICAL);
			retval &= (render_obj->Is_Not_Hidden_At_All () != 0);

			MEMBER_RELEASE (model);
		}		
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////
//
//	Initialize_Camera_Sim
//
/////////////////////////////////////////////////////////////////////////
void
VisPointGeneratorClass::Initialize_Camera_Sim (void)
{
	SAFE_DELETE_ARRAY (m_pCameraSimOffsets);
	m_pCameraSimOffsets = new Vector3[m_CameraSimPointCount];

	for (int index = 0; index < m_CameraSimPointCount; index ++) {
		
		//
		//	Calculate an arbitrary number of points around a circle
		// and store these 'offsets' in a vector array.
		//
		float angle = ((2 * 3.14159265359F) * index) / m_CameraSimPointCount;
		float x = CAMERA_SIM_RADIUS * ::cos (angle);
		float y = CAMERA_SIM_RADIUS * ::sin (angle);
		
		m_pCameraSimOffsets[index] = Vector3 (y, -x, 0);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Generate_Camera_Locations
//
//	This method generates extra phantom vis-points for a given position to
//	simulate areas the camera can be.  These points are checked to see if
//	they are over a different mesh.  If they are over a different mesh,
//	then the height information is used by the mesh when generating its
//	vis-points;
//
/////////////////////////////////////////////////////////////////////////
void
VisPointGeneratorClass::Generate_Camera_Locations (const Matrix3D &real_vis_point)
{
	Vector3 position = real_vis_point.Get_Translation ();

	struct CAMERA_SIM_POINTS
	{
		Matrix3D		vis_point;
		NodeClass *	node;
	};

	CAMERA_SIM_POINTS *sim_points = new CAMERA_SIM_POINTS[m_CameraSimPointCount];
	
	//
	//	Loop through each camera simulation point (they lie
	// in a circle around the position)
	//	
	int index;
	for (index = 0; index < m_CameraSimPointCount; index ++) {
				
		//
		//	Build a ray from the given position down 1000 meters
		//
		Vector3 end_point = position + m_pCameraSimOffsets[index];

		sim_points[index].vis_point.Make_Identity ();
		sim_points[index].node = NULL;

		//
		//	Test the point to see if lies over another mesh
		//
		Test_Camera_Sim_Point (	position,
										end_point,
										&sim_points[index].vis_point,
										&sim_points[index].node);
	}

	//
	//	Now submit the 'middle' point of each node series
	//
	int start_index = 0;
	NodeClass *current_node = NULL;
	for (index = 0; index < m_CameraSimPointCount; index ++) {

		//
		//	Has the node changed?
		//
		if ((sim_points[index].node != current_node) ||
			 (index == m_CameraSimPointCount - 1)) {
			
			if (index > 0) {
				
				//
				//	Attempt to find the 'middle' of the series
				//
				int middle_index = start_index + ((index - start_index) >> 1);
				bool found = false;
				int max = ((index - start_index) >> 1) + 1;
				for (int offset = 0; (offset < max) && !found; offset ++) {
					
					if (((middle_index + offset) < index) &&
						 (sim_points[middle_index + offset].node != NULL))
					{
						middle_index += offset;
						found = true;
					} else if (((middle_index - offset) >= start_index) &&
									(sim_points[middle_index - offset].node != NULL))
					{
						middle_index -= offset;
						found = true;
					}
				}

				//
				//	Add the point to our list
				//
				if (found) {

					//
					//	Convert the transform from an obj trasform to a camera transform
					//
					Matrix3D world_to_cam_tm (Vector3 (0, -1, 0), Vector3 (0, 0, 1), Vector3 (-1, 0, 0), Vector3 (0, 0, 0));
					Matrix3D camera_tm = sim_points[middle_index].vis_point * world_to_cam_tm;

					m_CurrentPointList->Add (camera_tm);
					m_TotalPoints ++;
				}
			}

			//
			//	Start working on points over another node.
			//
			start_index = index;
			current_node = sim_points[index].node;
		}
	}

	// Free our 'sim-points' array
	delete [] sim_points;
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Set_Current_Node
//
/////////////////////////////////////////////////////////////////////////
void
VisPointGeneratorClass::Set_Current_Node (NodeClass *node)
{
	m_CurrentNode = node;
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Post_Process_Nodes
//
/////////////////////////////////////////////////////////////////////////
void
VisPointGeneratorClass::Post_Process_Nodes (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Add_Manual_Nodes
//
/////////////////////////////////////////////////////////////////////////
void
VisPointGeneratorClass::Add_Manual_Nodes (void)
{
	/*VisPointNodeClass *vis_point = NULL;
	for (	vis_point = (VisPointNodeClass *)NodeMgrClass::Get_First (NODE_TYPE_VIS_POINT);
			vis_point != NULL;
			vis_point = (VisPointNodeClass *)NodeMgrClass::Get_Next (vis_point, NODE_TYPE_VIS_POINT))
	{
		//
		// Convert the object transform to a camera transform
		//
		Matrix3D	transform = vis_point->Get_Transform ();
		Matrix3D cam_transform (Vector3 (0, -1, 0), Vector3 (0, 0, 1), Vector3 (-1, 0, 0), Vector3 (0, 0, 0));
		Matrix3D new_transform = transform * cam_transform;

		//
		//	Allocate a new point list for this point
		//
		m_CurrentPointList = new VisPointListClass;
		m_CurrentPointList->transform		= transform;
		m_CurrentPointList->sample_point	= vis_point->Get_Vis_Tile_Location ();
		m_PointList.Add (m_CurrentPointList);
		m_TotalPoints ++;
	}*/

	return ;
}

