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
 *                     $Archive:: /Commando/Code/Combat/humanrecoil.h                         $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/22/00 8:09a                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef HUMANRECOIL_H
#define HUMANRECOIL_H

#include "always.h"

class RenderObjClass;
class Matrix3D;

/**
** HumanRecoilClass
** This class encapsulates all of the code to perform a programatic 'recoil' animation on
** a human skeleton in Renegade.
**
** WARNING: this code assumes that you are using the same bone structure for every model
** that you apply recoils to.  The first time it is called, it caches bone indexes for
** the named bones that it modifies.  If you call it with two totally different skeletons
** the wrong bones will be moved.  If this is needed in the future we can extend this class
** and store an instance of it per game object or something...
*/
class HumanRecoilClass
{
public:

	HumanRecoilClass(void);

	void		Capture_Bones(RenderObjClass * model);
	void		Apply_Recoil(const Matrix3D & recoil_tm,RenderObjClass * model,float scale);
	void		Release_Bones(RenderObjClass * model);

protected:

	void		Initialize(RenderObjClass * model);

	static bool IsInitted;
};



#endif //HUMANRECOIL_H

