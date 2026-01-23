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
 *                     $Archive:: /Commando/Code/wwphys/wheel.cpp                             $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/19/02 2:32p                                               $*
 *                                                                                             *
 *                    $Revision:: 27                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   SuspensionElementClass::SuspensionElementClass -- Constructor                             *
 *   SuspensionElementClass::~SuspensionElementClass -- Destructor                             *
 *   SuspensionElementClass::Init -- Initialization, pass in bone indices for this wheel       *
 *   SuspensionElementClass::Intersect_Spring -- Intersect the suspension spring with the terr *
 *   SuspensionElementClass::Update_Model -- Update the wheel transforms in the model.         *
 *   SuspensionElementClass::Translate_Wheel_On_Axis -- Translates the wheel to the contact po *
 *   SuspensionElementClass::Translate_Wheel -- Translates the wheel to the contact point      *
 *   SuspensionElementClass::Rotate_Fork -- Rotates the "fork" until the wheel touches         *
 *   WheelClass::WheelClass -- Constructor                                                     *
 *   WheelClass::~WheelClass -- Destructor                                                     *
 *   WheelClass::Init -- Initialize the wheel object                                           *
 *   WheelClass::Compute_Force_And_Torque -- Computes wheel forces                             *
 *   WheelClass::Compute_Suspension_Force -- Computes the suspension force                     *
 *   WheelClass::Get_Ideal_Drive_Wheel_Angular_Velocity -- Computes the current angular veloci *
 *   WheelClass::Apply_Forces -- Computes the final forces and adds them                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "wheel.h"
#include "wheelvehicle.h"
#include "trackedvehicle.h"
#include "lineseg.h"
#include "physcoltest.h"
#include "physcon.h"
#include "pscene.h"
#include "wwprofile.h"


// Wheel friction is a proportional controller, this is the constant.
// TODO: Should we make this an editable parameter of VehiclePhysDefClass?
const float		WHEEL_FRICTION_PENALTY_CONSTANT			= 1.0f / 7.5f;
const float		BIKE_WHEEL_FRICTION_PENALTY_CONSTANT	= 1.0f / 3.0f;
const float		TRACKWHEEL_FRICTION_PENALTY_CONSTANT	= 1.0f / 5.0f;
const float		VTOLWHEEL_FRICTION_PENALTY_CONSTANT		= 1.0f / 3.0f;

// Debugging
const Vector3	SUSPENSION_FORCE_COLOR					= Vector3(0,0,1);
const Vector3	LATERAL_FORCE_COLOR						= Vector3(0,1,0);
const Vector3	TRACTIVE_FORCE_COLOR						= Vector3(1,0,0);

// Maximum total acceleration the suspension will exert on the parent vehicle (in m/s^2)
// Note, each "real" wheel gets an equal fraction of this, so if you have 4 wheels, the max 
// acceleration any single wheel will exert is one quarter of the total.
const float		MAX_SUSPENSION_ACCEL = 50.0f;



/*
** Wheels:  An instance of this object will contain information on each
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
** - 'L' Left Track: this wheel is part of the left track of a tracked vehicle
** - 'R' Right Track: this wheel is part of the right track of a tracked vehicle
*/

/*********************************************************************************************************

	SuspensionElementClass Implementation

*********************************************************************************************************/


/***********************************************************************************************
 * SuspensionElementClass::SuspensionElementClass -- Constructor                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
SuspensionElementClass::SuspensionElementClass(void):
	Parent(NULL),
	Flags(DEFAULT_FLAGS),
	PositionBone(-1),
	AxisBone(-1),
	ForkBone(-1),
	ObjWheelTM(1),
	SpringConstant(DEFAULT_SPRING_CONSTANT),
	DampingCoefficient(DEFAULT_DAMPING_COEFFICIENT),
	SpringLength(DEFAULT_SPRING_LENGTH),
	SteeringAngle(0.0f),
	SuspensionForce(0,0,0),
	WheelTM(1),
	WheelP0(0,0,0),
	Contact(0,0,0),
	Normal(0,0,0),
	ContactSurface(0),
	LastPoint(0,0,0),
	TranslationScale(1.0f),
	ObjAxis(0,0,-1),
	ObjForkTM(1),
	ForkLength(0.0f),
	ForkZ(0.0f),
	ForkSin0(0.0f),
	ForkCos0(0.0f),
	ForkA(0.0f),
	ForkB(0.0f),
	SpringEndP0(0.0f,0.0f,0.0f),
	SpringEndP1(0.0f,0.0f,0.0f)
{
}


/***********************************************************************************************
 * SuspensionElementClass::~SuspensionElementClass -- Destructor                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/18/2000 gth : Created.                                                                 *
 *=============================================================================================*/
SuspensionElementClass::~SuspensionElementClass(void)
{
	if (Parent != NULL) {
		RenderObjClass * model = Parent->Peek_Model();
		if (model != NULL) {
			if (PositionBone != -1) {
				model->Release_Bone(PositionBone);
			}
			if (ForkBone != -1) {
				model->Release_Bone(ForkBone);
			}
			if (AxisBone != -1) {
				model->Release_Bone(AxisBone);
			}
		}
	}
}


