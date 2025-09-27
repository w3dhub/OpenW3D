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
 *                     $Archive:: /Commando/Code/wwphys/projectile.cpp                        $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 12/20/01 5:14p                                              $*
 *                                                                                             *
 *                    $Revision:: 60                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "projectile.h"
#include "pscene.h"
#include "lineseg.h"
#include "physcon.h"
#include "physcoltest.h"
#include "rendobj.h"
#include "persistfactory.h"
#include "wwphysids.h"
#include "simpledefinitionfactory.h"
#include "wwdebug.h"
#include "wwhack.h"
#include "wwprofile.h"



DECLARE_FORCE_LINK(projectile);

/***********************************************************************************************
**
** ProjectileClass Implementation
**
***********************************************************************************************/

/*
** Declare a PersistFactory for ProjectileClasses
*/
SimplePersistFactoryClass<ProjectileClass,PHYSICS_CHUNKID_PROJECTILE>	_ProjectileFactory;

/*
** ChunkID's used by ProjectileClass
*/
enum 
{
	PROJECTILE_CHUNK_MOVEABLEPHYS				= 0x03308765,			// parent class data
	PROJECTILE_CHUNK_VARIABLES,

	PROJECTILE_VARIABLE_POSITION				= 0x00,
	PROJECTILE_VARIABLE_VELOCITY,
	PROJECTILE_VARIABLE_COLLIDESONMOVE,
	PROJECTILE_VARIABLE_ORIENTATIONMODE,
	PROJECTILE_VARIABLE_TUMBLEAXIS,
	PROJECTILE_VARIABLE_TUMBLERATE,
	PROJECTILE_VARIABLE_LIFETIME,
	PROJECTILE_VARIABLE_BOUNCECOUNT,
};


ProjectileClass::ProjectileClass(void) :
	CollidesOnMove(true),
	OrientationMode(ORIENTATION_ALIGNED),
	TumbleAxis(1,0,0),
	TumbleRate(DEG_TO_RADF(10.0f)),
	Lifetime(2.0f),
	BounceCount(0)
{
	State.Position.Set(0,0,0);
	State.Velocity.Set(0,0,0);

	// turn off lighting for all projectiles
	Enable_Is_Pre_Lit(true);
}

void ProjectileClass::Init(const ProjectileDefClass & def)
{
	MoveablePhysClass::Init(def);
	CollidesOnMove = def.CollidesOnMove;
	OrientationMode = def.OrientationMode;
	TumbleAxis = def.TumbleAxis;
	TumbleRate = def.TumbleRate;
	Lifetime = def.Lifetime;
	BounceCount = def.BounceCount;

	State.Position.Set(0,0,0);
	State.Velocity.Set(0,0,0);

	if (TumbleAxis.Length2() == 0.0f) {
		TumbleAxis.Set(0,0,1);
	} else {
		TumbleAxis.Normalize();
	}

	// turn off lighting for all projectiles
	Enable_Is_Pre_Lit(true);
}

ProjectileClass::~ProjectileClass(void)
{
}

const AABoxClass & ProjectileClass::Get_Bounding_Box(void) const
{
	assert(Model);
	return Model->Get_Bounding_Box();
}

const Matrix3D & ProjectileClass::Get_Transform(void) const
{
	assert(Model);
	return Model->Get_Transform();
}

void ProjectileClass::Set_Transform(const Matrix3D & tm)
{
	tm.Get_Translation(&State.Position);

	assert(Model);
	Model->Set_Transform(tm);

	// TODO: don't use the actual bounds?  Just use the position and a pre-calced extent
	Update_Cull_Box();
}

void ProjectileClass::Set_Orientation_Mode_Tumbling(void)
{
	OrientationMode = ORIENTATION_TUMBLING;
	
	TumbleAxis.Set(0,0,0);
	while (TumbleAxis.Length() <= 0.001f) {
		TumbleAxis.X = WWMath::Random_Float();
		TumbleAxis.Y = WWMath::Random_Float();
		TumbleAxis.Z = WWMath::Random_Float();
	}
	TumbleAxis.Normalize();
	TumbleRate = WWMath::Random_Float((float)DEG_TO_RAD(2.0f),(float)DEG_TO_RAD(10.0f));
}


