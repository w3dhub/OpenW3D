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
*     $Archive: /Commando/Code/WWOnline/WOLUser.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Steve_t $
*
* VERSION INFO
*     $Revision: 19 $
*     $Modtime: 10/10/02 10:17a $
*
******************************************************************************/

#ifndef __WOLUSER_H__
#define __WOLUSER_H__

#include <windows.h>
#include "atlbase_compat.h"
#include "RefCounted.h"
#include "RefPtr.h"
#include <wwlib/Notify.h>
#include <wwlib/widestring.h>
#include "WOLChannel.h"
#include "WOLSquad.h"
#include "WOLLadder.h"

namespace WOL
{
#include <wolapi/WOLAPI.h>
#include <wolapi/chatdefs.h>
}

#include <vector>

namespace WWOnline {

typedef enum
	{
	USERLOCATION_UNKNOWN = 0,
	USERLOCATION_OFFLINE,
	USERLOCATION_HIDING,
	USERLOCATION_NO_CHANNEL,
	USERLOCATION_IN_CHANNEL
	} UserLocation;

class UserData :
		public RefCounted
	{
	public:
		// Create new User
		static RefPtr<UserData> Create(const WOL::User&);
		static RefPtr<UserData> Create(const unichar_t*);

		// Get WOL::User data
		WOL::User& GetData(void)
			{return mData;}

		void UpdateData(const WOL::User&);

		// Get user's name
		const WideStringClass& GetName(void) const
			{return mUserName;}

		// Check if this user is the one logged in.
		bool IsMe(void) const
			{return ((mData.flags & CHAT_USER_MYSELF) == CHAT_USER_MYSELF);}

		// Check if this user is the channel owner.
		bool IsChannelOwner(void) const
			{return ((mData.flags & CHAT_USER_CHANNELOWNER) == CHAT_USER_CHANNELOWNER);}

		// Check if this user has the floor to speak
		bool HasVoice(void) const
			{return ((mData.flags & CHAT_USER_VOICE) == CHAT_USER_VOICE);}

		// Check if this user is squelched
		bool IsSquelched(void) const
			{return ((mData.flags & CHAT_USER_SQUELCHED) == CHAT_USER_SQUELCHED);}

		// Squelch this user
		void Squelch(bool onoff);

		// Get the users location (IE: Offline, Hiding, In channel, etc...)
		UserLocation GetLocation(void) const
			{return mLocation;}

		// Set the users location
		void SetLocation(UserLocation location);

		// Get the channel the user is in (If available)
		const RefPtr<ChannelData> GetChannel(void)
			{return mChannel;}

		// Set the channel the user is in
		void SetChannel(const RefPtr<ChannelData>& channel);

		// Get user's clan ID
		unsigned int GetSquadID(void) const
			{return mData.squadID;}

		// Get access user's Clan data (This is shared with all other users in the same clan)
		RefPtr<SquadData> GetSquad(void) const
			{return mSquad;}

		// Set user's clan data
		void SetSquad(const RefPtr<SquadData>&);

		// Get user's team
		int GetTeam(void) const
			{return mData.team;}

		// Set user's team
		void SetTeam(int);

		// Get user's locale (IE: US, Germany, Korea, etc...)
		WOL::Locale GetLocale(void) const
			{return mData.locale;}

		// Set user's locale
		void SetLocale(WOL::Locale);

		// Set user's ladder
		void SetLadder(const RefPtr<LadderData>&);

		// Get user's ladder
		RefPtr<LadderData> GetLadder(void) const
			{return mUserLadder;}

		// Get the ladder ranking data for the users clan
		RefPtr<LadderData> GetClanLadder(void) const;

		void SetTeamLadder(const RefPtr<LadderData>&);

		RefPtr<LadderData> GetTeamLadder(void) const
			{return mTeamLadder;}

		void SetLadderFromType(const RefPtr<LadderData>& ladder, LadderType ladderType);
		RefPtr<LadderData> GetLadderFromType(LadderType ladderType);

		unsigned int mKickTimer;

	private:
		UserData(const WOL::User&);
		virtual ~UserData();

		// Prevent copy and assignment
		UserData(const UserData&);
		const UserData& operator=(const UserData&);

		WOL::User mData;

		WideStringClass mUserName;
		UserLocation mLocation;
		int mUserTeam;

		RefPtr<ChannelData> mChannel;
		RefPtr<SquadData> mSquad;
		RefPtr<LadderData> mUserLadder;
		RefPtr<LadderData> mTeamLadder;
	};


typedef std::vector< RefPtr<UserData> > UserList;

class UserEvent :
		public TypedEvent< UserEvent, const RefPtr<UserData> >
	{
	public:
		enum Event {Error = 0, NewData, Join, Leave, Located, Kicked, Banned, Locale, SquadInfo, LadderInfo};

		Event GetEvent(void) const
			{return mEvent;}

		UserEvent(Event event, const RefPtr<UserData>& user) :
				TypedEvent< UserEvent, const RefPtr<UserData> >(user),
				mEvent(event)
			{}

		~UserEvent()
			{}

	private:
		Event mEvent;
	};


class BuddyEvent :
		public TypedEvent< BuddyEvent, const UserList >
	{
	public:
		enum Event {Added = 0, Deleted, NewList};

		Event GetEvent(void) const
			{return mEvent;}

		BuddyEvent(Event event, const UserList& list) :
				TypedEvent<BuddyEvent, const UserList>(list),
				mEvent(event)
			{}

		~BuddyEvent()
			{}

	private:
		Event mEvent;
	};


class NativeWOLUserList
	{
	public:
		NativeWOLUserList(const UserList& users);
		~NativeWOLUserList();

		operator const WOL::User*(void)
			{return mNativeList;}

		operator WOL::User*(void)
			{return mNativeList;}

	protected:
		NativeWOLUserList(const NativeWOLUserList&);
		const NativeWOLUserList& operator=(const NativeWOLUserList&);

		WOL::User* mNativeList;
	};


class UserIPEvent :
		public TypedEvent< UserIPEvent, const WOL::User >
	{
	public:
		enum Event {Error = 0, GotIP};

		Event GetEvent(void) const
			{return mEvent;}

		UserIPEvent(Event event, const WOL::User &user) :
				TypedEvent<UserIPEvent, const WOL::User>(user),
				mEvent(event)
			{}

		~UserIPEvent()
			{}

	private:
		Event mEvent;
	};


RefPtr<UserData> FindUserInList(const unichar_t* name, const UserList& list);
RefPtr<UserData> RemoveUserInList(const unichar_t* name, UserList& list);

} // namespace WWOnline

#endif // __WOLUSER_H__