/***********************************************************************************************
 * SuspensionElementClass::Init -- Initialization, pass in bone indices for this wheel         *
 *                                                                                             *
 * INPUT:                                                                                      *
 * obj - parent vehicle physics object                                                         *
 * pbone - index of the position bone (position bone is the contact point for the wheel)       *
 * rbone - index of the center bone (center of rotation)                                       *
 * fbone - index of the fork bone for a "fork" constraint (bad name, this is the back wheel of *
 *         a motorcycle)                                                                       *
 * abone - index of the bone for an axis constraint.  (like the front wheel of a motorcycle)   *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/18/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void SuspensionElementClass::Init(VehiclePhysClass * obj,int pbone,int /* rbone */,int fbone,int abone)
{
	WWASSERT(obj != NULL);							// have to have a valid object
	WWASSERT(pbone != -1);							// have to have a position bone at least
	WWASSERT(obj->Peek_Model() != NULL);		// have to have a valid model

	// Store a pointer to the vehicle.  Not referenced because this class meant to be a private
	// member of wheeled vehicle.  A reference here would be circular.
	Parent = obj;

	// Grab a pointer to the render model
	RenderObjClass * model = obj->Peek_Model();

	// Store the bone indices and capture the bones
	PositionBone = pbone;
	ForkBone = fbone;
	AxisBone = abone;

	WWASSERT(PositionBone != -1);
	model->Capture_Bone(PositionBone);

	if (ForkBone != -1) {
		model->Capture_Bone(ForkBone);
	}

	if (AxisBone != -1) {
		model->Capture_Bone(AxisBone);
	}

	// Compute the object-space transform of the wheel
	Matrix3D position_bone,rootinv;
	const Matrix3D & root = model->Get_Transform();
	root.Get_Orthogonal_Inverse(rootinv);
	position_bone = model->Get_Bone_Transform(PositionBone);

	Matrix3D::Multiply(rootinv,position_bone,&ObjWheelTM);
	WheelTM = position_bone;
	WheelTM.Get_Translation(&WheelP0);

	// Initialize the last position of the wheel
	LastPoint = position_bone.Get_Translation();

	// If a fork is constraint being used, compute the constants for it
	if (ForkBone != -1) {
		Matrix3D::Multiply(rootinv,model->Get_Bone_Transform(ForkBone),&ObjForkTM);
		Vector3 r = ObjWheelTM.Get_Translation() - ObjForkTM.Get_Translation();
		ForkLength = r.Length();

		Vector3 pos;
		ObjWheelTM.Get_Translation(&pos);
		Matrix3D::Inverse_Transform_Vector(ObjForkTM,pos,&pos);

		WWASSERT(WWMath::Fabs(ForkLength - pos.Length()) < 0.1f);

		ForkZ = pos.Z;
		ForkSin0 = pos.Z / ForkLength;
		ForkCos0 = pos.X / ForkLength;
		ForkA = ForkCos0 / ForkSin0;
		ForkB = -(ForkCos0 * ForkCos0) / ForkSin0 - ForkSin0;

	}

	// If a translation axis constraint is being used, compute the axis
	if (AxisBone != -1) {
		Matrix3D axistm;
		Matrix3D::Multiply(rootinv,model->Get_Bone_Transform(AxisBone),&axistm);
		axistm.Get_Z_Vector(&ObjAxis);

		Vector3 springdir;
		position_bone.Get_Z_Vector(&springdir);
		TranslationScale = 1.0f / Vector3::Dot_Product(ObjAxis,springdir);
	}
}


