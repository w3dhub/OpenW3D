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
 *                     $Archive:: /Commando/Code/SControl/servercontrolsocket.cpp             $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 2/22/02 7:10p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *                                                                                             *
 * SocketHandlerClass::SocketHandlerClass -- Class constructor                                 *
 * SocketHandlerClass::~SocketHandlerClass -- Class destrctor                                  *
 * SocketHandlerClass::Open -- Opens a socket and binds it to the given port.                  *
 * SocketHandlerClass::Close -- Closes the socket if it's open. Shuts down the class.          *
 *                                                                                             *
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	<stdlib.h>
#include	"servercontrolsocket.h"
#include "systimer.h"
#include <algorithm>

/*
** All instances are tracked here.
*/

#define PACKET_ENCRYPTION

/***********************************************************************************************
 * ServerControlSocketClass::ServerControlSocketClass -- Class constructor                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/24/00 12:23PM ST : Created                                                              *
 *=============================================================================================*/
ServerControlSocketClass::ServerControlSocketClass(void)
{
	Socket = INVALID_SOCKET;
	Port = -1;

	for (int i=0 ; i<SERVER_CONTROL_MAX_STATIC_BUFFERS ; i++) {
		StaticInBuffers[i].InUse = false;
		StaticInBuffers[i].IsAllocated = false;

		StaticOutBuffers[i].InUse = false;
		StaticOutBuffers[i].IsAllocated = false;
	}

	InBuffersUsed = 0;
	OutBuffersUsed = 0;

	InBufferArrayPos = 0;
	OutBufferArrayPos = 0;

	strcpy(Key, "LAGORAMA");
}


/***********************************************************************************************
 * ServerControlSocketClass::~ServerControlSocketClass -- Class destrctor                      *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/24/00 12:24PM ST : Created                                                              *
 *=============================================================================================*/
ServerControlSocketClass::~ServerControlSocketClass(void)
{
	Close();
}



/***********************************************************************************************
 * ServerControlSocketClass::Open -- Opens a socket and binds it to the given port.            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    port number                                                                       *
 *                                                                                             *
 * OUTPUT:   true if socket was opened and bound OK                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/24/00 12:41PM ST : Created                                                              *
 *=============================================================================================*/
bool ServerControlSocketClass::Open(int port, bool loopback, unsigned int ip)
{
	struct sockaddr_in addr;
	static int socket_transmit_buffer_size = SERVER_CONTROL_SOCKET_BUFFER_SIZE;
	static int socket_receive_buffer_size = SERVER_CONTROL_SOCKET_BUFFER_SIZE;

	/*
	** Create our UDP socket
	*/
	DebugString(("ServerControlSocketClass - About to open a UDP socket\n"));
	Socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (Socket == INVALID_SOCKET) {
		return(false);
	}
	DebugString(("ServerControlSocketClass - Socket %d opened OK\n", Socket));

	/*
	** Bind our UDP socket to our UDP port number
	*/
	addr.sin_family = AF_INET;
	addr.sin_port = (unsigned short) htons((unsigned short)port);
	if (loopback) {
		addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	} else {
		if (ip == 0) {
			ip = INADDR_ANY;
		}
		addr.sin_addr.s_addr = htonl(ip);
	}

	DebugString(("ServerControlSocketClass - About to bind the UDP socket to port %d\n", port));
	if (bind (Socket, (LPSOCKADDR)&addr, sizeof(addr) ) == SOCKET_ERROR) {
		DebugString(("ServerControlSocketClass - bind failed with error code %d\n", WSAGetLastError()));
		Close();
		return(false);
	}

	DebugString(("ServerControlSocketClass - Socket bound OK\n"));

	Port = port;

	/*
	** Specify the size of the receive buffer.
	*/
	int err = setsockopt(Socket, SOL_SOCKET, SO_RCVBUF, (char*)&socket_receive_buffer_size, 4);
	if (err == INVALID_SOCKET) {
		DebugString(("ServerControlSocketClass - Failed to set socket option SO_RCVBUF - error code %d.\n", LAST_ERROR));
		fw_assert ( err != INVALID_SOCKET);
	} else {
		DebugString(("ServerControlSocketClass - Socket option SO_RCVBUF set OK\n"));
	}

	/*
	** Specify the size of the send buffer.
	*/
	err = setsockopt ( Socket, SOL_SOCKET, SO_SNDBUF, (char*)&socket_transmit_buffer_size, 4);
	if ( err == INVALID_SOCKET ) {
		DebugString(("ServerControlSocketClass - Failed to set socket option SO_SNDBUF - error code %d.\n", LAST_ERROR));
		fw_assert ( err != INVALID_SOCKET );
	} else {
		DebugString(("ServerControlSocketClass - Socket option SO_SNDBUF set OK\n"));
	}

	/*
	** Set the blocking mode of the socket to non-blocking.
	*/
	u_long nonblocking = true;
	err = ioctlsocket(Socket, FIONBIO, &nonblocking);
	if (err) {
		DebugString(("ServerControlSocketClass - Failed to set socket to non-blocking - error code %d.\n", LAST_ERROR));
	}

	DebugString(("ServerControlSocketClass - UDP Socket init complete\n"));

	return (true);
}



