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
// Filename:     rhost.cpp
// Project:      wwnet
// Author:       Tom Spencer-Smith
// Date:         June 1998
// Description:  Data about a remote host
//
//-----------------------------------------------------------------------------
#include "rhost.h" // I WANNA BE FIRST!

#include <stdlib.h>
#include <math.h>

#include "systimer.h"
#include "miscutil.h"
#include "mathutil.h"
#include "connect.h"
#include "wwdebug.h"
#include "packetmgr.h"
#include <algorithm>

bool cRemoteHost::AllowExtraModemBandwidthThrottling = true;
int cRemoteHost::PriorityUpdateRate = 15;

//
// class defines
//

//-----------------------------------------------------------------------------
cRemoteHost::cRemoteHost() :
	ReliablePacketSendId(0),
	UnreliablePacketSendId(0),
   ReliablePacketRcvId(0),
   UnreliablePacketRcvId(0),
   LastKeepaliveTimeMs(TIMEGETTIME()),
   IsFlowControlEnabled(cConnection::Is_Flow_Control_Enabled()),
   MustEvict(false),
   LastReliableSendId(-2),		// dummy value
   LastUnreliableSendId(-2),	// dummy value
	ResendTimeoutMs(cNetUtil::Get_Default_Resend_Timeout_Ms()),
	LastServiceCount(0),
	LastContactTime(0),
	TargetBps(0),
	MaximumBps(0),
	MaxInternalPingtimeMs(0),
	AverageInternalPingtimeMs(0),
	BandwidthMultiplier(1.0f),
	AverageObjectPriority(0.5f),
	IsLoading(false),
	ExpectPacketFlood(false),
	WasLoading(0),
	CreationTime(TIMEGETTIME()),
	TotalResends(0),
	PriorityUpdateCounter(0),
	ExtendedAveragePingTime(0),
	ExtendedAverageCount(0),
	LastAveragePingTime(0),
	IsOutgoingFlooded(false),
	TotalResentPacketsInQueue(0),
	NextOutgoingFloodActionTime(0),
	NumOutgoingFloods(0)
{
   //WWDEBUG_SAY(("cRemoteHost::cRemoteHost\n"));

	ZeroMemory(&Address, sizeof(struct sockaddr_in));

	if (IsFlowControlEnabled) {
      //ThresholdPriority = cNetUtil::Get_Initial_Threshold_Priority();
      //ThresholdPriority = 1.0;
      ThresholdPriority = 0.5;
   } else {
      ThresholdPriority = 0.0;
   }
//ThresholdPriority = 0.0;

	TPIncrement = 0.01;//TSS2001d

	Init_Stats();
}

//-----------------------------------------------------------------------------
void cRemoteHost::Init_Stats()
{
	Stats.Init_Net_Stats();

	//
	// Possibly, pull back ResendTimeoutMs if we have had a chance to measure ping time yet.
	//
	Adjust_Resend_Timeout();

	//if (MaxInternalPingtimeMs && AverageInternalPingtimeMs) {
	//	int candidate_resend_timeout_ms = min(3 * AverageInternalPingtimeMs,
	//		(int) (1.1 * MaxInternalPingtimeMs));
	//	if (candidate_resend_timeout_ms < ResendTimeoutMs) {
	//		WWASSERT(candidate_resend_timeout_ms < 0xffff);
	//		ResendTimeoutMs = candidate_resend_timeout_ms;
	//		//WWDEBUG_SAY((">> ResendTimeoutMs for rhost %d = %d\n", Id, ResendTimeoutMs));
	//	}
	//}

	//WWDEBUG_SAY(("ResendTimeoutMs for rhost %d = %d\n", Id, (unsigned int)ResendTimeoutMs));
	WWDEBUG_SAY(("ResendTimeoutMs for rhost = %d\n", (unsigned int)ResendTimeoutMs));

	TotalInternalPingtimeMs = 0;
	NumInternalPings = 0;
   AverageInternalPingtimeMs = 0;//-1;
   MinInternalPingtimeMs = 1000000;
   MaxInternalPingtimeMs = 0;
}

//-----------------------------------------------------------------------------
void cRemoteHost::Set_Last_Service_Count(int service_count)
{
	WWASSERT(service_count >= 0);
	LastServiceCount = service_count;
}

//-----------------------------------------------------------------------------
void cRemoteHost::Compute_List_Max(int list_type)
{
	WWASSERT(list_type >= 0 && list_type < 4);
	ListMax[list_type] = PacketList[list_type].Get_Count();
}

