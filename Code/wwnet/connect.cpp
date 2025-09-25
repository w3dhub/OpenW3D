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
// Filename:     connect.cpp
// Project:      wwnet
// Author:       Tom Spencer-Smith
// Date:         June 1998
// Description:
//
//------------------------------------------------------------------------------------
#include	"always.h"

#include "connect.h" // I WANNA BE FIRST!

//#include <stdlib.h>

#include "systimer.h"

#include "systimer.h"
#include "miscutil.h"
#include "netutil.h"
#include "singlepl.h"
#include "mathutil.h"
#include "wwdebug.h"
#include "wwmath.h"
#include "fromaddress.h"
#include "crc.h"
#include "msgstatlist.h"
#include "wwprofile.h"
#include "Commando/nat.h"
#include "Commando/natter.h"
#include "packetmgr.h"
#include "BWBalance.h"
#include <cstdio>
#include <algorithm>
#include "socket_wrapper.h"

#ifdef WWDEBUG
#include "Combat/crandom.h"

int cConnection::LatencyAddLow = 0;
int cConnection::LatencyAddHigh = 0;
int cConnection::CurrentLatencyAdd = 0;
unsigned long cConnection::LastLatencyChange = 0;

#endif //WWDEBUG

//
// class statics
//
BOOL cConnection::IsFlowControlEnabled				= true;
UINT cConnection::TotalCompressedBytesSent		= 0;
UINT cConnection::TotalUncompressedBytesSent		= 0;

static const int		INVALID_RHOST_ID			= -1;


//#ifdef WWDEBUG
/***********************************************************************************************
 * Addr_As_String -- Get a human readable internet address                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Address ptr                                                                       *
 *                                                                                             *
 * OUTPUT:   String representation                                                             *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/31/2001 3:48PM ST : Created                                                             *
 *=============================================================================================*/

#if defined(_WIN32) && (!defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0600)
#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif
static const char* inet_ntop(int af, const void* src, char* dst, size_t size) {
	if (af == AF_INET) {
		auto a = static_cast<const in_addr*>(src);
		const char* s = ::inet_ntoa(*a);
		if (!s) return nullptr;
		strncpy_s(dst, size, s, _TRUNCATE);
		return dst;
	}
	::WSASetLastError(WSAEAFNOSUPPORT);
	return nullptr;
}
#endif


const char* Addr_As_String(const sockaddr_in* addr)
{
	static char out[128];
	char ip[INET_ADDRSTRLEN] = { 0 };
	if (!inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip))) {
		const auto* bytes = reinterpret_cast<const uint8_t*>(&addr->sin_addr);
		std::snprintf(ip, sizeof(ip), "%u.%u.%u.%u", bytes[0], bytes[1], bytes[2], bytes[3]);
	}

	const unsigned port = (unsigned)ntohs(addr->sin_port);
	std::snprintf(out, sizeof(out), "%s ; %u", ip[0] ? ip : "0.0.0.0", port);
	return out;
}
//#endif //WWDEBUG


//------------------------------------------------------------------------------------
cConnection::cConnection() :
   NumRHosts(0),
   RefusalPacketSendId(0),
   HighestRefusalPacketRcvId(-1),
   MinRHost(-1),							// should be altered if IsServer
   //MaxRHost(0),							// should be altered if IsServer
   MaxRHost(-2),							// should be altered if IsServer
	InitDone(false),
	IsServer(false),
	IsDedicatedServer(false),
   SimulatedPacketLossPerRANDMAX(0),
   SimulatedPacketDuplicationPerRANDMAX(0),
   MaxAcceptablePacketlossPc(0),
   MinimumLatencyMs(0),
   MaximumLatencyMs(0),
	BandwidthBudgetOut(0),
	ServiceCount(0),
	ThisFrameTimeMs(TIMEGETTIME()),
	IsDestroy(false),
	PRHost(NULL),
	AcceptHandler(NULL),
	RefusalHandler(NULL),
	ServerBrokenConnectionHandler(NULL),
	ClientBrokenConnectionHandler(NULL),
	EvictionHandler(NULL),
	ConnHandler(NULL),
	ApplicationAcceptanceHandler(NULL),
	ServerPacketHandler(NULL),
	ClientPacketHandler(NULL),
	IsBadConnection(false),
	ExtraTimeoutTime(0),
	ExtraTimeoutTimeStarted(0),
	CanProcess(true)
{
	WWDEBUG_SAY(("cConnection::cConnection\n"));

   //
	// All this stuff is common to C & S
	//

   if (!cSinglePlayerData::Is_Single_Player()) {

      cNetUtil::Create_Unbound_Socket(Sock);

      //
      // Make socket non-blocking
      //
	  wwnet::SocketIoctlParam arg = 1L;
	  WSA_CHECK(wwnet::SocketIoctl(Sock, FIONBIO, &arg));

      //
      // Increase the send and rcv buffer sizes a bit
      //
		cNetUtil::Set_Socket_Buffer_Sizes(Sock);
	}

   /*
	for (int rhost_id = 0; rhost_id < MAX_RHOSTS; rhost_id++) {
      PRHost[rhost_id] = NULL;
   }
	*/

	//Init_Stats();

	PStatList = new cMsgStatList;
	WWASSERT(PStatList != NULL);
	PStatList->Init(PACKETTYPE_COUNT);
	for (int i = 0; i < PStatList->Get_Num_Stats(); i++) {
		//PStatList->Set_Name(i, Type_Translation(i));
		char message_trans[200];
		::strcpy(message_trans, Type_Translation(i));
		PStatList->Set_Name(i, &message_trans[11]);
	}
}

//------------------------------------------------------------------------------------
cConnection::~cConnection()
{
	WWDEBUG_SAY(("cConnection::~cConnection\n"));

	//Remove_All();

   if (!cSinglePlayerData::Is_Single_Player()) {
		//
		// Abortively shut down the socket
		//
      WSA_CHECK(shutdown(Sock, 2)); // SD_BOTH
      WSA_CHECK(::closesocket(Sock));
   }

   //for (int rhost_id = 0; rhost_id < MAX_RHOSTS; rhost_id++) {
   //for (int rhost_id = 0; rhost_id < MaxRHost; rhost_id++) {
   for (int rhost_id = MinRHost; rhost_id <= MaxRHost; rhost_id++) {
      if (PRHost[rhost_id] != NULL) {
         Destroy_Connection(rhost_id);
		}
   }

	delete [] PRHost;
	PRHost = NULL;

	delete PStatList;
	PStatList = NULL;

#ifdef WWDEBUG
	while (LaggedPackets.Count()) {
		delete LaggedPackets[0];
		LaggedPackets.Delete(0);
	}
	LaggedPacketTimes.Delete_All();
#endif //WWDEBUG
}

//------------------------------------------------------------------------------------
void cConnection::Init_Stats()
{
   CombinedStats.Init_Net_Stats();
   AveragedStats.Init_Net_Stats();

	for (int rhost_id = MinRHost; rhost_id <= MaxRHost; rhost_id++) {
		if (PRHost[rhost_id] != NULL) {
         PRHost[rhost_id]->Init_Stats();
      }
   }

	ThisFrameTimeMs = TIMEGETTIME();
}

//------------------------------------------------------------------------------------
void cConnection::Init_As_Client(struct sockaddr_in* p_server_address, unsigned short my_port)
{
   WWASSERT(p_server_address != NULL);
   WWASSERT(!InitDone);

   IsServer = false;
   IsDedicatedServer = false;
   LocalId = ID_UNKNOWN;

	//
	// Now the port is supplied by the firewall negotiation code. We have to use the same port here that the firewall code
	// was bound to.
	//
	// The server address and port are supplied by the firewall code too.
	//
   if (!cSinglePlayerData::Is_Single_Player()) {
      bool is_bound;
		is_bound = Bind(my_port);
      WWASSERT(is_bound);
   }

	//
	// Create rhost for server
	//
	MinRHost = 0;
	MaxRHost = 0;

	typedef cRemoteHost * PcRemoteHost;
	PRHost = new PcRemoteHost[1];
	WWASSERT(PRHost != NULL);

	PRHost[0] = new cRemoteHost();
   WWASSERT(PRHost[0] != NULL);
	PRHost[0]->Set_Id(0);//TSS2001
   NumRHosts++;
   WWASSERT(NumRHosts == 1);
   if (!cSinglePlayerData::Is_Single_Player()) {
      PRHost[0]->Set_Address(*p_server_address);
      WWASSERT(cNetUtil::Is_Same_Address(&PRHost[0]->Get_Address(), p_server_address));
   }

	Init_Stats();

	InitDone = true;
}

//------------------------------------------------------------------------------------
void cConnection::Init_As_Client(ULONG server_ip, USHORT server_port, unsigned short my_port)
{
   WWDEBUG_SAY(("cConnection::Init_As_Client(%s, %d, %d)\n",
		cNetUtil::Address_To_String(server_ip), server_port, my_port));

   WWASSERT(!InitDone);

	WWASSERT(server_port >= MIN_SERVER_PORT && server_port <= MAX_SERVER_PORT);

   struct sockaddr_in server_address;
	ZeroMemory(&server_address, sizeof(server_address));

   if (!cSinglePlayerData::Is_Single_Player()) {
		server_address.sin_family			= AF_INET;
		server_address.sin_addr.s_addr	= server_ip;
		server_address.sin_port				= ::htons(server_port);
   }

   Init_As_Client(&server_address, my_port);
}

//------------------------------------------------------------------------------------
void cConnection::Init_As_Server(USHORT server_port, int max_players,
	bool is_dedicated_server, ULONG addr)
{
   WWDEBUG_SAY(("cConnection::Init_As_Server\n"));

   WWASSERT(!InitDone);
   WWASSERT(server_port >= MIN_SERVER_PORT && server_port <= MAX_SERVER_PORT);
	WWASSERT(max_players >= 1);
	//WWASSERT(max_players < MAX_RHOSTS); // because MAX_RHOSTS is an array bound

   MinRHost = 1;
   MaxRHost = max_players;

	typedef cRemoteHost * PcRemoteHost;
	PRHost = new PcRemoteHost[max_players + 1];
	WWASSERT(PRHost != NULL);
	//ZeroMemory(PRHost, sizeof(PRHost));
	for (int rhost_id = MinRHost; rhost_id <= MaxRHost; rhost_id++) {
      PRHost[rhost_id] = NULL;
   }

 	Init_Stats();

	IsServer = true;
	IsDedicatedServer = is_dedicated_server;
   LocalId = 0;

	if (cSinglePlayerData::Is_Single_Player()) {
      IsFlowControlEnabled = false;
   } else {
      //
      // The server has to ::bind to his well-known port... or the nearest he can get
      // to it...
      //
      bool is_bound;
      int num_tries = 0;
      do {
         is_bound = Bind(server_port, addr);

         if (!is_bound) {
            WWDEBUG_SAY(("Failed to Bind to local port %d.\n", server_port));
            server_port++;
            num_tries++;
         }
      } while (!is_bound && num_tries < 50 && server_port <= MAX_SERVER_PORT);

      WWASSERT(num_tries < 50 && server_port <= MAX_SERVER_PORT);

		// Tell the firewall code that we started a new local server.
		WOLNATInterface.Set_Server(true);
   }

	InitDone = true;
}

