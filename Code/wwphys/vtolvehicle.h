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
 *                     $Archive:: /Commando/Code/wwphys/vtolvehicle.h                         $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/17/01 8:36p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef VTOLVEHICLE_H
#define VTOLVEHICLE_H

#include "vehiclephys.h"

class VTOLVehicleDefClass;

/**
** VTOLVehicleClass
** This physics object generates the behavior of a VTOL aircraft
**
** Physical Behavior:
** I think all VTOL craft can use the same behavior.  The vertical thrust
** is related to the user's setting and the position of the craft between
** the flight-map-meshes.  The orientation of the craft is controlled by
** a controller which exerts torques to align the craft to a desired orientation
** The x-y accelerations of the craft will be related to its orientation.
** Turning will be an independent controller I guess...
** In addition, "WheelP" bones should be used to provide landing gear in case 
** the orca tries to "land".  
**
** Model animation: 
** For Orcas, I want to show the engines tilting and engine flames lengthening 
** and shortening.  For Helecopters, I want to just tilt the rotor...  Maybe 
** I'll just have to do each in a derived class which is custom coded for Orca 
** or Helecopter behavior...
** - Decided to add support for all of the above behaviors to this class.
**
** Ideas:
** - Rotor bones (helecopters), these always spin at a rate related to the
**   engine and tilt for turning and accelerating
** - Thruster bones (orca engines), these rotate forward and back depending
**   on how much the user is accelerating forward.
** - Flame bones (orca engines), these translate along their Z-axis to stretch
**   a flame 'skin'.
*/
class VTOLVehicleClass : public VehiclePhysClass
{
public:
	VTOLVehicleClass(void);
	virtual ~VTOLVehicleClass(void);
	virtual VTOLVehicleClass * As_VTOLVehicleClass(void) override { return this; }
	const VTOLVehicleDefClass * Get_VTOLVehicleDef(void) { return (VTOLVehicleDefClass *)Definition; }

	void								Init(const VTOLVehicleDefClass & def);

	virtual void					Render(RenderInfoClass & rinfo) override;
	virtual void					Set_Model(RenderObjClass * model) override;
	virtual void					Timestep(float dt) override;

	/*
	** Save-Load System
	*/
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save (ChunkSaveClass &csave) override;
	virtual bool								Load (ChunkLoadClass &cload) override;
	virtual void								On_Post_Load (void) override;

protected:

	virtual bool					Can_Go_To_Sleep(void) { return false; }

	virtual void					Compute_Force_And_Torque(Vector3 * force,Vector3 * torque) override;
	virtual SuspensionElementClass *	Alloc_Suspension_Element(void) override;
	virtual float					Get_Normalized_Engine_Flame(void) override;

	void								Release_Engine_Bones(void);
	void								Update_Cached_Model_Parameters(void);

	/*
	** Captured bones for physics-based graphical effects
	*/
	SimpleVecClass<int>			EngineAngleBones;
	SimpleVecClass<int>			RotorAngleBones;				

	/*
	** Visual state variables
	*/
	float								NormalizedEngineRotation;
	float								NormalizedEngineThrust;
	float								RotorAngle;
	float								RotorAngularVelocity;

private:

	// not implemented
	VTOLVehicleClass(const VTOLVehicleClass &);
	VTOLVehicleClass & operator = (const VTOLVehicleClass &);
};


/**
** VTOLVehicleDefClass
** Initialization/Editor Integration for VTOLVehicleClass
*/
class VTOLVehicleDefClass : public VehiclePhysDefClass
{
public:
	VTOLVehicleDefClass(void);
	
	/*
	** From DefinitionClass
	*/
	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create(void) const override;

	/*
	** From PhysDefClass
	*/
	virtual const char *						Get_Type_Name(void) override				{ return "VTOLVehicleDef"; }
	virtual bool								Is_Type(const char *) override;

	/*
	** From PersistClass
	*/
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save(ChunkSaveClass &csave) override;
	virtual bool								Load(ChunkLoadClass &cload) override;

	/*
	** Editable interface requirements
	*/
	DECLARE_EDITABLE(VTOLVehicleDefClass,VehiclePhysDefClass);

protected:

	/*
	** Engine thrust in units of acceleration (force = mass * accel...)
	*/
	float											MaxVerticalAcceleration;
	float											MaxHorizontalAcceleration;

	/*
	** Vehicle behavior controls
	*/
	float											MaxFuselagePitch;				//	DEG_TO_RADF(15.0f);
	float											MaxFuselageRoll;				// DEG_TO_RADF(20.0f);
	float											PitchControllerGain;			// 45.5
	float											PitchControllerDamping;		// 12.75
	float											RollControllerGain;			// 45.5
	float											RollControllerDamping;		// 12.75

	float											MaxYawVelocity;				// DEG_TO_RADF(180.0f);
	float											YawControllerGain;			// 5.0f;

	/*
	** Engine Graphical Behavior
	*/
	float											MaxEngineRotation;			

	/*
	** Rotor Graphical Behavior
	*/
	float											RotorSpeed;		
	float											RotorAcceleration;
	float											RotorDeceleration;

	friend class VTOLVehicleClass;
};


#endif