//-----------------------------------------------------------------------------
int cRemoteHost::Get_List_Max(int list_type)
{
	WWASSERT(list_type >= 0 && list_type < 4);
	return ListMax[list_type];
}

//-----------------------------------------------------------------------------
void cRemoteHost::Set_List_Processing_Time(int list_type, int processing_time_ms)
{
	WWASSERT(list_type >= 0 && list_type < 4);
	ListProcessingTime[list_type] = processing_time_ms;
}

//-----------------------------------------------------------------------------
int cRemoteHost::Get_List_Processing_Time(int list_type)
{
	WWASSERT(list_type >= 0 && list_type < 4);
	return ListProcessingTime[list_type];
}

//-----------------------------------------------------------------------------
struct sockaddr_in & cRemoteHost::Get_Address()
{
	return Address;
}

//-----------------------------------------------------------------------------
cRemoteHost::~cRemoteHost()
{
   SLNode<cPacket> * objnode;
   cPacket * p_packet;
   for (int list_type = 0; list_type < 4; list_type++) {
      for (objnode = PacketList[list_type].Head(); objnode != NULL;) {
         p_packet = objnode->Data();
         WWASSERT(p_packet != NULL);
         objnode = objnode->Next();
         PacketList[list_type].Remove(p_packet);
		   p_packet->Flush();
         delete p_packet;
      }
   }
}

//------------------------------------------------------------------------------------
void cRemoteHost::Add_Packet(cPacket & packet, BYTE list_type)
{
	WWASSERT(
      list_type == RELIABLE_SEND_LIST   ||
      list_type == RELIABLE_RCV_LIST    ||
      list_type == UNRELIABLE_SEND_LIST ||
      list_type == UNRELIABLE_RCV_LIST);
	WWASSERT(packet.Get_Id() >= 0);

   if (list_type == RELIABLE_SEND_LIST) {
		//
		// Test to make sure that additions to send list are in sequence.
		//
		if (LastReliableSendId != -2) {
			WWASSERT(packet.Get_Id() == LastReliableSendId + 1);
		}
		LastReliableSendId = packet.Get_Id();
   } else if (list_type == UNRELIABLE_SEND_LIST) {

		if (LastUnreliableSendId != -2) {
			WWASSERT(packet.Get_Id() == LastUnreliableSendId + 1);
		}
		LastUnreliableSendId = packet.Get_Id();
   }

   cPacket * p_packet = new cPacket;
   WWASSERT(p_packet != NULL);
   *p_packet = packet; // copy data

	if (list_type == RELIABLE_SEND_LIST || list_type == UNRELIABLE_SEND_LIST) {
		PacketList[list_type].Add_Tail(p_packet);
	} else {

		//
		// Locate insertion point according to packet id
		//
		SLNode<cPacket> * objnode;
		cPacket * obj = NULL;
		for (objnode = PacketList[list_type].Head(); objnode; objnode = objnode->Next()) {
			obj = objnode->Data();
			WWASSERT(obj != NULL);

			if (obj->Get_Id() > packet.Get_Id()) {
				break;
			}
		}

		//
		// Insert packet at designated point
		// TSS - this is inefficient: we have already parsed the
		// list once
		//
		if (objnode == NULL) {
			PacketList[list_type].Add_Tail(p_packet);
		} else {
			PacketList[list_type].Insert_Before(p_packet, obj);
		}
	}
}

