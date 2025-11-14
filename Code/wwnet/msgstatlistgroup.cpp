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
// Filename:     msgstatlistgroup.cpp
// Project:      
// Author:       Tom Spencer-Smith
// Date:         
// Description:  
//
//------------------------------------------------------------------------------------
#include "msgstatlistgroup.h" // I WANNA BE FIRST!

#include "mathutil.h"
#include "wwdebug.h"

//
// Class statics
//

//------------------------------------------------------------------------------------
cMsgStatListGroup::cMsgStatListGroup(void) :
	NumLists(0)
{
}

//------------------------------------------------------------------------------------
cMsgStatListGroup::~cMsgStatListGroup(void)
{
	if (PStatList != NULL) {
		delete [] PStatList;
		PStatList = NULL;
	}
}

//-----------------------------------------------------------------------------
void cMsgStatListGroup::Init(int num_lists, int num_stats)
{
	WWASSERT(num_lists > 0);

	NumLists = num_lists;
	PStatList = new cMsgStatList[NumLists + 1];
	WWASSERT(PStatList != NULL);

	for (int i = 0; i < NumLists + 1; i++) {
		PStatList[i].Init(num_stats);
	}
}

//-----------------------------------------------------------------------------
void cMsgStatListGroup::Increment_Num_Msg_Sent(int list_num, int message_type, int increment)
{
	WWASSERT(list_num >= 0 && list_num < NumLists);
	WWASSERT(increment > 0); 

	PStatList[list_num].Increment_Num_Msg_Sent(message_type, increment);
	PStatList[NumLists].Increment_Num_Msg_Sent(message_type, increment);
}

//-----------------------------------------------------------------------------
void cMsgStatListGroup::Increment_Num_Byte_Sent(int list_num, int message_type, int increment)
{
	WWASSERT(list_num >= 0 && list_num < NumLists);
	WWASSERT(increment > 0); 

	PStatList[list_num].Increment_Num_Byte_Sent(message_type, increment);
	PStatList[NumLists].Increment_Num_Byte_Sent(message_type, increment);
}

//-----------------------------------------------------------------------------
void cMsgStatListGroup::Increment_Num_Msg_Recd(int list_num, int message_type, int increment)
{
	WWASSERT(list_num >= 0 && list_num < NumLists);
	WWASSERT(increment > 0); 

	PStatList[list_num].Increment_Num_Msg_Recd(message_type, increment);
	PStatList[NumLists].Increment_Num_Msg_Recd(message_type, increment);
}

//-----------------------------------------------------------------------------
void cMsgStatListGroup::Increment_Num_Byte_Recd(int list_num, int message_type, int increment)
{
	WWASSERT(list_num >= 0 && list_num < NumLists);
	WWASSERT(increment > 0); 

	PStatList[list_num].Increment_Num_Byte_Recd(message_type, increment);
	PStatList[NumLists].Increment_Num_Byte_Recd(message_type, increment);
}

//-----------------------------------------------------------------------------
unsigned int cMsgStatListGroup::Get_Num_Msg_Sent(int list_num, int message_type) const
{
	if (list_num == ALL_LISTS) {
		list_num = NumLists;
	}

	WWASSERT(list_num >= 0 && list_num <= NumLists);
	WWASSERT(message_type >= 0 && message_type <= ALL_MESSAGES);

	return PStatList[list_num].Get_Num_Msg_Sent(message_type);
}

//-----------------------------------------------------------------------------
unsigned int cMsgStatListGroup::Get_Num_Byte_Sent(int list_num, int message_type) const
{
	if (list_num == ALL_LISTS) {
		list_num = NumLists;
	}

	WWASSERT(list_num >= 0 && list_num <= NumLists);
	WWASSERT(message_type >= 0 && message_type <= ALL_MESSAGES);

	return PStatList[list_num].Get_Num_Byte_Sent(message_type);
}

//-----------------------------------------------------------------------------
unsigned int cMsgStatListGroup::Get_Num_Msg_Recd(int list_num, int message_type) const
{
	if (list_num == ALL_LISTS) {
		list_num = NumLists;
	}

	WWASSERT(list_num >= 0 && list_num <= NumLists);
	WWASSERT(message_type >= 0 && message_type <= ALL_MESSAGES);

	return PStatList[list_num].Get_Num_Msg_Recd(message_type);
}

//-----------------------------------------------------------------------------
unsigned int cMsgStatListGroup::Get_Num_Byte_Recd(int list_num, int message_type) const
{
	if (list_num == ALL_LISTS) {
		list_num = NumLists;
	}

	WWASSERT(list_num >= 0 && list_num <= NumLists);
	WWASSERT(message_type >= 0 && message_type <= ALL_MESSAGES);

	return PStatList[list_num].Get_Num_Byte_Recd(message_type);
}

//-----------------------------------------------------------------------------
unsigned int cMsgStatListGroup::Compute_Avg_Num_Byte_Sent(int list_num, int message_type) const
{
	if (list_num == ALL_LISTS) {
		list_num = NumLists;
	}

	WWASSERT(list_num >= 0 && list_num <= NumLists);
	WWASSERT(message_type >= 0 && message_type <= ALL_MESSAGES);

	return PStatList[list_num].Compute_Avg_Num_Byte_Sent(message_type);
}

//-----------------------------------------------------------------------------
unsigned int cMsgStatListGroup::Compute_Avg_Num_Byte_Recd(int list_num, int message_type) const
{
	if (list_num == ALL_LISTS) {
		list_num = NumLists;
	}

	WWASSERT(list_num >= 0 && list_num <= NumLists);
	WWASSERT(message_type >= 0 && message_type <= ALL_MESSAGES);

	return PStatList[list_num].Compute_Avg_Num_Byte_Recd(message_type);
}

//-----------------------------------------------------------------------------
cMsgStatList * cMsgStatListGroup::Get_Stat_List(int list_num)
{
	if (list_num == ALL_LISTS) {
		list_num = NumLists;
	}

	WWASSERT(list_num >= 0 && list_num <= NumLists);

	return &PStatList[list_num];
}

//-----------------------------------------------------------------------------
void cMsgStatListGroup::Set_Name(int message_type, const char *name)
{
	for (int i = 0; i <= NumLists; i++) {
		PStatList[i].Set_Name(message_type, name);
	}
}

