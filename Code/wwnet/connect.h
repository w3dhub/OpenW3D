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
// Filename:     connect.h
// Project:      wwnet
// Author:       Tom Spencer-Smith
// Date:         June 1998
// Description:
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef CONNECT_H
#define CONNECT_H

#include "rhost.h"
#include "netstats.h"
#include "bittype.h"
#include "netutil.h"
#include "slist.h"
#include "wwpacket.h"
#include "packettype.h"

//
// A server can have this many clients (a client has only 1 rhost: the server)
// -1 is used as ID_UNKNOWN
// This is used as an array bound and is not insulated,
// but using a namespace is ugly...
//
//const int MAX_RHOSTS = 256;

//
// These defines are useful at the app level
//
const int SERVER_RHOST_ID = 0;

//
// This is the default dummy ID used by a client until he receives a real id
// from the server
//
const int ID_UNKNOWN = -1;

typedef enum {
	REFUSAL_CLIENT_ACCEPTED = 0,
	REFUSAL_GAME_FULL,
	REFUSAL_BAD_PASSWORD,
	REFUSAL_VERSION_MISMATCH,
	REFUSAL_PLAYER_EXISTS,
	REFUSAL_BY_APPLICATION
} REFUSAL_CODE;

//
// Send flags. Normal send is individual, single.
// Strictly speaking SEND_UNRELIABLE could be deduced by absense
// of SEND_RELIABLE, but it's inclusion makes a clearer API.
//
const BYTE SEND_RELIABLE   = 0x01; // you must specify this or SEND_UNRELIABLE
const BYTE SEND_UNRELIABLE = 0x02; // you must specify this or SEND_RELIABLE
const BYTE SEND_MULTI      = 0x04; // For SEND_UNRELIABLE only. Default is single send.

class cMsgStatList;

typedef void (*Accept_Handler)(void);
typedef void (*Refusal_Handler)(REFUSAL_CODE refusal_code);
typedef void (*Server_Broken_Connection_Handler)(int broken_rhost_id);
typedef void (*Client_Broken_Connection_Handler)(void);
typedef void (*Eviction_Handler)(int evicted_rhost_id);
typedef void (*Conn_Handler)(int new_rhost_id);
typedef REFUSAL_CODE (*Application_Acceptance_Handler)(cPacket & packet);
typedef void (*Server_Packet_Handler)(cPacket & packet, int rhost_id);
typedef void (*Client_Packet_Handler)(cPacket & packet);

extern const char* Addr_As_String(const sockaddr_in* addr);
extern char* Addr_As_String(sockaddr_in* addr);

//-----------------------------------------------------------------------------
//
// This class represents the link between C & S.
// The 1-many S-C relationship is expressed in the cRemoteHost component.
//
class cConnection
{
	public:
		cConnection();
		~cConnection();

