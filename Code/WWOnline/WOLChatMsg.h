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
*     $Archive: /Commando/Code/WWOnline/WOLChatMsg.h $
*
* DESCRIPTION
*     Chat Message representation.
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 6 $
*     $Modtime: 10/23/01 10:19a $
*
******************************************************************************/

#ifndef __WOLCHATMSG_H__
#define __WOLCHATMSG_H__

#include <wwlib/widestring.h>

#include <vector>

namespace WOL
{
struct User;
}

namespace WWOnline {

class ChatMessage
	{
	public:
		ChatMessage(const WOL::User* sender, const char* message, bool isPrivate, bool isAction);
		ChatMessage(const WOL::User* sender, const unichar_t* message, bool isPrivate, bool isAction);
		ChatMessage(const unichar_t* sender, const unichar_t* message, bool isPrivate, bool isAction);
		ChatMessage(const ChatMessage&);
		virtual ~ChatMessage();

		const ChatMessage& operator=(const ChatMessage&);

		inline const WideStringClass& GetSendersName(void) const
			{return mSenderName;}

		inline const WideStringClass& GetMessage(void) const
			{return mMessage;}

		inline bool IsAction(void) const
			{return mIsAction;}

		inline bool IsPrivate(void) const
			{return mIsPrivate;}

		bool IsSenderMyself(void) const;
		bool IsSenderChannelOwner(void) const;
		bool IsSenderSquelched(void) const;
		bool IsSenderInClan(unsigned int clanID) const;

	protected:
		WideStringClass mSenderName;
		WideStringClass mMessage;
		bool mIsPrivate;
		bool mIsAction;
		unsigned int mSenderFlags;
		unsigned int mSenderClanID;
	};

typedef std::vector<ChatMessage> ChatMessageList;

bool IsAnsiText(const unichar_t* text);

} // namespace WWOnline

#endif // __WOLCHATMSG_H__
