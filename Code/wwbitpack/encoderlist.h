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
// Filename:     encoderlist.h
// Project:      wwbitpack.lib
// Author:       Tom Spencer-Smith
// Date:         June 1998
// Description:
//

#ifndef ENCODERLIST_H
#define ENCODERLIST_H

#include "encodertypeentry.h"
#include "miscutil.h"
#include "wwdebug.h"


const int MAX_ENCODERTYPES = 100;

class cEncoderList
{
	public:
		static void Clear_Entries();

		static void Set_Compression_Enabled(bool flag) {IsCompressionEnabled = flag;}
		static bool Is_Compression_Enabled() {return IsCompressionEnabled;}

		static cEncoderTypeEntry & Get_Encoder_Type_Entry(int index);

#pragma auto_inline(off)
		//------------------------------------------------------------------------------------
		template<class T> static T Set_Precision(int type, T min, T max,
			T resolution = 1)
		{
			WWASSERT(type >= 0 && type < MAX_ENCODERTYPES);
			WWASSERT(max - min > -MISCUTIL_EPSILON);
			WWASSERT(resolution > MISCUTIL_EPSILON);

			EncoderTypes[type].Init(
				static_cast<double>(min),
				static_cast<double>(max),
				static_cast<double>(resolution));

			WWDEBUG_SAY(("cEncoderList::Set_Precision for type %d: %d -> %d bits\n",
				type, sizeof(T) * 8, EncoderTypes[type].Get_Bit_Precision()));

			//
			// Return maximum representation error
			//
			return static_cast<T>(resolution / 2.0f + MISCUTIL_EPSILON);
			/*
			double max_error = EncoderTypes[type].Get_Resolution() / 2.0f + MISCUTIL_EPSILON;
			if (::fabs(max_error - static_cast<T>(max_error)) < MISCUTIL_EPSILON) {
				//return static_cast<T>(max_error);
				return static_cast<T>(max_error);
			} else {
				return static_cast<T>(ceil(max_error));
			}
			/**/
		}
		//------------------------------------------------------------------------------------
		static void Set_Precision(int type, int num_bits)
		{
			WWASSERT(type >= 0 && type < MAX_ENCODERTYPES);
			WWASSERT(num_bits > 0 && num_bits <= 32);

			EncoderTypes[type].Init(num_bits);

			WWDEBUG_SAY(("cEncoderList::Set_Precision for type %d: %d bits\n",
				type, num_bits));
		}
		//------------------------------------------------------------------------------------
#pragma auto_inline(on)

	private:
		static cEncoderTypeEntry EncoderTypes[MAX_ENCODERTYPES];

		static bool IsCompressionEnabled;
};

#endif // ENCODERLIST_H
