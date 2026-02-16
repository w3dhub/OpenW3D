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
*     $Archive: /Commando/Code/WWOnline/WaitCondition.cpp $
*
* DESCRIPTION
*     This class encapsulates a wait condition.
*
*     To wait on an event, create a wait condition. Pass the condition, or a
*     set of conditions, to WaitingCondition::Wait_For(). This will block
*     until the conditions are met, the user cancels the wait, or the timeout
*     is reached.
*
* PROGRAMMER
*     Steven Clinard
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Modtime: 1/11/02 6:20p $
*     $Revision: 13 $
*
******************************************************************************/

#include "always.h"

#include "WaitCondition.h"
#include "WOLString.h"
#include <wwlib/win.h>

#include "systimer.h"

/******************************************************************************
*
* NAME
*     WaitCondition::WaitCondition
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

WaitCondition::WaitCondition()
	{
	}


/******************************************************************************
*
* NAME
*     WaitCondition::~WaitCondition
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

WaitCondition::~WaitCondition()
	{
	}


/******************************************************************************
*
* NAME
*     WaitCondition::WaitFor
*
* DESCRIPTION
*     Wait for condition to be satisfied.
*
* INPUTS
*     Callback to invoke while waiting.
*     Timeout value in milliseconds
*
* RESULT
*     Wait result
*
******************************************************************************/

WaitCondition::WaitResult WaitCondition::WaitFor(CallbackHook& hook, unsigned int timeout)
	{
	WaitBeginning();

	DWORD startTime = TIMEGETTIME();

	while (GetResult() == Waiting)
		{
		// Yeild time to client callback
		if (hook.DoCallback())
			{
			EndWait(UserCancel, WOLSTRING("WOL_CANCELED"));
			break;
			}

		// Watch for timeout
		if ((TIMEGETTIME() - startTime) > timeout)
			{
			EndWait(TimeOut, WOLSTRING("WOL_TIMEDOUT"));
			}
		}

	return GetResult();
	}


/******************************************************************************
*
* NAME
*     SingleWait::Create
*
* DESCRIPTION
*     Create a single wait instance
*
* INPUTS
*     Text    - Text description of wait condition.
*     Timeout - Time to allow for wait condition to complete. If the wait
*               condition exceeds this time then it should be considered an
*               error.
*
* RESULT
*     SingleWait - New instance to a single wait condition
*
******************************************************************************/

RefPtr<SingleWait> SingleWait::Create(const unichar_t* text, unsigned int timeout)
	{
	return new SingleWait(text, timeout);
	}


/******************************************************************************
*
* NAME
*     SingleWait::SingleWait
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     Text    - Text description of wait condition.
*     Timeout - Time to allow for wait condition to complete. If the wait
*               condition exceeds this time then it should be considered an
*               error.
*
* RESULT
*     NONE
*
******************************************************************************/

SingleWait::SingleWait(const unichar_t* waitText, unsigned int timeout) :
	  mWaitText(waitText),
	  mEndResult(Waiting),
		mTimeout(timeout)
	{
	}


/******************************************************************************
*
* NAME
*     SingleWait::~SingleWait
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

SingleWait::~SingleWait()
	{
	}


/******************************************************************************
*
* NAME
*     SingleWait::WaitBeginning
*
* DESCRIPTION
*     Begin this wait condition.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void SingleWait::WaitBeginning(void)
	{
	}


/******************************************************************************
*
* NAME
*     SingleWait::GetResult
*
* DESCRIPTION
*     Get the result status of this wait condition.
*
* INPUTS
*     NONE
*
* RESULT
*     Result -
*
******************************************************************************/

WaitCondition::WaitResult SingleWait::GetResult(void)
	{
	return mEndResult;
	}


/******************************************************************************
*
* NAME
*     SingleWait::GetResultText
*
* DESCRIPTION
*     Get text description of wait result.
*
* INPUTS
*     NONE
*
* RESULT
*     Text - Text description of the result of the wait condition.
*
******************************************************************************/

