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
// Filename:     netutil.cpp
// Project:      wwnet
// Author:       Tom Spencer-Smith
// Date:         June 1998
// Description:
//
//-----------------------------------------------------------------------------
#include "netutil.h" // I WANNA BE FIRST!

#include <stdio.h>
#include <cstring>

#include "miscutil.h"
#include "mathutil.h"
#include "singlepl.h"
#include "wwpacket.h"
#include "wwdebug.h"
#include "ffactory.h"
#include "ini.h"
#include "systimer.h"
#include "fromaddress.h"

//
// class statics
//
//const WORD cNetUtil::WS_VERSION_REQD = MAKEWORD(1, 1); // Winsock 1.1
//USHORT cNetUtil::HeaderBytes;
//USHORT cNetUtil::MaxPacketAppDataSize = MAX_LAN_PACKET_APP_DATA_SIZE;
UINT cNetUtil::DefaultResendTimeoutMs = 200; // used for singleplayer
bool cNetUtil::IsInternet = false;

static const int INVALID_VALUE = -999;

const USHORT	cNetUtil::NETSTATS_SAMPLE_TIME_MS									= 2000;
const USHORT	cNetUtil::KEEPALIVE_TIMEOUT_MS										= 2000;
const USHORT	cNetUtil::MAX_RESENDS													= 50;
const USHORT	cNetUtil::MULTI_SENDS													= 10;
const USHORT	cNetUtil::RESEND_TIMEOUT_LAN_MS										= 300;
const USHORT	cNetUtil::RESEND_TIMEOUT_INTERNET_MS								= 500;
const	ULONG		cNetUtil::CLIENT_CONNECTION_LOSS_TIMEOUT							= 15000;		// Milliseconds til client gives up on server
const	ULONG		cNetUtil::SERVER_CONNECTION_LOSS_TIMEOUT							= 15000;		// Milliseconds til server gives up on client
const	ULONG		cNetUtil::SERVER_CONNECTION_LOSS_TIMEOUT_LOADING_ALLOWANCE	= 45000;		// Milliseconds extra allowed til server gives up on loading client.


//int cNetUtil::DefaultMultiSends							= INVALID_VALUE;
//int cNetUtil::DefaultMaxResends							= INVALID_VALUE;
//int cNetUtil::DefaultKeepaliveTimeoutMs				= INVALID_VALUE;
//int cNetUtil::DesiredSendBufferSizeBytes				= INVALID_VALUE;
//int cNetUtil::DesiredReceiveBufferSizeBytes			= INVALID_VALUE;
//int cNetUtil::DefaultServerPort							= INVALID_VALUE;
//int cNetUtil::MaxReceiveTimeMs							= INVALID_VALUE;

//float cNetUtil::PriorityToleranceDownwards			= INVALID_VALUE;
//float cNetUtil::PriorityToleranceUpwards				= INVALID_VALUE;
//float cNetUtil::MaxTPCorrectionDownwards				= INVALID_VALUE;
//float cNetUtil::MaxTPCorrectionUpwards					= INVALID_VALUE;
//float cNetUtil::PriorityNoiseFactor						= INVALID_VALUE;
//float cNetUtil::InitialThresholdPriority				= INVALID_VALUE;
//float cNetUtil::PriorityGrowthPerSecond				= INVALID_VALUE;

char cNetUtil::WorkingAddressBuffer[]					= "";

//-----------------------------------------------------------------------------
//
// Macro
//
#define ADD_CASE(exp)    case exp: ::sprintf(error_msg, #exp);  break;

void cNetUtil::Wsa_Error([[maybe_unused]] LPCSTR sFile, [[maybe_unused]] unsigned uLine)
{
   [[maybe_unused]] int err = wwnet::SocketGetLastError();
   WWDEBUG_SAY(("* %s:%d: WSA function returned error code: %s\n", sFile, uLine, Winsock_Error_Text(err)));
   DIE;
}


