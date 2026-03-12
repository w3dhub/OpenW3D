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
 *                     $Archive:: /Commando/Code/Commando/clientbboevent.cpp                    $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 11/27/01 3:12p                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "clientbboevent.h"

#include <stdio.h>
#include <stdlib.h>

#include "networkobjectfactory.h"
#include "cnetwork.h"
#include "networkobjectmgr.h"
#include "gameobjmanager.h"
#include "apppackettypes.h"


DECLARE_NETWORKOBJECT_FACTORY(cClientBboEvent, NETCLASSID_CLIENTBBOEVENT);

//-----------------------------------------------------------------------------
cClientBboEvent::cClientBboEvent(void)
{
	SenderId = 0;
	Bbo		= 0;

	Set_App_Packet_Type(APPPACKETTYPE_CLIENTBBOEVENT);
}

//-----------------------------------------------------------------------------
void
cClientBboEvent::Init(int bbo)
{
	WWASSERT(cNetwork::I_Am_Client());
	//WWASSERT(bbo > 0);

	SenderId = cNetwork::Get_My_Id();
	Bbo		= bbo;

	Set_Network_ID(NetworkObjectMgrClass::Get_New_Client_ID());

	if (cNetwork::I_Am_Server()) {
		Act();
	} else {
		Set_Object_Dirty_Bit(0, BIT_CREATION, true);
	}
}

//-----------------------------------------------------------------------------
void
cClientBboEvent::Act(void)
{
   WWASSERT(cNetwork::I_Am_Server());

	cRemoteHost * p_rhost = cNetwork::Get_Server_Rhost(SenderId);
	if (p_rhost != NULL)	{
		p_rhost->Set_Maximum_Bps(Bbo);
	   WWDEBUG_SAY(("Client %d adjusted bbo to %d.\n", SenderId, Bbo));
	}

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cClientBboEvent::Export_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Only_Client());

	cNetEvent::Export_Creation(packet);

	WWASSERT(SenderId > 0);
	WWASSERT(Bbo > 0);

	packet.Add(SenderId);
	packet.Add(Bbo);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cClientBboEvent::Import_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Server());

	cNetEvent::Import_Creation(packet);

	packet.Get(SenderId);
	packet.Get(Bbo);

	WWASSERT(SenderId > 0);
	WWASSERT(Bbo > 0);

	Act();
}
