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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/pathfindbox.h                         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/06/00 3:49p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __PATHFIND_BOX_H
#define __PATHFIND_BOX_H

#include "vector3.h"
#include "aabox.h"
#include "bittype.h"


////////////////////////////////////////////////////////////////////////////////////
//
//	PathfindBoxClass
//
//	Utility class that can store a quantized AABox in 8 bytes instead of 24 bytes.
//	It assumes a global 'block-size' where x and y values are stored in units
// of this block-size.  Z values are stored in 1/10's of a meter (which allows
// for a maximum height of 6,553 meters).
//
////////////////////////////////////////////////////////////////////////////////////
class PathfindBoxClass
{

public:

	//////////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////////
	PathfindBoxClass (void)
		:	x_pos (0),
			y_pos (0),
			z_pos (0),
			x_size (0),
			y_size (0)				{ }

	~PathfindBoxClass (void)	{ }

	//////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////
	void	Get_AABox (AABoxClass &box) const;
	void	Set_AABox (const AABoxClass &box);

	void	Set_Position (int x_pos, int y_pos, float z_pos);
	void	Set_Size (int cx, int cy);

	//////////////////////////////////////////////////////////////////////////
	//	Static methods
	//////////////////////////////////////////////////////////////////////////
	static const Vector3 &	Get_Block_Size (void);
	static void					Set_Block_Size (const Vector3 &size);

private:

	//////////////////////////////////////////////////////////////////////////
	//	Static member data
	//////////////////////////////////////////////////////////////////////////
	static Vector3 _BlockSize;

	//////////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////////
	uint16	x_pos;
	uint16	y_pos;
	uint16	z_pos;
	uint8		x_size;
	uint8		y_size;
};

//////////////////////////////////////////////////////////////////////////
//	Get_AABox
//////////////////////////////////////////////////////////////////////////
inline void
PathfindBoxClass::Get_AABox (AABoxClass &box) const
{
	box.Center.X = ((float)x_pos) * _BlockSize.X;
	box.Center.Y = ((float)y_pos) * _BlockSize.Y;
	box.Center.Z = ((float)z_pos) / 10.0F;

	box.Extent.X = ((float)x_size) * _BlockSize.X;
	box.Extent.Y = ((float)y_size) * _BlockSize.Y;
	box.Extent.Z = _BlockSize.Z;
	return ;
}

//////////////////////////////////////////////////////////////////////////
//	Set_AABox
//////////////////////////////////////////////////////////////////////////
inline void
PathfindBoxClass::Set_AABox (const AABoxClass &box)
{
	x_pos = uint16(box.Center.X / _BlockSize.X);
	y_pos = uint16(box.Center.Y / _BlockSize.Y);
	z_pos = uint16(box.Center.Z * 10);

	x_size = uint8(box.Extent.X / _BlockSize.X);
	y_size = uint8(box.Extent.Y / _BlockSize.Y);
	return ;
}

//////////////////////////////////////////////////////////////////////////
//	Set_Position
//////////////////////////////////////////////////////////////////////////
inline void
PathfindBoxClass::Set_Position (int new_x, int new_y, float new_z)
{
	x_pos = uint16(new_x);
	y_pos = uint16(new_y);
	z_pos = uint16(new_z * 10);
	return ;
}

//////////////////////////////////////////////////////////////////////////
//	Set_Size
//////////////////////////////////////////////////////////////////////////
inline void
PathfindBoxClass::Set_Size (int cx, int cy)
{
	x_size = uint8(cx);
	y_size = uint8(cy);
	return ;
}

#endif // __PATHFIND_BOX_H