//-----------------------------------------------------------------------------
//
// Just get the text for the specified error code.
//
const char * cNetUtil::Winsock_Error_Text(int error_code)
{
#ifdef _WINDOWS
   static char error_msg[500];

   switch (error_code) {

      //
      // Windows Sockets definitions of regular Microsoft C error constants
      //
      ADD_CASE(WSAEINTR)
      ADD_CASE(WSAEBADF)
      ADD_CASE(WSAEACCES)
      ADD_CASE(WSAEFAULT)
      ADD_CASE(WSAEINVAL)
      ADD_CASE(WSAEMFILE)

      //
      // Windows Sockets definitions of regular Berkeley error constants
      //
      ADD_CASE(WSAEWOULDBLOCK)
      ADD_CASE(WSAEINPROGRESS)
      ADD_CASE(WSAEALREADY)
      ADD_CASE(WSAENOTSOCK)
      ADD_CASE(WSAEDESTADDRREQ)
      ADD_CASE(WSAEMSGSIZE)
      ADD_CASE(WSAEPROTOTYPE)
      ADD_CASE(WSAENOPROTOOPT)
      ADD_CASE(WSAEPROTONOSUPPORT)
      ADD_CASE(WSAESOCKTNOSUPPORT)
      ADD_CASE(WSAEOPNOTSUPP)
      ADD_CASE(WSAEPFNOSUPPORT)
      ADD_CASE(WSAEAFNOSUPPORT)
      ADD_CASE(WSAEADDRINUSE)
      ADD_CASE(WSAEADDRNOTAVAIL)
      ADD_CASE(WSAENETDOWN)
      ADD_CASE(WSAENETUNREACH)
      ADD_CASE(WSAENETRESET)
      ADD_CASE(WSAECONNABORTED)
      ADD_CASE(WSAECONNRESET)
      ADD_CASE(WSAENOBUFS)
      ADD_CASE(WSAEISCONN)
      ADD_CASE(WSAENOTCONN)
      ADD_CASE(WSAESHUTDOWN)
      ADD_CASE(WSAETOOMANYREFS)
      ADD_CASE(WSAETIMEDOUT)
      ADD_CASE(WSAECONNREFUSED)
      ADD_CASE(WSAELOOP)
      ADD_CASE(WSAENAMETOOLONG)
      ADD_CASE(WSAEHOSTDOWN)
      ADD_CASE(WSAEHOSTUNREACH)
      ADD_CASE(WSAENOTEMPTY)
      ADD_CASE(WSAEPROCLIM)
      ADD_CASE(WSAEUSERS)
      ADD_CASE(WSAEDQUOT)
      ADD_CASE(WSAESTALE)
      ADD_CASE(WSAEREMOTE)

      //
      // Extended Windows Sockets error constant definitions
      ///
      ADD_CASE(WSASYSNOTREADY)
      ADD_CASE(WSAVERNOTSUPPORTED)
      ADD_CASE(WSANOTINITIALISED)
		ADD_CASE(WSAEDISCON)

		default:
         ::sprintf(error_msg, "Unknown Winsock Error (%d)", error_code);
         break;
   }

   return error_msg;
#else
	return ::strerror(error_code);
#endif
}



/*
int g_c_wouldblock = 0;
int g_c_nobufs = 0;
*/
//-----------------------------------------------------------------------------
bool cNetUtil::Send_Resource_Failure(LPCSTR sFile, unsigned uLine, int ret_code)
{
	bool return_code = false;

   if (ret_code == SOCKET_ERROR) {
		int wsa_error = wwnet::SocketGetLastError();
      if (wsa_error == WSAEWOULDBLOCK || wsa_error == WSAENOBUFS) {

			/*
			if (wsa_error == WSAEWOULDBLOCK) {
				g_c_wouldblock++;
			} else {
				g_c_nobufs++;
			}
			*/

         return_code = true;
      } else {
         Wsa_Error(sFile, uLine);
      }
   } else {
		return_code = false;
	}

   return return_code;
}

