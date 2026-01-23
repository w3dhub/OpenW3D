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
 *                     $Archive:: /Commando/Code/Commando/gamedata.cpp                        $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 11/01/02 4:32p                                              $*
 *                                                                                             *
 *                    $Revision:: 221                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "gamedata.h"

#include <stdio.h>
#include <string.h>

#include "win.h"
#include "miscutil.h"
#include "cnetwork.h"
#include "chatshre.h"
#include "wwdebug.h"
#include "netinterface.h"
#include "translatedb.h"
#include "playertype.h"
#include "teammanager.h"
#include "playermanager.h"
#include "assets.h"
#include "translatedb.h"
#include "string_ids.h"
#include "gamemode.h"
#include "savegame.h"
#include "bandwidth.h"
#include "gametype.h"
#include "systimer.h"
#include "objlibrary.h"
#include "scriptzone.h"
#include "aabox.h"
#include "rendobj.h"
#include "weaponbag.h"
#include "multihud.h"
#include "WWAudio.h"
#include "textdisplay.h"
#include "render2d.h"
#include "render2dsentence.h"
#include "wwprofile.h"
#include "winevent.h"
#include "realcrc.h"
#include "useroptions.h"
#include "gdsingleplayer.h"
#include "gdcnc.h"
#include "wolgmode.h"
#include "font3d.h"
#include "nat.h"
#include "csdamageevent.h"
#include "sctextobj.h"
#include "widestring.h"
#include "vehicle.h"
#include "WOLGameInfo.h"
#include "spawn.h"
#include "dlgcncwinscreen.h"
#include "dialogmgr.h"
#include "rawfile.h"
#include "ConsoleMode.h"
#include "gamedataupdateevent.h"
#include "stylemgr.h"
#include "modpackagemgr.h"
#include "modpackage.h"
#include "bandwidthcheck.h"
#include "ServerSettings.h"

#include "gamespyadmin.h"
#include "specialbuilds.h"
#include "demosupport.h"
#include "specialbuilds.h"

const int cGameData::MAX_TIME_LIMIT       = 999;


cGameData *	PTheGameData = NULL;

static WideStringClass renderer_time_text;
static WideStringClass renderer_dedicated_server_label;
static WideStringClass renderer_gameplay_label;
static unsigned renderer_dedicated_server_color;
static unsigned renderer_gameplay_color;

//
// Class statics
//
const StringClass			cGameData::INI_SECTION_NAME	= "Settings";
const float					cGameData::LIMITS_X_POS			= 0.8f;
Render2DSentenceClass	*		cGameData::PTextRenderer		= NULL;
int							cGameData::HostedGameNumber	= 0;
bool							cGameData::IsManualRestart		= false;
bool							cGameData::IsManualExit			= false;
WideStringClass			cGameData::WinText;

//
// hack
//
unsigned int g_ip_override = INADDR_NONE;

//------------------------------------------------------------------------------------
void cGameData::Onetime_Init(void)
{
	WWDEBUG_SAY(("cGameData::Onetime_Init\n"));
	if (!ConsoleBox.Is_Exclusive()) {
		PTextRenderer = new Render2DSentenceClass;
		StyleMgrClass::Assign_Font(PTextRenderer, StyleMgrClass::FONT_INGAME_TXT);
		PTextRenderer->Set_Mono_Spaced(true);
		PTextRenderer->Set_Clipping_Rect(Render2DClass::Get_Screen_Resolution());
	}
}

//------------------------------------------------------------------------------------
void cGameData::Onetime_Shutdown(void)
{
	WWDEBUG_SAY(("cGameData::Onetime_Shutdown\n"));

	if (!ConsoleBox.Is_Exclusive()) {
		WWASSERT(PTextRenderer != NULL);
		delete PTextRenderer;
		PTextRenderer = NULL;
	}
}

//-----------------------------------------------------------------------------
cGameData::cGameData(void)	:
	DoExeVersionsMatch (true),
	DoStringVersionsMatch (true),
	DoMapsLoop (true),
	IsMapCycleOver (false),
	MapCycleIndex(0)
{
   //WWDEBUG_SAY(("cGameData::cGameData\n"));

	IsIntermission.Set(				false);
	IsDedicated.Set(					false);
	IsAutoRestart.Set(				false);
	IsFriendlyFirePermitted.Set(	false);
	IsTeamChangingAllowed.Set(		true);
	IsPassworded.Set(					false);
	IsFreeWeapons.Set(				false);
	IsLaddered.Set(					false);
	IsClanGame.Set(					false);
	IsClientTrusted.Set(				true);
	RemixTeams.Set(					false);
	CanRepairBuildings.Set(			true);
	DriverIsAlwaysGunner.Set(		true);
	SpawnWeapons.Set(					false);

	mWinnerID							= -1;
	MaxPlayers							= 16;
	TimeLimitMinutes					= 0;
	RadarMode							= RADAR_ALL;
	IniFilename							= "";
	Motd.Format(U_CHAR(""));

	Set_Password(						U_CHAR(""));
   //Set_Owner(							"UNOWNED");
   Set_Owner(							cNetInterface::Get_Nickname());

	IntermissionTimeSeconds			= 15;
   Set_Version_Number(				cNetwork::Get_Exe_Key());
	Set_Ip_Address(0);
	Set_Port(							4848);//TSS - hardcoded port

   Set_Current_Players(0);
	IntermissionTimeRemaining		= 0;
	TimeRemainingSeconds				= 0;
	MaximumWorldDistance				= 0;
	MinQualifyingTimeMinutes		= 2;

	IsQuickMatchServer				= false;
	//ServerIsGameplayPermitted		= false;
	WinType								= WIN_TYPE_TIME;
	GameDurationS						= 0;
	LastServerConfigModTime			= 0;
	SettingsDescription = 			TRANSLATE(IDS_SERVER_SAVELOAD_CUSTOM_DEFAULT);

	Clear_Clans();

	MvpCount = 0;

#ifdef MULTIPLAYERDEMO
	MapName.Format("C&C_Under.mix");
#endif // MULTIPLAYERDEMO
}

//-----------------------------------------------------------------------------
cGameData::~cGameData(void)
{
   //WWDEBUG_SAY(("cGameData::~cGameData\n"));
}

//-----------------------------------------------------------------------------
cGameData& cGameData::operator=(const cGameData& rhs)
{
	IsIntermission						= rhs.IsIntermission;

	IsDedicated							= rhs.IsDedicated;
	IsAutoRestart						= rhs.IsAutoRestart;
	IsFriendlyFirePermitted			= rhs.IsFriendlyFirePermitted;
	IsTeamChangingAllowed			= rhs.IsTeamChangingAllowed;
	IsPassworded						= rhs.IsPassworded;
	IsFreeWeapons						= rhs.IsFreeWeapons;
	IsLaddered							= rhs.IsLaddered;
	IsClanGame							= rhs.IsClanGame;
	IsClientTrusted					= rhs.IsClientTrusted;
	RemixTeams							= rhs.RemixTeams;
	CanRepairBuildings				= rhs.CanRepairBuildings;
	DriverIsAlwaysGunner				= rhs.DriverIsAlwaysGunner;
	SpawnWeapons						= rhs.SpawnWeapons;
   Password								= rhs.Password;
	GameTitle							= rhs.GameTitle;
	Motd									= rhs.Motd;

#ifdef MULTIPLAYERDEMO
	MapName.Format("C&C_Under.mix");
#else
   MapName								= rhs.MapName;
	ModName								= rhs.ModName;
#endif

	Owner									= rhs.Owner;

   MaxPlayers							= rhs.MaxPlayers;
   TimeLimitMinutes					= rhs.TimeLimitMinutes;
	IntermissionTimeSeconds			= rhs.IntermissionTimeSeconds;
   VersionNumber						= rhs.VersionNumber;
	IpAddress							= rhs.IpAddress;
	Port									= rhs.Port;
	IniFilename							= rhs.IniFilename;
	RadarMode							= rhs.RadarMode;

	CurrentPlayers						= rhs.CurrentPlayers;
	IntermissionTimeRemaining		= rhs.IntermissionTimeRemaining;
	TimeRemainingSeconds				= rhs.TimeRemainingSeconds;
	MaximumWorldDistance				= rhs.MaximumWorldDistance;
	MinQualifyingTimeMinutes		= rhs.MinQualifyingTimeMinutes;

	for (int clanSlot = 0; clanSlot < MAX_CLAN_SLOTS; ++clanSlot) {
		mClanSlots[clanSlot] = rhs.mClanSlots[clanSlot];
	}

	return (*this);
}

//-----------------------------------------------------------------------------
void cGameData::Reset_Game(bool is_reloaded)
{
   WWDEBUG_SAY(("cGameData::Reset_Game\n"));

	//
	// Game_Reset is called on both C & S at the beginning of every game.
	//
	mWinnerID = -1;

	IsIntermission.Set(false);

	//
	// 12/11/01
	// This dialog flush was added to remove the purchase screen because it persists
	// over reload.
	//
	if (GameModeManager::Find ("Combat")->Is_Active ()) {
		if (GameModeManager::Find ("Menu")->Is_Active ()) {
			GameModeManager::Find ("Menu")->Deactivate ();
		} else {
			DialogMgrClass::Flush_Dialogs ();
		}
	}

	if (Get_Text_Display()) {
   	WWASSERT(Get_Text_Display() != NULL);
   	Get_Text_Display()->Flush();
	}

	if (!IS_MISSION) {
		WWAudioClass::Get_Instance()->Create_Instant_Sound("Game_Start", Matrix3D(1));
	}

	//TSS092601 if ( !is_reloaded ) {
	if ( !IS_MISSION || !is_reloaded ) {
		cPlayerManager::Reset_Players();
	}
	//}
   cTeamManager::Reset_Teams();

	if (Is_Time_Limit()) {
		Reset_Time_Remaining_Seconds();

		if (cNetwork::I_Am_Server()) {
			cGameDataUpdateEvent * p_event = new cGameDataUpdateEvent();
			p_event->Init(-1);
		}
   }

	if (cNetwork::I_Am_Server())
	{
		HostedGameNumber++;
	}
}

