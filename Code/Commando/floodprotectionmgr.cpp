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
 *                 Project Name : commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/floodprotectionmgr.cpp              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/25/02 1:22p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "floodprotectionmgr.h"
#include "combat.h"
#include "messagewindow.h"
#include "translatedb.h"
#include "string_ids.h"
#include "systimer.h"


//////////////////////////////////////////////////////////////////////
//	Static member initialization
//////////////////////////////////////////////////////////////////////
SimpleDynVecClass<FloodProtectionMgrClass::FLOOD_ENTRY>	FloodProtectionMgrClass::FloodList;


//////////////////////////////////////////////////////////////////////
//
//	Reset
//
//////////////////////////////////////////////////////////////////////
void
FloodProtectionMgrClass::Reset (void)
{
	FloodList.Delete_All ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Decay_Old_Entries
//
//////////////////////////////////////////////////////////////////////
void
FloodProtectionMgrClass::Decay_Old_Entries (void)
{
	const int DECAY_TIME	= 15000;

	uint32 curr_time = ::TIMEGETTIME ();

	//
	//	Loop over all the entries in the list
	//
	int count = FloodList.Count ();
	for (int index = 0; index < count; index ++) {

		//
		//	Has this entry expired?
		//
		uint32 time = FloodList[index].time;
		if ((curr_time - time) >= DECAY_TIME) {
			FloodList.Delete (index);
			index --;
			count --;
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Detect_Flooding
//
//////////////////////////////////////////////////////////////////////
bool
FloodProtectionMgrClass::Detect_Flooding (const unichar_t * /* text */)
{
	//
	//	First, remove any old entries
	//
	Decay_Old_Entries ();

	//
	//	The user is "flooding" if they have sent more then
	// 10 messages in the last 15 seconds
	//
	const int FLOOD_THRESHOLD	= 5;
	bool retval = (FloodList.Count () >= FLOOD_THRESHOLD);

	//
	//	If the user is spamming, then let them know it
	//
	if (retval) {
		Display_Flood_Message ();
	} else {

		//
		//	Add a new entry to the list
		//
		FLOOD_ENTRY entry;
		entry.time		= ::TIMEGETTIME ();
		entry.text_len	= 0;
		FloodList.Add (entry);
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Display_Flood_Message
//
//////////////////////////////////////////////////////////////////////
void
FloodProtectionMgrClass::Display_Flood_Message (void)
{
	MessageWindowClass *message_window = CombatManager::Get_Message_Window ();
	if (message_window != NULL) {

		//
		//	Display a message letting the user know they've been caught spamming
		//
		message_window->Add_Message (TRANSLATE (IDS_FLOOD_MSG), Vector3 (1.0F, 1.0F, 1.0F));
	}

	return ;
}
