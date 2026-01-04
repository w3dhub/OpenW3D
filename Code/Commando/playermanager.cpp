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
 *                     $Archive:: /Commando/Code/Commando/playermanager.cpp                  $*
 *                                                                                             *
 *                      $Author:: Denzil_l                                                    $*
 *                                                                                             *
 *                     $Modtime:: 2/27/02 6:08p                                               $*
 *                                                                                             *
 *                    $Revision:: 128                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "playermanager.h"

#include <win.h>
#include <stdio.h>
#include <float.h>

#include "teammanager.h"
#include "miscutil.h"
#include "_globals.h"
#include "assets.h"
#include "cnetwork.h"
#include "multihud.h"
#include "textdisplay.h"
#include "font3d.h"
#include "gamedata.h"
#include "wwdebug.h"
#include "chunkio.h"
#include "useroptions.h"
#include "smartgameobj.h"
#include "playertype.h"
#include "devoptions.h"
#include "render2d.h"
#include "wwprofile.h"
#include "gametype.h"
#include "translatedb.h"
#include "string_ids.h"
#include "systeminfolog.h"
#include "ConsoleMode.h"
#include "gamespyadmin.h"
#include "demosupport.h"

//
// Class statics
//
SList<cPlayer>				cPlayerManager::PlayerList;
cPlayer *					cPlayerManager::Player_Array[];
Render2DTextClass	*		cPlayerManager::PTextRenderer				= NULL;
Font3DInstanceClass *	cPlayerManager::PFont						= NULL;
const float					cPlayerManager::Y_INCREMENT_FACTOR		= 1.2f;
int							cPlayerManager::XPos							= 0;
int							cPlayerManager::YPos							= 0;
Notifier<PlayerMgrEvent> cPlayerManager::mNotifier;

//------------------------------------------------------------------------------------
void cPlayerManager::Onetime_Init(void)
{
	WWDEBUG_SAY(("cPlayerManager::Onetime_Init\n"));
	if (!ConsoleBox.Is_Exclusive()) {
		WWASSERT(PFont == NULL);
		WWASSERT(WW3DAssetManager::Get_Instance() != NULL);
   	PFont = WW3DAssetManager::Get_Instance()->Get_Font3DInstance("FONT6x8.TGA");
   	WWASSERT(PFont != NULL);
		PFont->Set_Mono_Spaced();
		SET_REF_OWNER(PFont);

		WWASSERT(PTextRenderer == NULL);
		PTextRenderer = new Render2DTextClass(PFont);
   	WWASSERT(PTextRenderer != NULL);
		PTextRenderer->Set_Coordinate_Range(Render2DClass::Get_Screen_Resolution());
	}
   ZeroMemory(Player_Array, sizeof(Player_Array));
}

//------------------------------------------------------------------------------------
void cPlayerManager::Onetime_Shutdown(void)
{
	WWDEBUG_SAY(("cPlayerManager::Onetime_Shutdown\n"));

	if (!ConsoleBox.Is_Exclusive()) {
		WWASSERT(PTextRenderer != NULL);
		delete PTextRenderer;
		PTextRenderer = NULL;

		WWASSERT(PFont != NULL);
		PFont->Release_Ref();
		PFont = NULL;
	}
}

//------------------------------------------------------------------------------------
void cPlayerManager::Think(void)
{
	WWPROFILE("cPlayerManager::Think");

	if (MultiHUDClass::Is_On()) {
		WWASSERT(PTextRenderer != NULL);
		Render_Player_List();
	}
	else {
		if (PTextRenderer) PTextRenderer->Reset();
	}
}

//-----------------------------------------------------------------------------------
void cPlayerManager::Render(void)
{
	WWPROFILE("cPlayerManager::Render");
	if (PTextRenderer != NULL) {
		WWASSERT(PTextRenderer != NULL);
		PTextRenderer->Render();
	}
}

//------------------------------------------------------------------------------------
cPlayer * cPlayerManager::Find_Player(int id)
{
   SLNode<cPlayer> * objnode;
	for (objnode = PlayerList.Head(); objnode; objnode = objnode->Next()) {
		cPlayer * p_player = objnode->Data();
      WWASSERT(p_player != NULL);
		if (p_player->Get_Is_Active().Is_True() &&
			 p_player->Get_Id() == id) {
			return p_player; // found it
		}
	}

	return NULL; // Not found
}

//------------------------------------------------------------------------------------
cPlayer * cPlayerManager::Find_Player(const WideStringClass & name)
{
   SLNode<cPlayer> * objnode;
	for (objnode = PlayerList.Head(); objnode; objnode = objnode->Next()) {
		cPlayer *p_player = objnode->Data();
      WWASSERT(p_player != NULL);

      if (p_player->Get_Is_Active().Is_True() &&
			 !name.Compare_No_Case(p_player->Get_Name())) {

			return p_player; // found it
		}
	}

	return NULL; // Not found
}

//------------------------------------------------------------------------------------
cPlayer * cPlayerManager::Find_Inactive_Player(const WideStringClass & name)
{
	cPlayer * p_result = NULL;

	for (
		SLNode<cPlayer> * objnode = PlayerList.Head();
		objnode != NULL;
		objnode = objnode->Next())
	{
		cPlayer * p_player = objnode->Data();
      WWASSERT(p_player != NULL);

      if (p_player->Get_Is_Active().Is_False() &&
			!p_player->Get_Name().Compare_No_Case(name)) {

			p_result = p_player;
			break;
		}
	}

	return p_result;
}

