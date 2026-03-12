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
 *                     $Archive:: /Commando/Code/Commando/gamespyscchallengeevent.cpp               $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 2/21/02 5:41p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "gamespyscchallengeevent.h"

#include "networkobjectfactory.h"
#include "cnetwork.h"
#include "apppackettypes.h"
#include "gamespycschallengeresponseevent.h"

DECLARE_NETWORKOBJECT_FACTORY(cGameSpyScChallengeEvent, NETCLASSID_GAMESPYSCCHALLENGEEVENT);

//-----------------------------------------------------------------------------
cGameSpyScChallengeEvent::cGameSpyScChallengeEvent
(
	void
)
{
	ChallengeString.Format("");

	Set_App_Packet_Type(APPPACKETTYPE_GAMESPYSCCHALLENGEEVENT);
}

//-----------------------------------------------------------------------------
void
cGameSpyScChallengeEvent::Init
(
	int				client_id,
	StringClass &	challenge_string
)
{
	WWDEBUG_SAY(("cGameSpyScChallengeEvent::Init for client %d\n", client_id));

	WWASSERT(cNetwork::I_Am_Server());
	WWASSERT(client_id >= 0);
	WWASSERT(challenge_string.Get_Length() > 0);
	WWASSERT(challenge_string.Get_Length() <= MAX_CHALLENGE_STRING_LENGTH);

	ChallengeString = challenge_string;

	if (cNetwork::I_Am_Client() && cNetwork::Get_My_Id() == client_id)
	{
		Act();
	}
	else
	{
		Set_Object_Dirty_Bit(client_id, BIT_CREATION, true);
	}
}

//-----------------------------------------------------------------------------
void
cGameSpyScChallengeEvent::Act
(
	void
)
{
	WWDEBUG_SAY(("cGameSpyScChallengeEvent::Act\n"));

	WWASSERT(cNetwork::I_Am_Client());

	//
	// Respond to the server's challenge
	//
	cGameSpyCsChallengeResponseEvent * p_event = new cGameSpyCsChallengeResponseEvent;
	p_event->Init(ChallengeString);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cGameSpyScChallengeEvent::Export_Creation
(
	BitStreamClass & packet
)
{
	WWDEBUG_SAY(("cGameSpyScChallengeEvent::Export_Creation\n"));

	WWASSERT(cNetwork::I_Am_Server());

	cNetEvent::Export_Creation(packet);

	WWASSERT(ChallengeString.Get_Length() <= MAX_CHALLENGE_STRING_LENGTH);
	packet.Add_Terminated_String(ChallengeString.Peek_Buffer());

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cGameSpyScChallengeEvent::Import_Creation
(
	BitStreamClass & packet
)
{
	WWDEBUG_SAY(("cGameSpyScChallengeEvent::Import_Creation\n"));

	WWASSERT(cNetwork::I_Am_Only_Client());

	cNetEvent::Import_Creation(packet);

	packet.Get_Terminated_String(
		ChallengeString.Get_Buffer(MAX_CHALLENGE_STRING_LENGTH), MAX_CHALLENGE_STRING_LENGTH);

	Act();
}