const unichar_t* SingleWait::GetResultText(void) const
	{
	return mEndText;
	}


/******************************************************************************
*
* NAME
*     SingleWait::EndWait
*
* DESCRIPTION
*     Terminate the wait condition.
*
* INPUTS
*     Result - Reason for ending the wait.
*     Text   - Text description of ending
*
* RESULT
*     NONE
*
******************************************************************************/

void SingleWait::EndWait(WaitResult result, const unichar_t* endText)
	{
	mEndResult = result;
	mEndText = endText;
	}


/******************************************************************************
*
* NAME
*     SingleWait::GetWaitText
*
* DESCRIPTION
*     Get wait description text
*
* INPUTS
*     NONE
*
* RESULT
*     Text - Text description of the wait operation.
*
******************************************************************************/

const unichar_t* SingleWait::GetWaitText(void) const
	{
	return mWaitText;
	}


/******************************************************************************
*
* NAME
*     SingleWait::SetWaitText
*
* DESCRIPTION
*     Changes the wait text in the dialog.
*
* INPUTS
*     New text to display
*
* RESULT
*     NONE
*
******************************************************************************/

void SingleWait::SetWaitText(const unichar_t* waitText)
	{
	mWaitText = waitText;
	}


/******************************************************************************
*
* NAME
*     SingleWait::GetTimeout
*
* DESCRIPTION
*     Get the time allowed for this wait condition to complete.
*
* INPUTS
*     NONE
*
* RESULT
*     Timeout - Timeout time in milliseconds.
*
******************************************************************************/

unsigned int SingleWait::GetTimeout(void) const
	{
	return mTimeout;
	}


/******************************************************************************
*
* NAME
*     SerialWait::Create
*
* DESCRIPTION
*     Create an instance of a serialized wait. SerialWait is primarily used
*     to hold other wait conditions that need to be executed in sequence.
*
* INPUTS
*     NONE
*
* RESULT
*     Wait - New wait condition
*
******************************************************************************/

RefPtr<SerialWait> SerialWait::Create(void)
	{
	return new SerialWait;
	}


/******************************************************************************
*
* NAME
*     SerialWait::SerialWait
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

SerialWait::SerialWait() :
	  mCurrentWait(-1),
	  mEndResult(Waiting),
		mStartTime(0)
	{
	}


/******************************************************************************
*
* NAME
*     SerialWait::~SerialWait
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

SerialWait::~SerialWait()
	{
	}


/******************************************************************************
*
* NAME
*     SerialWait::Add
*
* DESCRIPTION
*     Add a wait condition. Wait conditions are processed in the order they
*     are added.
*
* INPUTS
*     Wait - Wait condition to add.
*
* RESULT
*     NONE
*
******************************************************************************/

void SerialWait::Add(const RefPtr<WaitCondition>& wait)
	{
	if (wait.IsValid())
		{
		mWaits.push_back(wait);
		}
	else
		{
		assert(wait.IsValid() && "SerialWait: Invalid wait condition");
		EndWait(Error, U_CHAR("Invalid wait condition."));
		}
	}


/******************************************************************************
*
* NAME
*     SerialWait::RemainingWaits
*
* DESCRIPTION
*     Get then number of remaining wait conditions.
*
* INPUTS
*     NONE
*
* RESULT
*     Number of waits
*
******************************************************************************/

int SerialWait::RemainingWaits(void) const
{
	const int count = static_cast<int>(mWaits.size());

	return (mCurrentWait == -1) ? count :
		(mCurrentWait >= count) ? 0 : (count - mCurrentWait);
	}


