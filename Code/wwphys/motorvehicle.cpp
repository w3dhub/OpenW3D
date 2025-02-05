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
 *                     $Archive:: /Commando/Code/wwphys/motorvehicle.cpp                      $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 10/23/01 1:56p                                              $*
 *                                                                                             *
 *                    $Revision:: 37                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "motorvehicle.h"
#include "physcontrol.h"
#include "persistfactory.h"
#include "wwphysids.h"
#include "wwhack.h"
#include "wwprofile.h"
#include "simplevec.h"
#include "ode.h"
#include "lookuptable.h"


DECLARE_FORCE_LINK(motorvehicle);

const float GEAR_SHIFT_DELAY = 1.0f;
const float MIN_BRAKING_SPEED = 1.5f;		// below this forward speed, we drive in reverse

/***********************************************************************************************
**
** MotorVehicleClass Implementation
**
***********************************************************************************************/

/*
** Chunk Ids used by MotorVehicleClass
*/
enum
{
	MOTO_CHUNK_RIGIDBODY				= 0x01234500,		// used to be derived from RigidBody
	MOTO_CHUNK_VARIABLES,
	MOTO_CHUNK_VEHICLEPHYS,									// now derived from VehiclePhys

	OBSOLETE_MOTO_VARIABLE_CURENGINETORQUE	= 0x00,
	MOTO_VARIABLE_ENGINEANGULARVELOCITY,
	MOTO_VARIABLE_CURRENTGEAR,
	MOTO_VARIABLE_SHIFTIMER,
};

	
MotorVehicleClass::MotorVehicleClass(void) :
	EngineAngularVelocity(0.0f),
	CurrentGear(0),
	ShiftTimer(0),
	AcceleratorFraction(0.0f),
	IsBraking(false)
{
}

void MotorVehicleClass::Init(const MotorVehicleDefClass & def)
{ 
	VehiclePhysClass::Init(def);
}

MotorVehicleClass::~MotorVehicleClass(void)
{
}

void MotorVehicleClass::Timestep(float dt)
{	
	{
		WWPROFILE("MotorVehicle::Timestep");
	
		const MotorVehicleDefClass * def = Get_MotorVehicleDef();

		// Update the accelerator and braking state according to the current inputs
		AcceleratorFraction = 0.0f;
		IsBraking = false;
		if (Controller) {
			Vector3 objvel;
			Matrix3D::Inverse_Rotate_Vector(Get_Transform(),Velocity,&objvel);

			if ((Controller->Get_Move_Forward() < 0.0f) && (objvel.X > MIN_BRAKING_SPEED)) {
				IsBraking = true;
			} else if ((Controller->Get_Move_Forward() > 0) && (objvel.X < -MIN_BRAKING_SPEED)) {
				IsBraking = true;
			} else {
				AcceleratorFraction = Controller->Get_Move_Forward();
			}
		} 

		// Engine simulation revvs up if the drive wheels are not in contact.
		if (!Drive_Wheels_In_Contact()) {
			EngineAngularVelocity += dt * Compute_Engine_Angular_Acceleration();
		}

		// Update our current gear if we are in contact with the ground
		if ((ShiftTimer <= 0.0f) && (Drive_Wheels_In_Contact() == true)) {

			// if we're near red-line and our shift timer has expired, shift up.
			// if we're at very low rpms, shift down.
			if ((EngineAngularVelocity > def->ShiftUpAvel) && (CurrentGear < def->GearCount)) {
				Shift_Up();
			} else if ((EngineAngularVelocity < def->ShiftDownAvel) && (CurrentGear > 0)) {
				Shift_Down();
			}

		} else if (ShiftTimer > 0.0f) {
			ShiftTimer -= dt;
		}
	}
	
	VehiclePhysClass::Timestep(dt);
}


/***********************************************************************************************
**
** Engine Simulation code for MotorVehicleClass
**
***********************************************************************************************/

