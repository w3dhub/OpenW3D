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
*     $Archive: /Commando/Code/WWOnline/WOLString.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 2 $
*     $Modtime: 1/11/02 7:26p $
*
******************************************************************************/

#ifndef __WOLSTRING_H__
#define __WOLSTRING_H__

#include "unichar.h"
#include <stdlib.h>

namespace WWOnline {

typedef const unichar_t* (*WOLStringLookupFunc)(const char* token);

class WOLString
	{
	public:
		static void SetLookupFunc(WOLStringLookupFunc);

		static const unichar_t* Lookup(const char* token)
			{return _mLookupFunc(token);}

	private:
		static WOLStringLookupFunc _mLookupFunc;
	};

} // namespace WWOnline

#define WOLSTRING(token) WWOnline::WOLString::Lookup(token)

#endif // __WOLSTRING_H__

