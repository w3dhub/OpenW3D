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
 *                 Project Name : Combat                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/muzzlerecoil.h                        $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/22/00 8:28a                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef MUZZLERECOIL_H
#define MUZZLERECOIL_H

#include "always.h"

class RenderObjClass;

/**
** MuzzleRecoilClass
** This class tracks the recoil state of a muzzle for an ArmedGameObj.  To set up the object,
** call Init with the bone index that you want it to control.  To trigger a recoil, call
** Start_Recoil.  To make everything work, call Update once per frame with the model pointer
** and the amount of time that has elapsed.
*/
class MuzzleRecoilClass
{
public:
	MuzzleRecoilClass(void);

	void		Init(int bone_index);
	void		Start_Recoil(float recoil_scale,float recoil_time);
	void		Update(RenderObjClass * model);

protected:

	int		BoneIndex;			// bone to move
	float		RecoilScale;		// size of the initial translation
	float		RecoilTimer;		// counts down to 0.0s
	float		OORecoilTime;		// one-over the total time for the recoil effect

};



#endif //MUZZLERECOIL_H