int MotorVehicleClass::Set_State(const StateVectorClass & new_state,int start_index)
{
	// Whenever we get a new state from the integrator, make sure we update our
	// engine's angular velocity.  This will ensure that the torque output from the
	// engine is in sync with the vehicle's motion. (even though we're not really
	// "simulating" the angular velocity of the engine...)
	start_index = VehiclePhysClass::Set_State(new_state,start_index);

	const MotorVehicleDefClass * def = Get_MotorVehicleDef();
	
	if (Drive_Wheels_In_Contact() == true) {
		float wheel_avel = Get_Ideal_Drive_Axle_Angular_Velocity();
		EngineAngularVelocity = wheel_avel * def->GearRatio[CurrentGear] * def->FinalDriveGearRatio;
	}
	return start_index;
}

float MotorVehicleClass::Get_Engine_Torque(void)
{
	// Torque output by the engine depends on the torque curve, how much gas is being given, and 
	// the engine's maximum torque.  The maximum torque is used to scale the normalized engine
	// torque curve so that we can just re-use the same table for many vehicles.
	const MotorVehicleDefClass * def = Get_MotorVehicleDef();

	float normalized_torque = 0;
	if (def->EngineTorqueCurve != NULL) {
		normalized_torque = def->EngineTorqueCurve->Get_Value(WWMath::Fabs(EngineAngularVelocity));
	}

	return def->MaxEngineTorque * AcceleratorFraction * normalized_torque;
}

float MotorVehicleClass::Get_Axle_Angular_Velocity(void)
{
	const MotorVehicleDefClass * def = Get_MotorVehicleDef();
	return EngineAngularVelocity / (def->GearRatio[CurrentGear] * def->FinalDriveGearRatio);
}

float MotorVehicleClass::Get_Axle_Torque(void)
{
	const MotorVehicleDefClass * def = Get_MotorVehicleDef();
	return Get_Engine_Torque() * def->GearRatio[CurrentGear] * def->FinalDriveGearRatio;
}

float MotorVehicleClass::Get_Normalized_Engine_RPM(void)
{
	return Get_Engine_RPM() / Get_MotorVehicleDef()->ShiftUpRpm;
}

float MotorVehicleClass::Get_Max_Engine_Torque(void)
{ 
	return Get_MotorVehicleDef()->MaxEngineTorque; 
}


float MotorVehicleClass::Compute_Engine_Angular_Acceleration(void)
{
	const MotorVehicleDefClass * def = Get_MotorVehicleDef();
	return Get_Engine_Torque() / def->DriveTrainInertia;
}

void MotorVehicleClass::Shift_Up(void)
{
	const MotorVehicleDefClass * def = Get_MotorVehicleDef();
	if (CurrentGear < def->GearCount-1) {
		CurrentGear++;
		ShiftTimer = GEAR_SHIFT_DELAY;
	}
}

void MotorVehicleClass::Shift_Down(void)
{
	if (CurrentGear > 0) {
		CurrentGear--;
		ShiftTimer = GEAR_SHIFT_DELAY;
	}
}


/***********************************************************************************************
**
** Save-Load for MotorVehicleClass
** Note: MotorVehicleClass contains pure virtuals so it does not instantiate a PersistFactory...
**
***********************************************************************************************/

bool MotorVehicleClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk(MOTO_CHUNK_VEHICLEPHYS);
	VehiclePhysClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(MOTO_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,MOTO_VARIABLE_ENGINEANGULARVELOCITY,EngineAngularVelocity);
	WRITE_MICRO_CHUNK(csave,MOTO_VARIABLE_CURRENTGEAR,CurrentGear);
	WRITE_MICRO_CHUNK(csave,MOTO_VARIABLE_SHIFTIMER,ShiftTimer);
	csave.End_Chunk();
	return true;
}

bool MotorVehicleClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		
		switch(cload.Cur_Chunk_ID()) 
		{
			case MOTO_CHUNK_RIGIDBODY:				// used to be derived directly from RigidBody... Obsolete now
				RigidBodyClass::Load(cload);
				break;
				
			case MOTO_CHUNK_VEHICLEPHYS:
				VehiclePhysClass::Load(cload);
				break;

			case MOTO_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,MOTO_VARIABLE_ENGINEANGULARVELOCITY,EngineAngularVelocity);
						READ_MICRO_CHUNK(cload,MOTO_VARIABLE_CURRENTGEAR,CurrentGear);
						READ_MICRO_CHUNK(cload,MOTO_VARIABLE_SHIFTIMER,ShiftTimer);
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

