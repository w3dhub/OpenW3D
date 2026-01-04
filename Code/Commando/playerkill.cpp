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
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Commando/playerkill.cpp                    $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 11/09/01 12:31p                                             $* 
 *                                                                                             * 
 *                    $Revision:: 10                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "playerkill.h"

#include <stdio.h>
#include <stdlib.h>

#include "networkobjectfactory.h"
#include "textdisplay.h"
#include "WWAudio.h"
#include "playermanager.h"
#include "matrix3d.h"
#include "cnetwork.h"
#include "gametype.h"
#include "translatedb.h"
#include "string_ids.h"
#include "apppackettypes.h"
#include "messagewindow.h"


DECLARE_NETWORKOBJECT_FACTORY(cPlayerKill, NETCLASSID_PLAYERKILL);

//-----------------------------------------------------------------------------
cPlayerKill::cPlayerKill(void)
{
	KillerId = PLAYER_ID_UNKNOWN;
	VictimId = PLAYER_ID_UNKNOWN;

	Set_App_Packet_Type(APPPACKETTYPE_PLAYERKILLEVENT);
}

//-----------------------------------------------------------------------------
void
cPlayerKill::Init(int killer_id, int victim_id)
{
	WWASSERT(cNetwork::I_Am_Server());

	KillerId = killer_id;
	VictimId = victim_id;

	//
	// This skips the local client
	//
	Set_Object_Dirty_Bit(BIT_CREATION, true);

	if (cNetwork::I_Am_Client())
	{
		Act();
	}
}

//-----------------------------------------------------------------------------
void
cPlayerKill::Act(void)
{
	if (IS_MISSION || 
		GameModeManager::Find("Menu")->Is_Active() ||
		!GameModeManager::Find("Combat")->Is_Active()) {
		return;
	}

   WideStringClass	killer_name = TRANSLATION(IDS_MP_SOMEBODY);
   WideStringClass	victim_name = TRANSLATION(IDS_MP_SOMEBODY);

   cPlayer * p_killer = NULL;
   cPlayer * p_victim = NULL;

	if (KillerId != PLAYER_ID_UNKNOWN) {
      p_killer = cPlayerManager::Find_Player(KillerId);
		if (p_killer != NULL) {
			killer_name = p_killer->Get_Name();
		}
	}

	if (VictimId != PLAYER_ID_UNKNOWN) {
      p_victim = cPlayerManager::Find_Player(VictimId);
		if (p_victim != NULL) {
			victim_name = p_victim->Get_Name();
		}
	}

   /*
	if (cPlayerManager::Is_Kill_Treasonous(p_killer, p_victim)) {

		WideStringClass formatted_text;
		formatted_text.Format(
			U_CHAR("%s %s %s\n"), 
         killer_name,
         TRANSLATION(IDS_MP_TREASON_PHRASE), 
         victim_name);
		WWASSERT(Get_Text_Display() != NULL);
		Get_Text_Display()->Print_Informational(formatted_text);

   } else {

		WideStringClass formatted_text;
		formatted_text.Format(
			U_CHAR("%s %s %s\n"), 
         killer_name,
			TRANSLATION(IDS_MP_DEFAULT_KILL_PHRASE),
         victim_name);
		WWASSERT(Get_Text_Display() != NULL);
		Get_Text_Display()->Print_Informational(formatted_text);

		if (cNetwork::I_Am_Client() && KillerId == cNetwork::Get_My_Id() && 
			KillerId != VictimId) {
			WWAudioClass::Get_Instance()->Create_Instant_Sound("My_Kill", Matrix3D(1));
		}
   }
	*/

	if (p_killer != NULL && p_victim != NULL) {
		WWASSERT(CombatManager::Get_Message_Window() != NULL);		
		
		//
		//	Determine which message to display
		//
		WideStringClass message;
		if (cPlayerManager::Is_Kill_Treasonous(p_killer, p_victim)) {
			message.Format(TRANSLATION(IDS_MP_TREASON_PHRASE));
		} else {
			message.Format(TRANSLATION(IDS_MP_DEFAULT_KILL_PHRASE));

			if (cNetwork::I_Am_Client() && KillerId == cNetwork::Get_My_Id() && 
				KillerId != VictimId) {
				WWAudioClass::Get_Instance()->Create_Instant_Sound("My_Kill", Matrix3D(1));
			}
		}

		//
		//	Display the message...
		//
		WideStringClass formatted_text;
		formatted_text.Format(U_CHAR("%s %s %s\n"), killer_name.Peek_Buffer (), message.Peek_Buffer (), victim_name.Peek_Buffer ());
		CombatManager::Get_Message_Window ()->Add_Message (formatted_text, p_killer->Get_Color ());
	}

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cPlayerKill::Export_Creation(BitStreamClass & packet)
{
	cNetEvent::Export_Creation(packet);

	packet.Add(KillerId);
	packet.Add(VictimId);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cPlayerKill::Import_Creation(BitStreamClass & packet)
{
	cNetEvent::Import_Creation(packet);

	WWASSERT(cNetwork::I_Am_Only_Client());

	packet.Get(KillerId);
	packet.Get(VictimId);

	Act();
}
