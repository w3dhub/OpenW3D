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
 *                     $Archive:: /Commando/Code/wwphys/dynamicaabtreecull.cpp                $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/06/02 11:24a                                              $*
 *                                                                                             *
 *                    $Revision:: 22                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "dynamicaabtreecull.h"
#include "pscene.h"
#include "visrendercontext.h"
#include "visoptimizationcontext.h"
#include "visoptprogress.h"
#include "visrasterizer.h"
#include "boxrobj.h"
#include "vector3i.h"
#include "chunkio.h"
#include "colmathfrustum.h"
#include "colmathplane.h"
#include "colmathaabox.h"
#include "wwmemlog.h"

const float DEFUALT_MAXOBJRADIUS = 7.5f;
const float MIN_CELL_HEIGHT = 3.5f;

/*
** Chunk ID's used by DynamicAABTreeClass
*/
enum
{
	DYNAMICAABTREE_CHUNK_AABTREE_CLASS_DATA		= 622120600,
	DYNAMICAABTREE_CHUNK_VARIABLES,

	DYNAMICAABTREE_VARIABLE_BASEVISID				= 0x00,
	DYNAMICAABTREE_VARIABLE_MAXOBJRADIUS,
};

/*
** DynamicAABTreeCullClass is a derived AABTree which assumes it contains PhysClasses
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
** DynamicAABTreeCullClass Implementation
*/
DynamicAABTreeCullClass::DynamicAABTreeCullClass(PhysicsSceneClass * pscene) :
	PhysAABTreeCullClass(pscene),
	MaxObjRadius(DEFUALT_MAXOBJRADIUS),
	RenderBox(NULL),
	DebugIterator(this)
{
	/*
	** Modify the root node so that any object can be added into the tree
	*/
	WWASSERT(RootNode != NULL);
	RootNode->Box.Extent.Set(FLT_MAX/4.0f,FLT_MAX/4.0f,FLT_MAX/4.0f);
}

DynamicAABTreeCullClass::~DynamicAABTreeCullClass(void)
{
	REF_PTR_RELEASE(RenderBox);
}

void DynamicAABTreeCullClass::Add_Object(PhysClass * obj)
{
	/*
	** Find the appropriate node to insert this object into.
	*/
	int node_index = find_insertion_node(obj);

	/*
	** Add the object to that node
	*/
	Add_Object_Internal(obj,node_index);
}


void DynamicAABTreeCullClass::Update_Culling(CullableClass * obj)
{
	WWASSERT(obj);
	WWASSERT(obj->Get_Culling_System() == this);

	/*
	** unlink it from the node it is currently in
	*/
	AABTreeLinkClass * link = (AABTreeLinkClass *)obj->Get_Cull_Link();
	WWASSERT(link);
	AABTreeNodeClass * node = link->Node;
	WWASSERT(node);
	node->Remove_Object(obj);

	/*
	** link it back into the tree
	*/
	int node_index = find_insertion_node(obj);
	WWASSERT(node_index < NodeCount);
	IndexedNodes[node_index]->Add_Object(obj,false);
}


uint32 DynamicAABTreeCullClass::Get_Dynamic_Object_Vis_ID(const AABoxClass & obj_bounds,int * node_id)
{
	bool is_big_obj = (	(obj_bounds.Extent.X > MaxObjRadius) ||
								(obj_bounds.Extent.Y > MaxObjRadius) ||
								(obj_bounds.Extent.Z > MaxObjRadius) );

	/*
	** If the user supplied a node_id, we'll start searching from there
	*/
	AABTreeNodeClass * start_node = RootNode;

#if 0 // Disabling coherency because characters are getting stuck in local minima in the tree...
	if ((node_id != NULL) && (*node_id >= 0) && (*node_id < NodeCount)) {
		start_node = IndexedNodes[*node_id];
	}

	/*
	** Step 1: Walk towards the root of the tree until we find a node that still
	** contains this object (hopefully the starting node does!)
	*/
	if (is_big_obj) {

		/*
		** For "big" objects, we check the entire box against the node's box
		*/
		while (	(CollisionMath::Overlap_Test(start_node->Box,obj_bounds) != CollisionMath::INSIDE) &&
					(start_node != RootNode)	)
		{
			start_node = start_node->Parent;
		}

	} else {

		/*
		** For "normal" objects, we check the center point against the "deflated" boxes.  This
		** is more correct for the way that VIS was generated.
		*/
		while (	(deflated_box_contains_point(start_node->Box,obj_bounds.Center) == false) &&
					(start_node != RootNode)	)
		{
			start_node = start_node->Parent;
		}
	}
#endif

	/*
	** Step 2: Now recurse down the tree to find the node this object should derive its visibility
	** from
	*/
	int node_index = 0;
	if (is_big_obj) {
		find_optimal_node(start_node,obj_bounds,node_index);
	} else {
		find_optimal_deflated_node(start_node,obj_bounds.Center,node_index);
	}

	/*
	** Step 3: return the results to the user!
	*/
	if (node_id != NULL) {
		*node_id = node_index;
	}
	return IndexedNodes[node_index]->UserData;
}

