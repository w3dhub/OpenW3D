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
*     $Archive: /Commando/Code/WWOnline/RefCounted.h $
*
* DESCRIPTION
*     Base class for reference counted objects.
*     Use with the reference counting smart pointer RefPtr<Type>
*
*     ReleaseReference() is virtual. This helps support cached object and
*     singletons.
*
* PROGRAMMER
*     Steven Clinard
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Modtime: 7/06/01 11:18a $
*     $Revision: 1 $
*
******************************************************************************/

#ifndef __REFCOUNTED_H__
#define __REFCOUNTED_H__

#include <assert.h>

class RefCounted
	{
	public:
		// Add reference
		inline void AddReference(void)
			{++mRefCount;}

		// Release reference
		inline virtual void ReleaseReference(void)
			{if (--mRefCount == 0) delete this;}

		//! Retrieve current reference count.
		inline unsigned int ReferenceCount(void) const
			{return mRefCount;}

	protected:
		RefCounted() :
				mRefCount(0)
			{}

		RefCounted(const RefCounted&) :
				mRefCount(0)
			{}

		inline const RefCounted& operator=(const RefCounted&) = delete;

		virtual ~RefCounted()
			{assert(mRefCount == 0);}

		// Should not be allowed by default
		inline virtual bool operator==(const RefCounted&/* src*/) const
			{return false;}

		inline bool operator!=(const RefCounted&/* src*/) const
			{return false;}

	private:
		unsigned int mRefCount;
	};

#endif // __REFCOUNTED_H__
