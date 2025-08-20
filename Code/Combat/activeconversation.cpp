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
 *                     $Archive:: /Commando/Code/Combat/activeconversation.cpp      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/16/02 6:01p                                               $*
 *                                                                                             *
 *                    $Revision:: 31                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "activeconversation.h"
#include "soldier.h"
#include "chunkio.h"
#include "actionparams.h"
#include "timemgr.h"
#include "orator.h"
#include "Pathfind.h"
#include "globalsettings.h"
#include "soldierobserver.h"
#include "gameobjmanager.h"
#include "conversationmgr.h"
#include "debug.h"
#include "WWAudio.h"
#include "AudibleSound.h"
#include "translatedb.h"
#include "translateobj.h"
#include "combat.h"


////////////////////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES				= 0x08090727,
	CHUNKID_ORATOR,
	CHUNKID_MONITOR
};

enum
{
	XXXXX_CONVERSATION_PTR		= 0,
	VARID_CURRENT_REMARK,
	XXXXX_NEXT_REMARK_TIME,
	VARID_STATE,
	VARID_OLD_PTR,
	VARID_ID,
	VARID_INITIALIZING_TIMER,
	VARID_ACTION_ID,
	VARID_SPOKEN_BITMASK,
	VARID_CENTRAL_POS,
	VARID_PRIORITY,
	VARID_MAXDIST,
	VARID_IS_INTERRUPTABLE,
	VARID_CONVERSATION_ID,
	VARID_NEXT_REMARK_TIMER,
};

static const float RAND_STAND_DIST		= 2.0F;
static const float MIN_AUDIENCE_DIST	= 4.0F;


////////////////////////////////////////////////////////////////
//
//	ActiveConversationClass
//
////////////////////////////////////////////////////////////////
ActiveConversationClass::ActiveConversationClass (void)	:
	ID (0),
	Conversation (NULL),
	CurrentRemark (-1),
	NextRemarkTimer (0),
	State (STATE_INITIALIZING),
	InitializingTimeLeft (60000),
	ActionID (0),
	OratorSpokenBitmask (0),
	CentralPos (0, 0, 0),
	Priority (30),
	MaxDist (0),
	IsInterruptable (true),
	CurrentSound (NULL)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~ActiveConversationClass
