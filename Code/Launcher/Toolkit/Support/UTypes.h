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

/******************************************************************************
*
* FILE
*     $Archive: /Commando/Code/Launcher/Toolkit/Support/UTypes.h $
*
* DESCRIPTION
*     Generic user type definitions
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Modtime: 8/18/00 10:39a $
*     $Revision: 1 $
*
******************************************************************************/

#ifndef UTYPES_H
#define UTYPES_H

//! Signed integer value
typedef int Int;

//! Unsigned integer value
typedef unsigned int UInt;

//! Signed 8bit value (-127 - 128)
typedef char Int8;

//! Unsigned 8bit value (0 - 255)
typedef unsigned char UInt8;

//! Signed 16bit value (-32767 - 32768)
typedef short Int16;

//! Unsigned 16bit value (0 - 65535)
typedef unsigned short UInt16;

//! Signed 32bit value
typedef long Int32;

//! Unsigned 32bit value
typedef unsigned long UInt32;

//! Signed character (ASCII)
typedef char Char;

//! Unsigned character (ANSI)
typedef unsigned char UChar;

//! Wide character (Unicode)
typedef wchar_t WChar;

//! 32bit floating point value
typedef float Float32;

//! 64bit floating point value
typedef double Float64;

//! Floating point value
typedef Float32 Float;

//! TriState
typedef enum {OFF = false, ON = true, PENDING = -1} TriState;

//! Empty pointer
#ifndef NULL
#define NULL (0L)
#endif

#endif // UTYPES_H
