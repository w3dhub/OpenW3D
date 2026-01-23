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
 *                     $Archive:: /Commando/Code/wwphys/Path.h            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/06/01 11:01a                                              $*
 *                                                                                             *
 *                    $Revision:: 27                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __COMMANDO_PATH_H
#define __COMMANDO_PATH_H

#include "vector.h"
#include "vector3.h"
#include "hermitespline.h"
#include "PathObject.h"
#include "binheap.h"
#include "refcount.h"


/////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////
class WaypathClass;
class WaypointClass;
class PathSolveClass;
class ChunkSaveClass;
class ChunkLoadClass;


/////////////////////////////////////////////////////////////////////////
//
//	PathClass
//
/////////////////////////////////////////////////////////////////////////
class PathClass : public RefCountClass
{
public:

	/////////////////////////////////////////////////////////////////////////
	// Public data types
	/////////////////////////////////////////////////////////////////////////
	typedef enum
	{
		STATE_TRAVERSING_PATH					= 1,
		STATE_ACTION_REQUIRED,
		STATE_PATH_COMPLETE,

		FIRST_ERROR									= 100,
		ERROR_DRIFTED_FROM_PATH,
		ERROR_GENERAL,
		ERROR_NOT_INITIALIZED,

	}	STATE_DESC;

	typedef enum
	{
		LINEAR	= 0,
		SPLINE
	}	TRAVERSAL_TYPE;

	typedef enum
	{
		MOVE_X	= 1,
		MOVE_Y	= 2,
		MOVE_Z	= 4,
	}	MOVEMENT_DIRECTIONS;

	typedef enum
	{
		ACTION_NONE			= 0,
		ACTION_JUMP,
		ACTION_LEAP,
		ACTION_LADDER,
		ACTION_MECHANISM

	}	ACTION_ID;

	/////////////////////////////////////////////////////////////////////////
	// Public constructors/destructors
	/////////////////////////////////////////////////////////////////////////
	PathClass (void);
	~PathClass (void);

	/////////////////////////////////////////////////////////////////////////
	// Public methods
	/////////////////////////////////////////////////////////////////////////

	//
	//	Path evaluation
	//
	bool					Evaluate_Next_Point (const Vector3 &curr_pos, Vector3 &new_pos);

	const Vector3 &	Get_Start_Pos (void) const						{ return m_StartPos; }
	const Vector3 &	Get_Dest_Pos (void) const						{ return m_DestPos; }
	const Vector3 &	Get_Next_Pos (void) const						{ return m_ExpectedPos; }

	void					Set_Movement_Radius (float radius)			{ m_MovementRadius = radius; }
	float					Get_Movement_Radius (void) const				{ return m_MovementRadius; }

	float					Get_Curve_Sharpness (Vector3 *position) const;
	float					Get_Last_Eval_Time (void) const;

	float					Get_Remaining_Path_Length (void);

	ACTION_ID			Get_Action_Type (void);
	void					Get_Action_Destination (Vector3 &dest);
	void					Get_Action_Entrance (Vector3 &position);
	uint32				Get_Action_Mechanism (void);

	//
	//	Velocity methods
	//
	float					Get_Velocity (void) const		{ return m_Velocity; }
	void					Set_Velocity (float velocity)	{ m_Velocity = velocity; }


	//
	// Traversal type methods
	//
	TRAVERSAL_TYPE		Get_Traversal_Type (void) const;
	void					Set_Traversal_Type (TRAVERSAL_TYPE type);

	//
	// Movement direction control
	//
	void					Set_Movement_Directions (int directions)	{ m_MovementDirections = directions; }
	int					Get_Movement_Directions (void) const		{ return m_MovementDirections; }

	//
	// Initialization methods
	//
	void					Initialize (WaypathClass *waypath, int start_pt_id, int end_pt_id);
	void					Initialize (PathSolveClass &path_solve);
	void					Initialize (const Vector3 &start, const Vector3 &end);

	//
	// Initialization methods
	//
	STATE_DESC			Get_State (void) const	{ return m_State; }

	//
	// Traversing object
	//
	void					Set_Path_Object (PathObjectClass &path_object);
	void					Get_Path_Object (PathObjectClass &path_object) const;

	//
	//	Debug methods
	//
	void					Display_Path (bool onoff = true);

	//
	//	Save/load stuff
	//
	void					Save (ChunkSaveClass &csave);
	void					Load (ChunkLoadClass &cload);

	//
	//	Debug support
	//
	static bool			Are_Move_Vectors_Displayed (void)	{ return m_DisplayMoveVectors; }
	static void			Display_Move_Vectors (bool onoff)	{ m_DisplayMoveVectors = onoff; }

#ifdef WWDEBUG
	int					Get_Path_Vector_Length(void) {return(m_PathActions.Length());};
	int					Get_Path_Vector_Count(void) {return(m_PathActions.Count());};
#endif //WWDEBUG

protected:

