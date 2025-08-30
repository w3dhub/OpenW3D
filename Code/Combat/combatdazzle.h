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
 *                 Project Name : Renegade                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/combatdazzle.h                        $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/13/01 9:05a                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef COMBATDAZZLE_H
#define COMBATDAZZLE_H

#include "always.h"
#include "dazzle.h"

/**
** CombatDazzleClass - this class handles the visibility callback for the combat scene.
** This handler should be installed while the background scene and the game scene
** are rendered and it determines visibility by asking the game scene to cast a ray
** from the camera to the dazzle position.
*/
class CombatDazzleClass : public DazzleVisibilityClass
{
	float Compute_Dazzle_Visibility(	RenderInfoClass & rinfo,
												DazzleRenderObjClass * dazzle,
												const Vector3 & point) const override;
};

extern CombatDazzleClass _TheCombatDazzleHandler;


#endif

