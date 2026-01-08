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
 *                     $Archive:: /Commando/Code/Commando/teammanager.cpp                    $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 2/22/02 11:29a                                              $*
 *                                                                                             *
 *                    $Revision:: 73                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "teammanager.h" // I WANNA BE FIRST!

#include <stdio.h>
#include "miscutil.h"
#include "_globals.h"
#include "assets.h"
#include "cnetwork.h"
//#include "gamesettings.h"
#include "multihud.h"
#include <win.h>
#include "player.h"
#include "font3d.h"
#include "gamedata.h"
#include "useroptions.h"
#include "smartgameobj.h"
#include "playertype.h"
#include "colors.h"
#include "render2d.h"
#include "wwprofile.h"
#include "gametype.h"
#include "playermanager.h"
#include "translatedb.h"
#include "string_ids.h"
#include "ConsoleMode.h"
#include "gamespyadmin.h"

//
// Class statics
//
SList<cTeam>				cTeamManager::TeamList;
cTeam *						cTeamManager::Team_Array[];
float							cTeamManager::TeamListHeight				= 0;
Render2DTextClass	*		cTeamManager::PTextRenderer				= NULL;
Font3DInstanceClass *	cTeamManager::PFont							= NULL;
const float					cTeamManager::Y_INCREMENT_FACTOR			= 1.2f;
int							cTeamManager::XPos							= 0;
int							cTeamManager::YPos							= 0;

static WideStringClass renderer_team_heading;
static WideStringClass renderer_team_strings[MAX_TEAMS];
static Vector3 renderer_team_colors[MAX_TEAMS];
static bool renderer_is_intermission_true;

//------------------------------------------------------------------------------------
void cTeamManager::Onetime_Init(void)
{
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
	renderer_team_heading=U_CHAR("");
	renderer_is_intermission_true=false;

}