void ProjectileClass::Set_Orientation_Mode_Tumbling(const Vector3 & axis,float rate)
{
	OrientationMode = ORIENTATION_TUMBLING;
	TumbleAxis = axis;
	TumbleAxis.Normalize();
	TumbleRate = rate;
}

void ProjectileClass::Set_Orientation_Mode_Aligned_Fixed(void)
{
	OrientationMode = ORIENTATION_ALIGNED_FIXED;
	if (OrientationMode == ORIENTATION_ALIGNED_FIXED) {
	  	Matrix3D tm;
		tm.Obj_Look_At(State.Position,State.Position + State.Velocity,0.0f);
		Model->Set_Transform(tm);
	}
}



void	ProjectileClass::Set_Lifetime( float time )
{
	Lifetime = time;
}

float	ProjectileClass::Get_Lifetime( void )
{
	return Lifetime;
}

void ProjectileClass::Set_Bounce_Count(int count)
{
	BounceCount = count;
}

int ProjectileClass::Get_Bounce_Count(void)
{
	return BounceCount;
}

void ProjectileClass::Integrate(float dt)
{
	float accel = PhysicsConstants::GravityAcceleration.Z * GravScale;
	State.Position.X += State.Velocity.X * dt;
	State.Position.Y += State.Velocity.Y * dt;
	State.Position.Z += 0.5f * accel * dt * dt + State.Velocity.Z * dt;

	State.Velocity.Z += accel * dt;
}

