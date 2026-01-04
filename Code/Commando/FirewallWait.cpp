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
*     $Archive: /Commando/Code/Commando/FirewallWait.cpp $
*
* DESCRIPTION
*     Firewall negotiation wait condition.
*
* PROGRAMMER
*     $Author: Tom_s $
*
* VERSION INFO
*     $Revision: 16 $
*     $Modtime: 3/04/02 11:45a $
*
******************************************************************************/

#include "always.h"
#include "FirewallWait.h"
#include "nat.h"
#include	"string_ids.h"
#include "translatedb.h"
#include "natter.h"
#include <WWOnline/WOLSession.h>
#include <wwdebug/wwdebug.h>
#include "systimer.h"
#include <algorithm>

/*
** Wait code for firewall/NAT detection.
**
**
**
*/

RefPtr<FirewallDetectWait> FirewallDetectWait::Create(void)
	{
	return new FirewallDetectWait();
	}


FirewallDetectWait::FirewallDetectWait(void) :
		SingleWait(TRANSLATION(IDS_FIREWALL_NEGOTIATING_FIREWALL), 60000),
		mEvent(NULL),
		mPingsRemaining(UINT_MAX)
	{
	mWOLSession = WWOnline::Session::GetInstance(false);
	assert(mWOLSession.IsValid());
	}


FirewallDetectWait::~FirewallDetectWait()
	{
	WWDEBUG_SAY(("FirewallDetectWait: End - %S\n", mEndText));

	mWOLSession->EnablePinging(true);

	if (mEvent)
		{
		CloseHandle(mEvent);
		}
	}


void FirewallDetectWait::WaitBeginning(void)
	{
	WWDEBUG_SAY(("FirewallDetectWait: Beginning\n"));

	mEvent = CreateEventA(NULL, true, false, NULL);

	if (mEvent == NULL)
		{
		WWDEBUG_SAY(("FirewallDetectWait: Can't create event\n"));
		EndWait(Error, TRANSLATION(IDS_FIREWALL_CREATE_EVENT_FAILED));
		}
	else
		{
		mWOLSession->EnablePinging(false);
		mTimeout = 15000;
		}
	}


WaitCondition::WaitResult FirewallDetectWait::GetResult(void)
	{
	if (mEndResult == Waiting)
		{
		// Wait for pending pings to finish first
		unsigned int pingsWaiting = mWOLSession->GetPendingPingCount();

		if (mPingsRemaining != pingsWaiting)
			{
			mPingsRemaining = pingsWaiting;
			mTimeout = 60000;
			FirewallHelper.Detect_Firewall(mEvent);
			}

		if (mPingsRemaining == 0)
			{
			DWORD result = WaitForSingleObject(mEvent, 0);

			if (result == WAIT_OBJECT_0)
				{
				WWDEBUG_SAY(("FirewallDetectWait: ConditionMet\n"));
				WOLNATInterface.Save_Firewall_Info_To_Registry();
				EndWait(ConditionMet, TRANSLATION(IDS_FIREWALL_NEGOTIATION_COMPLETE));
				}
			else if (result == WAIT_FAILED)
				{
				WWDEBUG_SAY(("FirewallDetectWait: WAIT_FAILED\n"));
				EndWait(Error, TRANSLATION(IDS_FIREWALL_NEGOTIATION_FAILED));
				}
			}
		}

	if (mEndResult != Waiting)
		{
		mWOLSession->EnablePinging(true);
		}

	return mEndResult;
	}


/*
** Wait code for clients when trying to open up a firewall for a server connection.
**
*/
RefPtr<FirewallConnectWait> FirewallConnectWait::Create(void)
	{
	return new FirewallConnectWait;
	}


FirewallConnectWait::FirewallConnectWait(void) :
		SingleWait(TRANSLATION(IDS_FIREWALL_NEGOTIATING_WITH_SERVER)),
		mEvent(NULL),
		mCancelEvent(NULL),
		mSuccessFlag(FirewallHelperClass::FW_RESULT_UNKNOWN),
		mQueueCount(0),
		mLastQueueCount(0),
		mPingsRemaining(UINT_MAX)
	{
	mWOLSession = WWOnline::Session::GetInstance(false);
	assert(mWOLSession.IsValid());
	}


FirewallConnectWait::~FirewallConnectWait()
	{
	WWDEBUG_SAY(("FirewallConnectWait: End - %S\n", mEndText));

	mWOLSession->EnablePinging(true);

	if (mEvent)
		{
		CloseHandle(mEvent);
		}

	if (mCancelEvent)
		{
		CloseHandle(mCancelEvent);
		}
	}


