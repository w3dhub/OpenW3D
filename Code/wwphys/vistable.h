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
 *                     $Archive:: /Commando/Code/wwphys/vistable.h                            $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 8/13/00 11:43a                                              $*
 *                                                                                             *
 *                    $Revision:: 16                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef VISTABLE_H
#define VISTABLE_H

#include "vector.h"
#include "bittype.h"
#include "refcount.h"
#include "multilist.h"
#include "wwdebug.h"

class ChunkLoadClass;
class ChunkSaveClass;
class CompressedVisTableClass;

/*
** VisTableClass
** This is a bit vector which contains a bit for each static node in the world indicating
** whether that node can be seen from the current "vis-sector"
*/
class VisTableClass : public RefCountClass, public MultiListObjectClass
{
public:
	VisTableClass(unsigned bitcount,int id);
	VisTableClass(CompressedVisTableClass * ctable,int bitcount,int id);
	VisTableClass(const VisTableClass & that);
	~VisTableClass(void);

	VisTableClass & operator = (const VisTableClass & that);

	int			Get_Bit_Count(void) const								{ return BitCount; }

	void			Reset_All(void);
	void			Set_All(void);
	int			Get_Bit(int i) const;
	void			Set_Bit(int i,bool onoff);
	void			Delete_Bit(int i);

	void			Merge(const VisTableClass & that);
	void			Invert(void);
	bool			Is_Equal_To(const VisTableClass & that);
	int			Count_Differences(const VisTableClass & that);
	int			Count_True_Bits(void);
	float			Match_Fraction(const VisTableClass & that);

	void			Set_Vis_Sector_ID(int id)								{ VisSectorID = id; }
	int			Get_Vis_Sector_ID(void)									{ return VisSectorID; }
	void			Set_Time_Stamp(int timestamp)							{ Timestamp = timestamp; }
	int			Get_Time_Stamp(void) const								{ return Timestamp; }

protected:

	void			Alloc_Buffer(int bitcount);

	int			Get_Byte_Count(void) const;
	int			Get_Long_Count(void) const;
	uint8 *		Get_Bytes(void) const;
	uint32 *		Get_Longs(void) const;

	int			BitCount;
	uint32 *		Buffer;

	int			VisSectorID;
	int			Timestamp;

	// Not implemented:
	bool operator == (const VisTableClass & that);

	friend class CompressedVisTableClass;
};

/*
** CompressedVisTableClass
** This is the form that pvs data is stored in memory when it is not being used.  It
** is basically a wrapper around an allocated array with functions to compress and
** decompress to/from a VisTableClass and functions for saving and loading.
*/
class CompressedVisTableClass
{
public:

	CompressedVisTableClass(void);
	CompressedVisTableClass(VisTableClass * bits);
	CompressedVisTableClass(const CompressedVisTableClass &that);
	~CompressedVisTableClass(void);

	const CompressedVisTableClass &operator= (const CompressedVisTableClass &that);

	void			Load(void* hfile);
	void			Save(void* hfile);

	void			Load(ChunkLoadClass & cload);
	void			Save(ChunkSaveClass & csave);

protected:

	int			Get_Byte_Count(void) const;
	uint8 *		Get_Bytes(void);

	void			Compress(uint8 * src_buffer,int src_size);
	void			Decompress(uint8 * decomp_buffer,int decomp_size);

	int			BufferSize;
	uint8 *		Buffer;

	// Not implemented:
	bool operator == (const CompressedVisTableClass & that);

	friend class VisTableClass;
};



inline int VisTableClass::Get_Bit(int i) const
{
	WWASSERT(Buffer != NULL);
	WWASSERT(i < BitCount);

	return (Buffer[i>>5] & (0x80000000u >> (i & 0x1F)));
}

inline void VisTableClass::Set_Bit(int i,bool onoff)
{
	WWASSERT(Buffer != NULL);
	WWASSERT(i < BitCount);

	if (onoff) {
		Buffer[i>>5] |= (0x80000000u >> (i & 0x01F));
	} else {
		Buffer[i>>5] &= ~(0x80000000u >> (i & 0x01F));
	}
}


#endif
