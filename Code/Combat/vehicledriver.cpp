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
 *                     $Archive:: /Commando/Code/Combat/vehicledriver.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/22/02 7:13p                                               $*
 *                                                                                             *
 *                    $Revision:: 43                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "vehicledriver.h"
#include "smartgameobj.h"
#include "matrix3d.h"
#include "movephys.h"
#include "vehiclephys.h"
#include "Path.h"
#include "pscene.h"
#include "vehicle.h"
#include "soldier.h"
#include "combat.h"
#include "chunkio.h"
#include <windows.h>

#include "vehiclecurve.h"

////////////////////////////////////////////////////////////////
//	Save/Load constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES				= 0x11040454,
};

enum
{
	VARID_CURRENT_DEST			= 1,
	VARID_FINAL_DEST,
	VARID_MAX_SPEED,
	VARID_SPEED_FACTOR,
	VARID_LAST_POS,
	VARID_BAD_PROGRESS_COUNT,
	VARID_IS_BACKING_UP,
	VARID_IS_BACKUP_LOCKED,
	VARID_TURN_OFF_ENGINE,
	VARID_PATH_PTR,
	VARID_GAME_OBJ_PTR,
	VARID_ARRIVED_DIST
};


////////////////////////////////////////////////////////////////
//
//	VehicleDriverClass
//
////////////////////////////////////////////////////////////////
VehicleDriverClass::VehicleDriverClass (void)
	:	m_CurrentDest (0, 0, 0),
		m_FinalDest (0, 0, 0),
		m_CurrentPath (NULL),
		m_GameObj (NULL),
		m_IsBackingUp (false),
		m_MaxSpeed (20.0F),
		m_SpeedFactor (1.0F),
		m_LastPos (0, 0, 0),
		m_BadProgressCount (0),
		m_IsBackupLocked (false),
		m_TurnOffEngineWhenDone (false),
		m_ArrivedDist (1.0F),
		m_BrakingDist (0),
		m_LastFrameExpectedVelocity (0)
{
	return ;
}

////////////////////////////////////////////////////////////////
//
//	~VehicleDriverClass
//
////////////////////////////////////////////////////////////////
VehicleDriverClass::~VehicleDriverClass (void)
{
	Free();
}

