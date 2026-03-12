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
 *                     $Archive:: /Commando/Code/wwphys/physaabtreecull.cpp                   $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 5/05/01 5:14p                                               $*
 *                                                                                             *
 *                    $Revision:: 23                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "physaabtreecull.h"
#include "pscene.h"
#include "physcoltest.h"
#include "physinttest.h"
#include "wwstring.h"


/*
** Static members of PhysAABTreeCullClass
*/
bool	PhysAABTreeCullClass::_HierarchicalVisCullingEnabled = true;


/*
** PhysAABTreeCullClass is a derived AABTree which assumes it contains PhysClasses
** these two functions encapsulate some typecasting which happens in a lot
** of places...
*/
inline PhysClass * get_first_object(AABTreeNodeClass * node)
{
	return (PhysClass *)(node->Object);
}

inline PhysClass * get_next_object(PhysClass * tile)
{
	return (PhysClass *)(((AABTreeLinkClass *)tile->Get_Cull_Link())->NextObject);
}



/*
** Implementation of PhysAABTreeCullClass
*/
PhysAABTreeCullClass::PhysAABTreeCullClass(PhysicsSceneClass * pscene) :
	Scene(pscene)
{
}

PhysAABTreeCullClass::~PhysAABTreeCullClass(void)
{
}

bool PhysAABTreeCullClass::Verify(StringClass & error_report)
{
	return Verify_Recursive(RootNode,error_report);
}


bool PhysAABTreeCullClass::Cast_Ray_Recursive
(
	AABTreeNodeClass *				node,
	PhysRayCollisionTestClass &	raytest
)
{
	/*
	** Cull the collision test against the bounding volume of this node
	** If it is culled, stop descending the tree.
	*/
	if (raytest.Cull(node->Box)) {
		return false;
	}

	/*
	** Test any objects in this node
	*/
	bool res = false;
	if (node->Object) {
		PhysClass * obj = get_first_object(node);
		while (obj) {
			if (	Scene->Do_Groups_Collide(obj->Get_Collision_Group(),raytest.CollisionGroup) &&
					!obj->Is_Ignore_Me())
			{
				res |= obj->Cast_Ray(raytest);
			}
			obj = get_next_object(obj);
		}
	}

	/*
	** If it wasn't culled, collision test on to the children
	*/
	if (node->Back) {
		res = res | Cast_Ray_Recursive(node->Back,raytest);
	}
	if (node->Front) {
		res = res | Cast_Ray_Recursive(node->Front,raytest);
	}

	return res;
}


bool PhysAABTreeCullClass::Cast_AABox_Recursive
(
	AABTreeNodeClass *					node,
	PhysAABoxCollisionTestClass &		boxtest
)
{
	/*
	** Cull the collision test against the bounding volume of this node
	** If it is culled, stop descending the tree.
	*/
	if (boxtest.Cull(node->Box)) {
		return false;
	}

	/*
	** Test any objects in this node
	*/
	bool res = false;
	if (node->Object) {
		PhysClass * obj = get_first_object(node);
		while (obj) {
			if (	Scene->Do_Groups_Collide(obj->Get_Collision_Group(),boxtest.CollisionGroup) &&
					!obj->Is_Ignore_Me()	)
			{
				res |= obj->Cast_AABox(boxtest);
			}
			obj = get_next_object(obj);
		}
	}

	/*
	** If it wasn't culled, collision test on to the children
	*/
	if (node->Back) {
		res = res | Cast_AABox_Recursive(node->Back,boxtest);
	}
	if (node->Front) {
		res = res | Cast_AABox_Recursive(node->Front,boxtest);
	}

	return res;
}

bool PhysAABTreeCullClass::Cast_OBBox_Recursive
(
	AABTreeNodeClass *					node,
	PhysOBBoxCollisionTestClass &		boxtest
)
{
	/*
	** Cull the collision test against the bounding volume of this node
	** If it is culled, stop descending the tree.
	*/

	if (boxtest.Cull(node->Box)) {
		return false;
	}

	/*
	** Test any objects in this node
	*/
	bool res = false;
	if (node->Object) {
		PhysClass * obj = get_first_object(node);
		while (obj) {
			if (	Scene->Do_Groups_Collide(obj->Get_Collision_Group(),boxtest.CollisionGroup) &&
					!obj->Is_Ignore_Me()	) {
				res |= obj->Cast_OBBox(boxtest);
			}
			obj = get_next_object(obj);
		}
	}

	/*
	** If it wasn't culled, collision test on to the children
	*/
	if (node->Back) {
		res = res | Cast_OBBox_Recursive(node->Back,boxtest);
	}
	if (node->Front) {
		res = res | Cast_OBBox_Recursive(node->Front,boxtest);
	}

	return res;
}

