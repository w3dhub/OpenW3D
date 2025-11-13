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

/****************************************************************************
*
* FILE
*     $Archive: /Commando/Code/Tests/BitPackTest/Code/TypeEncoder.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* VERSION INFO
*     $Author: Denzil_l $
*     $Revision: 2 $
*     $Modtime: 5/31/00 9:33a $
*
****************************************************************************/

#include "typeencoder.h"
#include "bitpacker.h"
#include <limits.h>
#include <stdlib.h>
#include <assert.h>

TypeEncoder::EncoderTypeEntry TypeEncoder::_mEncoderTypes[MAX_ENCODERTYPES] =
{
	{X_POSITION, -1000.0f, 1000.0f, 0.0305f, 16},
	{Y_POSITION, -1000.0f, 1000.0f, 0.0305f, 16},
	{Z_POSITION, -1000.0f, 1000.0f, 0.0305f, 16},
};


/******************************************************************************
*
* NAME
*     TypeEncoder::SetTypePrecision
*
* DESCRIPTION
*     No description provided,
*
* INPUTS
*     Type       - Type to set precision for.
*     MinExtent  - Minimum value of type
*     MaxExtent  - Maximum value of type
*     Resolution - Unit resolution
*
* RESULTS
*     NONE
*
******************************************************************************/

void TypeEncoder::SetTypePrecision(EncoderType type, float min, float max,
		float resolution)
{
	EncoderTypeEntry& entry = _mEncoderTypes[type];

	entry.MinExtent = min;
	entry.MaxExtent = max;
	entry.Resolution = resolution;
	entry.BitPrecision = CalcBitPrecision(min, max, resolution);
}


/******************************************************************************
*
* NAME
*     TypeEncoder::SetTypePrecision
*
* DESCRIPTION
*     Set the type precision based on a desired resolution
*
* INPUTS
*     Type       - Type to set precision for.
*     MinExtent  - Minimum value of type
*     MaxExtent  - Maximum value of type
*     Resolution - Unit resolution
*
* RESULTS
*     NONE
*
******************************************************************************/

void TypeEncoder::SetTypePrecision(EncoderType type, float min, float max,
		unsigned int bitPrecision)
{
	EncoderTypeEntry& entry = _mEncoderTypes[type];

	entry.MinExtent = min;
	entry.MaxExtent = max;
	entry.Resolution = CalcResolution(min, max, bitPrecision);
	entry.BitPrecision = bitPrecision;
}


/******************************************************************************
*
* NAME
*     TypeEncoder::TypeEncoder
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     Buffer - Pointer to buffer to store bitpacked data.
*     Size   - Length of buffer in bytes.
*
* RESULTS
*     NONE
*
******************************************************************************/

TypeEncoder::TypeEncoder(void* buffer, unsigned int size)
	: BitPacker(buffer, size)
{
}


/******************************************************************************
*
* NAME
*     TypeEncoder::~TypeEncoder
*
* DESCRIPTION
*     Destructor
*
* INPUTS
*     NONE
*
* RESULTS
*     NONE
*
******************************************************************************/

TypeEncoder::~TypeEncoder()
{
}


/******************************************************************************
*
* NAME
*     TypeEncoder::GetBool
*
* DESCRIPTION
*     Retrieve a boolean value (1 bit)
*
* INPUTS
*     NONE
*
* RESULTS
*     Bool - Boolean value
*
******************************************************************************/

bool TypeEncoder::GetBool(void)
{
	return (BitPacker::GetBit() == 1);
}


/******************************************************************************
*
* NAME
*     TypeEncoder::PutBool
*
* DESCRIPTION
*     Write a boolean value (1 bit)
*
* INPUTS
*     Value - Boolean value
*
* RESULTS
*     Success - True if successful; otherwise false
*
******************************************************************************/

bool TypeEncoder::PutBool(bool value)
{
	return BitPacker::PutBit((int)value);
}


/******************************************************************************
*
* NAME
*     TypeEncoder::GetInt
*
* DESCRIPTION
*     Retrieve an integer value
*
* INPUTS
*     BitPrecision - Number of bits to use to represent integer
*
* RESULTS
*     Value - Integer value
*
******************************************************************************/

int TypeEncoder::GetInt(unsigned int bitPrecision)
{
	// Get the sign
	int sign = BitPacker::GetBit();

	// Get the number
	unsigned int code;
	BitPacker::GetBits(code, bitPrecision);

	// Adjust sign
	if (sign) {
		code = -((int)code);
	}

	return (int)code;
}


/******************************************************************************
*
* NAME
*     TypeEncoder::PutInt
*
* DESCRIPTION
*     Write an integer
*
* INPUTS
*     Value        - Numerical value to write
*     BitPrecision - Number of bits to represent value.
*
* RESULTS
*     Success - True if successful; False otherwise
*
******************************************************************************/