//------------------------------------------------------------------------------------
void cRemoteHost::Remove_Packet(int packet_id, BYTE list_type)
{
   WWASSERT(packet_id >= 0);
	WWASSERT(
      list_type == RELIABLE_SEND_LIST   ||
      list_type == RELIABLE_RCV_LIST    ||
      list_type == UNRELIABLE_SEND_LIST ||
      list_type == UNRELIABLE_RCV_LIST);

   SLNode<cPacket> * objnode;
	for (objnode = PacketList[list_type].Head(); objnode != NULL;) {

      cPacket * p_packet = objnode->Data();
      WWASSERT(p_packet != NULL);
      objnode = objnode->Next();

		if (packet_id == p_packet->Get_Id()) {

         if (list_type == RELIABLE_SEND_LIST) {// &&

				// Packets that require a resend shouldn't count towards ping time calculations. It may be being removed because
				// the ACK to the first send just came in and if we just resent it then the ping time will look really low so we get
				// biased towards a low resend timeout value on connections of variable quality. ST - 12/7/2001 12:48PM
				if (p_packet->Get_Resend_Count() == 0 || NumInternalPings == 0) {
					unsigned int time = TIMEGETTIME();
					int ping_time = time - p_packet->Get_Send_Time();

					if (p_packet->Get_Resend_Count() != 0) {
						WWASSERT(NumInternalPings == 0);

						// If we are not getting any timing info at all then we need to do something. Use the first send time. It's going
						// to make it big but that should cut down on the resends and let us get better timing info.
						if (NumInternalPings == 0) {
							ping_time = TIMEGETTIME() - p_packet->Get_First_Send_Time();
						}
					}
					TotalInternalPingtimeMs += ping_time;
					NumInternalPings++;
					if (NumInternalPings > 0) {
						AverageInternalPingtimeMs = cMathUtil::Round(TotalInternalPingtimeMs / (double) NumInternalPings);
					} else {
						AverageInternalPingtimeMs = 0;
					}
					if (ping_time < MinInternalPingtimeMs) {
						MinInternalPingtimeMs = ping_time;
					}
					if (ping_time > MaxInternalPingtimeMs) {
						MaxInternalPingtimeMs = ping_time;
#if (0)
						int candidate_resend_timeout_ms = (int) (MaxInternalPingtimeMs
							* 1.1);
						if (candidate_resend_timeout_ms > ResendTimeoutMs) {
							ResendTimeoutMs = candidate_resend_timeout_ms;
							//WWDEBUG_SAY((">> ResendTimeoutMs for rhost %d = %d\n", Id, ResendTimeoutMs));
						}
#endif //(0)
					}
				}
			}

         //if (list_type == RELIABLE_SEND_LIST) {
				//WWDEBUG_SAY(("Removing packet %d from RELIABLE_SEND_LIST\n", packet_id));
			//}

         PacketList[list_type].Remove(p_packet);
			p_packet->Flush();
         delete p_packet;

			break;
      }
	}
}

//------------------------------------------------------------------------------------
void cRemoteHost::Toggle_Flow_Control()
{
   IsFlowControlEnabled = !IsFlowControlEnabled;

   if (IsFlowControlEnabled) {
      //ThresholdPriority = 1.0;
      ThresholdPriority = 0.5;
   } else {
      ThresholdPriority = 0.0;
   }
//ThresholdPriority = 0.0;
}

//------------------------------------------------------------------------------------
void cRemoteHost::Adjust_Flow_If_Necessary(float /* sample_time_ms */)
{
   if (!IsFlowControlEnabled) {
      return;
   }

#ifdef OBSOLETE
	WWASSERT(TargetBps > 0);
	float sample_target_bits = sample_time_ms / 1000.0f * TargetBps;
	WWASSERT(sample_target_bits > 0);

	//TSS101401
	//float ratio = Stats.StatSnapshot[STAT_BitsSent] / sample_target_bits;
	float ratio = PacketManager.Get_Compressed_Bandwidth_Out(&Get_Address()) / sample_target_bits;

	if (ratio > MISCUTIL_EPSILON) {

		double last_tp = TPIncrement;
		if (ratio > 1)
		{
			TPIncrement = ::fabs(TPIncrement);
		}
		else
		{
			TPIncrement = -::fabs(TPIncrement);
		}

		if ((TPIncrement > 0 && last_tp > 0) ||
			 (TPIncrement < 0 && last_tp < 0))	{
			if (::fabs(TPIncrement) < 0.025) {
				TPIncrement *= 1.5;
			}
		} else {
			if (::fabs(TPIncrement) > 0.0001) {
				TPIncrement *= 0.5;
			}
		}

		double new_tp = ThresholdPriority + TPIncrement;

		if (new_tp < 0)
		{
			new_tp = 0;
		}
		else if (new_tp > 1)
		{
			new_tp = 1;
		}

		ThresholdPriority = new_tp;

		/*
		if (Id == 1) {
			WWDEBUG_SAY(("ratio = %-7d / %-7d = %5.2f, TPIncrement = %9.7f, ThresholdPriority = %9.7f\n",
				Stats.StatSnapshot[STAT_BitsSent], (int) sample_target_bits, ratio, TPIncrement, ThresholdPriority));
		}
		/**/
	}
#endif //OBSOLETE

	//
	// Do a similar calculation for the new bandwidth per object per client distribution method.
	//
	float actual = (float) PacketManager.Get_Compressed_Bandwidth_Out(&Get_Address());
	float desired = (float) TargetBps;	//sample_time_ms / 1000.0f * TargetBps;
	//if (Id > 1) {
		//WWDEBUG_SAY(("actual = %f, desired = %f\n", actual, desired));
	//}


	if (ExpectPacketFlood) {
		BandwidthMultiplier = 0.5f;	//1.5f;
		if (TIMEGETTIME() - FloodTimer > 8000) {
			ExpectPacketFlood = false;
		}
	} else {

		//
		// If we are sending way more than we know the remote host can receive then we need to send much less.
		//
		if (actual > desired) {
			BandwidthMultiplier = (BandwidthMultiplier * (desired / actual)) * 0.85f;
		} else {

			//
			// Another case we have to trap is a low bandwidth connection that has been accidentally flooded by having several
			// frames where we send more stuff than usual. It could also be a remote host that has incorrectly reported his
			// downstream bandwidth and can't receive as much as we think we can send to him. Or it could simply be some temporary
			// condition at either end of the connection or anywhere in between. If this happens we have to cut way back on sends
			// so that the connection can catch up.
			//
			if (AllowExtraModemBandwidthThrottling) {
				if (IsOutgoingFlooded) {

					//
					// We recently detected a outbound flooding condition. If we are still flooding then we have to take further action.
					//
					if (Is_Outgoing_Flooded()) {
						Dam_The_Flood();
					} else {
						IsOutgoingFlooded = false;
					}

				} else {

					//
					// See if we are flooding now.
					//
					if (Is_Outgoing_Flooded()) {

						IsOutgoingFlooded = true;

						//
						// Well, something is wrong. It's kind of hard to say exactly what the problem is so a remedy is only guesswork.
						// In the short term we can try reducing the BandwidthMultiplier to clamp down on non-guaranteed packets. If the
						// problem persists then we will have to reduce MaximumBps too.
						//
						NumOutgoingFloods++;
						NextOutgoingFloodActionTime = 0;
						Dam_The_Flood();
					}
				}
			}


			if (BandwidthMultiplier < 20.0f) {

				if (actual < (desired * 0.7f)) {

					/*
					** Give bandwidth a quicker boost
					*/
					BandwidthMultiplier += 0.5f * (1.0f - (actual / desired));
					if (BandwidthMultiplier > 20.0f) {
						BandwidthMultiplier = 20.0f;
					}
				} else {
					if (actual < (desired * 0.95f)) {
						/*
						** Gradually increase bandwidth usage until we are close to max utilization.
						*/
						BandwidthMultiplier += 0.1f * (1.0f - (actual / desired));
						if (BandwidthMultiplier > 20.0f) {
							BandwidthMultiplier = 20.0f;
						}
					}
				}
			}
		}
	}
}