//-----------------------------------------------------------------------------
bool cNetUtil::Would_Block(LPCSTR sFile, unsigned uLine, int ret_code)
{
	bool retcode = false;

   if (ret_code == SOCKET_ERROR) {
      if (wwnet::SocketGetLastError() == WSAEWOULDBLOCK) {
         retcode = true;
      } else {
         Wsa_Error(sFile, uLine);
         retcode = false;
      }
   } else {
      retcode = false;
	}

   return retcode;
}

//-----------------------------------------------------------------------------
//
// Returns up to max_addresses adapter addresses for the local host
//
int cNetUtil::Get_Local_Tcpip_Addresses(sockaddr_in ip_address[], unsigned short max_addresses)
{
	WWDEBUG_SAY(("cNetUtil::Get_Local_Tcpip_Addresses:\n"));

	char local_host_name[200];
	WSA_CHECK(wwnet::SocketGetHostName(local_host_name, sizeof local_host_name));
	WWDEBUG_SAY(("  Host name is %s\n", local_host_name));

	hostent* p_hostent = wwnet::SocketGetHostByName(local_host_name);

	int num_adapters = 0;
	if (p_hostent) {
		while (num_adapters < max_addresses && p_hostent->h_addr_list[num_adapters] != nullptr) {
			sockaddr_in& sa = ip_address[num_adapters];
			std::memset(&sa, 0, sizeof(sa));
			sa.sin_family = AF_INET;

			// copy one IPv4 address
			std::memcpy(&sa.sin_addr,
				p_hostent->h_addr_list[num_adapters],
				std::min<size_t>(sizeof(sa.sin_addr), p_hostent->h_length));

			WWDEBUG_SAY(("  Address: %s\n", Address_To_String(sa.sin_addr.s_addr)));
			++num_adapters;
		}
	}
	return num_adapters;
}

//-----------------------------------------------------------------------------
bool cNetUtil::Is_Same_Address(struct sockaddr_in* p_address1, const struct sockaddr_in* p_address2)
{
	//
	// C disallows comparison of structs...
	//

   WWASSERT(!cSinglePlayerData::Is_Single_Player());
   WWASSERT(p_address1 != NULL);
	WWASSERT(p_address2 != NULL);

   return
      p_address1->sin_addr.s_addr	== p_address2->sin_addr.s_addr &&
      p_address1->sin_port				== p_address2->sin_port;
}

//-------------------------------------------------------------------------------
void cNetUtil::Address_To_String(struct sockaddr_in* p_address, char * str, [[maybe_unused]] UINT len,
   USHORT & port)
{
	WWASSERT(p_address != NULL);
   WWASSERT(str != NULL);

	char temp_str[1000];

	::strcpy(temp_str, ::inet_ntoa(p_address->sin_addr));
   port = ::ntohs(p_address->sin_port);

	WWASSERT(::strlen(temp_str) <= len);
	::strcpy(str, temp_str);
}

//-------------------------------------------------------------------------------
LPCSTR cNetUtil::Address_To_String(ULONG ip)
{
	IN_ADDR in_addr;
	in_addr.s_addr = ip;
	char * p = ::inet_ntoa(in_addr);
	if (p == NULL) {
		::sprintf(WorkingAddressBuffer, "Invalid ip (%u)", ip);
	} else {
		::strcpy(WorkingAddressBuffer, p);
	}

	return WorkingAddressBuffer;
}

//-------------------------------------------------------------------------------
void cNetUtil::String_To_Address(struct sockaddr_in* p_address, LPCSTR str, USHORT port)
{
	WWASSERT(p_address != NULL);
	std::memset(p_address, 0, sizeof(struct sockaddr_in));

   p_address->sin_family			= AF_INET;
   p_address->sin_addr.s_addr		= ::inet_addr(str);
   p_address->sin_port				= ::htons(port);

   WWASSERT(p_address->sin_addr.s_addr != INADDR_NONE);
}

