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
 *                     $Archive:: /Commando/Code/wwphys/staticaabtreecull.cpp                 $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/19/02 5:38p                                               $*
 *                                                                                             *
 *                    $Revision:: 26                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "staticaabtreecull.h"
#include "staticphys.h"
#include "wwphystrig.h"
#include "ww3d.h"
#include "wwphysids.h"
#include "camera.h"
#include "shader.h"
#include "vertmaterial.h"
#include "vistable.h"
#include "visrasterizer.h"
#include "chunkio.h"
#include "visrendercontext.h"
#include "colmathfrustum.h"
#include "colmathplane.h"
#include "colmathaabox.h"
#include "wwmemlog.h"


#define LOG_HIERARCHICAL_CULLING				0
#define VIS_SAMPLE_DISTANCE					20.0f


/*
** Static variables
*/
static int _HierarchicalCellsRejected = 0;

/*
** Chunk Id's used by the static-aabtree code to save itself into a file
*/
enum 
{
	STATICAABTREE_CHUNK_VERSION					= 0x00000001,	// OBSOLETE version wrapper, contains 32bit version #
	STATICAABTREE_CHUNK_AABTREE_CLASS_DATA		= 0x00000104,	// wraps the AABTreeCullSystemClass's save data
};


/*
** StaticAABTreeCullClass is a derived AABTree which assumes it contains StaticPhysClasses
** these two functions encapsulate some typecasting which happens in a lot 
** of places...
*/
inline StaticPhysClass * get_first_object(AABTreeNodeClass * node)
{
	return (StaticPhysClass *)(node->Object);		
}

inline StaticPhysClass * get_next_object(StaticPhysClass * tile)
{
	return (StaticPhysClass *)(((AABTreeLinkClass *)tile->Get_Cull_Link())->NextObject);
}

 
 

/*
** Implementation of StaticAABTreeCullClass
*/
StaticAABTreeCullClass::StaticAABTreeCullClass(PhysicsSceneClass * pscene) :
	PhysAABTreeCullClass(pscene)
{
}

StaticAABTreeCullClass::~StaticAABTreeCullClass(void)
{
}

void StaticAABTreeCullClass::Add_Object(PhysClass * obj, int cull_node_id /*= -1*/)
{
	WWASSERT(Scene != NULL);
	WWASSERT(obj->As_StaticPhysClass() != NULL);
	PhysAABTreeCullClass::Add_Object(obj, cull_node_id);
// (gth) not resetting vis when adding a static object
//	Scene->Reset_Vis();
}

void StaticAABTreeCullClass::Remove_Object(PhysClass * obj)
{
	WWASSERT(Scene != NULL);
	WWASSERT(obj->As_StaticPhysClass() != NULL);
	PhysAABTreeCullClass::Remove_Object(obj);
// (gth) not resetting vis when removing a static object
//	Scene->Reset_Vis();
}

void StaticAABTreeCullClass::Update_Culling(CullableClass * obj)
{
	WWASSERT(Scene != NULL);
	PhysAABTreeCullClass::Update_Culling(obj);
// (gth) not resetting vis when moving a static object
//	Scene->Reset_Vis();
}

void StaticAABTreeCullClass::Collect_Visible_Objects
(
	const FrustumClass &		frustum,
	VisTableClass *			pvs,
	RefPhysListClass &		visobjlist,
	RefPhysListClass &		wsmeshlist
)
{
	WWASSERT(RootNode != NULL);

	if (pvs != NULL) {
	
		/*
		** If we got a pvs then we call the custom hierarchical visible object
		** collection function.  
		*/
	
		VisObjCollectContextClass context(frustum,*pvs,visobjlist,wsmeshlist);
		if (Scene->Is_Vis_Inverted() || !Is_Hierarchical_Vis_Culling_Enabled()) {
			Collect_Visible_Objects_No_HVis_Recursive(RootNode,context);
		} else {
			_HierarchicalCellsRejected = 0;
			Collect_Visible_Objects_Recursive(RootNode,context);
#if LOG_HIERARCHICAL_CULLING
			if (_HierarchicalCellsRejected > 0) {
				WWDEBUG_SAY(("HCells Rejected: %d\n",_HierarchicalCellsRejected));
			}
#endif
		}


	} else {

		/*
		** Otherwise, just call the built-in frustum collection function and 
		** link the objects into the given lists
		*/
		Reset_Collection();
		Collect_Objects(frustum);

		/*
		** Loop over each object, adding it into the list
		*/
		StaticPhysClass * obj;
		for (	obj = (StaticPhysClass *)Get_First_Collected_Object(); 
				obj != NULL; 
				obj = (StaticPhysClass *)Get_Next_Collected_Object(obj)) 
		{
			if (obj->Is_World_Space_Mesh()) {
				wsmeshlist.Add(obj);
			} else {
				visobjlist.Add(obj);
			}
		}
	}
}

