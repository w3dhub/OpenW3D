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
 *                     $Archive:: /Commando/Code/wwphys/physgridcull.cpp                      $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 8/13/00 12:13p                                              $*
 *                                                                                             *
 *                    $Revision:: 28                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "physgridcull.h"
#include "wwdebug.h"
#include "wwprofile.h"
#include "pscene.h"
#include "physcoltest.h"
#include "physinttest.h"
#include "ww3d.h"
#include "phys.h"
#include "camera.h"
#include "boxrobj.h"
#include "vistable.h"
#include "visrendercontext.h"
#include "chunkio.h"
#include "iostruct.h"
#include "colmathinlines.h"


#define  NEW_CAST_FUNCTIONS 1


/*
** Constants
*/
const int		MAX_PHYSGRID_CELLS = 2048;
const float		MIN_PHYSGRID_CELL_DIMENSION = 60.0f;


/*
** Persist save/load system
*/
enum 
{
	PHYSGRID_CHUNK_VARIABLES				= 0x00770001,	// variables wrapper, contains micro-chunks
	PHYSGRID_CHUNK_PARENT_CLASS			= 0x00770104	// wraps the parent class's save data
};


/*******************************************************************************************************
**
** PhysGridCullClass Implementation
**
*******************************************************************************************************/

PhysGridCullClass::PhysGridCullClass(PhysicsSceneClass * scene) :
	Scene(scene)
{
}


PhysGridCullClass::~PhysGridCullClass(void)
{
}

void PhysGridCullClass::Re_Partition(const Vector3 & min,const Vector3 & max,float objdim)
{
	/*
	** I want very coarse culling for this grid so force some of the constants.  
	*/
	TerminationCellCount = MAX_PHYSGRID_CELLS;
	MinCellSize.Set(MIN_PHYSGRID_CELL_DIMENSION,MIN_PHYSGRID_CELL_DIMENSION,MIN_PHYSGRID_CELL_DIMENSION);

	/*
	** Tell the parent class to repartition
	*/
	TypedGridCullSystemClass<PhysClass>::Re_Partition(min,max,objdim);
	WWASSERT(Scene != NULL);
}

void PhysGridCullClass::Collect_Visible_Objects(const FrustumClass & frustum,VisTableClass * pvs,RefPhysListClass & visobjlist)
{
	Reset_Collection();
	Collect_Visible_Objects(frustum,pvs);

	PhysClass * obj;
	for (	obj = Get_First_Collected_Object(); 
			obj != NULL; 
			obj = Get_Next_Collected_Object(obj)) 
	{
		visobjlist.Add(obj);
	}
}

void PhysGridCullClass::Collect_Visible_Objects(const FrustumClass & frustum,VisTableClass * pvs)
{
	if (pvs == NULL) {
		Collect_Objects(frustum);
		return;
	}
	
	/*
	** Collect all objects in the frustum that are in visible grid cells.
	*/
	VolumeStruct vol;
	init_volume(frustum,&vol);
	
	if (!vol.Is_Empty()) {

		int delta_x = vol.Max[0] - vol.Min[0];		
		int i,j,k;
		int address = map_indices_to_address(vol.Min[0],vol.Min[1],vol.Min[2]);
		
		for (k=vol.Min[2]; k<vol.Max[2]; k++) {
			for (j=vol.Min[1]; j<vol.Max[1]; j++) {
				for (i=vol.Min[0]; i<vol.Max[0]; i++) {
					GRIDCULL_NODE_TRIVIALLY_ACCEPTED;
					collect_visible_objects_in_leaf(frustum,*pvs,address);
					address++;
				}
				address -= delta_x;
				address += CellCount[0];
			}
			address = map_indices_to_address(vol.Min[0],vol.Min[1],k+1);
		}
	}
	
	/*
	** Collect the objects in the no-grid-list
	*/
	collect_objects_in_leaf(frustum,NoGridList);
}
	

inline void PhysGridCullClass::collect_visible_objects_in_leaf
(
	const FrustumClass & frustum,
	VisTableClass & pvs,
	int address
)
{
	CullableClass * head = Cells[address];
	if (head != NULL) {
	
		/*
		** Add all visible objects to the collection
		*/
		GridListIterator it(head);
		
		for (; !it.Is_Done(); it.Next()) {
			PhysClass * obj = (PhysClass *)it.Peek_Obj();
			if (	(pvs.Get_Bit(obj->Get_Vis_Object_ID()) != 0) &&
					(CollisionMath::Overlap_Test(frustum,obj->Get_Cull_Box()) != CollisionMath::OUTSIDE) ) 
			{
				Add_To_Collection(obj);
			}
		}
	}
}
	

