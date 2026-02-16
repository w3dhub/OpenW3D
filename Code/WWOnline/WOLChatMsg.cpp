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
*     $Archive: /Commando/Code/WWOnline/WOLChatMsg.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 7 $
*     $Modtime: 10/23/01 10:36a $
*
******************************************************************************/

#include "WOLChatMsg.h"
#include <wolapi/chatdefs.h>

namespace WOL
{
#include <wolapi/WOLAPI.h>
}

namespace WWOnline {

/******************************************************************************
*
* NAME
*     ChatMessage::ChatMessage
*
* DESCRIPTION
*     Constructor (ANSI)
*
* INPUTS
*     Sender  - Name of message sender
*     Message - Message to send
*     Private - Flag indicating if message is private.
*     Action  - Flag indicating if message in an action.
*
* RESULT
*
******************************************************************************/

ChatMessage::ChatMessage(const WOL::User* sender, const char* message, bool isPrivate, bool isAction) :
		mMessage(message),
		mIsPrivate(isPrivate),
		mIsAction(isAction),
		mSenderFlags(0),
		mSenderClanID(0)
	{
	if (sender)
		{
		mSenderName = (const char*)sender->name;
		mSenderFlags = sender->flags;
		mSenderClanID = sender->squadID;
		}
	}


/******************************************************************************
*
* NAME
*     ChatMessage::ChatMessage
*
* DESCRIPTION
*     Constructor (Unicode)
*
* INPUTS
*     Sender  - Name of message sender
*     Message - Message to send
*     Private - Flag indicating if message is private.
*     Action  - Flag indicating if message in an action.
*
* RESULT
*     NONE
*
******************************************************************************/

ChatMessage::ChatMessage(const WOL::User* sender, const unichar_t* message, bool isPrivate, bool isAction) :
		mMessage(message),
		mIsPrivate(isPrivate),
		mIsAction(isAction),
		mSenderFlags(0),
		mSenderClanID(0)
	{
	if (sender)
		{
		mSenderName = (const char*)sender->name;
		mSenderFlags = sender->flags;
		mSenderClanID = sender->squadID;
		}
	}


/******************************************************************************
*
* NAME
*     ChatMessage::ChatMessage
*
* DESCRIPTION
*     Constructor (Unicode)
*
* INPUTS
*     Sender  - Name of message sender
*     Message - Message to send
*     Private - Flag indicating if message is private.
*     Action  - Flag indicating if message in an action.
*
* RESULT
*     NONE
*
******************************************************************************/

ChatMessage::ChatMessage(const unichar_t* sender, const unichar_t* message, bool isPrivate, bool isAction) :
		mSenderName(sender),
		mMessage(message),
		mIsPrivate(isPrivate),
		mIsAction(isAction),
		mSenderFlags(CHAT_USER_MYSELF),
		mSenderClanID(0)
	{
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

ChatMessage::ChatMessage(const ChatMessage& chat)
	{
	*this = chat;
	}


/******************************************************************************
*
* NAME
*     ChatMessage::~ChatMessage
*
* DESCRIPTION
*     Destructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

ChatMessage::~ChatMessage()
	{
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

const ChatMessage& ChatMessage::operator=(const ChatMessage& chat)
	{
	mSenderName = chat.mSenderName;
	mMessage = chat.mMessage;
	mIsPrivate = chat.mIsPrivate;
	mIsAction = chat.mIsAction;
	mSenderFlags = chat.mSenderFlags;
	mSenderClanID = chat.mSenderClanID;
	return (*this);
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

bool ChatMessage::IsSenderMyself(void) const
	{
	return ((mSenderFlags & CHAT_USER_MYSELF) == CHAT_USER_MYSELF);
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

bool ChatMessage::IsSenderChannelOwner(void) const
	{
	return ((mSenderFlags & CHAT_USER_CHANNELOWNER) == CHAT_USER_CHANNELOWNER);
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

bool ChatMessage::IsSenderSquelched(void) const
	{
	return ((mSenderFlags & CHAT_USER_SQUELCHED) == CHAT_USER_SQUELCHED);
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

bool ChatMessage::IsSenderInClan(unsigned int clanID) const
	{
	return (mSenderClanID == clanID);
	}


/******************************************************************************
*
* NAME
*     InAnsiText
*
* DESCRIPTION
*     Test if a Unicode string is within ANSI range.
*
* INPUTS
*     Text - Unicode text to check.
*
* RESULT
*     True if entire string is within ANSI range.
*
******************************************************************************/

bool IsAnsiText(const unichar_t* text)
	{
	for (int index = 0; text[index] != 0; index ++)
		{
		unsigned short value = text[index];

		if (value > 255)
			{
			return false;
			}
		}

	return true;
	}

} // namespace WWOnline