/***********************************************************************************************
 * ServerControlSocketClass::Close -- Closes the socket if it's open. Shuts down the class.    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/24/00 12:43PM ST : Created                                                              *
 *=============================================================================================*/
void ServerControlSocketClass::Close(void)
{
	if (Socket != INVALID_SOCKET) {
		DebugString(("ServerControlSocketClass - Closing socket %d bound to port %d\n", Socket, Port));
		if (closesocket(Socket) != 0) {
			DebugString(("ServerControlSocketClass - closesocket failed with error code %d\n", LAST_ERROR));
		}
		Socket = INVALID_SOCKET;
	}
}



/***********************************************************************************************
 * ServerControlSocketClass::Discard_In_Buffers -- Discard any packets in our incoming buffers *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    3/1/00 11:52AM ST : Created                                                              *
 *=============================================================================================*/
void ServerControlSocketClass::Discard_In_Buffers(void)
{
	WinsockBufferType *packet;

	while (InBuffers.Count()) {
		packet = InBuffers[0];
		if (packet->IsAllocated) {
			delete packet;
		} else {
			packet->InUse = false;
			InBuffersUsed--;
		}
		InBuffers.Delete(0);
	}
	InBuffersUsed = 0;
	InBufferArrayPos = 0;
}


/***********************************************************************************************
 * ServerControlSocketClass::Discard_Out_Buffers -- Discard any packets in our outgoing buffers*
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    3/1/00 11:51AM ST : Created                                                              *
 *=============================================================================================*/
void ServerControlSocketClass::Discard_Out_Buffers(void)
{
	WinsockBufferType *packet;

	while (OutBuffers.Count()) {
		packet = OutBuffers[0];
		if (packet->IsAllocated) {
			delete packet;
		} else {
			packet->InUse = false;
			OutBuffersUsed--;
		}
		OutBuffers.Delete(0);
	}
	OutBuffersUsed = 0;
	OutBufferArrayPos = 0;
}




/***********************************************************************************************
 * ServerControlSocketClass::Clear_Socket_Error -- Clear any outstanding erros on the socket   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    3/1/00 11:51AM ST : Created                                                              *
 *=============================================================================================*/
void ServerControlSocketClass::Clear_Socket_Error(void)
{
	unsigned int error_code;
	int length = 4;

	if (Socket != INVALID_SOCKET) {
		getsockopt (Socket, SOL_SOCKET, SO_ERROR, (char*)&error_code, &length);
		error_code = 0;
		setsockopt (Socket, SOL_SOCKET, SO_ERROR, (char*)&error_code, length);
	}
}



/***********************************************************************************************
 * ServerControlSocketClass::Write -- Add packet into the outgoing queue                       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    ptr to buffer containing data to send                                             *
 *           length of data to send                                                            *
 *           address to send data to.                                                          *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    3/1/00 11:51AM ST : Created                                                              *
 *=============================================================================================*/
void ServerControlSocketClass::Write(void *buffer, int buffer_len, void *address, unsigned short port)
{
	/*
	** Create a temporary holding area for the packet.
	*/
	WinsockBufferType *packet = (WinsockBufferType*) Get_New_Out_Buffer();
	fw_assert (packet != NULL);

	/*
	** Copy the packet into the holding buffer.
	*/
	memcpy (packet->Buffer, buffer, buffer_len);
	packet->BufferLen = buffer_len;
	packet->IsBroadcast = false;
	memcpy (packet->Address, address, 4);
	packet->Port = port;
#ifdef PACKET_ENCRYPTION
	Encrypt(packet->Buffer, buffer_len);
#endif //PACKET_ENCRYPTION
	Build_Packet_CRC(packet);

	/*
	** Add it to our out list.
	*/
	OutBuffers.Add(packet);

	/*
	** Send it now if possible.
	*/
	Service();
}



