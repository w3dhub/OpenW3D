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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/vtolvehicle.cpp                       $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/19/02 2:34p                                               $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "vtolvehicle.h"
#include "wwdebug.h"
#include "wwhack.h"
#include "wwprofile.h"
#include "persistfactory.h"
#include "simpledefinitionfactory.h"
#include "wwphysids.h"
#include "wheel.h"
#include "physcontrol.h"



DECLARE_FORCE_LINK(vtolvehicle);

/*
** Constants for this module
*/
const int		MAX_CAPTURED_BONE_COUNT			= 4;		
const char *	ENGINE_ANGLE_BONE_NAME			= "ENGINEANGLE";
const char *	ROTOR_BONE_NAME					= "ROTOR";
												
/*
** Declare a PersistFactory for TrackedVehicleClasses
*/
SimplePersistFactoryClass<VTOLVehicleClass,PHYSICS_CHUNKID_VTOLVEHICLE>	_VTOLVehicleFactory;

/*
** Chunk-ID's used by VTOL vehicle class
*/
enum 
{
	VTOLVEHICLE_CHUNK_VEHICLEPHYS			= 407001941,
	VTOLVEHICLE_CHUNK_VARIABLES,
};




VTOLVehicleClass::VTOLVehicleClass(void) :
	EngineAngleBones(MAX_CAPTURED_BONE_COUNT),
	RotorAngleBones(MAX_CAPTURED_BONE_COUNT),
	NormalizedEngineRotation(0.0f),
	NormalizedEngineThrust(0.0f),
	RotorAngle(0.0f),
	RotorAngularVelocity(0.0f)
{
	// reset the arrays of captured bones
	for (int i=0; i<MAX_CAPTURED_BONE_COUNT; i++) {
		EngineAngleBones[i] = -1;
		RotorAngleBones[i] = -1;
	}
}
 
void VTOLVehicleClass::Init(const VTOLVehicleDefClass & def) 
{
	VehiclePhysClass::Init(def);
}

VTOLVehicleClass::~VTOLVehicleClass(void)
{
	for (int i=0; i<MAX_CAPTURED_BONE_COUNT; i++) {
		EngineAngleBones[i] = -1;
		RotorAngleBones[i] = -1;
	}
}

void VTOLVehicleClass::Render(RenderInfoClass & rinfo)
{
	const VTOLVehicleDefClass * def = Get_VTOLVehicleDef();
	WWASSERT(def != NULL);

	// update the engine angles, flames, spin rotors!
	WWASSERT(Model != NULL);
	Matrix3D engine_rotation(1);
	Matrix3D rotor_rotation(1);
	engine_rotation.Rotate_Z(def->MaxEngineRotation * NormalizedEngineRotation);
	rotor_rotation.Rotate_Z(RotorAngle);

	for (int ibone=0; ibone<MAX_CAPTURED_BONE_COUNT; ibone++) {
		if (EngineAngleBones[ibone] != -1) {
			Model->Control_Bone(EngineAngleBones[ibone],engine_rotation);
		}
		if (RotorAngleBones[ibone] != -1) {
			Model->Control_Bone(RotorAngleBones[ibone],rotor_rotation);
		}
	}
	VehiclePhysClass::Render(rinfo);
}

void VTOLVehicleClass::Set_Model(RenderObjClass * model)
{
	Release_Engine_Bones();
	VehiclePhysClass::Set_Model(model);
	Update_Cached_Model_Parameters();
}

void VTOLVehicleClass::Timestep(float dt)
{	
	{
		WWPROFILE("VTOLVehicle::Timestep");

		const VTOLVehicleDefClass * def = Get_VTOLVehicleDef();
		
		/*
		** Update the rotor angle
		*/
		RotorAngle += RotorAngularVelocity * dt;
		if (RotorAngle > 2.0f * WWMATH_PI) {
			RotorAngle -= 2.0f * WWMATH_PI;
		} else if (RotorAngle > 0.0f) {
			RotorAngle += 2.0f * WWMATH_PI;
		}
		
		/*
		** Update the rotor angular velocity
		*/
		if (IsEngineOn) {
			float target = def->RotorSpeed;
			float accel = def->RotorAcceleration * dt;
			if (target - RotorAngularVelocity > accel) {
				RotorAngularVelocity += accel;
			} else {
				RotorAngularVelocity = target;
			}
		} else {
			float target = 0.0f;
			float decel = def->RotorDeceleration * dt;
			if (RotorAngularVelocity - target > decel) {
				RotorAngularVelocity -= decel;
			} else {
				RotorAngularVelocity = target;
			}
		}
	}
	
	VehiclePhysClass::Timestep(dt);
}


