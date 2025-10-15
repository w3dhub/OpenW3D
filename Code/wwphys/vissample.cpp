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
 *                     $Archive:: /Commando/Code/wwphys/vissample.cpp                         $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 7/16/00 1:19p                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "vissample.h"
#include "wwdebug.h"
#include "chunkio.h"
#include <memory.h>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////
//
//	Constants
//
//////////////////////////////////////////////////////////////////////////////
typedef struct
{
	Matrix3D			transform;
	float				backface_fraction[6];
	VisStatusType	status[6];
	int				direction_bits;
	VisDirType		current_direction;
	unsigned long	reserved[4];
} PERSISTENT_VIS_SAMPLE_INFO;


VisSampleClass::VisSampleClass(void)
	:	DirectionBits (0),
		ViewTransform (1),
		CurDirection (VIS_FORWARD),
		BitsChanged(0)
{
	for (int i=0; i<VIS_DIRECTIONS; i++) {
		Status[i] = VIS_STATUS_NOT_TAKEN;
		BackfaceFraction[i] = 0.0f;
	}
	return ;
}


VisSampleClass::VisSampleClass(const Matrix3D & viewtm,VisDirBitsType direction_bits)
{
	ViewTransform = viewtm;
	DirectionBits = direction_bits;
	CurDirection = VIS_FORWARD;
	BitsChanged = 0;

	for (int i=0; i<VIS_DIRECTIONS; i++) {
		Status[i] = VIS_STATUS_NOT_TAKEN;
		BackfaceFraction[i] = 0.0f;
	}
}

void VisSampleClass::Init_Error(void)
{
	for (int i=0; i<VIS_DIRECTIONS; i++) {
		Status[i] = VIS_STATUS_NOT_TAKEN;
		BackfaceFraction[i] = 1.0f;
	}
}

bool VisSampleClass::Sample_Rejected(void) const
{
	for (int i=0; i<VIS_DIRECTIONS; i++) {
		if ((Status[i] == VIS_STATUS_BACKFACE_OVERFLOW) || (Status[i] == VIS_STATUS_ERROR)) {
			return true;
		}
	}
	return false;	
}

bool VisSampleClass::Sample_Useless(void) const
{
#if 1
	return Sample_Rejected();
#else
	// This returns true if this sample is useless, we 
	// obviously generated a point outside of where the player
	// should be allowed to be.  
	for (int i=0; i<VIS_DIRECTIONS; i++) {
		if (BackfaceFraction[i] > 0.5f) {
			return true;
		}
	}
	return false;	
#endif
}

bool VisSampleClass::Direction_Enabled(VisDirType direction_index) const
{
	WWASSERT(direction_index >= 0);
	WWASSERT(direction_index < VIS_DIRECTIONS);
	int mask = (1 << direction_index);
	return ((DirectionBits & mask) == mask);
}

Matrix3D VisSampleClass::Get_Camera_Transform(VisDirType direction_index) const
{
	WWASSERT(direction_index >= 0);
	WWASSERT(direction_index < VIS_DIRECTIONS);
	Matrix3D view = ViewTransform;							// vis forward

	switch(direction_index) {
	case VIS_FORWARD:
		break;
	case VIS_LEFT:
		view.Rotate_Y(DEG_TO_RADF(90.0f));
		break;
	case VIS_BACKWARDS:
		view.Rotate_Y(DEG_TO_RADF(180.0f));
		break;
	case VIS_RIGHT:
		view.Rotate_Y(DEG_TO_RADF(-90.0f));
		break;
	case VIS_UP:
		view.Rotate_X(DEG_TO_RADF(90.0f));
		break;
	case VIS_DOWN:
		view.Rotate_X(DEG_TO_RADF(-90.0f));
		break;
	}

	if (!(DirectionBits & VIS_DONT_RECENTER)) {
		view.Translate(Vector3(0,0,VIS_NEAR_CLIP));
	}

	return view;
}

int VisSampleClass::Sample_Status(VisDirType direction_index) const
{
	WWASSERT(direction_index >= 0);
	WWASSERT(direction_index < VIS_DIRECTIONS);
	return Status[direction_index];
}

float VisSampleClass::Backface_Fraction(VisDirType direction_index) const
{
	WWASSERT(direction_index >= 0);
	WWASSERT(direction_index < VIS_DIRECTIONS);
	return BackfaceFraction[direction_index];
}

	
void VisSampleClass::Set_Cur_Direction(VisDirType direction_index)
{
	WWASSERT(direction_index >= 0);
	WWASSERT(direction_index < VIS_DIRECTIONS);
	CurDirection = direction_index;
}

void VisSampleClass::Set_Results(VisStatusType status,float fraction)
{
	WWASSERT(CurDirection >= 0);
	WWASSERT(CurDirection < VIS_DIRECTIONS);

	Status[CurDirection] = status;
	BackfaceFraction[CurDirection] = fraction;
}

const char * VisSampleClass::Get_Cur_Direction_Name(void) const
{
	WWASSERT(CurDirection >= 0);
	WWASSERT(CurDirection < VIS_DIRECTIONS);

	static const char * _direction_names[] = 
	{
		"F","L","B","R","U","D"
	};

	return _direction_names[CurDirection];
}

float VisSampleClass::Get_Biggest_Fraction(void) const
{
	float biggest_fraction = BackfaceFraction[0];
	for (int direction = 1; direction < VIS_DIRECTIONS; direction ++) {
		biggest_fraction = std::max (biggest_fraction, BackfaceFraction[direction]);
	}

	return biggest_fraction;
}

int VisSampleClass::Get_Biggest_Fraction_Index(void) const
{
	int index = 0;
	float biggest_fraction = BackfaceFraction[0];
	for (int direction = 1; direction < VIS_DIRECTIONS; direction ++) {
		if (BackfaceFraction[direction] > biggest_fraction) {
			index = direction;
			biggest_fraction = BackfaceFraction[direction];
		}
	}

	return index;
}

int VisSampleClass::Get_Cur_Direction(void) const
{
	return CurDirection;
}

bool VisSampleClass::Save(ChunkSaveClass &chunk_save) const
{
	bool retval = false;

	PERSISTENT_VIS_SAMPLE_INFO info;
	::memset (&info, 0, sizeof (info));

	//
	//	Copy the sample information into the object
	//
	info.transform				= ViewTransform;
	info.direction_bits		= DirectionBits;
	info.current_direction	= CurDirection;
	
	for (int index = 0; index < VIS_DIRECTIONS; index ++) {
		info.status[index]				= Status[index];
		info.backface_fraction[index]	= BackfaceFraction[index];
	}

	//
	//	Write the sample information to the chunk
	//
	retval = (chunk_save.Write (&info, sizeof (info)) == sizeof (info));
	return retval;
}

bool VisSampleClass::Load(ChunkLoadClass &chunk_load)
{
	bool retval = false;

	//
	//	Read the sample information from the chunk
	//
	PERSISTENT_VIS_SAMPLE_INFO info;
	retval = (chunk_load.Read (&info, sizeof (info)) == sizeof (info));
	
	if (retval) {
		
		//
		//	Copy the sample information into the object
		//
		ViewTransform	= info.transform;
		DirectionBits	= info.direction_bits;
		CurDirection	= info.current_direction;
		
		for (int index = 0; index < VIS_DIRECTIONS; index ++) {
			Status[index]				= info.status[index];
			BackfaceFraction[index] = info.backface_fraction[index];
		}
	}

	return retval;
}

