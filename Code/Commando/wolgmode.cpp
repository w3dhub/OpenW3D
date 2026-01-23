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
 *                     $Archive:: /Commando/Code/Commando/wolgmode.cpp                        $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 10/19/02 12:51p                                             $*
 *                                                                                             *
 *                    $Revision:: 124                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "wolgmode.h"
#include "gamedata.h"
#include "gamechanlist.h"
#include "gamechannel.h"
#include <limits>
#include "gameinitmgr.h"
#include "WOLChatMgr.h"
#include "WOLBuddyMgr.h"
#include "WOLQuickMatch.h"
#include "WOLGameInfo.h"
#include "DlgWOLWait.h"
#include "DlgMessageBox.h"
#include "_globals.h"
#include "colors.h"
#include "cnetwork.h"
#include "GameResSend.h"
#include "playermanager.h"
#include "messagewindow.h"
#include "natter.h"
#include "wwprofile.h"
#include "bandwidthcheck.h"
#include "WOLLoginProfile.h"
#include "DlgDownload.h"
#include "AutoStart.h"
#include "cpudetect.h"
#include "dx8wrapper.h"
#include "systeminfolog.h"
#include "registry.h"
#include "init.h"
#include "debug.h"
#include <WWOnline/WOLString.h>
#include <WWOnline/WOLProduct.h>
#include <WWOnline/WOLSession.h>
#include <WWOnline/WOLChannel.h>
#include <WWOnline/WaitCondition.h>
#include <WWOnline/WOLGame.h>
#include <WWOnline/WOLGameOptions.h>
#include <WWOnline/WOLPageMsg.h>
#include <WWOnline/WOLLadder.h>
#include <WWOnline/WOLConnect.h>
#include <WWOnline/WOLServer.h>
#include <WWOnline/PingProfile.h>
#include <WWAudio/WWAudio.h>
#include <wwlib/realcrc.h>
#include "string_ids.h"
#include <wwtranslatedb/translatedb.h>
#include <wwdebug/wwdebug.h>
#include <wwdebug/wwprofile.h>
#include "specialbuilds.h"
#include "slavemaster.h"
#include "sctextobj.h"
#include "mainloop.h"
#include <cstdio>
#include <algorithm>

using namespace WWOnline;

// Check for patches every five minutes
#define PATCH_CHECK_FREQUENCY (1000 * 5)

const int RENEGADE_GAMECODE = 12;
const unichar_t* RENEGADE_LOBBY_PASSWORD = U_CHAR("not_a_valid_password");	// Password removed per Security review requirements. LFeenanEA - 27th January 2025

