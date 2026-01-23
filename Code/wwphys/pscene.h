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
 *                     $Archive:: /Commando/Code/wwphys/pscene.h                              $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 2/28/02 11:48a                                              $*
 *                                                                                             *
 *                    $Revision:: 125                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PSCENE_H
#define PSCENE_H

#include "always.h"
#include "scene.h"
#include "widgetuser.h"
#include "physlist.h"
#include "aabtreecull.h"
#include "gridcull.h"
#include "vector.h"
#include "shader.h"
#include "vissample.h"
#include "vistablemgr.h"
#include "phystexproject.h"
#include "simplevec.h"
#include "vissectorstats.h"

class	Matrix3D;
class ChunkLoadClass;
class ChunkSaveClass;
class PhysClass;
class LightPhysClass;
class LightClass;
class StaticPhysClass;
class RenderInfoClass;
class SpecialRenderInfoClass;
class VisRenderContextClass;
class VisOptimizationContextClass;
class AABoxClass;
class VertexMaterialClass;
class PathfindClass;
class PhysDecalSysClass;
class MeshClass;
class VisOptProgressClass;
class CameraShakeSystemClass;
class LightEnvironmentClass;
class StaticAnimPhysClass;
class StringClass;

// forward referencing the collision detection queries
class	PhysRayCollisionTestClass;
class PhysAABoxCollisionTestClass;
class PhysOBBoxCollisionTestClass;
class PhysAABoxIntersectionTestClass;
class PhysOBBoxIntersectionTestClass;
class PhysMeshIntersectionTestClass;

// forward referencing the culling system types.
class PhysAABTreeCullClass;
class StaticAABTreeCullClass;
class	DynamicAABTreeCullClass;
class PhysGridCullClass;
class StaticLightCullClass;

// Lighting solver
class LightSolveContextClass;

