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
// Filename:     singlepl.h
// Project:      wwnet
// Author:       Tom Spencer-Smith
// Date:         Nov 1998
// Description:  Single player stuff
//
// TODO:
// - merge into netutil ?
//
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef SINGLEPL_H
#define SINGLEPL_H

#include "bittype.h"
#include "slist.h"

enum {
	CLIENT_LIST = 0,
	SERVER_LIST
};

class cPacket;

//
// This is used in single-player mode and is global, not owned by the
// C or S threads. A critical section is used to control access by
// the C and S threads. The client writes to the end of the Server list and
// reads from the beginning of the Client list. Vice versa for the server.
// Send_Packet handles everything transparently.
// TSS - linked list
//

class cSinglePlayerData
{
	public:
		static void Init();
		static void Cleanup();

      static bool Is_Single_Player() {return IsSinglePlayer;}

		static SList<cPacket> * Get_Input_Packet_List(int type);

   private:
		static bool IsSinglePlayer;
		static SList<cPacket> InputPacketList[2];
};

//-----------------------------------------------------------------------------

#endif // SINGLEPL_H

