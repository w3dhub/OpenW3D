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

//*****************************************************************************
//
//	Copyright (c) 2000 Westwood Studios. All Rights Reserved.
//
//	localhost.cpp
//
// Created on 12 Apr 2001 by Tom Spencer-Smith (Westwood/Vegas)
//
//	Description: 
// Just a wee test to see if I can use localhost addressing (127.0.0.1) 
// on non-networked systems...
//
//*****************************************************************************

#include <stdio.h>
#include <conio.h>
#include <winsock2.h>
#include <assert.h>

//---------------------------------------------------------------------------
void main(void)
{
	WSADATA winsock_data;
	int rc_wsastartup = ::WSAStartup(MAKEWORD(1, 1), &winsock_data);
	assert(rc_wsastartup == 0);

	SOCKET sock = ::socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
	{
		::printf("::socket failed with error code %d\n", ::WSAGetLastError());
	}
	assert(sock != INVALID_SOCKET);
   
	ULONG is_nonblocking = true;
   int rc_ioctl = ::ioctlsocket(sock, FIONBIO, &is_nonblocking);
   assert(rc_ioctl == 0);
	
	struct sockaddr_in local_address;
	local_address.sin_family			= AF_INET;
	local_address.sin_addr.s_addr		= ::inet_addr("127.0.0.1");	// localhost
	local_address.sin_port				= ::htons(5555);					// arbitrary

	int rc_bind = ::bind(sock, reinterpret_cast<const SOCKADDR *>(&local_address), 
		sizeof(local_address));
	assert(rc_bind == 0);

	while (!::kbhit())
	{
		//
		// Send a packet periodically
		//
		static DWORD last_send_time_ms = 0;
		DWORD time_now_ms = ::timeGetTime();
		const DWORD SEND_INTERVAL_MS = 500;
		if (time_now_ms - last_send_time_ms > SEND_INTERVAL_MS)
		{
			last_send_time_ms = time_now_ms;

			static int send_count = 0;
			char send_data[200];
			::sprintf(send_data, "packet_%d", send_count++);
			int to_len = sizeof(local_address);
			int rc_send = ::sendto(sock, send_data, ::strlen(send_data), 0, 
				(LPSOCKADDR) &local_address, to_len);
			assert(rc_send != SOCKET_ERROR);
			::printf("Sent %d bytes (%s) on socket %d\n", rc_send, send_data, sock);
		}

		//
		// Receive all data available
		//
		int rc_recv = 0;
		do 
		{
			char recv_data[200];
			::memset(recv_data, 0, sizeof(recv_data));
			struct sockaddr_in from_address;
			int from_len = sizeof(from_address);
			rc_recv = ::recvfrom(sock, recv_data, sizeof(recv_data), 0,
				(LPSOCKADDR) &from_address, &from_len);

			if (rc_recv > 0) 
			{
				::printf("Recd %d bytes (%s) on socket %d\n", rc_recv, recv_data, sock);
			}
			else
			{
				assert(::WSAGetLastError() == WSAEWOULDBLOCK);
			}

		} while (rc_recv > 0);
	}

	int rc_cs = ::closesocket(sock);
	assert(rc_cs == 0);

	int rc_wsacleanup = ::WSACleanup();
	assert(rc_wsacleanup == 0);
}