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
 *                     $Archive:: /Commando/Code/Commando/cspingrequestevent.cpp                    $*
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

#include "cspingrequestevent.h"

#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "networkobjectfactory.h"
#include "cnetwork.h"
#include "networkobjectmgr.h"
#include "apppackettypes.h"
#include "scpingresponseevent.h"
#include "wwprofile.h"


DECLARE_NETWORKOBJECT_FACTORY(cCsPingRequestEvent, NETCLASSID_CSPINGREQUESTEVENT);

//-----------------------------------------------------------------------------
cCsPingRequestEvent::cCsPingRequestEvent(void)
{
	SenderId		= -1;
	PingNumber	= -1;

	Set_App_Packet_Type(APPPACKETTYPE_CSPINGREQUESTEVENT);
}

//-----------------------------------------------------------------------------
void
cCsPingRequestEvent::Init(int ping_number)
{
	//WWDEBUG_SAY(("cCsPingRequestEvent::Init at frame %d\n", WWProfileManager::Get_Frame_Count_Since_Reset()));

	WWASSERT(ping_number >= 0);

	WWASSERT(cNetwork::I_Am_Only_Client());

	SenderId		= cNetwork::Get_My_Id();
	PingNumber	= ping_number;

	Set_Network_ID(NetworkObjectMgrClass::Get_New_Client_ID());

	Set_Object_Dirty_Bit(0, BIT_CREATION, true);
}

//-----------------------------------------------------------------------------
void
cCsPingRequestEvent::Act(void)
{
	WWASSERT(cNetwork::I_Am_Server());

	cScPingResponseEvent * p_event = new cScPingResponseEvent;
	p_event->Init(SenderId, PingNumber);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cCsPingRequestEvent::Export_Creation(BitStreamClass & packet)
{
	//WWDEBUG_SAY(("cCsPingRequestEvent::Export at frame %d\n", WWProfileManager::Get_Frame_Count_Since_Reset()));

	WWASSERT(cNetwork::I_Am_Only_Client());

	cNetEvent::Export_Creation(packet);

	packet.Add(SenderId);
	packet.Add(PingNumber);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cCsPingRequestEvent::Import_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Server());

	cNetEvent::Import_Creation(packet);

	packet.Get(SenderId);
	packet.Get(PingNumber);

	Act();
}
