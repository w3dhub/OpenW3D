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
 *                     $Archive:: /Commando/Code/Commando/playermanager.h                 $* 
 *                                                                                             * 
 *                      $Author:: Denzil_l                                                    $* 
 *                                                                                             * 
 *                     $Modtime:: 2/27/02 5:51p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 46                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSV_VER)
#pragma once
#endif

#ifndef PLAYERMANAGER_H
#define PLAYERMANAGER_H

#include "vector3.h"
#include "bittype.h"
#include "slist.h"
#include "player.h"
#include "wwstring.h"
#include "widestring.h"
#include <wwlib/Notify.h>

static const int PLAYER_ID_UNKNOWN	= -99999;
static const int MAX_PLAYERS			= 255;

class ChunkSaveClass;
class ChunkLoadClass;
class Render2DTextClass;
class Font3DInstanceClass;

typedef enum {PLAYER_ADDED, PLAYER_REMOVED, PLAYER_DEACTIVATED, PLAYER_ACTIVATED} PLAYERMGR_ACTION;
typedef TypedActionPtr<PLAYERMGR_ACTION, cPlayer> PlayerMgrEvent;

//-----------------------------------------------------------------------------
class cPlayerManager {

public:

	static void							Onetime_Init(void);
	static void							Onetime_Shutdown(void);
	static void							Think(void);
	static void							Render(void);
	static bool							Save(ChunkSaveClass & csave);
	static bool							Load(ChunkLoadClass & cload);
   static void       				Reset_Players(void);
   static void       				Remove_All(void); // N.B. destructive
   static int        				Tally_Team_Size(int type);
   static bool       				Is_Kill_Treasonous(cPlayer * p_killer, cPlayer * p_victim);
   static void       				Sort_Players(bool fast_sort);
   //static int        				Get_Leaders_Id(void);
	static cPlayer * 					Find_Player(int id);
	static cPlayer * 					Find_Player(const WideStringClass & name);
	static cPlayer * 					Find_Inactive_Player(const WideStringClass & name);
	static cPlayer * 					Find_Team_Player(int team_number);
	static cPlayer	* 					Find_Random_Team_Player(int team_number);
	static cPlayer	* 					Find_Team_Mate(cPlayer * p_player);
	static cPlayer*						Find_Clan_Mate(cPlayer* player);
	static bool	      				Is_Player_Present(int id);
	static bool	      				Is_Player_Present(WideStringClass & name);
   static const WideStringClass &Get_Player_Name(int id);
	static SList<cPlayer> * 		Get_Player_Object_List(void) {return &PlayerList;}
   static void	      				Add(cPlayer * p_player);
   static void       				Remove(cPlayer * p_player); // N.B. destructive
	static int							Count(void);
	static void							Deactivated(cPlayer* p_player);
	static void							Activated(cPlayer* p_player);
	static void							Compute_Ladder_Points(int winner_id);
	static WideStringClass 			Determine_Mvp_Name(void);
	static void							Increment_Player_Times(void);
	static int							Compute_Full_Player_List_Height(void);
	static int							Get_Average_Ladder_Points(void);
	static unsigned short		Get_Average_WOL_Points(void);
	static int							Get_Average_Games_Played(void);
	static int							Get_Average_Ping(void);
	static int							Get_Average_FPS(void);
   static void       				Log_Player_List(void);

	static void Add_Event_Observer(Observer<PlayerMgrEvent>& observer)
		{mNotifier.AddObserver(observer);}

private:
   static void       				Render_Player_List(void);
   static int        				Player_Compare(const void * elem1, const void * elem2);
   static int        				Fast_Player_Compare(const void * elem1, const void * elem2);
	static void							List_Print(WideStringClass & text, Vector3 color);
	static void							Line(float x, float length, int line_color);
	static void							Construct_Heading(WideStringClass & heading, bool force_verbose = false);
	//static int							Get_Ranked_Id(int rank);
	static void							Remove_Inactive(void);
	static int							Compute_Fast_Sort_Key(cPlayer * player);
	
	static SList<cPlayer>			PlayerList;
   static cPlayer *					Player_Array[MAX_PLAYERS];
	static Render2DTextClass	*	PTextRenderer;
	static Font3DInstanceClass *	PFont;
	static const float				Y_INCREMENT_FACTOR;
	static int							XPos;
	static int							YPos;

	static Notifier<PlayerMgrEvent> mNotifier;
};

//-----------------------------------------------------------------------------

#endif // PLAYERMANAGER_H





