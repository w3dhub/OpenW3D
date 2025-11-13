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

//
// Filename:     msgstatlist.cpp
// Project:      
// Author:       Tom Spencer-Smith
// Date:         
// Description:  
//
//------------------------------------------------------------------------------------
#include "msgstatlist.h" // I WANNA BE FIRST!

#include "mathutil.h"
#include "wwdebug.h"

//
// Class statics
//

//------------------------------------------------------------------------------------
cMsgStatList::cMsgStatList(void) :
	NumStats(0)
{
}

//---------------- --------------------------------------------------------------------
cMsgStatList::~cMsgStatList(void)
{
	if (PStat != NULL) {
		delete [] PStat;
		PStat = NULL;
	}
}

//-----------------------------------------------------------------------------
void cMsgStatList::Init(int num_stats)
{
	WWASSERT(num_stats > 0);

	NumStats = num_stats;
	PStat = new cMsgStat[NumStats + 1];
	WWASSERT(PStat != NULL);
}

//-----------------------------------------------------------------------------
void cMsgStatList::Increment_Num_Msg_Sent(int message_type, int increment)
{
	WWASSERT(message_type >= 0 && message_type < NumStats);
	WWASSERT(increment > 0); 

	PStat[message_type].Increment_Num_Msg_Sent(increment);
	PStat[NumStats].Increment_Num_Msg_Sent(increment);
}

//-----------------------------------------------------------------------------
void cMsgStatList::Increment_Num_Byte_Sent(int message_type, int increment)
{
	WWASSERT(message_type >= 0 && message_type < NumStats);
	WWASSERT(increment > 0); 

	PStat[message_type].Increment_Num_Byte_Sent(increment);
	PStat[NumStats].Increment_Num_Byte_Sent(increment);
}

//-----------------------------------------------------------------------------
void cMsgStatList::Increment_Num_Msg_Recd(int message_type, int increment)
{
	WWASSERT(message_type >= 0 && message_type < NumStats);
	WWASSERT(increment > 0); 

	PStat[message_type].Increment_Num_Msg_Recd(increment);
	PStat[NumStats].Increment_Num_Msg_Recd(increment);
}

//-----------------------------------------------------------------------------
void cMsgStatList::Increment_Num_Byte_Recd(int message_type, int increment)
{
	WWASSERT(message_type >= 0 && message_type < NumStats);
	WWASSERT(increment > 0); 

	PStat[message_type].Increment_Num_Byte_Recd(increment);
	PStat[NumStats].Increment_Num_Byte_Recd(increment);
}

//-----------------------------------------------------------------------------
unsigned int cMsgStatList::Get_Num_Msg_Sent(int message_type) const
{
	if (message_type == ALL_MESSAGES) {
		message_type = NumStats;
	}

	WWASSERT(message_type >= 0 && message_type <= NumStats);
	return PStat[message_type].Get_Num_Msg_Sent();
}

//-----------------------------------------------------------------------------
unsigned int cMsgStatList::Get_Num_Byte_Sent(int message_type) const
{
	if (message_type == ALL_MESSAGES) {
		message_type = NumStats;
	}

	WWASSERT(message_type >= 0 && message_type <= NumStats);
	return PStat[message_type].Get_Num_Byte_Sent();
}

//-----------------------------------------------------------------------------
unsigned int cMsgStatList::Get_Num_Msg_Recd(int message_type) const
{
	if (message_type == ALL_MESSAGES) {
		message_type = NumStats;
	}

	WWASSERT(message_type >= 0 && message_type <= NumStats);
	return PStat[message_type].Get_Num_Msg_Recd();
}

//-----------------------------------------------------------------------------
unsigned int cMsgStatList::Get_Num_Byte_Recd(int message_type) const
{
	if (message_type == ALL_MESSAGES) {
		message_type = NumStats;
	}

	WWASSERT(message_type >= 0 && message_type <= NumStats);
	return PStat[message_type].Get_Num_Byte_Recd();
}

//-----------------------------------------------------------------------------
unsigned int cMsgStatList::Compute_Avg_Num_Byte_Sent(int message_type) const
{
	if (message_type == ALL_MESSAGES) {
		message_type = NumStats;
	}

	WWASSERT(message_type >= 0 && message_type <= NumStats);
	return PStat[message_type].Compute_Avg_Num_Byte_Sent();
}

//-----------------------------------------------------------------------------
unsigned int cMsgStatList::Compute_Avg_Num_Byte_Recd(int message_type) const
{
	if (message_type == ALL_MESSAGES) {
		message_type = NumStats;
	}

	WWASSERT(message_type >= 0 && message_type <= NumStats);
	return PStat[message_type].Compute_Avg_Num_Byte_Recd();
}

//-----------------------------------------------------------------------------
cMsgStat & cMsgStatList::Get_Stat(int message_type)
{
	if (message_type == ALL_MESSAGES) {
		message_type = NumStats;
	}

	WWASSERT(message_type >= 0 && message_type <= NumStats);
	return PStat[message_type];
}

//-----------------------------------------------------------------------------
void cMsgStatList::Set_Name(int message_type, const char *name)
{
	WWASSERT(message_type >= 0 && message_type < NumStats);

	PStat[message_type].Set_Name(name);
}

//-----------------------------------------------------------------------------
const char * cMsgStatList::Get_Name(int message_type) const
{
	WWASSERT(message_type >= 0 && message_type <= NumStats);

	return PStat[message_type].Get_Name();
}
