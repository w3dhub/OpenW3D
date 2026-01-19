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

//
// Filename:     lanchat.cpp
// Project:      Network.lib, for Commando
// Author:       Tom Spencer-Smith
// Date:         Dec 1998
// Description:  lan chat interface
//
// TODO:
// - allow non-unique player names on the LAN
// - no provision for graceless exit of person (must remove if don't hear from them)
// - num players in a channel not shown
//

//-----------------------------------------------------------------------------
#include "lanchat.h"
#include "systimer.h"

#include "wwdebug.h"
#include "_globals.h"
#include "netutil.h"
#include "langmode.h"
#include "cnetwork.h"
#include "gamemode.h"
#include "registry.h"
#include "gamechanlist.h"
#include "playermanager.h"
#include "gametype.h"
#include "translatedb.h"
#include "string_ids.h"
#include "fromaddress.h"
#include "DlgMPConnect.h"
#include "slavemaster.h"
#include "dlgmpchangelannickname.h"
#include "gamespyadmin.h"
#include "demosupport.h"

//
// Class statics
//
const USHORT	cLanChat::LAN_BROADCAST_INTERVAL_MS		= 1000;
// FIXME (TSS) BUMP LAN BCAST PORT FOR D1P
//const USHORT	cLanChat::LAN_PORT							= 0xEA00;
const USHORT	cLanChat::LAN_PORT							= 3373;

//-----------------------------------------------------------------------------
cLanChat::cLanChat(void) :
	Socket(INVALID_SOCKET),
   LastPositionBroadcastTimeMs(0),
	PositionBroadcastNumber(0),
	CurrentLocation(LANLOC_LOBBY)
{
   WWDEBUG_SAY(("cLanChat::cLanChat\n"));

	ZeroMemory(&Socket, sizeof(Socket));
	ZeroMemory(&LocalAddress, sizeof(LocalAddress));

   Load_Lan_Registry_Keys();
   Save_Lan_Registry_Keys();

	Init_Lan_Protocol_And_Socket();

   //cGameChannelList::Remove_All();
}

//-----------------------------------------------------------------------------
cLanChat::~cLanChat(void)
{
   WWDEBUG_SAY(("cLanChat::~cLanChat\n"));

	Save_Lan_Registry_Keys();
	if (Socket != INVALID_SOCKET) {
		cNetUtil::Close_Socket(Socket);
	}
}

//-----------------------------------------------------------------------------
void cLanChat::Load_Lan_Registry_Keys(void)
{
	WWDEBUG_SAY(("cLanChat::Load_Lan_Registry_Keys\n"));

	RegistryClass * registry = new RegistryClass(APPLICATION_SUB_KEY_NAME_NETOPTIONS);
	WWASSERT(registry);
	WWASSERT(registry->Is_Valid());

   char name[200];
	registry->Get_String("MyLanName", name, sizeof(name), "");

	WideStringClass widename;
	widename.Convert_From(name);

	if (!cGameSpyAdmin::Is_Gamespy_Game()) {
		if (widename.Is_Empty())
		{
			cNetInterface::Set_Random_Nickname();
		}
		else
		{
			cNetInterface::Set_Nickname(widename);
		}
	}

	int sidePref = registry->Get_Int("SidePref", -1);
	cNetInterface::Set_Side_Preference(sidePref);

	delete registry;
}

//-----------------------------------------------------------------------------
void cLanChat::Save_Lan_Registry_Keys(void)
{
	WWDEBUG_SAY(("cLanChat::Save_Lan_Registry_Keys...\n"));

	RegistryClass * registry = new RegistryClass(APPLICATION_SUB_KEY_NAME_NETOPTIONS);
	WWASSERT(registry);
	WWASSERT(registry->Is_Valid());

	if (!cGameSpyAdmin::Is_Gamespy_Game()) {
		StringClass string;
		cNetInterface::Get_Nickname().Convert_To(string);
		registry->Set_String("MyLanName", string);
	}

	registry->Set_Int("SidePref", cNetInterface::Get_Side_Preference());

	delete registry;
}

//-----------------------------------------------------------------------------
void cLanChat::Init_Lan_Protocol_And_Socket(void)
{
	WWASSERT(!IS_SOLOPLAY);
   if (cGameSpyAdmin::Is_Gamespy_Game()) {
	   return;
   }

   bool is_internet = false;
   if (!cNetUtil::Protocol_Init(is_internet)) {
      DIE;
   }

	//
	// Init socket
	//
	if (Socket != INVALID_SOCKET) {
		cNetUtil::Close_Socket(Socket);
	}

	[[maybe_unused]] bool succeeded = cNetUtil::Create_Bound_Socket(Socket, LAN_PORT, LocalAddress);
	WWASSERT(succeeded);
}

//-----------------------------------------------------------------------------
void cLanChat::Accept_Actions(void)
{
	DlgMPConnect::DoDialog(cNetInterface::Get_Side_Preference(), 0);
}

//-----------------------------------------------------------------------------
void cLanChat::Refusal_Actions(void)
{
   //
   // Must do this if join rejected
   //
}

