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
 *                     $Archive:: /Commando/Code/Commando/gdskirmish.cpp                           $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 4/12/02 4:01p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 12                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "gdskirmish.h"

#include "cnetwork.h"
#include "translatedb.h"
#include "string_ids.h"
#include "assets.h"
#include "multihud.h"
#include "playerdata.h"
#include "player.h"
#include "playertype.h"
#include "gameobjmanager.h"
#include "spawn.h"

//-----------------------------------------------------------------------------
cGameDataSkirmish::cGameDataSkirmish(void)	:
	IsPlaying (false),
	cGameData()
{
   //WWDEBUG_SAY(("cGameDataSkirmish::cGameDataSkirmish\n"));

	Set_Time_Limit_Minutes(25);
	Set_Ini_Filename("svrcfg_skirmish.ini");
	BaseDestructionEndsGame.Set(true);
	BeaconPlacementEndsGame.Set(true);
	StartingCredits = 400;
	//IsTeamChangingAllowed.Set(false);
	IsTeamChangingAllowed.Set(true);
	Set_Max_Players(1);
	Set_Radar_Mode(RADAR_TEAMMATES);

	Load_From_Server_Config();
	Save_To_Server_Config();
	
	//
	// Reload again to avoid LastServerConfigModTime issues
	//
	Load_From_Server_Config();
}

//-----------------------------------------------------------------------------
cGameDataSkirmish::~cGameDataSkirmish(void)
{
   //WWDEBUG_SAY(("cGameDataSkirmish::~cGameDataSkirmish\n"));
}

//-----------------------------------------------------------------------------
cGameDataSkirmish& cGameDataSkirmish::operator=(const cGameDataSkirmish& rhs)
{
	// 
	// Call the base class
	//
	cGameData::operator=(rhs);

	BaseDestructionEndsGame				= rhs.BaseDestructionEndsGame;
	BeaconPlacementEndsGame				= rhs.BeaconPlacementEndsGame;
	StartingCredits						= rhs.StartingCredits;

	return (*this);
}

//-----------------------------------------------------------------------------
const unichar_t* cGameDataSkirmish::Get_Static_Game_Name(void)
{
	return TRANSLATION(IDS_MP_GAME_TYPE_SKIRMISH);
}

//-----------------------------------------------------------------------------
void cGameDataSkirmish::Reset_Game(bool is_reloaded)
{
   WWDEBUG_SAY(("cGameDataSkirmish::Reset_Game\n"));

	cGameData::Reset_Game(is_reloaded);

	//
	// TSS091201 - hack - On_Game_End not called in correct places
	//
	if (IsPlaying) {
		BaseGDI.Shutdown ();
		BaseNOD.Shutdown ();
		IsPlaying = false;
	}

	//
	//	Initialize the base controllers
	//
	BaseGDI.Initialize ( PLAYERTYPE_GDI );
	BaseNOD.Initialize ( PLAYERTYPE_NOD );
	return ;
}

//-----------------------------------------------------------------------------
void
cGameDataSkirmish::On_Game_Begin (void)
{
   WWDEBUG_SAY(("cGameDataSkirmish::On_Game_Begin\n"));

	cGameData::On_Game_Begin();

	CombatManager::Set_Beacon_Placement_Ends_Game(BeaconPlacementEndsGame.Get());

	//
	//	Initialize the bases
	//
	IsPlaying = true;
}

//-----------------------------------------------------------------------------
void
cGameDataSkirmish::On_Game_End (void)
{
   WWDEBUG_SAY(("cGameDataSkirmish::On_Game_End\n"));

	//
	//	Close the bases
	//
	BaseGDI.Shutdown ();
	BaseNOD.Shutdown ();
	IsPlaying = false;

	cGameData::On_Game_End();
	return ;
}

//-----------------------------------------------------------------------------
void
cGameDataSkirmish::Soldier_Added (SoldierGameObj *soldier)
{
	cGameData::Soldier_Added (soldier);

	//
	//	Give the soldier some starting credits
	//
	if (	soldier != NULL &&
			soldier->Get_Player_Data() != NULL &&
			soldier->Get_Player_Data()->Get_Game_Time () == 0)
	{
		soldier->Get_Player_Data ()->Set_Money (StartingCredits);
	}

	return ;
}

//-----------------------------------------------------------------------------
void cGameDataSkirmish::Think(void)
{
	cGameData::Think();

	if (!cNetwork::I_Am_Server()) {
		return;
	}

	//
	//	Let the bases think
	//
	if (IsPlaying) {
		BaseGDI.Think ();
		BaseNOD.Think ();
	}

	//
	// Weed out any soldiers or spawners that shouldn't be present.
	//
	Filter_Soldiers();

	return ;
}

