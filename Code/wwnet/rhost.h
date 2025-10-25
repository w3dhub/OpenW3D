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
// Filename:     rhost.h
// Project:      wwnet
// Author:       Tom Spencer-Smith
// Date:         June 1998
// Description:
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef RHOST_H
#define RHOST_H

#include "netstats.h"
#include "wwpacket.h"
#include "bittype.h"
#include "slist.h"
#include "wwdebug.h"

#include "win.h"
#include "socket_wrapper.h"

//const USHORT MAX_MESSAGE_TYPES = 256;


//
// ResendTimeoutMs is dynamic, and works like this: It starts out very low,
// say 10ms, as given by DEFAULT_RESEND_TIMEOUT_MS. Then it is kept 10%
// higher than the highest ping. Every 60s the stats are reset, and
// at that time ResendTimeoutMs is set to be min(3 * the average
// ping for that sample, max ping for that sample). This means that
// rare outlandishly large pings don't cause our ResendTimeoutMs to
// be too conservative.
// It's all very ad hoc of course and will need refinement...
//

enum {
	RELIABLE_SEND_LIST = 0,
	RELIABLE_RCV_LIST,
	UNRELIABLE_SEND_LIST,
	UNRELIABLE_RCV_LIST
};

class cRemoteHost
{
   public:
      cRemoteHost();
      ~cRemoteHost();

		void Add_Packet(cPacket & packet, BYTE list_type);
		void Remove_Packet(int reliable_packet_id, BYTE list_type);
      void Toggle_Flow_Control();
		void Init_Stats();
		int Get_Last_Service_Count()						{return LastServiceCount;}
		void Set_Last_Service_Count(int service_count);
      void Compute_List_Max(int list_type);
      int Get_List_Max(int list_type);
		void Set_List_Processing_Time(int list_type, int processing_time_ms);
      int Get_List_Processing_Time(int list_type);


		int Get_Last_Contact_Time()						{return LastContactTime;}
		void Set_Last_Contact_Time(int time)			{LastContactTime = time;}

		struct sockaddr_in & Get_Address();
		void Set_Address(struct sockaddr_in & address)			{Address = address;}

		int	Get_Target_Bps(void) const					{return TargetBps;}
		//void	Set_Target_Bps(int bps)						{WWASSERT(bps > 0); TargetBps = bps;}
		void	Set_Target_Bps(int bps)						{TargetBps = bps;}

		int	Get_Maximum_Bps(void) const				{return MaximumBps;}
		//void	Set_Maximum_Bps(int bps)					{WWASSERT(bps > 0); MaximumBps = bps;}
		void	Set_Maximum_Bps(int bps)					{MaximumBps = bps;}

		cNetStats & Get_Stats()								{return Stats;}

		double Get_Threshold_Priority() const			{return ThresholdPriority;}
		float Get_Bandwidth_Multiplier(void) const	{return BandwidthMultiplier;}
		void Set_Average_Priority(float ave)			{AverageObjectPriority = ave;}
		float Get_Average_Priority(void)					{return(AverageObjectPriority);}


		USHORT Get_Resend_Timeout_Ms() const			{return ResendTimeoutMs;}

      //
      // Ping
      //
		int Get_Num_Internal_Pings() const				{return NumInternalPings;}
		int Get_Total_Internal_Pingtime_Ms() const	{return TotalInternalPingtimeMs;}
      int Get_Average_Internal_Pingtime_Ms() const {return AverageInternalPingtimeMs;}
      int Get_Min_Internal_Pingtime_Ms() const		{return MinInternalPingtimeMs;}
      int Get_Max_Internal_Pingtime_Ms() const		{return MaxInternalPingtimeMs;}

		int Get_Reliable_Packet_Send_Id()	const		{return ReliablePacketSendId;}
		int Get_Unreliable_Packet_Send_Id() const		{return UnreliablePacketSendId;}
		int Get_Reliable_Packet_Rcv_Id()		const		{return ReliablePacketRcvId;}
		int Get_Unreliable_Packet_Rcv_Id()	const		{return UnreliablePacketRcvId;}

		void Set_Reliable_Packet_Send_Id(int id)		{ReliablePacketSendId = id;}
		void Set_Unreliable_Packet_Send_Id(int id)	{UnreliablePacketSendId = id;}
		void Set_Reliable_Packet_Rcv_Id(int id)		{ReliablePacketRcvId = id;}
		void Set_Unreliable_Packet_Rcv_Id(int id)		{UnreliablePacketRcvId = id;}

		void Increment_Reliable_Packet_Send_Id()		{ReliablePacketSendId++;}
		void Increment_Unreliable_Packet_Send_Id()	{UnreliablePacketSendId++;}
		void Increment_Reliable_Packet_Rcv_Id()		{ReliablePacketRcvId++;}
		void Increment_Unreliable_Packet_Rcv_Id()		{UnreliablePacketRcvId++;}

