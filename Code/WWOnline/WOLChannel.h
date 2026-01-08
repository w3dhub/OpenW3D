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
*     $Archive: /Commando/Code/WWOnline/WOLChannel.h $
*
* DESCRIPTION
*     Westwood Online channel representation.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 17 $
*     $Modtime: 11/08/01 5:05p $
*
******************************************************************************/

#ifndef __WOLCHANNEL_H__
#define __WOLCHANNEL_H__

#include "RefCounted.h"
#include "RefPtr.h"
#include <wwlib/Notify.h>
#include <wwlib/widestring.h>

namespace WOL
{
#include <wolapi/WOLAPI.h>
}

#include <list>

namespace WWOnline {

class ChannelData :
		public RefCounted
	{
	public:
		static RefPtr<ChannelData> Create(const WOL::Channel&);
		static RefPtr<ChannelData> Create(const unichar_t* name, const unichar_t* password, int type);

		const WideStringClass& GetName(void)
			const {return mChannelName;}

		int GetType(void) const
			{return mData.type;}

		void SetLatency(int latency);

		int GetLatency(void) const
			{return mData.latency;}

		bool IsHidden(void) const
			{return (mData.hidden != 0);}

		void SetTopic(const char* topic);

		const char* GetTopic(void) const
			{return (const char*)mData.topic;}

		void SetExtraInfo(const char* exInfo);
	
		const char* GetExtraInfo(void) const
			{return (const char*)mData.exInfo;}

		// Check if this channel requires a password to join.
		bool IsPassworded(void) const;

		// Set the channels minimum and maximum users allowed.
		void SetMinMaxUsers(unsigned int minUsers, unsigned int maxUsers);

		// Get the minimum number of users for game.
		inline unsigned int GetMinUsers(void) const
			{return mData.minUsers;}

		// Get the maximum number of users for game.
		inline unsigned int GetMaxUsers(void) const
			{return mData.maxUsers;}

		inline unsigned int GetCurrentUsers(void) const
			{return mData.currentUsers;}

		// Set official status
		void SetOfficial(bool official);
		
		// Test if this channel is an official Westwood channel
		inline bool IsOfficial(void) const
			{return (mData.official != 0);}

		// Set the tournament type this channel is hosting (0 = none)
		void SetTournament(unsigned int tournamentType);
	
		// Get the tournament type for this channel
		inline unsigned int GetTournament(void) const
			{return mData.tournament;}
		
		inline unsigned int GetFlags(void) const
			{return mData.flags;}

		WOL::Channel& GetData(void)
			{return mData;}

		void UpdateData(const WOL::Channel&);

	protected:
		ChannelData(const WOL::Channel&);
		ChannelData(const unichar_t* name, const unichar_t* password, int type);
		virtual ~ChannelData();

		ChannelData(const ChannelData&);
		const ChannelData& operator=(const ChannelData&);

		WOL::Channel mData;
		WideStringClass mChannelName;
	};


typedef enum
	{
	ChannelError = -1,
	ChannelLeft = 0,
	ChannelJoined,
	ChannelCreated,
	ChannelNotFound,
	ChannelExists,
	ChannelBadPassword,
	ChannelFull,
	ChannelBanned,
	ChannelKicked,
	ChannelNewData,
	ChannelLeaving
	} ChannelStatus;

ChannelStatus GetChannelStatusFromHResult(HRESULT result);
const unichar_t* GetChannelStatusDescription(ChannelStatus status);

class ChannelEvent :
		public TypedEvent< ChannelEvent, const RefPtr<ChannelData> >
	{
	public:
		ChannelStatus GetStatus(void) const
			{return mStatus;}

		ChannelEvent(ChannelStatus status, const RefPtr<ChannelData>& channel) :
				TypedEvent< ChannelEvent, const RefPtr<ChannelData> >(channel),
				mStatus(status)
			{}

		~ChannelEvent()
			{}

	protected:
		ChannelStatus mStatus;
	};


typedef std::list< RefPtr<ChannelData> > ChannelList;

class ChannelListEvent :
		public TypedEvent<ChannelListEvent, ChannelList>
	{
	public:
		typedef enum {Error = -1, NewList, Update, Remove} Event;

		Event GetEvent(void) const
			{return mEvent;}

		int GetChannelType(void)
			{return mType;}

		ChannelListEvent(Event event, ChannelList& list, int type) :
				TypedEvent<ChannelListEvent, ChannelList>(list),
				mEvent(event),
				mType(type)
			{}

		~ChannelListEvent()
			{}

	protected:
		Event mEvent;
		int mType;
	};


ChannelList::iterator FindChannelNode(ChannelList& list, const char* name);
RefPtr<ChannelData> FindChannelInList(ChannelList& list, const unichar_t* name);
RefPtr<ChannelData> FindChannelInList(ChannelList& list, const char* name);

} // namespace WWOnline

#endif // __WOLCHANNEL_H__
