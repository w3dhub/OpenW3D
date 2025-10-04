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
*     $Archive: /Commando/Code/WWOnline/WOLGameOptions.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 4 $
*     $Modtime: 9/20/01 6:22p $
*
******************************************************************************/

#include "WOLGameOptions.h"

namespace WWOnline {

/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

GameOptionsMessage::GameOptionsMessage(const WOL::User* user, const WOL::Channel* channel,
			const char* options, bool isPrivate) :
		mOptions(options),
		mIsPrivate(isPrivate)
	{
	if (user)
		{
		memcpy(&mUser, user, sizeof(mUser));
		}
	else
		{
		memset(&mUser, 0, sizeof(mUser));
		}

	if (channel)
		{
		memcpy(&mChannel, channel, sizeof(mChannel));
		}
	else
		{
		memset(&mChannel, 0, sizeof(mChannel));
		}
	}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

GameOptionsMessage::~GameOptionsMessage()
	{
	}

} // namespace WWOnline