      void Init_As_Client(ULONG server_ip, USHORT server_port, unsigned short my_port = 0);
      void Init_As_Server(USHORT server_port, int max_players,
			bool is_dedicated_server, ULONG addr = 0);
      void Connect_Cs(cPacket & app_data);
      void Send_Packet_To_Individual(cPacket & packet, int addressee, BYTE send_flags);
      bool Have_Id() const {return LocalId != ID_UNKNOWN;}
      bool Is_Established() const;
		void Service_Read();
		void Service_Send(bool is_urgent = false);
		ULONG Get_Bandwidth_Budget_Out() const {return BandwidthBudgetOut;}
		void Set_Bandwidth_Budget_Out(ULONG bw_budget);
      void Destroy_Connection(int rhost_id);
		void Init_Stats();
      double Get_Threshold_Priority(int rhost_id);
      void Set_Packet_Loss(double percent_lost);
      void Set_Packet_Duplication(double percent_duplicated);
      void Set_Packet_Latency_Range(int minimum_latency_ms, int maximum_latency_ms);
      void Set_Max_Acceptable_Packetloss_Pc(double max_packetloss_pc);
      void Enable_Flow_Control(BOOL is_enabled) {IsFlowControlEnabled = is_enabled;}  // This should be called at startup.
	   SList<cPacket> * Get_Packet_List() {return &PacketList;}
		void Clear_Resend_Counts();
		int Get_Min_RHost()		{return MinRHost;}
		int Get_Max_RHost()		{return MaxRHost;}
		int Get_Num_RHosts()	{return NumRHosts;}
		cRemoteHost * Get_Remote_Host(int rhost);
		bool Is_Destroy() {return IsDestroy;}
      int Get_Local_Id() const {return LocalId;}
		double Get_Max_Acceptable_Packetloss_Pc() const {return MaxAcceptablePacketlossPc;}
		cNetStats & Get_Combined_Stats() {return CombinedStats;}
		cNetStats & Get_Averaged_Stats() {return AveragedStats;}
      static BOOL Is_Flow_Control_Enabled() {return IsFlowControlEnabled;}
		static UINT Get_Total_Compressed_Bytes_Sent(void)
			{return TotalCompressedBytesSent;}
		static UINT Get_Total_Uncompressed_Bytes_Sent(void)
			{return TotalUncompressedBytesSent;}
		cMsgStatList *	Get_Stat_List(void) {return PStatList;}
		bool Is_Bad_Connection(void) {return(IsBadConnection);};
		void Set_Rhost_Is_In_Game(int id, bool state);
		void Set_Rhost_Expect_Packet_Flood(int id, bool state);
		void Allow_Extra_Timeout_For_Loading(void);
		void Allow_Packet_Processing(bool set) {CanProcess = set;}

		void Install_Accept_Handler(Accept_Handler handler);
		void Install_Refusal_Handler(Refusal_Handler handler);
		void Install_Server_Broken_Connection_Handler(Server_Broken_Connection_Handler handler);
		void Install_Client_Broken_Connection_Handler(Client_Broken_Connection_Handler handler);
		void Install_Eviction_Handler(Eviction_Handler handler);
		void Install_Conn_Handler(Conn_Handler handler);
		void Install_Application_Acceptance_Handler(Application_Acceptance_Handler handler);
		void Install_Server_Packet_Handler(Server_Packet_Handler handler);
		void Install_Client_Packet_Handler(Client_Packet_Handler handler);

		// Need to use this. ST - 8/10/2001 11:52AM
      void Send_Packet_To_Address(cPacket & packet, struct sockaddr_in* p_address);


#ifdef WWDEBUG
		// Debug support for latency testing.
		static void Set_Latency(int low, int high);
		static void Get_Latency(int &low, int &high, int &current);
#endif //WWDEBUG


   private:
      cConnection(const cConnection& rhs); // Disallow copy (compile/link time)
      cConnection& operator=(const cConnection& rhs); // Disallow assignment (compile/link time)

      void Init_As_Client(struct sockaddr_in* p_server_address, unsigned short my_port = 0);
      bool Demultiplex_R_Or_U_Packet(cPacket * p_packet, int rhost_id);
      void Send_Accept_Sc(int new_rhost_id);
      bool Bind(USHORT port, ULONG addr = 0);
      bool Receive_Packet();
		int Low_Level_Send_Wrapper(cPacket & packet, struct sockaddr_in* p_address);
      int Send_Wrapper(cPacket & packet, struct sockaddr_in* p_address);
      int Send_Wrapper(cPacket & packet, int addressee);
		int Low_Level_Receive_Wrapper(cPacket & packet);
      int Receive_Wrapper(cPacket & packet);
      void Set_R_And_U_Packet_Id(cPacket & packet, int addressee, BYTE send_type);
      void R_And_U_Send(cPacket & packet, int addressee);
      void Send_Ack(struct sockaddr_in* p_address, int reliable_packet_id);
		void Send_Refusal_Sc(struct sockaddr_in* p_address, REFUSAL_CODE refusal_code);
      void Process_Connection_Request(cPacket & packet);
      void Send_Keepalives();
		static LPCSTR Type_Translation(int type);
      bool Sender_Id_Tests(cPacket & packet);
      USHORT Calculate_Packet_Bits(USHORT app_bytes);
      int Single_Player_sendto(cPacket & packet);
      int Single_Player_recvfrom(char * data);
      int Address_To_Rhostid(const struct sockaddr_in* p_address);
		bool Is_Time_To_Resend_Packet_To_Remote_Host(const cPacket *packet, cRemoteHost *rhost);
		bool Is_Packet_Too_Old(const cPacket *packet, cRemoteHost *rhost);

