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
 *                     $Archive:: /Commando/Code/wwphys/humanphys.cpp                         $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 10/27/01 4:32p                                              $*
 *                                                                                             *
 *                    $Revision:: 44                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   HumanPhysClass::HumanPhysClass -- Constructor                                             *
 *   HumanPhysClass::HumanPhysClass -- Constructor, initializes from a Definition              *
 *   HumanPhysClass::~HumanPhysClass -- Destructor                                             *
 *   HumanPhysClass::Timestep -- Simulate this object for time dt                              *
 *   HumanPhysClass::Check_Ground -- check the ground state                                    *
 *   HumanPhysClass::Ballistic_Move -- ballistic motion                                        *
 *   HumanPhysClass::Slide_Move -- Sliding down a slope                                        *
 *   HumanPhysClass::Normal_Move -- Moving under user control                                  *
 *   HumanPhysClass::Compute_Desired_Move_Vector -- compute the move vector                    *
 *   HumanPhysClass::Get_Factory -- returns the PersistFactory for save-load support           *
 *   HumanPhysClass::Save -- Save this object                                                  *
 *   HumanPhysClass::Load -- Load this object                                                  *
 *   HumanPhysClass::Render -- Render this object                                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "humanphys.h"
#include "wwdebug.h"
#include "physcoltest.h"
#include "pscene.h"
#include "physcon.h"
#include "physcontrol.h"
#include "wwphysids.h"
#include "persistfactory.h"
#include "simpledefinitionfactory.h"
#include "wwhack.h"
#include "wwprofile.h"

DECLARE_FORCE_LINK(humanphys);

#define VERBOSE_LOGGING 0

#if VERBOSE_LOGGING
#define VERBOSE_LOG(x) if (WWDEBUG_TRIGGER(WWDEBUG_TRIGGER_GENERIC0)) { WWDEBUG_SAY(x); }
#else
#define VERBOSE_LOG(x)
#endif


bool HumanPhysClass::_DisableHumanSimulation = false;
bool HumanPhysClass::_DisableHumanRendering = false;


/*
** Declare a PersistFactory for HumanPhysClass
*/
SimplePersistFactoryClass<HumanPhysClass,PHYSICS_CHUNKID_HUMANPHYS>	_HumanPhysFactory;

/*
** Chunk-ID's used by HumanPhysClass
*/
enum 
{
	HUMANPHYS_CHUNK_PHYS3			= 0x04040404,		// parent Phys3Class data
	HUMANPHYS_CHUNK_VARIABLES,

	HUMANPHYS_VARIABLE_JUSTJUMPED	= 0x00
};

/*
** SLOPE_SPEED_ADJUSTMENT - fraction of the speed to shave off when player is walking on 
** the steepest possible slope (just before sliding takes over).  Also the fraction to
** add when the player is walking straight downhill
** AIR_SPEED_ADJUSTMENT - scale factor to apply to the users move when in the air
*/
const float SLOPE_SPEED_REDUCTION = 0.1f;
const float AIR_MOVE_SCALE = 0.1f;


/***********************************************************************************************
 * HumanPhysClass::HumanPhysClass -- Constructor                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/16/2000  gth : Created.                                                                 *
 *=============================================================================================*/
HumanPhysClass::HumanPhysClass(void)	:
	IsAIControlledJump (false)
{
	JustJumped = false;
	Set_Mass(1.0f); // ??

	// (gth) turn on shadows for all humans for now
	Enable_Shadow_Generation(true);
}


