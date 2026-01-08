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
 *                     $Archive:: /Commando/Code/Commando/dlgmpwolchat.h        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/25/02 2:21p                                               $*
 *                                                                                             *
 *                    $Revision:: 15                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLG_MP_WOL_CHAT_H
#define __DLG_MP_WOL_CHAT_H

#include "menudialog.h"
#include "WOLChatMgr.h"

namespace WWOnline
{
class UserEvent;
class ChannelData;
}

////////////////////////////////////////////////////////////////
//
//	MPWolChatMenuClass
//
////////////////////////////////////////////////////////////////
class MPWolChatMenuClass :
	public MenuDialogClass,
	public Observer<WOLChatMgrEvent>,
	public Observer<WWOnline::UserEvent>
{
public:
	static void DoDialog(const RefPtr<WWOnline::ChannelData>& channel = NULL);

	//
	//	Debug methods
	//
	void		Refresh_Dialog (void);

protected:
	MPWolChatMenuClass (void);
	~MPWolChatMenuClass (void);

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void		On_Init_Dialog(void) override;
	void		On_Destroy(void) override;
	void		On_Frame_Update(void) override;
	void		On_Last_Menu_Ending(void) override;

	void		Refresh_Lobby_List(void);
	void		SelectLobbyFromChannel(const RefPtr<WWOnline::ChannelData>& channel);
	void		UpdateLobbyUserCount(ListCtrlClass* list, int listIndex);
	void		UpdateCurrentLobbyUserCount(void);
	void		Update_Current_Channel(void);

	void		Send_Message(bool is_emot);
	void		Add_Message(const unichar_t* text);
	void		Refresh_Message_List(void);
	void		Update_Message_Color(void);

	void		Add_Users(void);
	void		Remove_Users(void);
	void		Toggle_Squelch(void);
	void		Refresh_User_List(void);
	void		Update_User_Status(ListCtrlClass* list, int index, const RefPtr<WWOnline::UserData>& user);

	void		Set_Focus_To_Chat_Edit_Ctrl(void);

	void		On_Command(int ctrl_id, int mesage_id, unsigned int param) override;
	void		On_ListCtrl_Sel_Change(ListCtrlClass *list_ctrl, int ctrl_id, int old_index, int new_index) override;
	void		On_ListCtrl_Mouse_Over(ListCtrlClass *list_ctrl, int ctrl_id, int index) override;
	void		On_EditCtrl_Enter_Pressed (EditCtrlClass *edit_ctrl, int ctrl_id) override;

	void HandleNotification(WOLChatMgrEvent&) override;
	void HandleNotification(WWOnline::UserEvent&) override;

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	static MPWolChatMenuClass *_TheInstance;

	WOLChatMgr* mChatMgr;

	bool mLobbyListChanged;
	bool mLobbyChanged;
	bool mUserInListChanged;
	bool mUserOutListChanged;
	bool mMessageListChanged;
};

#endif //__DLG_MP_WOL_CHAT_H
