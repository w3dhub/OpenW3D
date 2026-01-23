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
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : commando                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/commando/renegadecheatmgr.cpp                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/06/01 11:06a                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "renegadecheatmgr.h"
#include "player.h"
#include "playermanager.h"
#include "combat.h"
#include "cnetwork.h"
#include "registry.h"
#include "_globals.h"

//////////////////////////////////////////////////////////////////////
//	Global instance
//////////////////////////////////////////////////////////////////////
static RenegadeCheatMgrClass _TheCheatMgr;


//////////////////////////////////////////////////////////////////////
//	Constants
//////////////////////////////////////////////////////////////////////
//static const char *KEY_NAME_SETTINGS	= "Software\\Westwood\\Renegade\\Options";
static const char *VALUE_NAME_CHEATS	= "Cheats";


//////////////////////////////////////////////////////////////////////
//
//	RenegadeCheatMgrClass
//
//////////////////////////////////////////////////////////////////////
RenegadeCheatMgrClass::RenegadeCheatMgrClass (void)
{
	//
	//	Attempt to open the registry key
	//
	RegistryClass registry (APPLICATION_SUB_KEY_NAME_OPTIONS);
	if (registry.Is_Valid ()) {

		//
		//	Read the values from the registry
		//
		//Flags = registry.Get_Int (VALUE_NAME_CHEATS, 0);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~RenegadeCheatMgrClass
//
//////////////////////////////////////////////////////////////////////
RenegadeCheatMgrClass::~RenegadeCheatMgrClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Enable_Cheat
//
//////////////////////////////////////////////////////////////////////
void
RenegadeCheatMgrClass::Enable_Cheat (int /* cheat */, bool /* onoff */)
{
#if 0

	CheatMgrClass::Enable_Cheat (cheat, onoff);

	//
	//	Save the values to the registry
	//
	RegistryClass registry (APPLICATION_SUB_KEY_NAME_OPTIONS);
	if (registry.Is_Valid ()) {
		registry.Set_Int (VALUE_NAME_CHEATS, Flags);
	}

	if (cheat == CHEAT_INVULNERABILITY) {

		//
		//	This may seem strange, but we only want to allow cheating in single
		// player and we need to be a client (which means we are a player).
		//
		if (IS_SOLOPLAY && CombatManager::I_Am_Client ()) {
			
			//
			//	Make the current player invulnerable or not
			//
			cPlayer *player = cPlayerManager::Find_Player (cNetwork::Get_My_Id ());
			if (player != NULL) {
				player->Invulnerable.Set (onoff);
				player->Mark_As_Modified();
			}
		}

	} else if (cheat == CHEAT_ALL_WEAPONS) {

		//
		// Grant all weapons to the player
		//
		if (onoff && IS_SOLOPLAY && COMBAT_STAR != NULL) {
			COMBAT_STAR->Give_All_Weapons ();
		}
	}

#endif //0

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Apply_Cheats
//
//////////////////////////////////////////////////////////////////////
void
RenegadeCheatMgrClass::Apply_Cheats (void)
{

#if 0
	CheatMgrClass::Apply_Cheats ();

	//
	//	Just force the options to be re-enabled
	//
	Enable_Cheat (CHEAT_INVULNERABILITY,	Is_Cheat_Set (CHEAT_INVULNERABILITY));
	Enable_Cheat (CHEAT_INFINITE_AMMO,		Is_Cheat_Set (CHEAT_INFINITE_AMMO));
	Enable_Cheat (CHEAT_ALL_WEAPONS,			Is_Cheat_Set (CHEAT_ALL_WEAPONS));
#endif //0

	return ;
}
