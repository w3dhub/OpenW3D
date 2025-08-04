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

#ifndef MUTEX_H
#define MUTEX_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "always.h"
#include "thread.h"

#include <mutex>
#include <atomic>


// ----------------------------------------------------------------------------
//
// Critical sections are faster than mutex classes and they should be used
// for all synchronization.
//
// ----------------------------------------------------------------------------

class CriticalSectionClass
{
    std::recursive_mutex mtx;

	// Lock and unlock are private so that you can't use them directly. Use LockClass as a sentry instead!
	void Lock();
	void Unlock();

public:
	// Name can (and usually should) be NULL. Use name only if you wish to create a globally unique mutex
	CriticalSectionClass();
	~CriticalSectionClass();

	class LockClass
	{
		CriticalSectionClass& CriticalSection;
	public:
		// In order to lock a mutex create a local instance of LockClass with mutex as a parameter.
		LockClass(CriticalSectionClass& c);
		~LockClass();
	private:
		LockClass &operator=(const LockClass&) { return(*this); }
	};
	friend class LockClass;
};

// ----------------------------------------------------------------------------
//
// Fast critical section is really fast version of CriticalSection. The downside
// of it is that it can't be locked multiple times from the same thread.
//
// ----------------------------------------------------------------------------
// 2025-08: note: this is /allegedly/ faster than the CriticalSectionClass.
// The assumptions made circa 2000 may no longer hold.
class FastCriticalSectionClass
{
#ifdef FAST_CRITICAL_IS_MUTEX
    std::mutex mtx;
#else
    std::atomic_flag Flag{};
#endif

	void Thread_Safe_Set_Flag()
	{
#ifdef FAST_CRITICAL_IS_MUTEX
        mtx.lock();
#else
        while (Flag.test_and_set(std::memory_order_acq_rel)) {
            Flag.wait(true, std::memory_order_relaxed);
        }
#endif
	}

    void Thread_Safe_Clear_Flag()
	{
#ifdef FAST_CRITICAL_IS_MUTEX
        mtx.unlock();
#else
        Flag.clear(std::memory_order_release);
        Flag.notify_one();
#endif
	}

public:
	// Name can (and usually should) be NULL. Use name only if you wish to create a globally unique mutex
    FastCriticalSectionClass() {}

	class LockClass
	{
		FastCriticalSectionClass& CriticalSection;
	public:
		LockClass(FastCriticalSectionClass& critical_section) : CriticalSection(critical_section)
		{
			CriticalSection.Thread_Safe_Set_Flag();
		}

		~LockClass()
		{
			CriticalSection.Thread_Safe_Clear_Flag();
		}
	private:
		LockClass &operator=(const LockClass&) { return(*this); }
	};

	friend class LockClass;
};



#endif
