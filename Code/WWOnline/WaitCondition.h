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
*     $Archive: /Commando/Code/WWOnline/WaitCondition.h $
*
* DESCRIPTION
*     This class encapsulates a wait condition.
*
*     To wait on an event, create a wait condition. Pass the condition, or a
*     set of conditions, to WaitingCondition::Wait_For(). This will block
*     until the conditions are met, the user cancels the wait, or the timeout
*     is reached. This will also cause the wait popup to be displayed.
*
* PROGRAMMER
*     Steven Clinard
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Modtime: 1/18/02 3:08p $
*     $Revision: 9 $
*
******************************************************************************/

#ifndef __WAITCONDITION_H__
#define __WAITCONDITION_H__

#include "RefCounted.h"
#include "RefPtr.h"
#include <wwlib/Notify.h>
#include <wwlib/CallbackHook.h>
#include <wwlib/widestring.h>
#include <vector>

class CallbackHook;

class WaitCondition :
		public RefCounted
	{
	public:
		enum WaitResult {Waiting, ConditionMet, UserCancel, TimeOut, Error};

		// Wait for this condition
		virtual WaitResult WaitFor(CallbackHook& hook, unsigned int timeout = 30000);

		/* Wait_Beginning is called when the wait actually begins.
	   * This useful if the activity being monitored should only begin after the
		 * wait has started.
		 *
		 * Normally, the request should begin as soon as possible and the condition
		 * may be met before the wait begins; this avoids the whole wait popup in
		 * the first place.
		 *
		 * Serially chained waits may require prerequisite conditions to be met
		 * before the request that initiates this wait. In this case, a derived
		 * wait class could override Wait_Beginning to make the request.
		 */
		virtual void WaitBeginning(void) = 0;

		// Get the result of the wait in progress.
		virtual WaitResult GetResult(void) = 0;

		// End the wait.
		virtual void EndWait(WaitResult, const unichar_t*) = 0;

		// Get a text description of completed wait
		virtual const unichar_t* GetResultText(void) const = 0;

		// Get a text description of the wait in progress.
		virtual const unichar_t* GetWaitText(void) const = 0;

		// Get the timeout in milliseconds for this wait condition
		virtual unsigned int GetTimeout(void) const = 0;

	protected:
		WaitCondition();
		virtual ~WaitCondition();

		WaitCondition(const WaitCondition&);
		const WaitCondition& operator=(const WaitCondition&);
	};


// Waits for a single event.
class SingleWait :
		public WaitCondition
	{
	public:
		static RefPtr<SingleWait> Create(const unichar_t* waitText, unsigned int timeout = 30000);

		virtual void WaitBeginning(void) override;
		virtual WaitResult GetResult(void) override;
		
		virtual void EndWait(WaitResult, const unichar_t*) override;

		virtual const unichar_t* GetResultText(void) const override;
		
		virtual const unichar_t* GetWaitText(void) const override;
		
		virtual void SetWaitText(const unichar_t* waitText);
	
		virtual unsigned int GetTimeout(void) const override;

	protected:
		SingleWait(const unichar_t* waitText, unsigned int timeout = 30000);
		virtual ~SingleWait();

		SingleWait(const SingleWait&);
		const SingleWait& operator=(const SingleWait&);

		WaitResult mEndResult;
		WideStringClass mEndText;
		WideStringClass mWaitText;

		unsigned int mTimeout;
		unsigned int mStartTime;
	};


// Waits for multiple events.
// All events must be satisfied and must occur in the order in which they are added.
class SerialWait :
		public WaitCondition
	{
	public:
		static RefPtr<SerialWait> Create(void);

		void Add(const RefPtr<WaitCondition>&);

		int RemainingWaits(void) const;

		virtual void WaitBeginning(void) override;
		
		virtual WaitResult GetResult(void) override;
		
		virtual void EndWait(WaitResult, const unichar_t*) override;

		virtual const unichar_t* GetResultText(void) const override;

		virtual const unichar_t* GetWaitText(void) const override;

		virtual unsigned int GetTimeout(void) const override;

	protected:
		SerialWait();
		virtual ~SerialWait();

		SerialWait(const SerialWait&);
		const SerialWait& operator=(const SerialWait&);

		std::vector< RefPtr<WaitCondition> > mWaits;

		mutable int mCurrentWait;
		mutable WaitResult mEndResult;
		mutable WideStringClass mEndText;

		unsigned int mMaxTimeout;
		unsigned int mStartTime;
	};