//------------------------------------------------------------------------------------
cPlayer * cPlayerManager::Find_Team_Player(int team_number)
{
	//WWASSERT(The_Game()->Is_Team_Game());
	WWASSERT(team_number >= 0 && team_number < MAX_TEAMS);

   SLNode<cPlayer> * objnode;
	for (objnode = PlayerList.Head(); objnode; objnode = objnode->Next()) {
		cPlayer * p_player = objnode->Data();
      WWASSERT(p_player != NULL);
		if (p_player->Get_Is_Active().Is_True() &&
			 p_player->Get_Player_Type() == team_number) {
			return p_player; // found it
		}
	}

	return NULL; // Not found
}


//------------------------------------------------------------------------------------
cPlayer * cPlayerManager::Find_Random_Team_Player(int team_number)
{
	//WWASSERT(The_Game()->Is_Team_Game());
	WWASSERT(team_number >= 0 && team_number < MAX_TEAMS);

   int chosen_player = rand() % Tally_Team_Size(team_number);

	int count = 0;
	SLNode<cPlayer> * objnode;
	for (objnode = PlayerList.Head(); objnode; objnode = objnode->Next()) {
		cPlayer * p_player = objnode->Data();
      WWASSERT(p_player != NULL);
		if (p_player->Get_Is_Active().Is_True() &&
			 p_player->Get_Player_Type() == team_number &&
			 count++ == chosen_player) {
			return p_player; // found it
		}
	}

	return NULL; // Not found
}

//------------------------------------------------------------------------------------
cPlayer * cPlayerManager::Find_Team_Mate(cPlayer * p_player1)
{
	WWASSERT(p_player1 != NULL);
	WWASSERT(p_player1->Get_Is_Active().Is_True());
	//WWASSERT(The_Game()->Is_Team_Game());

	int team_number = p_player1->Get_Player_Type();
	WWASSERT(team_number >= 0 && team_number < MAX_TEAMS);

   SLNode<cPlayer> * objnode;
	for (objnode = PlayerList.Head(); objnode; objnode = objnode->Next()) {
		cPlayer * p_player2 = objnode->Data();
      WWASSERT(p_player2 != NULL);
		if (p_player2->Get_Is_Active().Is_True() &&
			p_player2->Get_Id() != p_player1->Get_Id() &&
			p_player2->Get_Player_Type() == team_number) {
			return p_player2; // found it
		}
	}

	return NULL; // Not found
}

cPlayer* cPlayerManager::Find_Clan_Mate(cPlayer* player)
{
	if (player != NULL) {
		unsigned int clan = player->Get_WOL_ClanID();
		int playerID = player->Get_Id();

		SLNode<cPlayer>* node = PlayerList.Head();

		while (node) {
			cPlayer* mate = node->Data();
			WWASSERT(mate != NULL);

			if (playerID != mate->Get_Id()) {
				if (clan == mate->Get_WOL_ClanID()) {
					return mate;
				}
			}

			node = node->Next();
		}
	}

	return NULL;
}

//------------------------------------------------------------------------------------
bool cPlayerManager::Is_Player_Present(int id)
{
   return (Find_Player(id) != NULL);
}

//------------------------------------------------------------------------------------
bool cPlayerManager::Is_Player_Present(WideStringClass & name)
{
   return (Find_Player(name) != NULL);
}

//------------------------------------------------------------------------------------
const WideStringClass & cPlayerManager::Get_Player_Name(int id)
{
   cPlayer * p_player = Find_Player(id);
   WWASSERT(p_player != NULL);
   WWASSERT(p_player->Get_Is_Active().Is_True());
   return p_player->Get_Name();
}

//------------------------------------------------------------------------------------
void cPlayerManager::Add(cPlayer * p_player)
{
   WWASSERT(p_player != NULL);
   PlayerList.Add_Tail(p_player);

	PlayerMgrEvent event(PLAYER_ADDED, p_player);
	mNotifier.NotifyObservers(event);
}

//------------------------------------------------------------------------------------
void cPlayerManager::Remove(cPlayer * p_player)
{
	WWASSERT(p_player != NULL);
   PlayerList.Remove(p_player);

	PlayerMgrEvent event(PLAYER_REMOVED, p_player);
	mNotifier.NotifyObservers(event);
}

void cPlayerManager::Deactivated(cPlayer* p_player)
{
	PlayerMgrEvent event(PLAYER_DEACTIVATED, p_player);
	mNotifier.NotifyObservers(event);
}

void cPlayerManager::Activated(cPlayer* p_player)
{
	PlayerMgrEvent event(PLAYER_ACTIVATED, p_player);
	mNotifier.NotifyObservers(event);
}

//------------------------------------------------------------------------------------
int cPlayerManager::Get_Average_Ladder_Points(void)
{
	int numPlayers = 0;
	int totalPoints = 0;

	SList<cPlayer>* playerList = Get_Player_Object_List();
	SLNode<cPlayer>* playerNode = playerList->Head();

	while (playerNode) {
		cPlayer* player = playerNode->Data();

		if (player && player->Get_Is_Active().Is_True() && player->Is_Human()) {
			++numPlayers;
			totalPoints += player->Get_Ladder_Points();
		}

		playerNode = playerNode->Next();
	}

	if (numPlayers) {
		return totalPoints / numPlayers;
	}

	return 0;
}

