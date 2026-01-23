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
 *                     $Archive:: /Commando/Code/wwphys/phys3.cpp                             $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 1/08/02 5:52p                                               $*
 *                                                                                             *
 *                    $Revision:: 137                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Phys3Class::Phys3Class -- Constructor                                                     *
 *   Phys3Class::Phys3Class -- Constructor - from a Definition                                 *
 *   Phys3Class::~Phys3Class -- Destructor                                                     *
 *   Phys3Class::Get_Bounding_Box -- Returns bounding box of the model                         *
 *   Phys3Class::Get_Transform -- Returns the current transform                                *
 *   Phys3Class::Set_Transform -- Sets the current transform                                   *
 *   Phys3Class::Set_Position -- Sets the position                                             *
 *   Phys3Class::Get_Position -- Returns the current position                                  *
 *   Phys3Class::Set_Heading -- Set the heading of this object                                 *
 *   Phys3Class::Get_Heading -- returns the heading of this object                             *
 *   Phys3Class::Set_Slide_Angle -- Sets the maximum angle this object can climb               *
 *   Phys3Class::Get_Slide_Angle -- returns the slide angle                                    *
 *   Phys3Class::Update_Transform -- Recalculate the transform                                 *
 *   Phys3Class::Set_Model -- Set the model being used                                         *
 *   Phys3Class::Update_Cached_Model_Parameters -- Caches some data related to the model       *
 *   Phys3Class::Apply_Impulse -- Apply an impulse to this object                              *
 *   Phys3Class::Apply_Impulse -- Apply an impulse to this object                              *
 *   Phys3Class::Cast_Ray -- Check a ray for intersection with this object                     *
 *   Phys3Class::Cast_AABox -- Check a swept AABox for intersection with this obj              *
 *   Phys3Class::Cast_OBBox -- Check a swept OBBox for intersection with this obj              *
 *   Phys3Class::Intersection_Test -- Check an AABox for intersection with this object         *
 *   Phys3Class::Intersection_Test -- Check an OBBox for intersection with this object         *
 *   Phys3Class::Intersection_Test -- Check a mesh for intersection with this object           *
 *   Phys3Class::Timestep -- Simulate for time dt                                              *
 *   Phys3Class::Get_Ground_State -- validates and returns the gound state structure           *
 *   Phys3Class::Invalidate_Ground_State -- Marks the ground state as dirty                    *
 *   Phys3Class::Check_Ground -- Update the ground state                                       *
 *   Phys3Class::User_Move -- User Move mode.                                                  *
 *   Phys3Class::Ballistic_Move -- Ballistic motion                                            *
 *   Phys3Class::Slide_Move -- Sliding motion, down a slope                                    *
 *   Phys3Class::Normal_Move -- Normal motion, phys3 does nothing (infinite friction)          *
 *   Phys3Class::Collide_Move -- Move in response to someone colliding with me                 *
 *   Phys3Class::Apply_Move -- Apply a movement vector, constrained by world geometry          *
 *   Phys3Class::Debug_Verify_Position -- Verify that I am in a valid position                 *
 *   Phys3Class::Snap_To_Ground -- Following a move, snap back down to the ground              *
 *   Phys3Class::Clip_Move -- Clip a move vector by all of the active contact normals          *
 *   Phys3Class::Get_Shadow_Blob_Box -- Returns the bounding box to use for blob shadows       *
 *   Phys3Class::Get_Collision_Box -- returns the current world space collision box            *
 *   Phys3Class::Push -- Move in response to an animated object colliding with me              *
 *   Phys3Class::Collide -- Move in response to a collision                                    *
 *   Phys3Class::Can_Teleport -- Checks if this object can occupy the specified position       *
 *   Phys3Class::Can_Teleport_And_Stand -- Checks if this object can occupy the specified pos  *
 *   Phys3Class::Find_Teleport_Location -- Searches for a valid position near the given one    *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "phys3.h"
#include "wwdebug.h"
#include "wwhack.h"
#include "wwprofile.h"
#include "wwphystrig.h"
#include "physcoltest.h"
#include "physinttest.h"
#include "colmath.h"
#include "pscene.h"
#include "physcon.h"
#include "physcontrol.h"
#include "bitstream.h"
#include "persistfactory.h"
#include "simpledefinitionfactory.h"
#include "wwphysids.h"
#include "w3d_file.h"


DECLARE_FORCE_LINK(phys3);
const float STEP_HEIGHT = 0.25f;


#define VERBOSE_LOGGING 0

#if VERBOSE_LOGGING
#define VERBOSE_LOG(x) if (/*WWDEBUG_TRIGGER(WWDEBUG_TRIGGER_GENERIC0) &&*/ (strstr(Model->Get_Name(),"lt"))) { WWDEBUG_SAY(x); }
#else
#define VERBOSE_LOG(x)
#endif


/*
** Phys3 Networking constants:
*/
float Phys3Class::_CorrectionTime = 0.5f;
float Phys3Class::_AllowableError = 0.5f;
float Phys3Class::_PopError = 2.5f;


/*
** Phys3HistoryClass Parameters
** Control the implementation of the phys3 history tracking system with the following 
** parameters.
*/
const int		PHYS3_SNAPSHOT_COUNT = 16;					// must be power of 2!
const float		PHYS3_HISTORY_MIN_TIME = 1.25f;			// seconds of history to store

const int		PHYS3_SNAPSHOT_MASK = PHYS3_SNAPSHOT_COUNT - 1; 
const float		PHYS3_SNAPSHOT_INTERVAL = PHYS3_HISTORY_MIN_TIME / PHYS3_SNAPSHOT_COUNT;

#define PHYS3HISTORY_NO_CORRECTION			0
#define PHYS3HISTORY_ABSOLUTE_CORRECTION	0
#define PHYS3HISTORY_LERP_CORRECTION		1


/**
** Phys3HistoryClass
** This class is used to store a history of the state of a Phys3 object.  The network
** update code uses this history to intelligently update the state of the object when
** a packet is received
*/
class Phys3HistoryClass
{
public:
	Phys3HistoryClass(void);
	~Phys3HistoryClass(void);

	void		Init(const Vector3 & pos,const Vector3 & vel);
	void		Update_History(const Vector3 & pos,const Vector3 & vel, float dt);
	void		Compute_Old_State(float dt,Vector3 * set_pos,Vector3 * set_vel);
	void		Apply_Correction(const Vector3 & pos_correction);

	int		History_Count(void) { return PHYS3_SNAPSHOT_COUNT; }
	Vector3	Get_Historical_Position(int i) { return SnapshotArray[Wrap_Index(HeadIndex + i)].Position; }

	float		Find_Time_Of_Nearest_Point(const Vector3 & pos);
	void		Find_Nearest_Point(const Vector3 & pos,const Vector3 & vel,Vector3 * set_point);

private:

	int		Wrap_Index(int index) { return (index + PHYS3_SNAPSHOT_COUNT) & PHYS3_SNAPSHOT_MASK; }

	class StateSnapshotClass
	{
	public:
		StateSnapshotClass(void) : Position(0,0,0), Velocity(0,0,0), Age(0) { }
		StateSnapshotClass(const Vector3 & pos,const Vector3 & vel, float age) : Position(pos), Velocity(vel), Age(age) { }
		StateSnapshotClass & operator = (const StateSnapshotClass & that) { Position = that.Position; Velocity = that.Velocity; Age = that.Age; return *this; }

		void					Lerp(const StateSnapshotClass & a, const StateSnapshotClass & b, float fraction);
		void					Update_Age(float dt) { Age += dt; }

		Vector3				Position;
		Vector3				Velocity;
		float					Age;
	};

	bool						Initialized;
	StateSnapshotClass *	SnapshotArray;
	int						HeadIndex;			// history buffer is circular, this is the "head"
};

Phys3HistoryClass::Phys3HistoryClass(void) :
	Initialized(false),
	SnapshotArray(NULL),
	HeadIndex(0)
{
	SnapshotArray = new StateSnapshotClass[PHYS3_SNAPSHOT_COUNT];

	SnapshotArray[1].Age = 10000.0f;
}

Phys3HistoryClass::~Phys3HistoryClass(void)
{
	if (SnapshotArray != NULL) {
		delete[] SnapshotArray;
		SnapshotArray = NULL;
	}
}

void Phys3HistoryClass::Init(const Vector3 & pos,const Vector3 & vel)
{
	Initialized = true;
	int next_older_index = Wrap_Index(HeadIndex + 1);
	SnapshotArray[next_older_index].Age = 1000.0f;
	SnapshotArray[next_older_index].Position = pos;
	SnapshotArray[next_older_index].Velocity = vel;
}

void Phys3HistoryClass::Update_History(const Vector3 & pos,const Vector3 & vel, float dt)
{
	int next_older_index = Wrap_Index(HeadIndex + 1);

	/*
	** Special case the first time we insert a state
	*/
	if (SnapshotArray[next_older_index].Age == 0.0f) {
		SnapshotArray[next_older_index].Age = 1000.0f;
		SnapshotArray[next_older_index].Position = pos;
		SnapshotArray[next_older_index].Velocity = vel;
	}

	/*
	** See if enough time has passed so we need to ratchet forward in the circular buffer
	*/
	if (SnapshotArray[next_older_index].Age + dt > PHYS3_SNAPSHOT_INTERVAL) {
		HeadIndex = Wrap_Index(HeadIndex - 1);
		SnapshotArray[HeadIndex].Age = 0;
	}

	SnapshotArray[HeadIndex].Position = pos;
	SnapshotArray[HeadIndex].Velocity = vel;

	/*
	** Add age to all existing snapshots
	*/
	for (int i=0; i<PHYS3_SNAPSHOT_COUNT; i++) {
		if (i != HeadIndex) {
			SnapshotArray[i].Update_Age(dt);
		}
	}
}