//------------------------------------------------------------------------------------
bool cConnection::Bind(USHORT port, ULONG addr)
{
   WWASSERT(!cSinglePlayerData::Is_Single_Player());

   WWASSERT(!InitDone);
	//WWASSERT((!IsServer && port == 0) ||
	WWASSERT((!IsServer) ||
		    (IsServer && port >= MIN_SERVER_PORT && port <= MAX_SERVER_PORT));

   struct sockaddr_in address;
   cNetUtil::Create_Local_Address(&address, port);

   if (addr) address.sin_addr.s_addr = htonl(addr);

   if (::bind(Sock, (struct sockaddr*) &address, sizeof(struct sockaddr_in)) != SOCKET_ERROR) {
      LocalPort = port;
      WWDEBUG_SAY(("Bound to local port %d.\n", LocalPort));
      return true;
   } else {
      //
      // Any excuse other than address/port already used, is fatal.
      //
      if (::WSAGetLastError() != WSAEADDRINUSE) {
			WSA_ERROR;
      }
      return false;
   }

   //DIE; // won't get here
}

//------------------------------------------------------------------------------------
#define ADD_CASE(exp) case exp: return #exp;
LPCSTR cConnection::Type_Translation(int type)
{
	switch (type) {
		ADD_CASE(PACKETTYPE_UNRELIABLE);
		ADD_CASE(PACKETTYPE_RELIABLE);
		ADD_CASE(PACKETTYPE_ACK);
		ADD_CASE(PACKETTYPE_KEEPALIVE);
		ADD_CASE(PACKETTYPE_CONNECT_CS);
		ADD_CASE(PACKETTYPE_ACCEPT_SC);
		ADD_CASE(PACKETTYPE_REFUSAL_SC);
		ADD_CASE(PACKETTYPE_FIREWALL_PROBE);

	default:
		DIE;
		return ""; // to avoid warning
	}
}

//------------------------------------------------------------------------------------
bool cConnection::Sender_Id_Tests(cPacket & packet)
{
   WWASSERT(InitDone);

	int sender_id = packet.Get_Sender_Id();

   //
   // Clients should only receive packets from sender_id 0
   //
   if (!IsServer && sender_id != SERVER_RHOST_ID) {
		WWDEBUG_SAY(("Warning: Client received packet from non-server id %d\n", sender_id));
      CombinedStats.Increment_Stat_Sample(STAT_DiscardCount, 1);
		return false;
   }

   WWASSERT(sender_id >= MinRHost && sender_id <= MaxRHost);
   if (sender_id < MinRHost || sender_id > MaxRHost) {
		WWDEBUG_SAY(("Warning: Packet with bad sender_id %d\n", sender_id));
		return(false);
	}

   if (PRHost[sender_id] == NULL) {
		//
		// This can happen when the connection is broken... packets in-the-air
		// may still arrive
		//
		BYTE packet_type;
		packet_type = packet.Get_Type();
		WWASSERT(packet_type >= PACKETTYPE_FIRST && packet_type <= PACKETTYPE_LAST);
		WWDEBUG_SAY(("Packet from broken connection discarded: type %d, id %d, sender %d\n",
			packet_type, packet.Get_Id(), sender_id));
      CombinedStats.StatSample[STAT_DiscardCount]++;
		return false;
	}

   if (!cSinglePlayerData::Is_Single_Player() &&
      !cNetUtil::Is_Same_Address(&(PRHost[sender_id]->Get_Address()),
      &packet.Get_From_Address_Wrapper()->FromAddress)) {
      //
      // This can happen under 2 known conditions:
      // 1. A new player reuses an id and old packets from the previous player
      //    are still on the wire.
      // 2. The client has hacked the internal id field
      //
      // At this point we will just discard the packet.
      //
		WWDEBUG_SAY(("Warning: Packet sender id (%d) conflicts with actual send address\n", sender_id));
      CombinedStats.StatSample[STAT_DiscardCount]++;
		return false;
   }

   return true;
}

//------------------------------------------------------------------------------------
USHORT cConnection::Calculate_Packet_Bits(USHORT app_bytes)
{
   //USHORT packet_bits = 0;

   //
   // From the app bytes, work out an approximate expected number
   // of bits. Assume modem and PPP.
   //
   // 1. Add header bytes
   // 2. Assume worst case of PPP byte escaping: all of 0x00-0x1F and 0x7d, 0x7e
   //    = 34 bytes, so multiply by (256 + 34) / 256
   // 3. Multiple by 10 bits/byte for modem traffic (assume serial connection,
   //    1 start bit and 1 stop bit)
   //
   // So we have (app bytes + header bytes) * (256 + 34) / 256 * 10
   //
   // Assumption: no IP fragmentation occurs because we are keeping the
   // total packet size small (less than the IP MTU of 576).
   //
   // Our application will probably be targeting a client average inbound bps
   // of 28800 or less.
   //

	//
	// Header (assuming PPP):
	//
	//   IP 4.0 : 20 Bytes
	//   UDP    : 8 Bytes
	//   PPP    : 5 Bytes (typical value after negotiation)
	//           ---------
	//            33 bytes
	//
	//HeaderBytes = 33;

   USHORT packet_bits = (USHORT)((app_bytes + 33) * 11.328125);

   return packet_bits;
}

//------------------------------------------------------------------------------------
void cConnection::Set_Packet_Loss(double percent_lost)
{
   //
   // This sets simultedpacketloss to a common value for all rhosts
   // as well as for non-established connections
   //

   WWASSERT(percent_lost >= 0 && percent_lost <= 100);

   SimulatedPacketLossPerRANDMAX = (UINT) cMathUtil::Round(
		percent_lost / 100.0 * RAND_MAX);

	WWDEBUG_SAY(("cConnection::Set_Packet_Loss: %d / %d\n", SimulatedPacketLossPerRANDMAX, RAND_MAX));
}

//------------------------------------------------------------------------------------
void cConnection::Set_Packet_Duplication(double percent_duplicated)
{
   //
   // This sets simultedpacketloss to a common value for all rhosts
   // as well as for non-established connections
   //

   WWASSERT(percent_duplicated >= -WWMATH_EPSILON &&
      percent_duplicated <= 100 + WWMATH_EPSILON);

   //
   // Globally:
   //
   SimulatedPacketDuplicationPerRANDMAX = (UINT) cMathUtil::Round(
		percent_duplicated / 100.0 * RAND_MAX);

	WWDEBUG_SAY(("cConnection::Set_Packet_Duplication: %d / %d\n",  SimulatedPacketDuplicationPerRANDMAX, RAND_MAX));
}

//------------------------------------------------------------------------------------
void cConnection::Set_Packet_Latency_Range(int minimum_latency_ms, int maximum_latency_ms)
{
   WWASSERT(minimum_latency_ms >= 0);
   WWASSERT(maximum_latency_ms >= 0);
   WWASSERT(maximum_latency_ms >= minimum_latency_ms);

   MinimumLatencyMs = minimum_latency_ms;
   MaximumLatencyMs = maximum_latency_ms;
}

//------------------------------------------------------------------------------------
int cConnection::Single_Player_recvfrom(char * data)
{
	//WWDEBUG_SAY(("cConnection::Single_Player_recvfrom\n"));

   //
   // TSS - this relies on doing all recvs in one shot
   //

   WWASSERT(cSinglePlayerData::Is_Single_Player());

   int ret_code;

	SList<cPacket> * p_packet_list;
   if (IsServer) {
		p_packet_list = cSinglePlayerData::Get_Input_Packet_List(SERVER_LIST);
	} else {
		p_packet_list = cSinglePlayerData::Get_Input_Packet_List(CLIENT_LIST);
	}
	WWASSERT(p_packet_list != NULL);

   SLNode<cPacket> * objnode = p_packet_list->Head();
   if (objnode == NULL) {
      WSASetLastError(WSAEWOULDBLOCK);
      ret_code = SOCKET_ERROR; // no data received
   } else {

      cPacket * p_packet = objnode->Data();
      WWASSERT(p_packet != NULL);
      memcpy(data, p_packet->Get_Data(), p_packet->Get_Max_Size());

		ret_code = p_packet->Get_Compressed_Size_Bytes();

      p_packet->Flush();
      p_packet_list->Remove(p_packet);
      delete p_packet;
   }

   return ret_code;
}

