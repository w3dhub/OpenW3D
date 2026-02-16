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
*     $Archive: /Commando/Code/WWOnline/WOLChannel.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 23 $
*     $Modtime: 1/17/02 12:04p $
*
******************************************************************************/

#include "always.h"
#include <stdlib.h>
#include "WOLChannel.h"
#include "WOLString.h"
#include <wolapi/chatdefs.h>
#include <wwdebug/wwdebug.h>

namespace WWOnline {

/******************************************************************************
*
* NAME
*     ChannelData::Create(WOL::Channel)
*
* DESCRIPTION
*     Create Channel Data object from a WOLAPI channel.
*
* INPUTS
*     WOLChannel - WOLAPI channel structure.
*
* RESULT
*    Channel - Channel data object.
*
******************************************************************************/

RefPtr<ChannelData> ChannelData::Create(const WOL::Channel& wolChannel)
	{
	return new ChannelData(wolChannel);
	}


/******************************************************************************
*
* NAME
*     ChannelData::Create(Name, Password, Type)
*
* DESCRIPTION
*     Create a new Channel Data object
*
* INPUTS
*     Name     - Name of channel.
*     Password - Password needed to join channel
*     Type     - Type of channel
*
* RESULT
*    Channel - Channel data object.
*
******************************************************************************/

RefPtr<ChannelData> ChannelData::Create(const unichar_t* name, const unichar_t* password, int type)
	{
	if (name && u_strlen(name))
		{
		return new ChannelData(name, password, type);
		}

	return NULL;
	}


/******************************************************************************
*
* NAME
*     ChannelData::ChannelData(WOL::Channel)
*
* DESCRIPTION
*     WOLAPI channel constructor
*
* INPUTS
*     WOLChannel - WOLAPI channel structure.
*
* RESULT
*    NONE
*
******************************************************************************/

ChannelData::ChannelData(const WOL::Channel& channel) :
		mChannelName((char*)channel.name)
	{
	memcpy(&mData, &channel, sizeof(mData));
	mData.next = NULL;
	}


/******************************************************************************
*
* NAME
*     ChannelData::ChannelData(Name, Password, Type)
*
* DESCRIPTION
*     Named channel constructor
*
* INPUTS
*     Name     - Name of channel.
*     Password - Password needed to join channel
*     Type     - Type of channel
*
* RESULT
*     NONE
*
******************************************************************************/

ChannelData::ChannelData(const unichar_t* name, const unichar_t* password, int type) :
		mChannelName(name)
	{
	memset(&mData, 0, sizeof(mData));
	mData.type = type;

	WWASSERT(name && "NULL channel name");
	u_wstomb((char*)mData.name, name, sizeof(mData.name));
	mData.name[sizeof(mData.name) - 1] = 0;

	if (password)
		{
		u_wstomb((char*)mData.key, password, sizeof(mData.key));
		}
	}


/******************************************************************************
*
* NAME
*     ChannelData::~ChannelData
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

ChannelData::~ChannelData()
	{
	}


/******************************************************************************
*
* NAME
*     ChannelData::UpdateData
*
* DESCRIPTION
*     Update channel data with the WOLAPI channel
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void ChannelData::UpdateData(const WOL::Channel& wolChannel)
	{
	unichar_t name[64];
	u_mbtows(name, (const char*)wolChannel.name, sizeof(wolChannel.name));
	name[sizeof(wolChannel.name) - 1] = 0;
	WWASSERT(u_strlen(name));

	bool isValid = (!mChannelName.Is_Empty() && mChannelName.Compare_No_Case(name) == 0);
	WWASSERT(isValid && "WOLChannelData::UpdateData() channel mismatch");

	if (isValid)
		{
		memcpy(&mData, &wolChannel, sizeof(mData));
		mData.next = NULL;
		}
	}


/******************************************************************************
*
* NAME
*     ChannelData::SetLatency
*
* DESCRIPTION
*     Update the channel latency.
*
* INPUTS
*     Latency - Channel latency
*
* RESULT
*     NONE
*
******************************************************************************/

void ChannelData::SetLatency(int latency)
	{
	mData.latency = latency;
	}


/******************************************************************************
*
* NAME
*     ChannelData::SetTopic
*
* DESCRIPTION
*     Set the channel's topic
*
* INPUTS
*     Topic - New channel topic string.
*
* RESULT
*     NONE
*
******************************************************************************/

void ChannelData::SetTopic(const char* topic)
	{
	strncpy((char*)mData.topic, topic, sizeof(mData.topic));
	mData.topic[sizeof(mData.topic) - 1] = 0;
	}


/******************************************************************************
*
* NAME
*     ChannelData::SetExtraInfo
*
* DESCRIPTION
*     Set the channel's extra information.
*
* INPUTS
*     ExInfo - Data to be kept in the channel's ExInfo field.
*
* RESULT
*     NONE
*
******************************************************************************/

void ChannelData::SetExtraInfo(const char* exInfo)
	{
	strncpy((char*)mData.exInfo, exInfo, sizeof(mData.exInfo));
	mData.exInfo[sizeof(mData.exInfo) - 1] = 0;
	}


/******************************************************************************
*
* NAME
*     ChannelData::IsPassworded
*
* DESCRIPTION
*     Check if the channel requires a password to join.
*
* INPUTS
*     NONE
*
* RESULT
*     True if password required.
*
******************************************************************************/

bool ChannelData::IsPassworded(void) const
	{
	return ((mData.flags & CHAN_MODE_KEY) == CHAN_MODE_KEY);
	}


/******************************************************************************
*
* NAME
*     ChannelData::SetMinMaxUsers
*
* DESCRIPTION
*     Set the number of users that can join this channel.
*
* INPUTS
*     MinUsers - Minimum number of users required.
*     MaxUsers - Maximum number of users allowed.
*
* RESULT
*     NONE
*
******************************************************************************/

void ChannelData::SetMinMaxUsers(unsigned int minUsers, unsigned int maxUsers)
	{
	mData.minUsers = minUsers;
	mData.maxUsers = maxUsers;
	}


/******************************************************************************
*
* NAME
*     ChannelData::SetOfficial
*
* DESCRIPTION
*     Change the official state of this channel.
*
* INPUTS
*     Official - Flag indicating if this channel is official or not.
*
* RESULT
*     NONE
*
******************************************************************************/

void ChannelData::SetOfficial(bool official)
	{
	mData.official = official;
	}


/******************************************************************************
*
* NAME
*     ChannelData::SetTournament
*
* DESCRIPTION
*     Set the type of tournament game this channel hosting.
*
* INPUTS
*     Type - Type of tournament (0 = none)
*
* RESULT
*     NONE
*
******************************************************************************/

void ChannelData::SetTournament(unsigned int tournamentType)
	{
	mData.tournament = tournamentType;
	}


/******************************************************************************
*
* NAME
*     GetChannelStatusFromHRESULT
*
* DESCRIPTION
*     Get a channel status from the provided HRESULT
*
* INPUTS
*     HRESULT - Error / Status code.
*
* RESULT
*     ChannelStatus - Channel status
*
******************************************************************************/

ChannelStatus GetChannelStatusFromHResult(HRESULT result)
	{
	switch (result)
		{
		case CHAT_E_CHANNELEXISTS:
			return ChannelExists;
			break;

		case CHAT_E_CHANNELDOESNOTEXIST:
			return ChannelNotFound;
			break;

		case CHAT_E_BADCHANNELPASSWORD:
			return ChannelBadPassword;
			break;

		case CHAT_E_CHANNELFULL:
			return ChannelFull;
			break;

		case CHAT_E_BANNED:
			return ChannelBanned;
			break;

		default:
			break;
		}

	return ChannelError;
	}


/******************************************************************************
*
* NAME
*     GetChannelStatusDescription
*
* DESCRIPTION
*     Get a channel status description for its status code.
*
* INPUTS
*     Status - Channel status enumeration
*
* RESULT
*     Description - Text description of channel status.
*
******************************************************************************/

const unichar_t* GetChannelStatusDescription(ChannelStatus status)
	{
	static const char* _statusDesc[] =
		{
		"WOL_CHANNELERROR",
		"WOL_CHANNELLEFT",
		"WOL_CHANNELJOINED",
		"WOL_CHANNELCREATED",
		"WOL_CHANNELNOTFOUND",
		"WOL_CHANNELEXISTS",
		"WOL_BADPASSWORD",
		"WOL_CHANNELFULL",
		"WOL_CHANNELBANNED",
		"WOL_CHANNELKICKED",
		"WOL_BLANK", // ChannelNewData
		"WOL_CHANNELLEAVE"
		};

	WWASSERT(status >= 0 && status <= ChannelLeaving);
	return WOLSTRING(_statusDesc[status + 1]);
	}


// Find the channel node by name
ChannelList::iterator FindChannelNode(ChannelList& list, const char* name)
	{
	if (name == NULL)
		{
		return list.end();
		}

	ChannelList::iterator node = list.begin();

	while (node != list.end())
		{
		WOL::Channel& wolChannel = (*node)->GetData();

		if (stricmp(name, (const char*)wolChannel.name) == 0)
			{
			break;
			}

		node++;
		}

	return node;
	}


// Find a channel in the specified list by ANSI name
RefPtr<ChannelData> FindChannelInList(ChannelList& list, const char* name)
	{
	if (name)
		{
		ChannelList::iterator node = list.begin();

		while (node != list.end())
			{
			WOL::Channel& wolChannel = (*node)->GetData();

			if (stricmp(name, (const char*)wolChannel.name) == 0)
				{
				return *node;
				}

			node++;
			}
		}

	return NULL;
	}


// Find a channel in the specified list by Unicode name.
RefPtr<ChannelData> FindChannelInList(ChannelList& list, const unichar_t* name)
	{
	if (name)
		{
		ChannelList::iterator node = list.begin();

		while (node != list.end())
			{
			const WideStringClass& channame = (*node)->GetName();

			if (channame.Compare_No_Case(name) == 0)
				{
				return *node;
				}

			node++;
			}
		}

	return NULL;
	}

} // namespace WWOnline
