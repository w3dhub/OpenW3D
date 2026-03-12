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
 *                     $Archive:: /Commando/Code/Combat/gametype.h                              $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 11/09/01 3:42p                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef GAMETYPE_H
#define GAMETYPE_H

//-----------------------------------------------------------------------------

#define	IS_MISSION				cGameType::Is_Mission()
#define	IS_SKIRMISH				cGameType::Is_Skirmish()
#define	IS_MULTIPLAY			cGameType::Is_Multiplay()
#define	IS_SOLOPLAY				cGameType::Is_Soloplay()

enum GameTypeEnum
{
	GAMETYPE_NONE,				// Unassigned
	GAMETYPE_MISSION,			// Traditional soloplay
	GAMETYPE_SKIRMISH,		// C&C practice against AI's
	GAMETYPE_MULTIPLAY,		// C&C against humans
};

//-----------------------------------------------------------------------------
class cGameType
{
public:
	static void				Set_Game_Type(GameTypeEnum game_type)	{GameType = game_type;}
	static GameTypeEnum	Get_Game_Type(void)							{return GameType;}

	static bool				Is_Mission(void)								{return GameType == GAMETYPE_MISSION;}
	static bool				Is_Skirmish(void)								{return GameType == GAMETYPE_SKIRMISH;}
	static bool				Is_Multiplay(void)							{return GameType == GAMETYPE_MULTIPLAY;}
	static bool				Is_Soloplay(void)								{return GameType != GAMETYPE_MULTIPLAY;}

private:
	static GameTypeEnum	GameType;
};

//-----------------------------------------------------------------------------

#endif // GAMETYPE_H