void StaticAABTreeCullClass::Collect_Visible_Objects_Recursive
(
	AABTreeNodeClass *				node,
	VisObjCollectContextClass &	context
)
{
	/*
	** If this node is not visible, stop.
	*/
	if (context.PVS.Get_Bit(node->UserData) == 0) {
#if LOG_HIERARCHICAL_CULLING
	 	_HierarchicalCellsRejected++;
#endif
		NODE_REJECTED();
		return;
	}

	/*
	** Cull the bounding volume of this node against the frustum.
	** If it is culled, stop descending the tree.
	*/
	CollisionMath::OverlapType overlap;
	overlap = CollisionMath::Overlap_Test(context.Frustum,node->Box,context.PlanesPassed);
	if (overlap == CollisionMath::OUTSIDE) {
		NODE_REJECTED();
		return;
	} 

	NODE_ACCEPTED();
	
	/*
	** Test any objects in this node
	*/
	if (node->Object) {

		StaticPhysClass * obj = get_first_object(node);
		while (obj) {

			if (	(context.PVS.Get_Bit(obj->Get_Vis_Object_ID()) != 0) &&
					(CollisionMath::Overlap_Test(context.Frustum,obj->Get_Cull_Box(),context.PlanesPassed) != CollisionMath::OUTSIDE) )
			{
				if (obj->Is_World_Space_Mesh()) {
					context.WSMeshList.Add(obj);
				} else {
					context.VisObjList.Add(obj);
				}
			}
			obj = get_next_object(obj);
		}
	}

	/*
	** Recurse into the children
	*/
	if (node->Back) {
		Collect_Visible_Objects_Recursive(node->Back,context);
	}
	if (node->Front) {
		Collect_Visible_Objects_Recursive(node->Front,context);
	}
}



void StaticAABTreeCullClass::Collect_Visible_Objects_No_HVis_Recursive
(
	AABTreeNodeClass *				node,
	VisObjCollectContextClass &	context
)
{
	/*
	** Cull the bounding volume of this node against the frustum.
	** If it is culled, stop descending the tree.
	*/
	CollisionMath::OverlapType overlap;
	overlap = CollisionMath::Overlap_Test(context.Frustum,node->Box,context.PlanesPassed);
	if (overlap == CollisionMath::OUTSIDE) {
		NODE_REJECTED();
		return;
	} 

	NODE_ACCEPTED();
	
	/*
	** Test any objects in this node
	*/
	if (node->Object) {

		StaticPhysClass * obj = get_first_object(node);
		while (obj) {

			if (	(context.PVS.Get_Bit(obj->Get_Vis_Object_ID()) != 0) &&
					(CollisionMath::Overlap_Test(context.Frustum,obj->Get_Cull_Box(),context.PlanesPassed) != CollisionMath::OUTSIDE) )
			{
				if (obj->Is_World_Space_Mesh()) {
					context.WSMeshList.Add(obj);
				} else {
					context.VisObjList.Add(obj);
				}
			}
			obj = get_next_object(obj);
		}
	}

	/*
	** Recurse into the children
	*/
	if (node->Back) {
		Collect_Visible_Objects_No_HVis_Recursive(node->Back,context);
	}
	if (node->Front) {
		Collect_Visible_Objects_No_HVis_Recursive(node->Front,context);
	}
}


