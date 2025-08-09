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
// Filename:     lanchat.h
// Project:      Network.lib, for Commando
// Author:       Tom Spencer-Smith
// Date:         Dec 1998
// Description:  lan chat interface
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef LANCHAT_H
#define LANCHAT_H

#include "win.h"
#include "network-typedefs.h"

#include "chatshre.h"

class cPacket;

enum LanMessages {   
	LAN_MESSAGE_POSITION,
};

//-----------------------------------------------------------------------------
class cLanChat
{
	public:
      cLanChat(void);
      ~cLanChat(void);

	   void						Think(void);

      void						Lan_Packet_Handler(cPacket & packet);
		void						Go_To_Location(ChatLocationEnum location);

      void						Accept_Actions(void);
      void						Refusal_Actions(void);

      void						Load_Lan_Registry_Keys(void);
      void						Save_Lan_Registry_Keys(void);

		ChatLocationEnum		Get_Current_Location(void) {return CurrentLocation;}

      void						Init_Lan_Protocol_And_Socket(void);

	private:
      cLanChat(const cLanChat& rhs);             // Disallow copy (compile/link time)
      cLanChat& operator=(const cLanChat& rhs);  // Disallow assignment (compile/link time)

      void						Send_Position_Broadcast(void);
      void						Process_Position_Broadcast(cPacket & packet);

      SOCKET					Socket;
		struct sockaddr_in				LocalAddress;
      DWORD						LastPositionBroadcastTimeMs;
      int						PositionBroadcastNumber;
		ChatLocationEnum		CurrentLocation;
		static const USHORT	LAN_BROADCAST_INTERVAL_MS;
		static const USHORT	LAN_PORT;
};

#endif // LANCHAT_H


