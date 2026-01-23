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
 *                     $Archive:: /Commando/Code/wwphys/vistable.cpp                          $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 6/11/01 6:06p                                               $*
 *                                                                                             *
 *                    $Revision:: 24                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "vistable.h"
#include "wwphysids.h"
#include "pscene.h"
#include "chunkio.h"
#include "wwdebug.h"
#include "lzo.h"
#include "lzo1x.h"
#include "phys.h"
#include "wwmemlog.h"
#include <windows.h>

/*
** Chunk ID's used by a visibility table to save itself
*/
enum {
	VISTABLE_CHUNK_BYTECOUNT = 0x00000001,		// number of bytes in this pvs
	VISTABLE_CHUNK_BYTES,							// pvs bytes, compressed with lzhl (OBSOLETE!)
	VISTABLE_CHUNK_LZOBYTES							// pvs bytes, compressed with lzo
};


/**
** BitCounterClass 
** This class is used to accelerate some of the bit-vector comparing and counting that
** goes on in VisTableClass.  
*/
class BitCounterClass
{
public:
	BitCounterClass(void);
	
	int				Count_True_Bits(uint8 byte)							{ return TrueBits[byte]; }

protected:

	uint8				TrueBits[256];
};

BitCounterClass::BitCounterClass(void)
{
	/*
	** Initialize the table of how many "on" bits there are
	** in each number between 0 and 256
	*/
	for (int i=0; i<256; i++) {
		TrueBits[i] = 0;
		for (int bit=0; bit<8; bit++) {
			if (i & (1<<bit)) {
				TrueBits[i]++;
			}
		}
	}
}

static BitCounterClass _TheBitCounter;


/****************************************************************************************************
**
** VisTableClass Implementation
**
****************************************************************************************************/

VisTableClass::VisTableClass(unsigned bitcount,int id) :
	BitCount(bitcount),
	Buffer(NULL),
	VisSectorID(id),
	Timestamp(0)
{ 
	Alloc_Buffer(bitcount);
}

VisTableClass::VisTableClass(CompressedVisTableClass * ctable,int bitcount,int id) :
	BitCount(bitcount),
	Buffer(NULL),
	VisSectorID(id),
	Timestamp(0)
{
	WWASSERT(ctable != NULL);
	Alloc_Buffer(bitcount);
	ctable->Decompress(Get_Bytes(),Get_Byte_Count());
}

VisTableClass::VisTableClass(const VisTableClass & that) :
	BitCount(0),
	Buffer(NULL),
	VisSectorID(0),
	Timestamp(0)
{
	*this = that;
}

VisTableClass & VisTableClass::operator = (const VisTableClass & that)
{
	Alloc_Buffer(that.BitCount);
	memcpy(Buffer,that.Buffer,Get_Byte_Count());
	BitCount = that.BitCount;
	VisSectorID = that.VisSectorID;
	Timestamp = that.Timestamp;
	return *this;
}


VisTableClass::~VisTableClass(void)
{
	if (Buffer != NULL) {
		delete[] Buffer;
		Buffer = NULL;
	}
	BitCount = 0;
}

void VisTableClass::Alloc_Buffer(int bitcount)
{
	WWMEMLOG(MEM_VIS);
	if (Buffer != NULL) {
		delete[] Buffer;
		Buffer = NULL;
	}
	BitCount = bitcount;
	int count = ((bitcount + 31) / 32);
	Buffer = new uint32[count];
	memset(Buffer,0,count * sizeof(uint32));
}

uint8 * VisTableClass::Get_Bytes(void) const
{
	return (uint8*)Buffer;
}

int VisTableClass::Get_Byte_Count(void) const
{
	/*
	** In order to avoid "endian-ness" problems in the routines that
	** loop over the bytes in the buffer, I'm returning the byte count
	** up to the last longword.  The extra bits should always be zero
	** so they should not screw up anything.
	*/
	return Get_Long_Count() * 4; //(BitCount + 7) / 8;
}

uint32 * VisTableClass::Get_Longs(void) const
{
	return Buffer;
}

int VisTableClass::Get_Long_Count(void) const
{
	return (BitCount + 31) / 32;
}

void VisTableClass::Reset_All(void)
{ 
	if (Buffer != NULL) {
		memset(Buffer,0x00,Get_Byte_Count()); 
	}
}

