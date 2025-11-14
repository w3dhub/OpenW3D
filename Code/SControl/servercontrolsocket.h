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
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/SControl/servercontrolsocket.h               $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 2/22/02 1:05p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef SERVERCONTROLSOCKET_H
#define SERVERCONTROLSOCKET_H

#include	"assert.h"
#include	"vector.h"

#include	"network-typedefs.h"

#ifndef DebugString
#include	"wwdebug.h"
#ifdef WWDEBUG_SAY
#define DebugString WWDEBUG_SAY
#endif
#endif

#ifdef WWASSERT
#ifndef fw_assert
#define fw_assert WWASSERT
#endif //fw_assert
#else //WWASSERT
#define fw_assert assert
#endif //WWASSERT

#ifdef errno
#undef errno
#endif	//errno

#define errno (WSAGetLastError())
#define LAST_ERROR errno

#ifndef TIMER_SECOND
#define TIMER_SECOND 1000
#endif //TIMER_SECOND

/*
** Length of winsocks internal buffer.
*/
#define SERVER_CONTROL_SOCKET_BUFFER_SIZE 8192

/*
** Length of our temporary receive buffer. Needs to be more that the max packet size which is about 550 bytes.
*/
#define SERVER_CONTROL_RECEIVE_BUFFER_LEN	640

/*
** Number of statically allocated packet buffers for the class
*/
#define SERVER_CONTROL_MAX_STATIC_BUFFERS	32

/*
** Class to manage low level comms for talking to Mangler Servers.
**
** Can't use the Renegade packet comms since the packet format is different - Mangler servers expect C&C packet format.
*/
class ServerControlSocketClass
{
	public:

		/*
		** Constructor, destructor.
		*/
		ServerControlSocketClass(void);
		~ServerControlSocketClass(void);

		/*
		** Startup, shutdown.
		*/
		bool Open(int port, bool loopback = false, unsigned int ip = 0);
		void Close(void);
		void Discard_In_Buffers(void);
		void Discard_Out_Buffers(void);
		void Set_Encryption_Key(char *key);

		/*
		** Read, write.
		*/
		int Peek(void *buffer, int buffer_len, void *address, unsigned short *port, int packetnum = 0);
		int Read(void *buffer, int buffer_len, void *address, unsigned short *port, int packetnum = 0);
		void Write(void *buffer, int buffer_len, void *address, unsigned short port = 0);

		/*
		** Service.
		*/
		void Service(void);

		/*
		** Error handling.
		*/
		void Clear_Socket_Error(void);


	private:

		/*
		** The socket associated with this class.
		*/
		SOCKET Socket;

		/*
		** The port that this class listens on.
		*/
		int Port;

		/*
		** This struct contains the information needed for each incoming and outgoing packet.
		** It acts as a temporary control for these packets.
		*/
		struct WinsockBufferType {
			unsigned char		Address[4];		// Address. IN_ADDR
			int					BufferLen;		// Length of data in buffer
			bool					IsBroadcast;	// Flag to broadcast this packet
			bool					InUse;			// Useage state of buffer
			bool					IsAllocated;	// false means statically allocated.
			unsigned short		Port;				// Override port. Send to this port if not 0. Save incoming port number.
			unsigned int		CRC;				// CRC of packet for extra sanity.
			unsigned char		Buffer[SERVER_CONTROL_RECEIVE_BUFFER_LEN];	// Buffer to store packet in.
		};

		/*
		** Packet buffer allocation.
		*/
		void *Get_New_Out_Buffer(void);
		void *Get_New_In_Buffer(void);

		/*
		** Packet CRCs.
		*/
		void Add_CRC(unsigned int *crc, unsigned int val);
		virtual void Build_Packet_CRC(WinsockBufferType *packet);
		virtual bool Passes_CRC_Check(WinsockBufferType *packet);

		/*
		** Encryption.
		*/
		void Encrypt(unsigned char *packet, int size);
		void Decrypt(unsigned char *packet, int size);

		/*
		** Array of buffers to temporarily store incoming and outgoing packets.
		*/
		DynamicVectorClass<WinsockBufferType*> InBuffers;
		DynamicVectorClass<WinsockBufferType*> OutBuffers;

		/*
		** Array of buffers that are always available for incoming packets.
		*/
		WinsockBufferType StaticInBuffers[SERVER_CONTROL_MAX_STATIC_BUFFERS];
		WinsockBufferType StaticOutBuffers[SERVER_CONTROL_MAX_STATIC_BUFFERS];

		/*
		** Pointers to allow circular use of the buffer arrays.
		*/
		int InBufferArrayPos;
		int OutBufferArrayPos;

		/*
		** Usage count for each array.
		*/
		int InBuffersUsed;
		int OutBuffersUsed;

		/*
		** Temporary receive buffer to use when querying Winsock for incoming packets.
		*/
		unsigned char ReceiveBuffer[SERVER_CONTROL_RECEIVE_BUFFER_LEN];

		/*
		** Encryption key, only 1st 8 bytes used. The rest is for safety.
		*/
		char Key[12];
};


#endif //SERVERCONTROLSOCKET_H