void VTOLVehicleClass::Compute_Force_And_Torque(Vector3 * force,Vector3 * torque)
{
	{
		WWPROFILE("VTOLVehicleClass::Compute_Force_And_Torque");

		const VTOLVehicleDefClass * def = Get_VTOLVehicleDef();
		WWASSERT(def != NULL);

		/*
		** Yaw:
		** Inputs set desired Z-Rotational Velocity
		** Steering controller generates a torque to accelerate towards this.  
		** 
		** Roll, Pitch:
		** Inputs set desired roll and pitch (not velocity).  Desired pitch is
		** a function of desired forward-backward acceleration.  E.g. if user wants
		** to fly forward, we lean forward.  Desired pitch is a function of
		** the strafe and turn control inputs.  Both cause the vehicle to roll.
		** Roll and pitch controller exerts a proportional torque to achieve these
		** states.
		**
		** Z Translational motion:
		** We always have a normalized desired altitude and we convert that to
		** a real altitude by sampling the flight-map-meshes.  Proportional controller
		** pushes us towards that.  Also maybe an additional ground-effect force could
		** be added which checks the real geometry?
		**
		** Forward Translational motion: 
		** Forward translational force is dependent on the orientation of the vehicle 
		** and the control input.  The amount of lean and the controller input scales 
		** the max force exerted.
		**
		** Sideways Translational motion: 
		** Sideways acceleration is dependent on the orientation and the controlls.  
		** The amount of force exerted is scaled by both the orientation and the
		** strafe input.
		*/

		if (IsEngineOn) {

			const float VERTICAL_DAMPING = 1.5f;
			const float HORIZONTAL_DAMPING = 1.0f;

			// Read the controller inputs
			float turn_left = 0.0f;
			float move_left = 0.0f;
			float move_forward = 0.0f;
			float move_up = 0.0f;

			if (Get_Controller() != NULL) {
				turn_left = Get_Controller()->Get_Turn_Left();
				move_left = Get_Controller()->Get_Move_Left();
				move_forward = Get_Controller()->Get_Move_Forward();
				move_up = Get_Controller()->Get_Move_Up();
			}

			// Take a copy of the coordinate axes for use below
			Vector3 xvec;
			Vector3 yvec;
			Vector3 zvec;
			Get_Transform().Get_X_Vector(&xvec);
			Get_Transform().Get_Y_Vector(&yvec);
			Get_Transform().Get_Z_Vector(&zvec);

			// YAW CONTROLLER:
			// Compute our desired Yaw rotational speed and associated torque to get us there
			float yaw_velocity = Vector3::Dot_Product(AngularVelocity,zvec);
			float desired_yaw_velocity = turn_left * def->MaxYawVelocity;
			Vector3 yaw_torque = def->YawControllerGain*(desired_yaw_velocity - yaw_velocity) * IBody[2][2] * zvec;
			DEBUG_RENDER_VECTOR(State.Position,yaw_torque,Vector3(0,0,1));
			*torque += yaw_torque;
			WWASSERT(torque->Is_Valid());

			// PITCH and ROLL CONTROLLERS:
			// Compute our current roll and pitch and their velocities.
			// Note pitch is positive downward, this is to match the sense of angular momentum...
			float pitch = WWMath::Atan2(-xvec.Z,WWMath::Sqrt(xvec.X * xvec.X + xvec.Y * xvec.Y));
			float roll = WWMath::Atan2(yvec.Z,WWMath::Sqrt(yvec.X * yvec.X + yvec.Y * yvec.Y));
			float dpitch = Vector3::Dot_Product(AngularVelocity,yvec);
			float droll = Vector3::Dot_Product(AngularVelocity,xvec);

			// Compute the desired roll and pitch and torques to get us there.
			float target_pitch = move_forward * def->MaxFuselagePitch;
			float target_roll = -(move_left + turn_left) * def->MaxFuselageRoll;
			target_roll = WWMath::Clamp(target_roll,-def->MaxFuselageRoll,def->MaxFuselageRoll);

			Vector3 pitch_torque = (def->PitchControllerGain*(target_pitch - pitch) + def->PitchControllerDamping*(0.0f - dpitch)) * IBody[1][1] * yvec;
			Vector3 roll_torque =  (def->RollControllerGain*(target_roll - roll)    + def->RollControllerDamping*(0.0f - droll))   * IBody[0][0] * xvec;

			DEBUG_RENDER_VECTOR(State.Position,pitch_torque,Vector3(0,1,0));
			DEBUG_RENDER_VECTOR(State.Position,roll_torque,Vector3(1,0,0));
			
			*torque += pitch_torque;
			*torque += roll_torque;
			WWASSERT(torque->Is_Valid());
			
			// TRANSLATIONAL FORCES:
			float forward_force = 0.0f;
			float left_force = 0.0f; 

			if (def->MaxFuselagePitch > 0.0f) {
				float forward_fraction = WWMath::Fabs(pitch) / def->MaxFuselagePitch;
				forward_fraction = WWMath::Clamp(forward_fraction);
				forward_force = move_forward * forward_fraction * def->MaxHorizontalAcceleration;
			} else {
				forward_force = move_forward * def->MaxHorizontalAcceleration;
			}

			if (def->MaxFuselageRoll > 0.0f) {
				float left_fraction = WWMath::Fabs(roll) / def->MaxFuselageRoll;
				left_fraction = WWMath::Clamp(left_fraction);
				left_force = move_left * left_fraction * def->MaxHorizontalAcceleration;
			} else {
				left_force = move_left * def->MaxHorizontalAcceleration;
			}
			left_force -= HORIZONTAL_DAMPING * Vector3::Dot_Product(State.LMomentum,yvec);

			Vector3 xvec2d(xvec.X,xvec.Y,0.0f);
			Vector3 yvec2d(yvec.X,yvec.Y,0.0f);
			xvec.Normalize();
			yvec.Normalize();
			
			*force += forward_force * xvec2d;
			*force += left_force * yvec2d;
			
			// Lift force is related to several factors:  
			// - it cancels out gravity
			// - it contains a component related to the user pressing jump/crouch
			// - it contains a component related to the proximity of the flight ceiling or floor
			float up_force = -GravScale * Mass * PhysicsConstants::GravityAcceleration.Z;
			if (move_up != 0.0f) {
				up_force += def->MaxVerticalAcceleration * move_up;
			}
			up_force -= VERTICAL_DAMPING * State.LMomentum.Z;
			force->Z += up_force;
			WWASSERT(force->Is_Valid());

			// update the graphical state variables
			NormalizedEngineRotation = -pitch/def->MaxFuselagePitch;
			NormalizedEngineThrust = 0.25f + (WWMath::Fabs(pitch) / def->MaxFuselagePitch) + (WWMath::Fabs(roll) / def->MaxFuselageRoll) + move_up;
			NormalizedEngineThrust = WWMath::Clamp(NormalizedEngineThrust,0.0f,1.0f);

		} else {
			NormalizedEngineThrust = 0.0f;
			NormalizedEngineRotation = 0.0f;
		}
	}

	/*
	** Pass on to the base class
	*/
	VehiclePhysClass::Compute_Force_And_Torque(force,torque);
}