//------------------------------------------------------------------------------------
unsigned short cPlayerManager::Get_Average_WOL_Points(void)
{
	unsigned int numPlayers = 0;
	unsigned int totalPoints = 0;

	SList<cPlayer>* playerList = Get_Player_Object_List();
	SLNode<cPlayer>* playerNode = playerList->Head();

	while (playerNode) {
		cPlayer* player = playerNode->Data();

		if (player && player->Get_Is_Active().Is_True() && player->Is_Human()) {
			++numPlayers;
			totalPoints += player->Get_WOL_Points();
		}

		playerNode = playerNode->Next();
	}

	if (numPlayers) {
		return (unsigned short)(totalPoints / numPlayers);
	}

	return 0;
}

//------------------------------------------------------------------------------------
int cPlayerManager::Get_Average_Games_Played(void)
{
	int numPlayers = 0;
	int totalPlayed = 0;

	SList<cPlayer>* playerList = Get_Player_Object_List();
	SLNode<cPlayer>* playerNode = playerList->Head();

	while (playerNode) {
		cPlayer* player = playerNode->Data();

		if (player && player->Get_Is_Active().Is_True() && player->Is_Human()) {
			++numPlayers;
			totalPlayed += player->Get_Num_Wol_Games();
		}

		playerNode = playerNode->Next();
	}

	if (numPlayers) {
		return (totalPlayed / numPlayers);
	}

	return 0;
}

//------------------------------------------------------------------------------------
int cPlayerManager::Get_Average_Ping(void)
{
	int numPlayers = 0;
	int totalPing = 0;

	SList<cPlayer>* playerList = Get_Player_Object_List();
	SLNode<cPlayer>* playerNode = playerList->Head();

	while (playerNode) {
		cPlayer* player = playerNode->Data();

		//if (player && player->Get_Is_Active().Is_True() && player->Is_Human()) {
		if (player && player->Get_Is_Active().Is_True() && player->Is_Human() && player->Get_Ping() >= 0) {
			++numPlayers;
			//totalPing += player->Get_Avg_Ping();
			totalPing += player->Get_Ping();
		}

		playerNode = playerNode->Next();
	}

	if (numPlayers) {
		return totalPing / numPlayers;
	}

	return 0;
}


//------------------------------------------------------------------------------------
int cPlayerManager::Get_Average_FPS(void)
{
	int numPlayers = 0;
	int totalFPS = 0;

	SList<cPlayer>* playerList = Get_Player_Object_List();
	SLNode<cPlayer>* playerNode = playerList->Head();

	while (playerNode) {
		cPlayer* player = playerNode->Data();

		if (player && player->Get_Is_Active().Is_True() && player->Is_Human()) {
			++numPlayers;
			totalFPS += player->Get_Fps();
		}

		playerNode = playerNode->Next();
	}

	if (numPlayers) {
		return totalFPS / numPlayers;
	}

	return 0;
}

//------------------------------------------------------------------------------------
static int Sum_Positions(int position)
{
	WWASSERT(position >= 1);

	int retval = 0;

	for (int i = 0; i < position; i++) {
		retval += i;
	}

	return retval;
}

//------------------------------------------------------------------------------------
void cPlayerManager::Compute_Ladder_Points(int winning_team)
{
	WWASSERT(cNetwork::I_Am_Server());

	WWASSERT(PTheGameData != NULL);
	if (The_Game()->IsLaddered.Is_False())	{
		return;
	}

	cPlayerManager::Sort_Players(false);

	DWORD min_qualifying_time_ms = The_Game()->Get_Min_Qualifying_Time_Minutes() * 60 * 1000;

	int i = 0;

	//
	// Count qualifying winners and losers
	//
	int num_winners = 0;
	int num_losers = 0;
   for (i = 0; i < MAX_PLAYERS; i++) {

		if (Player_Array[i] == NULL ||
			 Player_Array[i]->Get_Total_Time() < min_qualifying_time_ms) {
			continue;
		}

		if (Player_Array[i]->Get_Player_Type() == winning_team) {
			num_winners++;
		} else {
			num_losers++;
		}
	}

	int win_pos = num_winners;
	int lose_pos = 1;

   for (i = 0; i < MAX_PLAYERS; i++) {

		if (Player_Array[i] == NULL ||
			 Player_Array[i]->Get_Total_Time() < min_qualifying_time_ms) {
			continue;
		}

		if (Player_Array[i]->Get_Player_Type() == winning_team) {
			Player_Array[i]->Set_Ladder_Points(Sum_Positions(win_pos));
			win_pos--;
			WWASSERT(win_pos >= 0);
		} else {
			Player_Array[i]->Set_Ladder_Points(-Sum_Positions(lose_pos));
			lose_pos++;
		}
	}

	//
	// TSS121501
	// Additional step. Let's now post-multiply ladder points by the fraction of
	// gametime that each player was present.
	//
	float game_duration_s = The_Game()->Get_Game_Duration_S();
	//WWASSERT(game_duration_s > 0);
	if (game_duration_s == 0)
	{
		game_duration_s = 1;
	}
   for (i = 0; i < MAX_PLAYERS; i++) {

		if (Player_Array[i] == NULL ||
			 Player_Array[i]->Get_Total_Time() < min_qualifying_time_ms) {
			continue;
		}

		float player_duration_s = Player_Array[i]->Get_Total_Time() / 1000.0;
		float ratio_present = player_duration_s / game_duration_s;
		float ladder_points = ratio_present * Player_Array[i]->Get_Ladder_Points();
		/*
		WWDEBUG_SAY(("%5.2f * %-4d = %5.2f (%d)\n",
			ratio_present,
			Player_Array[i]->Get_Ladder_Points(),
			ladder_points,
			cMathUtil::Round(ladder_points)
			));
		*/
		Player_Array[i]->Set_Ladder_Points(cMathUtil::Round(ladder_points));
	}

	//
	// Do another sort so that the list is sorted on these newly calculated ladder points
	//
	cPlayerManager::Sort_Players(false);
}

