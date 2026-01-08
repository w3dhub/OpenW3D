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
*     $Archive: /Commando/Code/WWOnline/WOLSquad.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 8 $
*     $Modtime: 12/10/01 5:15p $
*
******************************************************************************/

#ifndef __WOLSQUAD_H__
#define __WOLSQUAD_H__

#include "RefCounted.h"
#include "RefPtr.h"
#include <wwlib/widestring.h>
#include "WOLLadder.h"

namespace WOL
{
#include <wolapi/WOLAPI.h>
}

#include <vector>

namespace WWOnline {

class SquadData :
		public RefCounted
	{
	public:
		static void Reset(void);

		// Find a squad by its ID
		static RefPtr<SquadData> FindByID(unsigned int);

		// Find a squad by its abbreviation
		static RefPtr<SquadData> FindByAbbr(const unichar_t* abbr);

		// Create SquadData instance from WOL::Squad
		static RefPtr<SquadData> Create(const WOL::Squad&);

		// Update the squad information
		void UpdateData(const WOL::Squad&);

		// Get raw WOL::Squad data
		WOL::Squad& GetData(void)
			{return mData;}

		// Get ID of this squad
		unsigned int GetID(void) const
			{return mData.id;}

		// Get full name of this squad
    const char* GetName(void) const
			{return (const char*)mData.name;}

		// Get abbreviated name of this squad
		const char* GetAbbr(void) const
			{return (const char*)mData.abbreviation;}

		// Get the number of members in this squad
		int GetMemberCount(void) const
			{return mData.members;}

		// Get the squads rank
		int GetRank(void) const
			{return mData.rank;}

    int GetTeam(void) const
			{return mData.team;}

		int GetStatus(void) const
			{return mData.status;}

		// Get the email address for this squad
		const char* GetEmailAddress(void) const
			{return (const char*)mData.email;}

		// Get the ICQ address for this squad
		const char* GetICQAddress(void) const
			{return (const char*)mData.icq;}

		// Get the squads motto
		const char* GetMotto(void) const
			{return (const char*)mData.motto;}

		// Get the homepage URL for this squad
		const char* GetHomepageURL(void) const
			{return (const char*)mData.url;}

		// Get the ladder ranking informatio for this squad.
		RefPtr<LadderData> GetLadder(void) const
			{return mLadder;}

		// Set ladder ranking information for this squad
		void SetLadder(const RefPtr<LadderData>&);

	protected:
		SquadData(const WOL::Squad&);
		virtual ~SquadData();

	private:
		SquadData(const SquadData&);
		const SquadData& operator=(const SquadData&);

		WOL::Squad mData;
		RefPtr<LadderData> mLadder;

		typedef std::vector<RefPtr<SquadData> > SquadColl;
		static SquadColl _mSquadColl;
	};


class SquadEvent
	{
	public:
		SquadEvent(const unichar_t* memberName, const RefPtr<SquadData>& squadData) :
				mMemberName(memberName),
				mSquadData(squadData)
			{}

		~SquadEvent()
			{}

		const unichar_t* GetMemberName(void) const
			{return mMemberName;}

		const RefPtr<SquadData>& GetSquadData(void) const
			{return mSquadData;}

	protected:
		// Prevent copy and assignment
		SquadEvent(const SquadEvent&);
		const SquadEvent& operator=(const SquadEvent&);

		const unichar_t* mMemberName;
		RefPtr<SquadData> mSquadData;
	};

}

#endif //__WOLSQUAD_H__