void Phys3HistoryClass::Compute_Old_State(float t,Vector3 * set_pos,Vector3 * set_vel)
{
	int index = HeadIndex;
	bool done = false;
	
	while (!done) {
		if (SnapshotArray[index].Age <= t) {
			index = Wrap_Index(index + 1);
			
			/*
			** past the end of our history, just return the oldest known state
			*/
			if (index == HeadIndex) {
				int tail_index = Wrap_Index(HeadIndex - 1);
				if (set_pos != NULL) {
					*set_pos = SnapshotArray[tail_index].Position;
				}
				if (set_vel != NULL) {
					*set_vel = SnapshotArray[tail_index].Velocity;
				}
				return;
			}
		
		} else {
			done = true;
		}
	}
	
	int prev_index = Wrap_Index(index - 1);
	int next_index = index;

	StateSnapshotClass lerp;
	float fraction = (t - SnapshotArray[prev_index].Age) / (SnapshotArray[next_index].Age - SnapshotArray[prev_index].Age);
	lerp.Lerp(SnapshotArray[prev_index],SnapshotArray[next_index],fraction);
	if (set_pos != NULL) {
		*set_pos = lerp.Position;
	}
	if (set_vel != NULL) {
		*set_vel = lerp.Velocity;
	}
}

void Phys3HistoryClass::Apply_Correction(const Vector3 & pos_correction)
{
#if PHYS3HISTORY_ABSOLUTE_CORRECTION
	for (int counter=0; counter<PHYS3_SNAPSHOT_COUNT; counter++) {
		int index = Wrap_Index(HeadIndex + counter);
		SnapshotArray[index].Position += pos_correction;
	}
#endif
#if PHYS3HISTORY_LERP_CORRECTION
	for (int counter=0; counter<PHYS3_SNAPSHOT_COUNT; counter++) {
		int index = Wrap_Index(HeadIndex + counter);
		
		float fraction = (PHYS3_HISTORY_MIN_TIME - SnapshotArray[index].Age) / PHYS3_HISTORY_MIN_TIME;
		if (fraction > 0.0f) {
			SnapshotArray[index].Position += fraction * pos_correction;
		}
	}
#endif
}

float Phys3HistoryClass::Find_Time_Of_Nearest_Point(const Vector3 & pos)
{
	/*
	** Find the nearest line segment
	*/
	float min_dist = 10000.0f;
	float time = 0.0f;

	for (int counter=0; counter<PHYS3_SNAPSHOT_COUNT-1; counter++) {
		int index0 = Wrap_Index(HeadIndex + counter);
		int index1 = Wrap_Index(index0 + 1);

		LineSegClass segment(SnapshotArray[index0].Position,SnapshotArray[index1].Position);

		Vector3 point = segment.Find_Point_Closest_To(pos);

		float dist = (point - pos).Length();
		
		if (dist < min_dist) {
			float fraction = (point - segment.Get_P0()).Length() / segment.Get_Length();
			min_dist = dist;
			time = WWMath::Lerp(SnapshotArray[index0].Age, SnapshotArray[index1].Age, fraction);
		}
	}

	/*
	** Return the age of the nearest point
	*/
	return time;
}

void Phys3HistoryClass::Find_Nearest_Point(const Vector3 & pos,const Vector3 & vel,Vector3 * set_point)
{
	/*
	** Find the nearest line segment
	*/
	float min_dist = 10000.0f;

	for (int counter=0; counter<PHYS3_SNAPSHOT_COUNT-1; counter++) {
		int index0 = Wrap_Index(HeadIndex + counter);
		int index1 = Wrap_Index(index0 + 1);

		LineSegClass segment(SnapshotArray[index0].Position,SnapshotArray[index1].Position);

		Vector3 point = segment.Find_Point_Closest_To(pos);
		float dist = (point - pos).Length();

		/*
		** Ignore points with velocity more than 90 deg away from server vel
		*/
		float vdot = 0.0f;
		Vector3 history_vel;
		float fraction = 0.0f;
		if (segment.Get_Length() > 0.0f) {
			fraction = (point - segment.Get_P0()).Length() / segment.Get_Length();
		}
		Vector3::Lerp(SnapshotArray[index0].Velocity,SnapshotArray[index1].Velocity,fraction,&history_vel);
		vdot = Vector3::Dot_Product(vel,history_vel); 
		
		if ((dist < min_dist) && (vdot >= 0.0f)) {
			*set_point = point;
			min_dist = dist;
		}
	}
}


void Phys3HistoryClass::StateSnapshotClass::Lerp(const StateSnapshotClass & a, const StateSnapshotClass & b, float fraction)
{
	Vector3::Lerp(a.Position,b.Position,fraction,&Position);
	Vector3::Lerp(a.Velocity,b.Velocity,fraction,&Velocity);
	Age = WWMath::Lerp(a.Age,b.Age,fraction);
}


/***********************************************************************************************
**
** Phys3Class Implementation
**
***********************************************************************************************/
 
/*
** Declare a PersistFactory for Phys3Classes
*/
SimplePersistFactoryClass<Phys3Class,PHYSICS_CHUNKID_PHYS3>	_Phys3Factory;

/*
** Definition factory for Phys3DefClass.  This makes it creatable in the editor
*/
DECLARE_DEFINITION_FACTORY(Phys3DefClass, CLASSID_PHYS3DEF, "Phys3") _Phys3DefDefFactory;

/*
** Chunk ID's for Phys3Class
*/
enum
{
	PHYS3_CHUNK_MOVEABLEPHYS			= 0x00483200,
	PHYS3_CHUNK_VARIABLES,

	PHYS3_VARIABLE_COLLISION_AABOX	= 0x00,
	PHYS3_VARIABLE_ONGROUND,
	PHYS3_VARIABLE_INCOLLISION,
	PHYS3_VARIABLE_HEADING,
	PHYS3_VARIABLE_NORMSPEED,
	PHYS3_VARIABLE_SLIDEANGLE,
	PHYS3_VARIABLE_SLIDENORMALZ,
	PHYS3_VARIABLE_SLIDEANGLETAN,
	PHYS3_VARIABLE_STEPHEIGHT,
	PHYS3_VARIABLE_MOVEMODE,
	PHYS3_VARIABLE_POSITION,
	PHYS3_VARIABLE_VELOCITY
};


static const float DEFAULT_STEP_HEIGHT = 0.25f;						// the distance an object will "step up" over an obstacle
static const float DEFAULT_SLIDE_ANGLE = DEG_TO_RADF(45.0f);	// steepest angle the character can walk up
static const float DEFAULT_NORMALIZED_SPEED = 10.0f;	 

static const float GROUND_DISTANCE = 0.1f;							// On ground if within this distance
static const float GROUND_EPSILON = (GROUND_DISTANCE) / 5.0f;	// Stop at this distance from ground
static const float WALL_EPSILON = 0.5f;//(GROUND_DISTANCE - 0.001f);	// Stop at this distance from walls/slides

static const float MIN_STEP_MOVE = 0.25f;								// only try to step if we could move this distance
static const float MAX_STEP_MOVE_ANGLE_TAN = 1.0f;					// only step if moving at an angle close to the x-y plane

// Debug Vector colors
static const Vector3 VELOCITY_COLOR(1,0,0);				// color for the velocity debug vector
static const Vector3 CONTACT_COLOR(0.25f,0.7f,0.2f);	// color for contact vectors
static const Vector3 GROUND_COLOR(0.0f,1.0f,1.0f);

static inline void Clip_Move(const Vector3 * contacts,int contact_count,Vector3 * move);



/*************************************************************************************
**
** Phys3Class Implementation
**
*************************************************************************************/


/***********************************************************************************************
 * Phys3Class::Phys3Class -- Constructor                                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/15/2000  gth : Created.                                                                 *
 *=============================================================================================*/
Phys3Class::Phys3Class(void) 
{
	CollisionBox.Center.Set(0,0,1);
	CollisionBox.Extent.Set(1,1,1);
	OnGround = false;
	InCollision = false;
	HeadingChanged = false;
	GroundSurface = 0;
	Heading = 0.0f;
	NormSpeed = DEFAULT_NORMALIZED_SPEED;
	SlideAngle = DEFAULT_SLIDE_ANGLE;
	SlideNormalZ = WWMath::Cos(SlideAngle);
	SlideAngleTan = tan(SlideAngle);
	StepHeight = DEFAULT_STEP_HEIGHT;
	MoveMode = NORMAL_MOVE;
	GroundObject = NULL;
	AnimationMove.Set(0,0,0);
	History = NULL;
	LatencyError.Set(0,0,0);
	LastKnownPosition.Set(0,0,0);
	LastKnownVelocity.Set(0,0,0);
	Invalidate_Ground_State();
}	


/***********************************************************************************************
 * Phys3Class::Init -- initialize from a Definition                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/15/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void Phys3Class::Init(const Phys3DefClass & def)
{
	MoveablePhysClass::Init(def);

	CollisionBox.Center.Set(0,0,1);
	CollisionBox.Extent.Set(1,1,1);
	OnGround = false;
	InCollision = false;
	GroundSurface = 0;
	Heading = 0.0f;
	NormSpeed = def.NormSpeed;
	SlideAngle = def.SlideAngle;
	SlideNormalZ = WWMath::Cos(SlideAngle);
	SlideAngleTan = tan(SlideAngle);
	StepHeight = def.StepHeight;
	MoveMode = NORMAL_MOVE;
	GroundObject = NULL;
	AnimationMove.Set(0,0,0);
	LatencyError.Set(0,0,0);
	LastKnownPosition.Set(0,0,0);
	LastKnownVelocity.Set(0,0,0);

	Update_Cached_Model_Parameters();	
	Invalidate_Ground_State();
}


/***********************************************************************************************
 * Phys3Class::~Phys3Class -- Destructor                                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
Phys3Class::~Phys3Class(void)
{
	if (History != NULL) {
		delete History;
	}
}


/***********************************************************************************************
 * Phys3Class::Get_Bounding_Box -- Returns bounding box of the model                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/15/2000  gth : Created.                                                                 *
 *=============================================================================================*/
