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
 *                     $Archive:: /Commando/Code/wwphys/rbody.cpp                             $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 3/28/02 11:00a                                              $*
 *                                                                                             *
 *                    $Revision:: 119                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   RigidBodyClass::RigidBodyClass -- Constructor for RigidBodyClass                          *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "rbody.h"
#include "pscene.h"
#include "boxrobj.h"
#include "physcoltest.h"
#include "physinttest.h"
#include "physcon.h"
#include "octbox.h"
#include "bitstream.h"
#include "persistfactory.h"
#include "simpledefinitionfactory.h"
#include "wwphysids.h"
#include "wwhack.h"
#include "wwprofile.h"
#include "hlod.h"
#include "physcontrol.h"
#include "phys3.h"
#include <stdio.h>


DECLARE_FORCE_LINK(rbody);


#define RBODY_DEBUGGING					0
#define RBODY_DEBUG_FILTER				(stricmp(Model->Get_Name(),"V_GDI_ORCA_M") == 0) && (PhysicsSceneClass::Get_Instance()->Is_Debug_Display_Enabled())

#define JITTER_ELIMINATION_CODE		0


float RigidBodyClass::_CorrectionTime = 1.0f;

/***********************************************************************************************
**
** RBodyHistoryClass Implementation
**
***********************************************************************************************/
																						
/*
** RBodyHistoryClass Parameters
** Control the implementation of the phys3 history tracking system with the following 
** parameters.
*/
const int		RBODY_SNAPSHOT_COUNT = 16;					// must be power of 2!
const float		RBODY_HISTORY_MIN_TIME = 0.75f;			// seconds of history to store

const int		RBODY_SNAPSHOT_MASK = RBODY_SNAPSHOT_COUNT - 1; 
const float		RBODY_SNAPSHOT_INTERVAL = RBODY_HISTORY_MIN_TIME / RBODY_SNAPSHOT_COUNT;

#define RBODYHISTORY_NO_CORRECTION			0
#define RBODYHISTORY_ABSOLUTE_CORRECTION	0
#define RBODYHISTORY_LERP_CORRECTION		1


/**
** RBodyHistoryClass
** This class is used to store a history of the state of a RBody object.  The network
** update code uses this history to intelligently update the state of the object when
** a packet is received.  
*/
class RBodyHistoryClass
{
public:
	RBodyHistoryClass(void);
	~RBodyHistoryClass(void);

	void		Init(const RigidBodyStateStruct & state);
	void		Update_History(const RigidBodyStateStruct & state, float dt);
	void		Compute_Old_State(float dt,RigidBodyStateStruct * set_state);
	void		Apply_Correction(const RigidBodyStateStruct & error,float fraction);

	int		History_Count(void) { return RBODY_SNAPSHOT_COUNT; }
	const RigidBodyStateStruct & Get_Historical_State(int i) { return SnapshotArray[Wrap_Index(HeadIndex + i)]; }
	const Vector3 & Get_Historical_Position(int i) { return SnapshotArray[Wrap_Index(HeadIndex + i)].Position; }

	void		Find_Nearest_State(const RigidBodyStateStruct & input,RigidBodyStateStruct * output);

private:

	int		Wrap_Index(int index) { return (index + RBODY_SNAPSHOT_COUNT) & RBODY_SNAPSHOT_MASK; }
	
	struct StateSnapshotStruct : public RigidBodyStateStruct
	{
	public:
		StateSnapshotStruct(void) : Age(0) { }
		StateSnapshotStruct(const StateSnapshotStruct & that) : RigidBodyStateStruct(that) { Age = that.Age; }
		StateSnapshotStruct & operator = (const StateSnapshotStruct & that) { RigidBodyStateStruct::operator = (that); Age = that.Age; return *this; }
		StateSnapshotStruct & operator = (const RigidBodyStateStruct & that) { RigidBodyStateStruct::operator = (that); Age = 0.0f; return *this; }

		void					Lerp(const StateSnapshotStruct & a, const StateSnapshotStruct & b, float fraction);
		void					Update_Age(float dt) { Age += dt; }

		float					Age;
	};

	StateSnapshotStruct *	SnapshotArray;
	int							HeadIndex;			// history buffer is circular, this is the "head"
};

RBodyHistoryClass::RBodyHistoryClass(void) :
	SnapshotArray(NULL),
	HeadIndex(0)
{
	SnapshotArray = new StateSnapshotStruct[RBODY_SNAPSHOT_COUNT];
}

RBodyHistoryClass::~RBodyHistoryClass(void)
{
	if (SnapshotArray != NULL) {
		delete[] SnapshotArray;
		SnapshotArray = NULL;
	}
}

void RBodyHistoryClass::Init(const RigidBodyStateStruct & state)
{
	int next_older_index = Wrap_Index(HeadIndex + 1);

	SnapshotArray[HeadIndex] = state;
	SnapshotArray[HeadIndex].Age = 0.0f;
	SnapshotArray[next_older_index] = state;
	SnapshotArray[next_older_index].Age = 1000.0f;
}

void RBodyHistoryClass::Update_History(const RigidBodyStateStruct & state, float dt)
{
	int next_older_index = Wrap_Index(HeadIndex + 1);

	/*
	** See if enough time has passed so we need to ratchet forward in the circular buffer
	*/
	if (SnapshotArray[next_older_index].Age + dt > RBODY_SNAPSHOT_INTERVAL) {
		HeadIndex = Wrap_Index(HeadIndex - 1);
		SnapshotArray[HeadIndex].Age = 0;
	}

	SnapshotArray[HeadIndex] = state;

	/*
	** Add age to all existing snapshots
	*/
	for (int i=0; i<RBODY_SNAPSHOT_COUNT; i++) {
		if (i != HeadIndex) {
			SnapshotArray[i].Update_Age(dt);
		}
	}
}

void RBodyHistoryClass::Compute_Old_State(float t,RigidBodyStateStruct * set_state)
{
	WWASSERT(set_state != NULL);
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
				*set_state = SnapshotArray[tail_index];
				return;
			}
		
		} else {
			done = true;
		}
	}
	
	int prev_index = Wrap_Index(index - 1);
	int next_index = index;

	float fraction = (t - SnapshotArray[prev_index].Age) / (SnapshotArray[next_index].Age - SnapshotArray[prev_index].Age);
	RigidBodyStateStruct::Lerp(SnapshotArray[prev_index],SnapshotArray[next_index],fraction,set_state);
}

void RBodyHistoryClass::Apply_Correction(const RigidBodyStateStruct & error,float fraction)
{
#if (!RBODY_HISTORY_NO_CORRECTION)
	for (int counter=0; counter<RBODY_SNAPSHOT_COUNT; counter++) {
		int index = Wrap_Index(HeadIndex + counter);

		RigidBodyStateStruct ideal_state = SnapshotArray[index];
		ideal_state.Position += error.Position;
		ideal_state.Orientation = ideal_state.Orientation * error.Orientation;
		ideal_state.LMomentum += error.LMomentum;
		ideal_state.AMomentum += error.AMomentum;

		float lerp_fraction = 0.0f;
#if RBODYHISTORY_ABSOLUTE_CORRECTION
		lerp_fraction = fraction;
#endif
#if RBODYHISTORY_LERP_CORRECTION
		lerp_fraction = fraction * (RBODY_HISTORY_MIN_TIME - SnapshotArray[index].Age) / RBODY_HISTORY_MIN_TIME;
#endif
		RigidBodyStateStruct::Lerp(SnapshotArray[index],ideal_state,lerp_fraction,&(SnapshotArray[index]));
	}
#endif
}

void RBodyHistoryClass::Find_Nearest_State(const RigidBodyStateStruct & input,RigidBodyStateStruct * output)
{
	/*
	** Find the nearest line segment
	*/
	float min_dist = 100000.0f;
	float result_fraction = 0.0f;
	int result_index0 = -1;
	int result_index1 = -1;

	for (int counter=0; counter<RBODY_SNAPSHOT_COUNT-1; counter++) {
		int index0 = Wrap_Index(HeadIndex + counter);
		int index1 = Wrap_Index(index0 + 1);

		LineSegClass segment(SnapshotArray[index0].Position,SnapshotArray[index1].Position);

		Vector3 point = segment.Find_Point_Closest_To(input.Position);
		float dist = (point - input.Position).Length();
		float fraction = 0.0f;
		if (segment.Get_Length() > 0.0f) {
			fraction = (point - segment.Get_P0()).Length() / segment.Get_Length();
		}

#if 0
		/*
		** Ignore points with velocity more than 90 deg away from server vel
		*/
		float vdot = 0.0f;
		Vector3 history_vel;
		Vector3::Lerp(SnapshotArray[index0].Velocity,SnapshotArray[index1].Velocity,fraction,&history_vel);
		vdot = Vector3::Dot_Product(vel,history_vel); 
#endif
		
		if ((dist < min_dist) /*&& (vdot >= 0.0f)*/) {
			min_dist = dist;
			result_fraction = fraction;
			result_index0 = index0;
			result_index1 = index1;
		}
	}
	
	WWASSERT((result_index0 >= 0) && (result_index0 < RBODY_SNAPSHOT_COUNT));
	WWASSERT((result_index1 >= 0) && (result_index1 < RBODY_SNAPSHOT_COUNT));
	WWASSERT(output != NULL);
	RigidBodyStateStruct::Lerp(SnapshotArray[result_index0],SnapshotArray[result_index1],result_fraction,output);
}


void RBodyHistoryClass::StateSnapshotStruct::Lerp(const StateSnapshotStruct & a, const StateSnapshotStruct & b, float fraction)
{
	RigidBodyStateStruct::Lerp(a,b,fraction,this);
	Age = WWMath::Lerp(a.Age,b.Age,fraction);
}
																						
																						
																						

				
/***********************************************************************************************
**
** RigidBodyClass Implementation
**
***********************************************************************************************/
float DEFAULT_CONTACT_THICKNESS	= 0.2f;

#define IMPULSE_COLOR		Vector3(1,0,0)
#define LMOMENTUM_COLOR		Vector3(0,1,0)
#define AMOMENTUM_COLOR		Vector3(0,0,1)

												
/*
** Declare a PersistFactory for RigidBodyClasses
*/
SimplePersistFactoryClass<RigidBodyClass,PHYSICS_CHUNKID_RIGIDBODY>	_RigidBodyFactory;


