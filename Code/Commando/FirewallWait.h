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
*     $Archive: /Commando/Code/Commando/FirewallWait.h $
*
* DESCRIPTION
*     Firewall negotiation wait condition.
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 6 $
*     $Modtime: 8/28/01 3:22p $
*
******************************************************************************/

#ifndef __FIREWALLWAIT_H__
#define __FIREWALLWAIT_H__

#include <WWOnline/WaitCondition.h>
#include <windows.h>

namespace WWOnline
{
class Session;
}

/*
** Wait code for firewall/NAT detection.
**
**
**
*/

class FirewallDetectWait :
		public SingleWait
	{
	public:
		static RefPtr<FirewallDetectWait> Create(void);

		void WaitBeginning(void) override;
		WaitResult GetResult(void) override;

	protected:
		FirewallDetectWait();
		virtual ~FirewallDetectWait();

		FirewallDetectWait(const FirewallDetectWait&);
		const FirewallDetectWait& operator=(const FirewallDetectWait&);

		RefPtr<WWOnline::Session> mWOLSession;
		unsigned int mPingsRemaining;

		HANDLE mEvent;
	};




/*
** Wait class for clients when trying to open up a firewall for a server connection.
**
*/
class FirewallConnectWait :
		public SingleWait
	{
	public:
		static RefPtr<FirewallConnectWait> Create(void);

		void WaitBeginning(void) override;
		WaitResult GetResult(void) override;
		virtual void EndWait(WaitResult, const unichar_t* endText) override;

	protected:
		FirewallConnectWait();
		virtual ~FirewallConnectWait();

		FirewallConnectWait(const FirewallConnectWait&);
		const FirewallConnectWait& operator=(const FirewallConnectWait&);

		RefPtr<WWOnline::Session> mWOLSession;
		unsigned int mPingsRemaining;

		HANDLE mEvent;
		HANDLE mCancelEvent;

		/*
		** Did the port negotiation succeed?
		*/
		int mSuccessFlag;

		/*
		** How many players in the queue ahead of us?
		*/
		unsigned int mQueueCount;
		unsigned int mLastQueueCount;
		unsigned int mStartTime;
	};

#endif // __FIREWALLWAIT_H__