/***********************************************************************************************
 * HumanPhysClass::Init -- initializes from a Definition                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/16/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void HumanPhysClass::Init(const HumanPhysDefClass & def)
{
	Phys3Class::Init(def);

	// (gth) turn on shadows for all humans for now
	Enable_Shadow_Generation(true);
}


/***********************************************************************************************
 * HumanPhysClass::~HumanPhysClass -- Destructor                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
HumanPhysClass::~HumanPhysClass(void)
{
}


/***********************************************************************************************
 * HumanPhysClass::Timestep -- Simulate this object for time dt                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/16/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void HumanPhysClass::Timestep(float dt)
{
	VERBOSE_LOG(("HumanPhys::Timestep\r\n"));
	bool was_on_ground = OnGround;	

	if (Is_Asleep()) {
		if ((Controller != NULL) && (!Controller->Is_Inactive())) {
			Set_Flag(ASLEEP,false);
		}
	}


	Phys3Class::Timestep(dt);

	if (Is_Asleep()) {
		return;
	}


	{
		WWPROFILE("HumanPhys::Timestep");
	
		// if we didn't jump, we're not on the ground, we were on the ground, and our Z velocity is >0, set it to 0
		if (was_on_ground && !OnGround && !JustJumped && (State.Velocity.Z > 0.0f)) {
			State.Velocity.Z = 0.0f;
		}
		
		// clear the just jumped flag if its set and we've reached the apex.
		if (JustJumped && State.Velocity.Z < 0.0f) {
			JustJumped = false;
		}
	}
}


/***********************************************************************************************
 * HumanPhysClass::Render -- Render this object                                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
void HumanPhysClass::Render(RenderInfoClass & rinfo)
{
#ifdef WWDEBUG
	PhysicsSceneClass::Get_Instance()->Debug_Display_Dynamic_Vis_Node(VisNodeID);
#endif
	Phys3Class::Render(rinfo);
}


/***********************************************************************************************
 * HumanPhysClass::Check_Ground -- check the ground state                                      *
 *                                                                                             *
 *    Humans override this so that they don't stick to the ground when they want to jump       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/16/2000  gth : Created.                                                                 *
 *   7/20/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void HumanPhysClass::Check_Ground(const AABoxClass & box,GroundStateStruct * gs,float check_dist)
{
	if (JustJumped) {
		gs->OnGround = false;
	} else {
		Phys3Class::Check_Ground(box,gs,check_dist);
	}
}


/***********************************************************************************************
 * HumanPhysClass::Ballistic_Move -- ballistic motion                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/16/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool HumanPhysClass::Ballistic_Move(float dt)
{		
	WWPROFILE("HumanPhys::Ballistic_Move");
	VERBOSE_LOG(("HumanPhys::Ballistic_Move\r\n"));

	// Compute a move vector for the object flying through the air...
	Vector3 move;
	float accel = PhysicsConstants::GravityAcceleration.Z * GravScale;
	
	Vector3 start_vel = State.Velocity;
	Vector3 start_pos = State.Position;

	move.X = State.Velocity.X * dt;
	move.Y = State.Velocity.Y * dt;
	move.Z = 0.5f * accel * dt * dt + State.Velocity.Z * dt;

	bool moved = Apply_Move(move,dt);

	// Compute X,Y velocities from the actual move that was performed
	State.Velocity.X = (State.Position.X - start_pos.X) / dt;
	State.Velocity.Y = (State.Position.Y - start_pos.Y) / dt;

	// Compute the analytical Z velocity and the ad-hoc Z velocity, the
	// more negative one is the one to keep.  What this does is use the 
	// analytical velocity unless the character hits a roof.
	State.Velocity.Z = start_vel.Z + accel * dt;
#if 0
	State.Velocity.Z = min((State.Position.Z - start_pos.Z) / dt,start_vel.Z + accel * dt);
#endif
	
	// Now let the user adjust the movement a little
	if (Controller && !IsAIControlledJump) {
		Vector3 player_move(Controller->Get_Move_Vector());
		player_move.Rotate_Z(Heading);
		State.Velocity.X += AIR_MOVE_SCALE * NormSpeed * player_move.X;
		State.Velocity.Y += AIR_MOVE_SCALE * NormSpeed * player_move.Y;

		Vector3 xy_vel = State.Velocity;
		xy_vel.Z = 0;
		if (xy_vel.Length2() > NormSpeed * NormSpeed) {
			float scale = NormSpeed / xy_vel.Length();
			State.Velocity.X *= scale;
			State.Velocity.Y *= scale;
		}
	}

	IsAIControlledJump &= (Is_In_Contact () == false);
	return moved;
}


/***********************************************************************************************
 * HumanPhysClass::Slide_Move -- Sliding down a slope                                          *
 *                                                                                             *
 *    Humans can jump when sliding down a slope                                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/16/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool HumanPhysClass::Slide_Move(const GroundStateStruct & gs,float dt)
{
	WWPROFILE("HumanPhys::Slide_Move");
	VERBOSE_LOG(("HumanPhys::Slide_Move\r\n"));

	// Compute a move vector which causes the object to slide down the slope...
	Vector3 start_pos = State.Position;
	Vector3 move = NormSpeed * dt * gs.Down;
	if (Controller != NULL) {
		if (Controller->Get_Move_Vector().Z > 0.0f) {
			move.Z += Controller->Get_Move_Vector().Z * dt;
			JustJumped = true;
		}
	}

	bool moved = Apply_Move(move,dt,true,false,true);
	State.Velocity = (State.Position - start_pos)/dt;

	return moved;
}


/***********************************************************************************************
 * HumanPhysClass::Normal_Move -- Moving under user control                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/16/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool HumanPhysClass::Normal_Move(const GroundStateStruct & gs,float dt)
{
	WWPROFILE("HumanPhys::Normal_Move");
	VERBOSE_LOG(("HumanPhys::Normal_Move\r\n"));

	if (Controller == NULL) {
		return false;
	}

	/*
	** STEP ONE: Compute the desired Move.  The desired move is determined by the player's
	** controller setting and the plane that we are currently standing on.
	*/
	Vector3 move;
	Compute_Desired_Move_Vector(gs,dt,&move);

	/*
	** STEP TWO: Try to apply the move and snap back down to the ground.  If we did not
	** jump and we end up on an un-walkable slope, we'll re-wind and clip our original
	** move so that it avoids that slope.
	*/
	Vector3 start_position = State.Position;
	Vector3 start_move = move;

	bool moved = Apply_Move(move,dt,true,!JustJumped);

	if ((moved) && (!JustJumped)) {
		Snap_To_Ground(State.Position - start_position,true);
	}

	/*
	** STEP THREE: If we snapped down onto an un-walkable slope, revert our state,
	** clip the original move against the equivalent wall for this slope, and try
	** one more time.
	*/
	if (moved && !JustJumped && (GroundState.OnGround) && (GroundState.Normal.Z < SlideNormalZ)) {
		
		State.Position = start_position;
		move = start_move;

		Vector3 slope_wall = GroundState.Normal;
		slope_wall.Z = 0.0f;
		slope_wall.Normalize();

		Clip_Move(&slope_wall,1,&move);
		moved = Apply_Move(move,dt,true,!JustJumped);

		if ((moved) && (!JustJumped)) {
			Snap_To_Ground(State.Position - start_position,true);
		}
	}

	/*
	** STEP FOUR: Finally, compute our velocity in case we became airborne
	*/
	State.Velocity = (State.Position - start_position) / dt;
	// BMG Unless they just jumped
	if ( !JustJumped ) {
		State.Velocity.Z = WWMath::Min(State.Velocity.Z,0.0f);	// don't ever let humans launch off slopes
	}


	return moved;
}


