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
 *                     $Archive:: /Commando/Code/wwphys/phys3.h                               $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 10/24/01 11:15a                                             $*
 *                                                                                             *
 *                    $Revision:: 46                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PHYS3_H
#define PHYS3_H

#include "vector3.h"
#include "movephys.h"
#include "ode.h"


class Phys3DefClass;
class Phys3HistoryClass;

/** 
** Phys3Class
** 3 Degree of Freedom Physics
** This object can translate in X,Y,and Z.  It will be represented
** in the world by an axis-aligned box. 
*/
class Phys3Class : public MoveablePhysClass
{

public:

	Phys3Class(void);
	virtual ~Phys3Class(void);
	virtual Phys3Class *			As_Phys3Class(void) override { return this; }

	void								Init(const Phys3DefClass & def);

	virtual const AABoxClass & Get_Bounding_Box(void) const;
	virtual const Matrix3D &	Get_Transform(void) const override;
	virtual void					Set_Transform(const Matrix3D & m) override;

	virtual const AABoxClass &	Get_Collision_Box(void) const	{ return CollisionBox; }

	virtual bool					Cast_Ray(PhysRayCollisionTestClass & raytest) override;
	virtual bool					Cast_AABox(PhysAABoxCollisionTestClass & boxtest) override;
	virtual bool					Cast_OBBox(PhysOBBoxCollisionTestClass & boxtest) override;
	virtual bool					Intersection_Test(PhysAABoxIntersectionTestClass & test) override;
	virtual bool					Intersection_Test(PhysOBBoxIntersectionTestClass & test) override;
	virtual bool					Intersection_Test(PhysMeshIntersectionTestClass & test) override;

	virtual void					Set_Model(RenderObjClass * model) override;

	virtual void					Get_Velocity(Vector3 * set_vel) const override;
	virtual void					Set_Velocity(const Vector3 & newvel) override;

	virtual void					Apply_Impulse(const Vector3 & imp);
	virtual void					Apply_Impulse(const Vector3 & imp, const Vector3 & wpos);

	virtual void					Timestep(float dt) override;
	virtual bool 					Is_In_Contact(void) const { return OnGround; }

	// Set_In_Contact needed for network state updates so we don't have to wait for Check_Ground
	virtual void 					Set_In_Contact( bool onoff ) { OnGround = onoff; }
	virtual int						Get_Contact_Surface_Type(void) { return GroundSurface; }
	virtual void					Invalidate_Ground_State(void);
	PhysClass *						Peek_Ground_Object(void) { return GroundState.GroundObject; }
	void								Assert_State_Valid(void);

	/*
	** Teleport support
	*/
	virtual bool					Can_Teleport(const Matrix3D &new_tm,bool check_dyn_only = false,NonRefPhysListClass * result_list = NULL) override;
	virtual bool					Can_Teleport_And_Stand(const Matrix3D &new_tm, Matrix3D *out) override;
	virtual bool					Find_Teleport_Location(const Vector3 &start, float radius, Vector3 *out) override;
	virtual bool					Can_Move_To(const Matrix3D &new_tm) override;

	/*
	** Phys3 movement controls
	*/
	virtual void					Set_Position(const Vector3 & position);	
	virtual const Vector3 &		Get_Position(void) const;
	virtual void					Set_Heading(float heading);
	virtual float					Get_Heading(void) const;

	virtual void					Set_Slide_Angle(float angle);
	virtual float					Get_Slide_Angle(void) const;
	virtual void					Set_Normalized_Speed(float val)						{ NormSpeed = val; }
	virtual float					Get_Normalized_Speed(void)	const						{ return NormSpeed; }
	
	void								Add_Animation_Move(const Vector3 & move)			{ AnimationMove += move; }
	void								Get_Animation_Move(Vector3 * set_move)				{ *set_move = AnimationMove; }
	void								Reset_Animation_Move(void)								{ AnimationMove.Set(0,0,0); }

	/*
	** Shadow support
	*/
	virtual void					Get_Shadow_Blob_Box(AABoxClass * set_obj_space_box) override;
	void								Get_Collision_Box(AABoxClass * set_box);

	/*
	** MoveablePhysClass Push interface.  Phys3's try to move when they are pushed.
	*/
	virtual bool					Push(const Vector3 & move) override;
	virtual bool					Collide(const Vector3 & move);

	/*
	** Network state updates
	*/
	void								Network_State_Update(	const Vector3 & pos,
																		const Vector3 & vel);

	void								Network_Latency_State_Update(	const Vector3 & pos,
																				const Vector3 & vel);
	
	static void						Set_Correction_Time(float time) { _CorrectionTime = time; }
	static void						Set_Allowable_Error(float err) { _AllowableError = err; }
	static void						Set_Pop_Error(float err) { _PopError = err; }

	static float					Get_Correction_Time(void) { return _CorrectionTime; }
	static float					Get_Allowable_Error(void) { return _AllowableError; }
	static float					Get_Pop_Error(void) { return _PopError; }

	/*
	** Save-Load system
	*/
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save (ChunkSaveClass &csave) override;
	virtual bool								Load (ChunkLoadClass &cload) override;
	virtual void								On_Post_Load(void) override;

protected:

	/*
	** GroundStateStruct status of the object's contact with the ground
	*/
	struct GroundStateStruct
	{
		GroundStateStruct(void);
		void					Reset(void);
		void					Init_From_Collision_Result(PhysAABoxCollisionTestClass & test,float height);

