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
//    This routine will return the internal clock accumulator. This
//    accumulator is incremented approximately every clock tick.
//
// OUTPUT:  Returns the CPU clock value.
//
// HISTORY:
//   07/17/1996 JLB : Created.
//============================================================================

#pragma once

#include <cstdint>

uint64_t Get_CPU_Clock ( void );