/**

  Physics system

  All objects in the world which want to interact physically (as opposed to being
  simply decorations) will register a "PhysicalObject" with the physics sytem.
  The ABC PhysicalObjectClass is used by the physics manager for the following
  things:

  - checking for collisions between all objects; methods for collision detection
    will be pure virtual methods of the base class
  - computing forces between any "linked" objects
  - applying global gravity and damping to all of the objects
  - telling all physical objects to timestep themselves.

  Initializing the system

  - Physics system will init itself by clearing all of its lists and
    tables.

  - Each game object asks the physics system to create an instance of a
    physics object and specifies all of the important parameters...

  - Whenever an object is added to the system, it is added in sorted order
    to the x,y, and z "neighbor" lists.

  Each frame the following things will happen:

  - All game objects will exert their influence over their PhysicalObjects using
    methods provided.  Game objects will usually know exactly what PhysicalObject
	 type they have.  I.e. a HummVee will know that its physical object is a
	 Wheel4PhysClass and will be able to directly call methods of that class.

  - One at a time, each physical object will take its "move" for the frame.  Moves
    will be calculated in a framerate-independent way.  A single "move" for an object
	 may consist of rotation and translation.

  Each move, the following things will happen:

  - The object will rotate in place according to its angular velocity and the 
    amount of time which has passed for this frame

  - Collision detection code will "push" the object into a valid position in the world.
    If it is unable to push the object into a valid position, the rotation state will
	 be reverted.

  - The object will then compute a translational move for the frame based on its 
    velocity and the time which has passed.

  - Collision detection code will test the final position for penetration with the world.
    If penetration has occured, the object will be pushed out of the polygons it
	 is penetrating.  If this is not possible, we revert to the previous state.

  Brainstorming:

  - Collision between any pair of objects can be disabled or enabled as desired?

  - Multiple different types of collision objects or everything is a box?
    Or everything is three different sets of 3d boxes?

  - A separate sorted list for the x axis, y axis and z axis will be maintained.  It 
    will be used to optimize the collision detection algorithm.  A single, overall
	 bounding sphere / axis aligned box should be used for this quick rejection.

  - How will all of the separate physical object types interact???  The papers all
    handle everything as a rigid body, no special cases...

  - Different types of objects need to interact whenever a collision occurs.  The
    physics manager should just detect collisions, then the objects should ask for
	 any info they need to resolve the collision.  Whenever a value is computed for
	 a particular collision, it should be cached so that if the second object also
	 needs the same value, it is not re-computed.

  - "real" collision resolution requires the velocity of the collision point.

  - How do we generalize the dy/dt function for the ODE solver since we have multiple
    different types of objects (with different state vectors)
    * each object will be responsible for: (1) adding itself to the state vector
	   (2) getting its new state out of the state vector and (3) adding its
		derivatives to the derivative vector.  Each of these functions will 
		return the number of values which were added to the vector.  This should
		allow us to "dumb-down" the physics for objects which are far away too...

  - Can each object be asked for its dy/dt separately?
    * yes, each object will add its dy/dt values to a given array.

  - All of this copying into the gigantic state vector sounds really in-efficient.
    Find a better system.  What if we only decide to use Euler's method, it could
	 be easily hard coded into each object type...
	 * The giganitic state vector approach is overkill.  We will not use it.
	 
12/03/97

  - Each object will be "timestepped" individually.  Each object type will be
    free to use whatever integration method it wants to (again, no monolithic state
	 vector) 

  - One at a time, each object will be told to update their state.  They will be
    given an elapsed time to ensure that we are frame-rate independent.  
	 
  - The steps a typical physical object will need to take in order to update itself:

	 - Store its current state so that we can revert if we have to

	 - Compute a desired state for the end of the timestep.  This can be done
	   using the high-tech integrator and all of the rigid body dynamics equations.

    - Do the rotation portion of that move in place.

    - "Push" the object into a valid position (world polys and object polys)
	   If push goes into an infinite loop, revert and kill the angular velocity.

    - Subdivide the linear move into steps which are ~half the characteristic
	   length of the bounding box.  Store this vector

	 - Loop while we haven't eaten the entire translation:

      - Add the sub-move vector 

		- "Push" the object into a valid position, modify velocity by normal of the
		  polygon collided with.  If push goes into an infinite loop, revert to 
		  previous sub-move and exit
   

Feb 9, 1998

	Nope, try again, simplify, simplify, simplify!  
	
	- For objects which do not care about rotation causing collision (e.g. characters), 
	*use a collision object which doesn't care either!*  So, for characters and many of the 
	objects in the game, we will use either Axis-Aligned boxes or cylinders.  Since I have
	the math for casting an axis-aligned box, I will probably use that.  These objects will 
	rotate within thier box so to the physics system, rotation / orientation will be trivial.

	- Jello Physics! For objects like the vehicles which were going to be done with rigid body 
	dynamics I'm going to try a spring-mass model.  There are many problems with rigid body 
	dynamics.  It is very difficult and expensive to compute correct contact forces, and it
	is difficult to find contact points.  Assuming the bodies are rigid requires collision
	detection or at least penetration depth calculations for solids like Oriented Boxes or
	Convex Polyhedra.  I have solved "volume-sweeping" for oriented boxes but not "volume-
	rotation" or the penetration depth calculation.  The spring mass model avoids these 
	problems and it comes down to testing line-segments for intersection with the world 
	polygons.  

	- Each object will still be moved one at a time.  At the beginning of a move, an object
	must be in a valid position.  Its move will be "cast" through the world and the object
	will only actually move up to the first collision.  At that point, the velocity will be
	modified to slide along whatever it it (if possible).  Repeat until the entire move
	vector is eaten or the object is stopped completely.

April 14, 1998

  - Finally getting back to the physics system.  Month of march was spent converting code
  to the new version of surrender.  So, not all of the items in the February notes were
  implemented.  At this point, I have the beginnings of the character collision system.  It
  relies on the Cast_AABox function which sweeps an axis-aligned box through the world.  
  currently it only checks for collisions with the terrain polygons.  Also, the "move-one-
  at-a-time" system was implemented and the old simulation style system removed.

  - I have hidden the implementation of the terrain collision from the game engine.  Any
  terrain implementation must simply support the Cast_Ray, Cast_AABox, Cast_OBBox functions 
  and be able to give the user a pointer to a render object which can be added to the scene
  in which the terrain is to be rendered.  This should allow some freedom to experiment with
  octrees or bsp trees to speed up the cast functions...

  - The next step is to use the existing "partitioned" mesh to implement the rest of the
  needed collision detection functionality so that the rest of the team can work with it
  while I experiment with more efficient systems.  
   - Cast_AABox must be made to also check against the objects.  
	- Cast_Ray should be hooked in and a physics object for projectiles which is based on 
	  Cast_Ray should be created.
	- Communication system between the game engine and collision system should be tested.
     This is going to be a virtual class which simply defines a virtual callback method,
	  game objects will multiply inherit this class and get messages from the system
	  whenever a collision occurs.

June 1, 1998

  - Working on collision detection again.  Spent the past two weeks or so writing some
  bpt code which turns the terrain mesh into a combination bsp/box tree (I simply call it
  a Binary Partition Tree).  Each node contains a bounding box which bounds all polygons 
  in the tree below that node.  Building an efficient bpt is becoming a priority since 
  we're getting a lot of splits and also nodes with gobs of polys in them due to the
  way our terrains are constructed.  Should be easy to take advantage of our grid lines.

  - Collision system has been hooked into the game and a very simple projectile physics
  class written which uses rays instead of full blown boxes.  The collision system communicates
  back to the game through multiple inheritance of a "CollisionObserverClass".  The projectile
  physics work pretty well when you plug in RK4 or Midpoint as the integrator; same trajectory
  no matter what the framerate (within reason). (Mar2000 note: this was pretty stupid since you can
  directly calculate where the projectile would end up if the only force is gravity!)

  - Changing all of the internal collision functions to pass around "collision test classes" 
  which wrap the primitive, its movement vector, a results structure, and any pre-calculatable
  variables into one structure.  The main reason for this was just so I have a place to put
  the info that can be precalculated and re-used.  (e.g. put a min,max along with the AABoxClass)

  - Now writing the recursive AABox Cast function which casts the box against the bpt.  Once 
  this is in place, we should start getting a significant speedup from using the bpt.

  - Thinking again about using a hybrid between the "every object moves on its own" and 
  the "every object puts itself into the monolithic state vector" ideas.  I do need some objects
  to be simulated together if we want to completely avoid being tied to fixed timesteps.  For
  example, I need to simulate the camera along with the object its tied to...  If we do true
  contact... all things touching each other will need to be simulated together.

July 13, 1998

  - Removing the distinction between physical objects and terrain.  New engine idea will use
  a combination of a heightfield and tilemap.  This means we're going to have multiple "terrains"
  and we will also be having meshes which don't move but are collideable such as guard towers, etc.

August 31, 1998

  CURRENT SITUATION: several sub-systems need efficient culling:
  - rendering system needs to be able to only render objects which are in the view frustum 
  - collision system needs to quickly narrow down the number of objects checked for each 
    collision query made.
  - AI/Game Logic needs to be able to quickly find "nearby" objects, objects within certain
    radii, objects which are in a given cone etc...

  PROBLEMS:
  - current code has no culling except for the TileMap object which is itself a huge render
    object spanning the entire level that performs culling internally in its render call.
  - currently, the "SceneClass" is expected to only deal with RenderObj derived classes.
  - collision queries and the like must go through physics object interfaces so that some
    physics objects can use simple primitives
  - deriving physobj from renderobj doesn't make a lot of sense because render obj is a 
    relatively "heavy" class.  I don't want to make another wrapper (and have the associated
	 duplicated transforms, etc...)
  - Render objects can have their transforms changed at the programmers whim, there is
    no built-in provision for telling the scene to refresh its culling information

  SOLUTION:
  - make a physics "SceneClass" with culling datastructures for both static and dynamic culling
  - physics scene deals only with physics objects
  - therefore all objects in the "game world" will be represented by a physics object
  - when a physics object changes position, the culling database can be refreshed
  - physobj needs its linkage information built in.
	
	class PhysicsSceneClass : public SceneClass
	{
		void Add_Render_Obj(...)		{ assert(0); }
		void Remove_Render_Obj(...)	{ assert(0); }
		
		void Add_Physical_Object(PhysObjClass * pobj,hint = MOVING);
		void Remove_Physical_Object(PhysObjClass * pobj);
		
		void Render(CameraClass & cam);
		bool Cast_Ray( );
		bool Cast_AABox( );
		bool Cast_OBBox( );
		PhysObjListClass * Get_Spatial_Object_List(pos,radius);
		PhysObjListClass * Get_Spatial_Object_List(min,max);
		PhysObjListClass * Get_Spatial_Object_List(frustum);
		PhysObjListClass * Get_Spatial_Object_List(raytest);
		PhysObjListClass * Get_Spatial_Object_List(raytest);
	}

September 15, 1998

  Finished my work on the initial prototype of the level editor.  In the process of writing the
  level editor, I implemented the new PhysicsSceneClass which replaces the old classes: PhysicsSystem
  PrivatePhysicsSystem and CollisionSystem and also takes over the job of the main 3D game scene
  for rendering.  This caused some major re-organization and as a result most of WWPhys was #if 0'd...

  A new base class PhysClass was implemented along with a derived DecorationPhysClass which can
  be inserted into this scene and rendered, collided with, etc.  The AABTree culling system for
  all of the static meshes was written to be dynamically re-partitionable.  The Grid culling system
  was only implemented in terms of an interface.  Currently it just contains a linked list of
  PhysClasses.

  Next job is to add all of the functionality of the old system (Phys4, CharPhys, Tilemap, etc) into
  this system and get Commando running on top of it.

  NOTES:
  - New system contains provisions for updating the culling datastructures when an object moves.
    I should also add a "safe teleport" function which asks the system if it is ok for an object
	 to be in a given position and if not, move the object to a safe position.  This could get
	 rid of the possibility of "startbad's" on level load.

  - TileMapClass is now obsolete but much of its functionality will have to go into game code.
    The PhysicsSceneClass contains an axis-aligned bounding box tree which serves the purpose
	 that TileMap used to.  Other "TerrainPhysClass's" are still going to be single objects though.

November 30, 1998

  Just completed another massive port to the latest version of surrender (1.4x).  Also had to 
  rewrite much of the plugin this time.  
  
  Adding occlusion culling for all physics objects in the "static" scene.  I still have some 
  questions in my mind as to how this is going to work but the basic idea is that there will
  be "vis-sectors".  Each vis-sector will contain a bit-vector with one bit for every piece of
  static geometry which indicates whether that piece of geometry can be seen from this 
  "vis-sector".  These bit arrays will be created by rendering the scene from each sector
  in a way that writes the node's id rather than texturing, lighting, etc, and then reading
  the framebuffer to see which nodes were not occluded.  Currently I have two questions: 
  How are these vis sectors defined? and How can I cull dynamic objects using this information?
  
  Now working on the occlusion bit vectors.  Currently implementing it with a full bit vector for
  each object which contains a bit for every other object.  If we compress this, we could at 
  the start of each frame, decompress the vis bits for the current active node (or even only
  decompress it when the active node changes).  Only problem is in implementation, editor stuff
  will want an uncompressed one that is easily modified; game will want tight compressed vis table.

December 21, 1998
  
  Converted W3D to render directly to the gerd (bypass the sr scene graph).  This was the 
  easiest way to implement the vis-detection stuff and opens the door for us to 
  hierarchically cull lights.  Vis detection is implemented now.  

  Need to implement a file format for the map.  

  - What is in the file format?
  - Who reads/writes it?
  - If pscene just loads stuff directly, how does Byon get ahold of the objects?
  - VisBits are independent of culling system!  They only depend on the IDs not changing!
  - Its a lot simpler if this system *only* contains completely static meshes! (not walls, etc)
	 + none of them *need* game logic
	 + none of them change or move
	 + byon doesn't need a game object wrapped around them
	 - is there no pre-calculated lighting on the non static stuff?
	 - want as much as we can get into this since it should be faster and has no size restrictions

  File format is going to be chunk based.  Parsing of the top-levels of the file will occur
  in Commando.  Certain chunks will be passed down to the PhysicsSceneClass to be handled.
  These chunks will include a chunk which wraps all of the culling data, a chunk which wraps
  each static object or dynamic object, chunks for zones and lights, etc.

Jan 15, 1999

  Working on the lighting management system.  For the static lights in the level, I want
  to use an efficient culling system like the AABTree.  All of these lights will be entered
  into the scene through a custom call in the physics scene.  All dynamic lights will be
  added through the normal Add_Render_Object function or by being added as sub-objects
  of a model in the scene.

  Add_Render_Obj and Remove_Render_Obj will be implemented by having the physics scene
  automatically create a cullable object for the render object and storing the pointer
  to that object in the UserData field.  In addition, the cullable object created for them
  will need to be a "dirty culler" that assumes its bounding box or transform could change
  at any time since the user is obviously working with a render object and could change
  the transform underneath us (normally this is a no-no).  These will be added to a "dirty
  culler" list and caused to re-insert themselves into the dynamic culling system each
  frame.  These objects will never cause collisions either (just for sanity) they
  are only visual...
  
  Render function will look like this:

  Render() {
		
		update the "dirty cullers" 
		collect dynamic objects and vertex processors (dy_obj_list,dy_vp_list)
		push dynamic vertex processors
		render static terrain
		
		global optimization of LOD levels for the dynamic objects

		for each dynamic object
			push static lights which may affect it
			render
			pop static lights that were pushed

		pop dynamic vertex processors

May 27, 1999

  Moving the Visibility system into PhysicsSceneClass so that it can incorporate
  pre-calculated visibility for other things like dynamic objects.  Previously it
  was pretty nicely encapsulated inside of the static culling system. 

October 26, 1999

	Adding shadow generation code

Jan 2000

	Pluging in the decal system, texture projection system is mostly done except for
	optimizations and static projector generation (masses of tree shadows and stuff).

Feb 21, 2000

  Decals are done, Animated Static objects with collision are working now (translation
  only).  Adding code which generates the static projectors (tree shadows, window projections
  and things like that).  Optimized the view-frustum culling code to propogate the plane
  masks and perform trivial acceptance of entire sub-trees.

Junk 28, 1000

  Major VIS update.  Now the dynamic culling system is a pre-generated AABTree with 
  precalculated visiblity for each node in the tree.  In addition, the static object
  culling system contains hierarchical visibility data and each light will have a 
  pvs for what static objects and dynamic object cells it can "see".  The generation
  of the dynamic object AAB-Tree is "seeded" with a pathfind solve and a coarse 
  voxelization of the level.  This tree is "optimized" after VIS is computed; redundant
  leaves of the tree are deleted and the vis tables collapsed.

Aug 9, 2000

  Implementing a new lighting system which collects the lights affecting each object 
  which is not pre-lit and creates temporary directional lights for them.  This should
  solve the problem of having a level with thousands of lights but only a few that
  actually need to be active.  We'll still need to reduce the number of lights in
  some levels though.  The code for this lighting algorithm can be found in
  lightenvironment.cpp.

Sept 24, 2000
 
	Making the lighting system coherent across frames.  If an object doesn't move, it 
	will have its local static lighting environment cached and can just re-use the data.
	Whenever physics object move, they invalidate their internal cache and will ask
	the physics scene to recompute the lighting environment next time its needed.  This
	change involved adding the Compute_Static_Lighting function to pscene and the
	pair of functions: Invalidate_Static_Lighting_Cache and Get_Static_Lighting_Cache
	to PhysClass.  These caches must also be invalidated when lights in the scene change
	state (such as when the power goes out in a building...)

April 2001
	
	Installed the new vis renderer, it seems to simplify a lot of code!  Also converting
	the shadow rendering code to work under DX8.  I plan to add a new shadow mode which
	uses hardware to render-to-texture.

*/