//------------------------------------------------------------------------------------
void cPlayerManager::Increment_Player_Times(void)
{
	WWASSERT(cNetwork::I_Am_Server());

	//
	// Increment the participation time of all active players
	//

   for (int i = 0; i < MAX_PLAYERS; i++) {
      if (Player_Array[i] != NULL &&
			 Player_Array[i]->Is_Active()) {

			Player_Array[i]->Increment_Total_Time();
		}
	}
}

//------------------------------------------------------------------------------------
WideStringClass cPlayerManager::Determine_Mvp_Name(void)
{
	WWASSERT(cNetwork::I_Am_Server());

	WideStringClass mvp_name;

	WWASSERT(The_Game() != NULL);
	DWORD min_qualifying_time_ms = The_Game()->Get_Min_Qualifying_Time_Minutes() * 60 * 1000;

	//
	// Find the top active, time-qualifying player.
	// Player_Array has already been sorted on score.
	//
   for (int i = 0; i < MAX_PLAYERS; i++) {
      if (Player_Array[i] != NULL) {

			//Player_Array[i]->Increment_Total_Time();

			if (Player_Array[i]->Is_Active() &&
				 Player_Array[i]->Get_Total_Time() > min_qualifying_time_ms) {

				mvp_name = Player_Array[i]->Get_Name();
				break;
			}
		}
	}

	return mvp_name;
}

//------------------------------------------------------------------------------------
int cPlayerManager::Compute_Full_Player_List_Height(void)
{
	bool show_inactive = false;
#ifdef WWDEBUG
	if (cDevOptions::ShowInactivePlayers.Is_True()) {
		show_inactive = true;
	}
#endif // WWDEBUG

	int count = 0;
	if (show_inactive) {
		count = PlayerList.Get_Count();
	} else {
		count = Count();
	}

   WWASSERT(PFont != NULL);
	int height = (int)((count + 1) * PFont->Char_Height() * Y_INCREMENT_FACTOR);

	return height;
}

//------------------------------------------------------------------------------------
void cPlayerManager::Remove_Inactive(void)
{
	for (
		SLNode<cPlayer> * objnode = PlayerList.Head();
		objnode != NULL;)
	{
		cPlayer * p_player = objnode->Data();
		WWASSERT(p_player != NULL);
		objnode = objnode->Next();

		if (p_player->Get_Is_Active().Is_False())
		{
			PlayerList.Remove(p_player);
			delete p_player;
		}
	}
}

//------------------------------------------------------------------------------------
void cPlayerManager::Remove_All(void)
{
	WWDEBUG_SAY(("cPlayerManager::Remove_All\n"));

	for (SLNode<cPlayer> * objnode = PlayerList.Head(); objnode != NULL;) {

		cPlayer * p_player = objnode->Data();
      WWASSERT(p_player != NULL);
		objnode = objnode->Next();
		PlayerList.Remove(p_player);
		delete p_player;
	}

   WWASSERT(PlayerList.Get_Count() == 0);

	//Remove_Inactive();
}

//-----------------------------------------------------------------------------
int cPlayerManager::Count(void)
{
	int count = 0;

   for (
		SLNode<cPlayer> * objnode = PlayerList.Head();
		objnode != NULL;
		objnode = objnode->Next()) {

		cPlayer * p_player = objnode->Data();
      WWASSERT(p_player != NULL);

		if (p_player->Get_Is_Active().Is_True())
		{
			count++;
		}
   }

	return count;
}

//-----------------------------------------------------------------------------
void cPlayerManager::Reset_Players(void)
{
   //
	// Call reset on active players.
	// Remove all inactive players.
	//

	Remove_Inactive();

   for (
		SLNode<cPlayer> * objnode = PlayerList.Head();
		objnode != NULL;
		objnode = objnode->Next()) {

		cPlayer * p_player = objnode->Data();
      WWASSERT(p_player != NULL);
		p_player->Reset_Player();
   }
}

//-----------------------------------------------------------------------------
int cPlayerManager::Tally_Team_Size(int type)
{
	WWROOTPROFILE("Tally_Team_Size");
   //WWASSERT(team >= 0 && team < MAX_TEAMS);
   //WWASSERT(The_Game()->Is_Team_Game());

   int tally = 0;
   SLNode<cPlayer> * objnode;
	cPlayer * p_player = NULL;
   for (objnode = PlayerList.Head(); objnode != NULL; objnode = objnode->Next()) {
		p_player = objnode->Data();
      WWASSERT(p_player != NULL);
      if (p_player->Get_Is_Active().Is_True() &&
			 p_player->Get_Player_Type() == type) {
         tally++;
      }
   }
	PlayerInfoLog::Report_Tally_Size(type,tally);

   return tally;
}

//-----------------------------------------------------------------------------
bool cPlayerManager::Is_Kill_Treasonous(cPlayer * p_killer, cPlayer * p_victim)
{
   return (
		//The_Game()->Is_Team_Game() &&
		//The_Game()->Is_Team_Game() &&
		p_killer != NULL &&
      p_victim != NULL &&
		p_killer != p_victim &&
		p_killer->Get_Player_Type() == p_victim->Get_Player_Type());
}

