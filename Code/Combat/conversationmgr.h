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
 *                     $Archive:: /Commando/Code/Combat/conversationmgr.h        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/15/02 9:37p                                               $*
 *                                                                                             *
 *                    $Revision:: 16                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __CONVERSATION_MGR_H
#define __CONVERSATION_MGR_H

#include "saveloadsubsystem.h"
#include "vector.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class ChunkSaveClass;
class ChunkLoadClass;
class ConversationClass;
class ActiveConversationClass;
class PhysicalGameObj;
class OratorClass;

////////////////////////////////////////////////////////////////
//	Singleton instance
////////////////////////////////////////////////////////////////
extern class ConversationMgrClass	_ConversationMgrSaveLoad;


////////////////////////////////////////////////////////////////
//	Typedefs
////////////////////////////////////////////////////////////////
typedef DynamicVectorClass<ConversationClass *>			CONVERSATION_LIST;
typedef DynamicVectorClass<ActiveConversationClass *>	ACTIVE_CONVERSATION_LIST;


////////////////////////////////////////////////////////////////
//
//	ConversationMgrClass
//
////////////////////////////////////////////////////////////////
class ConversationMgrClass : public SaveLoadSubSystemClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constants
	////////////////////////////////////////////////////////////////
	typedef enum
	{
		CATEGORY_GLOBAL	= 0,
		CATEGORY_LEVEL,
		CATEGORY_MAX
	} CATEGORY;
	
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	ConversationMgrClass (void);
	virtual ~ConversationMgrClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Initialization
	//
	static void								Initialize (void)	{}
	static void								Shutdown (void)	{ Reset (); }


	//
	//	Conversation editing
	//
	static void								Add_Conversation (ConversationClass *conversation);
	static int								Get_Conversation_Count (void);
	static int								Get_Conversation_Count (int category_index)			{ return ConversationList[category_index].Count (); }
	static ConversationClass *			Peek_Conversation (int index);
	static ConversationClass *			Peek_Conversation (int category_index, int index)	{ return ConversationList[category_index][index]; }
	static void								Remove_Conversation (ConversationClass *conversation);

	//
	//	Runtime conversation access
	//
	static ActiveConversationClass *	Create_New_Conversation (ConversationClass *conversation, DynamicVectorClass<PhysicalGameObj *> &buddy_list);
	static ActiveConversationClass *	Create_New_Conversation (ConversationClass *conversation);

	static void								Start_Conversation (PhysicalGameObj *orator);
	static ActiveConversationClass *	Start_Conversation (PhysicalGameObj *orator, const char *conversation_name, bool force = false);
	static ActiveConversationClass *	Start_Conversation (PhysicalGameObj *orator, int conversation_id, bool force = false);
	static ActiveConversationClass *	Start_Conversation (PhysicalGameObj *orator, ConversationClass *conversation, bool force = false);
	static ConversationClass *			Pick_Conversation (PhysicalGameObj *initiator, const DynamicVectorClass<PhysicalGameObj *> &available_orator_list, DynamicVectorClass<PhysicalGameObj *> &orator_list);
	
	static ConversationClass *			Find_Conversation (const char *conversation_name);
	static ConversationClass *			Find_Conversation (int conversation_id);
	static ActiveConversationClass *	Find_Active_Conversation (int id);

	static void								Reset_All_Other_Conversations (ActiveConversationClass *active_conversation);
	static bool								Is_Key_Conversation_Playing (void);
	
	static int								Get_Active_Conversation_Count (void) { return ActiveConversationList.Count (); }
	static void								Think (void);
	
	//
	//	State access
	//
	static void								Reset (void);
	static void								Reset_Conversations (int category_index, bool reset_start_id = false);
	static void								Reset_Active_Conversations (void);
	static void								Set_Category_To_Save (CATEGORY category_index) { SaveCategoryID = category_index; }

	//
	//	EmotIcon control
	//
	static bool								Are_Emot_Icons_Displayed (void)	{ return DisplayEmotIcons; }
	static void								Display_Emot_Icons (bool onoff)	{ DisplayEmotIcons = onoff; }

	//
	//	From SaveLoadSubSystemClass
	//
	uint32									Chunk_ID (void) const override;

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	static void				Build_Buddy_List (PhysicalGameObj *orator, DynamicVectorClass<PhysicalGameObj *> &list, bool include_soldier);
	static bool				Test_Conversation (PhysicalGameObj *initiator, ConversationClass *conversation, DynamicVectorClass<PhysicalGameObj *> &buddy_list, bool force = false);
	static bool				Test_Orator (ConversationClass *conversation, OratorClass *orator, PhysicalGameObj *game_obj);
	
	//
	//	From SaveLoadSubSystemClass
	//
	bool						Contains_Data (void) const override			{ return true; }
	const char *			Name (void) const override						{ return "ConversationMgrClass"; }
	bool						Save (ChunkSaveClass &csave) override;
	bool						Load (ChunkLoadClass &cload) override;
	void						Load_Variables (ChunkLoadClass &cload);
	bool						Load_Conversations (ChunkLoadClass &cload, int category_id);

private:

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////	
	static CONVERSATION_LIST			ConversationList[CATEGORY_MAX];
	static ACTIVE_CONVERSATION_LIST	ActiveConversationList;
	static int								NextActiveConversationID;
	static int								NextGlobalConversationID;
	static int								NextLevelConversationID;
	static CATEGORY						SaveCategoryID;
	static bool								DisplayEmotIcons;
};


////////////////////////////////////////////////////////////////
//	Get_Conversation_Count
////////////////////////////////////////////////////////////////	
inline int
ConversationMgrClass::Get_Conversation_Count (void)
{
	int total = 0;
	for (int index = 0; index < CATEGORY_MAX; index ++) {
		total += ConversationList[index].Count ();
	}

	return total;
}


////////////////////////////////////////////////////////////////
//	Peek_Conversation
////////////////////////////////////////////////////////////////	
inline ConversationClass *
ConversationMgrClass::Peek_Conversation (int index_to_find)
{
	ConversationClass *conversation = NULL;	

	//
	//	Loop over all the categories
	//
	for (int index = 0; index < CATEGORY_MAX; index ++) {
		
		//
		//	Is the specified index inside of this category?
		//
		int curr_count = ConversationList[index].Count ();
		if (index_to_find < curr_count) {
			
			//
			//	Index into this category to find the conversation
			//
			conversation = ConversationList[index][index_to_find];
			break;
		} else {

			//
			//	Adjust the specified index to be within the range
			// of the next category
			//
			index_to_find -= curr_count;
		}
	}

	return conversation;
}


#endif //__CONVERSATION_MGR_H