/**
** PhysicsSceneClass
** Drop all of the objects into the world in here, represented by physics objects, then
** you can do fun things like rendering and collision detection.
*/
class PhysicsSceneClass : public SceneClass , public WidgetUserClass
{
public:

	PhysicsSceneClass(void);
	virtual ~PhysicsSceneClass(void);
	
	/*
	** For now, making the Physics Scene a singleton.  If we want the feature of
	** instantiating multiple physics scenes, we will need to add a scene pointer
	** to the base PhysClass and resolve all cases where we use this function.
	*/
	static PhysicsSceneClass * Get_Instance(void) { return TheScene; }
	
	/*
	** Timestep the world
	*/
	void Update(float dt,int frameid);
	void Pre_Render_Processing(CameraClass & camera);
	void Post_Render_Processing(void);

	/*
	** Methods to add and remove physical objects in the system
	*/
	void Add_Dynamic_Object(PhysClass * newobj);
	void Add_Static_Object(StaticPhysClass * newtile,int cull_node_id = -1);
	void Add_Dynamic_Light(LightPhysClass * light);
	void Add_Static_Light(LightPhysClass * newlight,int cull_node_id = -1);

	void Remove_Object(PhysClass * obj);
	void Remove_All(void);

	void Delayed_Remove_Object(PhysClass * obj);
	void Process_Release_List(void);
	
