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

//****************************************************************************
// Get_CPU_Clock -- Fetches the current CPU clock time.
//
//    This routine will return the internal Pentium clock accumulator. This
//    accumulator is incremented every clock tick. Since this clock value can
//    get very very large, the value returned is in 64 bits. The low half is
//    returned directly, the high half is stored in location specified.
//
// INPUT:   high  -- Reference to the high value of the 64 bit clock number.
//
// OUTPUT:  Returns with the low half of the CPU clock value.
//
// WARNINGS:   This instruction is only available on Pentium or later
// processors.
//
// HISTORY:
//   07/17/1996 JLB : Created.
//============================================================================

#ifdef __BORLANDC__

extern "C" unsigned Get_CPU_Clock ( void );

#else

unsigned Get_CPU_Clock ( void );

#endif
