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
 *                     $Archive:: /Commando/Code/Combat/dialogue.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/14/01 12:10p                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#include "vector.h"


#ifndef __DIALOGUE_H
#define __DIALOGUE_H


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class DialogueOptionClass;
class ChunkSaveClass;
class ChunkLoadClass;


////////////////////////////////////////////////////////////////
//	Typedefs
////////////////////////////////////////////////////////////////
typedef DynamicVectorClass<DialogueOptionClass *>	DIALOGUE_OPTION_LIST;


////////////////////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////////////////////
typedef enum {

	DIALOG_ON_TAKE_DAMAGE_FROM_FRIEND			= 0,
	DIALOG_ON_TAKE_DAMAGE_FROM_ENEMY,
	DIALOG_ON_DAMAGE_FRIEND,
	DIALOG_ON_DAMAGE_ENEMY,
	DIALOG_ON_KILLED_FRIEND,
	DIALOG_ON_KILLED_ENEMY,
	DIALOG_ON_SAW_FRIEND,
	DIALOG_ON_SAW_ENEMY,
	DIALOG_ON_OBSOLETE_01,
	DIALOG_ON_OBSOLETE_02,
	DIALOG_ON_DIE,
	DIALOG_ON_POKE_IDLE,
	DIALOG_ON_POKE_SEARCH,
	DIALOG_ON_POKE_COMBAT,

	DIALOG_STATE_FROM_IDLE_TO_COMBAT,
	DIALOG_STATE_FROM_IDLE_TO_SEARCH,
	DIALOG_STATE_FROM_SEARCH_TO_COMBAT,
	DIALOG_STATE_FROM_SEARCH_TO_IDLE,
	DIALOG_STATE_FROM_COMBAT_TO_SEARCH,
	DIALOG_STATE_FROM_COMBAT_TO_IDLE,

	DIALOG_MAX

}	DialogEvents;

extern const char * const DIALOG_EVENT_NAMES[DIALOG_MAX];


////////////////////////////////////////////////////////////////
//
//	DialogueOptionClass
//
////////////////////////////////////////////////////////////////
class DialogueOptionClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	DialogueOptionClass (void);
	DialogueOptionClass (const DialogueOptionClass &src);
	virtual ~DialogueOptionClass (void);

	////////////////////////////////////////////////////////////////
	//	Public operators
	////////////////////////////////////////////////////////////////
	const DialogueOptionClass &	operator= (const DialogueOptionClass &src);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Accessors
	//
	int			Get_Conversation_ID (void) const		{ return ConversationID; }
	float			Get_Weight (void) const					{ return Weight; }

	void			Set_Conversation_ID (int id)			{ ConversationID = id; }
	void			Set_Weight (float weight)				{ Weight = weight; }

	//
	//	Save/load
	//
	void			Save (ChunkSaveClass &csave);
	void			Load (ChunkLoadClass &cload);

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void			Load_Variables (ChunkLoadClass &cload);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	float			Weight;
	int			ConversationID;
};


////////////////////////////////////////////////////////////////
//
//	DialogueClass
//
////////////////////////////////////////////////////////////////
class DialogueClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	DialogueClass (void);
	DialogueClass (const DialogueClass &src);
	virtual ~DialogueClass (void);

	////////////////////////////////////////////////////////////////
	//	Public operators
	////////////////////////////////////////////////////////////////
	const DialogueClass &	operator= (const DialogueClass &src);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Accessors
	//
	DIALOGUE_OPTION_LIST	&	Get_Option_List (void)					{ return OptionList; }
	void							Free_Options (void);

	float							Get_Silence_Weight (void)				{ return SilenceWeight; }
	void							Set_Silence_Weight (float weight)	{ SilenceWeight = weight; }

	//
	//	Evaluation
	//
	int							Get_Conversation (void);

	//
	//	Save/load
	//
	void							Save (ChunkSaveClass &csave);
	void							Load (ChunkLoadClass &cload);

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void							Load_Variables (ChunkLoadClass &cload);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	DIALOGUE_OPTION_LIST		OptionList;
	float							SilenceWeight;
};


#endif //__DIALOGUE_H