	bool Contains(PhysClass * obj);

	RefPhysListIterator	Get_Dynamic_Object_Iterator(void);
	RefPhysListIterator	Get_Static_Object_Iterator(void);
	RefPhysListIterator	Get_Static_Anim_Object_Iterator(void);
	RefPhysListIterator	Get_Static_Light_Iterator(void);

	TexProjListIterator	Get_Static_Projector_Iterator(void);
	TexProjListIterator	Get_Dynamic_Projector_Iterator(void);

	StaticPhysClass *		Get_Static_Object_By_ID(uint32 id);

	/*
	** Dirty Cull List
	**
	**	The objects in this list update their culling every frame.
	** An example of this is a cinematic with an animated bounding
	** box.
	**
	*/
	void Add_To_Dirty_Cull_List(PhysClass *obj);
	void Remove_From_Dirty_Cull_List(PhysClass *obj);
	bool Is_In_Dirty_Cull_List(PhysClass *obj);

	/*
	** Lighting Controls
	**
	** Lighting Mode:  You can disable all lighting calculations, use the surrender lighting code, 
	** or use the el-cheapo lighting code which turns all lights into direction lights with no 
	** per-vertex attenuation or direction calculations.
	**
	** Scene Ambient Light:  The physics scene takes over the ambient light (is also implemented
	** in SceneClass) so that I can tweak the ambient light on a per-object basis.  This will
	** probably be done to make dynamic objects blend in with the light-mapped environment better.
	**
	** Lighting LOD: Set the cut-off intensity where point light sources are turned into pure
	** ambient lights.  For example, if this is set to 0.5f, then any light whose intensity
	** is less that 0.5 will be converted into a slightly dimmer ball of ambient and thus not
	** incur the directional lighting calculation cost.  NOTE: this lighting LOD is only
	** used when using LIGHTING_MODE_CHEAP lighting model
	**
	** Sun Light:  This controls the direction of the sun.  For Renegade, we probably won't ever
	** change this in-game.  However, if we did not use light-maps and we re-calculated all of
	** the static projectors each time the sun moved, we could have a moving sun.  
	**
	** NOTE: elevation is the angle that the sun makes with the horizontal,
	** rotation is the angle between the +X axis and the line of the sun projected onto the X-Y plane.
	** these variables are chose to match the ones in LightScape.
	*/
	enum {
		LIGHTING_MODE_NONE	= 0,
		LIGHTING_MODE_SURRENDER,
		LIGHTING_MODE_CHEAP
	};
	
	int							Get_Lighting_Mode(void)										{ return LightingMode; }
	void							Set_Lighting_Mode(int mode)								{ LightingMode = mode; }	

	virtual void				Set_Ambient_Light(const Vector3 & color) override				{ SceneAmbientLight = color; }
	virtual const Vector3 &	Get_Ambient_Light(void)	override									{ return SceneAmbientLight; }

	void							Set_Lighting_LOD_Cutoff(float intensity);
	float							Get_Lighting_LOD_Cutoff(void);

	bool							Is_Sun_Light_Enabled(void);
	void							Enable_Sun_Light(bool onoff);
	LightClass *				Get_Sun_Light(void);
	void							Set_Sun_Light_Orientation(float rotation,float elevation);
	void							Get_Sun_Light_Orientation(float * set_rotation,float * set_elevation);
	void							Get_Sun_Light_Vector(Vector3 * set_vector);
	
	void							Compute_Static_Lighting(LightEnvironmentClass * light_env,const Vector3 & obj_center,bool use_sun,int vis_object_id);
	void							Invalidate_Lighting_Caches(const AABoxClass & bounds);

	/*
	** Collision Detection Methods
	** Set_Collision_Region - collects objects in the given region into a list for subsequent collision checks
	**                        this is useful if you're going to do a lot of checks in the same general area (e.g. RigidBody)
	** Release_Collision_Region - releases the collision region list
	** Cast_Ray - casts a ray into the world, returning information about what was collided and at what point along the ray
	** Cast_AABox - casts an axis aligned box, returning information about what was collided and at what point
	** Cast_OBBox - casts an oriented box, returning information about what was collided and at what point
	** Intersection_Test - tests the given primitive for intersection with anything else in the system
	*/
	void Set_Collision_Region(const AABoxClass & bounds,int colgroup);
	void Release_Collision_Region(void);

	bool Cast_Ray(PhysRayCollisionTestClass & raytest,bool use_collision_region = false);
	bool Cast_AABox(PhysAABoxCollisionTestClass & boxtest,bool use_collision_region = false);
	bool Cast_OBBox(PhysOBBoxCollisionTestClass & boxtest,bool use_collision_region = false);
	
	bool Intersection_Test(PhysAABoxIntersectionTestClass & boxtest,bool use_collision_region = false);
	bool Intersection_Test(PhysOBBoxIntersectionTestClass & boxtest,bool use_collision_region = false);
	bool Intersection_Test(PhysMeshIntersectionTestClass & meshtest,bool use_collision_region = false);

	bool Intersection_Test(const AABoxClass & box,int collision_group,int collision_type,bool use_collision_region = false);
	bool Intersection_Test(const OBBoxClass & box,int collision_group,int collision_type,bool use_collision_region = false);

	void Force_Dynamic_Objects_Awake(const AABoxClass & box);

	/*
	** Collection Methods
	** Collect lists of objects which meet certain requirements.
	** TODO: is this the interface we want?  maybe the user can give a callback object?
	** maybe more filters over which objects are added to the list?
	*/
	void Collect_Objects(const Vector3 & point,bool static_objs, bool dynamic_objs,NonRefPhysListClass * list);
	void Collect_Objects(const AABoxClass & box,bool static_objs, bool dynamic_objs,NonRefPhysListClass * list);
	void Collect_Objects(const OBBoxClass & box,bool static_objs, bool dynamic_objs,NonRefPhysListClass * list);
	void Collect_Objects(const FrustumClass & frustum,bool static_objs, bool dynamic_objs,NonRefPhysListClass * list);