//
////////////////////////////////////////////////////////////////
ActiveConversationClass::~ActiveConversationClass (void)
{
	Stop_Current_Sound ();
	Free_Orator_List ();
	REF_PTR_RELEASE (Conversation);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Free_Orator_List
//
////////////////////////////////////////////////////////////////
void
ActiveConversationClass::Free_Orator_List (void)
{
	//
	//	Free each orator in our list
	//
	for (int index = 0; index < OratorList.Count (); index ++) {
		OratorClass *orator = OratorList[index];
		if (orator != NULL) {

			PhysicalGameObj *game_obj = orator->Get_Game_Obj ();
			if (game_obj != NULL) {

				//
				//	Was this a soldier we were talking to?
				//
				SoldierGameObj *soldier = game_obj->As_SoldierGameObj ();
				if (soldier != NULL) {

					//
					//	Turn idle animations back on for soldiers
					//
					soldier->As_SoldierGameObj ()->Set_Loiters_Allowed (true);
				
					//
					//	Make sure the soldier doesn't ramble on after the
					// conversation has ended.
					//
					soldier->Stop_Current_Speech ();

					//
					//	Reset the possibility that this soldier will have a conversation
					//
					SoldierObserverClass	*innate_ai = soldier->Get_Innate_Controller ();
					if (innate_ai != NULL) {
						innate_ai->Reset_Conversation_Timer ();
					}
					
					//
					//	If we were controlling the soldier's head, return
					// it to its default position
					//
					if (orator->Get_Flag (OratorClass::FLAG_DONT_TURN_HEAD) == false) {
						soldier->Cancel_Look_At ();
					}
				}

				//
				//	Let the game obj know its no longer in a conversation
				//
				game_obj->Set_Conversation (NULL);
			}

			delete orator;
			orator = NULL;
		}
	}

	OratorList.Delete_All ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Conversation
//
////////////////////////////////////////////////////////////////
void
ActiveConversationClass::Set_Conversation (ConversationClass *conversation)
{
	REF_PTR_SET (Conversation, conversation);
	CurrentRemark			= -1;
	NextRemarkTimer		= 0;
	OratorSpokenBitmask	= 0;
	Priority					= conversation->Get_Priority ();
	MaxDist					= conversation->Get_Max_Dist ();
	IsInterruptable		= conversation->Is_Interruptable ();
	Free_Orator_List ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Add_Orator
//
////////////////////////////////////////////////////////////////
OratorClass *
ActiveConversationClass::Add_Orator (PhysicalGameObj *game_obj)
{
	//
	//	Let the game obj know he's part of a conversation
	//
	if (game_obj != NULL) {
		game_obj->Set_Conversation (this);
	}

	//
	//	Allocate and initialize a new orator
	//
	OratorClass *orator = new OratorClass;
	orator->Initialize (game_obj);
	orator->Set_ID (OratorList.Count ());	

	//
	//	Add the new orator to our list
	//
	OratorList.Add (orator);

	//
	//	If orators are still being added to the conversation, the
	// the object is considered initializing.
	//
	State = STATE_INITIALIZING;
	return orator;
}


////////////////////////////////////////////////////////////////
//
//	Start_Conversation
//
////////////////////////////////////////////////////////////////
void
ActiveConversationClass::Start_Conversation (void)
{
	WWASSERT (OratorList.Count () > 0);
	OratorSpokenBitmask = 0;

	//
	//	Find the first "visible" orator in our list
	//
	PhysicalGameObj *main_speaker = NULL;
	for (	int orator_index = 0;
			main_speaker == NULL && orator_index < OratorList.Count ();
			orator_index ++)
	{
		main_speaker = OratorList[orator_index]->Get_Game_Obj ();
	}

	//
	//	Get the location of the first orator
	//
	if (main_speaker != NULL) {
		
		//
		//	Pick a center position somewhere around the first speaker
		//
		main_speaker->Get_Position (&CentralPos);

		//
		//	Find a safe central position for the conversation
		//
		PathfindClass *pathfind = PathfindClass::Get_Instance();
		if (pathfind->Find_Random_Spot( CentralPos, 2, &CentralPos)) {

			for (int index = 0; index < OratorList.Count (); index ++) {

				//
				//	Don't allow this unit to face the speaker by default.  (This
				// flag will be cleared by the ActionCodeClass if its priority is high enough)
				//
				OratorList[index]->Set_Flag (OratorClass::FLAG_TEMP_DONT_FACE, true);

				//
				//	If this soldier is allowed to move, then calculate a new position for them
				//
				if (OratorList[index]->Get_Flag (OratorClass::FLAG_DONT_MOVE) == false) {

					//
					//	Lookup a safe random position to stand
					//
					Vector3 new_location;
					if (pathfind->Find_Random_Spot (CentralPos, 2, &new_location)) {

						//
						//	Store this position in our list so the orator can query
						// for his position later.
						//
						OratorList[index]->Set_Position (new_location);
					}
				}
			}
		}

		//
		//	Now, let each orator know its part of a conversation
		//
		for (int index = 0; index < OratorList.Count (); index ++) {			
			PhysicalGameObj *game_obj = OratorList[index]->Get_Game_Obj ();
			if (game_obj != NULL) {

				//
				//	Turn off idle animations for soldiers when they are
				// having a conversation
				//
				if (game_obj->As_SoldierGameObj () != NULL) {
					game_obj->As_SoldierGameObj ()->Set_Loiters_Allowed (false);
				}

				//
				//	Make sure we don't take control of a human player
				//
				if (	game_obj->As_SmartGameObj () == NULL ||
						game_obj->As_SmartGameObj ()->Is_Human_Controlled () ||
						OratorList.Count () <= 2)
				{
					OratorList[index]->Set_Flag (OratorClass::FLAG_DONT_MOVE,		true);
					OratorList[index]->Set_Flag (OratorClass::FLAG_TEMP_DONT_FACE, false);
					
					//
					//	Don't force facing if its a human player or inanimate object
					//
					if (	game_obj->As_SmartGameObj () == NULL ||
							game_obj->As_SmartGameObj ()->Is_Human_Controlled ())
					{
						OratorList[index]->Set_Flag (OratorClass::FLAG_DONT_FACE, true);
					}
					
					//
					//	Since this object doesn't have to move -- reset its position
					//
					Vector3 position;
					game_obj->Get_Position (&position);
					OratorList[index]->Set_Position (position);

				} else {
					ActionParamsStruct parameters;
					parameters.Priority		= Priority;
					parameters.ActionID		= ActionID;
					parameters.ObserverID	= 0;
					parameters.Join_Conversation (ID);			
					game_obj->As_SmartGameObj ()->Get_Action ()->Have_Conversation (parameters);
				}
			}
		}
	}

	//
	//	Begin waiting for the participants to move to their location
	//
	State = STATE_WAITING_FOR_AUDIENCE;

	//
	//	Don't allow a key conversation to be interrupted
	//
	if (Conversation != NULL && Conversation->Is_Key ()) {
		Set_Is_Interruptable (false);

		//
		//	Clear all other conversations from the manager
		//
		//ConversationMgrClass::Reset_All_Other_Conversations (this);
	} else {

		//
		//	Stop this conversation before it can start if there is
		// a key conversation playing
		//
		if (ConversationMgrClass::Is_Key_Conversation_Playing ()) {
			Stop_Conversation (ACTION_COMPLETE_CONVERSATION_INTERRUPTED);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Think
//
////////////////////////////////////////////////////////////////
void
ActiveConversationClass::Think (void)
{
	//
	// Make each orator do something meaningful
	//
	for (int index = 0; index < OratorList.Count (); index ++) {
		PhysicalGameObj *game_obj = OratorList[index]->Get_Game_Obj ();
		if (game_obj != NULL && game_obj->As_SoldierGameObj () != NULL) {
			SoldierGameObj *soldier = game_obj->As_SoldierGameObj ();
			
			//
			//	Stop the conversation if the orator is dead, otherwise
			// take control of some of his movements
			//
			if (soldier->Is_Dead () || soldier->Is_Destroyed ()) {
				Stop_Conversation (ACTION_COMPLETE_CONVERSATION_INTERRUPTED);
			} else {
				Control_Orator (soldier);
			}
		}
	}

	if (State == STATE_WAITING_FOR_AUDIENCE) {
		Check_For_Audience ();
	} else if (State == STATE_TALKING) {
		
		//
		//	Should we advance to the next remark?
		//
		NextRemarkTimer -= TimeManager::Get_Frame_Seconds ();
		if (NextRemarkTimer <= 0) {
			
			//
			//	If everyone is listening, then move on to the next remark,
			// otherwise kick out of the conversation
			//
			if (Is_Audience_In_Place () == false) {
				Stop_Conversation (ACTION_COMPLETE_CONVERSATION_INTERRUPTED);
			} else {
				Say_Next_Remark ();
			}
		}

	} else if (State == STATE_INITIALIZING) {
		
		//
		//	Scrap the conversation if too much time was spent initializing (probably an error).
		//
		InitializingTimeLeft -= TimeManager::Get_Frame_Seconds ();
		if (InitializingTimeLeft <= 0) {
			Stop_Conversation (ACTION_COMPLETE_CONVERSATION_UNABLE_TO_INIT);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Say_Next_Remark
//
////////////////////////////////////////////////////////////////
void
ActiveConversationClass::Say_Next_Remark (void)
{
	//
	//	Move onto the next remark if possible
	//
	CurrentRemark ++;
	if (CurrentRemark < Conversation->Get_Remark_Count ()) {

		//
		//	Notify the monitors of the event
		//
		if (CurrentRemark > 0) {			
			Notify_Monitors (CUSTOM_EVENT_CONVERSATION_REMARK_ENDED, CurrentRemark - 1);
		}
		Notify_Monitors (CUSTOM_EVENT_CONVERSATION_REMARK_STARTED, CurrentRemark);


		//
		//	Lookup who says the next line
		//
		ConversationRemarkClass remark;
		Conversation->Get_Remark_Info (CurrentRemark, remark);
		int orator_id	= remark.Get_Orator_ID ();
		int text_id		= remark.Get_Text_ID ();

		//
		//	Make sure the data is valid
		//

		WWASSERT_PRINT (orator_id >= 0 && orator_id < OratorList.Count (), Conversation->Get_Name ());
		if (orator_id >= 0 && orator_id < OratorList.Count ()) {			
			PhysicalGameObj *orator = OratorList[orator_id]->Get_Game_Obj ();

			//
			//	Set a bit which lets us know this orator has spoken at least once...
			//
			OratorSpokenBitmask |= (1 << orator_id);

			//
			//	Have the orator start his remark
			//
			SoldierGameObj *soldier = NULL;
			if (orator != NULL) {
				soldier = orator->As_SoldierGameObj ();
			}
						
			float duration = 0;
			
			if (soldier != NULL) {
				duration = SoldierGameObj::Say_Dynamic_Dialogue (text_id, soldier);
			} else {
				REF_PTR_RELEASE (CurrentSound);
				duration = SoldierGameObj::Say_Dynamic_Dialogue (text_id, NULL, &CurrentSound);
			}

			//
			//	Play an animation on the orator
			//
			if (orator != NULL && remark.Get_Animation_Name ().Get_Length () > 0) {
				orator->Set_Animation (remark.Get_Animation_Name (), false);
			}

			//
			//	Determine when we should switch to the next remark
			//
			NextRemarkTimer = duration;
		}

	} else {
		Stop_Conversation (ACTION_COMPLETE_CONVERSATION_ENDED);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Is_Audience_In_Place
//
////////////////////////////////////////////////////////////////
bool
ActiveConversationClass::Is_Audience_In_Place (void)
{
	bool retval = true;

	//
	//	Determine how far away each participant can be before
	// we terminate the conversation
	//
	float max_dist = MaxDist;
	if (max_dist <= 0) {
		if (Conversation->Get_AI_State () == AI_STATE_COMBAT) {
			max_dist = GlobalSettingsDef::Get_Global_Settings ()->Get_Combat_Conversation_Dist ();
		} else {
			max_dist = GlobalSettingsDef::Get_Global_Settings ()->Get_Conversation_Dist ();
		}
	}
	
	//
	//	Square the distance for faster comparisons
	//
	float max_dist2 = max_dist * max_dist;

	//
	//	Check to see if everyone is close enough to have this conversation
	//
	for (int index = 0; index < OratorList.Count (); index ++) {
		PhysicalGameObj *game_obj = OratorList[index]->Get_Game_Obj ();
		if (game_obj != NULL) {
			
			//
			//	Get this soldier's position
			//
			Vector3 position;
			game_obj->Get_Position (&position);

			//
			//	Calculate how far from the conversation's center this
			// soldier is.
			//
			Vector3 delta = position - CentralPos;
			delta.Z = delta.Z * 0.5F;

			//
			//	If this distance is greater then the maximum allowed, then
			// the audience is NOT is place
			//
			float distance2 = delta.Length2 ();
			if (distance2 > max_dist2) {
				retval = false;
				break;
			}
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Check_For_Audience
//
////////////////////////////////////////////////////////////////
void
ActiveConversationClass::Check_For_Audience (void)
{
	if (State == STATE_WAITING_FOR_AUDIENCE) {

		//
		//	If everyone is close enough then start the conversation
		//
		if (Is_Audience_In_Place ()) {
			State = STATE_TALKING;
			Notify_Monitors (CUSTOM_EVENT_CONVERSATION_BEGAN, Conversation->Get_ID ());
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////
bool
ActiveConversationClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_VARIABLES);

		//
		//	Save the ID of the conversation (if necessary)
		//
		if (Conversation != NULL) {
			int conversation_id = Conversation->Get_ID ();
			WRITE_MICRO_CHUNK (csave, VARID_CONVERSATION_ID,	conversation_id);
		}

		WRITE_MICRO_CHUNK (csave, VARID_CURRENT_REMARK,			CurrentRemark);
		WRITE_MICRO_CHUNK (csave, VARID_NEXT_REMARK_TIMER,		NextRemarkTimer);
		WRITE_MICRO_CHUNK (csave, VARID_STATE,						State);
		WRITE_MICRO_CHUNK (csave, VARID_ID,							ID);
		WRITE_MICRO_CHUNK (csave, VARID_INITIALIZING_TIMER,	InitializingTimeLeft);
		WRITE_MICRO_CHUNK (csave, VARID_ACTION_ID,				ActionID);
		WRITE_MICRO_CHUNK (csave, VARID_SPOKEN_BITMASK,			OratorSpokenBitmask);
		WRITE_MICRO_CHUNK (csave, VARID_CENTRAL_POS,				CentralPos);
		WRITE_MICRO_CHUNK (csave, VARID_PRIORITY,					Priority);
		WRITE_MICRO_CHUNK (csave, VARID_MAXDIST,					MaxDist);
		WRITE_MICRO_CHUNK (csave, VARID_IS_INTERRUPTABLE,		IsInterruptable);
						
		//
		//	Save our current pointer so we can remap it on load
		//
		ActiveConversationClass *old_ptr = this;
		WRITE_MICRO_CHUNK_PTR (csave, VARID_OLD_PTR, old_ptr);
	
	csave.End_Chunk ();

	//
	//	Save each of the monitors
	//
	int index;
	for (index = 0; index < MAX_MONITORS; index ++) {		
		if (MonitorArray[index].Get_Ptr () != NULL) {
			csave.Begin_Chunk (CHUNKID_MONITOR);
				MonitorArray[index].Save (csave);
			csave.End_Chunk ();
		}		
	}

	//
	//	Save each of the orators
	//
	for (index = 0; index < OratorList.Count (); index ++) {
		csave.Begin_Chunk (CHUNKID_ORATOR);
			OratorList[index]->Save (csave);
		csave.End_Chunk ();
	}

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////
bool
ActiveConversationClass::Load (ChunkLoadClass &cload)
{
	Free_Orator_List ();
	int monitor_index = 0;

	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;

			case CHUNKID_ORATOR:
			{
				//
				//	Allocate and load a new orator object
				//
				OratorClass *orator = new OratorClass;
				orator->Load (cload);

				//
				//	Add this new object to our list
				//
				OratorList.Add (orator);
			}
			break;

			case CHUNKID_MONITOR:
			{				
				//
				//	Load the monitor's reference from the chunk
				//
				MonitorArray[monitor_index++].Load (cload);
			}
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
ActiveConversationClass::Load_Variables (ChunkLoadClass &cload)
{
	ActiveConversationClass *old_ptr = NULL;
	int conversation_id = 0;

	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK (cload, VARID_CONVERSATION_ID,		conversation_id);
			READ_MICRO_CHUNK (cload, VARID_CURRENT_REMARK,		CurrentRemark);
			READ_MICRO_CHUNK (cload, VARID_NEXT_REMARK_TIMER,	NextRemarkTimer);
			READ_MICRO_CHUNK (cload, VARID_STATE,					State);
			READ_MICRO_CHUNK (cload, VARID_ID,						ID);
			READ_MICRO_CHUNK_PTR (cload, VARID_OLD_PTR,				old_ptr);
			READ_MICRO_CHUNK (cload, VARID_INITIALIZING_TIMER,	InitializingTimeLeft);
			READ_MICRO_CHUNK (cload, VARID_ACTION_ID,				ActionID);
			READ_MICRO_CHUNK (cload, VARID_SPOKEN_BITMASK,		OratorSpokenBitmask);		
			READ_MICRO_CHUNK (cload, VARID_CENTRAL_POS,			CentralPos);
			READ_MICRO_CHUNK (cload, VARID_PRIORITY,				Priority);
			READ_MICRO_CHUNK (cload, VARID_MAXDIST,				MaxDist);
			READ_MICRO_CHUNK (cload, VARID_IS_INTERRUPTABLE,	IsInterruptable);
		}

		cload.Close_Micro_Chunk ();
	}

	//
	//	Lookup the conversation pointer
	//
	if (conversation_id != 0) {
		Conversation = ConversationMgrClass::Find_Conversation (conversation_id);
	}

	//
	//	Register our old pointer so other objects can safely remap to it
	//
	WWASSERT (old_ptr != NULL);
	SaveLoadSystemClass::Register_Pointer (old_ptr, this);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Set_Orator_Arrived
//
///////////////////////////////////////////////////////////////////////
void
ActiveConversationClass::Set_Orator_Arrived (PhysicalGameObj *orator, bool has_arrived)
{
	WWASSERT (orator != NULL);
	
	for (int index = 0; index < OratorList.Count (); index ++) {
		PhysicalGameObj *curr_orator = OratorList[index]->Get_Game_Obj ();
	
		//
		//	If this is the orator we were looking for, then set
		// its arrived state
		//
		if (curr_orator != NULL && curr_orator == orator) {
			OratorList[index]->Set_Has_Arrived (has_arrived);
			break;
		}
	}		

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Orator_Location
//
///////////////////////////////////////////////////////////////////////
bool
ActiveConversationClass::Get_Orator_Location (PhysicalGameObj *orator, Vector3 *position)
{
	WWASSERT (orator != NULL);
	WWASSERT (position != NULL);
	bool retval = false;			
	
	for (int index = 0; index < OratorList.Count (); index ++) {
		PhysicalGameObj *curr_orator = OratorList[index]->Get_Game_Obj ();
	
		//
		//	If this is the orator we were looking for, then pass the
		// expected position back to the caller
		//
		if (curr_orator != NULL && curr_orator == orator) {
			(*position) = OratorList[index]->Get_Position ();
			break;
		}
	}		

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Current_Orator_Location
//
///////////////////////////////////////////////////////////////////////
bool
ActiveConversationClass::Get_Current_Orator_Location (Vector3 *position)
{
	WWASSERT (position != NULL);
	WWASSERT (Conversation != NULL);
	bool retval = false;

	//
	//	Lookup who is saying the current line
	//		
	int orator_id = 0;
	if (CurrentRemark >= 0 && CurrentRemark < Conversation->Get_Remark_Count ()) {
		ConversationRemarkClass remark;
		Conversation->Get_Remark_Info (CurrentRemark, remark);
		orator_id = remark.Get_Orator_ID ();
	}

	//
	//	Make sure the data is valid
	//
	WWASSERT (orator_id >= 0 && orator_id < OratorList.Count ());
	if (orator_id >= 0 && orator_id < OratorList.Count ()) {			
		
		//
		//	Now, return this orator's current position
		//
		PhysicalGameObj *orator = OratorList[orator_id]->Get_Game_Obj ();
		if (orator != NULL) {
			orator->Get_Position (position);
		}
		retval = true;
	}

	return retval;	
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Current_Orator
//
///////////////////////////////////////////////////////////////////////
PhysicalGameObj *
ActiveConversationClass::Get_Current_Orator (void)
{
	PhysicalGameObj *current_orator = NULL;

	//
	//	Lookup who is saying the current line
	//			
	if (CurrentRemark >= 0 && CurrentRemark < Conversation->Get_Remark_Count ()) {
		
		ConversationRemarkClass remark;
		Conversation->Get_Remark_Info (CurrentRemark, remark);
		int orator_id	= remark.Get_Orator_ID ();

		if (orator_id >= 0 && orator_id < OratorList.Count ()) {
			current_orator = OratorList[orator_id]->Get_Game_Obj ();
		}
	}	

	return current_orator;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Conversation_Center
//
///////////////////////////////////////////////////////////////////////
void
ActiveConversationClass::Get_Conversation_Center (Vector3 *position)
{
	WWASSERT (position != NULL);
	(*position) = CentralPos;
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Orator_Information
//
///////////////////////////////////////////////////////////////////////
OratorClass *
ActiveConversationClass::Get_Orator_Information (PhysicalGameObj *soldier)
{
	WWASSERT (soldier != NULL);
	OratorClass *orator = NULL;
	
	for (int index = 0; index < OratorList.Count (); index ++) {
		PhysicalGameObj *curr_soldier = OratorList[index]->Get_Game_Obj ();
	
		//
		//	If this is the orator we were looking for, then pass the
		// expected information back to the caller
		//
		if (curr_soldier != NULL && curr_soldier == soldier) {
			orator = OratorList[index];
			break;
		}
	}		

	return orator;
}


///////////////////////////////////////////////////////////////////////
//
//	Register_Monitor
//
///////////////////////////////////////////////////////////////////////
void
ActiveConversationClass::Register_Monitor (ScriptableGameObj *game_obj)
{
	bool found			= false;
	int empty_index	= -1;

	//
	//	Check to ensure this game object isn't already registered as a monitor
	//
	for (int index = 0; index < MAX_MONITORS; index ++) {
		ScriptableGameObj *curr_game_obj = MonitorArray[index];
		if (curr_game_obj == game_obj) {
			found = true;
			break;
		} else if (curr_game_obj == NULL) {
			empty_index = index;
		}
	}
	
	if (found == false) {

		//
		//	Insert the reference inside our array
		//
		if (empty_index != -1) {
			MonitorArray[empty_index] = game_obj;
		} else {
			Debug_Say (("Exceeded max monitors for an active conversation.\n"));
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Unregister_Monitor
//
///////////////////////////////////////////////////////////////////////
void
ActiveConversationClass::Unregister_Monitor (ScriptableGameObj	*game_obj)
{
	//
	//	Remove the monitor from our list
	//
	for (int index = 0; index < MAX_MONITORS; index ++) {
		ScriptableGameObj *curr_game_obj = MonitorArray[index];
		if (curr_game_obj == game_obj) {
			MonitorArray[index] = NULL;
			break;
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Notify_Monitors_On_End
//
///////////////////////////////////////////////////////////////////////
void
ActiveConversationClass::Notify_Monitors_On_End (ActionCompleteReason reason)
{
	//
	//	Notify all the game objects
	//
	for (int index = 0; index < MAX_MONITORS; index ++) {
		ScriptableGameObj *game_obj = MonitorArray[index];

		//
		//	Notify all the observers of this game object
		//
		if (game_obj != NULL) {
			const GameObjObserverList &observer_list = game_obj->Get_Observers ();
			for (int observer_index = 0; observer_index < observer_list.Count (); observer_index ++) {
				observer_list[observer_index]->Action_Complete (game_obj, ActionID, reason);
			}
		}
	}
		
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Notify_Monitors
//
///////////////////////////////////////////////////////////////////////
void
ActiveConversationClass::Notify_Monitors (int custom_event_id, int param)
{
	//
	//	Notify all the game objects
	//
	for (int index = 0; index < MAX_MONITORS; index ++) {
		ScriptableGameObj *game_obj = MonitorArray[index];

		//
		//	Notify all the observers of this game object
		//
		if (game_obj != NULL) {
			const GameObjObserverList &observer_list = game_obj->Get_Observers ();
			for (int observer_index = 0; observer_index < observer_list.Count (); observer_index ++) {
				observer_list[observer_index]->Custom (game_obj, custom_event_id, param, NULL);
			}
		}
	}
		
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Control_Orator
//
///////////////////////////////////////////////////////////////////////
void
ActiveConversationClass::Control_Orator (SoldierGameObj *soldier)
{
	OratorClass *orator_info = Get_Orator_Information (soldier);
	if (orator_info == NULL || CurrentRemark < 0) {
		return ;
	}

	const float HEAD_HEIGHT	= 1.7F;
	
	//
	//	Lookup information about the current remark
	//				
	ConversationRemarkClass remark;	
	Conversation->Get_Remark_Info (CurrentRemark, remark);

	if (orator_info->Get_Flag (OratorClass::FLAG_DONT_TURN_HEAD) == false) {

		//
		//	Now determine where this participant should be looking
		//
		PhysicalGameObj *orator = Get_Current_Orator ();		
		if (orator != NULL) {

			Vector3 look_pos (0, 0, 0);
			bool is_something_to_look_at = false;
			bool force_facing = false;

			//
			//	Lookup the object we are trying to look at...
			//
			int look_at_id						= orator_info->Get_Look_At_Obj ();
			PhysicalGameObj *look_at_obj	= NULL;
			if (look_at_id > 0) {
				look_at_obj = GameObjManager::Find_PhysicalGameObj (look_at_id);					
			} else if (look_at_id == -1) {
				look_at_obj = COMBAT_STAR;
			}
			
			//
			//	Now, either look at a specific object, look at the speaker, or
			// find a random person to look at...
			//
			if (look_at_obj != NULL) {
				
				look_at_obj->Get_Position (&look_pos);
				look_pos.Z += HEAD_HEIGHT;
				soldier->Look_At (look_pos, 100.0F);
				is_something_to_look_at	= true;
				force_facing				= true;
				
			} else if (orator != soldier) {

				Get_Current_Orator_Location (&look_pos);
				look_pos.Z += HEAD_HEIGHT;
				soldier->Look_At (look_pos, 100.0F);
				is_something_to_look_at = true;

			} else if (OratorList.Count () > 1) {

				//
				//	Choose a different participant to look at
				//				
				int index = remark.Get_Orator_ID () + 1;
				if (index >= OratorList.Count ()) {
					index = 0;
				}

				//
				//	Look at someone else
				//
				PhysicalGameObj *someone_to_lookat = OratorList[index]->Get_Game_Obj ();
				if (someone_to_lookat != NULL) {
					someone_to_lookat->Get_Position (&look_pos);
					look_pos.Z += HEAD_HEIGHT;
					soldier->Look_At (look_pos, 100.0F);
					is_something_to_look_at = true;
				}		
			}

			//
			// If the head look would turn the soldier to far, then turn the body as well
			//
			if (	is_something_to_look_at &&
					(force_facing ||
					 ((orator_info->Get_Flag (OratorClass::FLAG_DONT_FACE) == false) &&
					 (orator_info->Get_Flag (OratorClass::FLAG_TEMP_DONT_FACE) == false)))) 
			{
				//
				//	Get the target relative to the head
				//
				Vector3 relative_look_pos;
				Matrix3D::Inverse_Transform_Vector( soldier->Get_Transform (), look_pos, &relative_look_pos );

				//Vector3 delta = look_pos - soldier->Get_Transform ().Get_Translation ();

				//float curr_facing			= soldier->Get_Facing ();
				//float head_turn_angle	= WWMath::Atan2 (delta.Y, delta.X);
				//float angle					= head_turn_angle - curr_facing;
				float angle = WWMath::Atan2 (relative_look_pos.Y, relative_look_pos.X);

				if (force_facing || WWMath::Fabs (angle) > DEG_TO_RADF (15)) {
					soldier->Set_Targeting (look_pos, false);
				}
			}
		}
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Stop_Conversation
//
///////////////////////////////////////////////////////////////////////
void
ActiveConversationClass::Stop_Conversation (ActionCompleteReason reason)
{
	if (Is_Finished ()) {
		return ;
	}

	State = STATE_FINISHED;

	Stop_Current_Sound ();

	if (CurrentRemark >= 0 && CurrentRemark < Conversation->Get_Remark_Count ()) {
		
		//
		//	Get the current speaker
		//
		ConversationRemarkClass remark;
		Conversation->Get_Remark_Info (CurrentRemark, remark);
		int orator_id	= remark.Get_Orator_ID ();

		//
		//	Sanity check
		//
		if (orator_id >= 0 && orator_id < OratorList.Count ()) {			
			PhysicalGameObj *orator = OratorList[orator_id]->Get_Game_Obj ();

			//
			//	Have the current speaker stop speaking
			//
			if (orator != NULL && orator->As_SoldierGameObj () != NULL) {
				orator->As_SoldierGameObj ()->Stop_Current_Speech ();
			}
		}
	}
	
	Free_Orator_List ();

	Notify_Monitors_On_End (reason);

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Conversation_Time
//
///////////////////////////////////////////////////////////////////////
float
ActiveConversationClass::Get_Conversation_Time (void)
{
	float retval = 0.0F;

	for (int index = 0; index < Conversation->Get_Remark_Count (); index ++) {

		//
		//	Lookup up the line of text that will be spoken
		//
		ConversationRemarkClass remark;
		Conversation->Get_Remark_Info (index, remark);
		int text_id	= remark.Get_Text_ID ();

		//
		//	Lookup the translation object from the strings database
		//
		TDBObjClass *translate_obj = TranslateDBClass::Find_Object (text_id);
		if (translate_obj != NULL) {
			

			//
			//	Create the sound object
			//
			uint32 sound_def_id = translate_obj->Get_Sound_ID ();
			if (sound_def_id != 0) {
				
				//
				//	Add the duration of the sound object to the total
				//
				AudibleSoundClass *speech = WWAudioClass::Get_Instance ()->Create_Sound (sound_def_id);
				if (speech != NULL) {
					retval += (speech->Get_Duration() / 1000.0F);
					REF_PTR_RELEASE (speech);
				}
			}
		}		
	}

	return (retval > 0.0F) ? retval : 2.0F;
}


///////////////////////////////////////////////////////////////////////
//
//	Stop_Current_Sound
//
///////////////////////////////////////////////////////////////////////
void
ActiveConversationClass::Stop_Current_Sound (void)
{
	//
	//	Kill the current sound we are making (speech, scream, grunt, etc)
	//
	if (CurrentSound != NULL) {
		CurrentSound->Stop ();
		CurrentSound->Release_Ref ();
		CurrentSound = NULL;
	}

	return ;
}