const AABoxClass & Phys3Class::Get_Bounding_Box(void) const
{
	assert(Model);
	return Model->Get_Bounding_Box();
}


/***********************************************************************************************
 * Phys3Class::Get_Transform -- Returns the current transform                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/15/2000  gth : Created.                                                                 *
 *=============================================================================================*/
const Matrix3D & Phys3Class::Get_Transform(void) const
{
	assert(Model);
	return Model->Get_Transform();
}


/***********************************************************************************************
 * Phys3Class::Set_Transform -- Sets the current transform                                     *
 *                                                                                             *
 *    Note that this "warps" the object to the specified position.  The user is responsible    *
 *    for providing a valid position.                                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/15/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void Phys3Class::Set_Transform(const Matrix3D & m)
{
	// copy the translation portion of the transform into our state
	m.Get_Translation( &State.Position );

#ifdef WWDEBUG
	if (!State.Position.Is_Valid()) {
		WWDEBUG_SAY(("Phys3Class::Set_Transform got an invalid position: %f, %f, %f\r\n",State.Position.X,State.Position.Y,State.Position.Z));
	}
#endif

	// copy the Z rotation portion of the transform into our heading variable (ugh...)
	Heading = m.Get_Z_Rotation();
	Update_Transform();
	Update_Cull_Box();

	// Wake the object up whenever it moves
	Set_Flag(ASLEEP,false);
	Invalidate_Ground_State();

	Assert_State_Valid();
}


/***********************************************************************************************
 * Phys3Class::Set_Position -- Sets the position                                               *
 *                                                                                             *
 *    This blindly warps the object to the given position.  The user is responsible for        *
 *    providing a valid position.                                                              *
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
void Phys3Class::Set_Position(const Vector3 & position)
{
	State.Position = position;
	VERBOSE_LOG(("Phys3::Set_Position %s, (%f, %f, %f)\r\n\r\n",Model->Get_Name(),position.X,position.Y,position.Z));
	Update_Transform(true);
	Update_Cull_Box();
	Set_Flag(ASLEEP,false);
	Invalidate_Ground_State();
}


/***********************************************************************************************
 * Phys3Class::Get_Position -- Returns the current position                                    *
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
const Vector3 & Phys3Class::Get_Position(void) const
{
	return State.Position;
}


/***********************************************************************************************
 * Phys3Class::Set_Heading -- Set the heading of this object                                   *
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
void Phys3Class::Set_Heading(float heading)
{
	WWASSERT(WWMath::Is_Valid_Float(heading));
	if (heading != Heading) {
		Heading = heading;
		HeadingChanged = true;
	}
	Update_Transform();
}


/***********************************************************************************************
 * Phys3Class::Get_Heading -- returns the heading of this object                               *
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
float Phys3Class::Get_Heading(void) const
{
	return Heading;
}


/***********************************************************************************************
 * Phys3Class::Set_Slide_Angle -- Sets the maximum angle this object can climb                 *
 *                                                                                             *
 * If a phys3 object is resting on a slope greater than this angle, it will slide down the     *
 * slope.                                                                                      *
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
void Phys3Class::Set_Slide_Angle(float angle)
{
	SlideAngle = angle;
	SlideNormalZ = WWMath::Cos(angle);
	SlideAngleTan = tan(angle);
}


/***********************************************************************************************
 * Phys3Class::Get_Slide_Angle -- returns the slide angle                                      *
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
float Phys3Class::Get_Slide_Angle(void) const
{
	return SlideAngle;
}


/***********************************************************************************************
 * Phys3Class::Update_Transform -- Recalculate the transform                                   *
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
void Phys3Class::Update_Transform(bool position_only)
{
	Vector3 pos_dif = Model->Get_Position();
	pos_dif -= State.Position;

	if ((position_only) && (!HeadingChanged)) {
	
		Model->Set_Position(State.Position);
		
	} else {
		
		Matrix3D tm(1);
		tm.Set_Translation(State.Position);
		tm.Rotate_Z(Heading);
		Model->Set_Transform(tm);
	}

	if ( pos_dif.Length2() > WWMATH_EPSILON2 ) {
		Update_Visibility_Status();
	}

	#ifdef WWDEBUG
	for (int j=0;j<3;j++) {

		WWASSERT(WWMath::Is_Valid_Float(State.Position.X));
		WWASSERT(WWMath::Is_Valid_Float(State.Position.Y));
		WWASSERT(WWMath::Is_Valid_Float(State.Position.Z));
		WWASSERT(WWMath::Is_Valid_Float(Heading));
	}
	#endif

	HeadingChanged = false;
}


/***********************************************************************************************
 * Phys3Class::Set_Model -- Set the model being used                                           *
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
void Phys3Class::Set_Model(RenderObjClass * model)
{
	// Let the base class have the model
	MoveablePhysClass::Set_Model(model);

	// update any members that depend on the model
	Update_Cached_Model_Parameters();

	// Update our culling box
	Update_Cull_Box();
}


/***********************************************************************************************
 * Phys3Class::Update_Cached_Model_Parameters -- Caches some data related to the model         *
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
void Phys3Class::Update_Cached_Model_Parameters(void)
{
	// if we don't have a model yet, just return
	if (Model == NULL) {
	
		return;

	} else {

		// Construct our collision box from the model
		RenderObjClass * box = NULL;

		if (Model->Class_ID() == RenderObjClass::CLASSID_DISTLOD) {
			RenderObjClass * lod0 = Model->Get_Sub_Object(0);
			box = lod0->Get_Sub_Object_By_Name("WORLDBOX");
			lod0->Release_Ref();
		} else {
			box = Model->Get_Sub_Object_By_Name("WORLDBOX");
		}
		
		if (box) {

			Matrix3D old_transform = Model->Get_Transform();
			Model->Set_Transform(Matrix3D(1));

			CollisionBox = box->Get_Bounding_Box();
			box->Release_Ref();
			Model->Set_Transform(old_transform);

		}
	}
}


/***********************************************************************************************
 * Phys3Class::Apply_Impulse -- Apply an impulse to this object                                *
 *                                                                                             *
 * As the code comment says... since we changed Phys3 to essentially have infinite friction,   *
 * this function will only work if the object is in the air currently.                         *
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
void Phys3Class::Apply_Impulse(const Vector3 & impulse)
{
	// Impluse applied to center of mass simply adds to the linear momentum
	// NOTE: the current algorithm does not maintain velocity uless undergoing
	// ballistic movement... this routine will often have no effect...
	State.Velocity += impulse * MassInv;
}


/***********************************************************************************************
 * Phys3Class::Apply_Impulse -- Apply an impulse to this object                                *
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
void Phys3Class::Apply_Impulse(const Vector3 & impulse, const Vector3 & /* wpos */)
{
	// Phys3 has only linear momentum so just apply the impulse to the CM...
	Apply_Impulse(impulse);
}


/***********************************************************************************************
 * Phys3Class::Cast_Ray -- Check a ray for intersection with this object                       *
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
bool Phys3Class::Cast_Ray(PhysRayCollisionTestClass & raytest) 
{
	if (Model->Cast_Ray(raytest)) {
		raytest.CollidedPhysObj = this;
		return true;
	}
	return false;
}


/***********************************************************************************************
 * Phys3Class::Cast_AABox -- Check a swept AABox for intersection with this obj                *
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
bool Phys3Class::Cast_AABox(PhysAABoxCollisionTestClass & boxtest)
{
	AABoxClass wrldbox;
	wrldbox.Extent = CollisionBox.Extent;
	wrldbox.Center = State.Position + CollisionBox.Center;

	if (CollisionMath::Collide(boxtest.Box,boxtest.Move,wrldbox,boxtest.Result)) {
		boxtest.CollidedPhysObj = this;
		return true;
	}
	return false;
}


/***********************************************************************************************
 * Phys3Class::Cast_OBBox -- Check a swept OBBox for intersection with this obj                *
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
bool Phys3Class::Cast_OBBox(PhysOBBoxCollisionTestClass & boxtest)
{
	AABoxClass wrldbox;
	wrldbox.Extent = CollisionBox.Extent;
	wrldbox.Center = State.Position + CollisionBox.Center;

	if (CollisionMath::Collide(boxtest.Box,boxtest.Move,wrldbox,Vector3(0,0,0),boxtest.Result)) {
		boxtest.CollidedPhysObj = this;
		return true;
	}
	return false;
}


/***********************************************************************************************
 * Phys3Class::Intersection_Test -- Check an AABox for intersection with this object           *
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
bool Phys3Class::Intersection_Test(PhysAABoxIntersectionTestClass & test)
{
	AABoxClass wrldbox;
	wrldbox.Extent = CollisionBox.Extent;
	wrldbox.Center = State.Position + CollisionBox.Center;

	if (CollisionMath::Intersection_Test(wrldbox,test.Box)) {
		test.Add_Intersected_Object(this);
		return true;
	}
	return false;
}


/***********************************************************************************************
 * Phys3Class::Intersection_Test -- Check an OBBox for intersection with this object           *
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
bool Phys3Class::Intersection_Test(PhysOBBoxIntersectionTestClass & test)
{
	AABoxClass wrldbox;
	wrldbox.Extent = CollisionBox.Extent;
	wrldbox.Center = State.Position + CollisionBox.Center;

	if (CollisionMath::Intersection_Test(wrldbox,test.Box)) {
		test.Add_Intersected_Object(this);
		return true;
	}
	return false;
}


/***********************************************************************************************
 * Phys3Class::Intersection_Test -- Check a mesh for intersection with this object             *
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
bool Phys3Class::Intersection_Test(PhysMeshIntersectionTestClass & test)
{
	AABoxClass wrldbox;
	wrldbox.Extent = CollisionBox.Extent;
	wrldbox.Center = State.Position + CollisionBox.Center;

	AABoxIntersectionTestClass our_test(wrldbox,test.CollisionType);
	if (test.Mesh->Intersect_AABox(our_test)) {
		test.Add_Intersected_Object(this);
		return true;
	}
	return false;
}


/***********************************************************************************
**
**
** Simulation Code
**
**
***********************************************************************************/