//-----------------------------------------------------------------------------
void cGameDataSkirmish::Set_Starting_Credits(int credits)
{
   Set_Generic_Num(credits, 0, MAX_CREDITS, StartingCredits);
}

//-----------------------------------------------------------------------------
void cGameDataSkirmish::Base_Destruction_Score_Tweaking(void)
{
	WWASSERT(cNetwork::I_Am_Server());

	cTeam * p_nod = cTeamManager::Find_Team(PLAYERTYPE_NOD);
	WWASSERT(p_nod != NULL);

	cTeam * p_gdi = cTeamManager::Find_Team(PLAYERTYPE_GDI);
	WWASSERT(p_gdi != NULL);

	float nod_score = p_nod->Get_Score();
	float gdi_score = p_gdi->Get_Score();

	const int BASE_DESTRUCTION_POINTS_REWARD = 5000;

	//
	// Base destroyer gets a points reward. 
	// If this isn't enough to beat other team, nudge score ahead by 1 point.
	//

	if (BaseGDI.Is_Base_Destroyed()) {

		nod_score += BASE_DESTRUCTION_POINTS_REWARD;
		if (nod_score <= gdi_score) {
			nod_score = gdi_score + 1;
		}

		p_nod->Set_Score(nod_score);

	} else {

		WWASSERT(BaseNOD.Is_Base_Destroyed());

		gdi_score += BASE_DESTRUCTION_POINTS_REWARD;
		if (gdi_score <= nod_score) {
			gdi_score = nod_score + 1;
		}

		p_gdi->Set_Score(gdi_score);
	}
}

//-----------------------------------------------------------------------------
bool cGameDataSkirmish::Is_Game_Over(void)
{
	WWASSERT(cNetwork::I_Am_Server());

 	bool is_game_over = cGameData::Is_Game_Over();

	if (!is_game_over && BaseDestructionEndsGame.Is_True())
	{
		//
		// If a base is destroyed then the game is over and the other team wins.
		// To ensure this we may need to tweak the base-destroyer's score.
		//

		if (BaseGDI.Is_Base_Destroyed() || BaseNOD.Is_Base_Destroyed()) {
			is_game_over = true;
			Set_Win_Type(WIN_TYPE_BASE_DESTRUCTION);
			Base_Destruction_Score_Tweaking();
		}
	}

	//
	// Beacon placement end game works via above base destruction method.
	//

	return is_game_over;
}

//-----------------------------------------------------------------------------
void cGameDataSkirmish::Load_From_Server_Config(void)
{
	cGameData::Load_From_Server_Config(Get_Ini_Filename());

   INIClass * p_ini = Get_INI(Get_Ini_Filename());
   WWASSERT(p_ini != NULL);

   bool				b;
   int				i;
   //float				f;

	
   i = p_ini->Get_Int(	INI_SECTION_NAME, "MaxPlayers",					Get_Max_Players());
	Set_Max_Players(i);
   
	b = p_ini->Get_Bool(	INI_SECTION_NAME, "IsFriendlyFirePermitted",	IsFriendlyFirePermitted.Get());
	IsFriendlyFirePermitted.Set(b);

	b = p_ini->Get_Bool(	INI_SECTION_NAME, "IsTeamChangingAllowed",	IsTeamChangingAllowed.Get());
	IsTeamChangingAllowed.Set(b);
	
	b = p_ini->Get_Bool(	INI_SECTION_NAME, "IsClanGame",					IsClanGame.Get());
	IsClanGame.Set(b);

	b = p_ini->Get_Bool(	INI_SECTION_NAME, "BaseDestructionEndsGame",			BaseDestructionEndsGame.Get());
	BaseDestructionEndsGame.Set(b);

	b = p_ini->Get_Bool(	INI_SECTION_NAME, "BeaconPlacementEndsGame",	BeaconPlacementEndsGame.Get());
	BeaconPlacementEndsGame.Set(b);

   i = p_ini->Get_Int(	INI_SECTION_NAME, "StartingCredits",				Get_Starting_Credits());
	Set_Starting_Credits(i);
	

   
	Release_INI(p_ini);
	return ;
}

