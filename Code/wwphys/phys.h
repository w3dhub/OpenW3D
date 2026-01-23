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
 *                     $Archive:: /Commando/Code/wwphys/phys.h                                $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 3/08/02 5:01p                                               $*
 *                                                                                             *
 *                    $Revision:: 89                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PHYS_H
#define PHYS_H

#include "always.h"
#include "refcount.h"
#include "matrix3d.h"
#include "physobserver.h"
#include "cullsys.h"
#include "rendobj.h"
#include "widgetuser.h"
#include "persist.h"
#include "editable.h"
#include "definition.h"
#include "multilist.h"
#include "phystexproject.h"
#include "wwstring.h"
#include "materialeffect.h"
#include "materialeffectlist.h"
#include "wwstring.h"

#include "umbrasupport.h"


class	CullSystemClass;
class	CullLinkClass;
class AABoxClass;
class CameraClass;
class srScene;
class srGERD;
class RenderInfoClass;
class SpecialRenderInfoClass;
class PhysClass;
class PhysRayCollisionTestClass;
class PhysAABoxCollisionTestClass;
class PhysOBBoxCollisionTestClass;
class PhysAABoxIntersectionTestClass;
class PhysOBBoxIntersectionTestClass;
class PhysMeshIntersectionTestClass;
class AABTreeNodeClass;
class CullLinkClass;
class BitStreamClass;
class LightEnvironmentClass;

class DynamicPhysClass;
class MoveablePhysClass;
class Phys3Class;
class HumanPhysClass;
class RigidBodyClass;
class VehiclePhysClass;
class MotorVehicleClass;
class WheeledVehicleClass;
class MotorcycleClass;
class	TrackedVehicleClass;
class VTOLVehicleClass;
class StaticPhysClass;
class StaticAnimPhysClass;
class RenderObjPhysClass;
class DecorationPhysClass;
class TimedDecorationPhysClass;
class DynamicAnimPhysClass;
class RenderObjPhysClass;
class ProjectileClass;
class LightPhysClass;
class ElevatorPhysClass;
class DamageableStaticPhysClass;
class DoorPhysClass;
class AccessiblePhysClass;

class PhysDefClass;

#if (UMBRASUPPORT)
namespace Umbra { class Object; }
#endif




/*
** Macros for rendering debug widgets.  In the release build, these will automatically
** be removed.  These macros can be used inside a member function of any physics object
** or by calling them through a pointer to a physics object e.g.  my_phys_obj->DEBUG_RENDER_POINT(...)
*/
#ifdef WWDEBUG
#define DEBUG_RENDER_POINT(v,c)		Add_Debug_Point(v,c)
#define DEBUG_RENDER_VECTOR(p,v,c)	Add_Debug_Vector(p,v,c)
#define DEBUG_RENDER_AABOX(box,c,o)	Add_Debug_AABox(box,c,o)
#define DEBUG_RENDER_OBBOX(box,c,o)	Add_Debug_OBBox(box,c,o)
#define DEBUG_RENDER_AXES(tm,c)		Add_Debug_Axes(tm,c)
#else
#define DEBUG_RENDER_POINT(v,c)		
#define DEBUG_RENDER_VECTOR(p,v,c)	
#define DEBUG_RENDER_AABOX(box,c,o)
#define DEBUG_RENDER_OBBOX(box,c,o)
#define DEBUG_RENDER_AXES(tm,c)		
#endif