/******************************************************************************
*
* NAME
*     SerialWait::WaitBeginning
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

void SerialWait::WaitBeginning(void)
	{
	// WaitBeginning should only be called once.
	if (mCurrentWait != -1)
		{
		assert(mCurrentWait == -1);
		return;
		}

	// Start the first wait condition in the queue.
	mCurrentWait = 0;

	if (mWaits.size() > 0)
		{
		RefPtr<WaitCondition> wait = mWaits[0];
		wait->WaitBeginning();
		}

	mStartTime = TIMEGETTIME();
	}


/******************************************************************************
*
* NAME
*     SerialWait::GetResult
*
* DESCRIPTION
*     Get the current wait status result.
*
* INPUTS
*     NONE
*
* RESULT
*     Result -
*
******************************************************************************/

WaitCondition::WaitResult SerialWait::GetResult(void)
	{
	// If we are not waiting the we must be done.
	if (mEndResult != Waiting)
		{
		return mEndResult;
		}

	// If there isn't a current wait processing then we must be waiting to begin.
	if (mCurrentWait == -1)
		{
		return Waiting;
		}

	// If all of the wait conditions have completed successfully then the wait
	// condition has been met.
	if ((unsigned)mCurrentWait >= mWaits.size())
		{
		return ConditionMet;
		}

	// Process outstanding waits
	do
		{
		RefPtr<WaitCondition>& wait = mWaits[mCurrentWait];

		// Get the result of the current wait.
		mEndResult = wait->GetResult();

		switch (mEndResult)
			{
			case Error:
				mEndText = wait->GetResultText();
				break;

			// If the current wait has completed successfully then advance to the
			// next wait condition.
			case ConditionMet:
				mEndText = wait->GetResultText();

				++mCurrentWait;

				if ((unsigned)mCurrentWait < mWaits.size())
					{
					mWaits[mCurrentWait]->WaitBeginning();
					mStartTime = TIMEGETTIME();
					}
				break;

			case Waiting:
				// Check for timeout
				if ((TIMEGETTIME() - mStartTime) > wait->GetTimeout())
					{
					wait->EndWait(TimeOut, WOLSTRING("WOL_TIMEDOUT"));
					EndWait(TimeOut, wait->GetWaitText());
					}
				break;

			default:
				break;
			}
		} while ((mEndResult == ConditionMet) && ((unsigned)mCurrentWait < mWaits.size()));

	return mEndResult;
	}


/******************************************************************************
*
* NAME
*     SerialWait::EndWait
*
* DESCRIPTION
*     End the wait condition.
*
* INPUTS
*     Result      - Reason for ending the wait condition.
*     Description - Text description of end.
*
* RESULT
*     NONE
*
******************************************************************************/

void SerialWait::EndWait(WaitResult result, const unichar_t* endText)
	{
	mEndResult = result;
	mEndText = endText;

	// End the current wait.
	if ((mCurrentWait >= 0) && ((unsigned)mCurrentWait < mWaits.size()))
		{
		mWaits[mCurrentWait]->EndWait(result, endText);
		}
	}


/******************************************************************************
*
* NAME
*     SerialWait::GetResultText
*
* DESCRIPTION
*     Get a text description of the wait condition result.
*
* INPUTS
*     NONE
*
* RESULT
*     ResultText
*
******************************************************************************/

const unichar_t* SerialWait::GetResultText(void) const
	{
	return mEndText;
	}


/******************************************************************************
*
* NAME
*     SerialWait::GetWaitText
*
* DESCRIPTION
*     Get a text description of the current wait.
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

const unichar_t* SerialWait::GetWaitText(void) const
	{
	if ((mCurrentWait >= 0) && ((unsigned)mCurrentWait < mWaits.size()))
		{
		return (mWaits[mCurrentWait]->GetWaitText());
		}

	return NULL;
	}


/******************************************************************************
*
* NAME
*     SerialWait::GetTimeout
*
* DESCRIPTION
*     Get the timeout for the current wait condition being processed.
*
* INPUTS
*     NONE
*
* RESULT
*     Timeout
*
******************************************************************************/

