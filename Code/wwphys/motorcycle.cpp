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
 *                     $Archive:: /Commando/Code/wwphys/motorcycle.cpp                        $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 8/17/01 8:45p                                               $*
 *                                                                                             *
 *                    $Revision:: 28                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "motorcycle.h"
#include "physcontrol.h"
#include "persistfactory.h"
#include "simpledefinitionfactory.h"
#include "wwphysids.h"
#include "wwhack.h"
#include "wwprofile.h"

DECLARE_FORCE_LINK(motorcycle);

/*
** Declare a PersistFactory for MotorcycleClasses
*/
SimplePersistFactoryClass<MotorcycleClass,PHYSICS_CHUNKID_MOTORCYCLE>	_MotorcycleFactory;


/*
** Chunk-ID's used by MotorcycleClass
*/
enum
{
	MOTORCYCLE_CHUNK_WHEELEDVEHICLE		= 0x06511110,
	MOTORCYCLE_CHUNK_VARIABLES,

	MOTORCYCLE_VARIABLE_LEANK0				= 0x00,
	MOTORCYCLE_VARIABLE_LEANK1				= 0x01,
};

MotorcycleClass::MotorcycleClass(void) :
	LeanK0(18.0f),
	LeanK1(5.0f)
{
}

void MotorcycleClass::Init(const MotorcycleDefClass & def)
{
	WheeledVehicleClass::Init(def);
	LeanK0 = def.LeanK0;
	LeanK1 = def.LeanK1;
}

MotorcycleClass::~MotorcycleClass(void)
{
}

void MotorcycleClass::Compute_Force_And_Torque(Vector3 * force,Vector3 * torque)
{
	static float MAX_ROLL		= DEG_TO_RADF(20.0f);
	static float MAX_VDOT		= 15.0f;						// at this speed, cycle will lean over MAX_ROLL
	static float MAX_TURN		= 1.0f;						// at this turn, cycle will lean over MAX_ROLL
	static float MAX_LVEL		= 10.0f;

	{
		WWPROFILE("MotorcycleClass::Compute_Force_And_Torque");

		// Read the controller inputs
		// Accept either strafe or turn as a turn command
		float turn_left = 0.0f;
		if (Get_Controller() != NULL) {
			if (WWMath::Fabs(Controller->Get_Turn_Left()) > WWMath::Fabs(Controller->Get_Move_Left())) {
				turn_left = Controller->Get_Turn_Left();
			} else {
				turn_left = Controller->Get_Move_Left();
			}
		}

		// What is our rotation about our x axis, we want it to be zero always
		Vector3 yvec;
		Get_Transform().Get_Y_Vector(&yvec);
		float roll = atan2(yvec.Z,sqrt(yvec.X * yvec.X + yvec.Y * yvec.Y));
		float target_roll;
		float vdot = Vector3::Dot_Product(Get_Transform().Get_X_Vector(),Velocity);

		// compute our lateral velocity, first we need a vector in the lateral direction
		Vector3 lateral_vel = Velocity - vdot * Get_Transform().Get_X_Vector();
		float lvel_sign = WWMath::Sign(Vector3::Dot_Product(lateral_vel,Get_Transform().Get_Y_Vector()));
		lateral_vel.Z = 0;
		float lvel = lateral_vel.Length();

		if (lvel > 0.0f) {

			float clamp_lvel = lvel/MAX_LVEL;
			if (clamp_lvel > 1.0f) clamp_lvel = 1.0f;

			float clamp_turn = turn_left/MAX_TURN;
			if (clamp_turn > 1.0f) clamp_turn = 1.0f;
			if (clamp_turn < -1.0f) clamp_turn = -1.0f;

			target_roll = MAX_ROLL * lvel_sign * clamp_lvel;

		} else {

			target_roll = 0.0f;
		}

		// What is our angular velocity about the x axis we want it to be zero as well
		float droll = Vector3::Dot_Product(AngularVelocity,Get_Transform().Get_X_Vector());

		WWASSERT(WWMath::Is_Valid_Float(roll));
		WWASSERT(WWMath::Is_Valid_Float(target_roll));
		WWASSERT(WWMath::Is_Valid_Float(droll));

		WWASSERT(WWMath::Is_Valid_Float(torque->X));
		WWASSERT(WWMath::Is_Valid_Float(torque->Y));
		WWASSERT(WWMath::Is_Valid_Float(torque->Z));

		// Apply a balancing torque
		*torque += (LeanK0 * (target_roll - roll) + LeanK1 * (0.0f - droll)) * Get_Transform().Get_X_Vector();

		// Record the amount that the character on the bike should lean
		LeanValue = (target_roll - roll) / MAX_ROLL;
	}

	// Parent classes compute their forces and torques
	WheeledVehicleClass::Compute_Force_And_Torque(force,torque);
}

