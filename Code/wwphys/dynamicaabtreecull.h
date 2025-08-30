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
 *                     $Archive:: /Commando/Code/wwphys/dynamicaabtreecull.h                  $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/24/01 1:26p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef DYNAMICAABTREECULL_H
#define DYNAMICAABTREECULL_H


#include "always.h"
#include "physaabtreecull.h"
#include "physlist.h"

class VisTableClass;
class VisRenderContextClass;
class VisOptimizationContextClass;
class AABoxRenderObjClass;

/**
** DynamicAABTreeCullClass
** This is a specialized AABTree which is designed to be used with dynamic objects.  It
** contains both a uniform voxelization of the level and special "seed" boxes which
** are generated from a pathfind floodfill.  Whenever a dynamic object moves, it is 
** re-inserted into the smallest leaf node in the tree.  So this is basically an AAB-Tree
** that partitions the level paying special attention to the areas that you seed 
** it with.  All nodes in the tree are assigned a visiblity id and are tracked in the
** vis system.
**
** This AABTree also over-rides the Add_Object and Update_Culling function to behave differently.
** The tree is constructed with a set of boxes which cover all of the space in the level.
** These boxes are inflated by the MaxObjRadius parameter.  When inserting objects into the
** tree we test their center-point against the non-inflated boxes and insert the object into
** the smallest leaf node that we can find.
*/
class DynamicAABTreeCullClass : public PhysAABTreeCullClass
{
public:

	DynamicAABTreeCullClass(PhysicsSceneClass * pscene);
	~DynamicAABTreeCullClass(void);

	/*
	** DynamicAABTreeCullClass over-rides Add_Object and Update_Culling because it
	** uses custom logic for determining which cell an object should end up in.
	*/
	virtual void		Add_Object(PhysClass * obj);
	virtual void		Update_Culling(CullableClass * obj) override;

	/*
	** Objects will request the visibility ID from this system.  
	** Note that if you pass in a pointer to your previous node_id, the lookup will be faster and this 
	** variable will be modified with the updated node id.  The first time this method is called, 
	** initialize the node_id to zero.
	*/
	uint32				Get_Dynamic_Object_Vis_ID(const AABoxClass & obj_bounds,int * node_id = NULL);

	/*
	** DynamicObjCullClass adds a new re-partitioning interface to AABTreeCullSystemClass
	** This interface allows the user to supply the grid parameters and the virtual occludees
	*/
	void					Re_Partition(	DynamicVectorClass<AABoxClass> * virtual_occludees,
												const Vector3 & grid_min,
												const Vector3 & grid_max,
												const Vector3 & min_grid_cell_size,
												int max_grid_cell_count,
												float max_obj_radius	);

	/*
	** DynamicObjCullClass adds a new collect function which takes the pvs data
	** into account.  It also puts objects into one of two lists, separating
	** world-space-meshes from the other objects.
	*/
	void					Collect_Visible_Objects(		const FrustumClass & frustum,
																	VisTableClass * pvs,
																	RefPhysListClass & visobjlist	);

	/*
	** Debugging
	** Render the visible leaf cells.
	** Tree visualization, walk the tree, rendering all leaf cells of current node.
	*/
	enum DisplayModeType {
		DISPLAY_NONE = 0,
		DISPLAY_ACTUAL_BOXES,
		DISPLAY_CENTERS,
		DISPLAY_OCCUPIED,
	};
	
	void					Render_Visible_Cells(RenderInfoClass & rinfo,VisTableClass * pvs,DisplayModeType mode);
	void					Debug_Reset_Node(void)				{ DebugIterator.Reset(); }
	bool					Debug_Enter_Parent(void)			{ return DebugIterator.Enter_Parent(); }
	bool					Debug_Enter_Sibling(void)			{ return DebugIterator.Enter_Sibling(); }	
	bool					Debug_Enter_Front_Child(void)		{ return DebugIterator.Enter_Front_Child(); }
	bool					Debug_Enter_Back_Child(void)		{ return DebugIterator.Enter_Back_Child(); }