//-------------------------------------------------------------------------------
bool cNetUtil::Is_Tcpip_Present(void)
{
   //
   // N.B. I tested EnumProtocols and found it unreliable.
   //

	bool retcode = true;

   SOCKET test_socket = ::socket(AF_INET, SOCK_DGRAM, 0);
   if (test_socket == INVALID_SOCKET) {
      if (wwnet::SocketGetLastError() == WSAEAFNOSUPPORT) {
         retcode = false;
      } else {
         WSA_ERROR;
      }
   } else {
	   WSA_CHECK(::closesocket(test_socket));
	}

   return retcode;
}

//-------------------------------------------------------------------------------
void cNetUtil::Wsa_Init()
{
	//
	// winsock 1.1
	//

	int rc = wwnet::SocketStartup();
	if (rc != 0) {
		wwnet::SocketSetLastError(rc);
		DIE;
	}
}

//-------------------------------------------------------------------------------
bool cNetUtil::Protocol_Init(bool is_internet)
{
	IsInternet = is_internet;

   bool retcode = false;

	if (Is_Tcpip_Present()) {
      retcode = true;

		if (IsInternet) {
			DefaultResendTimeoutMs = RESEND_TIMEOUT_INTERNET_MS;
		} else {
			DefaultResendTimeoutMs = RESEND_TIMEOUT_LAN_MS;
		}
   } else {
      retcode = false;
	}

   return retcode;
}

/*
//-------------------------------------------------------------------------------
float cNetUtil::Compute_Priority_Noise()
{
	//
	// Add some noise to increase the spread. This noise must be bigger
	// than the maximum threshold adjustment, so PriorityNoiseFactor should
	// be at least 1 if you are adding noise.
	// Do not jitter a priority of zero!
	//
	// Or maybe we can just specify a max noise now?
	//
	float noise_width = PriorityNoiseFactor * MaxTPCorrectionDownwards;
	return cMathUtil::Get_Hat_Pdf_Double(-noise_width / 2.0f, +noise_width / 2.0f);
}
*/

//-------------------------------------------------------------------------------
void cNetUtil::Set_Socket_Buffer_Sizes(SOCKET sock, int new_size)
{
   WWDEBUG_SAY(("cNetUtil::Set_Socket_Buffer_Sizes:\n"));

   int buffersize		= 0;
   socklen_t opt_len	= 0;

	buffersize = 0;
	opt_len = static_cast<socklen_t>(sizeof(buffersize));
   WSA_CHECK(wwnet::SocketGetSockOpt(sock, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char *>(&buffersize), &opt_len));
   //WWDEBUG_SAY(("  SO_SNDBUF = %d\n", buffersize));

   buffersize = 0;
   opt_len = static_cast<socklen_t>(sizeof(buffersize));
   WSA_CHECK(wwnet::SocketGetSockOpt(sock, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char *>(&buffersize), &opt_len));
   //WWDEBUG_SAY(("  SO_RCVBUF = %d\n", buffersize));

   buffersize = new_size;
   opt_len = static_cast<socklen_t>(sizeof(buffersize));
   WSA_CHECK(wwnet::SocketSetSockOpt(sock, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char *>(&buffersize), opt_len));
   //WWDEBUG_SAY(("  Attempting to set SO_SNDBUF = %d\n", buffersize));

   buffersize = new_size;
   opt_len = static_cast<socklen_t>(sizeof(buffersize));
   WSA_CHECK(wwnet::SocketSetSockOpt(sock, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char *>(&buffersize), opt_len));
   //WWDEBUG_SAY(("  Attempting to set SO_RCVBUF = %d\n", buffersize));

	buffersize = 0;
	opt_len = static_cast<socklen_t>(sizeof(buffersize));
   WSA_CHECK(wwnet::SocketGetSockOpt(sock, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char *>(&buffersize), &opt_len));
   //WWDEBUG_SAY(("  SO_SNDBUF = %d\n", buffersize));

	buffersize = 0;
	opt_len = static_cast<socklen_t>(sizeof(buffersize));
   WSA_CHECK(wwnet::SocketGetSockOpt(sock, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char *>(&buffersize), &opt_len));
   //WWDEBUG_SAY(("  SO_RCVBUF = %d\n", buffersize));
}

















