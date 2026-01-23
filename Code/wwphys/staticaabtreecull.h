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
 *                     $Archive:: /Commando/Code/wwphys/staticaabtreecull.h                   $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/19/01 2:41p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef STATICAABTREECULL_H
#define STATICAABTREECULL_H

#include "always.h"
#include "physaabtreecull.h"
#include "wwdebug.h"
#include "physlist.h"


class VisTableClass;
class CameraClass;
class VisRenderContextClass;
class VisSampleClass;
class ChunkLoadClass;
class ChunkSaveClass;


/*
** StaticAABTreeCullClass
** This is a derived axis-aligned bounding box tree for culling static objects.
** It adds the visibility code to the basic AAB-Tree culling system
*/
class StaticAABTreeCullClass : public PhysAABTreeCullClass
{

public:

	StaticAABTreeCullClass(PhysicsSceneClass * pscene);
	~StaticAABTreeCullClass(void);

	/*
	** StaticAABTreeCullClass over-rides the add and remove methods in order to add
	** vis-data support.
	** NOTE: the objects added should be derived from StaticPhysClass's.  This is
	** asserted internally.
	*/
	virtual void		Add_Object(PhysClass * obj,int cull_node_id = -1) override;
	virtual void		Remove_Object(PhysClass * obj) override;
	virtual void		Update_Culling(CullableClass * obj) override;

	/*
	** PhysAABTreeCullClass adds a new collect function which takes the pvs data
	** into account.  It also puts objects into one of two lists, separating
	** world-space-meshes from the other objects.
	*/
	void					Collect_Visible_Objects(		const FrustumClass & frustum,
																	VisTableClass * pvs,
																	RefPhysListClass & visobjlist,
																	RefPhysListClass & wsmeshlist		);

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

		VisObjCollectContextClass
		(
			const FrustumClass & frustum,
			VisTableClass & pvs,
			RefPhysListClass & visobjlist,
			RefPhysListClass & wsmeshlist
		) :
			Frustum(frustum),
			PVS(pvs),
			VisObjList(visobjlist),
			WSMeshList(wsmeshlist),
			PlanesPassed(0)
		{
		}
		
		const FrustumClass & Frustum;
		VisTableClass &		PVS;
		RefPhysListClass &	VisObjList;
		RefPhysListClass &	WSMeshList;
		int						PlanesPassed;
	};

	/*
	** Run-time visiblity support
	*/
	void					Collect_Visible_Objects_Recursive(AABTreeNodeClass * node,VisObjCollectContextClass & context);
	void					Collect_Visible_Objects_No_HVis_Recursive(AABTreeNodeClass * node,VisObjCollectContextClass & context); 

	int					Get_Vis_Sector_ID(const Vector3 & sample_point);
	StaticPhysClass *	Find_Vis_Tile(const Vector3 & sample_point);

	/*
	** Visibility Preprocessing support
	*/
	void					Assign_Vis_IDs(void);
	
	void					Evaluate_Occluder_Visibility(VisRenderContextClass & context,VisSampleClass & sample);
	void					Evaluate_Non_Occluder_Visibility(VisRenderContextClass & context);
		
	void					Render_Occluders(AABTreeNodeClass * node,VisRenderContextClass & context);
	void					Collect_Non_Occluders(AABTreeNodeClass * node,VisRenderContextClass & context,RefPhysListClass & non_occluder_list);

	void					Propogate_Hierarchical_Visibility(VisTableClass * pvs);
	void					Propogate_Hierarchical_Visibility_Recursive(AABTreeNodeClass * node,VisTableClass * pvs);
	bool					Is_Child_Visible(AABTreeNodeClass * node,VisTableClass * pvs);

	/*
	** Visibility Optimization support (preprocessing still)
	*/
	void					Merge_Vis_Object_IDs(uint32 id0,uint32 id1);
	void					Merge_Vis_Sector_IDs(uint32 id0,uint32 id1);

	/*
	** Making Physics Scene a friend so that some of the more ugly vis-system interfaces
	** can be accessed by it without exposing them to everyone else...
	*/
	friend class PhysicsSceneClass;
};

#endif //STATICAABTREECULL_H