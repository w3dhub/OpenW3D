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
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Combat/control.cpp                           $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 2/21/02 5:08p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 34                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "control.h"
#include "wwdebug.h"
#include "input.h"
#include "wwmath.h"
#include "wwpacket.h"
#include "chunkio.h"
#include "debug.h"
#include <memory.h>

#include "combat.h"
//#include "clientinterpmodels.h"
#include "encoderlist.h"
#include "bitpackids.h"

// Should pack analogs of 1 and -1 into single bit
//
// TSS 03/08/99: I have scaled the floats down to bytes.... more optimisation later!
//

ControlClass & ControlClass::operator = (const ControlClass & src)
{
	OneTimeBooleanBits		= src.OneTimeBooleanBits;
	ContinuousBooleanBits	= src.ContinuousBooleanBits;
	memcpy( AnalogValues, src.AnalogValues, sizeof(AnalogValues) ); 
	return *this;
}

/*
**
*/
enum	{
	CHUNKID_VARIABLES						=	914991110,

	MICROCHUNKID_ONE_TIME_BOOL			= 1,
	MICROCHUNKID_CONTINUOUS_BOOL,
	MICROCHUNKID_ANALOG,
	MICROCHUNKID_PENDING_ONE_TIME_BOOL,
	MICROCHUNKID_PENDING_CONTINUOUS_BOOL,
};

bool 	ControlClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ONE_TIME_BOOL, OneTimeBooleanBits );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_CONTINUOUS_BOOL, ContinuousBooleanBits );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_PENDING_ONE_TIME_BOOL, PendingOneTimeBooleanBits );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_PENDING_CONTINUOUS_BOOL, PendingContinuousBooleanBits );

		csave.Begin_Micro_Chunk(MICROCHUNKID_ANALOG);
		csave.Write(&AnalogValues[0],sizeof(AnalogValues));
		csave.End_Micro_Chunk();


	csave.End_Chunk();

	return true;
}

