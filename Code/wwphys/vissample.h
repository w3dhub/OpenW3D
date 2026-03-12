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
 *                     $Archive:: /Commando/Code/wwphys/vissample.h                           $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 7/10/00 5:07p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef VISSAMPLE_H
#define VISSAMPLE_H

#include "visenum.h"
#include "matrix3d.h"

// Forward declarations
class ChunkSaveClass;
class ChunkLoadClass;

/*
** VisSampleClass
** This class encapsulates the calculations to set up a vis-sample and the results
** of that sample.
*/
class VisSampleClass
{
public:
	VisSampleClass(void);
	VisSampleClass(const Matrix3D & inital_view,VisDirBitsType direction_bits);
	void					Init_Error(void);

	bool					Save(ChunkSaveClass &chunk_save) const;
	bool					Load(ChunkLoadClass &chunk_load);

	bool					Sample_Rejected(void) const;
	bool					Sample_Useless(void) const;
	bool					Direction_Enabled(VisDirType direction_index) const;
	Matrix3D				Get_Camera_Transform(VisDirType direction_index) const;
	int					Sample_Status(VisDirType direction_index) const;
	float					Backface_Fraction(VisDirType direction_index) const;
	float					Get_Biggest_Fraction(void) const;
	int					Get_Biggest_Fraction_Index(void) const;
	int					Get_Cur_Direction(void) const;
	const char *		Get_Cur_Direction_Name(void) const;

	void					Set_Cur_Direction(VisDirType direction_index);
	void					Set_Results(VisStatusType status,float fraction);

	int					Get_Bits_Changed(void)			{ return BitsChanged; }
	void					Set_Bits_Changed(int count)	{ BitsChanged = count; }

protected:

	Matrix3D				ViewTransform;			// intial view transform
	int					DirectionBits;			// which directions were enabled for this sample
	VisDirType			CurDirection;
	VisStatusType		Status[6];				// sample status for each direction
	float					BackfaceFraction[6];	// fraction of pixels that were backfacing for each direction
	int					BitsChanged;			// number of bits that changed with this sample

};





#endif