//-----------------------------------------------------------------------------
void cGameData::Swap_Team_Sides(void)
{
	//WWASSERT(Is_Team_Game());
	WWASSERT(cNetwork::I_Am_Server());

	for (
		SLNode<cPlayer> * player_node = cPlayerManager::Get_Player_Object_List()->Head();
		player_node;
		player_node = player_node->Next()) {

		cPlayer * p_player = player_node->Data();
		WWASSERT(p_player != NULL);

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
	}

	//
	// Inform the players
	//
	//WideStringClass text;
	//text.Format(U_CHAR("_TEAMS_SWAPPED_!_"));
	WideStringClass text = TRANSLATE(IDS_MP_TEAMS_SWAPPED);
	cScTextObj * p_text_obj = new cScTextObj();
	p_text_obj->Init(text, TEXT_MESSAGE_PUBLIC, false, HOST_TEXT_SENDER, -1);
}

//-----------------------------------------------------------------------------
void cGameData::Remix_Team_Sides(void)
{
	//WWASSERT(Is_Team_Game());
	WWASSERT(cNetwork::I_Am_Server());

	WWASSERT(IsClanGame.Is_False() && "Remix teams not allowed");

	if (IsClanGame.Is_True()) {
		return;
	}

	//
	// Randomly choose a team for each player
	//
	for (
		SLNode<cPlayer> * player_node = cPlayerManager::Get_Player_Object_List()->Head();
		player_node;
		player_node = player_node->Next()) {

		cPlayer * p_player = player_node->Data();
		WWASSERT(p_player != NULL);

		int new_team = PLAYERTYPE_NOD;
		if (rand() % 2 == 0)
		{
			new_team = PLAYERTYPE_GDI;
		}
		else
		{
			new_team = PLAYERTYPE_NOD;
		}

		p_player->Set_Player_Type(new_team);
	}

	//
	// Inform the players
	//
	//WideStringClass text;
	//text.Format(U_CHAR("_TEAMS_REMIXED_!_"));
	WideStringClass text = TRANSLATE(IDS_MP_TEAMS_REMIXED);
	cScTextObj * p_text_obj = new cScTextObj();
	p_text_obj->Init(text, TEXT_MESSAGE_PUBLIC, false, HOST_TEXT_SENDER, -1);
}

//-----------------------------------------------------------------------------
void cGameData::Rebalance_Team_Sides(void)
{
	//WWASSERT(Is_Team_Game());
	WWASSERT(cNetwork::I_Am_Server());

	WWASSERT(IsClanGame.Is_False() && "Rebalance teams not allowed");

	if (IsClanGame.Is_True()) {
		return;
	}

	//
	// Rebalance teams numerically
	//
	cTeam * p_nod = cTeamManager::Find_Team(PLAYERTYPE_NOD);
	WWASSERT(p_nod != NULL);

	cTeam * p_gdi = cTeamManager::Find_Team(PLAYERTYPE_GDI);
	WWASSERT(p_gdi != NULL);

	int move_count = 0;

	int nod_tally = 0;
	int gdi_tally = 0;

	do
	{
		nod_tally = p_nod->Tally_Size();
		gdi_tally = p_gdi->Tally_Size();

		if (nod_tally > gdi_tally + 1)
		{
			cPlayer * p_player = cPlayerManager::Find_Team_Player(PLAYERTYPE_NOD);
			WWASSERT(p_player != NULL);
			p_player->Set_Player_Type(PLAYERTYPE_GDI);
			move_count++;
		}
		else if (gdi_tally > nod_tally + 1)
		{
			cPlayer * p_player = cPlayerManager::Find_Team_Player(PLAYERTYPE_GDI);
			WWASSERT(p_player != NULL);
			p_player->Set_Player_Type(PLAYERTYPE_NOD);
			move_count++;
		}

	} while (::abs(nod_tally - gdi_tally) > 1);

	if (move_count > 0) {
		//
		// Inform the players
		//
		//WideStringClass text;
		//text.Format(U_CHAR("_TEAMS_REBALANCED_!_"));
		WideStringClass text = TRANSLATE(IDS_MP_TEAMS_REBALANCED);
		cScTextObj * p_text_obj = new cScTextObj();
		p_text_obj->Init(text, TEXT_MESSAGE_PUBLIC, false, HOST_TEXT_SENDER, -1);
	}
}

//-----------------------------------------------------------------------------
void cGameData::Set_Ip_And_Port(void)
{
	/*
	struct sockaddr_in local_address;
   bool retcode = cNetUtil::Get_Local_Address(&local_address);
	WWASSERT(retcode == true);

	//
	// hack
	//
	if (g_ip_override != INADDR_NONE) {
		local_address.sin_addr.s_addr = g_ip_override;
	}

	Set_Ip_Address(local_address.sin_addr.s_addr);
	*/

	unsigned int ip = 0;
	if (cGameSpyAdmin::Get_Is_Server_Gamespy_Listed()) {
		ip = cUserOptions::PreferredGameSpyNic.Get();
	} else {
		ip = cUserOptions::PreferredLanNic.Get();
	}

	if (g_ip_override != INADDR_NONE) {
		ip = g_ip_override;
	}

	//
	// IP is set automatically in WOL unless there is an IP override and a port overrride.
	//
	if (GameModeManager::Find("WOL")->Is_Active()) {

		if (g_ip_override == INADDR_NONE || WOLNATInterface.Get_Force_Port() == 0) {
			unsigned int temp = FirewallHelper.Get_Local_Address();
			if (temp) {
				//ip = temp;
				ip = ::ntohl(temp);
			}
		}
	}

	//WWASSERT(ip != 0);
	Set_Ip_Address(ip);
}

//-----------------------------------------------------------------------------
//
// A convenience function...
//
bool cGameData::Set_Generic_Num(int generic_num,
   int lower_bound, int upper_bound, int & set_num)
{
   if (generic_num >= lower_bound && generic_num <= upper_bound) {
      set_num = generic_num;
      return true;
   } else {
      return false;
   }
}

//-----------------------------------------------------------------------------
bool cGameData::Set_Max_Players(int max)
{
   return Set_Generic_Num(max, 0, MAX_PLAYERS, MaxPlayers);
}

//-----------------------------------------------------------------------------
bool cGameData::Set_Time_Limit_Minutes(int time_limit_minutes)
{
   return Set_Generic_Num(time_limit_minutes, 0, MAX_TIME_LIMIT, TimeLimitMinutes);
}

//-----------------------------------------------------------------------------
void cGameData::Set_Radar_Mode(RadarModeEnum mode)
{
   RadarMode = mode;
}

//-----------------------------------------------------------------------------
void cGameData::Set_Intermission_Time_Seconds(int time)
{
	WWASSERT(time >= 0);
	IntermissionTimeSeconds = time;
}

//-----------------------------------------------------------------------------
void cGameData::Set_Motd(const unichar_t * motd)
{
	WWASSERT(motd != NULL);

	Motd = motd;
}


//-----------------------------------------------------------------------------
void cGameData::Set_Mod_Name(const StringClass &mod_name)
{
	ModName = mod_name;
	ModPackageMgrClass::Set_Current_Package (ModName);
	return ;
}

//-----------------------------------------------------------------------------
void cGameData::Set_Map_Name(const StringClass & map_name)
{
#ifdef MULTIPLAYERDEMO
	MapName.Format("C&C_Under.mix");
#else
	MapName = map_name;
#endif
}

//-----------------------------------------------------------------------------
bool cGameData::Set_Current_Players(int current)
{
   return Set_Generic_Num(current, 0, MAX_PLAYERS, CurrentPlayers);
}

//-----------------------------------------------------------------------------
void cGameData::Set_Owner(const WideStringClass & owner)
{
   Owner = owner;
}

//-----------------------------------------------------------------------------
void cGameData::Set_Ip_Address(unsigned int ip_address)
{
	IpAddress = ip_address;
}

//-----------------------------------------------------------------------------
void cGameData::Set_Port(int port)
{
   WWASSERT(port >= MIN_SERVER_PORT && port <= MAX_SERVER_PORT);
	Port = port;
}

//-----------------------------------------------------------------------------
bool cGameData::Is_Limited(void) const
{
	return Is_Time_Limit();
}

//-----------------------------------------------------------------------------
bool cGameData::Is_Map_Valid(char **out_filename)
{
	bool map_exists = false;
	if (ModName.Is_Empty () == false) {
		if (out_filename) {
			*out_filename = ModName.Peek_Buffer();
		}
		map_exists = cMiscUtil::File_Exists (ModName);
	} else {
		if (out_filename) {
			*out_filename = MapName.Peek_Buffer();
		}
		map_exists = cMiscUtil::File_Exists (MapName);
	}

	return(map_exists);
}


#define PRINT_CONFIG_ERROR	ConsoleBox.Print("File %s - Error:\r\n\t ", Get_Ini_Filename());

