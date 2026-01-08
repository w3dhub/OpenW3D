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
 *                     $Archive:: /Commando/Code/Commando/gdcnc.cpp                           $*
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/15/02 4:12p                                               $*
 *                                                                                             *
 *                    $Revision:: 47                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "gdcnc.h"

#include "specialbuilds.h"
#include "cnetwork.h"
#include "translatedb.h"
#include "string_ids.h"
#include "assets.h"
#include "multihud.h"
#include "playerdata.h"
#include "player.h"
#include "playertype.h"
#include "ConsoleMode.h"


//-----------------------------------------------------------------------------
cGameDataCnc::cGameDataCnc(void)	:
	IsPlaying (false),
	cGameData()
{
   //WWDEBUG_SAY(("cGameDataCnc::cGameDataCnc\n"));

	Set_Time_Limit_Minutes(25);
	Set_Ini_Filename("svrcfg_cnc.ini");
	Set_Ip_And_Port();
	BaseDestructionEndsGame.Set(true);
	BeaconPlacementEndsGame.Set(true);
	StartingCredits = 0;
	IsTeamChangingAllowed.Set(false);
	RemixTeams.Set(true);
	Set_Radar_Mode(RADAR_TEAMMATES);
}

//-----------------------------------------------------------------------------
cGameDataCnc::~cGameDataCnc(void)
{
   //WWDEBUG_SAY(("cGameDataCnc::~cGameDataCnc\n"));
}

//-----------------------------------------------------------------------------
cGameDataCnc& cGameDataCnc::operator=(const cGameDataCnc& rhs)
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
const unichar_t* cGameDataCnc::Get_Static_Game_Name(void)
{
	return TRANSLATION(IDS_MP_GAME_TYPE_CNC);
}

//-----------------------------------------------------------------------------
void cGameDataCnc::Reset_Game(bool is_reloaded)
{
   WWDEBUG_SAY(("cGameDataCnc::Reset_Game\n"));

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
cGameDataCnc::On_Game_Begin (void)
{
   WWDEBUG_SAY(("cGameDataCnc::On_Game_Begin\n"));

	cGameData::On_Game_Begin();

	CombatManager::Set_Beacon_Placement_Ends_Game(BeaconPlacementEndsGame.Get());

	//
	//	Initialize the bases
	//
	IsPlaying = true;
	return ;
}

//-----------------------------------------------------------------------------
void
cGameDataCnc::On_Game_End (void)
{
   WWDEBUG_SAY(("cGameDataCnc::On_Game_End\n"));

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
cGameDataCnc::Soldier_Added (SoldierGameObj *soldier)
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
void cGameDataCnc::Think(void)
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

	return ;
}

//-----------------------------------------------------------------------------
void cGameDataCnc::Set_Starting_Credits(int credits)
{
   Set_Generic_Num(credits, 0, MAX_CREDITS, StartingCredits);
}

//-----------------------------------------------------------------------------
void cGameDataCnc::Export_Tier_2_Data(cPacket & packet)
{
	cGameData::Export_Tier_2_Data(packet);

	packet.Add(BaseDestructionEndsGame.Get());
	packet.Add(BeaconPlacementEndsGame.Get());
	packet.Add(StartingCredits);
}

//-----------------------------------------------------------------------------
void cGameDataCnc::Import_Tier_2_Data(cPacket & packet)
{
	cGameData::Import_Tier_2_Data(packet);

   bool	b_placeholder;
   int	i_placeholder;

	BaseDestructionEndsGame.Set(	packet.Get(b_placeholder));
	BeaconPlacementEndsGame.Set(	packet.Get(b_placeholder));
	Set_Starting_Credits(			packet.Get(i_placeholder));
}

//-----------------------------------------------------------------------------
void cGameDataCnc::Base_Destruction_Score_Tweaking(void)
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
bool cGameDataCnc::Is_Game_Over(void)
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
			Base_Destruction_Score_Tweaking();

			if (BaseGDI.Did_Beacon_Destroy_Base() || BaseNOD.Did_Beacon_Destroy_Base()) {
				Set_Win_Type(WIN_TYPE_BEACON);
			} else {
				Set_Win_Type(WIN_TYPE_BASE_DESTRUCTION);
			}
		}
	}

	//
	// Beacon placement end game works via above base destruction method.
	//

	return is_game_over;
}

