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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/clientpingmanager.cpp                    $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 12/09/01 6:40p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "clientpingmanager.h"

#include <windows.h>
#include "systimer.h"

#include "gamemode.h"
#include "cspingrequestevent.h"
#include "cnetwork.h"

//
// Class statics
//
int		cClientPingManager::PingNumber						= 0;
unsigned int		cClientPingManager::TimeSentMs						= 0;
unsigned int		cClientPingManager::LastRoundTripPingMs			= 0;
unsigned int		cClientPingManager::AvgRoundTripPingMs				= 0;
bool		cClientPingManager::IsAwaitingResponse				= false;
unsigned int		cClientPingManager::RoundTripPingSamplesMs[];

//-----------------------------------------------------------------------------
void
cClientPingManager::Init
(
	void
)
{
	PingNumber				= 0;
	TimeSentMs				= 0;
	LastRoundTripPingMs	= 0;
	AvgRoundTripPingMs	= 0;
	IsAwaitingResponse	= false;

	for (int i = 0; i < MAX_SAMPLES; i++)
	{
		RoundTripPingSamplesMs[i] = 0;
	}
}

//-----------------------------------------------------------------------------
void
cClientPingManager::Think
(
	void
)
{
	if (GameModeManager::Find("Combat")->Is_Active() && cNetwork::I_Am_Only_Client())
	{
		if (!IsAwaitingResponse)
		{
			unsigned int time_now_ms = TIMEGETTIME();
			if (time_now_ms - TimeSentMs >= MIN_PING_DELAY_MS)
			{
				PingNumber++;
				TimeSentMs = time_now_ms;
				IsAwaitingResponse = true;

				cCsPingRequestEvent * p_event = new cCsPingRequestEvent;
				p_event->Init(PingNumber);
			}
		}

		//
		// Propagate latency data to combat
		//
		CombatManager::Set_Last_Round_Trip_Ping_Ms(LastRoundTripPingMs);
		CombatManager::Set_Avg_Round_Trip_Ping_Ms(AvgRoundTripPingMs);
	}
}

//-----------------------------------------------------------------------------
unsigned int
cClientPingManager::Get_Last_Round_Trip_Ping_Ms
(
	void
)
{
	return LastRoundTripPingMs;
}

//-----------------------------------------------------------------------------
unsigned int
cClientPingManager::Get_Avg_Round_Trip_Ping_Ms
(
	void
)
{
	return AvgRoundTripPingMs;
}

//-----------------------------------------------------------------------------
void
cClientPingManager::Compute_Average_Round_Trip_Ping_Ms
(
	void
)
{
	//
	// This is the average of MAX_SAMPLES non-zero pings.
	//

	AvgRoundTripPingMs = 0;

	unsigned int num_pings = 0;
	unsigned int total_ping = 0;

	for (int i = 0; i < MAX_SAMPLES; i++)
	{
		if (RoundTripPingSamplesMs[i] != 0)
		{
			num_pings++;
			total_ping += RoundTripPingSamplesMs[i];
		}
	}

	if (num_pings > 0)
	{
		AvgRoundTripPingMs = (unsigned int)(total_ping / (float) num_pings);
	}
}

//-----------------------------------------------------------------------------
void
cClientPingManager::Response_Received
(
	int ping_number
)
{
	if (ping_number == PingNumber)
	{
		LastRoundTripPingMs = TIMEGETTIME() - TimeSentMs;
		RoundTripPingSamplesMs[PingNumber % MAX_SAMPLES] = LastRoundTripPingMs;
		Compute_Average_Round_Trip_Ping_Ms();
		IsAwaitingResponse = false;
	}
	else
	{
		WWDEBUG_SAY(("WARNING: cClientPingManager::Response_Received ping number mismatch (%d, %d)\n",
			ping_number, PingNumber));
	}
}