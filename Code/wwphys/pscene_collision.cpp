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
 *                     $Archive:: /Commando/Code/wwphys/pscene_collision.cpp                  $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 2/13/02 2:16p                                               $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "pscene.h"
#include "physcoltest.h"
#include "physinttest.h"
#include "staticaabtreecull.h"
#include "dynamicaabtreecull.h"
#include "physgridcull.h"
#include "lightcull.h"
#include "staticphys.h"



bool PhysicsSceneClass::Do_Groups_Collide(int group0,int group1)
{
	int index = group0 | (group1 << COLLISION_FLAG_SHIFT);
	return AllowCollisionFlags[index];
}


void PhysicsSceneClass::Enable_Collision_Detection(int group0,int group1)
{
	assert(group0 >= 0);
	assert(group1 >= 0);
	assert(group0 <= MAX_COLLISION_GROUP);
	assert(group1 <= MAX_COLLISION_GROUP);

	unsigned int index;

	index = group0 | (group1 << COLLISION_FLAG_SHIFT);
	AllowCollisionFlags[index] = 1;

	index = group1 | (group0 << COLLISION_FLAG_SHIFT);
	AllowCollisionFlags[index] = 1;
}

void PhysicsSceneClass::Disable_Collision_Detection(int group0,int group1)
{
	assert(group0 >= 0);
	assert(group1 >= 0);
	assert(group0 <= MAX_COLLISION_GROUP);
	assert(group1 <= MAX_COLLISION_GROUP);

	unsigned int index;

	index = group0 | (group1 << COLLISION_FLAG_SHIFT);
	AllowCollisionFlags[index] = 0;

	index = group1 | (group0 << COLLISION_FLAG_SHIFT);
	AllowCollisionFlags[index] = 0;
}

void PhysicsSceneClass::Enable_All_Collision_Detections(int group)
{
	assert(group >= 0);
	assert(group <= MAX_COLLISION_GROUP);

	for (int i=0; i <= MAX_COLLISION_GROUP; i++) {

		unsigned int index;
		index = group | (i << COLLISION_FLAG_SHIFT);
		AllowCollisionFlags[index] = 1;

		index = i | (group << COLLISION_FLAG_SHIFT);
		AllowCollisionFlags[index] = 1;
	}
}


void PhysicsSceneClass::Disable_All_Collision_Detections(int group)
{
	assert(group >= 0);
	assert(group <= MAX_COLLISION_GROUP);

	for (int i=0; i <= MAX_COLLISION_GROUP; i++) {

		unsigned int index;
		index = group | (i << COLLISION_FLAG_SHIFT);
		AllowCollisionFlags[index] = 0;

		index = i | (group << COLLISION_FLAG_SHIFT);
		AllowCollisionFlags[index] = 0;
	}
}

void PhysicsSceneClass::Set_Collision_Region(const AABoxClass & bounds,int colgroup)
{
	Collect_Collideable_Objects(bounds,colgroup,true,true,&CollisionRegionList);
}

void PhysicsSceneClass::Release_Collision_Region(void)
{
	CollisionRegionList.Reset_List();
}

bool PhysicsSceneClass::Cast_Ray(PhysRayCollisionTestClass & raytest,bool use_collision_region)
{
	/*
	** Assert that the result structure has been initialized with the 
	** optimistic result that the entire move will be taken.  Each call 
	** should whittle down the Fraction variable so that we are left 
	** with the fraction that the closest object allowed.  If StartBad
	** is ever set to true, we can bail out.
	*/
	assert(raytest.Result->Fraction == 1.0f);
	assert(raytest.Result->StartBad == false);
	raytest.CollidedPhysObj = NULL;

	/*
	** Check against physical objects in our vicinity
	*/
	bool res = false;

	if (use_collision_region) {

		/*
		** Use the cached collision region list
		*/
		NonRefPhysListIterator it(&CollisionRegionList);
		for ( ; !it.Is_Done(); it.Next()) {
			PhysClass * obj = it.Peek_Obj();
			if (	Do_Groups_Collide(obj->Get_Collision_Group(),raytest.CollisionGroup) && 
					!obj->Is_Ignore_Me()	) 
			{
				res |= obj->Cast_Ray(raytest);
			}
		}
	
	} else {

		/*
		** Cull the collision check using the culling systems
		*/
		if (raytest.CheckStaticObjs) {
			res |= StaticCullingSystem->Cast_Ray(raytest);
			if (raytest.Result->StartBad) return true;
		}
		
		if (raytest.CheckDynamicObjs) {
			res |= DynamicCullingSystem->Cast_Ray(raytest);
			if (raytest.Result->StartBad) return true;
		}
	
	} 

	return res;
}