/***********************************************************************************************
**
** MotorVehicleDefClass Implementation
**
***********************************************************************************************/

/*
** Declare a PersistFactory for MotorVehicleClasses
*/
SimplePersistFactoryClass<MotorVehicleDefClass,PHYSICS_CHUNKID_MOTORVEHICLEDEF>	_MotorVehicleDefFactory;

/*
** Chunk ID's used by MotorVehicleDefClass
*/
enum 
{
	MOTORVEHICLEDEF_CHUNK_RIGIDBODYDEF							= 0x00516000,		// (old parent class)
	MOTORVEHICLEDEF_CHUNK_VARIABLES,
	MOTORVEHICLEDEF_CHUNK_VEHICLEPHYSDEF,												// (current parent class)

	MOTORVEHICLEDEF_VARIABLE_MAXENGINETORQUE					= 0x00,
	MOTORVEHICLEDEF_VARIABLE_ENGINETORQUECURVEFILENAME,
	
	MOTORVEHICLEDEF_VARIABLE_GEARCOUNT,
	MOTORVEHICLEDEF_VARIABLE_GEARRATIO1,
	MOTORVEHICLEDEF_VARIABLE_GEARRATIO2,
	MOTORVEHICLEDEF_VARIABLE_GEARRATIO3,
	MOTORVEHICLEDEF_VARIABLE_GEARRATIO4,
	MOTORVEHICLEDEF_VARIABLE_GEARRATIO5,
	MOTORVEHICLEDEF_VARIABLE_GEARRATIO6,
	MOTORVEHICLEDEF_VARIABLE_FINALDRIVEGEARRATIO,

	OBSOLETE_MOTORVEHICLEDEF_VARIABLE_ENGINEINERTIA,
	OBSOLETE_MOTORVEHICLEDEF_VARIABLE_TRANSMISSIONINERTIA,
	OBSOLETE_MOTORVEHICLEDEF_VARIABLE_FINALDRIVEINERTIA,

	MOTORVEHICLEDEF_VARIABLE_SHIFTUPRPM,
	MOTORVEHICLEDEF_VARIABLE_SHIFTDOWNRPM,
	MOTORVEHICLEDEF_VARIABLE_DRIVETRAININERTIA,
};


MotorVehicleDefClass::MotorVehicleDefClass(void) :
	MaxEngineTorque(5.0f),
	EngineTorqueCurveFilename("Vehicles\\PhysicsTables\\DefaultEngineTorque.tbl"),
	EngineTorqueCurve(NULL),
	GearCount(4),
	FinalDriveGearRatio(2.92f),
	ShiftUpRpm(7000),
	ShiftDownRpm(2000),
	DriveTrainInertia(0.1f)
{
	
	GearRatio[0] = 12.01f;	// 1989 Ford Taurus gear ratios :-)
	GearRatio[1] = 7.82f;
	GearRatio[2] = 5.16f;
	GearRatio[3] = 3.81f;
	GearRatio[4] = 2.79f;
	GearRatio[5] = 1.0f;

	ShiftUpAvel = RPM_TO_RADS(ShiftUpRpm);
	ShiftDownAvel = RPM_TO_RADS(ShiftDownRpm);

	// make our parameters editable
	FLOAT_UNITS_PARAM(MotorVehicleDefClass, MaxEngineTorque, 0.01f, 100000.0f,"N*m");	
	FILENAME_PARAM(MotorVehicleDefClass, EngineTorqueCurveFilename, "Table Files", ".tbl");

	INT_EDITABLE_PARAM(MotorVehicleDefClass,GearCount,1,6);
	
	FLOAT_EDITABLE_PARAM(MotorVehicleDefClass,GearRatio[0],1.0f,100.0f);
	FLOAT_EDITABLE_PARAM(MotorVehicleDefClass,GearRatio[1],1.0f,100.0f);
	FLOAT_EDITABLE_PARAM(MotorVehicleDefClass,GearRatio[2],1.0f,100.0f);
	FLOAT_EDITABLE_PARAM(MotorVehicleDefClass,GearRatio[3],1.0f,100.0f);
	FLOAT_EDITABLE_PARAM(MotorVehicleDefClass,GearRatio[4],1.0f,100.0f);
	FLOAT_EDITABLE_PARAM(MotorVehicleDefClass,GearRatio[5],1.0f,100.0f);
	FLOAT_EDITABLE_PARAM(MotorVehicleDefClass,FinalDriveGearRatio,0.0f,100.0f);

	FLOAT_EDITABLE_PARAM(MotorVehicleDefClass,DriveTrainInertia,0.001f,100000.0f);

	FLOAT_EDITABLE_PARAM(MotorVehicleDefClass,ShiftUpRpm,1.0f,100000.0f);
	FLOAT_EDITABLE_PARAM(MotorVehicleDefClass,ShiftDownRpm,1.0f,100000.0f);
}