/***********************************************************************************************
 * HumanPhysClass::Compute_Desired_Move_Vector -- compute the move vector                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/16/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void HumanPhysClass::Compute_Desired_Move_Vector(const GroundStateStruct & gs,float dt,Vector3 * set_move)
{
	Vector3 move(Controller->Get_Move_Vector());
	
	if (move.Z < 0.0f) {
		move.Z = 0.0f;
	}
	
	float jump = move.Z;
	if (jump > 0.0f) {
		move.Z = 0.0f;
		JustJumped = true;
	}
	
	/*
	** rotate the move vector into "2D" world space
	*/
	move.Rotate_Z(Heading);

	/*
	** rotate the move vector onto the current contact plane
	*/
	Vector3 axis;
	Vector3::Cross_Product(Vector3(0,0,1),gs.Normal,&axis);
	float axis_len = axis.Length2();

	if (axis_len > 0.0f) {
		float s_angle = axis.Length();
		float c_angle = Vector3::Dot_Product(Vector3(0,0,1),gs.Normal);
		axis.Normalize();
		Matrix3 rotation(axis,s_angle,c_angle);
		Matrix3::Rotate_Vector(rotation,move,&move);	
	}

	/*
	** check the result to ensure it is 90deg from the plane normal (moving on plane)
	*/