//-----------------------------------------------------------------------------
void cGameDataCnc::Load_From_Server_Config(void)
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

	b = p_ini->Get_Bool(	INI_SECTION_NAME, "DoMapsLoop",	DoMapsLoop);
	DoMapsLoop = b;

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
void cGameDataCnc::Save_To_Server_Config(void)
{
	cGameData::Save_To_Server_Config(Get_Ini_Filename());

   INIClass * p_ini = Get_INI(Get_Ini_Filename());
   WWASSERT(p_ini != NULL);


	p_ini->Put_Bool(	INI_SECTION_NAME, "IsFriendlyFirePermitted",	IsFriendlyFirePermitted.Get());
	p_ini->Put_Bool(	INI_SECTION_NAME, "DoMapsLoop",					DoMapsLoop);
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
void cGameDataCnc::Show_My_Money(void)
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
void cGameDataCnc::Show_Game_Settings_Limits(void)
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
bool cGameDataCnc::Is_Limited(void) const
{
	return Is_Time_Limit() || BaseDestructionEndsGame.Is_True();
}

//-----------------------------------------------------------------------------
void cGameDataCnc::Get_Description(WideStringClass & description)
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



#define PRINT_CONFIG_ERROR	ConsoleBox.Print("File %s - Error:\r\n\t ", Get_Ini_Filename());

//-----------------------------------------------------------------------------
bool cGameDataCnc::Is_Valid_Settings(WideStringClass& outMsg, bool check_as_server)
{
	/*
	** Check the base settings first.
	*/
	if (!cGameData::Is_Valid_Settings(outMsg, check_as_server)) {
		return(false);
	}

#ifdef FREEDEDICATEDSERVER
	if (BaseDestructionEndsGame.Is_False() && BeaconPlacementEndsGame.Is_True()) {
		//cHelpText::Set(TRANSLATION(IDS_MP_GAMEMODE_NEEDS_LIMIT));
		PRINT_CONFIG_ERROR;
		ConsoleBox.Print("Cannot allow beacons to end game without base destruction ending game.\n\n");
		outMsg = TRANSLATE(IDS_HOPTERR_NO_GAMEEND);
		return false;
	}
#endif

	return(true);
}

//-----------------------------------------------------------------------------
bool cGameDataCnc::Is_Gameplay_Permitted(void)
{
	bool permitted = cGameData::Is_Gameplay_Permitted();

	if (permitted && Get_Max_Players() > 1)
	{
		//
		// If your maxplayers is set to more than 1 then you must have an opponent.
		//
		permitted = 
			cPlayerManager::Tally_Team_Size(PLAYERTYPE_NOD) > 0 &&
			cPlayerManager::Tally_Team_Size(PLAYERTYPE_GDI) > 0;
	}

	return permitted;
}









/*
void cGameDataCnc::Get_Description(WideStringClass & description)
{
	//
	// Call base class
	//
	cGameData::Get_Description(description);

	const WideStringClass newline	= U_CHAR("\n");
	const WideStringClass yes		= U_CHAR("yes");
	const WideStringClass no		= U_CHAR("no");

	WideStringClass line(0, true);

   line.Format(U_CHAR("Starting Credits\t%d"), StartingCredits);
	IDS_MENU_TEXT349
	description += line;
	description += newline;

   line.Format(U_CHAR("Base Destruction Ends Game\t%s"),		BaseDestructionEndsGame.Get() ? yes : no);
	IDS_MENU_TEXT314
	description += line;
	description += newline;

   line.Format(U_CHAR("Beacon Placement Ends Game\t%s"),		BeaconPlacementEndsGame.Get() ? yes : no);
	IDS_MENU_TEXT315
	description += line;
	description += newline;
}
*/

//-----------------------------------------------------------------------------