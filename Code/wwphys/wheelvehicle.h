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
 *                     $Archive:: /Commando/Code/wwphys/wheelvehicle.h                        $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 9/20/01 1:57p                                               $*
 *                                                                                             *
 *                    $Revision:: 22                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef WHEELVEHICLE_H
#define WHEELVEHICLE_H

#ifndef ALWAYS_H
#include "always.h"
#endif

#ifndef MOTORVEHICLE_H
#include "motorvehicle.h"
#endif

#ifndef LOOKUPTABLE_H
#include "lookuptable.h"
#endif

#include "wheel.h"
class WheelClass;
class NormalWheelClass;
class WheeledVehicleDefClass;

/**
** WheeledVehicleClass 
** This is a derivation of RigidBodyClass which basically adds wheels to 
** a rigid body object.  Each wheel detected in the model will have a friction
** model and a spring-damper force.  Each one will "roll" an amount derived
** from the motion of the contact point for the wheel.  Special engine wheels
** drive the model and steering wheels can be turned.  The goal here is for
** this class to handle all wheeled vehicles in the game.  3, 4, and 6 wheeled
** vehicles will hopefully be possible...
**
** Wheeled Vehicle Physics Update Feb 29,2000
** - The two most important things to simulate are the forces at the contact
**   patches of the tires and the behavior of the engine.
*/
class WheeledVehicleClass : public MotorVehicleClass
{
public:

	WheeledVehicleClass(void);
	virtual ~WheeledVehicleClass(void);
	virtual WheeledVehicleClass * As_WheeledVehicleClass(void) override { return this; }
	const WheeledVehicleDefClass * Get_WheeledVehicleDef(void);

	void											Init(const WheeledVehicleDefClass & def);

	/*
	** Save-Load System
	*/
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save (ChunkSaveClass &csave) override;
	virtual bool								Load (ChunkLoadClass &cload) override;

protected:

	virtual SuspensionElementClass *	Alloc_Suspension_Element(void) override;

	virtual void					Compute_Force_And_Torque(Vector3 * force,Vector3 * torque) override;
	virtual float					Get_Ideal_Drive_Axle_Angular_Velocity(void) override;
	virtual bool					Drive_Wheels_In_Contact(void) override;

	float								SteeringAngle;

private:

	WheeledVehicleClass(const WheeledVehicleClass &);
	WheeledVehicleClass & operator = (const WheeledVehicleClass &);

};

/**
** WheeledVehicleDefClass
** Initialization/Editor Integration for WheeledVehicleClass
*/
class WheeledVehicleDefClass : public MotorVehicleDefClass
{
public:
	WheeledVehicleDefClass(void);
	
	// From DefinitionClass
	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create(void) const override;

	// From PhysDefClass
	virtual const char *						Get_Type_Name(void) override				{ return "WheeledVehicleDef"; }
	virtual bool								Is_Type(const char *) override;

	// From PersistClass
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save(ChunkSaveClass &csave) override;
	virtual bool								Load(ChunkLoadClass &cload) override;

	// Read/Write access to our variables
	float											Get_Max_Steering_Angle(void) const	{ return MaxSteeringAngle; }
	void											Set_Max_Steering_Angle(float a)		{ MaxSteeringAngle = a; }

	//	Editable interface requirements
	DECLARE_EDITABLE(WheeledVehicleDefClass,MotorVehicleDefClass);

protected:

	float											MaxSteeringAngle;			// maximum angle for the steering wheels
	
	friend class WheeledVehicleClass;
};


/*
** Inline functions
*/

inline const WheeledVehicleDefClass * WheeledVehicleClass::Get_WheeledVehicleDef(void)
{
	return (WheeledVehicleDefClass *)Definition;
}


#endif