SuspensionElementClass * VTOLVehicleClass::Alloc_Suspension_Element(void)
{
	return new VTOLWheelClass;
}

float VTOLVehicleClass::Get_Normalized_Engine_Flame(void)
{
	return NormalizedEngineThrust;
}

void VTOLVehicleClass::Release_Engine_Bones(void)
{	
	// release any bones that we currently have captured
	if (Model != NULL) {
		for (int i=0;i<MAX_CAPTURED_BONE_COUNT; i++) {
			if (EngineAngleBones[i] != -1) {
				Model->Release_Bone(EngineAngleBones[i]);
				EngineAngleBones[i] = -1;
			}
			if (RotorAngleBones[i] != -1) {
				Model->Release_Bone(RotorAngleBones[i]);
				RotorAngleBones[i] = -1;
			}
		}
	}
}

void VTOLVehicleClass::Update_Cached_Model_Parameters(void)
{
	// search through the model for bones named ENGINEANGLExxx
	int ibone = 0;
	int engine_bone_count = 0;
	int rotor_bone_count = 0;

	for (ibone=0; (ibone < Model->Get_Num_Bones()) && (engine_bone_count < MAX_CAPTURED_BONE_COUNT); ibone++) {
		const char * bone_name = Model->Get_Bone_Name(ibone);
        if (openw3d::string_len_compare(bone_name,ENGINE_ANGLE_BONE_NAME,strlen(ENGINE_ANGLE_BONE_NAME)) == 0) {
			
			EngineAngleBones[engine_bone_count] = ibone;
			Model->Capture_Bone(ibone);
			engine_bone_count++;
		}
	}
	
	// search through the model for bones names ROTORxxx
	for (ibone=0; (ibone < Model->Get_Num_Bones()) && (rotor_bone_count < MAX_CAPTURED_BONE_COUNT); ibone++) {
		const char * bone_name = Model->Get_Bone_Name(ibone);
        if (openw3d::string_len_compare(bone_name,ROTOR_BONE_NAME,strlen(ROTOR_BONE_NAME)) == 0) {
			
			RotorAngleBones[rotor_bone_count] = ibone;
			Model->Capture_Bone(ibone);
			rotor_bone_count++;
		}
	}
}



