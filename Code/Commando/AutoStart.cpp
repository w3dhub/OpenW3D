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
 *                     $Archive:: /Commando/Code/Commando/AutoStart.cpp                       $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 8/13/02 4:58p                                               $*
 *                                                                                             *
 *                    $Revision:: 24                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"
#include "AutoStart.h"
//#include "dlgmplangametype.h"
#include "gameinitmgr.h"
#include "registry.h"
#include "_globals.h"
#include "gamedata.h"
#include "gameinitmgr.h"
#include "campaign.h"
#include "win.h"
#include "Except.h"
#include "listctrl.h"
#include "DlgWOLAutoStart.h"
#include "DlgDownload.h"
#include "translatedb.h"
#include "string_ids.h"
#include "slavemaster.h"
#include "gamesideservercontrol.h"
#include "gamedata.h"
#include "nat.h"
#include "ConsoleMode.h"
#include "WWOnline/WOLServer.h"
#include "Notify.h"
#include "ServerSettings.h"
#include "WOLBuddyMgr.h"
#include "bandwidthcheck.h"
#include "bandwidth.h"
#include "gamespyadmin.h"
#include "specialbuilds.h"
#include <cstdio>
#include <algorithm>

/*
** Single instance of restart class.
*/
AutoRestartClass AutoRestart;

/*
** Registry entries.
*/
const char *AutoRestartClass::REG_VALUE_AUTO_RESTART_FLAG = "AutoRestartFlag";
const char *AutoRestartClass::REG_VALUE_AUTO_RESTART_TYPE = "AutoRestartType";
static const char *WINDOWS_SUB_KEY_RUN_ONCE = "Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce\\";
static const char *WINDOWS_SUB_KEY_RUN_ONCE_APP = "Renegade";

/*
** Stupid extern for main loop exit.
*/
extern void Stop_Main_Loop (int exitCode);


AutoRestartProgressDialogClass *AutoRestartProgressDialogClass::Instance;


/***********************************************************************************************
 * AutoRestartClass::AutoRestartClass -- Class constructor                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/5/2001 3:21PM ST : Created                                                             *
 *=============================================================================================*/
AutoRestartClass::AutoRestartClass(void)
{
	RestartState = STATE_DONE;
	CancelRequest = false;
	GameMode = 0;
	NumChannelCreateTries = 0;
}


/***********************************************************************************************
 * AutoRestartClass::Restart_Game -- Initiate the restart process                              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/5/2001 3:22PM ST : Created                                                             *
 *=============================================================================================*/
void AutoRestartClass::Restart_Game(void)
{
	if (RestartState == STATE_DONE) {
		RestartState = STATE_FIRST;
		CancelRequest = false;
		RegistryClass registry (APPLICATION_SUB_KEY_NAME_WOLSETTINGS);
		if (registry.Is_Valid()) {
			GameMode = registry.Get_Int(REG_VALUE_AUTO_RESTART_TYPE, GameMode);
		}
		Set_Restart_Flag(false);
	}
}


/***********************************************************************************************
 * AutoRestartClass::Cancel -- Cancel game restart                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/5/2001 9:42PM ST : Created                                                             *
 *=============================================================================================*/
void AutoRestartClass::Cancel(void)
{
	CancelRequest = true;
}


/***********************************************************************************************
 * AutoRestartClass::Think -- Class service function                                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/5/2001 3:22PM ST : Created                                                             *
 *=============================================================================================*/