//-----------------------------------------------------------------------------
void cPlayerManager::Sort_Players(bool fast_sort)
{
	WWPROFILE("cPlayerManager::Sort_Players");

   ZeroMemory(Player_Array, sizeof(Player_Array));

   //
   // Copy non-spectators from SList into array usable by qsort
   //
   int num_players = 0;
	//int active_players = 0;
   cPlayer * p_player;
   SLNode<cPlayer> * objnode;
   for (objnode = Get_Player_Object_List()->Head(); objnode; objnode = objnode->Next()) {
		p_player = objnode->Data();
      WWASSERT(p_player != NULL);

		//if (p_player->Is_Living()) {
			//active_players++;
		//}

      WWASSERT(num_players < MAX_PLAYERS);
      Player_Array[num_players] = p_player;
      num_players++;
   }

	if (fast_sort) {

		//
		// If we're sorting the list in-game, use the fast sort
		//
		for (int i=0; i<num_players; i++) {
			Player_Array[i]->Set_Fast_Sort_Key(Compute_Fast_Sort_Key(Player_Array[i]));
		}

		qsort(&Player_Array[0], num_players, sizeof(cPlayer *), &Fast_Player_Compare);

	} else {

		//
		// Otherwise use the full-blown Player_Compare sort
		//
		qsort(&Player_Array[0], num_players, sizeof(cPlayer *), &Player_Compare);
	}

	//
	//	Set rungs. Only increment for active players.
	//
	/*
	for (int index = 0; index < num_players; index ++) {
		Player_Array[index]->Set_Rung (index + 1);
	}
	*/
	int rung = 1;
	for (int index = 0; index < num_players; index ++) {
		Player_Array[index]->Set_Rung(rung);
		if (Player_Array[index]->IsActive.Is_True()) {
			rung++;
		}
	}
}

//-----------------------------------------------------------------------------
int cPlayerManager::Player_Compare(const void * elem1, const void * elem2)
{
   //
   // Used by qsort
	// (gth) Note, this function is very expensive due to the number of
	// data-safe variables being accessed.
   //

   WWASSERT(elem1 != NULL);
   WWASSERT(elem2 != NULL);

   cPlayer * p_player1 = *((cPlayer **)elem1);
   cPlayer * p_player2 = *((cPlayer **)elem2);
   WWASSERT(p_player1 != NULL);
   WWASSERT(p_player2 != NULL);

   int result;

	int p1_type = p_player1->Get_Player_Type();
	if (p1_type > PLAYERTYPE_NOD) {
		p1_type = PLAYERTYPE_NOD;
	}
	int p2_type = p_player2->Get_Player_Type();
	if (p2_type > PLAYERTYPE_NOD) {
		p2_type = PLAYERTYPE_NOD;
	}

	//
	// Sort on Ladder Points
	//
	if (p_player1->Get_Ladder_Points() > p_player2->Get_Ladder_Points()) {
		result = -1;
	} else if (p_player1->Get_Ladder_Points() == p_player2->Get_Ladder_Points()) {
		//
		// Sort on Score
		//
		if (p_player1->Get_Score() > p_player2->Get_Score()) {
			result = -1;
		} else if (p_player1->Get_Score() == p_player2->Get_Score()) {
			//
			// Sort on KTD ratio
			//
			//if (p_player1->Get_Kill_To_Death_Ratio() > p_player2->Get_Kill_To_Death_Ratio()) {
			//	result = -1;
			//} else if (p_player1->Get_Kill_To_Death_Ratio() == p_player2->Get_Kill_To_Death_Ratio()) {
				//
				// Sort on Kills
				//
				if (p_player1->Get_Kills() > p_player2->Get_Kills()) {
					result = -1;
				} else if (p_player1->Get_Kills() == p_player2->Get_Kills()) {
					//
					// Sort on Deaths (less is better)
					//
					if (p_player1->Get_Deaths() < p_player2->Get_Deaths()) {
						result = -1;
					} else if (p_player1->Get_Deaths() == p_player2->Get_Deaths()) {
						//
						// Sort on team number. For non-teammembers this is a
						// redundant but harmless comparison
						//
						if (p_player1->Get_Player_Type() < p_player2->Get_Player_Type()) {
							result = -1;
						} else if (p_player1->Get_Player_Type() == p_player2->Get_Player_Type()) {
							//
							// Sort lexicographically on name
							//
							//if (stricmp(p_player1->Get_Name(), p_player2->Get_Name()) < 0) {
							if (p_player1->Get_Name() < p_player2->Get_Name()) {
								result = -1;
							//} else if (stricmp(p_player1->Get_Name(), p_player2->Get_Name()) == 0) {
							} else if (p_player1->Get_Name() == p_player2->Get_Name()) {
								//
								// Enough!
								//
								result = 0;
							} else {
								result = 1;
							}
						} else {
							result = 1;
						}
					} else {
						result = 1;
					}
				} else {
					result = 1;
				}
			//} else {
			//	result = 1;
			//}
		} else {
			result = 1;
		}
	} else {
		result = 1;
	}

   return result;
}


//-----------------------------------------------------------------------------
int cPlayerManager::Fast_Player_Compare(const void * elem1, const void * elem2)
{
   //
   // Used by qsort
   //
   WWASSERT(elem1 != NULL);
   WWASSERT(elem2 != NULL);

   cPlayer * p_player1 = *((cPlayer **)elem1);
   cPlayer * p_player2 = *((cPlayer **)elem2);

   WWASSERT(p_player1 != NULL);
   WWASSERT(p_player2 != NULL);

	if (p_player1->Get_Fast_Sort_Key() > p_player2->Get_Fast_Sort_Key()) {
		return -1;
	} else {
		return 1;
	}
}

//-----------------------------------------------------------------------------
int cPlayerManager::Compute_Fast_Sort_Key(cPlayer * player)
{
	int key = 0;
	key = player->Get_Score();
	return key;
}