bool	ControlClass::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ONE_TIME_BOOL, OneTimeBooleanBits );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_CONTINUOUS_BOOL, ContinuousBooleanBits );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_PENDING_ONE_TIME_BOOL, PendingOneTimeBooleanBits );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_PENDING_CONTINUOUS_BOOL, PendingContinuousBooleanBits );
						case MICROCHUNKID_ANALOG:	
							cload.Read( &AnalogValues[0],sizeof(AnalogValues) ); 
							break;

						default:
							Debug_Say(( "Unrecognized Control Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized Control chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

//JITTER
void	ControlClass::Clear_Boolean( void )	
{ 
	WWASSERT( (8 * sizeof(ControlClass::OneTimeBooleanBits)) >= ControlClass::NUM_BOOLEAN_ONE_TIME  );
	WWASSERT( (8 * sizeof(ControlClass::ContinuousBooleanBits)) >= ControlClass::NUM_BOOLEAN_CONTINUOUS );

	OneTimeBooleanBits	= 0;
	ContinuousBooleanBits	= 0;
}


void	ControlClass::Clear_Control( void )	
{ 
	WWASSERT( (8 * sizeof(ControlClass::OneTimeBooleanBits)) >= ControlClass::NUM_BOOLEAN_ONE_TIME );
	WWASSERT( (8 * sizeof(ControlClass::ContinuousBooleanBits)) >= ControlClass::NUM_BOOLEAN_CONTINUOUS );

	OneTimeBooleanBits	= 0;
	ContinuousBooleanBits	= 0;
	memset( AnalogValues, 0, sizeof(AnalogValues) ); 
}

enum {
	CONTROL_MOVE_FORWARD = 0,
	CONTROL_MOVE_BACKWARD,
	CONTROL_MOVE_LEFT,
	CONTROL_MOVE_RIGHT,
	CONTROL_MOVE_UP,
	CONTROL_MOVE_DOWN,
	CONTROL_TURN_LEFT,
	CONTROL_TURN_RIGHT,
} CONTROL_MOVE_BITS;

//-----------------------------------------------------------------------------
void	ControlClass::Import_Cs( BitStreamClass & packet )
{ 
	// Or in the new one time bits
	uint32_t otb_bits = packet.Get(otb_bits, BITPACK_ONE_TIME_BOOLEAN_BITS);
	OneTimeBooleanBits |= otb_bits;

	packet.Get(ContinuousBooleanBits, BITPACK_CONTINUOUS_BOOLEAN_BITS);

#if 01
	packet.Get(AnalogValues[ANALOG_MOVE_FORWARD],	BITPACK_ANALOG_VALUES);
   packet.Get(AnalogValues[ANALOG_MOVE_LEFT],		BITPACK_ANALOG_VALUES);
   packet.Get(AnalogValues[ANALOG_MOVE_UP],			BITPACK_ANALOG_VALUES);
   packet.Get(AnalogValues[ANALOG_TURN_LEFT],		BITPACK_ANALOG_VALUES);
#else
	int	control_move;
	packet.Get( control_move,								BITPACK_CONTROL_MOVES_CS);

	AnalogValues[ANALOG_MOVE_FORWARD]	= 0.0f;
	AnalogValues[ANALOG_MOVE_LEFT]		= 0.0f;
	AnalogValues[ANALOG_MOVE_UP]			= 0.0f;
	AnalogValues[ANALOG_TURN_LEFT]		= 0.0f;

	if ( control_move & (1<<CONTROL_MOVE_FORWARD) ) {
		AnalogValues[ANALOG_MOVE_FORWARD] = 1.0f;
	}
	if ( control_move & (1<<CONTROL_MOVE_BACKWARD) ) {
		AnalogValues[ANALOG_MOVE_FORWARD] = -1.0f;
	}
	if ( control_move & (1<<CONTROL_MOVE_LEFT) ) {
		AnalogValues[ANALOG_MOVE_LEFT] = 1.0f;
	}
	if ( control_move & (1<<CONTROL_MOVE_RIGHT) ) {
		AnalogValues[ANALOG_MOVE_LEFT] = -1.0f;
	}
	if ( control_move & (1<<CONTROL_MOVE_UP) ) {
		AnalogValues[ANALOG_MOVE_UP] = 1.0f;
	}
	if ( control_move & (1<<CONTROL_MOVE_DOWN) ) {
		AnalogValues[ANALOG_MOVE_UP] = -1.0f;
	}
	if ( control_move & (1<<CONTROL_TURN_LEFT) ) {
		AnalogValues[ANALOG_TURN_LEFT] = 1.0f;
	}
	if ( control_move & (1<<CONTROL_TURN_RIGHT) ) {
		AnalogValues[ANALOG_TURN_LEFT] = -1.0f;
	}
#endif

	for (int i = 0; i < ANALOG_CONTROL_COUNT; i++) {
		if (fabs(AnalogValues[i]) < 0.005) {
			//
			// The code elsewhere has no tolerance for precision error on this 
			// value around zero...
			//
			AnalogValues[i] = 0;
		}
	}
}

//-----------------------------------------------------------------------------
void ControlClass::Export_Cs( BitStreamClass & packet ) 
{
//	packet.Add(OneTimeBooleanBits,		BITPACK_ONE_TIME_BOOLEAN_BITS);
	packet.Add(PendingOneTimeBooleanBits,		BITPACK_ONE_TIME_BOOLEAN_BITS);
	PendingOneTimeBooleanBits = 0;
//	packet.Add(ContinuousBooleanBits,	BITPACK_CONTINUOUS_BOOLEAN_BITS);
	packet.Add(PendingContinuousBooleanBits,	BITPACK_CONTINUOUS_BOOLEAN_BITS);
	PendingContinuousBooleanBits = 0;

#if 01
	packet.Add(AnalogValues[ANALOG_MOVE_FORWARD],	BITPACK_ANALOG_VALUES);
   packet.Add(AnalogValues[ANALOG_MOVE_LEFT],		BITPACK_ANALOG_VALUES);
   packet.Add(AnalogValues[ANALOG_MOVE_UP],			BITPACK_ANALOG_VALUES);
   packet.Add(AnalogValues[ANALOG_TURN_LEFT],		BITPACK_ANALOG_VALUES);
#else
	int	control_move = 0;
	if ( AnalogValues[ANALOG_MOVE_FORWARD] > 0.5f ) {
		control_move |= 1<<CONTROL_MOVE_FORWARD;
	}
	if ( AnalogValues[ANALOG_MOVE_FORWARD] < -0.5f ) {
		control_move |= 1<<CONTROL_MOVE_BACKWARD;
	}
	if ( AnalogValues[ANALOG_MOVE_LEFT] > 0.5f ) {
		control_move |= 1<<CONTROL_MOVE_LEFT;
	}
	if ( AnalogValues[ANALOG_MOVE_LEFT] < -0.5f ) {
		control_move |= 1<<CONTROL_MOVE_RIGHT;
	}
	if ( AnalogValues[ANALOG_MOVE_UP] > 0.5f ) {
		control_move |= 1<<CONTROL_MOVE_UP;
	}
	if ( AnalogValues[ANALOG_MOVE_UP] < -0.5f ) {
		control_move |= 1<<CONTROL_MOVE_DOWN;
	}
	if ( AnalogValues[ANALOG_TURN_LEFT] > 0.5f ) {
		control_move |= 1<<CONTROL_TURN_LEFT;
	}
	if ( AnalogValues[ANALOG_TURN_LEFT] < -0.5f ) {
		control_move |= 1<<CONTROL_TURN_RIGHT;
	}
   packet.Add( control_move,  							BITPACK_CONTROL_MOVES_CS);
#endif

}

//-----------------------------------------------------------------------------
void ControlClass::Import_Sc(BitStreamClass & packet)
{ 
	packet.Get(ContinuousBooleanBits, BITPACK_CONTINUOUS_BOOLEAN_BITS);

#if 01
	packet.Get(AnalogValues[ANALOG_MOVE_FORWARD],	BITPACK_ANALOG_VALUES);
	packet.Get(AnalogValues[ANALOG_MOVE_LEFT],		BITPACK_ANALOG_VALUES);
	packet.Get(AnalogValues[ANALOG_MOVE_UP],			BITPACK_ANALOG_VALUES);
	packet.Get(AnalogValues[ANALOG_TURN_LEFT],		BITPACK_ANALOG_VALUES);
#else
	int	control_move;
	packet.Get( control_move,								BITPACK_CONTROL_MOVES_SC);

	AnalogValues[ANALOG_MOVE_FORWARD]	= 0.0f;
	AnalogValues[ANALOG_MOVE_LEFT]		= 0.0f;
	AnalogValues[ANALOG_MOVE_UP]			= 0.0f;

	if ( control_move & (1<<CONTROL_MOVE_FORWARD) ) {
		AnalogValues[ANALOG_MOVE_FORWARD] = 1.0f;
	}
	if ( control_move & (1<<CONTROL_MOVE_BACKWARD) ) {
		AnalogValues[ANALOG_MOVE_FORWARD] = -1.0f;
	}
	if ( control_move & (1<<CONTROL_MOVE_LEFT) ) {
		AnalogValues[ANALOG_MOVE_LEFT] = 1.0f;
	}
	if ( control_move & (1<<CONTROL_MOVE_RIGHT) ) {
		AnalogValues[ANALOG_MOVE_LEFT] = -1.0f;
	}
	if ( control_move & (1<<CONTROL_MOVE_UP) ) {
		AnalogValues[ANALOG_MOVE_UP] = 1.0f;
	}
	if ( control_move & (1<<CONTROL_MOVE_DOWN) ) {
		AnalogValues[ANALOG_MOVE_UP] = -1.0f;
	}
/*	if ( control_move & (1<<CONTROL_TURN_LEFT) ) {
		AnalogValues[ANALOG_TURN_LEFT] = 1.0f;
	}
	if ( control_move & (1<<CONTROL_TURN_RIGHT) ) {
		AnalogValues[ANALOG_TURN_LEFT] = -1.0f;
	}*/
#endif

	for (int i = 0; i < ANALOG_CONTROL_COUNT; i++) {
		if (fabs(AnalogValues[i]) < 0.005) {
			//
			// The code elsewhere has no tolerance for precision error on this 
			// value around zero...
			//
			AnalogValues[i] = 0;
		}
	}
}

//-----------------------------------------------------------------------------
void ControlClass::Export_Sc(BitStreamClass & packet) 
{
	packet.Add(ContinuousBooleanBits,	BITPACK_CONTINUOUS_BOOLEAN_BITS);

#if 01
	packet.Add(AnalogValues[ANALOG_MOVE_FORWARD],	BITPACK_ANALOG_VALUES);
   packet.Add(AnalogValues[ANALOG_MOVE_LEFT],		BITPACK_ANALOG_VALUES);
   packet.Add(AnalogValues[ANALOG_MOVE_UP],			BITPACK_ANALOG_VALUES);
   packet.Add(AnalogValues[ANALOG_TURN_LEFT],		BITPACK_ANALOG_VALUES);
#else
	int	control_move = 0;
	if ( AnalogValues[ANALOG_MOVE_FORWARD] > 0.5f ) {
		control_move |= 1<<CONTROL_MOVE_FORWARD;
	}
	if ( AnalogValues[ANALOG_MOVE_FORWARD] < -0.5f ) {
		control_move |= 1<<CONTROL_MOVE_BACKWARD;
	}
	if ( AnalogValues[ANALOG_MOVE_LEFT] > 0.5f ) {
		control_move |= 1<<CONTROL_MOVE_LEFT;
	}
	if ( AnalogValues[ANALOG_MOVE_LEFT] < -0.5f ) {
		control_move |= 1<<CONTROL_MOVE_RIGHT;
	}
	if ( AnalogValues[ANALOG_MOVE_UP] > 0.5f ) {
		control_move |= 1<<CONTROL_MOVE_UP;
	}
	if ( AnalogValues[ANALOG_MOVE_UP] < -0.5f ) {
		control_move |= 1<<CONTROL_MOVE_DOWN;
	}
/*	if ( AnalogValues[ANALOG_TURN_LEFT] > 0.5f ) {
		control_move |= 1<<CONTROL_TURN_LEFT;
	}
	if ( AnalogValues[ANALOG_TURN_LEFT] < -0.5f ) {
		control_move |= 1<<CONTROL_TURN_RIGHT;
	}*/
   packet.Add( control_move,  							BITPACK_CONTROL_MOVES_SC);
#endif

}

//-----------------------------------------------------------------------------
/*
float	ControlClass::Get_Clamp(AnalogControl control) 
{
   //
   // This will definitely need improving...
   //

   float clamp;
   switch (control) {
      case ANALOG_MOVE_FORWARD :
         clamp = Input::Get_Clamp(INPUT_FUNCTION_RUN_FORWARD);
         break;
      case ANALOG_MOVE_LEFT :
         clamp = Input::Get_Clamp(INPUT_FUNCTION_RUN_LEFT);
         break;
		case ANALOG_MOVE_UP :
         clamp = Input::Get_Clamp(INPUT_FUNCTION_RUN_FORWARD);
         break;
		case ANALOG_TURN_LEFT :
         clamp = Input::Get_Clamp(INPUT_FUNCTION_TURN_LEFT);
         break;
		case ANALOG_WEAPON_UP :
         clamp = Input::Get_Clamp(INPUT_FUNCTION_WEAPON_UP);
         break;
		case ANALOG_WEAPON_LEFT :
//       clamp = 1.0f;
         clamp = Input::Get_Clamp(INPUT_FUNCTION_WEAPON_LEFT);
         break;
      default:
		   WWASSERT(0);
   }

   WWASSERT(clamp > 0);
   return clamp;
}
*/

/*
BYTE ControlClass::Scale_Analog(float clamp, float unscaled) 
{
   WWASSERT(unscaled > -clamp - WWMATH_EPSILON);
   WWASSERT(unscaled < +clamp + WWMATH_EPSILON);
   return (BYTE) (255 * WWMath::Clamp((unscaled + clamp) / (2 * clamp)));
}

float ControlClass::Unscale_Analog(float clamp, BYTE scaled) 
{
   float unscaled;
   if (scaled == 127) { // scaling perturbs zero, special-case it
      unscaled = 0;
   } else {
      unscaled = scaled / 255.0f * 2 * clamp - clamp;
   }
   WWASSERT(unscaled > -clamp - WWMATH_EPSILON);
   WWASSERT(unscaled < +clamp + WWMATH_EPSILON);
   return unscaled;
}
*/

void	ControlClass::Set_Boolean( BooleanControl bcontrol, bool state )
{ 
	int control = bcontrol;
	if ( control >= BOOLEAN_CONTINUOUS_FIRST ) {
		control -= BOOLEAN_CONTINUOUS_FIRST;
		if (state) 	ContinuousBooleanBits |= (1 << control); 
		else 		  	ContinuousBooleanBits &= ~(1 << control); 

		PendingContinuousBooleanBits |= ContinuousBooleanBits;
	} else {
		control -= BOOLEAN_ONE_TIME_FIRST;
		if (state) 	OneTimeBooleanBits |= (1 << control); 
		else 		  	OneTimeBooleanBits &= ~(1 << control); 

		PendingOneTimeBooleanBits |= OneTimeBooleanBits;
	}
}

bool	ControlClass::Get_Boolean( BooleanControl bcontrol )
{ 
	int control = bcontrol;
	if ( control >= BOOLEAN_CONTINUOUS_FIRST ) {
		control -= BOOLEAN_CONTINUOUS_FIRST;
		return ( ContinuousBooleanBits & (1 << control ) ) ? true : false; 
	} else {
		control -= BOOLEAN_ONE_TIME_FIRST;
		return ( OneTimeBooleanBits & (1 << control ) ) ? true : false; 
	}
}

void ControlClass::Set_Precision(void)
{
	cEncoderList::Set_Precision(BITPACK_ONE_TIME_BOOLEAN_BITS,		NUM_BOOLEAN_ONE_TIME);
	cEncoderList::Set_Precision(BITPACK_CONTINUOUS_BOOLEAN_BITS,	NUM_BOOLEAN_CONTINUOUS);

	cEncoderList::Set_Precision(BITPACK_CONTROL_MOVES_CS,				CONTROL_TURN_RIGHT+1);	// 8
	cEncoderList::Set_Precision(BITPACK_CONTROL_MOVES_SC,				CONTROL_MOVE_DOWN+1); // 6

	//
	// TSS: I think that analog values are within this range, except for 
	// DEBUG_RAPID_MOVE...
	//
	cEncoderList::Set_Precision(BITPACK_ANALOG_VALUES,	-1.0, 1.0, 0.01);


	/*
	//
	// Proof that zero does not map to zero...
	//
	cPacket test;
	float zero = 0.0f;
	test.Add(zero, BITPACK_ANALOG_VALUES);

	float zero2 = -999;
	test.Get(zero2, BITPACK_ANALOG_VALUES);
	*/
}
