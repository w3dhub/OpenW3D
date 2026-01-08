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
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/gameinitmgr.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 6/21/02 2:24p                                               $*
 *                                                                                             *
 *                    $Revision:: 75                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "gameinitmgr.h"
#include "gamedata.h"
#include "gamemode.h"
#include "cnetwork.h"
#include "ww3d.h"
#include "singlepl.h"
#include "player.h"
#include "renegadedialogmgr.h"
#include "netinterface.h"
#include "langmode.h"
#include "wolgmode.h"
#include "playermanager.h"
#include "WWAudio.h"
#include "saveloadstatus.h"
#include "combatgmode.h"
#include "useroptions.h"
#include "lanchat.h"
#include "netutil.h"
#include "rendobj.h"
#include "phys.h"
#include "pscene.h"
#include "dx8renderer.h"
#include "gdsingleplayer.h"
#include "gdskirmish.h"
#include "playertype.h"
#include "gameobjmanager.h"
#include "gametype.h"
#include "god.h"
#include "bioevent.h"
#include "devoptions.h"
#include "svrgoodbyeevent.h"
#include	"natter.h"
#include "apppacketstats.h"
#include "packetmgr.h"
#include "AutoStart.h"
#include "wwmemlog.h"
#include "gamesideservercontrol.h"
#include "slavemaster.h"
#include "hud.h"
#include "gamespyadmin.h"
#include "ServerSettings.h"
#include "GameSpy_QnR.h"
#include "ConsoleMode.h"
#include "specialbuilds.h"
#include "modpackagemgr.h"

#include "translatedb.h"
#include "damage.h"
#include "ccamera.h"
#include "bones.h"
#include "surfaceeffects.h"
#include "ffactory.h"
#include "ini.h"
#include "dazzle.h"
#include "scriptman.h"



static void _reload_game_configuration_files(void);

// Defines.
#define PRE_SERVICE_TIME	1500 // Time in milliseconds.
#define POST_SERVICE_TIME	 250 // Time in milliseconds.


////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
bool		GameInitMgrClass::IsClientRequired	= false;
bool		GameInitMgrClass::IsServerRequired	= false;
bool		GameInitMgrClass::RestoreSFX			= false;
bool		GameInitMgrClass::RestoreMusic		= false;
bool		GameInitMgrClass::NeedsGameExit		= false;
bool		GameInitMgrClass::NeedsGameExitAll	= false;
int		GameInitMgrClass::Mode					= MODE_UNKNOWN;
int		GameInitMgrClass::WOLReturnDialog	= RenegadeDialogMgrClass::LOC_INTERNET_MAIN;


bool GameInitMgrClass::Is_Game_In_Progress(void)
{
	GameModeClass* mode = GameModeManager::Find("Combat");
	//return (mode && mode->Is_Active());
	return (mode != NULL && !mode->Is_Inactive());
}


