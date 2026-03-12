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
 *                     $Archive:: /Commando/Code/wwphys/physcontrol.cpp                       $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 4/04/00 10:55a                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "physcontrol.h"
#include "chunkio.h"
#include "wwdebug.h"

enum
{
	PHYSCONTROLLER_CHUNK_VARIABLES				= 0x00000100,
	PHYSCONTROLLER_VARIABLE_MOVEVECTOR			= 0x00,
	PHYSCONTROLLER_VARIABLE_TURNLEFT,
};

bool PhysControllerClass::Save(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(PHYSCONTROLLER_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,PHYSCONTROLLER_VARIABLE_MOVEVECTOR,MoveVector);
	WRITE_MICRO_CHUNK(csave,PHYSCONTROLLER_VARIABLE_TURNLEFT,TurnLeft);
	csave.End_Chunk();

	WWASSERT(WWMath::Is_Valid_Float(MoveVector.X));
	WWASSERT(WWMath::Is_Valid_Float(MoveVector.Y));
	WWASSERT(WWMath::Is_Valid_Float(MoveVector.Z));
	WWASSERT(WWMath::Is_Valid_Float(TurnLeft));

	return true;
}


bool PhysControllerClass::Load(ChunkLoadClass & cload)
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID())
		{
			case PHYSCONTROLLER_CHUNK_VARIABLES:

				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,PHYSCONTROLLER_VARIABLE_MOVEVECTOR,MoveVector);
						READ_MICRO_CHUNK(cload,PHYSCONTROLLER_VARIABLE_TURNLEFT,TurnLeft);
					}
					cload.Close_Micro_Chunk();
				}
				break;
			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
		}
		cload.Close_Chunk();
	}

	WWASSERT(WWMath::Is_Valid_Float(MoveVector.X));
	WWASSERT(WWMath::Is_Valid_Float(MoveVector.Y));
	WWASSERT(WWMath::Is_Valid_Float(MoveVector.Z));
	WWASSERT(WWMath::Is_Valid_Float(TurnLeft));

	return true;
}