bool PhysicsSceneClass::Cast_AABox(PhysAABoxCollisionTestClass & boxtest,bool use_collision_region)
{
	/*
	** Assert that the result structure has been initialized with the 
	** optimistic result that the entire move will be taken.  Each call 
	** should whittle down the Fraction variable so that we are left 
	** with the fraction that the closest object allowed.  If StartBad
	** is ever set to true, we can bail out.
	*/
	WWASSERT(boxtest.Result->Fraction == 1.0f);
	WWASSERT(boxtest.Result->StartBad == false);
	boxtest.CollidedPhysObj = NULL;
	
	/*
	** Check against physical objects in our vicinity
	*/
	bool res = false;

	if (use_collision_region) {

		/*
		** Use the cached collision region list
		*/
		NonRefPhysListIterator it(&CollisionRegionList);
		for ( ; !it.Is_Done(); it.Next()) {
			PhysClass * obj = it.Peek_Obj();
			if (	Do_Groups_Collide(obj->Get_Collision_Group(),boxtest.CollisionGroup) && 
					!obj->Is_Ignore_Me()	) 
			{
				res |= obj->Cast_AABox(boxtest);
			}
		}
	
	} else {	

		/*
		** Cull the collision check using the culling systems
		*/
		if (boxtest.CheckStaticObjs) {
			res |= StaticCullingSystem->Cast_AABox(boxtest);
			if (boxtest.Result->StartBad) return true;
		}
		
		if (boxtest.CheckDynamicObjs) {
			res |= DynamicCullingSystem->Cast_AABox(boxtest);
			if (boxtest.Result->StartBad) return true;
		}
	}

	return res;
}

bool PhysicsSceneClass::Cast_OBBox(PhysOBBoxCollisionTestClass & boxtest,bool use_collision_region)
{
	/*
	** Assert that the result structure has been initialized with the 
	** optimistic result that the entire move will be taken.  Each call 
	** should whittle down the Fraction variable so that we are left 
	** with the fraction that the closest object allowed.  If StartBad
	** is ever set to true, we can bail out.
	*/
	assert(boxtest.Result->Fraction == 1.0f);
	assert(boxtest.Result->StartBad == false);
	boxtest.CollidedPhysObj = NULL;
	
	/*
	** Check against physical objects in our vicinity
	*/
	bool res = false;

	if (use_collision_region) {

		/*
		** Use the cached collision region list
		*/
		NonRefPhysListIterator it(&CollisionRegionList);
		for ( ; !it.Is_Done(); it.Next()) {
			PhysClass * obj = it.Peek_Obj();
			if (	Do_Groups_Collide(obj->Get_Collision_Group(),boxtest.CollisionGroup) && 
					!obj->Is_Ignore_Me()	) 
			{
				res |= obj->Cast_OBBox(boxtest);
			}
		}
	
	} else {	

		/*
		** Cull the collision check using the culling systems
		*/
		if (boxtest.CheckStaticObjs) {
			res |= StaticCullingSystem->Cast_OBBox(boxtest);
			if (boxtest.Result->StartBad) return true;
		}
		
		if (boxtest.CheckDynamicObjs) {
			res |= DynamicCullingSystem->Cast_OBBox(boxtest);
			if (boxtest.Result->StartBad) return true;
		}
	}

	return res;
}

bool PhysicsSceneClass::Intersection_Test(PhysAABoxIntersectionTestClass & boxtest,bool use_collision_region)
{
	if (use_collision_region) {
	
		/*
		** Test for intersection with objects in the cached collision region
		*/
		NonRefPhysListIterator it(&CollisionRegionList);
		for ( ; !it.Is_Done(); it.Next()) {
			PhysClass * obj = it.Peek_Obj();
			if (	Do_Groups_Collide(obj->Get_Collision_Group(),boxtest.CollisionGroup) && 
					!obj->Is_Ignore_Me()	) 
			{
				if (obj->Intersection_Test(boxtest)) {
					return true;
				}
			}
		}
		
	} else {

		/*
		** Test for intersection with objects in the static and dynamic culling systems
		*/
		if (boxtest.CheckStaticObjs) {
			if (StaticCullingSystem->Intersection_Test(boxtest)) {
				return true;
			}
		}
		if (boxtest.CheckDynamicObjs) {
			if (DynamicCullingSystem->Intersection_Test(boxtest)) {
				return true;
			}
		}
	
	}

	return false;
}