	/*
	** Visibility Preprocesing.  For this culling system, visibility of all of the nodes is tracked
	** while the objects inside the culling system are ignored because they are assumed to be
	** dynamic.  For this reason, there are no occluders in the dynamic object culling system.
	*/
	void					Assign_Vis_IDs(void);
	void					Evaluate_Non_Occluder_Visibility(VisRenderContextClass & context);
	void					Prune_Redundant_Leaf_Nodes(VisOptimizationContextClass & context);
	void					Merge_Vis_Object_IDs(uint32 id0,uint32 id1);

	/*
	** Save/Load system.
	*/
	virtual void		Save_Static_Data(ChunkSaveClass & csave);
	virtual void		Load_Static_Data(ChunkLoadClass & cload);

protected:

	/*
	** VisObjCollectContextClass - this object is passed into the recursive function that collects
	** the visible objects each frame.
	*/
	class VisObjCollectContextClass
	{
	public:

		VisObjCollectContextClass(const FrustumClass & frustum,VisTableClass & pvs,RefPhysListClass & visobjlist) :
			Frustum(frustum),
			PVS(pvs),
			VisObjList(visobjlist),
			PlanesPassed(0)
		{
		}
		
		const FrustumClass & Frustum;
		VisTableClass &		PVS;
		RefPhysListClass &	VisObjList;
		int						PlanesPassed;
	};
	
	/*
	** Internal functions
	*/
	void					collect_visible_objects_recursive(AABTreeNodeClass * node,VisObjCollectContextClass & context);
	void					collect_visible_objects_no_hvis_recursive(AABTreeNodeClass * node,VisObjCollectContextClass & context);
	void					render_visible_cells_recursive(AABTreeNodeClass * node,RenderInfoClass & rinfo,VisTableClass * pvs,DisplayModeType mode);
	void					evaluate_non_occluder_visibility_recursive(AABTreeNodeClass * node,VisRenderContextClass & context);
	bool					subtree_is_visible(AABTreeNodeClass * node,VisRenderContextClass & context);
	void					set_tree_visibility(AABTreeNodeClass * node,VisRenderContextClass & context,bool onoff);

	int					find_insertion_node(CullableClass * obj);
	void					find_optimal_node(AABTreeNodeClass * node,const AABoxClass & box,int & node_index);
	void					find_optimal_deflated_node(AABTreeNodeClass * node,const Vector3 & center,int & node_index);
	
	bool					deflated_box_contains_point(const AABoxClass & box,const Vector3 & point);
	void					prune_redundant_leaf_nodes_recursive(AABTreeNodeClass * node,VisOptimizationContextClass & context);
	void					prune_child(AABTreeNodeClass * parent,AABTreeNodeClass * child,VisOptimizationContextClass & context);

	AABoxRenderObjClass * get_render_box(void);

	/*
	** Members
	*/
	float					MaxObjRadius;		// Object "inflation" radius used when the tree was created.
	AABoxRenderObjClass * RenderBox;		// Used to render nodes when debugging
	AABTreeIterator	DebugIterator;		// Used to walk the tree while debugging.  Render_Visible.. starts from this node

	/*
	** Making Physics Scene a friend so that some of the more ugly vis-system interfaces
	** can be accessed by it without exposing them to everyone else...
	*/
	friend class PhysicsSceneClass;

};

/*
** Inlines
*/

inline bool DynamicAABTreeCullClass::deflated_box_contains_point
(
	const AABoxClass & box,
	const Vector3 & point
)
{
	if (WWMath::Fabs(point.X - box.Center.X) > box.Extent.X - MaxObjRadius) return false;
	if (WWMath::Fabs(point.Y - box.Center.Y) > box.Extent.Y - MaxObjRadius) return false;
	if (WWMath::Fabs(point.Z - box.Center.Z) > box.Extent.Z - MaxObjRadius) return false;
	return true;
}

#endif //DYNAMICAABTREECULL_H