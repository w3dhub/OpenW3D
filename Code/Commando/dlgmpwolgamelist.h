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
 *                     $Archive:: /Commando/Code/Commando/dlgmpwolgamelist.h       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/10/02 1:41p                                               $*
 *                                                                                             *
 *                    $Revision:: 21                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLG_MP_WOL_GAME_LIST_H
#define __DLG_MP_WOL_GAME_LIST_H

#include "menudialog.h"
#include "WOLGameInfo.h"
#include <WWOnline/WOLSession.h>
#include <WWOnline/PingProfile.h>
#include <wwlib/Notify.h>
#include <wwlib/Signaler.h>

class DlgPasswordPrompt;

////////////////////////////////////////////////////////////////
//
//	MPWolGameListMenuClass
//
////////////////////////////////////////////////////////////////
class MPWolGameListMenuClass :
		public MenuDialogClass,
		public Observer<WWOnline::ChannelListEvent>,
		public Observer<WWOnline::ServerError>,
		protected Observer<WWOnline::SquadEvent>,
		protected Signaler<DlgPasswordPrompt>
	{
	public:
		static void DoDialog(void);

	protected:
		MPWolGameListMenuClass(void);
		~MPWolGameListMenuClass(void);

		// Prevent copy and assignment
		MPWolGameListMenuClass(const MPWolGameListMenuClass&);
		const MPWolGameListMenuClass& operator=(const MPWolGameListMenuClass&);
	
		void On_Init_Dialog(void) override;
		void On_Command(int id, int mesage_id, unsigned int param) override;
		void On_Last_Menu_Ending(void) override;
		bool On_Key_Down(uint32 key_id, uint32 key_data) override;

		void Join_Game(void);
		void ReceiveSignal(DlgPasswordPrompt&) override;

		void RequestGameList(void);
		void SortGameChannels(int column, bool isAscending, unsigned int param);

		void On_ListCtrl_Column_Click(ListCtrlClass* list, int id, int column) override;
		void On_ListCtrl_DblClk(ListCtrlClass* list, int id, int index) override;
		void On_ListCtrl_Delete_Entry(ListCtrlClass* list, int id, int index) override;
		void On_ListCtrl_Sel_Change(ListCtrlClass* list, int id, int oldIndex, int newIndex) override;

		void GetClanVSClanString(WOLGameInfo& gameInfo, WideStringClass& text);

		void HandleNotification(WWOnline::ChannelListEvent&) override;
		void UpdateChannels(ListCtrlClass* list, const WWOnline::ChannelList& chanList);

		void HandleNotification(WWOnline::ServerError&) override;
		void HandleNotification(WWOnline::SquadEvent&) override;

	protected:
		static MPWolGameListMenuClass* _mInstance;

		RefPtr<WWOnline::Session> mWOLSession;
		RefPtr<WWOnline::ChannelData> mGameToJoin;

		bool mChannelListPending;

		int mSortColumn;
		bool mIsSortAscending;
		unsigned int mSortFlags;

		WOLGameInfo mSelectedGame;

		PingProfile mPingProfile;
	};

#endif //__DLG_MP_WOL_GAME_LIST_H