bool PhysicsSceneClass::Intersection_Test(PhysOBBoxIntersectionTestClass & boxtest,bool use_collision_region)
{
	if (use_collision_region) {
		
		/*
		** Test for intersection with objects in the cached collision region
		*/
		NonRefPhysListIterator it(&CollisionRegionList);
		for ( ; !it.Is_Done(); it.Next()) {
			PhysClass * obj = it.Peek_Obj();
			if (	Do_Groups_Collide(obj->Get_Collision_Group(),boxtest.CollisionGroup) && 
					!obj->Is_Ignore_Me()	) 
			{
				if (obj->Intersection_Test(boxtest)) {
					return true;
				}
			}
		}
	
	} else {

		/*
		** Test for intersection with objects in the static and dynamic culling systems
		*/
		if (boxtest.CheckStaticObjs) {
			if (StaticCullingSystem->Intersection_Test(boxtest)) {
				return true;
			}
		}
		if (boxtest.CheckDynamicObjs) {
			if (DynamicCullingSystem->Intersection_Test(boxtest)) {
				return true;
			}
		}
	}
	return false;
}

bool PhysicsSceneClass::Intersection_Test(PhysMeshIntersectionTestClass & meshtest,bool use_collision_region)
{
	if (use_collision_region) {
		
		/*
		** Test for intersection with objects in the cached collision region
		*/
		NonRefPhysListIterator it(&CollisionRegionList);
		for ( ; !it.Is_Done(); it.Next()) {
			PhysClass * obj = it.Peek_Obj();
			if (	Do_Groups_Collide(obj->Get_Collision_Group(),meshtest.CollisionGroup) && 
					!obj->Is_Ignore_Me()	) 
			{
				if (obj->Intersection_Test(meshtest)) {
					return true;
				}
			}
		}
	
	} else {

		if (meshtest.CheckStaticObjs) {
			if (StaticCullingSystem->Intersection_Test(meshtest)) {
				return true;
			}
		}
		if (meshtest.CheckDynamicObjs) {
			if (DynamicCullingSystem->Intersection_Test(meshtest)) {
				return true;
			}
		}

	}
	return false;
}

bool PhysicsSceneClass::Intersection_Test(const AABoxClass & box,int collision_group,int collision_type,bool use_collision_region)
{
	NonRefPhysListClass intersect_list;
	PhysAABoxIntersectionTestClass test(box,collision_group,collision_type,&intersect_list);
	return Intersection_Test(test,use_collision_region);
}

bool PhysicsSceneClass::Intersection_Test(const OBBoxClass & box,int collision_group,int collision_type,bool use_collision_region)
{
	NonRefPhysListClass intersect_list;
	PhysOBBoxIntersectionTestClass test(box,collision_group,collision_type,&intersect_list);
	return Intersection_Test(test,use_collision_region);
}

void PhysicsSceneClass::Add_Collected_Objects_To_List
(
	bool static_objs,
	bool dynamic_objs,
	NonRefPhysListClass * list
)
{
	// link the static objects
	if (static_objs) {
		StaticPhysClass * obj;
		for (	obj = (StaticPhysClass *)StaticCullingSystem->Get_First_Collected_Object();
				obj != NULL;
				obj = (StaticPhysClass *)StaticCullingSystem->Get_Next_Collected_Object(obj) )
		{
			list->Add(obj);
		}
	}

	// link the dynamic objects
	if (dynamic_objs) {
		PhysClass * obj;
		for (	obj = (PhysClass *)DynamicCullingSystem->Get_First_Collected_Object();
				obj != NULL;
				obj = (PhysClass *)DynamicCullingSystem->Get_Next_Collected_Object(obj) )
		{
			list->Add(obj);
		}
	}
}


void PhysicsSceneClass::Add_Collected_Collideable_Objects_To_List
(
	int colgroup,
	bool static_objs,
	bool dynamic_objs,
	NonRefPhysListClass * list
)
{
	// link the static objects
	if (static_objs) {
		StaticPhysClass * obj;
		for (	obj = (StaticPhysClass *)StaticCullingSystem->Get_First_Collected_Object();
				obj != NULL;
				obj = (StaticPhysClass *)StaticCullingSystem->Get_Next_Collected_Object(obj) )
		{
			if (	Do_Groups_Collide(obj->Get_Collision_Group(),colgroup) && 
					!obj->Is_Ignore_Me()	) 
			{
				list->Add(obj);
			}
		}
	}

	// link the dynamic objects
	if (dynamic_objs) {
		PhysClass * obj;
		for (	obj = (PhysClass *)DynamicCullingSystem->Get_First_Collected_Object();
				obj != NULL;
				obj = (PhysClass *)DynamicCullingSystem->Get_Next_Collected_Object(obj) )
		{
			if (	Do_Groups_Collide(obj->Get_Collision_Group(),colgroup) && 
					!obj->Is_Ignore_Me()	) 
			{
				list->Add(obj);
			}
		}
	}
}

void PhysicsSceneClass::Collect_Objects
(
	const Vector3 & point,
	bool static_objs, 
	bool dynamic_objs,
	NonRefPhysListClass * list
)
{
	WWASSERT(list != NULL);

	if (static_objs) {
		StaticCullingSystem->Reset_Collection();
		StaticCullingSystem->Collect_Objects(point);
	}

	if (dynamic_objs) {
		DynamicCullingSystem->Reset_Collection();
		DynamicCullingSystem->Collect_Objects(point);
	}

	Add_Collected_Objects_To_List(static_objs,dynamic_objs,list);
}

