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

#ifndef RBODY_H
#define RBODY_H

#include "always.h"
#include "movephys.h"
#include "ode.h"
#include "aabox.h"
#include "quat.h"
#include "physcon.h"
#include "wwdebug.h"

class RigidBodyDefClass;
class OBBoxRenderObjClass;
class PaddedBoxClass;
class OctBoxClass;
class RBodyHistoryClass;
struct CastResultStruct;

const int MAX_CONTACT_POINTS = 10;
const float RBODY_SLEEP_DELAY	= 0.5f;	// if still for this many seconds, shut off simulation


/**
** RigidBodyStateStruct
** This is the state vector for a RigidBodyClass.  
*/
struct RigidBodyStateStruct 
{
	RigidBodyStateStruct(void) {}
	RigidBodyStateStruct(const RigidBodyStateStruct & that);
	RigidBodyStateStruct & operator = (const RigidBodyStateStruct & that);
	
	void				Reset(void);
	void				To_Vector(StateVectorClass & vec);
	int				From_Vector(const StateVectorClass & vec,int index);
	static void		Lerp(const RigidBodyStateStruct & s0,const RigidBodyStateStruct & s1,float fraction,RigidBodyStateStruct * res);
	bool				Is_Valid(void) { return (Position.Is_Valid() && Orientation.Is_Valid() && LMomentum.Is_Valid() && AMomentum.Is_Valid()); }

	Vector3			Position;
	Quaternion		Orientation;
	Vector3			LMomentum;
	Vector3			AMomentum;
};


/** 
** RigidBodyClass
** Simulation of a rigid body.  The shape is limited to be an oriented box.
*/
class RigidBodyClass : public MoveablePhysClass , public ODESystemClass
{

public:

	RigidBodyClass(void);
	virtual ~RigidBodyClass(void);
	virtual RigidBodyClass *	As_RigidBodyClass(void) override { return this; }
	RigidBodyDefClass *			Get_RigidBodyDef(void) { return (RigidBodyDefClass *)Definition; }

	void								Init(const RigidBodyDefClass & definition);

	virtual const AABoxClass & Get_Bounding_Box(void) const;
	virtual const Matrix3D &	Get_Transform(void) const override;
	virtual void					Set_Transform(const Matrix3D & m) override;

	virtual bool					Cast_Ray(PhysRayCollisionTestClass & raytest) override;
	virtual bool					Cast_AABox(PhysAABoxCollisionTestClass & boxtest) override;
	virtual bool					Cast_OBBox(PhysOBBoxCollisionTestClass & boxtest) override;
	virtual bool					Intersection_Test(PhysAABoxIntersectionTestClass & test) override;
	virtual bool					Intersection_Test(PhysOBBoxIntersectionTestClass & test) override;
	virtual bool					Intersection_Test(PhysMeshIntersectionTestClass & test) override;

	/*
	** Teleport support
	*/
	virtual bool					Can_Teleport(const Matrix3D &new_tm, bool check_dyn_only = false,NonRefPhysListClass * result_list = NULL) override;
	virtual bool					Can_Teleport_And_Stand(const Matrix3D &new_tm, Matrix3D *out) override;

	/*
	** State
	*/
	void								Get_Orientation(Quaternion * set_q) const { *set_q = State.Orientation; }
	virtual void					Get_Velocity(Vector3 * set_vel) const override;
	virtual void					Get_Angular_Velocity(Vector3 * set_avel) const;
	virtual void					Set_Velocity(const Vector3 & newvel) override;
	virtual void					Set_Angular_Velocity(const Vector3 & newavel);
	virtual void					Apply_Impulse(const Vector3 & imp);
	virtual void					Apply_Impulse(const Vector3 & imp, const Vector3 & wpos);
	virtual void					Timestep(float dt) override;
	void								Compute_Point_Velocity(const Vector3 & p,Vector3 * pdot);
	float								Get_Last_Timestep(void)		{ return LastTimestep; }

