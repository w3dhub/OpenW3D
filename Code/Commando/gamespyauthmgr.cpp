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
 *                     $Archive:: /Commando/Code/Commando/gamespyauthmgr.cpp                    $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                     $*
 *                                                                                             *
 *                     $Modtime:: 2/22/02 5:41p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "gamespyauthmgr.h"

#include "gamespyscchallengeevent.h"
#include "playermanager.h"
#include "cnetwork.h"
#include "gamespyadmin.h"
#include "GameSpy_QnR.h"
#include "CDKeyAuth.h"
#include "sctextobj.h"
#include "translatedb.h"
#include "string_ids.h"

//
// Class statics
//

//-----------------------------------------------------------------------------
void 
cGameSpyAuthMgr::Think
(
	void
)
{
	WWASSERT(cNetwork::I_Am_Server());
	WWASSERT(cGameSpyAdmin::Is_Gamespy_Game());

	//
	// Process all the players
	//

	WWASSERT(cPlayerManager::Get_Player_Object_List() != NULL);

	for (
		SLNode<cPlayer> * player_node = cPlayerManager::Get_Player_Object_List()->Head();
		player_node != NULL;
		player_node = player_node->Next()) 
	{
		cPlayer * p_player = player_node->Data();
		WWASSERT(p_player != NULL);

		if (!p_player->Is_Alive_And_Kicking())
		{
			continue;
		}

		if (GameSpyQnR.IsOffline()) {
			// Without master/heartbeat servers, skip external auth and let players in.
			p_player->Set_GameSpy_Auth_State(GAMESPY_AUTH_STATE_ACCEPTED);
			continue;
		}

		switch (p_player->Get_GameSpy_Auth_State())
		{
			case GAMESPY_AUTH_STATE_INITIAL:
			{
				if (cNetwork::I_Am_Client() && p_player->Get_Id() == cNetwork::Get_My_Id())
				{
					//
					// Do not bother authenticating the server's local client.
					//
					p_player->Set_GameSpy_Auth_State(GAMESPY_AUTH_STATE_ACCEPTED);
				}
				else
				{
					//
					// Challenge this player
					//
					StringClass challenge_string;
					challenge_string.Format("%s", CCDKeyAuth::GenChallenge(8));

					p_player->Set_GameSpy_Auth_State(GAMESPY_AUTH_STATE_CHALLENGED);
					p_player->Set_GameSpy_Challenge_String(challenge_string);
					
					cGameSpyScChallengeEvent * p_event = new cGameSpyScChallengeEvent;
					p_event->Init(p_player->Get_Id(), challenge_string);
				}
				break;
			}

			case GAMESPY_AUTH_STATE_CHALLENGED:
			{
				DWORD time_now_ms = TIMEGETTIME();
				if (time_now_ms - p_player->Get_GameSpy_Auth_State_Entry_Time_Ms() >
					CHALLENGE_RESPONSE_TIMEOUT_MS)
				{
					//
					// We challenged this guy and he hasn't responded in the required
					// time. We are going to have to boot him out.
					//
					int player_id = p_player->Get_Id();

					WWDEBUG_SAY(("cGameSpyAuthMgr::Think: player %d timed out on challenge response.\n", 
						player_id));

					//p_player->Set_GameSpy_Auth_State(GAMESPY_AUTH_STATE_REJECTING);
					//Evict_Player(player_id);
					cGameSpyAuthMgr::Initiate_Auth_Rejection(player_id);
				}
				break;
			}

			case GAMESPY_AUTH_STATE_VALIDATING:
			{ 
				//
				// At this stage I am not implementing any timeout for GameSpy to 
				// authenticate this user.
				//
				break;
			}

			case GAMESPY_AUTH_STATE_ACCEPTED:
			{
				//
				// The only peaceful, harmonious state.
				//
				break;
			}

			case GAMESPY_AUTH_STATE_REJECTING:
			{
				DWORD time_now_ms = TIMEGETTIME();
				if (time_now_ms - p_player->Get_GameSpy_Auth_State_Entry_Time_Ms() >
					REJECTION_DELAY_MS)
				{
					p_player->Set_GameSpy_Auth_State(GAMESPY_AUTH_STATE_REJECTED);
					Evict_Player(p_player->Get_Id());
				}
				break;
			}

			case GAMESPY_AUTH_STATE_REJECTED:
			{
				//
				// Do nothing
				//
				break;
			}

			default:
				DIE;
		}
	}
}

//-----------------------------------------------------------------------------
void 
cGameSpyAuthMgr::Initiate_Auth_Rejection
(
	int player_id
)
{
	WWDEBUG_SAY(("cGameSpyAuthMgr::Initiate_Auth_Rejection(%d)\n", player_id));

	WWASSERT(cNetwork::I_Am_Server());
	WWASSERT(cGameSpyAdmin::Is_Gamespy_Game());
	
	cPlayer * p_player = cPlayerManager::Find_Player(player_id);
	
	if (p_player != NULL) 
	{
		cScTextObj * p_message = new cScTextObj;
		//p_message->Init(U_CHAR("CD Authentication failed. Please quit."), TEXT_MESSAGE_PRIVATE, true, HOST_TEXT_SENDER, player_id);
		p_message->Init(TRANSLATE(IDS_MP_CD_AUTH_FAILED), TEXT_MESSAGE_PRIVATE, true, HOST_TEXT_SENDER, player_id);

		p_player->Set_GameSpy_Auth_State(GAMESPY_AUTH_STATE_REJECTING);
	}
}

//-----------------------------------------------------------------------------
void 
cGameSpyAuthMgr::Evict_Player
(
	int player_id
)
{
	WWDEBUG_SAY(("cGameSpyAuthMgr::Evict_Player(%d)\n", player_id));

	WWASSERT(cNetwork::I_Am_Server());
	WWASSERT(cGameSpyAdmin::Is_Gamespy_Game());

	//
	// The behaviour presently is fairly brutal - no feedback to evicted player.
	// This is fine if he is a cracker, but other cases exist. TODO_AUTH.
	//

	cNetwork::Server_Kill_Connection(player_id);
	cNetwork::Cleanup_After_Client(player_id);
}

//-----------------------------------------------------------------------------
const char * 
cGameSpyAuthMgr::Describe_Auth_State
(
	GAMESPY_AUTH_STATE_ENUM state
)
{
	switch (state)
	{
		case GAMESPY_AUTH_STATE_INITIAL:			return "INITIAL";
		case GAMESPY_AUTH_STATE_CHALLENGED:		return "CHALLENGED";
		case GAMESPY_AUTH_STATE_VALIDATING:		return "VALIDATING";
		case GAMESPY_AUTH_STATE_ACCEPTED:		return "ACCEPTED";
		case GAMESPY_AUTH_STATE_REJECTING:		return "REJECTING";
		case GAMESPY_AUTH_STATE_REJECTED:		return "REJECTED";
		default: DIE; return "bollocks"; // avoid compiler warning;
	}
}