void PhysicsSceneClass::Collect_Objects
(
	const AABoxClass & box,
	bool static_objs, 
	bool dynamic_objs,
	NonRefPhysListClass * list
)
{
	WWASSERT(list != NULL);

	if (static_objs) {
		StaticCullingSystem->Reset_Collection();
		StaticCullingSystem->Collect_Objects(box);
	}

	if (dynamic_objs) {
		DynamicCullingSystem->Reset_Collection();
		DynamicCullingSystem->Collect_Objects(box);
	}

	Add_Collected_Objects_To_List(static_objs,dynamic_objs,list);
}

void PhysicsSceneClass::Collect_Objects
(
	const OBBoxClass & box,
	bool static_objs, 
	bool dynamic_objs,
	NonRefPhysListClass * list
)
{
	WWASSERT(list != NULL);
	if (static_objs) {
		StaticCullingSystem->Reset_Collection();
		StaticCullingSystem->Collect_Objects(box);
	}

	if (dynamic_objs) {
		DynamicCullingSystem->Reset_Collection();
		DynamicCullingSystem->Collect_Objects(box);
	}

	Add_Collected_Objects_To_List(static_objs,dynamic_objs,list);
}

void PhysicsSceneClass::Collect_Objects
(
	const FrustumClass & frustum,
	bool static_objs, 
	bool dynamic_objs,
	NonRefPhysListClass * list
)
{
	WWASSERT(list != NULL);
	if (static_objs) {
		StaticCullingSystem->Reset_Collection();
		StaticCullingSystem->Collect_Objects(frustum);
	}

	if (dynamic_objs) {
		DynamicCullingSystem->Reset_Collection();
		DynamicCullingSystem->Collect_Objects(frustum);
	}

	Add_Collected_Objects_To_List(static_objs,dynamic_objs,list);
}

void PhysicsSceneClass::Collect_Collideable_Objects
(
	const AABoxClass & box,
	int colgroup,
	bool static_objs,
	bool dynamic_objs,
	NonRefPhysListClass * list
)
{
	WWASSERT(list != NULL);
	if (static_objs) {
		StaticCullingSystem->Reset_Collection();
		StaticCullingSystem->Collect_Objects(box);
	}

	if (dynamic_objs) {
		DynamicCullingSystem->Reset_Collection();
		DynamicCullingSystem->Collect_Objects(box);
	}

	Add_Collected_Collideable_Objects_To_List(colgroup,static_objs,dynamic_objs,list);
}

void PhysicsSceneClass::Collect_Collideable_Objects
(
	const OBBoxClass & box,
	int colgroup,
	bool static_objs,
	bool dynamic_objs,
	NonRefPhysListClass * list
)
{
	WWASSERT(list != NULL);
	if (static_objs) {
		StaticCullingSystem->Reset_Collection();
		StaticCullingSystem->Collect_Objects(box);
	}

	if (dynamic_objs) {
		DynamicCullingSystem->Reset_Collection();
		DynamicCullingSystem->Collect_Objects(box);
	}

	Add_Collected_Collideable_Objects_To_List(colgroup,static_objs,dynamic_objs,list);
}

void PhysicsSceneClass::Add_Collected_Lights_To_List
(
	bool static_lights,
	bool /* dynamic_lights */,
	NonRefPhysListClass * list
)
{
	// link the static lights
	if (static_lights) {
		LightPhysClass * obj;
		for (	obj = StaticLightingSystem->Get_First_Collected_Object();
				obj != NULL;
				obj = StaticLightingSystem->Get_Next_Collected_Object(obj) )
		{
			list->Add(obj);
		}
	}

	// link the dynamic lights
	// TODO!!
}

void PhysicsSceneClass::Collect_Lights
(
	const Vector3 & point,
	bool static_lights,
	bool dynamic_lights,
	NonRefPhysListClass * list
)
{
	WWASSERT(list != NULL);

	if (static_lights) {
		StaticLightingSystem->Reset_Collection();
		StaticLightingSystem->Collect_Objects(point);
	}

	// TODO: Dynamic lights!!

	Add_Collected_Lights_To_List(static_lights,dynamic_lights,list);
}


void PhysicsSceneClass::Collect_Lights
(
	const AABoxClass & bounds,
	bool static_lights,
	bool dynamic_lights,
	NonRefPhysListClass * list
)
{
	WWASSERT(list != NULL);

	if (static_lights) {
		StaticLightingSystem->Reset_Collection();
		StaticLightingSystem->Collect_Objects(bounds);
	}

	// TODO: Dynamic lights!!

	Add_Collected_Lights_To_List(static_lights,dynamic_lights,list);
}

