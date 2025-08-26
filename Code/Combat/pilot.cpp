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
 *                     $Archive:: /Commando/Code/Combat/pilot.cpp         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/26/01 5:07p                                              $*
 *                                                                                             *
 *                    $Revision:: 18                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "pilot.h"

#include "smartgameobj.h"
#include "matrix3d.h"
#include "movephys.h"
#include "vehiclephys.h"
#include "Path.h"
#include "pscene.h"
#include "vehicle.h"
#include "soldier.h"
#include "heightdb.h"
#include "debug.h"


////////////////////////////////////////////////////////////////
//	Save/Load constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES				= 0x11040504,
};

enum
{
	VARID_FINAL_DEST				= 1,
	VARID_NEXT_POINT,
	VARID_CURRENT_TM,
	VARID_OBJ_SPACE_DEST,
	VARID_OBJ_SPACE_WPOINT,
	VARID_GAMEOBJ_PTR,
	VARID_MAX_SPEED,
	VARID_SPEED_FACTOR,
	VARID_AGGRESSIVENESS,
	VARID_ARRIVED_DIST,
	VARID_HOVERDIST,
	VARID_FACE_TARGET,
	VARID_TARGET_LOCATION,
	VARID_MODE,
	VARID_FORWARD_SPEED,
	VARID_STRAFE_SPEED,
	VARID_LIFT_SPEED,
	VARID_TURN_SHARPNESS,
	VARID_CIRCLE_ANGLE,
	VARID_CIRCLE_DIST,
	VARID_MIN_CIRCLE_ANGLE,
	VARID_MAX_CIRCLE_ANGLE,
	VARID_ISEXACT_Z_IMPORT,
	VARID_PATH_PTR
};


////////////////////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////////////////////
const float MAX_HEIGHT_DELTA		= 2.5F;
const float MAX_STRAFE_DELTA		= 10.0F;
const float MAX_FORWARD_DELTA		= 10.0F;

////////////////////////////////////////////////////////////////
//	Clamp_Angle
////////////////////////////////////////////////////////////////
inline float
Clamp_Angle (float angle, float min_angle, float max_angle)
{
	//
	//	Make sure all the parameters are in the same range
	//
	angle			= WWMath::Wrap (angle,		0, DEG_TO_RADF (360));
	min_angle	= WWMath::Wrap (min_angle,	0, DEG_TO_RADF (360));
	max_angle	= WWMath::Wrap (max_angle,	0, DEG_TO_RADF (360));

	float result = angle;

	if (min_angle <= max_angle) {
		
		//
		//	Handle the 'typical' case where there is no 360-mark wrap-around
		//
		if (angle < min_angle) {
			result = min_angle;
		} else if (angle > max_angle) {		
			result = max_angle;
		}

	} else {
		
		//
		//	Handle the 360-mark wrap-around case
		//
		if (angle < min_angle && angle > max_angle) {
			float min_delta = min_angle - angle;
			float max_delta = angle - max_angle;

			//
			//	Which edge is the angle closer to?
			//
			if (min_delta < max_delta) {
				result = min_angle;
			} else {
				result = max_angle;
			}
		}
	}

	return result;
}


////////////////////////////////////////////////////////////////
//
//	PilotClass
//
////////////////////////////////////////////////////////////////
PilotClass::PilotClass (void)
	:	m_FinalDest (0, 0, 0),
		m_NextPoint (0, 0, 0),
		m_GameObj (NULL),
		m_MaxSpeed (20.0F),
		m_SpeedFactor (1.0F),
		m_Aggressiveness (0.25F),
		m_ArrivedDist (0),
		m_HoverDist (15.0F),
		m_FaceTarget (false),
		m_TargetLocation (0, 0, 0),
		m_Mode (MODE_HOVER),
		m_ForwardSpeed (0),
		m_StrafeSpeed (0),
		m_LiftSpeed (0),
		m_TurnSharpness (0),
		m_CurrentPath (NULL),
		m_CircleAngle (0),
		m_CircleDist (0),
		m_MinCircleAngle (-DEG_TO_RADF(180)),
		m_MaxCircleAngle (DEG_TO_RADF(180)),
		m_IsExactZImportant (false)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Object_Space_Velocity