void ProjectileClass::Timestep(float dt)
{
	WWPROFILE("Projectile::Timestep");
	const int MAX_BUMPS = 5;

	if (Is_User_Control_Enabled()) {
		return;
	}

	if (dt == 0.0f) {
		return;
	}

	WWASSERT(State.Position.Is_Valid());
	WWASSERT(State.Velocity.Is_Valid());

	if ( CollidesOnMove ) {

		WWPROFILE("Move and Collide");

		/*
		** Repeat until we eat all of the time
		*/
		float remaining_time = dt;
		float timestep;
		int bumps = 0;

		/*
		** This is that last object we choose to ignore
		*/
		PhysClass	*blocker = NULL;

		while ((remaining_time > 0.0f) && (bumps < MAX_BUMPS) && (BounceCount >= 0)) {

			timestep = remaining_time;
			bumps++;

			/*
			** Integrate the state of the object for some amount
			** of time: timestep
			*/
			ProjectileStateStruct oldstate = State;
			Integrate(timestep);

			/*
			** Check for collisions in the path of the object
			*/
			CastResultStruct res;
			LineSegClass ray(oldstate.Position,State.Position);
			PhysRayCollisionTestClass raytest(ray,&res,Get_Collision_Group(),COLLISION_TYPE_PROJECTILE);

			{ 
				WWPROFILE("Raycast");
				Inc_Ignore_Counter();
				PhysicsSceneClass::Get_Instance()->Cast_Ray(raytest);
				Dec_Ignore_Counter();
			}
			
			/*
			** If the result was a "startbad", just do the whole step
			*/
			if (raytest.Result->StartBad) {
				remaining_time -= timestep;
			} else {

				/*
				** If there was a collision, cut the timestep so that
				** we integrate right up to the collision
				*/
				if (raytest.Result->Fraction < 1.0f) {
					
					
					timestep = raytest.Result->Fraction * timestep;
					remaining_time -= timestep;

					/*
					** Compute the point of collision, stored in ContactPoint of the CastResStruct
					** so that observers have it.
					*/
					ray.Compute_Point(raytest.Result->Fraction,&(raytest.Result->ContactPoint));

					/*
					** Reset and integrate up to that point
					*/
					State = oldstate;
					Integrate(0.99f * timestep);
					
					/*
					** Notify the parties involved
					*/
					WWASSERT(raytest.CollidedPhysObj != NULL);

					CollisionReactionType reaction = COLLISION_REACTION_DEFAULT;

					CollisionEventClass event;
					event.CollisionResult = raytest.Result;
					event.CollidedRenderObj = raytest.CollidedRenderObj;

					{
						WWPROFILE("Callbacks");
						event.OtherObj = raytest.CollidedPhysObj;
						reaction |= Collision_Occurred(event);

						event.OtherObj = this;
						reaction |= raytest.CollidedPhysObj->Collision_Occurred(event);
					}

					if ( !(reaction & COLLISION_REACTION_NO_BOUNCE) ) {

						/*
						** Perform collision processing and loop
						*/
						float dot = Vector3::Dot_Product(State.Velocity,raytest.Result->Normal);
						State.Velocity -= (1.0f + Elasticity)*dot*raytest.Result->Normal;

						/*
						** If we are using ORIENTATION_MODE_ALIGNED_FIXED, update the
						** transform whenever we bounce
						*/
						if (OrientationMode == ORIENTATION_ALIGNED_FIXED) {
							Matrix3D tm;
							tm.Obj_Look_At(State.Position,State.Position + State.Velocity,0.0f);
							Model->Set_Transform(tm);
						}

						/*
						** Decrement the bounce count!
						*/
						BounceCount--;

					} else {

						/*
						** We were requested to fly through.  Mark the current blocker as ignore
						** so we collide with him no more this pass
						*/
						Integrate(0.02*timestep);

						if ( blocker ) {	 // Stop ignoring the last blocker
							blocker->Dec_Ignore_Counter();
						}
						blocker = raytest.CollidedPhysObj;
						if ( blocker ) {	// Start ignoring this blocker
							blocker->Inc_Ignore_Counter();
						}

					}

					/*
					** If requested to stop, progress no more
					*/
					if ( reaction & COLLISION_REACTION_STOP_MOTION ) {
						remaining_time = 0;
						State.Velocity = Vector3(0,0,0);
					}
				
				} else {
					
					remaining_time -= timestep;

				}
			}
		}

		if ( blocker ) {	// Stop ignoring the last blocker
			blocker->Dec_Ignore_Counter();
		}

	} else {

		Integrate(dt);

	}

	Update_Transform(dt);
	Update_Cull_Box();
	
	DEBUG_RENDER_AXES(Get_Transform(),Vector3(1.0f,1.0f,1.0f));

	/*
	** Decrement our life, check our bounce count
	*/
	Lifetime -= dt;
	if ((Lifetime < 0.0f) || (BounceCount < 0)) {
		Expire();
	}
}

bool ProjectileClass::Cast_Ray(PhysRayCollisionTestClass & raytest)
{
	assert(Model);
	if (Model->Cast_Ray(raytest)) {
		raytest.CollidedPhysObj = this;
		return true;
	}
	return false;
}

void ProjectileClass::Update_Transform(float dt)
{
	WWASSERT(Model);

	// Update the cached transformation matrix
	Matrix3D tm;

	switch(OrientationMode) {

		case ORIENTATION_ALIGNED:
			// compute a matrix aligned to the path
			tm.Obj_Look_At(State.Position,State.Position + State.Velocity,0.0f);
			Model->Set_Transform(tm);
			break;
		
		case ORIENTATION_ALIGNED_FIXED:
		case ORIENTATION_FIXED:
			// just update the position
			Model->Set_Position(State.Position);
			break;

		case ORIENTATION_TUMBLING:
			// update the position, tumble a little
			tm = Model->Get_Transform();
			tm.Set_Translation(State.Position);
			Matrix3D::Multiply(tm,Matrix3D(TumbleAxis,TumbleRate*dt),&tm);
			Model->Set_Transform(tm);
			break;
	
	};

	Update_Visibility_Status();
}

const PersistFactoryClass & ProjectileClass::Get_Factory (void) const
{
	return _ProjectileFactory;
}

