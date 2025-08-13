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
 *                     $Archive:: /Commando/Code/wwphys/animcollisionmanager.cpp              $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/14/01 1:34p                                              $*
 *                                                                                             *
 *                    $Revision:: 33                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   AnimCollisionManagerClass::CollideableObjClass::CollideableObjClass -- Constructor        *
 *   AnimCollisionManagerClass::CollideableObjClass::~CollideableObjClass -- Destructor        *
 *   AnimCollisionManagerClass::CollideableObjClass::operator = -- assignment operator         *
 *   AnimCollisionManagerClass::CollideableObjClass::Set_Collision_Object -- set collision mes *
 *   AnimCollisionManagerClass::CollideableObjClass::Intersect_Scene -- intersection test      *
 *   AnimCollisionManagerClass::CollideableObjClass::Clear_Collision_Bits -- disables collisio *
 *   AnimCollisionManagerClass::CollideableObjClass::Restore_Collision_Bits -- set collision b *
 *   AnimCollisionManagerClass::AnimCollisionManagerClass -- Constructor                       *
 *   AnimCollisionManagerClass::~AnimCollisionManagerClass -- Destructor                       *
 *   AnimCollisionManagerClass::Set_Animation_Mode -- set the current anim mode                *
 *   AnimCollisionManagerClass::Get_Animation_Mode -- Get the current anim mode                *
 *   AnimCollisionManagerClass::Set_Animation -- Set the current animation                     *
 *   AnimCollisionManagerClass::Peek_Animation -- Get the current animation                    *
 *   AnimCollisionManagerClass::Set_Target_Frame -- Set the target animation frame             *
 *   AnimCollisionManagerClass::Get_Target_Frame -- Get the target animation frame             *
 *   AnimCollisionManagerClass::Set_Target_Frame_End -- Set the target frame to the end of the *
 *   AnimCollisionManagerClass::Is_At_Target -- returns true if in TARGET mode and done animat *
 *   AnimCollisionManagerClass::Set_Loop_Start -- Sets frame0 for an anim loop                 *
 *   AnimCollisionManagerClass::Set_Loop_End -- Sets frame1 for an anim loop                   *
 *   AnimCollisionManagerClass::Get_Loop_Start -- returns frame0 for an anim loop              *
 *   AnimCollisionManagerClass::Get_Loop_End -- returns frame1 for an anim loop                *
 *   AnimCollisionManagerClass::Set_Current_Frame -- Set the current frame                     *
 *   AnimCollisionManagerClass::Get_Current_Frame -- Get the current frame                     *
 *   AnimCollisionManagerClass::Set_Collision_Mode -- Set the current collision mode           *
 *   AnimCollisionManagerClass::Get_Collision_Mode -- Get the current collision mode           *
 *   AnimCollisionManagerClass::Init -- Init this collision manager                            *
 *   AnimCollisionManagerClass::Update_Cached_Model_Parameters -- find all of the collideable  *
 *   AnimCollisionManagerClass::Is_Collision_Model -- classify a sub-object as collideable or  *
 *   AnimCollisionManagerClass::Recursive_Count_Collision_Models -- count the collideable mesh *
 *   AnimCollisionManagerClass::Recursive_Collect_Collision_Models -- collect collideable mesh *
 *   AnimCollisionManagerClass::Timestep -- Update the state of this object                    *
 *   AnimCollisionManagerClass::Check_Collision -- Check the given collision object            *
 *   AnimCollisionManagerClass::Is_Intersecting -- intersection test                           *
 *   AnimCollisionManagerClass::Push_Collided_Object -- push an object we collided with        *
 *   AnimCollisionManagerClass::Save -- save!                                                  *
 *   AnimCollisionManagerClass::Load -- Load!                                                  *
 *   AnimCollisionManagerClass::Revert_Animation_State -- revert to previous anim frame        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "animcollisionmanager.h"
#include "colmathaabox.h"
#include "pscene.h"
#include "physinttest.h"
#include "movephys.h"
#include "bitstream.h"
#include "assetmgr.h"
#include "hanim.h"
#include "boxrobj.h"
#include "chunkio.h"
#include "wwdebug.h"
#include "wwprofile.h"


/*
** Set this if you want verbose logging.  It will be un-set if debugging is not enabled...
*/
#define VERBOSE_LOGGING 0

#if VERBOSE_LOGGING
#define VERBOSE_LOG(x) /*if (strstr(Parent.Peek_Model()->Get_Name(),"HND_ELEV03"))*/ WWDEBUG_SAY(x)
#else
#define VERBOSE_LOG(x)
#endif



/**
** PushRecordClass
** This object is used to record each object's initial state before an animated collision
** moves them.  At the end of an update, if the animated object has to revert to its initial state,
** all of the objects in the "PushList" will be reverted to there original states as well.
*/
class PushRecordClass : public AutoPoolClass<PushRecordClass,32>
{
public:

	PushRecordClass(PhysClass * obj) : Object(NULL), Next(NULL)	{ REF_PTR_SET(Object,obj); WWASSERT(obj); Transform = obj->Get_Transform(); }
	~PushRecordClass(void)													{ REF_PTR_RELEASE(Object); };