bool TypeEncoder::PutInt(int value, unsigned int bitPrecision)
{
	BitPacker::PutBit(value < 0);
	BitPacker::PutBits((unsigned int)abs(value), bitPrecision);
	return true;
}


/******************************************************************************
*
* NAME
*     TypeEncoder::GetFloat
*
* DESCRIPTION
*     Retrieve a floating point value
*
* INPUTS
*     Min        - Mimimum extent of number
*     Max        - Maximum extent of number
*     Resolution - Unit resolution
*
* RESULTS
*     Value - Floating point value
*
******************************************************************************/

float TypeEncoder::GetFloat(float min, float max, float resolution)
{
	unsigned int bitPrecision = CalcBitPrecision(min, max, resolution);

	unsigned int code = 0;
	BitPacker::GetBits(code, bitPrecision);

	float value = (((float)code * resolution) + min);
	return value;
}


/******************************************************************************
*
* NAME
*     TypeEncoder::PutFloat
*
* DESCRIPTION
*     Write a floating point value
*
* INPUTS
*     Value      - Value to write
*     Min        - Mimimum extent of number
*     Max        - Maximum extent of number
*     Resolution - Unit resolution
*
* RESULTS
*     Success - True if successful; False otherwise
*
******************************************************************************/

bool TypeEncoder::PutFloat(float value, float min, float max, float resolution)
{
	unsigned int bitPrecision = CalcBitPrecision(min, max, resolution);

	unsigned int code = (unsigned int)((value - min) / resolution);
	BitPacker::PutBits(code, bitPrecision);
	return true;
}


/******************************************************************************
*
* NAME
*     TypeEncoder::GetType
*
* DESCRIPTION
*     Retrieve number of specified type
*
* INPUTS
*     Type - Precision type of number to retrieve
*
* RESULTS
*     Value - Number of specified type
*
******************************************************************************/

float TypeEncoder::GetType(EncoderType type)
{
	EncoderTypeEntry& entry = _mEncoderTypes[type];

	unsigned int code = 0;
	BitPacker::GetBits(code, entry.BitPrecision);

	float value = (((float)code * entry.Resolution) + entry.MinExtent);
	return value;
}


/******************************************************************************
*
* NAME
*     TypeEncoder::PutType
*
* DESCRIPTION
*     Write a number of the specified type
*
* INPUTS
*     Type  - Precision type to use to encode number
*     Value - Number to encode.
*
* RESULTS
*     Success - True if successful; False otherwise
*
******************************************************************************/

bool TypeEncoder::PutType(EncoderType type, float value)
{
	EncoderTypeEntry& entry = _mEncoderTypes[type];

	unsigned int code = (unsigned int)((value - entry.MinExtent) / entry.Resolution);
	BitPacker::PutBits(code, entry.BitPrecision);
	return true;
}


/******************************************************************************
*
* NAME
*     TypeEncoder::CalcBitPrecision
*
* DESCRIPTION
*     Calculate the minimum number of bits required to encode the value with
*     the specified resolution.
*
* INPUTS
*     Min        - Minimum extent.
*     Max        - Maximum extent
*     Resolution - Unit resolution
*
* RESULTS
*     Bits - Number of bits to encode value
*
******************************************************************************/

unsigned int TypeEncoder::CalcBitPrecision(float min, float max, float resolution)
{
	// Calculate the minimum number of bits required to encode this type with
	// the specified resolution.
	float range = (max - min);
	unsigned int units = (unsigned int)((range / resolution) + 0.5);

	unsigned int numBits = 32;
	unsigned int bitMask = (1U << 31);

	while (numBits > 0) {
		if (units & bitMask) {
			break;
		}

		numBits--;
		bitMask >>= 1;
	}

	assert(numBits >= 1);
	return numBits;
}


/******************************************************************************
*
* NAME
*     TypeEncoder::CalcResolution
*
* DESCRIPTION
*     No description provided,
*
* INPUTS
*     float min
*     float max
*     unsigned int bitPrecision
*
* RESULTS
*     float
*
******************************************************************************/

float TypeEncoder::CalcResolution(float min, float max, unsigned int bitPrecision)
{
	static unsigned int _precisionRange[32] = {
		0x00000001, 0x00000003, 0x00000007, 0x0000000F,
		0x0000001F, 0x0000003F, 0x0000007F, 0x000000FF,
		0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF,
		0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF,
		0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF,
		0x001FFFFF, 0x003FFFFF, 0x007FFFFF, 0x00FFFFFF,
		0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF,
		0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF
	};

	assert(bitPrecision >= 1);
	assert(bitPrecision <= 32);

	// Calculate the minimum resolution required to encode this type with
	// the specified bits.
	float range = (max - min);
	float resolution = (range / _precisionRange[bitPrecision]);
	return resolution;
}
