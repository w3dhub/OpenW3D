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
*     $Archive: /Commando/Code/WWOnline/WOLSquad.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 9 $
*     $Modtime: 1/12/02 9:45p $
*
******************************************************************************/

#include <stdlib.h>
#include <memory.h>
#include "WOLSquad.h"
#include <wwdebug/wwdebug.h>

namespace WWOnline {

SquadData::SquadColl SquadData::_mSquadColl;

/******************************************************************************
*
* NAME
*     SquadData::Reset
*
* DESCRIPTION
*     Release all cached squads
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void SquadData::Reset(void)
	{
	_mSquadColl.clear();
	}


/******************************************************************************
*
* NAME
*     SquadData::FindByID
*
* DESCRIPTION
*     Find a squad in the local cache by its ID
*
* INPUTS
*     SquadID - ID of squad to search for.
*
* RESULT
*     Squad - Reference to squad 
*
******************************************************************************/

RefPtr<SquadData> SquadData::FindByID(unsigned int id)
	{
	const size_t count = _mSquadColl.size();

	for (size_t index = 0; index < count; ++index)
		{
		const RefPtr<SquadData>& squad = _mSquadColl[index];
		WWASSERT(squad.IsValid());

		if (squad->GetID() == id)
			{
			return squad;
			}
		}

	return NULL;
	}


/******************************************************************************
*
* NAME
*     SquadData::FindByAbbr
*
* DESCRIPTION
*     Find a squad in the local cache by its abbreviation.
*
* INPUTS
*     Abbr - Abbreviation of squad to search for.
*
* RESULT
*     Squad - Reference to squad 
*
******************************************************************************/

RefPtr<SquadData> SquadData::FindByAbbr(const unichar_t* abbr)
	{
	if (abbr && (u_strlen(abbr) > 0))
		{
		char squadAbbr[64];
		u_wstomb(squadAbbr, abbr, sizeof(squadAbbr));
		squadAbbr[sizeof(squadAbbr) - 1] = 0;

		const size_t count = _mSquadColl.size();

		for (size_t index = 0; index < count; ++index)
			{
			const RefPtr<SquadData>& squad = _mSquadColl[index];
			WWASSERT(squad.IsValid());

			if ((strcmp(squad->GetAbbr(), squadAbbr) == 0))
				{
				return squad;
				}
			}
		}

	return NULL;
	}


/******************************************************************************
*
* NAME
*     SquadData::Create
*
* DESCRIPTION
*     Create a squad data
*
* INPUTS
*     WOLSquad - WOLAPI squad data structure
*
* RESULT
*     SquadData - Instance of squad data.
*
******************************************************************************/

RefPtr<SquadData> SquadData::Create(const WOL::Squad& wolSquad)
	{
	if (wolSquad.id == 0)
		{
		return NULL;
		}

	RefPtr<SquadData> squad = FindByID(wolSquad.id);

	if (squad.IsValid())
		{
		squad->UpdateData(wolSquad);
		}
	else
		{
		squad = new SquadData(wolSquad);
		_mSquadColl.push_back(squad);
		}

	return squad;
	}


/******************************************************************************
*
* NAME
*     SquadData::SquadData
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     WOLSquad - WOLAPI squad structure
*
* RESULT
*     NONE
*
******************************************************************************/

SquadData::SquadData(const WOL::Squad& squad)
	{
	WWDEBUG_SAY(("WOL: Instantiating SquadData '%s'\n", (char*)squad.name));
	UpdateData(squad);
	}


/******************************************************************************
*
* NAME
*     SquadData::~SquadData
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

SquadData::~SquadData()
	{
	WWDEBUG_SAY(("WOL: Destructing SquadData '%s'\n", (char*)mData.name));
	}


/******************************************************************************
*
* NAME
*     SquadData::UpdateData
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void SquadData::UpdateData(const WOL::Squad& squad)
	{
	memcpy(&mData, &squad, sizeof(mData));
	mData.next = NULL;
	}


/******************************************************************************
*
* NAME
*     SquadData::SetLadder
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void SquadData::SetLadder(const RefPtr<LadderData>& ladder)
	{
	mLadder = ladder;
	}

} // namespace WWOnline