	void Collect_Collideable_Objects(const AABoxClass & box,int colgroup,bool static_objs, bool dynamic_objs,NonRefPhysListClass * list);
	void Collect_Collideable_Objects(const OBBoxClass & box,int colgroup,bool static_objs, bool dynamic_objs,NonRefPhysListClass * list);

	void Collect_Lights(const Vector3 & point,bool static_lights,bool dynamic_lights,NonRefPhysListClass * list);
	void Collect_Lights(const AABoxClass & bounds,bool static_lights,bool dynamic_lights,NonRefPhysListClass * list);

	StaticPhysClass * 		Find_Static_Object( int instance_id );

	/*
	** Collision Groups:
	** 
	** Each physical object can be assigned to a collision group.  Then
	** collision detection between any pair of groups can be enabled or
	** disabled.  An object will completely ignore objects in collision
	** groups that are disabled relative to its collision group.
	**
	** Collision groups are numbered 0x0 - 0xF
	*/
	enum {
		COLLISION_GROUP_WORLD = 0x0F,
		MAX_COLLISION_GROUP =	0x0F,
		NUM_COLLISION_FLAGS =	256,
		COLLISION_FLAG_SHIFT	=	4
	};

	void							Enable_Collision_Detection(int group0,int group1);
	void							Disable_Collision_Detection(int group0,int group1);
	void							Enable_All_Collision_Detections(int group);
	void							Disable_All_Collision_Detections(int group);
	bool							Do_Groups_Collide(int group0,int group1);

	/*
	** Cause the culling systems to re-partition themselves. (Typically only done in the EDITOR!)
	** The Update_Culling_System_Bounding_Boxes can be used by the EDITOR to refresh the
	** boxes in the culling systems without re-partitioning (and losing vis data...)
	*/
	void							Re_Partition_Static_Objects(void);
	void							Re_Partition_Static_Lights(void);
	void							Re_Partition_Static_Projectors(void);
	void							Re_Partition_Dynamic_Culling_System(void);
	void							Re_Partition_Dynamic_Culling_System(DynamicVectorClass<AABoxClass> & virtual_occludees);
	void							Update_Culling_System_Bounding_Boxes(void);
	bool							Verify_Culling_Systems(StringClass & set_error_report);

	/*
	** Visibility System.  
	** Enable_Vis - turn the vis system on or off
	** Invert_Vis - invert the logic of vis, draw what would be occulded
	** Set_Vis_Quick_And_Dirty - when generating, make non-occluders always visible (only used in preprocessing)
	** Enable_Vis_Sector_Display - highlight the current vis sector in wireframe
	** Enable_Vis_Sector_History_Display - highlight the last few vis sectors
	** Disable_Vis_Sector_Fallback - don't fall back to the previous sector when there is no sector for the current position
	*/
	void							Enable_Vis(bool onoff);									// on by default
	void							Invert_Vis(bool onoff);									// off by default
	void							Set_Vis_Quick_And_Dirty(bool onoff);				// off by default
	void							Enable_Vis_Sector_Display(bool onoff);				// off by default
	void							Enable_Vis_Sector_History_Display(bool onoff);	// off by default
	void							Enable_Vis_Sector_Fallback(bool onoff);			// on by default
	void							Enable_Backface_Occluder_Debug(bool onoff);

	bool							Is_Vis_Enabled(void);
	bool							Is_Vis_Inverted(void);
	bool							Is_Vis_Quick_And_Dirty(void);
	bool							Is_Vis_Sector_Display_Enabled(void);
	bool							Is_Vis_Sector_History_Display_Enabled(void);
	bool							Is_Vis_Sector_Missing(void);
	bool							Is_Vis_Sector_Fallback_Enabled(void);
	bool							Is_Backface_Occluder_Debug_Enabled(void);

	void							Lock_Vis_Sample_Point(bool onoff);
	bool							Is_Vis_Sample_Point_Locked(void);

	void							Compute_Vis_Sample_Point(const CameraClass & camera,Vector3 * set_point);
	VisTableClass *			Get_Vis_Table(int vis_sector_id);
	VisTableClass *			Get_Vis_Table(const Vector3 & point);
	VisTableClass *			Get_Vis_Table(const CameraClass & camera);
	VisTableClass *			Get_Vis_Table_For_Rendering(const CameraClass & camera);

	virtual void				On_Vis_Occluders_Rendered(VisRenderContextClass & /* context */,VisSampleClass & /* sample */) {}

	/*
	** Dynamic object visibility system debugging.  This system is no longer implemented
	** as a "grid" but rather an AABTree... 
	*/
	enum {
		VIS_GRID_DISPLAY_NONE = 0,
		VIS_GRID_DISPLAY_ACTUAL_BOXES,
		VIS_GRID_DISPLAY_CENTERS,
		VIS_GRID_DISPLAY_OCCUPIED
	};

	void							Set_Vis_Grid_Display_Mode(int mode);
	int							Get_Vis_Grid_Display_Mode(void);

	void							Vis_Grid_Debug_Reset_Node(void);
	bool							Vis_Grid_Debug_Enter_Parent(void);
	bool							Vis_Grid_Debug_Enter_Sibling(void);
	bool							Vis_Grid_Debug_Enter_Front_Child(void);
	bool							Vis_Grid_Debug_Enter_Back_Child(void);

	/*
	** Dynamic objects look up their vis-ID throught this method.  Note that if you pass in a
	** pointer to your previous node_id, the lookup will be faster and this variable will be modified
	** with the updated node id.  The first time this method is called, initialize the node_id to zero.
	*/
	uint32						Get_Dynamic_Object_Vis_ID(const AABoxClass & obj_bounds,int * node_id = NULL);
	void							Debug_Display_Dynamic_Vis_Node(int node_id);

	/*
	** Visibility Pre-Processing System.   (Typically only used in the EDITOR!)
	** Reset_Vis - marks the visibility data as dirty, to be discarded at the next opportunity
	** Validate_Vis - manually validates the visibility data (used after loading for example)
	** Allocate_Vis_Object_Id - reserve id(s) for objects whose visibility is to be tracked
	** Allocate_Vis_Sector_Id - reserve id(s) for vis sectors (view cells).
	** Get_Vis_Table_Size - returns the number of Vis Object ID's reserved
	** Get_Vis_Table_Count - returns the number of Vis Sector ID's reserved
	** Update_Vis - performs a vis-sample from the given position or camera
	** Export_Vis_Data - saves just the visibility data to a file
	** Import_Vis_Data - loads the visibility data from a file
	** Show_Vis_Window - enable display of the vis render window
	** Is_Vis_Window_Visibile - test whether the vis render window is being displayed
	** Generate_Vis_Statistics_Report - generates a report giving the amount of polygons and textures
	**                                  visible from each sector in the system.
	** Optimize_Visibility_Data - compresses vis by removing redundant vis-id's and pruning the dynamic culling
	**                            system.  This should only be called *after* all vis samples have been completed.
	*/
	virtual void				Reset_Vis(bool doitnow = false);
	void							Validate_Vis(void);
	int							Allocate_Vis_Object_ID(int count = 1);
	int							Allocate_Vis_Sector_ID(int count = 1);
	int							Get_Vis_Table_Size(void);		// also max vis *object* ID and vis object count.
	int							Get_Vis_Table_Count(void);		// also max vis *sector* ID and vis sector count.

