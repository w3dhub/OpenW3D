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
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WWAudio                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWAudio/Threads.cpp                                                                                                                                                                                                                                                                                                                               $Modtime:: 7/17/99 3:32p                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "Threads.h"
#include "refcount.h"
#include "Utils.h"
#include "wwdebug.h"
#include "systimer.h"


///////////////////////////////////////////////////////////////////////////////////////////
//	Static member initialization
///////////////////////////////////////////////////////////////////////////////////////////
WWAudioThreadsClass::DELAYED_RELEASE_INFO	*WWAudioThreadsClass::m_ReleaseListHead	= nullptr;
WWAudioThreadsClass::DelayedThreadClass		*WWAudioThreadsClass::m_hDelayedReleaseThread = nullptr;
std::mutex					WWAudioThreadsClass::m_ListMutex;
std::condition_variable		WWAudioThreadsClass::m_hDelayedReleaseConditionVariable;
bool						WWAudioThreadsClass::m_IsFlushing				= false;

///////////////////////////////////////////////////////////////////////////////////////////
//
//	WWAudioThreadsClass
//
///////////////////////////////////////////////////////////////////////////////////////////
WWAudioThreadsClass::WWAudioThreadsClass (void)
{
	return ;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	~WWAudioThreadsClass
//
///////////////////////////////////////////////////////////////////////////////////////////
WWAudioThreadsClass::~WWAudioThreadsClass (void)
{
	return ;
}

///////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Delayed_Release_Thread
//
///////////////////////////////////////////////////////////////////////////////////////////
ThreadClass *
WWAudioThreadsClass::Create_Delayed_Release_Thread ()
{
	//
	//	If the thread isn't already running, then
	//
	if (m_hDelayedReleaseThread == nullptr) {
		m_hDelayedReleaseThread = new DelayedThreadClass;
		m_hDelayedReleaseThread->Execute();
	}

	return m_hDelayedReleaseThread;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	End_Delayed_Release_Thread
//
///////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioThreadsClass::End_Delayed_Release_Thread (uint32_t /*timeout*/)
{
	//
	//	If the thread is running, then wait for it to finish
	//
	if (m_hDelayedReleaseThread != nullptr) {
		m_hDelayedReleaseConditionVariable.notify_all();
		m_hDelayedReleaseThread->Stop();
		m_hDelayedReleaseThread	= nullptr;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Add_Delayed_Release_Object
//
///////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioThreadsClass::Add_Delayed_Release_Object
(
	RefCountClass *	object,
	uint32_t		delay
)
{
	if (m_IsFlushing) {
		REF_PTR_RELEASE (object);
	} else {

		//
		//	Make sure we have a thread running that will handle
		// the operation for us.
		//
		if (m_hDelayedReleaseThread == nullptr) {
			Create_Delayed_Release_Thread ();
		}

		//
		//	Wait for the release thread to finish using the
		// list pointer
		//
		{
			std::unique_lock lock(m_ListMutex);

			//
			//	Create a new delay-information structure and
			//	add it to our list
			//
			DELAYED_RELEASE_INFO *info = new DELAYED_RELEASE_INFO;
			info->object	= object;
			info->time		= TIMEGETTIME () + delay;
			info->next		= m_ReleaseListHead;
			info->prev		= nullptr;

			if (info->next != nullptr) {
				info->next->prev = info;
			}

			m_ReleaseListHead = info;
			m_hDelayedReleaseConditionVariable.notify_all();
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Flush_Delayed_Release_Objects
//
///////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioThreadsClass::Flush_Delayed_Release_Objects (void)
{
	std::lock_guard lock(m_ListMutex);
	m_IsFlushing = true;

	//
	//	Loop through all the objects in our delay list, and
	// free them now.
	//
	DELAYED_RELEASE_INFO *info = nullptr;
	DELAYED_RELEASE_INFO *next = nullptr;
	for (info = m_ReleaseListHead; info != nullptr; info = next) {
		next = info->next;

		//
		//	Free the object
		//
		REF_PTR_RELEASE (info->object);
		SAFE_DELETE (info);
	}

	m_ReleaseListHead = nullptr;
	return ;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Delayed_Release_Thread_Proc
//
///////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioThreadsClass::DelayedThreadClass::Thread_Function()
{
	const uint32_t base_timeout = 2000;
	uint32_t timeout = base_timeout + rand () % 1000;

	//
	//	Keep looping forever until we are signaled to quit (or an error occurs)
	//
	{
		std::unique_lock lock(m_ListMutex);
		while (m_hDelayedReleaseConditionVariable.wait_for(lock, std::chrono::milliseconds{timeout}) == std::cv_status::timeout) {

			//
			//	Loop through all the objects in our delay list, and
			// free any that have expired.
			//
			uint32_t current_time		= TIMEGETTIME ();
			DELAYED_RELEASE_INFO *curr	= nullptr;
			DELAYED_RELEASE_INFO *prev	= nullptr;
			DELAYED_RELEASE_INFO *next	= nullptr;
			for (curr = m_ReleaseListHead; curr != nullptr; curr = next) {
				next = curr->next;
				prev = curr->prev;

				//
				//	If the time has expired, free the object
				//
				// FIXME: wraparound can cause this comparison to never be true
				if (current_time >= curr->time) {

					//
					//	Unlink the object
					//
					if (curr == m_ReleaseListHead) {
						m_ReleaseListHead = next;
					}

					if (prev != nullptr) {
						prev->next = next;
					}

					if (next != nullptr) {
						next->prev = prev;
					}

					//
					//	Free the object
					//
					REF_PTR_RELEASE (curr->object);
					SAFE_DELETE (curr);
				}
			}

			//
			//	To avoid 'periodic' releases, randomize our timeout
			//
			timeout = base_timeout + rand () % 1000;
		}
	}

	Flush_Delayed_Release_Objects ();
	return ;
}
