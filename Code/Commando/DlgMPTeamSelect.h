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

/******************************************************************************
*
* FILE
*     $Archive: /Commando/Code/Commando/DlgMPTeamSelect.h $
*
* DESCRIPTION
*     Multiplayer team selection dialog.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 8 $
*     $Modtime: 2/11/02 11:28a $
*
******************************************************************************/

#ifndef __DLGMPTEAMSELECT_H__
#define __DLGMPTEAMSELECT_H__

#include "playermanager.h"
#include <wwui/menudialog.h>
#include <wwlib/Notify.h>
#include <wwlib/Signaler.h>
#include <WWOnline/RefPtr.h>
#include "WOLGameInfo.h"

namespace WWOnline
{
class Session;
class ChannelEvent;
class UserEvent;
class GameOptionsMessage;
};

class cPlayer;

typedef TypedEventPair<bool, int> MPChooseTeamSignal;

class DlgMPTeamSelect :
		public MenuDialogClass,
		protected Signaler<MPChooseTeamSignal>,
		protected Observer<WWOnline::ChannelEvent>,
		protected Observer<WWOnline::UserEvent>,
		protected Observer<WWOnline::GameOptionsMessage>,
		protected Observer<PlayerMgrEvent>
	{
	public:
		static void DoDialog(Signaler<MPChooseTeamSignal>& target);

	protected:
		DlgMPTeamSelect(void);
		~DlgMPTeamSelect();

		bool FinalizeCreate(void);

		void On_Init_Dialog(void);
		void On_Frame_Update(void);
		void On_Command(int ctrlID, int message, DWORD param);
		void On_Last_Menu_Ending(void);

		void InitSideChoice(int sidePref);
		void SelectSideChoice(int side);
		int GetSideChoice(void);

		void RequestWOLGameInfo(void);

		void ShowTimeRemaining(float remainingSecond);
		bool FindPlayerInListCtrl(const WCHAR* name, ListCtrlClass*& outList, int& outIndex);

		void HandleNotification(WWOnline::ChannelEvent&);
		void HandleNotification(WWOnline::UserEvent&);
		void HandleNotification(WWOnline::GameOptionsMessage&);
		void HandleNotification(PlayerMgrEvent&);

		static void ProcessWOLGameInfo(DlgMPTeamSelect& dialog, const char* data);
		static void ProcessWOLTeamInfo(DlgMPTeamSelect& dialog, const char* data);
		static void ProcessWOLPlayerInfo(DlgMPTeamSelect& dialog, const char* data);

		void PopulateWithLANPlayers(void);
		void AddLANPlayerInfo(cPlayer* lanPlayer);
		void RemoveLANPlayerInfo(cPlayer* lanPlayer);

	protected:
		bool mWOLGame;
		bool mCanChoose;
		float mTimeRemaining;

		RefPtr<WWOnline::Session> mWOLSession;
		WOLGameInfo mGameInfo;
	};

#endif // __DLGMPTEAMSELECT_H__