void VisTableClass::Set_All(void)
{ 
	/*
	** Set the buffer to FF's
	*/
	if (Buffer != NULL) {
		memset((uint8*)Buffer,0xFF,Get_Byte_Count()); 
	}
	
	/*
	** Make sure the trailing bits are zero'd (just for sanity...)
	*/
	for (int i=BitCount; i<32*Get_Long_Count(); i++) {
		Buffer[i>>5] &= ~(0x80000000u >> (i & 0x01F));
	}
}

void VisTableClass::Delete_Bit(int delete_index)
{
	int i;

	/*
	** first handle the int that the deleted bit is in
	*/
	int first_long = delete_index >> 5;
	int start_bits = WWMath::Min((first_long + 1)<<5,BitCount-1);
	
	for (i=delete_index; i<start_bits; i++) {
		Set_Bit(i,(Get_Bit(i+1) != 0));
	}
	
	/*
	** handle the reset of the buffer.  There are two cases here, if the deleted
	** bit was in the last int of the buffer, we simply have to clear the trailing
	** bit.  Otherwise we need to shift the rest of the longs in the buffer right
	** by one bit and or in the MSB from the following int.
	*/
	int long_count = Get_Long_Count();
	if (first_long == long_count-1) {
		Set_Bit(BitCount-1,false);
	} else {
		for (i=first_long+1; i<long_count-1; i++) {
			Buffer[i] = (Buffer[i]<<1) | ((Buffer[i+1] & 0x80000000) >> 31);
		}
		Buffer[long_count-1] = Buffer[long_count-1]<<1;
	}
	
	/*
	** Finally, record that a bit was removed.
	*/
	BitCount--;
}

void VisTableClass::Merge(const VisTableClass & that)
{
	if (that.BitCount != BitCount) {
		return;
	}
	for (int i=0; i<Get_Long_Count(); i++) {
		Buffer[i] = Buffer[i] | that.Buffer[i];
	}
}

void VisTableClass::Invert(void)
{
	for (int i=0; i<Get_Long_Count(); i++) {
		Buffer[i] = ~Buffer[i];
	}
}

bool VisTableClass::Is_Equal_To(const VisTableClass & that)
{
	if (BitCount != that.BitCount) {
		return false;
	}
	if (memcmp((void*)Buffer,(void*)that.Buffer,Get_Byte_Count()) != 0) {
		return false;
	}
	return true;
}

int VisTableClass::Count_Differences(const VisTableClass & that)
{
	if (BitCount != that.BitCount) {
		return BitCount;
	} 

	int counter = 0;
	int byte_count = Get_Byte_Count();
	uint8 * my_bytes = Get_Bytes();
	uint8 * his_bytes = that.Get_Bytes();

	for (int i=0; i<byte_count; i++) {
		counter += _TheBitCounter.Count_True_Bits(my_bytes[i] ^ his_bytes[i]);
	}
	return counter;
}

int VisTableClass::Count_True_Bits(void)
{
	int counter = 0;
	int byte_count = Get_Byte_Count();
	uint8 * my_bytes = Get_Bytes();

	for (int i=0; i<byte_count; i++) {
		counter += _TheBitCounter.Count_True_Bits(my_bytes[i]);
	}
	return counter;
}

float VisTableClass::Match_Fraction(const VisTableClass & that)
{
	if (BitCount != that.BitCount) {
		return 0.0f;
	}
	
	/*
	** or_counter will be the number of bits that were on in at least one of the vectors
	** xor_counter will be the count of bits that were different (1 in one and zero in the other)
	*/
	int xor_counter = 0;
	int or_counter = 0;

	int byte_count = Get_Byte_Count();
	uint8 * my_bytes = Get_Bytes();
	uint8 * his_bytes = that.Get_Bytes();

	for (int i=0; i<byte_count; i++) {
		xor_counter += _TheBitCounter.Count_True_Bits(my_bytes[i] ^ his_bytes[i]);
		or_counter += _TheBitCounter.Count_True_Bits(my_bytes[i] | his_bytes[i]);
	}

	/*
	** match fraction is 1 - (different_bits / on_bits)
	*/
	if (or_counter == 0) {
		return 1.0f;
	} else {
		return 1.0f - (float)xor_counter / (float)or_counter;
	}
}


/****************************************************************************************************
**
** CompressedVisTableClass Implementation
**
****************************************************************************************************/


CompressedVisTableClass::CompressedVisTableClass(void) :
	BufferSize(0),
	Buffer(NULL)
{
}

