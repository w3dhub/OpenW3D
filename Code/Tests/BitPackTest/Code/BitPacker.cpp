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
*     $Archive: /Commando/Code/Tests/BitPackTest/Code/BitPacker.cpp $
*
* DESCRIPTION
*     Provide bit level packing for bools, variable length intergers and
*     floats.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* VERSION INFO
*     $Author: Denzil_l $
*     $Revision: 2 $
*     $Modtime: 5/31/00 9:22a $
*
****************************************************************************/

#include "bitpacker.h"
#include "utypes.h"
#include <assert.h>

/******************************************************************************
*
* NAME
*     BitPacker::BitPacker
*
* DESCRIPTION
*     Default constructor
*
* INPUTS
*     NONE
*
* RESULTS
*     NONE
*
******************************************************************************/

BitPacker::BitPacker()
	: mBuffer(NULL),
	  mBufferSize(0),
		mBytePosition(0),
		mBitMask(0x80),
		mStore(0)
{
}


/******************************************************************************
*
* NAME
*     BitPacker::BitPacker
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     Buffer     - Pointer to buffer to store BitPacker data.
*     BufferSize - Size in bytes of buffer.
*
* RESULTS
*     NONE
*
******************************************************************************/

BitPacker::BitPacker(void* buffer, unsigned int bufferSize)
	: mBuffer(NULL),
	  mBufferSize(0),
		mBytePosition(0),
		mBitMask(0x80),
		mStore(0)
{
	SetBuffer(buffer, bufferSize);
}


/******************************************************************************
*
* NAME
*     BitPacker::~BitPacker
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

BitPacker::~BitPacker()
{
	Flush();
}


/******************************************************************************
*
* NAME
*     BitPacker::SetBuffer
*
* DESCRIPTION
*
* INPUTS
*     Buffer - Pointer to buffer to hold bitpacked data.
*     Size   - Size of buffer in bytes.
*
* RESULTS
*     NONE
*
******************************************************************************/

void BitPacker::SetBuffer(void* buffer, unsigned int bufferSize)
{
	assert(buffer != NULL);
	mBuffer = (unsigned char*)buffer;

	assert(bufferSize > 0);
	mBufferSize = bufferSize;
}


/******************************************************************************
*
* NAME
*     BitPacker::Flush
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULTS
*     NONE
*
******************************************************************************/

void BitPacker::Flush(void)
{
	if (mBitMask != 0x80) {
		mBuffer[mBytePosition] = mStore;
	}
}


/******************************************************************************
*
* NAME
*     BitPacker::Reset
*
* DESCRIPTION
*     Reset the packing stream.
*
* INPUTS
*     NONE
*
* RESULTS
*     NONE
*
******************************************************************************/

void BitPacker::Reset(void)
{
	mBytePosition = 0;
	mBitMask = 0x80;
}


/******************************************************************************
*
* NAME
*     BitPacker::GetPackedSize
*
* DESCRIPTION
*     Retrieve the number of bytes used to packed the stream.
*
* INPUTS
*     NONE
*
* RESULTS
*     unsigned int
*
******************************************************************************/

unsigned int BitPacker::GetPackedSize(void)
{
	unsigned int size = mBytePosition;

	if (mBitMask != 0x80) {
		size++;
	}

	return size;
}


/******************************************************************************
*
* NAME
*     BitPacker::GetBit
*
* DESCRIPTION
*     Retrieve a bit from the stream
*
* INPUTS
*     NONE
*
* RESULTS
*     Bit - bit value
*
******************************************************************************/

int BitPacker::GetBit(void)
{
	if (mBitMask == 0x80) {
		mStore = mBuffer[mBytePosition];
		mBytePosition++;
	}

	int value = ((mStore & mBitMask) ? 1 : 0);

	mBitMask >>= 1;

	if (mBitMask == 0) {
		mBitMask = 0x80;
	}

	return value;
}


/******************************************************************************
*
* NAME
*     BitPacker::PutBit
*
* DESCRIPTION
*     Write a bit to the stream.
*
* INPUTS
*     Bit - Bit to write
*
* RESULTS
*     Success - True if successful; false otherwise
*
******************************************************************************/

bool BitPacker::PutBit(int value)
{
	if (value) {
		mStore |= mBitMask;
	}

	mBitMask >>= 1;

	if (mBitMask == 0) {
		mBitMask = 0x80;
		mBuffer[mBytePosition] = mStore;
		mBytePosition++;
		mStore = 0;
	}

	return true;
}


/******************************************************************************
*
* NAME
*     BitPacker::GetBits
*
* DESCRIPTION
*     Retrieve bits from the stream.
*
* INPUTS
*     Bits    - On return; bits retrieved from stream.
*     NumBits - Number of bits to retrieve.
*
* RESULTS
*     BitsRead - Number of bits read.
*
******************************************************************************/

int BitPacker::GetBits(unsigned int& outBits, unsigned int numBits)
{
	outBits = 0;

	unsigned int mask = (1L << (numBits - 1));

	while (mask != 0) {
		if (mBitMask == 0x80) {
			mStore = mBuffer[mBytePosition];
			mBytePosition++;
		}

		if (mStore & mBitMask) {
			outBits |= mask;
		}

		mBitMask >>= 1;

		if (mBitMask == 0) {
			mBitMask = 0x80;
		}

		mask >>= 1;
	}

	return numBits;
}


/******************************************************************************
*
* NAME
*     BitPacker::PutBits
*
* DESCRIPTION
*     Write bits to the stream
*
* INPUTS
*     Bits    - Bits to write to stream.
*     NumBits - Number of bits to write.
*
* RESULTS
*     BitsWritten - Number of bits written.
*
******************************************************************************/

int BitPacker::PutBits(unsigned int bits, unsigned int numBits)
{
	unsigned int mask = (1L << (numBits - 1));

	while (mask != 0) {
		if (bits & mask) {
			mStore |= mBitMask;
		}

		mBitMask >>= 1;

		if (mBitMask == 0) {
			mBitMask = 0x80;
			mBuffer[mBytePosition] = mStore;
			mBytePosition++;
			mStore = 0;
		}

		mask >>= 1;
	}

	return numBits;
}