//------------------------------------------------------------------------------------
//
// Return true if we receive a valid packet on this call
//
bool cConnection::Receive_Packet()
{
	//WWDEBUG_SAY(("cConnection::Receive_Packet start\n"));

   WWASSERT(InitDone);

   cPacket packet;

	int ret_code = 0;

#if 0 //def WWDEBUG // TODO LaggedPacketRetCodes.Delete(p) ambiguous
	//
	// See if there are any old packets with simulated lag whos time has come.
	//
	if (LaggedPacketTimes.Count()) {
		unsigned long time_now = TIMEGETTIME();
		for (int p=0 ; p<LaggedPacketTimes.Count() ; p++) {
			if (LaggedPacketTimes[p] <= time_now) {
				packet = *LaggedPackets[p];
				delete LaggedPackets[p];
				LaggedPackets.Delete(p);
				LaggedPacketTimes.Delete(p);
				ret_code = LaggedPacketRetCodes[p];
				LaggedPacketRetCodes.Delete(p);
				break;
			}
		}
	}
#endif //WWDEBUG

   if (ret_code == 0) {
		ret_code = Receive_Wrapper(packet);

		if (ret_code == 0) {
			return(false);
		}
		if (cSinglePlayerData::Is_Single_Player() && ret_code == SOCKET_ERROR) {
			return(false);
		}


#ifndef WRAPPER_CRC
   	if (!packet.Is_Crc_Correct()) {
#ifdef WWDEBUG
			sockaddr_in *addr_ptr = (struct sockaddr_in*) &packet.Get_From_Address_Wrapper()->FromAddress;
#endif //WWDEBUG
      	WWDEBUG_SAY(("*** CRC FAILURE: PACKET DISCARDED ***\n"));
      	WWDEBUG_SAY(("*** CRC FAILURE: Packet from %s\n", Addr_As_String(addr_ptr)));
      	packet.Flush();
      	return true;
   	}
#endif //WRAPPER_CRC

#ifdef WWDEBUG
		//
		// Add simulated latency if required.
		//
		if (LatencyAddLow || LatencyAddHigh) {
			cPacket *new_packet = new cPacket;
			*new_packet = packet;
			unsigned long time = TIMEGETTIME();

			const int latency_adjust_delay = 1000 * 10;
			if (time - LastLatencyChange > latency_adjust_delay) {
				LastLatencyChange = time;
				if (LatencyAddLow == LatencyAddHigh) {
					CurrentLatencyAdd = LatencyAddLow;
				} else {
					CurrentLatencyAdd = FreeRandom.Get_Int(LatencyAddLow, LatencyAddHigh);
				}
			}

			time += CurrentLatencyAdd;
			LaggedPackets.Add(new_packet);
			LaggedPacketTimes.Add(time);
			LaggedPacketRetCodes.Add(ret_code);
			packet.Flush();
			return(true);
		}
#endif //WWDEBUG
	}

	//
	// Intercept packets intended for the firewall negotiation code.
	//
   if (packet.Get_Type() == PACKETTYPE_FIREWALL_PROBE) {
		WOLNATInterface.Intercept_Game_Packet(packet);
		packet.Flush();
      WWDEBUG_SAY(("cConnection:: Packet transferred to WOLNAT interface\n"));
		return(true);
	};


	//
	// A ret_code of zero indicates either:
	// 1. A graceful closure of the socket, or
	// 2. A zero-data packet.
	//
	// We are never going to attempt to recv after we have closed it, and
	// we never send zero-length packets, so assert this.
	//
	WWASSERT(ret_code > 0);

   //
	// Measurement stats
	//
	USHORT packet_bits = Calculate_Packet_Bits(ret_code);

   int addressee = Address_To_Rhostid(&packet.Get_From_Address_Wrapper()->FromAddress);
   if (addressee != INVALID_RHOST_ID) {
      PRHost[addressee]->Get_Stats().StatSample[STAT_PktRcv]++;
      PRHost[addressee]->Get_Stats().StatSample[STAT_AppByteRcv] += ret_code;
      //PRHost[addressee]->Get_Stats().StatSample[STAT_HdrByteRcv] += cNetUtil::Get_Header_Bytes();
      PRHost[addressee]->Get_Stats().StatSample[STAT_BitsRcv] += packet_bits;

		PRHost[addressee]->Set_Last_Contact_Time(TIMEGETTIME());
   }

	//
	// Aliases
	//
	const int packet_id = packet.Get_Id();
	const int sender_id = packet.Get_Sender_Id();
	struct sockaddr_in * p_from_address = &packet.Get_From_Address_Wrapper()->FromAddress;
	WWASSERT(p_from_address != NULL);
	cRemoteHost * p_sender_rhost = NULL;
	if (sender_id != cPacket::UNDEFINED_ID) {
		p_sender_rhost = PRHost[sender_id];

		//WWASSERT(p_sender_rhost != NULL);
      if (p_sender_rhost == NULL) {
			packet.Flush();
			WWDEBUG_SAY(("Packet from null rhost (%d) discarded.\n", sender_id));
			return true;
		}
	}

   switch (packet.Get_Type()) {

      case PACKETTYPE_KEEPALIVE: {
				//WWDEBUG_SAY(("CONNECT: PACKETTYPE_KEEPALIVE received\n"));

            if (!Sender_Id_Tests(packet)) {
					WWDEBUG_SAY(("PACKETTYPE_KEEPALIVE flushed due to Sender_Id_Tests. Packet id = %d\n", packet_id));
					packet.Flush();
               return true;
            }

			//
			// TSS110201
			//
			if ( LocalId == ID_UNKNOWN) {
				WWDEBUG_SAY(("PACKETTYPE_KEEPALIVE flushed due to LocalId == ID_UNKNOWN. Packet id = %d\n", packet_id));
				packet.Flush();
				return true;
			}

            //
            // The main purpose of the keepalive is to stimulate this ack.
            //
            Send_Ack(p_from_address, packet_id);

				float packetloss_pc = packet.Get(packetloss_pc);

				WWASSERT(p_sender_rhost != NULL);
				cNetStats & sender_stats = p_sender_rhost->Get_Stats();
            sender_stats.Set_Pc_Packetloss_Sent(packetloss_pc);

            int remote_service_count = packet.Get(remote_service_count);
            sender_stats.Set_Remote_Service_Count(remote_service_count);

            //
            // We still have to process the keepalive in the rcv list so that
            // we step past it's reliable id. It will be discarded at that
				// point, and not passed up to the application level.
            //
            WWASSERT(packet.Is_Flushed());

				p_sender_rhost->Add_Packet(packet, RELIABLE_RCV_LIST);
		      return true;
         }

      case PACKETTYPE_CONNECT_CS: {
				WWDEBUG_SAY(("CONNECT: PACKETTYPE_CONNECT_CS received\n"));

            WWASSERT(IsServer);
            WWASSERT(sender_id == ID_UNKNOWN);

            //
            // Reliable message, ack it.
            //
            Send_Ack(p_from_address, packet_id);

            Process_Connection_Request(packet);

				return true;
         }

      case PACKETTYPE_ACCEPT_SC: {
				//WWDEBUG_SAY(("cConnection::Receive_Packet : PACKETTYPE_ACCEPT_SC received\n"));
				WWDEBUG_SAY(("CONNECT: PACKETTYPE_ACCEPT_SC received\n"));

            WWASSERT(!IsServer);

            if (LocalId != ID_UNKNOWN) {
               //
               // This is a duplicate packet... discard here.
               //
               CombinedStats.StatSample[STAT_DiscardCount]++;
					WWDEBUG_SAY(("PACKETTYPE_ACCEPT_SC flushed due to LocalId != ID_UNKNOWN. Packet id = %d\n", packet_id));
               Send_Ack(p_from_address, packet_id);
               packet.Flush();
            } else {

               WWASSERT(sender_id == SERVER_RHOST_ID);

					packet.Get(LocalId); // This is where we learn our id
               WWDEBUG_SAY(("  Received LocalId:%d\n", LocalId));
               Send_Ack(p_from_address, packet_id);

               //
               // Now, client may do something if he wants
               //
               //Accept_Handler();
					WWASSERT(AcceptHandler != NULL);
					AcceptHandler();

               //
               // We still have to process the accept in the rcv list so that
               // we step past it's reliable id. It will be discarded at that point
               // and not passed up to the application level.
               //
					WWASSERT(packet.Is_Flushed());

					WWASSERT(p_sender_rhost != NULL);
               p_sender_rhost->Add_Packet(packet, RELIABLE_RCV_LIST);
            }

		      return true;
         }

      case PACKETTYPE_REFUSAL_SC: {
				WWDEBUG_SAY(("cConnection::Receive_Packet : PACKETTYPE_REFUSAL_SC received\n"));

            WWASSERT(!IsServer);

            if (LocalId == ID_UNKNOWN) {

               WWASSERT(sender_id == SERVER_RHOST_ID);

               if (packet_id > HighestRefusalPacketRcvId) {
                  HighestRefusalPacketRcvId = packet_id;

						int refusal_code = packet.Get(refusal_code);
				      //Refusal_Handler(refusal_code);
						WWASSERT(RefusalHandler != NULL);
						RefusalHandler((REFUSAL_CODE)refusal_code);
						IsDestroy = true;
               } else {
                  //
                  // It's a duplicate
                  //
                  packet.Flush();
               }
            } else {
               //
               // This is an inappropriate packet... discard here.
               // TSS - bug - duplicates not handled properly - sent * 5
               //
               CombinedStats.StatSample[STAT_DiscardCount]++;
               packet.Flush();
            }

		      return true;
         }

      case PACKETTYPE_UNRELIABLE: {
				//WWDEBUG_SAY(("CONNECT: PACKETTYPE_UNRELIABLE received\n"));

				//
            // Discard all unreliable packets until we have an id.
            //
				if (LocalId == ID_UNKNOWN) {
               packet.Flush();
					WWDEBUG_SAY(("Unreliable packet flushed due to unknown id.\n"));
               return true;
				}

				//
            // Discard data with an address mismatch.
            //
				if (!Sender_Id_Tests(packet)) {
               packet.Flush();
					WWDEBUG_SAY(("Unreliable packet flushed due to address mismatch.\n"));
               return true;
				}

				//
				// Discard out-of-date data.
            //
				WWASSERT(p_sender_rhost != NULL);
				if (packet_id < p_sender_rhost->Get_Unreliable_Packet_Rcv_Id()) {
               packet.Flush();
					//WWDEBUG_SAY(("Unreliable packet flushed due to being out-of-date.\n"));
               return true;
				}

            //
			   // Keep track of how many of each packet is received
			   //
				cNetStats & sender_stats = p_sender_rhost->Get_Stats();
   			sender_stats.StatSample[STAT_MsgRcv]++;
				sender_stats.StatSample[STAT_UPktRcv]++;
				sender_stats.StatSample[STAT_UByteRcv] += ret_code;

            if (packet_id > sender_stats.Get_Freeze_Packet_Id()) {
   				sender_stats.StatSample[STAT_UPktRcv2]++;
					sender_stats.Increment_Unreliable_Count();
            }

            if (packet_id > sender_stats.Get_Last_Unreliable_Packet_Id()) {
					sender_stats.Set_Last_Unreliable_Packet_Id(packet_id);
				}

            p_sender_rhost->Add_Packet(packet, UNRELIABLE_RCV_LIST);

			   return true;
         }


      case PACKETTYPE_RELIABLE: {

				//WWDEBUG_SAY(("CONNECT: PACKETTYPE_RELIABLE received\n"));
				//
            // Discard all reliable packets until we have an id.
				// We still don't have LocalId, therefore we can't Ack this.
				//
				if (LocalId == ID_UNKNOWN) {
               packet.Flush();
					WWDEBUG_SAY(("Reliable packet %d flushed due to unknown id.\n", packet_id));
               return true;
				}

				//
            // Discard data with an address mismatch.
				//
				if (!Sender_Id_Tests(packet)) {
               packet.Flush();
					WWDEBUG_SAY(("Reliable packet %d flushed due to address mismatch.\n", packet_id));
               return true;
				}

            Send_Ack(p_from_address, packet_id);

            //
			   // Keep track of how many of each packet is received
			   //

				WWASSERT(p_sender_rhost != NULL);
				cNetStats & sender_stats = p_sender_rhost->Get_Stats();
   			sender_stats.StatSample[STAT_MsgRcv]++;
            sender_stats.StatSample[STAT_RPktRcv]++;
            sender_stats.StatSample[STAT_RByteRcv] += ret_code;

            p_sender_rhost->Add_Packet(packet, RELIABLE_RCV_LIST);

				return true;
         }

      case PACKETTYPE_ACK: {
				//WWDEBUG_SAY(("CONNECT: PACKETTYPE_ACK received\n"));
            //WWDEBUG_SAY(("(Received Ack for packet %d)\n", packet_id));

            if (!Sender_Id_Tests(packet)) {
               return true;
            }

            WWASSERT(p_sender_rhost != NULL);
				cNetStats & sender_stats = p_sender_rhost->Get_Stats();
            sender_stats.StatSample[STAT_AckCountRcv]++;

            p_sender_rhost->Remove_Packet(packet_id, RELIABLE_SEND_LIST);

            return true;
         }

      default:
         DIE;
         break;
   }

   DIE; // shouldn't get here
   return true;
}

