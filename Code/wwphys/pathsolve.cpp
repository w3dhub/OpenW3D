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
 *                 Project Name : wwphys																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/pathsolve.cpp           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/10/01 12:40p                                             $*
 *                                                                                             *
 *                    $Revision:: 20                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "pathsolve.h"

#include <windows.h>
#include "Pathfind.h"
#include "PathfindPortal.h"
#include "PathNode.h"
#include "PathDebugPlotter.h"
#include "PathObject.h"
#include "accessiblephys.h"
#include "chunkio.h"
#include "pathmgr.h"
#include "wwmemlog.h"
#include "systimer.h"
#include <algorithm>



////////////////////////////////////////////////////////////////
//	Save/Load constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES				= 0x11040604,
};

enum
{
	VARID_STARTPOS					= 1,
	VARID_DESTPOS,
	VARID_START_SECTOR,
	VARID_DEST_SECTOR,
	VARID_PATH_OBJECT,
	VARID_OLD_PTR,
	VARID_PRIORITY
};


///////////////////////////////////////////////////////////////////////////
//	Static member initialization
///////////////////////////////////////////////////////////////////////////
int64_t PathSolveClass::_TicksPerMilliSec = 0;


///////////////////////////////////////////////////////////////////////////
//	Local inlines
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//
//	Get_Time
//
///////////////////////////////////////////////////////////////////////////
static inline int64_t
Get_Time (void)
{
	int64_t curr_time = 0;
	::QueryPerformanceCounter ((LARGE_INTEGER *)&curr_time);
	return curr_time;
}


///////////////////////////////////////////////////////////////////////////
//
//	Shrink_Box
//
///////////////////////////////////////////////////////////////////////////
static inline void
Shrink_Box (AABoxClass &box, float width)
{
	if (box.Extent.X > width) {
		box.Extent.X -= width;
	} else {
		box.Extent.X = 0.25F;
	}

	if (box.Extent.Y > width) {
		box.Extent.Y -= width;
	} else {
		box.Extent.Y = 0.25F;
	}

	return ;
}


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
	temp_point.X = std::max (temp_point.X, box.Center.X - box.Extent.X);
	temp_point.Y = std::max (temp_point.Y, box.Center.Y - box.Extent.Y);
	temp_point.Z = std::max (temp_point.Z, box.Center.Z - box.Extent.Z);
	temp_point.X = std::min (temp_point.X, box.Center.X + box.Extent.X);
	temp_point.Y = std::min (temp_point.Y, box.Center.Y + box.Extent.Y);
	temp_point.Z = std::min (temp_point.Z, box.Center.Z + box.Extent.Z);

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