void MotorcycleClass::Compute_Inertia(void)
{
	WheeledVehicleClass::Compute_Inertia();
}

const PersistFactoryClass & MotorcycleClass::Get_Factory (void) const
{
	return _MotorcycleFactory;
}

bool MotorcycleClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk(MOTORCYCLE_CHUNK_WHEELEDVEHICLE);
	WheeledVehicleClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(MOTORCYCLE_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,MOTORCYCLE_VARIABLE_LEANK0,LeanK0);
	WRITE_MICRO_CHUNK(csave,MOTORCYCLE_VARIABLE_LEANK1,LeanK1);
	csave.End_Chunk();
	return true;
}

bool MotorcycleClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case MOTORCYCLE_CHUNK_WHEELEDVEHICLE:
				WheeledVehicleClass::Load(cload);	
				break;
			
			case MOTORCYCLE_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,MOTORCYCLE_VARIABLE_LEANK0,LeanK0);
						READ_MICRO_CHUNK(cload,MOTORCYCLE_VARIABLE_LEANK1,LeanK1);
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
** MotorcycleDefClass Implementation
**
***********************************************************************************************/

/*
** Persist factory for MotorcycleDefClass
*/
SimplePersistFactoryClass<MotorcycleDefClass,PHYSICS_CHUNKID_MOTORCYCLEDEF>	_MotorcycleDefFactory;

/*
** Definition factory for MotorcycleDefClass.  This makes it show up in the editor
*/
DECLARE_DEFINITION_FACTORY(MotorcycleDefClass, CLASSID_MOTORCYCLEDEF, "Motorcycle") _MotorcycleDefDefFactory;

/*
** Chunk ID's used by MotorcycleDefClass
*/
enum 
{
	MOTORCYCLEDEF_CHUNK_WHEELEDVEHICLEDEF		= 0x00516000,			// (parent class)
	MOTORCYCLEDEF_CHUNK_VARIABLES,

	MOTORCYCLEDEF_VARIABLE_LEANK0					= 0x00,
	MOTORCYCLEDEF_VARIABLE_LEANK1,
};

MotorcycleDefClass::MotorcycleDefClass(void) :
	LeanK0(18.0f),
	LeanK1(5.0f)
{	
	// make our parameters editable!
	FLOAT_EDITABLE_PARAM(MotorcycleDefClass, LeanK0, 0.01f, 100000.0f);
	FLOAT_EDITABLE_PARAM(MotorcycleDefClass, LeanK1, 0.01f, 100000.0f);
}

uint32 MotorcycleDefClass::Get_Class_ID (void) const	
{ 
	return CLASSID_MOTORCYCLEDEF; 
}

PersistClass *	MotorcycleDefClass::Create(void) const
{
	MotorcycleClass * obj = NEW_REF(MotorcycleClass,());
	obj->Init(*this);
	return obj;
}

const PersistFactoryClass & MotorcycleDefClass::Get_Factory (void) const
{
	return _MotorcycleDefFactory;
}

bool MotorcycleDefClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(MOTORCYCLEDEF_CHUNK_WHEELEDVEHICLEDEF);
	WheeledVehicleDefClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(MOTORCYCLEDEF_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,MOTORCYCLEDEF_VARIABLE_LEANK0,LeanK0);
	WRITE_MICRO_CHUNK(csave,MOTORCYCLEDEF_VARIABLE_LEANK1,LeanK1);
	csave.End_Chunk();
	return true;
}

bool MotorcycleDefClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {

		switch(cload.Cur_Chunk_ID()) {

			case MOTORCYCLEDEF_CHUNK_WHEELEDVEHICLEDEF:
				WheeledVehicleDefClass::Load(cload);
				break;

			case MOTORCYCLEDEF_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,MOTORCYCLEDEF_VARIABLE_LEANK0,LeanK0);
						READ_MICRO_CHUNK(cload,MOTORCYCLEDEF_VARIABLE_LEANK1,LeanK1);
					}
					cload.Close_Micro_Chunk();
				}
				break;
		}

		cload.Close_Chunk();
	}
	return true;
}

bool MotorcycleDefClass::Is_Type(const char * type_name)
{
	if (stricmp(type_name,MotorcycleDefClass::Get_Type_Name()) == 0) {
		return true;
	} else {
		return WheeledVehicleDefClass::Is_Type(type_name);
	}
}