//------------------------------------------------------------------------------------
bool cRemoteHost::Is_Outgoing_Flooded(void)
{
	//
	// Don't take action if the rhost is loading.
	//
	if (Was_Recently_Loading()) {
		return(false);
	}

	//
	// We can try to detect outgoing floods in two ways.
	//

	//
	// 1. Look for a spike in the ping times. If we are sending more than the remote host can receive then ping times will
	// quickly rise as each packet backs up at the receive end and thus gets ack'd later than usual.
	//
	if (ExtendedAverageCount) {

		// average ping time over the life of the connection to use as a base line.
		int average = (int)(ExtendedAveragePingTime / (unsigned)ExtendedAverageCount);

		if ((LastAveragePingTime > 1500 && MaximumBps < 100000) || LastAveragePingTime > 500 && LastAveragePingTime > average * 3) {

			//
			// Ping time is bigger than we expect. If we are still receiving stuff from this host then chances are we are
			// flooding him.
			//
			if (TIMEGETTIME() - LastContactTime < 1000) {
				if (!IsOutgoingFlooded) {
					WWDEBUG_SAY(("Detected abnormal ping times - assuming outbound connection to rhost %d is flooded\n", Id));
					WWDEBUG_SAY(("Normal average ping time = %d, last average ping time = %d\n", average, LastAveragePingTime));
				}
				return(true);
			}
		}
	}

	//
	// 2. An excessive number of packets in the out queue that are older than the average ping time. Since acks aren't
	// coming back we resend more and so exacerbate the problem.
	//
	int total_in_queue = PacketList[RELIABLE_SEND_LIST].Get_Count();

	// Let's say that if more than 90% of the packets in the queue have been resent then there is a problem.
	if (total_in_queue > 20 && (TotalResentPacketsInQueue*10) > (total_in_queue*9)) {
		//
		// More resends than we expect. If we are still receiving stuff from this host then chances are we are
		// flooding him.
		//
		if (TIMEGETTIME() - LastContactTime < 1000) {
			WWDEBUG_SAY(("Detected abnormally high number of resends - assuming outbound connection to rhost %d is flooded\n", Id));
			WWDEBUG_SAY(("Total packets in queue = %d, queue packets resent = %d\n", total_in_queue, TotalResentPacketsInQueue));
			return(true);
		}
	}
	return(false);
}