/***********************************************************************************************
 * ServerControlSocketClass::Read -- Get the next pending incoming packet                      *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    ptr to buffer to receive input                                                    *
 *           length of buffer                                                                  *
 *           ptr to address to fill with address that packet was sent from                     *
 *           ptr to address of port number to return                                           *
 *           packet number to read                                                             *
 *                                                                                             *
 * OUTPUT:   number of bytes transfered to buffer                                              *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *           None.                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    3/8/00 1:12PM ST : Created                                                               *
 *=============================================================================================*/
int ServerControlSocketClass::Read(void *buffer, int buffer_len, void *address, unsigned short *port, int packetnum)
{
	/*
	** Call the Service function in case there are any outstanding unqueued packets.
	*/
	Service();

	/*
	** If there are no available packets then return 0
	*/
	if (InBuffers.Count() == 0) return (0);

	/*
	** If the packet number is out of range then return 0.
	*/
	if (packetnum >= InBuffers.Count()) {
		return(0);
	}

	WinsockBufferType *packet = InBuffers[packetnum];
	fw_assert(packet != NULL);

	if (packet == NULL) {
		return(0);
	}

	fw_assert(packet->InUse);
	fw_assert(buffer_len >= packet->BufferLen);

	/*
	** Copy the data and the address it came from into the supplied buffers.
	*/
	int bytes = std::min(packet->BufferLen, (int)sizeof(packet->Buffer));
	bytes = std::min(bytes, buffer_len);
	memcpy(buffer, packet->Buffer, bytes);
	memcpy(address, packet->Address, sizeof (packet->Address));
	if (port) {
		*port = packet->Port;
	}

	/*
	** Delete the temporary storage for the packet now that it is being passed to the game.
	*/
	InBuffers.Delete(packetnum);
	if (packet->IsAllocated) {
		delete packet;
	}else{
		packet->InUse = false;
		InBuffersUsed--;
	}

	DebugString(("ServerControlSocketClass - ServerControlSocketClass::Read - Returning %d bytes. %d InBuffers left\n", bytes, InBuffers.Count()));

	return (bytes);
}




/***********************************************************************************************
 * ServerControlSocketClass::Peek -- Get a copy of the specified packet                        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    ptr to buffer to receive input                                                    *
 *           length of buffer                                                                  *
 *           ptr to address to fill with address that packet was sent from                     *
 *           ptr to address of port number to return                                           *
 *           packet number to read                                                             *
 *                                                                                             *
 * OUTPUT:   number of bytes transfered to buffer                                              *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *           None.                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    3/8/00 1:12PM ST : Created                                                               *
 *=============================================================================================*/
int ServerControlSocketClass::Peek(void *buffer, int buffer_len, void *address, unsigned short *port, int packetnum)
{
	/*
	** Call the Service function in case there are any outstanding unqueued packets.
	*/
	Service();

	/*
	** If there are no available packets then return 0
	*/
	if (InBuffers.Count() == 0) return (0);

	/*
	** If the packet number is out of range then return 0.
	*/
	if (packetnum >= InBuffers.Count()) {
		return(0);
	}

	WinsockBufferType *packet = InBuffers[packetnum];
	fw_assert(packet != NULL);

	if (packet == NULL) {
		return(0);
	}

	fw_assert(packet->InUse);
	fw_assert(buffer_len >= packet->BufferLen);

	/*
	** Copy the data and the address it came from into the supplied buffers.
	*/
	int bytes = std::min(packet->BufferLen, (int)sizeof(packet->Buffer));
	bytes = std::min(bytes, buffer_len);
	memcpy(buffer, packet->Buffer, bytes);
	memcpy(address, packet->Address, sizeof (packet->Address));
	if (port) {
		*port = packet->Port;
	}

	DebugString(("ServerControlSocketClass - ServerControlSocketClass::Peek - Returning packet %d of %d bytes. %d InBuffers left\n", packetnum, bytes, InBuffers.Count()));
	return (bytes);
}




/***********************************************************************************************
 * ServerControlSocketClass::Build_Packet_CRC -- Create a CRC value for a packet.              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    ptr to packet                                                                     *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/5/99 1:26PM ST : Created                                                               *
 *=============================================================================================*/
