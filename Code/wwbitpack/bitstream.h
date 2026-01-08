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
 *                 Project Name : wwbitpack                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwbitpack/bitstream.h                        $*
 *                                                                                             *
 *              Original Author:: Tom Spencer-Smith                                            *
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 6/13/01 9:05a                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef BITSTREAM_H
#define BITSTREAM_H


#include "BitPacker.h"
#include "wwdebug.h"
#include "encoderlist.h"
#include "mathutil.h"
#include "math.h"
#include "widestring.h"

#define BYTE_DEPTH(x)		(sizeof(x))
#define BIT_DEPTH(x)			(8 * sizeof(x))


/**
** BitStreamClass
**
** Author:       Tom Spencer-Smith
** Date:         June 1998
** Description:  A class for minimal bit encoding.
**					  Notes:
**					  - Uncompressed data may be included in the bitstream.
**					  - Compression may be disabled entirely if desired.
**					  - Bools are compressed to 1 bit without requiring a precision
**					    setup.
**					  - Strings and raw data are uncompressed.
**
** (gth, 08/31/2000) - renamed this class to BitStreamClass (from cTypeEncoder) and
** cleaned it up to become the interface that all game and library code uses to
** package up their state variables for network transmission, converted to westwood 
** naming convention since it is going to propogate to a lot of other code.
*/

class BitStreamClass : public cBitPacker
{
	public:

		BitStreamClass();
      BitStreamClass& operator=(const BitStreamClass& rhs);

		uint32_t Get_Uncompressed_Size_Bytes() const {return UncompressedSizeBytes;}
		uint32_t Get_Compressed_Size_Bytes() const;
		uint32_t Get_Compression_Pc() const;

      //
      // For data which may include NULL's.
		// Data will not be compressed.
      //
      void Add_Raw_Data(const char * data, uint16_t data_size);
		void Get_Raw_Data(char * buffer, uint16_t buffer_size, uint16_t data_size);

      //
      // For data terminated with NULL.
		// Data will not be compressed.
		// You may permit or disallow empty strings to be passed.
      //
      void Add_Terminated_String(const char * string, bool permit_empty = false);
		void Get_Terminated_String(char * buffer, uint16_t buffer_size, bool permit_empty = false);

      //
      // For data terminated with NULL.
		// Data will not be compressed.
		// You may permit or disallow empty strings to be passed.
      //
      void Add_Wide_Terminated_String(const unichar_t *string, bool permit_empty = false);
		void Get_Wide_Terminated_String (unichar_t *buffer, uint16_t buffer_len, bool permit_empty = false);		

		//
		// Bool is special-cased because we know that we can always 
		// represent it as 1 bit.
		//
		void Add(bool value);
		bool Get(bool & value);

		// 
		// For all other data types that we want to support, call into our internal 
		// template function.  
		//
		enum {NO_ENCODER = -1};

		void		Add(uint8_t val,int type = NO_ENCODER)							{ Internal_Add(val,type); }
		void		Add(uint16_t val,int type = NO_ENCODER)						{ Internal_Add(val,type); }
        void		Add(uint32_t val,int type = NO_ENCODER)							{ Internal_Add(val,type); }
		void		Add(char val,int type = NO_ENCODER)							{ Internal_Add(val,type); }
		void		Add(int val,int type = NO_ENCODER)							{ Internal_Add(val,type); }
		void		Add(float val,int type = NO_ENCODER)						{ Internal_Add(val,type); }
#if !defined _MSC_VER || _MSC_VER > 1200
		void		Add(unichar_t val,int type = NO_ENCODER)						{ Internal_Add(val,type); }
#endif

		uint8_t		Get(uint8_t & set_val,int type = NO_ENCODER)					{ return Internal_Get(set_val,type); }
		uint16_t	Get(uint16_t & set_val,int type = NO_ENCODER)				{ return Internal_Get(set_val,type); }
        uint32_t		Get(uint32_t & set_val,int type = NO_ENCODER)				{ return Internal_Get(set_val,type); }
		char		Get(char & set_val,int type = NO_ENCODER)					{ return Internal_Get(set_val,type); }
		int		Get(int & set_val,int type = NO_ENCODER)					{ return Internal_Get(set_val,type); }
		float		Get(float & set_val,int type = NO_ENCODER)				{ return Internal_Get(set_val,type); }
#if !defined _MSC_VER || _MSC_VER > 1200
		unichar_t	Get(unichar_t & set_val,int type = NO_ENCODER)				{ return Internal_Get(set_val,type); }
#endif

	private:
		
		//
		// Add/Get for remaining atomic data types.
		// I really wish the following 3 methods were in the source file, but
		// the compiler won't accept this. Hopefully the pragma and MSVC
		// will prevent inlining.
		//
#pragma auto_inline(off)
      //------------------------------------------------------------------------------------
		template<class T> void Internal_Add(T value, int type = NO_ENCODER) {
			if (cEncoderList::Is_Compression_Enabled() && type != NO_ENCODER) {
				WWASSERT(type >= 0 && type < MAX_ENCODERTYPES);

				cEncoderTypeEntry & entry = cEncoderList::Get_Encoder_Type_Entry(type);

				//
				// If the following assert hits then the value of the type 
				// parameter is unknown.
				//
				WWASSERT(entry.Is_Valid());

				uint32_t scaled_value;
				bool is_in_range = entry.Scale(value, scaled_value);
				if (!is_in_range) {
					//WWDEBUG_SAY(("BitStreamClass::Add : Warning: out-of-range value clamped (type %d).\n",
					//	type));
					//DIE;
				}

				Add_Bits(scaled_value, entry.Get_Bit_Precision());

			} else {
				Add_Bits(*(reinterpret_cast<uint32_t *>(&value)), BIT_DEPTH(T));
			}

            UncompressedSizeBytes += BYTE_DEPTH(T);
		}
		
		//------------------------------------------------------------------------------------
      template<class T> T Internal_Get(T & value, int type = NO_ENCODER) {

			if (cEncoderList::Is_Compression_Enabled() && type != NO_ENCODER) {
				WWASSERT(type >= 0 && type < MAX_ENCODERTYPES);

				cEncoderTypeEntry & entry = cEncoderList::Get_Encoder_Type_Entry(type);

				//
				// If the following assert hits then the value of the type 
				// parameter is unknown.
				//
				WWASSERT(entry.Is_Valid());

				uint32_t u_value;
				Get_Bits(u_value, entry.Get_Bit_Precision());

				double f_value = entry.Unscale(u_value);

				if ((::fabs(f_value - static_cast<T>(f_value)) < MISCUTIL_EPSILON)) {
					//
					// N.B. More error may be introduced here
					//
					value = static_cast<T>(f_value);
				} else {
					value = static_cast<T>(cMathUtil::Round(f_value));
				}

				WWASSERT(entry.Is_Value_In_Range(value));

			} else {
				uint32_t u_value;
				Get_Bits(u_value, BIT_DEPTH(T));

				value = *(reinterpret_cast<T *>(&u_value));
			}
			return value;
		}

		//------------------------------------------------------------------------------------
#pragma auto_inline(on)

		uint32_t UncompressedSizeBytes; // for statistics only
};

#endif // TYPEENCODER_H
