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
 *                     $Archive:: /Commando/Code/WWAudio/Threads.h                                                                                                                                                                                                                                                                                                                               $Modtime:: 7/17/99 3:32p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __WWAUDIO_THREADS_H
#define __WWAUDIO_THREADS_H

#include "thread.h"
#include <condition_variable>
#include <mutex>

// Forward declarations
class RefCountClass;


//////////////////////////////////////////////////////////////////////////
//
//	WWAudioThreadsClass
//
//	Simple class that provides a common namespace for tying thread
// information together.
//
//////////////////////////////////////////////////////////////////////////
class WWAudioThreadsClass
{
	public:

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		WWAudioThreadsClass (void);
		~WWAudioThreadsClass (void);

		//////////////////////////////////////////////////////////////////////
		//	Public methods
		//////////////////////////////////////////////////////////////////////

		//
		//	Delayed release mechanism
		//
		static ThreadClass *Create_Delayed_Release_Thread ();
		static void			End_Delayed_Release_Thread (uint32_t timeout = 20000);
		static void			Add_Delayed_Release_Object (RefCountClass *object, uint32_t delay = 2000);
		static void			Flush_Delayed_Release_Objects (void);

	private:

		//////////////////////////////////////////////////////////////////////
		//	Private data types
		//////////////////////////////////////////////////////////////////////
		struct DELAYED_RELEASE_INFO
		{
			RefCountClass *	object;
			uint32_t		time;

			DELAYED_RELEASE_INFO *next;
			DELAYED_RELEASE_INFO *prev;

		};

		class DelayedThreadClass : public ThreadClass {
		protected:
			void Thread_Function() override;
		};

		//typedef DynamicVectorClass<DELAYED_RELEASE_INFO *>	RELEASE_LIST;

		//////////////////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////////////////
		static DelayedThreadClass			*	m_hDelayedReleaseThread;
		static std::condition_variable	m_hDelayedReleaseConditionVariable;
		static DELAYED_RELEASE_INFO *	m_ReleaseListHead;
		static std::mutex	m_ListMutex;
		static bool							m_IsFlushing;
};

#endif //__WWAUDIO_THREADS_H

