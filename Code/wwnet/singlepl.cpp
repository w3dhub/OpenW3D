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
// Filename:     singlepl.cpp
// Project:      wwnet
// Author:       Tom Spencer-Smith
// Date:         Nov 1998
// Description:
//
//-----------------------------------------------------------------------------
#include "singlepl.h" // I WANNA BE FIRST!

#include "netutil.h"
#include "miscutil.h"
#include "wwdebug.h"
#include "wwpacket.h"

//
// class defines
//
bool cSinglePlayerData::IsSinglePlayer = false;
SList<cPacket> cSinglePlayerData::InputPacketList[];

//-----------------------------------------------------------------------------
void cSinglePlayerData::Init()
{
	WWDEBUG_SAY(("cSinglePlayerData::cSinglePlayerData\n"));

	IsSinglePlayer = true;
}

//------------------------------------------------------------------------------------
void cSinglePlayerData::Cleanup()
{
	WWDEBUG_SAY(("cSinglePlayerData::~cSinglePlayerData\n"));

   SLNode<cPacket> * objnode;
   cPacket * p_packet;

   for (int list_type = 0; list_type < 2; list_type++) {
		for (objnode = InputPacketList[list_type].Head(); objnode != NULL;) {
			p_packet = objnode->Data();
			WWASSERT(p_packet != NULL);
			objnode = objnode->Next();
			InputPacketList[list_type].Remove(p_packet);
			delete p_packet;
		}
	}

   IsSinglePlayer = false;
}

//-----------------------------------------------------------------------------
SList<cPacket> * cSinglePlayerData::Get_Input_Packet_List(int type)
{
	WWASSERT(type == CLIENT_LIST || type == SERVER_LIST);
	return &InputPacketList[type];
}