bool PhysGridCullClass::Cast_Ray(PhysRayCollisionTestClass & raytest)
{
#if NEW_CAST_FUNCTIONS

	Reset_Collection();
	
	AABoxClass bounds;
	bounds.Init(raytest.Ray);
	Collect_Objects(bounds);

	bool res = false;
	PhysClass * obj = Get_First_Collected_Object();
	PhysicsSceneClass * scene = PhysicsSceneClass::Get_Instance();
	
	while (obj) {

		if (	
			scene->Do_Groups_Collide(obj->Get_Collision_Group(),raytest.CollisionGroup) && 
			!obj->Is_Ignore_Me()	
		) 
		{
			res |= obj->Cast_Ray(raytest);
		}

		obj = Get_Next_Collected_Object(obj);
	} 
	return res;

#else 	
	
	bool res = false;

	// hierarchically cull the objects in the grid
	VolumeStruct vol;
	init_volume(raytest.Ray,&vol);
	if (!vol.Is_Empty()) {
		res |= cast_ray_recursive(raytest,vol);
	}

	// linearly cull the objects in the NoGridList
	PhysicsSceneClass * scene = PhysicsSceneClass::Get_Instance();
	for (GridListIterator it(NoGridList); !it.Is_Done(); it.Next()) {
		PhysClass * obj = (PhysClass *)it.Peek_Obj();
		
		if (	
				scene->Do_Groups_Collide(obj->Get_Collision_Group(),raytest.CollisionGroup) && 
				!obj->Is_Ignore_Me()	
			) 
		{
			res |= obj->Cast_Ray(raytest);
		}
	}
	return res;

#endif

}

bool PhysGridCullClass::cast_ray_recursive
(
	PhysRayCollisionTestClass &						raytest,
	const GridCullSystemClass::VolumeStruct &		vol
) 
{
	AABoxClass box;
	compute_box(vol,&box);

	// does the ray enter this volume?
	if (raytest.Cull(box)) {
		return false;
	}

	// if we have recursed to a leaf, linearly check the objects in this leaf
	// otherwise, divide this box and recurse.
	bool res = false;
	if (vol.Is_Leaf()) {
		
		PhysClass * head = (PhysClass*)Cells[map_indices_to_address(vol.Min[0],vol.Min[1],vol.Min[2])];
		
		for (GridListIterator it(head); !it.Is_Done(); it.Next()) {
				
			PhysClass * obj = (PhysClass*)it.Peek_Obj();

			if (	
					Scene->Do_Groups_Collide(obj->Get_Collision_Group(),raytest.CollisionGroup) && 
					!obj->Is_Ignore_Me()	
				) 
			{
				res |= obj->Cast_Ray(raytest);
			}
		}
	
		return res;
	}

	// Not in a leaf, divide this volume and continue!
	VolumeStruct vol0,vol1;
	vol.Split(vol0,vol1);

	res |= cast_ray_recursive(raytest,vol0);
	res |= cast_ray_recursive(raytest,vol1);
	
	return res;
}



bool PhysGridCullClass::Cast_AABox(PhysAABoxCollisionTestClass & boxtest)
{
#if NEW_CAST_FUNCTIONS

	Reset_Collection();
	AABoxClass bounds;
	bounds.Init_Min_Max(boxtest.SweepMin,boxtest.SweepMax);
	Collect_Objects(bounds);

	bool res = false;
	PhysClass * obj = Get_First_Collected_Object();
	PhysicsSceneClass * scene = PhysicsSceneClass::Get_Instance();

	while (obj) {

		if (	
			scene->Do_Groups_Collide(obj->Get_Collision_Group(),boxtest.CollisionGroup) && 
			!obj->Is_Ignore_Me()	
		) 
		{
			res |= obj->Cast_AABox(boxtest);
		}

		obj = Get_Next_Collected_Object(obj);
	} 
	return res;

#else 	

	bool res = false;

	// hierarchically cull the objects in the grid
	VolumeStruct vol;
	init_volume(boxtest.SweepMin,boxtest.SweepMax,&vol);
	if (!vol.Is_Empty()) {
		res |= cast_aabox_recursive(boxtest,vol);
	}

	// linearly cull the objects in the NoGridList
	for (GridListIterator it(NoGridList); !it.Is_Done(); it.Next()) {
		PhysClass * obj = (PhysClass*)it.Peek_Obj();
		
		if (	
				Scene->Do_Groups_Collide(obj->Get_Collision_Group(),boxtest.CollisionGroup) && 
				!obj->Is_Ignore_Me()	
			) 
		{
			res |= obj->Cast_AABox(boxtest);
		}
	}
	return res;

#endif
}