/*
** Chunk ID's used by RigidBodyClass
*/
enum
{
	RBODY_CHUNK_MOVEABLE							= 0x00800900,
	RBODY_CHUNK_VARIABLES,

	RBODY_VARIABLE_ODESYSTEM_PTR				= 0x00,
	RBODY_VARIABLE_IBODY,
	RBODY_VARIABLE_IBODYINV,
	RBODY_VARIABLE_STATE_POSITION,
	RBODY_VARIABLE_STATE_ORIENTATION,
	RBODY_VARIABLE_STATE_LMOMENTUM,
	RBODY_VARIABLE_STATE_AMOMENTUM,
	RBODY_VARIABLE_CONTACT_STIFFNESS,		// OBSOLETE!
	RBODY_VARIABLE_CONTACT_DAMPING,			// OBSOLETE!
	RBODY_VARIABLE_CONTACT_LENGTH,
};										
						

/*
** Rigid Body Dynamics 
** 
** NOTES:
** - No matter what numerical integration technique I use, it seems that the 
**   orientation drifts.  I have to re-normalize frequently so the higher order 
**   integrators are not very attractive (more frequent calls to Compute_Forces...).  
** - Clamping angular velocity to an artificial maximum seems to cause problems 
**   as well.  I'm not sure why but in the test app, it causes "jumpiness" in
**   the simulation.
** - Impact/Contact plan:  Integrate a new state, ignoring collision but computing
**   penalty forces.  Then break that change in state into a change in orientation
**   along with a change in position.  Try the change in orientation, checking for
**   penetration; if penetration occurs, discard the orientation change.  Try
**   the translation, using the swept OBBox.  Compute impact impluse if contact
**   occurs.  When testing for penetration/impact use the normal collision box.
**   When computing penalty/contact forces use a slightly larger box and collect
**   the points inside the box and the box points "behind" the triangles.
**
** May 3, 1999
** - The idea about breaking the state update into separate orientation and translation
**   updates is completely flawed.  It just doesn't work.  Trying to find a new way
**   of detecting collisions without resorting to the binary search through time!
** - Binary searching for the time-of-collision requires a different sort of collision
**   query.  While iterating, it requires a simpler intersection query.  Once finished
**   iterating (found the TOC) it requires a query that will return what point is
**   causing the collision.
**
** June 25, 1999
** - Completely rigid motion may be less efficient in high-poly environments.  For
**   example, a rigid object sliding along the ground will experience a collision (and
**   therefore rewind the integrator) at every single new non-coplanar triangle.
** - On the other hand, the buggy seems to go over curvy surfaces fine since its springs
**   just adjust over time.  Maybe penalty forces are the way after all...  Going to have
**   to try the two-layer box idea...
** - Updating the integrator to a scheme that can handle multiple systems being combined
**   into a single coupled system.  
**
** August 16, 1999
** New Idea: Two-layer box with TOC searching and penalty based contact forces.  General 
** idea is that we let things interpenetrate their outer boxes but not their inner ones.
**
**   pseudo-code:
**   - collect all rigid bodies into a list (later on, we'll break into multiple lists)
**   - integrate them all, computing penalty or real contact forces at each active contact 
**	  - if interpenetration occurs:
**     - search for TOC
**     - rewind (interpolate) all objects to TOC
**     - find the colliding features on the objects involved (edge->edge or vertex->face)
**     - apply collision impulses to the two colliding objects
**     - add contact if resultant relative velocity is less than epsilon
**
**   Derivative calculations contact, constraint forces:
**   - zero each object's force accumulator (or reset it with gravity)
**   - apply all external forces (springs, wind, water)
**   - iterate over all contacts in the sytem, either putting them into a big contact solver
**     or computing a penalty based on their displacement.
**   - add the contact forces to all of the relavent objects
**   - have each object compute its derivatives and add them to the vector
**
**   Finding point of collision:
**   - binary search time until the objects are within some distance of each other
**   - perform an intersection check at time t+dt and record the polygons/boxes that penetrate
**     the outer skin
**   - perform distance calculations on all of those objects (polygons/boxes) at time t
**   - determine if the closest point is caused by a pair of edges or a vertex hitting a face
**   - at the collision point, compute and apply the necessary collision impulse
**   - if the resultant relative velocity is low enough, add a contact constraint/spring to the system
**
** Sept 23, 1999
** Another new idea: Padded boxes.
** - The collision box for an object has four pads on each face (for a total of 24 pads).  Each
**   of these pads can have one contact point.  
** - During each timestep (and sub-timestep) each pad will push out from its face until it hits something
**   or reaches the edge of the outer box.  If it hits something, a contact will be set up which records
**   the position, normal, velocity, and surface parameters.
** - Normally all contacts could behave like damped springs.
** - If the inner box is penetrated, we could search for a time when the outer is penetrated, the inner
**   isn't, and apply impulses to all incoming contact points.
** - Probably need to do some hierarchical culling to the contact checking, try the entire face and
**   if it hits something, then try the four sub-faces.
**
** Oct 14, 1999
** YAHPI	- Yet Another Hacky Physics Idea: Octant Boxes
** - Divide our collision box into 8 octants and detect one contact per octant.  
** - Contact detection will be achieved by using the box-sweeping code on each octant.
** - Octants start out inside the box and sweep along a diagonal some distance
** - At their starting positions these "octant boxes" need to overlap so that when at their
**   fully extended position there aren't gaps around the model.
** ALSO: a key idea is to *never* binary search for the "real" collision.  We just detect
** the collision, revert state, and divide our momentum down in the hopes that the forces
** will handle everything if we come in slower next frame.  This code will need to be improved...
**
** Oct 19, 1999
** Octant boxes are flawed because the contact point for each octant jitters around on the
** object when you're on flat ground.  Since the boxes overlap, the points can all jitter 
** to one side of the CM and then to another side.  This is not good :-)
** Solution: Add contact "hairs" to the corners of the box and "prefer" them whenever the 
** contact "compression" is close to that of its octant box. 
** Remaining/New Problems:
** - Need to compute parameters for these "oct-boxes" automatically which handle any combination
**   of OBBox + Mass
** - Objects can now "tunnel" need to do an extra collision check to prevent this.
** - Need to test the simulation code at the "worst-case" simulation framerate to see if it still
**   stabilizes...
** - Need to handle object-object collisions.  
**
** Oct 26,1999
** - Good success with the octbox + corner contacts idea.  Use both the corner contact and
**   the octant contact when both are present and the octant is closer than the corner.
** - Also still using the "divide down the momentum" approach when there is a collision.  Now
**   the code increases the divisor each frame that the object is stuck.  Not 100% sure if I
**   like this but it is so much faster than doing all of those binary searches through time...
** - Critically damped or overdamped contact springs make the system too stiff.  I use underdamped
**   springs (0.33*critical) to keep everything numerically stable.
** - Had to re-normalize the orientation on every sub-timestep.  It would be nice if we
**   didn't have to do this.  If the guts of the integrator was rotating the orientation
**   instead of adding to it we probably could avoid this.  
** - A first pass at (fake) friction is implemented but commented out.  I'm currently getting 
**   instability if I turn it on and it also never stops the object on slopes, etc.  
** TODO: write a custom integrator which rotates the orientation rather than doing the normal
** state += derivatives*dt...  Is this possible?  It is for Euler integration but we need
** at least MidPoint to keep the simulation stable.
** TODO: when a collision is detected, move the object to the extremeties of its contacts?  This
** should help smooth out collisions?  Will have to recursively try to move any objects in contact
** with this one too...  Probably will need this recursive "fake move" in order to handle obj-obj
** interaction anyway...  Hmmm, this needs more thought.
** TODO: handle obj-obj interaction :-)
**
** Oct 11, 2001
** - Latency handling network code.  Each packet causes us to compute an error between the 
** nearest point in their history; then in the timestep function we try to correct that error.
*/


/***********************************************************************************************
 * RigidBodyClass::RigidBodyClass -- Constructor for RigidBodyClass                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/15/99   gth : Created.                                                                 *
 *=============================================================================================*/
RigidBodyClass::RigidBodyClass(void) : 
	Box(NULL),
	ContactBox(NULL),
	IBody(1),
	IBodyInv(1),
	Rotation(1),
	IInv(1),
	Velocity(0,0,0),
	AngularVelocity(0,0,0),
	ContactCount(0),
	StickCount(0),
	LastTimestep(0.0f),
	GoToSleepTimer(RBODY_SLEEP_DELAY),
	History(NULL)
{
	State.Position.Set(0,0,0);
	State.Orientation.Make_Identity();
	State.LMomentum.Set(0,0,0);
	State.AMomentum.Set(0,0,0);
	
	LatencyError.Position.Set(0,0,0);
	LatencyError.Orientation.Make_Identity();
	LatencyError.LMomentum.Set(0,0,0);
	LatencyError.AMomentum.Set(0,0,0);

	LastKnownState.Position.Set(0,0,0);
	LastKnownState.Orientation.Make_Identity();
	LastKnownState.LMomentum.Set(0,0,0);
	LastKnownState.AMomentum.Set(0,0,0);

	ContactThickness = DEFAULT_CONTACT_THICKNESS;
}

void RigidBodyClass::Init(const RigidBodyDefClass & def)
{
	MoveablePhysClass::Init(def);

	State.Position.Set(0,0,0);
	State.Orientation.Make_Identity();
	State.LMomentum.Set(0,0,0);
	State.AMomentum.Set(0,0,0);

	LatencyError.Position.Set(0,0,0);
	LatencyError.Orientation.Make_Identity();
	LatencyError.LMomentum.Set(0,0,0);
	LatencyError.AMomentum.Set(0,0,0);

	LastKnownState.Position.Set(0,0,0);
	LastKnownState.Orientation.Make_Identity();
	LastKnownState.LMomentum.Set(0,0,0);
	LastKnownState.AMomentum.Set(0,0,0);

	ContactThickness = DEFAULT_CONTACT_THICKNESS;

	GoToSleepTimer = RBODY_SLEEP_DELAY;
}

RigidBodyClass::~RigidBodyClass(void)
{
	REF_PTR_RELEASE(Box);
	if (ContactBox != NULL) {
		delete ContactBox;
		ContactBox = NULL;
	}
}

