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
 *                     $Archive:: /Commando/Code/Combat/combatsound.h                         $*
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 11/29/00 2:07p                                              $*
 *                                                                                             *
 *                    $Revision:: 16                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	COMBATSOUND_H
#define	COMBATSOUND_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	VECTOR3_H
	#include "vector3.h"
#endif

class		ScriptableGameObj;

/*
**
*/
typedef enum {
	SOUND_TYPE_NONE,
	SOUND_TYPE_OLD_WEAPON,
	SOUND_TYPE_FOOTSTEPS,
	SOUND_TYPE_VEHICLE,
	SOUND_TYPE_GUNSHOT,
	SOUND_TYPE_BULLET_HIT,
	NUM_DEFINED_SOUND_TYPES,

	SOUND_TYPE_DESIGNER01	= 991,
	SOUND_TYPE_DESIGNER02,
	SOUND_TYPE_DESIGNER03,
	SOUND_TYPE_DESIGNER04,
	SOUND_TYPE_DESIGNER05,
	SOUND_TYPE_DESIGNER06,
	SOUND_TYPE_DESIGNER07,
	SOUND_TYPE_DESIGNER08,
	SOUND_TYPE_DESIGNER09,

} CombatSoundType;

/*
**
*/
class CombatSound {
public:
	Vector3				Position;
	CombatSoundType	Type;
	ScriptableGameObj * Creator;
};

/*
**
*/
class	CombatSoundManager {
public:
	static	void	Init( void );
	static	void	Shutdown( void );
};

#endif	// COMBATSOUND_H
