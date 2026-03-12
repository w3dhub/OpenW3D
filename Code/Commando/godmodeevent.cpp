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
 *                     $Archive:: /Commando/Code/Commando/godmodeevent.cpp                    $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 12/24/01 2:10p                                              $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "godmodeevent.h"

#include <stdio.h>
#include <stdlib.h>

#include "networkobjectfactory.h"
#include "cnetwork.h"
#include "networkobjectmgr.h"
#include "playermanager.h"
#include "gameobjmanager.h"
#include "devoptions.h"
#include "textdisplay.h"
#include "apppackettypes.h"
#include "realcrc.h"


DECLARE_NETWORKOBJECT_FACTORY(cGodModeEvent, NETCLASSID_GODMODEEVENT);

//-----------------------------------------------------------------------------
cGodModeEvent::cGodModeEvent(void)
{
	SenderId	= 0;

	Set_App_Packet_Type(APPPACKETTYPE_GODMODEEVENT);
}

//-----------------------------------------------------------------------------
void
cGodModeEvent::Init(StringClass & password)
{
	WWASSERT(cNetwork::I_Am_Client());

	SenderId	= cNetwork::Get_My_Id();
	Password = password;

	Set_Network_ID(NetworkObjectMgrClass::Get_New_Client_ID());

	if (cNetwork::I_Am_Server()) {
		Act();
	} else {
		Set_Object_Dirty_Bit(0, BIT_CREATION, true);
	}
}

//-----------------------------------------------------------------------------
void
cGodModeEvent::Act(void)
{
   WWASSERT(cNetwork::I_Am_Server());

#ifdef WWDEBUG

	if (!Password.Is_Empty())
	{
		WWDEBUG_SAY(("mike %u\n", CRC_Stringi(Password)));
	}

	if (cDevOptions::IBelieveInGod.Is_True() || CRC_Stringi(Password) == 4014842490)
	{
		cPlayer * p_player = cPlayerManager::Find_Player(SenderId);

		if (p_player != NULL) {

			p_player->Invulnerable.Toggle();
			p_player->Mark_As_Modified();

			if (p_player->Invulnerable.Is_True()) {

				//
				// Free weapons and keys!
				//

				SoldierGameObj * p_soldier = GameObjManager::Find_Soldier_Of_Client_ID(p_player->Get_Id());

				if (p_soldier != NULL) {

					p_soldier->Give_All_Weapons();

					for (int i = 0; i < 32; i++) {
						p_soldier->Give_Key(i);
					}
				}

				//
				// Shitloads of cash !
				//
				p_player->Increment_Money(100000);
			}
		}
	}
	else
	{
		if (Get_Text_Display()) {
			WWASSERT(Get_Text_Display() != NULL);
			Get_Text_Display()->Print_System("God status toggle request ignored due to your atheism.");
		}
	}
#endif // WWDEBUG

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cGodModeEvent::Export_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Client());

	cNetEvent::Export_Creation(packet);

	WWASSERT(SenderId > 0);

	packet.Add(SenderId);
	packet.Add_Terminated_String((LPCSTR) Password, true);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cGodModeEvent::Import_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Server());

	cNetEvent::Import_Creation(packet);

	packet.Get(SenderId);
	packet.Get_Terminated_String(Password.Get_Buffer(256), 256, true);

	WWASSERT(SenderId > 0);

	Act();
}