/***************************************************************************************

Physics Class Hierarchy (Pre-Apr 98):
------------------------------------
					  
                     PhysObjClass                        
                          |_______________________________
					           |                               |
					    MoveableObjClass                TerrainPhysClass
              ____________|_______________________________________
             |                             |					         |        
         Phys4Class                    Phys6Class             Phys3Class
             |                             |
        CharPhysClass        ______________|______________________________________
                            |                           |                         |
                      WheeledPhysClass           TrackedPhysClass          HoverPhysClass


  

Physics Class Hierarchy (Apr 98):
---------------------------------


  MoveableObjClass   RefCountClass  CollideableObjClass
         |                 |                |
			|_________________|________________|
                           |
 	   						PhysObjClass
		                     |
         __________________|_________________________________________________________________
		  |                  |                 |                                               |
   ProjectileClass     Phys4Class        Phys6Class                                     TerrainPhysClass
		                     |              ___|_________________________________
                           |             |                   |                 |
                     CharPhysClass  WheeledPhysClass   TrackedPhysClass   HoverPhysClass
						 

	NOTES:
	
	- CollideableClass and MoveableClass are organizational only.  I could just
	put all of their interfaces directly into PhysObjClass.

	- TerrainPhysClass's never move and are complex meshes which do polygon-level 
	collision detection

	- Projectiles can never be collided with, they collide with others, they

	- Physics system should store all "Collideable" objects in a list or
	actually in a spatial data structure for collision checking.

	- Physics system should also store all "Moveable" objects in another list
	for updating everything's motions (or should that be up to the game engine?)
 
   - Bullets are Phys3Class's, nothing can collide with them, they will use
	ray casting for collision, they do not have "size" or "orientation"

	- TerrainPhys is a big object that doesn't move and has complex geometry.

	- A lot of things will be Phys4Classes, these are axis-aligned boxes to
	the collision system.  They can move in x,y, and z and rotate inside their box


Physics Class Hierarchy (Sept 98):
----------------------------------

  PhysClass Hierarchy:

  Starting from scratch in some respects.  Culling and collision detection is the 
  main focus of the lowest levels of the Phys class hierarchy.  Wrote a new base
  class 'PhysClass' which everything will be derived from.  It must be as lightweight
  as possible since now everything rendered in the game scene must be a PhysClass.


							PhysClass
								 |
					----------|-----------------------------
					|													| 
	   	Decoration											Motion			
																		|
	
														Projectile			Controllable		
								
																				Char		Vehicle    



Dec 1, 1998

  Improving the culling linkage system.  Physics object now only contain a pointer
  to an abstract link type which each culling system can hang anything they want
  to on.  This is better than the old union inside of the Phys object because some
  of these data structures are going to have pointers to allocated memory and I didn't
  like the idea of having those kinds of things shadowed inside of a union.  The base
  CullLinkClass only contains an RTTI function which should rarely need to be called.

  Adding Occlusion culling...

Jan 5, 1999

  Basic Occlusion culling is working.  Adding Load/Save for static scene, re-organizing
  so that the AAB stuff can handle things like lights and zones.

Jan 15, 1999

  Improving list code to automatically track refs.  An object added to a "phys list" 
  will be automatically Add_Ref'd.  Moved the list code into a separate module for
  easier reading (physlist.h, cpp)

May 21, 1999
	
  Splitting much of the culling system code out into WWMath so that it can be re-used
  by other libraries such as the sound system.  Removing "CullableClass" from the 
  physics library and re-writing the culling stuf to not rely on phys-objects (only
  an abstract "cullable" object).

Sept 1, 1999

  Making the physics library use the new Persistant-Object save/load code.  This means
  that each type of PhysClass is a PersistClass now.  Each one needs to implement
  save/load methods and declare a PersistFactoryClass.  The physics system will have
  two SaveLoadSubSystems in it.  One for saving the data which is completely static
  for a level and another for the dynamic data.  Things that are treated as 100% static
  are: visibility data, pathfinding data, and culling datastructures (but not the 
  objects in them...)

August 29, 2000

	Adding  Export_State,Import_State functions which can be used by the App to do 
	network communication of physics object states.


****************************************************************************************/



/**
** PhysClass
** This is the base class for all objects in the physics system.
*/
class PhysClass : public CullableClass, public PersistClass, public MultiListObjectClass
{
public:

	PhysClass(void);
//	PhysClass(const PhysDefClass & def);
	virtual ~PhysClass(void);
	void		Init(const PhysDefClass & def);