/***********************************************************************************************
 * Phys3Class::Timestep -- Simulate for time dt                                                *
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
void Phys3Class::Timestep(float dt)
{	
	WWPROFILE("Phys3::Timestep");
	VERBOSE_LOG(("\r\n***** Phys3::Timestep. %s position: %f %f %f\r\n",Model->Get_Name(),State.Position.X,State.Position.Y,State.Position.Z));

	/*
	** Update our history buffer
	*/
	if (History != NULL) {
		History->Update_History(State.Position,State.Velocity,dt);

		/*
		** Debugging, Draw our history if present
		*/
		Vector3 p0 = State.Position;
		Vector3 p1; 
		
		for (int i=1; i<History->History_Count(); i++) {
			p1 = History->Get_Historical_Position(i);	
			DEBUG_RENDER_VECTOR(p0,p1-p0,VELOCITY_COLOR); 
			p0 = p1;
		}
	}

	/*
	** If there is remaining latency error, correct some of it
	*/
	if (LatencyError.Length2() > 0.001f) {

		Vector3 correction;
		float pop_error2 = _PopError * _PopError;
		float allowable_error2 = _AllowableError * _AllowableError;
		float latency_error2 = LatencyError.Length2();

		if (State.Velocity.Length2() > 1.0f) {
			allowable_error2 = 0.0f;
		}

		if (latency_error2 > pop_error2) {
			
			/*
			** Teleport to the network position:
			*/
			Network_Teleport_Correction();

		} else if (latency_error2 > allowable_error2) {

			/*
			** Try to gracefully correct the error:
			*/
			float frame_time = dt;
			float correction_time = _CorrectionTime;
			float correction_fraction;
		
			if (correction_time <= frame_time) {
				correction_fraction = 1.0f;
			} else {
				correction_fraction = WWMath::Clamp(frame_time / correction_time);
			}

			correction_fraction = WWMath::Clamp(correction_fraction,0.0f,0.5f);
			correction = correction_fraction * LatencyError;

			/*
			** Smoothly correct our position
			*/
			Vector3 start = State.Position;
			this->Apply_Move(correction,1.0f,true,false,false);
			Update_Transform();
			Vector3 delta = State.Position - start;
			History->Apply_Correction(delta);

			/*
			** If the correction completely fails, teleport
			*/
			float correction_len = correction.Length();
			float delta_len = delta.Length();
			if ((correction_len > 0.0f) && (delta_len/correction_len < 0.1f)) {
	
				Network_Teleport_Correction();

			} else {
				LatencyError -= delta;
			}
		}
	}

	/*
	** If we're asleep, short circuit
	*/
	if (Is_Asleep() && !Is_User_Control_Enabled()) {
		return;
	}

	Vector3 start_pos = State.Position;
	Vector3 move;
	AABoxClass box;
	CastResultStruct res;

	GroundStateStruct & gs = Get_Ground_State();

	OnGround = gs.OnGround;
	GroundSurface = gs.SurfaceType;
	WWASSERT(GroundSurface >= 0);
	WWASSERT(GroundSurface < SURFACE_TYPE_MAX);

	bool moved = false;

	if (Is_User_Control_Enabled()) {

		MoveMode = USER_OVERRIDE;
		moved = User_Move(dt);

	} else {

		if (!gs.OnGround) {

			MoveMode = BALLISTIC_MOVE;
			moved = Ballistic_Move(dt);

		} else {
			
			if (gs.Normal.Z < SlideNormalZ) {

				MoveMode = SLIDE_MOVE;
				moved = Slide_Move(gs,dt);

			} else {

				MoveMode = NORMAL_MOVE;
				moved = Normal_Move(gs,dt);			

			}
		}
	}

	
	if ((moved) || (HeadingChanged)) {
		WWPROFILE("Phys3Class::Timestep cleanup");
		Update_Transform(true);
		Update_Cull_Box();
		Add_Animation_Move(State.Position - start_pos);

		if (!OnGround) {
			Link_To_Carrier(NULL);
		}
	}

	if (GroundState.OnDynamicObj) {
		Invalidate_Ground_State();
		Set_Flag(ASLEEP,false);
	}


//	DEBUG_RENDER_VECTOR(State.Position,State.Velocity,VELOCITY_COLOR);

	WWASSERT(WWMath::Is_Valid_Float(State.Position.X));
	WWASSERT(WWMath::Is_Valid_Float(State.Position.Y));
	WWASSERT(WWMath::Is_Valid_Float(State.Position.Z));

	WWASSERT(WWMath::Is_Valid_Float(State.Velocity.X));
	WWASSERT(WWMath::Is_Valid_Float(State.Velocity.Y));
	WWASSERT(WWMath::Is_Valid_Float(State.Velocity.Z));

	VERBOSE_LOG((" ***** Phys3::Timestep ended. Position: %f %f %f\r\n\r\n",State.Position.X,State.Position.Y,State.Position.Z));
}


/***********************************************************************************************
 * Phys3Class::Get_Ground_State -- validates and returns the gound state structure             *
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
Phys3Class::GroundStateStruct & Phys3Class::Get_Ground_State(void)
{
	WWPROFILE("Phys3::Get_Ground_State");
	if (GroundState.IsDirty == true) {
		GroundState.IsDirty = false;
		AABoxClass box;
		Compute_WS_Collision_Box(State,&box);
		Check_Ground(box,&GroundState,GROUND_DISTANCE);
	}

	VERBOSE_LOG((" GS: OnGround = %d Normal = %f %f %f\r\n",GroundState.OnGround,GroundState.Normal.X,GroundState.Normal.Y,GroundState.Normal.Z));

	return GroundState;
}


/***********************************************************************************************
 * Phys3Class::Invalidate_Ground_State -- Marks the ground state as dirty                      *
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
void Phys3Class::Invalidate_Ground_State(void)
{
	GroundState.IsDirty = true;
}


/***********************************************************************************************
 * Phys3Class::Check_Ground -- Update the ground state                                         *
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
void Phys3Class::Check_Ground(const AABoxClass & box,GroundStateStruct * gs,float check_dist)
{
	WWPROFILE("Check_Ground");
	VERBOSE_LOG(("Phys3::Check_Ground\r\n"));
	CastResultStruct result;
	PhysAABoxCollisionTestClass test(	box,
													Vector3(0,0,-check_dist),
													&result,
													Get_Collision_Group(),
													COLLISION_TYPE_PHYSICAL	);

	Inc_Ignore_Counter();
	PhysicsSceneClass::Get_Instance()->Cast_AABox(test);
	Dec_Ignore_Counter();

	if (result.Fraction < 1.0f) {

		gs->Init_From_Collision_Result(test,result.Fraction * check_dist);
		
		if (!result.StartBad) {
			Link_To_Carrier(test.CollidedPhysObj,test.CollidedRenderObj);
			DEBUG_RENDER_VECTOR(State.Position,gs->Normal,GROUND_COLOR);
			VERBOSE_LOG(("  not intersecting\r\n"));
		}

	} else {
	
		gs->OnGround = false;
		gs->SurfaceType = SURFACE_TYPE_DEFAULT;
		VERBOSE_LOG(("  not on ground!\r\n"));
	}

	if (result.StartBad) {
		DEBUG_RENDER_AABOX(box,Vector3(1,0,0),0.25f);
		VERBOSE_LOG(("  INTERSECTING!\r\n"));
	}

}


/***********************************************************************************************
 * Phys3Class::User_Move -- User Move mode.                                                    *
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
bool Phys3Class::User_Move(float dt)
{
	VERBOSE_LOG(("Phys3::User_Move\r\n"));
	if (Controller) {
		
		Vector3 move(Controller->Get_Move_Vector());
		move.Rotate_Z(Heading);

		State.Velocity = NormSpeed * move;
		move = State.Velocity * dt;
		
		return Apply_Move(move,dt);
	}
	return false;
}


/***********************************************************************************************
 * Phys3Class::Ballistic_Move -- Ballistic motion                                              *
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
bool Phys3Class::Ballistic_Move(float dt)
{		
	// Compute a move vector for the object flying through the air...
	WWPROFILE("Phys3::Ballistic_Move");
	VERBOSE_LOG(("Phys3::Ballistic_Move\r\n"));

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
//	if (CollidedThisFrame) {
//		State.Velocity.Z = (State.Position.Z - start_pos.Z) / dt;
//	} else {
		State.Velocity.Z = start_vel.Z + accel * dt;
//	}
	return moved;
}


/***********************************************************************************************
 * Phys3Class::Slide_Move -- Sliding motion, down a slope                                      *
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
bool Phys3Class::Slide_Move(const GroundStateStruct & gs,float dt)
{
	// Compute a move vector which causes the object to slide down the slope...
	WWPROFILE("Phys3::Slide_Move");
	VERBOSE_LOG(("Phys3::Slide_Move\r\n"));

	Vector3 start_pos = State.Position;
	Vector3 move = NormSpeed * dt * gs.Down;

	bool moved = Apply_Move(move,dt,true,false);
	State.Velocity = (State.Position - start_pos) / dt;
	return moved;
}


/***********************************************************************************************
 * Phys3Class::Normal_Move -- Normal motion, phys3 does nothing (infinite grip)                *
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
bool Phys3Class::Normal_Move(const GroundStateStruct & /* gs */,float /* dt */)
{
	WWPROFILE("Phys3::Normal_Move");
	VERBOSE_LOG(("Phys3::Normal_Move\r\n"));
	
	// phys3 doesn't move unless on a slope or falling.
	State.Velocity.Set(0,0,0);
	return false;
}


