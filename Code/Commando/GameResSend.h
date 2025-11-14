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
*     $Archive: /Commando/Code/Commando/GameResSend.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 2 $
*     $Modtime: 8/16/01 7:03p $
*
******************************************************************************/

#ifndef __GAMERESSEND_H__
#define __GAMERESSEND_H__

#include <wwlib/slist.h>

class cGameData;
class cPlayer;

void SendGameResults(unsigned int gameID, cGameData* theGame, SList<cPlayer>* players);

#ifdef _DEBUG
void SendTestGameResults(void);
#endif

#endif // __GAMERESSEND_H__
