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
*     $Archive: /Commando/Code/Tests/BitPackTest/Code/BitPacker.h $
*
* DESCRIPTION
*     Provide variable length bit packing.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* VERSION INFO
*     $Author: Denzil_l $
*     $Revision: 2 $
*     $Modtime: 5/31/00 9:21a $
*
****************************************************************************/

#ifndef _BITPACKER_H_
#define _BITPACKER_H_

class BitPacker
	{
	public:
		BitPacker();
		BitPacker(void* buffer, unsigned int bufferSize);
		virtual ~BitPacker();

		// Set the buffer to use for read / write of bit packed data.
		void SetBuffer(void* buffer, unsigned int bufferSize);
		
		// Flush remainder bits to the stream.
		// (This must be called when finished writting)
		void Flush(void);

		// Reset the bitpacked stream.
		void Reset(void);

		// Retrieve the length of the packed stream (in bytes).
		unsigned int GetPackedSize(void);

		// Retrieve a bit from the stream,
		int GetBit(void);

		// Write a bit to the stream
		bool PutBit(int value);

		// Retrieve a series of bits from the stream (Max = 32)
		int GetBits(unsigned int& outBits, unsigned int numBits);

		// Write a series of bits to the stream (Max = 32)
		int PutBits(unsigned int bits, unsigned int numBits);

	private:
		unsigned char* mBuffer;
		unsigned int mBufferSize;
		unsigned int mBytePosition;
		unsigned int mBitMask;
		unsigned char mStore;
	};

#endif // _BITPACKER_H_