/*
//-------------------------------------------------------------------------------
void cNetUtil::Onetime_Init()
{
	FileClass * p_ini_file = _TheFileFactory->Get_File("netparams.ini");

	if (p_ini_file != NULL) {

		INIClass netparams_ini(*p_ini_file);

		WWASSERT(netparams_ini.Section_Count() == 1);

		const LPCSTR SECTION_NAME = "Settings";

		//NETSTATS_SAMPLE_TIME_MS					= netparams_ini.Get_Int(SECTION_NAME, "NETSTATS_SAMPLE_TIME_MS",				INVALID_VALUE);
		//WWASSERT(NETSTATS_SAMPLE_TIME_MS > 0);

		//RESEND_TIMEOUT_LAN_MS			= netparams_ini.Get_Int(SECTION_NAME, "RESEND_TIMEOUT_LAN_MS",		INVALID_VALUE);
		//WWASSERT(RESEND_TIMEOUT_LAN_MS > 0);

		//RESEND_TIMEOUT_INTERNET_MS	= netparams_ini.Get_Int(SECTION_NAME, "RESEND_TIMEOUT_INTERNET_MS", INVALID_VALUE);
		//WWASSERT(RESEND_TIMEOUT_INTERNET_MS > 0);

		//DefaultMultiSends						= netparams_ini.Get_Int(SECTION_NAME, "DefaultMultiSends",					INVALID_VALUE);
		//WWASSERT(DefaultMultiSends > 0);

		//DefaultMaxResends						= netparams_ini.Get_Int(SECTION_NAME, "DefaultMaxResends",					INVALID_VALUE);
		//WWASSERT(DefaultMaxResends > 0);

		//DefaultKeepaliveTimeoutMs			= netparams_ini.Get_Int(SECTION_NAME, "DefaultKeepaliveTimeoutMs",		INVALID_VALUE);
		//WWASSERT(DefaultKeepaliveTimeoutMs > 0);

		//DesiredSendBufferSizeBytes			= netparams_ini.Get_Int(SECTION_NAME, "DesiredSendBufferSizeBytes",		INVALID_VALUE);
		//WWASSERT(DesiredSendBufferSizeBytes > 0);

		//DESIRED_RECEIVE_BUFFER_SIZE_BYTES		= netparams_ini.Get_Int(SECTION_NAME, "DesiredReceiveBufferSizeBytes",	INVALID_VALUE);
		//WWASSERT(DesiredReceiveBufferSizeBytes > 0);

		//DefaultServerPort						= netparams_ini.Get_Int(SECTION_NAME, "DefaultServerPort",					INVALID_VALUE);
		//WWASSERT(DefaultServerPort >= MIN_SERVER_PORT && DefaultServerPort <= MAX_SERVER_PORT);

		//MaxReceiveTimeMs						= netparams_ini.Get_Int(SECTION_NAME, "MaxReceiveTimeMs",					INVALID_VALUE);
		//WWASSERT(MaxReceiveTimeMs > 0);

		//PriorityToleranceDownwards			= netparams_ini.Get_Float(SECTION_NAME, "PriorityToleranceDownwards",			INVALID_VALUE);
		//WWASSERT(PriorityToleranceDownwards > -1 - MISCUTIL_EPSILON && PriorityToleranceDownwards < 1 + MISCUTIL_EPSILON);

		//PriorityToleranceUpwards			= netparams_ini.Get_Float(SECTION_NAME, "PriorityToleranceUpwards",			INVALID_VALUE);
		//WWASSERT(PriorityToleranceUpwards > -1 - MISCUTIL_EPSILON && PriorityToleranceUpwards < 1 + MISCUTIL_EPSILON);

		//MaxTPCorrectionDownwards			= netparams_ini.Get_Float(SECTION_NAME, "MaxTPCorrectionDownwards",	INVALID_VALUE);
		//WWASSERT(MaxTPCorrectionDownwards > -1 - MISCUTIL_EPSILON && MaxTPCorrectionDownwards < 1 + MISCUTIL_EPSILON);

		//MaxTPCorrectionUpwards				= netparams_ini.Get_Float(SECTION_NAME, "MaxTPCorrectionUpwards",	INVALID_VALUE);
		//WWASSERT(MaxTPCorrectionUpwards > -1 - MISCUTIL_EPSILON && MaxTPCorrectionUpwards < 1 + MISCUTIL_EPSILON);

		//PriorityNoiseFactor					= netparams_ini.Get_Float(SECTION_NAME, "PriorityNoiseFactor",				INVALID_VALUE);
		//WWASSERT(PriorityNoiseFactor >= -MISCUTIL_EPSILON);

		//InitialThresholdPriority			= netparams_ini.Get_Float(SECTION_NAME, "InitialThresholdPriority",		INVALID_VALUE);
		//WWASSERT(InitialThresholdPriority >= -MISCUTIL_EPSILON);

		//PriorityNoiseFactor					= netparams_ini.Get_Float(SECTION_NAME, "PriorityNoiseFactor",				INVALID_VALUE);
		//WWASSERT(PriorityNoiseFactor >= -MISCUTIL_EPSILON);

		//PriorityGrowthPerSecond				= netparams_ini.Get_Float(SECTION_NAME, "PriorityGrowthPerSecond",		INVALID_VALUE);
		//WWASSERT(PriorityGrowthPerSecond >= -MISCUTIL_EPSILON);

		_TheFileFactory->Return_File(p_ini_file);
	}
}
*/

