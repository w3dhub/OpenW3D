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
 *                     $Archive:: /Commando/Code/Commando/loadingevent.cpp                    $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 12/04/01 12:30a                                             $*
 *                                                                                             *
 *                    $Revision:: 12                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "loadingevent.h"

#include <stdio.h>
#include <stdlib.h>

#include "networkobjectfactory.h"
#include "cnetwork.h"
#include "networkobjectmgr.h"
#include "playermanager.h"
#include "apppackettypes.h"
#include "gamedataupdateevent.h"


DECLARE_NETWORKOBJECT_FACTORY(cLoadingEvent, NETCLASSID_LOADINGEVENT);

//-----------------------------------------------------------------------------
cLoadingEvent::cLoadingEvent(void)
{
	SenderId		= 0;
	IsLoading	= false;

	Set_App_Packet_Type(APPPACKETTYPE_LOADINGEVENT);
}

//-----------------------------------------------------------------------------
void
cLoadingEvent::Init(bool flag)
{
	WWASSERT(cNetwork::I_Am_Client());

	SenderId		= cNetwork::Get_My_Id();
	IsLoading	= flag;

	Set_Network_ID(NetworkObjectMgrClass::Get_New_Client_ID());

	if (cNetwork::I_Am_Server()) {
		Act();
	} else {
		Set_Object_Dirty_Bit(0, BIT_CREATION, true);
	}
}

//-----------------------------------------------------------------------------
void
cLoadingEvent::Act(void)
{
   WWASSERT(cNetwork::I_Am_Server());

	cPlayer * p_player = cPlayerManager::Find_Player(SenderId);
	if (p_player != NULL)
	{
		p_player->Set_Is_In_Game(!IsLoading);
		p_player->Mark_As_Modified();

		if (p_player->Get_Is_In_Game().Is_True())
		{
			if (cNetwork::PServerConnection != NULL) {
				cNetwork::PServerConnection->Set_Rhost_Expect_Packet_Flood(p_player->Get_Id(), true);
				cNetwork::PServerConnection->Set_Rhost_Is_In_Game(p_player->Get_Id(), true);
			}

			//
			// This client has just finished loading.
			// Update him about any dynamic game data parameters.
			//
			cGameDataUpdateEvent * p_event = new cGameDataUpdateEvent();
			p_event->Init(p_player->Get_Id());
		}
	}

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cLoadingEvent::Export_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Client());

	//WWDEBUG_SAY(("cLoadingEvent::Export_Creation (%d)\n", IsLoading));

	cNetEvent::Export_Creation(packet);

	WWASSERT(SenderId > 0);

	packet.Add(SenderId);
	packet.Add(IsLoading);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cLoadingEvent::Import_Creation(BitStreamClass & packet)
{
	//WWDEBUG_SAY(("cLoadingEvent::Import_Creation\n"));

	cNetEvent::Import_Creation(packet);

	WWASSERT(cNetwork::I_Am_Server());

	packet.Get(SenderId);
	packet.Get(IsLoading);

	WWASSERT(SenderId > 0);

	Act();
}