/***********************************************************************************************
 * SuspensionElementClass::Intersect_Spring -- Intersect the suspension spring with the terrai *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/18/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void SuspensionElementClass::Intersect_Spring(void)
{
	WWASSERT(Parent != NULL);

	// transform the wheel coordinate system into world space.
	Matrix3D::Multiply(Parent->Get_Transform(),ObjWheelTM,&WheelTM);

	// compute WheelP0 and temporary p1 (endpoints of the spring)
	Vector3 p1;
	WheelTM.Get_Translation(&WheelP0);
	Matrix3D::Transform_Vector(WheelTM,Vector3(0,0,-SpringLength),&p1);

	// If the spring endpoints haven't changed don't do raycast (very expensive)
	// TODO: Ensure we are not on top of an animated mesh!
	if (p1==SpringEndP1 && WheelP0==SpringEndP0) return;
	SpringEndP1=p1;
	SpringEndP0=WheelP0;

	// cast a ray, colliding with "physical" objects
	LineSegClass line(WheelP0,p1);
	CastResultStruct result;
	PhysRayCollisionTestClass raytest(line,&result,Parent->Get_Collision_Group(),COLLISION_TYPE_PHYSICAL | COLLISION_TYPE_VEHICLE);
	PhysicsSceneClass::Get_Instance()->Cast_Ray(raytest,false);

	// evaluate the result of the raycast
	if (result.Fraction >= 1.0f) {
		Set_Flag(INCONTACT,false);
		WheelTM.Set_Translation(p1);
		Contact = p1;
	} else {
		Set_Flag(INCONTACT,true);
		line.Compute_Point(result.Fraction,&Contact);
		WheelTM.Set_Translation(Contact);
		Normal = result.Normal;
		ContactSurface = result.SurfaceType;

		// TODO: Low-level collision routines should handle this?
		if (Vector3::Dot_Product(Normal,line.Get_Dir()) > 0.0f) {
			Normal = -Normal;
		}
	}
}

/***********************************************************************************************
 * SuspensionElementClass::Non_Physical_Intersect_Spring -- used in ecyclopedia viewer...      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
void SuspensionElementClass::Non_Physical_Intersect_Spring(float suspension_fraction)
{
	WWASSERT(Parent != NULL);

	// transform the wheel coordinate system into world space.
	Matrix3D::Multiply(Parent->Get_Transform(),ObjWheelTM,&WheelTM);

	// compute WheelP0 and temporary p1 (endpoints of the spring)
	Vector3 p1;
	WheelTM.Get_Translation(&WheelP0);
	Matrix3D::Transform_Vector(WheelTM,Vector3(0,0,-SpringLength),&p1);

	// Form a ray to compute the contact point.
	Set_Flag(INCONTACT,true);
	LineSegClass line(WheelP0,p1);
	line.Compute_Point(suspension_fraction,&Contact);
	WheelTM.Set_Translation(Contact);
	Normal.Set(0.0f,0.0f,1.0f);
	ContactSurface = 0;
}


/***********************************************************************************************
 * SuspensionElementClass::Update_Model -- Update the wheel transforms in the model.           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/18/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void SuspensionElementClass::Update_Model(void)
{
	RenderObjClass * model = Parent->Peek_Model();
	if (model == NULL) return;

	// update the contact point.
	Intersect_Spring();
	
	// use the constraints to make the wheel touch the ground
	if (ForkBone != -1) {
		Rotate_Fork(model);
	} else if (AxisBone != -1) {
		Translate_Wheel_On_Axis(model);
	} else {
		Translate_Wheel(model);
	}
}


/***********************************************************************************************
 * SuspensionElementClass::Non_Physical_Update -- Update the wheel transforms in the model.           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/18/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void SuspensionElementClass::Non_Physical_Update(float suspension_fraction,float /* rotation */)
{
	RenderObjClass * model = Parent->Peek_Model();
	if (model == NULL) return;

	// Set the wheel up as if it had an intersection at 'fraction' of its length
	Non_Physical_Intersect_Spring(suspension_fraction);

	// Update the "constraints" 
	if (ForkBone != -1) {
		Rotate_Fork(model);
	} else if (AxisBone != -1) {
		Translate_Wheel_On_Axis(model);
	} else {
		Translate_Wheel(model);
	}
}



/***********************************************************************************************
 * SuspensionElementClass::Translate_Wheel_On_Axis -- Translates the wheel to the contact poin *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/18/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void SuspensionElementClass::Translate_Wheel_On_Axis(RenderObjClass * model)
{
	// This function handles moving the wheel down to touch the ground
	// when there is a translation bone.  We just translate down its -z axis
	Matrix3D position_tm(1);
	position_tm.Rotate_Z(SteeringAngle);

	if (Get_Flag(INCONTACT)) {
		Vector3 dp;
		Vector3 springdir;
		Vector3::Subtract(Contact,WheelP0,&dp);	
		WheelTM.Get_Z_Vector(&springdir);
		position_tm.Adjust_Z_Translation(TranslationScale * Vector3::Dot_Product(dp,springdir));
	} else {
		position_tm.Adjust_Z_Translation(TranslationScale * -SpringLength);
	}

	model->Control_Bone(AxisBone,position_tm);
		
}


/***********************************************************************************************
 * SuspensionElementClass::Translate_Wheel -- Translates the wheel to the contact point        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/18/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void SuspensionElementClass::Translate_Wheel(RenderObjClass * model)
{
	// This function handles moving the wheel down to touch the ground
	// when there is no "fork" present.  We just translate down the -z axis
	Matrix3D position_tm(1);
	position_tm.Rotate_Z(SteeringAngle);

	if (Get_Flag(INCONTACT)) {
		Vector3 dp;
		Vector3 springdir;
		Vector3::Subtract(Contact,WheelP0,&dp);	
		WheelTM.Get_Z_Vector(&springdir);
		position_tm.Adjust_Z_Translation(Vector3::Dot_Product(dp,springdir));
	} else {
		position_tm.Adjust_Z_Translation(-SpringLength);
	}

	model->Control_Bone(PositionBone,position_tm);
}


/***********************************************************************************************
 * SuspensionElementClass::Rotate_Fork -- Rotates the "fork" until the wheel touches           *
 *                                                                                             *
 *    Again, "fork" was the wrong name for this.  This is a "constraint" like the back wheel   *
 *    of a motorcycle.  The "fork" bone will be rotated about its Z axis until the WheelP      *
 *    bone is at the Z-position of the contact point.                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/18/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void SuspensionElementClass::Rotate_Fork(RenderObjClass * model)
{
	WWASSERT(ForkBone != -1);
	
	float fork_sin = 0.0f;
	float fork_cos = 1.0f;

	// transform the contact point into the fork coordinate system
	// - first, transform into object space
	// - then transform into "fork" space
	Vector3 p1;
	Matrix3D::Inverse_Transform_Vector(Parent->Get_Transform(),Contact,&p1);
	Matrix3D::Inverse_Transform_Vector(ObjForkTM,p1,&p1);

	// Only concerned with the x and z coordinates now.  And x is a function of z
	float x2 = ForkLength*ForkLength - p1.Z * p1.Z;
	if (x2 < 0.0f) {
		return;
	}
	p1.X = WWMath::Sqrt(x2);	

	// Compute the sine and cosine of the new rotation (without calling atan,sine or cosine!)
	float ooforklen = 1.0f / ForkLength;
	fork_sin = -(p1.X * ooforklen - ForkA * p1.Z * ooforklen) / ForkB;
	fork_cos = (p1.Z * ooforklen) / ForkSin0 + ForkA * fork_sin;

#if 0
	float angle1 = atan2(p1.Z,p1.X);
	float angledelta = angle1 - atan2(ForkSin0,ForkCos0);
	fork_sin = -WWMath::Sin(angledelta);
	fork_cos = WWMath::Cos(angledelta);
#endif
	
	Matrix3D fork_rotation(1);
	fork_rotation.Rotate_Y(fork_sin,fork_cos);
	model->Control_Bone(ForkBone,fork_rotation);
}


/*********************************************************************************************************

	WheelClass Implementation

*********************************************************************************************************/