/***********************************************************************************************
 * Phys3Class::Collide_Move -- Move in response to someone colliding with me                   *
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
bool Phys3Class::Collide_Move(const Vector3 & requested_move,float dt)
{
	WWPROFILE("Phys3::Collide_Move");
	VERBOSE_LOG(("Phys3::Collide_Move\r\n"));

	// if we're already being considered in a collision (collisions looped back to us) then return
	if (InCollision) {
		return false;
	}
	
	InCollision = true;
	MoveMode = COLLIDE_MOVE;
	Vector3 start_position = State.Position;
	bool moved = Apply_Move(requested_move,dt,true,false);
	
	/*
	** if we moved and we started on the ground, try to stick to the ground
	*/
	if (moved && OnGround) {
		Snap_To_Ground(State.Position - start_position,true);
	}
	InCollision = false;

	State.Velocity = (State.Position - start_position) / dt;
	Add_Animation_Move(State.Position - start_position);
	
	Update_Transform(true);
	Update_Cull_Box();

	return moved;
}


/***********************************************************************************************
 * Phys3Class::Apply_Move -- Apply a movement vector, constrained by world geometry            *
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
bool Phys3Class::Apply_Move
(
	const Vector3 & input_move,
	float dt,
	bool allow_sliding,
	bool allow_stepping,
	bool /* stop_on_walkable */
)
{
	WWPROFILE("Phys3::Apply_Move");
	VERBOSE_LOG(("Phys3::Apply_Move: %f %f %f\r\n",input_move.X,input_move.Y,input_move.Z));
	
#ifdef WWDEBUG
	if (!input_move.Is_Valid()) {
		WWDEBUG_SAY(("Phys3::Apply_Move called with invalid move vector: %f, %f, %f\r\n",input_move.X,input_move.Y,input_move.Z));
//		WWASSERT(input_move.Is_Valid());
		return false;
	}
#endif

	/*
	** Early exit if we're not really moving!
	*/
	float move_len = input_move.Length();
	if (move_len < WWMATH_EPSILON) {
		if (GroundState.OnGround) {
			Set_Flag(ASLEEP,true);
		}
		VERBOSE_LOG(("  no motion\r\n"));
		return false;
	} else {
		VERBOSE_LOG(("  applying move (%f,%f,%f)\r\n",input_move.X,input_move.Y,input_move.Z));
		Set_Flag(ASLEEP,false);
		Invalidate_Ground_State();
	}

	bool moved = false;
	bool done = false;
	bool collided = false;

	const int MAX_CONTACTS = 4;
	const int MAX_ITERATIONS = 4;
	
	int total_iterations = 0;
	int contact_count = 0;
	Vector3 contacts[MAX_CONTACTS];
	
	Vector3 start_position = State.Position;
	Vector3 move = input_move;
	Vector3 move_dir = move / move_len;

	bool already_stepped = false;

	Inc_Ignore_Counter();
	
	/*
	** Apply Move:
	** 
	** - WHILE (I still have movement to apply)
	**   - sweep our collision box up to the first collision
	**   - IF (I started out intersecting) give up, done
	**   - ELSE 
	**		 - IF (I was able to take the entire move without collision) accept the move, done 
	**	    - ELSE 
	**       - Accept the portion of the move up to the collision
	**       - Call the collision callbacks
	**       - IF (I have no movement remaining OR I'm not allowed to slide) done
	**       - ELSE 
	**			  - If (I can push the obstacle out of my way) push obstacle, go back to top of loop
	**			  - If (I can shatter this object) shatter it, go back to top of loop
	**			  - If (This contact is a slope, and I'm trying to walk up it) modify contact to act like a wall
	**         - Reflect the movement vector so that it does not violate the current contacts
	**
	** Notes:
	** - Try to stop movement some distance (COLLISION_EPSILON) away from collisions.
	** - When approaching slides (slopes that we'll slide off of), try to treat them like walls.
	** - Can't always treat slides like walls since we can fall onto them or walk onto them from the top...
	** - Trying to get stepping for free usually by just always bumping the character up before taking his move
	**
	*/
	
	while (!done) {
		
		if ( ++total_iterations > MAX_ITERATIONS ) {
			VERBOSE_LOG(( "Phys3 Exceeded max iterations!\r\n" ));
			done = true;
		}

		/*
		** Get the collision box for the object
		*/
		AABoxClass box;
		Compute_WS_Collision_Box(State,&box);

		CastResultStruct result;
		PhysAABoxCollisionTestClass test(	box,
														move,
														&result,
														Get_Collision_Group(),
														COLLISION_TYPE_PHYSICAL		);
		
		/*
		** If we're allowed to step up, try to
		*/
		if (OnGround && allow_stepping && !already_stepped) {		

			already_stepped = true;

			/*
			** Bump the box up Z by STEP_HEIGHT
			*/
			test.Translate(Vector3(0,0,STEP_HEIGHT));

			/*
			** Sweep the box
			*/
			PhysicsSceneClass::Get_Instance()->Cast_AABox(test);
			
			/*
			** IF: the 'step' test did not start out intersecting (no low roof) then
			** warp our position up and continue
			**
			** ELSE: Since the step test started out intersecting we must be under a low
			** roof.  In this case, perform the move from our original position; no
			** stepping over obstacles.
			*/
			if (!result.StartBad) {
				
				State.Position.Z += STEP_HEIGHT;
				moved = true;

			} else {

				result.Reset();
				test.Translate(Vector3(0,0,-STEP_HEIGHT));
				PhysicsSceneClass::Get_Instance()->Cast_AABox(test);
			
			}
		
		} else {

			PhysicsSceneClass::Get_Instance()->Cast_AABox(test);

		}
		
		VERBOSE_LOG(("  Checking collision results:  "));
		if (result.StartBad) {
			//WWDEBUG_WARNING(("result.StartBad\n"));
			//PhysicsSceneClass::Get_Instance()->Add_Debug_AABox(box,Vector3(1,0,0));
			VERBOSE_LOG(("StartBad!\r\n"));
			done = true;
		
		} else {

			if (result.Fraction == 1.0f) {

				/*
				** the move was successful, accept it.
				*/
				VERBOSE_LOG(("Entire move accepted.\r\n"));
				State.Position += move;
	
				done = true;
				moved = true;

			} else {

				VERBOSE_LOG(("Collided with %s. normal=(%f,%f,%f)\r\n",test.CollidedRenderObj->Get_Name(),result.Normal.X,result.Normal.Y,result.Normal.Z));
				collided = true;

				/*
				** How far should we stay away from this obstacle
				*/
				float epsilon = GROUND_EPSILON;
				if (result.Normal.Z < SlideNormalZ) {								// TODO: base this on surface type?
					epsilon = WALL_EPSILON;
				}

				/*
				** Try to move up to the obstacle
				*/
				float move_len = result.Fraction * move.Length();
				if (move_len > epsilon) {
					move_len -= epsilon;
					Vector3 direction = move;
					direction.Normalize();

					State.Position += move_len * direction;

					move -= result.Fraction * move;
					dt -= result.Fraction * dt;
					moved = true;
					contact_count = 0;					// since we moved, reset the contacts
				} 
				
				/*
				** If there is no time remaining, no movement remaining,
				** or we're not allowed to "slide" along contacts then we're done
				*/
				if (	(dt < WWMATH_EPSILON) || 
						(move.Length2() < WWMATH_EPSILON) ||
						(allow_sliding == false) ) 
				{
				
					/*
					** we're close enough to done 
					*/
					done = true;
				
				} else {
		
					/*
					** We hit something, call the collision callbacks.
					*/
					WWASSERT(test.CollidedPhysObj != NULL);
					CollisionReactionType reaction = COLLISION_REACTION_DEFAULT;

					CollisionEventClass event;
					event.OtherObj = test.CollidedPhysObj;
					reaction |= Collision_Occurred(event);
					event.OtherObj = this;
					reaction |= test.CollidedPhysObj->Collision_Occurred(event);
					
					if (reaction & COLLISION_REACTION_NO_BOUNCE) {
		
						/*
						** If we're instructed to not bounce, just erase the remaining time
						*/
						dt = 0.0f;
					
					} else {

						/*
						** assume we're going to have to clip the move
						*/
						bool clip_move = true;
						
						/*
						** PUSH: Try to push the obstacle
						*/
						if (clip_move) {
							Phys3Class * p3obj = test.CollidedPhysObj->As_Phys3Class();

							if (p3obj != NULL) {

								/*
								** Compute a move for the object we're contacting which makes it slide out
								** of our way if we're moving at an angle to it.
								*/
								Vector3 dc;
								Vector3::Subtract(p3obj->Get_Position(),State.Position,&dc);
								dc.Normalize();
								float fraction = 1.0f;
								if ((Mass > WWMATH_EPSILON) && (p3obj->Get_Mass() > WWMATH_EPSILON)) {
									fraction = (Mass / (Mass + p3obj->Get_Mass()));
								}
								dc *= fraction * move.Length();

								Dec_Ignore_Counter();
								if (p3obj->Collide_Move(dc,dt)) {
									clip_move = false;
								}
								Inc_Ignore_Counter();
							}
						}
					
						/*
						** SHATTER: If the mesh is shatterable, shatter it.  We are doing this so that 
						** shatterable meshes are purely cosmetic for multiplay.  
						*/
						MeshClass * mesh = NULL;
						if ((test.CollidedRenderObj != NULL) && (test.CollidedRenderObj->Class_ID() == RenderObjClass::CLASSID_MESH)) {
							mesh = (MeshClass*)test.CollidedRenderObj;
						}

						if	(	(mesh != NULL) && 
								(mesh->Get_W3D_Flags() & W3D_MESH_FLAG_SHATTERABLE) && 
								(mesh->Is_Not_Hidden_At_All())) 
						{

							PhysicsSceneClass::Get_Instance()->Shatter_Mesh(	mesh,
																								State.Position,
																								result.Normal,
																								State.Velocity	);
							if (Observer != NULL) {
								Observer->Object_Shattered_Something(this,test.CollidedPhysObj,result.SurfaceType);
							}

							clip_move = false;
						}


						/*
						** CLIP: If the obstacle didn't move out of our way and we didn't step over it, 
						** add the new contact and adjust our move to not hit it again
						*/
						if (clip_move) {

							if (contact_count < MAX_CONTACTS) {
				
								Vector3 normal = result.Normal;
								if (	(normal.Z < SlideNormalZ) && 
										(normal.Z > 0.0f) && 
										(GroundState.OnGround) && 
										(GroundState.Normal.Z > SlideNormalZ) &&
										(MoveMode == BALLISTIC_MOVE)) 
								{
									normal.Z = 0.0f;
									normal.Normalize();
								}
								contacts[contact_count++] = normal;
								DEBUG_RENDER_VECTOR(State.Position,1.5*normal,CONTACT_COLOR);

							} else {

								VERBOSE_LOG(("exceeded max contacts in Phys3::Apply_Move! position: %f, %f, %f\r\n",State.Position.X,State.Position.Y,State.Position.Z));
								done = true;

							}

							Clip_Move(contacts,contact_count,&move);
						}
					}
				}
			}
		}
	}

	Dec_Ignore_Counter();

	return moved;
}