const AABoxClass & RigidBodyClass::Get_Bounding_Box(void) const
{
	// TODO: propogate this into Phys?
	WWASSERT(Model);
	return Model->Get_Bounding_Box();
}

const Matrix3D & RigidBodyClass::Get_Transform(void) const
{
	// TODO: propogate this into Phys?
	WWASSERT(Model);
	return Model->Get_Transform();
}

void RigidBodyClass::Set_Transform(const Matrix3D & m)
{
	WWASSERT(Model);

	//TODO: rename this to Teleport!
	//TODO: warp model to desired position and verify that it is non-intersecting
	State.Orientation = Build_Quaternion(m);
	State.Position = m.Get_Translation();
	Model->Set_Transform(m);
	Set_Flag(ASLEEP,false);

	// each time the state changes, update the derived values
	Update_Auxiliary_State();
	Update_Cull_Box();
	Update_Visibility_Status();
}

bool RigidBodyClass::Cast_Ray(PhysRayCollisionTestClass & raytest)
{
	if (Model && Model->Cast_Ray(raytest)) {
		raytest.CollidedPhysObj = this;
		return true;
	}
	return false;
}

bool RigidBodyClass::Cast_AABox(PhysAABoxCollisionTestClass & boxtest)
{
	// only let others collide against what we use to collide with them...
	if (CollisionMath::Collide(boxtest.Box,boxtest.Move,ContactBox->Get_World_Inner_Box(),Vector3(0,0,0),boxtest.Result)) {
		boxtest.CollidedPhysObj = this;
		boxtest.CollidedRenderObj = Box;
		return true;
	}
	return false;
}

bool RigidBodyClass::Cast_OBBox(PhysOBBoxCollisionTestClass & boxtest)
{
	// only let others collide against what we use to collide with them...
	if (CollisionMath::Collide(boxtest.Box,boxtest.Move,ContactBox->Get_World_Inner_Box(),Vector3(0,0,0),boxtest.Result)) {
		boxtest.CollidedPhysObj = this;
		boxtest.CollidedRenderObj = Box;
		return true;
	}
	return false;
}


bool RigidBodyClass::Intersection_Test(PhysAABoxIntersectionTestClass & test)
{
	if (CollisionMath::Intersection_Test(ContactBox->Get_World_Inner_Box(),test.Box)) {
		test.Add_Intersected_Object(this);
		return true;
	} 
	return false;
}


bool RigidBodyClass::Intersection_Test(PhysOBBoxIntersectionTestClass & test)
{
	if (CollisionMath::Intersection_Test(ContactBox->Get_World_Inner_Box(),test.Box)) {
		test.Add_Intersected_Object(this);
		return true;
	}
	return false;
}


bool RigidBodyClass::Intersection_Test(PhysMeshIntersectionTestClass & test)
{
	// flip the test around and test our OBBox against the given mesh...
	OBBoxIntersectionTestClass our_test(ContactBox->Get_World_Inner_Box(),test.CollisionType);
	if (test.Mesh->Intersect_OBBox(our_test)) {
		test.Add_Intersected_Object(this);
		return true;
	}
	return false;
}

bool RigidBodyClass::Can_Teleport(const Matrix3D &new_tm, bool check_dyn_only,NonRefPhysListClass * result_list)
{
	bool intersecting = false;

	if (ContactBox) {
		ContactBox->Set_Transform(new_tm);
		intersecting = ContactBox->Is_Intersecting(result_list,!check_dyn_only,true);
		ContactBox->Set_Transform(Get_Transform());

	}
	return !intersecting;
}

bool RigidBodyClass::Can_Teleport_And_Stand(const Matrix3D &new_tm, Matrix3D *out)
{
	*out = new_tm;
	return Can_Teleport(new_tm);
}

void RigidBodyClass::Set_Model(RenderObjClass * model)
{
	MoveablePhysClass::Set_Model(model);
	Update_Cached_Model_Parameters();
}

void RigidBodyClass::Update_Cached_Model_Parameters(void)
{
	// if we don't have a model yet, just return
	if (Model == NULL) return;

	Set_Transform(Model->Get_Transform());

	// cache a pointer to the collision box...
	REF_PTR_RELEASE(Box);
	
	// Try to get the "WorldBox" from the model
	if (Model) {
		RenderObjClass * obj = Model->Get_Sub_Object_By_Name("WorldBox");
		if (obj && obj->Class_ID() == RenderObjClass::CLASSID_OBBOX) {
			REF_PTR_SET(Box,(OBBoxRenderObjClass *)obj);
		}
		REF_PTR_RELEASE(obj);

		// If we didn't finde WorldBox, try to find the LOD named "WorldBox" 
		// The LOD code generates a unique name for the mesh by appending A,B,C, etc to the name.
		// A is the lowest LOD, B is the next, and so on.  Our worldbox is specified in the highest
		// LOD so we have to construct the name by appending 'A'+LodCount to the name... icky
		if ((Box == NULL) && (Model->Class_ID() == RenderObjClass::CLASSID_HLOD)) {
			
			char namebuffer[64];
			sprintf(namebuffer,"WorldBox%c",'A' + ((HLodClass *)Model)->Get_Lod_Count() - 1);

			obj = Model->Get_Sub_Object_By_Name(namebuffer);
			if (obj && obj->Class_ID() == RenderObjClass::CLASSID_OBBOX) {
				REF_PTR_SET(Box,(OBBoxRenderObjClass *)obj);
			}
			REF_PTR_RELEASE(obj);
		}
	}
	
	// Otherwise just create one
	if (Box == NULL) {
		WWDEBUG_SAY(("Missing WorldBox in model: %s\r\n",Model->Get_Name()));
		Box = new OBBoxRenderObjClass(OBBoxClass(Vector3(0,0,0),Vector3(1,1,1)));
		Box->Set_Collision_Type(COLLISION_TYPE_PHYSICAL);
	}

	// Update our contact box
	Matrix3D tm = Get_Transform();
	Model->Set_Transform(Matrix3D(1));

	if (ContactBox != NULL) {
		delete ContactBox;
	}
	ContactBox = new OctBoxClass(*this,Box->Get_Box());
	ContactBox->Set_Thickness(ContactThickness);
	ContactBox->Update_Contact_Parameters();

	Model->Set_Transform(tm);
	ContactBox->Set_Transform(tm);

	// recompute our inertia tensor
	Compute_Inertia();

	// and update our auxiliary state (inertia effects it)
	Update_Auxiliary_State();

	// update our culling box
	Update_Cull_Box();
}

void RigidBodyClass::Get_Velocity(Vector3 * set_vel) const
{
	*set_vel = Velocity;
	Assert_State_Valid();
}

void RigidBodyClass::Get_Angular_Velocity(Vector3 * set_avel) const
{
	*set_avel = AngularVelocity;
	Assert_State_Valid();
}

void RigidBodyClass::Set_Velocity(const Vector3 & newvel)
{
	Assert_State_Valid();

#ifdef WWDEBUG 
	if (newvel.Is_Valid() != true) {
		WWDEBUG_SAY(("someone set an invalid velocity (%8.3f, %8.3f, %8.3f)\r\n",newvel.X,newvel.Y,newvel.Z));
	}
#endif

	Velocity = newvel;
	State.LMomentum = Velocity * Mass;

	Assert_State_Valid();
}

void RigidBodyClass::Set_Angular_Velocity(const Vector3 & newavel)
{
	WWASSERT(WWMath::Is_Valid_Float(newavel.X));
	WWASSERT(WWMath::Is_Valid_Float(newavel.Y));
	WWASSERT(WWMath::Is_Valid_Float(newavel.Z));

#ifdef WWDEBUG 
	if (newavel.Is_Valid() != true) {
		WWDEBUG_SAY(("someone set an invalid angular velocity (%8.3f, %8.3f, %8.3f)\r\n",newavel.X,newavel.Y,newavel.Z));
	}
#endif

	AngularVelocity = newavel;
	Matrix3 I = IInv.Inverse();
	State.AMomentum = I * newavel;
	Assert_State_Valid();
}

void RigidBodyClass::Network_Interpolate_State_Update
(
	const Vector3 & new_pos,
	const Quaternion & new_orientation,
	const Vector3 & new_vel,
	const Vector3 & new_avel,
	float fraction
)
{
	// If we are severely out of sync with the network update, we have to "pop"
	// to the new state.  If needed, we can add more criteria here in the future...
	float characteristic_length2 = 4.0f * ContactBox->Get_Extent_Length2();
	if ((new_pos - State.Position).Length2() > characteristic_length2) {
		fraction = 1.0f;
	}

	// interpolate a new state
	Vector3::Lerp(State.Position,new_pos,fraction,&(State.Position));
	::Fast_Slerp(State.Orientation,State.Orientation,new_orientation,fraction);
	
	Vector3 vel,avel;
	Vector3::Lerp(Velocity,new_vel,fraction,&vel);
	Vector3::Lerp(AngularVelocity,new_avel,fraction,&avel);

	Set_Velocity(vel);
	Set_Angular_Velocity(avel);
	Assert_State_Valid();

	// clear the latency error since we are not using it
	LatencyError.Reset();
	
	// each time the state changes, update the derived values
	Update_Auxiliary_State();
	Model->Set_Transform(Matrix3D(Rotation,State.Position));
	Update_Cull_Box();
	Update_Visibility_Status();
}


void RigidBodyClass::Network_Latency_State_Update
(			
	const Vector3 & new_pos,
	const Quaternion & new_orientation,
	const Vector3 & new_vel,
	const Vector3 & new_avel
)
{
	OBBoxClass debug_box;
	ContactBox->Get_Inner_Box(&debug_box,new_orientation,new_pos);
	DEBUG_RENDER_OBBOX(debug_box,Vector3(1.0f,0,0),0.3f);

	/*
	** Set up the input state
	*/
	LastKnownState.Position = new_pos;
	LastKnownState.Orientation = new_orientation;
	LastKnownState.LMomentum = new_vel * Mass;
	
	Matrix3 I = IInv.Inverse();
	LastKnownState.AMomentum = I * new_avel;

	/*
	** Allocate a history object if needed
	*/
	if (History == NULL) {
		History = new RBodyHistoryClass;
		History->Init(LastKnownState);
	}
	WWASSERT(History != NULL);

	/*
	** Search our history to find the point nearest this server update
	*/
	RigidBodyStateStruct nearest_state;
	History->Find_Nearest_State(LastKnownState,&nearest_state);

	/*
	** Now, compute the correction to apply to our current state
	*/
	LatencyError.Position = LastKnownState.Position - nearest_state.Position;
	LatencyError.Orientation = LastKnownState.Orientation / nearest_state.Orientation;
	LatencyError.LMomentum = LastKnownState.LMomentum - nearest_state.LMomentum;
	LatencyError.AMomentum= LastKnownState.AMomentum - nearest_state.AMomentum;
}