bool PhysGridCullClass::cast_aabox_recursive
(
	PhysAABoxCollisionTestClass &						boxtest,
	const GridCullSystemClass::VolumeStruct &		vol
)
{
	AABoxClass box;
	compute_box(vol,&box);

	// does the ray enter this volume?
	if (boxtest.Cull(box)) {
		return false;
	}

	// if we have recursed to a leaf, linearly check the objects in this leaf
	// otherwise, divide this box and recurse.
	bool res = false;
	if (vol.Is_Leaf()) {
		
		PhysClass * head = (PhysClass*)Cells[map_indices_to_address(vol.Min[0],vol.Min[1],vol.Min[2])];
		
		for (GridListIterator it(head); !it.Is_Done(); it.Next()) {
				
			PhysClass * obj = (PhysClass*)it.Peek_Obj();

			if (	
					Scene->Do_Groups_Collide(obj->Get_Collision_Group(),boxtest.CollisionGroup) && 
					!obj->Is_Ignore_Me()	
				) 
			{
				res |= obj->Cast_AABox(boxtest);
			}
		}
	
		return res;
	}

	// Not in a leaf, divide this volume and continue!
	VolumeStruct vol0,vol1;
	vol.Split(vol0,vol1);

	res |= cast_aabox_recursive(boxtest,vol0);
	res |= cast_aabox_recursive(boxtest,vol1);
	
	return res;
}

bool PhysGridCullClass::Cast_OBBox(PhysOBBoxCollisionTestClass & boxtest)
{
#if NEW_CAST_FUNCTIONS

	Reset_Collection();
	
	AABoxClass bounds;
	bounds.Init_Min_Max(boxtest.SweepMin,boxtest.SweepMax);
	Collect_Objects(bounds);

	bool res = false;
	PhysClass * obj = Get_First_Collected_Object();
	PhysicsSceneClass * scene = PhysicsSceneClass::Get_Instance();

	while (obj) {

		if (	
			scene->Do_Groups_Collide(obj->Get_Collision_Group(),boxtest.CollisionGroup) && 
			!obj->Is_Ignore_Me()	
		) 
		{
			res |= obj->Cast_OBBox(boxtest);
		}

		obj = Get_Next_Collected_Object(obj);
	} 
	return res;

#else 	

	bool res = false;

	// hierarchically cull the objects in the grid
	VolumeStruct vol;
	init_volume(boxtest.SweepMin,boxtest.SweepMax,&vol);
	if (!vol.Is_Empty()) {
		res |= cast_obbox_recursive(boxtest,vol);
	}

	// linearly cull the objects in the NoGridList
	for (GridListIterator it(NoGridList); !it.Is_Done(); it.Next()) {
		PhysClass * obj = (PhysClass*)it.Peek_Obj();
		
		if (	
				Scene->Do_Groups_Collide(obj->Get_Collision_Group(),boxtest.CollisionGroup) && 
				!obj->Is_Ignore_Me()	
			) 
		{
			res |= obj->Cast_OBBox(boxtest);
		}
	}
	return res;

#endif
}

bool PhysGridCullClass::cast_obbox_recursive
(
	PhysOBBoxCollisionTestClass &						boxtest,
	const GridCullSystemClass::VolumeStruct &		vol
)
{
	AABoxClass box;
	compute_box(vol,&box);

	// does the swept box enter this volume?
	if (boxtest.Cull(box)) {
		return false;
	}

	// if we have recursed to a leaf, linearly check the objects in this leaf
	// otherwise, divide this box and recurse.
	bool res = false;
	if (vol.Is_Leaf()) {
		
		PhysClass * head = (PhysClass*)Cells[map_indices_to_address(vol.Min[0],vol.Min[1],vol.Min[2])];
		
		for (GridListIterator it(head); !it.Is_Done(); it.Next()) {
				
			PhysClass * obj = (PhysClass*)it.Peek_Obj();

			if (	
					Scene->Do_Groups_Collide(obj->Get_Collision_Group(),boxtest.CollisionGroup) && 
					!obj->Is_Ignore_Me()	
				) 
			{
				res |= obj->Cast_OBBox(boxtest);
			}
		}
	
		return res;
	}

	// Not in a leaf, divide this volume and continue!
	VolumeStruct vol0,vol1;
	vol.Split(vol0,vol1);

	res |= cast_obbox_recursive(boxtest,vol0);
	res |= cast_obbox_recursive(boxtest,vol1);
	
	return res;
}

