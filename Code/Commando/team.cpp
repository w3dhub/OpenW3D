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
 *                     $Archive:: /Commando/Code/Commando/team.cpp                      $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 2/05/02 11:22a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 58                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "team.h"

#include <stdio.h>

#include "teammanager.h"
#include "miscutil.h"
#include "cnetwork.h"
#include "playermanager.h"
#include "gamedata.h"
#include "wwdebug.h"
#include "useroptions.h"
#include "playertype.h"
#include "colors.h"
#include "networkobjectfactory.h"
#include "netclassids.h"
#include "gameobjmanager.h"
#include "translatedb.h"
#include "string_ids.h"
#include "multihud.h"
#include "apppackettypes.h"
#include "devoptions.h"
#include "gamespyadmin.h"

DECLARE_NETWORKOBJECT_FACTORY(cTeam, NETCLASSID_TEAM);

//------------------------------------------------------------------------------------
cTeam::cTeam(void)
{
	Reset();

	Set_App_Packet_Type(APPPACKETTYPE_TEAM);

	cTeamManager::Add(this);
}

//------------------------------------------------------------------------------------
cTeam::~cTeam(void)
{
   //WWDEBUG_SAY(("* Destroying team object %d (%s)\n", TeamNumber, Name));
}

//------------------------------------------------------------------------------------
void cTeam::Init(int team_number)//, WideStringClass & name)
{
   WWASSERT(team_number >= 0 && team_number < MAX_TEAMS);
   WWASSERT(cNetwork::I_Am_Server());

   //WWDEBUG_SAY(("* Adding team object %d (%s)\n", team_number, name));

	Set_Object_Dirty_Bit(NetworkObjectClass::BIT_CREATION, true);

	/* 
	//
	// This doesn't work because at this point we are not yet a client
	//
	if (cNetwork::I_Am_Client()) {
		//
		// Turn off for self
		//
		Set_Object_Dirty_Bit(cNetwork::Get_My_Id(), NetworkObjectClass::BIT_CREATION, false);
	}
	/**/

   TeamNumber = team_number;
	Init_Team_Name();

	//
	// Special-case the IDs because this code is executed before loading, 
	// and in loading the dynamic range is reset.
	//
	if (TeamNumber == PLAYERTYPE_NOD) {
		Set_Network_ID(NETID_NOD_TEAM);
	} else {
		WWASSERT(TeamNumber == PLAYERTYPE_GDI);
		Set_Network_ID(NETID_GDI_TEAM);
	}
}

//------------------------------------------------------------------------------------
void cTeam::Init_Team_Name(void)
{
	if (TeamNumber == PLAYERTYPE_NOD) {
		Name = TRANSLATION(IDS_MP_TEAMNAME_MISSIONS_TEAM_0);
	} else {
		WWASSERT(TeamNumber == PLAYERTYPE_GDI);
		Name = TRANSLATION(IDS_MP_TEAMNAME_MISSIONS_TEAM_1);
	}
}

//------------------------------------------------------------------------------------
void cTeam::Reset(void)
{
   Kills = 0;
   Deaths = 0;
   Score = 0;
   Money = 0;

	Set_Object_Dirty_Bit(NetworkObjectClass::BIT_RARE, true);
}

//------------------------------------------------------------------------------------
void cTeam::Set_Kills(int new_kills)
{
   WWASSERT(cNetwork::I_Am_Only_Client());
   WWASSERT(new_kills >= 0);
   
   Kills = new_kills;

	Set_Object_Dirty_Bit(NetworkObjectClass::BIT_RARE, true);
}

//------------------------------------------------------------------------------------
void cTeam::Set_Deaths(int new_deaths)
{
   WWASSERT(cNetwork::I_Am_Only_Client());
   WWASSERT(new_deaths >= 0);

   Deaths = new_deaths;

	Set_Object_Dirty_Bit(NetworkObjectClass::BIT_RARE, true);
}