void RigidBodyClass::Network_Latency_Error_Correction(float dt)
{
	if (LatencyError.Position.Length2() > 0.01f) {

#ifdef WWDEBUG
		if (!LatencyError.Is_Valid()) {
			WWDEBUG_SAY(("RigidBodyClass - Invalid Latency Error!\r\n"));
			WWDEBUG_SAY((" position error: %f, %f, %f\r\n",LatencyError.Position.X,LatencyError.Position.Y,LatencyError.Position.Z));
			WWDEBUG_SAY((" orientation error: %f, %f, %f, %f\r\n",LatencyError.Orientation.X, LatencyError.Orientation.Y,LatencyError.Orientation.Z,LatencyError.Orientation.W));
			WWDEBUG_SAY((" L momentum error: %f, %f, %f\r\n",LatencyError.LMomentum.X,LatencyError.LMomentum.Y,LatencyError.LMomentum.Z));
			WWDEBUG_SAY((" A momentum error: %f, %f, %f\r\n",LatencyError.AMomentum.X,LatencyError.AMomentum.Y,LatencyError.AMomentum.Z));
		}
#endif
		if (!LatencyError.Is_Valid()) {
			LatencyError.Reset();
		}

		/*
		** Compute the theoretically fully corrected state
		*/
		RigidBodyStateStruct ideal_state(State);
		ideal_state.Position += LatencyError.Position;
		ideal_state.Orientation = ideal_state.Orientation * LatencyError.Orientation;
		ideal_state.LMomentum += LatencyError.LMomentum;
		ideal_state.AMomentum += LatencyError.AMomentum;

#if 0
	WWDEBUG_SAY(("Rigid Body %s network correction\r\n",Model->Get_Name()));
	WWDEBUG_SAY(("                               position error: %f\r\n",LatencyError.Position.Length()));
	WWDEBUG_SAY(("                               orientation error: %f\r\n",WWMath::Acos(LatencyError.Orientation.W) * 2.0f));
#endif

		/*
		** Decide whether to do the normal smooth correction or to pop
		** We'll pop whenever the position error is large and the 
		** velocity sent from the server is small.
		*/
		const float POP_POSITION_ERROR2 = 3.0f * 3.0f;
		const float POP_MAX_VELOCITY2 = 0.5f * 0.5f;
		const float POP_ANGLE_ERROR = 0.966f;				// cos(theta/2) for 30 degrees

		bool pop = (	(LatencyError.Position.Length2() > POP_POSITION_ERROR2) &&
							((LastKnownState.LMomentum * MassInv).Length2() < POP_MAX_VELOCITY2));

		float cos_half_theta = LatencyError.Orientation.W;

		if (WWMath::Fabs(cos_half_theta) < POP_ANGLE_ERROR) {
			pop = true;			
		}

		if (pop == true) {

			/*
			** Jump to the last given state
			*/
			State = LastKnownState;
			History->Init(LastKnownState);

			/*
			** Clear the latency error
			*/
			LatencyError.Position.Set(0,0,0);
			LatencyError.Orientation.Set(0,0,0);
			LatencyError.LMomentum.Set(0,0,0);
			LatencyError.AMomentum.Set(0,0,0);

		} else {

			/*
			** Blend a fraction of this "ideal" state into the current state
			*/
			float fraction = 0.1f;
			if (_CorrectionTime <= dt) {
				fraction = 1.0f;
			} else {
				fraction = WWMath::Clamp(dt / _CorrectionTime);
			}
			fraction = WWMath::Clamp(fraction,0.0f,0.5f);

			RigidBodyStateStruct::Lerp(State,ideal_state,fraction,&State);
			History->Apply_Correction(LatencyError,fraction);

			/*
			** Recalculate the remaining error
			*/
			LatencyError.Position = ideal_state.Position - State.Position;
			LatencyError.Orientation = ideal_state.Orientation / State.Orientation;
			LatencyError.LMomentum = ideal_state.LMomentum - State.LMomentum;
			LatencyError.AMomentum = ideal_state.AMomentum - State.AMomentum;
		}

		/*
		** each time the state changes, update the derived values
		*/
		Assert_State_Valid();
		Update_Auxiliary_State();
		Model->Set_Transform(Matrix3D(Rotation,State.Position));
		Update_Cull_Box();
		Update_Visibility_Status();
	}
}


void RigidBodyClass::Apply_Impulse(const Vector3 & imp)
{
	// Impluse applied to center of mass simply adds to the linear momentum
	State.LMomentum += imp;
	Velocity = State.LMomentum * MassInv;

	if (Is_Asleep()) {
		Set_Flag(ASLEEP,false);
	}

	Assert_State_Valid();
	DEBUG_RENDER_VECTOR(State.Position,imp,IMPULSE_COLOR);
}

void RigidBodyClass::Apply_Impulse(const Vector3 & imp, const Vector3 & wpos)
{
	// Impluse applied off center, adds to the linear momentum and also
	// adds to the torque.
	Vector3 aimp;
	Vector3::Cross_Product((wpos - State.Position),imp,&aimp);
	
	State.LMomentum += imp;
	State.AMomentum += aimp;

	Velocity = MassInv * State.LMomentum;
	AngularVelocity = IInv * State.AMomentum;

	if (Is_Asleep()) {
		Set_Flag(ASLEEP,false);
	}

	Assert_State_Valid();
#if RBODY_DEBUGGING
	if (RBODY_DEBUG_FILTER) {
		WWDEBUG_SAY(("Impulse applied: %10.10f %10.10f %10.10f\r\n",imp.X,imp.Y,imp.Z));
		DEBUG_RENDER_VECTOR(wpos,imp,IMPULSE_COLOR);
	}
#endif
}

void RigidBodyClass::Set_Mass(float mass)
{
	// In this function, we need to update the inertia tensor and
	// adjust the linear and angular momentum so that the object
	// retains the same velocity and angular velocity.
	Vector3 vel,avel;
	Get_Velocity(&vel);
	Get_Angular_Velocity(&avel);

	MoveablePhysClass::Set_Mass(mass);
	Compute_Inertia();
	Update_Auxiliary_State();

	Set_Velocity(vel);
	Set_Angular_Velocity(avel);

	ContactBox->Update_Contact_Parameters();
	Assert_State_Valid();
}

void RigidBodyClass::Get_Inertia_Inv(Matrix3 * set_I_inv)
{
	*set_I_inv = IInv;
}

void RigidBodyClass::Set_Inertia(const Matrix3 & I)
{
	Vector3 vel,avel;
	Get_Velocity(&vel);
	Get_Angular_Velocity(&avel);

	IBody = I;
	IBodyInv = IBody.Inverse();
	Update_Auxiliary_State();

	Set_Velocity(vel);
	Set_Angular_Velocity(avel);
}

void RigidBodyClass::Get_Inertia(Matrix3 * I)
{
	WWASSERT(I);
	*I = IBody;
}

void RigidBodyClass::Set_Contact_Parameters(float length)
{
	ContactThickness = length;
	ContactBox->Set_Thickness(ContactThickness);
}

void RigidBodyClass::Get_Contact_Parameters(float * stiffness,float * damping,float * length)
{
	if (stiffness) {
		if (ContactBox) {
			*stiffness = ContactBox->Get_Stiffness();
		} else {
			*stiffness = 0.0f;
		}
	}
	if (damping) {
		if (ContactBox) {
			*damping = ContactBox->Get_Damping();
		} else {
			*damping = 0.0f;
		}
	}
	if (length) {
		*length = ContactThickness;
	}
}



int RigidBodyClass::Compute_Derivatives
(
	[[maybe_unused]] float						t,
	StateVectorClass *	test_state,
	StateVectorClass *	dydt,
	int						index
)
{
	if (test_state) {
		Set_State(*test_state,index);
	}

#if RBODY_DEBUGGING
	if (RBODY_DEBUG_FILTER) {
		WWDEBUG_SAY(("  compute_derivatives: t = %f\r\n",t));
		Dump_State();
	}
#endif

	// time derivitive of position
	(*dydt)[index++] = Velocity[0];
	(*dydt)[index++] = Velocity[1];
	(*dydt)[index++] = Velocity[2];

	// time derivitive of orientation
	Quaternion avel(AngularVelocity.X,AngularVelocity.Y,AngularVelocity.Z,0.0f);
	Quaternion qdot = 0.5 * avel * State.Orientation;
	(*dydt)[index++] = qdot[0];
	(*dydt)[index++] = qdot[1];
	(*dydt)[index++] = qdot[2];
	(*dydt)[index++] = qdot[3];

	// time derivitives of momentum and angular momentum (a.k.a. force and torque)
	Vector3 force(0,0,0);
	Vector3 torque(0,0,0);
	Compute_Force_And_Torque(&force,&torque);

	WWASSERT(force.Is_Valid());
	WWASSERT(torque.Is_Valid());
#if RBODY_DEBUGGING
	if (RBODY_DEBUG_FILTER) {
		WWDEBUG_SAY(("Force:		%10.10f , %10.10f , %10.10f\r\n",force[0],force[1],force[2]));
		WWDEBUG_SAY(("Torque:	%10.10f , %10.10f , %10.10f\r\n",torque[0],torque[1],torque[2]));
	}
#endif
	
	(*dydt)[index++] = force[0];
	(*dydt)[index++] = force[1];
	(*dydt)[index++] = force[2];

	(*dydt)[index++] = torque[0];
	(*dydt)[index++] = torque[1];
	(*dydt)[index++] = torque[2];
	
#if RBODY_DEBUGGING
	if (RBODY_DEBUG_FILTER) {
		WWDEBUG_SAY(("  done. \r\n\r\n"));
	}
#endif
	return index;
}