MotorVehicleDefClass::~MotorVehicleDefClass(void)
{
	REF_PTR_RELEASE(EngineTorqueCurve);
}

uint32 MotorVehicleDefClass::Get_Class_ID (void) const	
{ 
	return CLASSID_MOTORVEHICLEDEF; 
}

const PersistFactoryClass & MotorVehicleDefClass::Get_Factory (void) const
{
	return _MotorVehicleDefFactory;
}

bool MotorVehicleDefClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(MOTORVEHICLEDEF_CHUNK_VEHICLEPHYSDEF);
	VehiclePhysDefClass::Save(csave);
	csave.End_Chunk();

	ShiftUpAvel = RPM_TO_RADS(ShiftUpRpm);
	ShiftDownAvel = RPM_TO_RADS(ShiftDownRpm);

	csave.Begin_Chunk(MOTORVEHICLEDEF_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,MOTORVEHICLEDEF_VARIABLE_MAXENGINETORQUE,MaxEngineTorque);
	WRITE_MICRO_CHUNK_WWSTRING(csave,MOTORVEHICLEDEF_VARIABLE_ENGINETORQUECURVEFILENAME,EngineTorqueCurveFilename);
	WRITE_MICRO_CHUNK(csave,MOTORVEHICLEDEF_VARIABLE_GEARCOUNT,GearCount);
	WRITE_MICRO_CHUNK(csave,MOTORVEHICLEDEF_VARIABLE_GEARRATIO1,GearRatio[0]);
	WRITE_MICRO_CHUNK(csave,MOTORVEHICLEDEF_VARIABLE_GEARRATIO2,GearRatio[1]);
	WRITE_MICRO_CHUNK(csave,MOTORVEHICLEDEF_VARIABLE_GEARRATIO3,GearRatio[2]);
	WRITE_MICRO_CHUNK(csave,MOTORVEHICLEDEF_VARIABLE_GEARRATIO4,GearRatio[3]);
	WRITE_MICRO_CHUNK(csave,MOTORVEHICLEDEF_VARIABLE_GEARRATIO5,GearRatio[4]);
	WRITE_MICRO_CHUNK(csave,MOTORVEHICLEDEF_VARIABLE_GEARRATIO6,GearRatio[5]);
	WRITE_MICRO_CHUNK(csave,MOTORVEHICLEDEF_VARIABLE_FINALDRIVEGEARRATIO,FinalDriveGearRatio);
	WRITE_MICRO_CHUNK(csave,MOTORVEHICLEDEF_VARIABLE_SHIFTUPRPM,ShiftUpRpm);
	WRITE_MICRO_CHUNK(csave,MOTORVEHICLEDEF_VARIABLE_SHIFTDOWNRPM,ShiftDownRpm);
	WRITE_MICRO_CHUNK(csave,MOTORVEHICLEDEF_VARIABLE_DRIVETRAININERTIA,DriveTrainInertia);
	csave.End_Chunk();
	return true;
}


