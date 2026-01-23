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
 *                     $Archive:: /Commando/Code/Commando/nethandler.cpp                      $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 1/08/02 4:01p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 70                                                         $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "nethandler.h"

#include "cnetwork.h"
#include "playermanager.h"
#include "playerkill.h"

//-----------------------------------------------------------------------------
bool GameCombatNetworkHandlerClass::Can_Damage(ArmedGameObj * /* p_armed_damager */, 
	PhysicalGameObj * p_phys_victim)
{
	WWASSERT(p_phys_victim != NULL);

	WWASSERT(PTheGameData != NULL);
	if (!The_Game()->Is_Gameplay_Permitted()) {
		return false;
	}

	SmartGameObj * p_victim_obj = p_phys_victim->As_SmartGameObj();
	if (p_victim_obj != NULL && p_victim_obj->Has_Player()) {
		cPlayer * p_player = cPlayerManager::Find_Player(p_victim_obj->Get_Control_Owner());
		if (p_player && p_player->Invulnerable.Is_True()) {
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
float GameCombatNetworkHandlerClass::Get_Damage_Factor
(
	ArmedGameObj *		/* p_armed_damager */, 
	PhysicalGameObj *	p_phys_victim
)
{
	float factor = 1.0f;

	SmartGameObj * p_victim_obj = p_phys_victim->As_SmartGameObj();
	if (p_victim_obj != NULL && p_victim_obj->Has_Player()) {
		cPlayer * p_player = cPlayerManager::Find_Player(p_victim_obj->Get_Control_Owner());
		if (p_player && p_player->Get_Damage_Scale_Factor() < 100) {
			factor = p_player->Get_Damage_Scale_Factor() / 100.0f;
		}
	}

	return factor;
}

//-----------------------------------------------------------------------------
void GameCombatNetworkHandlerClass::On_Soldier_Kill(SoldierGameObj * p_soldier, SoldierGameObj * p_victim)
{
	WWASSERT(p_soldier != NULL);
	WWASSERT(p_victim != NULL);

	WWASSERT(cNetwork::I_Am_Server());

	int killer_id = p_soldier->Get_Control_Owner();
	int victim_id = p_victim->Get_Control_Owner();

	if (p_soldier != p_victim) {
		cPlayer * p_soldier_player = NULL;
		if (p_soldier->Has_Player()) {
			p_soldier_player = cPlayerManager::Find_Player(killer_id);
			if (p_soldier_player != NULL) {
				p_soldier_player->Increment_Kills();
			}
		}

		cPlayer * p_victim_player = NULL;
		if (p_victim->Has_Player()) {
			p_victim_player = cPlayerManager::Find_Player(victim_id);
		}

		//
		// Send A killed B message (handled independently from scoring changes)
		// This presently includes AI killings
		//
		if (p_soldier_player != NULL && p_victim_player != NULL) {
			cPlayerKill * p_player_kill = new cPlayerKill;
			p_player_kill->Init(killer_id, victim_id);
		}

		if (p_soldier->Is_Team_Player()) {
			cTeam * p_team = cTeamManager::Find_Team(p_soldier->Get_Player_Type());
			WWASSERT(p_team != NULL);
			p_team->Increment_Kills();
		}
	}
}

//-----------------------------------------------------------------------------
void GameCombatNetworkHandlerClass::On_Soldier_Death(SoldierGameObj * p_soldier)
{
	WWASSERT(p_soldier != NULL);
	WWASSERT(cNetwork::I_Am_Server());

	if (p_soldier->Has_Player()) {
		cPlayer * p_player = cPlayerManager::Find_Player(p_soldier->Get_Control_Owner());
		if (p_player != NULL) {
			p_player->Increment_Deaths();
		}
	}

	if (p_soldier->Is_Team_Player()) {
		cTeam * p_team = cTeamManager::Find_Team(p_soldier->Get_Player_Type());
		WWASSERT(p_team != NULL);
		p_team->Increment_Deaths();
	}
}

//-----------------------------------------------------------------------------
bool GameCombatNetworkHandlerClass::Is_Gameplay_Permitted(void)
{
	WWASSERT(The_Game() != NULL);
	return The_Game()->Is_Gameplay_Permitted();
}



























	/*
	if (p_soldier->Has_Player()) {
		cPlayer * p_player = cPlayerManager::Find_Player(p_soldier->Get_Control_Owner());
		if (p_player != NULL) {
			p_player->Increment_Deaths();

			if (p_player->Is_Team_Player()) {
				cTeam * p_team = cTeamManager::Find_Team(p_player->Get_Player_Type());
				if (p_team != NULL) {
					p_team->Increment_Deaths();
				}
			}
		}
	}
	*/