/***********************************************************************************************
 * WheelClass::WheelClass -- Constructor                                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
WheelClass::WheelClass(void) :
	Radius(1.0f),
	RotationBone(-1),
	Rotation(0.0f),
	RotationDelta(0.0f),
	AxleTorque(0.0f),
	TractiveFrictionForce(0,0,0),
	LateralFrictionForce(0,0,0),
	SlipFactor(1.0f),
	IdealAngularVelocity(0.0f)
{
}


/***********************************************************************************************
 * WheelClass::~WheelClass -- Destructor                                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/18/2000 gth : Created.                                                                 *
 *=============================================================================================*/
WheelClass::~WheelClass(void)
{
	if (Parent != NULL) {
		RenderObjClass * model = Parent->Peek_Model();
		if (model != NULL) {
			if (RotationBone != -1) {
				model->Release_Bone(RotationBone);
			}
		}
	}
}


/***********************************************************************************************
 * WheelClass::Init -- Initialize the wheel object                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 * obj - parent vehicle physics object                                                         *
 * position_bone - WHEELP, contact point, wheel position                                       *
 * rotation_bone - WHEELC, center of rotation for the wheel                                    *
 * fork_bone - bone for a constraint like the back wheel of a motorcycle (rotates about Z)     *
 * axis_bone - bone for a constraint like the front wheel of a motorcycle (translates along Z) *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/18/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void WheelClass::Init
(
	VehiclePhysClass * obj,
	int position_bone,
	int rotation_bone,
	int fork_bone,
	int axis_bone
)
{
	// Allow the base class to init
	SuspensionElementClass::Init(obj,position_bone,rotation_bone,fork_bone,axis_bone);
	
	// Grab a pointer to the render model
	RenderObjClass * model = obj->Peek_Model();
	WWASSERT(model != NULL);

	// Store the bone indices and capture the bones
	RotationBone = rotation_bone;

	if (RotationBone != -1) {
		model->Capture_Bone(RotationBone);
	}

	// Compute the radius of the wheel
	Matrix3D rotation_tm(1);
	if (RotationBone != -1) {
		rotation_tm = model->Get_Bone_Transform(RotationBone);
		Radius = (WheelP0 - rotation_tm.Get_Translation()).Length();
	} 
}



/***********************************************************************************************
 * WheelClass::Compute_Force_And_Torque -- Computes wheel forces                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/18/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void WheelClass::Compute_Force_And_Torque(Vector3 * force,Vector3 * torque)
{
	WWPROFILE("WheelClass::Compute_Force_And_Torque");

	if (Get_Flag(FAKE)) return;

	// Reset variables
	SlipFactor = 1.0f;
	SuspensionForce.Set(0,0,0);
	TractiveFrictionForce.Set(0,0,0);
	LateralFrictionForce.Set(0,0,0);
	IdealAngularVelocity = 0.0f;
	
	// Intersect the spring with the world to find our contact point
	// If it doesn't hit anything, we are done!
	{
		WWPROFILE("Intersect_Spring");
		Intersect_Spring();	
	}
	if (!Get_Flag(INCONTACT)) {
		return;
	}
	
	// Rotate the wheel coordinate system by the steering angle
	WheelTM.Rotate_Z(SteeringAngle);

	// Grab the 'Definition' for our parent for quick access to the constants
	const VehiclePhysDefClass * vehicle_def = Parent->Get_VehiclePhysDef();

	// Compute the velocity of the contact point both in world space and in
	// the wheel's coordinate system
	Vector3 pdot,local_pdot;
	Parent->Compute_Point_Velocity(Contact,&pdot);		
	local_pdot = pdot - Vector3::Dot_Product(pdot,Normal)*Normal;
	Matrix3D::Inverse_Rotate_Vector(WheelTM,local_pdot,&local_pdot);
	IdealAngularVelocity = local_pdot.X / Radius;
	
	// Suspension Force:
	Compute_Suspension_Force(pdot,local_pdot,&SuspensionForce);

	// Get the load on this wheel.  I'm not using the spring force since it is too 
	// sloppy; instead, I'll just give each wheel an equal share of the vehicle weight.
	// Also, compute the lateral components of gravitational acceleration so that
	// the wheel can attempt to cancel them.
	// TODO: load distribution?
	Vector3 gravity(0.0f,0.0f,-Parent->Get_Weight());
	gravity /= Parent->Get_Real_Wheel_Count();
	Vector3 local_gravity;
	Matrix3D::Inverse_Rotate_Vector(WheelTM,gravity,&local_gravity);
	
	float wheel_normal_force = -local_gravity.Z;
	wheel_normal_force *= vehicle_def->Get_Traction_Multiplier();
		
	// Friction / Traction Forces (initialized so that they cancel gravity):
	// (gth, 8/19/2001) scale the gravity cancellation forces down a bit so they don't cause
	// vehicles to slide uphill!
	const float GRAVITATION_CANCELLATION_FACTOR = 1.0f;
	float tractive_force = -local_gravity.X * GRAVITATION_CANCELLATION_FACTOR;
	float lateral_force = -local_gravity.Y * GRAVITATION_CANCELLATION_FACTOR;

// testing:
#if 0
Vector3 fs,ft,fn;
Matrix3D::Rotate_Vector(WheelTM,Vector3(0,0,-local_gravity.Z),&fs);
Matrix3D::Rotate_Vector(WheelTM,Vector3(tractive_force,0,0),&ft);
Matrix3D::Rotate_Vector(WheelTM,Vector3(0,lateral_force,0),&fn);
Vector3 sum = fs+ft+fn-gravity;

if (sum.Length2() < 0.001f) {
	int test=0;
}
#endif

	Compute_Traction_Forces(local_pdot,wheel_normal_force,&lateral_force,&tractive_force);

	// Compute the world-space traction forces
	Matrix3D::Rotate_Vector(WheelTM,Vector3(tractive_force,0,0),&TractiveFrictionForce);
	Matrix3D::Rotate_Vector(WheelTM,Vector3(0,lateral_force,0),&LateralFrictionForce);
	
	// Apply the final forces and torques
	Apply_Forces(force,torque);
}


/***********************************************************************************************
 * WheelClass::Compute_Suspension_Force -- Computes the suspension force                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/18/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void WheelClass::Compute_Suspension_Force(const Vector3 & pdot,const Vector3 & /* local_pdot */,Vector3 * suspension_force)
{
	WWPROFILE("WheelClass::Compute_Suspension_Force");

	// -----------------------------------------------------------------------------
	// Suspension Force:
	// - Compute the displacement of the wheel along the spring axis
	// - Compute the velocity of the wheel projected onto the spring axis
	// - Compute the spring/damper force
	// -----------------------------------------------------------------------------
	Vector3 spring_dir;												
	WheelTM.Get_Z_Vector(&spring_dir);								// spring_dir points from the wheel up to the body
	
	float dv = Vector3::Dot_Product(pdot,spring_dir);			
	float dx = Vector3::Dot_Product(WheelP0 - Contact,spring_dir) - SpringLength;

	float sforce = - SpringConstant*dx - DampingCoefficient*dv;

	// Clamp the suspension force to produce at most an acceleration of MAX_SUSPENSION_ACCEL
	float max_sforce = MAX_SUSPENSION_ACCEL * Parent->Get_Mass() / WWMath::Max(Parent->Get_Real_Wheel_Count(),1.0f);
	sforce = WWMath::Clamp(sforce,-max_sforce,max_sforce);
	
	*suspension_force = Normal;
	*suspension_force *= sforce;
}


