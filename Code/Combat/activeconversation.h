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
 *                     $Archive:: /Commando/Code/Combat/activeconversation.h       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/05/01 1:46p                                              $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __ACTIVE_CONVERSATION_H
#define __ACTIVE_CONVERSATION_H

#include "refcount.h"
#include "conversation.h"
#include "vector.h"
#include "vector3.h"
#include "gameobjref.h"

////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class ChunkSaveClass;
class ChunkLoadClass;
class PhysicalGameObj;
class OratorClass;

////////////////////////////////////////////////////////////////
//
//	ActiveConversationClass
//
////////////////////////////////////////////////////////////////
class ActiveConversationClass : public RefCountClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	ActiveConversationClass (void);
	virtual ~ActiveConversationClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Save/load methods
	//
	bool						Save (ChunkSaveClass &csave);
	bool						Load (ChunkLoadClass &cload);

	//
	//	Identification
	//
	int						Get_ID (void) const	{ return ID; }
	void						Set_ID (int id)		{ ID = id; }

	//
	//	Conversation methods
	//
	ConversationClass *	Peek_Conversation (void) const;
	void						Set_Conversation (ConversationClass *conversation);

	//
	//	Conversation flow control
	//
	void						Start_Conversation (void);
	void						Stop_Conversation (ActionCompleteReason reason = ACTION_COMPLETE_CONVERSATION_ENDED);
	void						Think (void);
	bool						Is_Finished (void)				{ return bool(State == STATE_FINISHED); }

	//
	//	State evaluation methods
	//
	bool						Get_Orator_Location (PhysicalGameObj *orator, Vector3 *position);
	bool						Get_Current_Orator_Location (Vector3 *position);
	PhysicalGameObj *		Get_Current_Orator (void);
	void						Get_Conversation_Center (Vector3 *position);
	void						Set_Orator_Arrived (PhysicalGameObj *orator, bool has_arrived);

	void						Control_Orator (SoldierGameObj *orator);

	//
	//	Orator methods
	//
	OratorClass *			Add_Orator (PhysicalGameObj *orator);
	OratorClass *			Get_Orator_Information (PhysicalGameObj *orator);

	//
	//	Monitor support
	//
	void						Register_Monitor (ScriptableGameObj	*game_obj);
	void						Unregister_Monitor (ScriptableGameObj	*game_obj);
	void						Set_Action_ID (int id)			{ ActionID = id; }

	//
	//	Misc accessors
	//
	int						Get_Priority (void) const			{ return Priority; }
	void						Set_Priority (int priority)		{ Priority = priority; }

	float						Get_Max_Dist (void) const			{ return MaxDist; }
	void						Set_Max_Dist (float max_dist)		{ MaxDist = max_dist; }

	bool						Is_Interruptable (void) const			{ return IsInterruptable; }
	void						Set_Is_Interruptable (bool onoff)	{ IsInterruptable = onoff; }

	//
	//	Time estimation
	//
	float						Get_Conversation_Time (void);

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void						Load_Variables (ChunkLoadClass &cload);
	void						Free_Orator_List (void);

	bool						Is_Audience_In_Place (void);
	void						Check_For_Audience (void);
	void						Say_Next_Remark (void);
	void						Stop_Current_Sound (void);

	void						Notify_Monitors_On_End (ActionCompleteReason id);
	void						Notify_Monitors (int custom_event_id, int param);

	////////////////////////////////////////////////////////////////
	//	Protected data types
	////////////////////////////////////////////////////////////////
	enum
	{
		STATE_INITIALIZING				= 0,
		STATE_WAITING_FOR_AUDIENCE,
		STATE_TALKING,
		STATE_FINISHED,
		STATE_INTERRUPTED					= STATE_FINISHED
	};

	enum
	{
		MAX_MONITORS	= 10,
	};

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	int												ID;
	ConversationClass	*							Conversation;
	int												CurrentRemark;
	float												NextRemarkTimer;
	DynamicVectorClass<OratorClass *>		OratorList;
	int												ActionID;
	int												OratorSpokenBitmask;
	Vector3											CentralPos;
	GameObjReference								MonitorArray[MAX_MONITORS];
	AudibleSoundClass *							CurrentSound;

	int												State;
	float												InitializingTimeLeft;

	int												Priority;
	float												MaxDist;
	bool												IsInterruptable;
};


////////////////////////////////////////////////////////////////
//	Peek_Conversation
////////////////////////////////////////////////////////////////
inline ConversationClass *
ActiveConversationClass::Peek_Conversation (void) const
{
	return Conversation;
}


#endif //__ACTIVE_CONVERSATION_H