//-----------------------------------------------------------------------------
void cGameDataSkirmish::Save_To_Server_Config(void)
{
	cGameData::Save_To_Server_Config(Get_Ini_Filename());

   INIClass * p_ini = Get_INI(Get_Ini_Filename());
   WWASSERT(p_ini != NULL);


	p_ini->Put_Bool(	INI_SECTION_NAME, "IsFriendlyFirePermitted",	IsFriendlyFirePermitted.Get());
	p_ini->Put_Bool(	INI_SECTION_NAME, "IsTeamChangingAllowed",	IsTeamChangingAllowed.Get());
	p_ini->Put_Bool(	INI_SECTION_NAME, "IsClanGame",					IsClanGame.Get());
   p_ini->Put_Int(	INI_SECTION_NAME, "MaxPlayers",					Get_Max_Players());
	p_ini->Put_Bool(	INI_SECTION_NAME, "BaseDestructionEndsGame",	BaseDestructionEndsGame.Get());
	p_ini->Put_Bool(	INI_SECTION_NAME, "BeaconPlacementEndsGame",	BeaconPlacementEndsGame.Get());
	p_ini->Put_Int(   INI_SECTION_NAME, "StartingCredits",			Get_Starting_Credits());


	Save_INI(p_ini, Get_Ini_Filename());
   Release_INI(p_ini);
}

//-----------------------------------------------------------------------------
void cGameDataSkirmish::Show_My_Money(void)
{  
	if (cNetwork::I_Am_Client()) {
		cPlayer * p_player = cNetwork::Get_My_Player_Object();
		if (p_player != NULL) {
			WideStringClass text(0,true);
			text.Format(U_CHAR("%s: %d"), 
				TRANSLATION(IDS_MP_MONEY), (int) p_player->Get_Money());
			Add_Bottom_Text(text);
		}
	}
}

//-----------------------------------------------------------------------------
void cGameDataSkirmish::Show_Game_Settings_Limits(void)
{  
	/*
	if (IsIntermission.Is_True()) {
		return;
	}
	*/

	Show_My_Money();

	cGameData::Show_Game_Settings_Limits();
}

//-----------------------------------------------------------------------------
void cGameDataSkirmish::Filter_Soldiers(void)
{  
	//
	// Remove any soldiers or spawners on your side, leaving only opponents.
	//

	cPlayer * p_player = cNetwork::Get_My_Player_Object();

	if (p_player != NULL) {

		//int my_team = p_player->Get_Player_Type();

		/*
		//
		// Remove soldiers
		// Whoa, design now wants these... comment out this code.
		//
		for (
			SLNode<SmartGameObj> * objnode = GameObjManager::Get_Smart_Game_Obj_List()->Head(); 
			objnode != NULL; 
			objnode = objnode->Next()) 
		{
			SmartGameObj * p_smart_obj = objnode->Data();
			WWASSERT(p_smart_obj != NULL);

			if (	p_smart_obj->As_SoldierGameObj() != NULL && 
					!p_smart_obj->Is_Delete_Pending() && 
					!p_smart_obj->Has_Player() &&
					p_smart_obj->Get_Player_Type() == my_team) {

				p_smart_obj->Set_Delete_Pending();
			}
		}
		*/

		/*
		//
		// Remove spawners... ugh how do I select just the appropriate ones?
		//
		DynamicVectorClass<SpawnerClass*> spawner_list = SpawnManager::Get_Spawner_List();

		for (int i = 0; i < spawner_list.Count(); i++) {

			WWASSERT(spawner_list[i] != NULL);

			if (spawner_list[i]->Get_Definition().Get_Player_Type() == my_team) {
				spawner_list[i]->Enable(false);
			}
		}
		*/
	}
}

//-----------------------------------------------------------------------------
void cGameDataSkirmish::Get_Description(WideStringClass & description)
{
	//
	// Call base class
	// 
	cGameData::Get_Description(description);

	const WideStringClass delimiter	= U_CHAR("\t");
	const WideStringClass newline		= U_CHAR("\n");
	const WideStringClass yes			= TRANSLATE(IDS_YES);
	const WideStringClass no			= TRANSLATE(IDS_NO);

	WideStringClass attribute(0, true);
	WideStringClass value(0, true);

	//
	// Starting Credits
	//
	attribute = TRANSLATE(IDS_MENU_TEXT349);
	value.Format(U_CHAR("%d"), StartingCredits);
	description += (attribute + delimiter + value + newline);

	//
	// Base Destruction Ends Game
	//
	attribute = TRANSLATE(IDS_MENU_TEXT314);
	value = (BaseDestructionEndsGame.Get() ? yes : no);
	description += (attribute + delimiter + value + newline);

	//
	// Beacon Placement Ends Game
	//
	attribute = TRANSLATE(IDS_MENU_TEXT315);
	value = (BeaconPlacementEndsGame.Get() ? yes : no);
	description += (attribute + delimiter + value + newline);
}