bool ProjectileClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk(PROJECTILE_CHUNK_MOVEABLEPHYS);
	MoveablePhysClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(PROJECTILE_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,PROJECTILE_VARIABLE_POSITION,State.Position);
	WRITE_MICRO_CHUNK(csave,PROJECTILE_VARIABLE_VELOCITY,State.Velocity);
	WRITE_MICRO_CHUNK(csave,PROJECTILE_VARIABLE_COLLIDESONMOVE,CollidesOnMove);
	WRITE_MICRO_CHUNK(csave,PROJECTILE_VARIABLE_ORIENTATIONMODE,OrientationMode);
	WRITE_MICRO_CHUNK(csave,PROJECTILE_VARIABLE_TUMBLEAXIS,TumbleAxis);
	WRITE_MICRO_CHUNK(csave,PROJECTILE_VARIABLE_TUMBLERATE,TumbleRate);
	WRITE_MICRO_CHUNK(csave,PROJECTILE_VARIABLE_LIFETIME,Lifetime);
	WRITE_MICRO_CHUNK(csave,PROJECTILE_VARIABLE_BOUNCECOUNT,BounceCount);
	csave.End_Chunk();

	return true;
}

bool ProjectileClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		
		switch(cload.Cur_Chunk_ID()) 
		{
			case PROJECTILE_CHUNK_MOVEABLEPHYS:
				MoveablePhysClass::Load(cload);
				break;

			case PROJECTILE_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,PROJECTILE_VARIABLE_POSITION,State.Position);
						READ_MICRO_CHUNK(cload,PROJECTILE_VARIABLE_VELOCITY,State.Velocity);
						READ_MICRO_CHUNK(cload,PROJECTILE_VARIABLE_COLLIDESONMOVE,CollidesOnMove);
						READ_MICRO_CHUNK(cload,PROJECTILE_VARIABLE_ORIENTATIONMODE,OrientationMode);
						READ_MICRO_CHUNK(cload,PROJECTILE_VARIABLE_TUMBLEAXIS,TumbleAxis);
						READ_MICRO_CHUNK(cload,PROJECTILE_VARIABLE_TUMBLERATE,TumbleRate);
						READ_MICRO_CHUNK(cload,PROJECTILE_VARIABLE_LIFETIME,Lifetime);
						READ_MICRO_CHUNK(cload,PROJECTILE_VARIABLE_BOUNCECOUNT,BounceCount);
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
** ProjectileDefClass Implementation
**
***********************************************************************************************/

/*
** Declare a PersistFactory for ProjectileDefClasses, this allows them to save and load
*/
SimplePersistFactoryClass<ProjectileDefClass,PHYSICS_CHUNKID_PROJECTILEDEF>			_ProjectileDefFactory;

/*
** Declare a definition factory.  This exposes ProjectileDef to the editor.
*/
DECLARE_DEFINITION_FACTORY(ProjectileDefClass, CLASSID_PROJECTILEDEF, "Projectile") _ProjectileDefDefFactory;

/*
** Chunk ID's used by MoveablePhysDefClass
*/
enum 
{
	PROJECTILEDEF_CHUNK_MOVEABLEPHYSDEF				= 0x01210011,			// (parent class)
	PROJECTILEDEF_CHUNK_VARIABLES,

	PROJECTILEDEF_VARIABLE_COLLIDESONMOVE			= 0x00,
	PROJECTILEDEF_VARIABLE_ORIENTATIONMODE,
	PROJECTILEDEF_VARIABLE_TUMBLEAXIS,
	PROJECTILEDEF_VARIABLE_TUMBLERATE,
	PROJECTILEDEF_VARIABLE_LIFETIME,
	PROJECTILEDEF_VARIABLE_BOUNCECOUNT,
	
};


