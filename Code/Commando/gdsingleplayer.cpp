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

/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Commando/gdsingleplayer.cpp                  $* 
 *                                                                                             * 
 *                      $Author:: Denzil_l                                                    $* 
 *                                                                                             * 
 *                     $Modtime:: 12/13/01 2:35p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 22                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "gdsingleplayer.h"  // I WANNA BE FIRST!

#include "cnetwork.h"
#include "translatedb.h"
#include "string_ids.h"
#include "playertype.h"

//-----------------------------------------------------------------------------
cGameDataSinglePlayer::cGameDataSinglePlayer(void) : cGameData()
{
   //WWDEBUG_SAY(("cGameDataSinglePlayer::cGameDataSinglePlayer\n"));

	IsFriendlyFirePermitted.Set(true);
	IsTeamChangingAllowed.Set(false);
	SpawnWeapons.Set(true);

	Set_Max_Players(1);
}

//-----------------------------------------------------------------------------
cGameDataSinglePlayer::~cGameDataSinglePlayer(void)
{
   //WWDEBUG_SAY(("cGameDataSinglePlayer::~cGameDataSinglePlayer\n"));
}

//-----------------------------------------------------------------------------
cGameDataSinglePlayer& cGameDataSinglePlayer::operator=(const cGameDataSinglePlayer& rhs)
{
	// 
	// Call the base class
	//
	cGameData::operator=(rhs);

	return (*this);
}

//-----------------------------------------------------------------------------
const unichar_t* cGameDataSinglePlayer::Get_Static_Game_Name(void)
{
	return TRANSLATION(IDS_MP_GAME_TYPE_SINGLE_PLAYER);
}

//-----------------------------------------------------------------------------
int cGameDataSinglePlayer::Choose_Player_Type(cPlayer* /* player */, int /* team_choice */, bool is_grunt)
{
	WWASSERT(cNetwork::I_Am_Server());

	int team;

	if (is_grunt) {
		team = PLAYERTYPE_NOD;
	} else {
		team = PLAYERTYPE_GDI;
	}

	return team;
}





