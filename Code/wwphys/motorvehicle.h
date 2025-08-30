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
 *                     $Archive:: /Commando/Code/wwphys/motorvehicle.h                        $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 10/22/01 5:52p                                              $*
 *                                                                                             *
 *                    $Revision:: 18                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef MOTORVEHICLE_H
#define MOTORVEHICLE_H

#include "vehiclephys.h"

class MotorVehicleDefClass;
class LookupTableClass;


#define RPM_TO_RADS(x) ((x) * (2.0f * WWMATH_PI) / 60.0f)
#define RADS_TO_RPM(x) ((x) * 60.0f / (2.0f * WWMATH_PI))


/**
** MotorVehicleClass 
** This is simply a RigidBodyClass with an internal "engine".  In derived 
** classes, the engine will propel the object in some way.  
** 
** NOTES:
** March 13,2000:  
** --------------
** Adding an engine model which simulates the interaction between the engine speeding up 
** and the wheel forces resisting, etc.  The goal is to get the following behaviours:
** - when wheels aren't contacting the ground, the engine revs up 
** - if the engine is revved up and the wheels come in contact, there is a reaction force
** - wheels can "peel out" in low gears if the parameters are set correctly
** - we have a parameter (EngineAngularVelocity) which can nicely drive a realistic engine sound
** 
** To do this, I've added an EngineTorqueCurve which relates the rpm's of the engine
** to the torque it can output.  This is transmitted to the drive wheels (equally distributed
** between them).  The drive wheels "use up" as much of this torque as they can through
** friction forces on the ground.  The net result of this is the amount that the engine
** angular velocity accelerates.  In first gear for some vehicles, I'd like the engine to
** normally overpower friction and peel out but tune second and above to stay within the
** friction limits.
**
** (EngineTorque(EngineAngularVel) - ReactionTorque) / DriveTrainInertia = EngineAngularAccel
** ReactionTorque = MIN(EngineTorque,FrictionLimit)
** At higher speeds, something (DriveTrainInertia?) must bring EngineTorque down below
** FrictionLimit or we'll be peeling out again!  
** Maybe just fake this: if (gear>1) roll wheels according to actual movement
**
** March 21, 2000:
** Driving engine RPMs the other direction.  Normally, set the engine rpm to match the
** actual wheel speed.
*/
class MotorVehicleClass : public VehiclePhysClass
{
public:

	MotorVehicleClass(void);	
	virtual ~MotorVehicleClass(void);
	virtual MotorVehicleClass * As_MotorVehicleClass(void) override { return this; }

	void					Init(const MotorVehicleDefClass & def);
	
	virtual void		Timestep(float dt) override;

	// Accessors for the current state
	float					Get_Engine_Angular_Velocity(void)							{ return EngineAngularVelocity; }
	float					Get_Engine_RPM(void)												{ return RADS_TO_RPM(EngineAngularVelocity); }
	float					Get_Engine_Torque(void);
	float					Get_Axle_Angular_Velocity(void);
	float					Get_Axle_Torque(void);
	float					Get_Normalized_Engine_RPM(void);	// for sound effects

	int					Get_Current_Gear(void)											{ return CurrentGear; }
	bool					Is_Braking(void)													{ return IsBraking; }

	// Access to the constants for this motor vehicle
	const MotorVehicleDefClass * Get_MotorVehicleDef(void);
	float					Get_Max_Engine_Torque(void);

	// Save-Load system
	virtual bool		Save(ChunkSaveClass &csave) override;
	virtual bool		Load(ChunkLoadClass &cload) override;

protected:

	// Actual vehicles must provide these methods
	virtual float		Get_Ideal_Drive_Axle_Angular_Velocity(void) = 0;
	virtual bool		Drive_Wheels_In_Contact(void) = 0;

	// Integration system
	virtual int			Set_State(const StateVectorClass & new_state,int start_index) override;
	
