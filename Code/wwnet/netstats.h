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
// Filename:     netstats.h
// Project:      wwnet
// Author:       Tom Spencer-Smith
// Date:         Oct 1998
// Description:
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef NETSTATS_H
#define NETSTATS_H

#include "bittype.h"
#include "wwdebug.h"

//
// Statistics.
//   Abbrev:
//     R   = Reliable
//     U   = Unreliable
//     Pkt = packet
//     Rcv = Received
//     App = Application
//     Hdr = Header
//     Byte = bytes, including header
//
//     Syntax is U/R then Pkt/Byte/AppByte then Sent/Rcv
//
enum Statistic
{
   STAT_PktSent,
   STAT_PktRcv,

   STAT_MsgSent,       // Need to count separately since messages can be combined
   STAT_MsgRcv,        // into packets TSS - bug: this not accurate because of combining

   STAT_AppByteSent,
   STAT_AppByteRcv,

   //STAT_HdrByteSent,
   //STAT_HdrByteRcv,

   STAT_BitsSent,
	STAT_BitsRcv,

	STAT_UPktSent,
   STAT_RPktSent,
   STAT_UByteSent,
   STAT_RByteSent,
   STAT_UPktRcv,
   STAT_RPktRcv,
   STAT_UByteRcv,
   STAT_RByteRcv,

   /*
	STAT_AppDataSentPc,     // App / (App + Hdr)
   STAT_AppDataRcvPc,      // App / (App + Hdr)
	*/

   STAT_AckCountSent,
   STAT_AckCountRcv,

   STAT_DiscardCount,
   STAT_ResendCount,
   STAT_SendFailureCount,

	STAT_PktRcvRemote,

	STAT_UPktShouldRcv,

   STAT_UPktRcv2,            // used in packetloss calculations. Slightly less than STAT_UPktRcv

   STAT_ServiceCount,

	STAT_COUNT  // must include, used as array bound
};

//-----------------------------------------------------------------------------
class cNetStats
{
	public:
		cNetStats();
      ~cNetStats() {};

		void Init_Net_Stats();
		bool Update_If_Sample_Done(int this_frame_time, bool force_update = false);

      double Get_Pc_Packetloss_Received() const;
      void Set_Pc_Packetloss_Sent(double packetloss_pc);
      double Get_Pc_Packetloss_Sent() const {return RemotePacketloss;}
		int Get_Remote_Service_Count() {return RemoteServiceCount;}
		void Set_Remote_Service_Count(int remote_service_count);

		int Get_Sample_Start_Time() const {return SampleStartTime;}

      int Get_Last_Unreliable_Packet_Id() const {return LastUnreliablePacketId;}
      void Set_Last_Unreliable_Packet_Id(int id) {LastUnreliablePacketId = id;}

      int Get_Freeze_Packet_Id() const {return FreezePacketId;}

		void Increment_Unreliable_Count() {UnreliableCount++;}

      UINT Get_Stat_Sample(int stat)	const			{WWASSERT(stat >= 0 && stat < STAT_COUNT); return StatSample[stat];}
      UINT Get_Stat_Macro_Sample(int stat) const	{WWASSERT(stat >= 0 && stat < STAT_COUNT); return StatMacroSample[stat];}
      UINT Get_Stat_Snapshot(int stat)	const			{WWASSERT(stat >= 0 && stat < STAT_COUNT); return StatSnapshot[stat];}
      UINT Get_Stat_Macro_Snapshot(int stat)	const	{WWASSERT(stat >= 0 && stat < STAT_COUNT); return StatMacroSnapshot[stat];}
      UINT Get_Stat_Total(int stat)		const			{WWASSERT(stat >= 0 && stat < STAT_COUNT); return StatTotal[stat];}
      UINT Get_Stat_Average(int stat)	const			{WWASSERT(stat >= 0 && stat < STAT_COUNT); return StatAverage[stat];}

      void Increment_Stat_Sample(int stat, UINT increment)			{WWASSERT(stat >= 0 && stat < STAT_COUNT); StatSample[stat] += increment;}
      void Increment_Stat_Macro_Sample(int stat, UINT increment)	{WWASSERT(stat >= 0 && stat < STAT_COUNT); StatMacroSample[stat] += increment;}
      void Increment_Stat_Snapshot(int stat, UINT increment)		{WWASSERT(stat >= 0 && stat < STAT_COUNT); StatSnapshot[stat] += increment;}
      void Increment_Stat_Macro_Snapshot(int stat, UINT increment){WWASSERT(stat >= 0 && stat < STAT_COUNT); StatMacroSnapshot[stat] += increment;}
      void Increment_Stat_Total(int stat, UINT increment)			{WWASSERT(stat >= 0 && stat < STAT_COUNT); StatTotal[stat] += increment;}
      void Increment_Stat_Average(int stat, UINT increment)			{WWASSERT(stat >= 0 && stat < STAT_COUNT); StatAverage[stat] += increment;}

      //
		// TSS - shift these down, and use above access operators
		//
		UINT StatSample[STAT_COUNT];
		UINT StatMacroSample[STAT_COUNT];
      UINT StatSnapshot[STAT_COUNT];
      UINT StatMacroSnapshot[STAT_COUNT];
      UINT StatTotal[STAT_COUNT];
      UINT StatAverage[STAT_COUNT];

	private:
      cNetStats(const cNetStats& source); // Disallow copy (compile/link time)
      cNetStats& operator=(const cNetStats& rhs); // Disallow assignment (compile/link time)

      //int PrevLastUnreliable;
		unsigned long SampleStartTime;
      int LastUnreliablePacketId;
      int FreezePacketId;
      int UnreliableCount;

		unsigned long StartTime;
      double RemotePacketloss;
		int RemoteServiceCount;
};

//-----------------------------------------------------------------------------

#endif // NETSTATS_H














		//static const USHORT SAMPLE_TIME; // stats gathering period in ms