/***********************************************************************************************
 * WheelClass::Apply_Forces -- Computes the final forces and adds them in.                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/18/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void WheelClass::Apply_Forces(Vector3 * force,Vector3 * torque)
{	
	WWPROFILE("WheelClass::Apply_Forces");

	// Grab the 'Definition' for our parent for quick access to the constants
	const VehiclePhysDefClass * vehicle_def = Parent->Get_VehiclePhysDef();

	// -----------------------------------------------------------------------------
	// Compute the final forces and torques
	// I'm applying the suspension force and lateral friction forces in the X-Y plane of 
	// the CM of the vehicle since we really don't like vehicles that tip over.  May even 
	// need to add a self-balancing force to keep the vehicle from *ever* fliping over...
	// -----------------------------------------------------------------------------
	Vector3 wheel_pos;
	Vector3 r_lateral;			// moment arm for lateral forces
	Vector3 r_tractive;			// moment arm for tractive forces
	
	ObjWheelTM.Get_Translation(&wheel_pos);
	wheel_pos.Z = -vehicle_def->Get_Lateral_Moment_Arm();
	Matrix3D::Rotate_Vector(Parent->Get_Transform(),wheel_pos,&r_lateral);
	wheel_pos.Z = -vehicle_def->Get_Tractive_Moment_Arm();
	Matrix3D::Rotate_Vector(Parent->Get_Transform(),wheel_pos,&r_tractive);

	// Apply the suspension force in the x-y plane of the CM (actually doesn't matter...)
	*force += SuspensionForce;
	*torque += Vector3::Cross_Product(r_lateral,SuspensionForce);

	// Apply the lateral friction force
	*force += LateralFrictionForce;
	*torque += Vector3::Cross_Product(r_lateral,LateralFrictionForce);

	// Apply the tractive friction force
	*force += TractiveFrictionForce;
	*torque += Vector3::Cross_Product(r_tractive,TractiveFrictionForce);

	// Debugging
#ifdef WWDEBUG
	Vector3 position;
	Parent->Get_Position(&position);
	Parent->Add_Debug_Vector(position + r_lateral,SuspensionForce / Parent->Get_Mass(),SUSPENSION_FORCE_COLOR);
	Parent->Add_Debug_Vector(position + r_lateral,LateralFrictionForce / Parent->Get_Mass(),LATERAL_FORCE_COLOR);
	Parent->Add_Debug_Vector(position + r_tractive,TractiveFrictionForce / Parent->Get_Mass(),TRACTIVE_FORCE_COLOR);
#endif
}


/***********************************************************************************************
 * WheelClass::Get_Ideal_Drive_Wheel_Angular_Velocity -- Computes the current angular velocity *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/18/2000 gth : Created.                                                                 *
 *=============================================================================================*/