	/*
	** Networking code.
	** Network_Interpolate_State_Update - interpolate towards the given state
	** Network_Latency_State_Update - client player update, interpolate, considering latency.
	*/
	void								Network_Interpolate_State_Update(	const Vector3 & new_pos,
																						const Quaternion & new_orientation,
																						const Vector3 & new_vel,
																						const Vector3 & new_avel,
																						float blend_fraction);

	void								Network_Latency_State_Update(			const Vector3 & new_pos,
																						const Quaternion & new_orientation,
																						const Vector3 & new_vel,
																						const Vector3 & new_avel);
	
	static void						Set_Correction_Time(float time)	{ _CorrectionTime = time; }
	static float					Get_Correction_Time(void)			{ return _CorrectionTime; }

	/*
	** Properties
	*/
	virtual void					Set_Model(RenderObjClass * model) override;
	virtual void					Set_Mass(float mass) override;
	virtual void					Get_Inertia_Inv(Matrix3 * set_I_inv) override;
	void								Set_Inertia(const Matrix3 & I);
	void								Get_Inertia(Matrix3 * I);
	void								Set_Contact_Parameters(float length);
	void								Get_Contact_Parameters(float * stiffness,float * damping,float * length);
	float								Get_Weight(void) { return GravScale * Mass * (-PhysicsConstants::GravityAcceleration.Z); }

	/*
	** Shadow support
	*/
	virtual void					Get_Shadow_Blob_Box(AABoxClass * set_obj_space_box) override;

	/*
	** MoveablePhysClass Push interface.  RigidBody's try to move when they are pushed.
	*/
	virtual bool					Push(const Vector3 & move) override;

	/*
	** Save-Load System
	*/
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save (ChunkSaveClass &csave) override;
	virtual bool								Load (ChunkLoadClass &cload) override;
	virtual void								On_Post_Load(void) override;

protected:

	/*
	** Integration system
	*/
	virtual void					Get_State(StateVectorClass & set_state) override;
	virtual int						Set_State(const StateVectorClass & new_state,int start_index) override;
	void								Set_State(const RigidBodyStateStruct & new_state);
	virtual int						Compute_Derivatives(float t,StateVectorClass * test_state,StateVectorClass * set_derivs,int start_index) override;

	void								Integrate(float time);

	/*
	** Internal functions
	*/
	void								Update_Cached_Model_Parameters(void);
	virtual void					Compute_Inertia(void);
	void								Update_Auxiliary_State(void);
	virtual void					Compute_Force_And_Torque(Vector3 * force,Vector3 * torque);
	void								Compute_Impact(const CastResultStruct & result,Vector3 * impulse);
	void								Compute_Impact(const Vector3 & point,const Vector3 & normal,Vector3 * impulse);
	bool								Is_Colliding(const Vector3 & point, const Vector3 & normal);
	void								Set_Moving_Collision_Region(float dt);
	void								Set_Stationary_Collision_Region(void);
	virtual bool					Can_Go_To_Sleep(float dt);
	bool								Find_State_In_Contact_Zone(const RigidBodyStateStruct & oldstate);
	bool								Push_Phys3_Object_Away(Phys3Class * p3obj,const CastResultStruct & contact);
	void								Network_Latency_Error_Correction(float dt);

	void								Assert_State_Valid(void) const;
	void								Assert_Not_Intersecting(void);
	void								Dump_State(void) const;
	
	/*
	** Physical Description:
	*/
	OBBoxRenderObjClass *		Box;					// World-Space Box
	Matrix3							IBody;				// Body space inertia tensor
	Matrix3							IBodyInv;			// Inverse of the body space inertia tensor
	OctBoxClass *					ContactBox;			// datastructure for finding contact points
	float								ContactThickness;	// Dimension of the contact region

	/*
	** State vector
	*/
	RigidBodyStateStruct			State;				// current state vector
	
	/*
	** Quantities derived from (dependent on) the current state
	*/
	Matrix3							Rotation;
	Matrix3							IInv;		
	Vector3							Velocity;
	Vector3							AngularVelocity;

	/*
	** Other Member variables
	*/
	bool								IsInContact;
	int								ContactCount;
	Vector3							ContactPoint[MAX_CONTACT_POINTS];
	Vector3							ContactNormal;
	int								StickCount;
	float								LastTimestep;
	float								GoToSleepTimer;

