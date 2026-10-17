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
// Filename:     netutil.h
// Project:      wwnet
// Author:       Tom Spencer-Smith
// Date:         June 1998
// Description:
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef NETUTIL_H
#define NETUTIL_H

#include "socket_wrapper.h"
#include "win.h"
#include "network-typedefs.h"

#include "bittype.h"

class cPacket;

#define WOULD_BLOCK(exp)				cNetUtil::Would_Block(__FILE__, __LINE__, exp)
#define SEND_RESOURCE_FAILURE(exp)	cNetUtil::Send_Resource_Failure(__FILE__, __LINE__, exp)
#define WSA_ERROR							{cNetUtil::Wsa_Error(__FILE__, __LINE__);}
#define WSA_CHECK(exp)					{if ((exp) == SOCKET_ERROR) {cNetUtil::Wsa_Error(__FILE__, __LINE__);}}

typedef void (*LanPacketHandlerCallback)(cPacket & packet);

enum {
	MIN_SERVER_PORT = 1024,		//1025,
	MAX_SERVER_PORT = 65535		//5000
};

class cNetUtil
{
	public:
      static void		Wsa_Init();
      static bool		Protocol_Init(bool is_internet);
		static bool		Get_Local_Address(struct sockaddr_in* p_local_address);
		static void		Wsa_Error(const char * sFile, unsigned uLine);
		static bool		Is_Same_Address(struct sockaddr_in* p_address1, const struct sockaddr_in* p_address2);
		static bool		Would_Block(const char * sFile, unsigned uLine, int ret_code);
		static bool		Send_Resource_Failure(const char * sFile, unsigned uLine, int ret_code);
		static void		Address_To_String(struct sockaddr_in* p_address, char * str, unsigned len,
								uint16_t & port);
		static const char *	Address_To_String(uint32_t ip_address);
		static void		String_To_Address(struct sockaddr_in* p_address, const char * str, uint16_t port);
      static void		Create_Unbound_Socket(SOCKET & sock);
      static bool		Create_Bound_Socket(SOCKET & sock, uint16_t port, struct sockaddr_in & local_address);
      static void		Close_Socket(SOCKET & sock);
      static void		Create_Broadcast_Address(struct sockaddr_in* p_broadcast_address, uint16_t port);
      static void		Create_Local_Address(struct sockaddr_in* p_local_address, uint16_t port);
      static void		Broadcast(SOCKET & sock, uint16_t port, cPacket & packet);
      static bool		Is_Tcpip_Present();
      static void		Lan_Servicing(SOCKET & sock, LanPacketHandlerCallback p_callback);
		static bool		Is_Internet() {return IsInternet;}
		static void		Set_Socket_Buffer_Sizes(SOCKET sock, int new_size = 10000);
      static unsigned		Get_Default_Resend_Timeout_Ms() {return DefaultResendTimeoutMs;}

		static const	uint16_t NETSTATS_SAMPLE_TIME_MS;
		static const	uint16_t KEEPALIVE_TIMEOUT_MS;
		static const	uint16_t MAX_RESENDS;
		static const	uint16_t MULTI_SENDS;
		static const	uint16_t RESEND_TIMEOUT_LAN_MS;
		static const	uint16_t RESEND_TIMEOUT_INTERNET_MS;
		static const	unsigned	 CLIENT_CONNECTION_LOSS_TIMEOUT;
		static const	unsigned	 SERVER_CONNECTION_LOSS_TIMEOUT;
		static const	unsigned	 SERVER_CONNECTION_LOSS_TIMEOUT_LOADING_ALLOWANCE;

		static const char *Winsock_Error_Text(int error_code);

   private:
		static int		Get_Local_Tcpip_Addresses(struct sockaddr_in ip_address[], uint16_t max_addresses);

		static bool		IsInternet;
      static unsigned		DefaultResendTimeoutMs;
		static char		WorkingAddressBuffer[300];
};

#endif // NETUTIL_H