	VisSampleClass				Update_Vis(const Matrix3D & camera,VisDirBitsType direction_bits = VIS_ALL);
	VisSampleClass				Update_Vis(const Vector3 & sample_point,const Matrix3D & camera,VisDirBitsType direction_bits = VIS_ALL,CameraClass * alternate_camera = NULL,int user_vis_id = -1);
	int							Get_Static_Light_Count(void);
	void							Generate_Vis_For_Light(int light_index);
	
	void							Export_Vis_Data(ChunkSaveClass & csave);
	void							Import_Vis_Data(ChunkLoadClass & cload);

	void							Show_Vis_Window(bool onoff);
	bool							Is_Vis_Window_Visible(void);

	void							Generate_Vis_Statistics_Report(DynamicVectorClass<VisSectorStatsClass> & report);	

	void							Optimize_Visibility_Data(VisOptProgressClass & progress_status);


	/*
	** Texture Projection System.
	*/
	void							Enable_Static_Projectors(bool onoff);
	bool							Are_Static_Projectors_Enabled(void);
	void							Enable_Dynamic_Projectors(bool onoff);
	bool							Are_Dynamic_Projectors_Enabled(void);

	void							Add_Static_Texture_Projector(TexProjectClass * newprojector);
	void							Remove_Static_Texture_Projector(TexProjectClass * projector);
	void							Add_Dynamic_Texture_Projector(TexProjectClass * newprojector);
	void							Remove_Dynamic_Texture_Projector(TexProjectClass * projector);
	
	void							Remove_Texture_Projector(TexProjectClass * projector);
	bool							Contains(TexProjectClass * projector);
	
	/*
	** Shadow system, built on top of the Texture Projection system.
	*/
	enum ShadowEnum
	{
		SHADOW_MODE_NONE = 0,			// no shadows at all
		SHADOW_MODE_BLOBS,				// projected blob shadows
		SHADOW_MODE_BLOBS_PLUS,			// projected blobs with main character having a rendered shadow
		SHADOW_MODE_HARDWARE,			// use render-to-texture hardware
		SHADOW_MODE_COUNT,
	};

	void							Set_Shadow_Mode(ShadowEnum shadow_mode);
	ShadowEnum					Get_Shadow_Mode(void);
	void							Set_Shadow_Attenuation(float znear,float zfar);
	void							Get_Shadow_Attenuation(float * set_znear,float * set_zfar);
	void							Set_Shadow_Normal_Intensity(float normal_intensity);
	float							Get_Shadow_Normal_Intensity(void);

	void							Set_Shadow_Resolution(unsigned int res);
	unsigned int				Get_Shadow_Resolution(void);
	void							Set_Max_Simultaneous_Shadows(unsigned int count);
	unsigned int				Get_Max_Simultaneous_Shadows(void);

	CameraClass *				Get_Shadow_Camera(void);
	SpecialRenderInfoClass *Get_Shadow_Render_Context(int width,int height);
	MaterialPassClass *		Get_Shadow_Material_Pass(void);

	/*
	** Generate (or re-generate) all static shadows. Invalidate must be called before calling Generate.
	*/
	void							Invalidate_Static_Shadow_Projectors(void);
	void							Generate_Static_Shadow_Projectors(void);
	void							Setup_Static_Directional_Shadow(StaticAnimPhysClass & obj,const Vector3 & light_dir,TextureClass * render_target);
	
	/*
	** Decal system
	** Project a decal onto the geometry in the vicinity of the given coordinate system
	*/
	int							Create_Decal(	const Matrix3D &	tm,
														const char *		texture_name,
														float					radius,
														bool					is_permanent = false,
														bool					apply_to_translucent_polys = false,
														PhysClass *			only_this_obj = NULL	);

	bool							Remove_Decal(uint32 id);

	/*
	** Shatter system
	** Shatter a mesh into many pieces and send them flying.
	*/
	void							Shatter_Mesh(	MeshClass * mesh,
														const Vector3 & impact_point,
														const Vector3 & impact_normal,
														const Vector3 & impact_velocity	);
	
	/*
	** Camera Shake System
	** Drop in camera shake spheres and any camera in the vicinity will automatically be affected.
	** Prior to calling WW3D::Render(scene...) you need to pass your camera into Apply_Camera_Shakes
	*/
	void							Add_Camera_Shake(	const Vector3 & position,
															float radius = 50.0f,
															float duration = 1.5f,
															float power = 1.0f	);
	void							Apply_Camera_Shakes( CameraClass & camera );

	/*
	** Temporary accessors for the level editor.
	*/
	bool							Is_Vis_Reset_Needed (void) const	{ return VisResetNeeded; }
	void							Vis_Reset_Needed(bool needed)		{ VisResetNeeded = needed; }


	/*
	** Save-Load system.  There are three save-load sub-systems in the physics library.  
	** PhysStaticDataSaveSystem - saves static data like pathfind, vis, and static culling data.
	** PhysStaticObjectsSaveSystem - saves static objects like terrain pieces which have no dynamic state.
	** PhysDynamicDataSaveSystem - saves dynamic data, settings, and objects in the physics scene
	*/
	void							Save_Level_Static_Data(ChunkSaveClass & csave);
	void							Load_Level_Static_Data(ChunkLoadClass & cload);
	void							Post_Load_Level_Static_Data(void);

	void							Save_Level_Static_Objects(ChunkSaveClass & csave);
	void							Load_Level_Static_Objects(ChunkLoadClass & cload);
	void							Post_Load_Level_Static_Objects(void);

	void							Save_Level_Dynamic_Data(ChunkSaveClass & csave);
	void							Load_Level_Dynamic_Data(ChunkLoadClass & cload);
	void							Post_Load_Level_Dynamic_Data(void);


	/*
	** Control over the polygon budget
	*/
	void							Set_Polygon_Budgets(int static_count,int dynamic_count);
	void							Get_Polygon_Budgets(int * static_count,int * dynamic_count);


	void							Set_Update_Only_Visible_Objects(bool b) { UpdateOnlyVisibleObjects=b; }
	bool							Get_Update_Only_Visible_Objects() { return UpdateOnlyVisibleObjects; }