CompressedVisTableClass::CompressedVisTableClass(VisTableClass * bits) :
	BufferSize(0),
	Buffer(NULL)
{
	WWMEMLOG(MEM_VIS);
	WWASSERT(bits != NULL);
	Compress(bits->Get_Bytes(),bits->Get_Byte_Count());
}

CompressedVisTableClass::CompressedVisTableClass(const CompressedVisTableClass &that) :
	BufferSize(0),
	Buffer(NULL)
{
	(*this) = that;
}

CompressedVisTableClass::~CompressedVisTableClass(void)
{
	if (Buffer != NULL) {
		delete[] Buffer;
	}
}

const CompressedVisTableClass &CompressedVisTableClass::operator= (const CompressedVisTableClass &that)
{
	WWMEMLOG(MEM_VIS);
	if (Buffer != NULL) {
		delete [] Buffer;
		Buffer = NULL;
	}

	BufferSize = that.BufferSize;
	Buffer = new uint8[BufferSize];
	::memcpy (Buffer, that.Buffer, sizeof(uint8)*BufferSize);
	return *this;
}

uint8 * CompressedVisTableClass::Get_Bytes(void)
{
	return Buffer;
}

int CompressedVisTableClass::Get_Byte_Count(void) const
{
	return BufferSize;
}

void CompressedVisTableClass::Load(ChunkLoadClass & cload)
{
	WWMEMLOG(MEM_VIS);
	VisTableClass * old_table = NULL;
	if (Buffer != NULL) {
		old_table = NEW_REF(VisTableClass,(this,PhysicsSceneClass::Get_Instance()->Get_Vis_Table_Size(),0));
		delete[] Buffer;
	}
	
	cload.Open_Chunk();
	if (cload.Cur_Chunk_ID() != VISTABLE_CHUNK_BYTECOUNT) {
		cload.Close_Chunk();
		return;
	}
	cload.Read(&BufferSize,sizeof(BufferSize));
	cload.Close_Chunk();

	Buffer = new uint8[BufferSize];
	
	/*
	** Load the compressed visibility bits.  At one point in the past,
	** we were using the lzhl compression scheme, if we encounter that chunk,
	** just reset vis because that compressor had bugs and the data is probably 
	** invalid
	*/
	bool load_error = false;
	while (cload.Open_Chunk()) {
		switch (cload.Cur_Chunk_ID()) {
		case VISTABLE_CHUNK_BYTES:
			cload.Read(Buffer,BufferSize);
			PhysicsSceneClass::Get_Instance()->Reset_Vis();
			load_error = true;
			break;
		case VISTABLE_CHUNK_LZOBYTES:
			WWASSERT(cload.Cur_Chunk_Length() == (uint32)BufferSize);
			cload.Read(Buffer,BufferSize);
			break;
		default:
			WWDEBUG_SAY(("Unhandled chunk ID: %d in vistable.cpp\r\n",cload.Cur_Chunk_ID()));
			load_error = true;
			break;
		}
		cload.Close_Chunk();
	}

	/*
	** if we loaded a valid vis table and we had a previous valid table, merge
	** the two together
	*/
	if ((load_error == false) && (old_table != NULL)) {
		VisTableClass * new_table = NEW_REF(VisTableClass,(this,PhysicsSceneClass::Get_Instance()->Get_Vis_Table_Size(),0));
		new_table->Merge(*old_table);
		Compress(new_table->Get_Bytes(),new_table->Get_Byte_Count());
		REF_PTR_RELEASE(new_table);
	}
	REF_PTR_RELEASE(old_table);
}

void CompressedVisTableClass::Save(ChunkSaveClass & csave)
{
	uint32 bytecount = BufferSize;
	csave.Begin_Chunk(VISTABLE_CHUNK_BYTECOUNT);
	csave.Write(&bytecount,sizeof(bytecount));
	csave.End_Chunk();

	csave.Begin_Chunk(VISTABLE_CHUNK_LZOBYTES);
	csave.Write(Buffer,bytecount);
	csave.End_Chunk();
}

