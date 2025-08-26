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
 *                     $Archive:: /Commando/Code/Commando/wolgmode.h                          $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 10/13/02 10:05p                                             $*
 *                                                                                             *
 *                    $Revision:: 36                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSV_VER)
#pragma once
#endif

#ifndef __WOLGMODE_H__
#define __WOLGMODE_H__

#include "gamemode.h"
#include "WOLBuddyMgr.h"
#include <WWOnline/RefPtr.h>
#include <wwlib/Notify.h>
#include <wwlib/Signaler.h>

class cGameData;
class cPlayer;
class WOLChatMgr;
class WOLQuickMatch;
class DlgWOLWaitEvent;

namespace WWOnline
{
class Session;
class ChannelData;
class UserData;
class GameStartEvent;
class GameOptionsMessage;
class ChannelEvent;
class UserEvent;
class LadderInfoEvent;
enum ConnectionStatus;
class ServerError;
}


// Game Mode to do WWOnline (Internet) games
class	WolGameModeClass :
		public GameModeClass,
		public Signaler<WolGameModeClass>,
		public Observer<DlgWOLWaitEvent>,
		public Observer<WWOnline::GameStartEvent>,
		public Observer<WWOnline::GameOptionsMessage>,
		public Observer<WWOnline::ChannelEvent>,
		public Observer<WWOnline::UserEvent>,
		public Observer<WWOnline::LadderInfoEvent>,
		public Observer<WOLPagedEvent>,
		public Observer<WWOnline::ConnectionStatus>,
		public Observer<WWOnline::ServerError>
	{
	public:
		WolGameModeClass();
		~WolGameModeClass();

		inline const char* Name(void)
			{return "WOL";}

		void Init(void);
		void Shutdown(void);
		void Think(void);
		void Render(void) {}

		void Create_Game(cGameData*);
		void Leave_Game(void);

		void Start_Game(cGameData*);
		void End_Game(void);

		void Accept_Actions(void);
		void Refusal_Actions(void);

		void Init_WOL_Player(cPlayer* player);

		RefPtr<WWOnline::UserData> Get_WOL_User_Data(const wchar_t* name);

		void Page_WOL_User(const wchar_t* name, const wchar_t* msg);
		void Reply_Last_Page(const wchar_t* msg);

		void Locate_WOL_User(const wchar_t* name);
		void Invite_WOL_User(const wchar_t* name, const wchar_t* msg);
		void Join_WOL_User(const wchar_t* name);

		bool Kick_Player(const wchar_t* name);
		void Ban_Player(const wchar_t* name, unsigned long ip);
		bool Is_Banned(const char *player_name, unsigned long ip);
		void Read_Kick_List(void);
		void Auto_Kick(void);

		void System_Timer_Reset(void);

		inline cGameData* Get_Game(void) const
			{return mTheGame;}

		inline bool Channel_Create_OK(void)
			{return mChannelCreateSuccessFlag;}

		void Set_Quiet_Mode(bool quiet)
			{mQuietMode = quiet;}

		bool Post_Game_Check(void);
		static void Game_Start_Timeout_Callback(void);

	protected:
		void Evaluate_Clans(cGameData* theGame);
		void Update_Channel_Settings(cGameData* theGame, const RefPtr<WWOnline::ChannelData>& channel);
		void Quit_And_Restart(void);
		void Handle_Disconnect(void);

		void Game_Start_Timed_Out(void);

		void HandleNotification(DlgWOLWaitEvent&);
		void HandleNotification(WWOnline::GameStartEvent&);
		void HandleNotification(WWOnline::GameOptionsMessage&);
		void HandleNotification(WWOnline::ChannelEvent&);
		void HandleNotification(WWOnline::UserEvent&);
		void HandleNotification(WWOnline::LadderInfoEvent&);
		void HandleNotification(WOLPagedEvent&);
		void HandleNotification(WWOnline::ServerError&);
		void HandleNotification(WWOnline::ConnectionStatus&);

	private:
		RefPtr<WWOnline::Session> mWOLSession;
		WOLChatMgr* mWOLChatMgr;
		WOLBuddyMgr* mWOLBuddyMgr;
		WOLQuickMatch* mQuickMatch;

		cGameData* mTheGame;
		unsigned long mGameID;
		bool mGameInProgress;
		unsigned long mSendServerInfoTime;
		bool mQuietMode;
		bool mConnected;
		unsigned long mLastPatchCheckTime;
		unsigned long mStartQuitProcessTime;
		bool mPatchAvailable;
		bool mMonitorConnection;

		// This flag is set to true when we successfully create a game channel.
		// Used in conjunction with signalling to indicate completion status.
		bool mChannelCreateSuccessFlag;

		DynamicVectorClass<StringClass> KickNameList;
		DynamicVectorClass<unsigned long> KickIPList;


		DynamicVectorClass<StringClass> IdleKickNameList;
		DynamicVectorClass<unsigned int> IdleKickTimeList;

	};

#endif	// __WOLGMODE_H__