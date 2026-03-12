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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/commandoids.h                       $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 4/13/00 1:13p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	COMMANDOIDS_H
#define	COMMANDOIDS_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	SAVELOADIDS_H
	#include "saveloadids.h"
#endif

#ifndef	DEFINITIONCLASSIDS_H
	#include "definitionclassids.h"
#endif



/*
** CHUNKIDs
*/
enum {
	CHUNKID_COMBAT									= CHUNKID_COMBAT_BEGIN,

	CHUNKID_TIMER,
	CHUNKID_TIMER_GAME_OBJ,

	CHUNKID_GAME_OBJECTS_BEGIN					= CHUNKID_COMBAT_BEGIN + 0x100,

	CHUNKID_GAME_OBJECT_BULLET,

	CHUNKID_GAME_OBJECT_C4,
	CHUNKID_GAME_OBJECT_DEF_C4,

	CHUNKID_GAME_OBJECT_COMMANDO,
	CHUNKID_GAME_OBJECT_DEF_COMMANDO,

	CHUNKID_GAME_OBJECT_POWERUP,
	CHUNKID_GAME_OBJECT_DEF_POWERUP,

	CHUNKID_GAME_OBJECT_SAMSITE,
	CHUNKID_GAME_OBJECT_DEF_SAMSITE,

	CHUNKID_GAME_OBJECT_SIMPLE,
	CHUNKID_GAME_OBJECT_DEF_SIMPLE,

	XXXCHUNKID_GAME_OBJECT_SIMPLE3,
	XXXCHUNKID_GAME_OBJECT_DEF_SIMPLE3,

	CHUNKID_GAME_OBJECT_SOLDIER,
	CHUNKID_GAME_OBJECT_DEF_SOLDIER,

	CHUNKID_GAME_OBJECT_VEHICLE,

	CHUNKID_GAME_OBJECT_VISUAL,
	CHUNKID_GAME_OBJECT_TIMED_VISUAL,
	CHUNKID_GAME_OBJECT_ANIMATED_VISUAL,
	CHUNKID_GAME_OBJECT_PROJECTILE_VISUAL,
	CHUNKID_GAME_OBJECT_XXXXXXXXXXXXXXXXX,

	XXXCHUNKID_GAME_OBJECT_TANK,
	XXXCHUNKID_GAME_OBJECT_DEF_TANK,

	XXXCHUNKID_GAME_OBJECT_TURRET,
	XXXCHUNKID_GAME_OBJECT_DEF_TURRET,

	XXXCHUNKID_GAME_OBJECT_BIKE,
	XXXCHUNKID_GAME_OBJECT_DEF_BIKE,

	XXXCHUNKID_GAME_OBJECT_FLYING,
	XXXCHUNKID_GAME_OBJECT_DEF_FLYING,

	XXXCHUNKID_GAME_OBJECT_CAR,
	XXXCHUNKID_GAME_OBJECT_DEF_CAR,

	CHUNKID_SPAWNER,
	CHUNKID_SPAWNER_DEF,

	CHUNKID_GAME_OBJECT_ZONE,
	CHUNKID_GAME_OBJECT_DEF_ZONE,

	CHUNKID_GAME_OBJECT_TRANSITION,
	CHUNKID_GAME_OBJECT_DEF_TRANSITION,

	// MISC ITEMS
	CHUNKID_BACKGROUND_MGR,
	CHUNKID_WEAPON_DEF,
	CHUNKID_AMMO_DEF,

	CHUNKID_GAME_OBJECT_DEF_VEHICLE,

	CHUNKID_EXPLOSION_DEF,

	CHUNKID_GAME_OBJECT_OBSERVERS_BEGIN		= CHUNKID_COMBAT_BEGIN + 0x200,
	CHUNKID_SOLDIER_OBSERVER,
};


/*
** Game Object CLASSIDs
*/
enum {
	CLASSID_GAME_OBJECT_DEF_SOLDIER			= 	CLASSID_GAME_OBJECTS + 1,
	CLASSID_GAME_OBJECT_DEF_COMMANDO,
	CLASSID_GAME_OBJECT_DEF_POWERUP,
	CLASSID_GAME_OBJECT_DEF_SIMPLE,
	XXXCLASSID_GAME_OBJECT_DEF_SIMPLE3,
	CLASSID_GAME_OBJECT_DEF_C4,
	CLASSID_GAME_OBJECT_DEF_SAMSITE,
	XXCLASSID_GAME_OBJECT_DEF_TANK,
	XXCLASSID_GAME_OBJECT_DEF_TURRET,
	XXCLASSID_GAME_OBJECT_DEF_BIKE,
	XXCLASSID_GAME_OBJECT_DEF_FLYING,
	XXCLASSID_GAME_OBJECT_DEF_CAR,

	CLASSID_SPAWNER_DEF,

	CLASSID_GAME_OBJECT_DEF_ZONE,
	CLASSID_GAME_OBJECT_DEF_TRANSITION,

	CLASSID_GAME_OBJECT_DEF_VEHICLE,

	// MISC DEFINITIONS
};


/*
** Munitions CLASSIDs
*/
enum {
	CLASSID_DEF_WEAPON			= 	CLASSID_MUNITIONS + 1,
	CLASSID_DEF_AMMO,
	CLASSID_DEF_EXPLOSION,
};


#endif	//	COMMANDOIDS_H
