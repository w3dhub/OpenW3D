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
 *                     $Archive:: /Commando/Code/commando/changeteamevent.cpp                    $* 
 *                                                                                             * 
 *                      $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 1/17/02 3:30p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 22                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "changeteamevent.h"

#include <stdio.h>
#include <stdlib.h>

#include "networkobjectfactory.h"
#include "cnetwork.h"
#include "networkobjectmgr.h"
#include "playertype.h"
#include "playermanager.h"
#include "gamemode.h"
#include "gamedata.h"
#include "gameobjmanager.h"
#include "spawn.h"
#include "apppackettypes.h"
#include "sctextobj.h"
#include "translatedb.h"
#include "string_ids.h"
#include "c4.h"
#include "beacongameobj.h"
#include "weaponview.h"


DECLARE_NETWORKOBJECT_FACTORY(cChangeTeamEvent, NETCLASSID_CHANGETEAMEVENT);

//-----------------------------------------------------------------------------
cChangeTeamEvent::cChangeTeamEvent(void)
{
	SenderId = 0;

	Set_App_Packet_Type(APPPACKETTYPE_CHANGETEAMEVENT);
}

//-----------------------------------------------------------------------------
void
cChangeTeamEvent::Init(void)
{
	WWASSERT(cNetwork::I_Am_Client());

	SenderId = cNetwork::Get_My_Id();

	Set_Network_ID(NetworkObjectMgrClass::Get_New_Client_ID());

	if (cNetwork::I_Am_Server()) {
		Act();
	} else {
		Set_Object_Dirty_Bit(0, BIT_CREATION, true);
	}
}

//-----------------------------------------------------------------------------
void
cChangeTeamEvent::Act(void)
{
   WWASSERT(cNetwork::I_Am_Server());

	cPlayer * p_player = cPlayerManager::Find_Player(SenderId);

	if (p_player != NULL && 
		 The_Game() != NULL && 
		 //The_Game()->Is_Team_Game() && 
		 (The_Game()->IsTeamChangingAllowed.Is_True() || p_player->Invulnerable.Is_True()))
	{
		int team = p_player->Get_Player_Type();

		WWASSERT(team == PLAYERTYPE_NOD || team == PLAYERTYPE_GDI);

		int new_team = PLAYERTYPE_NOD;
		
		if (team == PLAYERTYPE_NOD) 
		{
			new_team = PLAYERTYPE_GDI;
		} 
		else 
		{
			new_team = PLAYERTYPE_NOD;
		}

		p_player->Set_Player_Type(new_team);

		WWDEBUG_SAY(("Client %d changed team.\n", SenderId));

		//
		//	Only reset the player's cash if they've changed teams after 60 seconds
		// have elapsed.
		//
		DWORD playing_time = (TIMEGETTIME() - p_player->Get_Join_Time ());
		if (playing_time > 30000) {
			p_player->Set_Score(0);
			p_player->Set_Money(0);
		}

		SoldierGameObj * p_soldier = GameObjManager::Find_Soldier_Of_Client_ID(SenderId);
		if (p_soldier != NULL)
		{

			if ( COMBAT_STAR == p_soldier ) {
				WeaponViewClass::Reset();
			}

			// Defuse all C4 belonging to this guy
			SLNode<BaseGameObj> *objnode;
			for (	objnode = GameObjManager::Get_Game_Obj_List()->Head(); objnode; objnode = objnode->Next()) {
				PhysicalGameObj *obj = objnode->Data()->As_PhysicalGameObj();
				if ( obj && obj->As_C4GameObj() ) {
					if ( obj->As_C4GameObj()->Get_Owner() == p_soldier ) {
						obj->As_C4GameObj()->Defuse();
					}
				}
				if ( obj && obj->As_BeaconGameObj() ) {
					if ( obj->As_BeaconGameObj()->Get_Owner() == p_soldier ) {
						// disarm C4
						OffenseObjectClass unused (0.0F, 0);
						obj->As_BeaconGameObj()->Completely_Damaged( unused );
					}
				}
			}



			//
			// We have to respawn him and possibly change his model... 
			// let's just destroy the soldier and leave the rest up to God.
			//
			p_soldier->Set_Delete_Pending();
		}

		//
		// Tell everyone
		//
		WideStringClass text;
		//text.Format(U_CHAR("_%s_changed_teams!_"), p_player->Get_Name());
		text.Format(U_CHAR("%s %s"), p_player->Get_Name(), TRANSLATE(IDS_MP_CHANGED_TEAMS));

		cScTextObj * p_message = new cScTextObj;
		p_message->Init(text, TEXT_MESSAGE_PUBLIC, false, HOST_TEXT_SENDER, -1);
	}

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cChangeTeamEvent::Export_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Only_Client());

	cNetEvent::Export_Creation(packet);

	WWASSERT(SenderId > 0);

	packet.Add(SenderId);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cChangeTeamEvent::Import_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Server());

	cNetEvent::Import_Creation(packet);

	packet.Get(SenderId);

	WWASSERT(SenderId > 0);

	Act();
}

//-----------------------------------------------------------------------------
bool 
cChangeTeamEvent::Is_Change_Team_Possible(void)
{
	return
		cNetwork::I_Am_Client() &&
		GameModeManager::Find("Combat") != NULL &&
		GameModeManager::Find("Combat")->Is_Active() &&
		The_Game() != NULL && 
		//The_Game()->Is_Team_Game() && 
		The_Game()->IsTeamChangingAllowed.Is_True();
}