//------------------------------------------------------------------------------------
void cTeam::Set_Score(float new_score)
{
   //WWASSERT(cNetwork::I_Am_Only_Client());

   Score = new_score;

	Set_Object_Dirty_Bit(NetworkObjectClass::BIT_OCCASIONAL, true);
}

//------------------------------------------------------------------------------------
int cTeam::Tally_Size(void) const
{
   //
   // For the sake of simplicity/at the cost of efficiency, we calculate
   // team size, rather than storing and maintaining it.
   // To remind you of this I use "tally" instead of "get".
   //
   
   return cPlayerManager::Tally_Team_Size(TeamNumber);
}

//------------------------------------------------------------------------------------
void cTeam::Increment_Kills(void)
{
	if (!CombatManager::Is_Gameplay_Permitted()) {
		return;
	}

   WWASSERT(cNetwork::I_Am_Server());

   Kills++;

	Set_Object_Dirty_Bit(NetworkObjectClass::BIT_RARE, true);
}

//------------------------------------------------------------------------------------
void cTeam::Increment_Deaths(void)
{
	if (!CombatManager::Is_Gameplay_Permitted()) {
		return;
	}

   WWASSERT(cNetwork::I_Am_Server());

   Deaths++;

	Set_Object_Dirty_Bit(NetworkObjectClass::BIT_RARE, true);
}

//------------------------------------------------------------------------------------
void cTeam::Increment_Score(float increment)
{
	if (!CombatManager::Is_Gameplay_Permitted()) {
		return;
	}

   WWASSERT(cNetwork::I_Am_Server());

   Score += increment;

	Set_Object_Dirty_Bit(NetworkObjectClass::BIT_OCCASIONAL, true);
}

//------------------------------------------------------------------------------------
float cTeam::Get_Kill_To_Death_Ratio(void) const 
{
	//
	// If Deaths = 0, return -1;
	//
	float ktd = -1;
	if (Deaths > 0) {
		ktd = Kills / (float) Deaths; 
	}

	return ktd;
}

//------------------------------------------------------------------------------------
void cTeam::Get_Team_String(int /* rank */, WideStringClass & string) const
{
   //
   // Compose a string description of a team's stats for display
   //
   
	string.Format(U_CHAR(""));

	WWASSERT(PTheGameData != NULL);
	//bool is_verbose = The_Game()->IsIntermission.Get() || MultiHUDClass::Get_Verbose_Lists();
	bool is_verbose = //force_verbose || 
		               The_Game()->IsIntermission.Is_True() || 
							//MultiHUDClass::Get_Verbose_Lists();
							(MultiHUDClass::Get_Playerlist_Format() == PLAYERLIST_FORMAT_FULL);

	WideStringClass substring;

   //
	// Num Players
	//
	substring.Format(U_CHAR("%-2d "), Tally_Size());
	string += substring;

   /*
	//
	// Standing
	//
	if (is_verbose) {
		substring.Format(U_CHAR(" %2d. "), rank);
		string += substring;
	}
	*/

	//
	// Name
	//
	//GAMESPY
	//substring.Format(U_CHAR("%-11s"), Name);
	if (cGameSpyAdmin::Is_Gamespy_Game()) {
		substring.Format(U_CHAR("%-36s"), Name);
	} else {
		substring.Format(U_CHAR("%-11s"), Name);
	}
   string += substring;


   //
	// Kills
	//
	if (is_verbose) {
      substring.Format(U_CHAR("%-8d"), Kills);
      string += substring;
   }

   //
	// Deaths
	//
	if (is_verbose) {
      substring.Format(U_CHAR("%-8d"), Deaths);
      string += substring;
   }

	//
	// Kill to Death ratio
	//
   if (is_verbose) {
		float ktd = Get_Kill_To_Death_Ratio();
		if (ktd >= 0) {
			substring.Format(U_CHAR("%-8.1f"), ktd);
		} else {
			substring.Format(U_CHAR("%-8s"), "-");
		}
      string += substring;
   }

	/**/
	//
	// Money
	//
	WWASSERT(PTheGameData != NULL);
	if ((The_Game()->Is_Cnc() || The_Game()->Is_Skirmish()) && is_verbose) {

#ifdef WWDEBUG
		bool show = cDevOptions::ShowMoney.Is_True() ||
#else
		bool show = 
#endif // WWDEBUG
			  cNetwork::I_Am_Only_Server() ||
			 (cNetwork::I_Am_Client() && 
			  cNetwork::Get_My_Player_Object() != NULL &&
			  (cNetwork::Get_My_Team_Number() == TeamNumber));

		if (show) {
	      substring.Format(U_CHAR("%-8d"), Tally_Money());
		} else {
	      substring.Format(U_CHAR("%-8s"), "-");
		}
      
      string += substring;
   }
	/**/

	//
	// Score
	//
	substring.Format(U_CHAR("%-8d"), (int) Score);
	string += substring;
}