bool PhysAABTreeCullClass::Intersection_Test(PhysAABoxIntersectionTestClass & boxtest)
{
	Reset_Collection();
	Collect_Objects(boxtest.Box);

	PhysClass * obj = Get_First_Collected_Object();
	while (obj != NULL) {

		if (	Scene->Do_Groups_Collide(obj->Get_Collision_Group(),boxtest.CollisionGroup) &&
				!obj->Is_Ignore_Me()	) {
			if (obj->Intersection_Test(boxtest)) {
				return true;
			}
		}

		obj = Get_Next_Collected_Object(obj);
	}

	return false;
}

bool PhysAABTreeCullClass::Intersection_Test(PhysOBBoxIntersectionTestClass & boxtest)
{
	Reset_Collection();
	Collect_Objects(boxtest.BoundingBox);

	PhysClass * obj = Get_First_Collected_Object();
	while (obj != NULL) {

		if (	Scene->Do_Groups_Collide(obj->Get_Collision_Group(),boxtest.CollisionGroup) &&
				!obj->Is_Ignore_Me()	) {
			if (obj->Intersection_Test(boxtest)) {
				return true;
			}
		}

		obj = Get_Next_Collected_Object(obj);
	}

	return false;
}

bool PhysAABTreeCullClass::Intersection_Test(PhysMeshIntersectionTestClass & meshtest)
{
	Reset_Collection();
	Collect_Objects(meshtest.BoundingBox);

	PhysClass * obj = Get_First_Collected_Object();
	while (obj != NULL) {

		if (	Scene->Do_Groups_Collide(obj->Get_Collision_Group(),meshtest.CollisionGroup) &&
				!obj->Is_Ignore_Me()	) {
			if (obj->Intersection_Test(meshtest)) {
				return true;
			}
		}

		obj = Get_Next_Collected_Object(obj);
	}

	return false;
}


bool PhysAABTreeCullClass::Verify_Recursive(AABTreeNodeClass * node,StringClass & error_report)
{
	/*
	** Test any objects in this node
	*/
	bool res = true;
	StringClass working;
	if (node->Object) {
		PhysClass * obj = get_first_object(node);
		while (obj) {

			const AABoxClass & box = node->Box;
			const AABoxClass & box2 = obj->Get_Cull_Box();
			bool outside = false;

			Vector3 dc;
			Vector3::Subtract(box.Center,box2.Center,&dc);

			if (box.Extent.X + box2.Extent.X < WWMath::Fabs(dc.X)) outside = true;
			if (box.Extent.Y + box2.Extent.Y < WWMath::Fabs(dc.Y)) outside = true;
			if (box.Extent.Z + box2.Extent.Z < WWMath::Fabs(dc.Z)) outside = true;

			if (	(dc.X + box2.Extent.X <= box.Extent.X + WWMATH_EPSILON)&
					(dc.Y + box2.Extent.Y <= box.Extent.Y + WWMATH_EPSILON) &&
					(dc.Z + box2.Extent.Z <= box.Extent.Z + WWMATH_EPSILON) &&
					(dc.X - box2.Extent.X >= -box.Extent.X - WWMATH_EPSILON) &&
					(dc.Y - box2.Extent.Y >= -box.Extent.Y - WWMATH_EPSILON) &&
					(dc.Z - box2.Extent.Z >= -box.Extent.Z - WWMATH_EPSILON))
			{
				outside = false;
			} else {
				outside = true;
			}

			if (	outside ) { //CollisionMath::Overlap_Test(node->Box,obj->Get_Cull_Box()) != CollisionMath::INSIDE) {
				//CollisionMath::Overlap_Test(node->Box,obj->Get_Cull_Box());
				working.Format("Node %d doesn't contain object: %s\r\n",node->Index,obj->Peek_Model()->Get_Name());
				error_report += working;
				res = false;
			}
			obj = get_next_object(obj);
		}
	}

	/*
	** If it wasn't culled, collision test on to the children
	*/
	if (node->Back) {
		res &= Verify_Recursive(node->Back,error_report);
	}
	if (node->Front) {
		res &= Verify_Recursive(node->Front,error_report);
	}

	return res;
}
