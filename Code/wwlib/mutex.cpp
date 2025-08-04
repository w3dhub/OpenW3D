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

#include "mutex.h"
#include "wwdebug.h"

// ----------------------------------------------------------------------------

CriticalSectionClass::CriticalSectionClass() : mtx()
{

}

CriticalSectionClass::~CriticalSectionClass()
{

}

void CriticalSectionClass::Lock()
{
    this->mtx.lock();
}

void CriticalSectionClass::Unlock()
{
    this->mtx.unlock();
}

// ----------------------------------------------------------------------------

CriticalSectionClass::LockClass::LockClass(CriticalSectionClass& critical_section) : CriticalSection(critical_section)
{
	CriticalSection.Lock();
}

CriticalSectionClass::LockClass::~LockClass()
{
	CriticalSection.Unlock();
}


