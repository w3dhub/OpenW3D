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
 *                     $Archive:: /Commando/Code/wwphys/animcollisionmanager.h                $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 9/09/01 2:44p                                               $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef ANIMCOLLISIONMANAGER_H
#define ANIMCOLLISIONMANAGER_H

#include "always.h"
#include "ridermanager.h"
#include "matrix3d.h"
#include "vector.h"
#include "rendobj.h"
#include "wwstring.h"

class AnimCollisionManagerDefClass;
class HAnimClass;
class ChunkSaveClass;
class ChunkLoadClass;
class BitStreamClass;
class PushRecordClass;

/**
** AnimCollisionManagerClass
** The job of this class is to handle collisions which are caused by animations.  Some examples
** of this are: elevators, doors, and "cinematic events".  StaticAnimPhysClass and DynamicAnimPhysClass
** both use this code to handle collisions with the dynamic objects in the scene.  
*/
class AnimCollisionManagerClass
{
public:

	AnimCollisionManagerClass(PhysClass & parent);
	~AnimCollisionManagerClass(void);

	/*
	** Initialization
	*/
	void											Init(const AnimCollisionManagerDefClass & def);
	void											Update_Cached_Model_Parameters(void);

	/*
	** Simulation, moves the animation forward, checks for animated collisions, updates riders
	** Returns true if the object's animation pose changed.
	*/
	bool											Timestep(float dt);

	/*
	** Animation Controls
	*/
	enum AnimModeType
	{
		ANIMATE_LOOP = 0,
		ANIMATE_TARGET,
		ANIMATE_MANUAL,
	};
	
	void											Set_Animation_Mode(AnimModeType mode);
	AnimModeType								Get_Animation_Mode(void);
	
	void											Set_Animation(const char * anim_name);
	HAnimClass *								Peek_Animation(void);

	void											Set_Target_Frame(float frame);
	void											Set_Target_Frame_End(void);
	float											Get_Target_Frame(void);
	bool											Is_At_Target(void);

	void											Set_Current_Frame(float frame);
	float											Get_Current_Frame(void);

	void											Set_Loop_Start(float frame0);
	void											Set_Loop_End(float frame1);
	float											Get_Loop_Start(void);
	float											Get_Loop_End(void);

	/*
	** Collision Control
	*/
	enum CollisionModeType
	{
		COLLIDE_NONE = 0,
		COLLIDE_STOP,
		COLLIDE_PUSH,
		COLLIDE_KILL,
	};

	void											Set_Collision_Mode(CollisionModeType mode);
	CollisionModeType							Get_Collision_Mode(void);

	/*
	** Rider Support
	*/
	bool											Link_Rider(PhysClass * rider)			{ RiderManager.Link_Rider(rider); return true; }
	bool											Unlink_Rider(PhysClass * rider)		{ RiderManager.Unlink_Rider(rider); return true; }

	/*
	** Save/Load Support
	*/
	bool											Save(ChunkSaveClass &csave);
	bool											Load(ChunkLoadClass &cload);

protected:


	/*
	** Multi-mesh collision detection support
	*/
	class CollideableObjClass
	{
	public:
		CollideableObjClass(void);
		CollideableObjClass(RenderObjClass * collisionmesh);
		CollideableObjClass(const CollideableObjClass & that);
		~CollideableObjClass(void);

		CollideableObjClass &				operator = (const CollideableObjClass & that);
		bool										operator == (const CollideableObjClass & ) const { return false; }
		bool										operator != (const CollideableObjClass & ) const { return true; }		

		void										Set_Collision_Object(RenderObjClass * mesh);
		RenderObjClass *						Peek_Collision_Object(void)						{ return CollisionMesh; }

		void										Cache_Start_Transform(void)						{ StartTransform = CollisionMesh->Get_Transform(); }
		void										Cache_End_Transform(void)							{ EndTransform = CollisionMesh->Get_Transform(); }
		Matrix3D &								Get_Start_Transform(void)							{ return StartTransform; }
		Matrix3D &								Get_End_Transform(void)								{ return EndTransform; }	
		void										Install_Start_Transform(void)						{ CollisionMesh->Set_Transform(StartTransform); }
		void										Install_End_Transform(void)						{ CollisionMesh->Set_Transform(EndTransform); }
		
		void										Intersect_Scene(int colgroup,NonRefPhysListClass * list);
		int										Clear_Collision_Bits(void);
		void										Restore_Collision_Bits(int oldbits);