//------------------------------------------------------------------------------------
void cRemoteHost::Dam_The_Flood(void)
{
	WWASSERT(IsOutgoingFlooded);

	//
	// How long since we last took action?
	//
	bool action_time = (TIMEGETTIME() > NextOutgoingFloodActionTime) ? true : false;

	//
	// Try something else every now and then until things improve.
	//
	if (action_time) {

		//
		// Try reducing BandwidthMultiplier. This will be a temporary change and will be allowed to revert once the connection
		// recovers.
		//
		if (BandwidthMultiplier > 0.5f || NextOutgoingFloodActionTime == 0) {
			BandwidthMultiplier = BandwidthMultiplier / 2.0f;

			//
			// Give this a half second or so to take effect.
			//
			if (BandwidthMultiplier <= 0.5f) {
				//
				// Reducing MaximumBps is a last resort. Give it a little more time to recover.
				//
				NextOutgoingFloodActionTime = 2000 + TIMEGETTIME();
			} else {
				NextOutgoingFloodActionTime = 600 + TIMEGETTIME();
			}

		} else {

			//
			// Try reducing MaximumBps. This will be a permanent change. 30,000ish should be our minimum since the min requirement is
			// a 33600 modem.
			//
			if (MaximumBps > 30000) {
#ifdef WWDEBUG
				int old_bps = MaximumBps;
#endif //WWDEBUG
				if (MaximumBps > 100000) {
					MaximumBps = 100000;
				} else {
					if (MaximumBps > 56000) {
						MaximumBps = 56000;
					} else {
						//if (MaximumBps > 40000) {
						//	MaximumBps = 40000;
						//} else {
							if (MaximumBps > 33600) {
								MaximumBps = 33600;
							} else {
								MaximumBps = 30000;
							}
						//}
					}
				}
				WWDEBUG_SAY(("Reduced MaximumBps for rhost %d from %d to %d\n", Id, old_bps, MaximumBps));

				/*
				** Give this a few seconds to take effect before stepping down again.
				*/
				NextOutgoingFloodActionTime = 5000 + TIMEGETTIME();
			}
		}
	}
}





//------------------------------------------------------------------------------------
void cRemoteHost::Set_Flood(bool state)
{
	ExpectPacketFlood = state;
	if (state) {
		FloodTimer = TIMEGETTIME();
	}
}



//------------------------------------------------------------------------------------
void cRemoteHost::Set_Is_Loading(bool state)
{
	if (IsLoading && !state) {
		WasLoading = TIMEGETTIME();
	}
	IsLoading = state;
}



//------------------------------------------------------------------------------------
bool cRemoteHost::Was_Recently_Loading(unsigned int time)
{
	if (IsLoading) {
		return(true);
	}
	if (time == 0) {
		time = TIMEGETTIME();
	}

	if (time - WasLoading < 1000 * 8) {
		return(true);
	}
	return(false);
}





//------------------------------------------------------------------------------------
void cRemoteHost::Adjust_Resend_Timeout(void)
{
	if (NumInternalPings > 0) {

		if (MaxInternalPingtimeMs && AverageInternalPingtimeMs) {
			int candidate_resend_timeout_ms = std::min(3 * AverageInternalPingtimeMs, (int) (1.3f * (float)MaxInternalPingtimeMs));

			// Make sure it stays inside a reasonable range.
			candidate_resend_timeout_ms = std::min(candidate_resend_timeout_ms, 3000);
			candidate_resend_timeout_ms = std::max(candidate_resend_timeout_ms, 333);

			ResendTimeoutMs = (ResendTimeoutMs + candidate_resend_timeout_ms) / 2;
			if (candidate_resend_timeout_ms < ResendTimeoutMs) {
				WWASSERT(candidate_resend_timeout_ms < 0xffff);
				ResendTimeoutMs = candidate_resend_timeout_ms;
				//WWDEBUG_SAY((">> ResendTimeoutMs for rhost %d = %d\n", Id, ResendTimeoutMs));
			}

			//WWDEBUG_SAY(("ResendTimeoutMs for rhost = %d\n", (unsigned int)ResendTimeoutMs));

			//
			// Keep track of the last average we calculated plus the average ping over the life of the connection.
			//
			ExtendedAveragePingTime += (unsigned) TotalInternalPingtimeMs;
			ExtendedAverageCount++;
			LastAveragePingTime = AverageInternalPingtimeMs;
		}

		TotalInternalPingtimeMs = 0;
		NumInternalPings = 0;
   	AverageInternalPingtimeMs = 0;//-1;
   	MinInternalPingtimeMs = 65535;
   	MaxInternalPingtimeMs = 0;
	}
}