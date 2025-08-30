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
 *                     $Archive:: /Commando/Code/wwphys/vehiclephys.h                         $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/14/01 2:09p                                              $*
 *                                                                                             *
 *                    $Revision:: 16                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef VEHICLEPHYS_H
#define VEHICLEPHYS_H

#include "rbody.h"
#include "simplevec.h"

class VehiclePhysDefClass;
class SuspensionElementClass;
class VehicleDazzleClass;


/**
** VehiclePhysClass
** This RigidBody-derived class adds the constants needed to define the suspension behavior
** for all vehicles.  Most of the actual force calculations are implemented in further 
** derived classes but this is the common base class for various classes (e.g. WheeledVehicle,
** TrackedVehicle, and VTOLPhys
*/
class VehiclePhysClass : public RigidBodyClass
{
public:

	VehiclePhysClass(void);
	virtual ~VehiclePhysClass(void);

	virtual VehiclePhysClass *		As_VehiclePhysClass(void) override		{ return this; }
	const VehiclePhysDefClass *	Get_VehiclePhysDef(void)		{ return (VehiclePhysDefClass*)Definition; }

	void									Init(const VehiclePhysDefClass & def);

	/*
	** DEBUGGING, re-initialize this object because our definition changed
	*/
	virtual void						Definition_Changed(void) override;

	/*
	** Rendering
	*/
	virtual void						Render(RenderInfoClass & rinfo) override;
	virtual void						Set_Model(RenderObjClass * model) override;
	float									Compute_Approximate_Ride_Height(void);

	/*
	** Simulation
	*/
	virtual void						Timestep(float dt) override;

	/*
	** Gentlemen Start your Engines! 
	*/
	void									Enable_Engine(bool onoff)		{ IsEngineOn = onoff; }
	bool									Is_Engine_Enabled(void)			{ return IsEngineOn; }
	
	/*
	** Wheel management
	*/
	int									Get_Wheel_Count(void)			{ return Wheels.Length(); }
	int									Get_Drive_Wheel_Count(void)	{ return DriveWheelCount; }
	int									Get_Real_Wheel_Count(void)		{ return RealWheelCount; }
	SuspensionElementClass *		Peek_Wheel(int wheel_index);
	bool									Is_In_Contact(void);
	void									Update_Wheels (void);

	/*
	** Non_Physical_Wheel_Update - used by Renegade's "encyclopedia" to graphically
	** update the wheels when there is no physics scene or processing happening.
	*/
	void									Non_Physical_Wheel_Update(float suspension_fraction,float rotation);

	/*
	** Teleport to the last good position.  This is used when vehicles get flipped over or stuck somehow.
	*/
	void									Teleport_To_Last_Good_Position(void);

	/*
	** Simulation and rendering toggles for all vehicle physics objects
	*/
	virtual bool						Is_Simulation_Disabled(void) override				{ return _DisableVehicleSimulation || RigidBodyClass::Is_Simulation_Disabled(); }
	virtual bool						Is_Rendering_Disabled(void) override				{ return _DisableVehicleRendering || RigidBodyClass::Is_Simulation_Disabled(); }
	static void							Disable_All_Simulation(bool onoff)		{ _DisableVehicleSimulation = onoff; }
	static void							Disable_All_Rendering(bool onoff)		{ _DisableVehicleRendering = onoff; }
	static bool							Is_All_Simulation_Disabled(void)			{ return _DisableVehicleSimulation; }
	static bool							Is_All_Rendering_Disabled(void)			{ return _DisableVehicleRendering; }

	/*
	** Save-Load system
	*/
	virtual bool						Save(ChunkSaveClass &csave) override;
	virtual bool						Load(ChunkLoadClass &cload) override;

protected:
	
	/*
	** Simulation
	*/
	virtual void						Compute_Force_And_Torque(Vector3 * force,Vector3 * torque) override;
	virtual bool						Can_Go_To_Sleep(float dt) override;

	/*
	** Suspension initialization
	*/
	void									Update_Cached_Model_Parameters(void);
	void									Release_Wheels(void);
	void									Create_Wheels(void);
	int									Find_Fork_Bone(RenderObjClass * model,const char * wpname);
	int									Find_Rotation_Bone(RenderObjClass * model,const char * wpname);
	int									Find_Translation_Bone(RenderObjClass * model,const char * wpname);