	protected:
		RenderObjClass *						CollisionMesh;
		Matrix3D									StartTransform;
		Matrix3D									EndTransform;
	};

	
	/*
	** Internal functions
	*/	
	bool											Is_Collision_Model(RenderObjClass * model);
	int											Recursive_Count_Collision_Models(RenderObjClass * model);
	void											Recursive_Collect_Collision_Models(RenderObjClass * model);
	void											Revert_Animation_State(void);
	bool											Check_Collision(CollideableObjClass & collisionobj);
	bool											Is_Intersecting(void);
	bool											Push_Collided_Object(PhysClass * obj,const Matrix3D & delta_transform);
	void											Internal_Set_Animation(const char * anim_name);

	/*
	** Parent reference 
	*/
	PhysClass &									Parent;

	/*
	** Animation support
	*/
	AnimModeType								AnimationMode;		// current animation mode and parameters.
	float											TargetFrame;		
	float											LoopStart;
	float											LoopEnd;
	
	HAnimClass *								CurAnimation;		// current animation state.
	float											CurFrame;
	HAnimClass *								PrevAnimation;		// temporary variable used in timestep
	float											PrevFrame;			// temporary variable used in timestep

	/*
	** Attached riders support
	*/
	RiderManagerClass							RiderManager;

	/*
	** Animated collision support 
	*/
	CollisionModeType							CollisionMode;
	DynamicVectorClass<CollideableObjClass>	CollisionMeshes;

	/*
	** Revert list, In the case that we have to revert our animation state, 
	** we have to undo all changes to external objects.
	*/
	PushRecordClass *							PushList;

};


/**
** AnimCollisionManagerDefClass
** This class is meant to be a component of a definition class for a physics object
** which has an AnimCollisionManager.  The member variables of this class are public since
** various physics def classes make them directly editable.
*/
class AnimCollisionManagerDefClass
{
public:

	AnimCollisionManagerDefClass(void);
	~AnimCollisionManagerDefClass(void);

	void											Validate_Parameters(void);

	bool											Save(ChunkSaveClass &csave);
	bool											Load(ChunkLoadClass &cload);

public:

	int											CollisionMode;
	int											AnimationMode;
	StringClass									AnimationName;

};


/*
** Use this macro to make all of the member variables in an AnimCollisionManagerDefClass editable.
** The first parameter to the macro is the type-name of your class (e.g. StaticAnimPhysDefClass) and
** the second parameter is the name of the member variable which is the animation manager (e.g. AnimManager).
*/
#ifdef PARAM_EDITING_ON																														

#define	ANIMCOLLISIONMANAGERDEF_EDITABLE_PARAMS( class_name, member_name )												\
  	PARAM_SEPARATOR( class_name, "Animation Settings");																						\
	EnumParameterClass *collision_param = new EnumParameterClass(& member_name .CollisionMode);					\
	collision_param->Set_Name ("CollisionMode");																						\
	collision_param->Add_Value("NONE",AnimCollisionManagerClass::COLLIDE_NONE);											\
	collision_param->Add_Value("STOP",AnimCollisionManagerClass::COLLIDE_STOP);											\
	collision_param->Add_Value("PUSH",AnimCollisionManagerClass::COLLIDE_PUSH);											\
	collision_param->Add_Value("KILL",AnimCollisionManagerClass::COLLIDE_KILL);											\
	GENERIC_EDITABLE_PARAM( class_name , collision_param);																		\
																																					\
	EnumParameterClass *anim_param = new EnumParameterClass(& member_name .AnimationMode);							\
	anim_param->Set_Name ("AnimationMode");																							\
	anim_param->Add_Value("LOOP",AnimCollisionManagerClass::ANIMATE_LOOP);													\
	anim_param->Add_Value("TARGET",AnimCollisionManagerClass::ANIMATE_TARGET);												\
	anim_param->Add_Value("MANUAL",AnimCollisionManagerClass::ANIMATE_MANUAL);												\
	GENERIC_EDITABLE_PARAM( class_name , anim_param);																				\
																																					\
	NAMED_EDITABLE_PARAM( class_name, ParameterClass::TYPE_STRING, member_name .AnimationName,"AnimationName"); \

#else

#define	ANIMCOLLISIONMANAGERDEF_EDITABLE_PARAMS( class_name, member_name )															

#endif																																		




#endif //ANIMCOLLISIONMANAGER_H

