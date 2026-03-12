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
 *                     $Archive:: /Commando/Code/wwphys/wwphysids.h                           $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 10/31/00 1:04p                                              $*
 *                                                                                             *
 *                    $Revision:: 16                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef WWPHYSIDS_H
#define WWPHYSIDS_H

#include "saveloadids.h"
#include "definitionclassids.h"

/*
** Persist Factory ID's for WWPHYS
** NOTE: It is important that *NONE* of these ID's are ever changed!
*/
enum
{
	// Sub-System chunk id's
	PHYSICS_CHUNKID_STATIC_DATA_SUBSYSTEM		= CHUNKID_WWPHYS_BEGIN,
	PHYSICS_CHUNKID_STATIC_OBJECTS_SUBSYSTEM,
	PHYSICS_CHUNKID_DYNAMIC_DATA_SUBSYSTEM		= CHUNKID_WWPHYS_BEGIN + 0x50,

	// Individual persist object chunk id's
	PHYSICS_CHUNKID_DECORATIONPHYS				= CHUNKID_WWPHYS_BEGIN + 0x100,
	PHYSICS_CHUNKID_HUMANPHYS,
	PHYSICS_CHUNKID_LIGHTPHYS,
	PHYSICS_CHUNKID_MOTORCYCLE,
	PHYSICS_CHUNKID_MOTORVEHICLE,
	PHYSICS_CHUNKID_PHYS3,
	PHYSICS_CHUNKID_PROJECTILE,
	PHYSICS_CHUNKID_RENDEROBJPHYS,
	PHYSICS_CHUNKID_RIGIDBODY,
	PHYSICS_CHUNKID_STATICPHYS,
	PHYSICS_CHUNKID_WHEELEDVEHICLE,
	PHYSICS_CHUNKID_STATICANIMPHYS,
	PHYSICS_CHUNKID_TIMEDDECORATIONPHYS,
	PHYSICS_CHUNKID_VEHICLEPHYS,
	PHYSICS_CHUNKID_TRACKEDVEHICLE,
	PHYSICS_CHUNKID_VTOLVEHICLE,
	PHYSICS_CHUNKID_WAYPATH,
	PHYSICS_CHUNKID_WAYPOINT,
	PHYSICS_CHUNKID_DYNAMICANIMPHYS,
	PHYSICS_CHUNKID_SHAKEABLESTATICPHYS,
	PHYSICS_CHUNKID_ACCESSIBLEPHYS,

	// Definition object chunk id's
	PHYSICS_CHUNKID_DECOPHYSDEF					= CHUNKID_WWPHYS_BEGIN + 0x500,
	PHYSICS_CHUNKID_HUMANPHYSDEF,
	PHYSICS_CHUNKID_LIGHTPHYSDEF,
	PHYSICS_CHUNKID_MOTORCYCLEDEF,
	PHYSICS_CHUNKID_MOTORVEHICLEDEF,
	PHYSICS_CHUNKID_PHYS3DEF,
	PHYSICS_CHUNKID_PROJECTILEDEF,
	PHYSICS_CHUNKID_RIGIDBODYDEF,
	PHYSICS_CHUNKID_STATICPHYSDEF,
	PHYSICS_CHUNKID_WHEELEDVEHICLEDEF,
	PHYSICS_CHUNKID_STATICANIMPHYSDEF,
	PHYSICS_CHUNKID_TIMEDDECOPHYSDEF,
	PHYSICS_CHUNKID_VEHICLEPHYSDEF,
	PHYSICS_CHUNKID_TRACKEDVEHICLEDEF,
	PHYSICS_CHUNKID_VTOLVEHICLEDEF,
	PHYSICS_CHUNKID_DYNAMICANIMPHYSDEF,
	PHYSICS_CHUNKID_SHAKEABLESTATICPHYSDEF,
	PHYSICS_CHUNKID_ACCESSIBLEPHYSDEF,

	// External persist object chunk id's
	PHYSICS_CHUNKID_DOORPHYS						= CHUNKID_WWPHYS_BEGIN + 0xA00,
	PHYSICS_CHUNKID_ELEVATORPHYS,
	PHYSICS_CHUNKID_DAMAGEABLESTATICPHYS,
	PHYSICS_CHUNKID_BUILDINGAGGREGATE,

	// External persist object def chunk id's
	PHYSICS_CHUNKID_DOORPHYSDEF					= CHUNKID_WWPHYS_BEGIN + 0xC00,
	PHYSICS_CHUNKID_ELEVATORPHYSDEF,
	PHYSICS_CHUNKID_DAMAGEABLESTATICPHYSDEF,
	PHYSICS_CHUNKID_BUILDINGAGGREGATEDEF,

};


/*
** DefinitionClass ClassID's for WWPHYS
*/
enum
{
	CLASSID_DECOPHYSDEF								= CLASSID_PHYSICS,
	CLASSID_HUMANPHYSDEF,
	CLASSID_MOTORCYCLEDEF,
	CLASSID_MOTORVEHICLEDEF,
	CLASSID_PHYS3DEF,
	CLASSID_RIGIDBODYDEF,
	CLASSID_WHEELEDVEHICLEDEF,
	CLASSID_STATICPHYSDEF,
	CLASSID_STATICANIMPHYSDEF,
	CLASSID_PROJECTILEDEF,
	CLASSID_TIMEDDECOPHYSDEF,
	CLASSID_VEHICLEPHYSDEF,
	CLASSID_TRACKEDVEHICLEDEF,
	CLASSID_VTOLVEHICLEDEF,
	CLASSID_DYNAMICANIMPHYSDEF,
	CLASSID_SHAKEABLESTATICPHYSDEF,
	CLASSID_ACCESSIBLEPHYSDEF,

	// External
	CLASSID_DOORPHYSDEF								= CLASSID_PHYSICS + 0x80,
	CLASSID_ELEVATORPHYSDEF,
	CLASSID_DAMAGEABLESTATICPHYSDEF,
	CLASSID_BUILDINGAGGREGATEDEF,
};



#endif
