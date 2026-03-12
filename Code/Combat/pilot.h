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
 *                     $Archive:: /Commando/Code/Combat/pilot.h           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 7/13/01 2:39p                                               $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __PILOT_H
#define __PILOT_H

#include "vector3.h"
#include "gameobjref.h"
#include "matrix3d.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class PathClass;
class SmartGameObj;
class ChunkSaveClass;
class ChunkLoadClass;


////////////////////////////////////////////////////////////////
//
//	PilotClass
//
////////////////////////////////////////////////////////////////
class PilotClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public data types
	////////////////////////////////////////////////////////////////
	typedef enum
	{
		MODE_HOVER						= 0,
		MODE_FLY_TO_POINT,
		MODE_CIRCLE_POINT,
	}	MODE;


	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	PilotClass (void);
	virtual ~PilotClass (void)	{ Free (); }

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Startup/shutdown
	//
	void					Initialize (SmartGameObj *game_obj);
	void					Reset (void);

	//
	//	Mode control
	// Destination - point for the vtol to try to fly to
	// Arrived_Distance - radius around the destination where we are "close enough"
	// Hover_Distance - radius around the destination where we try to slow down and maintain
	//                  our position.  Should be at least 10m more than Arrived_Distance!
	// Target - point to aim at (if NULL is passed, vehicle will not aim in any particular direction)
	//

	void					Set_Destination (const Vector3 &dest)	{ m_FinalDest = dest; m_NextPoint = dest; }
	const Vector3 &	Get_Destination (void) const				{ return m_FinalDest; }

	void					Set_Next_Point (const Vector3 &pos)		{ m_NextPoint = pos; }
	const Vector3 &	Get_Next_Point (void) const				{ return m_NextPoint; }

	void					Set_Arrived_Dist (float dist)				{ m_ArrivedDist = dist; }
	float					Get_Arrived_Dist (void) const				{ return m_ArrivedDist; }

	void					Set_Hover_Dist (float dist)			{ m_HoverDist = dist; }
	float					Get_Hover_Dist (void) const			{ return m_HoverDist; }

	void					Set_Target (const Vector3 *target);
	bool					Get_Target (Vector3 &target) const		{ target = m_TargetLocation; return m_FaceTarget; }

	void					Set_Mode (MODE mode);
	MODE					Get_Mode (void) const						{ return m_Mode; }

	void					Set_Circle_Dist (float circle_dist)		{ m_CircleDist = circle_dist; }
	float					Get_Circle_Dist (void) const				{ return m_CircleDist; }

	void					Set_Current_Path (PathClass *path)		{ m_CurrentPath = path; }
	PathClass *			Get_Current_Path (void) const				{ return m_CurrentPath; }

	void					Set_Circle_Angle (float angle);
	void					Set_Circle_Pos (const Vector3 &pos);
	float					Get_Circle_Angle (void) const				{ return m_CircleAngle; }
	float					Get_Current_Circle_Angle (void) const;

	void					Set_Circle_Bounds (float min_angle = -DEG_TO_RADF(180), float max_angle = DEG_TO_RADF(180));

	//
	//	Z interpretation
	//
	bool					Get_Is_Exact_Z_Important (void) const	{ return m_IsExactZImportant; }
	void					Set_Is_Exact_Z_Important (bool onoff)	{ m_IsExactZImportant = onoff; }

	//
	//	Timeslice control
	//
	bool					Think (void);

	//
	//	Status methods
	//
	bool					Has_Arrived (void) const;

	//
	//	Speed methods
	//
	float					Get_Aggressivness (void) const	{ return m_Aggressiveness; }
	void					Set_Aggressivness (float factor) { m_Aggressiveness = factor; }

	void					Set_Max_Speed (float max_speed)	{ m_MaxSpeed = max_speed; }
	float					Get_Max_Speed (void) const			{ return m_MaxSpeed;}

	void					Set_Speed_Factor (float factor)	{ m_SpeedFactor = factor; }
	float					Get_Speed_Factor (void) const		{ return m_SpeedFactor; }

	//
	//	Save/load stuff
	//
	void					Save (ChunkSaveClass &csave);
	void					Load (ChunkLoadClass &cload);

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void					Free () { }

	//
	//	Mode handlers
	//
	void					Process_Fly_To_Point (void);
	void					Process_Hover (void);
	void					Process_Circle_Point (void);

	//
	//	Calculation methods
	//
	void					Calculate_Turn_Sharpness (void);
	void					Calculate_Strafe_Speed (void);
	void					Calculate_Forward_Speed (float distance);
	void					Calculate_Lift_Speed (void);
	float					Calculate_Desired_Relative_Facing (void);

	void					Check_Completion (void);
	void					Update_Transform (void);
	float					Determine_Preferred_Height (void);

	//
	//	Game object control methods
	//
	void					Apply_Controls (void);

	//
	//	Information methods
	//
	void					Get_Object_Space_Velocity (Vector3 &vel_vector) const;

	//
	//	Save/load methods
	//
	void					Load_Variables (ChunkLoadClass &cload);

private:

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	Vector3				m_FinalDest;
	Vector3				m_NextPoint;

	Matrix3D				m_CurrentTM;
	Vector3				m_ObjSpaceDest;
	Vector3				m_ObjSpaceWaypoint;

	PathClass *			m_CurrentPath;
	SmartGameObj *		m_GameObj;
	float					m_MaxSpeed;
	float					m_SpeedFactor;
	float					m_Aggressiveness;
	float					m_ArrivedDist;
	float					m_HoverDist;

	bool					m_IsExactZImportant;
	bool					m_FaceTarget;
	Vector3				m_TargetLocation;

	MODE					m_Mode;

	float					m_ForwardSpeed;
	float					m_StrafeSpeed;
	float					m_LiftSpeed;
	float					m_TurnSharpness;

	float					m_CircleAngle;
	float					m_CircleDist;
	float					m_MinCircleAngle;
	float					m_MaxCircleAngle;
};


#endif //__PILOT_H