//-----------------------------------------------------------------------------
void cPlayerManager::Construct_Heading(WideStringClass & string, bool force_verbose)
{
	string.Format(U_CHAR(""));

	WWASSERT(PTheGameData != NULL);
	//bool is_verbose = force_verbose || The_Game()->IsIntermission.Get() || MultiHUDClass::Get_Verbose_Lists();
	bool is_verbose = force_verbose ||
		               The_Game()->IsIntermission.Is_True() ||
							//MultiHUDClass::Get_Verbose_Lists();
							(MultiHUDClass::Get_Playerlist_Format() == PLAYERLIST_FORMAT_FULL);

	WideStringClass substring(0,true);

	//
	// Standing
	//
	substring.Format(U_CHAR("%-5s"), U_CHAR(""));
   string += substring;

	//
	// Name
	// 
	//GAMESPY
	//substring.Format(U_CHAR("%-11s"), TRANSLATION(IDS_MP_PLAYER));
	if (cGameSpyAdmin::Is_Gamespy_Game()) {
		substring.Format(U_CHAR("%-36s"), TRANSLATION(IDS_MP_PLAYER));
	} else {
		substring.Format(U_CHAR("%-11s"), TRANSLATION(IDS_MP_PLAYER));
	}
   string += substring;

	//
	// Kills
	//
   if (is_verbose) {
		substring.Format(U_CHAR("%-8s"), TRANSLATION(IDS_MP_KILLS));
	   string += substring;
   }

	//
	// Deaths
	//
   if (is_verbose) {
		substring.Format(U_CHAR("%-8s"), TRANSLATION(IDS_MP_DEATHS));
	   string += substring;
   }

	//
	// Kill to Death ratio
	//
   if (is_verbose) {
		substring.Format(U_CHAR("%-8s"), TRANSLATION(IDS_MP_KILL_TO_DEATH_RATIO));
	   string += substring;
   }

	//
	// Money
	//
   if ((The_Game()->Is_Cnc() || The_Game()->Is_Skirmish()) && is_verbose) {
		substring.Format(U_CHAR("%-8s"), TRANSLATION(IDS_MP_MONEY));
	   string += substring;
   }

	//
	// Score
	//
	substring.Format(U_CHAR("%-8s"), TRANSLATION(IDS_MP_SCORE));
	string += substring;

	//
	// Ladder Points
	//
   if (force_verbose && The_Game()->IsLaddered.Is_True()) {
		substring.Format(U_CHAR("%-8s"), TRANSLATION(IDS_MP_LADDER));
	   string += substring;
	}

   //
	// WOL rank
	//
   if (GameModeManager::Find("WOL")->Is_Active() && is_verbose) {
		substring.Format(U_CHAR("%-8s"), TRANSLATION(IDS_MP_RANK));
	   string += substring;
   }

#ifdef WWDEBUG
   //
   // Ping
   //
   if (cDevOptions::ShowPing.Is_True()) {
		substring.Format(U_CHAR("%-8s"), U_CHAR("Ping"));
	   string += substring;
   }

   //
   // Player Id
   //
   if (cDevOptions::ShowId.Is_True()) {
		substring.Format(U_CHAR("%-8s"), U_CHAR("Id"));
	   string += substring;
   }

   //
   // Fps
   //
   if (cNetwork::I_Am_Server() && cDevOptions::ShowClientFps.Is_True()) {
		substring.Format(U_CHAR("%-8s"), U_CHAR("Fps"));
	   string += substring;
   }

	//
	// GameSpy auth. state
	//
   if (cNetwork::I_Am_Server() && cGameSpyAdmin::Is_Gamespy_Game() && 
		cDevOptions::ShowGameSpyAuthState.Is_True()) {
      substring.Format(U_CHAR("%-12s"), U_CHAR("GS_AUTH"));
      string += substring;
   }

	//
	// IP Address
	//
	if (cNetwork::I_Am_Server() && cDevOptions::ShowIpAddresses.Is_True()) {
		substring.Format(U_CHAR("%-30s"), U_CHAR("IP Address"));
	   string += substring;
	}
#endif // WWDEBUG
}

//-----------------------------------------------------------------------------
void cPlayerManager::List_Print(WideStringClass & text, Vector3 color)
{
   //WWASSERT(text != NULL);
   //WWASSERT(::strlen(text) > 0);

	if (PTextRenderer == NULL) {
		return;
	}

	WWASSERT(PTextRenderer != NULL);

	PTextRenderer->Set_Location(Vector2(cMathUtil::Round(XPos), cMathUtil::Round(YPos)));

	int c = ((int)(color[0]*255)&0xFF) << 16 | ((int)(color[1]*255)&0xFF) << 8 | ((int)(color[2]*255)&0xFF) << 0 | 0xFF000000;

	PTextRenderer->Draw_Text(text, c);

   WWASSERT(PFont != NULL);
   YPos += PFont->Char_Height() * Y_INCREMENT_FACTOR;
}

//-----------------------------------------------------------------------------
void cPlayerManager::Line(float x, float length, int line_color)
{
   WWASSERT(length > 0);

	if (PTextRenderer == NULL) {
		return;
	}

	WWASSERT(PTextRenderer != NULL);

	float y = YPos + PTextRenderer->Peek_Font()->Char_Height() / 2.0f;

	PTextRenderer->Draw_Block(RectClass(x, y, x + length, y + 1), line_color);
}

