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
// Filename:     netstats.cpp
// Project:      wwnet
// Author:       Tom Spencer-Smith
// Date:         Oct 1998
// Description:
//
//------------------------------------------------------------------------------------
#include "netstats.h" // I WANNA BE FIRST!

#include "win.h"
#include "systimer.h"
#include "miscutil.h"
#include "netutil.h"
#include "wwdebug.h"

//
// class defines
//

//------------------------------------------------------------------------------------
cNetStats::cNetStats() :
	LastUnreliablePacketId(-1),
   FreezePacketId(-1)
{
	Init_Net_Stats();
}

//------------------------------------------------------------------------------------
void cNetStats::Init_Net_Stats()
{
   StartTime = TIMEGETTIME();
   SampleStartTime = StartTime;

   for (int statistic = 0; statistic < STAT_COUNT; statistic++) {
      StatTotal[statistic]				= 0;
      StatAverage[statistic]			= 0;
      StatSnapshot[statistic]			= 0;
      StatMacroSnapshot[statistic]	= 0;
      StatSample[statistic]			= 0;
      StatMacroSample[statistic]		= 0;
   }

	RemotePacketloss = 0;
	UnreliableCount = 0;
   FreezePacketId = LastUnreliablePacketId;
	RemoteServiceCount = 0;
}

//------------------------------------------------------------------------------------
double cNetStats::Get_Pc_Packetloss_Received() const
{
   double packetloss_pc = 0;

   int packet_count = LastUnreliablePacketId - FreezePacketId;
   WWASSERT(packet_count >= 0);

   if (packet_count > 0) {
      //
      // Must add 1 to PrevLastUnreliable because packet id starts at 0
      //
		packetloss_pc = 100 * (1 - UnreliableCount / (double) packet_count);
   }

   return packetloss_pc;
}

//------------------------------------------------------------------------------------
void cNetStats::Set_Pc_Packetloss_Sent(double /* packetloss_pc */)
{
   /*TSS102901
	WWASSERT(packetloss_pc > -MISCUTIL_EPSILON && packetloss_pc < 100 + MISCUTIL_EPSILON);
   RemotePacketloss = packetloss_pc;
	*/
}

//------------------------------------------------------------------------------------
void cNetStats::Set_Remote_Service_Count(int remote_service_count)
{
	//WWASSERT(remote_service_count >= 0);
   RemoteServiceCount = remote_service_count;
}

//------------------------------------------------------------------------------------
bool cNetStats::Update_If_Sample_Done(int this_frame_time, bool force_update)
{
   bool is_updated = false;

	static int update_count = 0;

   if (force_update || this_frame_time - SampleStartTime > cNetUtil::NETSTATS_SAMPLE_TIME_MS) {

		update_count++;

		double total_time = (this_frame_time - StartTime) / 1000.0;

		if (total_time < MISCUTIL_EPSILON) {
			total_time = MISCUTIL_EPSILON;
		}

		for (int statistic = 0; statistic < STAT_COUNT; statistic++) {
			StatTotal[statistic]				+= StatSample[statistic];
			StatAverage[statistic]			= (unsigned int) (StatTotal[statistic] / total_time);
			StatSnapshot[statistic]			= StatSample[statistic];
			StatMacroSample[statistic]		+= StatSample[statistic];
			StatSample[statistic]			= 0;

			if (update_count % 4 == 0) {
				StatMacroSnapshot[statistic] = StatMacroSample[statistic];
				StatMacroSample[statistic]	  = 0;
			}
		}

		SampleStartTime = this_frame_time;

      is_updated = true;
   }

   return is_updated;
}



















//const USHORT cNetStats::SAMPLE_TIME = 500;

   // crash here when exit server thread before client
   //WWASSERT(packetloss_pc >= 0 && packetloss_pc <= 100);

		/*
		//
		// These 2 stats are computed from others
		//
		if (StatSample[STAT_AppByteSent] > 0) {
			StatSample[STAT_AppDataSentPc] = (unsigned int) (100 * StatSample[STAT_AppByteSent] /
				(double) (StatSample[STAT_AppByteSent] + StatSample[STAT_HdrByteSent]));
		}

		if (StatSample[STAT_AppByteRcv] > 0) {
			StatSample[STAT_AppDataRcvPc] = (unsigned int) (100 * StatSample[STAT_AppByteRcv] /
				(double) (StatSample[STAT_AppByteRcv] + StatSample[STAT_HdrByteRcv]));
		}
		*/