void FirewallConnectWait::WaitBeginning(void)
	{
	WWDEBUG_SAY(("FirewallConnectWait: Beginning\n"));

	mEvent = CreateEventA(NULL, true, false, NULL);
	mCancelEvent = CreateEventA(NULL, true, false, NULL);

	if (mEvent == NULL)
		{
		WWDEBUG_SAY(("FirewallConnectWait: Can't create event\n"));
		EndWait(Error, TRANSLATION(IDS_FIREWALL_CREATE_EVENT_FAILED));
		}
	else
		{
		mWOLSession->EnablePinging(false);
		WOLNATInterface.Tell_Server_That_Client_Is_In_Channel();
		mTimeout = 15000;
		mStartTime = TIMEGETTIME();
		}
	}


WaitCondition::WaitResult FirewallConnectWait::GetResult(void)
	{
	if (mEndResult == Waiting)
		{
		// Wait for pending pings to finish first
		unsigned int pingsWaiting = mWOLSession->GetPendingPingCount();

		if (mPingsRemaining != pingsWaiting)
			{
			mPingsRemaining = pingsWaiting;

			if (mPingsRemaining == 0)
				{
				FirewallHelper.Set_Client_Connect_Event(mEvent, mCancelEvent, &mSuccessFlag, (int*)&mQueueCount);
				mTimeout = 32000;
				mStartTime = TIMEGETTIME();
				}
			}

		if (mPingsRemaining == 0)
			{
			if ((TIMEGETTIME() - mStartTime) > mTimeout)
				{
				EndWait(TimeOut, TRANSLATION(IDS_FIREWALL_PORT_NEGOTIATION_TIMEOUT));
				}
			else
				{
				// Maybe change the wait text if there are players queued in front of us.
				if (mQueueCount != mLastQueueCount)
					{
					unichar_t temp[256];
					u_snprintf_u(temp, sizeof(temp), TRANSLATION(IDS_FIREWALL_QUEUE_NOTIFICATION), mQueueCount);
					WideStringClass text(temp, true);
					SetWaitText(text);
					mLastQueueCount = mQueueCount;
					mTimeout = std::max((unsigned)32000, ((mQueueCount * 32000) + 32000));
					mStartTime = TIMEGETTIME();
					}

				DWORD result = WaitForSingleObject(mEvent, 0);

				if (result == WAIT_OBJECT_0)
					{
					if (mSuccessFlag == FirewallHelperClass::FW_RESULT_SUCCEEDED)
						{
						WWDEBUG_SAY(("FirewallConnectWait: ConditionMet\n"));
						EndWait(ConditionMet, TRANSLATION(IDS_FIREWALL_PORT_NEGOTIATION_COMPLETE));
						}
					else
						{
						assert(mSuccessFlag == FirewallHelperClass::FW_RESULT_FAILED);
						WWDEBUG_SAY(("FirewallConnectWait: ConditionMet\n"));
						EndWait(Error, TRANSLATION(IDS_FIREWALL_PORT_NEGOTIATION_FAILED));
						}
					}
				else if (result == WAIT_FAILED)
					{
					WWDEBUG_SAY(("FirewallConnectWait: WAIT_FAILED\n"));
					EndWait(Error, TRANSLATION(IDS_FIREWALL_PORT_NEGOTIATION_FAILED));
					}
				}
			}
		}

	if (mEndResult != Waiting)
		{
		mWOLSession->EnablePinging(true);
		}

	return mEndResult;
	}



//
// Override base class end wait to check for cancel being pressed.
//
void FirewallConnectWait::EndWait(WaitResult result, const unichar_t* endText)
	{
	WWDEBUG_SAY(("FirewallConnectWait: EndWait\n"));

	if (result == UserCancel || result == TimeOut)
		{
		// Tell the firewall negotiation code to give up.
		SetEvent(mCancelEvent);
		}

		// Give the firewall code a little time to respond then remove it's cancel event anyway. It'll figure it out.
		for (int i=0 ; i<100 ; i++)
			{
			if (mSuccessFlag == FirewallHelperClass::FW_RESULT_CANCELLED)
				{
				break;
				}

			Sleep(1);
			}

	FirewallHelper.Set_Client_Connect_Event(NULL, NULL, NULL, NULL);

	SingleWait::EndWait(result, endText);
	}