void DynamicAABTreeCullClass::Re_Partition
(
	DynamicVectorClass<AABoxClass> *		seed_boxes,
	const Vector3 &							grid_min,
	const Vector3 &							grid_max,
	const Vector3 &							min_grid_cell_size,
	int											max_grid_cell_count,
	float											max_obj_radius
)
{
	/*
	** Remember the max object radius
	*/
	MaxObjRadius = max_obj_radius;

	/*
	** Determine the grid parameters for the world.  We want to get the grid cell size
	** as close to min_grid_cell_size without exceeding max_grid_cell_count.
	*/
	AABoxClass grid_bounds;
	grid_bounds.Init_Min_Max(grid_min,grid_max);

	Vector3 worldsize = grid_max - grid_min;

	Vector3i cellcount;
	cellcount.I = worldsize.X / min_grid_cell_size.X;
	cellcount.J = worldsize.Y / min_grid_cell_size.Y;
	cellcount.K = worldsize.Z / min_grid_cell_size.Z;

	int total_grid_count = cellcount.I * cellcount.J * cellcount.K;
	while (total_grid_count > max_grid_cell_count) {

		int i;
		Vector3 cellfraction;
		for (i=0; i<3; i++) {
			cellfraction[i] = (worldsize[i] / cellcount[i]) / min_grid_cell_size[i];
		}

		int smalldim = 0;
		float smallfraction = cellfraction[0];
		for (i=1; i<2; i++) {
			if (cellfraction[i] < smallfraction) {
				smalldim = i;
				smallfraction = cellfraction[i];
			}
		}

		cellcount[smalldim]--;
		total_grid_count = cellcount.I * cellcount.J * cellcount.K;
	}

	Vector3 cellsize;
	cellsize.X = worldsize.X / cellcount.I;
	cellsize.Y = worldsize.Y / cellcount.J;
	cellsize.Z = worldsize.Z / cellcount.K;

	/*
	** Allocate the complete set of seed boxes.  This will be *either* a voxelization
	** of the level *or* a set of seed boxes passed in (which were presumably generated
	** from a pathfind floodfill of the level).
	*/
	SimpleDynVecClass<AABoxClass> boxes;
	Vector3 boxmin,boxmax;
	AABoxClass box;

	if ((seed_boxes != NULL) && (seed_boxes->Count() > 0)) {

		/*
		** Add in the seed_boxes
		*/
		for (int i=0; i<seed_boxes->Count(); i++) {

			/*
			** Copy the input box, inflate it, add it to the array
			*/
			box = (*seed_boxes)[i];
			float height = 2.0f * box.Extent.Z;
			if (height < MIN_CELL_HEIGHT) {
				box.Extent.Z += (MIN_CELL_HEIGHT - height) / 2.0f;
				box.Center.Z += (MIN_CELL_HEIGHT - height) / 2.0f;
			}
			box.Extent += Vector3(MaxObjRadius,MaxObjRadius,MaxObjRadius);
			boxes.Add(box);
		}
	}

	/*
	** Add in the voxelization boxes
	*/
	for (int k=0; k<cellcount.K; k++) {
		for (int j=0; j<cellcount.J; j++) {
			for (int i=0; i<cellcount.I; i++) {

				/*
				** Compute the bounds of the grid cell
				*/
				boxmin.X = grid_min.X + i * cellsize.X;
				boxmin.Y = grid_min.Y + j * cellsize.Y;
				boxmin.Z = grid_min.Z + k * cellsize.Z;
				boxmax = boxmin + cellsize;

				/*
				** Inflate by MaxObjRadius
				*/
				boxmin -= Vector3(MaxObjRadius,MaxObjRadius,MaxObjRadius);
				boxmax += Vector3(MaxObjRadius,MaxObjRadius,MaxObjRadius);

				/*
				** Add to the array
				*/
				box.Init_Min_Max(boxmin,boxmax);
				boxes.Add(box);
			}
		}
	}

	/*
	** Call the base class re-partition function with our list of leaf boxes
	*/
	PhysAABTreeCullClass::Re_Partition(grid_bounds,boxes);

	/*
	** Mark the Visibility system dirty since our boxes all changed
	*/
	Scene->Reset_Vis();

	/*
	** Modify the root node so that any object can be added into the tree
	*/
	RootNode->Box.Extent.Set(FLT_MAX/4.0f,FLT_MAX/4.0f,FLT_MAX/4.0f);
}