/***********************************************************************************************
 * Phys3Class::Debug_Verify_Position -- Verify that I am in a valid position                   *
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
inline bool Phys3Class::Debug_Verify_Position(void)
{
#ifdef VERBOSE_LOGGING
	PhysicsSceneClass * scene = PhysicsSceneClass::Get_Instance();

	AABoxClass box;
	Compute_WS_Collision_Box(State,&box);

	NonRefPhysListClass list;

	PhysAABoxIntersectionTestClass test(box,Get_Collision_Group(),COLLISION_TYPE_PHYSICAL,&list);

	Inc_Ignore_Counter();
	bool intersecting = scene->Intersection_Test(test);
	Dec_Ignore_Counter();

	if (intersecting) {
		VERBOSE_LOG(("		%s is intersecting %s!\r\n",Model->Get_Name(),list.Peek_Head()->Peek_Model()->Get_Name()));
	} else {
		VERBOSE_LOG(("    %s not intersecting\r\n",Model->Get_Name()));
	}
	return intersecting;
#else
	return true;
#endif	
}


/***********************************************************************************************
 * Phys3Class::Snap_To_Ground -- Following a move, snap back down to the ground                *
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
void Phys3Class::Snap_To_Ground(const Vector3 & actual_move,bool was_stepping)
{
	if (Get_Flag(ASLEEP)) {
		return;
	}
	
	VERBOSE_LOG(("Phys3::Snap_To_Ground\r\n"));

	/*
	** Call this if you started a timestep on the ground and want to stay on the ground...
	** If we were on the ground, we snap down to the ground to ensure that the object does not
	** fly unrealistically off edges, etc.  The logic works like this:
	** - the object may have to snap at least back down to the same altitude it started at
	** - in addition to this, we want to keep the object "stuck" on slopes up to SlideAngle
	** - so, cast its box down this much:  delta_z + delta_xy * tan(SlideAngle)
	** - if what we hit has a slope that less than SlideAngle, snap down!
	*/
	float delta_z = actual_move.Z;
	float delta_xy = WWMath::Sqrt(actual_move.X * actual_move.X + actual_move.Y * actual_move.Y);

	/*
	** If we don't need to snap down, just return
	*/
	if (delta_z + delta_xy > 0.0f) { 
	
		float snap_dist = delta_z + delta_xy * SlideAngleTan + GROUND_DISTANCE;
		
		Inc_Ignore_Counter();

		AABoxClass box;
		Compute_WS_Collision_Box(State,&box);
		CastResultStruct result;
		PhysAABoxCollisionTestClass test(box,Vector3(0,0,-snap_dist),&result,Get_Collision_Group());
		PhysicsSceneClass::Get_Instance()->Cast_AABox(test);

		if ((result.Fraction > 0.0f) && (result.Fraction <= 1.0f) && (!result.StartBad)) {
			
			float snap_down = 0;
			if (result.Normal.Z > SlideNormalZ) {

				/*
				** Case 1: ground is within snap distance and that ground is a walkable slope
				** Snap down to within GROUND_EPSILON of the ground.
				*/
				snap_down = snap_dist * result.Fraction;
				if (snap_down > GROUND_EPSILON) {
					State.Position.Z -= snap_down - GROUND_EPSILON;
				}
				OnGround = true;
				Link_To_Carrier(test.CollidedPhysObj,test.CollidedRenderObj);

				DEBUG_RENDER_VECTOR(State.Position,result.Normal,GROUND_COLOR);
				VERBOSE_LOG(("  now above walkable slope, snapped down %f\r\n",snap_down - GROUND_EPSILON));

			} else {

				/*
				** Case 2: ground is within snap distance but it is not a walkable slope
				**	If we were stepping, we should snap down STEP_HEIGHT if we can.
				*/
				OnGround = false;
				if (was_stepping) {
					// (gth) just let the character snap down to slopes too!
					//snap_down = WWMath::Min(STEP_HEIGHT,snap_dist * result.Fraction);
					if (snap_down > GROUND_EPSILON) {
						State.Position.Z -= snap_down - GROUND_EPSILON;
					}
				}
				
				Link_To_Carrier(NULL);

				VERBOSE_LOG(("  now above non-walkable slope, snapped only up to step-height %f\r\n",snap_down-GROUND_EPSILON));
			}
		
		} else {
			
			/*
			** Case 3: ground is not within snap distance.
			** If we were stepping, snap back down the step distance.
			*/
			OnGround = false;
			if (was_stepping) {
				State.Position.Z -= STEP_HEIGHT;
			}

			VERBOSE_LOG(("  now airborne\r\n"));
		}

		/*
		** Update the GroundState
		*/
		GroundState.Init_From_Collision_Result(test,GROUND_EPSILON);

		Dec_Ignore_Counter();
	}
}


/***********************************************************************************************
 * Phys3Class::Clip_Move -- Clip a move vector by all of the active contact normals            *
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
void Phys3Class::Clip_Move(const Vector3 * contacts,int contact_count,Vector3 * move)
{
	WWASSERT(contacts != NULL);
	VERBOSE_LOG(("Phys3::Clip_Move\r\n"));

#ifdef WWDEBUG
	float move_len2 = move->Length2();
#endif
	
	// Loop over all of the contacts.
	//	On each one we modify our move vector to parallel that plane.  
	// Immediately after each modification, we check if the other planes are a problem.  
	// At the first time all planes are satisfied, we're done
	for (int i=0; i<contact_count; i++) {
		
#ifdef WWDEBUG
		if (fabs(contacts[i].Length() - 1.0f) > WWMATH_EPSILON) {
			WWDEBUG_SAY(("Bad Contact Normal: %f %f %f  Length = %f\r\n",contacts[i].X,contacts[i].Y,contacts[i].Z,contacts[i].Length()));
		}
#endif
		if (!( fabs(contacts[i].Length() - 1.0f) < WWMATH_EPSILON )) {
			*move = Vector3(0,0,0);
			return;
		}

		VERBOSE_LOG(("  normal[%d] = %f %f %f\r\n",i,contacts[i].X,contacts[i].Y,contacts[i].Z));
		
		// push the velocity a little bit away from the plane
		float dot = Vector3::Dot_Product(*move,contacts[i]);
		if (dot < 0.0f) {
			Vector3 adjustment = 1.01f * dot * contacts[i];
			*move -= adjustment;
			//WWASSERT(Vector3::Dot_Product(*move,contacts[i]) >= 0.0f);
		}
		
		int j;
		for (j=0; j<contact_count; j++) {
			float check = Vector3::Dot_Product(*move,contacts[j]);
			if (check < 0.0f) {
				break;	// this contact isn't happy yet... keep choppin.
			}
		}
		if (j == contact_count) {
			break;	// all contacts are happy!
		}
	}

#ifdef WWDEBUG
	float new_move_len2 = move->Length2();
	WWASSERT(new_move_len2 <= move_len2 + WWMATH_EPSILON);
#endif
}


/***********************************************************************************************
 * Phys3Class::Get_Shadow_Blob_Box -- Returns the bounding box to use for blob shadows         *
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
void Phys3Class::Get_Shadow_Blob_Box(AABoxClass * set_obj_space_box)
{
	WWASSERT(set_obj_space_box != NULL);
	*set_obj_space_box = CollisionBox;
}


/***********************************************************************************************
 * Phys3Class::Get_Collision_Box -- returns the current world space collision box              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/2001   gth : Created.                                                                 *
 *=============================================================================================*/