void RigidBodyClass::Get_State(StateVectorClass & set_state)
{
	State.To_Vector(set_state);
}

int RigidBodyClass::Set_State(const StateVectorClass & new_state,int index)
{
	WWPROFILE("RBody::Set_State");
	index = State.From_Vector(new_state,index);
	Update_Auxiliary_State();
	return index;
}

void RigidBodyClass::Set_State(const RigidBodyStateStruct & new_state)
{
	State = new_state;
	Update_Auxiliary_State();
}

void RigidBodyClass::Integrate(float time)
{
	Assert_State_Valid();

	//IntegrationSystem::Euler_Integrate(this,time);
	IntegrationSystem::Midpoint_Integrate(this,time);
	//IntegrationSystem::Runge_Kutta_Integrate(this,time);

	// normalize the orientation since it slowly drifts due to integrator error
	State.Orientation.Normalize();

	Update_Auxiliary_State();
	Assert_State_Valid();
}

void RigidBodyClass::Compute_Inertia(void)
{
	// I'm assuming that the CM is at the origin and the principal axes of inertia
	// are aligned with the coordinate system.  So I'm approximating I by using the 
	// formula for a box which extends both direction in the maximum that the actual
	// box entends in either...
	IBody.Make_Identity();

	if (Box) {
		AABoxClass objbox;
		Box->Get_Obj_Space_Bounding_Box(objbox);
		float dx = 2.0f * objbox.Extent.X + WWMath::Fabs(objbox.Center.X);
		float dy = 2.0f * objbox.Extent.Y + WWMath::Fabs(objbox.Center.Y);
		float dz = 2.0f * objbox.Extent.Z + WWMath::Fabs(objbox.Center.Z);
		IBody[0][0] = (1.0f / 12.0f) * Mass * (dz*dz + dy*dy);
		IBody[1][1] = (1.0f / 12.0f) * Mass * (dx*dx + dz*dz);
		IBody[2][2] = (1.0f / 12.0f) * Mass * (dx*dx + dy*dy);
	}
	IBodyInv = IBody.Inverse();
}

void RigidBodyClass::Update_Auxiliary_State(void)
{
	WWPROFILE("Rbody::Assert_State_Valid");
	Assert_State_Valid();

	State.Orientation.Normalize();

	// compute Rotation,IInv,Velocity,AngularVelocity
	Rotation = Build_Matrix3(State.Orientation);
	IInv = Rotation * IBodyInv * Rotation.Transpose();
	Velocity = MassInv * State.LMomentum;
	AngularVelocity = IInv * State.AMomentum;
	if (ContactBox) {
		ContactBox->Set_Transform(State.Orientation,State.Position);
	}

	Assert_State_Valid();
}

void RigidBodyClass::Compute_Force_And_Torque(Vector3 * force,Vector3 * torque)
{
	WWPROFILE("RigidBodyClass::Compute_Force_And_Torque");

	// NOTE: derived classes should perform their force calculations first and
	// then call us so because contact forces are computed at the end of
	// this routine.
	
	// NOTE: need to optimize this routine!

	// add in gravity
	*force += GravScale * Mass * PhysicsConstants::GravityAcceleration;


	// Add damping
	Vector3 vel_dir = Velocity;
	if (vel_dir.Length2() > WWMATH_EPSILON) {
		vel_dir.Normalize();
		
// DEBUG DEBUG
// FIXME (gth) HACK! zeroing bad velocities.
const float MAX_VEL = 500.0f;
const float MAX_ACCEL = 100.0f;
if (	(Velocity.Is_Valid() == false) || 
		(force->Is_Valid() == false) ||
		(Velocity.Length2() > MAX_VEL * MAX_VEL) ||
		(force->Length() * MassInv > MAX_ACCEL) )
{
	WWDEBUG_SAY(("clearing velocity, model=%s vel=(%f,%f,%f)\r\n",Model->Get_Name(),Velocity.X,Velocity.Y,Velocity.Z));
	Velocity.Set(0,0,0);
	AngularVelocity.Set(0,0,0);
	State.LMomentum.Set(0,0,0);
	State.AMomentum.Set(0,0,0);
	force->Set(0,0,0);
	torque->Set(0,0,0);
	vel_dir.Set(0,0,0);
}

		RigidBodyDefClass * def = Get_RigidBodyDef();
		*force -= def->AerodynamicDragCoefficient * Vector3::Dot_Product(Velocity,Velocity) * vel_dir;

		WWASSERT(force->Is_Valid());
	}

	Vector3 a_dir = AngularVelocity;

// DEBUG DEBUG
// FIXME (gth) HACK! zeroing bad angular velocities.
const float MAX_AVEL = 5.0f * 2.0f * WWMATH_PI;
if (a_dir.Length2() > MAX_AVEL * MAX_AVEL) {
	WWDEBUG_SAY(("clearing angluar velocity, model=%s avel=(%f,%f,%f)\r\n",Model->Get_Name(),AngularVelocity.X,AngularVelocity.Y,AngularVelocity.Z));
	Velocity.Set(0,0,0);
	AngularVelocity.Set(0,0,0);
	State.LMomentum.Set(0,0,0);
	State.AMomentum.Set(0,0,0);
	force->Set(0,0,0);
	torque->Set(0,0,0);
	a_dir.Set(0,0,0);
}

	if (a_dir.Length2() > WWMATH_EPSILON) {
		a_dir.Normalize();
		WWASSERT((a_dir.Length() - 1.0f) < WWMATH_EPSILON);
		*torque -= PhysicsConstants::AngularDamping * Vector3::Dot_Product(AngularVelocity,AngularVelocity) * a_dir;
		WWASSERT(torque->Is_Valid());
	}

	// TODO: boyancy forces, force fields
	// Detect contacts.
	if (Get_RigidBodyDef()->CollisionDisabled == false) {

		ContactBox->Compute_Contacts(false);
		
		for (int i=0; i<ContactBox->Get_Contact_Count(); i++) {

			const CastResultStruct & contact = ContactBox->Get_Contact(i);
			Vector3 r;
			Vector3::Subtract(contact.ContactPoint,State.Position,&r);

			/*
			** Compute Contact Force
			*/
			Vector3 pdot;
			Compute_Point_Velocity(contact.ContactPoint,&pdot);

			float dx = 1.0f - contact.Fraction;
			float dv = Vector3::Dot_Product(pdot,contact.Normal);			
			
			float force_magnitude = ContactBox->Get_Stiffness()*dx - ContactBox->Get_Damping()*dv;
			WWASSERT(WWMath::Is_Valid_Float(force_magnitude));
			Vector3 contact_force = force_magnitude * contact.Normal;
			
			Vector3 contact_torque;
			Vector3::Cross_Product(r,contact_force,&contact_torque);

#ifdef WWDEBUG
			if ((contact_force.Is_Valid() == false) || (contact_torque.Is_Valid() == false)) {
				WWDEBUG_SAY(("bad contact: normal=(%8.3f,%8.3f,%8.3f) r=(%8.3f,%8.3f,%8.3f) dx=%8.3f dv=%8.3f\n",
					contact.Normal.X,contact.Normal.Y,contact.Normal.Z,r.X,r.Y,r.Z,dx,dv));
				contact_force.Set(0,0,0);
				contact_torque.Set(0,0,0);
			}
#endif
	
			/*
			** If we are contacting a phys3 object, push it away from us.  If we
			** can't push it away, then exert a contact force.
			*/
			Phys3Class * p3obj = ContactBox->Peek_Contacted_Object(i)->As_Phys3Class();
			bool resolved = false;
			if (p3obj != NULL) {
				resolved = Push_Phys3_Object_Away(p3obj,contact);
			}
			if (resolved == false) {
				*force += contact_force;
				*torque += contact_torque;
			}

			/*
			** Friction/Drag to bring the object to rest when contacting the ground
			*/
			int contact_count = ContactBox->Get_Contact_Count();
			float contact_weight = Get_Weight() / contact_count;
			Vector3 gravity(0.0f,0.0f,-contact_weight);
			
			if (Get_Flag(FRICTION_DISABLED) == false) {
			
				Vector3 tan_vel = pdot - Vector3::Dot_Product(pdot,contact.Normal)*contact.Normal;
				float tan_vel_magnitude = tan_vel.Length2();

				float friction_coefficient =	PhysicsConstants::Get_Contact_Friction_Coefficient(
														PhysicsConstants::DYNAMIC_OBJ_TYPE_TIRE,
														contact.SurfaceType	);

				if (tan_vel_magnitude > WWMATH_EPSILON * WWMATH_EPSILON) {
					
					/*
					** Friction force points opposite the point's tangential motion
					*/
					tan_vel_magnitude = WWMath::Sqrt(tan_vel_magnitude);
					Vector3 tan_vel_dir = tan_vel / tan_vel_magnitude;
					
					/*
					** The magnitude is the friction coefficient times the portion of the weight supported
					** by this contact.  As the velocity approaches zero, this force approaches zero
					*/
					float friction_magnitude = friction_coefficient * WWMath::Min(0.1f * tan_vel_magnitude * contact_weight,contact_weight);
					Vector3 friction_force = -friction_magnitude * tan_vel_dir;

					/*
					** The active contacts divy up the gravitational force to oppose
					*/
					if (contact.Normal.Z > 0.0f) {
						friction_force -= gravity - Vector3::Dot_Product(gravity,contact.Normal) * contact.Normal;
					}

					/*
					** Finally, clamp the force to the friction circle
					*/
					float max_friction_force = friction_coefficient * contact_weight;
					if (friction_force.Length2() > max_friction_force * max_friction_force) {  
						float flen = friction_force.Length();
						friction_force /= flen;
						friction_force *= 0.5f * max_friction_force;
					}

					/*
					** Compute the torque and add the force and torque into the accumulators
					*/
					Vector3 friction_torque;
					Vector3::Cross_Product(r,friction_force,&friction_torque);

					*force += friction_force;
					*torque += friction_torque;
					DEBUG_RENDER_VECTOR(contact.ContactPoint,friction_force,Vector3(1,0,0));
				}
			}

			DEBUG_RENDER_VECTOR(contact.ContactPoint,contact_force / Get_Mass(),Vector3(0,1,0));
		}
	}

	/*
	** HACK!  Never let the force or torque completely reflect the angular or linear momentum
	** clamp them to a value that at most, drives the momentum to zero
	**
	** NOTE: this doesn't work, I'm just leaving it here in case I get a new idea
	** which will make it work...
	*/
#if JITTER_ELIMINATION_CODE
	Vector3 max_delta_lmomentum = - 1.0f * State.LMomentum / LastTimestep;
	Vector3 max_delta_amomentum = - 1.0f * State.AMomentum / LastTimestep;
	
	Vector3 old_force = *force;
	Vector3 old_torque = *torque;

	if ((max_delta_lmomentum.X < 0) && (force->X < max_delta_lmomentum.X)) force->X = max_delta_lmomentum.X;
	if ((max_delta_lmomentum.X > 0) && (force->X > max_delta_lmomentum.X)) force->X = max_delta_lmomentum.X;
	if ((max_delta_lmomentum.Y < 0) && (force->Y < max_delta_lmomentum.Y)) force->Y = max_delta_lmomentum.Y;
	if ((max_delta_lmomentum.Y > 0) && (force->Y > max_delta_lmomentum.Y)) force->Y = max_delta_lmomentum.Y;
	if ((max_delta_lmomentum.Z < 0) && (force->Z < max_delta_lmomentum.Z)) force->Z = max_delta_lmomentum.Z;
	if ((max_delta_lmomentum.Z > 0) && (force->Z > max_delta_lmomentum.Z)) force->Z = max_delta_lmomentum.Z;
	
	if ((max_delta_amomentum.X < 0) && (torque->X < max_delta_amomentum.X)) torque->X = max_delta_amomentum.X;
	if ((max_delta_amomentum.X > 0) && (torque->X > max_delta_amomentum.X)) torque->X = max_delta_amomentum.X;
	if ((max_delta_amomentum.Y < 0) && (torque->Y < max_delta_amomentum.Y)) torque->Y = max_delta_amomentum.Y;
	if ((max_delta_amomentum.Y > 0) && (torque->Y > max_delta_amomentum.Y)) torque->Y = max_delta_amomentum.Y;
	if ((max_delta_amomentum.Z < 0) && (torque->Z < max_delta_amomentum.Z)) torque->Z = max_delta_amomentum.Z;
	if ((max_delta_amomentum.Z > 0) && (torque->Z > max_delta_amomentum.Z)) torque->Z = max_delta_amomentum.Z;

	WWASSERT(1.00001f * old_force.Length2() >= force->Length2());
	WWASSERT(1.00001f * old_torque.Length2() >= torque->Length2());
#endif

	WWASSERT(WWMath::Is_Valid_Float(force->X));
	WWASSERT(WWMath::Is_Valid_Float(force->Y));
	WWASSERT(WWMath::Is_Valid_Float(force->Z));

}

