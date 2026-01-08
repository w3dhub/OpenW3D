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
 *                     $Archive:: /Commando/Code/Commando/bioevent.cpp                    $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 2/20/02 11:01p                                              $*
 *                                                                                             *
 *                    $Revision:: 26                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "bioevent.h"

#include <stdio.h>
#include <stdlib.h>

#include "networkobjectfactory.h"
#include "cnetwork.h"
#include "networkobjectmgr.h"
#include "evictionevent.h"
#include "playermanager.h"
#include "god.h"
#include "gametype.h"
#include "apppackettypes.h"
#include "gamedataupdateevent.h"
#include "ConsoleMode.h"
#include "winevent.h"
#include "gamespyadmin.h"

DECLARE_NETWORKOBJECT_FACTORY(cBioEvent, NETCLASSID_BIOEVENT);

//-----------------------------------------------------------------------------
cBioEvent::cBioEvent(void) :
	SenderId(0),
	TeamChoice(-1),
	ClanID(0)
{
	Set_App_Packet_Type(APPPACKETTYPE_BIOEVENT);
}

//-----------------------------------------------------------------------------
void
cBioEvent::Init(int teamChoice, unsigned int clanID)
{
	WWASSERT(cNetwork::I_Am_Client());

	SenderId		= cNetwork::Get_My_Id();
	Nickname		= cNetInterface::Get_Nickname();
	TeamChoice	= teamChoice;
	ClanID		= clanID;

	strcpy(MapName, The_Game()->Get_Map_Name().Peek_Buffer());

	Set_Network_ID(NetworkObjectMgrClass::Get_New_Client_ID());

	if (cNetwork::I_Am_Server()) {
		Act();
	} else {
		Set_Object_Dirty_Bit(0, BIT_CREATION, true);
	}
}

//-----------------------------------------------------------------------------
void
cBioEvent::Act(void)
{
   WWASSERT(cNetwork::I_Am_Server());

   //
   // This is where we validate a player. If we don't want him to play,
   // for whichever reason, after learning all about him, evict him here.
   // Note that the server has already seen the player name in
	// cNetwork::Application_Acceptance_Handler.
	// Now the server will see additional data.
	//

	bool is_accepted = true;
	EvictionCodeEnum eviction_code = EVICTION_POOR_BANDWIDTH;

   if (is_accepted) {
		//
		// Tell the new guy about all existing players
		//
		for (SLNode<cPlayer> * player_node = cPlayerManager::Get_Player_Object_List()->Head();
			player_node;
			player_node = player_node->Next()) {

			cPlayer * p_player = player_node->Data();
			WWASSERT(p_player != NULL);

			cNetwork::Send_Object_Update(p_player, SenderId);
		}

		//GAMESPY
		if (cGameSpyAdmin::Is_Gamespy_Game() &&
			cGameSpyAdmin::Is_Nickname_Collision(Nickname))	{

			WideStringClass new_nickname;
			int count = 1;
			do {
				new_nickname.Format(U_CHAR("%s(%d)"), Nickname, count++);
			} while (cGameSpyAdmin::Is_Nickname_Collision(new_nickname));

			Nickname = new_nickname;
		}

		cPlayer * p_player = cGod::Create_Player(SenderId, Nickname, TeamChoice, ClanID);
		WWASSERT(p_player != NULL);

		if (!IS_SOLOPLAY) {
			//
			// Record his IP address for diagnostic purposes
			//
			WWASSERT(cNetwork::Get_Server_Rhost(SenderId) != NULL);
			struct sockaddr_in & address = cNetwork::Get_Server_Rhost(SenderId)->Get_Address();
			p_player->Set_Ip_Address(address.sin_addr.s_addr);

			//
			// Tell the remote host object to expect a lot of temporary extra bandwidth.
			//
			if (cNetwork::PServerConnection != NULL) {
				cNetwork::PServerConnection->Set_Rhost_Expect_Packet_Flood(p_player->Get_Id(), true);
			}

			//
			// Update him about any dynamic game data parameters.
			//
			WWDEBUG_SAY(("BioEvent acting\n"));
			cGameDataUpdateEvent * p_event = new cGameDataUpdateEvent();
			p_event->Init(p_player->Get_Id());

			StringClass str(128, true);
			Nickname.Convert_To(str);
			ConsoleBox.Print_Maybe("Player %s joined the game\n", str.Peek_Buffer());

			//
			// If we (the server) are already in intermission mode then deactivate the player until the next map starts. The player
			// will have already loaded the next map. ST - 1/17/2002 7:43PM
			//
			if (The_Game()->IsIntermission.Is_True()) {

				//
				// If the new player has the old map loaded (which can happen if the intermission starts after the client has
				// started loading the old map) then send him a wineven so he knows to load the next map.
				//
				if (stricmp(MapName, The_Game()->Get_Map_Name().Peek_Buffer()) != 0) {
					cWinEvent * p_win = new cWinEvent;
					p_win->Init(The_Game()->Get_Winner_ID(), SenderId, The_Game()->Is_Map_Cycle_Over());
					cNetwork::Send_Object_Update(p_win, SenderId);
				} else {
					p_player->Set_Is_In_Game(false);
					p_player->Set_Is_Waiting_For_Intermission(true);
				}
			}

		}
   } else {
		cEvictionEvent * p_event = new cEvictionEvent;
		p_event->Init(SenderId, eviction_code);

		cNetwork::Flush();

      cNetwork::Server_Kill_Connection(SenderId);
   }

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cBioEvent::Export_Creation(BitStreamClass & packet)
{
	cNetEvent::Export_Creation(packet);

	WWASSERT(cNetwork::I_Am_Only_Client());

	WWASSERT(SenderId > 0);

	packet.Add(SenderId);
	packet.Add_Wide_Terminated_String(Nickname);
	packet.Add(TeamChoice);
	packet.Add(ClanID);
	packet.Add_Terminated_String(MapName, false);

	WWDEBUG_SAY(("cBioEvent sent\n"));

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cBioEvent::Import_Creation(BitStreamClass & packet)
{
	cNetEvent::Import_Creation(packet);

	WWASSERT(cNetwork::I_Am_Server());

	packet.Get(SenderId);
	packet.Get_Wide_Terminated_String(Nickname.Get_Buffer(256), 256, true);
	packet.Get(TeamChoice);
	packet.Get(ClanID);

	packet.Get_Terminated_String(MapName, sizeof(MapName), false);

	WWASSERT(SenderId > 0);

	WWDEBUG_SAY(("cBioEvent recd\n"));

	Act();
}