	/*
	** Scene Class methods.  These should *only* be used when absolutely necessary since
	** it is more efficient to operate through the physics interface (I can keep track
	** of whether an object has moved and update its culling etc).  If you insert a render
	** object through this interface, a DecoPhys object will be created for it and added to
	** a "Dirty Cullers" list.  The "Dirty Cullers" list will assume that those objects
	** have changed shape or moved every frame and always re-insert the object into the
	** culling system.  This code is needed in order to support render objects that
	** add other render objects to the scene like particle emitters.
	** Still don't support the iterator interface... maybe will some day...
	*/
	virtual void				Add_Render_Object(RenderObjClass * obj) override;
	virtual void				Remove_Render_Object(RenderObjClass * obj) override;
	virtual SceneIterator *	Create_Iterator(bool /*onlyvisible = false*/) override	{ assert(0); return NULL; }
	virtual void				Destroy_Iterator(SceneIterator * /*it*/) override			{ assert(0); }
	virtual void				Register(RenderObjClass * obj,RegType for_what) override;
	virtual void				Unregister(RenderObjClass * obj,RegType for_what) override;


	/*
	**	Accessors
	*/
	void							Get_Level_Extents (Vector3 &min, Vector3 &max);

	/*
	** Debugging control.  Turn on rendering of debug vectors, points, bounding boxes, etc.
	*/
	void							Enable_Debug_Display(bool onoff)					{ DebugDisplayEnabled = onoff; }
	bool							Is_Debug_Display_Enabled(void)					{ return DebugDisplayEnabled; }
	void							Enable_Projector_Debug_Display(bool onoff)	{ ProjectorDebugDisplayEnabled = onoff; }
	bool							Is_Projector_Debug_Display_Enabled(void)		{ return ProjectorDebugDisplayEnabled; }
	void							Enable_Dirty_Cull_Debug_Display(bool onoff)	{ DirtyCullDebugDisplayEnabled = onoff; }
	bool							Is_Dirty_Cull_Debug_Display_Enabled(void)		{ return DirtyCullDebugDisplayEnabled; }
	void							Enable_Lighting_Debug_Display(bool onoff)		{ LightingDebugDisplayEnabled = onoff; }
	bool							Is_Lighting_Debug_Display_Enabled(void)		{ return LightingDebugDisplayEnabled; }
	const Vector3 &			Get_Last_Camera_Position(void)					{ return LastCameraPosition; }

	/*
	** Performance Statistics
	*/
	struct StatsStruct
	{
		StatsStruct(void);
		void	Reset(void);

		int	FrameCount;
		int	CullNodeCount;
		int	CullNodesAccepted;
		int	CullNodesTriviallyAccepted;
		int	CullNodesRejected;

	};

	void							Per_Frame_Statistics_Update(void);
	const StatsStruct &		Get_Statistics(void);
	float							Compute_Vis_Mesh_Ram(void);

protected:
	
	/*
	** Rendering functions
	*/
	virtual void				Customized_Render(RenderInfoClass & rinfo) override;
	void							Render_Objects(RenderInfoClass& rinfo,RefPhysListClass * static_ws_list,RefPhysListClass * static_list,RefPhysListClass * dyn_list);
	void							Render_Object(RenderInfoClass & context,PhysClass * obj);
	void							Render_Backface_Occluders(RenderInfoClass & context,RefPhysListClass *  static_ws_list,RefPhysListClass * static_list);

	void							Optimize_LODs(	CameraClass & camera,
														RefPhysListClass * dyn_obj_list,
														RefPhysListClass * static_obj_list,
														RefPhysListClass * static_ws_mesh_list	);
	/*
	** Internal Vis functions.  These are used to generate the VIS data.
	*/
	void							Release_Vis_Resources(void);
	virtual void				Internal_Vis_Reset(void);
	CameraClass *				Get_Vis_Camera(void);
	void							Vis_Render_And_Scan(VisRenderContextClass & context,VisSampleClass & sample);
	void							Merge_Vis_Sector_IDs(uint32 id0,uint32 id1);
	void							Merge_Vis_Object_IDs(uint32 id0,uint32 id1);

	/*
	** Internal texture-projection functions
	*/
public:
	void							Release_Projector_Resources(void);
protected:
	void							Apply_Projectors(const CameraClass & camera);
	void							Apply_Projector_To_Objects(TexProjectClass * tex_proj,const CameraClass & camera);
	float							Compute_Projector_Attenuation(TexProjectClass * dynamic_projector,const Vector3 & view_pos,const Vector3 & view_dir);

	/*
	** Internal decal functions
	*/
	void							Allocate_Decal_Resources(void);
	void							Release_Decal_Resources(void);

	/*
	** Internal save-load code
	*/
	void							Save_LDD_Variables(ChunkSaveClass & csave);
	void							Save_Static_Objects(ChunkSaveClass & csave);
	void							Save_Dynamic_Objects(ChunkSaveClass & csave);
	void							Save_Static_Lights(ChunkSaveClass & csave);
	void							Save_Static_Object_States(ChunkSaveClass & csave);

	void							Load_LDD_Variables(ChunkLoadClass & cload);
	void							Load_Static_Objects(ChunkLoadClass & cload);
	void							Load_Dynamic_Objects(ChunkLoadClass & cload);
	void							Load_Static_Lights(ChunkLoadClass & cload);
	void							Load_Static_Object_States(ChunkLoadClass & cload);

	/*
	** Misc
	*/
	void							Internal_Add_Dynamic_Object(PhysClass * newobj);
	void							Internal_Add_Static_Object(StaticPhysClass * newtile);
	void							Internal_Add_Static_Light(LightPhysClass * newlight);

	void							Reset_Sun_Light(void);
	void							Load_Sun_Light(ChunkLoadClass & cload);
	void							Save_Sun_Light(ChunkSaveClass & csave);

	void							Add_Collected_Objects_To_List(bool static_objs,bool dynamic_objs,NonRefPhysListClass * list);
	void							Add_Collected_Collideable_Objects_To_List(int colgroup,bool static_objs,bool dynamic_objs,NonRefPhysListClass * list);
	void							Add_Collected_Lights_To_List(bool static_lights,bool dynamic_lights,NonRefPhysListClass * list);

	//- Volatile or Constant Member Variables -------------------------------------------------------------
	// 
	// These variables do not need to be saved or loaded for one of the following reasons:
	// - They are created/initialized when the physics scene is constructed and never change
	// - They are simple flags or variables that just store the last thing the user set
	//
	//-----------------------------------------------------------------------------------------------------
	
	/*
	** Cached Data during the Pre_Render, Render, and Post_Render sequence
	*/
	RefPhysListClass			VisibleStaticObjectList;
	RefPhysListClass			VisibleWSMeshList;
	RefPhysListClass			VisibleDynamicObjectList;
	TexProjListClass			ActiveTextureProjectors;