unsigned int SerialWait::GetTimeout(void) const
	{
	if ((mCurrentWait >= 0) && ((unsigned)mCurrentWait < mWaits.size()))
		{
		return (mWaits[mCurrentWait]->GetTimeout());
		}

	return 0;
	}


/******************************************************************************
*
* NAME
*     ANDWait::Create
*
* DESCRIPTION
*     Create a logical AND wait condition. When this wait condition is begun
*     all the waits will be started at once. When all the waits complete the
*     AND wait condition is finished. If any condition fails then the entire
*     condition fails.
*
* INPUTS
*     Text - Text description of this wait condition.
*
* RESULT
*     ANDWait - Instance of newly created ANDWait condition.
*
******************************************************************************/

RefPtr<ANDWait> ANDWait::Create(const unichar_t* waitText)
	{
	return new ANDWait(waitText);
	}


/******************************************************************************
*
* NAME
*     ANDWait::ANDWait
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     Text - Text describing the wait condition.
*
* RESULT
*     NONE
*
******************************************************************************/

ANDWait::ANDWait(const unichar_t* waitText) :
		mEndResult(Waiting),
		mWaitText(waitText),
		mMaxTimeout(0)
	{
	}


/******************************************************************************
*
* NAME
*     ANDWait::~ANDWait
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

ANDWait::~ANDWait()
	{
	}


/******************************************************************************
*
* NAME
*     ANDWait::Add
*
* DESCRIPTION
*     Add a wait condition.
*
* INPUTS
*     Wait - Wait condition to add.
*
* RESULT
*     NONE
*
******************************************************************************/

void ANDWait::Add(const RefPtr<WaitCondition>& wait)
	{
	if (wait.IsValid())
		{
		mWaits.push_back(wait);

		// The timeout should be the longest of the all the waits
		unsigned int timeout = wait->GetTimeout();

		if (timeout > mMaxTimeout)
			{
			mMaxTimeout = timeout;
			}
		}
	else
		{
		assert(wait.IsValid() && "ORWait: Invalid wait condition");
		EndWait(Error, U_CHAR("Invalid wait condition."));
		}
	}


/******************************************************************************
*
* NAME
*     ANDWait::WaitBeginning
*
* DESCRIPTION
*     Begin the wait conditions
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void ANDWait::WaitBeginning(void)
	{
	for (size_t index = 0; index < mWaits.size(); index++)
		{
		mWaits[index]->WaitBeginning();
		}

	mStartTime = TIMEGETTIME();
	}


/******************************************************************************
*
* NAME
*     ANDWait::
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

WaitCondition::WaitResult ANDWait::GetResult(void)
	{
	if (mEndResult == Waiting)
		{
		size_t metConditions = 0;
		const size_t count = mWaits.size();

		// Get the result of all the wait conditions being processed. If they are
		// all finished then the entire wait finished.
		for (size_t index = 0; index < count; ++index)
			{
			RefPtr<WaitCondition>& wait = mWaits[index];
			WaitResult result = wait->GetResult();

			if (ConditionMet == result)
				{
				++metConditions;
				}
			else if (Waiting != result)
				{
				EndWait(result, wait->GetResultText());
				return result;
				}
			}

		if (metConditions == count)
			{
			EndWait(ConditionMet, GetWaitText());
			}
		else
			{
			// Check for timeout while we are still waiting
			if ((TIMEGETTIME() - mStartTime) > GetTimeout())
				{
				EndWait(TimeOut, WOLSTRING("WOL_TIMEDOUT"));
				}
			}
		}

	return mEndResult;
	}


/******************************************************************************
*
* NAME
*     ANDWait::EndWait
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void ANDWait::EndWait(WaitResult result, const unichar_t* endText)
	{
	mEndResult = result;
	mEndText = endText;

	for (unsigned int index = 0; index < mWaits.size(); ++index)
		{
		mWaits[index]->EndWait(result, U_CHAR(""));
		}
	}


/******************************************************************************
*
* NAME
*     ANDWait::
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

const unichar_t* ANDWait::GetResultText(void) const
	{
	return mEndText;
	}


/******************************************************************************
*
* NAME
*     ANDWait::
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

const unichar_t* ANDWait::GetWaitText(void) const
	{
	return mWaitText;
	}


/******************************************************************************
*
* NAME
*     ANDWait::
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

unsigned int ANDWait::GetTimeout(void) const
	{
	return mMaxTimeout;
	}


/******************************************************************************
*
* NAME
*     ORWait::Create
*
* DESCRIPTION
*     Create a OR wait condition. When this wait condition is begun all the
*     waits will be started at once. When any wait completes for any reason
*     the OR wait condition is finished.
*
* INPUTS
*     Text - Text description of this wait condition.
*
* RESULT
*
******************************************************************************/

