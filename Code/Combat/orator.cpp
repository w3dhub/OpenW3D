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
 *                     $Archive:: /Commando/Code/Combat/orator.cpp          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/29/01 9:42p                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "orator.h"
#include "chunkio.h"
#include "activeconversation.h"
#include "physicalgameobj.h"
#include "oratortypes.h"


////////////////////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES			= 0x11060315,
	CHUNKID_GAMEOBJ
	
};

enum
{
	VARID_ID						= 1,	
	VARID_OLD_PTR,
	VARID_CONVERSATION,
	VARID_POSITION,
	VARID_HASARRIVED,
	VARID_FLAGS,
	XXX_VARID_PLAYER_TYPE,
	VARID_ORATOR_TYPE,
	VARID_IS_INVISIBLE,
	VARID_LOOK_AT_OBJID
};


//////////////////////////////////////////////////////////////////////////////
//
//	OratorClass
//
//////////////////////////////////////////////////////////////////////////////
OratorClass::OratorClass (void) :	
	Conversation (NULL),
	GameObj (NULL),
	Position (0, 0, 0),
	HasArrived (false),
	Flags (0),
	ID (-1),
	OratorType (0),
	IsInvisible (false),
	LookAtObjID (0)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	~OratorClass
//
//////////////////////////////////////////////////////////////////////////////
OratorClass::~OratorClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////////////
void
OratorClass::Initialize (PhysicalGameObj *game_obj)
{
	GameObj = game_obj;
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Set_Flags
//
//////////////////////////////////////////////////////////////////////////////
void
OratorClass::Set_Flags (int flags)
{
	Flags = flags;

	if (Flags & FLAG_DONT_MOVE) {
		HasArrived = true;
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Set_Flag
//
//////////////////////////////////////////////////////////////////////////////
void
OratorClass::Set_Flag (int flag, bool onoff)
{
	Flags &= ~flag;
	if (onoff) {
		Flags |= flag;
	}

	if (Flags & FLAG_DONT_MOVE) {
		HasArrived = true;
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Get_Flag
//
//////////////////////////////////////////////////////////////////////////////
bool
OratorClass::Get_Flag (int flag)
{
	return bool((Flags & flag) == flag);
}


//////////////////////////////////////////////////////////////////////////////
//
//	Get_Game_Obj
//
//////////////////////////////////////////////////////////////////////////////
PhysicalGameObj *
OratorClass::Get_Game_Obj (void) const
{
	PhysicalGameObj *game_obj = NULL;

	//
	//	Do some RTTI to see if we can return the soldier pointer
	//
	if (GameObj != NULL) {
		game_obj = GameObj.Get_Ptr ()->As_PhysicalGameObj ();
	}

	return game_obj;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Save
//
//////////////////////////////////////////////////////////////////////////////
bool
OratorClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_VARIABLES);

		WRITE_MICRO_CHUNK (csave, VARID_ID,					ID);
		WRITE_MICRO_CHUNK_PTR (csave, VARID_CONVERSATION,	Conversation);
		WRITE_MICRO_CHUNK (csave, VARID_POSITION,			Position);
		WRITE_MICRO_CHUNK (csave, VARID_HASARRIVED,		HasArrived);
		WRITE_MICRO_CHUNK (csave, VARID_FLAGS,				Flags);
		WRITE_MICRO_CHUNK (csave, VARID_ORATOR_TYPE,		OratorType);
		WRITE_MICRO_CHUNK (csave, VARID_IS_INVISIBLE,	IsInvisible);
		WRITE_MICRO_CHUNK (csave, VARID_LOOK_AT_OBJID,	LookAtObjID);		
		
		//
		//	Save our current pointer so we can remap it on load
		//
		OratorClass *old_ptr = this;
		WRITE_MICRO_CHUNK_PTR (csave, VARID_OLD_PTR, old_ptr);

	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_GAMEOBJ);
		GameObj.Save (csave);
	csave.End_Chunk ();

	return true;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Load
//
//////////////////////////////////////////////////////////////////////////////
bool
OratorClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_GAMEOBJ:
				GameObj.Load (cload);
				break;

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////
void
OratorClass::Load_Variables (ChunkLoadClass &cload)
{
	OratorClass *old_ptr = NULL;

	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK (cload, VARID_ID,					ID);
			READ_MICRO_CHUNK_PTR (cload, VARID_CONVERSATION,		Conversation);
			READ_MICRO_CHUNK (cload, VARID_POSITION,			Position);
			READ_MICRO_CHUNK (cload, VARID_HASARRIVED,		HasArrived);
			READ_MICRO_CHUNK (cload, VARID_FLAGS,				Flags);
			READ_MICRO_CHUNK_PTR (cload, VARID_OLD_PTR,			old_ptr);
			READ_MICRO_CHUNK (cload, VARID_ORATOR_TYPE,		OratorType);
			READ_MICRO_CHUNK (cload, VARID_IS_INVISIBLE,		IsInvisible);
			READ_MICRO_CHUNK (cload, VARID_LOOK_AT_OBJID,	LookAtObjID);
		}

		cload.Close_Micro_Chunk ();
	}

	//
	//	Fixup the pointer
	//
	if (Conversation != NULL) {
		REQUEST_REF_COUNTED_POINTER_REMAP ((RefCountClass **)&Conversation);

	}

	//
	//	Register our old pointer so other objects can safely remap to it
	//
	WWASSERT (old_ptr != NULL);
	SaveLoadSystemClass::Register_Pointer (old_ptr, this);
	return ;
}

