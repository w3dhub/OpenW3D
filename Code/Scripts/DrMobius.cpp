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
*
* DESCRIPTION
*
* PROGRAMMER
*     Patrick Smith
*
* VERSION INFO
*     $Author: Patrick $
*     $Revision: 6 $
*     $Modtime: 10/30/00 6:53p $
*     $Archive: /Commando/Code/Scripts/DrMobius.cpp $
*
******************************************************************************/

#include "scripts.h"
#include "DPrint.h"


DECLARE_SCRIPT(Dr_Mobius_Script, "")
{
	GameObject *CurrentLeader;

	////////////////////////////////////////////////////////////////////
	//	Created
	////////////////////////////////////////////////////////////////////
	void Created (GameObject *game_obj) override
	{
		CurrentLeader = NULL;
		Commands->Start_Timer (game_obj, this, 0.5F, 777);
		return ;
	}

	////////////////////////////////////////////////////////////////////
	//	Timer_Expired
	////////////////////////////////////////////////////////////////////
	void Timer_Expired (GameObject *game_obj, int timer_id) override
	{
		if (timer_id == 777) {
			Commands->Innate_Disable (game_obj);
			
			Vector3 pos = Commands->Get_Position(game_obj);
			GameObject * p_leader = Commands->Find_Closest_Soldier(pos, 0.1f, 2.0f, true);
			if (p_leader != NULL && p_leader != CurrentLeader) {
				ActionParamsStruct params;
				params.Set_Basic(this, 100, 100);
				params.Set_Movement(p_leader, 1.0f, 1.0f);
				params.MoveFollow = true;
				Commands->Action_Goto(game_obj, params);
				CurrentLeader = p_leader;
			}

			Commands->Start_Timer (game_obj, this, 0.5F, 777);
		}
		return ;
	}
};

