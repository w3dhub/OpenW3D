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
 *                     $Archive:: /Commando/Code/wwphys/pscene_decal.cpp                      $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/29/01 11:37a                                              $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "pscene.h"
#include "colmathaabox.h"
#include "assetmgr.h"
#include "vertmaterial.h"
#include "camera.h"
#include "physdecalsys.h"
#include "phys.h"




/*
** PhysicsSceneClass Decal Methods
*/

void PhysicsSceneClass::Allocate_Decal_Resources(void)
{
	WWASSERT(DecalSystem == NULL);
	DecalSystem = new PhysDecalSysClass(this);
}

void PhysicsSceneClass::Release_Decal_Resources(void)
{
	if (DecalSystem != NULL) {
		delete DecalSystem;
	}
}

int PhysicsSceneClass::Create_Decal
(
	const Matrix3D &	tm,
	const char *		texture_name,
	float					radius,
	bool					is_permanent,
	bool					apply_to_translucent_meshes,
	PhysClass *			only_this_obj
)
{
	WWASSERT(DecalSystem != NULL);
	return DecalSystem->Create_Decal(tm,texture_name,radius,is_permanent,apply_to_translucent_meshes,only_this_obj);
}

bool PhysicsSceneClass::Remove_Decal(uint32 id)
{
	WWASSERT(DecalSystem != NULL);
	return DecalSystem->Remove_Decal(id);
}


