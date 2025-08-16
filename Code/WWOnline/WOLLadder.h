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
*     $Archive: /Commando/Code/WWOnline/WOLLadder.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 6 $
*     $Modtime: 1/25/02 11:51a $
*
******************************************************************************/

#ifndef __WOLLADDER_H__
#define __WOLLADDER_H__

#pragma warning(disable : 4711)

#include "RefCounted.h"
#include "RefPtr.h"

namespace WOL
{
#include <wolapi/WOLAPI.h>
}

namespace WWOnline {

typedef enum
	{
	LadderType_Individual = (1<<16L),
	LadderType_Clan = (1<<23L),
	LadderType_Team = (1<<24L)
	} LadderType;

#define LADDERTYPE_MASK (LadderType_Individual | LadderType_Clan | LadderType_Team)

class LadderData :
		public RefCounted
	{
	public:
		static RefPtr<LadderData> Create(const WOL::Ladder& ladder, long time);

		bool UpdateData(const WOL::Ladder& ladder, long time);

		const char* GetName(void) const
			{return (const char*)mData.login_name;}

		unsigned int GetWins(void) const
			{return mData.wins;}
			
		unsigned int GetLosses(void) const
			{return mData.losses;}

		unsigned int GetPoints(void) const
			{return mData.points;}

		unsigned int GetKills(void) const
			{return mData.kills;}

		unsigned int GetRung(void) const
			{return mData.rung;}

		unsigned int GetReserved1(void) const
			{return mData.reserved1;}

		unsigned int GetReserved2(void) const
			{return mData.reserved2;}

		long GetTimeStamp(void) const
			{return mTimeStamp;}

		WOL::Ladder& GetData()
			{return mData;}

	protected:
		LadderData(const WOL::Ladder& ladder, long time);
		virtual ~LadderData();

		WOL::Ladder mData;
		long mTimeStamp;
	};

class LadderInfoEvent
	{
	public:
		LadderInfoEvent(const wchar_t* requested, const WOL::Ladder& ladder, long time);

		virtual ~LadderInfoEvent()
			{}

		bool IsRanked(void) const;

		LadderType GetLadderType(void) const;

		const wchar_t* GetRequestedName(void) const
			{return mRequestedName;}

		const char* GetReceivedName(void) const
			{return (const char*)mWOLLadder.login_name;}

		const WOL::Ladder& GetWOLLadder(void) const
			{return mWOLLadder;}

		long GetTimeStamp(void) const
			{return mTimeStamp;}

	protected:
		// Prevent copy and assignment
		LadderInfoEvent(const LadderInfoEvent&);
		const LadderInfoEvent& operator=(const LadderInfoEvent&);

		const wchar_t* mRequestedName;
		const WOL::Ladder& mWOLLadder;
		long mTimeStamp;
	};

}

#endif // __WOLLADDER_H__