void DynamicAABTreeCullClass::Collect_Visible_Objects
(
	const FrustumClass & frustum,
	VisTableClass * pvs,
	RefPhysListClass & visobjlist
)
{
	WWASSERT(RootNode != NULL);

	if (pvs != NULL) {

		/*
		** Recursively collect objects directly into the specified list.
		*/
		VisObjCollectContextClass collection_context(frustum,*pvs,visobjlist);
		if (Scene->Is_Vis_Inverted() || !Is_Hierarchical_Vis_Culling_Enabled()) {
			collect_visible_objects_no_hvis_recursive(RootNode,collection_context);
		} else {
			collect_visible_objects_recursive(RootNode,collection_context);
		}

	} else {

		/*
		** Reset the internal collection, call the built-in frustum collection function
		*/
		Reset_Collection();
		Collect_Objects(frustum);

		/*
		** Loop over each collected object, adding it into the list
		*/
		PhysClass * obj;
		for (	obj = Get_First_Collected_Object();
				obj != NULL;
				obj = Get_Next_Collected_Object(obj))
		{
			visobjlist.Add(obj);
		}
	}
}


void DynamicAABTreeCullClass::Render_Visible_Cells(RenderInfoClass & rinfo,VisTableClass * pvs,DisplayModeType mode)
{
	AABTreeNodeClass * start = IndexedNodes[DebugIterator.Get_Current_Node_Index()];
	render_visible_cells_recursive(start,rinfo,pvs,mode);
}


void DynamicAABTreeCullClass::Assign_Vis_IDs(void)
{
	/*
	** Allocate a vis id for each node in the tree.
	*/
	int nodecount = Partition_Node_Count();
	for (int i=0; i<nodecount; i++) {
		IndexedNodes[i]->UserData = Scene->Allocate_Vis_Object_ID();
	}
}


void DynamicAABTreeCullClass::Evaluate_Non_Occluder_Visibility(VisRenderContextClass & context)
{
	WWASSERT(context.VisRasterizer != NULL);

	context.VisRasterizer->Set_Render_Mode(IDBufferClass::NON_OCCLUDER_MODE);
	evaluate_non_occluder_visibility_recursive(RootNode,context);
}


void DynamicAABTreeCullClass::Save_Static_Data(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(DYNAMICAABTREE_CHUNK_AABTREE_CLASS_DATA);
	TypedAABTreeCullSystemClass<PhysClass>::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(DYNAMICAABTREE_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,DYNAMICAABTREE_VARIABLE_MAXOBJRADIUS,MaxObjRadius);
	csave.End_Chunk();
}

void DynamicAABTreeCullClass::Load_Static_Data(ChunkLoadClass & cload)
{
	WWMEMLOG(MEM_CULLINGDATA);
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case DYNAMICAABTREE_CHUNK_AABTREE_CLASS_DATA:
				TypedAABTreeCullSystemClass<PhysClass>::Load(cload);
				break;

			case DYNAMICAABTREE_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,DYNAMICAABTREE_VARIABLE_MAXOBJRADIUS,MaxObjRadius);
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
}