//------------------------------------------------------------------------------------
void cTeamManager::Onetime_Shutdown(void)
{
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
void cTeamManager::Think(void)
{
	WWPROFILE( "cTeamManager::Think" );

	if (PTextRenderer == NULL) {
		return;
	}
	if (MultiHUDClass::Is_On()) {
		WWASSERT(PTextRenderer != NULL);
		Render_Team_List();
	}
	else {
		// This will make sure the team list disappears if it is not displayed
		if (PTextRenderer) PTextRenderer->Reset();
	}
}

//-----------------------------------------------------------------------------------
void cTeamManager::Render(void)
{
	WWPROFILE("cTeamManager::Render");
	if (PTextRenderer == NULL) {
		return;
	}

	WWASSERT(PTextRenderer != NULL);
	PTextRenderer->Render();
}

//------------------------------------------------------------------------------------
cTeam * cTeamManager::Find_Team(int team_number)
{
   WWASSERT(team_number >= 0 && team_number < MAX_TEAMS);

   SLNode<cTeam> * objnode;

	cTeam *obj;
   for (objnode = TeamList.Head(); objnode; objnode = objnode->Next()) {
		obj = objnode->Data();
      WWASSERT(obj != NULL);
		if (obj->Get_Id() == team_number) {
			return obj; // found it
		}
	}

	return NULL; // Not found
}

cTeam* cTeamManager::Find_Empty_Team(void)
{
	SLNode<cTeam>* node = TeamList.Head();

	while (node) {
		cTeam* team = node->Data();

		if (team && (team->Tally_Size() == 0)) {
			return team;
		}

		node = node->Next();
	}

	return NULL;
}

//------------------------------------------------------------------------------------
void cTeamManager::Add(cTeam * p_team)
{
   WWASSERT(p_team != NULL);
   TeamList.Add_Tail(p_team);
}

//------------------------------------------------------------------------------------
void cTeamManager::Remove(cTeam * p_team)
{
   WWASSERT(p_team != NULL);
   TeamList.Remove(p_team);
   delete p_team;
}

//------------------------------------------------------------------------------------
void cTeamManager::Remove(int team_number)
{
   WWASSERT(team_number >= 0 && team_number < MAX_TEAMS);

   cTeam * p_team = Find_Team(team_number);
   if (p_team != NULL) {
      Remove(p_team);
   }
}

//------------------------------------------------------------------------------------
void cTeamManager::Remove_All(void)
{
	for (SLNode<cTeam> * objnode = TeamList.Head(); objnode != NULL;) {
		cTeam * p_team = objnode->Data();
		objnode = objnode->Next();
		Remove(p_team);
	}
}

//------------------------------------------------------------------------------------
int cTeamManager::Compute_Team_List_Height(void)
{
	/*
	int height = 0;

	if (The_Game()->Is_Team_Game()) {
		WWASSERT(PFont != NULL);
		height = (int)(4 * PFont->Char_Height() * Y_INCREMENT_FACTOR);
	}
	*/
	WWASSERT(PFont != NULL);
	int height = (int)(4 * PFont->Char_Height() * Y_INCREMENT_FACTOR);

	return height;
}

//-----------------------------------------------------------------------------
void cTeamManager::Log_Team_List(void)
{
	WWDEBUG_SAY(("cTeamManager::Log_Team_List\n"));

	StringClass results_filename;
	results_filename.Format("results%d.txt", cUserOptions::ResultsLogNumber.Get());
	FILE * file = ::fopen(results_filename, "wt");

   if (file != NULL) {

		char line[2000] = "";

		WideStringClass wide_text(0, true);
		Construct_Heading(wide_text);//, true);

		StringClass text;
		wide_text.Convert_To(text);

		::sprintf(line, "%s\n", text.Peek_Buffer());
		::fwrite(line, 1, ::strlen(line), file);

		for (int j = 0; j < MAX_TEAMS; j++) {
			cTeam * p_team = Team_Array[j];
			if (p_team != NULL) {
				//p_team->Get_Player_String(j + 1, wide_text, true);
				p_team->Get_Team_String(j + 1, wide_text);
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
WideStringClass cTeamManager::Get_Team_Name(int team_number)
{
   WWASSERT(team_number >= 0 && team_number < MAX_TEAMS);

   cTeam * p_team = cTeamManager::Find_Team(team_number);
   WWASSERT(p_team != NULL);
   return p_team->Get_Name();
}

//-----------------------------------------------------------------------------
void cTeamManager::Reset_Teams(void)
{
   SLNode<cTeam> * objnode;
	cTeam * p_team = NULL;
   for (objnode = TeamList.Head(); objnode != NULL; objnode = objnode->Next()) {
		p_team = objnode->Data();
      WWASSERT(p_team != NULL);
      p_team->Reset();
   }
}

//-----------------------------------------------------------------------------
int cTeamManager::Get_Non_Empty_Team_Count(void)
{
   int num_teams = 0;
   cTeam * p_team;
   for (SLNode<cTeam> * objnode = TeamList.Head(); objnode != NULL; objnode = objnode->Next()) {
		p_team = objnode->Data();
      WWASSERT(p_team != NULL);

      if (p_team->Tally_Size() > 0) {
         num_teams++;
      }
   }

   return num_teams;
}

//-----------------------------------------------------------------------------
void cTeamManager::Sort_Teams(void)
{
	WWPROFILE("cTeamManager::Sort_Teams");

   ZeroMemory(Team_Array, sizeof(Team_Array));

   int num_teams = 0;
   cTeam * p_team;
   for (SLNode<cTeam> * objnode = TeamList.Head(); objnode != NULL; objnode = objnode->Next()) {
		p_team = objnode->Data();
      WWASSERT(p_team != NULL);

      WWASSERT(num_teams < MAX_TEAMS);
      Team_Array[num_teams] = p_team;
      num_teams++;
   }

   qsort(&Team_Array[0], num_teams, sizeof(cTeam *), &Team_Compare);
}

//-----------------------------------------------------------------------------
int cTeamManager::Get_Leaders_Id(void)
{
   WWASSERT(Team_Array[0] != NULL);
   return Team_Array[0]->Get_Id();
}

//-----------------------------------------------------------------------------
int cTeamManager::Sort_Score_Ktd_Kills(cTeam * p_team1, cTeam * p_team2)
{
   WWASSERT(p_team1 != NULL);
   WWASSERT(p_team2 != NULL);

   int result;

   if (p_team1->Get_Score() > p_team2->Get_Score()) {
      result = -1;
   } else if (p_team1->Get_Score() == p_team2->Get_Score()) {
      //if (p_team1->Get_Kill_To_Death_Ratio() > p_team2->Get_Kill_To_Death_Ratio()) {
      //  result = -1;
      //} else if (p_team1->Get_Kill_To_Death_Ratio() == p_team2->Get_Kill_To_Death_Ratio()) {
         if (p_team1->Get_Kills() > p_team2->Get_Kills()) {
            result = -1;
         } else if (p_team1->Get_Kills() == p_team2->Get_Kills()) {
            //
            // OK, give up, they are equal
            //
            result = 0;
         } else {
            result = 1;
         }
      //} else {
      //   result = 1;
      //}
   } else {
      result = 1;
   }

   return result;
}

//-----------------------------------------------------------------------------
int cTeamManager::Team_Compare(const void * elem1, const void * elem2)
{
   //
   // Used by qsort
   //

   WWASSERT(elem1 != NULL);
   WWASSERT(elem2 != NULL);

   cTeam * p_team1 = *((cTeam **)elem1);
   cTeam * p_team2 = *((cTeam **)elem2);
   WWASSERT(p_team1 != NULL);
   WWASSERT(p_team2 != NULL);

   //
   // Sort order:
   // 1. If there is a life limit, sort on lives.
   // 2. Sort on score.
   // 3. Sort on KTD ratio.
   // 4. Sort on Kills
   //
	int result = Sort_Score_Ktd_Kills(p_team1, p_team2);

   return result;
}

//-----------------------------------------------------------------------------
void cTeamManager::List_Print(WideStringClass & text, Vector3 color)
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
void cTeamManager::Construct_Heading(WideStringClass & string)
{
	string.Format(U_CHAR(""));

	WWASSERT(PTheGameData != NULL);
	//bool is_verbose = The_Game()->IsIntermission.Get() || MultiHUDClass::Get_Verbose_Lists();
	bool is_verbose = //force_verbose ||
		               The_Game()->IsIntermission.Is_True() ||
							//MultiHUDClass::Get_Verbose_Lists();
							(MultiHUDClass::Get_Playerlist_Format() == PLAYERLIST_FORMAT_FULL);

	WideStringClass substring(0,true);

   //
	// Num Players
	//
	substring.Format(U_CHAR("%-3s"), U_CHAR(""));
	string += substring;

	/*
	//
	// Standing
	//
   if (is_verbose) {
		substring.Format(U_CHAR("%-5s"), U_CHAR(""));
		string += substring;
   }
	*/

	//
	// "Team" or "Clan"
	//
	//GAMESPY
	//substring.Format(U_CHAR("%-11s"), The_Game()->Get_Team_Word());
	if (cGameSpyAdmin::Is_Gamespy_Game()) {
		substring.Format(U_CHAR("%-36s"), The_Game()->Get_Team_Word());
	} else {
		substring.Format(U_CHAR("%-11s"), The_Game()->Get_Team_Word());
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

	/**/
	//
	// Money
	//
	if ((The_Game()->Is_Cnc() || The_Game()->Is_Skirmish()) && is_verbose) {
		substring.Format(U_CHAR("%-8s"), TRANSLATION(IDS_MP_MONEY));
	   string += substring;
   }
	/**/

	//
	// Score
	//
	substring.Format(U_CHAR("%-8s"), TRANSLATION(IDS_MP_SCORE));
	string += substring;
}

//-----------------------------------------------------------------------------
void cTeamManager::Render_Team_List(void)
{
	if (PTextRenderer == NULL) {
		return;
	}
   WWASSERT(PFont != NULL);
	WWASSERT(PTextRenderer != NULL);

	WWASSERT(PTheGameData != NULL);
	if (GameModeManager::Find("Combat") == NULL ||
		!GameModeManager::Find("Combat")->Is_Active() ||
		The_Game()->IsIntermission.Is_True()) {

		PTextRenderer->Reset();
		return;
	}

   //
   // Format:
   // AAAAAAAAAxxNNNNxxNNNxNNNxNNNxNxxNNNxxNNNNN, e.g:
   // Charlie    5     25  13    1.5   12   1203
   //
   // Fields are name, num players, kills, death, kills-to-deaths, num-lives, score
   //

   /*
	if (!The_Game()->Is_Team_Game()) {
		PTextRenderer->Reset();
      return;
   }
	*/

   /*
	//
   // Count the non-empty teams
   //
   int num_teams = Get_Non_Empty_Team_Count();
   if (num_teams == 0) {
      return;
   }
	*/

	bool changed=false;

   //
   // Build heading
   //
	WideStringClass heading(0,true);
	Construct_Heading(heading);
	if (heading!=renderer_team_heading) {
		renderer_team_heading=heading;
		changed=true;
	}

	// Verify that the team list is valid
	int j;
   for (j = 0; j < MAX_TEAMS; j++) {
		if (!Team_Array[j]) {
			Sort_Teams();
			break;
		}
	}
   for (j = 0; j < MAX_TEAMS; j++) {
	   cTeam * p_team = Team_Array[j];
		WWASSERT(p_team != NULL);
		renderer_team_colors[j]=p_team->Get_Color();
		if (changed) {
			p_team->Get_Team_String(j + 1, renderer_team_strings[j]);
		}
		else {
			WideStringClass team_string(0,true);
			p_team->Get_Team_String(j + 1, team_string);
			if (team_string!=renderer_team_strings[j]) {
				changed=true;
				renderer_team_strings[j]=team_string;
			}
		}
   }

	WWASSERT(PTheGameData != NULL);
	if (The_Game()->IsIntermission.Is_True()!=renderer_is_intermission_true) {
		renderer_is_intermission_true=The_Game()->IsIntermission.Is_True();
		changed=true;
	}

	if (!changed) return;

	PTextRenderer->Reset();

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

	YPos = 10;

	//
	// Show Win text
	//
	WWASSERT(PTheGameData != NULL);
	if (The_Game()->IsIntermission.Is_True()) {

		int combined_height =
			Compute_Team_List_Height() +
			cPlayerManager::Compute_Full_Player_List_Height() +
			2 * PFont->Char_Height() * Y_INCREMENT_FACTOR;

		YPos = (int)(Render2DClass::Get_Screen_Resolution().Height() / 2.0 - combined_height / 2.0);
		if (YPos < 10) {
			YPos = 10;
		}

		WideStringClass text(cGameData::Get_Win_Text(),true);

		float x = Render2DClass::Get_Screen_Resolution().Center().X -
			PFont->String_Width(text) / 2.0f;
		float y = YPos;
		PTextRenderer->Set_Location(Vector2(cMathUtil::Round(x), cMathUtil::Round(y)));
		PTextRenderer->Draw_Text(text);

		YPos += 2 * PFont->Char_Height() * Y_INCREMENT_FACTOR;
	}

   //
   // Draw the teamlist heading
   //
   List_Print(heading, Vector3(1, 1, 1));

   for (j = 0; j < MAX_TEAMS; j++) {
		List_Print(renderer_team_strings[j], renderer_team_colors[j]);
   }

}

