void AutoRestartClass::Think(void)
{
	static unsigned int time;

	bool can_render = ConsoleBox.Is_Exclusive() ? false : true;

	switch (RestartState) {

		/*
		** Idle, not doing anything.
		*/
		case STATE_DONE:
			break;

		/*
		** This is the first restart state. Initialise WOL and login.
		*/
		case STATE_FIRST:

			if (CancelRequest) {
				RestartState = STATE_CANCELLED;
			}

			/*
			** Create the progress dialog.
			*/
			if (can_render) {
				START_DIALOG(AutoRestartProgressDialogClass);
				AutoRestartProgressDialogClass::Get_Instance()->Add_Text(U_CHAR("Auto starting game. Type 'quit' at the console window to abort"));
			}
			ConsoleBox.Print("*** Auto starting game. Type 'quit' to abort ***\n");

			/*
			** Start listening for server control messages.
			*/
			GameSideServerControlClass::Init();

			/*
			** Initialise WOL or LAN mode.
			*/
			if (!GameMode) {
				if (can_render) {
					AutoRestartProgressDialogClass::Get_Instance()->Add_Text(U_CHAR("Initializing LAN Mode"));
				}
				if (cGameSpyAdmin::Get_Is_Server_Gamespy_Listed()) {
					ConsoleBox.Print("Initializing GameSpy Mode\n");
				} else {
					ConsoleBox.Print("Initializing LAN Mode\n");
				}
				GameInitMgrClass::Initialize_LAN();
				RestartState = STATE_GAME_MODE_WAIT;
			} else {
				if (can_render) {
					AutoRestartProgressDialogClass::Get_Instance()->Add_Text(U_CHAR("Initializing Westwood Online"));
				}
				ConsoleBox.Print("Initializing Westwood Online Mode\n");
				GameInitMgrClass::Initialize_WOL();
				RestartState = STATE_GAME_MODE_WAIT;

				/*
				** Force a new server list update. This will cause a complete WOLAPI reset too btw.
				*/
				RefPtr<WWOnline::Session> wol_session = WWOnline::Session::GetInstance(true);
				if (wol_session.IsValid()) {
					wol_session->Reset();
				} else {
					ConsoleBox.Print("Warning: WOL session not available to reset.\n");
				}

				/*
				** Hide the page dialog - we don't want that sucker popping up when there's no-one around.
				*/
				WOLBuddyMgr* buddy = WOLBuddyMgr::GetInstance(false);
				if (buddy) {
					buddy->HidePagedDialog();
					buddy->Release_Ref();
				}
			}
			break;


		case STATE_GAME_MODE_WAIT:
			if (cGameData::Is_Manual_Exit()) {
				/*
				** Allow the page dialog to display again.
				*/
				if (GameMode) {
					WOLBuddyMgr* buddy = WOLBuddyMgr::GetInstance(false);
					if (buddy) {
						buddy->ShowPagedDialog();
						buddy->Release_Ref();
					}
				}
				RestartState = STATE_DONE;
				Stop_Main_Loop (EXIT_SUCCESS);
				break;
			} else {

				if (!GameMode) {
					GameModeClass *game_mode = GameModeManager::Find("LAN");
					if (game_mode && game_mode->Is_Active()) {
						RestartState = STATE_CREATE_GAME;
						break;
					}

					/*
					** It won't have initialzed above if a shutdown was already pending. Maybe we need to try again.
					*/
					if (game_mode && game_mode->Is_Inactive()) {
						GameInitMgrClass::Initialize_LAN();
						break;
					}
				} else {
					GameModeClass *game_mode = GameModeManager::Find("WOL");
					if (game_mode && game_mode->Is_Active()) {
						/*
						** Logon to WOL.
						*/
							if (can_render) {
								AutoRestartProgressDialogClass::Get_Instance()->Add_Text(U_CHAR("Logging on"));
							}
							LogonAction = (WOLLogonAction) -1;
							WOLLogonMgr::Set_Quiet_Mode(true);
							RefPtr<WWOnline::Session> wol_session = WWOnline::Session::GetInstance(true);
							if (wol_session.IsValid()) {
								Observer<WWOnline::ServerError>::NotifyMe(*wol_session);
								WOLLogonMgr::Logon(this);
								RestartState = STATE_LOGIN;
							} else {
								ConsoleBox.Print("Warning: WOL session unavailable; aborting auto-login.\n");
								RestartState = STATE_CANCELLED;
							}
							break;
						}

					/*
					** It won't have initialzed above if a shutdown was already pending. Maybe we need to try again.
					*/
					if (game_mode && game_mode->Is_Inactive()) {
						GameInitMgrClass::Initialize_WOL();
						break;
					}
				}
			}
			break;



		/*
		** Wait for the login process to complete.
		*/
		case STATE_LOGIN:
			if (cGameData::Is_Manual_Exit()) {
				/*
				** Allow the page dialog to display again.
				*/
				if (GameMode) {
					WOLBuddyMgr* buddy = WOLBuddyMgr::GetInstance(false);
					if (buddy) {
						buddy->ShowPagedDialog();
						buddy->Release_Ref();
					}
				}
				Observer<WWOnline::ServerError>::StopObserving();
				RestartState = STATE_DONE;
				Stop_Main_Loop (EXIT_SUCCESS);
				break;
			}
			switch (LogonAction) {

				/*
				** Login failed. Might be that the user is still logged in from last time so retry.
				*/
				case WOLLOGON_FAILED:
					if (CancelRequest) {
						Observer<WWOnline::ServerError>::StopObserving();
						RestartState = STATE_CANCELLED;
						break;
					}
					if (can_render) {
						AutoRestartProgressDialogClass::Get_Instance()->Add_Text(U_CHAR("Failed, retrying"));
					}
					ConsoleBox.Print("Failed to log in, retrying\n");
					LogonAction = (WOLLogonAction) -1;
					if (can_render) {
						AutoRestartProgressDialogClass::Get_Instance()->Add_Text(U_CHAR("Logging on"));
					}
					ConsoleBox.Print("Logging on....\n");
					WOLLogonMgr::Logon(this);
					break;

				/*
				** Logged in OK. Move onto the next step.
				*/
				case WOLLOGON_SUCCESS:
					if (CancelRequest) {
						Observer<WWOnline::ServerError>::StopObserving();
						WOLLogonMgr::Logoff();
						RestartState = STATE_CANCELLED;
						break;
					}
					if (can_render) {
						AutoRestartProgressDialogClass::Get_Instance()->Add_Text(U_CHAR("Logged on OK"));
					}
					ConsoleBox.Print("Logged on OK\n");
					WOLLogonMgr::Set_Quiet_Mode(false);
					RestartState = STATE_CREATE_GAME;
					time = TIMEGETTIME();
					Observer<WWOnline::ServerError>::StopObserving();
					break;

				/*
				** Patch available. At this point we want to
				**
				**  1. Download the patch.
				**  2. Quit to apply the patch.
				**  3. Reload the level after the restart.
				**
				*/
				case WOLLOGON_PATCHREQUIRED:
				{
					RefPtr<WWOnline::Session> wol_session = WWOnline::Session::GetInstance(false);
					ConsoleBox.Print("Downloading patch\n");
					DlgDownload::DoDialog(TRANSLATE(IDS_WOL_DOWNLOAD), wol_session->GetPatchDownloadList(), true);
					Set_Restart_Flag(true);
					RestartState = STATE_DONE;
					Observer<WWOnline::ServerError>::StopObserving();
					break;
				}

				/*
				** Fatal error. Abort restart process.
				*/
				case WOLLOGON_CANCEL:
					WOLLogonMgr::Set_Quiet_Mode(false);
					RestartState = STATE_CANCELLED;
					Observer<WWOnline::ServerError>::StopObserving();
					break;
			}
			break;


		/*
		** Create the game object and initialise it with the settings from the last game.
		*/
		case STATE_CREATE_GAME:
		{
			if (cGameData::Is_Manual_Exit()) {
				/*
				** Allow the page dialog to display again.
				*/
				if (GameMode) {
					WOLBuddyMgr* buddy = WOLBuddyMgr::GetInstance(false);
					if (buddy) {
						buddy->ShowPagedDialog();
						buddy->Release_Ref();
					}
				}
				RestartState = STATE_DONE;
				Stop_Main_Loop (EXIT_SUCCESS);
				break;
			}

			if (CancelRequest) {
				if (GameMode) {
					WOLLogonMgr::Logoff();
				}
				RestartState = STATE_CANCELLED;
				break;
			}

			/*
			** We have to wait for the firewall thread to discover the local IP before we can build the game.
			*/
			if (GameMode) {
				if (FirewallHelper.Get_Local_Address() == 0) {
					if (TIMEGETTIME() - time < 1000 * 6) {
						break;
					}
				}
			}

			/*
			** Work out what type of game to create.
			*/
			if (can_render) {
				AutoRestartProgressDialogClass::Get_Instance()->Add_Text(U_CHAR("Building game info"));
			}

			/*
			int last_game_type = 0;
			RegistryClass registry (APPLICATION_SUB_KEY_NAME_WOLSETTINGS);
			if (registry.Is_Valid ()) {
				last_game_type = registry.Get_Int(REG_VALUE_LAST_GAME_TYPE, last_game_type);
			}

			last_game_type = std::min(last_game_type, NUM_GAME_TYPE_MENU_ENTRIES-1);
			last_game_type = std::max(last_game_type, 0);

			int game_type = MPLanGameTypeMenuClass::GameTypeList[last_game_type].GameType;
			*/

			int game_type = cGameData::GAME_TYPE_CNC;

			/*
			** Create the new game data.
			*/
			PTheGameData = cGameData::Create_Game_Of_Type((cGameData::GameTypeEnum)game_type);
			WWASSERT(PTheGameData != NULL);

			/*
			** Apply command line/ini settings.
			*/
			if (ServerSettingsClass::Parse(true) == false) {
				/*
				** Allow the page dialog to display again.
				*/
				if (GameMode) {
					WOLBuddyMgr* buddy = WOLBuddyMgr::GetInstance(false);
					if (buddy) {
						buddy->ShowPagedDialog();
						buddy->Release_Ref();
					}
				}
				RestartState = STATE_DONE;
				Stop_Main_Loop (EXIT_SUCCESS);
				break;
			}

			/*
			** Start listening for server control messages.
			*/
			//GameSideServerControlClass::Init();

			/*
			** Load alternate server settings if required.
			*/
			if (SlaveMaster.Am_I_Slave()) {
				RegistryClass reg(APPLICATION_SUB_KEY_NAME_OPTIONS);
				char file_name[MAX_PATH];
				reg.Get_String("MultiplayerSettings", file_name, sizeof(file_name), "");
				if (strlen(file_name)) {
					WWDEBUG_SAY(("Loading multiplayer settings from file %s\n", file_name));
					The_Game()->Set_Ini_Filename(file_name);
				}
			}

			/*
			** Load the game settings.
			*/
			The_Game()->Load_From_Server_Config();


			/*
			** Set MaxPlayers based on Bandwidth test results if MaxPlayers is 
			** set to 0 in the INI file.
			*/
			if (ServerSettingsClass::Get_Game_Mode() == ServerSettingsClass::MODE_WOL && 
				The_Game()->Get_Max_Players() == 0 && BandwidthCheckerClass::Got_Bandwidth()) {

				int max_players = (cBandwidth::Get_Bandwidth_Bps_From_Type(BANDWIDTH_AUTO) / 250000) * 4;
				if (max_players < 2) {
					if (cBandwidth::Get_Bandwidth_Bps_From_Type(BANDWIDTH_AUTO) > 100000) {
						max_players = 4;
					} else {
						max_players = 2;
					}
				} else if (max_players > 32) {
					max_players = 32;
				}
				The_Game()->Set_Max_Players(max_players);
			}

			/*
			** Override gama settings in command line mode.
			*/
			if (ServerSettingsClass::Is_Command_Line_Mode()) {
				The_Game()->IsAutoRestart.Set(true);
				The_Game()->IsDedicated.Set(true);
			}

			StringClass inifile = The_Game()->Get_Ini_Filename();
			StringClass nick(32, true);
			cNetInterface::Get_Nickname().Convert_To(nick);
			ConsoleBox.Set_Title(nick.Peek_Buffer(), inifile.Peek_Buffer());

			if (GameMode) {
				NumChannelCreateTries = 0;
				RestartState = STATE_CREATE_CHANNEL;
			} else {
				RestartState = STATE_START_GAME;
			}
			break;
		}

		/*
		** Create the game channel on the chat server.
		*/
		case STATE_CREATE_CHANNEL:
		{
			if (cGameData::Is_Manual_Exit()) {
				/*
				** Allow the page dialog to display again.
				*/
				if (GameMode) {
					WOLBuddyMgr* buddy = WOLBuddyMgr::GetInstance(false);
					if (buddy) {
						buddy->ShowPagedDialog();
						buddy->Release_Ref();
					}

					/*
					** Log off.
					*/
					WOLLogonMgr::Logoff();
				}
				RestartState = STATE_DONE;
				Stop_Main_Loop (EXIT_SUCCESS);
				break;
			}

			if (CancelRequest) {
				WOLLogonMgr::Logoff();
				RestartState = STATE_CANCELLED;
				break;
			}
			if (can_render) {
				AutoRestartProgressDialogClass::Get_Instance()->Add_Text(U_CHAR("Creating game channel...."));
			}
			ConsoleBox.Print("Creating game channel...\n");

			/*
			** Create the game channel.
			*/
			GameModeClass *game_mode = GameModeManager::Find("WOL");
			LastChannelCreateTime = TIMEGETTIME();

			if (game_mode && game_mode->Is_Active()) {
				WolGameModeClass* wol_game = (WolGameModeClass*) game_mode;
				WWASSERT(wol_game);
				wol_game->Set_Quiet_Mode(true);
				wol_game->SignalMe(*this);
				WWASSERT(PTheGameData != NULL);
				NumChannelCreateTries++;
				wol_game->Create_Game(The_Game());
				RestartState = STATE_WAIT_CHANNEL_CREATE;
			} else {
				RestartState = STATE_START_GAME;
			}
			break;
		}

		/*
		** Wait for the game channel creation result.
		*/
		case STATE_WAIT_CHANNEL_CREATE:
			if (cGameData::Is_Manual_Exit()) {
				/*
				** Allow the page dialog to display again.
				*/
				if (GameMode) {
					WOLBuddyMgr* buddy = WOLBuddyMgr::GetInstance(false);
					if (buddy) {
						buddy->ShowPagedDialog();
						buddy->Release_Ref();
					}

					/*
					** Log off.
					*/
					WOLLogonMgr::Logoff();
				}
				RestartState = STATE_DONE;
				Stop_Main_Loop (EXIT_SUCCESS);
			}
			if (CancelRequest) {
				WOLLogonMgr::Logoff();
				RestartState = STATE_CANCELLED;
				break;
			}
			break;


		/*
		** Waiting to retry channel create.
		*/
		case STATE_WAIT_CHANNEL_CREATE_RETRY:
			if (cGameData::Is_Manual_Exit()) {
				/*
				** Allow the page dialog to display again.
				*/
				if (GameMode) {
					WOLBuddyMgr* buddy = WOLBuddyMgr::GetInstance(false);
					if (buddy) {
						buddy->ShowPagedDialog();
						buddy->Release_Ref();
					}

					/*
					** Log off.
					*/
					WOLLogonMgr::Logoff();
				}
				RestartState = STATE_DONE;
				Stop_Main_Loop (EXIT_SUCCESS);
				break;
			}
			if (CancelRequest) {
				WOLLogonMgr::Logoff();
				RestartState = STATE_CANCELLED;
				break;
			}
			/*
			** Give up and restart if we fail enough times when trying to create a channel.
			*/
			if (NumChannelCreateTries > 10) {
				Set_Exit_On_Exception(true);
         	cGameData::Set_Manual_Exit(true);
			}
			if (TIMEGETTIME() - LastChannelCreateTime > 5*1000) {
				ConsoleBox.Print("Retrying channel create\n");
				RestartState = STATE_CREATE_CHANNEL;
			}
			break;


		/*
		** Load the game.
		*/
		case STATE_START_GAME:
		{
			if (cGameData::Is_Manual_Exit()) {
				/*
				** Allow the page dialog to display again.
				*/
				if (GameMode) {
					WOLBuddyMgr* buddy = WOLBuddyMgr::GetInstance(false);
					if (buddy) {
						buddy->ShowPagedDialog();
						buddy->Release_Ref();
					}

					/*
					** Log off.
					*/
					WOLLogonMgr::Logoff();
				}
				RestartState = STATE_DONE;
				Stop_Main_Loop (EXIT_SUCCESS);
				break;
			}

			if (can_render) {
				AutoRestartProgressDialogClass::Get_Instance()->Add_Text(U_CHAR("Channel created OK"));
			}

			ConsoleBox.Print("Channel created OK\n");

			SlaveMaster.Startup_Slaves();

			GameModeClass *game_mode = GameModeManager::Find("WOL");
			if (game_mode && game_mode->Is_Active()) {
				WolGameModeClass* wol_game = (WolGameModeClass*) game_mode;
				WWASSERT(wol_game);
				wol_game->Set_Quiet_Mode(false);
			}

			//ConsoleBox.Print("Loading level...\n");
			WWASSERT(PTheGameData != NULL);
			CampaignManager::Select_Backdrop_Number_By_MP_Type(The_Game()->Get_Game_Type());
			GameInitMgrClass::Set_Is_Client_Required(false);
			GameInitMgrClass::Set_Is_Server_Required(true);
			if (can_render) {
				AutoRestartProgressDialogClass::Get_Instance()->End_Dialog();
			}
			GameInitMgrClass::Start_Game(The_Game()->Get_Map_Name(), -1, 0);
			//ConsoleBox.Print("Level loaded OK\n");
			RestartState = STATE_DONE;

			/*
			** Allow the page dialog to display again (it can't really anyway since it's disabled when the game is in progress.
			*/
			if (GameMode) {
				WOLBuddyMgr* buddy = WOLBuddyMgr::GetInstance(false);
				if (buddy) {
					buddy->ShowPagedDialog();
					buddy->Release_Ref();
				}
			}
			break;
		}

		/*
		** User cancelled.
		*/
		case STATE_CANCELLED:
			RestartState = STATE_DONE;

			/*
			** Allow the page dialog to display again.
			*/
			WOLBuddyMgr* buddy = WOLBuddyMgr::GetInstance(false);
			if (buddy) {
				buddy->ShowPagedDialog();
				buddy->Release_Ref();
			}

			if (can_render) {
				AutoRestartProgressDialogClass::Get_Instance()->End_Dialog();
			}
			if (SlaveMaster.Am_I_Slave()) {
				if (GameMode) {
					WOLLogonMgr::Logoff();
				}
				RestartState = STATE_DONE;
				Stop_Main_Loop (EXIT_SUCCESS);
			} else {
				RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_MAIN_MENU);
			}
			break;

	}
}