float WheelClass::Get_Ideal_Drive_Wheel_Angular_Velocity(float max_avel)
{
	if (Get_Flag(INCONTACT)) {
		if (IdealAngularVelocity > max_avel) {
			return IdealAngularVelocity;
		}
	} 
	return max_avel;
}

/*************************************************************************************

	WVWheelClass Implementation (Wheeled-Vehicle-Wheel)

*************************************************************************************/ 
void WVWheelClass::Compute_Traction_Forces
(
	const Vector3 & local_pdot,
	float wheel_normal_force,
	float * set_lateral_force,
	float * set_tractive_force
)
{
	WWPROFILE("WVWheelClass::Compute_Traction_Forces");

	if (wheel_normal_force > 0.0f) {

		// Look up the friction coefficient and drag coefficient for this surface
		float drag_coefficient =		PhysicsConstants::Get_Contact_Drag_Coefficient(
													PhysicsConstants::DYNAMIC_OBJ_TYPE_TIRE,
													ContactSurface	);

		float friction_coefficient =	PhysicsConstants::Get_Contact_Friction_Coefficient(
													PhysicsConstants::DYNAMIC_OBJ_TYPE_TIRE,
													ContactSurface	);
		
		// Friction force from sliding should act like a proportional controller while the
		// engine torque or braking force should just be added in (later to be clamped to the
		// friction circle)
		float tractive_friction_coefficient = 0.001f;
		float lateral_friction_coefficient = friction_coefficient;
		if ((Get_Flag(BRAKING) == true) || (Parent->Is_Engine_Enabled() == false)) {
			tractive_friction_coefficient = lateral_friction_coefficient;
		}
		

		// Friction "penalty" force, proportional to the velocity, tuned so that the force
		// will be small when the velocity is small.  The lateral and tractive friction forces
		// will be set to the MIN between this proportional force and the maximum friction force.
		float penalty_k = WHEEL_FRICTION_PENALTY_CONSTANT;
		if (Parent->Get_Real_Wheel_Count() < 4) {
			penalty_k = BIKE_WHEEL_FRICTION_PENALTY_CONSTANT;
		}

		float tractive_force = WWMath::Min	(
													(penalty_k * wheel_normal_force * WWMath::Fabs(local_pdot.X)),
													(wheel_normal_force * tractive_friction_coefficient)
												);

		float lateral_force = WWMath::Min	(
													(penalty_k * wheel_normal_force * WWMath::Fabs(local_pdot.Y)),
													(wheel_normal_force * lateral_friction_coefficient)
												);

		tractive_force *= -WWMath::Sign(local_pdot.X);
		lateral_force *= -WWMath::Sign(local_pdot.Y);
		tractive_force -= drag_coefficient * local_pdot.X * local_pdot.X;

		if (Get_Flag(ENGINE) && !Get_Flag(BRAKING)) {
			tractive_force += AxleTorque / Radius; 
		}

		// Add in the initial lateral and tractive force (gravitation cancelling)
		lateral_force += *set_lateral_force;
		tractive_force += *set_tractive_force;

		// Clamp to the friction circle
		Vector2 friction_force(tractive_force,lateral_force);
		float max_friction = friction_coefficient * wheel_normal_force;
		float max_friction2 = max_friction * max_friction;
		float friction2 = friction_force.Length2();
		if (friction2 > max_friction2) {
			float friction = WWMath::Sqrt(friction2);
			friction_force *= max_friction / friction;
			SlipFactor = friction / max_friction;
		}

		// Set the output
		*set_tractive_force = friction_force.X;
		*set_lateral_force = friction_force.Y;

	} else {
		*set_tractive_force = 0.0f;
		*set_lateral_force = 0.0f;
		SlipFactor = 1.0f;
	}
}


