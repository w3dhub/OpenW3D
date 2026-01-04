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
 *                     $Archive:: /Commando/Code/commando/winevent.cpp               $*
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 2/07/02 9:43a                                               $*
 *                                                                                             *
 *                    $Revision:: 25                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "winevent.h"

#include <stdio.h>
#include <stdlib.h>

#include "networkobjectfactory.h"
#include "WWAudio.h"
#include "gamemode.h"
#include "playermanager.h"
#include "matrix3d.h"
#include "cnetwork.h"
#include "gametype.h"
#include "translatedb.h"
#include "string_ids.h"
#include "realcrc.h"
#include "apppackettypes.h"
#include "specialbuilds.h"
#include "modpackagemgr.h"


DECLARE_NETWORKOBJECT_FACTORY(cWinEvent, NETCLASSID_WIN);

//-----------------------------------------------------------------------------
cWinEvent::cWinEvent(void)
{
	Winner				= PLAYER_ID_UNKNOWN;
	Loser					= PLAYER_ID_UNKNOWN;
	IsMapCycleOver		= false;

	HostedGameNumber = -1;

	Set_App_Packet_Type(APPPACKETTYPE_WINEVENT);
}

//-----------------------------------------------------------------------------
void
cWinEvent::Init(int winner, int loser, bool is_cycle_over)
{
	WWASSERT(cNetwork::I_Am_Server());

	Winner			= winner;
	Loser				= loser;
	IsMapCycleOver	= is_cycle_over;

	WWASSERT(The_Game() != NULL);
	HostedGameNumber = The_Game()->Get_Hosted_Game_Number();

	Set_Object_Dirty_Bit(BIT_CREATION, true);

	if (cNetwork::I_Am_Client())
	{
		Act();
	}
}

//-----------------------------------------------------------------------------
void
cWinEvent::Act(void)
{
	if (IS_MISSION || !GameModeManager::Find("Combat")->Is_Active()) {
		return;
	}

	WWASSERT(cNetwork::I_Am_Client());

	WWAudioClass::Get_Instance()->Create_Instant_Sound("Game_Over", Matrix3D(1));

	WideStringClass win_text;

	WWASSERT(PTheGameData != NULL);
	//if (The_Game()->Is_Team_Game()) {

	WideStringClass champ_text;
	champ_text.Format(
		U_CHAR("%s %s"),
		The_Game()->Get_Team_Word(),
		cTeamManager::Get_Team_Name(Winner));

	win_text.Format(
		U_CHAR("%s %s"),
		champ_text,
		TRANSLATION(IDS_MP_WIN_FORMATTING));

	/*
	} else {

		if (Loser == PLAYER_ID_UNKNOWN) {
			win_text.Format(
				U_CHAR("%s %s"),
				cPlayerManager::Get_Player_Name(Winner),
				TRANSLATION(IDS_MP_WIN_FORMATTING));
		} else {
			win_text.Format(
				U_CHAR("%s %s %s"),
				cPlayerManager::Get_Player_Name(Winner),
				TRANSLATION(IDS_MP_DEFEATS),
				cPlayerManager::Get_Player_Name(Loser));
		}
	}
	*/

	cGameData::Set_Win_Text(win_text);

	//
	//	Let the game know if the map cycle is over
	//
	if (The_Game () != NULL) {
		The_Game ()->Set_Is_Map_Cycle_Over (IsMapCycleOver);
	}

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cWinEvent::Export_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Server());
	WWASSERT(The_Game() != NULL);

	cNetEvent::Export_Creation(packet);

	packet.Add(Winner);
	packet.Add(Loser);
	packet.Add(HostedGameNumber);
	packet.Add(IsMapCycleOver);
	packet.Add((int) The_Game()->Get_Win_Type());
	packet.Add(The_Game()->Get_Game_Duration_S());
   packet.Add_Wide_Terminated_String(The_Game()->Get_Mvp_Name(), true);
	packet.Add(The_Game()->Get_Mvp_Count());

#ifndef MULTIPLAYERDEMO
	packet.Add(::CRC_Stringi(The_Game()->Get_Mod_Name()));
	packet.Add(::CRC_Stringi(The_Game()->Get_Map_Name()));
#endif // MULTIPLAYERDEMO

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cWinEvent::Import_Creation(BitStreamClass & packet)
{
	cNetEvent::Import_Creation(packet);

	WWASSERT(cNetwork::I_Am_Only_Client());

	int win_type = 0;
	unsigned int duration_s = 0;
	WideStringClass mvp_name;
	int mvp_count = 0;

	packet.Get(Winner);
	packet.Get(Loser);
	packet.Get(HostedGameNumber);
	packet.Get(IsMapCycleOver);
	packet.Get(win_type);
	packet.Get(duration_s);
	packet.Get_Wide_Terminated_String(mvp_name.Get_Buffer(256), 256, true);
	packet.Get(mvp_count);

	//TSS092601
	WWASSERT(The_Game() != NULL);

	//
	// This is just causing no end of trouble so I'm going to simplify it.
	// The host has no business sending win events to people not in the same game anyway. ST - 1/18/2002 2:33PM
	//
// FIXME (TSS) Could this be causing problems?
	The_Game()->Set_Hosted_Game_Number(HostedGameNumber + 1);
#if (0)
	if (HostedGameNumber != The_Game()->Get_Hosted_Game_Number())
	{
		return;
	}
	else
	{
		The_Game()->Increment_Hosted_Game_Number();//TSS092901
	}
#endif //(0)

	The_Game()->Set_Winner_ID(Winner);
	The_Game()->Set_Win_Type((cGameData::WinTypeEnum) win_type);
	The_Game()->Set_Game_Duration_S(duration_s);
	The_Game()->Set_Mvp_Name(mvp_name);
	The_Game()->Set_Mvp_Count(mvp_count);

#ifndef MULTIPLAYERDEMO
	unsigned int mod_name_crc = packet.Get(mod_name_crc);
	unsigned int map_name_crc = packet.Get(map_name_crc);

	//
	//	Try to find the map from its CRC
	//
	StringClass mod_name(0, true);
	StringClass map_name(0, true);
	ModPackageMgrClass::Get_Mod_Map_Name_From_CRC (mod_name_crc, map_name_crc, &mod_name, &map_name);
	The_Game()->Set_Mod_Name(mod_name);
	The_Game()->Set_Map_Name(map_name);


#endif // MULTIPLAYERDEMO

	The_Game()->Begin_Intermission();

	Act();
}