void StaticAABTreeCullClass::Assign_Vis_IDs(void)
{
	/*
	** Allocate sector and object ID's for the objects in the leaf nodes.
	*/
	int nodecount = Partition_Node_Count();

	for (int i=0; i<nodecount; i++) {
		AABTreeNodeClass * node = IndexedNodes[i];

		/*
		** assign a vis-object-id to each node in the tree (for hierarchical visibility!)
		*/
		node->UserData = Scene->Allocate_Vis_Object_ID();

		/*
		** assign vis-object-id's to all objects in this node.
		** assign vis-sector-id's to any object that contains vis meshes.
		*/
		StaticPhysClass * obj = get_first_object(node);
		while(obj) {
			obj->Set_Vis_Object_ID(Scene->Allocate_Vis_Object_ID());
			if (obj->Is_Vis_Sector()) {
				obj->Set_Vis_Sector_ID(Scene->Allocate_Vis_Sector_ID());
			}
			obj = get_next_object(obj);
		}
	}
}

void StaticAABTreeCullClass::Evaluate_Occluder_Visibility
(
	VisRenderContextClass &		context,
	VisSampleClass &				sample
)
{
	context.VisRasterizer->Set_Render_Mode(IDBufferClass::OCCLUDER_MODE);
	context.VisRasterizer->Clear();

	Render_Occluders(RootNode,context);

	context.Scan_Frame_Buffer(&sample);
}

void StaticAABTreeCullClass::Evaluate_Non_Occluder_Visibility
(
	VisRenderContextClass &		context
)
{
	/*
	** Tell the rasterizer to not write into its Z-buffer and to always exit early
	*/
	context.VisRasterizer->Set_Render_Mode(IDBufferClass::NON_OCCLUDER_MODE);

	/*
	** Collect the non-occluder render objects
	*/
	RefPhysListClass non_occluders;
	Collect_Non_Occluders(RootNode,context,non_occluders);


	if (context.Is_Vis_Quick_And_Dirty()) {
		
		RefPhysListIterator it(&non_occluders);
		for (it.First(); !it.Is_Done(); it.Next()) {
			StaticPhysClass * obj = (StaticPhysClass *)it.Peek_Obj();
			WWASSERT(obj != NULL);
			context.VisTable.Set_Bit(obj->Get_Vis_Object_ID(),true);
		}

	} else {

		RefPhysListIterator it(&non_occluders);
		for (it.First(); !it.Is_Done(); it.Next()) {
			StaticPhysClass * obj = (StaticPhysClass *)it.Peek_Obj();
			WWASSERT(obj != NULL);

			/*
			** If the camera is inside the bounding box of this non-occluder; mark it visible
			*/
			if (obj->Get_Bounding_Box().Contains(context.Camera.Get_Position())) {
				context.VisTable.Set_Bit(obj->Get_Vis_Object_ID(),true);
			}

			/*
			** Check this object if its bounding box is in view and we don't already
			** think it can be seen.
			*/
			if (	!context.Camera.Cull_Box(obj->Get_Bounding_Box()) && 
					(context.VisTable.Get_Bit(obj->Get_Vis_Object_ID()) == 0) ) 
			{
				context.VisRasterizer->Reset_Pixel_Counter();
				context.Set_Vis_ID(obj->Get_Vis_Object_ID());	// set up the vis id
				obj->Vis_Render(context);								// render the model

				if (context.VisRasterizer->Get_Pixel_Counter() > 0) {			// if any pixels passed, the obj is visible!
					context.VisTable.Set_Bit(obj->Get_Vis_Object_ID(),true);
				}
			}
		}
	}
}


void StaticAABTreeCullClass::Render_Occluders(AABTreeNodeClass * node,VisRenderContextClass & context)
{
	if (context.Camera.Cull_Box(node->Box)) {
		return;
	}

	if (node->Object) {
		StaticPhysClass * obj = get_first_object(node);
		while(obj) {

			if (obj->Is_Occluder()) {
				context.Set_Vis_ID(obj->Get_Vis_Object_ID());
				obj->Vis_Render(context);
			}

			obj = get_next_object(obj);
		}
	}

	if (node->Back) {
		Render_Occluders(node->Back,context);
	}
	if (node->Front) {
		Render_Occluders(node->Front,context);
	}
}

