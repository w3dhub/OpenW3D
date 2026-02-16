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
*     $Archive: /Commando/Code/Commando/WOLQuickMatch.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 18 $
*     $Modtime: 2/20/02 5:07p $
*
******************************************************************************/

#ifndef __WOLQUICKMATCH_H__
#define __WOLQUICKMATCH_H__

#include <wwlib/refcount.h>
#include <WWOnline/RefPtr.h>
#include <WWOnline/WOLSession.h>
#include <wwlib/Notify.h>

class WaitCondition;
class DlgWOLWaitEvent;

class QuickMatchEvent :
		public TypedEvent<QuickMatchEvent, const WideStringClass>
	{
	public:
		enum Event {QMERROR = 0, QMINFO, QMMSG, QMMATCHED, QMUNKNOWN};

		Event GetEvent(void) const
			{return mEvent;}

		QuickMatchEvent(Event event, const WideStringClass& msg) :
			TypedEvent<QuickMatchEvent, const WideStringClass>(msg),
					mEvent(event)
			{}

		~QuickMatchEvent()
			{}

	private:
		Event mEvent;
	};


class WOLQuickMatch :
		public RefCountClass,
		public Notifier<QuickMatchEvent>,
		public Observer<WWOnline::ServerError>,
		public Observer<WWOnline::ChatMessage>
	{
	public:
		static WOLQuickMatch* Create(void);

		RefPtr<WaitCondition> ConnectClient(void);
		RefPtr<WaitCondition> Disconnect(void);

		bool SendClientInfo(void);
		void SendServerInfo(const char* exInfo, const char* topic);
			
		DECLARE_NOTIFIER(QuickMatchEvent)

	protected:
		WOLQuickMatch();
		~WOLQuickMatch();

		bool FinalizeCreate(void);
		
		void SendStatus(const unichar_t* statusMsg);

		void ParseResponse(const unichar_t* message);

		void HandleNotification(WWOnline::ServerError&) override;
		void HandleNotification(WWOnline::ChatMessage&) override;

	private:
		// Prevent copy and assignment
		WOLQuickMatch(const WOLQuickMatch&);
		const WOLQuickMatch& operator=(const WOLQuickMatch&);

		static void ProcessInfo(WOLQuickMatch*, const unichar_t*);
		static void ProcessError(WOLQuickMatch*, const unichar_t*);
		static void ProcessStart(WOLQuickMatch*, const unichar_t*);
		static void ProcessUnknown(WOLQuickMatch*, const unichar_t*);

	protected:
		RefPtr<WWOnline::Session> mWOLSession;
	};

#endif // __WOLQUICKMATCH_H__