////////////////////////////////////////////////////////////////
//
//	Get_Velocity
//
//	Returns the object-space velocity vector
//
////////////////////////////////////////////////////////////////
void
VehicleDriverClass::Get_Velocity (const Matrix3D &tm, Vector3 &vel_vector)
{
	//
	//	Get a pointer to the physics object for this vehicle
	//
	MoveablePhysClass *phys_obj = m_GameObj->Peek_Physical_Object()->As_MoveablePhysClass ();
	if (phys_obj != NULL) {

		//
		//	Get the world-space velocity vector for this vehicle and transform
		// it into object space.
		//
		Vector3 vel_vector_world;
		phys_obj->Get_Velocity (&vel_vector_world);
		Matrix3D::Inverse_Rotate_Vector (tm, vel_vector_world, &vel_vector);
	} else {
		vel_vector.Set (0, 0, 0);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////
void
VehicleDriverClass::Initialize (SmartGameObj *game_obj, PathClass *path)
{
	m_GameObj = game_obj;

	//
	//	If the game object is driving a vehicle, then get the vehicle instead
	//
	SoldierGameObj	*soldier_game_obj = game_obj->As_SoldierGameObj ();
	if (soldier_game_obj != NULL) {
		VehicleGameObj	*vehicle_game_obj = soldier_game_obj->Get_Profile_Vehicle ();
		if (vehicle_game_obj != NULL) {
			m_GameObj = vehicle_game_obj;
		}
	}

	m_CurrentPath		= path;
	m_FinalDest			= path->Get_Dest_Pos ();
	m_CurrentDest		= path->Get_Dest_Pos ();
	m_BrakingDist		= 0;
	m_LastFrameExpectedVelocity = 0;

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
			m_TurnOffEngineWhenDone = true;
		} else {
			m_TurnOffEngineWhenDone = false;
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Find_Tangent_Angle
//
///////////////////////////////////////////////////////////////////////////
static float
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
	if (dist == 0) {
		dist = 0.0001F;
	}

	//
	//	Determine the offset angle (from the line between the point and center)
	// where the 2 tangent points lie.
	//
	float angle_offset	= WWMath::Fast_Acos (radius / dist);
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

	if (dist < radius) {
		angle = DEG_TO_RADF (360.0F);
	}

	return angle;
}


////////////////////////////////////////////////////////////////
//
//	Drive
//
////////////////////////////////////////////////////////////////
bool
VehicleDriverClass::Drive (void)
{
	bool retval = false;

	//
	//	Get the turn radius of the current vehicle
	//
	PathObjectClass path_obj;
	m_CurrentPath->Get_Path_Object (path_obj);
	float turn_radius = path_obj.Get_Turn_Radius ();

	//
	//	Drive either using wheeled vehicle logic or tracked vehicle logic
	//
	if (turn_radius != 0) {
		retval = Drive_Wheeled ();
	} else {
		retval = Drive_Tracked ();
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Drive_Wheeled
//
////////////////////////////////////////////////////////////////
bool
VehicleDriverClass::Drive_Wheeled (void)
{

	//
	//	Convert the destinations from world to object space
	//
	Matrix3D tm			= m_GameObj->Get_Transform ();
	Vector3 pos			= tm.Get_Translation ();
	float facing		= tm.Get_Z_Rotation ();
	tm.Make_Identity ();
	tm.Set_Translation (pos);
	tm.Rotate_Z (facing);

		/*PhysClass *phys_obj = m_GameObj->Peek_Physical_Object ();
		RenderObjClass *model = phys_obj->Peek_Model ();

		AABoxClass bounding_box;
		bounding_box = model->Get_Bounding_Box ();

	Vector3 offset;
	Matrix3D::Rotate_Vector (tm, Vector3 (bounding_box.Extent.X, 0, 0), &offset);
	tm.Set_Translation (tm.Get_Translation () - offset);

	PhysicsSceneClass::Get_Instance ()->Add_Debug_AABox (AABoxClass (tm.Get_Translation (), Vector3 (0.5F,0.5F,0.5F)), Vector3 (1, 0, 1));*/

	//
	//	Make sure we have the final destination point
	//
	if (m_CurrentPath != NULL) {
		m_FinalDest = m_CurrentPath->Get_Dest_Pos ();
	}

	float max_speed		= m_MaxSpeed * m_SpeedFactor;
	float curve_modifier	= 1.0F;

	//
	//	Modify some of our parameters if we are force moving backwards
	//
	if (m_IsBackupLocked) {
		tm.Rotate_Z (DEG_TO_RADF (180));
		//curve_modifier = 2.0F;
	}

	//
	//	Convert the current and final destination points into object space
	//
	Vector3 final_dest;
	Vector3 current_dest;
	Matrix3D::Inverse_Transform_Vector (tm, m_FinalDest, &final_dest);
	Matrix3D::Inverse_Transform_Vector (tm, m_CurrentDest, &current_dest);
	final_dest.Z	= 0;
	current_dest.Z	= 0;

	//
	//	Get the vehicle's current position
	//
	Vector3 curr_pos;
	m_GameObj->Get_Position (&curr_pos);

	//
	//	Calculate the distance to the destination
	//
	float dist_on_path = m_CurrentPath->Get_Remaining_Path_Length ();
	float dist_to_goal = (current_dest.Length () + dist_on_path);

	//
	//	Are we there yet?
	//
	bool is_complete	= (dist_to_goal <= m_ArrivedDist);
	if (is_complete) {
		dist_to_goal = 0;
	}

	if (is_complete == false) {

		//
		//	Get the sharpness of the curve from the path object
		//
		Vector3 curve_pos (0, 0, 0);
		float	curve_sharpness = m_CurrentPath->Get_Curve_Sharpness (&curve_pos);

		//
		//	Take into account the distance to the curve
		//
		float curve_dist	= (curve_pos - tm.Get_Translation ()).Length ();
		curve_sharpness	*= 1.0F - WWMath::Clamp (curve_dist / max_speed, 0, 1.0F);
		curve_sharpness	*= curve_modifier;

		//
		//	Calculate what the angle is between us and the next destination point
		//
		float turn_angle	= WWMath::Atan2 (current_dest.Y, current_dest.X);
		turn_angle			= WWMath::Wrap (turn_angle, DEG_TO_RADF(-180), DEG_TO_RADF(180));

		//
		//	Calculate how sharp we need to turn based on the sharpness of the curve
		//
		float max_angle	= DEG_TO_RADF (10.0F) + (1.0F - curve_sharpness) * DEG_TO_RADF (30.0F);
		float turn_accel	= turn_angle / max_angle;


		/*PathObjectClass path_obj;
		m_CurrentPath->Get_Path_Object (path_obj);
		float turn_radius = path_obj.Get_Turn_Radius ();*/

		/*if ((turn_radius * turn_radius) > current_dest.Length2 () && WWMath::Fabs (turn_angle) > DEG_TO_RADF (60.0F)) {
			m_IsBackingUp = true;
		}*/

		//
		//	Calculate our (current) normalized speed
		//
		Vector3 vel_vector;
		Get_Velocity (tm, vel_vector);
		float speed			= vel_vector.X;
		float norm_speed	= speed / max_speed;

		//
		//	Calculate how fast we should accelerate
		//
		float expected_velocity = 1.0F - (0.3F * curve_sharpness);

		//
		//	Take into account the braking speed as we approach the goal
		//
		float brake_velocity = Calculate_Brake (dist_to_goal, expected_velocity, speed, max_speed);
		expected_velocity		= min (brake_velocity, expected_velocity);

		//
		//	Calculate how far off our last velocity request was
		//
		float vel_factor = 1.0F;
		if (norm_speed != 0) {
			vel_factor = m_LastFrameExpectedVelocity / WWMath::Fabs (norm_speed);
		}

		//
		//	Try to account for our last velocity request error
		//
		m_LastFrameExpectedVelocity = WWMath::Fabs (expected_velocity);
		float forward_accel = (expected_velocity - norm_speed) * vel_factor;

		/*StringClass message;
		message.Format ("forward accel = %f\n", forward_accel);
		WWDEBUG_SAY (((const char *)message));*/

		//
		//	Invert everything if we are locked in driving backwards mode
		//
		if (m_IsBackupLocked) {
			forward_accel	= -forward_accel;
			turn_accel		= -turn_accel;
		}

		//
		//	Handle the case where we are backing up
		//
		/*if (m_IsBackingUp) {

			//
			//	Switch out of backup mode if we have a pretty good facing on the
			// destination or we got stuck while backing up.
			//
			if (WWMath::Fabs (turn_angle) < DEG_TO_RADF (25.0F) || Are_We_Stuck (vel_vector)) {
				m_IsBackingUp			= false;
				m_BadProgressCount	= 0;
			} else {

				//
				//	Backup and turn in the opposite direction
				//
				forward_accel	= -forward_accel;
				turn_accel		= -turn_accel;
			}

		} else if (Are_We_Stuck (vel_vector)) {

			m_IsBackingUp = true;
			m_BadProgressCount = 0;
		}*/

		//
		//	Clamp the analog controls to -1 and 1
		//
		turn_accel		= WWMath::Clamp (turn_accel, -1.0F, 1.0F);
		forward_accel	= WWMath::Clamp (forward_accel, -1.0F, 1.0F);

		//
		//	Pass the controls onto the vehicle
		//
		Apply_Controls (forward_accel, turn_accel);

	} else {

		//
		//	Calculate our (current) normalized speed
		//
		Vector3 vel_vector;
		Get_Velocity (tm, vel_vector);
		float speed				= vel_vector.X;
		float norm_speed		= speed / max_speed;

		//
		//	Try to zero out our forward movement
		//
		float forward_accel	= WWMath::Clamp (-norm_speed * 5.0F, -1.0F, 1.0F);

		if (m_GameObj->Peek_Physical_Object ()->As_VTOLVehicleClass ()) {
			forward_accel	= WWMath::Clamp (-norm_speed * 5.0F, -1.0F, 1.0F);
		}

		//
		//	Invert everything if we are locked in driving backwards mode
		//
		if (m_IsBackupLocked) {
			forward_accel	= -forward_accel;
		}

		//
		//	Try to hover about the point
		//
		Apply_Controls (forward_accel, 0);

		//
		//	We aren't really done unless we've stopped most motion
		//
		if (m_GameObj->Peek_Physical_Object ()->As_VTOLVehicleClass ()) {
			is_complete = (WWMath::Fabs (norm_speed) < 0.0001F);
		}

		//
		//	Do we need to turn off the engine?
		//
		if (is_complete && m_TurnOffEngineWhenDone) {

			//
			//	Turn off the engine if we are finished moving
			//
			VehicleGameObj *vehicle = m_GameObj->As_VehicleGameObj ();
			if (vehicle != NULL) {
				Apply_Controls (0, 0);
				vehicle->Enable_Engine (false);
			}
		}
	}

	return is_complete;
}


////////////////////////////////////////////////////////////////
//
//	Calculate_Brake
//
////////////////////////////////////////////////////////////////
float
VehicleDriverClass::Calculate_Brake
(
	float dist_to_goal,
	float expected_velocity,
	float curr_speed,
	float max_speed
)
{
	float brake_velocity = expected_velocity;

	//
	//	Handle VTOL's differently
	//
	if (m_GameObj->Peek_Physical_Object ()->As_VTOLVehicleClass () == NULL) {

		//
		//	Start braking when the vehicle is 1 second away from its destination
		//
		if (m_BrakingDist == 0 && dist_to_goal < curr_speed) {
			m_BrakingDist = curr_speed;
		}

		//
		//	Calculate what speed we should be given the distance to the destination
		//
		if (m_BrakingDist > 0) {
			float end_velocity	= (0.5F / max_speed);
			float percent			= WWMath::Clamp (dist_to_goal / (m_BrakingDist - m_ArrivedDist), -1.0F, 1.0F);
			brake_velocity			= ((1.0F - percent) * end_velocity) + (percent * expected_velocity);
		}

	} else {


		//
		//	Start braking when the vehicle is 3 seconds away from its destination
		//
		if (m_BrakingDist == 0 && dist_to_goal < max_speed * 3) {
			m_BrakingDist = max_speed * 3;
		}

		//
		//	Calculate what speed we should be given the distance to the destination
		//
		if (m_BrakingDist > 0) {
			float percent			= WWMath::Clamp (dist_to_goal / (m_BrakingDist - m_ArrivedDist), -1.0F, 1.0F);
			float end_velocity	= WWMath::Clamp ((-(curr_speed / max_speed) * 5.0F) + (0.75F / max_speed), -1.0F, 1.0F);
			brake_velocity			= ((1.0F - percent) * end_velocity) + (percent * expected_velocity);
		}
	}

	return brake_velocity;
}


////////////////////////////////////////////////////////////////
//
//	Drive_Tracked
//
////////////////////////////////////////////////////////////////
bool
VehicleDriverClass::Drive_Tracked (void)
{
	//
	//	Convert the destinations from world to object space
	//
	Matrix3D tm			= m_GameObj->Get_Transform ();
	Vector3 pos			= tm.Get_Translation ();
	float facing		= tm.Get_Z_Rotation ();
	tm.Make_Identity ();
	tm.Set_Translation (pos);
	tm.Rotate_Z (facing);
	float max_speed	= m_MaxSpeed * m_SpeedFactor;

	//
	//	Let the path know how fast we are moving
	//
	Vector3 vel_vector;
	Get_Velocity (tm, vel_vector);
	m_CurrentPath->Set_Velocity (WWMath::Fabs (vel_vector.X));

	//
	//	Make sure we have the final destination point
	//
	if (m_CurrentPath != NULL) {
		m_FinalDest		= m_CurrentPath->Get_Dest_Pos ();
		m_CurrentDest	= m_CurrentPath->Get_Next_Pos ();
	}

	//
	//	Modify some of our parameters if we are force moving backwards
	//
	if (m_IsBackupLocked) {
		tm.Rotate_Z (DEG_TO_RADF (180));
	}

	//
	//	Convert the current and final destination points into object space
	//
	Vector3 final_dest;
	Vector3 current_dest;
	Matrix3D::Inverse_Transform_Vector (tm, m_FinalDest, &final_dest);
	Matrix3D::Inverse_Transform_Vector (tm, m_CurrentDest, &current_dest);
	final_dest.Z	= 0;
	current_dest.Z	= 0;

	//
	//	Get the vehicle's current position
	//
	Vector3 curr_pos;
	m_GameObj->Get_Position (&curr_pos);

	//
	//	Calculate the distance to the destination
	//
	float dist_on_path = m_CurrentPath->Get_Remaining_Path_Length ();
	float dist_to_goal = (current_dest.Length () + dist_on_path);

	//
	//	Are we there yet?
	//
	bool is_complete	= (dist_to_goal <= m_ArrivedDist);
	if (is_complete) {
		dist_to_goal = 0;
	}

	if (is_complete == false) {

		//
		//	Calculate what the angle is between us and the next destination point
		//
		float turn_angle	= WWMath::Atan2 (current_dest.Y, current_dest.X);
		turn_angle			= WWMath::Wrap (turn_angle, DEG_TO_RADF(-180), DEG_TO_RADF(180));

		//StringClass message;
//		message.Format ("turn_angle = %f\n", turn_angle * 180 / 3.1415926F);
		//WWDEBUG_SAY (((const char *)message));

		//
		//	Calculate how sharp we need to turn based on the sharpness of the curve
		//
		float turn_accel	= turn_angle / DEG_TO_RADF (45.0F);

		//
		//	Calculate our (current) normalized speed
		//
		float speed			= vel_vector.X;
		float norm_speed	= speed / max_speed;

		//
		//	Calculate how fast we should accelerate
		//
		float expected_velocity = 1.0F;

		//
		//	Start braking when the vehicle is 1 second away from its destination
		//
		if (m_BrakingDist == 0 && dist_to_goal < speed) {
			m_BrakingDist = speed;
		}

		if (m_BrakingDist > 0) {
			float end_velocity		= (0.5F / max_speed);
			float percent				= WWMath::Clamp (dist_to_goal / (m_BrakingDist - m_ArrivedDist), -1.0F, 1.0F);
			float braking_vel			= ((1.0F - percent) * end_velocity) + (percent * expected_velocity);

			expected_velocity			= min (expected_velocity, braking_vel);
		}

		float forward_accel = expected_velocity - norm_speed;

		//
		//	Make tracked vehicles turn in place
		//
		if (WWMath::Fabs (turn_angle) > DEG_TO_RADF (10.0F)) {
			forward_accel /= 4.0F;
		}

		//
		//	Invert everything if we are locked in driving backwards mode
		//
		if (m_IsBackupLocked) {
			forward_accel	= -forward_accel;
		}

		//
		//	Clamp the analog controls to -1 and 1
		//
		turn_accel		= WWMath::Clamp (turn_accel, -1.0F, 1.0F);
		forward_accel	= WWMath::Clamp (forward_accel, -1.0F, 1.0F);

		//
		//	Pass the controls onto the vehicle
		//
		Apply_Controls (forward_accel, turn_accel);

	} else {

		//
		//	Calculate our (current) normalized speed
		//
		Vector3 vel_vector;
		Get_Velocity (tm, vel_vector);
		float speed				= vel_vector.X;
		float norm_speed		= speed / max_speed;

		//
		//	Try to zero out our forward movement
		//
		float forward_accel	= WWMath::Clamp (-norm_speed * 2, -1.0F, 1.0F);

		//
		//	Invert everything if we are locked in driving backwards mode
		//
		if (m_IsBackupLocked) {
			forward_accel	= -forward_accel;
		}

		//
		//	Try to hover about the point
		//
		Apply_Controls (forward_accel, 0);

		//
		//	Do we need to turn off the engine?
		//
		if (m_TurnOffEngineWhenDone) {

			//
			//	Turn off the engine if we are finished moving
			//
			VehicleGameObj *vehicle = m_GameObj->As_VehicleGameObj ();
			if (vehicle != NULL) {
				Apply_Controls (0, 0);
				vehicle->Enable_Engine (false);
			}
		}
	}

	return is_complete;
}




////////////////////////////////////////////////////////////////
//
//	Apply_Controls
//
////////////////////////////////////////////////////////////////
void
VehicleDriverClass::Apply_Controls
(
	float forward_accel,
	float left_accel
)
{
	if (!WWMath::Is_Valid_Float(forward_accel)) {
#ifdef WWDEBUG
		const char* name = m_GameObj->Peek_Physical_Object()->Peek_Model()->Get_Name();
		WWDEBUG_SAY(("VehicleDriverClass::Apply_Controls - BAD FLOAT forward_access = %f - Model name = %s\n", forward_accel, name));
#endif //WWDEBUG
		forward_accel = 0.0f;
	}
	if (!WWMath::Is_Valid_Float(left_accel)) {
#ifdef WWDEBUG
		const char* name = m_GameObj->Peek_Physical_Object()->Peek_Model()->Get_Name();
		WWDEBUG_SAY(("VehicleDriverClass::Apply_Controls - BAD FLOAT left_access = %f - Model name = %s\n", left_accel, name));
#endif //WWDEBUG
		left_accel = 0.0f;
	}

	//
	//	Now pass the controls onto the game object
	//
	m_GameObj->Set_Analog_Control (ControlClass::ANALOG_MOVE_FORWARD,	forward_accel);
	m_GameObj->Set_Analog_Control (ControlClass::ANALOG_TURN_LEFT,		left_accel);
	m_GameObj->Set_Analog_Control (ControlClass::ANALOG_MOVE_LEFT,		0);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Force_Backup
//
////////////////////////////////////////////////////////////////
void
VehicleDriverClass::Force_Backup (bool onoff)
{
	//
	//	Simply set these flags, the Drive method will
	// make use of them.
	//
	m_IsBackingUp		= onoff;
	m_IsBackupLocked	= onoff;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Are_We_Stuck
//
////////////////////////////////////////////////////////////////
bool
VehicleDriverClass::Are_We_Stuck (const Vector3 &vel_vector)
{
	//
	//	Determine if we are not making significant progress
	//
	if (WWMath::Fabs (vel_vector.X) < (m_MaxSpeed * m_SpeedFactor) / 10.0F) {
		m_BadProgressCount ++;
	} else {
		m_BadProgressCount = 0;
	}

	//
	//	If we aren't making progress for 30 frames in a row,
	// then assume we are stuck
	//
	return bool(m_BadProgressCount > 30);
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////////////////////////////////
void
VehicleDriverClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_VARIABLES);

		//
		//	Save each variable to its own microchunk
		//
		WRITE_MICRO_CHUNK (csave, VARID_CURRENT_DEST,			m_CurrentDest);
		WRITE_MICRO_CHUNK (csave, VARID_FINAL_DEST,				m_FinalDest);
		WRITE_MICRO_CHUNK (csave, VARID_MAX_SPEED,				m_MaxSpeed);
		WRITE_MICRO_CHUNK (csave, VARID_SPEED_FACTOR,			m_SpeedFactor);
		WRITE_MICRO_CHUNK (csave, VARID_LAST_POS,					m_LastPos);
		WRITE_MICRO_CHUNK (csave, VARID_BAD_PROGRESS_COUNT,	m_BadProgressCount);
		WRITE_MICRO_CHUNK (csave, VARID_IS_BACKING_UP,			m_IsBackingUp);
		WRITE_MICRO_CHUNK (csave, VARID_IS_BACKUP_LOCKED,		m_IsBackupLocked);
		WRITE_MICRO_CHUNK (csave, VARID_TURN_OFF_ENGINE,		m_TurnOffEngineWhenDone);
		WRITE_MICRO_CHUNK (csave, VARID_PATH_PTR,					m_CurrentPath);
		WRITE_MICRO_CHUNK (csave, VARID_GAME_OBJ_PTR,			m_GameObj);
		WRITE_MICRO_CHUNK (csave, VARID_ARRIVED_DIST,			m_ArrivedDist);

	csave.End_Chunk ();
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////////////////////////////////
void
VehicleDriverClass::Load (ChunkLoadClass &cload)
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
VehicleDriverClass::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK (cload, VARID_CURRENT_DEST,			m_CurrentDest);
			READ_MICRO_CHUNK (cload, VARID_FINAL_DEST,			m_FinalDest);
			READ_MICRO_CHUNK (cload, VARID_MAX_SPEED,				m_MaxSpeed);
			READ_MICRO_CHUNK (cload, VARID_SPEED_FACTOR,			m_SpeedFactor);
			READ_MICRO_CHUNK (cload, VARID_LAST_POS,				m_LastPos);
			READ_MICRO_CHUNK (cload, VARID_BAD_PROGRESS_COUNT,	m_BadProgressCount);
			READ_MICRO_CHUNK (cload, VARID_IS_BACKING_UP,		m_IsBackingUp);
			READ_MICRO_CHUNK (cload, VARID_IS_BACKUP_LOCKED,	m_IsBackupLocked);
			READ_MICRO_CHUNK (cload, VARID_TURN_OFF_ENGINE,		m_TurnOffEngineWhenDone);
			READ_MICRO_CHUNK (cload, VARID_PATH_PTR,				m_CurrentPath);
			READ_MICRO_CHUNK (cload, VARID_GAME_OBJ_PTR,			m_GameObj);
			READ_MICRO_CHUNK (cload, VARID_ARRIVED_DIST,			m_ArrivedDist);
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
	//	Request that the path ptr gets remapped
	//
	if (m_CurrentPath != NULL) {
		REQUEST_REF_COUNTED_POINTER_REMAP ((RefCountClass **)&m_CurrentPath);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Reset
//
///////////////////////////////////////////////////////////////////////
void
VehicleDriverClass::Reset (void)
{
	m_CurrentPath						= NULL;
	m_GameObj							= NULL;
	m_BrakingDist						= 0;
	m_LastFrameExpectedVelocity	= 0;
	m_TurnOffEngineWhenDone			= false;
	m_BadProgressCount				= 0;
	return ;
}