/***********************************************************************************************
 * AutoRestartClass::HandleNotification -- Notification callback for WOL login                 *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Success code                                                                      *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/5/2001 3:23PM ST : Created                                                             *
 *=============================================================================================*/
void AutoRestartClass::HandleNotification(WOLLogonAction& action)
{
	LogonAction = action;
}


/***********************************************************************************************
 * WolGameModeClass::HandleNotification -- Handle server error notifications                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Server error tyoe                                                                 *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/8/2001 10:21PM ST : Created                                                            *
 *=============================================================================================*/
void AutoRestartClass::HandleNotification(WWOnline::ServerError& server_error)
{
	ConsoleBox.Print("Error - %S\n", server_error.GetDescription());
}


/***********************************************************************************************
 * AutoRestartClass::ReceiveSignal -- Channel creation signal handler                          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Game mode                                                                         *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/5/2001 3:23PM ST : Created                                                             *
 *=============================================================================================*/
void AutoRestartClass::ReceiveSignal(WolGameModeClass &game_mode)
{
	if (RestartState == STATE_WAIT_CHANNEL_CREATE) {
		if (game_mode.Channel_Create_OK()) {
			RestartState = STATE_START_GAME;
		} else {
			bool can_render = ConsoleBox.Is_Exclusive() ? false : true;
			if (can_render) {
				AutoRestartProgressDialogClass::Get_Instance()->Add_Text(U_CHAR("Failed, retrying"));
			}
			ConsoleBox.Print("Failed to create channel\n");
			RestartState = STATE_WAIT_CHANNEL_CREATE_RETRY;
		}
	}
}