void StaticAABTreeCullClass::Collect_Non_Occluders
(
	AABTreeNodeClass *			node,
	VisRenderContextClass &		context,
	RefPhysListClass &			non_occluder_list
)
{
	if (context.Camera.Cull_Box(node->Box)) {
		return;
	}

	if (node->Object) {
		StaticPhysClass * obj = get_first_object(node);
		while(obj) {
			if (!obj->Is_Occluder()) {
				non_occluder_list.Add(obj);
			}
			obj = get_next_object(obj);
		}
	}

	if (node->Back) {
		Collect_Non_Occluders(node->Back,context,non_occluder_list);
	}
	if (node->Front) {
		Collect_Non_Occluders(node->Front,context,non_occluder_list);
	}
}

StaticPhysClass * StaticAABTreeCullClass::Find_Vis_Tile(const Vector3 & sample_point)
{
	Vector3 p0 = sample_point;
	Vector3 p1 = p0;
	p1.Z -= VIS_SAMPLE_DISTANCE;

	//
	// Cast a fat box down
	//
	CastResultStruct box_result;
	AABoxClass box(p0,Vector3(0.05f,0.05f,0.0f));
	PhysAABoxCollisionTestClass boxtest(box,p1-p0,&box_result,0,COLLISION_TYPE_VIS);
	boxtest.CheckDynamicObjs = false;
	Cast_AABox(boxtest);

	//
	// Cast a ray down
	//
	CastResultStruct ray_result;
	LineSegClass ray(p0,p1);
	PhysRayCollisionTestClass raytest(ray,&ray_result,0,COLLISION_TYPE_VIS);
	raytest.CheckDynamicObjs = false;
	Cast_Ray(raytest);

	// 
	// Use the ray result, unless the fat box result was a lot closer to
	// the camera point (this is the case where the ray went through a crack or something. (ug)
	//
	float box_dist = -(box_result.Fraction * (p1.Z - p0.Z));
	float ray_dist = -(ray_result.Fraction * (p1.Z - p0.Z));

	if ((ray_dist > 3.5f) && (box_dist < ray_dist - 1.0f) && (box_result.Fraction < 1.0f)) {
	
		// return pointer to the tile that the box hit
		WWASSERT(boxtest.CollidedPhysObj);
		WWASSERT(boxtest.CollidedPhysObj->As_StaticPhysClass() != NULL);
		return (StaticPhysClass *)boxtest.CollidedPhysObj;
	
	} else if (ray_result.Fraction < 1.0f) {

		// return pointer to the tile that the ray hit
		WWASSERT(raytest.CollidedPhysObj);
		WWASSERT(raytest.CollidedPhysObj->As_StaticPhysClass() != NULL);
		return (StaticPhysClass *)raytest.CollidedPhysObj;

	} else {

		return NULL;

	}
}

int StaticAABTreeCullClass::Get_Vis_Sector_ID(const Vector3 & sample_point)
{
	StaticPhysClass * tile = Find_Vis_Tile(sample_point);
	if (tile == NULL) {
		return -1;
	}
	return tile->Get_Vis_Sector_ID();
}

void StaticAABTreeCullClass::Propogate_Hierarchical_Visibility(VisTableClass * pvs)
{
	Propogate_Hierarchical_Visibility_Recursive(RootNode,pvs);
}

void StaticAABTreeCullClass::Propogate_Hierarchical_Visibility_Recursive
(
	AABTreeNodeClass * node,
	VisTableClass * pvs
)
{
	/*
	** Recurse to the bottom of the tree first
	*/
	if (node->Front != NULL) {
		Propogate_Hierarchical_Visibility_Recursive(node->Front,pvs);
	} 

	if (node->Back != NULL) {
		Propogate_Hierarchical_Visibility_Recursive(node->Back,pvs);
	}

	/*
	** We can only turn on the visibility of this node if any of its children
	** are visible.  Turning off the visibility is not allowed because we may
	** be sharing our vis-object-id with another node due to vis-compression.
	*/
	bool child_visible = Is_Child_Visible(node,pvs);
	if (child_visible) {
		pvs->Set_Bit(node->UserData,true);
	}
}