	// Internal engine simulation methods	
	float					Compute_Engine_Angular_Acceleration(void);
	void					Shift_Up(void);
	void					Shift_Down(void);

	// Variables
	float					EngineAngularVelocity;		// This is a state variable to be integrated with the others
	int					CurrentGear;					
	float					ShiftTimer;						// delay in seconds between allowed shifting
		
	float					AcceleratorFraction;			// amount of gas being given to the engine
	bool					IsBraking;						// braking state

private:

	// Not implemented..
	MotorVehicleClass(const MotorVehicleClass &);
	MotorVehicleClass & operator = (const MotorVehicleClass &);

};

inline const MotorVehicleDefClass * MotorVehicleClass::Get_MotorVehicleDef(void)						
{ 
	// All motor vehicles have to have definitions... Most of their parameters are only
	// stored in the definition now...
	WWASSERT(Definition != NULL);
	return (MotorVehicleDefClass *)Definition; 
}


/**
** MotorVehicleDefClass
** Initialization Structure/Factory/Editor Integration for a MotorVehicleClass
*/
class MotorVehicleDefClass : public VehiclePhysDefClass
{
public:
	
	MotorVehicleDefClass(void);
	virtual ~MotorVehicleDefClass(void);
	
	// From PersistClass
	virtual uint32								Get_Class_ID (void) const override;

	// From PhysDefClass
	virtual const char *						Get_Type_Name(void) override			{ return "MotorVehicleDef"; }
	virtual bool								Is_Type(const char *) override;

	// From PersistClass
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save(ChunkSaveClass &csave) override;
	virtual bool								Load(ChunkLoadClass &cload) override;

	// Read/Write access (DEBUGGING/TESTING ONLY)
	float				Get_Max_Engine_Torque(void) const					{ return MaxEngineTorque; }
	int				Get_Gear_Count(void) const								{ return GearCount; }
	float				Get_Gear_Ratio(int gear) const						{ return GearRatio[gear]; }
	float				Get_Shift_Up_Rpm(void) const							{ return ShiftUpRpm; }
	float				Get_Shift_Down_Rpm(void) const						{ return ShiftDownRpm; }

	void				Set_Max_Engine_Torque(float t)						{ MaxEngineTorque = t; }
	void				Set_Gear_Count(int count)								{ if ((count >= 0) && (count <= 5)) GearCount = count; }
	void				Set_Gear_Ratio(int gear,float ratio)				{ if ((gear >= 0) && (gear <= 5))GearRatio[gear] = ratio; }
	void				Set_Shift_Up_Rpm(float rpm)							{ ShiftUpRpm = rpm; ShiftUpAvel = RPM_TO_RADS(ShiftUpRpm); }
	void				Set_Shift_Down_Rpm(float rpm)							{ ShiftDownRpm = rpm; ShiftDownAvel = RPM_TO_RADS(ShiftDownRpm); }

	//	Editable interface requirements
	DECLARE_EDITABLE(MotorVehicleDefClass,VehiclePhysDefClass);

protected:

	// Engine response
	float				MaxEngineTorque;
	StringClass		EngineTorqueCurveFilename;
	LookupTableClass * EngineTorqueCurve;			// pointer to the torque curve 

	// Transmission and Differential Gear Ratios
	enum { MAX_GEAR_COUNT = 6 };

	int				GearCount;
	float				GearRatio[MAX_GEAR_COUNT];
	float				PeakEngineTorque[MAX_GEAR_COUNT];
	float				FinalDriveGearRatio;
	
	// Inertias in the drive train
	float				DriveTrainInertia;

	// Shifting variables
	float				ShiftUpRpm;
	float				ShiftDownRpm;
	float				ShiftUpAvel;		// same as ShiftUpRpm but in radians per second
	float				ShiftDownAvel;		// same as ShiftDownRpm but in radians per second

	friend class MotorVehicleClass;
};


#endif
