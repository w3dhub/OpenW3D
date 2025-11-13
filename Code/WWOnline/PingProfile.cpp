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

/******************************************************************************
*
* FILE
*     $Archive: /Commando/Code/WWOnline/PingProfile.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 8 $
*     $Modtime: 1/11/02 5:45p $
*
******************************************************************************/

#include "always.h"
#include "PingProfile.h"
#include <WWOnline/WOLSession.h>
#include <WWOnline/WOLServer.h>
#include <WWOnline/WOLString.h>
#include <wwlib/widestring.h>
#include <algorithm>
#include <wwdebug/wwdebug.h>
#include <limits.h>
#include <math.h>
#include <cstdio>
#include <algorithm>

// NOTE: Test to force CRLF
using namespace WWOnline;

static PingProfile gPingProfile;

/******************************************************************************
*
* NAME
*     RecalculatePingProfile
*
* DESCRIPTION
*     Calculate ping profile based on the current session ping times.
*
* INPUTS
*     Session - WWOnline session
*
* RESULT
*     True if ping times are valid
*
******************************************************************************/

bool RecalculatePingProfile(const RefPtr<Session>& session)
	{
	memset(gPingProfile.Pings, 0xFF, sizeof(gPingProfile.Pings));

	if (session.IsValid())
		{
		const PingServerList& pingers = session->GetPingServerList();

		// The ping profile holds up to eight pings.
		const size_t count = std::min(static_cast<size_t>(8), pingers.size());

		// If there aren't any ping servers then fail.
		if (count > 0)
			{
			for (size_t index = 0; index < count; index++)
				{
				int pingTime = pingers[index]->GetPingTime();

				// If a ping time is invalid then fail.
				if (pingTime < 0)
					{
					return false;
					}

				// Clamp pings time to a maximum of 1000 ms
				pingTime = std::min<int>(pingTime, 1000);

				// Scale ping time to from 0-1000 to 0-255
				gPingProfile.Pings[index] = (((unsigned long)pingTime * 255) / 1000);
				}

			return true;
			}
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     GetLocalPingProfile
*
* DESCRIPTION
*     Get the ping profile for this client.
*
* INPUTS
*     NONE
*
* RESULT
*     Profile - Ping profile to for current session.
*
******************************************************************************/

const PingProfile& GetLocalPingProfile(void)
	{
	return gPingProfile;
	}


/******************************************************************************
*
* NAME
*     ComparePingProfile
*
* DESCRIPTION
*     Calcualte the best time between two ping profiles.
*
* INPUTS
*     Profile1 - Ping profile
*     Profile2 - Ping profile
*
* RESULT
*     Weight - Smallest time between two profiles.
*
******************************************************************************/

long ComparePingProfile(const PingProfile& ping1, const PingProfile& ping2)
	{
	long minWeight = LONG_MAX;

	for (int index = 0; index < 8; index++)
		{
		long a = ping1.Pings[index];
		long b = ping2.Pings[index];
		long weight = (a * a) + (b * b);

		minWeight = std::min<long>(weight, minWeight);
		}

	return minWeight;
	}


/******************************************************************************
*
* NAME
*     EncodePingProfile
*
* DESCRIPTION
*     Encode the ping profile into a clear text string that can be transmited
*     via chat.
*
* INPUTS
*     Pings  - Ping profile to encode.
*     Buffer - Buffer to encode pings into.
*
* RESULT
*     Length - Number of characters used to encode the profile.
*
******************************************************************************/

int EncodePingProfile(const PingProfile& pings, char* buffer)
	{
	assert(buffer != NULL);

	if (buffer == NULL)
		{
		return 0;
		}

	char temp[18];
	int count = sprintf(temp, "%02X%02X%02X%02X%02X%02X%02X%02X",
			pings.Pings[0], pings.Pings[1], pings.Pings[2], pings.Pings[3],
			pings.Pings[4], pings.Pings[5], pings.Pings[6], pings.Pings[7]);

	strcat(buffer, temp);

	return count;
	}


/******************************************************************************
*
* NAME
*     DecodePingProfile
*
* DESCRIPTION
*     Decode the ping profile from clear text string.
*
* INPUTS
*     String - Encode ping profile.
*     Pings  - Ping profile structure.
*
* RESULT
*     NONE
*
******************************************************************************/

void DecodePingProfile(const char* buffer, PingProfile& pings)
	{
	if (buffer)
		{
		unsigned long ping[8];

		sscanf(buffer, "%02X%02X%02X%02X%02X%02X%02X%02X",
			&ping[0], &ping[1], &ping[2], &ping[3], &ping[4], &ping[5], &ping[6], &ping[7]);

		pings.Pings[0] = ping[0];
		pings.Pings[1] = ping[1];
		pings.Pings[2] = ping[2];
		pings.Pings[3] = ping[3];
		pings.Pings[4] = ping[4];
		pings.Pings[5] = ping[5];
		pings.Pings[6] = ping[6];
		pings.Pings[7] = ping[7];
		}
	else
		{
		memset(&pings, 0xFF, sizeof(pings));
		}
	}


/******************************************************************************
*
* NAME
*     PingProfileWait::Create
*
* DESCRIPTION
*     Create a wait condition to obtain an update ping profile.
*
* INPUTS
*     Session - WWOnline session
*
* RESULT
*     Wait - Wait condition to process.
*
******************************************************************************/

RefPtr<PingProfileWait> PingProfileWait::Create(void)
	{
	return new PingProfileWait;
	}


/******************************************************************************
*
* NAME
*     PingProfileWait::PingProfileWait
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*
* RESULT
*
******************************************************************************/

PingProfileWait::PingProfileWait() :
		SingleWait(WOLSTRING("WOL_PINGPROFILEREQUEST")),
		mCount(0)
	{
	WWDEBUG_SAY(("WOL: PingProfileWait Instantiated\n"));
	mWOLSession = Session::GetInstance(false);
	}


/******************************************************************************
*
* NAME
*     PingProfileWait::~PingProfileWait
*
* DESCRIPTION
*     Destructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

PingProfileWait::~PingProfileWait()
	{
	WWDEBUG_SAY(("WOL: PingProfileWait Destroyed\n"));
	}


/******************************************************************************
*
* NAME
*     PingProfileWait::WaitBeginning
*
* DESCRIPTION
*     Begin wait condition
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void PingProfileWait::WaitBeginning(void)
	{
	WWDEBUG_SAY(("WOL: PingProfileWait Beginning\n"));

	if (mWOLSession.IsValid() == false)
		{
		EndWait(Error, WOLSTRING("WOL_NOTINITIALIZED"));
		return;
		}

		const PingServerList& pingers = mWOLSession->GetPingServerList();

		// Handle up to eight servers
		const size_t count = std::min(static_cast<size_t>(8), pingers.size());
		mCount = static_cast<unsigned int>(count);

	if (mCount == 0)
		{
		EndWait(Error, WOLSTRING("WOL_NOPINGSERVER"));
		return;
		}

	Observer<RawPing>::NotifyMe(*mWOLSession);

	bool pingsValid = true;

	for (unsigned int index = 0; index < mCount; index++)
		{
		int pingTime = pingers[index]->GetPingTime();

		// If a ping time is invalid then request it.
		if (pingTime == -1)
			{
			pingsValid = false;

			const char* address = pingers[index]->GetHostAddress();
			mWOLSession->RequestPing(address);
			}
		}

	if (pingsValid)
		{
		RecalculatePingProfile(mWOLSession);
		EndWait(ConditionMet, WOLSTRING("WOL_PINGPROFILERECEIVED"));
		}
	}


/******************************************************************************
*
* NAME
*     PingProfileWait::HandleNotification(RawPing)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void PingProfileWait::HandleNotification(RawPing& ping)
	{
	if ((mEndResult == Waiting) && (mCount > 0))
		{
		const char* address = ping.GetHostAddress();

		const PingServerList& pingers = mWOLSession->GetPingServerList();

		for (unsigned int index = 0; index < pingers.size(); index++)
			{
			const char* pinger = pingers[index]->GetHostAddress();

			if (stricmp(address, pinger) == 0)
				{
				WWDEBUG_SAY(("WOL: PingProfileWait received ping for '%s'\n", pingers[index]->GetData().name));
				mCount--;
				break;
				}
			}

		if (mCount == 0)
			{
			RecalculatePingProfile(mWOLSession);
			EndWait(ConditionMet, WOLSTRING("WOL_PINGPROFILERECEIVED"));
			}
		}
	}