//-----------------------------------------------------------------------------
void cConnection::Process_Connection_Request(cPacket & packet)
{
	struct sockaddr_in* p_address = &packet.Get_From_Address_Wrapper()->FromAddress;

   WWASSERT(InitDone);
   WWASSERT(IsServer);

   int new_rhost_id = ID_UNKNOWN;

	//
   // Make sure we don't already know him, and find him a slot
   //
   for (int player_id = MinRHost; player_id <= MaxRHost; player_id++) {
		if (PRHost[player_id] != NULL) {
         if (!cSinglePlayerData::Is_Single_Player() &&
            cNetUtil::Is_Same_Address(&(PRHost[player_id]->Get_Address()),
            p_address)) {
            //
            // He already has an id. This must be a resend or duplicate.
            //
            return;
         }
		} else if (new_rhost_id == ID_UNKNOWN) {
			new_rhost_id = player_id;
		}
   }

	if (new_rhost_id == ID_UNKNOWN) {

      WWDEBUG_SAY(("  Warning: server cannot accept this client; no free slots\n"));
		Send_Refusal_Sc(p_address, REFUSAL_GAME_FULL);

	} else {

		WWASSERT(ApplicationAcceptanceHandler != NULL);
		REFUSAL_CODE refusal = ApplicationAcceptanceHandler(packet);

		if (refusal != REFUSAL_CLIENT_ACCEPTED) {
			packet.Flush();
			Send_Refusal_Sc(p_address, refusal);
			return;
		}

		//
		// TSS091701
		//
		int bbo = packet.Get(bbo);
		WWDEBUG_SAY(("New clients BBO is %d\n", bbo));

      WWASSERT(PRHost[new_rhost_id] == NULL);
	   PRHost[new_rhost_id] = new cRemoteHost();
      WWASSERT(PRHost[new_rhost_id] != NULL);
		PRHost[new_rhost_id]->Set_Id(new_rhost_id);//TSS2001
      NumRHosts++;
      WWASSERT(NumRHosts <= MaxRHost - MinRHost + 1);
      PRHost[new_rhost_id]->Set_Address(*p_address);
		PRHost[new_rhost_id]->Set_Maximum_Bps(bbo);

      Send_Accept_Sc(new_rhost_id);

      //Connection_Handler(new_rhost_id);
		WWASSERT(ConnHandler != NULL);
		ConnHandler(new_rhost_id);

		//
		// Although processed individually, PACKETTYPE_CONNECT_CS
		// is essentially reliable, and we must track the id appropriately.
		//
		PRHost[new_rhost_id]->Get_Reliable_Packet_Rcv_Id();
		PRHost[new_rhost_id]->Increment_Reliable_Packet_Rcv_Id();
	}
}

//------------------------------------------------------------------------------------
int cConnection::Single_Player_sendto(cPacket & packet)
{
	//WWDEBUG_SAY(("cConnection::Single_Player_sendto\n"));

   WWASSERT(cSinglePlayerData::Is_Single_Player());

	SList<cPacket> * p_packet_list;
   if (IsServer) {
		p_packet_list = cSinglePlayerData::Get_Input_Packet_List(CLIENT_LIST);
	} else {
		p_packet_list = cSinglePlayerData::Get_Input_Packet_List(SERVER_LIST);
	}
	WWASSERT(p_packet_list != NULL);

   cPacket * p_packet = new cPacket;
   WWASSERT(p_packet != NULL);
   *p_packet = packet;
   p_packet_list->Add_Tail(p_packet);

   return packet.Get_Compressed_Size_Bytes();
}

//------------------------------------------------------------------------------------
int cConnection::Address_To_Rhostid(const struct sockaddr_in* p_address)
{
   WWASSERT(p_address != NULL);

   if (cSinglePlayerData::Is_Single_Player()) {
      return INVALID_RHOST_ID;
   }

   //
   // TSS - this searching is very inefficient.
   //
   for (int rhost_id = MinRHost; rhost_id <= MaxRHost; rhost_id++) {
      if (PRHost[rhost_id] != NULL &&
         cNetUtil::Is_Same_Address(&(PRHost[rhost_id]->Get_Address()), p_address)) {
         return rhost_id;
      }
   }

   return INVALID_RHOST_ID;
}


//#include "packetmgr.h"
//unsigned char last_packet[1024];
//unsigned char delta_packet[1024];
//unsigned char fixed_packet[1024];
//int last_packet_len = 0;

//------------------------------------------------------------------------------------
int cConnection::Low_Level_Send_Wrapper(cPacket & packet, struct sockaddr_in* p_address)
{
	WWASSERT(p_address != NULL);

   int ret_code = 0;

#if (0)
if (last_packet_len == (int)packet.Get_Compressed_Size_Bytes()) {
	int delta_size = PacketManagerClass::Build_Delta_Packet_Patch(last_packet, (unsigned char*)packet.Get_Data(), delta_packet, last_packet_len, last_packet_len);
	if (delta_size != -1) {
		WWDEBUG_SAY(("Packet size = %d, last packet delta = %d\n", last_packet_len, delta_size));
	}

	int bytes = PacketManagerClass::Reconstruct_From_Delta(last_packet, fixed_packet, delta_packet, last_packet_len);

	assert(bytes == last_packet_len);
	assert(memcmp(packet.Get_Data(), fixed_packet, last_packet_len) == 0);

}
last_packet_len = (int)packet.Get_Compressed_Size_Bytes();
memcpy(last_packet, packet.Get_Data(), last_packet_len);
#endif //(0)

   if (cSinglePlayerData::Is_Single_Player()) {
	   ret_code = Single_Player_sendto(packet);
   } else {
		//WWDEBUG_SAY(("cConnection: sendto %s\n", Addr_As_String(p_address)));

		//
		// Just pass the packet to the packet manager for deltaing and coagulation.
		//
		bool took = PacketManager.Take_Packet((unsigned char *)packet.Get_Data(), packet.Get_Compressed_Size_Bytes(), (unsigned char*)&p_address->sin_addr.s_addr, p_address->sin_port, Sock);

		if (!took) {
			WWDEBUG_SAY(("Low_Level_Send_Wrapper - Failed to pass packet to packet manager\n"));
			return(0);
			//ret_code = sendto(Sock, packet.Get_Data(),
			//	packet.Get_Compressed_Size_Bytes(), 0,
			//	(LPSOCKADDR) p_address, sizeof(struct sockaddr_in));
		} else {
			PacketManager.Flush();
			ret_code = packet.Get_Compressed_Size_Bytes();
			if (ret_code < 0) {
				ret_code = 0;
			}
		}

			//unsigned long bytes;
			//int result = ioctlsocket(Sock, FIONREAD, &bytes);
			//if (result == 0 && bytes != 0) {
			//	WWDEBUG_SAY(("ioctlsocket - bytes left to read = %d\n", bytes));
			//}

	}

   return ret_code;
}

//------------------------------------------------------------------------------------
int cConnection::Send_Wrapper(cPacket & packet, struct sockaddr_in* p_address)
{
   WWASSERT(p_address != NULL);

	cPacket full_packet;
	cPacket::Construct_Full_Packet(full_packet, packet);

	//
	// Update stats
	//
	BYTE packet_type = packet.Get_Type();
	WWASSERT(packet_type >= PACKETTYPE_FIRST && packet_type <= PACKETTYPE_LAST);
	PStatList->Increment_Num_Msg_Sent(packet_type);
	PStatList->Increment_Num_Byte_Sent(packet_type, full_packet.Get_Compressed_Size_Bytes());

   bool is_lost = false;

#ifdef WWDEBUG
   is_lost = (rand() < SimulatedPacketLossPerRANDMAX);
#endif

   int ret_code = 0;
   if (is_lost) {
      ret_code = full_packet.Get_Compressed_Size_Bytes(); // just don't send !
   } else {
		ret_code = Low_Level_Send_Wrapper(full_packet, p_address);
	}

   return ret_code;
}

//------------------------------------------------------------------------------------
int cConnection::Send_Wrapper(cPacket & packet, int addressee)
{
	WWASSERT(addressee >= 0);
   WWASSERT(addressee != ID_UNKNOWN);
   WWASSERT(PRHost[addressee] != NULL);
   return Send_Wrapper(packet, &(PRHost[addressee]->Get_Address()));//, addressee);
}

//------------------------------------------------------------------------------------
int cConnection::Low_Level_Receive_Wrapper(cPacket & packet)
{
	int ret_code = 0;

   if (cSinglePlayerData::Is_Single_Player()) {
		ret_code = Single_Player_recvfrom(packet.Get_Data());
	} else {

		const int max_errors = 250;
		int error_count = 0;
		int bytes = 0;

		//
		// Try getting a packet from the packet manager. Keep trying for a number of times if we get a socket error. If we don't
		// do this and just fail then we fall out of the receive packet loop and no more packets are received this frame. 15 of
		// these a second and we don't get any packets in at all.
		//
		unsigned char ip_address[4];
		unsigned short port = 0;

		while (error_count < max_errors) {
			bytes = PacketManager.Get_Packet(Sock, (unsigned char *)packet.Get_Data(), packet.Get_Max_Size(), ip_address, port);

			//
			// A return value of less than 0 indicates a fatal socket error. Try and ditch the offending client.
			//
			if (bytes < 0) {
				bytes = 0;
				error_count++;
				bool found_bad = false;
         	if (IsServer) {

					//
					// If we try to invoke the broken connection handler when loading a level it will access the player list
					// which will cause a DataSafe access from the wrong (main) thread with potentially catastrophic effects.
					// ST - 1/17/2002 11:18AM
					//
					if (CanProcess) {

						WWASSERT(ServerBrokenConnectionHandler != NULL);

						for (int i=MinRHost ; i<MaxRHost ; i++) {
							cRemoteHost *rhost_ptr = PRHost[i];

							if (rhost_ptr) {
								struct sockaddr_in rhost_addr = rhost_ptr->Get_Address();

								if (memcmp(ip_address, &rhost_addr.sin_addr.s_addr, 4) == 0) {
									if (rhost_addr.sin_port == port) {
										int rhost_id = rhost_ptr->Get_Id();
                  				Destroy_Connection(rhost_id);
										ServerBrokenConnectionHandler(rhost_id);
										found_bad = true;
										break;
									}
								}
							}
						}
						if (!found_bad) {
							WWDEBUG_SAY(("WSAECONNRESET address not in host list\n"));
						}
					}
				}
			} else {
				break;
			}
		}



		if (bytes) {
			sockaddr_in *addr_ptr = (struct sockaddr_in*) &packet.Get_From_Address_Wrapper()->FromAddress;
			memcpy(&addr_ptr->sin_addr.s_addr, ip_address, 4);
			addr_ptr->sin_port = port;
		}
		ret_code = bytes;

#if (0)
		socklen_t address_size = sizeof(struct sockaddr_in);
		ret_code = wwnet::SocketRecvFrom(Sock, packet.Get_Data(),
			packet.Get_Max_Size(), 0,
			(LPSOCKADDR)&packet.Get_From_Address_Wrapper()->FromAddress, &address_size);

		if (ret_code > 0) {
			//WWDEBUG_SAY(("cConnection: recvfrom %s\n", Addr_As_String((struct sockaddr_in*) &packet.Get_From_Address_Wrapper()->FromAddress)));
		}
#endif //(0)

		/*
		//
		// diagnostic
		//
		if (ret_code > 0) {
			ULONG ip = packet.Get_From_Address_Wrapper()->FromAddress.sin_addr.s_addr;
			WWDEBUG_SAY(("cConnection::Low_Level_Receive_Wrapper: %s\n",
				cNetUtil::Address_To_String(ip)));
		}
		/**/
	}

	return ret_code;
}

//------------------------------------------------------------------------------------
int cConnection::Receive_Wrapper(cPacket & packet)
{
	cPacket full_packet;
	int ret_code = Low_Level_Receive_Wrapper(full_packet);

	if (ret_code > 0) {
		//
		// We won't be able to read the header from the full packet unless we
		// set the bit length (approximately).
		// The returned packet object will have the exact bit length set correctly.
		//
		full_packet.Set_Bit_Length(ret_code * 8);

		cPacket::Construct_App_Packet(packet, full_packet);

#ifndef WRAPPER_CRC
		if (packet.Is_Crc_Correct()) {
#endif //WRAPPER_CRC
			//
			// Update receive stats
			//
			BYTE packet_type = packet.Get_Type();
			WWASSERT(packet_type >= PACKETTYPE_FIRST && packet_type <= PACKETTYPE_LAST);
			PStatList->Increment_Num_Msg_Recd(packet_type);
			PStatList->Increment_Num_Byte_Recd(packet_type, ret_code);
#ifndef WRAPPER_CRC
		}
#endif //WRAPPER_CRC
	}

	return ret_code;
}

