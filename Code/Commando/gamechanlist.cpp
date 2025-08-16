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
// Filename:     gamechanlist.cpp
// Project:      Network.lib, for Commando
// Author:       Tom Spencer-Smith
// Date:         Dec 1998
// Description:  
//

#include "gamedata.h"
#include "gamechannel.h"
#include "gamechanlist.h"
#include "wwdebug.h"
#include <WWOnline/WOLChannel.h>
//
// Class statics
//
SList<cGameChannel> cGameChannelList::ChanList;

//-----------------------------------------------------------------------------
void cGameChannelList::Add_Channel(cGameData * p_game_data, const RefPtr<WWOnline::ChannelData>& p_channel)
{
	WWASSERT(p_game_data != NULL);

   cGameChannel * p_game_channel = Find_Channel(p_game_data->Get_Owner());
	if (p_game_channel == NULL) {
		cGameChannel * p_game_channel = new cGameChannel(p_game_data, p_channel);
		WWASSERT(p_game_channel != NULL);
		ChanList.Add_Tail(p_game_channel);
   } else {
		//
		// Update any dynamic data
		// 
		p_game_channel->Get_Game_Data()->Set_Current_Players(
			p_game_data->Get_Current_Players());
		
		p_game_channel->Get_Game_Data()->Set_Map_Name(
			p_game_data->Get_Map_Name());

		p_game_channel->WolChannel = p_channel;

		delete p_game_data;
	}
}

//-----------------------------------------------------------------------------
cGameChannel * cGameChannelList::Find_Channel(const WideStringClass & owner)
{
	for (
		SLNode<cGameChannel> * objnode = ChanList.Head(); 
		objnode; 
		objnode = objnode->Next()) {

		cGameChannel * p_channel = objnode->Data();
		WWASSERT(p_channel != NULL);
		if (p_channel->Get_Game_Data()->Get_Owner() == owner) {
			return p_channel;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
void cGameChannelList::Remove_Channel(const WideStringClass & owner)
{
	cGameChannel * p_channel = Find_Channel(owner);
	if (p_channel != NULL) {
		ChanList.Remove(p_channel);
		p_channel->Release_Ref ();
		//delete p_channel;
	}
}

//-----------------------------------------------------------------------------
void cGameChannelList::Remove_All(void)
{
	WWDEBUG_SAY(("cGameChannelList::Remove_All\n"));

	for (
		SLNode<cGameChannel> * objnode = ChanList.Head(); 
		objnode; 
		objnode = objnode->Next()) {

		cGameChannel * p_channel = objnode->Data();
		WWASSERT(p_channel != NULL);
		p_channel->Release_Ref ();
	}

	ChanList.Remove_All();
}