// Waits for multiple events.
// All events are started at the same time. All events must be satisfied,
// but may occur in any order.
class ANDWait :
		public WaitCondition
	{
	public:
		static RefPtr<ANDWait> Create(const unichar_t*);

		void Add(const RefPtr<WaitCondition>&);

		virtual void WaitBeginning(void) override;

		virtual WaitResult GetResult(void) override;
		
		virtual void EndWait(WaitResult, const unichar_t*) override;

		virtual const unichar_t* GetResultText(void) const override;
		
		virtual const unichar_t* GetWaitText(void) const override;

		virtual unsigned int GetTimeout(void) const override;

	protected:
		ANDWait(const unichar_t*);
		virtual ~ANDWait();

		ANDWait(const ANDWait&);
		const ANDWait& operator=(const ANDWait&);

		std::vector< RefPtr<WaitCondition> > mWaits;

		mutable WaitResult mEndResult;
		mutable WideStringClass mEndText;
		WideStringClass mWaitText;

		unsigned int mMaxTimeout;
		unsigned int mStartTime;
	};


// ORWait: Waits for multiple events.
// All events are started at the same time. Wait ends when any event is satisfied.
class ORWait :
		public WaitCondition
	{
	public:
		static RefPtr<ORWait> Create(const unichar_t*);

		void Add(const RefPtr<WaitCondition>&);

		virtual void WaitBeginning(void) override;

		virtual WaitResult GetResult(void) override;
		
		virtual void EndWait(WaitResult, const unichar_t*) override;

		virtual const unichar_t* GetResultText(void) const override;
		
		virtual const unichar_t* GetWaitText(void) const override;

		virtual unsigned int GetTimeout(void) const override
			{return mMaxTimeout;}

	protected:
		ORWait(const unichar_t*);
		virtual ~ORWait();

		ORWait(const ORWait&);
		const ORWait& operator=(const ORWait&);

		std::vector< RefPtr<WaitCondition> > mWaits;

		mutable WaitResult mEndResult;
		mutable WideStringClass mEndText;
		WideStringClass mWaitText;

		unsigned int mMaxTimeout;
		unsigned int mStartTime;
	};


// Wait for an event from a notifier
template<typename Event> class EventWait :
		public SingleWait,
		public Observer<Event>
	{
	public:
		static RefPtr< EventWait<Event> > Create(const unichar_t* waitText)
			{new EventWait(waitText);}

		static RefPtr< EventWait<Event> > CreateAndObserve(Notifier<Event>& notifier,
				const unichar_t* waitText)
			{
			EventWait<Event>* wait = new EventWait(waitText);
			notifier.AddObserver(*wait);
			return wait;
			}

		virtual void HandleNotification(Event&)
			{if (mEndResult == Waiting) {this->EndWait(ConditionMet, U_CHAR(""));}}

	protected:
		EventWait(const unichar_t* waitText) :
				SingleWait(waitText)
			{}

		EventWait(const EventWait<Event>&);
		const EventWait<Event>& operator=(const EventWait<Event>&);
	};


// Wait for an event with a particular value from a notifier
template<typename Event> class EventValueWait :
		public SingleWait,
		public Observer<Event>
	{
	public:
		static RefPtr< EventValueWait<Event> > Create(const Event& value, const unichar_t* waitText)
			{new EventValueWait(value, waitText);}

		static RefPtr< EventValueWait<Event> > CreateAndObserve(Notifier<Event>& notifier,
				const Event value, const unichar_t* waitText)
			{
			EventValueWait<Event>* wait = new EventValueWait(value, waitText);
			notifier.AddObserver(*wait);
			return wait;
			}

		virtual void HandleNotification(Event& event) override
			{
			if (mEndResult == Waiting)
				{
				if (event == mMatchValue)
					{
					EndWait(ConditionMet, U_CHAR(""));
					}
				else
					{
					EndWait(mEndResult, U_CHAR(""));
					}
				}
			}

	protected:
		EventValueWait(const Event& value, const unichar_t* waitText) :
				SingleWait(waitText),
			  mMatchValue(value)
			{}

		EventValueWait(const EventValueWait<Event>&);
		const EventValueWait& operator=(const EventValueWait&);

		Event mMatchValue;
	};

#endif // __WAITCONDITION_H__
