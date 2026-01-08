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
*     $Archive: /Commando/Code/WWOnline/WOLLadder.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 7 $
*     $Modtime: 12/03/01 3:22p $
*
******************************************************************************/

#include <string.h>
#include <memory.h>
#include "WOLLadder.h"
#include <wwdebug/wwdebug.h>

namespace WWOnline {

/******************************************************************************
*
* NAME
*     LadderData::Create
*
* DESCRIPTION
*
* INPUTS
*     WOLLadder - WOLAPI ladder structure
*
* RESULT
*     Ladder - 
*
******************************************************************************/

RefPtr<LadderData> LadderData::Create(const WOL::Ladder& ladder, int time)
	{
	return new LadderData(ladder, time);
	}


/******************************************************************************
*
* NAME
*     LadderData::LadderData
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     WOLLadder - WOLAPI ladder structure
*
* RESULT
*     NONE
*
******************************************************************************/

LadderData::LadderData(const WOL::Ladder& ladder, int time) :
		mTimeStamp(time)
	{
//	WWDEBUG_SAY(("WOL: Instantiating LadderData\n"));
	memcpy(&mData, &ladder, sizeof(mData));
	mData.next = NULL;
	}


/******************************************************************************
*
* NAME
*     LadderData::~LadderData
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

LadderData::~LadderData()
	{
//	WWDEBUG_SAY(("WOL: Destructing LadderData\n"));
	}


/******************************************************************************
*
* NAME
*     LadderData::UpdateData
*
* DESCRIPTION
*     Update the ladder information
*
* INPUTS
*     WOLLadder - WOLAPI ladder structure
*
* RESULT
*     Updated - True if ladder data was updated.
*
******************************************************************************/

bool LadderData::UpdateData(const WOL::Ladder& ladder, int time)
	{
	if (time > mTimeStamp)
		{
		memcpy(&mData, &ladder, sizeof(mData));
		mData.next = NULL;
		return true;
		}

	return false;
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
*     NONE
*
******************************************************************************/

LadderInfoEvent::LadderInfoEvent(const unichar_t* requested, const WOL::Ladder& ladder, int time) :
		mRequestedName(requested),
		mWOLLadder(ladder),
		mTimeStamp(time)
	{
	}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

bool LadderInfoEvent::IsRanked(void) const
	{
	return (strlen((const char*)mWOLLadder.login_name) > 0);
	}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

LadderType LadderInfoEvent::GetLadderType(void) const
	{
	LadderType type = (LadderType)(mWOLLadder.sku & LADDERTYPE_MASK);

	if (type == 0)
		{
		type = LadderType_Individual;
		}

	return type;
	}

} // namespace WWOnline