void WVWheelClass::Update_Model(void)
{
	WheelClass::Update_Model();
	Roll_Wheel();
}

void WVWheelClass::Non_Physical_Update(float suspension_fraction,float rotation)
{
	RenderObjClass * model = Parent->Peek_Model();
	if (model == NULL) return;

	// Set up the position and constraints
	WheelClass::Non_Physical_Update(suspension_fraction,rotation);

	// Set the rotation 
	Matrix3D rotation_tm(1);
	rotation_tm.Rotate_Z(rotation);
	model->Control_Bone(RotationBone,rotation_tm);
}


void WVWheelClass::Roll_Wheel(void)
{
	RenderObjClass * model = Parent->Peek_Model();
	WheeledVehicleClass * wv_parent = Parent->As_WheeledVehicleClass();
	
	if ((model == NULL) || (wv_parent == NULL) || (RotationBone == -1)) {
		return;
	}

	// if we're braking, the wheels lock (not realistic but fun!)
	if (wv_parent->Is_Braking() == false) {
		
		if (Get_Flag(INCONTACT) || (Get_Flag(ENGINE) /*&& Not_In_Neutral*/)) {

			// Rotate the wheel based on movement
			Vector3 forward;
			Vector3 move;
			
			WheelTM.Get_X_Vector(&forward);
			Vector3::Subtract(Contact,LastPoint,&move);										
			float dist = Vector3::Dot_Product(move,forward);
			
			RotationDelta = -dist/Radius;
			
			if (Get_Flag(ENGINE) && (SlipFactor > 1.0f) && (SlipFactor < 100.0f)) {
				RotationDelta *= 2.0f * SlipFactor;
			}
			
			Rotation += RotationDelta;

			WWASSERT(WWMath::Is_Valid_Float(RotationDelta));
			WWASSERT(WWMath::Is_Valid_Float(Rotation));

		} else {
			
			// Rotate the wheel at the rate it was last rolling and slow it down a little
			Rotation += RotationDelta;
			RotationDelta *= 0.97f;

			WWASSERT(WWMath::Is_Valid_Float(RotationDelta));
			WWASSERT(WWMath::Is_Valid_Float(Rotation));
		}
	}
	
	Matrix3D rotation_tm(1);
	rotation_tm.Rotate_Z(Rotation);

	model->Control_Bone(RotationBone,rotation_tm);
	if (Rotation > 2.0f*(float)WWMATH_PI) Rotation -= 2.0f*(float)WWMATH_PI;
	else if (Rotation < 0.0f) Rotation += 2.0f*(float)WWMATH_PI;
	LastPoint = Contact;
}

/*************************************************************************************

	TrackWheelClass Implementation
	These wheels differ from WVWheelClass in that they do not roll independently.  
	They ask their TrackedVehicle parent how much their track has rolled and roll 
	with it.  Also, their friction forces may not behave the same way as wheeled
	vehicle wheels... Tanks probably shouldn't slide very much.

*************************************************************************************/ 

void TrackWheelClass::Compute_Traction_Forces
(
	const Vector3 & local_pdot,
	float wheel_normal_force,
	float * set_lateral_force,
	float * set_tractive_force
)
{
	WWPROFILE("TrackWheelClass::Compute_Traction_Forces");

	// Look up the friction coefficient for this surface
	float friction_coefficient =	PhysicsConstants::Get_Contact_Friction_Coefficient(
												PhysicsConstants::DYNAMIC_OBJ_TYPE_TIRE,
												ContactSurface	);
	
	// Friction force from sliding should act like a proportional controller while the
	// engine torque or braking force should just be added in (later to be clamped to the
	// friction circle)
	float tractive_friction_coefficient = 0.0001f;
	float lateral_friction_coefficient = friction_coefficient;

	// Try to stop if our engine is off
	if (Parent->Is_Engine_Enabled() == false) {
		tractive_friction_coefficient = lateral_friction_coefficient;
	}

	// Friction "penalty" force, proportional to the velocity, tuned so that the force
	// will be small when the velocity is small.  The lateral and tractive friction forces
	// will be set to the MIN between this proportional force and the maximum friction force.
#if 0
	float tractive_force = WWMath::Min (
		(TRACKWHEEL_FRICTION_PENALTY_CONSTANT * wheel_normal_force * WWMath::Fabs(local_pdot.X)),
		(wheel_normal_force * tractive_friction_coefficient) );

	float lateral_force = WWMath::Min (
		(TRACKWHEEL_FRICTION_PENALTY_CONSTANT * wheel_normal_force * WWMath::Fabs(local_pdot.Y)),
		(wheel_normal_force * lateral_friction_coefficient) );
#else
	float tractive_force = TRACKWHEEL_FRICTION_PENALTY_CONSTANT * wheel_normal_force * WWMath::Fabs(local_pdot.X);
	float lateral_force = TRACKWHEEL_FRICTION_PENALTY_CONSTANT * wheel_normal_force * WWMath::Fabs(local_pdot.Y);
#endif

	tractive_force *= -WWMath::Sign(local_pdot.X);
	tractive_force += AxleTorque / Radius; 
	tractive_force += *set_tractive_force;
	
	lateral_force *= -WWMath::Sign(local_pdot.Y);
	lateral_force += *set_lateral_force;

	// Clamp to the friction circle
	Vector2 wheel_force(tractive_force,lateral_force);
	float max_friction = 0.9f * wheel_normal_force;
	float max_friction2 = max_friction * max_friction;
	float wheel_force2 = wheel_force.Length2();
	if (wheel_force2 > max_friction2) {
		float initial_wheel_force = WWMath::Sqrt(wheel_force2);
		wheel_force *= max_friction / initial_wheel_force;
		SlipFactor = initial_wheel_force / max_friction;
	}

	// Return the result
	*set_tractive_force = wheel_force.X;
	*set_lateral_force = wheel_force.Y;
}