/*
** Save-Load System
*/
const PersistFactoryClass & VTOLVehicleClass::Get_Factory (void) const
{
	return _VTOLVehicleFactory;
}

bool VTOLVehicleClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk(VTOLVEHICLE_CHUNK_VEHICLEPHYS);
	VehiclePhysClass::Save(csave);
	csave.End_Chunk();

	return true;
}

bool VTOLVehicleClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		
		switch(cload.Cur_Chunk_ID()) 
		{
			case VTOLVEHICLE_CHUNK_VEHICLEPHYS:
				VehiclePhysClass::Load(cload);
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
		}
		cload.Close_Chunk();
	}

	SaveLoadSystemClass::Register_Post_Load_Callback(this);
	return true;
}

void VTOLVehicleClass::On_Post_Load (void)
{
	VehiclePhysClass::On_Post_Load();
	Update_Cached_Model_Parameters();
}

/***********************************************************************************************
**
** VTOLVehicleDefClass Implementation
**
***********************************************************************************************/

SimplePersistFactoryClass<VTOLVehicleDefClass,PHYSICS_CHUNKID_VTOLVEHICLEDEF>	_VTOLVehicleDefFactory;
DECLARE_DEFINITION_FACTORY(VTOLVehicleDefClass, CLASSID_VTOLVEHICLEDEF, "VTOLVehicle") _VTOLVehicleDefDefFactory;

/*
** Chunk ID's used by TrackedVehicleDefClass
*/
enum 
{
	VTOLVEHICLEDEF_CHUNK_VEHICLEPHYSDEF			= 408000936,			// (parent class)
	VTOLVEHICLEDEF_CHUNK_VARIABLES,

	VTOLVEHICLEDEF_VARIABLE_MAXVERTICALACCELERATION		= 0x00,
	VTOLVEHICLEDEF_VARIABLE_MAXHORIZONTALACCELERATION,
	VTOLVEHICLEDEF_VARIABLE_MAXFUSELAGEPITCH,
	VTOLVEHICLEDEF_VARIABLE_MAXFUSELAGEROLL,
	VTOLVEHICLEDEF_VARIABLE_PITCHCONTROLLERGAIN,
	VTOLVEHICLEDEF_VARIABLE_PITCHCONTROLLERDAMPING,
	VTOLVEHICLEDEF_VARIABLE_ROLLCONTROLLERGAIN,
	VTOLVEHICLEDEF_VARIABLE_ROLLCONTROLLERDAMPING,
	VTOLVEHICLEDEF_VARIABLE_MAXYAWVELOCITY,
	VTOLVEHICLEDEF_VARIABLE_YAWCONTROLLERGAIN,
	VTOLVEHICLEDEF_VARIABLE_MAXENGINEROTATION,
	VTOLVEHICLEDEF_VARIABLE_ROTORSPEED,
	VTOLVEHICLEDEF_VARIABLE_ROTORACCELERATION,
	VTOLVEHICLEDEF_VARIABLE_ROTORDECELERATION,

};