//-----------------------------------------------------------------------------
void cPlayerManager::Render_Player_List(void)
{
   //
   // Format:
   // AAAAAAAAAxxNNNNxxNNNxNNNxNNN.NxxNNNxxNNNNN, e.g:
   // Charlie    220   25  13    5.3    3     12
   //
   // Fields are name, ping, kills, death, kills-to-deaths, num-lives, score
   //

	if (PTextRenderer == NULL) {
		return;
	}

	WWASSERT(PTheGameData != NULL);
	if (GameModeManager::Find("Combat") == NULL ||
		!GameModeManager::Find("Combat")->Is_Active() ||
		The_Game()->IsIntermission.Is_True()) {

		PTextRenderer->Reset();
		return;
	}

	//bool is_verbose = The_Game()->IsIntermission.Get() || MultiHUDClass::Get_Verbose_Lists();
	bool is_verbose = The_Game()->IsIntermission.Get() || (MultiHUDClass::Get_Playerlist_Format() == PLAYERLIST_FORMAT_FULL);

	bool changed=false;

	static WideStringClass renderer_player_heading;
	static WideStringClass renderer_player_strings[MAX_PLAYERS];
	static Vector3 renderer_player_colors[MAX_PLAYERS];
	static int renderer_player_count;
	static int renderer_displayed_player_count;
	static int renderer_star_index = -1;
	static float renderer_star_x;
	static float renderer_star_y;
	static bool renderer_is_intermission_true;
	static int LastSortedSecond = 0;

   //
   // Sort the list of players once a second
   //

	bool sort=is_verbose;
	//bool sort = false;//XXX
	int seconds = cMathUtil::Round(TimeManager::Get_Seconds());
	if (!sort) {
		if (seconds != LastSortedSecond) {
			LastSortedSecond = seconds;
			sort=true;
		}
	}
	sort=true;//XXX

	if (sort) {
		WWPROFILE("Team & Player sorts");
		cTeamManager::Sort_Teams();
		cPlayerManager::Sort_Players(true);
	}

   //
   // Build heading
   //
	WideStringClass heading(0,true);
	Construct_Heading(heading);
	if (heading!=renderer_player_heading) {
		renderer_player_heading=heading;
		changed=true;
	}

	if (The_Game()->IsIntermission.Is_True()!=renderer_is_intermission_true) {
		changed=true;
		renderer_is_intermission_true=The_Game()->IsIntermission.Is_True();
	}

	// Build player list

	//int current_count = Count();
	int current_count = PlayerList.Get_Count();

	if (renderer_player_count!= current_count) {
		renderer_player_count = current_count;
		changed=true;
	}

	LPCSTR star_text = "*";

	int displayed_player_count=0;
	int count = current_count;

	bool show_inactive = false;
#ifdef WWDEBUG
	if (cDevOptions::ShowInactivePlayers.Is_True()) {
		show_inactive = true;
	}
#endif // WWDEBUG
	int j;
	for (j = 0; j < count; j++) {
      WWASSERT(j < MAX_PLAYERS);
	   cPlayer * p_player = Player_Array[j];
		if (!p_player) continue;

		//
		// In the abbreviated form we only show the score leader and ourself.
		// 09/26/01 And the guy at the bottom of the list !
		// 10/29/01 Now we just show me.
		//
		bool is_me = cNetwork::I_Am_Client() && (p_player->Get_Id() == cNetwork::Get_My_Id());
		//if (!is_verbose && (j != 0) && (j != count - 1) && !is_me) continue;
		//if (!is_verbose && !is_me) {
		if ((MultiHUDClass::Get_Playerlist_Format() == PLAYERLIST_FORMAT_TINY) && !is_me) {
			continue;
		}

		if (p_player->Get_Is_Active().Is_False() && !show_inactive) {
			continue;
		}

      if (changed) {
	      p_player->Get_Player_String(j + 1, renderer_player_strings[displayed_player_count]);
			renderer_player_colors[displayed_player_count]=p_player->Get_Color();
		}
		else {
			WideStringClass player_string(0,true);
			p_player->Get_Player_String(j + 1, player_string);
			if (player_string!=renderer_player_strings[displayed_player_count]) {
				renderer_player_strings[displayed_player_count]=player_string;
				changed=true;
			}
			// If list hasn't changed so far, check if the player color is the same
			if (renderer_player_colors[displayed_player_count]!=p_player->Get_Color()) {
				renderer_player_colors[displayed_player_count]=p_player->Get_Color();
				changed=true;
			}
		}

		if (cNetwork::I_Am_Client() && p_player->Get_Id() == cNetwork::Get_My_Id()) {
         //
         // Put a symbol next to my name so that it stands out
         //
			if (renderer_star_index!=displayed_player_count) {
				renderer_star_index=displayed_player_count;
				changed=true;
			}
      }

		displayed_player_count++;
   }

	if (renderer_displayed_player_count!=displayed_player_count) {
		renderer_displayed_player_count=displayed_player_count;
		changed=true;
	}

	if (!changed) return;

	PTextRenderer->Reset();


   WWASSERT(PFont != NULL);
	WWASSERT(PTextRenderer != NULL);

	DEMO_SECURITY_CHECK;

   //
   // Determine left edge placement
   //
	XPos = 0;
	float text_len = PFont->String_Width(heading);
	if (The_Game()->IsIntermission.Is_True()) {
		XPos = Render2DClass::Get_Screen_Resolution().Center().X - text_len / 2.0f;
	} else {
		XPos = Render2DClass::Get_Screen_Resolution().Right - 20 - text_len;
	}

	//
	// Top placement
	//
	if (The_Game()->IsIntermission.Is_True()) {
		//YPos = 70;

		int combined_height =
			cTeamManager::Compute_Team_List_Height() +
			cPlayerManager::Compute_Full_Player_List_Height() +
			2 * PFont->Char_Height() * Y_INCREMENT_FACTOR;

		YPos = (int)(Render2DClass::Get_Screen_Resolution().Height() / 2.0 - combined_height / 2.0);
		if (YPos < 10) {
			YPos = 10;
		}

		/*
		if (The_Game()->Is_Team_Game()) {
			YPos += 2 * PFont->Char_Height() * Y_INCREMENT_FACTOR;
			YPos += cTeamManager::Compute_Team_List_Height();
		} else {
			//
			// Show the win text
			//
			WideStringClass text(cGameData::Get_Win_Text(),true);

			float x = Render2DClass::Get_Screen_Resolution().Center().X -
				PFont->String_Width(text) / 2.0f;
			float y = YPos;
			PTextRenderer->Set_Location(Vector2(cMathUtil::Round(x), cMathUtil::Round(y)));
			PTextRenderer->Draw_Text(text);

			YPos += 2 * PFont->Char_Height() * Y_INCREMENT_FACTOR;
		}
		*/

		YPos += 2 * PFont->Char_Height() * Y_INCREMENT_FACTOR;
		YPos += cTeamManager::Compute_Team_List_Height();

	} else {
		//YPos = 50;
		/*
		YPos = 10;
		if (The_Game()->Is_Team_Game())	{
			//YPos += 4 * PFont->Char_Height() * Y_INCREMENT_FACTOR;
			YPos += cTeamManager::Compute_Team_List_Height();
		}
		*/
		YPos = 10 + cTeamManager::Compute_Team_List_Height();
	}

	//
   // Draw the heading
   //
   /*
	if (The_Game()->Is_Team_Game()) {
      List_Print(heading, Vector3(1, 1, 1));
   } else {
      List_Print(heading, Vector3(1, 1, 0));
   }
	*/
   List_Print(heading, Vector3(1, 1, 1));

	for (j = 0; j < renderer_displayed_player_count; j++) {
		//
		// Put a symbol next to my name so that it stands out
		//
		//if (is_verbose && renderer_star_index==j) {
		if ((MultiHUDClass::Get_Playerlist_Format() != PLAYERLIST_FORMAT_TINY) &&
			 (renderer_star_index == j)) {
			float x = XPos - 2 * PFont->String_Width(star_text);
			float y = YPos;
			PTextRenderer->Set_Location(Vector2(cMathUtil::Round(x), cMathUtil::Round(y)));
			PTextRenderer->Draw_Text(star_text);
		}

		List_Print(renderer_player_strings[j], renderer_player_colors[j]);
	}
}