void ServerControlSocketClass::Build_Packet_CRC(WinsockBufferType *packet)
{
	fw_assert (packet->InUse);
	fw_assert (packet->BufferLen);

	packet->CRC = 0;

	unsigned int *crc_ptr = &(packet->CRC);
	unsigned int *packetptr = (unsigned int*) &(packet->Buffer[0]);

	for (int i=0 ; i<packet->BufferLen/4 ; i++) {
		Add_CRC (crc_ptr, *packetptr++);
	}

	int leftover = packet->BufferLen & 3;
	if (leftover) {
		unsigned int val = *packetptr;
		val = val & (0xffffffff >> ((4-leftover) << 3));
		Add_CRC (crc_ptr, val);
	}
}


/***********************************************************************************************
 * WIC::Passes_CRC_Check -- Checks the CRC for a packet                                        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    ptr to packet                                                                     *
 *                                                                                             *
 * OUTPUT:   true if packet passes CRC check                                                   *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/5/99 1:26PM ST : Created                                                               *
 *=============================================================================================*/
bool ServerControlSocketClass::Passes_CRC_Check(WinsockBufferType *packet)
{
	fw_assert (packet->InUse);
	fw_assert (packet->BufferLen < 768);

	if (packet->BufferLen >= 768) {
		return (false);
	}

	unsigned int crc = 0;

	unsigned int *crc_ptr = &crc;
	unsigned int *packetptr = (unsigned int*) &(packet->Buffer[0]);

	for (int i=0 ; i<packet->BufferLen/4 ; i++) {
		Add_CRC (crc_ptr, *packetptr++);
	}

	int leftover = packet->BufferLen & 3;
	if (leftover) {
		unsigned int val = *packetptr;
		val = val & (0xffffffff >> ((4-leftover) << 3));
		Add_CRC (crc_ptr, val);
	}

	if (crc == packet->CRC) {
		return (true);
	}

	fw_assert (crc == packet->CRC);
	DebugString(("ServerControlSocketClass - Error in Winsock packet CRC\n"));
	return (false);
}





/***********************************************************************************************
 * ServerControlSocketClass::Get_New_Out_Buffer -- Get a holding buffer for an outgoing packet       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   ptr to out buffer                                                                 *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/2/00 12:39PM ST : Created                                                               *
 *=============================================================================================*/
void *ServerControlSocketClass::Get_New_Out_Buffer(void)
{
	WinsockBufferType *buffer = NULL;
	int pos;

	fw_assert (OutBuffersUsed <= SERVER_CONTROL_MAX_STATIC_BUFFERS);

	/*
	** If there are no more free buffers in the heap then allocate one.
	*/
	if (OutBuffersUsed == SERVER_CONTROL_MAX_STATIC_BUFFERS) {
		buffer = new WinsockBufferType;
		buffer->IsAllocated = true;
		buffer->InUse = true;
	}else{
		/*
		** Find the next free buffer in the heap.
		*/
		for (int i=0 ; i<SERVER_CONTROL_MAX_STATIC_BUFFERS ; i++) {

			pos = OutBufferArrayPos++;
			if (OutBufferArrayPos > SERVER_CONTROL_MAX_STATIC_BUFFERS-1) {
				OutBufferArrayPos = 0;
			}

			if (StaticOutBuffers[pos].InUse == false) {
				buffer = &StaticOutBuffers[pos];
				buffer->InUse = true;
				OutBuffersUsed++;
				break;
			}
		}
	}

	fw_assert (buffer != NULL);
	return (buffer);
}


/***********************************************************************************************
 * ServerControlSocketClass::Get_New_In_Buffer -- Get a holding buffer for an incoming packet        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   ptr to in buffer                                                                  *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/2/00 12:39PM ST : Created                                                               *
 *=============================================================================================*/