      int LocalId;		// Each client has a unique id
      USHORT LocalPort; // port we are bound to.
		double MaxAcceptablePacketlossPc;
		cNetStats CombinedStats;
		cNetStats AveragedStats;
      unsigned int ThisFrameTimeMs; // to avoid excess timeGetTime clls
      bool IsServer; // for C/S specific-code
      bool IsDedicatedServer; // for C/S specific-code
      static BOOL IsFlowControlEnabled;
		bool InitDone; // Used to ensure certain API's are used at the right time.
      SOCKET Sock;
      USHORT SimulatedPacketLossPerRANDMAX;
      USHORT SimulatedPacketDuplicationPerRANDMAX;
      int MinimumLatencyMs;
      int MaximumLatencyMs;
      int RefusalPacketSendId; // server
      int HighestRefusalPacketRcvId; // client
		ULONG BandwidthBudgetOut;
      SList<cPacket> PacketList;
		int ServiceCount;
		bool IsBadConnection;
		cRemoteHost ** PRHost;
		int MinRHost;
		int MaxRHost;
      int					NumRHosts;
		bool					IsDestroy;
		static UINT			TotalCompressedBytesSent;
		static UINT			TotalUncompressedBytesSent;
		cMsgStatList *		PStatList;
		unsigned int		ExtraTimeoutTime;
		unsigned int		ExtraTimeoutTimeStarted;
		bool					CanProcess;

		Accept_Handler								AcceptHandler;
		Refusal_Handler							RefusalHandler;
		Server_Broken_Connection_Handler		ServerBrokenConnectionHandler;
		Client_Broken_Connection_Handler		ClientBrokenConnectionHandler;
		Eviction_Handler							EvictionHandler;
		Conn_Handler								ConnHandler;
		Application_Acceptance_Handler		ApplicationAcceptanceHandler;
		Server_Packet_Handler					ServerPacketHandler;
		Client_Packet_Handler					ClientPacketHandler;

#ifdef WWDEBUG
		// Testing support for high latency connections.
		// Dynamic vector is ineffecient here but it doesn't matter since this is a debug testing only kinda thing.
		static int LatencyAddLow;
		static int LatencyAddHigh;
		static int CurrentLatencyAdd;
		static unsigned int LastLatencyChange;
		DynamicVectorClass<cPacket*>			LaggedPackets;
		DynamicVectorClass<unsigned int>	LaggedPacketTimes;
		DynamicVectorClass<int>					LaggedPacketRetCodes;
#endif //WWDEBUG

};

#endif // CONNECT_H













      //void Init_As_Client(LPCSTR server_ip, USHORT server_port);
      //void Send_Packet_To_All(cPacket & packet, BYTE send_flags); // send to all rhosts
		//cRemoteHost * PRHost[MAX_RHOSTS];
		//virtual void Server_Broken_Connection_Handler(int rhost_id);
		//virtual void Client_Broken_Connection_Handler();
		//virtual void Accept_Handler();
		//virtual void Refusal_Handler(int refusal_code);
      //virtual void Connection_Handler(int new_rhost_id);
		//virtual bool Application_Acceptance_Handler(cPacket & packet);
      //virtual void Eviction_Handler(int evicted_rhost_id);
		//virtual void Server_Packet_Handler(cPacket & packet, int rhost_id);
		//virtual void Client_Packet_Handler(cPacket & packet);