	/*
	** DEBUGGING, re-initialize this object because our definition changed
	*/
	virtual void					 Definition_Changed(void)					{ }
	
	/*
	** Certain physics objects expire when an internal timer runs out or they are crushed.  This function
	** is used to handle informing any observers and putting this object into the destroy list which is 
	** processed at the end of each frame
	*/
	bool								Expire(void);

	/*
	** Timestep function - system informs the object to update itself for the specified
	** amount of time.
	*/
	virtual bool					Needs_Timestep(void)							{ return false; }
	virtual void					Timestep(float dt)							= 0; 
	virtual void					Post_Timestep_Process(void)				{ }

	/*
	** Access to the Position/Orientation state of the object
	*/
	virtual const Matrix3D &	Get_Transform(void) const					= 0;
	virtual void					Set_Transform(const Matrix3D & m)		= 0;
	void								Get_Position(Vector3 * set_pos) const	{ Get_Transform().Get_Translation(set_pos); }
	void								Set_Position(const Vector3 & pos)		{ Matrix3D tm = Get_Transform(); tm.Set_Translation(pos); Set_Transform(tm); }
	float								Get_Facing(void) const						{ return Get_Transform().Get_Z_Rotation(); }
	void								Set_Facing(float new_facing);

	/*
	** Collision detection - all collideable objects provide the following collision detection
	** functions so that other objects do not pass through them.  These functions should test
	** the given primitive against this object's geometric representation.
	*/
	virtual bool					Cast_Ray(PhysRayCollisionTestClass & /* raytest */)		{ return false; }
	virtual bool					Cast_AABox(PhysAABoxCollisionTestClass & /* boxtest */)	{ return false; }
	virtual bool					Cast_OBBox(PhysOBBoxCollisionTestClass & /* boxtest */)	{ return false; }

	virtual bool					Intersection_Test(PhysAABoxIntersectionTestClass & /* test */)		{ return false; }
	virtual bool					Intersection_Test(PhysOBBoxIntersectionTestClass & /* test */)		{ return false; }
	virtual bool					Intersection_Test(PhysMeshIntersectionTestClass & /* test */)		{ return false; }

	/*
	** Inter-Object Geometric Dependency.  These functions don't really perform any 
	** physics-based motion, only purely kinematic.  You can link a moveable physics object
	** (the rider) to another object that it is standing on (the carrier).  This will cause 
	** the carrier to move the rider whenever it moves (by calling Push).
	**
	** Set_Carrier - when a carrier is being destroyed, it tells all riders as it unlinks them
	** Push - carriers push their riders around, also objects not being carried can be pushed
	** Internal_Link_Rider - an object is being attached to you, move him when you move
	** Internal_Unlink_Rider - stop moving this object when you move.
	*/
	virtual void					Link_To_Carrier(PhysClass * /* carrier */,RenderObjClass * /* carrier_sub_obj */ = NULL)	{ }
	virtual RenderObjClass *	Peek_Carrier_Sub_Object(void)											{ return NULL; }
	virtual bool					Push(const Vector3 & /* move */)												{ return false; }
	
	virtual bool					Internal_Link_Rider(PhysClass * /* rider */)								{ return false; }
	virtual bool					Internal_Unlink_Rider(PhysClass * /* rider */)							{ return false; }

	/*
	** Culling, this function updates the culling box used by the object.  The default implementation
	** is to copy the bounding box of the current Model.  
	*/
	void								Update_Cull_Box(void);

	/*
	** Set the model used by this physics object
	*/
	virtual void					Set_Model(RenderObjClass * model);
	void								Set_Model_By_Name(const char * model_type_name);
	RenderObjClass *				Get_Model(void);
	WWINLINE RenderObjClass *	Peek_Model(void) { return Model; }

	/*
	** Set the name of this physics model instance
	*/
	void								Set_Name(const char * name);
	const char *					Get_Name(void);

