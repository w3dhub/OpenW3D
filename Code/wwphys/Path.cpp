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
 *                     $Archive:: /Commando/Code/wwphys/Path.cpp                 $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/20/02 6:52p                                               $*
 *                                                                                             *
 *                    $Revision:: 52                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "Path.h"
#include <windows.h>
#include "colmathaabox.h"
#include "Pathfind.h"
#include "PathfindPortal.h"
#include "pathsolve.h"
#include "PathDebugPlotter.h"
#include "cardinalspline.h"
#include "PathObject.h"
#include "vehiclecurve.h"
#include "waypath.h"
#include "waypoint.h"
#include "chunkio.h"
#include "persistfactory.h"
#include "assetmgr.h"
#include "wwmemlog.h"


////////////////////////////////////////////////////////////////
//	Save/Load constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES				= 0x11040535,
	CHUNKID_SPLINE
};

enum
{
	VARID_STATE						= 1,
	VARID_TRAVERSAL_TYPE,
	VARID_START_POS,
	VARID_DEST_POS,
	VARID_EXPECTED_POS,
	VARID_LOOK_AHEAD_DIST,
	VARID_LOOK_AHEAD_TIME,
	VARID_MOVEMENT_RADIUS,
	VARID_SPLINE_TIME,
	VARID_VELOCITY,
	VARID_CURRENT_ACTION,
	VARID_MOVEMENT_DIRECTIONS,
	VARID_PATH_OBJECT,
	VARID_START_TIME,
	VARID_END_TIME,
	VARID_ISLOOPING,
	VARID_PATH_ACTION,
	VARID_OLD_PTR,
	VARID_TOTAL_DIST
};


///////////////////////////////////////////////////////////////////////////
//	Local macros
///////////////////////////////////////////////////////////////////////////
#define SAFE_DELETE(pobject)					\
			if (pobject) {							\
				delete pobject;					\
				pobject = NULL;					\
			}											\


///////////////////////////////////////////////////////////////////////////
//	Constants
///////////////////////////////////////////////////////////////////////////
static const float DEF_VEHICLE_VELOCITY	= 40.0F;
static const float DEF_HUMAN_VELOCITY		= 2.5F; //5.0F;
static const float ASSUMED_FPS				= 30.0F; //15.0F;


///////////////////////////////////////////////////////////////////////////
//	Static member initialization
///////////////////////////////////////////////////////////////////////////
bool	PathClass::m_DisplayMoveVectors	= false;

static int _InstanceCount = 0;