void TrackWheelClass::Update_Model(void)
{
	WheelClass::Update_Model();
	Roll_Wheel();
}

void TrackWheelClass::Non_Physical_Update(float suspension_fraction,float rotation)
{
	RenderObjClass * model = Parent->Peek_Model();
	if (model == NULL) return;

	// Set up the position and constraints
	WheelClass::Non_Physical_Update(suspension_fraction,rotation);

	// Set the rotation 
	Matrix3D rotation_tm(1);
	rotation_tm.Rotate_Z(rotation);
	model->Control_Bone(RotationBone,rotation_tm);
}


void TrackWheelClass::Roll_Wheel(void)
{
	// roll the wheel according to the contact point motion
	RenderObjClass * model = Parent->Peek_Model();
	TrackedVehicleClass * track_parent = Parent->As_TrackedVehicleClass();

	if ((model == NULL) || (track_parent == NULL) || (RotationBone == -1)) {
		return;
	}

	float dist;
	if (Get_Flag(LEFT_TRACK)) {
		dist = SlipFactor * track_parent->Get_Left_Track_Movement();
	} else {
		dist = SlipFactor * track_parent->Get_Right_Track_Movement();
	}
	
	RotationDelta = -dist/Radius;
	Rotation += RotationDelta;

	// apply the rotation to the wheel's center bone
	Matrix3D rotation_tm(1);
	rotation_tm.Rotate_Z(Rotation);

	model->Control_Bone(RotationBone,rotation_tm);
	if (Rotation > 2.0f*(float)WWMATH_PI) Rotation -= 2.0f*(float)WWMATH_PI;
	else if (Rotation < 0.0f) Rotation += 2.0f*(float)WWMATH_PI;
	LastPoint = Contact;
}



/*************************************************************************************

	VTOLWheelClass Implemention
	These wheels just don't like to move.  Their purpose is to just hold the vehicle
	up off the ground when it lands.  

*************************************************************************************/ 

void VTOLWheelClass::Update_Model(void)
{
	WheelClass::Update_Model();
	Roll_Wheel();
}

void VTOLWheelClass::Non_Physical_Update(float suspension_fraction,float rotation)
{
	RenderObjClass * model = Parent->Peek_Model();
	if (model == NULL) return;

	// Set up the position and constraints
	WheelClass::Non_Physical_Update(suspension_fraction,rotation);

	// Set the rotation 
	Matrix3D rotation_tm(1);
	rotation_tm.Rotate_Z(rotation);
	model->Control_Bone(RotationBone,rotation_tm);
}

void VTOLWheelClass::Compute_Traction_Forces	
(
	const Vector3 & local_pdot,
	float normal_force,
	float * set_lateral_force,
	float * set_tractive_force
)
{
	WWPROFILE("VTOLWheelClass::Compute_Traction_Forces");

	// Friction "penalty" force, proportional to the velocity, tuned so that the force
	// will be small when the velocity is small.  The lateral and tractive friction forces
	*set_tractive_force -= VTOLWHEEL_FRICTION_PENALTY_CONSTANT * normal_force * local_pdot.X;
	*set_lateral_force  -= VTOLWHEEL_FRICTION_PENALTY_CONSTANT * normal_force * local_pdot.Y;
}

void VTOLWheelClass::Roll_Wheel(void)
{
	RenderObjClass * model = Parent->Peek_Model();

	if ((RotationBone != -1) && (model != NULL)) {
		if (Get_Flag(INCONTACT)) {
			
			// Rotate the wheel based on movement
			Vector3 forward;
			Vector3 move;
			
			WheelTM.Get_X_Vector(&forward);
			Vector3::Subtract(Contact,LastPoint,&move);										
			float dist = Vector3::Dot_Product(move,forward);
			
			RotationDelta = -dist/Radius;
			Rotation += RotationDelta;

		} else {
			
			// Rotate the wheel at the rate it was last rolling and slow it down a little
			Rotation += RotationDelta;
			RotationDelta *= 0.97f;
		
		}
		
		Matrix3D rotation_tm(1);
		rotation_tm.Rotate_Z(Rotation);

		model->Control_Bone(RotationBone,rotation_tm);
		if (Rotation > 2.0f*(float)WWMATH_PI) Rotation -= 2.0f*(float)WWMATH_PI;
		else if (Rotation < 0.0f) Rotation += 2.0f*(float)WWMATH_PI;
		LastPoint = Contact;
	}
}

