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
// Filename:     gamechanlist.h
// Project:      Network.lib, for Commando
// Author:       Tom Spencer-Smith
// Date:         Dec 1998
// Description:  
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef GAMECHANLIST_H
#define GAMECHANLIST_H

#include "always.h"
#include "bittype.h"
#include <WWOnline/RefPtr.h>

class cGameData;
class cGameChannel;

namespace WWOnline
{
class ChannelData;
}

#include "slist.h"

class WideStringClass;

//-----------------------------------------------------------------------------
class cGameChannelList
{
   public:
		 static void Add_Channel(cGameData * p_game_data, const RefPtr<WWOnline::ChannelData>& channel = RefPtr<WWOnline::ChannelData>());
		static void Remove_Channel(const WideStringClass & owner);
		static void Remove_All(void);
		static cGameChannel * Find_Channel(const WideStringClass & owner);
		static SList<cGameChannel> * Get_Chan_List(void) {return &ChanList;}

   private:
		static SList<cGameChannel> ChanList;
};	

//-----------------------------------------------------------------------------
#endif // GAMECHANLIST_H





