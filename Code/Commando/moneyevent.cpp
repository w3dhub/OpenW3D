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
 *                     $Archive:: /Commando/Code/Commando/moneyevent.cpp                    $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 11/10/01 1:03p                                              $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "moneyevent.h"

#include <stdio.h>
#include <stdlib.h>

#include "networkobjectfactory.h"
#include "cnetwork.h"
#include "networkobjectmgr.h"
#include "gameobjmanager.h"
#include "playermanager.h"
#include "apppackettypes.h"


DECLARE_NETWORKOBJECT_FACTORY(cMoneyEvent, NETCLASSID_MONEYEVENT);

//-----------------------------------------------------------------------------
cMoneyEvent::cMoneyEvent(void)
{
	SenderId = 0;
	Amount = 0;

	Set_App_Packet_Type(APPPACKETTYPE_MONEYEVENT);
}

//-----------------------------------------------------------------------------
void
cMoneyEvent::Init(int amount)
{
	WWASSERT(cNetwork::I_Am_Client());

	SenderId = cNetwork::Get_My_Id();
	Amount = amount;

	Set_Network_ID(NetworkObjectMgrClass::Get_New_Client_ID());

	if (cNetwork::I_Am_Server()) {
		Act();
	} else {
		Set_Object_Dirty_Bit(0, BIT_CREATION, true);
	}
}

//-----------------------------------------------------------------------------
void
cMoneyEvent::Act(void)
{
   WWASSERT(cNetwork::I_Am_Server());

	cPlayer * p_player = cPlayerManager::Find_Player(SenderId);

	if (p_player != NULL && p_player->Invulnerable.Is_True()) {

		p_player->Set_Money(Amount);

	   WWDEBUG_SAY(("Client %d setting money to %d.\n", SenderId, Amount));
	}

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cMoneyEvent::Export_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Client());

	cNetEvent::Export_Creation(packet);

	WWASSERT(SenderId > 0);

	packet.Add(SenderId);
	packet.Add(Amount);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cMoneyEvent::Import_Creation(BitStreamClass & packet)
{
	cNetEvent::Import_Creation(packet);

	WWASSERT(cNetwork::I_Am_Server());

	packet.Get(SenderId);
	packet.Get(Amount);

	WWASSERT(SenderId > 0);

	Act();
}