ProjectileDefClass::ProjectileDefClass(void) :
	CollidesOnMove(true),
	OrientationMode(ProjectileClass::ORIENTATION_ALIGNED),
	TumbleAxis(1,2,1),
	TumbleRate(DEG_TO_RADF(10.0f)),
	Lifetime(2.0f),
	BounceCount(0)
{
#ifdef PARAM_EDITING_ON
	// make our parameters editable!
	EDITABLE_PARAM(ProjectileDefClass, ParameterClass::TYPE_BOOL, CollidesOnMove);

	EnumParameterClass *param = new EnumParameterClass(&OrientationMode);
	param->Set_Name ("OrientationMode");
	param->Add_Value("ALIGNED",ProjectileClass::ORIENTATION_ALIGNED);
	param->Add_Value("FIXED",ProjectileClass::ORIENTATION_FIXED);
	param->Add_Value("TUMBLE",ProjectileClass::ORIENTATION_TUMBLING);
	GENERIC_EDITABLE_PARAM(ProjectileDefClass,param)

	FLOAT_EDITABLE_PARAM(ProjectileDefClass, TumbleAxis.X, 0.0f, 10.0f);	
	FLOAT_EDITABLE_PARAM(ProjectileDefClass, TumbleAxis.Y, 0.0f, 10.0f);	
	FLOAT_EDITABLE_PARAM(ProjectileDefClass, TumbleAxis.Z, 0.0f, 10.0f);	
	ANGLE_EDITABLE_PARAM(ProjectileDefClass, TumbleRate, DEG_TO_RADF(0.1f), DEG_TO_RADF(90.0f));
	FLOAT_EDITABLE_PARAM(ProjectileDefClass, Lifetime, 0.01f, 100.0f);	
	INT_EDITABLE_PARAM(ProjectileDefClass, BounceCount, 0, 32);
#endif
}

const PersistFactoryClass & ProjectileDefClass::Get_Factory (void) const
{
	return _ProjectileDefFactory;
}

uint32 ProjectileDefClass::Get_Class_ID (void) const	
{ 
	return CLASSID_PROJECTILEDEF; 
}

PersistClass * ProjectileDefClass::Create(void) const
{
	ProjectileClass * obj = NEW_REF(ProjectileClass,());
	obj->Init(*this);
	return obj;
}

bool ProjectileDefClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(PROJECTILEDEF_CHUNK_MOVEABLEPHYSDEF);
	MoveablePhysDefClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(PROJECTILEDEF_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,PROJECTILEDEF_VARIABLE_COLLIDESONMOVE,CollidesOnMove);
	WRITE_MICRO_CHUNK(csave,PROJECTILEDEF_VARIABLE_ORIENTATIONMODE,OrientationMode);
	WRITE_MICRO_CHUNK(csave,PROJECTILEDEF_VARIABLE_TUMBLEAXIS,TumbleAxis);
	WRITE_MICRO_CHUNK(csave,PROJECTILEDEF_VARIABLE_TUMBLERATE,TumbleRate);
	WRITE_MICRO_CHUNK(csave,PROJECTILEDEF_VARIABLE_LIFETIME,Lifetime);
	WRITE_MICRO_CHUNK(csave,PROJECTILEDEF_VARIABLE_BOUNCECOUNT,BounceCount);
	csave.End_Chunk();
	return true;
}


bool ProjectileDefClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {

		switch(cload.Cur_Chunk_ID()) {

			case PROJECTILEDEF_CHUNK_MOVEABLEPHYSDEF:
				MoveablePhysDefClass::Load(cload);
				break;

			case PROJECTILEDEF_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,PROJECTILEDEF_VARIABLE_COLLIDESONMOVE,CollidesOnMove);
						READ_MICRO_CHUNK(cload,PROJECTILEDEF_VARIABLE_ORIENTATIONMODE,OrientationMode);
						READ_MICRO_CHUNK(cload,PROJECTILEDEF_VARIABLE_TUMBLEAXIS,TumbleAxis);
						READ_MICRO_CHUNK(cload,PROJECTILEDEF_VARIABLE_TUMBLERATE,TumbleRate);
						READ_MICRO_CHUNK(cload,PROJECTILEDEF_VARIABLE_LIFETIME,Lifetime);
						READ_MICRO_CHUNK(cload,PROJECTILEDEF_VARIABLE_BOUNCECOUNT,BounceCount);
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

bool ProjectileDefClass::Is_Type(const char * type_name)
{
	if (stricmp(type_name,ProjectileDefClass::Get_Type_Name()) == 0) {
		return true;
	} else {
		return MoveablePhysDefClass::Is_Type(type_name);
	}
}