/***********************************************************************************************
 * AutoRestartClass::Set_Restart_Flag -- Set state of auto restart mode                        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    New state                                                                         *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/5/2001 3:32PM ST : Created                                                             *
 *=============================================================================================*/
void AutoRestartClass::Set_Restart_Flag(bool enable)
{
	RegistryClass registry (APPLICATION_SUB_KEY_NAME_WOLSETTINGS);
	if (registry.Is_Valid ()) {
		registry.Set_Int(REG_VALUE_AUTO_RESTART_FLAG, enable ? 1 : 0);

		GameModeClass *game_mode = GameModeManager::Find("WOL");
		if (game_mode && game_mode->Is_Active()) {
			GameMode = 1;
		} else {
			GameModeClass *game_mode = GameModeManager::Find("LAN");
			if (game_mode && game_mode->Is_Active()) {
				GameMode = 0;
			}
		}

		if (enable) {
			registry.Set_Int(REG_VALUE_AUTO_RESTART_TYPE, GameMode);
			Set_Exit_On_Exception(true);
		} else {
			Set_Exit_On_Exception(false);
		}

		RegistryClass registry_too(WINDOWS_SUB_KEY_RUN_ONCE);
		if (registry_too.Is_Valid()) {

			if (enable) {
				/*
				** The the current path and build a path/file combo that points to the launcher.
				*/
				char path_to_exe[256];
				char drive[_MAX_DRIVE];
				char dir[_MAX_DIR];
				char path[_MAX_PATH];
				GetModuleFileNameA(ProgramInstance, path_to_exe, sizeof(path_to_exe));
				_splitpath(path_to_exe, drive, dir, NULL, NULL);
#ifdef FREEDEDICATEDSERVER
				_makepath(path, drive, dir, "renegadeserver", "exe");
#else  //FREEDEDICATEDSERVER
				_makepath(path, drive, dir, "renegade", "exe");

				char options[256];
				options[0] = 0;
				if (ServerSettingsClass::Is_Active()) {
					sprintf(options, " /startserver=%s", ServerSettingsClass::Get_Settings_File_Name());
				}

				if (ConsoleBox.Is_Exclusive()) {
					strcat(options, " /nodx");
				}

				strcat(path, options);
#endif //FREEDEDICATEDSERVER
				WWDEBUG_SAY(("Writing %s to RunOnce key\n", path));
				registry_too.Set_String(WINDOWS_SUB_KEY_RUN_ONCE_APP, path);
			} else {
				WWDEBUG_SAY(("Removing RunOnce key\n"));
				registry_too.Delete_Value(WINDOWS_SUB_KEY_RUN_ONCE_APP);
			}
		}
	}
}




/***********************************************************************************************
 * AutoRestartClass::Get_Restart_Flag -- Is a restart indicated by the registry?               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    New state                                                                         *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/5/2001 3:32PM ST : Created                                                             *
 *=============================================================================================*/
bool AutoRestartClass::Get_Restart_Flag(void)
{
	bool flag = false;

	RegistryClass registry (APPLICATION_SUB_KEY_NAME_WOLSETTINGS);
	if (registry.Is_Valid()) {
		int restart = registry.Get_Int(REG_VALUE_AUTO_RESTART_FLAG, 0);
		flag = restart ? true : false;
	}
	return(flag);
}