void cNetUtil::Create_Unbound_Socket(SOCKET & sock)
{
   sock = ::socket(AF_INET, SOCK_DGRAM, 0);
   if (sock == INVALID_SOCKET) {
      WSA_ERROR;
   }

   //
   // Enable broadcasts
   //
   int optval = true;
   WSA_CHECK(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *) &optval, sizeof(optval)));

   //
   // Make socket non-blocking
   //
   wwnet::SocketIoctlParam arg = 1L;
   WSA_CHECK(wwnet::SocketIoctl(sock, FIONBIO, &arg));
}

//-------------------------------------------------------------------------------
bool cNetUtil::Create_Bound_Socket(SOCKET & sock, USHORT port, struct sockaddr_in & local_address)
{
   //
   // TSS - is all this necessary or is above function OK?
   //

   Create_Unbound_Socket(sock);

   Create_Local_Address(&local_address, port);
	int result = ::bind(sock, (const sockaddr*)&local_address, sizeof(struct sockaddr_in));
	if (result == 0) {
		return true;
   } else {
      WWASSERT(result == SOCKET_ERROR);
      //if (wwnet::SocketGetLastError() != WSAEADDRINUSE) {
         WSA_ERROR;
      //}
      return false;
   }
}

//-------------------------------------------------------------------------------
void cNetUtil::Close_Socket(SOCKET & sock)
{
   ::closesocket(sock);
}

//-----------------------------------------------------------------------------
void cNetUtil::Broadcast(SOCKET & sock, USHORT port, cPacket & packet)
{
   struct sockaddr_in broadcast_address;
   Create_Broadcast_Address(&broadcast_address, port);
   int bytes_sent;
	//WSA_CHECK(bytes_sent = sendto(sock, packet.Data, packet.SendLength,
   //   0, &broadcast_address, sizeof(struct sockaddr_in)));
   socklen_t addr_len = sizeof(struct sockaddr_in);
   bytes_sent = wwnet::SocketSendTo(sock, packet.Get_Data(), packet.Get_Compressed_Size_Bytes(),
	   0, (const sockaddr*)&broadcast_address, &addr_len);
// FIXME (TSS) WSAENOBUFS
   //WWDEBUG_SAY(("Sent broadcast, length = %d bytes\n", bytes_sent));
}