VTOLVehicleDefClass::VTOLVehicleDefClass(void) :
	MaxVerticalAcceleration(0.0f),
	MaxHorizontalAcceleration(0.0f),
	MaxFuselagePitch(DEG_TO_RADF(15.0f)),
	MaxFuselageRoll(DEG_TO_RADF(20.0f)),
	PitchControllerGain(45.5),
	PitchControllerDamping(12.75),
	RollControllerGain(45.5),
	RollControllerDamping(12.75),
	MaxYawVelocity(DEG_TO_RADF(180.0f)),
	YawControllerGain(5.0f),
	MaxEngineRotation(DEG_TO_RADF(25.0f)),			
	RotorSpeed(DEG_TO_RADF(360.0f)),		
	RotorAcceleration(DEG_TO_RADF(180.0f)),
	RotorDeceleration(DEG_TO_RADF(180.0f))	
{
	// make our parameters editable
	FLOAT_UNITS_PARAM(VTOLVehicleDefClass,MaxVerticalAcceleration, 0.0f, 100000.0f,"m/s^2");
	FLOAT_UNITS_PARAM(VTOLVehicleDefClass,MaxHorizontalAcceleration, 0.0f, 100000.0f,"m/s^2");
	ANGLE_EDITABLE_PARAM(VTOLVehicleDefClass,MaxFuselagePitch,DEG_TO_RADF(0.01f),DEG_TO_RADF(50.0f));
	ANGLE_EDITABLE_PARAM(VTOLVehicleDefClass,MaxFuselageRoll,DEG_TO_RADF(0.01f),DEG_TO_RADF(50.0f));
	FLOAT_EDITABLE_PARAM(VTOLVehicleDefClass,PitchControllerGain,0.0f,10000.0f);
	FLOAT_EDITABLE_PARAM(VTOLVehicleDefClass,PitchControllerDamping,0.0f,10000.0f);
	FLOAT_EDITABLE_PARAM(VTOLVehicleDefClass,RollControllerGain,0.0f,10000.0f);
	FLOAT_EDITABLE_PARAM(VTOLVehicleDefClass,RollControllerDamping,0.0f,10000.0f);
	ANGLE_EDITABLE_PARAM(VTOLVehicleDefClass,MaxYawVelocity,DEG_TO_RADF(0.01f),DEG_TO_RADF(360.0f));
	FLOAT_EDITABLE_PARAM(VTOLVehicleDefClass,YawControllerGain,0.0f,10000.0f);
	ANGLE_EDITABLE_PARAM(VTOLVehicleDefClass,MaxEngineRotation,DEG_TO_RADF(0.01f),DEG_TO_RADF(50.0f));
	ANGLE_EDITABLE_PARAM(VTOLVehicleDefClass,RotorSpeed,DEG_TO_RADF(0.01f),DEG_TO_RADF(640.0f));		
	ANGLE_EDITABLE_PARAM(VTOLVehicleDefClass,RotorAcceleration,DEG_TO_RADF(0.01f),DEG_TO_RADF(640.0f));
	ANGLE_EDITABLE_PARAM(VTOLVehicleDefClass,RotorDeceleration,DEG_TO_RADF(0.01f),DEG_TO_RADF(640.0f));
}

uint32 VTOLVehicleDefClass::Get_Class_ID (void) const	
{ 
	return CLASSID_VTOLVEHICLEDEF; 
}

const PersistFactoryClass & VTOLVehicleDefClass::Get_Factory (void) const
{
	return _VTOLVehicleDefFactory;
}

PersistClass * VTOLVehicleDefClass::Create(void) const
{
	VTOLVehicleClass * obj = NEW_REF(VTOLVehicleClass,());
	obj->Init(*this);
	return obj;
}

