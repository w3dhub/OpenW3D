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
 *                     $Archive:: /Commando/Code/Commando/natsock.h                           $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 9/17/01 4:09p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef NATSOCK_H
#define NATSOCK_H

#include	"always.h"
#include	"assert.h"
#include	"vector.h"

#include	"socket_wrapper.h"

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

#define TIMER_SECOND 1000

/*
** Length of winsocks internal buffer.
*/
#define SOCKET_BUFFER_SIZE	1024*512

/*
** Length of our temporary receive buffer. Needs to be more that the max packet size which is about 550 bytes.
*/
#define RECEIVE_BUFFER_LEN	640

/*
** Number of statically allocated packet buffers for the class
*/
#define MAX_STATIC_BUFFERS	32


//#define PACKET_LOSS_PERCENTAGE 15

/*
** Class to manage low level comms for talking to Mangler Servers.
**
** Can't use the Renegade packet comms since the packet format is different - Mangler servers expect C&C packet format.
*/
class SocketHandlerClass
{
	public:

		/*
		** Constructor, destructor.
		*/
		SocketHandlerClass(void);
		~SocketHandlerClass(void);

		/*
		** Startup, shutdown.
		*/
		bool Open(int inport, int outport);
		void Close(void);
		void Discard_In_Buffers(void);
		void Discard_Out_Buffers(void);

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
		static void Service_All(void);
		inline void Service_Never(void) {CanService = false;};

		/*
		** Error handling.
		*/
		void Clear_Socket_Error(void);

		/*
		** Query functions.
		*/
		int Get_Num_Queued_Receive_Packets(void) {return(InBuffers.Count());};
		int Get_Num_Queued_Outgoing_Packets(void) {return(OutBuffers.Count());};
		int Get_Num_Local_Addresses(void) {return (LocalAddresses.Count());};
		unsigned char * Get_Local_Address (int a) {return (LocalAddresses[a]);};
		int Get_Incoming_Port(void) {return(IncomingPort);};
		SOCKET Get_Socket(void) {return(Socket);};


	private:

		/*
		** The socket associated with this class.
		*/
		SOCKET Socket;

		/*
		** The port that this class listens on.
		*/
		int IncomingPort;

		/*
		** The port that the class writes to.
		*/
		int OutgoingPort;

		/*
		** List of local addresses.
		*/
		DynamicVectorClass <unsigned char *> LocalAddresses;

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
			unsigned long		CRC;				// CRC of packet for extra sanity.
			unsigned char		Buffer[RECEIVE_BUFFER_LEN];	// Buffer to store packet in.
		};

		/*
		** Packet buffer allocation.
		*/
		void *Get_New_Out_Buffer(void);
		void *Get_New_In_Buffer(void);

		/*
		** Packet CRCs.
		*/
		void Add_CRC(unsigned long *crc, unsigned long val);
		virtual void Build_Packet_CRC(WinsockBufferType *packet);
		virtual bool Passes_CRC_Check(WinsockBufferType *packet);

		/*
		** Array of buffers to temporarily store incoming and outgoing packets.
		*/
		DynamicVectorClass<WinsockBufferType*> InBuffers;
		DynamicVectorClass<WinsockBufferType*> OutBuffers;

		/*
		** Array of buffers that are always available for incoming packets.
		*/
		WinsockBufferType StaticInBuffers[MAX_STATIC_BUFFERS];
		WinsockBufferType StaticOutBuffers[MAX_STATIC_BUFFERS];

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
		unsigned char ReceiveBuffer[RECEIVE_BUFFER_LEN];

		/*
		** All instances of this class.
		*/
		static DynamicVectorClass<SocketHandlerClass*> AllSocketHandlers;

		/*
		** Can the regular service code be called for this class?
		*/
		bool CanService;
};


#endif //NATSOCK_H
