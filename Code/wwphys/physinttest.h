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
 *                     $Archive:: /Commando/Code/wwphys/physinttest.h                         $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/14/01 1:40p                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PHYSINTTEST_H
#define PHYSINTTEST_H

#include "inttest.h"
#include "mesh.h"
#include "physlist.h"


// 
// Derived versions of the Intersection Test Classes which contain
// a list of intersected PhysObj's and the collision group.
//

///////////////////////////////////////////////////////////////////////////
// 
// PhysAABoxIntersectionTestClass
// Axis-Aligned box intersections.  Derived from the W3D class for the
// same thing.  Adds the collision group and intersected objects list.
//
///////////////////////////////////////////////////////////////////////////
class PhysAABoxIntersectionTestClass : public AABoxIntersectionTestClass
{
public:
	PhysAABoxIntersectionTestClass(const AABoxClass & box,int col_group,int col_type,NonRefPhysListClass * result_list = NULL) :
		AABoxIntersectionTestClass(box,col_type),
		CollisionGroup(col_group),
		IntersectedObjects(result_list),
		CheckStaticObjs(true),
		CheckDynamicObjs(true)
	{
	}

	void							Add_Intersected_Object(PhysClass * obj) { if (IntersectedObjects) IntersectedObjects->Add(obj); }

public:
	int							CollisionGroup;
	bool							CheckStaticObjs;
	bool							CheckDynamicObjs;

private:
	NonRefPhysListClass *	IntersectedObjects;

	// not implemented:
	PhysAABoxIntersectionTestClass(const PhysAABoxIntersectionTestClass & );
	PhysAABoxIntersectionTestClass & operator = (const PhysAABoxIntersectionTestClass & );
};

///////////////////////////////////////////////////////////////////////////
// 
// PhysOBBoxIntersectionTestClass
// Oriented box intersections.  Derived from the W3D class for the
// same thing.  Adds the collision group and intersected objects list.
//
///////////////////////////////////////////////////////////////////////////
class PhysOBBoxIntersectionTestClass : public OBBoxIntersectionTestClass
{
public:
	PhysOBBoxIntersectionTestClass(const OBBoxClass & box,int col_group,int col_type,NonRefPhysListClass * result_list = NULL) :
		OBBoxIntersectionTestClass(box,col_type),
		CollisionGroup(col_group),
		IntersectedObjects(result_list),
		CheckStaticObjs(true),
		CheckDynamicObjs(true)
	{
	}
	
	void							Add_Intersected_Object(PhysClass * obj) { if (IntersectedObjects) IntersectedObjects->Add(obj); }

public:
	int							CollisionGroup;
	bool							CheckStaticObjs;
	bool							CheckDynamicObjs;

private:
	NonRefPhysListClass *	IntersectedObjects;

	// not implemented:
	PhysOBBoxIntersectionTestClass(const PhysOBBoxIntersectionTestClass & );
	PhysOBBoxIntersectionTestClass & operator = (const PhysOBBoxIntersectionTestClass & );
};

///////////////////////////////////////////////////////////////////////////
// 
// PhysMeshIntersectionTestClass
// Mesh intersections.  Currently there is no W3D equivalent.
//
///////////////////////////////////////////////////////////////////////////
class PhysMeshIntersectionTestClass : public IntersectionTestClass
{
public:
	
	PhysMeshIntersectionTestClass(MeshClass * mesh,int col_group,int col_type,NonRefPhysListClass * result_list) :
		IntersectionTestClass(col_type),
		Mesh(NULL),
		CollisionGroup(col_group),
		IntersectedObjects(result_list),
		CheckStaticObjs(true),
		CheckDynamicObjs(true)
	{
		WWASSERT(mesh != NULL);
		REF_PTR_SET(Mesh,mesh);
		BoundingBox = Mesh->Get_Bounding_Box();
	}

	~PhysMeshIntersectionTestClass(void)
	{
		REF_PTR_RELEASE(Mesh);
	}

	bool							Cull(const Vector3 & min,const Vector3 & max);
	bool							Cull(const AABoxClass & box);
	void							Add_Intersected_Object(PhysClass * obj) { if (IntersectedObjects) IntersectedObjects->Add(obj); }

public:
	MeshClass *					Mesh;
	AABoxClass					BoundingBox;
	int							CollisionGroup;
	bool							CheckStaticObjs;
	bool							CheckDynamicObjs;

private:
	NonRefPhysListClass *	IntersectedObjects;

private:
	// not implemented:
	PhysMeshIntersectionTestClass(const PhysMeshIntersectionTestClass & );
	PhysMeshIntersectionTestClass & operator = (const PhysMeshIntersectionTestClass & );
};

inline bool PhysMeshIntersectionTestClass::Cull(const Vector3 & cull_min,const Vector3 & cull_max)
{
	Vector3 box_min;
	Vector3::Subtract(BoundingBox.Center,BoundingBox.Extent,&box_min);
	
	Vector3 box_max;
	Vector3::Add(BoundingBox.Center,BoundingBox.Extent,&box_max);

	if ((box_min.X > cull_max.X) || (box_max.X < cull_min.X)) return true;
	if ((box_min.Y > cull_max.Y) || (box_max.Y < cull_min.Y)) return true;
	if ((box_min.Z > cull_max.Z) || (box_max.Z < cull_min.Z)) return true;
	
	return false;
}

inline bool PhysMeshIntersectionTestClass::Cull(const AABoxClass & cull_box)
{
	Vector3 dc;
	Vector3 r;
	Vector3::Subtract(cull_box.Center,BoundingBox.Center,&dc);
	Vector3::Add(cull_box.Extent,BoundingBox.Extent,&r);

	if (WWMath::Fabs(dc.X) > r.X) return true;
	if (WWMath::Fabs(dc.Y) > r.Y) return true;
	if (WWMath::Fabs(dc.Z) > r.Z) return true;

	return false;
}

#endif