		unsigned long Get_Last_Keepalive_Time_Ms() const	{return LastKeepaliveTimeMs;}
		void Set_Last_Keepalive_Time_Ms(unsigned long time_ms)	{LastKeepaliveTimeMs = time_ms;}

      SList<cPacket> & Get_Packet_List(int index)	{WWASSERT(index >= 0 && index < 4); return PacketList[index];}

		bool Must_Evict() const								{return MustEvict;}
		void Set_Must_Evict(bool flag)					{MustEvict = flag;}

      void Adjust_Flow_If_Necessary(float sample_time_ms);
		void Adjust_Resend_Timeout(void);

		void	Set_Id(int id)									{WWASSERT(id >= 0); Id = id;}
		int	Get_Id(void)									{WWASSERT(Id >= 0); return Id;}

		void Set_Is_Loading(bool state);
		bool Get_Is_Loading(void)							{return(IsLoading);}
		bool Was_Recently_Loading(unsigned long time = 0);

		void Set_Flood(bool state);
		bool Get_Flood(void)									{return(ExpectPacketFlood);}

		unsigned long Get_Creation_Time(void)			{return(CreationTime);}
		unsigned long Get_Total_Resends(void)			{return(TotalResends);}
		void Increment_Resends(void)						{TotalResends++;}
		void Set_Total_Resent_Packets_In_Queue (int resent_packets) {TotalResentPacketsInQueue = resent_packets;}


		inline int Get_Priority_Update_Counter(void)	{return(PriorityUpdateCounter);}
		inline void Increment_Priority_Count(void)	{PriorityUpdateCounter++; if (PriorityUpdateCounter > PriorityUpdateRate) PriorityUpdateCounter = 0;}
		static void Set_Priority_Update_Rate(int rate)	{PriorityUpdateRate = rate;}

		static inline void Set_Allow_Extra_Modem_Bandwidth_Throttling(bool set) {AllowExtraModemBandwidthThrottling = set;}

   private:
      cRemoteHost(const cRemoteHost& rhs); // Disallow copy (compile/link time)
      cRemoteHost& operator=(const cRemoteHost& rhs); // Disallow assignment (compile/link time)
		void Dam_The_Flood(void);
		bool Is_Outgoing_Flooded(void);


		cNetStats		Stats;
		double			ThresholdPriority;
		double			TPIncrement;
		int				LastReliableSendId;
		int				LastUnreliableSendId;
		USHORT			ResendTimeoutMs;
		int				NumInternalPings;
		int				TotalInternalPingtimeMs;
      int				AverageInternalPingtimeMs;
      int				MinInternalPingtimeMs;
      int				MaxInternalPingtimeMs;
      struct sockaddr_in		Address;
		int				ReliablePacketSendId;
		int				UnreliablePacketSendId;
		int				ReliablePacketRcvId;
		int				UnreliablePacketRcvId;
      SList<cPacket>	PacketList[4];	// list of all player objects
      int				ListMax[4];
      int				ListProcessingTime[4];
      unsigned long	LastKeepaliveTimeMs;
      bool				MustEvict;
      BOOL				IsFlowControlEnabled;
		int				LastServiceCount;
		int				LastContactTime;
		int				TargetBps;
		int				MaximumBps;
		int				Id;
		float				BandwidthMultiplier;
		float				AverageObjectPriority;
		bool				IsLoading;
		bool				ExpectPacketFlood;
		unsigned long	FloodTimer;
		unsigned long	WasLoading;
		unsigned long	TotalResends;
		unsigned long	CreationTime;
		int				PriorityUpdateCounter;

		//
		// Variables for detecting outgoing packet floods.
		//
		unsigned long	ExtendedAveragePingTime;
		int				ExtendedAverageCount;
		int				LastAveragePingTime;
		bool				IsOutgoingFlooded;
		int				TotalResentPacketsInQueue;
		unsigned long	NextOutgoingFloodActionTime;
		int				NumOutgoingFloods;

		static bool		AllowExtraModemBandwidthThrottling;
		static int		PriorityUpdateRate;

};

#endif // RHOST_H















      //
      // When this is set to true, sends to ALL will include this rhost.
      // This is needed to allow a slot to be reserved for a rhost before
      // using it extensively. Generally, when a client is accepted into a
      // game he wil need to do an extensive data load during which there
      // will be no network servicing. When that is finished he should signal
      // readyness.
      //
      //bool IsReadyForAllData;


				//int NumFailsInSampleTime;
		//int NumConsecutiveTPCorrectionsDownwards;
		//int NumConsecutiveTPCorrectionsUpwards;
