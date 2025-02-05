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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/dialogue.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/14/01 12:10p                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dialogue.h"
#include "chunkio.h"
#include "wwmath.h"


////////////////////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_OPTION_VARIABLES		= 0x08040528,

	CHUNKID_DIALOGUE_VARIABLES		= 0x08040529,
	CHUNKID_DIALOGUE_OPTION
};

enum
{
	VARID_WEIGHT						= 0,
	XXX_VARID_REMARK_TEXT_ID,
	VARID_CONVERSATION_ID,

	VARID_DIALOGUE_SILENCE			= 0,
};

const char * const DIALOG_EVENT_NAMES[DIALOG_MAX] =
{
	"TAKE_DAMAGE_FROM_FRIEND",
	"TAKE_DAMAGE_FROM_ENEMY",
	"DAMAGE_FRIEND",
	"DAMAGE_ENEMY",
	"KILLED_FRIEND",
	"KILLED_ENEMY",
	"SAW_FRIEND",
	"SAW_ENEMY",
	"OBSOLETE_01",
	"OBSOLETE_02",
	"DIE",
	"POKE_IDLE",
	"POKE_SEARCH",
	"POKE_COMBAT",

	"IDLE_TO_COMBAT",
	"IDLE_TO_SEARCH",
	"SEARCH_TO_COMBAT",
	"SEARCH_TO_IDLE",
	"COMBAT_TO_SEARCH",
	"COMBAT_TO_IDLE"
};


////////////////////////////////////////////////////////////////
//
//	DialogueOptionClass
//
////////////////////////////////////////////////////////////////
DialogueOptionClass::DialogueOptionClass (void)
	:	Weight (1),
		ConversationID (0)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	DialogueOptionClass
//
////////////////////////////////////////////////////////////////
DialogueOptionClass::DialogueOptionClass (const DialogueOptionClass &src)
	:	Weight (1),
		ConversationID (0)
{
	(*this) = src;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~DialogueOptionClass
//
////////////////////////////////////////////////////////////////
DialogueOptionClass::~DialogueOptionClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	operator=
//
////////////////////////////////////////////////////////////////
const DialogueOptionClass &
DialogueOptionClass::operator= (const DialogueOptionClass &src)
{
	Weight			= src.Weight;
	ConversationID	= src.ConversationID;
	return (*this);
}


////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////
void
DialogueOptionClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_OPTION_VARIABLES);
		WRITE_MICRO_CHUNK (csave, VARID_WEIGHT, Weight);
		WRITE_MICRO_CHUNK (csave, VARID_CONVERSATION_ID, ConversationID);
	csave.End_Chunk ();
		
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////
void
DialogueOptionClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_OPTION_VARIABLES:
				Load_Variables (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////
void
DialogueOptionClass::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK (cload, VARID_WEIGHT, Weight);
			READ_MICRO_CHUNK (cload, VARID_CONVERSATION_ID, ConversationID);
		}

		cload.Close_Micro_Chunk ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	DialogueClass
//
////////////////////////////////////////////////////////////////
DialogueClass::DialogueClass (void)
	:	SilenceWeight (1)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	DialogueClass
//
////////////////////////////////////////////////////////////////
DialogueClass::DialogueClass (const DialogueClass &src)
	:	SilenceWeight (1)
{
	(*this) = src;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~DialogueClass
//
////////////////////////////////////////////////////////////////
DialogueClass::~DialogueClass (void)
{
	Free_Options ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	operator=
//
////////////////////////////////////////////////////////////////
const DialogueClass &
DialogueClass::operator= (const DialogueClass &src)
{
	SilenceWeight	= src.SilenceWeight;
	
	//
	//	Free any option objects we may contain
	//
	Free_Options ();

	//
	//	Copy all the option  objects from the src object
	//
	for (int index = 0; index < src.OptionList.Count (); index ++) {
		DialogueOptionClass *option = src.OptionList[index];
		if (option != NULL) {
			OptionList.Add (new DialogueOptionClass (*option));
		}
	}

	return (*this);
}


////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////
void
DialogueClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_DIALOGUE_VARIABLES);
		WRITE_MICRO_CHUNK (csave, VARID_DIALOGUE_SILENCE, SilenceWeight);
	csave.End_Chunk ();

	//
	//	Save the options
	//
	for (int index = 0; index < OptionList.Count (); index ++) {
		csave.Begin_Chunk (CHUNKID_DIALOGUE_OPTION);
			OptionList[index]->Save (csave);
		csave.End_Chunk ();
	}
		
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////
void
DialogueClass::Load (ChunkLoadClass &cload)
{
	Free_Options ();

	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_DIALOGUE_VARIABLES:
				Load_Variables (cload);
				break;

			case CHUNKID_DIALOGUE_OPTION:
			{
				//
				//	Create a new option object and add it to the list
				//
				DialogueOptionClass *option = new DialogueOptionClass;
				option->Load (cload);
				OptionList.Add (option);
			}
			break;
		}

		cload.Close_Chunk ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////
void
DialogueClass::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK (cload, VARID_DIALOGUE_SILENCE, SilenceWeight);
		}

		cload.Close_Micro_Chunk ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Free_Options
//
///////////////////////////////////////////////////////////////////////
void
DialogueClass::Free_Options (void)
{
	for (int index = 0; index < OptionList.Count (); index ++) {
		DialogueOptionClass *option = OptionList[index];
		if (option != NULL) {
			delete option;
		}
	}

	OptionList.Delete_All ();
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Conversation
//
///////////////////////////////////////////////////////////////////////
int
DialogueClass::Get_Conversation (void)
{
	int conv_id = 0;

	//
	//	Make a number we can use to index linearly into the option list 
	// to determine which one to use.
	//
	float total = SilenceWeight;
	int index;
	for (index = 0; index < OptionList.Count (); index ++) {
		total += OptionList[index]->Get_Weight ();
	}

	//
	//	Choose a random value in this linear range
	//
	float value = WWMath::Random_Float (0, total);

	//
	//	Now find the object this value corresponds to
	//
	float count = SilenceWeight;
	for (index = 0; value > count && index < OptionList.Count (); index ++) {
		conv_id	= OptionList[index]->Get_Conversation_ID ();
		count		+= OptionList[index]->Get_Weight ();
	}

	return conv_id;
}