/*
//------------------------------------------------------------------------------------
void cConnection::Handle_Send_Resource_Failure(int rhost_id)
{
   if (rhost_id != INVALID_RHOST_ID) {
		WWASSERT(PRHost[rhost_id] != NULL);
		PRHost[rhost_id]->Get_Stats().StatSample[STAT_SendFailureCount]++;
   }

   int orgbuffersize;
   int newbuffersize;
   int len;

	len = sizeof(int);
   WSA_CHECK(::getsockopt(Sock, SOL_SOCKET, SO_SNDBUF,
      (char *)&orgbuffersize, &len));

	static int time_of_last_reset = 0;
	int time_now = TIMEGETTIME();

	Clear_Resend_Counts();

	if (time_now - time_of_last_reset > 5000) {

		failcount++;

		float failure_ratio;
		if (succcount == 0) {
			failure_ratio = 1;
		} else {
			failure_ratio = failcount / (float) succcount;
		}

		succcount = 0;
		failcount = 0;
		extern int g_c_wouldblock;
		extern int g_c_nobufs;
		g_c_wouldblock = 0;
		g_c_nobufs = 0;

		if (orgbuffersize < 1000000) {
         //
         // 2-pronged approach. Firstly, increase send buffer size.
         // Secondly, if it was a significant failure, immediately
         // reduce bw out.
         //

			newbuffersize = 4 * orgbuffersize;
			len = sizeof(int);
			WSA_CHECK(setsockopt(Sock, SOL_SOCKET, SO_SNDBUF,
				(char *)&newbuffersize, len));

			len = sizeof(int);
			WSA_CHECK(::getsockopt(Sock, SOL_SOCKET, SO_SNDBUF,
				(char *)&newbuffersize, &len));

			WWDEBUG_SAY(("SO_SNDBUF %d -> %d\n",
				orgbuffersize, newbuffersize));

         if (failure_ratio > 0.05f) {
				time_of_last_reset = time_now;
         }
		} else {
         //
         // If we max out the send buffer and are still getting fails here
         // then we need to throttle back our output through this socket.
         //
         if (failure_ratio > 0.05f) {
				time_of_last_reset = time_now;
			}
		}
	}
}
*/

//------------------------------------------------------------------------------------
void cConnection::Send_Packet_To_Address(cPacket & packet, struct sockaddr_in* p_address)
{
   WWASSERT(p_address != NULL);

   WWASSERT(InitDone);

	// TSS - need reverse lookup of addressee from address
   int rhost_id = Address_To_Rhostid(p_address);
   if (rhost_id != INVALID_RHOST_ID) {
      WWASSERT(PRHost[rhost_id] != NULL);
   }

	if (rand() < SimulatedPacketDuplicationPerRANDMAX) {
		//
		// we'll send a duplicate
		//
		packet.Set_Num_Sends(packet.Get_Num_Sends() + 1);
	}

	//static int succcount = 0;
	//static int failcount = 0;

   for (int i = 0; i < packet.Get_Num_Sends(); i++) {

      int ret_code;
      if (rhost_id == INVALID_RHOST_ID) {
         ret_code = Send_Wrapper(packet, p_address);
      } else {
         ret_code = Send_Wrapper(packet, rhost_id);
      }

		if (SEND_RESOURCE_FAILURE(ret_code)) {

			//Handle_Send_Resource_Failure(rhost_id);
			WWDEBUG_SAY(("WARNING: cConnection::Send_Packet_To_Address : SEND_RESOURCE_FAILURE\n"));

      } else {

			//succcount++;
			TotalCompressedBytesSent	+= packet.Get_Compressed_Size_Bytes();
			TotalUncompressedBytesSent += packet.Get_Uncompressed_Size_Bytes();

         USHORT bits_sent = Calculate_Packet_Bits(packet.Get_Compressed_Size_Bytes());

         if (rhost_id != INVALID_RHOST_ID) {
            PRHost[rhost_id]->Get_Stats().StatSample[STAT_PktSent]++;
			   PRHost[rhost_id]->Get_Stats().StatSample[STAT_AppByteSent] += packet.Get_Compressed_Size_Bytes();
			   //PRHost[rhost_id]->Get_Stats().StatSample[STAT_HdrByteSent] += cNetUtil::Get_Header_Bytes();
			   PRHost[rhost_id]->Get_Stats().StatSample[STAT_BitsSent] += bits_sent;
         }
		}
   }
}

//------------------------------------------------------------------------------------
void cConnection::Set_R_And_U_Packet_Id(cPacket & packet, int addressee, BYTE send_type)
{
	WWASSERT(PRHost[addressee] != NULL);

   if (send_type == PACKETTYPE_RELIABLE) {
		packet.Set_Id(PRHost[addressee]->Get_Reliable_Packet_Send_Id());
		PRHost[addressee]->Increment_Reliable_Packet_Send_Id();

   } else {
      WWASSERT(send_type == PACKETTYPE_UNRELIABLE);
		packet.Set_Id(PRHost[addressee]->Get_Unreliable_Packet_Send_Id());
		PRHost[addressee]->Increment_Unreliable_Packet_Send_Id();
   }
}

//------------------------------------------------------------------------------------
void cConnection::R_And_U_Send(
	cPacket & packet,
	int addressee)
{
	WWASSERT(PRHost[addressee] != NULL);

   if (packet.Get_Type() == PACKETTYPE_RELIABLE) {
      PRHost[addressee]->Add_Packet(packet, RELIABLE_SEND_LIST);
   } else {
		WWASSERT(packet.Get_Type() == PACKETTYPE_UNRELIABLE);
      PRHost[addressee]->Add_Packet(packet, UNRELIABLE_SEND_LIST);
   }
}

//------------------------------------------------------------------------------------
void cConnection::Send_Packet_To_Individual(cPacket & packet, int addressee, BYTE send_flags)
{
   WWASSERT(InitDone);

   //
   // Validate inputs
   //
   WWASSERT(packet.Get_Compressed_Size_Bytes() > 0);
   WWASSERT(addressee >= MinRHost && addressee <= MaxRHost);
   WWASSERT(PRHost[addressee] != NULL);
   WWASSERT(
      send_flags == SEND_RELIABLE ||
      send_flags == SEND_UNRELIABLE ||
      send_flags == (SEND_UNRELIABLE | SEND_MULTI));

   int num_sends = 1;
   if (send_flags & SEND_MULTI) {
      num_sends = cNetUtil::MULTI_SENDS;
   }

	packet.Set_Num_Sends(num_sends);

   if (send_flags & SEND_RELIABLE) {
		packet.Set_Type(PACKETTYPE_RELIABLE);
   } else {
		packet.Set_Type(PACKETTYPE_UNRELIABLE);
   }

   Set_R_And_U_Packet_Id(packet, addressee, packet.Get_Type());

	packet.Set_Sender_Id(LocalId);

	//
	// Keep track of how many of each packet is sent.
	//
   PRHost[addressee]->Get_Stats().StatSample[STAT_MsgSent] += num_sends;
	R_And_U_Send(packet, addressee);
}

/*
//------------------------------------------------------------------------------------
void cConnection::Send_Packet_To_All(cPacket & packet, BYTE send_flags)
{
   WWASSERT(InitDone);

   //
   // Validate inputs
   //
   WWASSERT(packet.Get_Compressed_Size_Bytes() > 0);
   WWASSERT(
      send_flags == SEND_RELIABLE ||
      send_flags == SEND_UNRELIABLE ||
      send_flags == (SEND_UNRELIABLE | SEND_MULTI));

   if (NumRHosts == 0) {
      return;
   }

	//int num_sends = 0;

   for (int rhost_id = MinRHost; rhost_id <= MaxRHost; rhost_id++) {
      if (PRHost[rhost_id] != NULL) {
			//if (IsServer && !PRHost[rhost_id]->Is_Ready_For_All_Data()) {
         //   continue;
         //}

         Send_Packet_To_Individual(packet, rhost_id, send_flags);
			//num_sends++;
      }
	}

	//return num_sends;
}
*/

//------------------------------------------------------------------------------------
bool cConnection::Is_Established() const
{
	bool is_established = true;
	if (!IsServer) {
		is_established = (LocalId != ID_UNKNOWN) && (PRHost[SERVER_RHOST_ID] != NULL);
	}

	return is_established;
}

//------------------------------------------------------------------------------------
void cConnection::Connect_Cs(cPacket & packet)
{
   WWASSERT(InitDone);
   WWASSERT(!IsServer);
   WWASSERT(PRHost[SERVER_RHOST_ID] != NULL);
   WWASSERT(LocalId == ID_UNKNOWN);

   //WWDEBUG_SAY(("Connect_Cs at time %s\n", cMiscUtil::Get_Text_Time()));

   int packet_id = PRHost[SERVER_RHOST_ID]->Get_Reliable_Packet_Send_Id();
	PRHost[SERVER_RHOST_ID]->Increment_Reliable_Packet_Send_Id();
   WWASSERT(packet_id == 0);

   //WWDEBUG_SAY(("cConnection::Connect_Cs : Sending PACKETTYPE_CONNECT_CS\n"));
	WWDEBUG_SAY(("CONNECT: PACKETTYPE_CONNECT_CS sent\n"));

	packet.Set_Type(PACKETTYPE_CONNECT_CS);
	packet.Set_Id(packet_id);

	//packet.Set_Sender_Id(LocalId);//NEW

   PRHost[SERVER_RHOST_ID]->Add_Packet(packet, RELIABLE_SEND_LIST);
}

//-----------------------------------------------------------------------------
void cConnection::Send_Accept_Sc(int new_rhost_id)
{
	WWASSERT(new_rhost_id >= 0);
   WWASSERT(InitDone);
	WWASSERT(IsServer);
   WWASSERT(new_rhost_id >= MinRHost && new_rhost_id <= MaxRHost);

   //WWDEBUG_SAY(("cConnection::Send_Accept_Sc(%d)\n", new_rhost_id));
	WWDEBUG_SAY(("CONNECT: PACKETTYPE_ACCEPT_SC (%d) sent\n", new_rhost_id));

   int packet_id = PRHost[new_rhost_id]->Get_Reliable_Packet_Send_Id();
	PRHost[new_rhost_id]->Increment_Reliable_Packet_Send_Id();

   cPacket packet;
	packet.Add(new_rhost_id);

	packet.Set_Type(PACKETTYPE_ACCEPT_SC);
	packet.Set_Id(packet_id);
	packet.Set_Sender_Id(LocalId);

   PRHost[new_rhost_id]->Add_Packet(packet, RELIABLE_SEND_LIST);
}

