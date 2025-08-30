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
 *                     $Archive:: /Commando/Code/wwphys/wheel.h                               $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/19/01 3:47p                                              $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef WHEEL_H
#define WHEEL_H

#include "vector3.h"
#include "matrix3d.h"

class VehiclePhysClass;
class RenderObjClass;


/*
** Some default suspension constants
*/
const float		DEFAULT_SPRING_CONSTANT					= 3.0f;
const float		DEFAULT_DAMPING_COEFFICIENT			= 0.75f;
const float		DEFAULT_SPRING_LENGTH					= 1.0f;

/**
** Wheels.  An instance of a wheel object will contain information on each
** wheel detected in the vehicle model.  Wheels use a pair of named bones.
** The "position" bone is used to move the wheel up and down according to 
** the suspension system and to rotate if this is a "steering" wheel.  The
** "rotation" bone is used to roll the wheel on the terrain.
**
** More Info on the "Position" and "Center" bones:
** - The "center" bone is only used in computing the radius of the wheel and
**   it is rotated about its z-axis to make the wheel appear to roll
** - The "position" bone is used in conjunction with the length parameter
**   to define the suspension springs.
** 
** Defining the "spring-segment" using the position bone:
** - The default location of the position bone is considered the point of maximum-
**   compression.  (this point should be *inside* the collision box for the model!) 
** - The spring will extend down the -z axis in the position bone's coordinate system
**
** Graphical constraints for the wheel
** - One of three methods can be used to graphically constrain the wheel with the
**   ground: Translation along the z-axis of the position bone, or rotation of
**   a rotation constraint bone.
** - Normal case: There are two bones for the wheel: position and center.  The position
**   bone is moved along its Z-axis to the point of collision with the ground
** - Translation: There is an additional translation bone (WheelTxx) which defines 
**   the axis that the wheel is to be translated along.
** - Fork/Rotation: a "fork" bone (WheelFxx) is rotated such that the Z-coordinate (in 
**   the fork's coordinate system) meets the ground.
** 
** Wheel Flags:
** - 'E' Engine. this wheel is connected to the engine and should exert its force 
** - 'S' Steering: The position bone for this wheel rotates about its Z-axis for steering
** - 's' Inverse Steering: The position bone for this wheel rotates the opposite way
** - 'L' Left Track: this wheel is part of the left track of a tracked vehicle
** - 'R' Right Track: this wheel is part of the right track of a tracked vehicle
** - 'F' Fake: just move, don't compute any forces
*/


/**
** SuspensionElementClass
** This class only has the functionality of a support strut with a contact point.  Derived
** wheel classes add in the forces exerted at the contact patch.
*/

class SuspensionElementClass
{
public:

	enum FlagsType
	{
		FAKE						= 0x0001,			// this wheel is for looks only (e.g. some of the tank wheels)
		STEERING					= 0x0002,			// this wheel turns with the steering input
		INV_STEERING			= 0x0004,			// this wheel turns opposite of the steering input
		TILT_STEERING			= 0x0008,			// this wheel turns with tilt abount the X axis of the vehicle (bikes).

		ENGINE					= 0x0010,			// this wheel exerts the engine force
		LEFT_TRACK				= 0x0020,			// this wheel is part of the left track of a tank
		RIGHT_TRACK				= 0x0040,			// this wheel is part of the right track of a tank
		
		DISABLED					= 0x0100,			// this wheel is disabled
		INCONTACT				= 0x0200,			// this wheel is in contact with the ground
		BRAKING					= 0x0400,			// this wheel is undergoing braking
		DEFAULT_FLAGS			= 0
	};

	SuspensionElementClass(void);
	virtual ~SuspensionElementClass(void);
	virtual void	Init(VehiclePhysClass * obj,int pbone,int rbone,int forkbone,int axisbone);

	/*
	** Accessors to properties
	*/
	bool				Get_Flag(FlagsType flag)					{ return ((Flags & flag) == flag); }
	void				Set_Flag(FlagsType flag,bool onoff)		{ (onoff ? Flags |= flag : Flags &= ~flag); }
	float				Get_Spring_Constant(void)					{ return SpringConstant; }
	void				Set_Spring_Constant(float k)				{ SpringConstant = k; }
	float				Get_Damping_Coefficient(void)				{ return DampingCoefficient; }
	void				Set_Damping_Coefficient(float k)			{ DampingCoefficient = k; }
	float				Get_Spring_Length(void)						{ return SpringLength; }
	void				Set_Spring_Length(float len)				{ SpringLength = len; }

