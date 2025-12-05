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
 *                     $Archive:: /Commando/Code/Commando/AutoStart.h                         $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 8/13/02 4:57p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once
#ifndef _AUTOSTART_H
#define _AUTOSTART_H

#include "WOLLogonMgr.h"
#include <wwlib/Signaler.h>
#include "wolgmode.h"
#include "menudialog.h"
#include <WWOnline/RefPtr.h>
#include <WWOnline/WOLSession.h>

namespace WOL {
	class WOLSession;
	class ServerError;
}

/*
**
** This class handles restarting a server after a crash or system reboot.
**
**
*/
class AutoRestartClass : public Observer<WOLLogonAction>, public Observer<WWOnline::ServerError>, protected Signaler<WolGameModeClass>
{

	public:
		/*
		** Constructor, destructor.
		*/
		AutoRestartClass(void);

		/*
		** Misc public functions.
		*/
		void Restart_Game(void);
		void Think(void);
		bool Is_Active(void) {return((bool)(RestartState != STATE_DONE));}
		void Set_Restart_Flag(bool enable);
		bool Get_Restart_Flag(void);
		void Cancel(void);


		/*
		** Callbacks.
		*/
		void HandleNotification(WOLLogonAction&) override;
		void HandleNotification(WWOnline::ServerError& server_error) override;
		void ReceiveSignal(WolGameModeClass&) override;

		/*
		** Enum of steps to go through to restart a game.
		*/
		typedef enum {
			STATE_FIRST,
			STATE_GAME_MODE_WAIT,
			STATE_LOGIN,
			STATE_CREATE_GAME,
			STATE_CREATE_CHANNEL,
			STATE_WAIT_CHANNEL_CREATE,
			STATE_WAIT_CHANNEL_CREATE_RETRY,
			STATE_START_GAME,
			STATE_CANCELLED,
			STATE_DONE,
		} RestartStateType;

		static const char *REG_VALUE_AUTO_RESTART_FLAG;
		static const char *REG_VALUE_AUTO_RESTART_TYPE;


	private:

		/*
		** WOL Login state.
		*/
		WOLLogonAction	LogonAction;

		/*
		** Restart state.
		*/
		RestartStateType RestartState;

		/*
		** Cancel request flag.
		*/
		bool CancelRequest;

		/*
		** Game mode. 0 = LAN, 1 = internet.
		*/
		int GameMode;

		/*
		** Reference pointer to WOLSession.
		*/
		RefPtr<WWOnline::Session> WOLSession;

		/*
		** Time we last tried to create the game channel.
		*/
		unsigned int LastChannelCreateTime;

		/*
		** Number of times we tried to create the channel.
		*/
		int NumChannelCreateTries;

};


extern AutoRestartClass AutoRestart;






#endif //_AUTOSTART_H
