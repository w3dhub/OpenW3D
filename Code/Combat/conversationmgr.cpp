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
 *                     $Archive:: /Commando/Code/Combat/conversationmgr.cpp         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/15/02 9:37p                                               $*
 *                                                                                             *
 *                    $Revision:: 24                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "conversationmgr.h"
#include "combatchunkid.h"
#include "chunkio.h"
#include "vector3.h"
#include "pscene.h"
#include "physicalgameobj.h"
#include "aabox.h"
#include "conversation.h"
#include "wwmath.h"
#include "soldierobserver.h"
#include "playertype.h"
#include "activeconversation.h"
#include "phys.h"
#include "string_ids.h"
#include "oratortypes.h"


////////////////////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES						= 0x08090315,
	CHUNKID_OLD_CONVERSATION,
	CHUNKID_ACTIVE_CONVERSATION,
	CHUNKID_CONVERSATION_CATEGORY,
	CHUNKID_CONVERSATION
};

enum
{
	VARID_NEXT_ACTIVE_CONVERSATION_ID			= 0,
	XXX_VARID_NEXT_GLOBAL_CONVERSATION_ID,
	VARID_NEXT_LEVEL_CONVERSATION_ID,
	VARID_NEXT_GLOBAL_CONVERSATION_ID,
};

static const float	BUDDY_LOCATE_CX					= 7;
static const float	BUDDY_LOCATE_CY					= 7;
static const float	BUDDY_LOCATE_CZ					= 3;
static const int		ACTIVE_CONVERSATION_START_ID	= 1000;
static const int		LEVEL_CONVERSATION_START_ID	= 1;
static const int		GLOBAL_CONVERSATION_START_ID	= 100000;


////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
CONVERSATION_LIST						ConversationMgrClass::ConversationList[CATEGORY_MAX];
ACTIVE_CONVERSATION_LIST			ConversationMgrClass::ActiveConversationList;
int										ConversationMgrClass::NextActiveConversationID	= ACTIVE_CONVERSATION_START_ID;
int										ConversationMgrClass::NextGlobalConversationID	= GLOBAL_CONVERSATION_START_ID;
int										ConversationMgrClass::NextLevelConversationID	= LEVEL_CONVERSATION_START_ID;
ConversationMgrClass::CATEGORY	ConversationMgrClass::SaveCategoryID				= CATEGORY_GLOBAL;
bool										ConversationMgrClass::DisplayEmotIcons				= false;


////////////////////////////////////////////////////////////////
//	Singleton instance
////////////////////////////////////////////////////////////////
ConversationMgrClass	_ConversationMgrSaveLoad;