////////////////////////////////////////////////////////////////
//
//	Start_Game
//
////////////////////////////////////////////////////////////////
void
GameInitMgrClass::Start_Game (const char *map_name, int teamChoice, unsigned int clanID)
{
	unsigned int time;

	WWASSERT(map_name != NULL);
   WWDEBUG_SAY (("GameInitMgrClass::Start_Game(%s)\n", map_name));

	// NOTE: Multi-play does not need this fix because it does not sound page swap.
	if (IS_SOLOPLAY) {
	
		// IML: First, allow a short period to process any outstanding sound effects that may have
		// been started by the caller.
		time = TIMEGETTIME();
		while (TIMEGETTIME() - time < PRE_SERVICE_TIME) {
			WWAudioClass::Get_Instance ()->On_Frame_Update (0);
		}
		
 		// IML: Ensure that there are no sound effects lingering on any playlist. 
		WWAudioClass::Get_Instance ()->Flush_Playlist();

		// IML: Allow audio system to clean-up after flush.
		time = TIMEGETTIME();
		while (TIMEGETTIME() - time < POST_SERVICE_TIME) {
			WWAudioClass::Get_Instance ()->On_Frame_Update (0);
		}
	}

	//
	// Kill off any old suspended game
	//
	if (GameModeManager::Find ("Combat")->Is_Suspended ()) {
		End_Game ();
		GameModeManager::Safely_Deactivate ();
	}

	//
	//	Set the map name
	//
	StringClass map(map_name,true);
	WWASSERT(PTheGameData != NULL);
	The_Game ()->Set_Map_Name (map);

	//
	//	Determine if there is a mod specified... if so, load the mod package
	//
	ModPackageMgrClass::Load_Current_Mod ();

	//
	// Reload the sub-systems that may be affected by a mod
	//
	_reload_game_configuration_files();

	//
	//	Check to ensure the game is configured correctly
	//
	#ifdef WWDEBUG
	WideStringClass outMsg;
	
	if (!The_Game()->Is_Valid_Settings(outMsg)) {
		WWDEBUG_SAY(("ERROR: %S\n", (const unichar_t*)outMsg));
		WWASSERT("The_Game()->Is_Valid_Settings()");
	}
	#endif

	//
	// Reset Data Safe state.
	//
	GenericDataSafeClass::Reset();

	//
	// Reset packet optimizer bandwidth stats.
	//
	PacketManager.Reset_Stats();

	//
	//	Start either the client or server (or both) depending
	// on which mode we are in.
	//
	Start_Client_Server ();

	//
	//	Deactivate the menu system
	//
	INIT_STATUS ("Deactivate menu");
	GameModeManager::Find ("Menu")->Deactivate ();

	//
	//	Active the combat system
	//
	INIT_STATUS ("Activate combat");
   GameModeManager::Find ("Combat")->Activate ();
	INIT_STATUS ("");

	//
	//	Load the level
	//
	CombatGameModeClass *game_mode = static_cast<CombatGameModeClass*>(GameModeManager::Find ("Combat"));
	game_mode->Load_Level ();

   //
	//	Let the LAN or WOL interface know we are starting a game
	//
	if (Mode == MODE_LAN) {
		INIT_STATUS ("Go to location");
		PLC->Go_To_Location (LANLOC_INGAME);
	} else if (Mode == MODE_WOL) {
		INIT_STATUS ("Go to game channel");
	}

	//
	//	Reset some rendering data
	//
	PhysicsSceneClass::Get_Instance()->Release_Projector_Resources ();
	TheDX8MeshRenderer.Invalidate ();

	//
	// Prevent the first couple frames from rendering, so that all textures get cached.
	//
	GameModeManager::Hide_Render_Frames (2);

	//
	//	Send team/player information to the server (if necessary)
	//
	Transmit_Player_Data (teamChoice, clanID);

	//
	// Set the auto restart flag if required.
	//
	AutoRestart.Set_Restart_Flag((The_Game()->IsAutoRestart.Is_True()) ? true : false);

	//
	// Listen for server control messages.
	//
	GameSideServerControlClass::Init();

	return ;
}


