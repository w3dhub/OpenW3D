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
 *                     $Archive:: /Commando/Code/Commando/comnetrcvinst.cpp                   $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 2/22/02 7:00p                                               $*
 *                                                                                             *
 *                    $Revision:: 118                                                        $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "comnetrcvinst.h"

#include "gameobjmanager.h"
#include "playermanager.h"
#include "cnetwork.h"
#include "WWAudio.h"
#include "ccamera.h"
#include "gametype.h"
#include "textdisplay.h"
#include "wwprofile.h"
#include "serverfps.h"
#include "useroptions.h"
#include "ServerSettings.h"
#include "ConsoleMode.h"
#include "demosupport.h"

//-----------------------------------------------------------------------------
void	CombatNetworkReceiverInstanceClass::Print( const char *format, ... )
{
	va_list arg_list;
	va_start (arg_list, format);
	StringClass string;
	string.Format_Args( format, arg_list );

	ConsoleBox.Print_Maybe(string);
	if (!ConsoleBox.Is_Exclusive()) {
		WWASSERT( Get_Text_Display() );
		Get_Text_Display()->Print_System( string );
	}
	va_end (arg_list);
}

//-----------------------------------------------------------------------------
void	CombatNetworkReceiverInstanceClass::Print( const Vector3 & color, const char *format, ... )
{
	WWASSERT( Get_Text_Display() );
	va_list arg_list;
	va_start (arg_list, format);
	StringClass string;
	string.Format_Args( format, arg_list );
	WideStringClass ws(string, true);
	ConsoleBox.Add_Message(&ws, (Vector3*)&color);
	if (!ConsoleBox.Is_Exclusive()) {
		WWASSERT( Get_Text_Display() );
		Get_Text_Display()->Print( string, color );
	}
	va_end (arg_list);
}

//-----------------------------------------------------------------------------
void CombatNetworkReceiverInstanceClass::Server_Send_Delete_Notifications(void)
{
   WWASSERT(cNetwork::I_Am_Server());

	if (IS_SOLOPLAY) {
		return;
	}

   //
   // Figure out what data to send to each player (client)
   //
   for (
		SLNode<cPlayer> * player_node = cPlayerManager::Get_Player_Object_List()->Head();
		player_node;
		player_node = player_node->Next()) {

		cPlayer * p_player = player_node->Data();
      WWASSERT(p_player != NULL);

      int client_id = p_player->Get_Id();

		if (client_id < 0) {
			//
			// AI... no sends
			//
			continue;
		}

		if (p_player->Get_Is_Active().Is_False()) {
			continue;
		}

		if (p_player->Get_Is_In_Game().Is_False()) {
			//
			// Loading... delay send
			//
			continue;
		}

		cNetwork::Tell_Client_About_Delete_Notifications(client_id);
   }

	return ;
}

//-----------------------------------------------------------------------------
bool CombatNetworkReceiverInstanceClass::Server_Update_Dynamic_Objects(bool is_urgent)
{
   WWASSERT(cNetwork::I_Am_Server());

	if (IS_SOLOPLAY) {
		return(false);
	}

	DEMO_SECURITY_CHECK;

	//
	// Limit the maximum rate at which the server sends updates.
	// This code will result in updates at a little less than NetUpdateRate.
	// This saves bandwidth at the cost of added latency.
	//
	static DWORD last_update_time = 0;
   DWORD time_now = TIMEGETTIME();
	if (!is_urgent && (time_now - last_update_time < 1000 / (float) cUserOptions::NetUpdateRate.Get())) {
		return(false);
	}
	last_update_time = time_now;

	//
	// cRemoteHost figures out when to update priorities based on this so keep it in sync with the NetUpdateRate.
	//
	cRemoteHost::Set_Priority_Update_Rate(cUserOptions::NetUpdateRate.Get());

   //
   // TSS - bug
	// Must handle sniper... also, should use camera position
   // rather than commando position
   //

   //
   // Figure out what data to send to each player (client)
   //
   for (
		SLNode<cPlayer> * player_node = cPlayerManager::Get_Player_Object_List()->Head();
		player_node;
		player_node = player_node->Next()) {

		cPlayer * p_player = player_node->Data();
      WWASSERT(p_player != NULL);

      int client_id = p_player->Get_Id();

		if (client_id < 0) {
			//
			// AI... no sends
			//
			continue;
		}

		if (p_player->Get_Is_Active().Is_False()) {
			continue;
		}

		if (!is_urgent && p_player->Get_Is_In_Game().Is_False()) {
			//
			// Loading... delay sends
			//
			continue;
		}

		//is_urgent = true;//XXX

		if (!is_urgent) {
			//
			// Do not send to the client at a rate much higher than his framerate
			// This code will only impact if his framerate is below NetUpdateRate.
			// This code should help prevent players whose framerate bogs from
			// getting packet-swamped.
			//
			int min_delay_ms = 0;
			if (p_player->Get_Fps() > 0) {
				const float arbitrary_scale_factor = 1.0;
				min_delay_ms = (int)(arbitrary_scale_factor * 1000 / (float) p_player->Get_Fps());
			}
			int time_elapsed_ms = time_now - p_player->Get_Last_Update_Time_Ms();
			if (time_elapsed_ms < min_delay_ms) {
				continue;
			}
		}

		p_player->Set_Last_Update_Time_Ms(time_now);

      SmartGameObj * p_soldier = GameObjManager::Find_Soldier_Of_Client_ID(client_id);

		Vector3 dest_pos;
      if (p_soldier == NULL) {
			//
			// Act like the guy is far, far away.
			// He'll only receive priority 1 messages.
			//
			dest_pos.Set(Vector3(0, 0, -10000));
		} else {
         p_soldier->Get_Position(&dest_pos);

			//
			// His eyes are say 1.5 m above his feet
			//
			dest_pos.Z += 1.5;
		}

      cNetwork::Tell_Client_About_Dynamic_Objects(client_id, dest_pos);
   }
	return(true);
}

//-----------------------------------------------------------------------------
bool CombatNetworkReceiverInstanceClass::Client_Update_Dynamic_Objects(bool is_urgent)
{
   //
	// TSS092001
	// Do not send updates to the server at a rate higher than NetUpdateRate.
	// If the server framerate is below this, match the client send rate to it.
	//

	/*TSS092101*/
	WWASSERT(cNetwork::I_Am_Client());

	static DWORD last_update_time_ms = 0;
   DWORD time_now_ms = TIMEGETTIME();
	DWORD time_elapsed_ms = time_now_ms - last_update_time_ms;

	int max_updates_per_second = cUserOptions::NetUpdateRate.Get();
	WWASSERT(cServerFps::Get_Instance() != NULL);
	int server_fps = cServerFps::Get_Instance()->Get_Fps();
	if (server_fps > 0 && server_fps < cUserOptions::NetUpdateRate.Get()) {
		max_updates_per_second = server_fps;
	}

	DWORD min_delay_ms = (DWORD)(1000 / (float) max_updates_per_second);

	//is_urgent = true;//XXX

	if (is_urgent || (time_elapsed_ms >= min_delay_ms)) {
		last_update_time_ms = time_now_ms;
		cNetwork::Tell_Server_About_Dynamic_Objects();
		return(true);
	}
	return(false);
	/**/
}