	/*
	** Instance ID related methods
	*/
	uint32							Get_ID(void) const											{ return InstanceID; }
	void								Set_ID(uint32 id)												{ InstanceID = id; }

	/*
	** Vis Object ID.  Every phys object can store a vis object id.  Static objects will
	** have constant ID's assigned by the vis generation process, dynamic objects will
	** update their id based on their current location.
	*/
	void								Set_Vis_Object_ID(int new_id)								{ VisObjectID = new_id; }
	virtual int						Get_Vis_Object_ID(void)										{ return VisObjectID; }

	/*
	** This is just a shortcut to rendering the phys object's render object if it has one
	*/
	virtual void					Render(RenderInfoClass & rinfo);
	virtual void					Vis_Render(SpecialRenderInfoClass & rinfo);
	
	/*
	** Lighting system.  Each physics object caches its static lighting environment.  This cache must be
	** invalidated when the object moves or when static lights near it change state.  The simulation code
	** for each physics object will be responsible for invalidating the cache when the object moves.  The
	** scene will be responsible for invalidating the caches of objects near lights that change state.
	*/
	void								Invalidate_Static_Lighting_Cache(void);
	LightEnvironmentClass *		Get_Static_Lighting_Environment(void);

	/*
	** This is a debugging feature which causes all vis-sector meshes to be rendered
	*/
	virtual void					Render_Vis_Meshes(RenderInfoClass & /* rinfo */)			{ }

	/*
	** When rendering shadows in BLOB mode, this function defines the bounds of
	** the blob.  Derived classes will implement this to return something like
	** their collision box.
	*/
	virtual void					Get_Shadow_Blob_Box(AABoxClass * set_obj_space_box);

	/*
	** This function is used to determine whether an object is projecting a shadow.
	** If the object is projecting a shadow, it shouldn't be considered an occluder
	** for the sun.
	*/
	virtual bool					Is_Casting_Shadow(void)										{ return false; }

	/*
	** Material Effect Support.
	** As the scene is rendered, the textures being projected onto each object will be
	** linked in through the following interface.  Then when this object is rendered,
	** it should push those material passes into the render pipeline.  As this is done,
	** the list will be reset.  Other user-created material effects can also be
	** linked to the object through this interface.
	*/
	void								Add_Effect_To_Me(MaterialEffectClass * effect);
	void								Remove_Effect_From_Me(MaterialEffectClass * effect);
	bool								Do_Any_Effects_Suppress_Shadows(void);

	/*
	** Set the Collision Group for this physics object.  The collision group is an integer
	** between 0 and 15.  Collisions between any two groups can be enabled/disabled through
	** the physics system.
	*/
	void								Set_Collision_Group(unsigned char group)				{ group &= COLLISION_MASK; Flags &= ~COLLISION_MASK; Flags |= group; }
	unsigned char					Get_Collision_Group(void) const							{ return Flags & COLLISION_MASK; }


	/*
	** The IGNOREME state is basically used when the object is processing its move.  It
	** is a way of temporarily making the collision system ignore an object (so you don't
	** collide with yourself for example)
	*/
	void								Inc_Ignore_Counter(void);
	void								Dec_Ignore_Counter(void);
	bool								Is_Ignore_Me(void) const									{ return ((Flags & IGNORE_MASK) > 0); }


	/*
	** The IMMOVABLE state is used to turn off an object's simulation.
	*/
	void								Set_Immovable(bool onoff)									{ Set_Flag(IMMOVABLE,onoff); }
	bool								Is_Immovable(void) const									{ return Get_Flag(IMMOVABLE); }

	/*
	** Some objects (like light sources) can be disabled.
	*/
	void								Set_Disabled(bool onoff)									{ Set_Flag(DISABLED,onoff); }
	bool								Is_Disabled(void) const										{ return Get_Flag(DISABLED); }