void Phys3Class::Get_Collision_Box(AABoxClass * set_box)
{
	WWASSERT(set_box != NULL);
	Compute_WS_Collision_Box(State,set_box);
}

/***********************************************************************************************
 * Phys3Class::Push -- Move in response to an animated object colliding with me                *
 *                                                                                             *
 *    This function is called when StaticAnimPhys objects detect a collision.  The key is that *
 *    we don't want to slide along any contacts; just move as far as we can along the given    *
 *    move vector.                                                                             *
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
bool Phys3Class::Push(const Vector3 & move)
{
	Vector3 old_pos = State.Position;
	Apply_Move(move,1.0f,false,false);
	Update_Transform(true);
	Update_Cull_Box();

	Vector3 error = (State.Position - old_pos) - move;

#if VERBOSE_LOGGING
	Vector3 actual_move = State.Position - old_pos;
	bool success = (State.Position - old_pos) == move;
	VERBOSE_LOG((" Phys3::Push. Apparent success: %d Error: %f %f %f\r\n",success, error.X,error.Y,error.Z));
#endif

 	return (error.Length2() < WWMATH_EPSILON * WWMATH_EPSILON);
}


/***********************************************************************************************
 * Phys3Class::Collide -- Move in response to a collision                                      *
 *                                                                                             *
 *    This function is similar to the 'Push' function, however the phys3 object is allowed to  *
 *    slide along geometry in order to get out of the way.                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/2001   gth : Created.                                                                 *
 *=============================================================================================*/
bool Phys3Class::Collide(const Vector3 & move)
{
	Vector3 old_pos = State.Position;
	Collide_Move(move,1.0f);
	return ((State.Position - old_pos) == move);
}


/***********************************************************************************************
 * Phys3Class::Can_Teleport -- Checks if this object can occupy the specified position         *
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
bool Phys3Class::Can_Teleport(const Matrix3D &test_tm, bool check_dyn_only, NonRefPhysListClass * result_list)
{
	//
	//	Determine what the world-space collision box for
	// this object would be at the given transform.
	//
	StateStruct test_state;
	test_state.Position.Set (test_tm.Get_Translation ());
	test_state.Velocity.Set (0, 0, 0);
	
	AABoxClass collision_box;
	Compute_WS_Collision_Box (test_state, &collision_box);

	//
	//	Intersect the box with the world.
	//
	PhysAABoxIntersectionTestClass test(collision_box,
													Get_Collision_Group(),
													COLLISION_TYPE_PHYSICAL,
													result_list);

	if (check_dyn_only) {
		test.CheckStaticObjs = false;
	}
	
	Inc_Ignore_Counter ();
	bool intersect = PhysicsSceneClass::Get_Instance ()->Intersection_Test(test);
	Dec_Ignore_Counter ();

	return (intersect == false);
}


/***********************************************************************************************
 * Phys3Class::Can_Teleport_And_Stand -- Checks if this object can occupy the specified pos    *
 *                                                                                             *
 * Also returns an alternate position if the object cannot occupy the specified position.      *
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
bool Phys3Class::Can_Teleport_And_Stand(const Matrix3D &test_tm, Matrix3D *new_tm)
{
	//
	//	Determine what the world-space collision box for
	// this object would be at the given transform.
	//
	StateStruct test_state;
	test_state.Position.Set (test_tm.Get_Translation ());
	test_state.Velocity.Set (0, 0, 0);
	
	AABoxClass collision_box;
	Compute_WS_Collision_Box (test_state, &collision_box);

	Inc_Ignore_Counter ();

	//
	//	Cast a box 'down' from this location.
	//	This checks to make sure it doesn't intersect anything at its
	// starting position, and its not off the edge of a cliff
	//
	CastResultStruct result;
	Vector3 move_vector (0, 0, -5.0F);
	PhysAABoxCollisionTestClass test(	collision_box,
													move_vector,
													&result,
													0,
													COLLISION_TYPE_PHYSICAL);

	PhysicsSceneClass::Get_Instance ()->Cast_AABox (test);

	Dec_Ignore_Counter ();

	//
	//	Did the box land on a suface we can stand on?
	//
	bool retval = false;
	if ((result.StartBad == false) && ((result.Fraction == 1.0F) || (result.Normal.Z >= SlideNormalZ))) {
				
		//
		//	Calculate a new transform for the object
		//
		Vector3 new_pos = test_tm.Get_Translation () + (result.Fraction * move_vector);
		(*new_tm) = test_tm;
		new_tm->Set_Translation (new_pos);

		retval = true;
	}
	
	return retval;
}


/***********************************************************************************************
 * Phys3Class::Find_Teleport_Location -- Searches for a valid position near the given one      *
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
bool Phys3Class::Find_Teleport_Location
(
	const Vector3 &	start,
	float					radius,
	Vector3 *			out
)
{
	bool retval = false;

	//
	//	Test the actual point the caller passed to us.
	//
	Matrix3D initial_tm (start);
	if (Can_Teleport_And_Stand (initial_tm, &initial_tm)) {
		(*out) = initial_tm.Get_Translation ();
		return true;
	}

	const int MAX_ATTEMPTS	= 10;
	const float MIN_DIST		= 0.1F;
	
	//
	//	Try a number of times to find a valid location
	//
	for (int index = 0; !retval && index < MAX_ATTEMPTS; index ++) {
		
		//
		//	Get a random distance and direction to make the attempt in
		//
		float dist		= WWMath::Random_Float (MIN_DIST, radius);
		float angle		= WWMath::Random_Float (0, DEG_TO_RADF (360));
		float z_delta	= dist * SlideNormalZ;
		
		//
		//	Calculate what the new transform would be at this location
		//
		Vector3 test_pos;
		test_pos.X = start.X + WWMath::Cos (angle) * dist;
		test_pos.Y = start.Y + WWMath::Sin (angle) * dist;
		test_pos.Z = start.Z + z_delta;
		Matrix3D test_tm (test_pos);

		//
		//	Do the test to make sure this is a valid teleport location.
		//
		Matrix3D new_tm (1);
		if (Can_Teleport_And_Stand (test_tm, &new_tm)) {
			(*out) = new_tm.Get_Translation ();
			retval = true;
		}
	}

	return retval;
}

bool Phys3Class::Can_Move_To(const Matrix3D &new_tm)
{
	// 
	// Do a sweep from our current position to the new position.
	// 
	Vector3 start_position = Get_Position();
	Vector3 new_position = new_tm.Get_Translation();
	Vector3 move = new_position - start_position;

	AABoxClass box;
	Compute_WS_Collision_Box(State,&box);

	Inc_Ignore_Counter();
	CastResultStruct result;
	PhysAABoxCollisionTestClass test(	box,
													move,
													&result,
													0,
													COLLISION_TYPE_PHYSICAL		);
	PhysicsSceneClass::Get_Instance()->Cast_AABox(test);
	Dec_Ignore_Counter();

	return (result.Fraction == 1.0f);
}



void Phys3Class::Assert_State_Valid(void)
{
	AABoxClass box;
	Compute_WS_Collision_Box(State,&box);

	CastResultStruct result;
	PhysAABoxCollisionTestClass test(	box,
													Vector3(0,0,0),
													&result,
													0,
													COLLISION_TYPE_PHYSICAL		);

	Inc_Ignore_Counter();
	PhysicsSceneClass::Get_Instance()->Cast_AABox(test);
	Dec_Ignore_Counter();
	
#if VERBOSE_LOGGING
	if (result.StartBad) {
		VERBOSE_LOG(("Phys3 Intersection! ModelName = %s\r\n",Model->Get_Name()));
	}
#endif

	WWASSERT(State.Position.Is_Valid());
	WWASSERT(State.Velocity.Is_Valid());


}

void Phys3Class::Network_State_Update(const Vector3 & pos,const Vector3 & vel)
{
	Vector3 delta = pos - State.Position;
	Set_Position(pos);
	Set_Velocity(vel);
	Snap_To_Ground(delta,false);
	Update_Transform( true );
}


void Phys3Class::Network_Latency_State_Update(const Vector3 & net_pos,const Vector3 & net_vel)
{
	DEBUG_RENDER_AABOX(AABoxClass(CollisionBox.Center + net_pos,CollisionBox.Extent),Vector3(0,1,0),0.25f);

	/*
	** Allocate a history object if needed
	*/
	if (History == NULL) {
		History = new Phys3HistoryClass;
		History->Init(net_pos,net_vel);
	}
	WWASSERT(History != NULL);

	/*
	** Search our history to find the point nearest this server update
	*/
	Vector3 old_pos;
	History->Find_Nearest_Point(net_pos,net_vel,&old_pos);
	LatencyError = net_pos - old_pos;
	LastKnownPosition = net_pos;
	LastKnownVelocity = net_vel;
}

void Phys3Class::Network_Teleport_Correction(void)
{
	Vector3 new_pos = LastKnownPosition;
	Vector3 correction = new_pos - State.Position;

	Set_Position(new_pos);
	Set_Velocity(LastKnownVelocity);
#if 0
	History->Apply_Correction(correction);
#else
	History->Init(LastKnownPosition,LastKnownVelocity);
#endif
	LatencyError.Set(0,0,0);
}


/*************************************************************************************
**
**	Phys3Class Save-Load support
**
*************************************************************************************/