void *ServerControlSocketClass::Get_New_In_Buffer(void)
{
	WinsockBufferType *buffer = NULL;
	int pos;

	fw_assert (InBuffersUsed <= SERVER_CONTROL_MAX_STATIC_BUFFERS);

	/*
	** If there are no more free buffers in the heap then allocate one.
	*/
	if (InBuffersUsed == SERVER_CONTROL_MAX_STATIC_BUFFERS) {
		buffer = new WinsockBufferType;
		buffer->IsAllocated = true;
		buffer->InUse = true;
	}else{
		/*
		** Find the next free buffer in the heap.
		*/
		for (int i=0 ; i<SERVER_CONTROL_MAX_STATIC_BUFFERS ; i++) {

			pos = InBufferArrayPos++;
			if (InBufferArrayPos > SERVER_CONTROL_MAX_STATIC_BUFFERS-1) {
				InBufferArrayPos = 0;
			}

			if (StaticInBuffers[pos].InUse == false) {
				buffer = &StaticInBuffers[pos];
				buffer->InUse = true;
				InBuffersUsed++;
				break;
			}
		}
	}

	fw_assert (buffer != NULL);
	return (buffer);
}



/***********************************************************************************************
 * ServerControlSocketClass::Service -- Service the connection - do all reads and writes             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/2/00 1:04PM ST : Created                                                                *
 *=============================================================================================*/
void ServerControlSocketClass::Service(void)
{
	u_long bytes;
	struct sockaddr_in addr;
	int addr_len;
	WinsockBufferType *packet;
	int result;
	unsigned int timeout_check = TIMEGETTIME();
	int times = 0;

	for (;;) {

		/*
		** Some bail out code, just in case things get stuck.
		*/
		if (times > 5 && (TIMEGETTIME() - timeout_check) > (TIMER_SECOND*5)) {
			break;
		}
		times++;
		Sleep(0);

		/*
		**
		** First, check to see if there is any data waiting to be read.
		**
		**
		**
		*/
		result = ioctlsocket(Socket, FIONREAD, &bytes);

		/*
		** Result of 0 is success.
		*/
		if (result != 0) {
			DebugString(("ioctlsocket returned error code %d\n", LAST_ERROR));
			break;
		} else {

			/*
			** If there is outstanding data, 'bytes' will contain the size of the next queued datagram.
			*/
			if (bytes == 0) {
				break;
			} else {

				/*
				** Call recvfrom function to get the outstanding packet.
				*/
				addr_len = sizeof(addr);
				result = recvfrom(Socket, (char*)ReceiveBuffer, sizeof(ReceiveBuffer), 0, (LPSOCKADDR)&addr, &addr_len);

				/*
				** See if we got an error.
				*/
				if (result == SOCKET_ERROR) {
					DebugString(("ServerControlSocketClass - recvfrom returned error code %d\n", LAST_ERROR));
					Clear_Socket_Error();
					break;
				} else {

					/*
					** Create a new holding buffer to store this packet in.
					*/
					packet = (WinsockBufferType*) Get_New_In_Buffer();
					fw_assert (packet != NULL);

					/*
					** Store the packet into the holding buffer.
					** result is the number of bytes read.
					*/
					packet->BufferLen = result - sizeof(packet->CRC);
					packet->CRC = *((unsigned int*) (&ReceiveBuffer[0]));
					memcpy (packet->Buffer, ReceiveBuffer + sizeof(packet->CRC), packet->BufferLen);

					/*
					** Make sure the CRC looks right.
					*/
					if (!Passes_CRC_Check(packet)) {

						/*
						** Bad CRC, throw away the packet.
						*/
						DebugString(("ServerControlSocketClass - Throwing away malformed packet\n"));
						if (packet->IsAllocated) {
							delete packet;
						}else{
							packet->InUse = false;
							OutBuffersUsed--;
						}
					} else {

						/*
						** Copy the address data into the holding buffer address area.
						*/
						memset (packet->Address, 0, sizeof (packet->Address));

						memcpy (packet->Address, &addr.sin_addr.s_addr, 4);
						packet->Port = ntohs(addr.sin_port);
						//DebugString(("ServerControlSocketClass - recvfrom %s ; %d\n", IPAddressClass(packet->Address).As_String(), (unsigned int)((unsigned short)ntohs(addr.sin_port))));

#ifdef PACKET_ENCRYPTION
						Decrypt(packet->Buffer, packet->BufferLen);
#endif //PACKET_ENCRYPTION

						/*
						** Add the holding buffer to the packet list.
						*/
						InBuffers.Add (packet);
						DebugString(("ServerControlSocketClass - InBuffers.Count() == %d\n", InBuffers.Count()));
					}
				}
			}
		}
	}


	//DebugString(("ServerControlSocketClass - SocketHandler service\n"));
	timeout_check = TIMEGETTIME();
	times = 0;
	/*
	**
	** Send any packets in the outgoing queue.
	**
	**
	**
	*/
	while (OutBuffers.Count()) {

		/*
		** Some bail out code, just in case things get stuck.
		*/
		if (times > 5 && (TIMEGETTIME() - timeout_check) > (TIMER_SECOND*5)) {
			break;
		}
		times++;
		Sleep(0);

		/*
		** Get a pointer to the first packet.
		*/
		packet = OutBuffers[0];
		fw_assert (packet->InUse);

		/*
		** Set up the address structure of the outgoing packet
		*/
		addr.sin_family = AF_INET;
		if (packet->Port == 0) {
			addr.sin_port = (unsigned short) htons((unsigned short)Port);
		} else {
			addr.sin_port = htons(packet->Port);
		}
		memcpy (&addr.sin_addr.s_addr, packet->Address, 4);

		//DebugString(("ServerControlSocketClass - sendto %s ; %d\n", IPAddressClass(packet->Address).As_String(), (unsigned int)((unsigned short)ntohs(addr.sin_port))));

		/*
		** Send it.
		*/
		result = sendto(Socket, ((char const *)packet->Buffer) - sizeof(packet->CRC), packet->BufferLen + sizeof(packet->CRC), 0, (LPSOCKADDR)&addr, sizeof (addr));

		if (result == SOCKET_ERROR){
			if (LAST_ERROR != WSAEWOULDBLOCK) {
				DebugString(("ServerControlSocketClass - sendto returned error code %d\n", LAST_ERROR));
				Clear_Socket_Error();
			} else {

				/*
				** No more room for outgoing packets.
				*/
				DebugString(("ServerControlSocketClass - sendto returned WSAEWOULDBLOCK\n"));
				Sleep(0);
			}
			break;
		}

		/*
		** Delete the sent packet.
		*/
		OutBuffers.Delete(0);
		if (packet->IsAllocated) {
			delete packet;
		}else{
			packet->InUse = false;
			OutBuffersUsed--;
		}
	}
}


