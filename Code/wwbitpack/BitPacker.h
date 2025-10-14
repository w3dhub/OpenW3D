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
// Filename:     bitpacker.h
// Project:      wwbitpack.lib
// Author:       Tom Spencer-Smith
// Date:         June 1998
// Description:  Minimal bit encoding
//

#ifndef BITPACKER_H
#define BITPACKER_H


#include "always.h"
#include "bittype.h"

static const int MAX_BITS = 32;

// 1400 is too big. Minimum MTU allowable on the internet is 576. IP Header is 20 bytes. UDP header is 8 bytes
// So our max packet size is 576 - 28 = 548
//static const int MAX_BUFFER_SIZE = 1400;
static const int MAX_BUFFER_SIZE = 548;

class cBitPacker
{
	public:
		//cBitPacker(UINT buffer_size);
		cBitPacker();
		virtual ~cBitPacker();

		char * Get_Data() const {return (char *) Buffer;}
		//UINT Get_Buffer_Size() const {return BufferSize;}
		UINT Get_Buffer_Size() const {return MAX_BUFFER_SIZE;}
		void Flush() {BitReadPosition = BitWritePosition;}
		bool Is_Flushed() const {return (BitReadPosition == BitWritePosition);}

		void Add_Bits(ULONG value, UINT num_bits);
		void Get_Bits(ULONG & value, UINT num_bits);

		void Set_Bit_Write_Position(UINT position);
		UINT Get_Bit_Write_Position() const {return BitWritePosition;}

	protected:
      cBitPacker& operator=(const cBitPacker& rhs);

	private:

      cBitPacker(const cBitPacker& source); // Disallow copy constructor

		//BYTE * Buffer;
		//const UINT BufferSize;
		BYTE Buffer[MAX_BUFFER_SIZE];
		UINT BitWritePosition;
		UINT BitReadPosition;
};

#endif // BITPACKER_H




		/*
		void Reset() {BitWritePosition = 0;}
		UINT Get_Compressed_Size_Bytes() const;

		void Flush() {NumBits = 0;}
		bool Is_Flushed() const {return (NumBits < 8);}

		void Set_Num_Bits(int num) {WWASSERT(num >= 0); NumBits = num;}
		int Get_Num_Bits(void) {return NumBits;}

		void Increment_Bit_Position(int num_bits);


		//inline void Advance_Bit_Position();

		//int NumBits;
		*/