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

//
// Filename:     bitpacker.cpp
// Project:      wwbitpack.lib
// Author:       Tom Spencer-Smith
// Date:         June 1998
// Description:  Minimal bit encoding
//

#include "bitpacker.h"

#include <string.h>	// for memset

#include "wwdebug.h"

//-----------------------------------------------------------------------------
//cBitPacker::cBitPacker(UINT buffer_size) :
cBitPacker::cBitPacker() :
	//BufferSize(buffer_size),
	BitWritePosition(0),
	BitReadPosition(0)
{
	//WWASSERT(BufferSize > 0);

	//Buffer = new BYTE[BufferSize];
	//WWASSERT(Buffer != NULL);
	//memset(Buffer, 0, BufferSize);
	memset(Buffer, 0, MAX_BUFFER_SIZE);
}

//-----------------------------------------------------------------------------
cBitPacker::~cBitPacker() 
{
	//delete [] Buffer;
}

//-----------------------------------------------------------------------------
cBitPacker& cBitPacker::operator=(const cBitPacker& rhs)
{
	//WWASSERT(BufferSize == rhs.BufferSize);

	//memcpy(Buffer, rhs.Buffer, rhs.BufferSize);
	memcpy(Buffer, rhs.Buffer, MAX_BUFFER_SIZE);
	BitReadPosition		= rhs.BitReadPosition;
	BitWritePosition		= rhs.BitWritePosition;

   return * this;
}

//-----------------------------------------------------------------------------
//
// This method needs optimization
//
// 02-14-2002 Jani: Optimized the code somewhat. Note that the old code reverted
// the bit order and the new one doesn't, so the versions are not compatible.
// If you use optimized Add_Bits() you need to also use optimize Get_Bits().
//

void cBitPacker::Add_Bits(ULONG value, UINT num_bits)
{
	//
	// N.B. Presently you cannot use this class with an atomic type of more 
	// than 4 bytes, such as a double. Hopefully you would be using a float 
	// instead anyway.
	//
#if 0	// Old version
	WWASSERT(num_bits > 0 && num_bits <= MAX_BITS);

	ULONG mask = 1 << (num_bits - 1);
	while (mask > 0) {

		//WWASSERT(BitWritePosition < BufferSize * 8);
		WWASSERT(BitWritePosition < MAX_BUFFER_SIZE * 8);

		UINT byte_num = BitWritePosition / 8;
		UINT bit_offset = BitWritePosition % 8;
		bool bit_value = (value & mask) != 0;
		Buffer[byte_num] |= bit_value << bit_offset;

		BitWritePosition++;

		mask >>= 1;
	}

#else	// New faster version

	// Verify that we're not writing over buffer
	WWASSERT(num_bits > 0 && num_bits <= MAX_BITS);
	WWASSERT(BitWritePosition+num_bits <= MAX_BUFFER_SIZE * 8);

	// Fill the remaining bits of the write byte first
	UINT byte_num = BitWritePosition >> 3;
	UINT bit_offset = BitWritePosition & 0x7;
	BitWritePosition+=num_bits;		// Advance the write position

	// If write buffer is not byte aligned, write the remaining bits first
	value <<= 32-num_bits;
	if (bit_offset) {
		UINT bit_count = 8 - bit_offset;
		if (bit_count>num_bits) bit_count=num_bits;

		ULONG bit_value = value;
		value <<= bit_count;					// Remove the copied bits
		num_bits -= bit_count;
		bit_value >>= (24+bit_offset);
		Buffer[byte_num++] |= bit_value;
	}

	// Write the rest of the data as bytes
	if (num_bits>8) {
		for (unsigned a=0;a<num_bits;a+=8) {
			Buffer[byte_num++]=unsigned char(value>>24);
			value<<=8;
		}
	}
	else {
		Buffer[byte_num]=unsigned char(value>>24);
	}
#endif
}

//-----------------------------------------------------------------------------
//
// This method needs optimization
// 02-14-2002 Jani: Optimized. See Add_Bits() for notes.
//
void cBitPacker::Get_Bits(ULONG & value, UINT num_bits)
{
#if 0	// Old version
	WWASSERT(num_bits > 0 && num_bits <= MAX_BITS);

	value = 0;
	for (int bit = num_bits - 1; bit >= 0; bit--) {

		//WWASSERT(BitReadPosition < BufferSize * 8);
		WWASSERT(BitReadPosition < MAX_BUFFER_SIZE * 8);
		WWASSERT(BitReadPosition < BitWritePosition);
		UINT byte_num = BitReadPosition / 8;
		UINT bit_offset = BitReadPosition % 8;
		bool b = (Buffer[byte_num] & (1 << bit_offset)) != 0;

		value += (b << bit);	

		BitReadPosition++;
	}
#else // New faster version

	// Verify that we're not reading over buffer or write pointer
	WWASSERT(num_bits > 0 && num_bits <= MAX_BITS);
	WWASSERT(BitReadPosition+num_bits <= MAX_BUFFER_SIZE * 8);
	WWASSERT(BitReadPosition+num_bits <= BitWritePosition);

	UINT read_len=num_bits;
	UINT byte_num = BitReadPosition / 8;
	UINT bit_offset = BitReadPosition % 8;
	BitReadPosition += num_bits;

	UINT bit_count = 8 - bit_offset;
	if (bit_count>num_bits) bit_count=num_bits;
	value = (ULONG(Buffer[byte_num++]) << (bit_offset+24));
	num_bits-=bit_count;

	int shift;
	for (shift=24-bit_count;shift>0;shift-=8,num_bits-=8) value|=unsigned(Buffer[byte_num++]) << shift;
	if (num_bits>0) value|=Buffer[byte_num++]>>(-shift);

	value >>= 32-read_len;
#endif
}

//-----------------------------------------------------------------------------
//
// This method is only for use by a packet class when data is received.
//

void cBitPacker::Set_Bit_Write_Position(UINT position)
{
	//WWASSERT(position <= BufferSize * 8);
	WWASSERT(position <= MAX_BUFFER_SIZE * 8);
	BitWritePosition = position;
}










/*
//-----------------------------------------------------------------------------
void cBitPacker::Increment_Bit_Position(int num_bits)
{
	WWASSERT(num_bits >= 0);

	for (int i = 0; i < num_bits; i++) {
		Advance_Bit_Position();
		NumBits++;
	}
}

//-----------------------------------------------------------------------------
UINT cBitPacker::Get_Compressed_Size_Bytes() const 
{
	return (int) ceil(BitWritePosition / 8.0f);
}

//-----------------------------------------------------------------------------
inline void cBitPacker::Advance_Bit_Position()
{
	BitWritePosition++;
	
	//
	// If the following assert hits then our buffer is not large enough.
	// We can advance BitWritePosition one bit past the end of the buffer, but
	// we cannot write there.
	//
	//WWASSERT(BitWritePosition < BufferSize * 8);
	WWASSERT(BitWritePosition <= BufferSize * 8);
}

*/