		bool					IsDirty;					// data contained within is invalid
		bool					OnGround;				// indicates whether the object is "on the ground"
		bool					OnDynamicObj;			// must "dirty" the groundstate at end of frame if we're on a dyn obj
		int					SurfaceType;			// surface type the object is resting on
		float					Height;					// our height above the ground
		Vector3				Normal;					// normal for the surface
		Vector3				Down;						// slide direction for the surface
		PhysClass *			GroundObject;			// pointer to the object we are on
		RenderObjClass *	GroundRenderObject;
	};

	/*
	** State of a Phys3 object
	*/
	struct StateStruct 
	{
		StateStruct(void);
		StateStruct(const StateStruct & that);
		StateStruct &	operator = (const StateStruct & that);

		Vector3				Position;
		Vector3				Velocity;		
	};


	/*
	** Phys3 Movement algorithm
	*/
	virtual GroundStateStruct & Get_Ground_State(void);
	virtual void					Check_Ground(const AABoxClass & box,GroundStateStruct * gs,float check_dist);

	virtual bool					User_Move(float dt);
	virtual bool					Ballistic_Move(float dt);
	virtual bool					Slide_Move(const GroundStateStruct & gs,float dt);
	virtual bool					Normal_Move(const GroundStateStruct & gs,float dt);
	virtual bool					Collide_Move(const Vector3 & requested_move,float dt);
	
	bool								Apply_Move(const Vector3 & move,float dt,bool allow_sliding = true,bool allow_stepping = false,bool stop_on_walkable = false);
	void								Snap_To_Ground(const Vector3 & actual_move,bool was_stepping);
	void								Clip_Move(const Vector3 * contacts,int contact_count,Vector3 * move);
	
	void								Attach_To_Ground_Object(void);

	/*
	** Internal functions
	*/
	void								Update_Cached_Model_Parameters(void);
	void								Update_Transform(bool position_only = false);
	void								Compute_WS_Collision_Box(const StateStruct & state,AABoxClass * set_box);
	bool								Debug_Verify_Position(void);
	void								Network_Teleport_Correction(void);

	enum MoveModeType
	{
		USER_OVERRIDE = 0,
		BALLISTIC_MOVE,
		SLIDE_MOVE,
		NORMAL_MOVE,
		COLLIDE_MOVE
	};

	/*
	** Members
	*/
	AABoxClass						CollisionBox;		// object space collision box
	bool								OnGround;			// flag indicates whether object is resting on something
	bool								InCollision;		// this object is already participating in a collision
	bool								HeadingChanged;	// If the heading changes, transform will be updated on next timestep
	int								GroundSurface;		// surface type id of the ground we're standing on.
	StateStruct						State;				// state vector
	float								Heading;				// heading, a move of 1,0,0 will move in this direction.
	float								NormSpeed;			// speed to move when controller is 1.0
	float								SlideAngle;			// slope angle at which this object slides off 
	float								SlideNormalZ;		// cos(SlideAngle)
	float								SlideAngleTan;		// tan(SlideAngle)
	float								StepHeight;			// step side that this object will hop over
	MoveModeType					MoveMode;			// current movement mode
	PhysClass *						GroundObject;		// object that we are standing on

	GroundStateStruct				GroundState;		// info on the surface we're standing on (if any)
	Vector3							AnimationMove;		// how far this object moved for animation purposes

	Phys3HistoryClass *			History;				// history of our state for smarter network updating
	Vector3							LatencyError;		// remaining latency error
	Vector3							LastKnownPosition;// last position received from server
	Vector3							LastKnownVelocity;// last velocity received from server

	static float					_CorrectionTime;	// Network correction handling constants
	static float					_AllowableError;
	static float					_PopError;
	
private:

	// not implemented
	Phys3Class(const Phys3Class &);
	Phys3Class & operator = (const Phys3Class &);

};



inline void Phys3Class::Get_Velocity(Vector3 * set_vel) const 
{ 
	*set_vel = State.Velocity; 
}

inline void Phys3Class::Set_Velocity(const Vector3 & newvel)
{
	State.Velocity = newvel;
}

inline void Phys3Class::Compute_WS_Collision_Box(const StateStruct & state,AABoxClass * set_box)
{
	set_box->Center = CollisionBox.Center + state.Position;
	set_box->Extent = CollisionBox.Extent;
}

inline Phys3Class::StateStruct::StateStruct(void) :
	Position(0,0,0),
	Velocity(0,0,0)
{ 
}

inline Phys3Class::StateStruct::StateStruct(const StateStruct & that)
{ 
	*this = that; 
}

inline 
Phys3Class::StateStruct & 
Phys3Class::StateStruct::operator = (const StateStruct & that)	
{ 
	Position = that.Position; 
	Velocity = that.Velocity; 
	return *this;
}


/**
** Phys3DefClass
** Initialization/Game-Database support for Phys3Class
*/
class Phys3DefClass : public MoveablePhysDefClass
{
public:
	
	Phys3DefClass(void);
	
	// From Definition
	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create(void) const override;

	// From PhysDefClass
	virtual const char *						Get_Type_Name(void) override			{ return "Phys3Def"; }
	virtual bool								Is_Type(const char *) override;

	// From PersistClass
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save(ChunkSaveClass &csave) override;
	virtual bool								Load(ChunkLoadClass &cload) override;

		//	Editable interface requirements
	DECLARE_EDITABLE(Phys3DefClass,MoveablePhysDefClass);

protected:

	float								NormSpeed;			// speed to move when controller is 1.0
	float								SlideAngle;			// slope angle at which this object slides off 
	float								StepHeight;			// step side that this object will hop over

	friend class Phys3Class;

};

#endif