	/*
	** The Debug Display flag is used (in debug builds) to enable displaying 
	** contact points, impulse vectors, etc.
	*/
	void								Enable_Debug_Display(bool onoff)							{ (onoff ? Flags |= DEBUGDISPLAY : Flags &= ~DEBUGDISPLAY); }
	bool								Is_Debug_Display_Enabled(void) const;

	/*
	** User Control.  Enabling this flag makes the physics object ingore all
	** physics and just move according to its controller
	*/
	void								Enable_User_Control(bool onoff)							{ Set_Flag(USERCONTROL,onoff); Set_Flag(ASLEEP,false); }
	bool								Is_User_Control_Enabled(void) const						{ return Get_Flag(USERCONTROL); }

	/*
	** Shadow casting.  If this option is enabled, a shadow volume will be
	** calculated for the object each frame
	*/
	void								Enable_Shadow_Generation(bool onoff)					{ Set_Flag(CASTSHADOW,onoff); }
	bool								Is_Shadow_Generation_Enabled(void) const				{ return Get_Flag(CASTSHADOW); }

	/*
	** Blob Shadow override.  There is a shadow mode where all objects get blob
	** shadows except the "main character".  This option indicates that this physics
	** object should cast a proper shadow when the shadow mode is "BLOBS_PLUS"
	*/
	void								Enable_Force_Projection_Shadow(bool onoff)			{ Set_Flag(FORCE_PROJECTION_SHADOW,onoff); }
	bool								Is_Force_Projection_Shadow_Enabled(void) const		{ return Get_Flag(FORCE_PROJECTION_SHADOW); }

	/*
	** DontSave!  This is used for transient objects like glass fragments which
	** we don't want to save.  There is also a problem with them saving caused by
	** the fact that thier model is procedurally generated and doesn't save...
	*/
	void								Enable_Dont_Save(bool onoff)								{ Set_Flag(DONT_SAVE,onoff); }
	bool								Is_Dont_Save_Enabled(void) const							{ return Get_Flag(DONT_SAVE); }

	/*
	** Asleep.  This indicates that the phyics object has "settled" into a stable
	** configuration and has stopped simulating.  Physics objects internally manage this flag.
	*/
	bool								Is_Asleep(void) const										{ return ((Flags & ASLEEP) == ASLEEP); }
	void								Force_Awake(void)												{ Set_Flag(ASLEEP,false); }

	/*
	** Static-World-Space-Mesh.  This flag indicates that the phys object is a static world
	** space mesh (identity transform) and enables some optimizations in the rendering loop.
	*/
	void								Enable_Is_World_Space_Mesh(bool onoff)					{ Set_Flag(IS_WS_MESH,onoff); }
	bool								Is_World_Space_Mesh(void)									{ return Get_Flag(IS_WS_MESH); }

	/*
	** Is Pre-Lit.  This flag indicates that this object has precomputed light
	** maps and does not need to have the static lights applied to it.
	*/
	void								Enable_Is_Pre_Lit(bool onoff)								{ Set_Flag(IS_PRE_LIT,onoff); }
	bool								Is_Pre_Lit(void); //												{ return Get_Flag(IS_PRE_LIT) | Model->Get_F; }

	/*
	** Is In-the-sun.  This flag indicates whether the object is being illuminated by
	** sunlight.
	*/
	void								Enable_Is_In_The_Sun(bool onoff)							{ Set_Flag(IS_IN_THE_SUN,onoff); }
	bool								Is_In_The_Sun(void)											{ return Get_Flag(IS_IN_THE_SUN); }

	/*
	** Is_State_Dirty.  This flag indicates that the physics code has changed the state of
	** this object.  It will be set whenever an object changes but it is only cleared by the
	** external user.
	*/
	void								Enable_Is_State_Dirty(bool onoff)						{ Set_Flag(IS_STATE_DIRTY,onoff); }
	bool								Is_State_Dirty(void)											{ return Get_Flag(IS_STATE_DIRTY); }