//-------------------------------------------------------------------------------
void cNetUtil::Create_Broadcast_Address(struct sockaddr_in* p_broadcast_address,
   USHORT port)
{
   WWASSERT(p_broadcast_address != NULL);
   std::memset(p_broadcast_address, 0, sizeof(struct sockaddr_in));

	p_broadcast_address->sin_family			= AF_INET;
	p_broadcast_address->sin_addr.s_addr	= INADDR_BROADCAST; // ::inet_addr("255.255.255.255");
	p_broadcast_address->sin_port				= ::htons(port);
}

//-------------------------------------------------------------------------------
void cNetUtil::Create_Local_Address(struct sockaddr_in* p_local_address, USHORT port)
{
   WWASSERT(p_local_address != NULL);
   std::memset(p_local_address, 0, sizeof(struct sockaddr_in));

	p_local_address->sin_family			= AF_INET;
	p_local_address->sin_addr.s_addr		= INADDR_ANY;
	p_local_address->sin_port				= ::htons(port);
}

//-------------------------------------------------------------------------------
bool cNetUtil::Get_Local_Address(struct sockaddr_in* p_local_address)
{
	WWASSERT(p_local_address != NULL);

	/*
	const USHORT MAX_ADDRESSES = 1;
	int num_addresses = Get_Local_Tcpip_Addresses(p_local_address, MAX_ADDRESSES);

	return (num_addresses == 1);
	*/

	const USHORT MAX_ADDRESSES = 10;
	struct sockaddr_in local_address[MAX_ADDRESSES];
	int num_addresses = Get_Local_Tcpip_Addresses(local_address, MAX_ADDRESSES);

	if (num_addresses > 0) {
		::memcpy(p_local_address, &local_address[0], sizeof(struct sockaddr_in));
	}

	return (num_addresses > 0);
}

//-----------------------------------------------------------------------------
void cNetUtil::Lan_Servicing(SOCKET & sock, LanPacketHandlerCallback p_callback)
{
   int retcode;

   unsigned int start_time = TIMEGETTIME();

   do {
	   cPacket packet;
	   socklen_t address_len = sizeof(struct sockaddr_in);

		//
		// If we appear to crash INSIDE recvfrom then this tends to indicate
		// that net neighbourhood broke.
		//
		   retcode = wwnet::SocketRecvFrom(sock, packet.Get_Data(), packet.Get_Max_Size(),
			   0, (sockaddr*)&packet.Get_From_Address_Wrapper()->FromAddress, &address_len);

			if (retcode == SOCKET_ERROR) {
				if (wwnet::SocketGetLastError() != WSAEWOULDBLOCK) {
					WSA_ERROR;
				}
		} else {
			/*
			//
			// diagnostic
			//
			ULONG ip = packet.Get_From_Address_Wrapper()->FromAddress.sin_addr.s_addr;
			WWDEBUG_SAY(("cNetUtil::Lan_Servicing: %s\n", cNetUtil::Address_To_String(ip)));
			*/

			//packet.Set_Received_Length(retcode);
			packet.Set_Bit_Length(retcode * 8);
			(*p_callback)(packet);
		}
	} while (retcode != SOCKET_ERROR); // this will indicate no more data

   unsigned int time_spent = TIMEGETTIME() - start_time;
   if (time_spent > 100) {
      WWDEBUG_SAY(("*** cNetUtil::Lan_Servicing: Too much time (%d ms)) spent receiving lan packets.\n",
         time_spent));
   }
}