bool VTOLVehicleDefClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(VTOLVEHICLEDEF_CHUNK_VEHICLEPHYSDEF);
	VehiclePhysDefClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(VTOLVEHICLEDEF_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,VTOLVEHICLEDEF_VARIABLE_MAXVERTICALACCELERATION,MaxVerticalAcceleration);
	WRITE_MICRO_CHUNK(csave,VTOLVEHICLEDEF_VARIABLE_MAXHORIZONTALACCELERATION,MaxHorizontalAcceleration);
	WRITE_MICRO_CHUNK(csave,VTOLVEHICLEDEF_VARIABLE_MAXFUSELAGEPITCH,MaxFuselagePitch);
	WRITE_MICRO_CHUNK(csave,VTOLVEHICLEDEF_VARIABLE_MAXFUSELAGEROLL,MaxFuselageRoll);
	WRITE_MICRO_CHUNK(csave,VTOLVEHICLEDEF_VARIABLE_PITCHCONTROLLERGAIN,PitchControllerGain);
	WRITE_MICRO_CHUNK(csave,VTOLVEHICLEDEF_VARIABLE_PITCHCONTROLLERDAMPING,PitchControllerDamping);
	WRITE_MICRO_CHUNK(csave,VTOLVEHICLEDEF_VARIABLE_ROLLCONTROLLERGAIN,RollControllerGain);
	WRITE_MICRO_CHUNK(csave,VTOLVEHICLEDEF_VARIABLE_ROLLCONTROLLERDAMPING,RollControllerDamping);
	WRITE_MICRO_CHUNK(csave,VTOLVEHICLEDEF_VARIABLE_MAXYAWVELOCITY,MaxYawVelocity);
	WRITE_MICRO_CHUNK(csave,VTOLVEHICLEDEF_VARIABLE_YAWCONTROLLERGAIN,YawControllerGain);
	WRITE_MICRO_CHUNK(csave,VTOLVEHICLEDEF_VARIABLE_MAXENGINEROTATION,MaxEngineRotation);
	WRITE_MICRO_CHUNK(csave,VTOLVEHICLEDEF_VARIABLE_ROTORSPEED,RotorSpeed);
	WRITE_MICRO_CHUNK(csave,VTOLVEHICLEDEF_VARIABLE_ROTORACCELERATION,RotorAcceleration);
	WRITE_MICRO_CHUNK(csave,VTOLVEHICLEDEF_VARIABLE_ROTORDECELERATION,RotorDeceleration);
	csave.End_Chunk();
	return true;
}


bool VTOLVehicleDefClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {

		switch(cload.Cur_Chunk_ID()) {

			case VTOLVEHICLEDEF_CHUNK_VEHICLEPHYSDEF:
				VehiclePhysDefClass::Load(cload);
				break;

			case VTOLVEHICLEDEF_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,VTOLVEHICLEDEF_VARIABLE_MAXVERTICALACCELERATION,MaxVerticalAcceleration);
						READ_MICRO_CHUNK(cload,VTOLVEHICLEDEF_VARIABLE_MAXHORIZONTALACCELERATION,MaxHorizontalAcceleration);
						READ_MICRO_CHUNK(cload,VTOLVEHICLEDEF_VARIABLE_MAXFUSELAGEPITCH,MaxFuselagePitch);
						READ_MICRO_CHUNK(cload,VTOLVEHICLEDEF_VARIABLE_MAXFUSELAGEROLL,MaxFuselageRoll);
						READ_MICRO_CHUNK(cload,VTOLVEHICLEDEF_VARIABLE_PITCHCONTROLLERGAIN,PitchControllerGain);
						READ_MICRO_CHUNK(cload,VTOLVEHICLEDEF_VARIABLE_PITCHCONTROLLERDAMPING,PitchControllerDamping);
						READ_MICRO_CHUNK(cload,VTOLVEHICLEDEF_VARIABLE_ROLLCONTROLLERGAIN,RollControllerGain);
						READ_MICRO_CHUNK(cload,VTOLVEHICLEDEF_VARIABLE_ROLLCONTROLLERDAMPING,RollControllerDamping);
						READ_MICRO_CHUNK(cload,VTOLVEHICLEDEF_VARIABLE_MAXYAWVELOCITY,MaxYawVelocity);
						READ_MICRO_CHUNK(cload,VTOLVEHICLEDEF_VARIABLE_YAWCONTROLLERGAIN,YawControllerGain);
						READ_MICRO_CHUNK(cload,VTOLVEHICLEDEF_VARIABLE_MAXENGINEROTATION,MaxEngineRotation);
						READ_MICRO_CHUNK(cload,VTOLVEHICLEDEF_VARIABLE_ROTORSPEED,RotorSpeed);
						READ_MICRO_CHUNK(cload,VTOLVEHICLEDEF_VARIABLE_ROTORACCELERATION,RotorAcceleration);
						READ_MICRO_CHUNK(cload,VTOLVEHICLEDEF_VARIABLE_ROTORDECELERATION,RotorDeceleration);
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
		}

		cload.Close_Chunk();
	}

	return true;
}


bool VTOLVehicleDefClass::Is_Type(const char * type_name)
{
	if (openw3d::string_compare(type_name,VTOLVehicleDefClass::Get_Type_Name()) == 0) {
		return true;
	} else {
		return VehiclePhysDefClass::Is_Type(type_name);
	}
}