	/*
	** Is Object's Simulation Enabled.  This flag can be used to disable the physics simulation
	** on an object-by-object basis.  The other simulation methods disable whole classes
	** of objects.
	*/
	void								Enable_Objects_Simulation(bool onoff)					{ Set_Flag(SIMULATION_DISABLED,!onoff); }
	bool								Is_Objects_Simulation_Enabled(void) const				{ return !Get_Flag(SIMULATION_DISABLED); }
	bool								Is_Object_Simulating(void)									{ return Is_Objects_Simulation_Enabled() && !Is_Simulation_Disabled(); }

	/*
	** If you install a collision observer, it will be notified of any collisions which occur 
	** involving this object.  One way to do this is to derive your game objects from 
	** CollisionObserverClass so that they can be directly installed here.  Currently, there
	** can only be one observer for any physics object.  You are responsible for removing
	** the observer before this phys object is destroyed: Set_Observer(NULL)
	*/
	void								Set_Observer(PhysObserverClass * o)						{ Observer = o; }
	PhysObserverClass *			Get_Observer(void)											{ return Observer; }
	CollisionReactionType		Collision_Occurred(CollisionEventClass & event);

	/*
	** Definition access.  Many physics objects are created from a "definition".  The definition
	** object contains constants.  If this object has a definition, you can access it here.
	*/
	const PhysDefClass *				Get_Definition(void)										{ return Definition; }

	/*
	** Physics RTTI.
	*/
	virtual DynamicPhysClass *		As_DynamicPhysClass(void)								{ return NULL; }
	virtual MoveablePhysClass *	As_MoveablePhysClass(void)								{ return NULL; }
	virtual Phys3Class *				As_Phys3Class(void)										{ return NULL; }
	virtual HumanPhysClass *		As_HumanPhysClass(void)									{ return NULL; }
	virtual RigidBodyClass *		As_RigidBodyClass(void)									{ return NULL; }
	virtual VehiclePhysClass *		As_VehiclePhysClass(void)								{ return NULL;	}
	virtual MotorVehicleClass *	As_MotorVehicleClass(void)								{ return NULL; }
	virtual WheeledVehicleClass *	As_WheeledVehicleClass(void)							{ return NULL;	}
	virtual MotorcycleClass *		As_MotorcycleClass(void)								{ return NULL; }
	virtual TrackedVehicleClass *	As_TrackedVehicleClass(void)							{ return NULL; }
	virtual VTOLVehicleClass *		As_VTOLVehicleClass(void)								{ return NULL; }
	virtual StaticPhysClass *		As_StaticPhysClass(void)								{ return NULL; }
	virtual StaticAnimPhysClass *	As_StaticAnimPhysClass(void)							{ return NULL; }
	virtual ElevatorPhysClass *	As_ElevatorPhysClass(void)								{ return NULL; }
	virtual DamageableStaticPhysClass *	As_DamageableStaticPhysClass (void)			{ return NULL; }
	virtual DoorPhysClass *			As_DoorPhysClass(void)									{ return NULL; }
	virtual DecorationPhysClass *	As_DecorationPhysClass(void)							{ return NULL; }
	virtual TimedDecorationPhysClass * As_TimedDecorationPhysClass(void)				{ return NULL; }
	virtual DynamicAnimPhysClass *As_DynamicAnimPhysClass(void)							{ return NULL; }
	virtual LightPhysClass *		As_LightPhysClass(void)									{ return NULL; }
	virtual RenderObjPhysClass *	As_RenderObjPhysClass(void)							{ return NULL; }
	virtual ProjectileClass *		As_ProjectileClass(void)								{ return NULL; }
	virtual AccessiblePhysClass *	As_AccessiblePhysClass(void)							{ return NULL; }

	/*
	** Persistant object save/load system
	*/
	virtual bool						Save (ChunkSaveClass &csave) override;
	virtual bool						Load (ChunkLoadClass &cload) override;

