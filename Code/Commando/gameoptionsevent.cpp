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
 *                     $Archive:: /Commando/Code/Commando/gameoptionsevent.cpp               $*
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/29/02 4:36p                                               $*
 *                                                                                             *
 *                    $Revision:: 24                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "gameoptionsevent.h"

#include <stdio.h>
#include <stdlib.h>

#include "networkobjectfactory.h"
#include "gamemode.h"
#include "cnetwork.h"
#include "gametype.h"
#include "dialogbase.h"
#include "dialogmgr.h"
#include "DlgMPConnect.h"
#include "renegadedialog.h"
#include "apppackettypes.h"
#include "modpackagemgr.h"
#include "specialbuilds.h"
#include "DlgMPConnectionRefused.h"
#include "translatedb.h"
#include "string_ids.h"
#include <wwlib/realcrc.h>

DECLARE_NETWORKOBJECT_FACTORY(cGameOptionsEvent, NETCLASSID_GAMEOPTIONSEVENT);

//-----------------------------------------------------------------------------
cGameOptionsEvent::cGameOptionsEvent(void)
{
	Set_App_Packet_Type(APPPACKETTYPE_GAMEOPTIONSEVENT);

	HostedGameNumber = -1;
}

//-----------------------------------------------------------------------------
void
cGameOptionsEvent::Init(int client_id)
{
	WWASSERT(cNetwork::I_Am_Server());

	WWASSERT(The_Game() != NULL);
	HostedGameNumber = The_Game()->Get_Hosted_Game_Number();

	Set_Object_Dirty_Bit(client_id, BIT_CREATION, true);
}

//-----------------------------------------------------------------------------
void
cGameOptionsEvent::Act(void)
{
	WWASSERT(cNetwork::I_Am_Only_Client());
	WWASSERT(The_Game() != NULL);

	The_Game()->Set_Hosted_Game_Number(HostedGameNumber);

	if (!IS_SOLOPLAY) {
		DialogBaseClass* dialog = DialogMgrClass::Find_Dialog((int)RenegadeDialogID::IDD_MULTIPLAY_CONNECTING);

		if (dialog != NULL) {
	 		((DlgMPConnect*)dialog)->Connected(The_Game());
		}
	}
}

//-----------------------------------------------------------------------------
void
cGameOptionsEvent::Export_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Server());

	cNetEvent::Export_Creation(packet);

	WWASSERT(PTheGameData != NULL);

	//GAMESPY
	//
	// Due to gamespy support, we can no longer assume that the client exe
	// knows about tier 1 options, so transmit them.
	//
	The_Game()->Export_Tier_1_Data((cPacket &) packet);

	The_Game()->Export_Tier_2_Data((cPacket &) packet);
   packet.Add(The_Game()->Get_Time_Remaining_Seconds());

	packet.Add(HostedGameNumber);

#ifndef MULTIPLAYERDEMO
	packet.Add(CRC_Stringi(The_Game()->Get_Mod_Name()));
	packet.Add(CRC_Stringi(The_Game()->Get_Map_Name()));
#endif // MULTIPLAYERDEMO

	WWDEBUG_SAY(("cGameOptionsEvent sent\n"));

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cGameOptionsEvent::Import_Creation(BitStreamClass & packet)
{
	cNetEvent::Import_Creation(packet);

	WWASSERT(cNetwork::I_Am_Only_Client());

	WWASSERT(PTheGameData != NULL);

	The_Game()->Import_Tier_1_Data((cPacket &) packet);

	The_Game()->Import_Tier_2_Data((cPacket &) packet);
   float time_remaining_seconds = packet.Get(time_remaining_seconds);
   The_Game()->Set_Time_Remaining_Seconds(time_remaining_seconds);

	packet.Get(HostedGameNumber);
	bool act = true;

#ifndef MULTIPLAYERDEMO
	//
	// TSS103001...n.b. need test that Find_Map_Name succeeds...
	//
	unsigned int mod_name_crc = packet.Get(mod_name_crc);
	unsigned int map_name_crc = packet.Get(map_name_crc);

	// Find the mod and map names from their CRC
	StringClass mod_name(0, true);
	StringClass map_name(0, true);
	ModPackageMgrClass::Get_Mod_Map_Name_From_CRC (mod_name_crc, map_name_crc, &mod_name, &map_name);
	The_Game()->Set_Mod_Name(mod_name);
	The_Game()->Set_Map_Name(map_name);

	if (!IS_SOLOPLAY) {
		if (!The_Game()->Is_Map_Valid()) {
			DialogBaseClass* dialog = DialogMgrClass::Find_Dialog((int)RenegadeDialogID::IDD_MULTIPLAY_CONNECTING);

			if (dialog != NULL) {
	 			((DlgMPConnect*)dialog)->Failed_To_Connect();
				act = false;
			}
			WideStringClass tval;
			tval.Format(U_CHAR("%s  %s"), TRANSLATE(IDS_MP_CONNECTION_REFUSED_BY_APPLICATION), 
				TRANSLATE(IDS_MENU_MISSING_MAP));
			DlgMPConnectionRefused::DoDialog(tval, false);
		}
	}

#endif // MULTIPLAYERDEMO

	WWDEBUG_SAY(("cGameOptionsEvent recd\n"));

	if (act) {
		Act();
	}

	Set_Delete_Pending();
}





