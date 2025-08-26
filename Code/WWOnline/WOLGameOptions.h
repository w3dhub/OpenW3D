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
*     $Archive: /Commando/Code/WWOnline/WOLGameOptions.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 4 $
*     $Modtime: 10/04/01 10:28p $
*
******************************************************************************/

#ifndef __WOLGAMEOPTIONS_H__
#define __WOLGAMEOPTIONS_H__

#include "RefPtr.h"
#include <wwlib/wwstring.h>

namespace WOL
{
#include "WOLAPI.h"
}

#ifdef _MSC_VER
#pragma warning (push,3)
#endif

#include <vector>

#ifdef _MSC_VER
#pragma warning (pop)
#endif

namespace WWOnline {

class GameOptionsMessage
	{
	public:
		GameOptionsMessage(const WOL::User* user, const WOL::Channel* channel, const char* options, bool isPrivate);

		virtual ~GameOptionsMessage();

		inline const char* GetSendersName(void) const
			{return (const char*)mUser.name;}

		inline const WOL::User& GetWOLUser(void) const
			{return mUser;}

		inline const WOL::Channel& GetWOLChannel(void) const
			{return mChannel;}

		inline const StringClass& GetOptions(void) const
			{return mOptions;}

		inline bool IsPrivate(void) const
			{return mIsPrivate;}

	protected:
		WOL::User mUser;
		WOL::Channel mChannel;
		StringClass mOptions;
		bool mIsPrivate;

	private:
		GameOptionsMessage(const GameOptionsMessage&);
		const GameOptionsMessage& operator=(const GameOptionsMessage&);
	};

}

#endif // __WOLGAMEOPTIONS_H__