//-----------------------------------------------------------------------------
bool cGameData::Is_Valid_Settings(WideStringClass& outMsg, bool check_as_server)
{
	if (MapName.Is_Empty()) {
		//cHelpText::Set(TRANSLATION(IDS_MP_NO_LEVEL_SELECTED));
		Debug_Say(( "cGameData::No level file selected.\n" ));
		PRINT_CONFIG_ERROR;
		ConsoleBox.Print("No initial level file selected\n\n");
		outMsg = TRANSLATE(IDS_HOPTERR_NO_LEVEL);
		return false;
	} else {

		//
		//	Test to see whether or not the map exists
		//
		char *filename = NULL;
		if (!Is_Map_Valid(&filename)) {
			Debug_Say(("cGameData::Is_Valid_Settings: Level \"%s\" not found\n", filename));
			PRINT_CONFIG_ERROR;
			ConsoleBox.Print("Map file '%s' not found\n\n", filename);
			outMsg.Format(TRANSLATE(IDS_HOPTERR_MAP_NOTFOUND), filename);
			return(false);
		}
	}

	if (IsPassworded.Is_True() && Password.Get_Length () == 0) {
		//cHelpText::Set(TRANSLATION(IDS_MP_BLANK_PASSWORD_ILLEGAL));
		Debug_Say(( "cGameData::Is_Valid_Settings: You can not use a blank password.\n" ));
		PRINT_CONFIG_ERROR;
		ConsoleBox.Print("Blank passwords are not allowed\n\n");
		outMsg = TRANSLATE(IDS_HOPTERR_BLANK_PASS);
		return false;
	}

#ifndef FREEDEDICATEDSERVER
	if (GameModeManager::Find("WOL")->Is_Active()) {
#endif //FREEDEDICATEDSERVER
		if (IsPassworded.Is_True() && Is_QuickMatch_Server()) {
			Debug_Say(("cGameData::Is_Valid_Settings: Quickmatch can not have passwords.\n" ));
			PRINT_CONFIG_ERROR;
			ConsoleBox.Print("A password cannot be used on a Quickmatch server\n\n");
			outMsg = TRANSLATE(IDS_HOPTERR_QM_HASPASS);
			return false;
		}

		if (Is_QuickMatch_Server() && IsLaddered.Is_False()) {
			Debug_Say(("cGameData::Is_Valid_Settings: Quickmatch must be laddered.\n" ));
			PRINT_CONFIG_ERROR;
			ConsoleBox.Print("A Quickmatch server must also be laddered\n\n");
			outMsg = TRANSLATE(IDS_HOPTERR_QM_NOLADDER);
			return false;
		}

		if (Is_QuickMatch_Server() && IsClanGame.Is_True()) {
			Debug_Say(("cGameData::Is_Valid_Settings: Quickmatch cannot be clanned.\n" ));
			PRINT_CONFIG_ERROR;
			ConsoleBox.Print("Clans are not allowed in a quickmatch server\n\n");
			outMsg = TRANSLATE(IDS_HOPTERR_QM_NOCLANS);
			return false;
		}

		if (IsLaddered.Is_True() && IsTeamChangingAllowed.Is_True()) {
			PRINT_CONFIG_ERROR;
			ConsoleBox.Print("Team changing not allowed on a laddered server\n\n");
			outMsg = TRANSLATE(IDS_HOPTERR_NO_TEAMCHANGE);
			return false;
		}

		if (IsClanGame.Is_True() && (IsTeamChangingAllowed.Is_True() || RemixTeams.Is_True())) {
			PRINT_CONFIG_ERROR;
			ConsoleBox.Print("Team changing or Team Remix not allowed on a clan game server.\n");
			outMsg = "Team changing or Team Remix not allowed on a clan game server."; //TRANSLATE(IDS_HOPTERR_NO_TEAMCHANGE);
			return false;
		}
#ifndef FREEDEDICATEDSERVER
	}
#endif //FREEDEDICATEDSERVER

#ifdef FREEDEDICATEDSERVER
	if (Get_Max_Players() == 0 && ServerSettingsClass::Get_Game_Mode() !=
		ServerSettingsClass::MODE_WOL) {
#else
	if (Get_Max_Players() == 0) {
#endif
		PRINT_CONFIG_ERROR;
		ConsoleBox.Print("Max player count must be greater than 0\n\n");
		outMsg = TRANSLATE(IDS_HOPTERR_MAXPLAYER_0);
		return(false);
	}

	// Check server only parameters
	if (check_as_server || ConsoleBox.Is_Exclusive()) {
		if (Get_Max_Players() > NetworkObjectClass::MAX_CLIENT_COUNT-1) {
			PRINT_CONFIG_ERROR;
			ConsoleBox.Print("Max player count must be less than %d\n\n", NetworkObjectClass::MAX_CLIENT_COUNT);
			outMsg.Format(TRANSLATE(IDS_HOPTERR_MAXPLAYER_TOOBIG), NetworkObjectClass::MAX_CLIENT_COUNT);
			return(false);
		}

		if (DoMapsLoop && Get_Map_Cycle(0).Get_Length() == 0) {
			PRINT_CONFIG_ERROR;
			ConsoleBox.Print("Map cycling is enabled but there are no maps in the cycle\n\n");
			outMsg = TRANSLATE(IDS_HOPTERR_NO_MAPCYCLE);
			return(false);
		}

		/*
		** Make sure all the maps exist.
		*/
		if (DoMapsLoop && ModName.Is_Empty ()) {
			for (int i=0 ; i<MAX_MAPS ; i++) {
				StringClass map_name = Get_Map_Cycle(i);
				if (map_name.Get_Length()) {
					char filename[_MAX_PATH];
					sprintf(filename, "data\\%s", map_name.Peek_Buffer());
					RawFileClass file(filename);
					if (!file.Is_Available()) {
						PRINT_CONFIG_ERROR;
						ConsoleBox.Print("Map file '%s' not found\n\n", filename);
						outMsg.Format(TRANSLATE(IDS_HOPTERR_MAP_NOTFOUND), filename);
						return(false);
					}
				}
			}
		}

		/*
		** Check the game end conditions.
		*/
		if (!Is_Limited()) {
			//cHelpText::Set(TRANSLATION(IDS_MP_GAMEMODE_NEEDS_LIMIT));
			PRINT_CONFIG_ERROR;
			ConsoleBox.Print("Game must end by time limit or base destruction\n\n");
			outMsg = TRANSLATE(IDS_HOPTERR_NO_GAMEEND);
			return false;
		}

		/*
		** Check the Message of the day for excessive length.
		*/
		if (Motd.Get_Length() > MAX_MOTD_LENGTH) {
			PRINT_CONFIG_ERROR;
			ConsoleBox.Print("Message of the day cannot exceed %d characters\n\n", MAX_MOTD_LENGTH);
			outMsg.Format(TRANSLATE(IDS_HOPTERR_MOTD_TOOBIG), MAX_MOTD_LENGTH);
			return false;
		}

		/*
		** Check the game title for length. The buffers are pretty much all 256 bytes but 100 seems a more reasonable figure.
		*/
		if (GameTitle.Get_Length() > 100) {
			PRINT_CONFIG_ERROR;
			ConsoleBox.Print("Game title cannot exceed %d characters\n\n", 100);
			outMsg.Format(TRANSLATE(IDS_HOPTERR_TITLE_TOOBIG), 100);
			return false;
		}
	}

	return true; // it's valid !
}


//-----------------------------------------------------------------------------
//
// Server options are exported in 2 tiers. Tier 1 is the basic parameters
// that show up in the list of games. Tier 2 is the remainder of the full
// set of server settings.
//
void cGameData::Export_Tier_1_Data(cPacket & packet)
{
	packet.Add(IpAddress);
	packet.Add_Wide_Terminated_String(Owner);
	packet.Add_Wide_Terminated_String(GameTitle, true);

	packet.Add(Port);
	packet.Add(CurrentPlayers);
   packet.Add(MaxPlayers);
	packet.Add(VersionNumber);

	//
	//	Send the individual CRC's (for internal use)
	//
	packet.Add(cNetwork::Get_Exe_CRC ());
	packet.Add(cNetwork::Get_Strings_CRC ());

	packet.Add(IsDedicated.Get());
	packet.Add(IsTeamChangingAllowed.Get());
	packet.Add(IsPassworded.Get());
	packet.Add(IsLaddered.Get());
	packet.Add(IsClanGame.Get());

#ifndef MULTIPLAYERDEMO
	packet.Add(::CRC_Stringi(MapName));
	packet.Add(::CRC_Stringi(ModName));
#endif // MULTIPLAYERDEMO
}

//-----------------------------------------------------------------------------
void cGameData::Import_Tier_1_Data(cPacket & packet)
{
	unsigned int ip_address = packet.Get(ip_address);
   Set_Ip_Address(ip_address);

	WideStringClass owner;
	packet.Get_Wide_Terminated_String(owner.Get_Buffer(256), 256);
   Set_Owner(owner);

	WideStringClass title;
	packet.Get_Wide_Terminated_String(title.Get_Buffer(256), 256, true);
   Set_Game_Title(title);

	int	i_placeholder;
	bool	b_placeholder;

   Set_Port(						packet.Get(i_placeholder));
   Set_Current_Players(			packet.Get(i_placeholder));
   Set_Max_Players(				packet.Get(i_placeholder));
   Set_Version_Number(			packet.Get(i_placeholder));

	//
	//	Compare the individual CRC's against our own
	//
	DoExeVersionsMatch		= (packet.Get(i_placeholder) == cNetwork::Get_Exe_CRC ());
	DoStringVersionsMatch	= (packet.Get(i_placeholder) == cNetwork::Get_Strings_CRC ());

	IsDedicated.Set(				packet.Get(b_placeholder));
	IsTeamChangingAllowed.Set(	packet.Get(b_placeholder));
	IsPassworded.Set(				packet.Get(b_placeholder));
	IsLaddered.Set(				packet.Get(b_placeholder));
	IsClanGame.Set(				packet.Get(b_placeholder));

#ifndef MULTIPLAYERDEMO
	//
	//	Get the CRC of the map and the mod
	//
	unsigned int map_name_crc =		packet.Get(map_name_crc);
	unsigned int mod_name_crc =		packet.Get(mod_name_crc);


	//
	//	Determine what the name of the mod and the map are from their CRC's
	//
	StringClass mod_name(0,true);
	StringClass map_name(0,true);
	if (ModPackageMgrClass::Get_Mod_Map_Name_From_CRC (mod_name_crc, map_name_crc, &mod_name, &map_name)) {
		Set_Mod_Name (mod_name);
		Set_Map_Name (map_name);
	} else {
		ModName = U_CHAR("");
		MapName = U_CHAR("");
	}

#endif // MULTIPLAYERDEMO

	return ;
}


//-----------------------------------------------------------------------------
bool cGameData::Does_Map_Exist (void)
{
	bool retval = true;

	//
	//	If the map and mod names are empty then it is due to the fact
	// that we couldn't find the map and/or mod when we looked
	//
	if (ModName.Is_Empty () && MapName.Is_Empty ()) {
		retval = false;
	}

	return retval;
}


//-----------------------------------------------------------------------------
void cGameData::Import_Tier_1_Data(const WOLGameInfo& gameInfo)
{
	Set_Map_Name(gameInfo.MapName());
	Set_Mod_Name(gameInfo.ModName());

	WideStringClass title(0, true);
	title.Convert_From(gameInfo.Title());
	Set_Game_Title(title);

	Set_Max_Players(gameInfo.MaxPlayers());
	Set_Current_Players(gameInfo.NumPlayers());

	// Set game flags
	IsDedicated.Set(gameInfo.IsDedicated());
	IsPassworded.Set(gameInfo.IsPassworded());
	IsLaddered.Set(gameInfo.IsLaddered());
	IsFriendlyFirePermitted.Set(gameInfo.IsFriendlyFire());
	IsFreeWeapons.Set(gameInfo.IsFreeWeapons());
	RemixTeams.Set(gameInfo.IsTeamRemix());
	IsTeamChangingAllowed.Set(gameInfo.IsTeamChange());
	IsClanGame.Set(gameInfo.IsClanGame());
	IsQuickMatchServer = gameInfo.IsQuickmatch();

	CanRepairBuildings.Set(gameInfo.IsRepairBuildings());
	DriverIsAlwaysGunner.Set(gameInfo.IsDriverGunner());
	SpawnWeapons.Set(gameInfo.IsSpawnWeapons());

	Clear_Clans();
	mClanSlots[0] = gameInfo.ClanID1();
	mClanSlots[1] = gameInfo.ClanID2();
}


//-----------------------------------------------------------------------------
void cGameData::Export_Tier_2_Data(cPacket & packet)
{
   packet.Add(TimeLimitMinutes);
   packet.Add((int) RadarMode);
   packet.Add(IntermissionTimeSeconds);
   packet.Add(MinQualifyingTimeMinutes);

	packet.Add(IsFriendlyFirePermitted.Get());
	packet.Add(IsFreeWeapons.Get());
	packet.Add(IsClientTrusted.Get());
	packet.Add(RemixTeams.Get());

	if (Is_Cnc()) {
		packet.Add(CanRepairBuildings.Get());
		packet.Add(DriverIsAlwaysGunner.Get());
		packet.Add(SpawnWeapons.Get());
	}

	packet.Add_Wide_Terminated_String(Motd, true);
}

//-----------------------------------------------------------------------------
void cGameData::Import_Tier_2_Data(cPacket & packet)
{
   int	i_placeholder = 0;
   bool	b_placeholder = false;

   Set_Time_Limit_Minutes(						packet.Get(i_placeholder));
   Set_Radar_Mode((RadarModeEnum)			packet.Get(i_placeholder));
   Set_Intermission_Time_Seconds(			packet.Get(i_placeholder));
   Set_Min_Qualifying_Time_Minutes(			packet.Get(i_placeholder));

	IsFriendlyFirePermitted.Set(				packet.Get(b_placeholder));
	IsFreeWeapons.Set(							packet.Get(b_placeholder));
	IsClientTrusted.Set(							packet.Get(b_placeholder));
	RemixTeams.Set(								packet.Get(b_placeholder));

	if (Is_Cnc()) {
		CanRepairBuildings.Set(					packet.Get(b_placeholder));
		DriverIsAlwaysGunner.Set(				packet.Get(b_placeholder));
		SpawnWeapons.Set(							packet.Get(b_placeholder));
	}

	WideStringClass motd;
	packet.Get_Wide_Terminated_String(motd.Get_Buffer(MAX_MOTD_LENGTH), MAX_MOTD_LENGTH, true);
	Set_Motd(motd);
}

//-----------------------------------------------------------------------------
void cGameData::Load_From_Server_Config(LPCSTR config_file)
{
	WWASSERT(config_file != NULL);
   WWASSERT(cMiscUtil::Is_String_Different(config_file, ""));

   INIClass * p_ini = Get_INI(config_file);
	StringClass full_filename(config_file, true);

	if (p_ini == NULL) {
      full_filename.Format("data\\%s", config_file);
      FILE * file = fopen(full_filename, "w");
	   fclose(file);

		p_ini = Get_INI(config_file);
   }
	WWASSERT(p_ini != NULL);

	LastServerConfigModTime = Get_Config_File_Mod_Time();

	char		map_name[MAX_MAPNAME_SIZE];
	char		mod_name[MAX_MAPNAME_SIZE];
	char		string8[256];
   bool		b;
   int		i;
   //float		f;


	//p_ini->Get_String(	INI_SECTION_NAME, "Password", Get_Password(), password, sizeof(password));
	//Set_Password(password);

   p_ini->Get_String(	INI_SECTION_NAME, "MapName", "", map_name, sizeof(map_name));
	StringClass map(map_name,true);
   Set_Map_Name(map);

   p_ini->Get_String(	INI_SECTION_NAME, "ModName", "", mod_name, sizeof(mod_name));
	StringClass mod(mod_name,true);
   Set_Mod_Name(mod);

	i = p_ini->Get_Int(	INI_SECTION_NAME, "TimeLimitMinutes",			Get_Time_Limit_Minutes());
	Set_Time_Limit_Minutes(i);

   i = p_ini->Get_Int(	INI_SECTION_NAME, "RadarMode",					Get_Radar_Mode());
	Set_Radar_Mode((RadarModeEnum) i);

   //i = p_ini->Get_Int(	INI_SECTION_NAME, "IntermissionTimeSeconds",	Get_Intermission_Time_Seconds());
	//Set_Intermission_Time_Seconds(i);
#ifndef FREEDEDICATEDSERVER
	i = p_ini->Get_Int(	INI_SECTION_NAME, "Port",							Get_Port());
	Set_Port(i);

	b = p_ini->Get_Bool(	INI_SECTION_NAME, "IsDedicated",					IsDedicated.Get());
	IsDedicated.Set(b);
#else ////FREEDEDICATEDSERVER
	IsDedicated.Set(true);
#endif //FREEDEDICATEDSERVER

	b = p_ini->Get_Bool(	INI_SECTION_NAME, "IsAutoRestart",				IsAutoRestart.Get());
	IsAutoRestart.Set(b);

	b = p_ini->Get_Bool(	INI_SECTION_NAME, "IsPassworded",				IsPassworded.Get());
	IsPassworded.Set(b);

	b = p_ini->Get_Bool(	INI_SECTION_NAME, "IsQuickMatch",				IsQuickMatchServer.Get());
	IsQuickMatchServer.Set(b);

	b = p_ini->Get_Bool(	INI_SECTION_NAME, "IsLaddered",					IsLaddered.Get());
#ifndef FREEDEDICATEDSERVER
	b = ((IsQuickMatchServer.Get() == true) ? true : b);
#endif //FREEDEDICATEDSERVER
	IsLaddered.Set(b);

	b = p_ini->Get_Bool(	INI_SECTION_NAME, "IsClanGame",					IsClanGame.Get());
#ifndef FREEDEDICATEDSERVER
	b = ((IsQuickMatchServer.Get() == true) ? false : b);
#endif //FREEDEDICATEDSERVER
	IsClanGame.Set(b);

	b = p_ini->Get_Bool(	INI_SECTION_NAME, "RemixTeams",					RemixTeams.Get());
	RemixTeams.Set(b);

	b = p_ini->Get_Bool(	INI_SECTION_NAME, "CanRepairBuildings",		CanRepairBuildings.Get());
	CanRepairBuildings.Set(b);

	b = p_ini->Get_Bool(	INI_SECTION_NAME, "DriverIsAlwaysGunner",		DriverIsAlwaysGunner.Get());
	DriverIsAlwaysGunner.Set(b);

	b = p_ini->Get_Bool(	INI_SECTION_NAME, "SpawnWeapons",				SpawnWeapons.Get());
	SpawnWeapons.Set(b);

	//
	// Disguise "IsClientTrusted" with something that sounds like you need it but
	// that doesn't scream "hack me"
	//
	//b = p_ini->Get_Bool(	INI_SECTION_NAME, "IsClientTrusted",			IsClientTrusted.Get());
	b = p_ini->Get_Bool(	INI_SECTION_NAME, "UseLagReduction",			IsClientTrusted.Get());
	IsClientTrusted.Set(b);

	// Read the 8 bit versions of the strings first and use those as defaults.
   p_ini->Get_String(INI_SECTION_NAME, "bGameTitle", "", string8, sizeof(string8));
	Set_Game_Title(WideStringClass(string8, true));

   p_ini->Get_String(INI_SECTION_NAME, "bMotd", "", string8, sizeof(string8));
	Set_Motd(WideStringClass(string8, true));

   p_ini->Get_String(INI_SECTION_NAME, "bPassword", "", string8, sizeof(string8));
	Set_Password(WideStringClass(string8, true));

	p_ini->Get_String(INI_SECTION_NAME, "bConfigName", "", string8, sizeof(string8));
	Set_Settings_Description(WideStringClass(string8, true));

	// Now read the 16 bit versions as overrides.
	WideStringClass wide_string(Get_Game_Title(), true);
	p_ini->Get_Wide_String(wide_string, INI_SECTION_NAME, "wGameTitle", wide_string.Peek_Buffer());
	Set_Game_Title(wide_string);

	wide_string = Get_Password();
	p_ini->Get_Wide_String(wide_string, INI_SECTION_NAME, "wPassword", wide_string.Peek_Buffer());
	Set_Password(wide_string);

	wide_string = Get_Motd();
	p_ini->Get_Wide_String(wide_string, INI_SECTION_NAME, "wMotd", wide_string.Peek_Buffer());
	Set_Motd(wide_string);

	wide_string = Get_Settings_Description();
	p_ini->Get_Wide_String(wide_string, INI_SECTION_NAME, "wConfigName", wide_string.Peek_Buffer());
	Set_Settings_Description(wide_string);

	for (int j = 0; j < MAX_MAPS; j++) {
		StringClass item_name(0,true);
		item_name.Format("MapName%02d", j);
		p_ini->Get_String(	INI_SECTION_NAME, item_name, "", map_name, sizeof(map_name));
		StringClass map(map_name,true);
		MapCycle[j] = map;
	}

	//char motd[2 * MAX_MOTD_LENGTH];
	//p_ini->Get_String(	INI_SECTION_NAME, "Motd", "", motd, sizeof(motd));
	//if (::strlen(motd) > 0) {
	//	Motd.Convert_From(motd);
	//}

   Release_INI(p_ini);
}

//-----------------------------------------------------------------------------
void cGameData::Save_To_Server_Config(LPCSTR config_file)
{
	WWASSERT(config_file != NULL);
   WWASSERT(cMiscUtil::Is_String_Different(config_file, ""));

   INIClass * p_ini = Get_INI(config_file);
   WWASSERT(p_ini != NULL);


	//
	// We can't overwrite entries, so clear them out first.
	//
	p_ini->Clear(INI_SECTION_NAME);
	p_ini->Put_Wide_String(	INI_SECTION_NAME, "wConfigName",					Get_Settings_Description());
   p_ini->Put_Wide_String(	INI_SECTION_NAME, "wPassword",					Get_Password());
	p_ini->Put_String(		INI_SECTION_NAME, "MapName",						Get_Map_Name());
	p_ini->Put_String(		INI_SECTION_NAME, "ModName",						Get_Mod_Name());
   p_ini->Put_Int(			INI_SECTION_NAME, "TimeLimitMinutes",			Get_Time_Limit_Minutes());
   p_ini->Put_Int(	   	INI_SECTION_NAME, "RadarMode",					Get_Radar_Mode());
   //p_ini->Put_Int(	   	INI_SECTION_NAME, "IntermissionTimeSeconds",	Get_Intermission_Time_Seconds());
   p_ini->Put_Int(			INI_SECTION_NAME, "Port",							Get_Port());
	p_ini->Put_Bool(			INI_SECTION_NAME, "IsDedicated",					IsDedicated.Get());
	p_ini->Put_Bool(			INI_SECTION_NAME, "IsAutoRestart",				IsAutoRestart.Get());
	p_ini->Put_Bool(			INI_SECTION_NAME, "IsPassworded",				IsPassworded.Get());
	p_ini->Put_Bool(			INI_SECTION_NAME, "IsQuickMatch",				IsQuickMatchServer.Get());
	p_ini->Put_Bool(			INI_SECTION_NAME, "IsLaddered",					IsLaddered.Get());
	p_ini->Put_Bool(			INI_SECTION_NAME, "IsClanGame",					IsClanGame.Get());
	p_ini->Put_Bool(			INI_SECTION_NAME, "RemixTeams",					RemixTeams.Get());
	p_ini->Put_Bool(			INI_SECTION_NAME, "CanRepairBuildings",		CanRepairBuildings.Get());
	p_ini->Put_Bool(			INI_SECTION_NAME, "DriverIsAlwaysGunner",		DriverIsAlwaysGunner.Get());
	p_ini->Put_Bool(			INI_SECTION_NAME, "SpawnWeapons",				SpawnWeapons.Get());
	//p_ini->Put_Bool(			INI_SECTION_NAME, "IsClientTrusted",			IsClientTrusted.Get());
	p_ini->Put_Bool(			INI_SECTION_NAME, "UseLagReduction",			IsClientTrusted.Get());
   p_ini->Put_Wide_String(	INI_SECTION_NAME, "wGameTitle",					Get_Game_Title());
   p_ini->Put_Wide_String(	INI_SECTION_NAME, "wMOTD",							Get_Motd());
#if (0)
	// Save out 8 bit string versions too.
	StringClass string8(256, true);
	WideStringClass(Get_Game_Title(), true).Convert_To(string8);
   p_ini->Put_String(		INI_SECTION_NAME, "bGameTitle",					string8);
	WideStringClass(Get_Motd(), true).Convert_To(string8);
   p_ini->Put_String(		INI_SECTION_NAME, "bMOTD",							string8);
	WideStringClass(Get_Password(), true).Convert_To(string8);
   p_ini->Put_String(		INI_SECTION_NAME, "bPassword",						string8);
#endif //(0)

	for (int j = 0; j < MAX_MAPS; j++) {
		StringClass item_name(0,true);
		item_name.Format("MapName%02d", j);
		p_ini->Put_String(	INI_SECTION_NAME, item_name,					MapCycle[j]);
	}

	if (!Motd.Is_Empty()) {
		StringClass motd;
		Motd.Convert_To(motd);
		p_ini->Put_String(	INI_SECTION_NAME, "Motd", motd.Peek_Buffer());
	}

	Save_INI(p_ini, config_file);
   Release_INI(p_ini);
}

//-----------------------------------------------------------------------------
void cGameData::Soldier_Added(SoldierGameObj * p_soldier)
{
	WWASSERT(p_soldier != NULL);

	if (IsFreeWeapons.Is_True() &&
		 p_soldier->Get_Player_Type() != PLAYERTYPE_NEUTRAL) {

		p_soldier->Give_All_Weapons();
		int weapon = 0;

		if (p_soldier->Is_Human_Controlled()) {
			weapon = 1;
		} else {
			weapon = 1 + rand() % 7;
		}

		WWASSERT(p_soldier->Get_Weapon_Bag() != NULL);
		p_soldier->Get_Weapon_Bag()->Select_Key_Number(weapon);
	}
}

//-----------------------------------------------------------------------------
void cGameData::Begin_Intermission(void)
{
	// If we are transitioning from one game to another we must first end the
	// previous game.
	GameModeClass* gameMode = GameModeManager::Find("WOL");

	if (gameMode && gameMode->Is_Active()) {
		WolGameModeClass* wolGame = reinterpret_cast<WolGameModeClass*>(gameMode);
		wolGame->End_Game();
	}

	IsIntermission.Set(true);
   IntermissionTimeRemaining = Get_Intermission_Time_Seconds();

	//
	//	Display a dialog with the win information on it
	//
	if (!ConsoleBox.Is_Exclusive()) {
		START_DIALOG (CNCWinScreenMenuClass);
	}
	return ;
}

//-----------------------------------------------------------------------------
void cGameData::Set_Maximum_World_Distance(float distance)
{
	//
	//	You will typically get this assert if you don't have the level file.
	//
	WWASSERT(distance > 0);
	MaximumWorldDistance = distance;
}

//-----------------------------------------------------------------------------
void cGameData::Set_Min_Qualifying_Time_Minutes(int minutes)
{
	WWASSERT(minutes >= 0);
	MinQualifyingTimeMinutes = minutes;
}

//-----------------------------------------------------------------------------
int cGameData::Get_Duration_Seconds(void)
{
	return (int)(TimeLimitMinutes * 60 - TimeRemainingSeconds);
}

//-----------------------------------------------------------------------------
WideStringClass cGameData::Get_Team_Word(void)
{
	//
	// In clan games, the work "Clan" is used instead of "Team"
	//

	if (IsClanGame.Is_True()) {
		return TRANSLATION(IDS_MP_CLAN);
	} else {
		return TRANSLATION(IDS_MP_TEAM);
	}
}

//-----------------------------------------------------------------------------
void cGameData::Set_Time_Remaining_Seconds(float time_remaining_seconds)
{
   WWASSERT(time_remaining_seconds >= 0);

	TimeRemainingSeconds = time_remaining_seconds;
}

//-----------------------------------------------------------------------------
void cGameData::Reset_Time_Remaining_Seconds(void)
{
	TimeRemainingSeconds = 60 * TimeLimitMinutes;
}

//-----------------------------------------------------------------------------
float cGameData::Get_Time_Remaining_Seconds(void)
{
	return TimeRemainingSeconds;
}

//-----------------------------------------------------------------------------
bool cGameData::Is_Valid_Player_Type(int player_type)
{
	bool is_valid = true;

	if (!Is_Single_Player()) {
		//if (player_type == PLAYERTYPE_RENEGADE &&
		//	 Is_Team_Game()) {
		if (player_type == PLAYERTYPE_RENEGADE) {
			is_valid = false;
		}

		/*
		if ((player_type == PLAYERTYPE_NOD || player_type == PLAYERTYPE_GDI) &&
			 !Is_Team_Game()) {
			is_valid = false;
		}
		*/
	}

	return is_valid;
}

//-----------------------------------------------------------------------------
int cGameData::Choose_Player_Type(cPlayer* player, int team_choice, bool is_grunt)
{
	if (IsClanGame.Is_True() && !is_grunt) {
		// Bots are assigned to the smallest team.
		if (player && !player->Is_Human()) {
			int team = Choose_Smallest_Team();

			WWDEBUG_SAY(("CLANS: Bot '%S' assigned to smallest team (%d)\n",
				(const unichar_t*)player->Get_Name(), team));

			return team;
		}

		// If there are other players from the same clan then team up them.
		// Otherwise we must be the first of our clan so select the next available team.
		cPlayer* clanMate = cPlayerManager::Find_Clan_Mate(player);

		if (clanMate) {
			int team = clanMate->Get_Player_Type();

			WWDEBUG_SAY(("CLANS: Player '%S' assigned to team with clanmates (%d)\n",
				(const unichar_t*)player->Get_Name(), team));

			return team;
		} else {
			int team = Choose_Available_Team(team_choice);

			WWDEBUG_SAY(("CLANS: Player '%S' assigned to available team (%d)\n",
				(const unichar_t*)player->Get_Name(), team));

			return team;
		}

		WWDEBUG_SAY(("CLANS: ERROR - Player not assigned to team\n"));
		WWASSERT("ERROR: Player not assigned to team");
	} else {
		if (PLAYERTYPE_RENEGADE == team_choice || IsTeamChangingAllowed.Is_False()) {
			return Choose_Smallest_Team();
		}
	}

	return team_choice;
}


int cGameData::Choose_Available_Team(int preference)
{
	// If the perferred team is available then use it.
	if (preference != -1) {
		cTeam* team = cTeamManager::Find_Team(preference);

		if (team && (team->Tally_Size() == 0)) {
			return team->Get_Id();
		}
	}

	// Find the next available team
	cTeam* team = cTeamManager::Find_Empty_Team();

	if (team) {
		return team->Get_Id();
	}

	WWDEBUG_SAY(("ERROR: No available team found\n"));
	WWASSERT("ERROR: No available team found");
	return -1;
}


//-----------------------------------------------------------------------------
int cGameData::Choose_Smallest_Team(void)
{
	//WWASSERT(Is_Team_Game());

	cTeam * p_team_0 = cTeamManager::Find_Team(0);
	WWASSERT(p_team_0 != NULL);
	cTeam * p_team_1 = cTeamManager::Find_Team(1);
	WWASSERT(p_team_1 != NULL);

	int team = -999;

	int size_team_0 = p_team_0->Tally_Size();
	int size_team_1 = p_team_1->Tally_Size();

	if (size_team_0 < size_team_1) {
		team = p_team_0->Get_Id();
	} else if (size_team_1 < size_team_0) {
		team = p_team_1->Get_Id();
	} else {
		//
		// Both teams are equal in size, so choose the losing team.
		//
		if (p_team_0->Get_Score() < p_team_1->Get_Score()) {
			team = p_team_0->Get_Id();
		} else if (p_team_0->Get_Score() > p_team_1->Get_Score()) {
			team = p_team_1->Get_Id();
		} else {
			//
			// Both teams have the same score. Choose Randomly.
			//
			if (rand() % 2 == 0) {
				team = p_team_0->Get_Id();
			} else {
				team = p_team_1->Get_Id();
			}
		}
	}

	WWASSERT(team != -999);

	return team;
}

//-----------------------------------------------------------------------------
bool cGameData::Is_Game_Over(void)
{
	WWASSERT(cNetwork::I_Am_Server());

	bool is_game_over = false;

   //
	// Games may be manually ended with the "gameover" command.
	//
	if (IsManualRestart) {
      IsManualRestart = false;
		WinType = WIN_TYPE_FORCED;
      is_game_over = true;
   }

	//
	// Games may be ended with the "quit" command.
	//
	if (!is_game_over && IsManualExit) {
		WinType = WIN_TYPE_FORCED;
      is_game_over = true;
	}

   //
   // Time expired?
   //
   if (!is_game_over && Is_Time_Limit() && TimeRemainingSeconds <= WWMATH_EPSILON)  {
		WinType = WIN_TYPE_TIME;
      is_game_over = true;
   }

	//
	// If game time has passed but gameplay now pends, end the game
	//
   if (!is_game_over && Get_Game_Duration_S() > 0 && !Is_Gameplay_Permitted())  {
		WinType = WIN_TYPE_FORCED;
      is_game_over = true;
   }

	/*
	//
	// THIS NEEDS REFINEMENT
	//
	// The following test will end a non-mission game when your opponents
	// abandon you. To determine whether any combat has occurred we simply
	// examine the IsBloodShed flag.
	//
	if (!Is_Mission() && IsBloodShed.Is_True() &&
      ((Is_Team_Game() && cTeamManager::Get_Living_Count() <= 1) ||
       (!Is_Team_Game() && cPlayerManager::Get_Living_Count() <= 1))) {
      return true;
   }
	*/

	if (is_game_over)
	{
		TimeRemainingSeconds = 0;
	}

	return is_game_over;
}




//-----------------------------------------------------------------------------
bool cGameData::Has_Config_File_Changed(void)
{
	unsigned int mod_time = Get_Config_File_Mod_Time();

	if (LastServerConfigModTime != mod_time) {
		return(true);
	}
	return(false);
}



//-----------------------------------------------------------------------------
unsigned int cGameData::Get_Config_File_Mod_Time(void)
{
	StringClass full_filename(IniFilename, true);
	RawFileClass file(full_filename);

	if (!file.Is_Available()) {
      full_filename.Format("data\\%s", IniFilename);
		file.Set_Name(full_filename);
   }

	if (file.Is_Available()) {
		file.Open();
		unsigned int mod_time = file.Get_Date_Time();
		file.Close();
		return(mod_time);
	}
	return(0);
}




//-----------------------------------------------------------------------------
void cGameData::Game_Over_Processing(void)
{
   WWDEBUG_SAY(("cGameData::Game_Over_Processing\n"));

	WWASSERT(cNetwork::I_Am_Server());

   int winning_team = cTeamManager::Get_Leaders_Id();

	//
	// Reload the game settings if they changed on disk.
	//
	if (Has_Config_File_Changed()) {
		//
		// Verify that the new settings are good.
		//
		cGameDataCnc *test_game_settings = (cGameDataCnc*) Create_Game_Of_Type(cGameData::GAME_TYPE_CNC);
		WWASSERT(test_game_settings != NULL);

		bool ok = false;

		if (test_game_settings) {
			test_game_settings->Set_Ini_Filename(IniFilename);
			test_game_settings->Load_From_Server_Config();

			WideStringClass outMsg;
			ok = test_game_settings->Is_Valid_Settings(outMsg, true);
			delete test_game_settings;
		}

		//
		// If they are good settings then reload them.
		//
		if (ok) {
			ConsoleBox.Print("Game settings changed - realoading settings\n");
			Load_From_Server_Config();
		}
	}

	Rotate_Map();

	cPlayerManager::Increment_Player_Times();

	//
	// Compute the game duration
	//
	unsigned int duration_s = (int)((TIMEGETTIME() - GameStartTimeMs) / 1000.0f);
	Set_Game_Duration_S(duration_s);

	//
	// Record the MVP name and track the number of consecutives
	//
	WideStringClass mvp_name = cPlayerManager::Determine_Mvp_Name();
	if (mvp_name.Compare_No_Case(MvpName)) {
		Set_Mvp_Count(1);
		Set_Mvp_Name(cPlayerManager::Determine_Mvp_Name());
	} else {
		Increment_Mvp_Count();
	}

	mWinnerID = winning_team;

	cWinEvent * p_win = new cWinEvent;
	p_win->Init(mWinnerID, PLAYER_ID_UNKNOWN, IsMapCycleOver);

	cPlayerManager::Compute_Ladder_Points(winning_team);

	cTeamManager::Log_Team_List();
	cPlayerManager::Log_Player_List();

	//
	// Results logs wrap back to 1 after 100, and overwrite.
	//
	int log_num = cUserOptions::ResultsLogNumber.Get();
	log_num++;
	if (log_num > 100)
	{
		log_num = 1;
	}
	cUserOptions::ResultsLogNumber.Set(log_num);


	Begin_Intermission();
}

//-----------------------------------------------------------------------------
bool cGameData::Is_Gameplay_Permitted(void)
{
	bool permitted = true;

	if (IsIntermission.Is_True()) {
		permitted = false;
	}

	if (!GameModeManager::Find ("Combat")->Is_Active ()) {
		permitted = false;
	}

	return permitted;
}

void cGameData::Set_Clan(int slot, unsigned int clanID)
{
	WWASSERT(slot >= 0 && slot < MAX_CLAN_SLOTS);
	mClanSlots[slot] = clanID;
}


unsigned int cGameData::Get_Clan(int slot) const
{
	WWASSERT(slot >= 0 && slot < MAX_CLAN_SLOTS);
	return mClanSlots[slot];
}


void cGameData::Clear_Clans(void)
{
	for (int slot = 0; slot < MAX_CLAN_SLOTS; ++slot) {
		mClanSlots[slot] = 0;
	}
}


int cGameData::Find_Free_Clan_Slot(void) const
{
	if (IsClanGame.Is_True()) {
		for (int slot = 0; slot < MAX_CLAN_SLOTS; ++slot) {
			if (mClanSlots[slot] == 0) {
				return slot;
			}
		}
	}

	return -1;
}


bool cGameData::Is_Clan_Competing(unsigned int clanID) const
{
	if (IsClanGame.Is_True() && (clanID != 0)) {
		for (int slot = 0; slot < MAX_CLAN_SLOTS; ++slot) {
			if (mClanSlots[slot] == clanID) {
				return true;
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
bool cGameData::Is_Clan_Game_Open(void) const
{
	if (IsClanGame.Is_True()) {
		for (int slot = 0; slot < MAX_CLAN_SLOTS; ++slot) {
			if (mClanSlots[slot] == 0) {
				return true;
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
cGameData * cGameData::Create_Game_Of_Type([[maybe_unused]] GameTypeEnum game_type)
{
	/*
	cGameData * p_game_data = NULL;

	switch (game_type) {
		//case GAME_TYPE_DEATHMATCH:			p_game_data = new cGameDataDeathMatch;			break;
		//case GAME_TYPE_TEAM_DEATHMATCH:	p_game_data = new cGameDataTeamDeathMatch;	break;
		case GAME_TYPE_CNC:					p_game_data = new cGameDataCnc;					break;
		default:									DIE;														break;
	}

	return p_game_data;
	*/

	WWASSERT(game_type == GAME_TYPE_CNC);

	return new cGameDataCnc;
}

/*
//-----------------------------------------------------------------------------
const char* cGameData::Get_Game_Type_Name(GameTypeEnum type)
{
	static const char* _modeNames[] = {"SP", "SK", "CNC"};
	return _modeNames[type];
}
*/

//-----------------------------------------------------------------------------
const char * cGameData::Get_Game_Type_Name(void) const
{
	WideStringClass wide_name;
	wide_name.Format(U_CHAR("%s"), Get_Game_Name());
	StringClass name;
	wide_name.Convert_To(name);

	return name.Peek_Buffer();
}

//-----------------------------------------------------------------------------
void cGameData::Add_Bottom_Text(WideStringClass & text)
{
	//WWASSERT(text != NULL);

/* 	WWASSERT(PFont != NULL);
	float x = Render2DClass::Get_Screen_Resolution().Center().X -
		PFont->String_Width(text) / 2.0f;

	float y = MultiHUDClass::Get_Bottom_Text_Y_Pos();
	y -= 1.2 * PFont->Char_Height();
	MultiHUDClass::Set_Bottom_Text_Y_Pos(y);

   WWASSERT(PTextRenderer != NULL);
	PTextRenderer->Set_Location(Vector2(cMathUtil::Round(x), cMathUtil::Round(y)));
	PTextRenderer->Draw_Text(text);
*/
	BottomText.Add(text);
}

//-----------------------------------------------------------------------------
void cGameData::Show_Game_Settings_Limits(void)
{
	if (PTextRenderer == NULL) {
		return;
	}

	bool changed=false;

	WideStringClass text(0,true);
	unsigned color;

	Get_Time_Limit_Text(text);
	if (text!=renderer_time_text) {
		renderer_time_text=text;
		changed=true;
	}

	// Dedicated server label
	if (cNetwork::I_Am_Only_Server()) {
		Get_Dedicated_Server_Label(text,color);
	}
	else {
		text="";
		color=0;
	}
	if (text!=renderer_dedicated_server_label) {
		renderer_dedicated_server_label=text;
		changed=true;
	}
	if (color!=renderer_dedicated_server_color) {
		renderer_dedicated_server_color=color;
		changed=true;
	}

	// Gameplay pending label -
	bool show_gameplay_label = false;
	if (!Is_Gameplay_Permitted() || (COMBAT_STAR == NULL))	{
		show_gameplay_label = true;
	}
	//if (IsIntermission.Is_False() && (!Is_Gameplay_Permitted() || (COMBAT_STAR == NULL))) {
	if (show_gameplay_label) {
		Get_Gameplay_Not_Permitted_Label(text,color);
	} else {
		text="";
		color=0;
	}
	if (text!=renderer_gameplay_label) {
		renderer_gameplay_label=text;
		changed=true;
	}
	if (color!=renderer_gameplay_color) {
		renderer_gameplay_color=color;
		changed=true;
	}

	if (!changed) {
		if (BottomText.Count()!=OldBottomText.Count()) {
			changed=true;
		}
		else {
			for (int j=0;j<BottomText.Count();++j) {
				if (BottomText[j]!=OldBottomText[j]) {
					changed=true;
					break;
				}
			}
		}
	}


	// Render if needed  -------------------------------------------------------
	float charHeight = PTextRenderer->Peek_Font()->Get_Char_Height();

	if (!changed) {
		// Proceed the multihud by one line (as we are caching the time render which would usually do this)
		float y = MultiHUDClass::Get_Bottom_Text_Y_Pos();
		y -= 1.2 * charHeight;
		MultiHUDClass::Set_Bottom_Text_Y_Pos(y);
		BottomText.Reset_Active();
		return;
	}

	// Re-compose the OldBottomText vector

	OldBottomText.Reset_Active();
	int j;
	for (j=0;j<BottomText.Count();++j) {
		OldBottomText.Add(BottomText[j]);
	}
	BottomText.Reset_Active();

	PTextRenderer->Reset();

	float x,y;

	// Render bottom text (start with time text, which is to be first)
	Vector2 textExtent = PTextRenderer->Get_Text_Extents(text);
	x = Render2DClass::Get_Screen_Resolution().Center().X -  textExtent.X / 2.0f;

	//Always draw on the bottom
	MultiHUDClass::Set_Bottom_Text_Y_Pos( Render2DClass::Get_Screen_Resolution().Bottom - 15 );

	y = MultiHUDClass::Get_Bottom_Text_Y_Pos();
	y -= 1.2 * charHeight;
	Vector2 loc(cMathUtil::Round(x), cMathUtil::Round(y));
	PTextRenderer->Set_Location(loc);
	PTextRenderer->Build_Sentence(renderer_time_text);
	PTextRenderer->Draw_Sentence();

	for (j=0;j<OldBottomText.Count();++j) {
		y -= 1.2 * charHeight;
		loc[1]=cMathUtil::Round(y);
		PTextRenderer->Set_Location(loc);
		PTextRenderer->Build_Sentence(OldBottomText[j]);
		PTextRenderer->Draw_Sentence();
	}
	MultiHUDClass::Set_Bottom_Text_Y_Pos(y);



	// Dedicated server label
	if (cNetwork::I_Am_Only_Server()) {
		Vector2 extent = PTextRenderer->Get_Text_Extents(renderer_dedicated_server_label);
		x = Render2DClass::Get_Screen_Resolution().Center().X - extent.X / 2.0f;

		y = Render2DClass::Get_Screen_Resolution().Center().Y;
		PTextRenderer->Set_Location(Vector2(cMathUtil::Round(x), cMathUtil::Round(y)));
		PTextRenderer->Build_Sentence(renderer_dedicated_server_label);
		PTextRenderer->Draw_Sentence(renderer_dedicated_server_color);
	}

	// Gameplay pending label
	if (show_gameplay_label) {
		Vector2 extent = PTextRenderer->Get_Text_Extents(renderer_gameplay_label);
		x = Render2DClass::Get_Screen_Resolution().Center().X -  extent.X / 2.0f;
		y = Render2DClass::Get_Screen_Resolution().Center().Y + 1.2 * charHeight;
		PTextRenderer->Set_Location(Vector2(cMathUtil::Round(x), cMathUtil::Round(y)));
		PTextRenderer->Build_Sentence(renderer_gameplay_label);
		PTextRenderer->Draw_Sentence(color);
	}
}

//-----------------------------------------------------------------------------
void cGameData::Get_Dedicated_Server_Label(WideStringClass& text, unsigned& color)
{
	color = 0xFFFFFFFF; // white
	if (cMathUtil::Round(TimeManager::Get_Seconds()) % 2 != 0) {
		color = 0xFF00FF00;	// green
	}

	text = TRANSLATION(IDS_MP_DEDICATED_SERVER);
}

//-----------------------------------------------------------------------------
void cGameData::Get_Gameplay_Not_Permitted_Label(WideStringClass& text, unsigned& color)
{
	color = 0xFFFFFFFF; // white
	if (cMathUtil::Round(TimeManager::Get_Seconds()) % 2 != 0) {
		color = 0xFF00FF00;	// green
	}

	text = TRANSLATION(IDS_MP_GAMEPLAY_PENDING);
}


//-----------------------------------------------------------------------------
void cGameData::Get_Time_Limit_Text(WideStringClass& text)
{
   if (IsIntermission.Is_True()) {

      text.Format(
			U_CHAR("%s: %d"),
			TRANSLATION(IDS_MP_NEXTGAME_COUNTDOWN),
			cMathUtil::Round(Get_Intermission_Time_Remaining()));

	} else if (Is_Time_Limit()) {

      int hours	= 0;
      int mins		= 0;
      int seconds	= 0;

		cMiscUtil::Seconds_To_Hms(TimeRemainingSeconds, hours, mins, seconds);

      WideStringClass time_string(0, true);
      time_string.Format(U_CHAR("%02d:%02d:%02d"), hours, mins, seconds);

		text.Format(U_CHAR("%s: %s"), TRANSLATION(IDS_MP_TIME_REMAINING), time_string);
   }

}

//-----------------------------------------------------------------------------
void cGameData::Think(void)
{
	if (IsIntermission.Is_True()) {
		IntermissionTimeRemaining -= TimeManager::Get_Frame_Real_Seconds();
		if (IntermissionTimeRemaining < 0) {

			IntermissionTimeRemaining = 0;
		}
	} else if (Is_Gameplay_Permitted()) {
		TimeRemainingSeconds -= TimeManager::Get_Frame_Real_Seconds();
		if (TimeRemainingSeconds < 0) {
			TimeRemainingSeconds = 0;
		}

		unsigned int duration_s = (int)((TIMEGETTIME() - GameStartTimeMs) / 1000.0f);
		Set_Game_Duration_S(duration_s);
	}

	if (cNetwork::I_Am_Server()) {
		//
		// Update CurrentPlayers
		//
		int max = (int) WWMath::Max(
			cPlayerManager::Count(),
			cNetwork::PServerConnection->Get_Num_RHosts());
		Set_Current_Players(max);
	}

	DEMO_SECURITY_CHECK;

	FrameCount++;
}

//-----------------------------------------------------------------------------
void cGameData::Render(void)
{
	Show_Game_Settings_Limits();

	if (PTextRenderer != NULL) {
		WWPROFILE("cGameData::Render");
		PTextRenderer->Render();
	}
}

//-----------------------------------------------------------------------------
void cGameData::On_Game_Begin(void)
{
	//
	// Note, On_Game_Begin happens after load.
	//

   WWDEBUG_SAY(("cGameData::On_Game_Begin\n"));

	GetSystemTime(&GameStartTime);
	FrameCount = 0;

	GameStartTimeMs = TIMEGETTIME();

	//
	// Clear the MVP name
	// Let MVP carry over into next game.
	//
	//MvpName.Format(U_CHAR(""));
	//MvpCount = 0;

	//
	// Reset the game duration
	//
	GameDurationS = 0;

	//
	// Notify the combat layer about various server options
	//
	cCsDamageEvent::Set_Are_Clients_Trusted(IsClientTrusted.Get());
	BuildingGameObj::Set_Can_Repair_Buildings(CanRepairBuildings.Get());
	VehicleGameObj::Set_Default_Driver_Is_Gunner(DriverIsAlwaysGunner.Get());
	CombatManager::Set_Friendly_Fire_Permitted(IsFriendlyFirePermitted.Get());
	CombatManager::Set_Beacon_Placement_Ends_Game(false);

	//
	// Filter out unwanted spawners
	//
	if (cNetwork::I_Am_Server()) {
		Filter_Spawners();
	}

	//
	// Adjust teaming for auto-teamed games
	//
	//if (!IS_MISSION && cNetwork::I_Am_Server() && Is_Team_Game() && IsTeamChangingAllowed.Is_False()) {
	if (!IS_MISSION && cNetwork::I_Am_Server()) {

		if (IsClanGame.Is_False() && IsTeamChangingAllowed.Is_False() && RemixTeams.Is_True()) {
			//
			// Remix Teams if the server option is set.
			//
			Remix_Team_Sides();
		} else {
			//
			// In non-remixed C&C mode, we have a 50-50 chance of the teams being swapped. I prefer this
			// to simply alternating, because if you alternate and the number of maps in the
			// cycle is even, you will always play GDI on the same 50% of the maps.
			//
			if (Is_Skirmish() || (Is_Cnc() && (rand() % 2 == 0))) {
				Swap_Team_Sides();
			}
		}

		if (IsClanGame.Is_False() && IsTeamChangingAllowed.Is_False())
		{
			//
			// We always rebalance the teams numerically
			//
			Rebalance_Team_Sides();
		}
	}

	GameModeClass* gameMode = GameModeManager::Find("WOL");

	if (gameMode && gameMode->Is_Active()) {
		WolGameModeClass* wolGame = reinterpret_cast<WolGameModeClass*>(gameMode);
		WWASSERT(wolGame != NULL);
		wolGame->Start_Game(this);
	}
}

//-----------------------------------------------------------------------------
void cGameData::On_Game_End(void)
{
   WWDEBUG_SAY(("cGameData::On_Game_End\n"));

	GameModeClass* gameMode = GameModeManager::Find("WOL");

	if (gameMode && gameMode->Is_Active()) {
		WolGameModeClass* wolGame = reinterpret_cast<WolGameModeClass*>(gameMode);
		wolGame->End_Game();
	}
}

//-----------------------------------------------------------------------------
int	cGameData::Get_Mission_Number_From_Map_Name( const char * map_name )
{
	// If tutorial, return tutorial number
	if ( ::strnicmp( map_name, "M00_T", 5 ) == 0 ) {
		#define	TUTORIAL_LOAD_MENU_NUMBER		90
		return TUTORIAL_LOAD_MENU_NUMBER;
	}


	// Assume the map name is in the form "mXX.mix" where xx is the level number
	if ( ::strlen( map_name ) > 0 ) {
		return ::atoi( ((const char *)map_name)+1 );
	}
	return 0;
}

//-----------------------------------------------------------------------------
const StringClass& cGameData::Get_Map_Cycle(int map)
{
	WWASSERT(map >= 0 && map < MAX_MAPS);

#ifdef MULTIPLAYERDEMO
	MapCycle[map].Format("C&C_Under.mix");
#endif // MULTIPLAYERDEMO

	return MapCycle[map];
}

//-----------------------------------------------------------------------------
void
cGameData::Set_Map_Cycle(int map, const StringClass & map_name)
{
	WWASSERT(map >= 0 && map < MAX_MAPS);

	MapCycle[map] = map_name;
}

//-----------------------------------------------------------------------------
void
cGameData::Clear_Map_Cycle(void)
{
	for (int i = 0; i < MAX_MAPS; i++)
	{
		MapCycle[i].Format("");
	}
}

//-----------------------------------------------------------------------------
void
cGameData::Rotate_Map(void)
{
	IsMapCycleOver = false;

	//
	// Determine the current map
	//
	int current_map = MapCycleIndex;
#if (0)
	int current_map = 0;
	for (int i = 0; i < MAX_MAPS; i++)
	{
		if (MapName == MapCycle[i]) {
			current_map = i;
			break;
		}
	}
#endif //(0)
	
	//
	// Increment it
	//
	current_map++;

	//
	// Go back to the first map if necessary
	//
	if (current_map >= MAX_MAPS || MapCycle[current_map].Is_Empty()) {
		current_map = 0;

		if (DoMapsLoop == false) {
			IsMapCycleOver = true;
		}
	}

	//
	// Set the map name
	//
#ifdef MULTIPLAYERDEMO
	MapName.Format("C&C_Under.mix");
#else
	MapName = MapCycle[current_map];
#endif

	MapCycleIndex = current_map;

	return ;
}

//-----------------------------------------------------------------------------
void cGameData::Set_Win_Text(WideStringClass & text)
{
	WinText = text;
}

//-----------------------------------------------------------------------------
void cGameData::ReceiveSignal(MPChooseTeamSignal& choice)
{
	if (choice.GetItemA() == true) {
		//
		// Do nothing if the player selected 'auto' rather than GDI or NOD.
		//
		if (choice.GetItemB() != PLAYERTYPE_RENEGADE) {
			cNetwork::SwitchTeam(choice.GetItemB());
		}
	}
}

//-----------------------------------------------------------------------------
void cGameData::Filter_Spawners(void)
{
	WWASSERT(cNetwork::I_Am_Server());

	if (SpawnWeapons.Is_False()) {
		//
		// Disable multiplay weapons spawners
		//
		DynamicVectorClass<SpawnerClass*> spawner_list = SpawnManager::Get_Spawner_List();

		for (int i = 0; i < spawner_list.Count(); i++) {

			WWASSERT(spawner_list[i] != NULL);

			if (spawner_list[i]->Get_Definition().Is_Multiplay_Weapon_Spawner()) {
				spawner_list[i]->Enable(false);
			}
		}
	}
}

//------------------------------------------------------------------------------------
void cGameData::Set_Mvp_Name(const WideStringClass name)
{
	MvpName = name;
}

//------------------------------------------------------------------------------------
void cGameData::Set_Mvp_Count(int count)
{
	WWASSERT(count >= 0);

	MvpCount = count;
}

//------------------------------------------------------------------------------------
void cGameData::Set_Win_Type(WinTypeEnum type)
{
	WinType = type;
}

//------------------------------------------------------------------------------------
void cGameData::Set_Game_Duration_S(unsigned int seconds)
{
	GameDurationS = seconds;
}

//-----------------------------------------------------------------------------
void cGameData::Get_Description(WideStringClass & description)
{
	const WideStringClass delimiter	= U_CHAR("\t");
	const WideStringClass newline		= U_CHAR("\n");
	const WideStringClass yes			= TRANSLATE(IDS_YES);
	const WideStringClass no			= TRANSLATE(IDS_NO);

	WideStringClass attribute(0, true);
	WideStringClass value(0, true);

	//
	// Host
	//
	attribute = TRANSLATE(IDS_MP_HOST);
	value = Owner;
	description += (attribute + delimiter + value + newline);

	//
	// Game Name
	//
	attribute = TRANSLATE(IDS_MENU_TEXT292);
	value = GameTitle;
	description += (attribute + delimiter + value + newline);

	//
	// Map Name
	//
	if (ModName.Is_Empty () == false) {
		attribute = U_CHAR("Mod Name:");
		value = ModName;
		description += (attribute + delimiter + value + newline);
	}

	//
	// Map Name
	//
	attribute = TRANSLATE(IDS_MP_GAME_LIST_HEADER_GAME_MAP);
	value = MapName;
	description += (attribute + delimiter + value + newline);

	//
	// Time Limit
	//
	attribute = TRANSLATE(IDS_MENU_TEXT298);
	if (Is_Time_Limit()) {
		value.Format(U_CHAR("%d %s"), TimeLimitMinutes, TRANSLATE(IDS_MP_MINUTES));
	} else {
		value = TRANSLATE(IDS_MP_NONE);
	}
	description += (attribute + delimiter + value + newline);

	//
	// Max. Players
	//
	attribute = TRANSLATE(IDS_MENU_TEXT294);
	value.Format(U_CHAR("%d"), MaxPlayers);
	description += (attribute + delimiter + value + newline);

	//
	// Radar
	//
	attribute = TRANSLATE(IDS_MENU_TEXT488);
	switch (RadarMode) {
		case RADAR_NOBODY:
		   value = TRANSLATION(IDS_MP_RADAR_MODE_NOBODY);
			break;

		case RADAR_TEAMMATES:
		   value = TRANSLATION(IDS_MP_RADAR_MODE_TEAMMATES);
			break;

		case RADAR_ALL:
		   value = TRANSLATION(IDS_MP_RADAR_MODE_ALL);
			break;

		default:
			DIE;
	}
	description += (attribute + delimiter + value + newline);

	//
	// Allow Quickmatching
	//
	attribute = TRANSLATE(IDS_MENU_TEXT584);
	value = (IsQuickMatchServer.Get() ? yes : no);
	description += (attribute + delimiter + value + newline);

	//
	// Dedicated
	//
	attribute = TRANSLATE(IDS_MENU_TEXT309);
	value = (IsDedicated.Get() ? yes : no);
	description += (attribute + delimiter + value + newline);

	//
	// Friendly Fire
	//
	attribute = TRANSLATE(IDS_MENU_TEXT301);
	value = (IsFriendlyFirePermitted.Get() ? yes : no);
	description += (attribute + delimiter + value + newline);

	//
	// Manual Teaming
	//
	attribute = TRANSLATE(IDS_MENU_TEXT490);
	value = (IsTeamChangingAllowed.Get() ? yes : no);
	description += (attribute + delimiter + value + newline);

	//
	// Passworded
	//
	attribute = TRANSLATE(IDS_MP_PASSWORDED);
	value = (IsPassworded.Get() ? yes : no);
	description += (attribute + delimiter + value + newline);

	//
	// Laddered
	//
	attribute = TRANSLATE(IDS_MENU_TEXT496);
	value = (IsLaddered.Get() ? yes : no);
	description += (attribute + delimiter + value + newline);

	//
	// Clan Game
	//
	attribute = TRANSLATE(IDS_MENU_TEXT497);
	value = (IsClanGame.Get() ? yes : no);
	description += (attribute + delimiter + value + newline);

	/*
	//
	// Trust Clients
	//
	attribute = TRANSLATE(IDS_MENU_TEXT585);
	value = (IsClientTrusted.Get() ? yes : no);
	description += (attribute + delimiter + value + newline);
	*/

	//
	// Remix Teams
	//
	attribute = TRANSLATE(IDS_MENU_TEXT587);
	value = (RemixTeams.Get() ? yes : no);
	description += (attribute + delimiter + value + newline);

	//
	// Can Repair Buildings
	//
	attribute = TRANSLATE(IDS_MENU_TEXT590);
	value = (CanRepairBuildings.Get() ? yes : no);
	description += (attribute + delimiter + value + newline);

	//
	// Driver Is Always Gunner
	//
	attribute = TRANSLATE(IDS_MENU_TEXT591);
	value = (DriverIsAlwaysGunner.Get() ? yes : no);
	description += (attribute + delimiter + value + newline);

	//
	// Weapon spawning
	//
	attribute = TRANSLATE(IDS_MENU_TEXT592);
	value = (SpawnWeapons.Get() ? yes : no);
	description += (attribute + delimiter + value + newline);
}

//-----------------------------------------------------------------------------

//cGameData *						The_Game(void)							{WWASSERT(PTheGameData != NULL); return PTheGameData;}
cGameData *						The_Game(void)							{return PTheGameData;}

cGameDataSinglePlayer *		The_Single_Player_Game(void)		{WWASSERT(The_Game()->As_Single_Player() != NULL);		return The_Game()->As_Single_Player();}
cGameDataSkirmish *			The_Skirmish_Game(void)				{WWASSERT(The_Game()->As_Skirmish() != NULL);			return The_Game()->As_Skirmish();}
cGameDataCnc *					The_Cnc_Game(void)					{WWASSERT(The_Game()->As_Cnc() != NULL);					return The_Game()->As_Cnc();}







	/*
	if (Is_Team_Game()) {
		has_opponents =
			cPlayerManager::Get_Count(PLAYERTYPE_NOD) > 0 &&
			cPlayerManager::Get_Count(PLAYERTYPE_GDI) > 0;
	} else {
		has_opponents = (cPlayerManager::Get_Count(PLAYERTYPE_RENEGADE) > 1);
	}
	*/

/*
//-----------------------------------------------------------------------------
void cGameData::Set_Min_Game_Time_Required_Mins(int mins)
{
	WWASSERT(mins >= 0 );
	MinGameTimeRequiredMins = mins;
}
*/

/*
//-----------------------------------------------------------------------------
void cGameData::Set_Full_Score_Time_Threshold_Mins(int mins)
{
	WWASSERT(mins >= 0);
	FullScoreTimeThresholdMins = mins;
}
*/

//#include "helptext.h"
//#include "gddeathmatch.h"
//#include "gdteamdeathmatch.h"
	//MinGameTimeRequiredMins			= 10;
	//FullScoreTimeThresholdMins		= 30;
	//MinGameTimeRequiredMins			= rhs.MinGameTimeRequiredMins;
	//FullScoreTimeThresholdMins		= rhs.FullScoreTimeThresholdMins;
   //packet.Add(MinGameTimeRequiredMins);
   //packet.Add(FullScoreTimeThresholdMins);
   //Set_Min_Game_Time_Required_Mins(			packet.Get(i_placeholder));
   //Set_Full_Score_Time_Threshold_Mins(		packet.Get(i_placeholder));
//cGameDataDeathMatch *		The_Deathmatch_Game(void)			{WWASSERT(The_Game()->As_Deathmatch() != NULL);			return The_Game()->As_Deathmatch();}
//cGameDataTeamDeathMatch *	The_Team_Deathmatch_Game(void)	{WWASSERT(The_Game()->As_Team_Deathmatch() != NULL);	return The_Game()->As_Team_Deathmatch();}

	//if (Is_Team_Game()) {
	/*
	} else {
		has_opponents = (cPlayerManager::Tally_Team_Size(PLAYERTYPE_RENEGADE) > 1);
	}
	*/

	//IsBloodShed.Set(					false);
	//IsBloodShed							= rhs.IsBloodShed;

   /*
	if (cNetwork::I_Am_Server()) {
		IsBloodShed.Set(false);
   }
	*/