	/////////////////////////////////////////////////////////////////////////
	// Protected data types
	/////////////////////////////////////////////////////////////////////////
	typedef struct _PATH_NODE
	{
		float			time;
		float			next_time;
		bool			tighten_spline;
		ACTION_ID	action_id;
		uint32		mechanism_id;
		Vector3		dest_pos;
		Vector3		pos;

		_PATH_NODE (void) :
			time (0),
			next_time (0),
			tighten_spline (false),
			action_id (ACTION_NONE),
			mechanism_id (0),
			dest_pos (0, 0, 0),
			pos (0, 0, 0) {}

		bool operator== (const _PATH_NODE &/* src*/)	{ return true; }
		bool operator!= (const _PATH_NODE &/* src*/)	{ return false; }

	} PATH_NODE;


	/////////////////////////////////////////////////////////////////////////
	// Protected methods
	/////////////////////////////////////////////////////////////////////////
	void					Initialize_Spline (DynamicVectorClass<PATH_NODE> &node_list);
	void					Initialize_Vehicle_Spline (DynamicVectorClass<PATH_NODE> &node_list);
	void					Initialize_Human_Spline (DynamicVectorClass<PATH_NODE> &node_list);
	void					Clip_Spline_To_Pathfind_Data (DynamicVectorClass<PATH_NODE> &node_list, PathSolveClass &path_solve);

	void					Add_Waypath_Data (DynamicVectorClass<PATH_NODE> &node_list, WaypathClass *waypath, int start_index, int end_index);
	void					Add_Waypoint_Info_To_Node_List (DynamicVectorClass<PATH_NODE> &node_list, WaypointClass *waypoint, WaypointClass *next_point);


	typedef DynamicVectorClass<AABoxClass *>		BOX_LIST;
	bool					Is_Point_In_Boxes (const Vector3 &point, BOX_LIST &box_list);

	void					Clip_Control_Point (const Vector3 &start_point, Vector3 *point, BOX_LIST &sector_list, BOX_LIST &portal_list);

	//
	//	Save/load methods
	//
	void					Load_Variables (ChunkLoadClass &cload);

private:

	/////////////////////////////////////////////////////////////////////////
	// Private member data
	/////////////////////////////////////////////////////////////////////////
	STATE_DESC									m_State;
	TRAVERSAL_TYPE								m_TraversalType;

	Vector3										m_StartPos;
	Vector3										m_DestPos;
	Vector3										m_ExpectedPos;
	float											m_LookAheadDist;
	float											m_LookAheadTime;
	float											m_MovementRadius;
	float											m_SplineTime;
	float											m_Velocity;
	float											m_TotalDist;

	DynamicVectorClass<PATH_NODE>			m_PathActions;
	int											m_CurrentAction;
	int											m_MovementDirections;

	Curve3DClass *								m_Spline;

	PathObjectClass							m_PathObject;

	float											m_StartTime;
	float											m_EndTime;
	bool											m_IsLooping;

	static bool									m_DisplayMoveVectors;
};


/////////////////////////////////////////////////////////////////////////
//	Inlines
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
//	Get_Action_Type
/////////////////////////////////////////////////////////////////////////
inline PathClass::ACTION_ID
PathClass::Get_Action_Type (void)
{
	ACTION_ID retval = ACTION_NONE;
	if (m_CurrentAction >= 0 && m_CurrentAction < m_PathActions.Count ()) {
		retval = m_PathActions[m_CurrentAction].action_id;
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////
//	Get_Action_Destination
/////////////////////////////////////////////////////////////////////////
inline void
PathClass::Get_Action_Destination (Vector3 &dest)
{
	if (m_CurrentAction >= 0 && m_CurrentAction < m_PathActions.Count ()) {
		dest = m_PathActions[m_CurrentAction].dest_pos;
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////
//	Get_Action_Entrance
/////////////////////////////////////////////////////////////////////////
inline void
PathClass::Get_Action_Entrance (Vector3 &position)
{
	if (m_CurrentAction >= 0 && m_CurrentAction < m_PathActions.Count ()) {
		position = m_PathActions[m_CurrentAction].pos;
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////
//	Get_Action_Mechanism
/////////////////////////////////////////////////////////////////////////
inline uint32
PathClass::Get_Action_Mechanism (void)
{
	uint32 mechanism_id = 0;

	if (m_CurrentAction >= 0 && m_CurrentAction < m_PathActions.Count ()) {
		mechanism_id = m_PathActions[m_CurrentAction].mechanism_id;
	}

	return mechanism_id;
}


#endif //__COMMANDO_PATH_H