	void Revert(void)															{ Object->Set_Transform(Transform); }	

	PushRecordClass * Get_Next(void) const								{ return Next; }
	void Set_Next(PushRecordClass * next)								{ Next = next; }


	static void Add_To_List(PushRecordClass ** head, PhysClass * obj)
	{
		PushRecordClass * new_rec = new PushRecordClass(obj);
		new_rec->Set_Next(*head);
		*head = new_rec;
	}
	
	static void Revert_List(PushRecordClass * head)
	{
		while (head) {
			head->Revert();
			head = head->Get_Next();
		}
	}

	static void Delete_List(PushRecordClass * head)	
	{ 
		while (head) {
			PushRecordClass * next = head->Get_Next();
			delete head;
			head = next;
		}
	}

protected:

	PhysClass *				Object;
	Matrix3D					Transform;
	PushRecordClass *		Next;

};

DEFINE_AUTO_POOL(PushRecordClass,32);


/*
** Chunk ID's used by AnimCollisionManagerClass
*/
enum
{
	ANIMCOLLISIONMANAGER_CHUNK_VARIABLES					= 0525000421,

	ANIMCOLLISIONMANAGER_VARIABLE_COLLISIONMODE			= 0x00,
	ANIMCOLLISIONMANAGER_VARIABLE_ANIMATIONMODE,
	ANIMCOLLISIONMANAGER_VARIABLE_TARGETFRAME,
	ANIMCOLLISIONMANAGER_VARIABLE_CURFRAME,
	ANIMCOLLISIONMANAGER_VARIABLE_ANIMATIONNAME,
	ANIMCOLLISIONMANAGER_VARIABLE_LOOPSTART,
	ANIMCOLLISIONMANAGER_VARIABLE_LOOPEND,
	ANIMCOLLISIONMANAGER_VARIABLE_PREVFRAME,
	ANIMCOLLISIONMANAGER_VARIABLE_PREVANIMATIONNAME,
};										


/********************************************************************************************
**
** AnimCollisionManagerClass::CollideableObjClass Implementation
**
********************************************************************************************/


/***********************************************************************************************
 * AnimCollisionManagerClass::CollideableObjClass::CollideableObjClass -- Constructor          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
AnimCollisionManagerClass::CollideableObjClass::CollideableObjClass(void) :
	CollisionMesh(NULL),
	StartTransform(1),
	EndTransform(1)
{
}


/***********************************************************************************************
 * AnimCollisionManagerClass::CollideableObjClass::CollideableObjClass -- Constructor          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
AnimCollisionManagerClass::CollideableObjClass::CollideableObjClass
(
	RenderObjClass * collisionmesh
) :
	CollisionMesh(NULL),
	StartTransform(1),
	EndTransform(1)
{
	Set_Collision_Object(collisionmesh);
}

/***********************************************************************************************
 * AnimCollisionManagerClass::CollideableObjClass::CollideableObjClass -- Constructor          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
AnimCollisionManagerClass::CollideableObjClass::CollideableObjClass
(
	const CollideableObjClass & that
) :
	CollisionMesh(NULL),
	StartTransform(1),
	EndTransform(1)
{
	*this = that;
}


/***********************************************************************************************
 * AnimCollisionManagerClass::CollideableObjClass::~CollideableObjClass -- Destructor          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
AnimCollisionManagerClass::CollideableObjClass::~CollideableObjClass(void)
{
	REF_PTR_RELEASE(CollisionMesh);
}


/***********************************************************************************************
 * AnimCollisionManagerClass::CollideableObjClass::operator = -- assignment operator           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
AnimCollisionManagerClass::CollideableObjClass &	
AnimCollisionManagerClass::CollideableObjClass::operator = (const CollideableObjClass & that)
{
	if (this != &that) {
		REF_PTR_SET(CollisionMesh,that.CollisionMesh);
		StartTransform = that.StartTransform;
		EndTransform = that.EndTransform;
	}
	return *this;
}
	

/***********************************************************************************************
 * AnimCollisionManagerClass::CollideableObjClass::Set_Collision_Object -- set collision mesh  *
 *                                                                                             *
 * Sets the render object being tracked by this CollideableObjClass                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void 
AnimCollisionManagerClass::CollideableObjClass::Set_Collision_Object(RenderObjClass * mesh)
{	
	REF_PTR_SET(CollisionMesh,mesh);
	StartTransform = EndTransform = CollisionMesh->Get_Transform();
}


/***********************************************************************************************
 * AnimCollisionManagerClass::CollideableObjClass::Intersect_Scene -- intersection test        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void 
AnimCollisionManagerClass::CollideableObjClass::Intersect_Scene
(
	int colgroup,
	NonRefPhysListClass * intersect_list
)
{
	if (CollisionMesh == NULL) return;
	PhysicsSceneClass * scene = PhysicsSceneClass::Get_Instance();

#if VERBOSE_LOGGING
	Vector3 pos = CollisionMesh->Get_Transform().Get_Translation();
	WWDEBUG_SAY(("Checking obj: %s position: %10.2f, %10.2f, %10.2f\r\n",CollisionMesh->Get_Name(),pos.X,pos.Y,pos.Z));
#endif

	switch (CollisionMesh->Class_ID()) {
	case RenderObjClass::CLASSID_MESH:
		{
			MeshClass * mesh = (MeshClass *)CollisionMesh;
			PhysMeshIntersectionTestClass test(mesh,colgroup,COLLISION_TYPE_PHYSICAL,intersect_list);
			test.CheckStaticObjs = false;
			scene->Intersection_Test(test);
			VERBOSE_LOG(("Mesh intersection: %d\r\n",!intersect_list->Is_Empty()));
		}
		break;

	case RenderObjClass::CLASSID_OBBOX:
		{
			OBBoxRenderObjClass * boxrobj = (OBBoxRenderObjClass *)CollisionMesh;
			PhysOBBoxIntersectionTestClass test(boxrobj->Get_Box(),colgroup,COLLISION_TYPE_PHYSICAL,intersect_list);
			test.CheckStaticObjs = false;
			scene->Intersection_Test(test);
			VERBOSE_LOG(("OBBox intersection: %d\r\n",!intersect_list->Is_Empty()));
		}
		break;
	case RenderObjClass::CLASSID_AABOX:
		{
			AABoxRenderObjClass * boxrobj = (AABoxRenderObjClass *)CollisionMesh;
			PhysAABoxIntersectionTestClass test(boxrobj->Get_Box(),colgroup,COLLISION_TYPE_PHYSICAL,intersect_list);
			test.CheckStaticObjs = false;
			scene->Intersection_Test(test);
			VERBOSE_LOG(("AABox intersection: %d\r\n",!intersect_list->Is_Empty()));
		}
		break;
	};
}


/***********************************************************************************************
 * AnimCollisionManagerClass::CollideableObjClass::Clear_Collision_Bits -- disables collision  *
 *                                                                                             *
 * Disables all collision on the render object that this CollideableObjClass is managing       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
int AnimCollisionManagerClass::CollideableObjClass::Clear_Collision_Bits(void)
{
	int oldbits = CollisionMesh->Get_Collision_Type();
	CollisionMesh->Set_Collision_Type(0);
	return oldbits;
}


/***********************************************************************************************
 * AnimCollisionManagerClass::CollideableObjClass::Restore_Collision_Bits -- set collision bit *
 *                                                                                             *
 * restores the collision bits on the render object that this CollideableObjClass is managing  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void AnimCollisionManagerClass::CollideableObjClass::Restore_Collision_Bits(int oldbits)
{
	CollisionMesh->Set_Collision_Type(oldbits);
}

 
/********************************************************************************************
**
** AnimCollisionManagerClass Implementation
**
********************************************************************************************/