//------------------------------------------------------------------------------------
Vector3 cTeam::Get_Color(void) const
{	
	return Get_Color_For_Team(TeamNumber);
}

//------------------------------------------------------------------------------------
int cTeam::Tally_Money(void) const
{
	int tally = 0;

	for (	SLNode<cPlayer> * player_node = cPlayerManager::Get_Player_Object_List()->Head();
			player_node != NULL; 
			player_node = player_node->Next ())
	{
		cPlayer * p_player = player_node->Data();
		WWASSERT(p_player != NULL);

		if (p_player->Get_Is_Active().Is_True() && 
			 p_player->Get_Player_Type() == TeamNumber) 
		{
			tally += (int) p_player->Get_Money();
		}
	}

	return tally;
}

//------------------------------------------------------------------------------------
void cTeam::Export_Creation(BitStreamClass &packet)
{
	NetworkObjectClass::Export_Creation (packet);

	packet.Add(TeamNumber);
   //packet.Add_Wide_Terminated_String(Name);//XXX
}

//------------------------------------------------------------------------------------
void cTeam::Import_Creation(BitStreamClass &packet)
{
	NetworkObjectClass::Import_Creation(packet);

   packet.Get(TeamNumber);
   //packet.Get_Wide_Terminated_String(Name, sizeof(Name));
	//packet.Get_Wide_Terminated_String(Name.Get_Buffer(256), 256);
	Init_Team_Name();

	WWASSERT(cNetwork::I_Am_Only_Client());
}

//------------------------------------------------------------------------------------
void cTeam::Export_Rare(BitStreamClass &packet)
{
	packet.Add(Kills);
	packet.Add(Deaths);
	//packet.Add(Score);
	////packet.Add(Money);

}

//------------------------------------------------------------------------------------
void cTeam::Import_Rare(BitStreamClass &packet)
{
	int placeholder = 0;
	//float float_placeholder = 0;

	Set_Kills(			packet.Get(placeholder));
   Set_Deaths(			packet.Get(placeholder));
   //Set_Score(			packet.Get(float_placeholder));
   ////Set_Money(			packet.Get(placeholder));

}

//------------------------------------------------------------------------------------
void cTeam::Export_Occasional(BitStreamClass &packet)
{
	packet.Add(Score);
	//packet.Add(Money);
}

//------------------------------------------------------------------------------------
void cTeam::Import_Occasional(BitStreamClass &packet)
{
	//int placeholder = 0;
	float float_placeholder = 0;

   Set_Score(			packet.Get(float_placeholder));
   //Set_Money(			packet.Get(placeholder));
}

//------------------------------------------------------------------------------------
void cTeam::Export_Frequent(BitStreamClass &/* packet */)
{
}

//------------------------------------------------------------------------------------
void cTeam::Import_Frequent(BitStreamClass &/* packet */)
{
}



















   //FlagCaps = 0;
   //FlagLosses = 0;
