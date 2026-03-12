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
 *                     $Archive:: /Commando/Code/Commando/clientgoodbyeevent.cpp               $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 10/10/01 4:53p                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "clientgoodbyeevent.h"

#include "networkobjectfactory.h"
#include "cnetwork.h"
#include "networkobjectmgr.h"
#include "apppackettypes.h"

//
// TSS2001 Problem - we have lost the unreliable multiblast effect for this message
//

DECLARE_NETWORKOBJECT_FACTORY(cClientGoodbyeEvent, NETCLASSID_CLIENTGOODBYEEVENT);

//-----------------------------------------------------------------------------
cClientGoodbyeEvent::cClientGoodbyeEvent(void)
{
	WWDEBUG_SAY(("cClientGoodbyeEvent::cClientGoodbyeEvent\n"));

	SenderId = -1;

	Set_App_Packet_Type(APPPACKETTYPE_CLIENTGOODBYEEVENT);
}

//-----------------------------------------------------------------------------
void
cClientGoodbyeEvent::Init(void)
{
	WWDEBUG_SAY(("cClientGoodbyeEvent::Init\n"));

	WWASSERT(cNetwork::I_Am_Client());

	SenderId	= cNetwork::Get_My_Id();

	Set_Network_ID(NetworkObjectMgrClass::Get_New_Client_ID());

	if (cNetwork::I_Am_Server()) {
		Act();
	} else {
		Set_Object_Dirty_Bit(0, BIT_CREATION, true);
	}
}

//-----------------------------------------------------------------------------
void
cClientGoodbyeEvent::Act(void)
{
	WWDEBUG_SAY(("cClientGoodbyeEvent::Act\n"));

	WWASSERT(cNetwork::I_Am_Server());

	WWASSERT(SenderId > 0);

	Set_Delete_Pending();

	//
	// TSS092101 - crashy.
	// This cleanup probably deletes *this* object...
	//
	/**/
	cNetwork::Server_Kill_Connection(SenderId);
	cNetwork::Cleanup_After_Client(SenderId);
	/**/
}

//-----------------------------------------------------------------------------
void
cClientGoodbyeEvent::Export_Creation(BitStreamClass & packet)
{
	WWDEBUG_SAY(("cClientGoodbyeEvent::Export_Creation\n"));

	WWASSERT(cNetwork::I_Am_Only_Client());

	cNetEvent::Export_Creation(packet);

	WWASSERT(SenderId > 0);

	packet.Add(SenderId);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cClientGoodbyeEvent::Import_Creation(BitStreamClass & packet)
{
	WWDEBUG_SAY(("cClientGoodbyeEvent::Import_Creation\n"));

	WWASSERT(cNetwork::I_Am_Server());

	cNetEvent::Import_Creation(packet);

	packet.Get(SenderId);

	WWASSERT(SenderId > 0);

	Act();
}