bool PhysGridCullClass::Intersection_Test(PhysAABoxIntersectionTestClass & boxtest)
{
	Reset_Collection();
	Collect_Objects(boxtest.Box);

	bool res = false;
	PhysClass * obj = Get_First_Collected_Object();
	while (obj) {
		if (	
				Scene->Do_Groups_Collide(obj->Get_Collision_Group(),boxtest.CollisionGroup) && 
				!obj->Is_Ignore_Me()	
			) 
		{
			res |= obj->Intersection_Test(boxtest);
		}

		obj = Get_Next_Collected_Object(obj);
	} 
	return res;
}

bool PhysGridCullClass::Intersection_Test(PhysOBBoxIntersectionTestClass & boxtest)
{
	Reset_Collection();
	Collect_Objects(boxtest.BoundingBox);

	bool res = false;
	PhysClass * obj = Get_First_Collected_Object();
	while (obj) {
		if (	
				Scene->Do_Groups_Collide(obj->Get_Collision_Group(),boxtest.CollisionGroup) && 
				!obj->Is_Ignore_Me()	
			) 
		{
			res |= obj->Intersection_Test(boxtest);
		}

		obj = Get_Next_Collected_Object(obj);
	} 
	return res;
}

bool PhysGridCullClass::Intersection_Test(PhysMeshIntersectionTestClass & meshtest)
{
	Reset_Collection();
	Collect_Objects(meshtest.BoundingBox);

	bool res = false;
	PhysClass * obj = Get_First_Collected_Object();
	while (obj) {
		if (	
				Scene->Do_Groups_Collide(obj->Get_Collision_Group(),meshtest.CollisionGroup) && 
				!obj->Is_Ignore_Me()	
			) 
		{
			res |= obj->Intersection_Test(meshtest);
		}

		obj = Get_Next_Collected_Object(obj);
	} 
	return res;
}

void PhysGridCullClass::Load_Static_Data(ChunkLoadClass & cload)
{
	while(cload.Open_Chunk()) 
	{
		switch(cload.Cur_Chunk_ID()) {
/* (gth) no variables for now...
		case PHYSGRID_CHUNK_VARIABLES:
			Load_Static_Variables(cload);
			break;
*/

		case PHYSGRID_CHUNK_PARENT_CLASS:
			TypedGridCullSystemClass<PhysClass>::Load(cload);
			break;
		}

		cload.Close_Chunk();
	}
}

void PhysGridCullClass::Load_Static_Variables(ChunkLoadClass & /* cload */)
{
/* (gth) no variables for now...
	uint32 version;
	while (cload.Open_Micro_Chunk()) {

		switch(cload.Cur_Micro_Chunk_ID()) {
		}
		
		cload.Close_Micro_Chunk();
	}
*/
}

void PhysGridCullClass::Save_Static_Data(ChunkSaveClass & csave)
{
/* (gth) no variables for now...
	csave.Begin_Chunk(PHYSGRID_CHUNK_VARIABLES);
	Save_Static_Variables(csave);
	csave.End_Chunk();
*/
	csave.Begin_Chunk(PHYSGRID_CHUNK_PARENT_CLASS);
	TypedGridCullSystemClass<PhysClass>::Save(csave);
	csave.End_Chunk();
}

void PhysGridCullClass::Save_Static_Variables(ChunkSaveClass & /* csave */)
{
/* (gth) no variables for now...
	uint32 version = PHYSGRID_CURRENT_VERSION;

	WRITE_MICRO_CHUNK(csave,PHYSGRID_VARIABLE_VERSION,version);
	WRITE_MICRO_CHUNK(csave,PHYSGRID_VARIABLE_DUMMYVISID,DummyVisId);
	WRITE_MICRO_CHUNK(csave,PHYSGRID_VARIABLE_BASEVISID,BaseVisId);
*/
}