void RigidBodyClass::Compute_Point_Velocity(const Vector3 & p,Vector3 * pdot)
{
	// REMEMBER: p is assumed to be in world coordinates!  pdot is as well.
	// pdot = velocity + CROSS(angular_velocity , r)
	Vector3 r;
	
	Vector3::Subtract(p,State.Position,&r);
	Vector3::Cross_Product(AngularVelocity,r,pdot);
	Vector3::Add(*pdot,Velocity,pdot);
}

bool RigidBodyClass::Is_Colliding(const Vector3 & point, const Vector3 & normal)
{
	Vector3 padot;
	Compute_Point_Velocity(point,&padot);
	float vrel = Vector3::Dot_Product(normal,padot);
	return vrel < 0.0f;
}

void RigidBodyClass::Set_Moving_Collision_Region(float dt)
{
	AABoxClass region;
	ContactBox->Get_Outer_Bounds(&region);		// start with bounds of collision box

	// Recenter and enlarge to contain our velocity (scaled by a bit)
	Vector3 move = 1.5f * Velocity * dt;
	region.Center += 0.5f * move;
	region.Extent.X += WWMath::Fabs(move.X);
	region.Extent.Y += WWMath::Fabs(move.Y);
	region.Extent.Z += WWMath::Fabs(move.Z);

	// Now, scale to account for any rotational effects
	region.Extent *= 2.0f;

	PhysicsSceneClass::Get_Instance()->Set_Collision_Region(region,Get_Collision_Group());
}

void RigidBodyClass::Set_Stationary_Collision_Region(void)
{
	AABoxClass region;
	ContactBox->Get_Outer_Bounds(&region);		// start with bounds of collision box
	PhysicsSceneClass::Get_Instance()->Set_Collision_Region(region,Get_Collision_Group());
}


bool RigidBodyClass::Can_Go_To_Sleep(float dt)
{
	/*
	** RigidBodies go to sleep if their oct-box is resting on at least three contacts and
	** their velocities are below some thresh-hold and their controller isn't doing anything.
	*/
	if ((Controller != NULL) && (Controller->Is_Inactive() != true)) {
		GoToSleepTimer = RBODY_SLEEP_DELAY;
		return false;
	}
	
	const float VEL_THRESHHOLD = 0.05f;
	const float AVEL_THRESHHOLD = 0.05f;

	float max_lmomentum2 = Mass * VEL_THRESHHOLD * VEL_THRESHHOLD;
	float max_amomentum2 = IBody[1][1] * AVEL_THRESHHOLD * AVEL_THRESHHOLD;
	bool tried_to_sleep = false;

	if ((ContactBox->ContactCount >= 3) || (Get_RigidBodyDef()->CollisionDisabled)) {
		
		if ((State.LMomentum.Length2() < max_lmomentum2) && 
			(State.AMomentum.Length2() < max_amomentum2) ) 
		{
			tried_to_sleep = true;
			if (GoToSleepTimer < 0.0f) {
				return true;
			}
		} 
	}

	if (tried_to_sleep) {
		GoToSleepTimer -= dt;
	} else {
		GoToSleepTimer = RBODY_SLEEP_DELAY;
	}

	return false;
}

void RigidBodyClass::Compute_Impact(const CastResultStruct & result,Vector3 * impulse)
{
	Compute_Impact(result.ContactPoint,result.Normal,impulse);	
}

void RigidBodyClass::Compute_Impact(const Vector3 & point,const Vector3 & normal,Vector3 * impulse)
{
	// NOTE: this is only a temporary solution, it assumes we are colliding with
	// an immovable object (infinite mass)...
	Vector3 padot;
	Compute_Point_Velocity(point,&padot);
	float vrel = Vector3::Dot_Product(normal,padot);
	
	if (vrel > 0.0f) {
	
		// moving away
		impulse->Set(0,0,0);	

	} else {
		
		// collision
		float num = -(1.0f + Elasticity) * vrel;
		Vector3 ra = point - State.Position;
		
		Vector3 tmp1,tmp2;
		Vector3::Cross_Product(ra,normal,&tmp1);
		Matrix3::Rotate_Vector(IInv,tmp1,&tmp2);
		Vector3::Cross_Product(tmp2,ra,&tmp1);
		float den = MassInv + Vector3::Dot_Product(normal,tmp1);
		
		if (WWMath::Fabs(den) > WWMATH_EPSILON) {
			float mag = num / den;
			*impulse = mag * normal;
		} else {
			WWASSERT(0);
			impulse->Set(0,0,0);	
		}
	}

	WWASSERT(WWMath::Is_Valid_Float(impulse->X));
	WWASSERT(WWMath::Is_Valid_Float(impulse->Y));
	WWASSERT(WWMath::Is_Valid_Float(impulse->Z));

}

void RigidBodyClass::Assert_State_Valid(void) const
{
	WWASSERT(State.Position.Is_Valid());
	WWASSERT(State.Orientation.Is_Valid());
	WWASSERT(State.LMomentum.Is_Valid());
	WWASSERT(State.LMomentum.Is_Valid());
	WWASSERT(Velocity.Is_Valid());
	WWASSERT(AngularVelocity.Is_Valid());
}

inline void RigidBodyClass::Dump_State(void) const
{
	WWDEBUG_SAY(("Position:		%10.10f , %10.10f , %10.10f \r\n",State.Position.X,State.Position.Y,State.Position.Z));
	WWDEBUG_SAY(("Orientation:	%10.10f , %10.10f , %10.10f , %10.10f\r\n",State.Orientation.X,State.Orientation.Y,State.Orientation.Z,State.Orientation.W));
	WWDEBUG_SAY(("LMomentum:	%10.10f , %10.10f , %10.10f \r\n",State.LMomentum.X,State.LMomentum.Y,State.LMomentum.Z));
	WWDEBUG_SAY(("AMomentum:	%10.10f , %10.10f , %10.10f \r\n",State.AMomentum.X,State.AMomentum.Y,State.AMomentum.Z));
	WWDEBUG_SAY(("ContactBox intersecting: %d\r\n",(ContactBox->Is_Intersecting() ? 1 : 0)));
	WWDEBUG_SAY(("\r\n"));
}

