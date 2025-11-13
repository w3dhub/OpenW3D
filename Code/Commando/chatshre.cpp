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
// Filename:     chatshre.cpp
// Project:      Network.lib, for Commando
// Author:       Tom Spencer-Smith
// Date:         Dec 1998
// Description:  
//
//-----------------------------------------------------------------------------
#include "chatshre.h"

#include "wwdebug.h"

//-----------------------------------------------------------------------------

#define ADD_CASE(exp) case exp: return #exp; break;  
const char *Translate_Location(ChatLocationEnum location)
{
   switch (location) {
		
		ADD_CASE(WOLLOC_EXIT);
		ADD_CASE(WOLLOC_NOLIST);
		ADD_CASE(WOLLOC_LOBBYLIST);
		ADD_CASE(WOLLOC_LOBBY);
		ADD_CASE(WOLLOC_GAMESLIST);
		ADD_CASE(WOLLOC_GAMEDATA);
		ADD_CASE(WOLLOC_INGAME);

		ADD_CASE(LANLOC_EXIT);
		ADD_CASE(LANLOC_LOBBY);
		ADD_CASE(LANLOC_GAMESLIST);
		ADD_CASE(LANLOC_GAMEDATA);
		ADD_CASE(LANLOC_INGAME);

		default: 
			WWASSERT(0);
			return ""; // to avoid compiler warning
	}
}