/***********************************************************************************************
 * Add_CRC -- Adds a value to a CRC                                                            *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    crc      ptr to crc                                                               *
 *           val      value to add                                                             *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/09/1995 BRR : Created                                                                  *
 *=============================================================================================*/
void ServerControlSocketClass::Add_CRC(unsigned int *crc, unsigned int val)
{
	int hibit;

	if ((*crc) & 0x80000000) {
		hibit = 1;
	} else {
		hibit = 0;
	}

	(*crc) <<= 1;
	(*crc) += val;
	(*crc) += hibit;
}



/***********************************************************************************************
 * ServerControlSocketClass::Set_Encryption_Key -- Sets the key for encryption                 *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Key string (only 1st 8 bytes used)                                                *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *         2/22/2002 12:36PM ST : Created                                                      *
 *=============================================================================================*/
void ServerControlSocketClass::Set_Encryption_Key(char *key)
{
	strncpy(Key, key, 8);
}


/***********************************************************************************************
 * ServerControlSocketClass::Encrypt -- Encrypt packet                                         *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to packet                                                                     *
 *           Size of packet                                                                    *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/22/2002 12:37PM ST : Created                                                            *
 *=============================================================================================*/
void ServerControlSocketClass::Encrypt(unsigned char *packet, int size)
{
	char key[8];
	memcpy(key, Key, 8);
	char a;

	for (int i=0 ; i<size ; i++) {

		a = packet[i];

		/*
		** Add some number.
		*/
		a += ((char) i) - 50;

		/*
		** Xor in the key.
		*/
		a ^= key[i & 7];

		/*
		** Write the byte back and update the key.
		*/
		packet[i] = a;
		key[i & 7] ^= a;
	}
}



/***********************************************************************************************
 * ServerControlSocketClass::Decrypt -- Decrypt packet                                         *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to packet                                                                     *
 *           Size of packet                                                                    *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/22/2002 12:37PM ST : Created                                                            *
 *=============================================================================================*/
void ServerControlSocketClass::Decrypt(unsigned char *packet, int size)
{
	char key[8];
	memcpy(key, Key, 8);
	char a, b;

	for (int i=0 ; i<size ; i++) {

		a = packet[i];
		b = a;

		/*
		** Undo the xor.
		*/
		a ^= key[i & 7];

		/*
		** Subtract the number we first thought of.
		*/
		a -= ((char) i) - 50;

		/*
		** Write the byte back and update the key.
		*/
		packet[i] = a;
		key[i & 7] ^= b;
	}
}