///////////////////////////////////////////////////////////////////////////
//
//	Clip_Point
//
///////////////////////////////////////////////////////////////////////////
static inline void
Clip_Point (Vector3 *point, const AABoxClass &box, float edge_dist)
{
	Vector3 temp_point = *point;

	AABoxClass clip_box = box;
	if (clip_box.Extent.X > edge_dist) {
		clip_box.Extent.X -= edge_dist;
	} else {
		clip_box.Extent.X = 0;
	}

	if (clip_box.Extent.Y > edge_dist) {
		clip_box.Extent.Y -= edge_dist;
	} else {
		clip_box.Extent.Y = 0;
	}

	//
	//	Clip the temporary point
	//
	temp_point.X = std::max (temp_point.X, clip_box.Center.X - clip_box.Extent.X);
	temp_point.Y = std::max (temp_point.Y, clip_box.Center.Y - clip_box.Extent.Y);
	temp_point.Z = std::max (temp_point.Z, clip_box.Center.Z - clip_box.Extent.Z);
	temp_point.X = std::min (temp_point.X, clip_box.Center.X + clip_box.Extent.X);
	temp_point.Y = std::min (temp_point.Y, clip_box.Center.Y + clip_box.Extent.Y);
	temp_point.Z = std::min (temp_point.Z, clip_box.Center.Z + clip_box.Extent.Z);

	//
	//	Pass the new point back to the caller
	//
	(*point) = temp_point;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	PathSolveClass
//
///////////////////////////////////////////////////////////////////////////
PathSolveClass::PathSolveClass (void)
	:	m_StartPos (0, 0, 0),
		m_DestPos (0, 0, 0),
		m_StartSector (NULL),
		m_DestSector (NULL),
		m_CompletedNode (NULL),
		m_State (ERROR_INVALID_START_POS),
		m_BinaryHeap (10000),
		m_Priority (0.5F),
		m_BirthTime (0)
{
	//
	//	Determine how many performance-counter ticks
	// per millisecond we will get.
	//
	if (_TicksPerMilliSec == 0) {
		::QueryPerformanceFrequency ((LARGE_INTEGER *)&_TicksPerMilliSec);
		_TicksPerMilliSec /= 1000;
	}

	m_NodeList.Set_Growth_Step (500);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	PathSolveClass
//
///////////////////////////////////////////////////////////////////////////
PathSolveClass::PathSolveClass (const Vector3 &start, const Vector3 &dest)
	:	m_StartPos (0, 0, 0),
		m_DestPos (0, 0, 0),
		m_StartSector (NULL),
		m_DestSector (NULL),
		m_CompletedNode (NULL),
		m_State (ERROR_INVALID_START_POS),
		m_BinaryHeap (10000),
		m_Priority (0.5F),
		m_BirthTime (0)
{
	//
	//	Determine how many performance-counter ticks
	// per millisecond we will get.
	//
	if (_TicksPerMilliSec == 0) {
		::QueryPerformanceFrequency ((LARGE_INTEGER *)&_TicksPerMilliSec);
		_TicksPerMilliSec /= 1000;
	}

	Reset (start, dest);
	m_NodeList.Set_Growth_Step (500);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	~PathSolveClass
//
///////////////////////////////////////////////////////////////////////////
PathSolveClass::~PathSolveClass (void)
{
	Reset_Lists ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Reset
//
///////////////////////////////////////////////////////////////////////////
PathSolveClass::STATE_DESC
PathSolveClass::Reset
(
	const Vector3 &start,
	const Vector3 &dest,
	float				sector_fudge
)
{
	WWMEMLOG(MEM_PATHFIND);
	Reset_Lists ();

	m_StartPos		= start;
	m_DestPos		= dest;
	Initialize (sector_fudge);
	return m_State;
}


///////////////////////////////////////////////////////////////////////////
//
//	Begin_Distributed_Solve
//
///////////////////////////////////////////////////////////////////////////
/*void
PathSolveClass::Begin_Distributed_Solve (void)
{
	for (int index = 0; index < m_NodeList.Count (); index ++) {
		PathNodeClass *node = m_NodeList[index];
		if (node != NULL) {
			node->Reconnect_To_Portal ();
		}
	}

	return ;
}*/


///////////////////////////////////////////////////////////////////////////
//
//	Unlink_Pathfind_Hooks
//
///////////////////////////////////////////////////////////////////////////
void
PathSolveClass::Unlink_Pathfind_Hooks (void)
{
	for (int index = 0; index < m_NodeList.Count (); index ++) {
		m_NodeList[index]->Disconnect_From_Portal ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Resolve_Path
//
///////////////////////////////////////////////////////////////////////////
void
PathSolveClass::Resolve_Path (unsigned int milliseconds)
{
	WWMEMLOG(MEM_PATHFIND);

	int64_t start_time	= Get_Time ();
	int64_t end_time		= start_time + (((int64_t)milliseconds) * _TicksPerMilliSec);

	int iterations = 0;
	//Begin_Distributed_Solve ();

	do
	{
		//
		//	Pop the least cost path 'node' from the open list and process
		// all of its adjacent sectors.
		//
		PathNodeClass *node = (PathNodeClass *)m_BinaryHeap.Remove_Min ();

		//
		//	Have we found our path?
		//
		if (node == NULL) {
			m_State = ERROR_NO_PATH;
		} else  if (node->Peek_Sector () == m_DestSector) {
			m_State = SOLVED_PATH;

			//
			//	Record this path as 'final'
			//
			m_CompletedNode = node;
			m_CompletedNode->Add_Ref ();

			//
			//	Mark all the nodes that are on the final path
			//
			for (	PathNodeClass *path_node = m_CompletedNode;
					path_node != NULL;
					path_node = path_node->Peek_Parent_Node ())
			{
				path_node->On_Final_Path (true);
			}

			//
			//	'Straighten' the path as much as possible
			//
			Post_Process_Path ();

		} else {
			Process_Portals (node);
		}

		iterations ++;

	//
	//
	//	Keep going until we've either found the path, or we ran out of time.
	//
	} while ((m_State == THINKING) && (Get_Time () < end_time));

	//End_Distributed_Solve ();

	//WWDebug_Printf ("Time spent in pathfind: %d 1/100 millis, loops = %d, finished = %d.\r\n", (unsigned int)((Get_Time () - start_time) / (_TicksPerMilliSec/100)), iterations, (int)(m_State == TRAVERSING_PATH));
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Timestep
//
///////////////////////////////////////////////////////////////////////////
PathSolveClass::STATE_DESC
PathSolveClass::Timestep (unsigned int milliseconds)
{

	//
	//	Spend some time trying to resolve the path
	//
	if (m_State == THINKING) {
		Resolve_Path (milliseconds);
	}

	return m_State;
}


///////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
///////////////////////////////////////////////////////////////////////////
void
PathSolveClass::Initialize (float sector_fudge)
{
	m_State				= THINKING;
	m_CompletedNode	= NULL;
	m_BirthTime			= TIMEGETTIME ();

	//
	//	Lookup the start and destination sectors
	//
	m_StartSector	= PathfindClass::Get_Instance ()->Find_Sector (m_StartPos, sector_fudge);
	m_DestSector	= PathfindClass::Get_Instance ()->Find_Sector (m_DestPos, sector_fudge);

	//
	//	Sanity check, are the starting and ending points
	// inside of pathfind sectors?
	//
	if (m_StartSector == NULL) {
		m_State = ERROR_INVALID_START_POS;
	} else if (m_DestSector == NULL) {
		m_State = ERROR_INVALID_DEST_POS;
	} else {

		//
		//	Clip the start and destination points inside the sectors if necessary.
		//
		//if (sector_fudge > 0) {
		//::Clip_Point (&m_StartPos, m_StartSector->Get_Bounding_Box (), m_PathObject.Get_Width ());
		::Clip_Point (&m_StartPos, m_StartSector->Get_Bounding_Box ());
		::Clip_Point (&m_DestPos, m_DestSector->Get_Bounding_Box ());
		//}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Process_Initial_Sector
//
///////////////////////////////////////////////////////////////////////////
void
PathSolveClass::Process_Initial_Sector (void)
{
	if (m_StartSector == NULL) {
		return ;
	}

	//
	//	Create a set of path 'nodes' that represent all the portals of the
	//	starting sector.
	//
	int index = m_StartSector->Get_Portal_Count ();
	while (index --) {
		PathfindPortalClass *portal = m_StartSector->Peek_Portal (index);
		if (portal != NULL) {

			AABoxClass portal_box;
			portal->Get_Bounding_Box (portal_box);

			//
			//	Find where we should enter the portal...
			//
			Vector3 dest_point = m_StartPos;
			::Clip_Point (&dest_point, portal_box, m_PathObject.Get_Width ());
			dest_point.Z = portal_box.Center.Z - (portal_box.Extent.Z - 0.1F);

			//
			//	Determine how we should be facing when we enter the portal.
			//
			Vector3 x_vector = dest_point - m_StartPos;
			Vector3 y_vector;
			Vector3 z_vector (0, 0, 1);
			x_vector.Normalize ();
			y_vector = Vector3::Cross_Product (x_vector, z_vector);
			Matrix3D ending_tm (x_vector, y_vector, z_vector, dest_point);

			//
			//	Sumbit a node for this portal
			//
			if (Does_Object_Have_Access_To_Portal (portal)) {
				Submit_Node (	0,
									NULL,
									portal,
									portal->Peek_Dest_Sector (m_StartSector),
									Matrix3D (m_StartPos),
									ending_tm);
			}
		}
	}

	if (m_StartSector == m_DestSector) {

		//
		//	If the start and destination are the same
		// sector, then we can just beeline.
		//
		m_State = SOLVED_PATH;
		m_Path.Delete_All ();
		m_Path.Add (PathDataStruct (NULL, m_StartPos));
		m_Path.Add (PathDataStruct (NULL, m_DestPos));

	} else {
		m_State = THINKING;
	}

	return ;
}


//
//
//	Steps:
//
//
//		- For each portal in the current sector:
//			- Can the object drive to that portal?
//				- Yes: Can the object fit in the portal?
//					- Yes: Can the object fit in the dest sector?
//						- Yes:  Record the node, stick it on the open list.
//						- No: Punt
//					- No: Punt
//				- No: Punt
//
//
//
//


///////////////////////////////////////////////////////////////////////////
//
//	Find_Tangent_Angle
//
///////////////////////////////////////////////////////////////////////////
float
Find_Tangent_Angle
(
	const Vector2 &	center,
	float					radius,
	const Vector2 &	point,
	bool					clockwise
)
{
	float angle = 0;

	//
	//	Calculate the distance from the point to the center of the circle
	//
	float delta_x = point.X - center.X;
	float delta_y = point.Y - center.Y;
	float dist = ::sqrt (delta_x * delta_x + delta_y * delta_y);

	//
	//	Determine the offset angle (from the line between the point and center)
	// where the 2 tangent points lie.
	//
	float angle_offset	= WWMath::Acos (radius / dist);
	float base_angle		= WWMath::Atan2 (delta_x, -delta_y);
	base_angle = WWMath::Wrap (base_angle, 0, WWMATH_PI * 2);

	//
	//	Return which ever tangent point we would come across first, depending
	// on our orientation
	//
	if (clockwise) {
		angle = (WWMATH_PI * 3) - (base_angle + angle_offset);
	} else {
		angle = base_angle - angle_offset;
	}

	angle = WWMath::Wrap (angle, 0, WWMATH_PI * 2);
	return angle;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Turn_Arc_Center
//
///////////////////////////////////////////////////////////////////////////
void
Get_Turn_Arc_Center
(
	float			wheel_offset,
	float			turn_radius,
	bool			left_turn,
	Vector2 *	arc_center

)
{
	arc_center->X = wheel_offset;

	if (left_turn) {
		arc_center->Y = turn_radius;
	} else {
		arc_center->Y = -turn_radius;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Find_Adjacent_Portals
//
///////////////////////////////////////////////////////////////////////////
void
Find_Adjacent_Portals
(
	PathfindSectorClass *sector,
	PathfindPortalClass *portal,
	PathfindSectorClass * /* dest_sector */,
	PathfindPortalClass **portal1,
	PathfindPortalClass **portal2
)
{
	AABoxClass portal_box;
	portal->Get_Bounding_Box (portal_box);

	float x_value1 = 0;
	float x_value2 = 0;
	float y_value1 = 0;
	float y_value2 = 0;
	bool compare_x = false;

	if (portal_box.Extent.X > portal_box.Extent.Y) {

		x_value1 = portal_box.Center.X - portal_box.Extent.X;
		x_value2 = portal_box.Center.X + portal_box.Extent.X;
		y_value1 = portal_box.Center.Y;
		y_value2 = portal_box.Center.Y;
		compare_x = true;

	} else {

		x_value1 = portal_box.Center.X;
		x_value2 = portal_box.Center.X;
		y_value1 = portal_box.Center.Y - portal_box.Extent.Y;
		y_value2 = portal_box.Center.Y + portal_box.Extent.Y;
		compare_x = false;
	}


	int index = sector->Get_Portal_Count ();
	while (index --) {
		PathfindPortalClass *curr_portal = sector->Peek_Portal (index);
		if (curr_portal && curr_portal != portal) {

			AABoxClass curr_box;
			curr_portal->Get_Bounding_Box (curr_box);

			if (compare_x) {
				float curr_x1	= curr_box.Center.X + curr_box.Extent.X;
				float curr_x2	= curr_box.Center.X - curr_box.Extent.X;
				float curr_y	= curr_box.Center.Y;

				//
				//	Is this the y-value we are looking for?
				//
				if (WWMath::Fabs (curr_y - y_value1) <= WWMATH_EPSILON) {

					//
					//	Is this either the left or right adjacent portal?
					//
					if (WWMath::Fabs (curr_x1 - x_value1) <= WWMATH_EPSILON) {
						(*portal1) = curr_portal;
					} else if (WWMath::Fabs (curr_x2 - x_value2) <= WWMATH_EPSILON) {
						(*portal2) = curr_portal;
					}
				}

			} else {

				float curr_y1	= curr_box.Center.Y + curr_box.Extent.Y;
				float curr_y2	= curr_box.Center.Y - curr_box.Extent.Y;
				float curr_x	= curr_box.Center.X;

				//
				//	Is this the x-value we are looking for?
				//
				if (WWMath::Fabs (curr_x - x_value1) <= WWMATH_EPSILON) {

					//
					//	Is this either the up or down adjacent portal?
					//
					if (WWMath::Fabs (curr_y1 - y_value1) <= WWMATH_EPSILON) {
						(*portal1) = curr_portal;
					} else if (WWMath::Fabs (curr_y2 - y_value2) <= WWMATH_EPSILON) {
						(*portal2) = curr_portal;
					}
				}
			}
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Does_Object_Fit_Through_Portal
//
///////////////////////////////////////////////////////////////////////////
bool
Does_Object_Fit_Through_Portal
(
	const PathObjectClass &	object,
	PathfindSectorClass *	sector,
	PathfindPortalClass *	portal
)
{
	bool retval = false;

	AABoxClass portal_box;
	AABoxClass obj_box;

	portal->Get_Bounding_Box (portal_box);
	object.Get_Collision_Box (obj_box);

	float min_pos = 0;
	float max_pos = 0;
	if (portal_box.Extent.X > portal_box.Extent.Y) {
		min_pos = portal_box.Center.X - portal_box.Extent.X;
		max_pos = portal_box.Center.X + portal_box.Extent.X;
	} else {
		min_pos = portal_box.Center.Y - portal_box.Extent.Y;
		max_pos = portal_box.Center.Y + portal_box.Extent.Y;
	}

	//
	//	Can we fit through this portal?
	//
	if (object.Get_Width () <= (max_pos - min_pos)) {
		retval = true;
	} else {

		//
		//	This object could not fit through the given portal, so try to
		// find any adjacent portals that might pass through a different
		// sector but would end up in the correct sector.  We will combine
		// the total length of these portals and see if the object fits...
		//

		PathfindSectorClass *dest_sector = portal->Peek_Dest_Sector (sector);

		//
		//	Try to find two adjacent portals which can make it to the destination
		//	sector...
		//
		PathfindPortalClass *portal1 = NULL;
		PathfindPortalClass *portal2 = NULL;
		::Find_Adjacent_Portals (sector, portal, dest_sector, &portal1, &portal2);

		//
		//	If the vehicle can fit in through the combined portals, then we will
		// allow the vehicle to pass...
		//
		if (portal1 != NULL || portal2 != NULL) {

			//
			//	We assume portal1 is 'less-then' the test portal, so use its
			// position and size to determine the lower bounds of our extended portal.
			//
			if (portal1 != NULL) {

				AABoxClass temp_box;
				portal1->Get_Bounding_Box (temp_box);

				if (portal_box.Extent.X > portal_box.Extent.Y) {
					min_pos = temp_box.Center.X - temp_box.Extent.X;
				} else {
					min_pos = temp_box.Center.Y - temp_box.Extent.Y;
				}
			}

			//
			//	We assume portal2 is 'greater-then' the test portal, so use its
			// position and size to determine the upper bounds of our extended portal.
			//
			if (portal2 != NULL) {

				AABoxClass temp_box;
				portal2->Get_Bounding_Box (temp_box);

				if (portal_box.Extent.X > portal_box.Extent.Y) {
					max_pos = temp_box.Center.X + temp_box.Extent.X;
				} else {
					max_pos = temp_box.Center.Y + temp_box.Extent.Y;
				}
			}

			//
			//	Can we fit through this extended portal?
			//
			if (object.Get_Width () <= (max_pos - min_pos)) {
				retval = true;
			}
		}
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Does_Object_Have_Access_To_Portal
//
///////////////////////////////////////////////////////////////////////////
bool
PathSolveClass::Does_Object_Have_Access_To_Portal (PathfindPortalClass *portal)
{
	bool retval = true;

	//
	//	Does this portal interact with some sort of mechanism
	//
	PathfindActionPortalClass *action_portal = portal->As_PathfindActionPortalClass ();
	if (	action_portal != NULL &&
			action_portal->Get_Action_Type () == PathClass::ACTION_MECHANISM)
	{
		//
		//	Lookup the mechanism this portal uses
		//
		uint32 mechanism_id			= action_portal->Get_Mechanism_ID ();
		StaticPhysClass *mechanism	= PhysicsSceneClass::Get_Instance ()->Find_Static_Object (mechanism_id);
		if (mechanism != NULL) {
			AccessiblePhysClass *accessible_obj = mechanism->As_AccessiblePhysClass ();

			//
			//	See if we can unlock this mechanism
			//
			if (accessible_obj != NULL && accessible_obj->Get_Lock_Code () != 0) {
				retval = accessible_obj->Can_Unlock (m_PathObject.Get_Key_Ring ());
			}
		}
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Can_Object_Go_Through_Portal
//
///////////////////////////////////////////////////////////////////////////
bool
PathSolveClass::Can_Object_Go_Through_Portal
(
	const Matrix3D &			current_tm,
	PathfindSectorClass *	sector,
	PathfindPortalClass *	portal,
	Matrix3D *					ending_tm
)
{
	bool retval = false;

	//
	//	Calculate the size of the portal we are trying
	// to pass through
	//
	AABoxClass portal_box;
	portal->Get_Bounding_Box (portal_box);

	//
	//	Can we fit through this portal?
	//
	if (	portal->Does_Size_Matter () == false ||
			::Does_Object_Fit_Through_Portal (m_PathObject, sector, portal))
	{
		//
		//	Find where we should enter the portal...
		//
		Vector3 curr_pos		= current_tm.Get_Translation ();
		Vector3 dest_point	= curr_pos;

		if (portal->As_PathfindActionPortalClass () != NULL) {
			dest_point = portal_box.Center;
			//::Clip_Point (&dest_point, portal_box, m_PathObject.Get_Width ());
		} else {
			::Clip_Point (&dest_point, portal_box, m_PathObject.Get_Width () * 2);
		}

		dest_point.Z = portal_box.Center.Z - (portal_box.Extent.Z - 0.1F);

		//
		//	Calculate a complete transform for the portal enter position
		//
		Vector3 x_vector = dest_point - curr_pos;
		Vector3 y_vector;
		Vector3 z_vector (0, 0, 1);
		x_vector.Normalize ();
		y_vector = Vector3::Cross_Product (x_vector, z_vector);

		ending_tm->Set (x_vector, y_vector, z_vector, dest_point);
		retval = true;
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Can_Object_Enter_Portal
//
///////////////////////////////////////////////////////////////////////////
bool
Can_Object_Enter_Portal
(
	const AABoxClass &		portal_box,
	const PathObjectClass &	object,
	const Vector3 &			direction_vector
)
{
	bool retval = false;

	AABoxClass obj_box;
	object.Get_Collision_Box (obj_box);

	//
	//	Can this object be rotated about Z?
	//
	if (object.Is_Flag_Set (PathObjectClass::CAN_BOX_ROTATE)) {

		//
		//	Build a rotation matrix from the direction vector
		//
		Vector3 x_vector = ::Normalize (direction_vector);
		Vector3 z_vector (0, 0, 1);
		Vector3 y_vector = Vector3::Cross_Product (x_vector, z_vector);
		Matrix3 tm (x_vector, y_vector, z_vector);

		//
		//	Rotate the edges of the box (toss the z component, we don't care)
		//
		Vector3 v1 (obj_box.Extent.X, -obj_box.Extent.Y, 0);
		Vector3 v2 (obj_box.Extent.X, obj_box.Extent.Y, 0);
		Vector3 v3 (-obj_box.Extent.X, obj_box.Extent.Y, 0);
		Vector3 v4 (-obj_box.Extent.X, -obj_box.Extent.Y, 0);
		v1 = tm * v1;
		v2 = tm * v2;
		v3 = tm * v3;
		v4 = tm * v4;

		//
		//	Determine the new axis-aligned extents from this rotated box
		//
		float max_x = std::max (WWMath::Fabs (v1.X), WWMath::Fabs (v2.X));
		max_x = std::max (max_x, WWMath::Fabs (v3.X));
		max_x = std::max (max_x, WWMath::Fabs (v4.X));

		float max_y = std::max (WWMath::Fabs (v1.Y), WWMath::Fabs (v2.Y));
		max_y = std::max (max_y, WWMath::Fabs (v3.Y));
		max_y = std::max (max_y, WWMath::Fabs (v4.Y));

		obj_box.Extent.X = max_x / 2;
		obj_box.Extent.Y = max_y / 2;
	}

	//
	//	Portal boxes are generally long and very skinny.
	// So skinny in fact they are conceptually planes.
	// We find out which direction they are skinny in
	// and simulate 'shoving' the object through in
	// that direction.
	//
	if (portal_box.Extent.X > 	portal_box.Extent.Y) {
		retval = (obj_box.Extent.X < portal_box.Extent.X);
	} else {
		retval = (obj_box.Extent.Y < portal_box.Extent.Y);
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Process_Portals
//
///////////////////////////////////////////////////////////////////////////
void
PathSolveClass::Process_Portals (PathNodeClass *node)
{
	PathfindSectorClass *sector		= node->Peek_Sector ();
	PathfindPortalClass *last_portal = node->Peek_Portal ();
	const Matrix3D &current_tm			= node->Get_Transform ();

	//
	//	Loop over all the portals that this sector has access to
	//
	for (int index = 0; index < sector->Get_Portal_Count (); index ++) {
		PathfindPortalClass *portal = sector->Peek_Portal (index);

		//
		//	Don't process any portals we can't get to from the current portal
		//
		if (sector->Can_Access_Portal (last_portal, portal)) {

			//
			//	Get this portal's destination
			//
			PathfindSectorClass *dest_sector = portal->Peek_Dest_Sector (sector);
			if (dest_sector != NULL) {

				//
				//	Determine if we can pass through this portal, and if so where
				// we should pass through...
				//
				Matrix3D ending_tm (1);
				if (	Does_Object_Have_Access_To_Portal (portal) &&
						Can_Object_Go_Through_Portal (current_tm, sector, portal, &ending_tm))
				{
					//
					//	Submit a new node for the destination sector
					//
					Submit_Node (node->Get_Traversal_Cost (), node, portal,
						dest_sector, current_tm, ending_tm);
				}
			}
		}
	}

	if (last_portal != NULL) {
		last_portal->m_ClosedListPtr = node;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Calculate_Heuristic_Cost
//
///////////////////////////////////////////////////////////////////////////
float
Calculate_Heuristic_Cost
(
	const PathObjectClass &		object,
	const PathfindPortalClass &portal,
	const PathfindSectorClass &sector,
	const Vector3 &				curr_pos,
	const Vector3 &				dest_pos
)
{
	float cost = (curr_pos - dest_pos).Length () * 0.9F;

	if (sector.As_PathfindWaypathSectorClass () != NULL) {

		//
		//	Make waypath sectors slightly more appealing
		//
		cost = cost * 0.5F;

	} else if (object.Is_Flag_Set (PathObjectClass::IS_VEHICLE)) {

		//
		//	Do some special case heuristics for vehicles
		//

		//
		//	Make one-way portals very expensive for vehicles
		//
		if (portal.Does_Size_Matter () && portal.Is_Two_Way_Portal () == false) {
			cost = cost * 2.0F;
		}

		AABoxClass sector_box	= sector.Get_Bounding_Box ();
		float sector_area			= (sector_box.Extent.X * sector_box.Extent.Y);

		//AABoxClass object_box;
		//object.Get_Collision_Box (object_box);
		//float	object_area			= (object_box.Extent.X * object_box.Extent.Y);

		float	object_area			= 50.0F;
		float factor = (object_area / sector_area);
		factor = std::max (factor, 0.01F);
		factor = std::min (factor, 100.0F);

		cost = cost * factor;
	}

	return cost;
}


///////////////////////////////////////////////////////////////////////////
//
//	Submit_Node
//
///////////////////////////////////////////////////////////////////////////
void
PathSolveClass::Submit_Node
(
	float							traversal_cost,
	PathNodeClass *			current_node,
	PathfindPortalClass *	portal,
	PathfindSectorClass *	dest_sector,
	const Matrix3D &			current_tm,
	const Matrix3D &			ending_tm
)
{
	const Vector3 &current_pos = current_tm.Get_Translation ();
	const Vector3 &dest_pos		= ending_tm.Get_Translation ();

	//
	//	Calculate the total traversal cost for this path.
	//
	Vector3 dest_vector	= current_pos - dest_pos;
	dest_vector.Z			= 0;
	float dest_dist		= dest_vector.Length ();

	float current_traversal_cost	= traversal_cost + dest_dist;

	//
	//	Calculate the heuristic for this portal
	//
	float heuristic_cost = ::Calculate_Heuristic_Cost (m_PathObject, *portal, *dest_sector,
										dest_pos, m_DestPos);

	//
	//	Try to keep vehicles from taking one-way portals
	//
	if (	m_PathObject.Is_Flag_Set (PathObjectClass::IS_VEHICLE) &&
			portal->Is_Two_Way_Portal () == false)
	{
		current_traversal_cost += dest_dist * 2.0F;
	}

	//
	//	Is this sector already in the open list?
	//
	int open_index = portal->Get_Heap_Location ();
	if (open_index > 0) {
		PathNodeClass *open_version = (PathNodeClass *)m_BinaryHeap.Peek_Node (open_index);

		//
		//	If the traversal cost is lower from our current 'path', then
		// remove the old path and re-insert the new path.
		//
		if (current_traversal_cost < open_version->Get_Traversal_Cost ()) {

			open_version->Set_Sector (dest_sector);
			open_version->Set_Parent_Node (current_node);
			open_version->Set_Traversal_Cost (current_traversal_cost);
			open_version->Set_Transform (ending_tm);
			open_version->Set_Heuristic_Cost (heuristic_cost);

			m_BinaryHeap.Percolate_Up (open_index);
		}

	} else {

		//
		//	Is this sector already in the closed list?
		//
		if (portal->m_ClosedListPtr != NULL) {
			PathNodeClass *closed_version	= portal->m_ClosedListPtr;

			//
			//	If the traversal cost is lower from our current 'path', then
			// remove the old path and re-insert the new path.
			//
			if (current_traversal_cost < closed_version->Get_Traversal_Cost ()) {

				portal->m_ClosedListPtr = NULL;
				closed_version->Set_Sector (dest_sector);
				closed_version->Set_Parent_Node (current_node);
				closed_version->Set_Traversal_Cost (current_traversal_cost);
				closed_version->Set_Transform (ending_tm);
				closed_version->Set_Heuristic_Cost (heuristic_cost);

				m_BinaryHeap.Insert (closed_version);
			}

		} else {

			//
			//	Create a new path 'node' that represents that path from start
			// up until this sector.
			//
			PathNodeClass *new_node = new PathNodeClass;
			new_node->Set_Sector (dest_sector);
			new_node->Set_Parent_Node (current_node);
			new_node->Set_Portal (portal);

			//
			//	Calculate the path's total traversal cost
			//
			new_node->Set_Traversal_Cost (current_traversal_cost);
			new_node->Set_Transform (ending_tm);
			new_node->Set_Heuristic_Cost (heuristic_cost);

			//
			//	Insert this node into the open list
			//
			m_BinaryHeap.Insert (new_node);

			//
			//	Keep track of this node's pointer (for cleanup)
			//
			m_NodeList.Add (new_node);
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Reset_Lists
//
///////////////////////////////////////////////////////////////////////////
void
PathSolveClass::Reset_Lists (void)
{
	//
	//	Ensure we unlink our data before we destroy it...
	//
	if (PathMgrClass::Peek_Active_Path () == this) {
		Unlink_Pathfind_Hooks ();
	}

	REF_PTR_RELEASE (m_CompletedNode);

	//
	//	Free all our nodes
	//
	for (int index = 0; index < m_NodeList.Count (); index ++) {
		PathNodeClass *node = m_NodeList[index];
		REF_PTR_RELEASE (node);
	}

	m_BinaryHeap.Flush_Array ();
	m_NodeList.Reset_Active ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Does_Line_Go_Through_Portal
//
/////////////////////////////////////////////////////////////////////////////////
bool
PathSolveClass::Does_Line_Go_Through_Portal
(
	const Vector3 &		start,
	const Vector3 &		end,
	const AABoxClass &	box
)
{
	float delta_x = end.X - start.X;
	float delta_y = end.Y - start.Y;
	float percent = 1000;

	if ((box.Extent.X <= box.Extent.Y) && (delta_x != 0)) {

		//
		//	Calculate where the line intersects the X plane
		//
		percent = (box.Center.X - start.X) / delta_x;

	} else if ((box.Extent.Y <= box.Extent.X) && (delta_y != 0)) {

		//
		//	Calculate where the line intersects the Y plane
		//
		percent = (box.Center.Y - start.Y) / delta_y;
	}

	//
	//	Calculate the point where the line intersects one of the
	// planes of the box.
	//
	Vector3 dest_point = start + ((end - start) * percent);

	//
	//	Check to make sure this point isn't outside the bounds of the
	// box, if it is outside the bounds, then the line DOES NOT pass
	// through the portal.
	//
	bool retval = true;
	if ((box.Center.X + box.Extent.X) < dest_point.X) retval = false;
	if ((box.Center.Y + box.Extent.Y) < dest_point.Y) retval = false;
	if ((box.Center.X - box.Extent.X) > dest_point.X) retval = false;
	if ((box.Center.Y - box.Extent.Y) > dest_point.Y) retval = false;

	return retval;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Relax_Line
//
/////////////////////////////////////////////////////////////////////////////////
Vector3
PathSolveClass::Relax_Line (const Vector3 &start, const Vector3 &end, const AABoxClass &box)
{
	Vector3 result = box.Center;

	float delta_x = end.X - start.X;
	float delta_y = end.Y - start.Y;
	float percent = 1000;

	if ((box.Extent.X < box.Extent.Y) && (delta_x != 0)) {

		//
		//	Calculate where the line intersects the X plane
		//
		percent = (box.Center.X - start.X) / delta_x;

	} else if ((box.Extent.Y < box.Extent.X) && (delta_y != 0)) {

		//
		//	Calculate where the line intersects the Y plane
		//
		percent = (box.Center.Y - start.Y) / delta_y;
	}

	//
	//	Calculate the point where the line intersects one of the
	// planes of the box.
	//
	//if (percent >= 0 && percent < 1.0F) {
		result = start + ((end - start) * percent);

		Clip_Point (&result, box, m_PathObject.Get_Width () * 3.0F);
		//result = box.Center;
		//result.Z = box.Center.Z - (box.Extent.Z - 0.1F);

	/*} else {
		int test = 0;
	}*/

	return result;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Check_Line
//
//	Note:  The purpose of this function is to determine if the unit's collision
// box will be jutting outside the sector at the point where the path begins
// to enter the portal.
//
//
/////////////////////////////////////////////////////////////////////////////////
bool
Check_Line
(
	const Vector3 &		p0,
	const Vector3 &		p1,
	const AABoxClass &	sector,
	const AABoxClass &	portal,
	float						width,
	Vector3 *				result
)
{
	bool retval = false;

	if (	WWMath::Fabs (portal.Center.X - sector.Center.X + sector.Extent.X) < WWMATH_EPSILON ||
			WWMath::Fabs (portal.Center.X - sector.Center.X - sector.Extent.X) < WWMATH_EPSILON)
	{
		float y_val0 = portal.Center.Y - portal.Extent.Y;
		float y_val1 = portal.Center.Y + portal.Extent.Y;

		bool do_it = false;
		float y_val = 0;

		if (	(p0.Y < y_val0 && p1.Y > y_val0) ||
				(p0.Y > y_val0 && p1.Y < y_val0))
		{
			do_it = true;
			y_val = y_val0;

		} else if (	(p0.Y < y_val1 && p1.Y > y_val1) ||
						(p0.Y > y_val1 && p1.Y < y_val1))
		{
			do_it = true;
			y_val = y_val1;
		}

		if (do_it) {
			float percent	= (y_val - p0.Y) / (p1.Y - p0.Y);
			Vector3 pos		= p0 + (p1 - p0) * percent;

			if (pos.X + width > (sector.Center.X + sector.Extent.X)) {

				(*result) = pos;
				result->X = (sector.Center.X + sector.Extent.X) - width;
				result->X = std::max (result->X, sector.Center.X);
				retval = true;

			} else if (pos.X - width < (sector.Center.X - sector.Extent.X)) {

				(*result) = pos;
				result->X = (sector.Center.X - sector.Extent.X) + width;
				result->X = std::min (result->X, sector.Center.X);
				retval = true;
			}
		}

	} else if (	WWMath::Fabs (portal.Center.Y - sector.Center.Y + sector.Extent.Y) < WWMATH_EPSILON ||
					WWMath::Fabs (portal.Center.Y - sector.Center.Y - sector.Extent.Y) < WWMATH_EPSILON)
	{

		float x_val0 = portal.Center.X - portal.Extent.X;
		float x_val1 = portal.Center.X + portal.Extent.X;

		bool do_it = false;
		float x_val = 0;

		if (	(p0.X < x_val0 && p1.X > x_val0) ||
				(p0.X > x_val0 && p1.X < x_val0))
		{
			do_it = true;
			x_val = x_val0;

		} else if (	(p0.X < x_val1 && p1.X > x_val1) ||
						(p0.X > x_val1 && p1.X < x_val1))
		{
			do_it = true;
			x_val = x_val1;
		}

		if (do_it) {
			float percent	= (x_val - p0.X) / (p1.X - p0.X);
			Vector3 pos		= p0 + (p1 - p0) * percent;

			if (pos.Y + width > (sector.Center.Y + sector.Extent.Y)) {

				(*result) = pos;
				result->Y = (sector.Center.Y + sector.Extent.Y) - width;
				result->Y = std::max (result->Y, sector.Center.Y);
				retval = true;

			} else if (pos.Y - width < (sector.Center.Y - sector.Extent.Y)) {

				(*result) = pos;
				result->Y = (sector.Center.Y - sector.Extent.Y) + width;
				result->Y = std::min (result->Y, sector.Center.Y);
				retval = true;
			}
		}
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Post_Process_Path
//
///////////////////////////////////////////////////////////////////////////
PathSolveClass::PATHNODE_LIST PathSolveClass::temp_node_list;

void
PathSolveClass::Post_Process_Path (void)
{
	m_Path.Delete_All ();

	//
	//	Build a list of the nodes (in order) the path passes through.
	//
	temp_node_list.Reset_Active();
	for (PathNodeClass *node = m_CompletedNode; node != NULL; node = node->Peek_Parent_Node ()) {
		temp_node_list.Add_Head (node);
	}


	//
	//	Just do it
	//
	m_Path.Add (PathDataStruct (NULL, m_StartPos));
	m_Path[m_Path.Count () - 1].m_SectorCenter = m_StartSector->Get_Bounding_Box ().Center;
	m_Path[m_Path.Count () - 1].m_SectorExtent = m_StartSector->Get_Bounding_Box ().Extent;

	int index;
	for (index = 0; index < temp_node_list.Count (); index ++) {

		PathNodeClass *node				= temp_node_list[index];
		PathfindPortalClass *portal	= node->Peek_Portal ();

		m_Path.Add (PathDataStruct (portal, node->Get_Position ()));
		m_Path[m_Path.Count () - 1].m_SectorCenter.Set (0, 0, 0);
		m_Path[m_Path.Count () - 1].m_SectorExtent.Set (0, 0, 0);

		PathfindSectorClass *sector = node->Peek_Sector ();
		if (sector != NULL) {
			m_Path[m_Path.Count () - 1].m_SectorCenter = sector->Get_Bounding_Box ().Center;
			m_Path[m_Path.Count () - 1].m_SectorExtent = sector->Get_Bounding_Box ().Extent;
		}
	}
	m_Path.Add (PathDataStruct (NULL, m_DestPos));
	m_Path[m_Path.Count () - 1].m_SectorCenter = m_DestSector->Get_Bounding_Box ().Center;
	m_Path[m_Path.Count () - 1].m_SectorExtent = m_DestSector->Get_Bounding_Box ().Extent;

	//
	//	Try to adjust the path so it doesn't make any sharp turns.  To do this
	//	we will average the closest portal points starting from the beginning
	// and then starting from the end.
	//
	typedef struct
	{
		Vector3 backward;
		Vector3 forward;
	} PATH_POINT;

	int count = m_Path.Count ();
	PATH_POINT *path_points = new PATH_POINT[count];

	Vector3 next_point = m_DestPos;
	for (index = m_Path.Count () - 2; index > 0; index --) {

		//
		//	Do we have a portal we can clip the point to?
		//
		PathfindPortalClass *portal = m_Path[index].m_Portal;
		if (portal != NULL && portal->As_PathfindActionPortalClass () == NULL) {

			//
			//	Clip the point to the portal's box
			//
			AABoxClass portal_box;
			portal->Get_Bounding_Box (portal_box);

			Vector3 curr_point = next_point;
			::Clip_Point (&curr_point, portal_box, m_PathObject.Get_Width () * 3.0F);

			//
			//	Move the point to the floor
			//
			curr_point.Z = portal_box.Center.Z - portal_box.Extent.Z;

			//
			//	Add this point to the array
			//
			path_points[index].backward	= curr_point;
			next_point							= curr_point;
		} else {

			//
			//	No portal, so simply add the point to the array
			//
			path_points[index].backward	= m_Path[index].m_Point;
			next_point							= m_Path[index].m_Point;
		}
	}

	next_point = m_StartPos;
	for (index = 1; index < m_Path.Count () - 1; index ++) {

		//
		//	Do we have a portal we can clip the point to?
		//
		PathfindPortalClass *portal = m_Path[index].m_Portal;
		if (portal != NULL && portal->As_PathfindActionPortalClass () == NULL) {

			//
			//	Clip the point to the portal's box
			//
			AABoxClass portal_box;
			portal->Get_Bounding_Box (portal_box);

			Vector3 curr_point = next_point;
			::Clip_Point (&curr_point, portal_box, m_PathObject.Get_Width () * 3.0F);

			//
			//	Move the point to the floor
			//
			curr_point.Z = portal_box.Center.Z - portal_box.Extent.Z;

			//
			//	Add this point to the array
			//
			path_points[index].forward = curr_point;
			next_point = curr_point;
		} else {

			//
			//	No portal, so simply add the point to the array
			//
			path_points[index].forward		= m_Path[index].m_Point;
			next_point							= m_Path[index].m_Point;
		}
	}

	//
	//	Now average the points
	//
	for (index = 1; index < m_Path.Count () - 1; index ++) {
		Vector3 avg_point = (path_points[index].forward + path_points[index].backward) * 0.5F;
		m_Path[index].m_Point = avg_point;
	}

	delete [] path_points;


	//
	//	Relax the points
	//
	for (index = m_Path.Count () - 2; index > 1; index --) {
		Vector3 &prev_point				= m_Path[index + 1].m_Point;
		Vector3 &next_point				= m_Path[index - 1].m_Point;
		PathfindPortalClass *portal	= m_Path[index].m_Portal;
		if (portal != NULL && portal->As_PathfindActionPortalClass () == NULL) {

			//
			//	Relax the between the last point and the next point, making
			//	sure the point lies in the poral box
			//
			AABoxClass portal_box;
			portal->Get_Bounding_Box (portal_box);

			m_Path[index].m_Point = Relax_Line (prev_point, next_point, portal_box);
		}
	}

	//
	//	Ensure that no object will poke its bounding box outside of
	// the sector it's in
	//
	Keep_Unit_Inside_Sectors ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Keep_Unit_Inside_Sectors
//
/////////////////////////////////////////////////////////////////////////////////
void
PathSolveClass::Keep_Unit_Inside_Sectors (void)
{
	if (m_Path.Count () == 0) {
		return ;
	}

	//
	//	Clip the starting point so that the unit will stay on the inside of its sector
	//
	Vector3 temp_point = m_Path[0].m_Point;
	AABoxClass sector_box (m_Path[0].m_SectorCenter, m_Path[0].m_SectorExtent);
	::Clip_Point (&temp_point, sector_box, m_PathObject.Get_Width () * 2);
	m_Path[0].m_Point = temp_point;

	//
	//	Now check each "line" along the path to see if the
	// unit would jut out at any point
	//
	for (int index = 0; index < m_Path.Count () - 1; index ++) {

		Vector3 curr_point	= m_Path[index].m_Point;
		Vector3 next_point	= m_Path[index + 1].m_Point;

		//
		//	Is the current point contained in a sector, and will it
		// be passing through a portal
		//
		if (	(m_Path[index].m_SectorExtent.X > 0) ||
				(m_Path[index].m_SectorExtent.Y > 0))
		{
			//
			//	Get the sector's box
			//
			AABoxClass sector_box (m_Path[index].m_SectorCenter, m_Path[index].m_SectorExtent);
			float width				= m_PathObject.Get_Width () * 3;
			Vector3 start_point	= curr_point;

			//
			//	Lookup the current and next portals
			//
			PathfindPortalClass *prev_portal = m_Path[index].m_Portal;
			PathfindPortalClass *next_portal = m_Path[index + 1].m_Portal;

			//
			//	Test the object at the point it will be entering the sector
			//
			if (prev_portal != NULL && prev_portal->As_PathfindActionPortalClass () == NULL) {

				//
				//	Get the portal's box
				//
				AABoxClass portal_box;
				prev_portal->Get_Bounding_Box (portal_box);

				//
				//	Check to see if the object will jut outside the sector as it moves
				// along this line
				//
				Vector3 new_point (0, 0, 0);
				if (::Check_Line (start_point, next_point, sector_box, portal_box, width, &new_point)) {

					//
					//	Yup, we would jut outside the sector, so add a new point
					// to avoid this
					//
					m_Path.Insert (index + 1, PathDataStruct (NULL, new_point));
					index ++;
					start_point = new_point;
				}
			}

			//
			//	Test the object at the point it will be exiting the sector
			//
			if (next_portal != NULL && next_portal->As_PathfindActionPortalClass () == NULL) {

				//
				//	Get the portal's box
				//
				AABoxClass portal_box;
				next_portal->Get_Bounding_Box (portal_box);

				//
				//	Check to see if the object will jut outside the sector as it moves
				// along this line
				//
				Vector3 new_point (0, 0, 0);
				if (::Check_Line (start_point, next_point, sector_box, portal_box, width, &new_point)) {

					//
					//	Yup, we would jut outside the sector, so add a new point
					// to avoid this
					//
					m_Path.Insert (index + 1, PathDataStruct (NULL, new_point));
					index ++;
				}
			}
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Set_Path_Object
//
/////////////////////////////////////////////////////////////////////////////////
void
PathSolveClass::Set_Path_Object (PathObjectClass &path_object)
{
	m_PathObject = path_object;
	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Get_Path_Object
//
/////////////////////////////////////////////////////////////////////////////////
void
PathSolveClass::Get_Path_Object (PathObjectClass &path_object) const
{
	path_object = m_PathObject;
	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Get_Volumes
//
/////////////////////////////////////////////////////////////////////////////////
void
PathSolveClass::Get_Volumes
(
	BOX_LIST &sector_list,
	BOX_LIST &portal_list
)
{
	//
	//	Build a list of the nodes (in order) the path passes through.
	//
	for (	PathNodeClass *node = m_CompletedNode;
			node != NULL;
			node = node->Peek_Parent_Node ())
	{
		//
		//	Add the box from the sector to the list
		//
		AABoxClass *box = new AABoxClass (node->Peek_Sector ()->Get_Bounding_Box ());
		//box->Extent.X = std::max (0.0F, (box->Extent.X - m_PathObject.Get_Width () * 3));
		//box->Extent.Y = std::max (0.0F, (box->Extent.Y - m_PathObject.Get_Width () * 3));
		sector_list.Add (box);

		//
		//	If this node has a portal, then add the portal's box to the list
		//
		if (node->Peek_Portal () != NULL) {
			AABoxClass *portal_box	= new AABoxClass;
			node->Peek_Portal ()->Get_Bounding_Box (*portal_box);
			portal_list.Add (portal_box);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////////////////////////////////
void
PathSolveClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_VARIABLES);

		//
		//	Save each variable to its own microchunk
		//
		WRITE_MICRO_CHUNK (csave, VARID_STARTPOS,			m_StartPos);
		WRITE_MICRO_CHUNK (csave, VARID_DESTPOS,			m_DestPos);
		WRITE_MICRO_CHUNK_PTR (csave, VARID_START_SECTOR,	m_StartSector);
		WRITE_MICRO_CHUNK_PTR (csave, VARID_DEST_SECTOR,		m_DestSector);
		WRITE_MICRO_CHUNK (csave, VARID_PATH_OBJECT,		m_PathObject);
		WRITE_MICRO_CHUNK (csave, VARID_PRIORITY,			m_Priority);

		PathSolveClass *this_ptr = this;
		WRITE_MICRO_CHUNK_PTR (csave, VARID_OLD_PTR,			this_ptr);

	csave.End_Chunk ();
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////////////////////////////////
void
PathSolveClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	SaveLoadSystemClass::Register_Post_Load_Callback (this);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	On_Post_Load
//
///////////////////////////////////////////////////////////////////////
void
PathSolveClass::On_Post_Load (void)
{
	//
	//	This basically kicks off the pathfind
	//
	Initialize (1.0F);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////
void
PathSolveClass::Load_Variables (ChunkLoadClass &cload)
{
	PathSolveClass *old_ptr = NULL;
	m_StartSector	= NULL;
	m_DestSector	= NULL;

	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK (cload, VARID_STARTPOS,		m_StartPos);
			READ_MICRO_CHUNK (cload, VARID_DESTPOS,		m_DestPos);
			READ_MICRO_CHUNK (cload, VARID_PATH_OBJECT,	m_PathObject);
			READ_MICRO_CHUNK_PTR (cload, VARID_OLD_PTR,		old_ptr);
			READ_MICRO_CHUNK (cload, VARID_PRIORITY,		m_Priority);
		}

		cload.Close_Micro_Chunk ();
	}

	//
	//	Register our old ptr so other objects can remap to us
	//
	if (old_ptr != NULL) {
		SaveLoadSystemClass::Register_Pointer (old_ptr, this);
	}

	return ;
}