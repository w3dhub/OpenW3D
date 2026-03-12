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
 *                     $Archive:: /Commando/Code/Commando/warpevent.cpp                    $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 10/10/01 7:53p                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "warpevent.h"

#include <stdio.h>
#include <stdlib.h>

#include "networkobjectfactory.h"
#include "cnetwork.h"
#include "networkobjectmgr.h"
#include "gameobjmanager.h"
#include "apppackettypes.h"
#include "playermanager.h"


DECLARE_NETWORKOBJECT_FACTORY(cWarpEvent, NETCLASSID_WARPEVENT);

//-----------------------------------------------------------------------------
cWarpEvent::cWarpEvent(void)
{
	SenderId = 0;

	Set_App_Packet_Type(APPPACKETTYPE_WARPEVENT);
}

//-----------------------------------------------------------------------------
void
cWarpEvent::Init(WideStringClass & player_name)
{
	WWASSERT(cNetwork::I_Am_Client());

	SenderId = cNetwork::Get_My_Id();
	PlayerName = player_name;

	Set_Network_ID(NetworkObjectMgrClass::Get_New_Client_ID());

	if (cNetwork::I_Am_Server()) {
		Act();
	} else {
		Set_Object_Dirty_Bit(0, BIT_CREATION, true);
	}
}

//-----------------------------------------------------------------------------
void
cWarpEvent::Act(void)
{
   WWASSERT(cNetwork::I_Am_Server());

	SoldierGameObj * p_warp_soldier = GameObjManager::Find_Soldier_Of_Client_ID(SenderId);

	if (p_warp_soldier != NULL) {

		SoldierGameObj * p_other_soldier = NULL;

		if (PlayerName.Is_Empty())
		{
			p_other_soldier = GameObjManager::Find_Different_Player_Soldier(SenderId);
		}
		else
		{
			cPlayer * p_player = cPlayerManager::Find_Player(PlayerName);
			if (p_player != NULL)
			{
				p_other_soldier = GameObjManager::Find_Soldier_Of_Client_ID(p_player->Get_Id());
			}
		}

		if (p_other_soldier != NULL) {
			Matrix3D soldier_tm = p_other_soldier->Get_Transform();
			p_warp_soldier->Set_Transform(soldier_tm);
			p_warp_soldier->Perturb_Position(2);

		   WWDEBUG_SAY(("Client %d warped.\n", SenderId));
		}
	}

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cWarpEvent::Export_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Client());

	cNetEvent::Export_Creation(packet);

	WWASSERT(SenderId > 0);

	packet.Add(SenderId);
	packet.Add_Wide_Terminated_String(PlayerName, true);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cWarpEvent::Import_Creation(BitStreamClass & packet)
{
	cNetEvent::Import_Creation(packet);

	WWASSERT(cNetwork::I_Am_Server());

	packet.Get(SenderId);
	packet.Get_Wide_Terminated_String(PlayerName.Get_Buffer(256), 256, true);

	WWASSERT(SenderId > 0);

	Act();
}