RefPtr<ORWait> ORWait::Create(const unichar_t* waitText)
	{
	return new ORWait(waitText);
	}


/******************************************************************************
*
* NAME
*     ORWait::ORWait
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     Text -
*
* RESULT
*     NONE
*
******************************************************************************/

ORWait::ORWait(const unichar_t* waitText) :
	  mEndResult(Waiting),
	  mWaitText(waitText),
		mMaxTimeout(0)
	{
	}


/******************************************************************************
*
* NAME
*     ORWait::~ORWait
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

ORWait::~ORWait()
	{
	}


/******************************************************************************
*
* NAME
*     ORWait::Add
*
* DESCRIPTION
*     Add wait condition. All the wait conditions will be started at once.
*
* INPUTS
*     Wait condition
*
* RESULT
*     NONE
*
******************************************************************************/

void ORWait::Add(const RefPtr<WaitCondition>& wait)
	{
	if (wait.IsValid())
		{
		mWaits.push_back(wait);

		// Use the longest timeout value.
		unsigned int timeout = wait->GetTimeout();

		if (timeout > mMaxTimeout)
			{
			mMaxTimeout = timeout;
			}
		}
	else
		{
		assert(wait.IsValid() && "ORWait: Invalid wait condition");
		EndWait(Error, U_CHAR("Invalid wait condition."));
		}
	}


/******************************************************************************
*
* NAME
*     ORWait::WaitBeginning
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void ORWait::WaitBeginning(void)
	{
	for (unsigned int index = 0; index < mWaits.size(); index++)
		{
		mWaits[index]->WaitBeginning();
		}
	}


/******************************************************************************
*
* NAME
*     ORWait::GetResult
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

WaitCondition::WaitResult ORWait::GetResult(void)
	{
	// If we are not waiting the we are finished
	if (mEndResult != Waiting)
		{
		return mEndResult;
		}

	// Get the result of all the wait conditions being processed. If any one
	// is finished then the entire wait finished.
	for (unsigned int index = 0; index < mWaits.size(); index++)
		{
		mEndResult = mWaits[index]->GetResult();

		if (mEndResult != Waiting)
			{
			mEndText = mWaits[index]->GetResultText();
			return mEndResult;
			}
		}

	return Waiting;
	}


/******************************************************************************
*
* NAME
*     ORWait::EndWait
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void ORWait::EndWait(WaitResult result, const unichar_t* endText)
	{
	mEndText = endText;
	mEndResult = result;

	for (unsigned int index = 0; index < mWaits.size(); ++index)
		{
		mWaits[index]->EndWait(result, U_CHAR(""));
		}
	}


/******************************************************************************
*
* NAME
*     ORWait::GetResultText
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

const unichar_t* ORWait::GetResultText(void) const
	{
	return mEndText;
	}


/******************************************************************************
*
* NAME
*     ORWait::WaitText
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

const unichar_t* ORWait::GetWaitText(void) const
	{
	return mWaitText;
	}
