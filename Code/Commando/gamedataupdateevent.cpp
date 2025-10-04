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
 *                     $Archive:: /Commando/Code/Commando/gamedataupdateevent.cpp               $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 1/17/02 8:32p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "gamedataupdateevent.h"

#include "networkobjectfactory.h"
#include "cnetwork.h"
#include "apppackettypes.h"

DECLARE_NETWORKOBJECT_FACTORY(cGameDataUpdateEvent, NETCLASSID_GAMEDATAUPDATEEVENT);

//-----------------------------------------------------------------------------
cGameDataUpdateEvent::cGameDataUpdateEvent(void)
{
	Reset();
}

//-----------------------------------------------------------------------------
void cGameDataUpdateEvent::Reset(void)
{
	Set_App_Packet_Type(APPPACKETTYPE_GAMEDATAUPDATEEVENT);
	TimeRemainingSeconds = 0;
	//ServerIsGameplayPermitted = true;
}

//-----------------------------------------------------------------------------
void
cGameDataUpdateEvent::Init(int client_id)
{
	WWASSERT(cNetwork::I_Am_Server());
	//WWASSERT(client_id >= 0);

	Reset();

	WWASSERT(The_Game() != NULL);
	TimeRemainingSeconds = (int) The_Game()->Get_Time_Remaining_Seconds();
	//ServerIsGameplayPermitted = The_Game()->Get_Server_Is_Gameplay_Permitted();
	HostedGameNumber = The_Game()->Get_Hosted_Game_Number();

	if (client_id == -1) {
		Set_Object_Dirty_Bit(BIT_CREATION, true);
	} else {
		Set_Object_Dirty_Bit(client_id, BIT_CREATION, true);
	}
}

//-----------------------------------------------------------------------------
void
cGameDataUpdateEvent::Act(void)
{
	WWASSERT(cNetwork::I_Am_Only_Client());

	if (The_Game() != NULL && TimeRemainingSeconds > 0)
	{
		The_Game()->Set_Time_Remaining_Seconds(TimeRemainingSeconds);
		//The_Game()->Set_Server_Is_Gameplay_Permitted(ServerIsGameplayPermitted);
	}
	The_Game()->Set_Hosted_Game_Number(HostedGameNumber);

	//Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cGameDataUpdateEvent::Export_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Server());

	cNetEvent::Export_Creation(packet);

	packet.Add(TimeRemainingSeconds);
	//packet.Add(ServerIsGameplayPermitted);
	packet.Add(HostedGameNumber);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cGameDataUpdateEvent::Import_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Only_Client());

	cNetEvent::Import_Creation(packet);

	packet.Get(TimeRemainingSeconds);
	//packet.Get(ServerIsGameplayPermitted);
	packet.Get(HostedGameNumber);

	Act();

	Set_Delete_Pending();
}