//
//	Returns the object-space velocity vector
//
////////////////////////////////////////////////////////////////
void
PilotClass::Get_Object_Space_Velocity (Vector3 &vel_vector) const
{
	//
	//	Get a pointer to the physics object for this vehicle
	//
	MoveablePhysClass *phys_obj = m_GameObj->Peek_Physical_Object()->As_MoveablePhysClass ();
	WWASSERT (phys_obj != NULL);

	//
	//	Get the world-space velocity vector for this vehicle and transform
	// it into object space.
	//
	Vector3 vel_vector_world;
	phys_obj->Get_Velocity (&vel_vector_world);
	Matrix3D::Inverse_Rotate_Vector (m_GameObj->Get_Transform (), vel_vector_world, &vel_vector);			
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Target
//
////////////////////////////////////////////////////////////////
void
PilotClass::Set_Target (const Vector3 *target)
{
	if (target != NULL) {
		m_TargetLocation = *target;
		
		if (m_Mode != MODE_CIRCLE_POINT) {
			m_FaceTarget = true;
		}

	} else {
		m_FaceTarget = false;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////
void
PilotClass::Initialize (SmartGameObj *game_obj)
{
	m_GameObj		= game_obj;
	m_CurrentPath	= NULL;

	//
	//	If the game object is flying a vehicle, then get the vehicle instead
	//
	SoldierGameObj	*soldier_game_obj = game_obj->As_SoldierGameObj ();
	if (soldier_game_obj != NULL) {
		VehicleGameObj	*vehicle_game_obj = soldier_game_obj->Get_Profile_Vehicle ();
		if (vehicle_game_obj != NULL) {
			m_GameObj = vehicle_game_obj;
		}
	}

	//
	//	By default assume our final destination is our current location
	//
	game_obj->Get_Position (&m_FinalDest);
	game_obj->Get_Position (&m_NextPoint);	

	//
	//	Determine if this game object is a vehicle or not (it better be...)
	//
	VehicleGameObj *vehicle = m_GameObj->As_VehicleGameObj ();
	if (vehicle != NULL) {
		
		//
		//	Start this vehicle's engine's running
		//
		if (vehicle->Is_Engine_Enabled () == false) {
			vehicle->Enable_Engine (true);
		}
	}
	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Calculate_Desired_Relative_Facing
//
////////////////////////////////////////////////////////////////
float
PilotClass::Calculate_Desired_Relative_Facing (void)
{
	float base_angle	= 0;
	
	//
	//	Should we be facing the destination or the current target?
	//
	if (m_FaceTarget) {

		//
		//	Convert the target location from world to object space
		//
		Vector3 target_pos;
		Matrix3D::Inverse_Transform_Vector (m_CurrentTM, m_TargetLocation, &target_pos);
		
		//
		//	Calculate the absolute turn angle
		//
		base_angle	= WWMath::Atan2 (target_pos.Y, target_pos.X);
		base_angle	= WWMath::Wrap (base_angle, 0, WWMATH_PI * 2);

	} else if (m_Mode != MODE_HOVER) {

		//
		//	Calculate the absolute turn angle
		//
		base_angle	= WWMath::Atan2 (m_ObjSpaceWaypoint.Y, m_ObjSpaceWaypoint.X);
		base_angle	= WWMath::Wrap (base_angle, 0, WWMATH_PI * 2);
	
	} 
	
	//
	//	Convert the angle from [0, 360] to [-180, 180]
	//
	return WWMath::Wrap (base_angle, -WWMATH_PI, WWMATH_PI);
}


////////////////////////////////////////////////////////////////
//
//	Think
//
////////////////////////////////////////////////////////////////
bool
PilotClass::Think (void)
{
	//
	//	Update the current
	//
	if (m_CurrentPath != NULL) {
		m_FinalDest = m_CurrentPath->Get_Dest_Pos ();
		m_NextPoint	= m_CurrentPath->Get_Next_Pos ();
	}

	//
	//	Let each mode think independently
	//
	switch (m_Mode)
	{
		case MODE_HOVER:
			Process_Hover ();
			break;

		case MODE_FLY_TO_POINT:
			Process_Fly_To_Point ();
			break;

		case MODE_CIRCLE_POINT:
			Process_Circle_Point ();
			break;
	}

	//
	//	Pass the current controls onto the game object
	//
	Apply_Controls ();	

	//
	//	Return true when we've arrived at the destination
	//
	return Has_Arrived ();
}


////////////////////////////////////////////////////////////////
//
//	Calculate_Strafe_Speed
//
////////////////////////////////////////////////////////////////
void
PilotClass::Calculate_Strafe_Speed (void)
{
	m_StrafeSpeed = 0;

	bool should_strafe = m_FaceTarget;
	if (should_strafe) {
		m_StrafeSpeed = m_ObjSpaceWaypoint.Y / MAX_STRAFE_DELTA;
		m_StrafeSpeed = WWMath::Clamp (m_StrafeSpeed, -1.0F, 1.0F);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Calculate_Forward_Speed
//
////////////////////////////////////////////////////////////////
void
PilotClass::Calculate_Forward_Speed (float distance)
{
	m_ForwardSpeed = 0;

	//
	//	Pick a speed based on our distance from the destination
	//
	m_ForwardSpeed	= distance / MAX_FORWARD_DELTA;		
	m_ForwardSpeed	= WWMath::Clamp (m_ForwardSpeed, -1.0F, 1.0F);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Calculate_Lift_Speed
//
////////////////////////////////////////////////////////////////
void
PilotClass::Calculate_Lift_Speed (void)
{
	//bool is_exact_height_important = (m_NextPoint != m_FinalDest);

	//
	//	Determine what z position we should aim for
	//
	float preferred_height	= m_NextPoint.Z;
	float flight_floor		= -5000.0F;
	if (m_IsExactZImportant == false) {

		flight_floor			= Determine_Preferred_Height ();
		preferred_height		= flight_floor;
		if (preferred_height < m_NextPoint.Z) {
			preferred_height = m_NextPoint.Z;
		}

	} else {
		preferred_height = m_NextPoint.Z;
	}

	//
	//	Get the vehicle's current position
	//
	Vector3 curr_pos;
	m_GameObj->Get_Position (&curr_pos);

	//
	//	Calculate the lift speed based on the change in altitude
	// Once the height delta is within MAX_HIEGHT_DELTA, we set the desired velocity to zero
	// and try to let the hovering code maintain the desired altitude.
	//
	float delta_z		= preferred_height - curr_pos.Z;
	float abs_delta_z = WWMath::Fabs(delta_z);

	if (m_Mode != MODE_HOVER || abs_delta_z > MAX_HEIGHT_DELTA) {
		m_LiftSpeed = delta_z / MAX_HEIGHT_DELTA;
		m_LiftSpeed = WWMath::Clamp (m_LiftSpeed, -1.0F, 1.0F);
	} else {
		m_LiftSpeed = 0.0f;
	}

	//
	//	Keep the vehicle from 'sinking' sharply
	//
	if (	m_IsExactZImportant == false &&
			m_Mode == MODE_FLY_TO_POINT &&
			delta_z < 0)
	{
		m_LiftSpeed = m_LiftSpeed * 0.25f;
	}

	//
	//	Slow down our forward speed to give us time to change altitude (if necessary)
	//
	if (curr_pos.Z < flight_floor) {
		//float new_speed = WWMath::Clamp (1.0F - m_LiftSpeed, 0.5F, 1.0F);
		//m_ForwardSpeed = min (m_ForwardSpeed, new_speed);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Calculate_Turn_Sharpness
//
////////////////////////////////////////////////////////////////
void
PilotClass::Calculate_Turn_Sharpness (void)
{
	float facing_angle = Calculate_Desired_Relative_Facing ();

	//
	//	Calculate how sharp we should turn to make this facing
	//
	m_TurnSharpness	= facing_angle / DEG_TO_RADF (45);
	m_TurnSharpness	= WWMath::Clamp (m_TurnSharpness, -1.0F, 1.0F);

	//
	//	If the vehicle needs to face its target, then make it turn faster then normal
	//
	if (m_FaceTarget || m_Mode == MODE_CIRCLE_POINT) {
		m_TurnSharpness	*= 2.0F;
		m_TurnSharpness	= min (m_TurnSharpness, 1.0F);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Process_Hover
//
////////////////////////////////////////////////////////////////
void
PilotClass::Process_Hover (void)
{
	// 
	// Update the "simplified" transform
	//
	Update_Transform ();

	// 
	// When hovering, we want to maintain a zero horizontal velocity
	// while moving towards our desired altitude
	//
	m_ForwardSpeed	= 0.0f;
	m_StrafeSpeed	= 0.0f;

	// 
	// Update the "simplified" transform
	//
	Update_Transform ();

	// 
	// When hovering, we want to maintain a zero horizontal velocity
	// while moving towards our desired altitude
	//
	m_ForwardSpeed = 0.0f;
	m_StrafeSpeed = 0.0f;

	//
	//	When hovering its valid to face a target, so update
	// the vehicle's facing
	//
	if (m_FaceTarget) {
		Calculate_Turn_Sharpness ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Process_Circle_Point
//
////////////////////////////////////////////////////////////////
void
PilotClass::Process_Circle_Point (void)
{
	//
	//	Reset all control inputs
	//
	m_ForwardSpeed	= 0;
	m_StrafeSpeed	= 0;
	m_LiftSpeed		= 0;

	//
	//	When hovering its valid to face a target, so update
	// the vehicle's facing
	//
	Update_Transform ();

	//
	//	Make sure we are facing the destination
	//
	Calculate_Turn_Sharpness ();

	//
	//	If we are facing the destination, then we can
	// start our 'circling' movement
	//
	if (::fabs (m_TurnSharpness) < 0.1F) {

		//
		//	Make sure the vehicle is the appropriate distance from the circle point
		//
		Calculate_Forward_Speed (m_ObjSpaceDest.X - m_CircleDist);
		Calculate_Lift_Speed ();

		//
		//	Calculate what our current circling angle is
		//
		Vector3 curr_pos;
		m_GameObj->Get_Position (&curr_pos);
		Vector3 delta = curr_pos - m_FinalDest;
		float curr_angle	= WWMath::Atan2 (delta.Y, delta.X);
		float angle_delta	= WWMath::Wrap (curr_angle - m_CircleAngle, -WWMATH_PI, WWMATH_PI);

		//
		//	Calculate how fast we should strafe in order to circle the target
		//
		m_StrafeSpeed = angle_delta / DEG_TO_RADF (5);
		m_StrafeSpeed = WWMath::Clamp (m_StrafeSpeed, -1.0F, 1.0F);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Process_Fly_To_Point
//
////////////////////////////////////////////////////////////////
void
PilotClass::Process_Fly_To_Point (void)
{
	//
	//	Get the vehicle's current transform and check to see if
	// the vehicle has arrived at its destination yet
	//
	Update_Transform ();
	Check_Completion ();

	//
	//	Calculate how sharp the vehicle needs to turn
	//
	Calculate_Turn_Sharpness ();

	//
	//	Calculate how fast the vehicle needs to move in each direction
	//
	Calculate_Strafe_Speed ();
	
	//
	//	If we are on a path then we need to calculate the forward speed
	// as if we are going full speed the whole way there
	//
	bool is_on_path = (m_NextPoint != m_FinalDest);
	if (is_on_path) {

		//
		//	Clamp the speed to -1, 0, or 1.
		//
		m_ForwardSpeed = m_ObjSpaceWaypoint.X;
		if (m_ForwardSpeed > 0) {
			m_ForwardSpeed = 1.0F;
		} else if (m_ForwardSpeed < 0) {
			m_ForwardSpeed = -1.0F;
		} else {
			m_ForwardSpeed = 0.0F;
		}

		// 
		// Reduce forward/back speed if we are mostly strafing.
		//
		if (WWMath::Fabs (m_StrafeSpeed) > 0.25f) {
			m_ForwardSpeed = m_ForwardSpeed * (1.25F - WWMath::Fabs (m_StrafeSpeed));
		}

		//
		//	Let the pilot slow down to handle sharp corners
		//
		if (WWMath::Fabs (m_TurnSharpness) > 0.65F) {
			m_ForwardSpeed = m_ForwardSpeed * (1.25F - WWMath::Fabs (m_TurnSharpness));
		}		

	} else {
		Calculate_Forward_Speed (m_ObjSpaceDest.X);
	}

	Calculate_Lift_Speed ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Check_Completion
//
////////////////////////////////////////////////////////////////
void
PilotClass::Check_Completion (void)
{
	//
	//	Get the vehicle's bounding box
	//
	MoveablePhysClass *phys_obj		= m_GameObj->Peek_Physical_Object ()->As_MoveablePhysClass ();
	RenderObjClass *render_obj			= phys_obj->Peek_Model ();
	AABoxClass bounding_box;
	render_obj->Get_Obj_Space_Bounding_Box (bounding_box);

	//
	//	Calculate the distance remaining on the path
	//
	float dist_to_goal = 0;
	if (m_CurrentPath != NULL) {
		float dist_on_path = m_CurrentPath->Get_Remaining_Path_Length ();
		dist_to_goal = (m_ObjSpaceWaypoint.Length () + dist_on_path);		
	} else {
		dist_to_goal = m_ObjSpaceDest.Length ();
	}


	//
	//	Take into account the 'arrived distance' setting.  We basically fly towards the
	// closest point on a sphere around the destination point.  m_ObjSpaceDest will
	// be initialized to that point transformed into the vehicle's coordinate system.
	//
	if (dist_to_goal > m_ArrivedDist * m_ArrivedDist) {
		Vector3 arrived_len = m_ObjSpaceDest;
		arrived_len.Normalize ();
		arrived_len *= m_ArrivedDist;
		m_ObjSpaceDest -= arrived_len;
		m_ObjSpaceDest += bounding_box.Center;
	}

	// 
	// Once we come within m_HoverDist of the desired position we switch into "hover" mode.
	//
	if (dist_to_goal < m_HoverDist && m_NextPoint == m_FinalDest) {
		Vector2 xy_delta(m_ObjSpaceWaypoint.X, m_ObjSpaceWaypoint.Y);
		if (xy_delta.Length2() < m_HoverDist * m_HoverDist) {
			Set_Mode(MODE_HOVER);

			//
			// If we don't have a target, we need to create a fake target so that
			// the orientation of the vehicle doesn't drift.
			//
			if (m_FaceTarget == false) {
				Vector3 fake_target;
				Matrix3D::Transform_Vector(m_CurrentTM,Vector3(1000.0f,0.0f,0.0f),&fake_target);
				Set_Target(&fake_target);
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Transform
//
////////////////////////////////////////////////////////////////
void
PilotClass::Update_Transform (void)
{
	Matrix3D obj_tm = m_GameObj->Get_Transform ();
	
	//
	//	Build a transform that only uses the facing and position
	// of the vehicle.
	//
	m_CurrentTM.Make_Identity ();
	m_CurrentTM.Rotate_Z (obj_tm.Get_Z_Rotation ());
	m_CurrentTM.Set_Translation (obj_tm.Get_Translation ());

	//
	//	Now convert the destination from world space to object space
	//
	Matrix3D::Inverse_Transform_Vector (m_CurrentTM, m_FinalDest, &m_ObjSpaceDest);
	Matrix3D::Inverse_Transform_Vector (m_CurrentTM, m_NextPoint, &m_ObjSpaceWaypoint);

	//
	//	If its not important for the vehicle to tightly follow the Z value
	// of its points, then simply zero out the Z component
	//
	if (m_IsExactZImportant == false) {
		m_ObjSpaceWaypoint.Z = 0;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Determine_Preferred_Height
//
////////////////////////////////////////////////////////////////
float
PilotClass::Determine_Preferred_Height (void)
{
	float height = m_NextPoint.Z;
	if (m_Mode != MODE_FLY_TO_POINT) {
		return height;
	}

	//
	//	Get a pointer to the physics object for this vehicle
	//
	MoveablePhysClass *phys_obj = m_GameObj->Peek_Physical_Object()->As_MoveablePhysClass ();
	WWASSERT (phys_obj != NULL);

	//
	//	Get the world-space velocity vector for this vehicle and transform
	// it into object space.
	//
	Vector3 vel_vector_world;
	phys_obj->Get_Velocity (&vel_vector_world);

	//
	//	Try to determine where the aircraft will be in 1 and 2 seconds
	// from now.
	//	
	Vector3 curr_pos;
	m_GameObj->Get_Position (&curr_pos);

	RenderObjClass *render_obj			= phys_obj->Peek_Model ();
	const AABoxClass &bounding_box	= render_obj->Get_Bounding_Box ();

	Vector3 curr_pos1 = curr_pos;
	Vector3 curr_pos2 = curr_pos + Vector3 (bounding_box.Extent.X, bounding_box.Extent.Y, 0);
	Vector3 curr_pos3 = curr_pos + Vector3 (-bounding_box.Extent.X, -bounding_box.Extent.Y, 0);
	Vector3 curr_pos4 = curr_pos + Vector3 (bounding_box.Extent.X, -bounding_box.Extent.Y, 0);
	Vector3 curr_pos5 = curr_pos + Vector3 (-bounding_box.Extent.X, bounding_box.Extent.Y, 0);

	Vector3 future_pos1 = curr_pos1 + (vel_vector_world * 1.25F);
	Vector3 future_pos2 = curr_pos2 + (vel_vector_world * 1.25F);
	Vector3 future_pos3 = curr_pos3 + (vel_vector_world * 1.25F);
	Vector3 future_pos4 = curr_pos4 + (vel_vector_world * 1.25F);
	Vector3 future_pos5 = curr_pos5 + (vel_vector_world * 1.25F);

	Vector3 future_pos6 = curr_pos1 + (vel_vector_world * 2.25F);
	Vector3 future_pos7 = curr_pos2 + (vel_vector_world * 2.25F);
	Vector3 future_pos8 = curr_pos3 + (vel_vector_world * 2.25F);
	Vector3 future_pos9 = curr_pos4 + (vel_vector_world * 2.25F);
	Vector3 future_pos10 = curr_pos5 + (vel_vector_world * 2.25F);

	future_pos1.Z = curr_pos.Z;
	future_pos2.Z = curr_pos.Z;
	future_pos3.Z = curr_pos.Z;
	future_pos4.Z = curr_pos.Z;
	future_pos5.Z = curr_pos.Z;
	future_pos6.Z = curr_pos.Z;
	future_pos7.Z = curr_pos.Z;
	future_pos8.Z = curr_pos.Z;
	future_pos9.Z = curr_pos.Z;
	future_pos10.Z = curr_pos.Z;

	
	//
	//	Now, lookup the preferred height for these future positions
	//
	float height1 = HeightDBClass::Get_Height (future_pos1);
	float height2 = HeightDBClass::Get_Height (future_pos2);
	float height3 = HeightDBClass::Get_Height (future_pos3);
	float height4 = HeightDBClass::Get_Height (future_pos4);
	float height5 = HeightDBClass::Get_Height (future_pos5);
	float height6 = HeightDBClass::Get_Height (future_pos6);
	float height7 = HeightDBClass::Get_Height (future_pos7);
	float height8 = HeightDBClass::Get_Height (future_pos8);
	float height9 = HeightDBClass::Get_Height (future_pos9);
	float height10 = HeightDBClass::Get_Height (future_pos10);

	//
	//	Return the largest height to the caller
	//
	height	= max (height1, height2);
	height	= max (height, height3);
	height	= max (height, height4);
	height	= max (height, height5);
	height	= max (height, height6);
	height	= max (height, height7);
	height	= max (height, height8);
	height	= max (height, height9);
	height	= max (height, height10);
	return height;
}


////////////////////////////////////////////////////////////////
//
//	Apply_Controls
//
////////////////////////////////////////////////////////////////
void
PilotClass::Apply_Controls (void)
{
	//
	//	If the vehicle has a driver, then pass the controls onto the driver
	// instead...
	//
	SmartGameObj *game_obj	= m_GameObj;
	VehicleGameObj *vehicle = m_GameObj->As_VehicleGameObj ();
	if (vehicle != NULL) {
		if (vehicle->Get_Driver () != NULL) {
			//game_obj = vehicle->Get_Driver ();
		}
	}

	//
	//	Calculate our (current) normalized speeds
	//
	Vector3 vel_vector;
	Get_Object_Space_Velocity (vel_vector);
	float curr_forward_speed	= WWMath::Clamp ((vel_vector.X / m_MaxSpeed), -1.0F, 1.0F);
	float curr_strafe_speed		= WWMath::Clamp ((vel_vector.Y / m_MaxSpeed), -1.0F, 1.0F);
	float curr_lift_speed		= WWMath::Clamp ((vel_vector.Z / m_MaxSpeed), -1.0F, 1.0F);

	//
	//	Calculate how much we need to accelerate or decelerate in the three directions
	// in order to meet our target speeds
	//
	float forward_accel	= 0.0f;
	float strafe_accel	= 0.0f;
	float lift_accel		= 0.0f;

	if (m_Mode == MODE_HOVER) {
		forward_accel	= 5.5f * (m_ForwardSpeed - curr_forward_speed) + 0.25f * m_ObjSpaceDest.X;
		strafe_accel	= 3.0f * (m_StrafeSpeed - curr_strafe_speed) + 0.5f * m_ObjSpaceDest.Y;
		lift_accel		= 1.0f * (m_LiftSpeed - curr_lift_speed) + 0.5f * m_ObjSpaceDest.Z;

		forward_accel	= WWMath::Clamp(forward_accel,-1.0f,1.0f);
		strafe_accel	= WWMath::Clamp(strafe_accel,-1.0f,1.0f);
		lift_accel		= WWMath::Clamp(lift_accel,-1.0f,1.0f);

	} else {
		forward_accel	= WWMath::Clamp (5.0f * (m_ForwardSpeed - curr_forward_speed), -1.0F, 1.0F);
		strafe_accel	= WWMath::Clamp (1.0f * (m_StrafeSpeed - curr_strafe_speed), -1.0F, 1.0F);
		lift_accel		= WWMath::Clamp (3.0f * (m_LiftSpeed - curr_lift_speed), -1.0F, 1.0F);	
	}

	//
	//	Make the vehicle go forward/backward
	//
	game_obj->Set_Analog_Control (ControlClass::ANALOG_MOVE_FORWARD, forward_accel);

	//
	//	Make the vehicle strafe left/right
	//
	game_obj->Set_Analog_Control (ControlClass::ANALOG_MOVE_LEFT, strafe_accel);

	//
	//	Make the vehicle go up/down
	//
	game_obj->Set_Analog_Control (ControlClass::ANALOG_MOVE_UP, lift_accel);

	//
	//	Make the vehicle turn
	//
	game_obj->Set_Analog_Control (ControlClass::ANALOG_TURN_LEFT, m_TurnSharpness);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Mode
//
////////////////////////////////////////////////////////////////
void
PilotClass::Set_Mode (PilotClass::MODE mode)
{
	if (m_Mode != mode) {

		switch (mode)
		{
			case MODE_HOVER:
				break;

			case MODE_FLY_TO_POINT:
				break;

			case MODE_CIRCLE_POINT:
			{
				m_FaceTarget = false;
				
				//
				//	Initialize the circling distance
				//
				Vector3 curr_pos;
				m_GameObj->Get_Position (&curr_pos);
				m_CircleDist = (curr_pos - m_FinalDest).Length ();
			}
			break;
		}

		// 
		// When we change modes, we reset the target in case we had
		// a fake target from hover mode
		//
		Set_Target(NULL);
		m_Mode = mode;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Circle_Pos
//
////////////////////////////////////////////////////////////////
void
PilotClass::Set_Circle_Pos (const Vector3 &pos)
{
	Vector3 delta = pos - m_FinalDest;
	m_CircleAngle = WWMath::Atan2 (delta.Y, delta.X);
	m_CircleAngle = WWMath::Wrap (m_CircleAngle, 0, DEG_TO_RADF (360));
	m_CircleAngle = ::Clamp_Angle (m_CircleAngle, m_MinCircleAngle, m_MaxCircleAngle);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Circle_Angle
//
////////////////////////////////////////////////////////////////
void
PilotClass::Set_Circle_Angle (float angle)
{
	m_CircleAngle = WWMath::Wrap (angle, 0, DEG_TO_RADF (360));
	m_CircleAngle = ::Clamp_Angle (m_CircleAngle, m_MinCircleAngle, m_MaxCircleAngle);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Circle_Bounds
//
////////////////////////////////////////////////////////////////
void
PilotClass::Set_Circle_Bounds (float min_angle, float max_angle)
{
	m_MinCircleAngle	= min_angle;
	m_MaxCircleAngle	= max_angle;
	m_CircleAngle		= ::Clamp_Angle (m_CircleAngle, m_MinCircleAngle, m_MaxCircleAngle);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Has_Arrived
//
////////////////////////////////////////////////////////////////
bool
PilotClass::Has_Arrived (void) const
{
	bool has_arrived = false;

	if (m_Mode == MODE_HOVER) {

		/*bool 
		if (m_CurrentPath != NULL) {
			m_ObjSpaceDest.Length ()
		}*/
		
		//
		//	In hovering mode, we have 'arrived' if we have mostly finished moving
		//
		Vector3 vel_vector;
		Get_Object_Space_Velocity (vel_vector);
		has_arrived = (WWMath::Fabs (vel_vector.X) < 0.5F &&
							WWMath::Fabs (vel_vector.Y) < 0.5F &&
							WWMath::Fabs (vel_vector.Z) < 0.5F);
	}

	return has_arrived;
}


////////////////////////////////////////////////////////////////
//
//	Get_Current_Circle_Angle
//
////////////////////////////////////////////////////////////////
float
PilotClass::Get_Current_Circle_Angle (void) const
{
	Vector3 curr_pos;
	m_GameObj->Get_Position (&curr_pos);
	Vector3 delta = curr_pos - m_FinalDest;

	return WWMath::Atan2 (delta.Y, delta.X);
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////////////////////////////////
void
PilotClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_VARIABLES);

		//
		//	Save each variable to its own microchunk
		//		
		
		WRITE_MICRO_CHUNK (csave, VARID_FINAL_DEST,			m_FinalDest);
		WRITE_MICRO_CHUNK (csave, VARID_NEXT_POINT,			m_NextPoint);
		WRITE_MICRO_CHUNK (csave, VARID_CURRENT_TM,			m_CurrentTM);
		WRITE_MICRO_CHUNK (csave, VARID_OBJ_SPACE_DEST,		m_ObjSpaceDest);
		WRITE_MICRO_CHUNK (csave, VARID_OBJ_SPACE_WPOINT,	m_ObjSpaceWaypoint);		
		WRITE_MICRO_CHUNK (csave, VARID_PATH_PTR,				m_CurrentPath);	
		WRITE_MICRO_CHUNK (csave, VARID_GAMEOBJ_PTR,			m_GameObj);	
		WRITE_MICRO_CHUNK (csave, VARID_MAX_SPEED,			m_MaxSpeed);
		WRITE_MICRO_CHUNK (csave, VARID_SPEED_FACTOR,		m_SpeedFactor);
		WRITE_MICRO_CHUNK (csave, VARID_AGGRESSIVENESS,		m_Aggressiveness);
		WRITE_MICRO_CHUNK (csave, VARID_ARRIVED_DIST,		m_ArrivedDist);
		WRITE_MICRO_CHUNK (csave, VARID_HOVERDIST,			m_HoverDist);
		WRITE_MICRO_CHUNK (csave, VARID_ISEXACT_Z_IMPORT,	m_IsExactZImportant);
		WRITE_MICRO_CHUNK (csave, VARID_FACE_TARGET,			m_FaceTarget);
		WRITE_MICRO_CHUNK (csave, VARID_TARGET_LOCATION,	m_TargetLocation);
		WRITE_MICRO_CHUNK (csave, VARID_MODE,					m_Mode);
		WRITE_MICRO_CHUNK (csave, VARID_FORWARD_SPEED,		m_ForwardSpeed);
		WRITE_MICRO_CHUNK (csave, VARID_STRAFE_SPEED,		m_StrafeSpeed);
		WRITE_MICRO_CHUNK (csave, VARID_LIFT_SPEED,			m_LiftSpeed);
		WRITE_MICRO_CHUNK (csave, VARID_TURN_SHARPNESS,		m_TurnSharpness);
		WRITE_MICRO_CHUNK (csave, VARID_CIRCLE_ANGLE,		m_CircleAngle);
		WRITE_MICRO_CHUNK (csave, VARID_CIRCLE_DIST,			m_CircleDist);
		WRITE_MICRO_CHUNK (csave, VARID_MIN_CIRCLE_ANGLE,	m_MinCircleAngle);
		WRITE_MICRO_CHUNK (csave, VARID_MAX_CIRCLE_ANGLE,	m_MaxCircleAngle); 

	csave.End_Chunk ();
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////////////////////////////////
void
PilotClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

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
PilotClass::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK (cload, VARID_FINAL_DEST,			m_FinalDest);
			READ_MICRO_CHUNK (cload, VARID_NEXT_POINT,			m_NextPoint);
			READ_MICRO_CHUNK (cload, VARID_CURRENT_TM,			m_CurrentTM);
			READ_MICRO_CHUNK (cload, VARID_OBJ_SPACE_DEST,		m_ObjSpaceDest);
			READ_MICRO_CHUNK (cload, VARID_OBJ_SPACE_WPOINT,	m_ObjSpaceWaypoint);
			READ_MICRO_CHUNK (cload, VARID_PATH_PTR,				m_CurrentPath);	
			READ_MICRO_CHUNK (cload, VARID_GAMEOBJ_PTR,			m_GameObj);	
			READ_MICRO_CHUNK (cload, VARID_MAX_SPEED,				m_MaxSpeed);
			READ_MICRO_CHUNK (cload, VARID_SPEED_FACTOR,			m_SpeedFactor);
			READ_MICRO_CHUNK (cload, VARID_AGGRESSIVENESS,		m_Aggressiveness);
			READ_MICRO_CHUNK (cload, VARID_ARRIVED_DIST,			m_ArrivedDist);
			READ_MICRO_CHUNK (cload, VARID_HOVERDIST,				m_HoverDist);
			READ_MICRO_CHUNK (cload, VARID_ISEXACT_Z_IMPORT,	m_IsExactZImportant);
			READ_MICRO_CHUNK (cload, VARID_FACE_TARGET,			m_FaceTarget);
			READ_MICRO_CHUNK (cload, VARID_TARGET_LOCATION,		m_TargetLocation);
			READ_MICRO_CHUNK (cload, VARID_MODE,					m_Mode);
			READ_MICRO_CHUNK (cload, VARID_FORWARD_SPEED,		m_ForwardSpeed);
			READ_MICRO_CHUNK (cload, VARID_STRAFE_SPEED,			m_StrafeSpeed);
			READ_MICRO_CHUNK (cload, VARID_LIFT_SPEED,			m_LiftSpeed);
			READ_MICRO_CHUNK (cload, VARID_TURN_SHARPNESS,		m_TurnSharpness);
			READ_MICRO_CHUNK (cload, VARID_CIRCLE_ANGLE,			m_CircleAngle);
			READ_MICRO_CHUNK (cload, VARID_CIRCLE_DIST,			m_CircleDist);
			READ_MICRO_CHUNK (cload, VARID_MIN_CIRCLE_ANGLE,	m_MinCircleAngle);
			READ_MICRO_CHUNK (cload, VARID_MAX_CIRCLE_ANGLE,	m_MaxCircleAngle); 
		}

		cload.Close_Micro_Chunk ();
	}

	//
	//	Request that the game object ptr gets remapped
	//
	if (m_GameObj != NULL) {
		REQUEST_POINTER_REMAP ((void **)&m_GameObj);
	}

	//
	//	Request that the path object ptr gets remapped
	//
	if (m_CurrentPath != NULL) {
		REQUEST_POINTER_REMAP ((void **)&m_CurrentPath);
	}	

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Reset
//
///////////////////////////////////////////////////////////////////////
void
PilotClass::Reset (void)
{
	m_CurrentPath						= NULL;
	m_GameObj							= NULL;
	m_Mode								= MODE_HOVER,
	m_IsExactZImportant				= false;
	return ;
}