const PersistFactoryClass & Phys3Class::Get_Factory (void) const
{
	return _Phys3Factory;
}

bool Phys3Class::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk(PHYS3_CHUNK_MOVEABLEPHYS);
	MoveablePhysClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(PHYS3_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,PHYS3_VARIABLE_COLLISION_AABOX,CollisionBox);
	WRITE_MICRO_CHUNK(csave,PHYS3_VARIABLE_ONGROUND,OnGround);
	WRITE_MICRO_CHUNK(csave,PHYS3_VARIABLE_INCOLLISION,InCollision);
	WRITE_MICRO_CHUNK(csave,PHYS3_VARIABLE_HEADING,Heading);
	WRITE_MICRO_CHUNK(csave,PHYS3_VARIABLE_NORMSPEED,NormSpeed);
	WRITE_MICRO_CHUNK(csave,PHYS3_VARIABLE_SLIDEANGLE,SlideAngle);
	WRITE_MICRO_CHUNK(csave,PHYS3_VARIABLE_SLIDENORMALZ,SlideNormalZ);
	WRITE_MICRO_CHUNK(csave,PHYS3_VARIABLE_SLIDEANGLETAN,SlideAngleTan);
	WRITE_MICRO_CHUNK(csave,PHYS3_VARIABLE_STEPHEIGHT,StepHeight);
	WRITE_MICRO_CHUNK(csave,PHYS3_VARIABLE_MOVEMODE,MoveMode);
	WRITE_MICRO_CHUNK(csave,PHYS3_VARIABLE_POSITION,State.Position);
	WRITE_MICRO_CHUNK(csave,PHYS3_VARIABLE_VELOCITY,State.Velocity);
	csave.End_Chunk();

	return true;
}

bool Phys3Class::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		
		switch(cload.Cur_Chunk_ID()) 
		{
			case PHYS3_CHUNK_MOVEABLEPHYS:
				MoveablePhysClass::Load(cload);
				break;

			case PHYS3_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,PHYS3_VARIABLE_COLLISION_AABOX,CollisionBox);
						READ_MICRO_CHUNK(cload,PHYS3_VARIABLE_ONGROUND,OnGround);
						READ_MICRO_CHUNK(cload,PHYS3_VARIABLE_INCOLLISION,InCollision);
						READ_MICRO_CHUNK(cload,PHYS3_VARIABLE_HEADING,Heading);
						READ_MICRO_CHUNK(cload,PHYS3_VARIABLE_NORMSPEED,NormSpeed);
						READ_MICRO_CHUNK(cload,PHYS3_VARIABLE_SLIDEANGLE,SlideAngle);
						READ_MICRO_CHUNK(cload,PHYS3_VARIABLE_SLIDENORMALZ,SlideNormalZ);
						READ_MICRO_CHUNK(cload,PHYS3_VARIABLE_SLIDEANGLETAN,SlideAngleTan);
						READ_MICRO_CHUNK(cload,PHYS3_VARIABLE_STEPHEIGHT,StepHeight);
						READ_MICRO_CHUNK(cload,PHYS3_VARIABLE_MOVEMODE,MoveMode);
						READ_MICRO_CHUNK(cload,PHYS3_VARIABLE_POSITION,State.Position);
						READ_MICRO_CHUNK(cload,PHYS3_VARIABLE_VELOCITY,State.Velocity);
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

	Invalidate_Ground_State();
	SaveLoadSystemClass::Register_Post_Load_Callback(this);
	return true;
}

void Phys3Class::On_Post_Load (void)
{
	MoveablePhysClass::On_Post_Load();
	Update_Cached_Model_Parameters();
}



/*************************************************************************************
**
** Phys3Class::GroundStateStruct Implementation
**
*************************************************************************************/
Phys3Class::GroundStateStruct::GroundStateStruct(void) : 
	IsDirty(true),
	OnGround(false),
	OnDynamicObj(false),
	SurfaceType(SURFACE_TYPE_DEFAULT),
	Height(0.0f),
	Normal(0,0,1),
	Down(1,0,0),
	GroundObject(NULL),
	GroundRenderObject(NULL)
{
	Reset();
}

void Phys3Class::GroundStateStruct::Reset(void)
{
	IsDirty = true;
	OnGround = false;
	OnDynamicObj = false;
	SurfaceType = SURFACE_TYPE_DEFAULT;
	Height = 0.0f;
	Normal.Set(0,0,1);
	Down.Set(1,0,0);
	GroundObject = NULL; 
	GroundRenderObject = NULL;
}

void Phys3Class::GroundStateStruct::Init_From_Collision_Result(PhysAABoxCollisionTestClass & test,float height)
{
	Reset();
	IsDirty = false;

	if (test.Result->Fraction < 1.0f) {
		OnGround = true;
		SurfaceType = test.Result->SurfaceType;
		Normal = test.Result->Normal;
		GroundObject = test.CollidedPhysObj;
		GroundRenderObject = test.CollidedRenderObj;
		Height = height;

		if (GroundObject != NULL) {
			if (	(GroundObject->As_HumanPhysClass() != NULL) ||
					(GroundObject->As_RigidBodyClass() != NULL))
			{
				OnDynamicObj = true;
			}
		}

		if (test.Result->StartBad) {
			
			IsDirty = true;
			Normal.Set(0,0,1);
//			VERBOSE_LOG(("ERROR - intersecting object: %s!\r\n",GroundRenderObject->Get_Name()));

		} else {

			if (Normal.Length2() <= 0.0f) {
				WWDEBUG_SAY(("ERROR - detected non-unit normal!\r\n"));
			}
			
			// compute the down vector for this plane
			// down = N x -Z x N
			Vector3 tmp;
			Vector3::Cross_Product(Normal,Vector3(0,0,-1),&tmp);
			Vector3::Cross_Product(tmp,Normal,&(Down));	
			Down.Normalize();
			
//			VERBOSE_LOG(("  on ground, normal=(%f,%f,%f)sliding=%d\r\n",gs->Normal.X,gs->Normal.Y,gs->Normal.Z,(gs->Normal.Z>SlideNormalZ ? 0 : 1)));
		}

	} else {
		OnGround = false;
		OnDynamicObj = false;
		GroundObject = NULL;
		GroundRenderObject = NULL;
		SurfaceType = SURFACE_TYPE_DEFAULT;
	}
}


/***********************************************************************************************
**
** Phys3DefClass Implementation
**
***********************************************************************************************/

/*
** Declare a PersistFactory for Phys3DefClasses 
*/
SimplePersistFactoryClass<Phys3DefClass,PHYSICS_CHUNKID_PHYS3DEF>	_Phys3DefFactory;


/*
** Chunk ID's used by MoveablePhysDefClass
*/
enum 
{
	PHYS3DEF_CHUNK_MOVEABLEPHYSDEF				= 0x04486000,			// (parent class)
	PHYS3DEF_CHUNK_VARIABLES,

	PHYS3DEF_VARIABLE_NORMSPEED					= 0x00,
	PHYS3DEF_VARIABLE_SLIDEANGLE,
	PHYS3DEF_VARIABLE_STEPHEIGHT,
};


Phys3DefClass::Phys3DefClass(void) :
	NormSpeed(DEFAULT_NORMALIZED_SPEED),
	SlideAngle(DEFAULT_SLIDE_ANGLE),
	StepHeight(DEFAULT_STEP_HEIGHT)
{
	// make our parameters editable!
	EDITABLE_PARAM(Phys3DefClass, ParameterClass::TYPE_FLOAT, NormSpeed);
	ANGLE_EDITABLE_PARAM(Phys3DefClass, SlideAngle, DEG_TO_RADF(0.0f), DEG_TO_RADF(90.0f));
	FLOAT_EDITABLE_PARAM(Phys3DefClass, StepHeight, 0.0f, 10.0f);	
}


const PersistFactoryClass & Phys3DefClass::Get_Factory (void) const
{
	return _Phys3DefFactory;
}

uint32 Phys3DefClass::Get_Class_ID (void) const	
{ 
	return CLASSID_PHYS3DEF; 
}

PersistClass * Phys3DefClass::Create(void) const
{
	Phys3Class * obj = NEW_REF(Phys3Class,());
	obj->Init(*this);
	return obj;
}

bool Phys3DefClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(PHYS3DEF_CHUNK_MOVEABLEPHYSDEF);
	MoveablePhysDefClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(PHYS3DEF_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,PHYS3DEF_VARIABLE_NORMSPEED,NormSpeed);
	WRITE_MICRO_CHUNK(csave,PHYS3DEF_VARIABLE_SLIDEANGLE,SlideAngle);
	WRITE_MICRO_CHUNK(csave,PHYS3DEF_VARIABLE_STEPHEIGHT,StepHeight);
	csave.End_Chunk();
	return true;
}


bool Phys3DefClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {

		switch(cload.Cur_Chunk_ID()) {

			case PHYS3DEF_CHUNK_MOVEABLEPHYSDEF:
				MoveablePhysDefClass::Load(cload);
				break;

			case PHYS3DEF_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,PHYS3DEF_VARIABLE_NORMSPEED,NormSpeed);
						READ_MICRO_CHUNK(cload,PHYS3DEF_VARIABLE_SLIDEANGLE,SlideAngle);
						READ_MICRO_CHUNK(cload,PHYS3DEF_VARIABLE_STEPHEIGHT,StepHeight);
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

bool Phys3DefClass::Is_Type(const char * type_name)
{
	if (stricmp(type_name,Phys3DefClass::Get_Type_Name()) == 0) {
		return true;
	} else {
		return MoveablePhysDefClass::Is_Type(type_name);
	}
}