void DynamicAABTreeCullClass::evaluate_non_occluder_visibility_recursive
(
	AABTreeNodeClass * node,
	VisRenderContextClass & context
)
{
	/*
	** Try to avoid this processing if possible.  If this node and all of its
	** children are already considered potentially visible, return
	*/
	if (subtree_is_visible(node,context)) {
		return;
	}

	/*
	** now, determine whether this bounding box is visible.  Note that
	** we consider it visible if the view-point is inside the box, otherwise
	** we have to render the box and scan for it.
	** Note also that I use the "un-inflated" box for this.
	*/
	AABoxClass nodebox = node->Box;
	nodebox.Extent -= Vector3(MaxObjRadius,MaxObjRadius,MaxObjRadius);

	bool box_is_visible = false;

	int vis_id = node->UserData;

	if (	(nodebox.Contains(context.Camera.Get_Position())) ||
			(context.VisTable.Get_Bit(vis_id) != 0) ||
			(context.Is_Vis_Quick_And_Dirty()))
	{

		box_is_visible = true;
		context.VisTable.Set_Bit(vis_id,true);

	} else {

		if (!context.Camera.Cull_Box(node->Box)) {

			context.Set_Vis_ID(vis_id);								//	use the node's vis-id
			context.VisRasterizer->Reset_Pixel_Counter();

			AABoxRenderObjClass * rbox = get_render_box();
			rbox->Set_Local_Center_Extent(nodebox.Center,nodebox.Extent);
			rbox->Special_Render(context);							// render the bounding volume
			REF_PTR_RELEASE(rbox);

			if (context.VisRasterizer->Get_Pixel_Counter() > 0) {
				context.VisTable.Set_Bit(vis_id,true);
				box_is_visible = true;
			}
		}
	}

	/*
	** Recursively test the child nodes.
	*/
	if (box_is_visible) {

		if (node->Front != NULL) {
			evaluate_non_occluder_visibility_recursive(node->Front,context);
		}

		if (node->Back != NULL) {
			evaluate_non_occluder_visibility_recursive(node->Back,context);
		}
	}
}

bool DynamicAABTreeCullClass::subtree_is_visible
(
	AABTreeNodeClass * node,
	VisRenderContextClass & context
)
{
	if (context.VisTable.Get_Bit(node->UserData) == false) return false;
	if (node->Front && !subtree_is_visible(node->Front,context)) return false;
	if (node->Back && !subtree_is_visible(node->Back,context)) return false;
	return true;
}

void DynamicAABTreeCullClass::set_tree_visibility
(
	AABTreeNodeClass * node,
	VisRenderContextClass & context,
	bool onoff
)
{
	if (node == NULL) return;
	context.VisTable.Set_Bit(node->UserData,onoff);
	set_tree_visibility(node->Front,context,onoff);
	set_tree_visibility(node->Back,context,onoff);
}

AABoxRenderObjClass * DynamicAABTreeCullClass::get_render_box(void)
{
	if (RenderBox == NULL) {
		RenderBox = NEW_REF(AABoxRenderObjClass,());
	}

	WWASSERT(RenderBox != NULL);
	RenderBox->Add_Ref();
	return RenderBox;
}


int DynamicAABTreeCullClass::find_insertion_node(CullableClass * obj)
{
	int node_index = 0;
	const AABoxClass & cullbox = obj->Get_Cull_Box();
	if (	(cullbox.Extent.X > MaxObjRadius) ||
			(cullbox.Extent.Y > MaxObjRadius) ||
			(cullbox.Extent.Z > MaxObjRadius) )
	{
		find_optimal_node(RootNode,cullbox,node_index);
	} else {
		find_optimal_deflated_node(RootNode,cullbox.Center,node_index);
	}
	return node_index;
}

void DynamicAABTreeCullClass::find_optimal_node
(
	AABTreeNodeClass * node,
	const AABoxClass & box,
	int & node_index
)
{
	/*
	** We want to find the smallest node in the tree that contains
	** the given bounding box.
	*/
	int input_node_index = node_index;

	if (node->Front) {
		if (CollisionMath::Overlap_Test(node->Front->Box,box) == CollisionMath::INSIDE) {
			find_optimal_node(node->Front,box,node_index);
		}
	}

	if (node->Back) {
		if (CollisionMath::Overlap_Test(node->Back->Box,box) == CollisionMath::INSIDE) {
			find_optimal_node(node->Back,box,node_index);
		}
	}

	/*
	** If we have no children or none of our children contained the object,
	** then we check if we contain the object.
	*/
	if (node_index == input_node_index) {
		if (node->Box.Extent.Quick_Length() < IndexedNodes[node_index]->Box.Extent.Quick_Length()) {
			node_index = node->Index;
		}
	}
}

