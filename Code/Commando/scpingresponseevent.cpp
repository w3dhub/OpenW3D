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
 *                     $Archive:: /Commando/Code/Commando/scpingresponseevent.cpp                    $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 10/09/01 2:05p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "scpingresponseevent.h"

#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "networkobjectfactory.h"
#include "cnetwork.h"
#include "networkobjectmgr.h"
#include "apppackettypes.h"
#include "clientpingmanager.h"
#include "wwprofile.h"


DECLARE_NETWORKOBJECT_FACTORY(cScPingResponseEvent, NETCLASSID_SCPINGRESPONSEEVENT);

//-----------------------------------------------------------------------------
cScPingResponseEvent::cScPingResponseEvent(void)
{
	PingNumber	= -1;

	Set_App_Packet_Type(APPPACKETTYPE_SCPINGRESPONSEEVENT);
}

//-----------------------------------------------------------------------------
void
cScPingResponseEvent::Init(int sender_id, int ping_number)
{
	//WWDEBUG_SAY(("cScPingResponseEvent::Init at frame %d\n", WWProfileManager::Get_Frame_Count_Since_Reset()));

	WWASSERT(sender_id >= 0);
	WWASSERT(ping_number >= 0);

	WWASSERT(cNetwork::I_Am_Server());

	PingNumber	= ping_number;

	Set_Object_Dirty_Bit(sender_id, BIT_CREATION, true);
}

//-----------------------------------------------------------------------------
void
cScPingResponseEvent::Act(void)
{
	WWASSERT(cNetwork::I_Am_Only_Client());

	cClientPingManager::Response_Received(PingNumber);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cScPingResponseEvent::Export_Creation(BitStreamClass & packet)
{
	//WWDEBUG_SAY(("cScPingResponseEvent::Export_Creation at frame %d\n", WWProfileManager::Get_Frame_Count_Since_Reset()));

	WWASSERT(cNetwork::I_Am_Server());

	cNetEvent::Export_Creation(packet);

	packet.Add(PingNumber);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cScPingResponseEvent::Import_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Only_Client());

	cNetEvent::Import_Creation(packet);

	packet.Get(PingNumber);

	Act();
}