#ifdef WWDEBUG
	Vector3 movedir = move;
	movedir.Normalize();
	WWASSERT(fabs(Vector3::Dot_Product(movedir,gs.Normal)) < WWMATH_EPSILON);
#endif

	/*
	** adjust the vector to make uphill slower and downhill faster
	*/
	float dot = Vector3::Dot_Product(move,gs.Down);
	float scale = (1.0f - gs.Normal.Z) / (1.0f - SlideNormalZ);
	move = move + SLOPE_SPEED_REDUCTION * scale * dot * move;

	/*
	** Multiply by the human velocity
	*/
	move *= NormSpeed;
	if (jump > 0.0f) {
		move.Z = jump;
	}
	move *= dt;

	/*
	** done!
	*/
	*set_move = move;
}



/***********************************************************************************************
 * HumanPhysClass::Get_Factory -- returns the PersistFactory for save-load support             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/16/2000  gth : Created.                                                                 *
 *=============================================================================================*/
const PersistFactoryClass & HumanPhysClass::Get_Factory(void) const
{
	return _HumanPhysFactory;
}


/***********************************************************************************************
 * HumanPhysClass::Save -- Save this object                                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/16/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool HumanPhysClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(HUMANPHYS_CHUNK_PHYS3);
	Phys3Class::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(HUMANPHYS_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,HUMANPHYS_VARIABLE_JUSTJUMPED,JustJumped);
	csave.End_Chunk();

	return true;
}


/***********************************************************************************************
 * HumanPhysClass::Load -- Load this object                                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/16/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool HumanPhysClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		
		switch(cload.Cur_Chunk_ID()) 
		{
			case HUMANPHYS_CHUNK_PHYS3:
				Phys3Class::Load(cload);
				break;

			case HUMANPHYS_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,HUMANPHYS_VARIABLE_JUSTJUMPED,JustJumped);
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
 * HumanPhysClass::Jump_To_Point -- Jump to the given X, Y, Z location.								  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/19/2000  pds : Created.                                                                *
 *=============================================================================================*/