void DynamicAABTreeCullClass::find_optimal_deflated_node
(
	AABTreeNodeClass * node,
	const Vector3 & center,
	int & node_index
)
{
	/*
	** We want to find the smallest "de-flated" leaf node that contains the
	** given center point.
	*/
	int input_node_index = node_index;

	if (node->Front) {
		if (deflated_box_contains_point(node->Front->Box,center)) {
			find_optimal_deflated_node(node->Front,center,node_index);
		}
	}

	if (node->Back) {
		if (deflated_box_contains_point(node->Back->Box,center)) {
			find_optimal_deflated_node(node->Back,center,node_index);
		}
	}

	/*
	** If we have no children or none of our children contained the object,
	** then we check if we contain the object.
	*/
	if (node_index == input_node_index) {
		if (node->Box.Extent.Quick_Length() < IndexedNodes[node_index]->Box.Extent.Quick_Length()) {
			node_index = node->Index;
		}
	}
}


void DynamicAABTreeCullClass::collect_visible_objects_recursive
(
	AABTreeNodeClass * node,
	VisObjCollectContextClass & context
)
{
	/*
	** If this node is not visible, stop
	** In Debug and Profile builds, we also recurse if vis was inverted (we can no-longer
	** early-exit the entire sub-tree in this case)
	*/
	if (context.PVS.Get_Bit(node->UserData) == 0) {
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
//	else if (overlap == CollisionMath::INSIDE) {
//		collect_visible_objects_recursive(node,context);
//		return;
//	}

	NODE_ACCEPTED();

	/*
	** Test any objects in this node
	*/
	if (node->Object) {

		if (context.PVS.Get_Bit(node->UserData) != 0) {

			PhysClass * obj = get_first_object(node);
			while (obj) {
				if (CollisionMath::Overlap_Test(context.Frustum,obj->Get_Cull_Box(),context.PlanesPassed) != CollisionMath::OUTSIDE) {
					context.VisObjList.Add(obj);
				}
				obj = get_next_object(obj);
			}
		}
	}

	/*
	** Recurse into the children
	*/
	if (node->Back) {
		collect_visible_objects_recursive(node->Back,context);
	}
	if (node->Front) {
		collect_visible_objects_recursive(node->Front,context);
	}
}


void DynamicAABTreeCullClass::collect_visible_objects_no_hvis_recursive
(
	AABTreeNodeClass * node,
	VisObjCollectContextClass & context
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
//	else if (overlap == CollisionMath::INSIDE) {
//		collect_visible_objects_recursive(node,context);
//		return;
//	}

	NODE_ACCEPTED();

	/*
	** Test any objects in this node
	*/
	if (node->Object) {

		if (context.PVS.Get_Bit(node->UserData) != 0) {

			PhysClass * obj = get_first_object(node);
			while (obj) {
				if (CollisionMath::Overlap_Test(context.Frustum,obj->Get_Cull_Box(),context.PlanesPassed) != CollisionMath::OUTSIDE) {
					context.VisObjList.Add(obj);
				}
				obj = get_next_object(obj);
			}
		}
	}

	/*
	** Recurse into the children
	*/
	if (node->Back) {
		collect_visible_objects_no_hvis_recursive(node->Back,context);
	}
	if (node->Front) {
		collect_visible_objects_no_hvis_recursive(node->Front,context);
	}
}


void DynamicAABTreeCullClass::render_visible_cells_recursive
(
	AABTreeNodeClass * node,
	RenderInfoClass & rinfo,
	VisTableClass * pvs,
	DisplayModeType mode
)
{
	/*
	** Cull the bounding volume of this node against the frustum.
	** If it is culled or not visible, stop descending the tree.
	*/
	int visid = node->UserData;
	if (	(CollisionMath::Overlap_Test(rinfo.Camera.Get_Frustum(),node->Box) == CollisionMath::OUTSIDE) ||
			(!Scene->Is_Vis_Inverted() && (pvs != NULL) && (pvs->Get_Bit(visid) == 0)) )
	{
		return;
	}

	/*
	** If we're in a leaf or node with one child that is visible, draw a translucent box
	*/
	int child_count = 0;
	if (node->Front != NULL) child_count++;
	if (node->Back != NULL) child_count++;

	if ((child_count <= 1) || (mode == DISPLAY_OCCUPIED)) {

		if ((pvs == NULL) || (pvs->Get_Bit(visid))) {

			if ((mode != DISPLAY_OCCUPIED) || (node->Object != NULL)) {

				// force boxes to get rendered (yuck)
				int oldmask = WW3D::Get_Collision_Box_Display_Mask();
				WW3D::Set_Collision_Box_Display_Mask(oldmask | 0x01);

				AABoxRenderObjClass * rbox = get_render_box();

				if (mode == DISPLAY_CENTERS) {
					rbox->Set_Local_Center_Extent(node->Box.Center,Vector3(0.3f,0.3f,0.3f));
				} else {
					rbox->Set_Local_Center_Extent(node->Box.Center,node->Box.Extent - Vector3(MaxObjRadius,MaxObjRadius,MaxObjRadius));		// set the box dimensions
				}

				float red,green,blue;
				if (child_count == 0) {
					green = 1.0f; //((visid % 13468) & 0xFF) / 255.0f;
					blue = green / 2.0f;
					red = green / 3.0f;
				} else if (child_count == 1) {
					blue = 1.0f; //((visid % 13468) & 0xFF) / 255.0f;
					red = blue / 2.0f;
					green = blue / 3.0f;
				} else {
					red = 1.0f; //((visid % 13468) & 0xFF) / 255.0f;
					blue = red / 2.0f;
					green = red / 3.0f;
				}
				rbox->Set_Color(Vector3(red,green,blue));

				rbox->Render(rinfo);																	// render the bounding volume
				REF_PTR_RELEASE(rbox);

				// restore the old box mask
				WW3D::Set_Collision_Box_Display_Mask(oldmask);
			}
		}
	}

	/*
	** recurse into children
	*/
	if (node->Front) {
		render_visible_cells_recursive(node->Front,rinfo,pvs,mode);
	}
	if (node->Back) {
		render_visible_cells_recursive(node->Back,rinfo,pvs,mode);
	}
}

void DynamicAABTreeCullClass::Prune_Redundant_Leaf_Nodes(VisOptimizationContextClass & context)
{
	prune_redundant_leaf_nodes_recursive(RootNode,context);
}

void DynamicAABTreeCullClass::prune_redundant_leaf_nodes_recursive
(
	AABTreeNodeClass * node,
	VisOptimizationContextClass & context
)
{
	/*
	** Recurse down to the leaves first
	*/
	if (node->Front) {
		prune_redundant_leaf_nodes_recursive(node->Front,context);
	}
	if (node->Back) {
		prune_redundant_leaf_nodes_recursive(node->Back,context);
	}

	/*
	** Try to prune our children.  They will only be removed if their
	** vis matches ours and they are a leaf node.
	*/
	prune_child(node,node->Front,context);
	prune_child(node,node->Back,context);

	context.Get_Progress_Object().Increment_Completed_Operations();
}

void DynamicAABTreeCullClass::prune_child
(
	AABTreeNodeClass * parent,
	AABTreeNodeClass * child,
	VisOptimizationContextClass & context
)
{
	/*
	** If the child node is NULL just return
	*/
	if ((parent == NULL) || (child == NULL)) {
		return;
	}

	/*
	** If the child is a leaf node and its VIS matches the parent vis, delete the
	** child, fixup the vis tables and ID's, and re-insert the child's objects into the tree.
	*/
	if ((child->Front == NULL) && (child->Back == NULL)) {

		int parent_object_id = parent->UserData;
		int child_object_id = child->UserData;

		float match_fraction = context.Compute_Object_Table_Match_Fraction(parent_object_id,child_object_id);
		WWDEBUG_SAY(("parent: %d, child: %d, match fraction: %f\n",parent_object_id,child_object_id,match_fraction));
		if (match_fraction >= context.Get_Dyn_Cell_Prune_Match_Fraction()) {

			/*
			** unlink the node from its parent so that subsequent calls back into the tree will not see it.
			*/
			if (parent->Front == child) {
				parent->Front = NULL;
			}
			if (parent->Back == child) {
				parent->Back = NULL;
			}

			/*
			** re-index the tree to remove the final dangling pointer.
			*/
			Re_Index_Nodes();

			/*
			** re-insert all objects into the tree
			** zero the bounding box of the node to remove any chance of the objects from being left
			** in the child node!
			*/
			child->Box.Extent.Set(0,0,0);
			PhysClass * obj = get_first_object(child);
			while (obj) {
				Update_Culling(obj);
				obj = get_first_object(child);
			}

			/*
			** delete the node
			*/
			delete child;

			/*
			** Tell the optimizer to combine the tables.  This is going to call back into the tree
			** so make sure it is in a consistent state...
			*/
			context.Combine_Object_Tables(parent_object_id,child_object_id);

			context.Get_Progress_Object().Increment_Dynamic_Cells_Removed();
		}
	}
}


void DynamicAABTreeCullClass::Merge_Vis_Object_IDs(uint32 id0,uint32 id1)
{
	/*
	** Each node has a vis object id.
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
	}
}

