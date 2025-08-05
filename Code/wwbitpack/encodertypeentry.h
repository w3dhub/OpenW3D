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
// Filename:     encodertypeentry.h
// Project:      wwbitpack.lib
// Author:       Tom Spencer-Smith
// Date:         June 2000
// Description:  
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef ENCODERTYPEENTRY_H
#define ENCODERTYPEENTRY_H

#include "bittype.h"

class cEncoderTypeEntry
{
	public:
		cEncoderTypeEntry();

		void Init(double min, double max, double resolution);
		void Init(int num_bits);

		bool Scale(double value, ULONG & scaled_value);
		double Unscale(ULONG u_value);
		double Clamp(double value);

		UINT	Get_Bit_Precision()	const	{return BitPrecision;}
		double	Get_Resolution()	const	{return Resolution;}

		bool Is_Valid() const;
		void Invalidate();

		bool Is_Value_In_Range(double value) const;

	private:
      cEncoderTypeEntry(const cEncoderTypeEntry& source);			// Disallow
      cEncoderTypeEntry& operator=(const cEncoderTypeEntry& rhs);	// Disallow

		void Calc_Bit_Precision(double resolution);

		double Min;
		double Max;
		double Resolution;
		UINT BitPrecision;
};

#endif // ENCODERTYPEENTRY_H
