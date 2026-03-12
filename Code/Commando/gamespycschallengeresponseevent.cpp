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
 *                     $Archive:: /Commando/Code/Commando/gamespycschallengeresponseevent.cpp               $*
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/22/02 12:32p                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "gamespycschallengeresponseevent.h"

#include "networkobjectfactory.h"
#include "cnetwork.h"
#include "apppackettypes.h"
#include "networkobjectmgr.h"
#include "playermanager.h"
#include "CDKeyAuth.h"


DECLARE_NETWORKOBJECT_FACTORY(cGameSpyCsChallengeResponseEvent, NETCLASSID_GAMESPYCSCHALLENGERESPONSEEVENT);

//-----------------------------------------------------------------------------
cGameSpyCsChallengeResponseEvent::cGameSpyCsChallengeResponseEvent
(
	void
)
{
	ClientId = -1;
	ChallengeResponseString.Format("");

	Set_App_Packet_Type(APPPACKETTYPE_GAMESPYCSCHALLENGERESPONSEEVENT);
}

//-----------------------------------------------------------------------------
void
cGameSpyCsChallengeResponseEvent::Init
(
	StringClass & challenge_string
)
{
	WWDEBUG_SAY(("cGameSpyCsChallengeResponseEvent::Init\n"));

	WWASSERT(cNetwork::I_Am_Client());

	ClientId = cNetwork::Get_My_Id();

	//TODO_AUTH - construct ChallengeResponseString here from challenge_string,
	// CD-KEY, random value.
	CCDKeyAuth::AuthSerial(challenge_string.Peek_Buffer(), ChallengeResponseString);

	// Get Serial Number

	Set_Network_ID(NetworkObjectMgrClass::Get_New_Client_ID());

	if (cNetwork::I_Am_Server()) {
		Act();
	} else {
		Set_Object_Dirty_Bit(0, BIT_CREATION, true);
	}
}

//-----------------------------------------------------------------------------
void
cGameSpyCsChallengeResponseEvent::Act
(
	void
)
{
	WWDEBUG_SAY(("cGameSpyCsChallengeResponseEvent::Act\n"));

   WWASSERT(cNetwork::I_Am_Server());

	cPlayer * p_player = cPlayerManager::Find_Player(ClientId);
	if (p_player != NULL)
	{
		WWDEBUG_SAY(("  Validating client %d against validation server.\n", ClientId));

		CCDKeyAuth::AuthenticateUser(ClientId, p_player->Get_Ip_Address(),
			p_player->Get_GameSpy_Challenge_String().Peek_Buffer(),
			ChallengeResponseString.Peek_Buffer());

		if (ChallengeResponseString.Get_Length() > 31) {
			StringClass tmpstr = ChallengeResponseString;
			if (tmpstr.Get_Length() > 32) tmpstr.Erase(32, tmpstr.Get_Length()-32);
			p_player->Set_GameSpy_Hash_Id(tmpstr);
		}
		p_player->Set_GameSpy_Auth_State(GAMESPY_AUTH_STATE_VALIDATING);
	}

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cGameSpyCsChallengeResponseEvent::Export_Creation
(
	BitStreamClass & packet
)
{
	WWDEBUG_SAY(("cGameSpyCsChallengeResponseEvent::Export_Creation\n"));

   WWASSERT(cNetwork::I_Am_Only_Client());

	cNetEvent::Export_Creation(packet);

	packet.Add(ClientId);
	WWASSERT(ChallengeResponseString.Get_Length() <= MAX_CHALLENGE_RESPONSE_STRING_LENGTH);
	packet.Add_Terminated_String(ChallengeResponseString.Peek_Buffer());

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cGameSpyCsChallengeResponseEvent::Import_Creation
(
	BitStreamClass & packet
)
{
	WWDEBUG_SAY(("cGameSpyCsChallengeResponseEvent::Import_Creation\n"));

	WWASSERT(cNetwork::I_Am_Server());

	cNetEvent::Import_Creation(packet);

	packet.Get(ClientId);
	packet.Get_Terminated_String(
		ChallengeResponseString.Get_Buffer(MAX_CHALLENGE_RESPONSE_STRING_LENGTH), MAX_CHALLENGE_RESPONSE_STRING_LENGTH);

	Act();
}