void CompressedVisTableClass::Load (void* hfile)
{
	WWMEMLOG(MEM_VIS);
	/*
	** Free the buffer
	*/
	if (Buffer != NULL) {
		delete [] Buffer;
		Buffer = NULL;
		BufferSize = 0L;
	}

	if ((HANDLE)hfile != INVALID_HANDLE_VALUE) {

		/*
		** Read the buffer size
		*/
		DWORD dwbytes_read = 0;
		::ReadFile ((HANDLE)hfile, &BufferSize, sizeof (BufferSize), &dwbytes_read, NULL);

		/*
		** Read the buffer
		*/
		Buffer = new uint8[BufferSize];
		::ReadFile ((HANDLE)hfile, Buffer, sizeof (uint8) * BufferSize, &dwbytes_read, NULL);
	}
	
	return;
}

void CompressedVisTableClass::Save (void* hfile)
{
	if ((HANDLE)hfile != INVALID_HANDLE_VALUE) {

		/*
		** Write the buffer size
		*/
		DWORD dwbytes_written = 0;
		::WriteFile ((HANDLE)hfile, &BufferSize, sizeof (BufferSize), &dwbytes_written, NULL);

		/*
		** Write the buffer
		*/
		::WriteFile ((HANDLE)hfile, Buffer, sizeof (uint8) * BufferSize, &dwbytes_written, NULL);		
	}
	
	return;
}

void CompressedVisTableClass::Compress(uint8 * src_buffer,int src_size)
{
	WWMEMLOG(MEM_VIS);
	if (Buffer != NULL) {
		delete[] Buffer;
		Buffer = NULL;
	}
	
	uint8 * comp_buffer = new uint8[LZO_BUFFER_SIZE(src_size)];
	lzo_uint comp_size;
	[[maybe_unused]] int lzocode = LZOCompressor::Compress(src_buffer,src_size,comp_buffer,&comp_size);
	WWASSERT(lzocode == LZO_E_OK);

	BufferSize = comp_size;
	Buffer = new uint8[BufferSize];
	memcpy(Buffer,comp_buffer,BufferSize);

#ifdef WWDEBUG
	lzo_uint decomp_size;
	LZOCompressor::Decompress(Buffer,BufferSize,comp_buffer,&decomp_size);
	WWASSERT(decomp_size == (lzo_uint)src_size);
	WWASSERT(src_size % 4 == 0);
#endif

	delete[] comp_buffer;
}

void CompressedVisTableClass::Decompress(uint8 * decomp_buffer,[[maybe_unused]] int decomp_size)
{
	WWMEMLOG(MEM_VIS);
	lzo_uint size;
	LZOCompressor::Decompress(Buffer,BufferSize,decomp_buffer, &size);
	WWASSERT((int)size == decomp_size);
}

#if 0
void CompressedVisTableClass::Compare_Compression(void)
{
#ifdef WWDEBUG
	static int num_compressions = 0;
	static int total_size = 0;
	static int lcw_size = 0;
	static int lzo_size = 0;
	static int lcw_failures = 0;
	static int lzo_failures = 0;

	int test_size = tmp_size;
	uint8 * test_buf = tmp_buffer;

	num_compressions++;
	total_size += test_size;
	
	// Testing LCW
	uint8 * lcw_comp_buf = new uint8[test_size * 2];
	int lcw_comp_size = LCW_Comp(test_buf, lcw_comp_buf, test_size);

	uint8 * lcw_decomp_buf = new uint8[test_size * 2];
	int lcw_decomp_size = LCW_Uncomp(lcw_comp_buf, lcw_decomp_buf);

	lcw_size+=lcw_comp_size;
	if ((memcmp(test_buf,lcw_decomp_buf,test_size) != 0) || (lcw_decomp_size != test_size)) {
		lcw_failures++;
	}
	
	// Testing LZO
	uint8 * lzo_comp_buf = new uint8[test_size * 2]; //LZO_BUFFER_SIZE(test_size)];
	lzo_uint lzo_comp_size;
	LZOCompressor::Compress(test_buf,test_size,lzo_comp_buf,&lzo_comp_size);

	uint8 * lzo_decomp_buf = new uint8[test_size];
	lzo_uint lzo_decomp_size;
	LZOCompressor::Decompress(lzo_comp_buf,lzo_comp_size,lzo_decomp_buf,&lzo_decomp_size);

	lzo_size+=lzo_comp_size;
	if ((memcmp(test_buf,lzo_decomp_buf,test_size) != 0) || ((int)lzo_decomp_size != test_size)) {
		lzo_failures++;
	}

	delete[] lcw_comp_buf;
	delete[]	lcw_decomp_buf;
	delete[]	lzo_comp_buf;
	delete[]	lzo_decomp_buf;
#endif
	delete[] tmp_buffer;
}

#endif
