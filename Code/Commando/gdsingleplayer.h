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
 *                     $Archive:: /Commando/Code/Commando/gdsingleplayer.h                         $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 4/12/02 4:00p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 18                                                         $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef GDSINGLEPLAYER_H
#define GDSINGLEPLAYER_H

#include "gamedata.h"

class	cGameDataSinglePlayer : public cGameData {
	public:
      cGameDataSinglePlayer(void);
      ~cGameDataSinglePlayer(void);
      cGameDataSinglePlayer& operator=(const cGameDataSinglePlayer& rhs);

		bool	Is_Single_Player(void) const override						{return true;}
		cGameDataSinglePlayer * As_Single_Player(void) override		{return this;}

		static const unichar_t* Get_Static_Game_Name(void);

		virtual const unichar_t*	Get_Game_Name(void)					const override {return this->Get_Static_Game_Name();}
      virtual GameTypeEnum	Get_Game_Type(void)					const override	{return GAME_TYPE_SINGLE_PLAYER;}
		//virtual bool	Is_Team_Game(void)							const	{return true;}
		virtual bool	Is_Limited(void)								const override	{return true;}
		virtual int		Choose_Player_Type(cPlayer* player, int team_choice, bool is_grunt) override;
		virtual	bool	Remember_Inventory( void )	const override				{ return true; }

};

#endif	// GDSINGLEPLAYER_H