	/*
	** Inputs from the parent object 
	*/
	float				Get_Steering_Angle(void)					{ return SteeringAngle; }
	void				Set_Steering_Angle(float angle)			{ SteeringAngle = angle; }

	/*
	** Accessors to state variables
	*/
	void				Get_Wheel_Position(Vector3 * set_pos)	{ WheelTM.Get_Translation(set_pos); }
	int				Get_Contact_Surface(void) const			{ return ContactSurface; }
	const Vector3 &Get_Contact_Point(void) const				{ return Contact; }
	const Vector3 &Get_Contact_Normal(void) const			{ return Normal; }
	const Matrix3D&Get_Wheel_Transform(void) const			{ return WheelTM; }
	virtual float	Get_Slip_Factor(void) const				{ return 1.0f; }
	virtual float	Get_Rotation_Delta(void) const			{ return 0.0f; }

	/*
	** Physics processing
	*/
	virtual void	Compute_Force_And_Torque(Vector3 * force,Vector3 * torque)		= 0;
	virtual void	Update_Model(void);

	virtual void	Non_Physical_Update(float suspension_fraction,float rotation);

protected:

	/*
	** Internal functions
	*/
	void				Intersect_Spring(void);
	void				Non_Physical_Intersect_Spring(float suspension_fraction);
	void				Translate_Wheel(RenderObjClass * model);
	void				Translate_Wheel_On_Axis(RenderObjClass * model);
	void				Rotate_Fork(RenderObjClass * model);


	/*
	** Properties
	*/
	VehiclePhysClass * Parent;					// Vehicle I am attached to.
	int				Flags;						// bit-field of flags used by this and derived classes
	int				PositionBone;				// Bone index of the position bone (wheel coordinate system)
	int				ForkBone;					// Bone index for the "fork" constraint (if not -1, we use fork constraints)
	int				AxisBone;					// Bone index for the translation axis bone (if not -1, we translate on its z-axis instead)
	
	Matrix3D			ObjWheelTM;					// Wheel coordinate system in object-space

	float				SpringConstant;			// spring
	float				DampingCoefficient;		// shock absorber
	float				SpringLength;				// unstretched/compressed length

	/*
	** Inputs
	*/
	float				SteeringAngle;				// current steering angle

	/*
	** State
	*/
	Vector3			SuspensionForce;			// current suspension force
	Matrix3D			WheelTM;						// current wheel coordinate system (positioned at contact point)
	Vector3			WheelP0;						// current position of the top of the spring
	Vector3			Contact;						// current contact point (only valid if INCONTACT is true)
	Vector3			Normal;						// current contact normal (only valid if INCONTACT is true)
	int				ContactSurface;			// current contact surface (only valid if INCONTACT is true)

	/*
	** Visual State
	*/
	Vector3			LastPoint;					// last position of the wheel (for rolling)

	/*
	** Constraint variables
	*/
	float				TranslationScale;			
	Vector3			ObjAxis;						// axis used for wheels with an overridden translation axis
	Matrix3D			ObjForkTM;					// these are constants used for "fork" constrained wheels
	float				ForkLength;					// I'm calling the back wheel of a motorcycle a "fork" this
	float				ForkZ;						// is not the best name... Its more like an "arm"
	float				ForkSin0;					// These constants are all used in the equation to determine
	float				ForkCos0;					// how far to rotate the "arm" or "fork" in order for the
	float				ForkA;						// wheel to touch the ground.
	float				ForkB;

	/*
	** The latest known location of the spring end points - this is used to determine if the spring has moved
	*/
	Vector3			SpringEndP1;
	Vector3			SpringEndP0;
private:
	
	// not implemented
	SuspensionElementClass(const SuspensionElementClass & that);
	SuspensionElementClass & operator = (const SuspensionElementClass & that);

};

/**
** WheelClass
** This class adds code which is common to all rolling wheels.  It adds a wheel radius
** and wheel rotation variables.
*/
class WheelClass : public SuspensionElementClass
{
public:
	WheelClass(void);
	virtual ~WheelClass(void);