void RigidBodyClass::Timestep(float dt)
{	
	WWPROFILE("RigidBody::Timestep");
	LastTimestep = dt;

// DEBUG DEBUG
Vector3 vel0 = Velocity;
Vector3 avel0 = AngularVelocity;

	/*
	** Update our history buffer
	*/
	if (History != NULL) {
		History->Update_History(State,dt);

		/*
		** Debugging, Draw our history if present
		*/
		OBBoxClass box;
		for (int i=1; i<History->History_Count(); i++) {
		
			const RigidBodyStateStruct & state = History->Get_Historical_State(i);
			ContactBox->Get_Inner_Box(&box,state.Orientation,state.Position);
			DEBUG_RENDER_OBBOX(box,Vector3(0,1.0f,0),0.05f);
		}
	}

	/*
	** If we have any latency error, attempt to correct it
	*/
	Assert_State_Valid();
	if (LatencyError.Position.Length2() > 0.001f) {
		Network_Latency_Error_Correction(dt);
	}
	Assert_State_Valid();


	/*
	** If we're currently asleep, see if we need to wake up.
	*/
	if (Is_Asleep()) {
		if ((Controller != NULL) && (Controller->Is_Inactive() == false)) {
			Set_Flag(ASLEEP,false);
		} else {
			return;
		}
	}

	if (Is_Immovable()) {
		return;
	}

	WWASSERT(Model);
	WWASSERT(ContactBox);

	Inc_Ignore_Counter();

#if RBODY_DEBUGGING
	if (RBODY_DEBUG_FILTER) {
		WWDEBUG_SAY(("------------------------------\r\n"));
		WWDEBUG_SAY(("RigidBody Timestep Begin (dt=%f).\r\n",dt));
		WWDEBUG_SAY((" Beginning State:\r\n"));
		Dump_State();
	}
#endif

	/*
	** Set our active collision region
	*/
	Set_Moving_Collision_Region(dt);

	/*
	** Repeat until we eat all of the time
	*/
	const int MAX_COLLISIONS = 10;
	int collisions = 0;
	float remaining_time = dt;
	float timestep;
	
	while ((remaining_time > 0.0f) && (collisions < MAX_COLLISIONS)) {

		Assert_State_Valid();
		WWPROFILE("RigidBodyClass integration loop");
		timestep = remaining_time;

		/*
		** Integrate the state of the object
		*/
		RigidBodyStateStruct oldstate = State;
		Integrate(timestep);

		/*
		** Check the final state of the object for collision.
		*/
		if (!ContactBox->Is_Intersecting()) {
		
			/*
			** Not intersecting so we're accepting the entire move
			*/
			remaining_time -= timestep;
			StickCount = 0;

		} else {

			WWPROFILE("Impulsive Collision Handling");

			StickCount++;

			/*
			** Search for a state where we can get valid contact points
			*/
			bool found = Find_State_In_Contact_Zone(oldstate);

			/*
			** Ad-hoc impact algorithm.  Killing angular momentum, clipping
			** linear momentum to the contact normals.
			*/
			State.AMomentum /= (float)StickCount+1;

			if (found) {
				ContactBox->Compute_Contacts();
				
				if (ContactBox->Get_Contact_Count() > 0) {

					Vector3 contact_centroid(0,0,0);
					
					for (int ci=0; ci<ContactBox->Get_Contact_Count(); ci++) {
			
						const CastResultStruct & contact = ContactBox->Get_Contact(ci);

						/*
						** Eliminate any momentum not tangential to this contact normal
						*/
						float dot = Vector3::Dot_Product(State.LMomentum,contact.Normal);
						if (dot < 0.0f) {
							
							Vector3 impulse = -1.01f * dot * contact.Normal;

							/*
							** If the object we collided with is a dynamic object, apply an impulse
							** to it as well.
							*/
							PhysClass * other_obj = ContactBox->Peek_Contacted_Object(ci);

							if ((other_obj != NULL) && (other_obj->As_RigidBodyClass() != NULL)) {
							
								RigidBodyClass * other_rbody = other_obj->As_RigidBodyClass();
								float fraction = Mass / (Mass + other_rbody->Get_Mass());
									
								State.LMomentum += fraction * impulse;
								other_rbody->Apply_Impulse(-(1.0f - fraction) * impulse, contact.ContactPoint);

							} else if ((other_obj != NULL) && (other_obj->As_Phys3Class() != NULL)) {
								
								if (!Push_Phys3_Object_Away(other_obj->As_Phys3Class(),contact)) {
									State.LMomentum += impulse;
								}
								remaining_time = 0.0f;	// I don't want to resolve a crowd chain reaction all in one frame

							} else {

								State.LMomentum += impulse;

							}

						}

						/*
						** Accumulate the contact centroid
						*/
						contact_centroid += contact.ContactPoint;
					}

					/*
					** Apply an instantaneous change to the angular momentum to make the physics
					** feel more realistic.  Compute the impluse that was applied to the linear 
					** momentum and apply a fraction of it to the angular momentum at the contact 
					** centroid.  (of course, this is all "ad-hoc", not true rigid-body dynamics...)
					*/
					Vector3 impulse = 0.3f * (State.LMomentum - oldstate.LMomentum);
					contact_centroid /= ContactBox->Get_Contact_Count();
					
					Vector3 r = contact_centroid - State.Position;
					Vector3 a_impulse;
					Vector3::Cross_Product(r,impulse,&a_impulse);

					State.AMomentum += a_impulse;


					/*
					** Done, update the rest of the rigid body state
					*/
					Update_Auxiliary_State();

#if RBODY_DEBUGGING
				if (RBODY_DEBUG_FILTER) {
					WWDEBUG_SAY((" Intersection occured, found state in contact zone.\r\n"));
					WWDEBUG_SAY((" Resulting new state:\r\n"));
					Dump_State();
				}
#endif
				}

			} else {
				
				/*
				** If all else fails: 
				** Fall back to the old, kill the momentum and try to let the
				** contact spring forces sort it out.
				*/
#if RBODY_DEBUGGING
				if (RBODY_DEBUG_FILTER) {
					WWDEBUG_SAY(("Rigid Body Object: %s is intersecting (%f,%f,%f).\n",Model->Get_Name(),State.Position.X,State.Position.Y,State.Position.Z));
					WWDEBUG_SAY(("Reverting to previous position: (%f,%f,%f)\r\n",oldstate.Position.X,oldstate.Position.Y,oldstate.Position.Z));
				}
#endif

				State.LMomentum /= (float)(StickCount + 1);
				State.Position = oldstate.Position;
				State.Orientation = oldstate.Orientation;
				remaining_time = 0.0f;
				Update_Auxiliary_State();

				// We've reverted the state, now display the total force and torque that is causing us to "stick"
				#ifdef WWDEBUG
				Vector3 force(0,0,0);
				Vector3 torque(0,0,0);
				Compute_Force_And_Torque(&force,&torque);
			
				DEBUG_RENDER_VECTOR(State.Position,force,Vector3(0,1,0));
				DEBUG_RENDER_VECTOR(State.Position,force,Vector3(0,0,1));
				#endif	

#if RBODY_DEBUGGING
				if (RBODY_DEBUG_FILTER) {
					WWDEBUG_SAY((" Un-handled intersection!  StickCount = %d\r\n",StickCount));
					WWDEBUG_SAY((" Reverted State:\r\n"));
					Dump_State();
				}
#endif

			}
		}
		collisions++;
	}

#if 0 // DEBUG DEBUG DEBUG
	if (ContactBox->Is_Intersecting()) {
		State = BackupState;
		goto doitagain;
	}
#endif

	DEBUG_RENDER_VECTOR(State.Position,Velocity,LMOMENTUM_COLOR);
	DEBUG_RENDER_VECTOR(State.Position,AngularVelocity,AMOMENTUM_COLOR);	

#ifdef WWDEBUG
	if (ContactBox->Is_Intersecting()) {
		OBBoxClass box;
		ContactBox->Get_Inner_Box(&box);
		DEBUG_RENDER_OBBOX(box,Vector3(1,0,0),0.3f);
	}
#endif

	Dec_Ignore_Counter();
	Model->Set_Transform(Matrix3D(Rotation,State.Position));
	Update_Cull_Box();
	Update_Visibility_Status();

	/*
	** Release our active collision region
	*/
	PhysicsSceneClass::Get_Instance()->Release_Collision_Region();

	/*
	** See if we can go to sleep and stop simulating!
	*/
	if (Can_Go_To_Sleep(dt)) {
		Set_Flag(ASLEEP,true);
	}

// DEBUG DEBUG	
	Vector3 vel_change = Velocity - vel0;
	Vector3 avel_change = AngularVelocity - avel0;
	const float VEL_CHANGE_SPIKE = 10.0f;
	const float AVEL_CHANGE_SPIKE = 10.0f;

	if (	(vel_change.Length() > VEL_CHANGE_SPIKE) ||
			(avel_change.Length() > AVEL_CHANGE_SPIKE))
	{
		WWDEBUG_SAY(("***** Velocity spike detected during RBody::Timestep!  "));
		WWDEBUG_SAY(("initial vel: %f    final vel: %f\r\n",vel0.Length(),Velocity.Length()));
		WWDEBUG_SAY(("initial avel: %f   final avel: %f\r\n",avel0.Length(),AngularVelocity.Length()));
	}
}


bool RigidBodyClass::Push_Phys3_Object_Away(Phys3Class * p3obj,const CastResultStruct & contact)
{
	/*
	** Notify any observer of the phys3 object that this impact has happened
	*/
	CollisionEventClass event;
	event.CollisionResult = &contact;
	event.CollidedRenderObj = NULL;
	event.OtherObj = this;
	p3obj->Collision_Occurred(event);

	/*
	** Compute a movement vector to push the phy3 object away
	*/
	Vector3 move = p3obj->Get_Transform().Get_Translation() - State.Position;
	move.Normalize();

	Vector3 point_vel;
	Compute_Point_Velocity(p3obj->Get_Transform().Get_Translation(),&point_vel);
	float pvel_relative = Vector3::Dot_Product(point_vel,move);
	if (pvel_relative > 0.0f) {
		pvel_relative *= LastTimestep;
	} else {
		pvel_relative = 0.0f;
	}

	move *= ContactBox->Get_Thickness() * ((1.0f - contact.Fraction) + pvel_relative);
	
	Dec_Ignore_Counter();
	p3obj->Collide(move);
	Inc_Ignore_Counter();

#if RBODY_DEBUGGING
	if (RBODY_DEBUG_FILTER) {
		WWDEBUG_SAY(("  Pushing Phys3 Object %x away (%f, %f, %f)\r\n",p3obj,move.X,move.Y,move.Z));
	}
#endif
	
	/*
	** Check if the object is now out of collision with us.  If the external game
	** logic kills the phys3 object, its collision group will be changed so we check
	** for that first.
	*/
	if (PhysicsSceneClass::Get_Instance()->Do_Groups_Collide(p3obj->Get_Collision_Group(),Get_Collision_Group())) {
		AABoxClass p3box;
		p3obj->Get_Collision_Box(&p3box);
		OBBoxClass rbox;
		ContactBox->Get_Outer_Box(&rbox);

#if RBODY_DEBUGGING
	if (RBODY_DEBUG_FILTER) {
		WWDEBUG_SAY(("  completely clear of our outer collision box\r\n"));
	}
#endif
		return (CollisionMath::Overlap_Test(rbox,p3box) == CollisionMath::OUTSIDE);

	} else {

#if RBODY_DEBUGGING
	if (RBODY_DEBUG_FILTER) {
		WWDEBUG_SAY(("  still intersecting our outer collision box\r\n"));
	}
#endif
		return true;
	}
}


