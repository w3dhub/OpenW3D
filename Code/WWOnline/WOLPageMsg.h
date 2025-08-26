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
*     $Archive: /Commando/Code/WWOnline/WOLPageMsg.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 5 $
*     $Modtime: 12/19/01 2:00p $
*
******************************************************************************/

#ifndef __WOLPAGEMSG_H__
#define __WOLPAGEMSG_H__

#include <wwlib/widestring.h>

#ifdef _MSC_VER
#pragma warning (push,3)
#endif

#include <vector>

#ifdef _MSC_VER
#pragma warning (pop)
#endif

namespace WWOnline {

class PageMessage
	{
	public:
		PageMessage(const char* pagersName, const char* message);
		PageMessage(const wchar_t* pagersName, const wchar_t* message);
		PageMessage(const PageMessage&);
		virtual ~PageMessage();

		const PageMessage& operator=(const PageMessage&);

		inline const WideStringClass& GetPagersName(void) const
			{return mPager;}

		inline const WideStringClass& GetPageMessage(void) const
			{return mMessage;}

	protected:
		WideStringClass mPager;
		WideStringClass mMessage;
	};

typedef enum
	{
	PAGESEND_ERROR = 0,
	PAGESEND_OFFLINE,
	PAGESEND_HIDING,
	PAGESEND_SENT,
	} PageSendStatus;

typedef std::vector<PageMessage> PageMessageList;

} // namespace WWOnline

#endif // __WOLPAGEMSG_H__
