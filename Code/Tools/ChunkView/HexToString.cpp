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
 *                 Project Name : ChunkView                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/ChunkView/HexToString.cpp              $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 9/28/99 9:48a                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "stdafx.h"
#include "HexToString.h"
#include <ctype.h>

const int BYTES_PER_LINE = 16;
const int SHORTS_PER_LINE = 8;
const int LONGS_PER_LINE = 4;


HexToStringClass::HexToStringClass(const uint8 * data,uint32 size) :
	Data(data),
	Size(size)
{
}

/*******************************************************************************************
**
** HexToStringByteClass
**
*******************************************************************************************/
	
HexToStringByteClass::HexToStringByteClass(const uint8 * data,uint32 size) :
	HexToStringClass(data,size)
{
	Reset();
}

void HexToStringByteClass::Reset(void)
{
	CurPos = Data;
}

bool HexToStringByteClass::Is_Done(void)
{
	return CurPos >= Data + Size;
}

CString HexToStringByteClass::Get_Next_Line(void)
{
	if (Is_Done()) return CString("");

	int i;
	CString line_string;
	CString tmp_string;
	const uint8 * workptr = CurPos;
	uint32 offset = (uint32)(CurPos - Data);
	int bytes_to_eat = min(BYTES_PER_LINE,Size - offset);

	// print hex dump
	line_string.Format("%08x:  ",offset);
	for (i=0; i<bytes_to_eat; i++) {
		tmp_string.Format("%02X ",*workptr++);
		line_string += tmp_string;
	}

	// print blanks at end of buffer
	for (i=0; i<BYTES_PER_LINE - bytes_to_eat; i++) 
	{
		line_string += CString("   ");
	}
	
	// spaces separate the hex from the characters
	line_string += CString("  ");
	workptr = CurPos;
	
	// print the characters
	for (i=0; i<bytes_to_eat; i++) {
		if (isalnum(*workptr)) {
			tmp_string.Format("%c",*workptr);
		} else {
			tmp_string.Format(".");
		}
		line_string += tmp_string;
		workptr++;
	}

	CurPos = workptr;
	return line_string;
}


/*******************************************************************************************
**
** HexToStringShortClass
**
*******************************************************************************************/

HexToStringShortClass::HexToStringShortClass(const uint8 * data,uint32 size) :
	HexToStringClass(data,size)
{
	// Round size down to the nearest word
	Size &= ~1;
	Reset();
}

void HexToStringShortClass::Reset(void)
{
	CurPos = (uint16*)Data;
}

bool HexToStringShortClass::Is_Done(void)
{
	uint32 offset = (uint32)((uint8*)CurPos - Data);
	return offset >= Size;
}

CString HexToStringShortClass::Get_Next_Line(void)
{
	if (Is_Done()) return CString("");

	int i;
	CString line_string;
	CString tmp_string;
	const uint16 * workptr = CurPos;
	uint32 offset = (uint32)((uint8*)CurPos - Data);
	int shorts_to_eat = min(SHORTS_PER_LINE,(Size - offset) / sizeof(uint16));  //yeah shorts_to_eat!

	// print hex dump
	line_string.Format("%08x:  ",offset);
	for (i=0; i<shorts_to_eat; i++) {
		tmp_string.Format("%04X ",*workptr++);
		line_string += tmp_string;
	}

	CurPos = workptr;
	return line_string;
}

/*******************************************************************************************
**
** HexToStringLongClass
**
*******************************************************************************************/
HexToStringLongClass::HexToStringLongClass(const uint8 * data,uint32 size) :
	HexToStringClass(data,size)
{
	// Round size down to the nearest int
	Size &= ~3;
	Reset();
}

void HexToStringLongClass::Reset(void)
{
	CurPos = (uint32*)Data;
}

bool HexToStringLongClass::Is_Done(void)
{
	uint32 offset = (uint32)((uint8*)CurPos - Data);
	return offset >= Size;
}

CString HexToStringLongClass::Get_Next_Line(void)
{
	if (Is_Done()) return CString("");

	int i;
	CString line_string;
	CString tmp_string;
	const uint32 * workptr = CurPos;
	uint32 offset = (uint32)((uint8*)CurPos - Data);
	int longs_to_eat = min(LONGS_PER_LINE,(Size - offset)/sizeof(uint32));  

	// print hex dump
	line_string.Format("%08x:  ",offset);
	for (i=0; i<longs_to_eat; i++) {
		tmp_string.Format("%08X ",*workptr++);
		line_string += tmp_string;
	}

	CurPos = workptr;
	return line_string;
}


