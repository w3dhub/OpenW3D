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
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/commando/inputconfig.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 7/18/01 6:09p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "inputconfig.h"
#include "chunkio.h"
#include "debug.h"


////////////////////////////////////////////////////////////////
//	Save/load constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES			= 0x07181106,
};

enum
{
	VARID_DISPLAY_NAME			= 1,
	VARID_FILENAME,
	VARID_IS_DEFAULT,
	VARID_IS_CUSTOM,
};


////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////
void
InputConfigClass::Save (ChunkSaveClass &csave)
{
	//
	//	Save the variables to their own chunk
	//
	csave.Begin_Chunk (CHUNKID_VARIABLES);
		WRITE_MICRO_CHUNK_WIDESTRING	(csave, VARID_DISPLAY_NAME,	DisplayName);
		WRITE_MICRO_CHUNK_WWSTRING		(csave, VARID_FILENAME,			Filename);
		WRITE_MICRO_CHUNK					(csave, VARID_IS_DEFAULT,		IsDefault);
		WRITE_MICRO_CHUNK					(csave, VARID_IS_CUSTOM,		IsCustom);
	csave.End_Chunk ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////
void
InputConfigClass::Load (ChunkLoadClass &cload)
{
	//
	//	Read all the sub-chunks
	//
	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;

			default:
				Debug_Say (("Unrecognized InputConfigClass::Load Chunk ID\n"));
				break;
		}

		cload.Close_Chunk ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
////////////////////////////////////////////////////////////////
void
InputConfigClass::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Read all the microchunks
	//
	while (cload.Open_Micro_Chunk ()) {
		switch(cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK_WIDESTRING	(cload, VARID_DISPLAY_NAME,	DisplayName);
			READ_MICRO_CHUNK_WWSTRING		(cload, VARID_FILENAME,			Filename);
			READ_MICRO_CHUNK					(cload, VARID_IS_DEFAULT,		IsDefault);
			READ_MICRO_CHUNK					(cload, VARID_IS_CUSTOM,		IsCustom);
		}

		cload.Close_Micro_Chunk ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	operator=
//
////////////////////////////////////////////////////////////////
const InputConfigClass &
InputConfigClass::operator= (const InputConfigClass &src)
{
	if (&src != this) {
		DisplayName	= src.DisplayName;
		Filename		= src.Filename;
		IsDefault	= src.IsDefault;
		IsCustom		= src.IsCustom;
	}

	return *this;
}