	/*
	** Auxilary bones
	*/
	void									Release_Auxiliary_Bones(void);
	void									Capture_Auxiliary_Bones(void);

	/*
	** Dazzle controllers
	*/	
	void									Release_Dazzles(void);
	void									Capture_Dazzles(void);
	VehicleDazzleClass *				Create_Dazzle_Controller(RenderObjClass * obj);

	/*
	** Virtual Wheel Creation, Derived classes implement this to create 
	** the types of wheels that they need to use.  (NOTE: wheels should be 'new'ed
	** since VehiclePhysClass will 'delete' them when done.)
	*/
	virtual SuspensionElementClass *	Alloc_Suspension_Element(void) = 0;

	/*
	** Implement this to support translating engine flames
	*/
	virtual float						Get_Normalized_Engine_Flame(void) { return 0.0f; }


	/*
	** Variables
	*/
	bool									IsEngineOn;				// is the engine on? vehicles will stop if the engine is off

	int									RealWheelCount;		// active wheel count
	int									DriveWheelCount;		// drive wheel count
	SimpleVecClass<SuspensionElementClass *>	Wheels;	// array of wheel objects
	SimpleVecClass<VehicleDazzleClass *>		Dazzles;	// array of dazzle controllers (for headlights, taillights, blinky lights, etc)	
	SimpleVecClass<int>				EngineFlameBones;		// indices to "engine-flame" bones
	Matrix3D								LastGoodPosition;		// last state where all wheels were on the ground
	float									ExpireTimer;			// time left before blowing up when rolled over	

	static bool							_DisableVehicleSimulation;
	static bool							_DisableVehicleRendering;

};


/**
** VehiclePhysDefClass
** This definition contains a set of constants used by all types of vehicles.  
*/
class VehiclePhysDefClass : public RigidBodyDefClass
{
public:

	VehiclePhysDefClass(void);
	virtual ~VehiclePhysDefClass(void);
	
	// From DefinitionClass
	virtual uint32								Get_Class_ID (void) const override;

	// From PhysDefClass
	virtual const char *						Get_Type_Name(void) override						{ return "VehiclePhysDef"; }
	virtual bool								Is_Type(const char *) override;

	// Read access to the constants
	float											Get_Spring_Constant(void) const		{ return SpringConstant; }
	float											Get_Damping_Constant(void) const		{ return DampingConstant; }
	float											Get_Spring_Length(void) const			{ return SpringLength; }
	float											Get_Traction_Multiplier(void) const	{ return TractionMultiplier; }
	float											Get_Lateral_Moment_Arm(void) const	{ return LateralMomentArm; }
	float											Get_Tractive_Moment_Arm(void) const { return TractiveMomentArm; }
	bool											Is_Fake(void) const						{ return IsFake; }

	// Write access (DEBUGGING/TESTING ONLY)
	void											Set_Spring_Constant(float ks)			{ SpringConstant = ks; }
	void											Set_Damping_Constant(float kd)		{ DampingConstant = kd; }
	void											Set_Spring_Length(float l)				{ SpringLength = l; }
	void											Set_Traction_Multiplier(float k)		{ TractionMultiplier = k; }
	void											Set_Lateral_Moment_Arm(float r)		{ LateralMomentArm = r; }
	void											Set_Tractive_Moment_Arm(float r)		{ TractiveMomentArm = r; }


	// Save/Load support from PersistClass
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save(ChunkSaveClass &csave) override;
	virtual bool								Load(ChunkLoadClass &cload) override;

	//	Editable interface requirements
	DECLARE_EDITABLE(VehiclePhysDefClass,RigidBodyDefClass);

protected:

	float											SpringConstant;			// suspension spring constant
	float											DampingConstant;			// suspension damping constant (shock absorber)
	float											SpringLength;				// suspension spring length
	float											TractionMultiplier;		// scales the downforce for more traction
	float											LateralMomentArm;			// arbitrary dist from x-y plane to apply lateral tire forces
	float											TractiveMomentArm;		// arbitrary dist from x-y plane to apply tractive tire forces
	float											EngineFlameLength;		// max length of the engine flame (how far to translate at flame=1.0)
	bool											IsFake;						// short-circuit all possible physics calculations!	

	friend class VehiclePhysClass;
};



#endif