bool MotorVehicleDefClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {

		switch(cload.Cur_Chunk_ID()) {

			case MOTORVEHICLEDEF_CHUNK_RIGIDBODYDEF:		// old parent class
				RigidBodyDefClass::Load(cload);
				break;

			case MOTORVEHICLEDEF_CHUNK_VEHICLEPHYSDEF:	// current parent class
				VehiclePhysDefClass::Load(cload);
				break;

			case MOTORVEHICLEDEF_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,MOTORVEHICLEDEF_VARIABLE_MAXENGINETORQUE,MaxEngineTorque);
						READ_MICRO_CHUNK_WWSTRING(cload,MOTORVEHICLEDEF_VARIABLE_ENGINETORQUECURVEFILENAME,EngineTorqueCurveFilename);
						READ_MICRO_CHUNK(cload,MOTORVEHICLEDEF_VARIABLE_GEARCOUNT,GearCount);
						READ_MICRO_CHUNK(cload,MOTORVEHICLEDEF_VARIABLE_GEARRATIO1,GearRatio[0]);
						READ_MICRO_CHUNK(cload,MOTORVEHICLEDEF_VARIABLE_GEARRATIO2,GearRatio[1]);
						READ_MICRO_CHUNK(cload,MOTORVEHICLEDEF_VARIABLE_GEARRATIO3,GearRatio[2]);
						READ_MICRO_CHUNK(cload,MOTORVEHICLEDEF_VARIABLE_GEARRATIO4,GearRatio[3]);
						READ_MICRO_CHUNK(cload,MOTORVEHICLEDEF_VARIABLE_GEARRATIO5,GearRatio[4]);
						READ_MICRO_CHUNK(cload,MOTORVEHICLEDEF_VARIABLE_GEARRATIO6,GearRatio[5]);
						READ_MICRO_CHUNK(cload,MOTORVEHICLEDEF_VARIABLE_FINALDRIVEGEARRATIO,FinalDriveGearRatio);
						OBSOLETE_MICRO_CHUNK(OBSOLETE_MOTORVEHICLEDEF_VARIABLE_ENGINEINERTIA);
						OBSOLETE_MICRO_CHUNK(OBSOLETE_MOTORVEHICLEDEF_VARIABLE_TRANSMISSIONINERTIA);
						OBSOLETE_MICRO_CHUNK(OBSOLETE_MOTORVEHICLEDEF_VARIABLE_FINALDRIVEINERTIA);
						READ_MICRO_CHUNK(cload,MOTORVEHICLEDEF_VARIABLE_SHIFTUPRPM,ShiftUpRpm);
						READ_MICRO_CHUNK(cload,MOTORVEHICLEDEF_VARIABLE_SHIFTDOWNRPM,ShiftDownRpm);
						READ_MICRO_CHUNK(cload,MOTORVEHICLEDEF_VARIABLE_DRIVETRAININERTIA,DriveTrainInertia);
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",__FILE__,__LINE__));
				break;
		}

		cload.Close_Chunk();
	}

	ShiftUpAvel = RPM_TO_RADS(ShiftUpRpm);
	ShiftDownAvel = RPM_TO_RADS(ShiftDownRpm);

	REF_PTR_RELEASE(EngineTorqueCurve);
	if (!EngineTorqueCurveFilename.Is_Empty()) {

		// strip the path off the filename
		const char * fname = strrchr(EngineTorqueCurveFilename,'\\');
		if (fname == NULL) {
			EngineTorqueCurve = LookupTableMgrClass::Get_Table(EngineTorqueCurveFilename);
		} else {
			EngineTorqueCurve = LookupTableMgrClass::Get_Table(fname + 1);
		}
	}
	if (EngineTorqueCurve == NULL) {
		WWDEBUG_SAY(("Missing EngineTorqueCurve Table file: %s\r\n",EngineTorqueCurveFilename));
		EngineTorqueCurve = LookupTableMgrClass::Get_Table("DefaultTable");
	}

	return true;
}

bool MotorVehicleDefClass::Is_Type(const char * type_name)
{
	if (stricmp(type_name,MotorVehicleDefClass::Get_Type_Name()) == 0) {
		return true;
	} else {
		return VehiclePhysDefClass::Is_Type(type_name);
	}
}


