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
// Filename:     playertype.cpp
// Author:       Byon Garrabrant
// Date:         Jun 2001
// Description:
//
//-----------------------------------------------------------------------------

#include "playertype.h"

const char * Player_Type_Name( int player_type )
{
	switch( player_type ) {
		case 	PLAYERTYPE_SPECTATOR:	return "Spectator";
   	case 	PLAYERTYPE_MUTANT:		return "Mutant";
		case 	PLAYERTYPE_NEUTRAL:		return "Neutral";
		case 	PLAYERTYPE_RENEGADE:		return "Renegade";
		case 	PLAYERTYPE_NOD:			return "NOD";
		case 	PLAYERTYPE_GDI:			return "GDI";
	}
	return "Unknown";
}