	/*
	** Debug rendering of vectors, points, boxes, etc etc.  These functions actually add debug
	** widgets to the physics scene.  Use the macros defined at the top of this file in order to
	** have these debug calls removed from the release build.
	*/
#ifdef WWDEBUG
	void									Add_Debug_Point(const Vector3 & p,const Vector3 & color);
	void									Add_Debug_Vector(const Vector3 & p,const Vector3 & v,const Vector3 & color);
	void									Add_Debug_AABox(const AABoxClass & box,const Vector3 & color,float opacity = 0.25f);
	void									Add_Debug_OBBox(const OBBoxClass & box,const Vector3 & color,float opacity = 0.25f);
	void									Add_Debug_Axes(const Matrix3D & transform,const Vector3 & color);
#endif

	/*
	** Simulation and Rendering toggles by type.  Derived classes implement these functions to test
	** a static flag which is used to indicate whether rendering or simluation of all instances of
	** that class are disabled or not.
	*/
	virtual bool						Is_Simulation_Disabled(void)				{ return false; }
	virtual bool						Is_Rendering_Disabled(void)				{ return false; }

	/*
	** Umbra Testing
	*/
#if (UMBRASUPPORT)
	Umbra::Object *					Peek_Umbra_Object(void)						{ return UmbraObject; }
#endif

	unsigned Get_Last_Visible_Frame() const { return LastVisibleFrame; }
	void Set_Last_Visible_Frame(unsigned frame) { LastVisibleFrame=frame; }

protected:

	bool									Get_Flag(unsigned int flag) const 					{ return ((Flags & flag) == flag); }
	void									Set_Flag(unsigned int flag,bool onoff)			 	{ (onoff ? Flags |= flag : Flags &= ~flag); }
	
	void									Push_Effects(RenderInfoClass & rinfo);
	void									Pop_Effects(RenderInfoClass & rinfo);

	virtual void						Update_Sun_Status(void);

	enum {
		COLLISION_MASK =				0x0000000F,		// bits for the collision group
		IMMOVABLE =						0x00000100,		// this object is immovable.
		DISABLED =						0x00000200,		// Some objects can be disabled (e.g. lights)
		DEBUGDISPLAY =					0x00000400,		// Render debugging aids (forces, impacts, etc)
		USERCONTROL =					0x00000800,		// Ignore physics, move according to controller directly
		CASTSHADOW =					0x00001000,		// Does this object cast a shadow?
		FORCE_PROJECTION_SHADOW =	0x00002000,		// When the shadow mode is BLOBS_PLUS, this object still uses a "proper" shadow
		DONT_SAVE =						0x00004000,		// Scene should never save this object (used for transient things like glass fragments)
		ASLEEP =							0x00008000,		// This object is not moving so its simulation was skipped
		IS_WS_MESH =					0x00010000,		// Enable the static-world-space-mesh rendering optimizations.
		IS_PRE_LIT =					0x00020000,		// Is this a light-mapped object that doesn't need static lights applied.
		IS_IN_THE_SUN =				0x00040000,		// Is this object illuminated by the sun?
		IS_STATE_DIRTY =				0x00080000,		// This object's state has changed. 
		STATIC_LIGHTING_DIRTY =		0x00100000,		// This object's static lighting cache is dirty
		FRICTION_DISABLED =			0x00200000,		// Friction is disabled for this object (vehicles disable body-friction when their wheels are in contact)
		SIMULATION_DISABLED =		0x00400000,		// Turn on/off simulation for this object

		IGNORE_SHIFT =					28,				// shift count for the 'ignore-me' counter
		IGNORE_MASK =					0xF0000000,		// mask for the 'ignore-me' counter
		DEFAULT_FLAGS =				0,
	};

	/*
	** flags for things like whether this object is currently being considered immovable
	*/
	unsigned int					Flags;
	
	/*
	** Render model
	*/
	RenderObjClass *				Model;

	/*
	** Optional instance name
	*/
	StringClass						Name;

	/*
	** Optional instance identifier (unique if non-zero)
	*/
	uint32							InstanceID;