	/*
	** Current frame number, last camera position, etc
	*/
	int							FrameNum;
	Vector3						LastCameraPosition;
	int							LastValidVisId;			// id of the last valid vis sector
	bool							DebugDisplayEnabled;
	bool							ProjectorDebugDisplayEnabled;
	bool							DirtyCullDebugDisplayEnabled;
	bool							LightingDebugDisplayEnabled;
	StatsStruct					LastValidStats;			// last complete statistics sample
	StatsStruct					CurrentStats;				// currently accumulating statistics sample
	bool							StaticProjectorsDirty;

	/*
	** Visibility system variables
	*/
	enum 
	{ 
		VIS_RENDER_WIDTH = 256,								// resolution width for the vis render window
		VIS_RENDER_HEIGHT = 256								// resolution height for the vis render window
	};						

	bool							VisEnabled;					// use vis data if present? (default true)
	bool							VisInverted;				// invert vis data if present? (default false)
	bool							VisQuickAndDirty;			// ignore non-occluders in generating vis? (quick-n-dirty vis)
	bool							VisResetNeeded;			// indicates that the vis system is to be reset

	bool							VisSectorDisplayEnabled;// debugging, highlight the vis sector mesh
	bool							VisSectorHistoryEnabled;// debugging, highlight the previous 3 vis sectors.
	int							VisGridDisplayMode;		// debugging, display the visible grid cells
	bool							VisSectorMissing;			// debugging, true when no vis sector was found!
	bool							VisSectorFallbackEnabled;	// Use the last valid vis sector when no sector is found
	bool							BackfaceDebugEnabled;	// is backface debugging enabled.
	bool							VisSamplePointLocked;	// is the sample point for vis being over-ridden?
	Vector3						LockedVisSamplePoint;	// position to sample vis from when locked/overridden.

	CameraClass *				VisCamera;					// camera set up for vis-rendering
	VisTableClass *			CurrentVisTable;			// current active vis table

	/*
	** Shadow system variables
	*/
	bool							StaticProjectorsEnabled;	// toggle static shadows (shadows cast by static objs onto dynamic objs)
	bool							DynamicProjectorsEnabled;	// toggle dynamic shadows (shadows cast by dynamic objs onto everything)
	ShadowEnum					ShadowMode;						// current shadow mode
	float							ShadowAttenStart;				// distance to start of shadow attenuation							
	float							ShadowAttenEnd;				// distance to end of shadow attenuation
	float							ShadowNormalIntensity;		// "normal" non attenuated shadow intensity
	TextureClass *				ShadowBlobTexture;			// texture to use for fake "blob" shadows

	SpecialRenderInfoClass *ShadowRenderContext;			// render context for shadows																	
	CameraClass *				ShadowCamera;					// camera for rendering shadow textures
	MaterialPassClass *		ShadowMaterialPass;			// material pass for shadows
	int							ShadowResWidth;
	int							ShadowResHeight;

	/*
	** Decal System
	*/
	PhysDecalSysClass *		DecalSystem;					

	/*
	** Pathfinding
	*/
	PathfindClass *			Pathfinder;

	/*
	** Camera Shaking
	*/
	CameraShakeSystemClass *CameraShakeSystem;

	/*
	** Render objects processing lists
	*/
	RefRenderObjListClass	UpdateList;
	RefRenderObjListClass	VertexProcList;
	RefPhysListClass			ReleaseList;

	/*
	** Miscellaneous resources
	*/
	MaterialPassClass *		HighlightMaterialPass;


	//- Level Static Data ---------------------------------------------------------------------------------
	// 
	// This data is constant throughout the course of a level and therefore is stored in the 
	// LSD (L)evel (S)tatic (D)ata file.
	// - The structure of the Static Object AABTree
	// - The structure of the Static Light AABTree
	// - The structure of the Dynamic Object culling grid
	// - All of the StaticPhysClasses and StaticLightClasses (in StaticObjList and StaticLightList)
	//
	//-----------------------------------------------------------------------------------------------------

	/*
	** Culling systems.  These will have a structure which is based on the 
	** static terrain for the level and do not change during the course of a level (though
	** the objects may link and unlink from them)
	*/
	StaticAABTreeCullClass *										StaticCullingSystem;
	StaticLightCullClass *											StaticLightingSystem;
	PhysGridCullClass *												DynamicCullingSystem;
	DynamicAABTreeCullClass *										DynamicObjVisSystem;

	TypedAABTreeCullSystemClass<TexProjectClass> *			StaticProjectorCullingSystem;
	TypedGridCullSystemClass<TexProjectClass> *				DynamicProjectorCullingSystem;
	
	/*
	** Visibility Tables
	*/
	VisTableMgrClass			VisTableManager;

	/*
	** Lighting:
	** Sun Light.  The sun is special cased to generate shadows and a lens
	** flare.  It should not be changed unless you are in the level editor.
	*/
	int							LightingMode;
	Vector3						SceneAmbientLight;
	bool							UseSun;
	float							SunPitch;
	float							SunYaw;
	LightClass *				SunLight;
	

	//- Level Dynamic Data --------------------------------------------------------------------------------
	// 
	// This is the dynamic data/objects which all need to be saved in either the LDD file or SAV file.
	//
	//-----------------------------------------------------------------------------------------------------

	/*
	** Collision group tables
	*/
	static bool					AllowCollisionFlags[NUM_COLLISION_FLAGS];

	/*
	** Polygon budget for the LOD system
	*/
	int							DynamicPolyBudget;
	int							StaticPolyBudget;

	/*
	** Master lists of the objects in the system.   Each object will be in *one* of the 
	** following lists.
	*/
	RefPhysListClass			ObjList;
	RefPhysListClass			StaticObjList;
	RefPhysListClass			StaticLightList;
	TexProjListClass			StaticProjectorList;
	TexProjListClass			DynamicProjectorList;

	/*
	** Auxiliary lists.  These lists are used to perform certain special
	** processing on certain objects or to avoid performing certain operations
	** on objects.  A given object can be in any or all of these lists...
	*/
	RefPhysListClass			DirtyCullList;		// objects that have 'dirty culling' must be re-inserted each frame...
	RefPhysListClass			TimestepList;		// objects which need to be time-stepped go in here
	RefPhysListClass			StaticAnimList;	// list of the StaticAnim objects, these can cast shadows, change states, etc
	NonRefPhysListClass		CollisionRegionList;	// cached list of objects in the current collision region

	bool							UpdateOnlyVisibleObjects;
	unsigned						CurrentFrameNumber;

private:
	
	/*
	** The physics scene is a singleton.  If we want to support multiple 
	** physics scenes, we would need to add a scene pointer to each physics object
	** and everywhere they are calling Get_Instance() they can just use their pointer
	*/
	static PhysicsSceneClass * TheScene;

	/*
	** WW3D is a friend because we let it call our protected Render function (which no
	** one else is allowed to call... trying to "contain" surrender :-)
	*/
	friend class WW3D;

	/*
	** VisOptimizationContextClass is a friend so that it can merge vis-id's.  
	*/
	friend class VisOptimizationContextClass;

};



	

#endif