void RigidBodyClass::Assert_Not_Intersecting(void)
{
	Inc_Ignore_Counter();

	OBBoxClass obbox = Box->Get_Box();
	CastResultStruct result;
	PhysOBBoxCollisionTestClass test(	obbox,
													Vector3(0,0,0),
													&result,
													Get_Collision_Group(),
													COLLISION_TYPE_PHYSICAL);
	PhysicsSceneClass::Get_Instance()->Cast_OBBox(test);
//	WWASSERT(result.StartBad != true);			
	if (result.StartBad) {
		WWDEBUG_SAY(("   !!!!!!!!!!!!!!!!!!!!!!!!!   Rigid Body %s intersecting!\r\n",Model->Get_Name()));
	} else {
		WWDEBUG_SAY(("   not intersecting\r\n"));
	}

	Dec_Ignore_Counter();
}

void RigidBodyClass::Get_Shadow_Blob_Box(AABoxClass * set_obj_space_box)
{
	WWASSERT(set_obj_space_box != NULL);
	if (Box) {
		Box->Get_Obj_Space_Bounding_Box(*set_obj_space_box);
	} else {
		MoveablePhysClass::Get_Shadow_Blob_Box(set_obj_space_box);
	}
}


bool RigidBodyClass::Find_State_In_Contact_Zone
(
	const RigidBodyStateStruct & oldstate
)
{
	RigidBodyStateStruct state0 = oldstate;
	RigidBodyStateStruct state1 = State;
	RigidBodyStateStruct teststate;
	
	bool done = false;
	bool success = false;
	int iteration_count = 0;
	const int MAX_ITERATIONS = 5;

	/*
	** Binary search for a state where the box is close enough to 
	** the terrain to generate contacts but the inner box is not
	** intersecting.
	*/
	while (!done) {
		RigidBodyStateStruct::Lerp(state0,state1,0.5f,&teststate);
		Set_State(teststate);
		
		if (ContactBox->Is_In_Contact_Zone()) {
			if (ContactBox->Is_Intersecting()) {
				state1 = teststate;
			} else {
				success = true;
				done = true;
			}
		} else {
			state0 = teststate;
		}
		iteration_count++;
		if (iteration_count > MAX_ITERATIONS) {
			done = true;
		}
	}
	
	return success;
}


bool RigidBodyClass::Push(const Vector3 & move)
{
	Vector3 old_pos = State.Position;

	/*
	** Create a collision test which sweeps our collision box along the given move vector
	*/
	OBBoxClass box = ContactBox->Get_World_Inner_Box();
	CastResultStruct result;
	PhysOBBoxCollisionTestClass test(	box,
													move,
													&result,
													Get_Collision_Group(),
													COLLISION_TYPE_PHYSICAL );
		
 	/*
	** Sweep the box
	*/
	Inc_Ignore_Counter();
	PhysicsSceneClass::Get_Instance()->Cast_OBBox(test);
	Dec_Ignore_Counter();

	/*
	** Move as far as we were allowed
	*/
	if (!result.StartBad) {
		
		if (result.Fraction > 0.0f) {
			State.Position += result.Fraction * move;
		}

		/*
		** Update the rest of our parameters (TODO: clean this process up)
		*/
		Set_Flag(ASLEEP,false);
		Update_Auxiliary_State();
		Update_Visibility_Status();

		Matrix3D m(Rotation,State.Position);
		Model->Set_Transform(m);
		if (ContactBox) {
			ContactBox->Set_Transform(m);
		}

		return (result.Fraction == 1.0f);
	
	} else {
	
		return false;

	}
}


/***********************************************************************************
**
** Save-Load System
**
***********************************************************************************/
const PersistFactoryClass & RigidBodyClass::Get_Factory (void) const
{
	return _RigidBodyFactory;
}

bool RigidBodyClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk(RBODY_CHUNK_MOVEABLE);
	MoveablePhysClass::Save(csave);
	csave.End_Chunk();
	
	ODESystemClass * ode_ptr = (ODESystemClass *)this;
	csave.Begin_Chunk(RBODY_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK_PTR(csave,RBODY_VARIABLE_ODESYSTEM_PTR,ode_ptr);
	WRITE_MICRO_CHUNK(csave,RBODY_VARIABLE_IBODY,IBody);
	WRITE_MICRO_CHUNK(csave,RBODY_VARIABLE_IBODYINV,IBodyInv);
	WRITE_MICRO_CHUNK(csave,RBODY_VARIABLE_STATE_POSITION,State.Position);
	WRITE_MICRO_CHUNK(csave,RBODY_VARIABLE_STATE_ORIENTATION,State.Orientation);
	WRITE_MICRO_CHUNK(csave,RBODY_VARIABLE_STATE_LMOMENTUM,State.LMomentum);
	WRITE_MICRO_CHUNK(csave,RBODY_VARIABLE_STATE_AMOMENTUM,State.AMomentum);
	WRITE_MICRO_CHUNK(csave,RBODY_VARIABLE_CONTACT_LENGTH,ContactThickness);
	csave.End_Chunk();
	return true;
}

bool RigidBodyClass::Load (ChunkLoadClass &cload)
{
	ODESystemClass * odesys = NULL;

	while (cload.Open_Chunk()) {
		
		switch(cload.Cur_Chunk_ID()) 
		{
			case RBODY_CHUNK_MOVEABLE:
				MoveablePhysClass::Load(cload);
				break;

			case RBODY_CHUNK_VARIABLES:
			
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK_PTR(cload,RBODY_VARIABLE_ODESYSTEM_PTR,odesys);
						READ_MICRO_CHUNK(cload,RBODY_VARIABLE_IBODY,IBody);
						READ_MICRO_CHUNK(cload,RBODY_VARIABLE_IBODYINV,IBodyInv);
						READ_MICRO_CHUNK(cload,RBODY_VARIABLE_STATE_POSITION,State.Position);
						READ_MICRO_CHUNK(cload,RBODY_VARIABLE_STATE_ORIENTATION,State.Orientation);
						READ_MICRO_CHUNK(cload,RBODY_VARIABLE_STATE_LMOMENTUM,State.LMomentum);
						READ_MICRO_CHUNK(cload,RBODY_VARIABLE_STATE_AMOMENTUM,State.AMomentum);
						READ_MICRO_CHUNK(cload,RBODY_VARIABLE_CONTACT_LENGTH,ContactThickness);

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

	
	if (odesys != NULL) {
		SaveLoadSystemClass::Register_Pointer(odesys,(ODESystemClass *)this);
	}

	SaveLoadSystemClass::Register_Post_Load_Callback(this);
	Update_Auxiliary_State();

	return true;
}


void RigidBodyClass::On_Post_Load(void)
{
	Update_Cached_Model_Parameters();
}


/***********************************************************************************************
**
** RigidBodyDefClass Implementation
**
***********************************************************************************************/

/*
** Persist factory for RigidBodyDefClass's
*/
SimplePersistFactoryClass<RigidBodyDefClass,PHYSICS_CHUNKID_RIGIDBODYDEF>	_RigidBodyDefFactory;

/*
** Definition factory for RigidBodyDefClass.  This makes it show up in the editor
*/
DECLARE_DEFINITION_FACTORY(RigidBodyDefClass, CLASSID_RIGIDBODYDEF, "RigidBody") _RigidBodyDefDefFactory;

/*
** Chunk ID's used by RigidBodyDefClass
*/
enum 
{
	RIGIDBODYDEF_CHUNK_MOVEABLEPHYSDEF						= 0x01106650,			// (parent class)
	RIGIDBODYDEF_CHUNK_VARIABLES,

	RIGIDBODYDEF_VARIABLE_AERODYNAMICDRAGCOEFFICIENT	= 0x00,
	RIGIDBODYDEF_VARIABLE_COLLISIONDISABLED,
};


RigidBodyDefClass::RigidBodyDefClass(void) :
	AerodynamicDragCoefficient(0.0f),
	CollisionDisabled(false)
{
	// make our parameters editable
	FLOAT_EDITABLE_PARAM(RigidBodyDefClass, AerodynamicDragCoefficient, 0.0f, 100.0f);
}

uint32 RigidBodyDefClass::Get_Class_ID (void) const	
{ 
	return CLASSID_RIGIDBODYDEF; 
}

PersistClass * RigidBodyDefClass::Create(void) const
{
	RigidBodyClass * obj = NEW_REF(RigidBodyClass,());
	obj->Init(*this);
	return obj;
}

const PersistFactoryClass & RigidBodyDefClass::Get_Factory (void) const
{
	return _RigidBodyDefFactory;
}

bool RigidBodyDefClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(RIGIDBODYDEF_CHUNK_MOVEABLEPHYSDEF);
	MoveablePhysDefClass::Save(csave);
	csave.End_Chunk();
	
	csave.Begin_Chunk(RIGIDBODYDEF_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,RIGIDBODYDEF_VARIABLE_AERODYNAMICDRAGCOEFFICIENT,AerodynamicDragCoefficient);
	WRITE_MICRO_CHUNK(csave,RIGIDBODYDEF_VARIABLE_COLLISIONDISABLED,CollisionDisabled);
	csave.End_Chunk();
	return true;
}

bool RigidBodyDefClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {

		switch(cload.Cur_Chunk_ID()) {

			case RIGIDBODYDEF_CHUNK_MOVEABLEPHYSDEF:
				MoveablePhysDefClass::Load(cload);
				break;

			case RIGIDBODYDEF_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,RIGIDBODYDEF_VARIABLE_AERODYNAMICDRAGCOEFFICIENT,AerodynamicDragCoefficient);
						READ_MICRO_CHUNK(cload,RIGIDBODYDEF_VARIABLE_COLLISIONDISABLED,CollisionDisabled);
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

bool RigidBodyDefClass::Is_Type(const char * type_name)
{
	if (stricmp(type_name,RigidBodyDefClass::Get_Type_Name()) == 0) {
		return true;
	} else {
		return MoveablePhysDefClass::Is_Type(type_name);
	}
}