/***********************************************************************************************
 * AnimCollisionManagerClass::AnimCollisionManagerClass -- Constructor                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
AnimCollisionManagerClass::AnimCollisionManagerClass(PhysClass & parent) :
	Parent(parent),
	AnimationMode(ANIMATE_LOOP),
	TargetFrame(0.0f),
	LoopStart(0.0f),
	LoopEnd(0.0f),
	CurAnimation(NULL),
	CurFrame(0.0f),
	PrevAnimation(NULL),
	PrevFrame(0.0f),
	CollisionMode(COLLIDE_NONE),
	PushList(NULL)
{
}


/***********************************************************************************************
 * AnimCollisionManagerClass::~AnimCollisionManagerClass -- Destructor                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
AnimCollisionManagerClass::~AnimCollisionManagerClass(void)
{
	WWASSERT(PushList == NULL);
	REF_PTR_RELEASE(CurAnimation);
	REF_PTR_RELEASE(PrevAnimation);
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Set_Animation_Mode -- set the current anim mode                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void AnimCollisionManagerClass::Set_Animation_Mode(AnimModeType mode)
{
	AnimationMode = mode;
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Get_Animation_Mode -- Get the current anim mode                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
AnimCollisionManagerClass::AnimModeType 
AnimCollisionManagerClass::Get_Animation_Mode(void)
{
	return AnimationMode;
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Set_Animation -- Set the current animation                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void AnimCollisionManagerClass::Set_Animation(const char * anim_name)
{
	Internal_Set_Animation(anim_name);
	
	/*
	** Automatically loop this animation by default
	*/
	LoopStart = 0;
	if (CurAnimation != NULL) {
		LoopEnd = TargetFrame = CurAnimation->Get_Num_Frames() - 1;
	} else {
		LoopEnd = TargetFrame = 0;
	}

}


/***********************************************************************************************
 * AnimCollisionManagerClass::Internal_Set_Animation -- Set the current animation pointer      *
 *                                                                                             *
 * This function doesn't change the animation mode, target frame, or current frame             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/13/2000   gth : Created.                                                                *
 *=============================================================================================*/
