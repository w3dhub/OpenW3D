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
*     $Archive: /Commando/Code/WWOnline/WOLUser.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Steve_t $
*
* VERSION INFO
*     $Revision: 19 $
*     $Modtime: 10/10/02 10:38a $
*
******************************************************************************/

#include "always.h"

#include "WOLUser.h"
#include "WOLChannel.h"
#include "WOLSquad.h"
#include "WOLLadder.h"
#include <wwdebug/wwdebug.h>

namespace WWOnline {

/******************************************************************************
*
* NAME
*     UserData::Create
*
* DESCRIPTION
*     Create a new User data instance
*
* INPUTS
*     WOLUser - Wolapi user structure
*
* RESULT
*     User -
*
******************************************************************************/

RefPtr<UserData> UserData::Create(const WOL::User& user)
	{
	return new UserData(user);
	}


/******************************************************************************
*
* NAME
*     UserData::Create
*
* DESCRIPTION
*     Create a new User data instance
*
* INPUTS
*     Name - Name of user
*
* RESULT
*     User -
*
******************************************************************************/

RefPtr<UserData> UserData::Create(const unichar_t* name)
	{
	if (name)
		{
		WOL::User user;
		memset(&user, 0, sizeof(user));
		u_wstomb((char*)user.name, name, sizeof(user.name));
		user.name[sizeof(user.name) - 1] = 0;

		return Create(user);
		}

	return RefPtr<UserData>();
	}


/******************************************************************************
*
* NAME
*     UserData::UserData
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

UserData::UserData(const WOL::User& user) :
		mUserName((char*)user.name),
		mLocation(USERLOCATION_UNKNOWN)
	{
	WWDEBUG_SAY(("WOL: Instantiating UserData '%S'\n", (const unichar_t*)mUserName));
	memcpy(&mData, &user, sizeof(mData));
	mKickTimer = 0;
	mData.next = NULL;
	}


/******************************************************************************
*
* NAME
*     UserData::~UserData
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

UserData::~UserData()
	{
	WWDEBUG_SAY(("WOL: Destructing UserData '%S'\n", (const unichar_t*)mUserName));
	}


/******************************************************************************
*
* NAME
*     UserData::UpdateData
*
* DESCRIPTION
*
* INPUTS
*     WOlUser - WOL user structure to update with.
*
* RESULT
*     NONE
*
******************************************************************************/

void UserData::UpdateData(const WOL::User& wolUser)
	{
	unichar_t name[64];
	u_mbtows(name, (const char*)wolUser.name, sizeof(wolUser.name));
	name[sizeof(wolUser.name) - 1] = 0;
	WWASSERT(u_strlen(name) && "Empty user name");

	bool isValid = (!mUserName.Is_Empty() && mUserName.Compare_No_Case(name) == 0);
	WWASSERT(isValid && "WOLUserData::UpdateData() mismatch");

	if (isValid)
		{
		if (mData.squadID != wolUser.squadID)
			{
			mSquad.Release();
			mData.squadID = wolUser.squadID;
			}
		}
	}


/******************************************************************************
*
* NAME
*     UserData::Squelch
*
* DESCRIPTION
*
* INPUTS
*     OnOff - Squelch state
*
* RESULT
*     NONE
*
******************************************************************************/

void UserData::Squelch(bool onoff)
	{
	mData.flags = (onoff == true) ?
		(mData.flags | CHAT_USER_SQUELCHED) : (mData.flags ^ CHAT_USER_SQUELCHED);
	}


/******************************************************************************
*
* NAME
*     UserData::SetLocation
*
* DESCRIPTION
*     Set the users location.
*
* INPUTS
*     Location - User's location
*
* RESULT
*     NONE
*
******************************************************************************/

void UserData::SetLocation(UserLocation location)
	{
	mLocation = location;
	}


/******************************************************************************
*
* NAME
*     UserData::SetChannel
*
* DESCRIPTION
*     Set the channel the user is in.
*
* INPUTS
*     Channel - Channel user is in.
*
* RESULT
*     NONE
*
******************************************************************************/

void UserData::SetChannel(const RefPtr<ChannelData>& channel)
	{
	mChannel = channel;
	}


/******************************************************************************
*
* NAME
*     UserData::SetSquad
*
* DESCRIPTION
*     Associate a squad for this user.
*
* INPUTS
*     Squad
*
* RESULT
*     NONE
*
******************************************************************************/

void UserData::SetSquad(const RefPtr<SquadData>& squadData)
	{
	mSquad = squadData;

	if (squadData.IsValid())
		{
		mData.squadID = squadData->GetID();

		// Syncronize common data with updated squad information.
		strncpy((char*)mData.squadname, squadData->GetName(), sizeof(mData.squadname));
		mData.squadname[sizeof(mData.squadname) - 1] = 0;

		strncpy((char*)mData.squadabbrev, squadData->GetAbbr(), sizeof(mData.squadabbrev));
		mData.squadabbrev[sizeof(mData.squadabbrev) - 1] = 0;
		}
	else
		{
		mData.squadname[0] = 0;
		mData.squadabbrev[0] = 0;
		}
	}


/******************************************************************************
*
* NAME
*     UserData::SetTeam
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void UserData::SetTeam(int team)
	{
	mData.team = team;
	}


/******************************************************************************
*
* NAME
*     UserData::SetLocale
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void UserData::SetLocale(WOL::Locale locale)
	{
	if (locale >= WOL::LOC_UNKNOWN && locale <= WOL::LOC_TURKEY)
		{
		mData.locale = locale;
		}
	}


/******************************************************************************
*
* NAME
*     UserData::SetLadder
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void UserData::SetLadder(const RefPtr<LadderData>& ladder)
	{
	mUserLadder = ladder;
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

RefPtr<LadderData> UserData::GetClanLadder(void) const
	{
	if (mSquad.IsValid())
		{
		return mSquad->GetLadder();
		}

	return NULL;
	}


/******************************************************************************
*
* NAME
*     UserData::SetTeamLadder
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void UserData::SetTeamLadder(const RefPtr<LadderData>& ladder)
	{
	mTeamLadder = ladder;
	}


/******************************************************************************
*
* NAME
*     UserData::
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void UserData::SetLadderFromType(const RefPtr<LadderData>& ladder, LadderType type)
	{
	if (type == LadderType_Team)
		{
		SetTeamLadder(ladder);
		return;
		}

	if (type == LadderType_Clan)
		{
		if (mSquad.IsValid())
			{
			mSquad->SetLadder(ladder);
			}

		return;
		}

	SetLadder(ladder);
	}


/******************************************************************************
*
* NAME
*     UserData::
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

RefPtr<LadderData> UserData::GetLadderFromType(LadderType type)
	{
	if (type == LadderType_Team)
		{
		return GetTeamLadder();
		}

	if (type == LadderType_Clan)
		{
		return GetClanLadder();
		}

	return GetLadder();
	}
				

/******************************************************************************
*
* NAME
*     NativeWOLUserList::
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

NativeWOLUserList::NativeWOLUserList(const UserList& users) :
		mNativeList(NULL)
	{
	const size_t count = users.size();

	if (count > 0)
	{
		mNativeList = new WOL::User[count];

		if (mNativeList)
		{
			for (size_t index = 0; index < count; index++)
			{
				WOL::User* wolUser = &users[index]->GetData();
				memcpy(&mNativeList[index], wolUser, sizeof(WOL::User));

				if (index == (count - 1))
					{
					mNativeList[index].next = NULL;
					}
				else
					{
					mNativeList[index].next = &mNativeList[index + 1];
					}
				}
			}
		}
	}


/******************************************************************************
*
* NAME
*     NativeWOLUserList::
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

NativeWOLUserList::~NativeWOLUserList()
	{
	if (mNativeList)
		{
		delete []mNativeList;
		}
	}


/******************************************************************************
*
* NAME
*     FindUserInList
*
* DESCRIPTION
*     Search for a user with the specified name in the list.
*
* INPUTS
*     Name - Name of user to search for.
*     List - UserData list to search.
*
* RESULT
*     User - 
*
******************************************************************************/

RefPtr<UserData> FindUserInList(const unichar_t* username, const UserList& list)
	{
	for (unsigned int index = 0; index < list.size(); index++)
		{
		const WideStringClass& name = list[index]->GetName();

		if (name.Compare_No_Case(username) == 0)
			{
			return list[index];
			}
		}

	return RefPtr<UserData>();
	}


/******************************************************************************
*
* NAME
*     RemoveUserInList
*
* DESCRIPTION
*     Remove the 
*
* INPUTS
*     Name - Name of user to remove.
*     List - UserData list to search.
*
* RESULT
*     Removed - User removed from list.
*
******************************************************************************/

RefPtr<UserData> RemoveUserInList(const unichar_t* username, UserList& list)
	{
	RefPtr<UserData> removedUser;

	UserList::iterator iter = list.begin();

	while (iter != list.end())
		{
		const WideStringClass& name = (*iter)->GetName();

		if (name.Compare_No_Case(username) == 0)
			{
			removedUser = *iter;
			list.erase(iter);
			break;
			}

		iter++;
		}

	return removedUser;
	}

} // namespace WWOnline
