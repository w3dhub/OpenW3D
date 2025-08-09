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

#include <mutex>


// Always use mutex or critical section when accessing the same data from multiple threads!

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


#endif