//-----------------------------------------------------------------------------
void cPlayerManager::Log_Player_List(void)
{
	WWDEBUG_SAY(("cPlayerManager::Log_Player_List\n"));

	StringClass results_filename;
	results_filename.Format("results%d.txt", cUserOptions::ResultsLogNumber.Get());
	FILE * file = ::fopen(results_filename, "at");

   if (file != NULL) {

		char line[2000] = "";

		WideStringClass wide_text(0, true);
		Construct_Heading(wide_text, true);

		StringClass text;
		wide_text.Convert_To(text);

		::sprintf(line, "%s\n", text.Peek_Buffer());
		::fwrite(line, 1, ::strlen(line), file);

		for (int j = 0; j < MAX_PLAYERS; j++) {
			cPlayer * p_player = Player_Array[j];
			if (p_player != NULL) {
				p_player->Get_Player_String(j + 1, wide_text, true);
				wide_text.Convert_To(text);
				::sprintf(line, "%s\n", text.Peek_Buffer());
				::fwrite(line, 1, ::strlen(line), file);
			}
		}

		::sprintf(line, "\n");
		::fwrite(line, 1, ::strlen(line), file);

		::fclose(file);
	}
}

//-----------------------------------------------------------------------------
enum {
	CHUNKID_PLAYERLIST = 1232345,
	CHUNKID_PLAYER,
};

//-----------------------------------------------------------------------------
bool cPlayerManager::Save(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(CHUNKID_PLAYERLIST);

   SLNode<cPlayer> * objnode;
	for (objnode = PlayerList.Head(); objnode; objnode = objnode->Next()) {
		cPlayer * p_player = objnode->Data();
      WWASSERT(p_player != NULL);
		csave.Begin_Chunk(CHUNKID_PLAYER);
		p_player->Save(csave);
		csave.End_Chunk();
	}

	csave.End_Chunk();

	return true;
}

//-----------------------------------------------------------------------------
bool cPlayerManager::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PLAYERLIST:

				//Remove_All(); // TSS091401

				//TSS110101. WWASSERT(PlayerList.Get_Count() == 0);
				while (cload.Open_Chunk()) {
					switch(cload.Cur_Chunk_ID()) {

						case CHUNKID_PLAYER:
							cPlayer * p_player;
							p_player = new cPlayer();
							p_player->Load(cload);

							/*
							if (!The_Game()->Is_Valid_Player_Type(p_player->Get_Player_Type())) {
								Debug_Say(("* Removing loaded player due to invalid playertype for this game.\n"));
								Remove(p_player);
							}
							*/

							//
							// TSS110101.
							// Do not perturb the save/load code, but nullify it's effect.
							//
							if ( !IS_MISSION ) {
								Remove(p_player);
							}

							break;

						default:
							Debug_Say(( "Unrecognized cPlayerManager chunkID\n" ));
							break;

					}
					cload.Close_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized cPlayerManager chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

//-----------------------------------------------------------------------------