//-----------------------------------------------------------------------------
void cConnection::Send_Refusal_Sc(struct sockaddr_in* p_address, REFUSAL_CODE refusal_code)
{
   WWASSERT(p_address != NULL);

   //
   // This is a refusal originating from the wwnet layer
   //

   WWDEBUG_SAY(("cConnection::Send_Refusal_Sc (%d)\n", refusal_code));

	WWASSERT(InitDone);
	WWASSERT(IsServer);

   //
   // The id is not per-client... because we don't hold per client information
   // for this guy.
   //
   int packet_id = RefusalPacketSendId++;

   cPacket packet;
	packet.Add((int)refusal_code);

	packet.Set_Type(PACKETTYPE_REFUSAL_SC);
	packet.Set_Id(packet_id);
	packet.Set_Sender_Id(LocalId);

   //
   // Send redundantly, because it's unreliable
   //
   packet.Set_Num_Sends(cNetUtil::MULTI_SENDS);
	Send_Packet_To_Address(packet, p_address);
}

//-----------------------------------------------------------------------------
void cConnection::Send_Ack(struct sockaddr_in* p_address, int packet_id)
{
   WWASSERT(p_address != NULL);
   WWASSERT(InitDone);
   WWASSERT(packet_id >= 0);
   WWASSERT(LocalId != ID_UNKNOWN); //TSS - bug - asserted here when 50% packet loss or crc failures

	//WWDEBUG_SAY(("Ack reply for packet is %d\n", packet_id));
	//WWDEBUG_SAY(("Sending ack for packet %d to %s\n", packet_id, Addr_As_String(p_address)));

	cPacket packet;

	packet.Set_Type(PACKETTYPE_ACK);
	packet.Set_Id(packet_id);
	packet.Set_Sender_Id(LocalId);

   //
   // Acks are unreliable
   //

   int addressee = Address_To_Rhostid(p_address);
   if (addressee != INVALID_RHOST_ID) {
      WWASSERT(PRHost[addressee] != NULL);
      PRHost[addressee]->Get_Stats().StatSample[STAT_AckCountSent]++;
      PRHost[addressee]->Get_Stats().StatSample[STAT_UPktSent]++;
      PRHost[addressee]->Get_Stats().StatSample[STAT_UByteSent] += packet.Get_Compressed_Size_Bytes();
   }

	//unsigned long time = TIMEGETTIME() / 1000;
	//WWDEBUG_SAY(("Sending ack at %d\n", time));

   Send_Packet_To_Address(packet, p_address);
}

//-----------------------------------------------------------------------------
void cConnection::Destroy_Connection(int rhost_id)
{
	WWASSERT(rhost_id >= 0);

   WWASSERT(InitDone);
   WWASSERT(rhost_id >= MinRHost && rhost_id <= MaxRHost);
   if (PRHost[rhost_id] != NULL) {
		delete PRHost[rhost_id];
		PRHost[rhost_id] = NULL;
		NumRHosts--;
      WWASSERT(NumRHosts >= 0);
	}
}

//-----------------------------------------------------------------------------
void cConnection::Send_Keepalives()
{
   //
   /////// Keepalives are only sent when we have heard nothing from the rhost for a while.
   //

	WWASSERT(InitDone);

   if (LocalId != ID_UNKNOWN) {
      for (int rhost_id = MinRHost; rhost_id <= MaxRHost; rhost_id++) {
		   if (PRHost[rhost_id] != NULL && ThisFrameTimeMs -
				PRHost[rhost_id]->Get_Last_Keepalive_Time_Ms() > cNetUtil::KEEPALIVE_TIMEOUT_MS) {

				int service_rate = (int)(1000 *
					(ServiceCount - PRHost[rhost_id]->Get_Last_Service_Count()) / (ThisFrameTimeMs -
					PRHost[rhost_id]->Get_Last_Keepalive_Time_Ms()));
				PRHost[rhost_id]->Set_Last_Service_Count(ServiceCount);

            PRHost[rhost_id]->Set_Last_Keepalive_Time_Ms(ThisFrameTimeMs);


            //WWDEBUG_SAY(("Sending keepalive to rhost %d at time %d\n", rhost_id, TIMEGETTIME()));

            //
            // Keepalive is a reliable message
            //
				int packet_id = PRHost[rhost_id]->Get_Reliable_Packet_Send_Id();
				PRHost[rhost_id]->Increment_Reliable_Packet_Send_Id();

				cPacket packet;
				packet.Add((float) PRHost[rhost_id]->Get_Stats().Get_Pc_Packetloss_Received());
				packet.Add(service_rate);

				packet.Set_Type(PACKETTYPE_KEEPALIVE);
				packet.Set_Id(packet_id);
				packet.Set_Sender_Id(LocalId);

				PRHost[rhost_id]->Add_Packet(packet, RELIABLE_SEND_LIST);
         }
      }
   }
}

//-----------------------------------------------------------------------------
double cConnection::Get_Threshold_Priority(int rhost_id)
{
	WWASSERT(rhost_id >= 0);
   WWASSERT(PRHost[rhost_id] != NULL);
   return PRHost[rhost_id]->Get_Threshold_Priority();
}

//-----------------------------------------------------------------------------
void cConnection::Set_Max_Acceptable_Packetloss_Pc(double max_packetloss_pc)
{
   WWASSERT(max_packetloss_pc >= 0 && max_packetloss_pc <= 100);

	//WWDEBUG_SAY(("cConnection::Set_Max_Acceptable_Packetloss_Pc: %5.2f\n",
	//	MaxAcceptablePacketlossPc));

   MaxAcceptablePacketlossPc = max_packetloss_pc;
}

//-----------------------------------------------------------------------------
bool cConnection::Demultiplex_R_Or_U_Packet(cPacket * p_packet, int rhost_id)
{
   WWASSERT(p_packet != NULL);
	WWASSERT(rhost_id >= 0);

   bool is_aborted;

   WWASSERT(PRHost[rhost_id] != NULL);

   if (IsServer) {
		//WWPROFILE("cConnection::Server_Packet_Handler");
		//Server_Packet_Handler(*p_packet, rhost_id);
		WWASSERT(ServerPacketHandler != NULL);
		ServerPacketHandler(*p_packet, rhost_id);
	} else {
		//WWPROFILE("cConnection::Client_Packet_Handler");
		//Client_Packet_Handler(*p_packet);
		WWASSERT(ClientPacketHandler != NULL);
		ClientPacketHandler(*p_packet);
	}

   if (PRHost[rhost_id] == NULL) {
      is_aborted = true;
   } else {
      //WWASSERT(p_packet->Is_Flushed());
      is_aborted = false;
   }

   return is_aborted;
}