////////////////////////////////////////////////////////////////
//
//	ConversationMgrClass
//
////////////////////////////////////////////////////////////////
ConversationMgrClass::ConversationMgrClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~ConversationMgrClass
//
////////////////////////////////////////////////////////////////
ConversationMgrClass::~ConversationMgrClass (void)
{
	Reset ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Reset_Conversations
//
////////////////////////////////////////////////////////////////
void
ConversationMgrClass::Reset_Conversations (int category_index, bool reset_start_id)
{
	if (reset_start_id && category_index == CATEGORY_LEVEL) {
		NextLevelConversationID	= LEVEL_CONVERSATION_START_ID;
	}

	int count = ConversationList[category_index].Count ();

	//
	//	Release our hold on all the conversations in this category
	//
	for (int index = 0; index < count; index ++) {
		ConversationClass *conversation = ConversationList[category_index][index];
		REF_PTR_RELEASE (conversation);
	}

	ConversationList[category_index].Delete_All ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Reset_All_Other_Conversations
//
////////////////////////////////////////////////////////////////
void
ConversationMgrClass::Reset_All_Other_Conversations (ActiveConversationClass *active_conversation)
{
	NextActiveConversationID = active_conversation->Get_ID () + 1;

	//
	//	Release our hold on all the other conversations
	//
	int count = ActiveConversationList.Count ();
	for (int index = 0; index < count; index ++) {
		ActiveConversationClass *conversation = ActiveConversationList[index];
		if (conversation != NULL && conversation != active_conversation) {
			conversation->Stop_Conversation (ACTION_COMPLETE_CONVERSATION_INTERRUPTED);
			REF_PTR_RELEASE (conversation);
		}		
	}
	
	ActiveConversationList.Delete_All ();

	//
	//	Add this conversation back into the list
	//
	ActiveConversationList.Add (active_conversation);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Reset_Active_Conversations
//
////////////////////////////////////////////////////////////////
void
ConversationMgrClass::Reset_Active_Conversations (void)
{
	NextActiveConversationID = ACTIVE_CONVERSATION_START_ID;

	//
	//	Release our hold on all the active conversations
	//
	while (ActiveConversationList.Count () > 0) {
		ActiveConversationClass *active_conversation = ActiveConversationList[0];
		ActiveConversationList.Delete (0);
		if (active_conversation != NULL) {
			active_conversation->Stop_Conversation (ACTION_COMPLETE_CONVERSATION_INTERRUPTED);
		}
		REF_PTR_RELEASE (active_conversation);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Reset
//
////////////////////////////////////////////////////////////////
void
ConversationMgrClass::Reset (void)
{
	//
	//	Release our hold on all the active conversations
	//	
	Reset_Active_Conversations ();

	//
	//	Release our hold on all the conversations
	//
	for (int cat_index = 0; cat_index < CATEGORY_MAX; cat_index ++) {
		Reset_Conversations (cat_index);
	}
		
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Chunk_ID
//
////////////////////////////////////////////////////////////////
uint32
ConversationMgrClass::Chunk_ID (void) const
{
	return CHUNKID_CONVERSATION_MGR;
}


////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////
bool
ConversationMgrClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_VARIABLES);		
		WRITE_MICRO_CHUNK (csave, VARID_NEXT_ACTIVE_CONVERSATION_ID,	NextActiveConversationID);
		
		if (SaveCategoryID == CATEGORY_GLOBAL) {
			WRITE_MICRO_CHUNK (csave, VARID_NEXT_GLOBAL_CONVERSATION_ID,	NextGlobalConversationID);
		} else {
			WRITE_MICRO_CHUNK (csave, VARID_NEXT_LEVEL_CONVERSATION_ID,		NextLevelConversationID);
		}

	csave.End_Chunk ();

	//
	//	Save the conversations in the current category
	//
	csave.Begin_Chunk (CHUNKID_CONVERSATION_CATEGORY);

		//
		//	Save the category ID
		//
		csave.Write (&SaveCategoryID, sizeof (SaveCategoryID));
		
		//
		//	Save each conversation in this category
		//
		int count = ConversationList[SaveCategoryID].Count ();
		int index;
		for (index = 0; index < count; index ++) {
			ConversationClass *conversation = ConversationList[SaveCategoryID][index];
			if (conversation != NULL) {
				csave.Begin_Chunk (CHUNKID_CONVERSATION);
					conversation->Save (csave);
				csave.End_Chunk ();
			}
		}

	csave.End_Chunk ();

	//
	//	Save each active conversation to its own chunk
	//
	count = ActiveConversationList.Count ();
	for (index = 0; index < count; index ++) {
		ActiveConversationClass *active_conversation = ActiveConversationList[index];
		if (active_conversation != NULL) {
			csave.Begin_Chunk (CHUNKID_ACTIVE_CONVERSATION);
				active_conversation->Save (csave);
			csave.End_Chunk ();
		}
	}

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load_Conversations
//
////////////////////////////////////////////////////////////////
bool
ConversationMgrClass::Load_Conversations (ChunkLoadClass &cload, [[maybe_unused]] int category_id)
{
	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_CONVERSATION:
			{
				//
				//	Create a new object and its state from the chunk
				//
				ConversationClass *conversation = new ConversationClass;
				SET_REF_OWNER( conversation );
				conversation->Load (cload);
				
				//
				//	Debug check to ensure we've got the correct cateogry
				//
				WWASSERT (conversation->Get_Category_ID () == category_id);
				
				//
				//	Add this conversation to our list
				//
				Add_Conversation (conversation);
				REF_PTR_RELEASE (conversation);
			}
			break;
		}

		cload.Close_Chunk ();
	}

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////
bool
ConversationMgrClass::Load (ChunkLoadClass &cload)
{
	int old_style_conv_count = 0;

	//
	//	Remove all currently active conversations
	//
	Reset_Active_Conversations ();

	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_ACTIVE_CONVERSATION:
			{
				//
				//	Create a new object and its state from the chunk
				//
				ActiveConversationClass *active_conversation = new ActiveConversationClass;
				active_conversation->Load (cload);
				
				//
				//	Add this new active conversation to our list
				//
				ActiveConversationList.Add (active_conversation);
			}
			break;

			case CHUNKID_CONVERSATION_CATEGORY:
			{
				int category_id = 0;
				cload.Read (&category_id, sizeof (category_id));

				//
				//	Remove all conversations in this category
				//
				Reset_Conversations (category_id);

				//
				//	Load all the conversations in this category
				//
				Load_Conversations (cload, category_id);
			}
			break;

			case CHUNKID_OLD_CONVERSATION:
			{
				//
				//	Reset the list of level conversations if this
				// the first one loaded
				//
				if (old_style_conv_count == 0) {
					Reset_Conversations (CATEGORY_LEVEL);
				}
				old_style_conv_count ++;

				//
				//	Create a new object and its state from the chunk
				//
				ConversationClass *conversation = new ConversationClass;
				conversation->Load (cload);
				conversation->Set_Category_ID (CATEGORY_LEVEL);
				
				//
				//	Add this conversation to our list
				//
				Add_Conversation (conversation);
				REF_PTR_RELEASE (conversation);
			}
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
ConversationMgrClass::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			READ_MICRO_CHUNK (cload, VARID_NEXT_ACTIVE_CONVERSATION_ID,	NextActiveConversationID);
			READ_MICRO_CHUNK (cload, VARID_NEXT_GLOBAL_CONVERSATION_ID,	NextGlobalConversationID);
			READ_MICRO_CHUNK (cload, VARID_NEXT_LEVEL_CONVERSATION_ID,	NextLevelConversationID);
		}

		cload.Close_Micro_Chunk ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Build_Buddy_List
//
////////////////////////////////////////////////////////////////
void
ConversationMgrClass::Build_Buddy_List
(
	PhysicalGameObj *									orator,
	DynamicVectorClass<PhysicalGameObj *> &	buddy_list,
	bool													include_orator
)
{
	//
	//	Build a box, centered around the orator, that we can use to
	// collect other 'friendly' orators in the area
	//
	Vector3 orator_pos (0, 0, 0);
	orator->Get_Position (&orator_pos);
	orator_pos.Z += BUDDY_LOCATE_CZ / 3.0F;
	AABoxClass box (orator_pos, Vector3 (BUDDY_LOCATE_CX, BUDDY_LOCATE_CY, BUDDY_LOCATE_CZ));

	//
	//	Collect all the dynamic objects in this box
	//
	NonRefPhysListClass obj_list;
	PhysicsSceneClass::Get_Instance ()->Collect_Objects (box, false, true, &obj_list);
	
	//
	//	Loop over all the collected objects
	//
	NonRefPhysListIterator it (&obj_list);
	for (it.First(); !it.Is_Done(); it.Next()) {
		PhysClass *phys_obj = it.Peek_Obj ();
		
		//
		//	Check to ensure this game object is a orator
		//
		CombatPhysObserverClass *phys_observer = reinterpret_cast<CombatPhysObserverClass *>(phys_obj->Get_Observer ());
		if (phys_observer != NULL) {
			PhysicalGameObj *game_obj = phys_observer->As_PhysicalGameObj();
			if (game_obj != NULL) {			
				
				//
				//	Check to make sure we found a orator who can participate
				// in this conversation
				//
				PhysicalGameObj *other_orator = game_obj->As_PhysicalGameObj ();
				if (other_orator != NULL && (include_orator || other_orator != orator)) {

					//
					//	Can this orator hold a conversation?
					//
					if (	other_orator->Is_In_Conversation () == false &&
							other_orator->Are_Innate_Conversations_Enabled ())
					{
						buddy_list.Add (other_orator);
					}
				}
			}
		}
	}
	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Find_Active_Conversation
//
////////////////////////////////////////////////////////////////
ActiveConversationClass *
ConversationMgrClass::Find_Active_Conversation (int id)
{
	ActiveConversationClass *conversation = NULL;

	//
	//	Loop over each entry in the active conversation list, looking for one with
	// the supplied identifier.
	//
	for (int index = 0; index < ActiveConversationList.Count (); index ++) {
		ActiveConversationClass *curr_conversation = ActiveConversationList[index];
		
		//
		//	Is this the conversation we were looking for?
		//
		if (curr_conversation != NULL && curr_conversation->Get_ID () == id) {
			conversation = curr_conversation;
			conversation->Add_Ref ();
			break;
		}
	}

	return conversation;
}


////////////////////////////////////////////////////////////////
//
//	Find_Conversation
//
////////////////////////////////////////////////////////////////
ConversationClass *
ConversationMgrClass::Find_Conversation (const char *conversation_name)
{
	ConversationClass *conversation = NULL;

	//
	//	Loop over all the conversation categories
	//
	for (int cat_index = 0; cat_index < CATEGORY_MAX; cat_index ++) {
		
		//
		//	Loop over each conversation in this category, looking for one with
		// the requested name.
		//
		int count = ConversationList[cat_index].Count ();
		for (int index = 0; index < count; index ++) {
			ConversationClass *curr_conversation = ConversationList[cat_index][index];
			
			//
			//	Is this the conversation we were looking for?
			//
			if (	curr_conversation != NULL &&
					::strcmpi (curr_conversation->Get_Name (), conversation_name) == 0)
			{
				conversation = curr_conversation;
				conversation->Add_Ref ();
				break;
			}
		}
	}

	return conversation;
}


////////////////////////////////////////////////////////////////
//
//	Find_Conversation
//
////////////////////////////////////////////////////////////////
ConversationClass *
ConversationMgrClass::Find_Conversation (int conversation_id)
{
	ConversationClass *conversation = NULL;

	//
	//	Loop over all the conversation categories
	//
	for (int cat_index = 0; cat_index < CATEGORY_MAX; cat_index ++) {
		
		//
		//	Loop over each conversation in this category, looking for one with
		// the requested name.
		//
		int count = ConversationList[cat_index].Count ();
		for (int index = 0; index < count; index ++) {
			ConversationClass *curr_conversation = ConversationList[cat_index][index];
			
			//
			//	Is this the conversation we were looking for?
			//
			if (	(curr_conversation != NULL) &&
					(curr_conversation->Get_ID () == conversation_id))
			{
				conversation = curr_conversation;
				conversation->Add_Ref ();
				break;
			}
		}
	}

	return conversation;
}


////////////////////////////////////////////////////////////////
//
//	Start_Conversation
//
////////////////////////////////////////////////////////////////
ActiveConversationClass  *
ConversationMgrClass::Start_Conversation (PhysicalGameObj *orator, int conversation_id, bool force)
{
	ActiveConversationClass *active_conversation = NULL;

	//
	//	Try to find the requested conversation
	//
	ConversationClass *conversation = Find_Conversation (conversation_id);
	if (conversation != NULL) {

		//
		//	Start the conversation
		//
		active_conversation = Start_Conversation (orator, conversation, force);
		REF_PTR_RELEASE (conversation);
	}	

	//
	//	Return a pointer to the conversation.  Note:  This object has
	// an extra ref count on it, its the caller's responsibility to release
	// this ref count when they are finished with it.
	//
	return active_conversation;
}


////////////////////////////////////////////////////////////////
//
//	Start_Conversation
//
////////////////////////////////////////////////////////////////
ActiveConversationClass  *
ConversationMgrClass::Start_Conversation (PhysicalGameObj *orator, const char *conversation_name, bool force)
{
	ActiveConversationClass *active_conversation = NULL;

	//
	//	Try to find the requested conversation
	//
	ConversationClass *conversation = Find_Conversation (conversation_name);
	if (conversation != NULL) {

		//
		//	Start the conversation
		//
		active_conversation = Start_Conversation (orator, conversation, force);
		REF_PTR_RELEASE (conversation);
	}	

	//
	//	Return a pointer to the conversation.  Note:  This object has
	// an extra ref count on it, its the caller's responsibility to release
	// this ref count when they are finished with it.
	//
	return active_conversation;
}


////////////////////////////////////////////////////////////////
//
//	Start_Conversation
//
////////////////////////////////////////////////////////////////
ActiveConversationClass  *
ConversationMgrClass::Start_Conversation (PhysicalGameObj *orator, ConversationClass *conversation, bool force)
{
	//
	//	Build a list of potential participants in the conversation
	//
	DynamicVectorClass<PhysicalGameObj *> buddy_list;
	Build_Buddy_List (orator, buddy_list, false);
	
	//
	//	If we have enough people to hold this conversation, then activate
	// the new conversation.
	//
	ActiveConversationClass *active_conversation = NULL;
	if (Test_Conversation (orator, conversation, buddy_list, force)) {
		active_conversation = Create_New_Conversation (conversation, buddy_list);
	}

	//
	//	Return a pointer to the conversation.  Note:  This object has
	// an extra ref count on it, its the caller's responsibility to release
	// this ref count when they are finished with it.
	//
	return active_conversation;
}


////////////////////////////////////////////////////////////////
//
//	Create_New_Conversation
//
////////////////////////////////////////////////////////////////
ActiveConversationClass *
ConversationMgrClass::Create_New_Conversation
(
	ConversationClass *							conversation,
	DynamicVectorClass<PhysicalGameObj *> &	buddy_list
)
{
	ActiveConversationClass *active_conversation = new ActiveConversationClass;
	active_conversation->Set_Conversation (conversation);

	//
	//	Add the participants to the active conversation.
	// Note:  Its assumed that the buddy_list is in the correct
	// order for this conversation.
	//
	int count = conversation->Get_Orator_Count ();
	for (int index = 0; index < count; index ++) {
		active_conversation->Add_Orator (buddy_list[index]);
	}

	//
	//	Add it to our active conversation list
	//
	active_conversation->Set_ID (NextActiveConversationID ++);
	ActiveConversationList.Add (active_conversation);

	//
	//	Initialize the conversation
	//	
	active_conversation->Start_Conversation ();	

	//
	//	Increment the reference count on the conversation and
	// return it to the caller.
	//
	active_conversation->Add_Ref ();
	return active_conversation;
}


////////////////////////////////////////////////////////////////
//
//	Start_Conversation
//
////////////////////////////////////////////////////////////////
void
ConversationMgrClass::Start_Conversation (PhysicalGameObj *orator)
{
	//
	//	Build a list of potential participants in the conversation
	//
	DynamicVectorClass<PhysicalGameObj *> available_buddy_list;
	Build_Buddy_List (orator, available_buddy_list, false);
	
	//
	//	Try to find a conversation that this list of orators can have
	//
	DynamicVectorClass<PhysicalGameObj *> orator_list;
	ConversationClass *conversation = Pick_Conversation (orator, available_buddy_list, orator_list);
	if (conversation != NULL) {
		ActiveConversationClass *active_conversation = Create_New_Conversation (conversation, orator_list);
		REF_PTR_RELEASE (active_conversation);
	}
	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Test_Conversation
//
////////////////////////////////////////////////////////////////
bool
ConversationMgrClass::Test_Conversation
(
	PhysicalGameObj *									initiator,
	ConversationClass *								conversation,
	DynamicVectorClass<PhysicalGameObj *> &	buddy_list,
	bool													force
)
{
	int orator_count				= conversation->Get_Orator_Count ();
	bool retval						= (orator_count > 0);
	bool initiator_included		= false;

	DynamicVectorClass<PhysicalGameObj *> available_buddy_list = buddy_list;
	buddy_list.Reset_Active ();

	//
	//	Loop over all the orator requirements for this conversation and
	// see if they are in the buddy list
	//		
	for (int orator_index = 0; orator_index < orator_count; orator_index ++) {
		OratorClass *orator = conversation->Get_Orator (orator_index);
		
		//
		//	If the orator is invisible, it automatically passes the test (we
		// don't need a physical object to take the part).
		//
		bool found = false;
		if (orator->Is_Invisible ()) {
			buddy_list.Add (NULL);
			found = true;			
		} else {

			//
			//	Test the initiator
			//
			if (initiator_included == false) {
				if (force || Test_Orator (conversation, orator, initiator)) {
					buddy_list.Add (initiator);
					initiator_included	= true;
					found						= true;
				}
			}

			//
			//	Try to find an entry in our buddy list that matches the orator
			// requirements
			//			
			for (int index = 0; index < available_buddy_list.Count (); index ++) {
				PhysicalGameObj *game_obj = available_buddy_list[index];
				
				//
				//	Does this game object fit the orator description?
				//
				if (Test_Orator (conversation, orator, game_obj)) {
					buddy_list.Add (game_obj);

					//
					//	Remove this orator from the buddy list
					//
					available_buddy_list.Delete (index);
					found = true;					
					break;
				}				
			}
		}
		
		//
		//	If we can't match up even ONE of the orators, then the
		// conversation can't be executed
		//
		if (found == false) {
			retval = false;
			break;
		}
	}

	//
	//	We can't pass the conversation if the initiator isn't
	// included in the conversation.
	//
	if (initiator_included == false) {
		retval = false;
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Test_Orator
//
////////////////////////////////////////////////////////////////
bool
ConversationMgrClass::Test_Orator
(
	ConversationClass *		conversation,
	OratorClass *				orator,
	PhysicalGameObj *			game_obj
)
{
	bool retval = false;

	//
	//	Check to ensure the orator types match
	//
	if (orator->Get_Orator_Type () == game_obj->Get_Definition ().Get_Orator_Type ()) {

		//
		//	Check to see (if its a soldier) if this orator is in the correct state
		//
		SoldierGameObj *soldier = game_obj->As_SoldierGameObj ();
		if (	soldier == NULL ||
				soldier->Is_Human_Controlled () ||
				soldier->Get_AI_State () == conversation->Get_AI_State ())
		{
			retval = true;
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Pick_Conversation
//
////////////////////////////////////////////////////////////////
ConversationClass *
ConversationMgrClass::Pick_Conversation
(
	PhysicalGameObj *											initiator,
	const DynamicVectorClass<PhysicalGameObj *> &	available_orator_list,
	DynamicVectorClass<PhysicalGameObj *> &			orator_list
)
{
	ConversationClass *conversation = NULL;	
	
	//
	//	Try to find a conversation that matches the criteria
	//
	float best_match = 0;

	//
	//	Loop over all the conversation categories
	//
	for (int cat_index = 0; cat_index < CATEGORY_MAX; cat_index ++) {
		
		//
		//	Loop over all the conversations in this category
		//
		int count = ConversationList[cat_index].Count ();
		for (int index = 0; index < count; index ++) {
			ConversationClass *curr_conversation = ConversationList[cat_index][index];

			//
			//	Can this conversation be used innately?
			//
			if (	curr_conversation != NULL &&
					curr_conversation->Is_Innate ())
			{
				//
				//	Test this conversation to ensure all requirements are met
				//
				DynamicVectorClass<PhysicalGameObj *> curr_orator_list = available_orator_list;
				if (Test_Conversation (initiator, curr_conversation, curr_orator_list)) {
					
					//
					//	Randomize the selection process
					//
					float match_percent	= WWMath::Random_Float (0.9F, 1.1F);
					match_percent			*= curr_conversation->Get_Probability ();
					if (match_percent > best_match) {
						
						//
						//	This is our best bet so far, so save this conversation
						//
						best_match			= match_percent;
						conversation		= curr_conversation;
						orator_list			= curr_orator_list;
					}
				}
			}
		}
	}

	//
	//	If we've found a conversation, decrease its probability so
	// it will be less likely to be picked next time.
	//
	if (conversation != NULL) {
		float probability = conversation->Get_Probability ();
		probability			= probability * 0.75F;
		conversation->Set_Probability (probability);		
	}

	return conversation;
}


////////////////////////////////////////////////////////////////
//
//	Is_Key_Conversation_Playing
//
////////////////////////////////////////////////////////////////
bool
ConversationMgrClass::Is_Key_Conversation_Playing (void)
{
	bool retval = false;

	//
	//	Check for any "key" conversations
	//
	int index = ActiveConversationList.Count ();
	while (index --) {
		ActiveConversationClass *active_conversation = ActiveConversationList[index];

		//
		//	Is this a "key" conversation?
		//
		if (active_conversation->Peek_Conversation ()->Is_Key ()) {
			retval = true;
			break;
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Think
//
////////////////////////////////////////////////////////////////
void
ConversationMgrClass::Think (void)
{
	bool is_key = Is_Key_Conversation_Playing ();

	//
	//	Stop any non-key conversations that are playing (if there is a key conversation playing)
	//
	if (is_key) {
		bool release_key_conv = false;
		int index = ActiveConversationList.Count ();
		while (index --) {
			ActiveConversationClass *active_conversation = ActiveConversationList[index];
			if (active_conversation->Peek_Conversation ()->Is_Key () == false || release_key_conv) {

				//
				//	Stop any non-key conversation
				//
				active_conversation->Stop_Conversation ();
				
				//
				//	Remove the conversation from the list
				//
				ActiveConversationList.Delete (index);
				REF_PTR_RELEASE (active_conversation);
			} else {
				release_key_conv = true;
			}
		}
	}

	//
	//	Loop over all the remaining active conversations
	//
	int count = ActiveConversationList.Count ();
	for (int index = 0; index < count; index ++) {
		ActiveConversationClass *active_conversation = ActiveConversationList[index];
		
		//
		//	Let this conversation process
		//
		bool remove_from_list = true;
		if (active_conversation != NULL) {
			active_conversation->Think ();
			remove_from_list = active_conversation->Is_Finished ();
		}

		//
		//	Remove this conversation from our control (if necessary)
		//
		if (remove_from_list) {
			ActiveConversationList.Delete (index);
			REF_PTR_RELEASE (active_conversation);
			index --;
			count --;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Add_Conversation
//
////////////////////////////////////////////////////////////////
void
ConversationMgrClass::Add_Conversation (ConversationClass *conversation)
{
	WWASSERT (conversation != NULL);
	if (conversation == NULL) {
		return ;
	}

	//
	//	Assign this conversation an ID (if necessary)
	//
	if (conversation->Get_ID () == 0) {
		
		if (conversation->Get_Category_ID () == CATEGORY_LEVEL) {
			conversation->Set_ID (NextLevelConversationID ++);
		} else {
			conversation->Set_ID (NextGlobalConversationID ++);
		}
	}

	//
	//	Add a reference to the converation, then add it to our list
	//
	conversation->Add_Ref ();
	ConversationList[conversation->Get_Category_ID ()].Add (conversation);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_New_Conversation
//
////////////////////////////////////////////////////////////////
ActiveConversationClass *
ConversationMgrClass::Create_New_Conversation (ConversationClass *conversation)
{
	ActiveConversationClass *active_conversation = NULL;

	if (conversation != NULL) {

		//
		//	Allocate the new conversation
		//
		active_conversation = new ActiveConversationClass;
		active_conversation->Set_Conversation (conversation);

		//
		//	Add it to our active conversation list
		//
		active_conversation->Set_ID (NextActiveConversationID ++);
		ActiveConversationList.Add (active_conversation);
		active_conversation->Add_Ref ();
	}

	//
	//	Return a pointer to the conversation.  Note:  This object has
	// an extra ref count on it, its the caller's responsibility to release
	// this ref count when they are finished with it.
	//	
	return active_conversation;
}


////////////////////////////////////////////////////////////////
//
//	Remove_Conversation
//
////////////////////////////////////////////////////////////////
void
ConversationMgrClass::Remove_Conversation (ConversationClass *conversation)
{
	int cat_index	= conversation->Get_Category_ID ();
	int count		= ConversationList[cat_index].Count ();

	//
	//	Loop over all the conversations in this category
	//
	for (int index = 0; index < count; index ++) {
		ConversationClass *curr_conversation = ConversationList[cat_index][index];
		if (conversation == curr_conversation) {
			
			//
			//	Remove this conversation from the list
			//
			ConversationList[cat_index].Delete (index);
			conversation->Set_ID (0);
			REF_PTR_RELEASE (conversation);
			break;
		}
	}

	return ;
}
