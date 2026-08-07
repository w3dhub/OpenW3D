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
 *                     $Archive:: /Commando/Code/wwphys/ridermanager.h                        $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 9/09/01 2:54p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef RIDERMANAGER_H
#define RIDERMANAGER_H

#include "always.h"
#include "physlist.h"

class Matrix3D;
class RenderObjClass;

/**
** RiderManagerClass
** This object is meant to be embedded in physics objects which can have other
** physics objects ride on/in them.  It keeps track of the objects which are attached
** and provides functions for updating all of them when you move.
*/
class RiderManagerClass
{
public:

	RiderManagerClass(void);
	~RiderManagerClass(void);

	void						Link_Rider(PhysClass * obj);
	void						Unlink_Rider(PhysClass * obj);
	bool						Contains(PhysClass * obj);

	void						Move_Riders(const Matrix3D & delta,RenderObjClass * carrier_sub_obj = nullptr);

	NonRefPhysListClass * Get_Rider_List(void)	{ return &RiderList; }

protected:

	NonRefPhysListClass	RiderList;

};


#endif