//-----------------------------------------------------------------------------
//
// Service_Read() should be called once per frame on both C & S
//
void cConnection::Service_Read()
{
	//WWDEBUG_SAY(("cConnection::Service_Read\n"));

   WWASSERT(InitDone);

   CombinedStats.StatSample[STAT_ServiceCount]++;

	ThisFrameTimeMs = TIMEGETTIME();

	if (ExtraTimeoutTime && (ThisFrameTimeMs - ExtraTimeoutTimeStarted) > (ExtraTimeoutTime * 2)) {
		ExtraTimeoutTime = 0;
		ExtraTimeoutTimeStarted = 0;
	}

   //
   // Receive as many packets as are available
   //
   //int start_time;

	//start_time = TIMEGETTIME();

   /*
	if (Is_Packet_Latency_Simulation_Active()) {
		Buffer_Packets();
	}
	*/

	{
	WWPROFILE("Receive Packets");
   while (Receive_Packet());
	}

   /*
   int time_spent = (int) TIMEGETTIME() - start_time;
	if (time_spent > cNetUtil::Get_Max_Receive_Time_Ms()) {
      WWDEBUG_SAY(("*** WWNET: Too much time spent (%d ms)) receiving packets.\n",
         time_spent));
   }
	*/

	if (!CanProcess) {
		return;
	}

	int rhost_id;

	//
   // Process as many reliable packets as are available in sequence
	//
	//start_time = (int) TIMEGETTIME();

	{
	WWPROFILE("Process R Packets");

	for (rhost_id = MinRHost; rhost_id <= MaxRHost; rhost_id++) {
		if (PRHost[rhost_id] != NULL) {
         PRHost[rhost_id]->Compute_List_Max(RELIABLE_RCV_LIST);

	      for (SLNode<cPacket> * objnode = PRHost[rhost_id]->Get_Packet_List(RELIABLE_RCV_LIST).Head();
            objnode != NULL;) {

            cPacket * p_packet = objnode->Data();
            WWASSERT(p_packet != NULL);
            objnode = objnode->Next();

				int comparison = p_packet->Get_Id() - PRHost[rhost_id]->Get_Reliable_Packet_Rcv_Id();

				if (comparison < 0) {
					//
               // Duplicate packet, discard
               //
               CombinedStats.StatSample[STAT_DiscardCount]++;

					/*
					WWDEBUG_SAY(("%s removing duplicate reliable packet (id %d)) [ReliablePacketRcvId is %d]\n",
                  IsServer ? "Server" : "Client",
						p_packet->Get_Id(),
						PRHost[rhost_id]->Get_Reliable_Packet_Rcv_Id()));
					*/

					PRHost[rhost_id]->Get_Packet_List(RELIABLE_RCV_LIST).Remove_Head();
					p_packet->Flush();
					delete p_packet;

            } else if (comparison == 0) {

					/*
               bool need_abort = false;

					switch (p_packet->Get_Type()) {

                  case PACKETTYPE_KEEPALIVE:
                  case PACKETTYPE_ACCEPT_SC:
							//
							// Don't pass on reliable system packets... they are in
                     // the list solely to maintain packet sequencing.
							//
                     p_packet->Flush();// is this necessary?
                     break;

                  case PACKETTYPE_RELIABLE:
                     need_abort = Demultiplex_R_Or_U_Packet(p_packet, rhost_id);
                     break;

                  default:
                     DIE;
               }

					if (need_abort) {
						break;
					}
					*/

					WWASSERT(p_packet->Get_Type() >= PACKETTYPE_FIRST && p_packet->Get_Type() <= PACKETTYPE_LAST);

					if (p_packet->Get_Type() == PACKETTYPE_RELIABLE) {
                  bool abort = Demultiplex_R_Or_U_Packet(p_packet, rhost_id);
						if (abort) {
							break;
						}
					}

					//
					// This may help detect if the packet got deallocated or something bad...
					//
					WWASSERT(p_packet->Get_Type() >= PACKETTYPE_FIRST && p_packet->Get_Type() <= PACKETTYPE_LAST);

					PRHost[rhost_id]->Get_Packet_List(RELIABLE_RCV_LIST).Remove_Head();
					p_packet->Flush();
					delete p_packet;

					PRHost[rhost_id]->Increment_Reliable_Packet_Rcv_Id();

            } else {
					WWASSERT(comparison > 0);
					//
					// We are done... we cannot process the next packet in the list yet.
					//
					break;
				}
         }
		}
	}
	}

   /*
   time_spent = (int) TIMEGETTIME() - start_time;
	if (time_spent > cNetUtil::Get_Max_Receive_Time_Ms()) {
      WWDEBUG_SAY(("*** WWNET: Too much time (%d ms)) spent receiving reliable queued packets.\n",
         time_spent));
   }
	*/

	//
   // Process all unreliable packets.
	//

	//start_time = (int) TIMEGETTIME();

	{
	WWPROFILE("Process U Packets");

	for (rhost_id = MinRHost; rhost_id <= MaxRHost; rhost_id++) {
		if (PRHost[rhost_id] != NULL) {

         PRHost[rhost_id]->Compute_List_Max(UNRELIABLE_RCV_LIST);

			//broken PRHost[rhost_id]->Set_List_Packet_Size(UNRELIABLE_RCV_LIST, 0);

			unsigned long list_processing_start = TIMEGETTIME();

	      for (SLNode<cPacket> * objnode = PRHost[rhost_id]->Get_Packet_List(UNRELIABLE_RCV_LIST).Head();
            objnode != NULL; objnode = objnode->Next()) {

            cPacket * p_packet = objnode->Data();
            WWASSERT(p_packet != NULL);
            WWASSERT(p_packet->Get_Id() >= 0);

				if (p_packet->Get_Id() < PRHost[rhost_id]->Get_Unreliable_Packet_Rcv_Id()) {
					//
               // Duplicate packet, discard
               //
               CombinedStats.StatSample[STAT_DiscardCount]++;

					/*
               WWDEBUG_SAY(("Ignoring duplicate unreliable packet (id %d)) [UnreliablePacketRcvId is %d]\n",
						p_packet->Id, PRHost[rhost_id]->UnreliablePacketRcvId));
               */

            } else {

               //WWASSERT(p_packet->Get_Type() == (BYTE) PACKETTYPE_UNRELIABLE);

					{
					//WWPROFILE("Demultiplex_R_Or_U_Packet");
               bool abort = Demultiplex_R_Or_U_Packet(p_packet, rhost_id);
               if (abort) {
                  break;
               }
					}

               PRHost[rhost_id]->Set_Unreliable_Packet_Rcv_Id(p_packet->Get_Id());
				}
			}

         if (PRHost[rhost_id] != NULL) {
				PRHost[rhost_id]->Set_List_Processing_Time(UNRELIABLE_RCV_LIST, TIMEGETTIME() - list_processing_start);
			}

         if (PRHost[rhost_id] != NULL) {
				//
				// Destroy list
				//
				for (SLNode<cPacket> * objnode = PRHost[rhost_id]->Get_Packet_List(UNRELIABLE_RCV_LIST).Head();
					objnode != NULL; objnode = objnode->Next()) {
					cPacket * p_packet = objnode->Data();
					p_packet->Flush();
					WWASSERT(p_packet != NULL);
					delete p_packet;
				}

				PRHost[rhost_id]->Get_Packet_List(UNRELIABLE_RCV_LIST).Remove_All();
			}
		}
	}

	}

   /*
   time_spent = (int) TIMEGETTIME() - start_time;
	if (time_spent > cNetUtil::Get_Max_Receive_Time_Ms()) {
		WWDEBUG_SAY(("*** WWNET: Too much time (%d ms)) spent receiving unreliable queued packets.\n",
         time_spent));
   }
	*/


	if (IsServer) {
		//
		// Notify Server of recommended evictions
		//
		for (rhost_id = MinRHost; rhost_id <= MaxRHost; rhost_id++) {

			if (PRHost[rhost_id] != NULL && PRHost[rhost_id]->Must_Evict()) {
				PRHost[rhost_id]->Set_Must_Evict(false);
				//Eviction_Handler(rhost_id);
				WWASSERT(EvictionHandler != NULL);
				EvictionHandler(rhost_id);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void cConnection::Set_Bandwidth_Budget_Out(ULONG bw_budget)
{
	//WWASSERT(bw_budget >= 0);
	BandwidthBudgetOut = bw_budget;

	//
	// If we only have very limited bandwidth available then it's silly to have huge send buffers since that can mask
	// problems with outgoing packets until it's too late to recover. ST - 10/17/2001 12:51PM
	//
   if (!cSinglePlayerData::Is_Single_Player()) {
		if (Sock != INVALID_SOCKET) {
			// make the buffers big enough for 3 seconds of data.
			int new_buffer_size = (bw_budget / 8) * 3;
			new_buffer_size = std::min(new_buffer_size, 250000);
			cNetUtil::Set_Socket_Buffer_Sizes(Sock, new_buffer_size);
		}
	}
}

//-----------------------------------------------------------------------------
void cConnection::Clear_Resend_Counts()
{
	//WWDEBUG_SAY(("cConnection::Clear_Resend_Counts()\n"));

   SLNode<cPacket> * objnode;
	cPacket * p_packet;
	for (int rhost_id = MinRHost; rhost_id <= MaxRHost; rhost_id++) {
		if (PRHost[rhost_id] != NULL) {

			for (objnode = PRHost[rhost_id]->Get_Packet_List(RELIABLE_SEND_LIST).Head();
            objnode != NULL; objnode = objnode->Next()) {

            p_packet = objnode->Data();
            WWASSERT(p_packet != NULL);
            if (p_packet->Get_Resend_Count() > 0) {
					p_packet->Clear_Resend_Count();
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
cRemoteHost * cConnection::Get_Remote_Host(int rhost)
{
	WWASSERT(rhost >= MinRHost && rhost <= MaxRHost);
	/*
	if (rhost < MinRHost || rhost > MaxRHost) {
		WWDEBUG_SAY((">>> %d: %d,%d   IS:%d\n", rhost, MinRHost, MaxRHost,
			IsServer));
		DIE;
	}
	*/
	return PRHost[rhost];
}

//-----------------------------------------------------------------------------
//
// Service_Send() should be called once per frame on both C & S
//
void cConnection::Service_Send(bool is_urgent)
{
   WWASSERT(InitDone);

	ServiceCount++;

	//
	// Set TargetBps for all rhosts
	//
	int num_real_remote_hosts = NumRHosts;
	if (IsServer && !IsDedicatedServer) {
		num_real_remote_hosts--;
		if (PRHost[1] != NULL) {
			PRHost[1]->Set_Target_Bps(10000000);//TSS - won't this just be overwritten below???
		}
	}

	if (num_real_remote_hosts > 0) { // necessary?

		if (IsServer && BandwidthBalancer.IsEnabled) {
			BandwidthBalancer.Adjust(this, IsDedicatedServer);
		} else {

			ULONG bps_per_rhost = (ULONG) (BandwidthBudgetOut / (float) num_real_remote_hosts);

			for (int rhost_id = MinRHost; rhost_id <= MaxRHost; rhost_id++) {
				if (PRHost[rhost_id] != NULL) {

					//
					// Do not exceed the max bps set by the client.
					//
					int bps = bps_per_rhost;
					int max_bps = PRHost[rhost_id]->Get_Maximum_Bps();
					if (max_bps != 0 && max_bps < bps) {
						bps = max_bps;
					}

					PRHost[rhost_id]->Set_Target_Bps(bps);

					//WWDEBUG_SAY(("Compressed bandwidth out to client = %d bps\n", PacketManager.Get_Compressed_Bandwidth_Out(&(PRHost[rhost_id]->Get_Address()))));
				}
			}
		}
	}
	int rhost_id;

   //
   // Reliable sends and resends
   //
	bool any_bad = false;
	for (rhost_id = MinRHost; rhost_id <= MaxRHost; rhost_id++) {

		cRemoteHost * p_rhost = PRHost[rhost_id];

		if (p_rhost != NULL) {

         p_rhost->Compute_List_Max(RELIABLE_SEND_LIST);

         /*
			//
         // Send any reliable combined-packet
         //
         cPacket * & p_packet_rs = p_rhost->Get_P_Comb_Rel_Packet();
         if (p_packet_rs != NULL) {
            Internal_Send_Packet_To_Individual(*p_packet_rs, rhost_id, PACKETTYPE_RELIABLE);
            p_packet_rs->Flush();
            delete p_packet_rs;
            p_packet_rs = NULL;
         }
			*/

			int resent_packets = 0;

         //
         // Send any appropriate reliable queued packets
         //
	      for (SLNode<cPacket> * objnode = p_rhost->Get_Packet_List(RELIABLE_SEND_LIST).Head();
            objnode != NULL;) {

            cPacket * p_packet = objnode->Data();
            objnode = objnode->Next();
            WWASSERT(p_packet != NULL);

				if (p_packet->Get_Resend_Count() > 1) {
					resent_packets++;
					if (ThisFrameTimeMs - p_packet->Get_First_Send_Time() > 5000) {
						any_bad = true;
					}
				}

				if (Is_Time_To_Resend_Packet_To_Remote_Host(p_packet, p_rhost)) {

               //
               // This will hold true for new packets because Sendtime is set high negative
               //

					//
					// Resends are reliable
					//
               p_rhost->Get_Stats().StatSample[STAT_RPktSent]++;
					p_rhost->Get_Stats().StatSample[STAT_RByteSent] += p_packet->Get_Compressed_Size_Bytes();

					//
					// Send!
					//
					//if (p_packet->Get_Resend_Count() > 0) {
					//if (p_packet->Get_Resend_Count() > 0 && p_packet->Get_Resend_Count() % 10 == 0) {
						//WWDEBUG_SAY(("Resending packet %d to %s after %dms. Resent %d times\n", p_packet->Get_Id(), Addr_As_String(&(p_rhost->Get_Address())), ThisFrameTimeMs - p_packet->Get_First_Send_Time(), p_packet->Get_Resend_Count()));
					//}
					Send_Packet_To_Address(*p_packet, &(p_rhost->Get_Address()));

					p_packet->Set_Send_Time();

					/*
					if (p_rhost->Is_Ready_For_All_Data()) {
						p_packet->Increment_Resend_Count();
					}
					*/
					p_packet->Increment_Resend_Count();

					if (p_packet->Get_Resend_Count() > 0) {
                  p_rhost->Get_Stats().StatSample[STAT_ResendCount]++;
						p_rhost->Increment_Resends();
               }

					// In a LAN game, if resend time is low. Say 4 ms, and we only resend 50 times, thats 200ms total before the game
					// craps out. Any kind of network or machine hiccup could cause the game to quit. This is an experimant to see if that's
					// what is causing our disconnect at game start.
					//
					// As it happens, the main thread can get lost loading textures for several seconds at the end of a game load
					// so some sort of overall timeout is neccessary.
					//
					// TSS 09/24/01 - new loading lags worse than 10 seconds... bumping it up
					//                to 20s.
					//
					// ST 1/24/2002 2:16PM. Can't time out players when we are loading or we get DataSafe access from the wrong
					// thread.
					//
               if (Is_Packet_Too_Old(p_packet, p_rhost) && CanProcess) {

						WWDEBUG_SAY(("*** WWNET: Connection timed out - assuming connection to rhost %d is broken.\n", rhost_id));
						WWDEBUG_SAY(("*** WWNET: ThisFrameTimeMs - p_packet->Get_First_Send_Time() == %d\n", ThisFrameTimeMs - p_packet->Get_First_Send_Time()));
                  //
                  // Define the connection as broken
                  //
                  WWDEBUG_SAY(("*** Breaking connection on packet %d ... internal ping = %d/%d/%d ms, ResendTimeoutMs = %d ms\n",
                     p_packet->Get_Id(),
		               p_rhost->Get_Min_Internal_Pingtime_Ms(),
		               p_rhost->Get_Average_Internal_Pingtime_Ms(),
		               p_rhost->Get_Max_Internal_Pingtime_Ms(),
		               p_rhost->Get_Resend_Timeout_Ms()));


                  Destroy_Connection(rhost_id);
                  //WWDEBUG_SAY(("*** WWNET: Exceeded maximum resends (%d)) - assuming connection to rhost %d is broken.\n",
						//	cNetUtil::MAX_RESENDS, rhost_id));

                  if (IsServer) {
                     //Server_Broken_Connection_Handler(rhost_id); // Inform app level of this disaster
							WWASSERT(ServerBrokenConnectionHandler != NULL);
							ServerBrokenConnectionHandler(rhost_id);
                  } else {
                     //Client_Broken_Connection_Handler(); // Inform app level of this disaster
							WWASSERT(ClientBrokenConnectionHandler != NULL);
							ClientBrokenConnectionHandler();
                  }
                  break;
               }
				}
			}

			// Keep track of how many packets in the queue are waiting on late acks.
			p_rhost->Set_Total_Resent_Packets_In_Queue(resent_packets);
		}
	}

	IsBadConnection = any_bad;


	//
   // Unreliable sends and resends
   //
	for (rhost_id = MinRHost; rhost_id <= MaxRHost; rhost_id++) {

		cRemoteHost * p_rhost = PRHost[rhost_id];

		if (p_rhost != NULL) {

         p_rhost->Compute_List_Max(UNRELIABLE_SEND_LIST);

         /*
			//
         // Send any unreliable combined-packet
         //
         cPacket * & p_packet_u = p_rhost->Get_P_Comb_Unrel_Packet();
         if (p_packet_u != NULL) {
            Internal_Send_Packet_To_Individual(*p_packet_u, rhost_id, PACKETTYPE_UNRELIABLE);
            p_packet_u->Flush();
            delete p_packet_u;
            p_packet_u = NULL;
         }
			*/

         //
         // Send any appropriate queued packets
         //
		 SLNode<cPacket> * objnode;
	      for (objnode = p_rhost->Get_Packet_List(UNRELIABLE_SEND_LIST).Head();
            objnode != NULL; objnode = objnode->Next()) {

            cPacket * p_packet = objnode->Data();
            WWASSERT(p_packet != NULL);

            p_rhost->Get_Stats().StatSample[STAT_UPktSent]++;
				p_rhost->Get_Stats().StatSample[STAT_UByteSent] += p_packet->Get_Compressed_Size_Bytes();

            //
				// Send!
				//
				Send_Packet_To_Address(*p_packet, &(p_rhost->Get_Address()));
         }

	      // destroy all
			for (objnode = p_rhost->Get_Packet_List(UNRELIABLE_SEND_LIST).Head();
            objnode != NULL; objnode = objnode->Next()) {

            cPacket * p_packet = objnode->Data();
            WWASSERT(p_packet != NULL);
				p_packet->Flush();
				delete p_packet;
         }
			p_rhost->Get_Packet_List(UNRELIABLE_SEND_LIST).Remove_All();
      }
   }

   //
   // Send keepalives if necessary
   //
   Send_Keepalives();


	//
	// Monkey with the stats for a while
	//
	float sample_time_ms = ThisFrameTimeMs - CombinedStats.Get_Sample_Start_Time();

   if (IsServer && NumRHosts > 0 &&
      sample_time_ms > cNetUtil::NETSTATS_SAMPLE_TIME_MS) {

      for (int statistic = 0; statistic < STAT_COUNT; statistic++) {
	      for (rhost_id = MinRHost; rhost_id <= MaxRHost; rhost_id++) {
		      if (PRHost[rhost_id] != NULL) {
               CombinedStats.StatSample[statistic] += PRHost[rhost_id]->Get_Stats().StatSnapshot[statistic];
            }
         }

         AveragedStats.StatSample[statistic] = cMathUtil::Round(
            CombinedStats.StatSample[statistic] / (double) NumRHosts);
      }

      AveragedStats.Update_If_Sample_Done(ThisFrameTimeMs);
   }

   CombinedStats.Update_If_Sample_Done(ThisFrameTimeMs);

	for (rhost_id = MinRHost; rhost_id <= MaxRHost; rhost_id++) {
		if (PRHost[rhost_id] != NULL) {
			bool is_updated = PRHost[rhost_id]->Get_Stats().Update_If_Sample_Done(
            ThisFrameTimeMs, false);

         if (is_updated) {
				PRHost[rhost_id]->Adjust_Resend_Timeout();
				PRHost[rhost_id]->Adjust_Flow_If_Necessary(sample_time_ms);
			}
      }
   }

	//
	// Service the packet manager
	//
	PacketManager.Flush(is_urgent);
}

//-----------------------------------------------------------------------------
void cConnection::Install_Accept_Handler(Accept_Handler handler)
{
	WWASSERT(handler != NULL);

	AcceptHandler = handler;
}

//-----------------------------------------------------------------------------
void cConnection::Install_Refusal_Handler(Refusal_Handler handler)
{
	WWASSERT(handler != NULL);

	RefusalHandler = handler;
}

//-----------------------------------------------------------------------------
void cConnection::Install_Server_Broken_Connection_Handler(Server_Broken_Connection_Handler handler)
{
	WWASSERT(handler != NULL);

	ServerBrokenConnectionHandler = handler;
}

//-----------------------------------------------------------------------------
void cConnection::Install_Client_Broken_Connection_Handler(Client_Broken_Connection_Handler handler)
{
	WWASSERT(handler != NULL);

	ClientBrokenConnectionHandler = handler;
}

//-----------------------------------------------------------------------------
void cConnection::Install_Eviction_Handler(Eviction_Handler handler)
{
	WWASSERT(handler != NULL);

	EvictionHandler = handler;
}

//-----------------------------------------------------------------------------
void cConnection::Install_Conn_Handler(Conn_Handler handler)
{
	WWASSERT(handler != NULL);

	ConnHandler = handler;
}

//-----------------------------------------------------------------------------
void cConnection::Install_Application_Acceptance_Handler(Application_Acceptance_Handler handler)
{
	WWASSERT(handler != NULL);

	ApplicationAcceptanceHandler = handler;
}

//-----------------------------------------------------------------------------
void cConnection::Install_Server_Packet_Handler(Server_Packet_Handler handler)
{
	WWASSERT(handler != NULL);

	ServerPacketHandler = handler;
}

//-----------------------------------------------------------------------------
void cConnection::Install_Client_Packet_Handler(Client_Packet_Handler handler)
{
	WWASSERT(handler != NULL);

	ClientPacketHandler = handler;
}

//-----------------------------------------------------------------------------
void cConnection::Set_Rhost_Is_In_Game(int id, bool state)
{
	if (IsServer) {
		for (int i = MinRHost; i <= MaxRHost; i++) {
			if (PRHost[i] && PRHost[i]->Get_Id() == id) {
				PRHost[i]->Set_Is_Loading(!state);
				break;
			}
		}
	}
}


void cConnection::Set_Rhost_Expect_Packet_Flood(int id, bool state)
{
	if (IsServer) {
		for (int i = MinRHost; i <= MaxRHost; i++) {
			if (PRHost[i] && PRHost[i]->Get_Id() == id) {
				PRHost[i]->Set_Flood(state);
				break;
			}
		}
	}
}



//-----------------------------------------------------------------------------
bool cConnection::Is_Time_To_Resend_Packet_To_Remote_Host(const cPacket *packet, cRemoteHost *rhost)
{
	WWASSERT(ThisFrameTimeMs >= 0);
	WWASSERT(packet);
	WWASSERT(rhost);

	if (!packet || !rhost) {
		return(false);
	}

	unsigned long last_send_time = packet->Get_Send_Time();
	if (last_send_time == cPacket::Get_Default_Send_Time()) {
		return(true);
	}

	//
	// Basically this slows down the resend rate each time we resend
	//
	float resend_timeout = (float) rhost->Get_Resend_Timeout_Ms();
	WWASSERT(resend_timeout != 0);
	float resend_count = (float) packet->Get_Resend_Count();
	float timeout_multiplier = 0.5f;
	float total_timeout = resend_timeout;

	if (IsServer) {
		bool loading = rhost->Get_Is_Loading();

		//
		// Slow down resend rate if remote host is loading. He might not be able to respond anyway.
		//
		if (loading) {
			timeout_multiplier = timeout_multiplier * 2.0f;
		}
	}
	total_timeout = resend_timeout + (resend_timeout * resend_count * timeout_multiplier);

	//
	// Max resend timeout of 3 secs.
	//
	total_timeout = std::min(total_timeout, 3000.0f);

	if (ThisFrameTimeMs - packet->Get_Send_Time() >= (unsigned long)total_timeout) {
		//WWDEBUG_SAY(("Time to resend packet %d, age = %d, timeout = %d, resend count = %d\n", packet->Get_Id(), (int)(ThisFrameTimeMs - packet->Get_Send_Time()), (int)total_timeout, packet->Get_Resend_Count()));
		return(true);
	}
	return(false);
}



//-----------------------------------------------------------------------------
bool cConnection::Is_Packet_Too_Old(const cPacket *packet, cRemoteHost *rhost)
{
	WWASSERT(ThisFrameTimeMs >= 0);
	WWASSERT(packet);
	WWASSERT(rhost);

	if (!packet || !rhost) {
		return(false);
	}

	if (packet->Get_Send_Time() == cPacket::Get_Default_Send_Time()) {
		return(false);
	}

	unsigned long timeout = 0;

	if (IsServer) {

		timeout = cNetUtil::SERVER_CONNECTION_LOSS_TIMEOUT;

		//
		// Server needs to allow more time for players entering the game.
		//
		if (rhost->Get_Is_Loading() || rhost->Was_Recently_Loading(ThisFrameTimeMs)) {
			timeout += cNetUtil::SERVER_CONNECTION_LOSS_TIMEOUT_LOADING_ALLOWANCE;
		} else {

			int num_remote_hosts = Get_Num_RHosts();
			if (!IsDedicatedServer) {
				num_remote_hosts--;
			}
			if (num_remote_hosts < 1) {
				timeout += cNetUtil::SERVER_CONNECTION_LOSS_TIMEOUT_LOADING_ALLOWANCE;
			}
		}
	} else {

		//
		// Client just uses a constant value.
		//
		timeout = cNetUtil::CLIENT_CONNECTION_LOSS_TIMEOUT;

		//
		// Modified when server is potentially loading.
		//
		timeout += ExtraTimeoutTime;
	}

	if (ThisFrameTimeMs > packet->Get_First_Send_Time()) {
		if (ThisFrameTimeMs - packet->Get_First_Send_Time() > timeout) {
			return(true);
		}
	}

	return(false);
}



//-----------------------------------------------------------------------------
void cConnection::Allow_Extra_Timeout_For_Loading(void)
{
	ExtraTimeoutTime = cNetUtil::SERVER_CONNECTION_LOSS_TIMEOUT_LOADING_ALLOWANCE;
	ExtraTimeoutTimeStarted = TIMEGETTIME();
}



#ifdef WWDEBUG
void cConnection::Set_Latency(int low, int high)
{
	LatencyAddLow = low;
	LatencyAddHigh = high;
	CurrentLatencyAdd = LatencyAddLow + ((LatencyAddHigh - LatencyAddLow) / 2);
}

void cConnection::Get_Latency(int &low, int &high, int &current)
{
	low = LatencyAddLow;
	high = LatencyAddHigh;
	current = CurrentLatencyAdd;
}
#endif //WWDEBUG




	//KEEPALIVE_TIMEOUT_MS(cNetUtil::Get_Default_Keepalive_Timeout_Ms()),
	//MAX_RESENDS(cNetUtil::Get_Default_Max_Resends()),
	//MULTI_SENDS(cNetUtil::Get_Default_Multi_Sends()),		//if (!Application_Acceptance_Handler(packet)) {