void AnimCollisionManagerClass::Internal_Set_Animation(const char * anim_name)
{
	HAnimClass * anim = WW3DAssetManager::Get_Instance()->Get_HAnim(anim_name);
	if ( anim == NULL && anim_name != NULL ) {
		WWDEBUG_SAY(( "FAILED TO FIND ANIM IN AnimCollisionManagerClass::Internal_Set_Animation(\"%s\")\n", anim_name ));
	}
	REF_PTR_SET(CurAnimation,anim);
	REF_PTR_RELEASE(anim);

	if (Parent.Peek_Model() != NULL) {
		Parent.Peek_Model()->Set_Animation(CurAnimation,CurFrame);
	}
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Peek_Animation -- Get the current animation                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
HAnimClass * AnimCollisionManagerClass::Peek_Animation(void)
{
	return CurAnimation;
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Set_Target_Frame -- Set the target animation frame               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void AnimCollisionManagerClass::Set_Target_Frame(float frame)
{
	TargetFrame = frame;
	Parent.Enable_Is_State_Dirty(true);
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Set_Target_Frame_End -- Set the target frame to the end of the c *
 *                                                                                             *
 *    Sets the target frame to the end of the current anim                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/13/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void AnimCollisionManagerClass::Set_Target_Frame_End(void)
{
	if (CurAnimation != NULL) {
		Set_Target_Frame(CurAnimation->Get_Num_Frames() - 1);
	}
}

/***********************************************************************************************
 * AnimCollisionManagerClass::Get_Target_Frame -- Get the target animation frame               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
float AnimCollisionManagerClass::Get_Target_Frame(void)
{
	return TargetFrame;
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Is_At_Target -- returns true if in TARGET mode and done animatin *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/14/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool AnimCollisionManagerClass::Is_At_Target(void)
{
	return ((TargetFrame == CurFrame) && (AnimationMode == ANIMATE_TARGET));
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Set_Loop_Start -- Sets frame0 for an anim loop                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * This setting is only used if you are in AnimationMode==ANIMATE_LOOP                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/29/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void AnimCollisionManagerClass::Set_Loop_Start(float frame0)
{
	LoopStart = frame0;
	Parent.Enable_Is_State_Dirty(true);
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Set_Loop_End -- Sets frame1 for an anim loop                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * This setting is only used if you are in AnimationMode==ANIMATE_LOOP                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/29/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void AnimCollisionManagerClass::Set_Loop_End(float frame1)
{
	LoopEnd = frame1;
	Parent.Enable_Is_State_Dirty(true);
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Get_Loop_Start -- returns frame0 for an anim loop                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/29/2000  gth : Created.                                                                 *
 *=============================================================================================*/
float AnimCollisionManagerClass::Get_Loop_Start(void)
{
	return LoopStart;
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Get_Loop_End -- returns frame1 for an anim loop                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/29/2000  gth : Created.                                                                 *
 *=============================================================================================*/
float AnimCollisionManagerClass::Get_Loop_End(void)
{
	return LoopEnd;
}

/***********************************************************************************************
 * AnimCollisionManagerClass::Set_Current_Frame -- Set the current frame                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void AnimCollisionManagerClass::Set_Current_Frame(float frame)
{
	CurFrame = frame;
	Parent.Enable_Is_State_Dirty(true);
	
#if VERBOSE_LOGGING
	WWASSERT(!Is_Intersecting());
#endif
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Get_Current_Frame -- Get the current frame                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
float AnimCollisionManagerClass::Get_Current_Frame(void)
{
	return CurFrame;
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Set_Collision_Mode -- Set the current collision mode             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void AnimCollisionManagerClass::Set_Collision_Mode(CollisionModeType mode)
{
	CollisionMode = mode;
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Get_Collision_Mode -- Get the current collision mode             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
AnimCollisionManagerClass::CollisionModeType AnimCollisionManagerClass::Get_Collision_Mode(void)
{
	return CollisionMode;
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Init -- Init this collision manager                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void AnimCollisionManagerClass::Init(const AnimCollisionManagerDefClass & def)
{
	/*
	** Set the collision and animation mode
	*/
	CollisionMode = (CollisionModeType)def.CollisionMode;
	AnimationMode = (AnimModeType)def.AnimationMode;

	/*
	** Plug in the animation
	*/
	if (Parent.Peek_Model()) {
		Set_Current_Frame(0.0f);		
		
		/*
		** Get the name of the animation from the definition
		*/
		StringClass anim_name = def.AnimationName;

		/*
		** If the user didn't specify an  animation name, then
		** build an animation name from the model name.
		*/
		if (anim_name.Is_Empty ()) {
			StringClass model_name(Parent.Peek_Model()->Get_Name(),true);
			anim_name = model_name;
			anim_name+=".";
			anim_name+=model_name;
		}
				
		Set_Animation(anim_name);
	}
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Update_Cached_Model_Parameters -- find all of the collideable me *
 *                                                                                             *
 * This function mainly finds all of the animating collideable meshes.                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void AnimCollisionManagerClass::Update_Cached_Model_Parameters(void)
{
	/*
	** Build the array of collideable animated meshes
	*/
	int count = Recursive_Count_Collision_Models(Parent.Peek_Model());
	CollisionMeshes.Delete_All();
	CollisionMeshes.Resize(count);
	
	Recursive_Collect_Collision_Models(Parent.Peek_Model());
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Is_Collision_Model -- classify a sub-object as collideable or no *
 *                                                                                             *
 * A sub object needs a collision object if it is attached to a bone other than the root       *
 * and it is collideable.                                                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
bool AnimCollisionManagerClass::Is_Collision_Model(RenderObjClass * subobj)
{
	/*
	** If this sub-object is collideable and it is not attached to the RootTransform (0)
	** then it is considered an animated collideable mesh.
	*/
	return (	(subobj->Get_Collision_Type() & COLLISION_TYPE_PHYSICAL) &&
				(subobj->Get_Container() != NULL) &&
				(subobj->Get_Container()->Get_Sub_Object_Bone_Index(subobj)	!= 0) );
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Recursive_Count_Collision_Models -- count the collideable meshes *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
int AnimCollisionManagerClass::Recursive_Count_Collision_Models(RenderObjClass * model)
{
	int count = 0;
	if (model == NULL) {
		return 0;
	}

	for (int i=0; i<model->Get_Num_Sub_Objects(); i++) {
		RenderObjClass * subobj = model->Get_Sub_Object(i);
		if (subobj->Get_Num_Sub_Objects() > 0) {
			count += Recursive_Count_Collision_Models(subobj);				// recurse
		} else {
			if (Is_Collision_Model(model)) {
				count++;
			}
		}
		subobj->Release_Ref();
	}

	return count;
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Recursive_Collect_Collision_Models -- collect collideable meshes *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void AnimCollisionManagerClass::Recursive_Collect_Collision_Models(RenderObjClass * model)
{
	if (model == NULL) {
		return;
	}

	for (int i=0; i<model->Get_Num_Sub_Objects(); i++) {
		RenderObjClass * subobj = model->Get_Sub_Object(i);
		if (subobj->Get_Num_Sub_Objects() > 0) {
			Recursive_Collect_Collision_Models(subobj);				// recurse
		} else {

			/*
			** If this sub-object is collideable and it is not attached to the RootTransform (0)
			** then it is a candidate for being a collideable object
			*/
			if (Is_Collision_Model(subobj)) {

				/*
				** Add this collideable mesh to our array of collision meshes
				*/
				CollideableObjClass colmesh(subobj);
				CollisionMeshes.Add(colmesh);
			}
		}
		subobj->Release_Ref();
	}
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Timestep -- Update the state of this object                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
bool AnimCollisionManagerClass::Timestep(float dt)
{
	WWPROFILE("AnimColMgr::Timestep");
	VERBOSE_LOG(("\r\nTimestep for %s - begin\r\n",Parent.Peek_Model()->Get_Name()));
	bool object_animated = false;

	/*
	** In the case where we're not going to do anything bail out immediately!
	*/
	if ((CurFrame == PrevFrame) && (CurAnimation == PrevAnimation)) {
		if (AnimationMode == ANIMATE_MANUAL) {
			return false;
		}
		if ((AnimationMode == ANIMATE_TARGET) && (CurFrame == TargetFrame)) {
			return false;
		}
	}

	/*
	** Store the collision transform prior to updating the animation
	*/
	if (CollisionMode != COLLIDE_NONE) {
		for (int i=0; i<CollisionMeshes.Count(); i++) {
			CollisionMeshes[i].Cache_Start_Transform();
		}
	}
	
#if VERBOSE_LOGGING
	bool started_intersecting = false;
	if (CollisionMode != COLLIDE_NONE) {
		started_intersecting = Is_Intersecting();
		WWDEBUG_SAY(("Checking started_intersecting: %d\r\n",started_intersecting));
	}
#endif

	/*
	** move animation forward
	*/
	if (CurAnimation != NULL) {
		switch (AnimationMode) {
		
			case ANIMATE_LOOP:
				CurFrame += CurAnimation->Get_Frame_Rate() * dt;
				
				if (CurFrame >= LoopEnd) {
					CurFrame -= (LoopEnd - LoopStart);
				}
				break;

			case ANIMATE_TARGET:
				if (CurFrame < TargetFrame) {
					CurFrame += CurAnimation->Get_Frame_Rate() * dt;
					// if we overshoot targetframe, snap to targetframe
					if (CurFrame >= TargetFrame) {								
						CurFrame = TargetFrame;
					}
				} else if (CurFrame > TargetFrame) {
					CurFrame -= CurAnimation->Get_Frame_Rate() * dt;
					// if we overshoot targetframe, snap to targetframe
					if (CurFrame <= TargetFrame) {
						CurFrame = TargetFrame;
					}
				}
				break;

			case ANIMATE_MANUAL:	
				break;

			default:
				WWASSERT_PRINT(0,("Invalid Animation Mode!\n"));

		}

		VERBOSE_LOG(("Cur-Frame: %f Target-Frame: %f\r\n",CurFrame,TargetFrame));
		Parent.Peek_Model()->Set_Animation(CurAnimation,CurFrame);
	}

	/*
	** Check for collisions.
	** - cache a copy of the ending transform for each collideable object
	** - revert all collideable objects back to their start transform
	** - for each object:
	**   - install its ending transform and check for collision.
	**   - if collided, compute the movement vector and push objects.
	**   - if unable to push objects out of the way, set the revert flag and exit
	*/
	if ((CurAnimation != PrevAnimation) || (CurFrame != PrevFrame)) {

		/*
		** Add our riders to our push list
		*/
		object_animated = true;
		bool intersecting = false;
		
		if (CollisionMode != COLLIDE_NONE) {
			int ci;

			/*
			** Save the transform as the "ending transform"
			** and roll back to the starting transform
			*/
			for (ci=0; ci<CollisionMeshes.Count(); ci++) {
				CollisionMeshes[ci].Cache_End_Transform();
				CollisionMeshes[ci].Install_Start_Transform();
			}

			/*
			** Move this mesh forward to the End Transform
			** and see if it collides with anything
			*/
			for (ci=0; (ci<CollisionMeshes.Count()) && (!intersecting); ci++) {
				CollisionMeshes[ci].Install_End_Transform();
				intersecting |= Check_Collision(CollisionMeshes[ci]);
			}
			
			/*
			** If we ended up intersecting, then we have to revert
			*/
			if (intersecting) {
				Revert_Animation_State();
				object_animated = false;
			} 

#if VERBOSE_LOGGING	
			WWDEBUG_SAY(("checking is_now_intersecting\r\n"));
			bool is_now_intersecting = Is_Intersecting();
			if ( (started_intersecting == false) && (is_now_intersecting == true)) {
				WWDEBUG_SAY(("Reverting did not fix intersections!\r\n"));
			}
#endif
		}
	}
	
	/*
	** Release the push records
	*/
	PushRecordClass::Delete_List(PushList);
	PushList = NULL;

	/*
	** Ratchet our animation state forward for the next frame
	*/
	REF_PTR_RELEASE(PrevAnimation);
	REF_PTR_SET(PrevAnimation,CurAnimation);
	PrevFrame = CurFrame;

	VERBOSE_LOG(("Timestep - done\r\n"));
	return object_animated;
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Revert_Animation_State -- revert to previous anim frame          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void AnimCollisionManagerClass::Revert_Animation_State(void)
{
	REF_PTR_SET(CurAnimation,PrevAnimation);
	CurFrame = PrevFrame;
	Parent.Peek_Model()->Set_Animation(CurAnimation,CurFrame);

	VERBOSE_LOG(("Reverted to frame: %f\r\n",CurFrame));

	/*
	** Force the transforms of the collideable meshes to update
	*/
	for (int ci=0; ci<CollisionMeshes.Count(); ci++) {
		CollisionMeshes[ci].Cache_End_Transform();
		CollisionMeshes[ci].Install_End_Transform();
	}

	/*
	** Put everyone that we pushed back into their original state
	*/
	PushRecordClass::Revert_List(PushList);
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Check_Collision -- Check the given collision object              *
 *                                                                                             *
 * This returns true if the object collided with something and could not move it out of the    *
 * way.  (You must revert or kill the object in this case)                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
bool AnimCollisionManagerClass::Check_Collision(CollideableObjClass & collisionobj)
{
	/*
	** Algorithm:
	** - Determine how each attached object should move due to our motion.
	** - Ask all of our attached objects to apply the same delta.
	** - Ask physics scene to do an intersection test between our mesh and all nearby dynamic objects
	** - If (CollisionMode == STOP) REVERT animation and return
	** - Else For each object intersecting the collision mesh:
	**   - If it is one of our attached objects, SQUISH and REVERT
	**   - Else
	**     - Determine how the collided object should move and try to move it
	**     - If still colliding, SQUISH and REVERT
	**     - Else OK
	** - If no object caused a revert, ACCEPT new animation state and return. 
	** - Else REVERT and return
	*/

	/*
	** Compute the change in the transform of the collision mesh.  Note that this
	** is currently only "supposed" to work with pure translational motion...
	*/
	Matrix3D delta_transform;
	Matrix3D pinv;

	collisionobj.Get_Start_Transform().Get_Orthogonal_Inverse(pinv);
	Matrix3D::Multiply(collisionobj.Get_End_Transform(),pinv,&delta_transform);

	/*
	** Make all attached objects move with us (people stick on elevators, etc)
	** Adding a push record for each rider in case it gets moved...
	*/
	int collision_bits = collisionobj.Clear_Collision_Bits();

	NonRefPhysListIterator rider_iterator(RiderManager.Get_Rider_List());
	VERBOSE_LOG(("   rider_list: "));
	while (!rider_iterator.Is_Done()) {
		VERBOSE_LOG(("%s, ",rider_iterator.Peek_Obj()->Peek_Model()->Get_Name()));
		PushRecordClass::Add_To_List(&PushList,rider_iterator.Peek_Obj());
		rider_iterator.Next();
	}
	VERBOSE_LOG(("\r\n"));

	RiderManager.Move_Riders(delta_transform,collisionobj.Peek_Collision_Object());
	collisionobj.Restore_Collision_Bits(collision_bits);

	/*
	** Perform intersection detection
	*/
	VERBOSE_LOG(("Check Collision: %s\r\n",collisionobj.Peek_Collision_Object()->Get_Name()));
	NonRefPhysListClass intersection_list;
	collisionobj.Intersect_Scene(Parent.Get_Collision_Group(),&intersection_list);

	/*
	** If nothing is intersecting us, return.
	*/
	if (intersection_list.Is_Empty()) {
		return false;
	}

	/*
	** CollisionMode == COLLIDE_STOP
	** If something is intersecting us and we just revert when that happens, revert and return
	*/
	if (CollisionMode == COLLIDE_STOP) {
		return true;
	}

	/*
	** CollisionMode == COLLIDE_KILL
	** If something is intersecting us and we are in KILL mode, then we kill them!
	*/
	if (CollisionMode == COLLIDE_KILL) {
		NonRefPhysListIterator it(&intersection_list);
		for (it.First(); !it.Is_Done(); it.Next()) {
			it.Peek_Obj()->Expire();
		}
	}

	/*
	** CollisionMode == COLLIDE_PUSH
	** Handle collisions by pushing the objects out of our way
	*/
	bool revert = false;
	NonRefPhysListIterator it(&intersection_list);
	for (it.First();!it.Is_Done();it.Next()) {
		PhysClass * obj = it.Peek_Obj();
		
		VERBOSE_LOG(("intersecting object: %s\r\n",obj->Peek_Model()->Get_Name()));
		
		if (RiderManager.Contains(obj) && (obj->Peek_Carrier_Sub_Object() == collisionobj.Peek_Collision_Object())) {
		
			/*
			** If an object that we hit is attached to us already, squish and revert
			*/
			//obj->Notify_Squished(this);
			revert = true;
// FIXME (gth) commenting out rider squishing code for critical review!
			VERBOSE_LOG(("Squishing a rider!\r\n"));
		
		} else {

			/*
			** Otherwise, try to push the object out of our way.
			*/
			int collision_bits = collisionobj.Clear_Collision_Bits();
  			if (Push_Collided_Object(obj,delta_transform) == false) {
				VERBOSE_LOG(("SAPO %s Failed to push a rider\r\n",Parent.Peek_Model()->Get_Name()));
				revert = true;
			} 
			collisionobj.Restore_Collision_Bits(collision_bits);
		}
	}

	return revert;
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Is_Intersecting -- intersection test                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
bool AnimCollisionManagerClass::Is_Intersecting(void)
{
	NonRefPhysListClass intersection_list;
	for (int ci=0; ci<CollisionMeshes.Count() && intersection_list.Is_Empty(); ci++) {
		CollisionMeshes[ci].Intersect_Scene(Parent.Get_Collision_Group(),&intersection_list);
	}
	return !intersection_list.Is_Empty();
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Push_Collided_Object -- push an object we collided with          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
bool AnimCollisionManagerClass::Push_Collided_Object(PhysClass * obj,const Matrix3D & delta_transform)
{
	bool result = false;
	MoveablePhysClass * move_obj = obj->As_MoveablePhysClass();
	if (move_obj != NULL) {
		
		VERBOSE_LOG(("Pushing object: %s, vector: (%f, %f, %f)\r\n",move_obj->Peek_Model()->Get_Name(),
				delta_transform.Get_Translation().X,
				delta_transform.Get_Translation().Y,
				delta_transform.Get_Translation().Z ));

		PushRecordClass::Add_To_List(&PushList,obj);
		result = move_obj->Push(delta_transform.Get_Translation());
	} 

	return result;
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Save -- save!                                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
bool AnimCollisionManagerClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(ANIMCOLLISIONMANAGER_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,ANIMCOLLISIONMANAGER_VARIABLE_COLLISIONMODE,CollisionMode);
	WRITE_MICRO_CHUNK(csave,ANIMCOLLISIONMANAGER_VARIABLE_ANIMATIONMODE,AnimationMode);
	WRITE_MICRO_CHUNK(csave,ANIMCOLLISIONMANAGER_VARIABLE_TARGETFRAME,TargetFrame);
	WRITE_MICRO_CHUNK(csave,ANIMCOLLISIONMANAGER_VARIABLE_CURFRAME,CurFrame);
	WRITE_MICRO_CHUNK(csave,ANIMCOLLISIONMANAGER_VARIABLE_PREVFRAME,PrevFrame);
	WRITE_MICRO_CHUNK(csave,ANIMCOLLISIONMANAGER_VARIABLE_LOOPSTART,LoopStart);
	WRITE_MICRO_CHUNK(csave,ANIMCOLLISIONMANAGER_VARIABLE_LOOPEND,LoopEnd);

	if (CurAnimation != NULL) {
		WRITE_MICRO_CHUNK_STRING(csave,ANIMCOLLISIONMANAGER_VARIABLE_ANIMATIONNAME,CurAnimation->Get_Name());
	}

	if (PrevAnimation != NULL) {
		WRITE_MICRO_CHUNK_STRING(csave,ANIMCOLLISIONMANAGER_VARIABLE_PREVANIMATIONNAME,PrevAnimation->Get_Name());
	}

	csave.End_Chunk();
	return true;
}


/***********************************************************************************************
 * AnimCollisionManagerClass::Load -- Load!                                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2000   gth : Created.                                                                 *
 *=============================================================================================*/
bool AnimCollisionManagerClass::Load(ChunkLoadClass &cload)
{
	StringClass anim_name;
	StringClass prev_anim_name;

	/*
	** Read in the chunks from the file
	*/
	while (cload.Open_Chunk()) {
		
		switch(cload.Cur_Chunk_ID()) 
		{
			case ANIMCOLLISIONMANAGER_CHUNK_VARIABLES:
			
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,ANIMCOLLISIONMANAGER_VARIABLE_COLLISIONMODE,CollisionMode);
						READ_MICRO_CHUNK(cload,ANIMCOLLISIONMANAGER_VARIABLE_ANIMATIONMODE,AnimationMode);
						READ_MICRO_CHUNK(cload,ANIMCOLLISIONMANAGER_VARIABLE_TARGETFRAME,TargetFrame);
						READ_MICRO_CHUNK(cload,ANIMCOLLISIONMANAGER_VARIABLE_CURFRAME,CurFrame);
						READ_MICRO_CHUNK(cload,ANIMCOLLISIONMANAGER_VARIABLE_PREVFRAME,PrevFrame);
						READ_MICRO_CHUNK_WWSTRING(cload,ANIMCOLLISIONMANAGER_VARIABLE_ANIMATIONNAME,anim_name);
						READ_MICRO_CHUNK_WWSTRING(cload,ANIMCOLLISIONMANAGER_VARIABLE_PREVANIMATIONNAME,prev_anim_name);
						READ_MICRO_CHUNK(cload,ANIMCOLLISIONMANAGER_VARIABLE_LOOPSTART,LoopStart);
						READ_MICRO_CHUNK(cload,ANIMCOLLISIONMANAGER_VARIABLE_LOOPEND,LoopEnd);
					}
					cload.Close_Micro_Chunk();	
				}
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
		};
				
		cload.Close_Chunk();
	}

	if (!anim_name.Is_Empty()) {
		Internal_Set_Animation(anim_name);
	}

	if (!prev_anim_name.Is_Empty()) {
		HAnimClass * anim = WW3DAssetManager::Get_Instance()->Get_HAnim(prev_anim_name);
		if ( anim == NULL ) {
			WWDEBUG_SAY(( "FAILED TO FIND PREV ANIM IN AnimCollisionManagerClass::Internal_Set_Animation(\"%s\")\n", prev_anim_name ));
		}
		REF_PTR_SET(PrevAnimation,anim);
	}

	return true;
}	


/********************************************************************************************
**
** AnimCollisionManagerDefClass Implementation
** Note, AnimCollisionManagerDef is not a full-fleged definition class.  It is meant to be
** embedded inside another real definition.  (e.g. StaticAnimPhys)
**
********************************************************************************************/

enum 
{
	ANIMCOLLISIONMANAGERDEF_CHUNK_VARIABLES							= 525000306,
	
	ANIMCOLLISIONMANAGERDEF_VARIABLE_COLLISIONMODE					= 0x00,
	ANIMCOLLISIONMANAGERDEF_VARIABLE_ANIMATIONMODE,
	ANIMCOLLISIONMANAGERDEF_VARIABLE_ANIMATIONNAME,
};

AnimCollisionManagerDefClass::AnimCollisionManagerDefClass(void) :
	CollisionMode(AnimCollisionManagerClass::COLLIDE_PUSH),
	AnimationMode(AnimCollisionManagerClass::ANIMATE_LOOP)
{
}

AnimCollisionManagerDefClass::~AnimCollisionManagerDefClass(void)
{
}

void AnimCollisionManagerDefClass::Validate_Parameters(void)
{
	if (CollisionMode < 0) CollisionMode = 0;
	if (CollisionMode > AnimCollisionManagerClass::ANIMATE_MANUAL) CollisionMode = AnimCollisionManagerClass::ANIMATE_MANUAL;
}

bool AnimCollisionManagerDefClass::Save(ChunkSaveClass &csave)
{
	Validate_Parameters();

	csave.Begin_Chunk(ANIMCOLLISIONMANAGERDEF_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,ANIMCOLLISIONMANAGERDEF_VARIABLE_COLLISIONMODE,CollisionMode);	
	WRITE_MICRO_CHUNK(csave,ANIMCOLLISIONMANAGERDEF_VARIABLE_ANIMATIONMODE,AnimationMode);	
	WRITE_MICRO_CHUNK_WWSTRING(csave,ANIMCOLLISIONMANAGERDEF_VARIABLE_ANIMATIONNAME,AnimationName);	
	csave.End_Chunk();

	return true;
}

bool AnimCollisionManagerDefClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {

		switch(cload.Cur_Chunk_ID()) {

			case ANIMCOLLISIONMANAGERDEF_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,ANIMCOLLISIONMANAGERDEF_VARIABLE_COLLISIONMODE,CollisionMode);	
						READ_MICRO_CHUNK(cload,ANIMCOLLISIONMANAGERDEF_VARIABLE_ANIMATIONMODE,AnimationMode);	
						READ_MICRO_CHUNK_WWSTRING(cload,ANIMCOLLISIONMANAGERDEF_VARIABLE_ANIMATIONNAME,AnimationName);	
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