////////////////////////////////////////////////////////////////
//
//	End_Game
//
////////////////////////////////////////////////////////////////
void
GameInitMgrClass::End_Game (void)
{
	unsigned int time;

	WWDEBUG_SAY (("GameInitMgrClass::End_Game\n"));

	// Do nothing if the game is not in progress.
	if ( !IS_MISSION && (!Is_Game_In_Progress())) {
		return;
	}

	// NOTE: Multi-play does not need this fix because it does not sound page swap.
	if (IS_SOLOPLAY) {

		// IML: Allow a short period to process any outstanding sound effects that may have
		// been started by the caller.
		time = TIMEGETTIME();
		while (TIMEGETTIME() - time < PRE_SERVICE_TIME) {
			WWAudioClass::Get_Instance ()->On_Frame_Update (0);
		}

		// IML: Ensure that there are no sound effects lingering on any playlist. 
		WWAudioClass::Get_Instance ()->Flush_Playlist();

		// IML: Allow audio system to clean-up after flush.
		time = TIMEGETTIME();
		while (TIMEGETTIME() - time < POST_SERVICE_TIME) {
			WWAudioClass::Get_Instance ()->On_Frame_Update (0);
		}
	}

#ifndef MULTIPLAYERDEMO
	if ( IS_MISSION && COMBAT_STAR ) {
		cGod::Store_Inventory( COMBAT_STAR );
	}
#endif // !MULTIPLAYERDEMO

	// Stop reporting to Gamespy.
	if (Mode == MODE_LAN && GameSpyQnR.IsEnabled()) {
		GameSpyQnR.Shutdown();
	}

	//
	// A dedicated server will disable sfx & music. Restore them here.
	//
	if (WWAudioClass::Get_Instance() != NULL) {
		if (RestoreSFX) {
			WWAudioClass::Get_Instance()->Allow_Sound_Effects(true);
		}
		if (RestoreMusic) {
			WWAudioClass::Get_Instance()->Allow_Music(true);
		}
	}

   if (GameModeManager::Find( "Combat" )->Is_Active()) {
      //
      // Combat is still active during the ingame menu and multiplayer gameplay.
      // Suspend it first because that is the state that it is in for
      // single-player.
      //
      GameModeManager::Find( "Combat" )->Suspend();
   }

	//
	//	Notify the game data object that the game is over
	//
	cGameData* theGame = PTheGameData;

	//
	// Shut down slave game servers.
	//
	SlaveMaster.Shutdown_Slaves();

	//
	// Stop listening for server control messages.
	//
	GameSideServerControlClass::Shutdown();

	if (theGame) {
		theGame->On_Game_End();
	}

	//
	// Disable auto restart mode.
	//
	// For forced exits the mode will already be correct.
	//
	if (!cGameData::Is_Manual_Exit()) {
		AutoRestart.Set_Restart_Flag(false);
	}

	//
	//	Shutdown the combat system
	//
	GameModeManager::Find ("Combat")->Deactivate ();

	//
	//	Let the game mode manager think to cleanup all pending states
	//
	GameModeManager::Think ();

 	//
	//	Shutdown the menu system as necessary
	//
	if (GameModeManager::Find ("Menu")->Is_Active ()) {
		GameModeManager::Find ("Menu")->Deactivate ();
	}

	// Leave the WWOnline game.
	GameModeClass* gameMode = GameModeManager::Find("WOL");

	if (gameMode && gameMode->Is_Active()) {
		WolGameModeClass* wolGame = static_cast<WolGameModeClass*>(gameMode);
		WWASSERT(wolGame != NULL);
		wolGame->Leave_Game();
	}

	if (cNetwork::I_Am_Server()) {

		bool is_quick_full_exit_requested = false;
#ifdef WWDEBUG
		is_quick_full_exit_requested = cDevOptions::QuickFullExit.Get();
#endif // WWDEBUG

// FIXME (TSS) ***** Memory leak here - please fix (ST - 6/14/2001 2:06PM) *****
		cSvrGoodbyeEvent * p_event = new cSvrGoodbyeEvent;
		p_event->Init(is_quick_full_exit_requested);
	}

	cNetwork::Flush();

	End_Client_Server();

	//
	//	Remove all players
	//
	cPlayerManager::Remove_All();
	cTeamManager::Remove_All();

	//
	// TSS092301
	// Destroy all netobjects !
	//
	NetworkObjectMgrClass::Set_All_Delete_Pending();
	NetworkObjectMgrClass::Delete_Pending();

	cGod::Reset();

	//
	//	Unload whatever mod is currently loaded (if necessary)...
	//
	ModPackageMgrClass::Unload_Current_Mod ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Continue_Game
//
////////////////////////////////////////////////////////////////
void
GameInitMgrClass::Continue_Game(void)
{
	unsigned int time;

	// IML : First, allow a short period to process any outstanding sound effects that may have been started by the caller.
	// NOTE: Multi-play does not need this fix because it does not sound page swap.
	if (IS_SOLOPLAY) {
		time = TIMEGETTIME();
		while (TIMEGETTIME() - time < PRE_SERVICE_TIME) {
			WWAudioClass::Get_Instance ()->On_Frame_Update (0);
		}
	}

	GameModeManager::Find ("Menu")->Deactivate ();
	GameModeManager::Find ("Combat" )->Resume ();

	//Force the hud to rebuild, in case weapn chart text changed
	HUDClass::Force_Weapon_Chart_Update();

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Display_End_Game_Menu
//
////////////////////////////////////////////////////////////////
void
GameInitMgrClass::Display_End_Game_Menu (void)
{
	switch (Mode)
	{
		//
		//	Display the main menu
		//
		default:
		case MODE_SP:
		case MODE_SKIRMISH:
			RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_MAIN_MENU);
			break;

		//
		//	Display the LAN main menu
		//
		case MODE_LAN:
			//GAMESPY
			if (cGameSpyAdmin::Is_Gamespy_Game()) {
				RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_GAMESPY_MAIN);
			} else {
				RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_LAN_MAIN);
			}
			break;

		//
		//	Display the WOL main menu
		//
		case MODE_WOL:
			RenegadeDialogMgrClass::Goto_Location ((RenegadeDialogMgrClass::LOCATION)WOLReturnDialog);
			break;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Transmit_Player_Data
//
////////////////////////////////////////////////////////////////
void
GameInitMgrClass::Transmit_Player_Data (int teamChoice, unsigned int clanID)
{
	WWMEMLOG(MEM_NETWORK);
   WWDEBUG_SAY (("GameInitMgrClass::Transmit_Player_Data\n"));

	if (Mode == MODE_SP || Mode == MODE_SKIRMISH) {

		//
		//	Send generic team information for the client in a single player game
		//
		cBioEvent * p_event = new cBioEvent;
		p_event->Init(teamChoice, clanID);

	} else if (IsClientRequired) {

		//
		//	Send the player's team choice to the server
		//
		cBioEvent * p_event = new cBioEvent;
		p_event->Init(teamChoice, clanID);
	}

   WWDEBUG_SAY (("GameInitMgrClass::Transmit_Player_Data Done\n"));
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Start_Client_Server
//
////////////////////////////////////////////////////////////////
void
GameInitMgrClass::Start_Client_Server (void)
{
   WWDEBUG_SAY (("GameInitMgrClass::Start_Client_Server\n"));

	assert(GameModeManager::Find("WOL"));
		if (GameModeManager::Find("WOL")->Is_Active()) {
			if (PTheGameData != NULL) {
				const unsigned short wol_port = WOLNATInterface.Get_Port_As_Server();
				if (wol_port >= MIN_SERVER_PORT && wol_port <= MAX_SERVER_PORT) {
					The_Game()->Set_Port(wol_port);
			} else {
				WWDEBUG_SAY(("WOL port %hu outside valid range, keeping existing game port\n", wol_port));
			}
		}
		} else if (GameModeManager::Find("LAN")->Is_Active() && cGameSpyAdmin::Is_Gamespy_Game()) {
			if (PTheGameData != NULL) {
				const int gamespy_port = cUserOptions::GameSpyGamePort.Get();
				if (gamespy_port >= MIN_SERVER_PORT && gamespy_port <= MAX_SERVER_PORT) {
					The_Game()->Set_Port(gamespy_port);
					ConsoleBox.Print("GameSpy server port set to %d\n", gamespy_port);
				} else {
					WWDEBUG_SAY(("GameSpy port %d outside valid range, keeping existing game port\n", gamespy_port));
				}
			}
		}

#ifdef WWDEBUG
	cRemoteHost::Set_Allow_Extra_Modem_Bandwidth_Throttling(cDevOptions::ExtraModemBandwidthThrottling.Get());
#endif //WWDEBUG

	//
	//	Start the server (if necessary)
	//
	if (IsServerRequired && !cNetwork::I_Am_Server ()) {
		cNetwork::Init_Server ();
		PacketManager.Set_Is_Server(true);

		//
		// Dedicated server disables playing of sfx & music
		//
		if ((IsClientRequired == false) && WWAudioClass::Get_Instance () != NULL) {

			if (WWAudioClass::Get_Instance ()->Are_Sound_Effects_On ()) {
				WWAudioClass::Get_Instance ()->Allow_Sound_Effects (false);
				RestoreSFX = true;
			}

			if (WWAudioClass::Get_Instance ()->Is_Music_On ()) {
				WWAudioClass::Get_Instance ()->Allow_Music (false);
				RestoreMusic = true;
			}
		}
	}

	//
	//	Start the client (if necessary)
	//
	if (IsClientRequired && !cNetwork::I_Am_Client ()) {

		if (!IsServerRequired) {
			PacketManager.Set_Is_Server(false);
		}

		assert(GameModeManager::Find("WOL"));
		if (GameModeManager::Find("WOL")->Is_Active()) {
			const unsigned short wol_client_port = WOLNATInterface.Get_Port_As_Server_Client();
			if (wol_client_port >= MIN_SERVER_PORT && wol_client_port <= MAX_SERVER_PORT) {
				cNetwork::Init_Client(wol_client_port);
			} else {
				WWDEBUG_SAY(("WOL client port %hu outside valid range, falling back to default client init\n", wol_client_port));
				cNetwork::Init_Client();
			}
		} else {
			cNetwork::Init_Client();
		}

		//
		//	Wait for the client to connect to the server
		//
		WWDEBUG_SAY(("BEFORE GameInitMgrClass::Start_Client_Server tight update loop\n"));
		WWDEBUG_SAY(("Game IP = %s\n", cNetUtil::Address_To_String(The_Game()->Get_Ip_Address())));
		unsigned int time = TIMEGETTIME();
		do {
			cNetwork::Update ();
			if (TIMEGETTIME() - time > 20*1000) {
				break;
			}
		} while (!cNetwork::PClientConnection->Is_Established ());
		WWDEBUG_SAY(("AFTER GameInitMgrClass::Start_Client_Server tight update loop\n"));
	}

	// Sample output every 2 seconds.
	PacketManager.Set_Stats_Sampling_Frequency_Delay(2000);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	End_Client_Server
//
////////////////////////////////////////////////////////////////
void
GameInitMgrClass::End_Client_Server (void)
{
   WWDEBUG_SAY (("GameInitMgrClass::End_Client_Server\n"));

	//
	//	Cleanup the client
	//
	if (cNetwork::I_Am_Client ()) {
		cNetwork::Cleanup_Client ();
	}

	//
	//	Cleanup the server
	//
	if (cNetwork::I_Am_Server ()) {
		cNetwork::Cleanup_Server ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Initialize_SP
//
////////////////////////////////////////////////////////////////
void
GameInitMgrClass::Initialize_SP (void)
{
#ifndef MULTIPLAYERDEMO

   WWDEBUG_SAY (("GameInitMgrClass::Initialize_SP\n"));

	if (Mode != MODE_UNKNOWN) {
		Shutdown ();
	}

	//
	// Notify combat
	//
	//cSingleData::Set_Is_Single_Player (true);
	cGameType::Set_Game_Type(GAMETYPE_MISSION);

	//
	// Notify wwnet
	//
	cSinglePlayerData::Init ();

	WideStringClass widestring;
	widestring.Convert_From("Renegade");
   cNetInterface::Set_Nickname(widestring);

	//
	//	Create the new game type
	//
	WWASSERT (PTheGameData == NULL);
	PTheGameData = new cGameDataSinglePlayer;
	WWASSERT(PTheGameData != NULL);

	//
	//	Remember our state
	//
	IsClientRequired	= true;
	IsServerRequired	= true;
	Mode					= MODE_SP;
	return ;

#endif // !MULTIPLAYERDEMO
}


////////////////////////////////////////////////////////////////
//
//	Shutdown_SP
//
////////////////////////////////////////////////////////////////
void
GameInitMgrClass::Shutdown_SP (void)
{
#ifndef MULTIPLAYERDEMO

   WWDEBUG_SAY (("GameInitMgrClass::Shutdown_SP\n"));

//// FIXME TSS Fix memory leak here

	//cSingleData::Set_Is_Single_Player(false);
	cGameType::Set_Game_Type(GAMETYPE_NONE);

#endif // !MULTIPLAYERDEMO
}


////////////////////////////////////////////////////////////////
//
//	Initialize_Skirmish
//
////////////////////////////////////////////////////////////////
void
GameInitMgrClass::Initialize_Skirmish(void)
{
#ifndef MULTIPLAYERDEMO

   WWDEBUG_SAY(("GameInitMgrClass::Initialize_Skirmish\n"));

	if (Mode != MODE_UNKNOWN) {
		Shutdown ();
	}

	//
	// Notify combat
	//
	//cSingleData::Set_Is_Single_Player(true);
	cGameType::Set_Game_Type(GAMETYPE_SKIRMISH);

	//
	//	Notify wwnet
	//
	cSinglePlayerData::Init ();

	WideStringClass widestring;
	widestring.Convert_From("Renegade");
   cNetInterface::Set_Nickname(widestring);

	//
	//	Create the new game type
	//
	WWASSERT (PTheGameData == NULL);
	PTheGameData = new cGameDataSkirmish;
	WWASSERT(PTheGameData != NULL);

	//
	//	Remember our state
	//
	IsClientRequired	= true;
	IsServerRequired	= true;
	Mode					= MODE_SKIRMISH;

#endif // !MULTIPLAYERDEMO
}


////////////////////////////////////////////////////////////////
//
//	Shutdown_Skirmish
//
////////////////////////////////////////////////////////////////
void
GameInitMgrClass::Shutdown_Skirmish(void)
{
#ifndef MULTIPLAYERDEMO

   WWDEBUG_SAY(("GameInitMgrClass::Shutdown_Skirmish\n"));

	//cSingleData::Set_Is_Single_Player(false);
	cGameType::Set_Game_Type(GAMETYPE_NONE);

#endif // !MULTIPLAYERDEMO
}


////////////////////////////////////////////////////////////////
//
//	Initialize_LAN
//
////////////////////////////////////////////////////////////////
void
GameInitMgrClass::Initialize_LAN (void)
{
   WWDEBUG_SAY (("GameInitMgrClass::Initialize_LAN\n"));

	if (Mode != MODE_UNKNOWN) {
		Shutdown ();
	}

	//cSingleData::Set_Is_Single_Player (false);
	cGameType::Set_Game_Type(GAMETYPE_MULTIPLAY);

	//
	// Activate LAN interface
	//
	GameModeManager::Find ("LAN")->Activate ();

	//
	//	Remember our state
	//
	IsClientRequired	= false;
	IsServerRequired	= false;
	Mode					= MODE_LAN;

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Shutdown_LAN
//
////////////////////////////////////////////////////////////////
void
GameInitMgrClass::Shutdown_LAN (void)
{
   WWDEBUG_SAY (("GameInitMgrClass::Shutdown_LAN\n"));

   //
	//	Deactive the LAN interface
	//
	GameModeManager::Find ("LAN")->Deactivate ();

	cGameType::Set_Game_Type(GAMETYPE_NONE);
}


////////////////////////////////////////////////////////////////
//
//	Initialize_WOL
//
////////////////////////////////////////////////////////////////
void
GameInitMgrClass::Initialize_WOL (void)
{
#ifndef MULTIPLAYERDEMO

#ifdef _WIN64
	// WOLAPI COM binaries are 32-bit only; bail out early in 64-bit builds.
	ConsoleBox.Print("Westwood Online is not available in 64-bit builds. Returning to main menu.\n");
	RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_MAIN_MENU);
	cGameType::Set_Game_Type(GAMETYPE_NONE);
	Mode = MODE_UNKNOWN;
	return;
#endif

	WWDEBUG_SAY (("GameInitMgrClass::Initialize_WOL\n"));

	if (Mode != MODE_UNKNOWN) {
		Shutdown ();
	}

	//cSingleData::Set_Is_Single_Player (false);
	cGameType::Set_Game_Type(GAMETYPE_MULTIPLAY);

	//
	// Activate WOL interface
	//
	GameModeManager::Find ("WOL")->Activate ();

	//
	//	Remember our state
	//
	IsClientRequired	= false;
	IsServerRequired	= false;
	Mode					= MODE_WOL;
	return ;

#endif // !MULTIPLAYERDEMO
}


////////////////////////////////////////////////////////////////
//
//	Shutdown_WOL
//
////////////////////////////////////////////////////////////////
void
GameInitMgrClass::Shutdown_WOL (void)
{
#ifndef MULTIPLAYERDEMO

   WWDEBUG_SAY (("GameInitMgrClass::Shutdown_WOL\n"));

	GameModeManager::Find ("WOL")->Deactivate ();

	cGameType::Set_Game_Type(GAMETYPE_NONE);

#endif // !MULTIPLAYERDEMO
}


////////////////////////////////////////////////////////////////
//
//	Shutdown
//
////////////////////////////////////////////////////////////////
void
GameInitMgrClass::Shutdown (void)
{
   WWDEBUG_SAY (("GameInitMgrClass::Shutdown\n"));

	switch (Mode)
	{
		case MODE_SP:
			Shutdown_SP ();
			break;

		case MODE_SKIRMISH:
			Shutdown_Skirmish ();
			break;

		case MODE_LAN:
			Shutdown_LAN ();
			break;

		case MODE_WOL:
			Shutdown_WOL ();
			break;
	}

	//
	//	Reset our state
	//
	IsClientRequired	= false;
	IsServerRequired	= false;
	Mode					= MODE_UNKNOWN;

	End_Client_Server();

	if (cSinglePlayerData::Is_Single_Player()) {
		//
		// This needs to be done after the Cleanup_Client and Cleanup_Server
		//
		cSinglePlayerData::Cleanup();
	}

	//
	//	Free the old game data
	//
	if (PTheGameData != NULL) {
		delete PTheGameData;
		PTheGameData = NULL;
	}

	//
	//	Reset state
	//
	IsClientRequired	= false;
	IsServerRequired	= false;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Think
//
////////////////////////////////////////////////////////////////
void
GameInitMgrClass::Think (void)
{
	//
	//	Safely exit the game and return to the menu (as necessary)
	//
	if (NeedsGameExit) {
		GameInitMgrClass::End_Game ();
		GameInitMgrClass::Display_End_Game_Menu ();
		NeedsGameExit = false;
	}

	if (NeedsGameExitAll) {
		GameInitMgrClass::End_Game ();
		extern void Stop_Main_Loop (int exitCode);
		Stop_Main_Loop (EXIT_SUCCESS);
	}

	return ;
}




void _reload_game_configuration_files(void)
{
	//
	// (gth) All of this stuff is part of the day 120 Renegade patch and
	// is designed to make it possible to change the following systems
	// in mods
	//
	ArmorWarheadManager::Init();
	BonesManager::Init();
	SurfaceEffectsManager::Init();

#if 0 // re-initting cameras seems to cause problems...
	CCameraClass::Init();
#endif

	// Reload dazzles
	FileClass * dazzle_ini_file = _TheFileFactory->Get_File("DAZZLE.INI");
	if (dazzle_ini_file) {
		INIClass dazzle_ini(*dazzle_ini_file);
		DazzleRenderObjClass::Init_From_INI(&dazzle_ini);
		_TheFileFactory->Return_File(dazzle_ini_file);
	}

	//	Reload the strings table
	TranslateDBClass::Initialize();
	FileClass *file	= _TheFileFactory->Get_File( "STRINGS.TDB" );
	if (file != NULL) {
		file->Open (FileClass::READ);				//	Open or the file
		if ( file->Is_Available() ) {
			ChunkLoadClass cload (file);				// Load the database
			SaveLoadSystemClass::Load(cload);
		}
		file->Close ();								// Close the file
		_TheFileFactory->Return_File (file);
	}

	// Reload scripts.dll
	ScriptManager::Destroy_Pending();
	ScriptManager::Shutdown();
	ScriptManager::Init();
}