///////////////////////////////////////////////////////////////////////////
//
//	PathClass
//
///////////////////////////////////////////////////////////////////////////
PathClass::PathClass (void)
	:	m_ExpectedPos (0, 0, 0),
		m_StartPos (0, 0, 0),
		m_DestPos (0, 0, 0),
		m_StartTime (0),
		m_EndTime (1.0F),
		m_LookAheadTime (0),
		m_LookAheadDist (0),
		m_SplineTime (0),
		m_State (ERROR_NOT_INITIALIZED),
		m_TraversalType (SPLINE),
		m_Velocity (DEF_VEHICLE_VELOCITY),
		m_Spline (NULL),
		m_MovementRadius (0.1F),
		m_MovementDirections (MOVE_X | MOVE_Y),
		m_CurrentAction (-1),
		m_IsLooping (false),
		m_TotalDist (0)
{
	_InstanceCount ++;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	~PathClass
//
///////////////////////////////////////////////////////////////////////////
PathClass::~PathClass (void)
{
	_InstanceCount --;
	SAFE_DELETE (m_Spline);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
///////////////////////////////////////////////////////////////////////////
void
PathClass::Initialize (const Vector3 &start, const Vector3 &end)
{
	m_State					= STATE_TRAVERSING_PATH;
	m_SplineTime			= 0;
	m_LookAheadTime		= 0;
	m_TotalDist				= 0;
	m_LookAheadDist		= 0;
	m_CurrentAction		= -1;
	m_StartTime				= 0;
	m_EndTime				= 1.0F;
	m_IsLooping				= false;

	PATH_NODE node1;
	node1.time					= 0;
	node1.next_time			= 1.0F;
	node1.action_id			= ACTION_NONE;
	node1.mechanism_id		= 0;
	node1.tighten_spline		= false;
	node1.pos					= start;
	node1.dest_pos.Set (0, 0, 0);

	PATH_NODE node2;
	node2.time					= 0;
	node2.next_time			= 1.0F;
	node2.action_id			= ACTION_NONE;
	node2.mechanism_id		= 0;
	node2.tighten_spline		= false;
	node2.pos					= end;
	node2.dest_pos.Set (0, 0, 0);

	DynamicVectorClass<PATH_NODE> node_list;
	node_list.Add (node1);
	node_list.Add (node2);

	//
	//	Convert the nodes into a spline and a set of action requests
	//
	Initialize_Spline (node_list);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
///////////////////////////////////////////////////////////////////////////
void
PathClass::Initialize (PathSolveClass &path_solve)
{
	WWMEMLOG(MEM_PATHFIND);
	
	m_State					= STATE_TRAVERSING_PATH;
	m_SplineTime			= 0;
	m_LookAheadTime		= 0;
	m_TotalDist				= 0;
	m_LookAheadDist		= 0;
	m_CurrentAction		= -1;
	m_StartTime				= 0;
	m_EndTime				= 1.0F;
	m_IsLooping				= false;

	DynamicVectorClass<PATH_NODE> node_list;

	//
	//	Get the raw path data from the solver, and convert it into
	// a format we can digest
	//
	PathSolveClass::PATHPOINT_LIST &raw_path = path_solve.Get_Raw_Path ();
	for (int index = 0; index < raw_path.Count (); index ++) {		
	
		//
		//	Setup a path node that contains the information we
		// need from the path solver
		//
		PATH_NODE node;
		node.time				= 0;
		node.next_time			= 1.0F;
		node.action_id			= ACTION_NONE;
		node.mechanism_id		= 0;
		node.tighten_spline	= false;
		node.pos					= raw_path[index].m_Point;
		node.dest_pos.Set (0, 0, 0);  

		//
		//	Check for any action requirements
		//
		bool is_valid = true;
		PathfindPortalClass *portal = raw_path[index].m_Portal;
		if (portal != NULL) {

			//
			//	See what kind of portal this is
			//
			PathfindActionPortalClass *action_portal		= portal->As_PathfindActionPortalClass ();
			PathfindWaypathPortalClass *waypath_portal	= portal->As_PathfindWaypathPortalClass ();
			if (action_portal != NULL) {

				//
				//	Check to see if we are inadvertantly starting on an "exit" portal
				//
				if (index == 1 && action_portal->Get_Action_Type () == PathClass::ACTION_NONE) {

					//
					//	Find the entrance portal
					//
					PathfindActionPortalClass *enter_portal = action_portal->Get_Enter_Portal ();
					if (enter_portal != NULL) {

						AABoxClass box;
						enter_portal->Get_Bounding_Box (box);

						//
						//	Insert a node that will force the unit to do the entrance portal
						//
						PATH_NODE temp_node;
						temp_node.time					= 0;
						temp_node.next_time			= 1.0F;
						temp_node.tighten_spline	= true;
						temp_node.pos					= box.Center;
						temp_node.action_id			= enter_portal->Get_Action_Type ();
						temp_node.dest_pos			= enter_portal->Get_Destination ();
						temp_node.mechanism_id		= enter_portal->Get_Mechanism_ID ();
						node_list.Add (temp_node);
					}
				}

				//
				//	This is an action portal so set the action parameters
				//
				node.action_id			= action_portal->Get_Action_Type ();
				node.dest_pos			= action_portal->Get_Destination ();
				node.mechanism_id		= action_portal->Get_Mechanism_ID ();				
				node.tighten_spline	= true;

				//
				//	Override the positions so that its in the center
				// of the entrance portal.  (This ensures guys won't
				// get stuck when the door opens).
				//
				AABoxClass box;
				action_portal->Get_Bounding_Box (box);
				node.pos	= box.Center;

			} else if (waypath_portal != NULL) {
				is_valid = false;

				//
				//	Lookup the next portal in the list (it should be the waypath exit portal)
				//
				if (index + 1 < raw_path.Count ()) {
					PathfindWaypathPortalClass *next_portal = raw_path[index + 1].m_Portal->As_PathfindWaypathPortalClass ();
					if (next_portal != NULL) {

						//
						//	Create a temporary waypath segment from the portal information
						//
						const WaypathPositionClass &start_pos	= waypath_portal->Get_Waypath_Pos ();
						const WaypathPositionClass &end_pos		= next_portal->Get_Waypath_Pos ();
						WaypathClass *path_segment = new WaypathClass (start_pos, end_pos);

						//
						//	Insert all the data from this waypath segment into our path
						//
						Add_Waypath_Data (node_list, path_segment, 0, path_segment->Get_Point_Count () - 1);
						REF_PTR_RELEASE (path_segment);

						//
						//	Skip past the next portal
						//
						index ++;
					}
				}

			} else if (portal->Is_Two_Way_Portal () == false) {
				
				/*if (index < (raw_path.Count () - 1) && raw_path[index+1].m_Portal != NULL) {
					
					AABoxClass curr_box;
					AABoxClass next_box;
					
					portal->Get_Bounding_Box (curr_box);
					raw_path[index+1].m_Portal->Get_Bounding_Box (next_box);

					if (curr_box.Center.Z > (next_box.Center.Z + 1.5F)) {
						curr_box.Center.Z = 0;
						next_box.Center.Z = 0;
						float portal_dist = (curr_box.Center - next_box.Center).Length ();
						if (portal_dist < 665.0F) {

							//
							//	Calculate an intermediate point that is one meter towards
							// the center of the next sector
							//
							Vector3 temp_delta = (raw_path[index+1].m_SectorCenter - next_box.Center);
							temp_delta.Normalize ();
							Vector3 intermediate_point = next_box.Center + temp_delta;

							//
							//	Insert a node that will force the unit to jump down
							//
							PATH_NODE temp_node;
							temp_node.time					= 0;
							temp_node.next_time			= 1.0F;
							temp_node.tighten_spline	= true;
							temp_node.pos					= intermediate_point;
							temp_node.action_id			= ACTION_JUMP;
							temp_node.mechanism_id		= 0;
							temp_node.dest_pos.Set (intermediate_point);
							node_list.Add (temp_node);
						}
					}
				}*/
			}
		}
		
		if (is_valid) {
			node_list.Add (node);
		}
	}

	//
	//	Convert the nodes into a spline and a set of action requests
	//
	Initialize_Spline (node_list);

	//
	//	Clip the spline to the sectors and portals that the solver knows is safe
	//
	if (m_PathObject.Is_Flag_Set (PathObjectClass::IS_VEHICLE) == false) {
		Clip_Spline_To_Pathfind_Data (node_list, path_solve);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Add_Waypoint_Info_To_Node_List
//
///////////////////////////////////////////////////////////////////////////
void
PathClass::Add_Waypoint_Info_To_Node_List
(
	DynamicVectorClass<PathClass::PATH_NODE> &	node_list,
	WaypointClass *										waypoint,
	WaypointClass *										next_point
)
{
	//
	//	Fill in a path node structure for this waypoint
	//
	PathClass::PATH_NODE node;
	node.time			= 0;
	node.next_time		= 1.0F;
	node.action_id		= ACTION_NONE;
	node.mechanism_id	= 0;
	node.dest_pos.Set (0, 0, 0);
	node.pos				= waypoint->Get_Position ();

	//
	//	Check to see if there are any special actions to perform at
	// this waypoint
	//
	if (waypoint->Get_Flag (WaypointClass::FLAG_REQUIRES_JUMP)) {
		node.action_id			= ACTION_JUMP;
		node.tighten_spline	= true;

		//
		//	Record the jump-to point (if there is one)
		//
		if (next_point != NULL) {
			node.dest_pos = next_point->Get_Position ();
		}
	} else if (waypoint->Get_Flag (WaypointClass::FLAG_REQUIRES_ACTION)) {

		//
		//	Lookup the action portal from the waypoint
		//
		PathfindActionPortalClass *action_portal = waypoint->Get_Action_Portal ();
		if (action_portal != NULL) {
			
			//
			//	Configure the node's action parameters
			//
			node.action_id			= action_portal->Get_Action_Type ();
			node.dest_pos			= action_portal->Get_Destination ();
			node.mechanism_id		= action_portal->Get_Mechanism_ID ();
			node.tighten_spline	= true;

			//
			//	Override the positions so that its in the center
			// of the entrance portal.  (This ensures guys won't
			// get stuck when the door opens).
			//
			AABoxClass box;
			action_portal->Get_Bounding_Box (box);
			node.pos	= box.Center;
		}
	}

	//
	//	Add the node to the list
	//
	node_list.Add (node);	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Add_Waypath_Data
//
///////////////////////////////////////////////////////////////////////////
void
PathClass::Add_Waypath_Data
(
	DynamicVectorClass<PATH_NODE> &	node_list,
	WaypathClass *							waypath,
	int										start_index,
	int										end_index
)
{
	//
	//	If this is a looping path, then add the last waypoint first, so we can
	// wrap the spline from "last_point-1" all the way through "first_point+1".
	// If we don't do this, the spline will not look continuous.
	//
	if (m_IsLooping) {
		Add_Waypoint_Info_To_Node_List (node_list, waypath->Get_Point (end_index),
													waypath->Get_Point (start_index));
	}

	//
	//	Loop along the waypath either forwards or backwards, depending
	// on what the caller has requested
	//
	int count	= WWMath::Fabs (end_index - start_index);
	int inc		= (end_index > start_index) ? 1 : -1;
	for (int index = start_index; count >= 0; index += inc, count --) {

		WaypointClass *waypoint		= waypath->Get_Point (index);
		WaypointClass *next_point	= waypath->Get_Point (index + inc);
		
		//
		//	Add data from each waypoint into our path node list
		//
		if ((waypoint == NULL || next_point == NULL) || waypoint->Get_Position () != next_point->Get_Position ()) {
			Add_Waypoint_Info_To_Node_List (node_list, waypoint, next_point);
		}
	}

	//
	//	If this is a looping path, then add the first and second waypoints again. 
	//	This is done so we can wrap the spline from "last_point-1" all the way
	//	through "first_point+1". If we don't do this, the spline will not look
	//	continuous.
	//
	if (m_IsLooping) {
		
		Add_Waypoint_Info_To_Node_List (node_list, waypath->Get_Point (start_index),
													waypath->Get_Point (start_index + inc));

		Add_Waypoint_Info_To_Node_List (node_list, waypath->Get_Point (start_index + inc),
													waypath->Get_Point (start_index + inc + inc));
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
///////////////////////////////////////////////////////////////////////////
void
PathClass::Initialize (WaypathClass *waypath, int start_pt_id, int end_pt_id)
{
	WWMEMLOG(MEM_PATHFIND);

	m_State					= ERROR_NOT_INITIALIZED;	
	m_SplineTime			= 0;
	m_LookAheadTime		= 0;
	m_LookAheadDist		= 0;
	m_CurrentAction		= -1;
	m_StartTime				= 0;
	m_EndTime				= 1.0F;
	m_IsLooping				= false;

	//
	//	Initialize the path from this waypath
	//
	if (waypath != NULL) {

		//
		//	Determine if this is a looping path or not
		//
		m_IsLooping = waypath->Get_Flag (WaypathClass::FLAG_LOOPING);
		
		//
		//	Get the number of waypoints on this path
		//
		int count = waypath->Get_Point_Count ();
		if (count > 0) {

			//
			//	Determine the starting and ending indicies
			//
			int start_index	= 0;
			int end_index		= count - 1;
			for (int index  = 0; index < count; index ++) {
				if (waypath->Get_Point (index)->Get_ID () == start_pt_id) {
					start_index = index;
				}
				if (waypath->Get_Point (index)->Get_ID () == end_pt_id) {
					end_index = index;
				}
			}			
			
			//
			//	Initialize our first position
			//
			m_ExpectedPos	= waypath->Get_Point (start_index)->Get_Position ();
			m_State			= STATE_TRAVERSING_PATH;
			
			//
			//	Add the waypath data to our node list
			//
			DynamicVectorClass<PATH_NODE> node_list;
			Add_Waypath_Data (node_list, waypath, start_index, end_index);
			
			//
			//	Now create a spline from the waypath data
			//
			Initialize_Spline (node_list);			
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Evaluate_Next_Point
//
///////////////////////////////////////////////////////////////////////////
bool
PathClass::Evaluate_Next_Point (const Vector3 &curr_pos, Vector3 &new_pos)
{
	//
	//	Be careful not to evaluate when we are in a invalid state
	//
	if (m_State >= FIRST_ERROR) {
		new_pos = curr_pos;
		return false;
	} else if (m_State == STATE_PATH_COMPLETE) {
		new_pos = m_ExpectedPos;
		return true;
	}

	//
	//	Get the delta from our current position to the point where we
	// should be heading
	//
	Vector3 delta = m_ExpectedPos - curr_pos;
	
	//
	// Zero out any directions we aren't moving in
	//
	if ((m_MovementDirections & MOVE_X) == 0) {
		delta.X = 0;
	}

	if ((m_MovementDirections & MOVE_Y) == 0) {
		delta.Y = 0;
	}

	if ((m_MovementDirections & MOVE_Z) == 0) {
		delta.Z = 0;
	}

	//
	// Calculate the distance we have yet to travel to
	// reach the next point
	//
	float delta_len	= delta.Length ();
	delta_len			-= m_MovementRadius;

	//
	//	Reset our state
	//
	m_State = STATE_TRAVERSING_PATH;

	//
	//	Recalculate our look-ahead distance
	//
	//m_LookAheadDist = (m_Velocity * 4.0F) / ASSUMED_FPS;

	//
	//	Did we reach the expected position?
	//	
	bool advance_dest = delta_len < m_LookAheadDist;
	if (advance_dest) {

		float new_time = m_SplineTime + m_LookAheadTime;
		if (new_time > (m_EndTime - (m_LookAheadTime * 0.5F))) {
			new_time = m_EndTime;
		}

		//
		//	Have we successfully traversed the spline?
		//
		if (new_time >= m_EndTime) {

			//
			//	Is this a looping path?
			//
			if (m_IsLooping) {

				//
				//	If this path is looping, then wrap from the end of the
				// path to the beginning of the path
				//
				m_SplineTime		= (new_time - m_EndTime) + m_StartTime;
				m_CurrentAction	= -1;

			} else {

				//
				//	If we are close to the destination, then consider us 'complete'.
				//
				//if (delta_len < 0.25F) {
					m_State				= STATE_PATH_COMPLETE;
					m_SplineTime		= m_EndTime;
					m_CurrentAction	= -1;
				//}
			}

		} else {
				
			//
			//	Check to see if the object needs to perform an action at this point
			//
			if (m_CurrentAction + 1 < m_PathActions.Count ()) {
				PATH_NODE &node = m_PathActions[m_CurrentAction + 1];
				
				//
				//	Should we activate this action request?
				//
				float action_time = node.time;
				if (WWMath::Fabs (action_time - m_SplineTime) < (m_LookAheadTime * 0.5F)) {
					
					//
					//	Set the new state and remember which action we are to perform
					//
					m_State = STATE_ACTION_REQUIRED;
					m_CurrentAction ++;

					//
					//	Snap to the action point
					//
					new_time = node.next_time;
				}
			}

			//
			//	Increase our position along the spline
			//
			m_SplineTime = new_time;
		}

		//
		//	Evaluate the spline at the given 'time' to determine our new position
		//
		m_Spline->Evaluate (m_SplineTime, &m_ExpectedPos);
		
		if (m_PathObject.Is_Flag_Set (PathObjectClass::IS_VEHICLE)) {
			m_SplineTime = ((VehicleCurveClass *)m_Spline)->Get_Last_Eval_Time ();
		}
	}

	new_pos = m_ExpectedPos;

	/*static PhysClass *_DebugObj = NULL;

	if (_DebugObj == NULL) {
		_DebugObj = new DecorationPhysClass;
		_DebugObj->Set_Model (WW3DAssetManager::Get_Instance ()->Create_Render_Obj ("C_HAVOC"));
		_DebugObj->Inc_Ignore_Counter ();
		PhysicsSceneClass::Get_Instance ()->Add_Dynamic_Object (_DebugObj);
	}

	_DebugObj->Set_Transform (Matrix3D (m_ExpectedPos));*/

	if (m_DisplayMoveVectors) {
		PhysicsSceneClass::Get_Instance ()->Add_Debug_AABox (AABoxClass (curr_pos, Vector3 (0.25F,0.25F,0.25F)), Vector3 (1, 0, 0));
		PhysicsSceneClass::Get_Instance ()->Add_Debug_AABox (AABoxClass (m_ExpectedPos, Vector3 (0.25F,0.25F,0.25F)), Vector3 (0, 1, 0));
		PhysicsSceneClass::Get_Instance ()->Add_Debug_Vector (curr_pos, m_ExpectedPos - curr_pos, Vector3 (0, 0.6F, 1));
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Curve_Sharpness
//
///////////////////////////////////////////////////////////////////////////
float
PathClass::Get_Curve_Sharpness (Vector3 *position) const
{
	float sharpness = 0;

	if (m_PathObject.Is_Flag_Set (PathObjectClass::IS_VEHICLE)) {
		Vector3 foo;
		m_Spline->Evaluate (m_SplineTime, &foo);
		sharpness = ((VehicleCurveClass *)m_Spline)->Get_Current_Sharpness (position);
	}

	return sharpness;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Last_Eval_Time
//
///////////////////////////////////////////////////////////////////////////
float
PathClass::Get_Last_Eval_Time (void) const
{
	return m_SplineTime;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Remaining_Path_Length
//
///////////////////////////////////////////////////////////////////////////
float
PathClass::Get_Remaining_Path_Length (void)
{
	float length = m_TotalDist;

	//
	//	The length remaining should correspond to the amount of time
	// remaining.
	//
	if (m_IsLooping == false && m_EndTime > 0) {
		length = m_TotalDist * ((m_EndTime - m_SplineTime) / m_EndTime);
	}

	return length;
}


///////////////////////////////////////////////////////////////////////////
//
//	Display_Path
//
///////////////////////////////////////////////////////////////////////////
void
PathClass::Display_Path (bool onoff)
{
	if (onoff == false) {
		PathDebugPlotterClass::Get_Instance ()->Display (false);
	} else if (m_State < FIRST_ERROR) {
		
		//
		//	Turn off painting
		//
		PathDebugPlotterClass::Get_Instance ()->Display (false);
		PathDebugPlotterClass::Get_Instance ()->Reset ();
			
		//
		//	Get the first position on the spline
		//
		Vector3 last_pos;
		m_Spline->Evaluate (0, &last_pos);

		//
		//	Plot the spline
		//
		for (float t = m_LookAheadTime; t <= 1.0F; t += m_LookAheadTime) {
		//for (float t = m_LookAheadTime; t <= 1.0F; t += m_LookAheadTime / 2.0F) {
			Vector3 curr_pos;
			m_Spline->Evaluate (t, &curr_pos);

			if (m_PathObject.Is_Flag_Set (PathObjectClass::IS_VEHICLE)) {
				t = ((VehicleCurveClass *)m_Spline)->Get_Last_Eval_Time ();
			}

			PathDebugPlotterClass::Get_Instance ()->Add (last_pos, curr_pos, Vector3 (0, 0.5F, 1));
			last_pos = curr_pos;
		}

		//
		//	Turn painting back on
		//
		PathDebugPlotterClass::Get_Instance ()->Display (true);
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Set_Traversal_Type
//
/////////////////////////////////////////////////////////////////////////////////
void
PathClass::Set_Traversal_Type (TRAVERSAL_TYPE type)
{
	//
	//	Do we need to re-initialize any data?
	//
	if (m_TraversalType != type) {
		m_TraversalType = type;
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Find_Intersection_X
//
/////////////////////////////////////////////////////////////////////////////////
bool
Find_Intersection_X
(
	const Vector3 &	start,
	const Vector3 &	end,
	const Vector3 &	rgn_center,
	const Vector3 &	rgn_extent,
	Vector3 *			intersect_point
)
{
	float percent = (rgn_center.X - start.X) / (end.X - start.X);
	(*intersect_point) = start + ((end - start) * percent);
	
	bool retval	= false;
	if (percent >= 0 && percent < 1.0F) {
		retval	= (intersect_point->Y >= (rgn_center.Y - rgn_extent.Y));
		retval	&= (intersect_point->Y <= (rgn_center.Y + rgn_extent.Y));
		retval	&= (intersect_point->Z <= (rgn_center.Z + rgn_extent.Z));
		retval	&= (intersect_point->Z <= (rgn_center.Z + rgn_extent.Z));
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Find_Intersection_Y
//
/////////////////////////////////////////////////////////////////////////////////
bool
Find_Intersection_Y
(
	const Vector3 &	start,
	const Vector3 &	end,
	const Vector3 &	rgn_center,
	const Vector3 &	rgn_extent,
	Vector3 *			intersect_point
)
{
	float percent = (rgn_center.Y - start.Y) / (end.Y - start.Y);
	(*intersect_point) = start + ((end - start) * percent);
	
	bool retval	= false;
	if (percent >= 0 && percent < 1.0F) {
		retval	= (intersect_point->X >= (rgn_center.X - rgn_extent.X));
		retval	&= (intersect_point->X <= (rgn_center.X + rgn_extent.X));
		retval	&= (intersect_point->Z <= (rgn_center.Z + rgn_extent.Z));
		retval	&= (intersect_point->Z <= (rgn_center.Z + rgn_extent.Z));
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Find_Side_Intersection
//
/////////////////////////////////////////////////////////////////////////////////
bool
Find_Side_Intersection
(
	int					index,
	const AABoxClass &box,
	const Vector3 &	start,
	const Vector3 &	end,
	Vector3 *			intersection_point
)
{
	bool retval = false;

	if (index == 0) {

		Vector3 center	= box.Center;
		center.X			+= box.Extent.X;
		Vector3 extent = Vector3 (0, box.Extent.Y, box.Extent.Z);

		retval = Find_Intersection_X (start, end, center, extent, intersection_point);
		
	} else if (index == 1) {

		Vector3 center	= box.Center;
		center.X			-= box.Extent.X;
		Vector3 extent = Vector3 (0, box.Extent.Y, box.Extent.Z);

		retval = Find_Intersection_X (start, end, center, extent, intersection_point);		
	} else if (index == 2) {

		Vector3 center	= box.Center;
		center.Y			+= box.Extent.Y;
		Vector3 extent = Vector3 (box.Extent.X, 0, box.Extent.Z);

		retval = Find_Intersection_Y (start, end, center, extent, intersection_point);		
	} else if (index == 3) {

		Vector3 center	= box.Center;
		center.Y			-= box.Extent.Y;
		Vector3 extent = Vector3 (box.Extent.X, 0, box.Extent.Z);

		retval = Find_Intersection_Y (start, end, center, extent, intersection_point);		
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Is_Point_In_Boxes
//
/////////////////////////////////////////////////////////////////////////////////
bool
PathClass::Is_Point_In_Boxes
(
	const Vector3 &	point,
	BOX_LIST &			box_list
)
{
	bool retval = false;

	//
	//	Test each box in the list
	//
	for (int index = 0; (index < box_list.Count ()) && !retval; index ++) {
		retval |= box_list[index]->Contains (point);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Clip_Control_Point
//
/////////////////////////////////////////////////////////////////////////////////
void
PathClass::Clip_Control_Point
(
	const Vector3 &start_point,
	Vector3 *		point,
	BOX_LIST &		sector_list,
	BOX_LIST &		portal_list
)
{
	//
	//	Loop through all the boxes we need to clip against
	//
	for (int index = 0; index < sector_list.Count (); index ++) {		
		AABoxClass &box = *(sector_list[index]);


		//
		//	Clip the line to each of the four sides of the box (don't care about
		// top or bottom for the moment).
		//
		for (int side_index = 0; side_index < 4; side_index ++) {
			
			//
			//	Does the line pass through this side?
			//
			Vector3 intersect_point;
			if (Find_Side_Intersection (side_index, box, start_point, *point, &intersect_point)) {
				
				//
				//	If there isn't a portal where this line passes through the side, then
				// clip the line to the side.
				//
				if (Is_Point_In_Boxes (intersect_point, portal_list) == false) {															
					(*point) = intersect_point;
				}
			}
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Initialize_Vehicle_Spline
//
/////////////////////////////////////////////////////////////////////////////////
void
PathClass::Initialize_Vehicle_Spline (DynamicVectorClass<PATH_NODE> &node_list)
{	
	m_Spline = new VehicleCurveClass (m_PathObject.Get_Turn_Radius ());

	Vector3 point;
	Vector3 last_point = m_StartPos;

	//
	//	Setup the spline using each 'point' on the path
	// as a key along the spline.
	//
	float current_dist	= 0;
	for (int index = 0; index < node_list.Count (); index ++) {
		point = node_list[index].pos;
		
		//
		//	Add this point as a key along the spline
		//
		current_dist		+= (point - last_point).Length ();

		float curr_time = 0.0f;
		if (m_TotalDist != 0.0f) {
			curr_time = current_dist / m_TotalDist;
		}
		m_Spline->Add_Key (point, curr_time);

		//
		//	Is this a looping path?
		//
		if (m_IsLooping) {
		
			//
			//	Record where the loop starts and ends...
			//
			if (index == 1) {
				m_StartTime = curr_time;
			} else if (index == node_list.Count () - 2) {
				m_EndTime = curr_time;
			}			
		}

		
		last_point = point;
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Initialize_Human_Spline
//
/////////////////////////////////////////////////////////////////////////////////
void
PathClass::Initialize_Human_Spline(DynamicVectorClass<PATH_NODE> &node_list)
{
	CardinalSpline3DClass temp_spline;
	float tightness = (m_TraversalType == SPLINE) ? 0.3F : 1.0F;

	//
	//	Assume we do the whole path (looping paths only do a subset of
	// the path)
	//

	Vector3 point;
	Vector3 last_point = m_StartPos;

	//
	//	Setup the temporary spline using each 'point' on the path
	// as a key along the spline.
	//
	float current_dist		= 0;
	bool fixup_last_action	= false;
	for (int index = 0; index < node_list.Count (); index ++) {
		point = node_list[index].pos;
		
		//
		//	Add this point as a key along the spline
		//
		current_dist		+= (point - last_point).Length ();
		float curr_time = 0.0f;
		if (m_TotalDist != 0.0f) {
			curr_time = current_dist / m_TotalDist;
		}
		temp_spline.Add_Key (point, curr_time);

		//
		//	Is this a looping path?
		//
		if (m_IsLooping) {
		
			//
			//	Record where the loop starts and ends...
			//
			if (index == 1) {
				m_StartTime = curr_time;
			} else if (index == node_list.Count () - 2) {
				m_EndTime = curr_time;
			}			
		}


		//
		//	Do we need to fix-up the time values for the last action node?
		//
		if (fixup_last_action) {
			PATH_NODE &node	= m_PathActions[m_PathActions.Count () - 1];
			node.next_time		= curr_time;
			fixup_last_action	= false;
		}
		
		//
		//	Do we have an action at this node that we need to store?
		//
		if (node_list[index].action_id != 0) {

			//
			//	Add an action node to our list so when the unit gets
			// to this point along the spline, we can have it perform
			// the requested action.
			//
			PATH_NODE node;
			node.time			= curr_time;
			node.next_time		= 1.0F;
			node.action_id		= node_list[index].action_id;
			node.mechanism_id	= node_list[index].mechanism_id;
			node.dest_pos		= node_list[index].dest_pos;
			node.pos				= node_list[index].pos;
			m_PathActions.Add (node);

			//
			//	We need to fix up the 'next_time' member during the next iteration
			//
			fixup_last_action = true;
		}

		//
		//	For some type of nodes we need to tighten the spline so its
		// as sharp as possible (actions generally require this)
		//
		if (node_list[index].tighten_spline) {
			temp_spline.Set_Tightness (index, 1.0F);
		} else {		
			temp_spline.Set_Tightness (index, tightness);
		}			

		last_point = point;
	}

	//
	//	Convert the temp spline to a hermite spline so we
	// can adjust the tangent points if necessary (for clipping).
	//
	temp_spline.Update_Tangents ();
	m_Spline = new HermiteSpline3DClass;
	(*((HermiteSpline3DClass*)m_Spline)) = temp_spline;
	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Clip_Spline_To_Pathfind_Data
//
/////////////////////////////////////////////////////////////////////////////////
void
PathClass::Clip_Spline_To_Pathfind_Data
(
	DynamicVectorClass<PATH_NODE> &node_list,
	PathSolveClass &					path_solve
)
{
	//
	//	Get the portal and sector list from the pathsolver
	//
	BOX_LIST	sector_list;
	BOX_LIST	portal_list;
	path_solve.Get_Volumes (sector_list, portal_list);

	//
	//	Loop through the points in the hermite spline, convert
	// each tangent to bezier curve control points, clip these
	// points to the sectors and portals the path contains,
	// and convert the clipped points back into hermite spline
	// tangents.
	//
	int index;
	for (index = 0; index < node_list.Count () - 1; index ++) {
		
		//
		//	Get the current and next points on the path
		//
		Vector3 point			= node_list[index].pos;
		Vector3 next_point	= node_list[index + 1].pos;

		//
		//	Get the in and out tangents for each of these points
		//
		Vector3 tangent_in;
		Vector3 tangent_out;
		Vector3 next_tangent_in;
		Vector3 next_tangent_out;
		((HermiteSpline3DClass*)m_Spline)->Get_Tangents (index, &tangent_in, &tangent_out);
		((HermiteSpline3DClass*)m_Spline)->Get_Tangents (index + 1, &next_tangent_in, &next_tangent_out);

		//
		//	Convert the tangents to bezier curve control points
		//
		float one_third	= 1.0F / 3.0F;
		Vector3 ctrl_pt1	= point + (tangent_out * one_third);
		Vector3 ctrl_pt2	= next_point - (next_tangent_in * one_third);

		//
		//	Clip the control points to the pathfind sectors and portals
		//
		Clip_Control_Point (point, &ctrl_pt1, sector_list, portal_list);
		Clip_Control_Point (next_point, &ctrl_pt2, sector_list, portal_list);
		
		//
		//	Convert the control points back into tangents
		//
		tangent_out			= (ctrl_pt1 - point) * 3.0F;
		next_tangent_in	= (next_point - ctrl_pt2) * 3.0F;

		//
		//	Pass the tangents back to the hermite spline
		//
		((HermiteSpline3DClass*)m_Spline)->Set_Tangents (index, tangent_in, tangent_out);
		((HermiteSpline3DClass*)m_Spline)->Set_Tangents (index + 1, next_tangent_in, next_tangent_out);		
	}

	//
	//	Free the temporary portal-box list
	//
	for (index = 0; index < portal_list.Count (); index ++) {
		AABoxClass *portal_box = portal_list[index];
		delete portal_box;
	}

	//
	//	Free the temporary sector-box list
	//
	for (index = 0; index < sector_list.Count (); index ++) {		
		AABoxClass *box = sector_list[index];
		delete box;
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Initialize_Spline
//
/////////////////////////////////////////////////////////////////////////////////
void
PathClass::Initialize_Spline (DynamicVectorClass<PATH_NODE> &node_list)
{
	if (node_list.Count () > 0) {
		
		//
		//	Start fresh
		//
		m_PathActions.Delete_All ();
		SAFE_DELETE (m_Spline);

		//
		//	Get the first and last positions from node list
		//
		m_StartPos	= node_list[0].pos;
		m_DestPos	= node_list[node_list.Count () - 1].pos;

		if (m_IsLooping && node_list.Count () > 1) {
			m_StartPos	= node_list[1].pos;
			m_DestPos	= m_StartPos;
		}


		Vector3 point;
		Vector3 last_point = m_StartPos;

		//
		//	Determine how long the spline is
		//
		m_TotalDist = 0;
		for (int index = 0; index < node_list.Count (); index ++) {
			point = node_list[index].pos;
			
			//
			//	Add the distance of this point from the last point
			// into the total
			//
			m_TotalDist += (point - last_point).Length ();
			last_point = point;
		}

		//
		//	Initialize the spline depending on the path object type
		//
		if (m_PathObject.Is_Flag_Set (PathObjectClass::IS_VEHICLE)) {
			Initialize_Vehicle_Spline (node_list);
			m_Velocity = DEF_VEHICLE_VELOCITY;
		} else {
			Initialize_Human_Spline (node_list);
			m_Velocity = DEF_HUMAN_VELOCITY;
		}

		//
		//	Setup the variables to look-ahead 8 frames and switch
		// the new look-ahead after the unit has traveled 4 frames
		//
		float approx_frames	= (m_TotalDist * ASSUMED_FPS) / m_Velocity;
		m_LookAheadTime		= 8.0F / approx_frames;
		m_LookAheadDist		= (m_Velocity * 4.0F) / ASSUMED_FPS;

		//
		//	Vehicles automatically look ahead to each turn, so make sure
		// we don't skip one...
		//
		if (m_PathObject.Is_Flag_Set (PathObjectClass::IS_VEHICLE)) {
			m_LookAheadTime = m_LookAheadTime / 5.0F;
		}

		/*if (m_TotalDist > 0) {
			m_LookAheadTime		= 2.0F / m_TotalDist;
			m_LookAheadDist		= 4.0F;
		}*/

		m_SplineTime	= m_StartTime;
		m_ExpectedPos	= m_StartPos;
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Set_Path_Object
//
/////////////////////////////////////////////////////////////////////////////////
void
PathClass::Set_Path_Object (PathObjectClass &path_object)
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
PathClass::Get_Path_Object (PathObjectClass &path_object) const
{
	path_object = m_PathObject;
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////////////////////////////////
void
PathClass::Save (ChunkSaveClass &csave)
{
	if (m_Spline != NULL) {
		
		//
		//	Use the spline's factory to save it to its own chunk
		//
		csave.Begin_Chunk (CHUNKID_SPLINE);
			csave.Begin_Chunk (m_Spline->Get_Factory ().Chunk_ID ());
			m_Spline->Get_Factory ().Save (csave, m_Spline);
			csave.End_Chunk ();
		csave.End_Chunk ();
	}

	csave.Begin_Chunk (CHUNKID_VARIABLES);

		//
		//	Save each variable to its own microchunk
		//
		WRITE_MICRO_CHUNK (csave, VARID_STATE,						m_State);
		WRITE_MICRO_CHUNK (csave, VARID_TRAVERSAL_TYPE,			m_TraversalType);
		WRITE_MICRO_CHUNK (csave, VARID_START_POS,				m_StartPos);
		WRITE_MICRO_CHUNK (csave, VARID_DEST_POS,					m_DestPos);
		WRITE_MICRO_CHUNK (csave, VARID_EXPECTED_POS,			m_ExpectedPos);
		WRITE_MICRO_CHUNK (csave, VARID_LOOK_AHEAD_DIST,		m_LookAheadDist);
		WRITE_MICRO_CHUNK (csave, VARID_LOOK_AHEAD_TIME,		m_LookAheadTime);
		WRITE_MICRO_CHUNK (csave, VARID_MOVEMENT_RADIUS,		m_MovementRadius);
		WRITE_MICRO_CHUNK (csave, VARID_SPLINE_TIME,				m_SplineTime);
		WRITE_MICRO_CHUNK (csave, VARID_VELOCITY,					m_Velocity);		
		WRITE_MICRO_CHUNK (csave, VARID_CURRENT_ACTION,			m_CurrentAction);
		WRITE_MICRO_CHUNK (csave, VARID_MOVEMENT_DIRECTIONS,	m_MovementDirections);		
		WRITE_MICRO_CHUNK (csave, VARID_PATH_OBJECT,				m_PathObject);
		WRITE_MICRO_CHUNK (csave, VARID_START_TIME,				m_StartTime);
		WRITE_MICRO_CHUNK (csave, VARID_END_TIME,					m_EndTime);
		WRITE_MICRO_CHUNK (csave, VARID_ISLOOPING,				m_IsLooping);
		WRITE_MICRO_CHUNK (csave, VARID_TOTAL_DIST,				m_TotalDist);
		
		PathClass *this_ptr = this;
		WRITE_MICRO_CHUNK (csave, VARID_OLD_PTR,					this_ptr);

		//
		//	Save each of the action nodes for this path
		//
		for (int index = 0; index < m_PathActions.Count (); index ++) {
			PATH_NODE &node = m_PathActions[index];
			WRITE_MICRO_CHUNK (csave, VARID_PATH_ACTION,	node);
		}

	csave.End_Chunk ();
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////////////////////////////////
void
PathClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {
			
			case CHUNKID_SPLINE:
			{				
				//
				//	Use the spline's factory to load it from disk
				//
				if (cload.Open_Chunk ()) {
					PersistFactoryClass *factory = SaveLoadSystemClass::Find_Persist_Factory (cload.Cur_Chunk_ID ());
					if (factory != NULL) {
						m_Spline = (Curve3DClass *)factory->Load (cload);
					}
					cload.Close_Chunk ();				
				}
			}
			break;

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////
void
PathClass::Load_Variables (ChunkLoadClass &cload)
{
	PathClass *old_ptr = NULL;

	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK (cload, VARID_STATE,						m_State);
			READ_MICRO_CHUNK (cload, VARID_TRAVERSAL_TYPE,			m_TraversalType);
			READ_MICRO_CHUNK (cload, VARID_START_POS,					m_StartPos);
			READ_MICRO_CHUNK (cload, VARID_DEST_POS,					m_DestPos);
			READ_MICRO_CHUNK (cload, VARID_EXPECTED_POS,				m_ExpectedPos);
			READ_MICRO_CHUNK (cload, VARID_LOOK_AHEAD_DIST,			m_LookAheadDist);
			READ_MICRO_CHUNK (cload, VARID_LOOK_AHEAD_TIME,			m_LookAheadTime);
			READ_MICRO_CHUNK (cload, VARID_MOVEMENT_RADIUS,			m_MovementRadius);
			READ_MICRO_CHUNK (cload, VARID_SPLINE_TIME,				m_SplineTime);
			READ_MICRO_CHUNK (cload, VARID_VELOCITY,					m_Velocity);		
			READ_MICRO_CHUNK (cload, VARID_CURRENT_ACTION,			m_CurrentAction);
			READ_MICRO_CHUNK (cload, VARID_MOVEMENT_DIRECTIONS,	m_MovementDirections);		
			READ_MICRO_CHUNK (cload, VARID_PATH_OBJECT,				m_PathObject);
			READ_MICRO_CHUNK (cload, VARID_START_TIME,				m_StartTime);
			READ_MICRO_CHUNK (cload, VARID_END_TIME,					m_EndTime);
			READ_MICRO_CHUNK (cload, VARID_ISLOOPING,					m_IsLooping);
			READ_MICRO_CHUNK (cload, VARID_TOTAL_DIST,				m_TotalDist);
			READ_MICRO_CHUNK (cload, VARID_OLD_PTR,					old_ptr);			

			case VARID_PATH_ACTION:
			{
				//
				//	Read the action information from disk and add it to our list
				//
				PATH_NODE node;
				LOAD_MICRO_CHUNK (cload, node);
				m_PathActions.Add (node);
			}
			break;
		}

		cload.Close_Micro_Chunk ();
	}

	//
	//	Register our old ptr so other objects can remap to us
	//
	if (old_ptr != NULL) {
		SaveLoadSystemClass::Register_Pointer( old_ptr, this );
	}

	return ;
}