	virtual void	Init(VehiclePhysClass * obj,int postion_bone,int rotation_bone=-1,int fork_bone=-1,int axis_bone=-1) override;
	virtual void	Compute_Force_And_Torque(Vector3 * force,Vector3 * torque) override;

	/*
	** Accessors
	*/
	bool				Is_Drive_Wheel(void)								{ return (Get_Flag(ENGINE)||Get_Flag(LEFT_TRACK)||Get_Flag(RIGHT_TRACK)); }
	float				Get_Slip_Factor(void) const override					{ return SlipFactor; }
	float				Get_Rotation_Delta(void) const override				{ return RotationDelta; }
	float				Get_Ideal_Drive_Wheel_Angular_Velocity(float max_avel);
	float				Get_Radius(void) const							{ return Radius; }

	/*
	** Inputs
	*/
	float				Get_Axle_Torque(void)						{ return AxleTorque; }
	void				Set_Axle_Torque(float torque)				{ AxleTorque = torque; }

protected:

	/*
	** Component functions for Compute_Force_And_Torque
	*/
	void				Compute_Suspension_Force(const Vector3 & pdot,const Vector3 & local_pdot,Vector3 * suspension_force);
	void				Apply_Forces(Vector3 * force,Vector3 * torque);
	
	/*
	** Derived wheel classes implement this to compute the traction forces
	*/
	virtual void	Compute_Traction_Forces(const Vector3 & local_pdot,float normal_force,float * set_lateral_force,float * set_tractive_force) = 0;

	/*
	** Properties 
	*/
	float				Radius;						// radius of the wheel (for rolling)
	int				RotationBone;				// bone index of the rotation bone
	float				Rotation;					// amount the wheel has rolled.
	float				RotationDelta;				// so we can keep the wheel rolling in the air

	/*
	** Inputs
	*/
	float				AxleTorque;					// engine torque applied to the axle

	/*
	** State
	*/
	Vector3			TractiveFrictionForce;	// current tractive friction force
	Vector3			LateralFrictionForce;	// current lateral friction force
	float				SlipFactor;					// fraction of force "clipped" off to traction circle
	float				IdealAngularVelocity;	// non-slipping angular velocity (IF IN CONTACT)

private:

	//not implemented
	WheelClass(const WheelClass & that);
	const WheelClass & operator = (const WheelClass & that);

};


/**
** WVWheelClass (Wheeled-Vehicle-Wheel)
** This wheel class is used by things like Humvees and Buggys.   
*/
class WVWheelClass : public WheelClass
{
public:
	WVWheelClass(void)						{ }
	virtual ~WVWheelClass(void)			{ }

	virtual void	Update_Model(void) override;
	virtual void	Non_Physical_Update(float suspension_fraction,float rotation) override;

protected:

	virtual void	Compute_Traction_Forces(const Vector3 & local_pdot,float normal_force,float * set_lateral_force,float * set_tractive_force) override;
	void				Roll_Wheel(void);
};


/**
** TrackWheelClass
** This wheel type is used by TrackedVehicleClass.  The differences between
** it and the WVWheelClass are minor, mainly in the logic for how the wheels
** roll and possibly the lack of a traction circle.  
*/
class TrackWheelClass : public WheelClass
{
public:

	TrackWheelClass(void)						{ }
	virtual ~TrackWheelClass(void)			{ }

	virtual void	Update_Model(void) override;
	virtual void	Non_Physical_Update(float suspension_fraction,float rotation) override;

protected:

	virtual void	Compute_Traction_Forces(const Vector3 & local_pdot,float normal_force,float * set_lateral_force,float * set_tractive_force) override;
	void				Roll_Wheel(void);

};


/**
** VTOLWheelClass
** This wheel type is used by the VTOL vehicles.  It simply resists movement in
** all directions and rolls depending on its motion (if it has a WheelC bone...)
*/
class VTOLWheelClass : public WheelClass
{
public:
	VTOLWheelClass(void)						{ }
	virtual ~VTOLWheelClass(void)			{ }

	virtual void	Update_Model(void) override;
	virtual void	Non_Physical_Update(float suspension_fraction,float rotation) override;

protected:

	virtual void	Compute_Traction_Forces(const Vector3 & local_pdot,float normal_force,float * set_lateral_force,float * set_tractive_force) override;
	void				Roll_Wheel(void);

};



#endif //WHEEL_H


