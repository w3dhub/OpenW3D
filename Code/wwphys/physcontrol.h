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
 *                     $Archive:: /Commando/Code/wwphys/physcontrol.h                         $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 4/28/00 5:18p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PHYSCONTROL_H
#define PHYSCONTROL_H

#include "always.h"
#include "vector3.h"

class	ChunkSaveClass;
class	ChunkLoadClass;


class ChunkLoadClass;
class ChunkSaveClass;


/*

	PhysControllerClass

	This is an object which abstractly describes the control state
	for a physics object.

	PhysControllers are not persistant objects on their own but they
	do provide a save and load method so that you can embed them in
	another object if you want to.

*/


class PhysControllerClass
{
public:

	PhysControllerClass(void) : MoveVector(0,0,0), TurnLeft(0) { }

	void					Reset(void)									{ Reset_Move(); Reset_Turn(); }

	void					Set_Move_Forward(float scl)			{ MoveVector.X = scl; }
	void					Set_Move_Left(float scl)				{ MoveVector.Y = scl; }
	void					Set_Move_Up(float scl)					{ MoveVector.Z = scl; }
	void					Set_Turn_Left(float scl)				{ TurnLeft = scl; }

	float					Get_Move_Forward(void)					{ return MoveVector.X; }
	float					Get_Move_Left(void)						{ return MoveVector.Y; }
	float					Get_Move_Up(void)							{ return MoveVector.Z; }
	float					Get_Turn_Left(void)						{ return TurnLeft; }

	void					Reset_Move(void)							{ MoveVector.Set(0,0,0); }
	const Vector3	&	Get_Move_Vector(void)					{ return MoveVector; }
	void					Reset_Turn(void)							{ TurnLeft = 0.0f; }

	bool					Is_Inactive(void);

	bool					Save(ChunkSaveClass & csave);
	bool					Load(ChunkLoadClass & cload);

private:

	Vector3				MoveVector;
	float					TurnLeft;
};


inline bool PhysControllerClass::Is_Inactive(void)
{
	return ((TurnLeft == 0.0f) && (MoveVector.Length2() == 0.0f));
}

#endif
