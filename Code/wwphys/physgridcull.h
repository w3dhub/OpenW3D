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
 *                     $Archive:: /Commando/Code/wwphys/physgridcull.h                        $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 8/13/00 12:13p                                              $*
 *                                                                                             *
 *                    $Revision:: 16                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PHYSGRIDCULL_H
#define PHYSGRIDCULL_H

#include "gridcull.h"
#include "phys.h"
#include "physlist.h"
#include "wwdebug.h"

class PhysicsSceneClass;
class VisRenderContextClass;
class VisTableClass;
class AABoxRenderObjClass;

/*
** PhysGridCullClass
** This is a culling system derived from TypedGridCullSystemClass which adds collision
** detection functions.
*/
class PhysGridCullClass : public TypedGridCullSystemClass<PhysClass>
{
public:
	PhysGridCullClass(PhysicsSceneClass * scene);
	virtual ~PhysGridCullClass(void);
	
	void	Re_Partition(const Vector3 & min,const Vector3 & max,float objdim) override;
	
	bool	Cast_Ray(PhysRayCollisionTestClass & raytest);
	bool	Cast_AABox(PhysAABoxCollisionTestClass & boxtest);
	bool	Cast_OBBox(PhysOBBoxCollisionTestClass & boxtest);
	
	bool	Intersection_Test(PhysAABoxIntersectionTestClass & boxtest);
	bool	Intersection_Test(PhysOBBoxIntersectionTestClass & boxtest);
	bool	Intersection_Test(PhysMeshIntersectionTestClass & meshtest);

	void	Collect_Visible_Objects(const FrustumClass & frustum,VisTableClass * pvs,RefPhysListClass & visobjlist);	
	void	Collect_Visible_Objects(const FrustumClass & frustum,VisTableClass * pvs);

	// Persist support
	void	Load_Static_Data(ChunkLoadClass & cload);
	void	Save_Static_Data(ChunkSaveClass & csave);

protected:

	void	Save_Static_Variables(ChunkSaveClass & csave);
	void	Load_Static_Variables(ChunkLoadClass & cload);

	bool	cast_ray_recursive(PhysRayCollisionTestClass & raytest,const VolumeStruct & vol);
	bool	cast_aabox_recursive(PhysAABoxCollisionTestClass & boxtest,const VolumeStruct & vol);
	bool	cast_obbox_recursive(PhysOBBoxCollisionTestClass & boxtest,const VolumeStruct & vol);

	void	collect_visible_objects_recursive(const VolumeStruct & vol,const FrustumClass & frustum,VisTableClass & vistable,int planes_passed);
	void	collect_visible_objects_in_leaf(const FrustumClass & frustum,VisTableClass & pvs,int address);

	// pointer to the physics scene that this culling system is part of
	PhysicsSceneClass * Scene;

};


#endif // PHYSGRIDCULL_H





