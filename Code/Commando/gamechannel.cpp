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
// Filename:     gamechannel.cpp
// Project:      Network.lib, for Commando
// Author:       Tom Spencer-Smith
// Date:         Dec 1998
// Description:  
//

#include "gamedata.h"
#include "gamechannel.h"
#include "wwdebug.h"
#include <WWOnline/WOLChannel.h>

//-----------------------------------------------------------------------------
cGameChannel::cGameChannel(cGameData * p_game_data, const RefPtr<WWOnline::ChannelData>& channel)
{
	WWASSERT(p_game_data != NULL);
	PGameData = p_game_data;
	WolChannel = channel;
}

//-----------------------------------------------------------------------------
cGameChannel::~cGameChannel(void)
{
	delete PGameData;
}

WOL::Channel* cGameChannel::Get_Wol_Channel(void)
{
	return &WolChannel->GetData();
}