	/*
	** Network history for latency handling
	*/
	RBodyHistoryClass	*			History;
	RigidBodyStateStruct			LatencyError;
	RigidBodyStateStruct			LastKnownState;
	static float					_CorrectionTime;

private:

	// not implemented
	RigidBodyClass(const RigidBodyClass &);
	RigidBodyClass & operator = (const RigidBodyClass &);

};


inline RigidBodyStateStruct::RigidBodyStateStruct(const RigidBodyStateStruct & that)
{
	*this = that;
}

inline RigidBodyStateStruct & RigidBodyStateStruct::operator = (const RigidBodyStateStruct & that)
{
	Position = that.Position;
	Orientation = that.Orientation;
	LMomentum = that.LMomentum;
	AMomentum = that.AMomentum;
	return *this;
}

inline void RigidBodyStateStruct::Reset(void)
{
	Position.Set(0,0,0);
	Orientation.Make_Identity();
	LMomentum.Set(0,0,0);
	AMomentum.Set(0,0,0);
}

inline void RigidBodyStateStruct::To_Vector(StateVectorClass & vec)
{
	vec.Add(Position[0]);
	vec.Add(Position[1]);
	vec.Add(Position[2]);
	
	vec.Add(Orientation[0]);
	vec.Add(Orientation[1]);
	vec.Add(Orientation[2]);
	vec.Add(Orientation[3]);

	vec.Add(LMomentum[0]);
	vec.Add(LMomentum[1]);
	vec.Add(LMomentum[2]);

	vec.Add(AMomentum[0]);
	vec.Add(AMomentum[1]);
	vec.Add(AMomentum[2]);
}

inline int RigidBodyStateStruct::From_Vector(const StateVectorClass & vec,int index)
{
	Position[0] = vec[index++];
	Position[1] = vec[index++];
	Position[2] = vec[index++];

	Orientation[0] = vec[index++];
	Orientation[1] = vec[index++];
	Orientation[2] = vec[index++];
	Orientation[3] = vec[index++];

	LMomentum[0] = vec[index++];
	LMomentum[1] = vec[index++];
	LMomentum[2] = vec[index++];
	
	AMomentum[0] = vec[index++];
	AMomentum[1] = vec[index++];
	AMomentum[2] = vec[index++];

	return index;
}

inline void RigidBodyStateStruct::Lerp
(
	const RigidBodyStateStruct & s0,
	const RigidBodyStateStruct & s1,
	float fraction,
	RigidBodyStateStruct * res
)
{
	Vector3::Lerp(s0.Position,s1.Position,fraction,&(res->Position));
	Vector3::Lerp(s0.LMomentum,s1.LMomentum,fraction,&(res->LMomentum));
	Vector3::Lerp(s0.AMomentum,s1.AMomentum,fraction,&(res->AMomentum));
	::Fast_Slerp(res->Orientation,s0.Orientation,s1.Orientation,fraction);
}


/**
** RigidBodyDefClass
** Initialization data structure for RigidBodyClass
*/
class RigidBodyDefClass : public MoveablePhysDefClass
{
public:
	RigidBodyDefClass(void);
	
	// From DefinitionClass
	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create(void) const override;

	// From PhysDefClass
	virtual const char *						Get_Type_Name(void) override			{ return "RigidBodyDef"; }
	virtual bool								Is_Type(const char *) override;

	// From PersistClass
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save(ChunkSaveClass &csave) override;
	virtual bool								Load(ChunkLoadClass &cload) override;

	// In-Game Editing (DEBUGGING/TESTING ONLY)
	float											Get_Aerodynamic_Drag(void)	{ return AerodynamicDragCoefficient; }
	void											Set_Aerodynamic_Drag(float new_drag) { AerodynamicDragCoefficient = new_drag; }

	//	Editable interface requirements
	DECLARE_EDITABLE(RigidBodyDefClass,MoveablePhysDefClass);

protected:

	// Variables
	float											AerodynamicDragCoefficient;
	bool											CollisionDisabled;

	friend class RigidBodyClass;
};


#endif