void HumanPhysClass::Jump_To_Point (const Vector3 &dest_pos)
{	
	const float gravity	= PhysicsConstants::GravityAcceleration.Z * Get_Gravity_Multiplier();
	const float minangle	= 0.3491f;	// 20 degrees.
	const float	maxangle	= 1.2217f;	// 70 degrees.

	Vector3 displacement, velocity, acceleration;
	float	  x, y, theta;	
	float	  time;	

	acceleration.Set (0.0f, 0.0f, gravity);
	displacement = (dest_pos - Get_Transform().Get_Translation());
	
	// Calculate a launch/landing angle most appropriate for the jump to be made.
	// Horizontal jumps will use the minimum launch angle, nera vertical jumps will
	// use the maximum launch/landing angle.
	x		= Vector2 (displacement.X, displacement.Y).Length();
	y		= WWMath::Fabs (displacement.Z);
	theta = WWMath::Lerp (minangle, maxangle, y / (x + y));
	
	// If character is jumping up then theta is the landing angle
	// - otherwise theta is the launch angle.
	if (displacement.Z >= 0.0f) {

		// Using standard projectile math, final velocity is given by:
		//
		// vfy = viy + g * t (1)
		//
		// and
		// 
		// vfx = vix = x / t	(2)
		//
		// and
		//
		//	viy = y / t - 0.5 * g * t (3)
		//
		//	where vfy is final vertical velocity, viy is initial vertical velocity, g is acceleration
		// due to gravity, t is time, vfx is final horizontal velocity, vix is initial horizontal
		// velocity, y is vertical displacement.
		//
		// We require at landing time:
		//	tan (theta) = vfy / vfx
		//					= (viy + g * t) / (x / t) (4).
		//
		// Substituting (3) in (4):
		//
		// tan (theta) = (y / t - 0.5 * g * t + g * t) / (x / t).
		//	
		// Rearranging gives:
		//
		// t = sqrt ((2.0f * (-x * tan (theta) - y)) / g).
		time = sqrt (2.0f * ((-x * tanf (theta)) - y) / gravity);

	} else {

		// Using standard projectile math, vertical displacement is given by:
		//
		// y = s * sin(theta) * t + 0.5 * g * t^2 (1)
		//
		// where s is speed, theta is launch angle, g is acceleration due to gravity(-ve),
		// and t is time.
		// 
		// Horizontal displacement is given by:
		//
		// x = s * sin(theta) * t (2)
		//
		// because horizontal velocity is constant.
		// Substituting (2) for t in (1) gives:
		//
		// y = (s * sin(theta) * x / (s * cos(theta))) + 0.5 * g * t^2
		//
		// which simplifies in terms of t to:
		//
		// t = sqrt (2.0 * (y - x * tan(theta)) / g).
		time = sqrt (2.0f * (-y - (x * tanf (theta))) / gravity);
	}

	// Calculate velocity vector using standard projectile math.
	velocity = (displacement / time) - ((0.5f * acceleration) * time);

	//
	//	Set the new velocity
	//
	Set_Velocity(velocity);

	//
	//	Reset some internal states so the physics object knows its
	// in a jumping state.
	//
	Invalidate_Ground_State();
	Set_Flag(ASLEEP,false);
	JustJumped				= true;
	IsAIControlledJump	= true;
	return ;
}


/****************************************************************************************************
**
** HumanPhysDefClass Implementation
**
****************************************************************************************************/

/*
** Persist factory for HumanPhysDefClass
*/
SimplePersistFactoryClass<HumanPhysDefClass,PHYSICS_CHUNKID_HUMANPHYSDEF>	_HumanPhysDefFactory;

/*
** Definition factory for HumanPhysDefClass.  This makes it show up in the editor
*/
DECLARE_DEFINITION_FACTORY(HumanPhysDefClass, CLASSID_HUMANPHYSDEF, "Human") _HumanPhysDefDefFactory;

/*
** Chunk ID's used by HumanPhysDefClass
*/
enum 
{
	HUMANPHYSDEF_CHUNK_PHYS3DEF		= 0x00516000,			// phys3def data (parent class)
};

HumanPhysDefClass::HumanPhysDefClass(void) 
{
}

uint32 HumanPhysDefClass::Get_Class_ID (void) const	
{ 
	return CLASSID_HUMANPHYSDEF; 
}

PersistClass * HumanPhysDefClass::Create(void) const
{
	HumanPhysClass * obj = NEW_REF(HumanPhysClass,());
	obj->Init(*this);
	return obj;
}

const PersistFactoryClass & HumanPhysDefClass::Get_Factory (void) const
{
	return _HumanPhysDefFactory;
}

bool HumanPhysDefClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(HUMANPHYSDEF_CHUNK_PHYS3DEF);
	Phys3DefClass::Save(csave);
	csave.End_Chunk();
	
	// no variables for now
	return true;
}

bool HumanPhysDefClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case HUMANPHYSDEF_CHUNK_PHYS3DEF:
				Phys3DefClass::Load(cload);
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
		}
		cload.Close_Chunk();
	}
	return true;
}


bool HumanPhysDefClass::Is_Type(const char * type_name)
{
	if (openw3d::stricmp(type_name,HumanPhysDefClass::Get_Type_Name()) == 0) {
		return true;
	} else {
		return Phys3DefClass::Is_Type(type_name);
	}
}