const unichar_t* __cdecl Translate_WOLString(const char* token)
{
	if (token) {
		StringClass desc(80, true);
		desc.Format("IDS_%s", token);
		const unichar_t* text = TRANSLATE_BY_DESC(desc);

		#ifdef WWDEBUG
		if (STRING_NOT_FOUND == text) {
			WWDEBUG_SAY(("ERROR: WOL String '%s' not found!\n", token));
			return U_CHAR("WOL_NO_STRING");
		}
		#endif

		return text;
	}

	WWDEBUG_SAY(("WARNING: WOL_xxxx token is NULL\n"));
	return U_CHAR("WOL_BADSTRING");
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::WolGameModeClass
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

WolGameModeClass::WolGameModeClass() :
		mWOLChatMgr(NULL),
		mWOLBuddyMgr(NULL),
		mGameInProgress(false),
		mQuietMode(false),
		mConnected(false),
		mLastPatchCheckTime(0),
		mStartQuitProcessTime(0),
		mPatchAvailable(false),
		mMonitorConnection(false)
	{
	WWDEBUG_SAY(("WOLGameModeClass Instantiated\n"));

	WOLString::SetLookupFunc(Translate_WOLString);
	}


/******************************************************************************
*
* NAME
*     WolGameModeClass::~WolGameModeClass
*
* DESCRIPTION
*     Destructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

WolGameModeClass::~WolGameModeClass()
	{
	WWDEBUG_SAY(("WOLGameModeClass Destroyed\n"));
	}


/******************************************************************************
*
* NAME
*     WolGameModeClass::Init
*
* DESCRIPTION
*     Activate Westwood Online game mode
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::Init(void)
{
	WWDEBUG_SAY(("WolGameModeClass Init\n"));

	// Check if product is already initialized
	RefPtrConst<Product> product = Product::Current();

	if (!product.IsValid()) {
		Product::Initializer(APPLICATION_SUB_KEY_NAME, RENEGADE_GAMECODE, RENEGADE_LOBBY_PASSWORD, RENEGADE_BASE_SKU);
	}

	mWOLSession = Session::GetInstance(true);

	if (mWOLSession.IsValid()) {
		mWOLSession->EnableProgressiveChannelList(true);
		mWOLSession->SetAutoRequestFlags(REQUEST_NONE);

		Observer<LadderInfoEvent>::NotifyMe(*mWOLSession);

		mWOLBuddyMgr = WOLBuddyMgr::GetInstance(true);
		WWASSERT_PRINT(mWOLBuddyMgr, "WOLBuddyManager failed to instantiate.");

		mWOLChatMgr = WOLChatMgr::GetInstance(true);
		WWASSERT_PRINT(mWOLChatMgr, "WOLChatMgr failed to instantiate.");

		WOLNATInterface.Init();

		LoginProfile::EnableSaving(mWOLSession->IsStoreLoginAllowed());
	}

	mQuickMatch = NULL;
	mTheGame = NULL;
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::Shutdown
*
* DESCRIPTION
*     Deactivate Westwood Online game mode.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::Shutdown(void)
{
	WWDEBUG_SAY(("WolGameModeClass Shutdown\n"));

	if (mWOLSession.IsValid()) {
		WOLNATInterface.Shutdown();

		RefPtr<WaitCondition> logoutWait = mWOLSession->Logout();
		WWASSERT(logoutWait.IsValid());
		DlgWOLWait::DoDialog(IDS_WOL_LOGOFF, logoutWait);
	}

	if (mWOLBuddyMgr) {
		mWOLBuddyMgr->Release_Ref();
		mWOLBuddyMgr = NULL;
	}

	if (mWOLChatMgr) {
		mWOLChatMgr->Release_Ref();
		mWOLChatMgr = NULL;
	}

	// Release current profile.
	LoginProfile::SetCurrent(NULL);
	LoginInfo::ClearList();

	mMonitorConnection = false;

	mWOLSession.Release();
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::Think
*
* DESCRIPTION
*     Periodic processing; called once each game loop.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::Think(void)
{
	WWPROFILE("WOL Think");
	static unsigned int _last_auto_kick = 0;
	static unsigned int _kick_history_persist_time = 1000 * 60 * 60;
	unsigned int time = TIMEGETTIME();

	//---------------------------------------------------------------------------
	// Yield time to WWOnline
	//---------------------------------------------------------------------------
	if (mWOLSession.IsValid()) {
		mWOLSession->Process();
	}

	//---------------------------------------------------------------------------
	// Periodically update quickmatch bot with recent server information
	//---------------------------------------------------------------------------
	if (mGameInProgress) {// && mQuickMatch) {
		unsigned int theTime = TIMEGETTIME();

		if (theTime >= mSendServerInfoTime) {
			//mSendServerInfoTime = (theTime + (120 * 1000));
			//
			// Try to help with hacks by updating the topic more frequently. Grrrr.
			//
			mSendServerInfoTime = (theTime + (20 * 1000));

			RefPtr<ChannelData> channel = mWOLSession->GetCurrentChannel();

			Update_Channel_Settings(mTheGame, channel);
			mWOLSession->SendChannelTopic();

			if (mQuickMatch) {
				const char* exInfo = channel->GetExtraInfo();
				const char* topic = channel->GetTopic();
				mQuickMatch->SendServerInfo(exInfo, topic);
			}
		}
	}

	//---------------------------------------------------------------------------
	// Autokick
	//---------------------------------------------------------------------------
	if (mGameInProgress && (time - _last_auto_kick > 2000 || time < _last_auto_kick)) {
		_last_auto_kick = time;
		Auto_Kick();

		//
		// Check for deadbeat channel lurkers.
		//
		if (cNetwork::I_Am_Server() && mGameInProgress) {
		
			unsigned int time = TIMEGETTIME();

			//
			// Clear out old kicklist entries.
			//
			for (int i = IdleKickNameList.Count() - 1 ; i >= 0 ; i--) {
				if (time - IdleKickTimeList[i] > _kick_history_persist_time || time < IdleKickTimeList[i]) {
					IdleKickTimeList.Delete(i);
					IdleKickNameList.Delete(i);
				}
			}

			const UserList& userList = mWOLSession->GetUserList();
			const size_t count = userList.size();

			for (size_t index = 0; index < count; index++) {
				const RefPtr<UserData>& user = userList[index];
				WWASSERT(user.IsValid());
			
				if (user.IsValid()) {
				
					//
					// Make sure it's not me.
					//
					const RefPtr<LoginInfo>& login = mWOLSession->GetCurrentLogin();

					WideStringClass myname;
					if (login.IsValid()) {
						myname = login->GetNickname();
					}
				
					if (myname.Compare_No_Case(user->GetName())) {
				
						//
						// Handle timer wrap.
						//
						if (time < user->mKickTimer) {
							user->mKickTimer = time;
						}
				
						if (!cPlayerManager::Find_Player(user->GetName())) {
							if (time - user->mKickTimer > 120 * 1000) {
								StringClass tempstr;
								user->GetName().Convert_To(tempstr);
								WWDEBUG_SAY(("Kicking '%s' for sitting in the channel while not in the game\n", tempstr.Peek_Buffer()));
								mWOLSession->KickUser(user->GetName());
								user->mKickTimer += 10*1000;

								//
								// If he's already been kicked for this recently then ban him.
								//
								int chances = 3;
								for (int i=0 ; i < IdleKickNameList.Count() ; i++) {
									if (IdleKickNameList[i].Compare_No_Case(tempstr) == 0) {
										chances--;
										if (chances == 0) {
											mWOLSession->BanUser(user->GetName(), true);
											WWDEBUG_SAY(("Banning '%s' for being kicked too often.\n", tempstr.Peek_Buffer()));
											break;
										}
									}
								}

								if (chances > 0) {
									WWDEBUG_SAY(("'%s' not banned - %d chances left.\n", tempstr.Peek_Buffer(), chances));
									
									StringClass stringy(user->GetName());
									IdleKickNameList.Add(stringy);
									IdleKickTimeList.Add(TIMEGETTIME());
								}
							}
						} else {
							user->mKickTimer = time;
						}	
					}
				}	
			}
		}
	}

	//---------------------------------------------------------------------------
	// Check for restart conditions.
	//---------------------------------------------------------------------------
	if (mMonitorConnection) {

		// Check for a pending restart for a new patch.
		if ((mPatchAvailable || !mConnected) && mStartQuitProcessTime) {
			if (time - mStartQuitProcessTime > 1000 * 4) {
				if (SlaveMaster.Am_I_Slave()) {
					Set_Exit_On_Exception(true);
         		cGameData::Set_Manual_Exit(true);
				} else {
					Quit_And_Restart();
				}
				return;
			}
		}

		// Check for patches.
		// This doesn't work because you can't get a server list without also doing a complete reset of wolapi.
#if (0)
		if (time - mLastPatchCheckTime > PATCH_CHECK_FREQUENCY) {
			mLastPatchCheckTime = time;
			if (mWOLSession.IsValid()) {
#ifdef WWDEBUG
				bool success =
#endif //WWDEBUG
					mWOLSession->RequestServerList(true);
				WWASSERT(success);
			}
		}
#endif //(0)

		// If we have lost connection and the number of players drops to zero then we can quit.
		if (mMonitorConnection && !mConnected) {
			if (The_Game() && The_Game()->Get_Time_Limit_Minutes()) {
				if (cPlayerManager::Count() == 0) {
					Quit_And_Restart();
				}
			}
		}
	}

	// Firewall code service.
	WOLNATInterface.Service();
}



/******************************************************************************
*
* NAME
*     WolGameModeClass::System_Timer_Reset
*
* DESCRIPTION
*     Reset any internal variables as needed when the system timer resets.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::System_Timer_Reset(void)
{
	mSendServerInfoTime = (TIMEGETTIME() + (120 * 1000));
}



/******************************************************************************
*
* NAME
*     WolGameModeClass::Create_Game
*
* DESCRIPTION
*     Perform necessary game creation tasks for WWOnline.
*     NOTE: This should only be called by game hosts.
*
* INPUTS
*     Game - Instance of game to create for Westwood Online.
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::Create_Game(cGameData* theGame)
{
	WWDEBUG_SAY(("WolGameModeClass::Create_Game\n"));

	if (theGame) {
		mTheGame = theGame;
		mGameID = 0;
		mGameInProgress = false;

		//-------------------------------------------------------------------------
		// Create and initialize channel for the new game.
		//-------------------------------------------------------------------------
		const WideStringClass& name = theGame->Get_Owner();
		const unichar_t* password = theGame->Get_Password();
		WWDEBUG_SAY(("Creating game channel '%S' Password: '%S'\n", (const unichar_t*)name, password));

		RefPtrConst<Product> product = Product::Current();
		WWASSERT(product.IsValid());
		int gameCode = product->GetGameCode();

		RefPtr<ChannelData> channel = ChannelData::Create(name, password, gameCode);
		WWASSERT(channel.IsValid());

		// Export channel settings for the game.
		Update_Channel_Settings(theGame, channel);

		//-------------------------------------------------------------------------
		// Request WWOnline channel creation
		//-------------------------------------------------------------------------
		mChannelCreateSuccessFlag = false;
		RefPtr<WaitCondition> createWait = mWOLSession->CreateChannel(channel, password);
		WWASSERT(createWait.IsValid());
		DlgWOLWait::DoDialog(IDS_GAME_CREATECHANNEL, createWait, this, 0, mQuietMode ? 0xffffffff : 0);
	}
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::Leave_Game
*
* DESCRIPTION
*     Leave WWOnline game
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::Leave_Game(void)
{
	WWDEBUG_SAY(("WolGameModeClass::Leave_Game\n"));

	// Stop listening WOLSession notifications.
	Observer<GameOptionsMessage>::StopObserving();
	Observer<ChannelEvent>::StopObserving();
	Observer<UserEvent>::StopObserving();
	Observer<ServerError>::StopObserving();
	Observer<ConnectionStatus>::StopObserving();

	// Shutdown quickmatch
	if (mQuickMatch) {
		mQuickMatch->Release_Ref();
		mQuickMatch = NULL;
	}

	// Leave the game channel.
	if (mTheGame) {
		WWASSERT(mWOLSession.IsValid());
		RefPtr<WaitCondition> wait = mWOLSession->LeaveChannel();
		WWASSERT(wait.IsValid());
		DlgWOLWait::DoDialog(IDS_GAME_LEAVECHANNEL, wait);
	}

	mTheGame = NULL;
	mGameID = 0;
	mGameInProgress = false;
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::Start_Game
*
* DESCRIPTION
*     Start of game processing. This is performed by both the host and clients
*     at the begining of each game.
*
* INPUTS
*     Game - Instance of game starting
*
* RESULT
*     NONE
*
******************************************************************************/

void Get_Compact_Detail_String(StringClass& tmp);

void WolGameModeClass::Start_Game(cGameData* theGame)
{
	WWDEBUG_SAY(("WolGameModeClass::Start_Game\n"));

	// The game host (server) is responsible for obtaining the game ID
	if (cNetwork::I_Am_Server()) {
		WWASSERT(theGame == mTheGame && "Start_Game cGameData* mismatch");

		// Re-evaluate the clans in this game.
		Evaluate_Clans(theGame);

		//-------------------------------------------------------------------------
		// Update the channel settings for the new game.
		//-------------------------------------------------------------------------
		RefPtr<ChannelData> channel = mWOLSession->GetCurrentChannel();
		WWASSERT(channel.IsValid());
		Update_Channel_Settings(theGame, channel);

		// Send the chat server the new channel settings.
		mWOLSession->SendChannelExtraInfo();
		mWOLSession->SendChannelTopic();

		if (mQuickMatch) {
			const char* exInfo = channel->GetExtraInfo();
			const char* topic = channel->GetTopic();
			mQuickMatch->SendServerInfo(exInfo, topic);
		}

		// Reset timer to send server information for quickmatch
		mSendServerInfoTime = (TIMEGETTIME() + (120 * 1000));

		//-------------------------------------------------------------------------
		// Get WWOnline game id for this game.
		//-------------------------------------------------------------------------
		const WideStringClass& name = theGame->Get_Owner();

		UserList players;
		RefPtr<UserData> user = mWOLSession->FindUser(name);
		players.push_back(user);

		RefPtr<WaitCondition> wait = GameStartWait::Create(players, &Game_Start_Timeout_Callback);
		WWASSERT(wait.IsValid());

		Observer<GameStartEvent>::NotifyMe(*mWOLSession);
		DlgWOLWait::DoDialog(TRANSLATE (IDS_MENU_STARTING_WOL_GAME), wait, NULL, 0, mQuietMode ? 0xffffffff : 0);

		if (The_Game() && The_Game()->IsDedicated.Is_True()) {
			mConnected = true;
			mLastPatchCheckTime = TIMEGETTIME();
			mStartQuitProcessTime = 0;
			mMonitorConnection = true;
			Observer<ServerError>::NotifyMe(*mWOLSession);
			Observer<ConnectionStatus>::NotifyMe(*mWOLSession);
		}
	} else {
		mTheGame = theGame;
	}

	// Prevent the page dialog from showing while we are playing the game.
	// We will handle the displaying of pages and invitations within the game
	// messaging system.
	mWOLBuddyMgr->HidePagedDialog();
	Observer<WOLPagedEvent>::NotifyMe(*mWOLBuddyMgr);

	// Listen to events from WOLSession
	Observer<ChannelEvent>::NotifyMe(*mWOLSession);
	Observer<UserEvent>::NotifyMe(*mWOLSession);

	mGameInProgress = true;
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::End_Game
*
* DESCRIPTION
*     Do end of game processing.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::End_Game(void)
{
	if (mGameInProgress) {
		WWDEBUG_SAY(("WolGameModeClass::End_Game\n"));

		// The server sends the end of game results
		if (mTheGame) {
			if (cNetwork::I_Am_Server()) {
				SendGameResults(mGameID, mTheGame, cPlayerManager::Get_Player_Object_List());
				mWOLSession->GetChatObject()->RequestGameEnd();

				//
				// Re-read the ban list.
				//
				KickNameList.Delete_All();
				KickIPList.Delete_All();
				Read_Kick_List();

			} else {
				#ifdef BETACLIENT
				// Jani do stuff here (send client system information string)
				StringClass info(255, true);
				info = "SYSINFO:";
				info += CPUDetectClass::Get_Compact_Log();
				info += DX8Wrapper::Get_Current_Caps()->Get_Compact_Log();

				StringClass tmp(255,true);
				Get_Compact_Detail_String(tmp);
				info += tmp;

				PlayerInfoLog::Get_Compact_Log(tmp);
				info += tmp;

				SystemInfoLog::Get_Compact_Log(tmp);
				info += tmp;

				BandwidthCheckerClass::Get_Compact_Log(tmp);
				info += tmp;

				WOLNATInterface.Get_Compact_Log(tmp);
				info += tmp;

				WWDEBUG_SAY(("Sending compact log: %s\n", info.Peek_Buffer()));

				mWOLSession->SendPrivateGameOptions(mTheGame->Get_Owner(), info);
				#endif	// BETACLIENT
			}
		}

		// Allow the page dialog to show while we are no longer playing the game.
		Observer<WOLPagedEvent>::StopObserving();
		mWOLBuddyMgr->ShowPagedDialog();

		mGameID = 0;
		mGameInProgress = false;
	}
}


/***********************************************************************************************
 * WolGameModeClass::Post_Game_Check -- Call when game is about to cycle.                      *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   True if cycle should be aborted                                                   *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/7/2001 6:07PM ST : Created                                                             *
 *=============================================================================================*/
bool WolGameModeClass::Post_Game_Check(void)
{
	bool retval = false;

	if (mMonitorConnection && !mConnected) {
		Quit_And_Restart();
		retval = true;
	}

	mMonitorConnection = false;
	return(retval);
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::Accept_Actions
*
* DESCRIPTION
*
* INPUTS
*     Game - Instance of game joining
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::Accept_Actions(void)
{
	WWDEBUG_SAY(("WolGameModeClass::Accept_Actions\n"));
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::Refusal_Actions
*
* DESCRIPTION
*
* INPUTS
*     Game - Instance of game joining
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::Refusal_Actions(void)
{
	WWDEBUG_SAY(("WolGameModeClass::Refusal_Actions\n"));

	// If the client's request to join the game was refused then we need to make
	// sure we leave the host's channel.
	RefPtr<WaitCondition> wait = mWOLSession->LeaveChannel();
	DlgWOLWait::DoDialog(IDS_GAME_LEAVECHANNEL, wait);
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::Evaluate_Clans
*
* DESCRIPTION
*
* INPUTS
*     Game - Instance of game.
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::Evaluate_Clans(cGameData* theGame)
{
	if (theGame && cNetwork::I_Am_Server()) {
		theGame->Clear_Clans();

		if (theGame->IsClanGame.Is_True()) {
			WWDEBUG_SAY(("CLANS: Evaluating clan assignments\n"));

			// If the host is a member of a clan then his clan takes a slot
			RefPtr<UserData> host = mWOLSession->GetCurrentUser();

			if (host.IsValid()) {
				unsigned int hostClanID = host->GetSquadID();
				theGame->Set_Clan(0, hostClanID);

				WWDEBUG_SAY(("CLANS: Assigning slot 0 to '%S' (host) clan #%lu\n", (const unichar_t*)host->GetName(), hostClanID));
			}

			// Determine which clans are in the game.
			const UserList& userList = mWOLSession->GetUserList();
			const size_t count = userList.size();

			for (size_t index = 0; index < count; ++index) {
				const RefPtr<UserData>& user = userList[index];
				WWASSERT(user.IsValid());

				unsigned int userClanID = user->GetSquadID();

				if (userClanID != 0) {

					// If the clan is not already assigned then assign this clan to the next slot.
					if (!theGame->Is_Clan_Competing(userClanID)) {
						int slot = theGame->Find_Free_Clan_Slot();
						WWASSERT(slot != -1 && "More clans in game than slots available");

						if (slot != -1) {
							WWDEBUG_SAY(("CLANS: Assigning slot %d to clan #%lu\n",  slot, userClanID));
							theGame->Set_Clan(slot, userClanID);
						}
					}
				}
			}

			// Force server information to be resent.
			mSendServerInfoTime = 0;
		}
	}
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::Update_Channel_Settings
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::Update_Channel_Settings(cGameData* theGame, const RefPtr<ChannelData>& channel)
{
	WWASSERT(theGame != NULL);
	WWASSERT(channel.IsValid());

	if (theGame && channel.IsValid()) {
		WOLGameInfo gameInfo(*theGame);
		gameInfo.ExportToChannel(channel);

		// Quickmatch settings are appended to the channel topic
		if (theGame->Is_QuickMatch_Server()) {
			//---------------------------------------------------------------------------
			// Get average FPS of game (Capped at 255 fps)
			//---------------------------------------------------------------------------
			unsigned int fps = TimeManager::Get_Average_Frame_Rate();
			fps = std::min<unsigned int>(fps, 255);

			int numPlayers = theGame->Get_Current_Players();

			int avgPing = cPlayerManager::Get_Average_Ping();
			avgPing = std::min<int>(avgPing, UCHAR_MAX);

			unsigned short avgPoints = cPlayerManager::Get_Average_WOL_Points();

			int avgPlayed = cPlayerManager::Get_Average_Games_Played();
			avgPlayed = std::min<int>(avgPlayed, USHRT_MAX);

			// Quickmatch settings format is: |fps,mode,avg_ping,avg_rank,num_players
			const char* topic = channel->GetTopic();

			char newTopic[81];
			sprintf(newTopic, "%s|%02lx%02x%04x%04x%c", topic, fps, avgPing, avgPoints,
				avgPlayed, (32 + numPlayers));

			channel->SetTopic(newTopic);
		}
	}
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::Init_WOL_Player
*
* DESCRIPTION
*
* INPUTS
*     Player - Set players WOL settings.
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::Init_WOL_Player(cPlayer* player)
{
	if (player) {
		RefPtr<UserData> user = mWOLSession->FindUser(player->Get_Name());

		if (user.IsValid()) {
			RefPtr<LadderData> ladder = user->GetTeamLadder();

			if (ladder.IsValid()) {
				player->Set_WOL_Points(ladder->GetPoints());
				player->Set_Wol_Rank(ladder->GetRung());
				player->Set_Num_Wol_Games(ladder->GetReserved2());
				return;
			}
		}

		// Ladder points start at 10000 and vary up or down depending on the players
		// performance.
		player->Set_WOL_Points(10000);

		// Ranking of -1 indicates that the player doesn't have a rank.
		player->Set_Wol_Rank(-1);

		player->Set_Num_Wol_Games(0);
	}
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::Get_WOL_User_Data
*
* DESCRIPTION
*
* INPUTS
*     Player - Name of player to find.
*
* RESULT
*     User - WWOnline user data
*
******************************************************************************/

RefPtr<UserData> WolGameModeClass::Get_WOL_User_Data(const unichar_t* name)
{
	return mWOLSession->FindUser(name);
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::Page_WOL_User
*
* DESCRIPTION
*
* INPUTS
*     User    - Name of user to page.
*     Message - Message to send to user.
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::Page_WOL_User(const unichar_t* name, const unichar_t* msg)
{
	if (name && (u_strlen(name) > 0) && msg && (u_strlen(msg) > 0)) {
		mWOLSession->PageUser(name, msg);

		WideStringClass message(0, true);
		message.Format(TRANSLATE(IDS_GAME_PAGING), name, msg);
		CombatManager::Get_Message_Window()->Add_Message(message, COLOR_CONSOLE_TEXT);
	}
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::Reply_Last_Page
*
* DESCRIPTION
*     Send a reply message to the last user who paged.
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::Reply_Last_Page(const unichar_t* msg)
{
	if (msg && (u_strlen(msg) > 0)) {
		const unichar_t* pager = mWOLBuddyMgr->GetLastPagersName();

		if (pager && (u_strlen(pager) > 0)) {
			mWOLBuddyMgr->PageUser(pager, msg);

			WideStringClass message(0, true);
			message.Format(TRANSLATE(IDS_GAME_PAGE_REPLYING), pager, msg);
			CombatManager::Get_Message_Window()->Add_Message(message, COLOR_CONSOLE_TEXT);
		} else {
			CombatManager::Get_Message_Window()->Add_Message(TRANSLATE(IDS_GAME_PAGE_REPLY_CANT), COLOR_CONSOLE_TEXT);
		}
	}
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::Locate_WOL_User
*
* DESCRIPTION
*
* INPUTS
*     User - Name of user to locate.
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::Locate_WOL_User(const unichar_t* name)
{
	if (name && u_strlen(name) > 0) {
		mWOLSession->RequestLocateUser(name);

		WideStringClass message(0, true);
		message.Format(TRANSLATE(IDS_GAME_LOCATING_USER), name);
		CombatManager::Get_Message_Window()->Add_Message(message, COLOR_CONSOLE_TEXT);
	}
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::Invite_WOL_User
*
* DESCRIPTION
*
* INPUTS
*     User - Name of user to locate.
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::Invite_WOL_User(const unichar_t* name, const unichar_t* msg)
{
	if (name && u_strlen(name)) {
		cPlayer* player = cPlayerManager::Find_Player(name);

		if (player) {
			WideStringClass message(0, true);
			message.Format(TRANSLATE (IDS_MENU_ALREADY_IN_GAME), name);
			CombatManager::Get_Message_Window()->Add_Message(message, COLOR_INVITE_TEXT);
			WWAudioClass::Get_Instance()->Create_Instant_Sound("Private_Message", Matrix3D(1));
		} else {
			mWOLBuddyMgr->InviteUser(name, msg);

			WideStringClass message(0, true);
			message.Format(TRANSLATE(IDS_GAME_INVITING_USER), name);
			CombatManager::Get_Message_Window()->Add_Message(message, COLOR_CONSOLE_TEXT);
		}
	}
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::Join_WOL_User
*
* DESCRIPTION
*
* INPUTS
*     User - Name of user to join.
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::Join_WOL_User(const unichar_t* name)
{
	// If the name is valid and it is not me
	if (name && u_strlen(name) && !mWOLSession->IsCurrentUser(name)) {
		cPlayer* player = cPlayerManager::Find_Player(name);

		if (player) {
			WideStringClass message(0, true);
			message.Format(TRANSLATE(IDS_MENU_YOURE_ALREADY_IN_GAME), name);
			CombatManager::Get_Message_Window()->Add_Message(message, COLOR_INVITE_TEXT);
			WWAudioClass::Get_Instance()->Create_Instant_Sound("Private_Message", Matrix3D(1));
		} else {
			RefPtr<UserData> user = UserData::Create(name);
			mWOLBuddyMgr->JoinUser(user);

			WideStringClass message(0, true);
			message.Format(TRANSLATE(IDS_GAME_JOINING_USER), name);
			CombatManager::Get_Message_Window()->Add_Message(message, COLOR_CONSOLE_TEXT);
		}
	}
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::Kick_Player
*
* DESCRIPTION
*     Kick a player from the game. Players kicked from a game are also banned
*     from rejoining that game. Note: This is only allowed by game server hosts.
*
* INPUTS
*     Player - Name of player to kick from the game.
*
* RESULT
*     NONE
*
******************************************************************************/

bool WolGameModeClass::Kick_Player(const unichar_t* name)
{
	// If the name is not NULL. we are the server and the player to kick is
	// not ourself then proceed with the kick.
	if (name && (u_strlen(name) > 0)) {
		if (cNetwork::I_Am_Server() && !mWOLSession->IsCurrentUser(name)) {
			mWOLSession->KickUser(name);
			mWOLSession->BanUser(name, true);

			// Disconnect the player from the server
			WideStringClass playername(0, true);
			playername = name;

			cPlayer* player = cPlayerManager::Find_Player(playername);

			if (player && player->Is_Human()) {
				WideStringClass message(0, true);
				message.Format(TRANSLATE(IDS_GAME_KICKING_USER), name);
				CombatManager::Get_Message_Window()->Add_Message(message, COLOR_CONSOLE_TEXT);

				int playerID = player->Get_Id();
				cNetwork::Server_Kill_Connection(playerID);
				cNetwork::Cleanup_After_Client(playerID);
				return(true);
			}
		}

		WideStringClass message(0, true);
		message.Format(TRANSLATE(IDS_GAME_KICKING_NOT_ALLOWED), name);
		CombatManager::Get_Message_Window()->Add_Message(message, COLOR_CONSOLE_TEXT);
	}
	return(false);
}


/***********************************************************************************************
 * WolGameModeClass::Ban_Player -- Add player to ban list                                      *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Player name                                                                       *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/8/2002 4:33PM ST : Created                                                              *
 *=============================================================================================*/
void WolGameModeClass::Ban_Player(const unichar_t* name, unsigned int ip)
{
	// If the name is not NULL. we are the server and the player to kick is
	// not ourself then proceed with the kick.
	if (name && cNetwork::I_Am_Server() && !mWOLSession->IsCurrentUser(name)) {

		Kick_Player(name);
		WideStringClass playername(0, true);
		playername = name;
		StringClass pn;
		playername.Convert_To(pn);

		if (!Is_Banned(pn, ip)) {

			KickNameList.Add(pn);
			KickIPList.Add(ip);
			char ipstr[128];
			pn += ":";
			unsigned char *ip_ptr = (unsigned char*)&ip;
			sprintf(ipstr, "%u.%u.%u.%u\n", (unsigned int)ip_ptr[0], (unsigned int)ip_ptr[1], (unsigned int)ip_ptr[2], (unsigned int)ip_ptr[3]);
			pn += ipstr;

	   	FILE *kick_list = fopen("wolbanlist.txt", "at");
	   	if (kick_list != NULL) {
			   fwrite(pn.Peek_Buffer(), 1, pn.Get_Length(), kick_list);
			   fclose(kick_list);
	   	}
		}
	}
}


/***********************************************************************************************
 * WolGameModeClass::Auto_Kick -- Kick any banned users from the game                          *
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
 *   8/8/2002 9:24PM ST : Created                                                              *
 *=============================================================================================*/
void WolGameModeClass::Auto_Kick(void)
{
	if (cNetwork::I_Am_Server()) {
		cPlayer *player = NULL;
		for (SLNode<cPlayer> *player_node = cPlayerManager::Get_Player_Object_List()->Head() ; player_node != NULL; player_node = player_node->Next()) {
			player = player_node->Data();
			if (player->Is_Active()) {
				if (!mWOLSession->IsCurrentUser(player->Get_Name())) {
					StringClass player_name(player->Get_Name());
					if (Is_Banned(player_name, player->Get_Ip_Address())) {
						Kick_Player(player->Get_Name());
					}
				}
			}
		}
	}
}


/***********************************************************************************************
 * WolGameModeClass::Is_Banned -- Is this player banned from our WOL server?                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Player name                                                                       *
 *           IP Address                                                                        *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/8/2002 9:14PM ST : Created                                                              *
 *=============================================================================================*/
bool WolGameModeClass::Is_Banned(const char *player_name, unsigned int ip)
{
	int i;

	if (KickNameList.Count() == 0) {
		Read_Kick_List();
	}

	for (i=0 ; i<KickNameList.Count() ; i++) {
		if (stricmp(player_name, KickNameList[i].Peek_Buffer()) == 0) {
			return(true);
		}
	}

	for (i=0 ; i<KickIPList.Count() ; i++) {
		if (KickIPList[i] == ip) {
			return(true);
		}
	}

	return(false);
}


/***********************************************************************************************
 * WolGameModeClass::Read_Kick_List -- Read the list of autokick players                       *
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
 *   8/8/2002 9:14PM ST : Created                                                              *
 *=============================================================================================*/
void WolGameModeClass::Read_Kick_List(void)
{
	FILE *kick_list = fopen("wolbanlist.txt", "rt");
	if (kick_list) {

		int i = 0;
		bool eof = false;
		char temp[256];

		while (!eof) {
			for (i=0 ; i<256 ; i++) {
				if (fread(&temp[i], 1, 1, kick_list) == 0) {
					eof = true;
					break;
				}
				if (temp[i] == '\n') {
					break;
				}
			}
			if (i > 10) {
				temp[i + 1] = 0;
				char *colon_ptr = strchr(temp, ':');
				if (colon_ptr) {
					*colon_ptr = 0;
					KickNameList.Add(temp);
					unsigned int ip = inet_addr(colon_ptr + 1);
					KickIPList.Add(ip);
				}
			}
		}
		fclose(kick_list);
	}
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::HandleNotification(ChannelEvent)
*
* DESCRIPTION
*
* INPUTS
*     Event -
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::HandleNotification(ChannelEvent& event)
{
	// This user has been kicked from the game.
	if (ChannelKicked == event.GetStatus()) {
		WWDEBUG_SAY(("WolGameModeClass Player Kicked!\n"));

		mGameInProgress = false;

		GameInitMgrClass::End_Game();
		GameInitMgrClass::Display_End_Game_Menu();

		DlgMsgBox::DoDialog(TRANSLATE(IDS_MENU_SERVER_MESSAGE_TITLE), TRANSLATE (IDS_MENU_SERVER_KICKED_MESSAGE));
	}
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::HandleNotification(UserEvent)
*
* DESCRIPTION
*
* INPUTS
*     Event -
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::HandleNotification(UserEvent& event)
{
	switch (event.GetEvent()) {
		// A user has been kicked from the game.
		case UserEvent::Kicked: {
			WideStringClass message(0, true);
			message.Format(TRANSLATE(IDS_MENU_PLAYER_KICKED_MESSAGE), event.Subject()->GetName());
			CombatManager::Get_Message_Window()->Add_Message(message, COLOR_PUBLIC_TEXT);
			WWAudioClass::Get_Instance()->Create_Instant_Sound("Public_Message", Matrix3D(1));
		}
		break;

		case UserEvent::Located: {
			const RefPtr<UserData>& user = event.Subject();

			// Get the description of the user's location
			WideStringClass location(64, true);
			WOLBuddyMgr::GetLocationDescription(user, location);

			// Build a string containing the user's name
			WideStringClass message(0, true);
			message.Format(TRANSLATE(IDS_CHAT_LOCATEDUSER), user->GetName());
			message += U_CHAR(" - ");
			message += location;
			CombatManager::Get_Message_Window()->Add_Message(message, COLOR_PRIVATE_TEXT);
			WWAudioClass::Get_Instance()->Create_Instant_Sound("Private_Message", Matrix3D(1));
		}
		break;

		case UserEvent::LadderInfo:
			if (mTheGame) {
				const RefPtr<UserData>& user = event.Subject();
				cPlayer* player = cPlayerManager::Find_Player(user->GetName());
				Init_WOL_Player(player);
			}
		break;

		case UserEvent::Join:
			if (mTheGame && cNetwork::I_Am_Server()) {
				const RefPtr<UserData>& user = event.Subject();
				bool requestDetails = true;

				// If the user is joining a clan game.
				if (mTheGame->IsClanGame.Is_True()) {
					WWDEBUG_SAY(("CLANS: User join clan assignment\n"));

					unsigned int userClanID = user->GetSquadID();
					WWASSERT(userClanID != 0 && "User not in a clan");

					// If the game is open to a new clan then assign the user to a
					// participating clan or a new slot.
					if (mTheGame->Is_Clan_Game_Open()) {

						// If the user is not a member of a participating clan then assign
						// the next free slot to this users clan.
						if (!mTheGame->Is_Clan_Competing(userClanID)) {
							int slot = mTheGame->Find_Free_Clan_Slot();

							if (slot != -1) {
								WWDEBUG_SAY(("CLANS: Slot %d filled by '%S' Clan #%lu\n", slot,
									(const unichar_t*)user->GetName(), userClanID));

								mTheGame->Set_Clan(slot, userClanID);
							}

							// Send the chat server the new channel settings.
							RefPtr<ChannelData> channel = mWOLSession->GetCurrentChannel();
							Update_Channel_Settings(mTheGame, channel);
							mWOLSession->SendChannelExtraInfo();
						}
					} else {
						// If the game is closed and the user is not a member of one of the
						// competing clans then kick him.
						if (!mTheGame->Is_Clan_Competing(userClanID)) {
							WWDEBUG_SAY(("CLANS: Game closed. Kicking user '%S'\n",
								(const unichar_t*)user->GetName()));

							mWOLSession->KickUser(user->GetName());
							requestDetails = false;
						}
					}
				}

				if (requestDetails) {
					mWOLSession->RequestUserDetails(user, REQUEST_LADDERINFO|REQUEST_SQUADINFO);
				}
			}
			break;

		case UserEvent::Leave:
			if (mTheGame && cNetwork::I_Am_Server()) {
				if (mTheGame->IsClanGame.Is_True()) {
					WWDEBUG_SAY(("CLANS: User leave re-evaluate clan assignment\n"));
					Evaluate_Clans(mTheGame);
				}
			}
			break;

		default:
		break;
	}
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::HandleNotification(DlgWOLWaitEvent)
*
* DESCRIPTION
*     Response to completion of game channel creation.
*
* INPUTS
*     Event -
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::HandleNotification(DlgWOLWaitEvent& wolEvent)
{
	if (wolEvent.Result() == WaitCondition::ConditionMet) {
		WWASSERT(mTheGame != NULL);

		mChannelCreateSuccessFlag = true;

		// Startup quickmatch if we are serving quickmatch games.
		if (mTheGame->Is_QuickMatch_Server()) {
			mQuickMatch = WOLQuickMatch::Create();
			WWASSERT(mQuickMatch);
		}

		mSendServerInfoTime = 0;

		// Start listening to Game Options messages from clients
		Observer<GameOptionsMessage>::NotifyMe(*mWOLSession);

		// Listen to events from WOLSession
		Observer<ChannelEvent>::NotifyMe(*mWOLSession);
		Observer<UserEvent>::NotifyMe(*mWOLSession);
	} else {
		if (!mQuietMode) {
			DlgMsgBox::DoDialog(TRANSLATE (IDS_MENU_FAILED_TO_CREATE_GAME), wolEvent.Subject()->GetResultText());
		}	
	}

	Signaler<WolGameModeClass>::SendSignal(*this);
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::HandleNotification(GameStartEvent)
*
* DESCRIPTION
*     Handle game start acknowledgement.
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::HandleNotification(GameStartEvent& start)
{
	WWDEBUG_SAY(("GameStartEvent received\n"));
	mGameID = 0;

	if (start.IsSuccess()) {
		mGameID = start.GetGameID();
		WWDEBUG_SAY(("GameID = %ld\n", mGameID));
	}
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::HandleNotification(GameOptionsMessage)
*
* DESCRIPTION
*     Handle game options requests coming from other users.
*
* INPUTS
*     OptionsMessage -
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::HandleNotification(GameOptionsMessage& message)
{
	if (cNetwork::I_Am_Server() && mTheGame) {
		const char* request = message.GetOptions();

		// Request for game information?
		if (strcmp("RGINFO", request) == 0) {
			WideStringClass requestor(0, true);
			requestor = message.GetSendersName();

			//-----------------------------------------------------------------------
			// Send game information
			//-----------------------------------------------------------------------
			unsigned int mapCRC = CRC_Stringi(mTheGame->Get_Map_Name());
			float seconds = mTheGame->Get_Time_Remaining_Seconds();

			// Game info sent as: MapCRC Seconds remaining
			StringClass info(0, true);
			info.Format("GINFO:%08lx %.4f", mapCRC, seconds);

			WWDEBUG_SAY(("%S\n", info));
			mWOLSession->SendPrivateGameOptions(requestor, info);

			//-----------------------------------------------------------------------
			// Send team information
			//-----------------------------------------------------------------------
			SList<cTeam>* teamList = cTeamManager::Get_Team_Object_List();
			WWASSERT(teamList != NULL);

			SLNode<cTeam>* teamNode = teamList->Head();

			while (teamNode) {
				cTeam* team = teamNode->Data();
				WWASSERT(team != NULL);

				int teamID = team->Get_Id();
				int teamScore = team->Get_Score();

				info.Format("TINFO:%d %d", teamID, teamScore);

				WWDEBUG_SAY(("%S\n", info));
				mWOLSession->SendPrivateGameOptions(requestor, info);

				teamNode = teamNode->Next();
			}

			//-----------------------------------------------------------------------
			// Send player information
			//-----------------------------------------------------------------------
			SList<cPlayer>* playerList = cPlayerManager::Get_Player_Object_List();
			SLNode<cPlayer>* playerNode = playerList->Head();

			while (playerNode) {
				cPlayer* player = playerNode->Data();

				if (player && player->Is_Active() && player->Is_Human()) {
					const WideStringClass& name = player->Get_Name();
					int type = player->Get_Player_Type();
					int rung = player->Get_Rung();
					int kills = player->Get_Kills();
					int deaths = player->Get_Deaths();
					int score = player->Get_Score();

					// PINFO string format: Name fps,type,rank,kills,deaths
					info.Format("PINFO:%S %d %d %d %d %d", (const unichar_t*)name, type, rung, kills, deaths, score);

					mWOLSession->SendPrivateGameOptions(requestor, info);
				}

				playerNode = playerNode->Next();
			}
		} else {
			// Write client system info to disk
			const char* cmd = strstr(request, "SYSINFO:");

			if (cmd == request) {
				static int sysinfo_log_disabled=-1;
				if (sysinfo_log_disabled==-1) {	// Read from registry only once per run
					RegistryClass registry( APPLICATION_SUB_KEY_NAME_DEBUG );
					if ( registry.Is_Valid() ) {
						sysinfo_log_disabled=registry.Get_Int( VALUE_NAME_DISABLE_SERVER_SYSINFO_COLLECTING, -1 );
						// Write to registry to create the key if it didn't exist
						if (sysinfo_log_disabled==-1) {
							sysinfo_log_disabled=0;
							registry.Set_Int( VALUE_NAME_DISABLE_SERVER_SYSINFO_COLLECTING, sysinfo_log_disabled);
						}
					}
				}
				// Only copy the sysinfo if it isn't disabled in registry
				if (sysinfo_log_disabled==0) {
					const char* data = (request + strlen("SYSINFO:"));
					const size_t datalen = ::strlen(data);
					if (datalen == 0) return;
					StringClass tmp(0,true);

					StringClass requestor(0, true);
					requestor = message.GetSendersName();

					StringClass datastring;
					datastring=requestor;
					datastring+="\t";

					unsigned int versionminor,versionmajor;
					Get_Version_Number(&versionmajor,&versionminor);

					SYSTEMTIME time;
					GetSystemTime(&time);
					tmp.Format("%d/%d/%d %d:%d\t%d.%d\t",
						time.wMonth,
						time.wDay,
						time.wYear,
						time.wHour,
						time.wMinute,
						versionmajor,
						versionminor);
					datastring+=tmp;

					datastring+=data;
					datastring+="\r\n";

					// Verify the sysinfo folder
					StringClass dirname(0,true);
					dirname.Format("sysinfo_%d",DebugManager::Get_Version_Number());
					if (GetFileAttributesA(dirname)==0xffffffff) {
						if (!CreateDirectoryA(dirname,NULL)) {
							return;
						}
					}

					StringClass filename(0,true);
					filename=dirname;
					filename+="\\";
					tmp=requestor;
					filename+=tmp;
					filename+=".txt";

					DWORD written;
					HANDLE file;
					file = CreateFileA(filename, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
							FILE_ATTRIBUTE_NORMAL, NULL);
					if (INVALID_HANDLE_VALUE != file) {
						SetFilePointer(file, 0, NULL, FILE_END);
						const size_t data_length = ::strlen(datastring);
						WWASSERT(data_length <= std::numeric_limits<DWORD>::max());
						WriteFile(file, datastring, static_cast<DWORD>(data_length), &written, NULL);
						CloseHandle(file);
					}
				}
			}
		}
	}
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::HandleNotification(LadderInfoEvent)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::HandleNotification(LadderInfoEvent& ladderEvent)
{
	if (ladderEvent.IsRanked()) {
		LadderType type = ladderEvent.GetLadderType();

		// If this is for the locale user and the ladder information is cached
		// then update there profile with the latest ladder information.
		if (LadderType_Team == type) {
			WideStringClass name(64, true);
			name = ladderEvent.GetReceivedName();

			if (mWOLSession->IsCurrentUser(name)) {
				LoginProfile* profile = LoginProfile::Get(name, false);

				if (profile) {
					WWDEBUG_SAY(("Updating team ladder info for %S\n", profile->GetName()));
					const WOL::Ladder& wolLadder = ladderEvent.GetWOLLadder();

					// Save the number of games played.
					profile->SetGamesPlayed(wolLadder.reserved2);

					// Save the ranking information
					LoginProfile::Ranking rank;
					rank.Wins = wolLadder.wins;
					rank.Losses = wolLadder.losses;
					rank.Deaths = wolLadder.reserved1;
					rank.Kills = wolLadder.kills;
					rank.Points = wolLadder.points;
					rank.Rank = wolLadder.rung;
					profile->SetRanking(LadderType_Team, rank);

					profile->SaveSettings();
					profile->Release_Ref();
				}
			}
		} else if (LadderType_Clan == type) {
			RefPtr<UserData> user = mWOLSession->GetCurrentUser();

			if (user.IsValid()) {
				RefPtr<SquadData> squad = user->GetSquad();

				if (squad.IsValid() && (stricmp(squad->GetAbbr(), ladderEvent.GetReceivedName()) == 0)) {
					LoginProfile* profile = LoginProfile::Get(user->GetName(), false);

					if (profile) {
						WWDEBUG_SAY(("Updating clan ladder info for %S\n", profile->GetName()));
						const WOL::Ladder& wolLadder = ladderEvent.GetWOLLadder();

						// Save the ranking information
						LoginProfile::Ranking rank;
						rank.Wins = wolLadder.wins;
						rank.Losses = wolLadder.losses;
						rank.Deaths = wolLadder.reserved1;
						rank.Kills = wolLadder.kills;
						rank.Points = wolLadder.points;
						rank.Rank = wolLadder.rung;
						profile->SetRanking(LadderType_Clan, rank);

						profile->SaveSettings();
						profile->Release_Ref();
					}
				}
			}
		}
	}
}


/***********************************************************************************************
 * WolGameModeClass::HandleNotification -- Handle connection status notifications              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Connection status                                                                 *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/8/2001 10:20PM ST : Created                                                            *
 *=============================================================================================*/
void WolGameModeClass::HandleNotification(ConnectionStatus &status)
{
	if (mMonitorConnection && status == ConnectionDisconnected) {
		Handle_Disconnect();
	}
}


/***********************************************************************************************
 * WolGameModeClass::Handle_Disconnect -- Handle disconnect from chat server                   *
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
 *   11/8/2001 10:21PM ST : Created                                                            *
 *=============================================================================================*/
void WolGameModeClass::Handle_Disconnect(void)
{
	if (mMonitorConnection && mConnected) {
		mConnected = false;

		if (cNetwork::I_Am_Server()) {

			if (The_Game() && The_Game()->IsDedicated.Is_True() && AutoRestart.Get_Restart_Flag()) {

				/*
				** If the game has a time limit and players then we will wait until the next map change otherwise we will stop now.
				*/
				WWASSERT(PTheGameData != NULL);
				//if (!The_Game()->Get_Time_Limit_Minutes() || SlaveMaster.Am_I_Slave()) {
					if (cPlayerManager::Count() == 0) {
						if (SlaveMaster.Am_I_Slave()) {
							AutoRestart.Set_Restart_Flag(false);
							Set_Exit_On_Exception(true);
         				cGameData::Set_Manual_Exit(true);
						} else {
							Quit_And_Restart();
						}
					} else {
						/*
						** Tell the players we have to quit
						*/
						mStartQuitProcessTime = TIMEGETTIME();
						if (cNetwork::I_Am_Server()) {
							WideStringClass widestring(TRANSLATE (IDS_MENU_QUITTING_DUE_TO_CONN_LOSS_MSG), true);
							cScTextObj * message = new cScTextObj;
							message->Init(widestring, TEXT_MESSAGE_PUBLIC, true, HOST_TEXT_SENDER, -1);
						}
					}
				//} else {
					//Quit_And_Restart();
				//}
			}
		}
	}
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
void WolGameModeClass::HandleNotification(ServerError& /* server_error */)
{
/*
** This doesn't work because you can't get a server list without also doing a complete reset of wolapi.
*/
#if (0)
	HRESULT code = server_error.GetErrorCode();

	if (code == CHAT_E_MUSTPATCH) {
		if (mMonitorConnection && mConnected) {
			mPatchAvailable = true;

			/*
			** If the game has a time limit and players then we will wait until the next map change otherwise we will stop now.
			*/
			WWASSERT(PTheGameData != NULL);

			if (!The_Game()->Get_Time_Limit_Minutes() || cPlayerManager::Count() == 0) {
				mStartQuitProcessTime = TIMEGETTIME();
			} else {
				Quit_And_Restart();
			}
		}
	}
#endif //(0)
}


/***********************************************************************************************
 * WolGameModeClass::Quit_And_Restart -- Setup a restart then quit the game                    *
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
 *   11/8/2001 10:21PM ST : Created                                                            *
 *=============================================================================================*/
void WolGameModeClass::Quit_And_Restart(void)
{
	static bool reenter_ye_not = false;

	if (!reenter_ye_not) {

		reenter_ye_not = true;
		/*
		** This will cause the server to leave the channel if needed.
		*/
		GameInitMgrClass::End_Game();

		/*
		** If there is a patch available then download it.
		*/
		if (mConnected) {
			WOLLogonMgr::Logoff();
			if (mPatchAvailable) {
				RefPtr<WWOnline::Session> wolSession = WWOnline::Session::GetInstance(false);
				DlgDownload::DoDialog(TRANSLATE(IDS_WOL_DOWNLOAD), wolSession->GetPatchDownloadList(), true);
			}
		} else {
			/*
			** If we lost connection then drop out of the game and try to re-establish connection.
			*/
			AutoRestart.Set_Restart_Flag(true);
			Set_Exit_On_Exception(true);
			cGameData::Set_Manual_Exit(true);
			Stop_Main_Loop(EXIT_SUCCESS);
		}
		reenter_ye_not = false;
	}
}


/******************************************************************************
*
* NAME
*     WolGameModeClass::HandleNotification(WOLPagedEvent)
*
* DESCRIPTION
*     Handle the display of pages and invitations from users outside the game.
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void WolGameModeClass::HandleNotification(WOLPagedEvent& event)
{
	PageMessage* page = event.Subject();
	WWASSERT(page && "NULL page in WOLPagedEvent");

	switch (event.GetAction()) {
		case PAGE_RECEIVED: {
			WideStringClass message(255, true);
			message.Format(U_CHAR("%s: %s"), page->GetPagersName(), page->GetPageMessage());
			CombatManager::Get_Message_Window()->Add_Message(message, COLOR_PAGED_TEXT);
			WWAudioClass::Get_Instance()->Create_Instant_Sound("Private_Message", Matrix3D(1));
			break;
		}

		case INVITATION_RECEIVED:
		case INVITATION_DECLINED:
			CombatManager::Get_Message_Window()->Add_Message(page->GetPageMessage(), COLOR_INVITE_TEXT);
			WWAudioClass::Get_Instance()->Create_Instant_Sound("Private_Message", Matrix3D(1));
			break;

		case PAGE_ERROR:
		case PAGE_NOT_THERE:
		case PAGE_TURNED_OFF: {
			WideStringClass message(255, true);
			message.Format(U_CHAR("%s %s"), TRANSLATE(IDS_WOL_PAGEUSERERROR), page->GetPageMessage());
			CombatManager::Get_Message_Window()->Add_Message(page->GetPageMessage(), COLOR_CONSOLE_TEXT);
			break;
		}
	}
}



void WolGameModeClass::Game_Start_Timeout_Callback(void)
{
	GameModeClass* game = GameModeManager::Find("WOL");
	if (game && game->Is_Active()) {
		WolGameModeClass* wolgame = reinterpret_cast<WolGameModeClass*>(game);
		wolgame->Game_Start_Timed_Out();
	}
}			  
		
		
void WolGameModeClass::Game_Start_Timed_Out(void)
{
	/*
	** If we lost connection without getting a report from WW Online then the only clue we have is that we will fail to
	** get the game ID by timing out. Since this is pretty much fatal anyway, let's quit and restart and see if things fix 
	** themselves up.
	** 
	** ST - 8/21/2002 11:27AM
	*/
	if (cNetwork::I_Am_Server()) {
		if (The_Game() && The_Game()->IsDedicated.Is_True() && AutoRestart.Get_Restart_Flag()) {
			mConnected = false;
			if (SlaveMaster.Am_I_Slave()) {
				AutoRestart.Set_Restart_Flag(false);
				Set_Exit_On_Exception(true);
         	cGameData::Set_Manual_Exit(true);
			} else {
				Quit_And_Restart();
			}
		}
	}
	
	
	
}			  