	/*
	** Vis Object ID.  Every phys object can store a vis object id.  Static objects will
	** have constant ID's assigned by the vis generation process, dynamic objects will
	** update their id based on their current location.
	*/
	uint32							VisObjectID;

	/*
	** Observer object 
	*/
	PhysObserverClass *			Observer;

	/*
	** Definition object, contains constants which are shared between instances
	*/
	const PhysDefClass *			Definition;

	/*
	** List of projected textures that are being applied to this object
	*/
	//TexProjListClass				ProjectionsOnMe;
	RefMaterialEffectListClass	MaterialEffectsOnMe;

	/*
	** Static lighting cache.
	*/
	LightEnvironmentClass *		StaticLightingCache;

	/*
	** Frame time at which the sun status was last updated. The sun status is cached and only updated few times per second.
	*/
	unsigned	SunStatusLastUpdated;

	/*
	**	The pscene uses this to figure out if the mesh is visible, to do some physics optimizations.
	*/
	unsigned LastVisibleFrame;

	/*
	** UMBRA Testing
	*/
#if (UMBRASUPPORT)
	Umbra::Object *				UmbraObject;
#endif

private:

	// Not Implemented:
	PhysClass(const PhysClass & src);
	PhysClass & operator = (const PhysClass & src);
};


inline void PhysClass::Inc_Ignore_Counter(void)		
{ 
	int count = (Flags & IGNORE_MASK) >> IGNORE_SHIFT;
	count++;
	WWASSERT(count < 12);
	Flags &= ~IGNORE_MASK;
	Flags |= (count << IGNORE_SHIFT);
}

inline void PhysClass::Dec_Ignore_Counter(void)
{
	int count = (Flags & IGNORE_MASK) >> IGNORE_SHIFT;
	WWASSERT(count > 0);
	count--;
	Flags &= ~IGNORE_MASK;
	Flags |= (count << IGNORE_SHIFT);
}


inline CollisionReactionType PhysClass::Collision_Occurred(CollisionEventClass & event)
{ 
	if (Observer) {
		return Observer->Collision_Occurred(event); 
	} else {
		return COLLISION_REACTION_DEFAULT;
	}
}

inline void PhysClass::Update_Cull_Box(void)
{
	if (Model) {
		Set_Cull_Box(Model->Get_Bounding_Box());
	}
}

inline void PhysClass::Add_Effect_To_Me(MaterialEffectClass * effect)
{
	WWASSERT(effect != NULL);
	MaterialEffectsOnMe.Add(effect);
}

inline void PhysClass::Remove_Effect_From_Me(MaterialEffectClass * effect)
{
	WWASSERT(effect != NULL);
	MaterialEffectsOnMe.Remove(effect);
}

inline bool PhysClass::Is_Pre_Lit(void)
{
	if (Model) {
		return (Get_Flag(IS_PRE_LIT) | (Model->Has_User_Lighting() != 0)); 
	} else {
		return Get_Flag(IS_PRE_LIT);
	}
}


/**
** PhysDefClass - Initialization structure for a PhysClass
*/
class PhysDefClass : public DefinitionClass
{
public:
	
	PhysDefClass(void);
	
	// From PersistClass
	virtual bool					Save(ChunkSaveClass &csave) override;
	virtual bool					Load(ChunkLoadClass &cload) override;
	
	// PhysDef type filtering mechanism
	virtual const char *			Get_Type_Name(void)				{ return "PhysDef"; }
	virtual bool					Is_Type(const char *);

	// Validation methods
	virtual bool					Is_Valid_Config (StringClass &message) override;

	// accessors
	const StringClass &			Get_Model_Name()					{ return ModelName; }
	bool								Get_Is_Pre_Lit()					{ return IsPreLit; }
	
	//	Editable interface requirements
	DECLARE_EDITABLE(PhysDefClass,DefinitionClass);

protected:
	
	StringClass						ModelName;
	bool								IsPreLit;
	
	friend class PhysClass;
};



#endif