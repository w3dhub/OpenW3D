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
 *                     $Archive:: /Commando/Code/wwphys/lightcull.h                           $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 4/02/01 12:23p                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LIGHTCULL_H
#define LIGHTCULL_H

#include "aabtreecull.h"
#include "lightphys.h"
#include "ww3d.h"


/*
** LightCullClass
** This is a derived axis-aligned bounding box tree for spatially subdividing the
** static (non-moving) lights in a level.   
*/
class StaticLightCullClass : public TypedAABTreeCullSystemClass<LightPhysClass>
{

public:

	StaticLightCullClass(void);
	~StaticLightCullClass(void);

	/*
	** StaticLightCullClass over-rides the add and remove methods in order to add
	** vis-data support.
	*/
	virtual void		Add_Object(LightPhysClass * obj);
	virtual void		Remove_Object(LightPhysClass * obj) override;
	virtual void		Update_Culling(CullableClass * obj) override;

	/*
	** Visibility.  Each static light allocates a vis-sector which is used
	** for occlusion culling when deciding which dynamic objects should be 
	** affected by the light.
	*/
	void					Assign_Vis_IDs(void);
	void					Merge_Vis_Sector_IDs(uint32 id0,uint32 id1);

	/*
	** Save-Load support.  
	** The physics scene will call this class's save function from 
	** PhysicsSceneClass::Save_Level_Static_Data. 
	*/
	void					Save_Static_Data(ChunkSaveClass & csave);
	void					Load_Static_Data(ChunkLoadClass & cload);

};


#endif //LIGHTCULL_H