//-----------------------------------------------------------------------------
void cLanChat::Send_Position_Broadcast(void)
{
	//GAMESPY
	if (cGameSpyAdmin::Is_Gamespy_Game()) {
		return;
	}

   //Debug_Say(("Send_Position_Broadcast\n"));
	if (TIMEGETTIME() - LastPositionBroadcastTimeMs > LAN_BROADCAST_INTERVAL_MS) {

		//
		// Everybody broadcasts their position periodically, as well as
		// immediately after changing location.
		//

		//
		// Broadcast number is used to decide who has to change their name if
		// there is a name collision. The first guy in wins out.
		//
		LastPositionBroadcastTimeMs = TIMEGETTIME();
		PositionBroadcastNumber++;

		cPacket packet;
		packet.Add((BYTE) LAN_MESSAGE_POSITION);
		packet.Add_Wide_Terminated_String(cNetInterface::Get_Nickname());
		packet.Add(PositionBroadcastNumber);
		bool is_hosting = cNetwork::I_Am_Server() &&
         GameModeManager::Find("Combat")->Is_Active();
		packet.Add(is_hosting);

		if (is_hosting) {
			WWASSERT(PTheGameData != NULL);
			packet.Add((int) The_Game()->Get_Game_Type());
			The_Game()->Export_Tier_1_Data(packet);
		}

		cNetUtil::Broadcast(Socket, LAN_PORT, packet);
	}
}

//-----------------------------------------------------------------------------
void cLanChat::Process_Position_Broadcast(cPacket & packet)
{
   //Debug_Say(("Process_Position_Broadcast\n"));

	WideStringClass sender;
	packet.Get_Wide_Terminated_String(sender.Get_Buffer(256), 256);
	int broadcast_number = packet.Get(broadcast_number);
	bool is_hosting = packet.Get(is_hosting);

	//if (sender == cNetInterface::Get_Nickname() &&
	if (!sender.Compare_No_Case(cNetInterface::Get_Nickname()) &&
		broadcast_number > PositionBroadcastNumber) {

      packet.Flush();

		WWDEBUG_SAY(("*** LAN NICKNAME COLLISION (%s) ***\n", sender));

		DlgMpChangeLanNickname::DoDialog();

      return;
   }

   if (is_hosting) {

		int game_type = packet.Get(game_type);
		cGameData * p_game_data = cGameData::Create_Game_Of_Type(
			(cGameData::GameTypeEnum) game_type);
		WWASSERT(p_game_data != NULL);
		p_game_data->Import_Tier_1_Data(packet);

		/*
		//
		// TSS2001f
		// Manually correct hosting ip address according to recv address
		//
		WWASSERT(packet.Get_From_Address_Wrapper() != NULL);
		p_game_data->Set_Ip_Address(packet.Get_From_Address_Wrapper()->FromAddress.sin_addr.s_addr);
		WWDEBUG_SAY(("Manually setting ip to %s\n",
			cNetUtil::Address_To_String(p_game_data->Get_Ip_Address())));
		*/

		//
		//	Don't add this game to the channel list if the player doesn't
		// have the map locally
		//
		if (p_game_data->Does_Map_Exist ()) {
			cGameChannelList::Add_Channel(p_game_data);
		} else {
			delete p_game_data;
			p_game_data = NULL;
		}

	} else {
	   cGameChannelList::Remove_Channel(sender);
      //Debug_Say(("Removing channel for %s\n", sender));
   }
}

//-----------------------------------------------------------------------------
void cLanChat::Lan_Packet_Handler(cPacket & packet)
{
	//GAMESPY
	if (cGameSpyAdmin::Is_Gamespy_Game()) {
		packet.Flush();
		return;
	}

	if (CurrentLocation == LANLOC_EXIT) {
		//
		// Ignore our own broadcasts
		//
		packet.Flush();

	} else {

		BYTE packet_type = packet.Get(packet_type);

		switch (packet_type) {

			case LAN_MESSAGE_POSITION:
				Process_Position_Broadcast(packet);
				break;

			default:
				WWDEBUG_SAY(("cLanChat::Lan_Packet_Handler: received bogus packet_type (%d)\n", packet_type));
				//DIE;
				packet.Flush();
				break;
		}
	}
}

//-----------------------------------------------------------------------------
static void External_Lan_Packet_Handler(cPacket & packet)
{
   //WWASSERT(LanGameModeClass::PLanChat != NULL);
   PLC->Lan_Packet_Handler(packet);
}

//-----------------------------------------------------------------------------
void cLanChat::Go_To_Location(ChatLocationEnum location)
{
	CurrentLocation = location;
}

//-----------------------------------------------------------------------------
void cLanChat::Think(void)
{
	if (cGameSpyAdmin::Is_Gamespy_Game()) {
		return;
	}

   if (CurrentLocation != LANLOC_EXIT) {
		Send_Position_Broadcast();
		cNetUtil::Lan_Servicing(Socket, External_Lan_Packet_Handler);

		DEMO_SECURITY_CHECK;
	}
}
























		/*
		if (MPLanGameListMenuClass::Get_Instance() != NULL) {
			MPLanGameListMenuClass::Get_Instance()->Set_Is_Name_Colliding();
		}
		*/
		//MPLanGameListMenuClass::Set_Is_Name_Colliding();

//#include "dlgmplangamelist.h"
		//cHelpText::Set(TRANSLATION(IDS_MP_NICKNAME_IN_USE));

		//WWDEBUG_SAY(("*** LAN NICKNAME COLLISION (%s) ***\n", sender));
	/*
	if (!cNetInterface::Is_Valid_Name(name)) {
		cNetInterface::Set_Random_Nickname();
		::strcpy(name, cNetInterface::Get_Nickname());
	}
	*/