bool StaticAABTreeCullClass::Is_Child_Visible
(
	AABTreeNodeClass * node,
	VisTableClass * pvs
)
{
	/*
	** If this is a leaf node, we check for visibility of any contained objects
	** Otherwise we check if either of the child nodes are visible
	*/
	if ((node->Front != NULL) && (pvs->Get_Bit(node->Front->UserData) != 0)) { 
		return true;
	}

	if ((node->Back != NULL) && (pvs->Get_Bit(node->Back->UserData) != 0)) {
		return true;
	}

	StaticPhysClass * obj = get_first_object(node);
	while(obj) {
		if (pvs->Get_Bit(obj->Get_Vis_Object_ID()) != 0) {
			return true;
		}
		obj = get_next_object(obj);
	}

	return false;
}

void StaticAABTreeCullClass::Merge_Vis_Object_IDs(uint32 id0,uint32 id1)
{
	/*
	** Each node and each staticphys object has a vis object id.  
	** Whenever we encounter one of these with id1, set it to id0.
	** Whenever we encounter one with id > id1, subtract one from it.
	*/
	for (int i=0; i<NodeCount; i++) {

		AABTreeNodeClass * node = IndexedNodes[i];
		if (node->UserData == id1) {
			node->UserData = id0;
		} else if (node->UserData > id1) {
			node->UserData--;
		}

		StaticPhysClass * obj = get_first_object(node);
		while (obj) {
			uint32 obj_id = obj->Get_Vis_Object_ID();
			if (obj_id != 0xFFFFFFFF) {
				if (obj_id == id1) {
					obj_id = id0;
				} else if (obj_id > id1) {
					obj_id--;
				}
				
				obj->Set_Vis_Object_ID(obj_id);
			}
			obj = get_next_object(obj);
		}
	}
}

void StaticAABTreeCullClass::Merge_Vis_Sector_IDs(uint32 id0,uint32 id1)
{
	/*
	** Each staticphys object may have a vis sector id.
	** Whenever we encounter one of these with id1, set it to id0.
	** Whenever we encounter one with id > id1, subtract one from it.
	*/
	for (int i=0; i<NodeCount; i++) {

		AABTreeNodeClass * node = IndexedNodes[i];

		StaticPhysClass * obj = get_first_object(node);
		while (obj) {

			uint32 sector_id = obj->Get_Vis_Sector_ID();
			if (sector_id != 0xFFFFFFFF) {
				if (sector_id == id1) {
					sector_id = id0;
				} else if (sector_id > id1) {
					sector_id--;
				}
				
				obj->Set_Vis_Sector_ID(sector_id);
			}
			obj = get_next_object(obj);
		}
	}
}


void StaticAABTreeCullClass::Load_Static_Data(ChunkLoadClass & cload)
{
	WWMEMLOG(MEM_CULLINGDATA);
	while(cload.Open_Chunk()) 
	{
		switch(cload.Cur_Chunk_ID()) 
		{
		
		case STATICAABTREE_CHUNK_AABTREE_CLASS_DATA:
			{
				TypedAABTreeCullSystemClass<PhysClass>::Load(cload);	
			}
			break;

		default:
			WWDEBUG_SAY(("Unhandled chunk type: %d in StaticAABTreeCullClass::Load\r\n",cload.Cur_Chunk_ID()));
			break;
		}
	
		cload.Close_Chunk();
	}
}

void StaticAABTreeCullClass::Save_Static_Data(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(STATICAABTREE_CHUNK_AABTREE_CLASS_DATA);
	TypedAABTreeCullSystemClass<PhysClass>::Save(csave);
	csave.End_Chunk();
}

