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
// Filename:     chatshre.h
// Project:      Network.lib, for Commando
// Author:       Tom Spencer-Smith
// Date:         Dec 1998
// Description:  
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef CHATSHRE_H
#define CHATSHRE_H

#include "bittype.h"

//-----------------------------------------------------------------------------
enum ChatLocationEnum {
   
   //
	// Wol interface
	//
   WOLLOC_EXIT,
   WOLLOC_NOLIST,
   WOLLOC_LOBBYLIST,
   WOLLOC_LOBBY,
   WOLLOC_GAMESLIST,
	WOLLOC_GAMEDATA,
   WOLLOC_INGAME,

   //
	// Lan interface
	//
	LANLOC_EXIT,
	LANLOC_LOBBY,
	LANLOC_GAMESLIST,
	LANLOC_GAMEDATA,
   LANLOC_INGAME,
};

const char *Translate_Location(ChatLocationEnum location);

//-----------------------------------------------------------------------------

#endif // CHATSHRE_H
  