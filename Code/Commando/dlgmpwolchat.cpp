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
 *                     $Archive:: /Commando/Code/Commando/dlgmpwolchat.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/25/02 5:02p                                               $*
 *                                                                                             *
 *                    $Revision:: 38                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgmpwolchat.h"
#include "renegadedialog.h"
#include "renegadedialogmgr.h"
#include "wollocalemgr.h"
#include "DlgMessageBox.h"
#include <wolapi/chatdefs.h>
#include <WWOnline/WOLChannel.h>
#include <wwui/stylemgr.h>
#include <wwui/dialogmgr.h>
#include <wwui/editctrl.h>
#include <wwui/listctrl.h>
#include <wwui/menuentryctrl.h>
#include <wwui/shortcutbarctrl.h>
#include <ww3d2/render2d.h>
#include "string_ids.h"
#include <wwtranslatedb/translatedb.h>
#include <limits>

using namespace WWOnline;

////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
static const Vector3	PRIVATE_COLOR		(0.35F, 0.8F, 1.0F);
static const Vector3	EMOT_COLOR			(0.1F, 0.75F, 0.0F);
static const Vector3	SQUELCHED_COLOR	(0.5F, 0.5F, 0.5F);
static const Vector3	SYSOP_COLOR			(1.0F, 1.0F, 1.0F);

////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
MPWolChatMenuClass* MPWolChatMenuClass::_TheInstance = NULL;


////////////////////////////////////////////////////////////////
//
//	MPWolChatMenuClass
//
////////////////////////////////////////////////////////////////
MPWolChatMenuClass::MPWolChatMenuClass (void)	:
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MP_WOL_CHAT)),
	mChatMgr(NULL),
	mLobbyListChanged(false),
	mLobbyChanged(false),
	mUserInListChanged(false),
	mUserOutListChanged(false),
	mMessageListChanged(false)
{
	WWDEBUG_SAY(("MPWolChatMenuClass instantiated\n"));
	WWASSERT (_TheInstance == NULL);
	_TheInstance = this;
}


////////////////////////////////////////////////////////////////
//
//	~MPWolChatMenuClass
//
////////////////////////////////////////////////////////////////
MPWolChatMenuClass::~MPWolChatMenuClass (void)
{
	WWDEBUG_SAY(("MPWolChatMenuClass destroyed\n"));
	_TheInstance = NULL;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void MPWolChatMenuClass::On_Init_Dialog(void)
{
	//	Configure the shortcut bar
	ShortcutBarCtrlClass* bar = (ShortcutBarCtrlClass*)Get_Dlg_Item(IDC_SHORTCUT_BAR);

	if (bar) {
		bar->Add_Button(IDC_MP_SHORTCUT_GAMELIST, TRANSLATE(IDS_MENU_TEXT365));
		bar->Add_Button(IDC_MP_SHORTCUT_BUDDIES, TRANSLATE(IDS_MP_SHORTCUT_BUDDIES));
		bar->Add_Button(IDC_MP_SHORTCUT_PAGE_BUDDY, TRANSLATE(IDS_MP_SHORTCUT_PAGE_BUDDY));
		bar->Add_Button(IDC_MP_SHORTCUT_INTERNET_OPTIONS, TRANSLATE(IDS_INTERNET_OPTIONS));
		bar->Add_Button(IDC_MP_SHORTCUT_NEWS, TRANSLATE(IDS_MP_SHORTCUT_NEWS));
		bar->Add_Button(IDC_MP_SHORTCUT_CLANS, TRANSLATE(IDS_MP_SHORTCUT_CLANS));
		bar->Add_Button(IDC_MP_SHORTCUT_RANKINGS, TRANSLATE(IDS_MP_SHORTCUT_RANKINGS));
		bar->Add_Button(IDC_MP_SHORTCUT_NET_STATUS, TRANSLATE(IDS_MP_SHORTCUT_NET_STATUS));
	}

	//	Configure the lobby list ctrl
	ListCtrlClass* list_ctrl = (ListCtrlClass*)Get_Dlg_Item(IDC_LOBBY_LIST_CTRL);

	if (list_ctrl) {
		list_ctrl->Set_Wants_Focus(false);

		// Lobby name column
		list_ctrl->Add_Column(L"", 0.8F, Vector3 (1, 1, 1));

		// Lobby user count column
		list_ctrl->Add_Column(L"", 0.2F, Vector3 (1, 1, 1));
	}

	//	Configure the user list ctrl
	list_ctrl = (ListCtrlClass*)Get_Dlg_Item(IDC_PLAYERS_LIST_CTRL);

	if (list_ctrl) {
		list_ctrl->Allow_Multiple_Selection(true);
		list_ctrl->Allow_NoSelection(true);
		list_ctrl->Set_Wants_Focus(false);

		// User flags column
		list_ctrl->Add_Column(L"", .15F, Vector3 (1, 1, 1));

		// Username column
		list_ctrl->Add_Column(L"", .50F, Vector3 (1, 1, 1));

		// User clan column
		list_ctrl->Add_Column(L"", .35F, Vector3 (1, 1, 1));
	}

	//	Configure the message list ctrl
	list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_MESSAGE_LIST_CTRL);

	if (list_ctrl) {
		list_ctrl->Add_Column (L"", 1.0F, Vector3 (1, 1, 1));
		list_ctrl->Allow_Selection(false);
		list_ctrl->Set_Wants_Focus(false);
	}

	// Get Chat Manager
	mChatMgr = WOLChatMgr::GetInstance(true);

	if (mChatMgr) {
		mChatMgr->Start();
		Observer<WOLChatMgrEvent>::NotifyMe(*mChatMgr);
		mChatMgr->RefreshLobbyList();
	}

	RefPtr<Session> wolSession = Session::GetInstance(false);

	if (wolSession.IsValid()) {
		Observer<UserEvent>::NotifyMe(*wolSession);
	}

	MenuDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void MPWolChatMenuClass::On_Command(int ctrl_id, int message_id, unsigned int param)
{
	switch (ctrl_id) {
		case IDC_MENU_BACK_BUTTON:
			mChatMgr->LeaveLobby();
			break;

		case IDC_EMOT_MESSAGE_BUTTON:
			Send_Message(true);
			break;

		case IDC_SQUELCH_BUTTON:
			Toggle_Squelch();
			break;

		case IDC_REFRESH_BUTTON:
			mChatMgr->RefreshLobbyList();
			break;
	}

	MenuDialogClass::On_Command (ctrl_id, message_id, param);

	Set_Focus_To_Chat_Edit_Ctrl();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Destroy
//
////////////////////////////////////////////////////////////////
void MPWolChatMenuClass::On_Destroy(void)
{
	Observer<WOLChatMgrEvent>::StopObserving();

	if (mChatMgr) {
		mChatMgr->Stop();
		mChatMgr->Release_Ref();
		mChatMgr = NULL;
	}

	MenuDialogClass::On_Destroy ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void MPWolChatMenuClass::On_Frame_Update(void)
{
	MenuDialogClass::On_Frame_Update();

	//	Do we need to update the lobby list?
	if (mLobbyListChanged) {
		Refresh_Lobby_List();
	}

	//	Do we need to change the text of the current lobby control?
	if (mLobbyChanged) {
		Update_Current_Channel();
	}

	//	Do we need to add users to the player list?
	if (mUserInListChanged) {
		Add_Users();
	}

	//	Do we need to remove users from the player list?
	if (mUserOutListChanged) {
		Remove_Users();
	}
	
	//	Do we need to update the message list?
	if (mMessageListChanged) {
		Refresh_Message_List();
	}
}


void MPWolChatMenuClass::On_Last_Menu_Ending(void)
{
	// If this is the last menu the bring up the internet game main menu.
	RenegadeDialogMgrClass::Goto_Location(RenegadeDialogMgrClass::LOC_INTERNET_MAIN);
}


////////////////////////////////////////////////////////////////
//
//	Refresh_Lobby_List
//
////////////////////////////////////////////////////////////////
void MPWolChatMenuClass::Refresh_Lobby_List(void)
{
	// Remember that we've gotten this data
	mLobbyListChanged = false;

	ListCtrlClass* list = (ListCtrlClass*)Get_Dlg_Item(IDC_LOBBY_LIST_CTRL);

	if (list) {
		// Remove existing lobby names
		list->Delete_All_Entries();

		// Add the new lobbies
		const LobbyList& lobbyList = mChatMgr->GetLobbyList();

		if (lobbyList.empty()) {
			DlgMsgBox::DoDialog(TRANSLATE(IDS_MENU_SERVER_MESSAGE_TITLE), TRANSLATE(IDS_MENU_NO_CHAT_AVAIL));
			End_Dialog();
			return;
		}
		
		LobbyList::const_iterator iter = lobbyList.begin();

		while (iter != lobbyList.end()) {
			const RefPtr<ChannelData>& lobby = (*iter);

			WideStringClass lobbyName(64, true);
			mChatMgr->GetLobbyDisplayName(lobby, lobbyName);

			int listIndex = list->Insert_Entry(list->Get_Entry_Count(), lobbyName);

			if (listIndex >= -1) {
				// Associate the channel with this lobby entry
				const ChannelData* channel = lobby.ReferencedObject();
				list->Set_Entry_Data(listIndex, 0, (uintptr_t)channel);

				UpdateLobbyUserCount(list, listIndex);
			}

			iter++;
		}

		// Select either the current lobby we are joined to or the first
		// lobby in the list.
		RefPtr<ChannelData> lobby = mChatMgr->GetCurrentLobby();

		if (!lobby.IsValid() && (list->Get_Entry_Count() > 0)) {
			lobby = (ChannelData*)list->Get_Entry_Data(0, 0);
			mChatMgr->JoinLobby(lobby);
		}

		SelectLobbyFromChannel(lobby);
	}
}


void MPWolChatMenuClass::UpdateLobbyUserCount(ListCtrlClass* list, int listIndex)
{
	if (list && (listIndex >= 0)) {
		const ChannelData* channel = (const ChannelData*)list->Get_Entry_Data(listIndex, 0);

		if (channel) {
			WideStringClass countText(0, true);
			countText.Format(L"(%u)", channel->GetCurrentUsers());
			list->Set_Entry_Text(listIndex, 1, countText);
		}
	}
}


void MPWolChatMenuClass::UpdateCurrentLobbyUserCount(void)
{
	RefPtr<ChannelData> lobby = mChatMgr->GetCurrentLobby();

	if (lobby.IsValid()) {
		ListCtrlClass* list = (ListCtrlClass*)Get_Dlg_Item(IDC_LOBBY_LIST_CTRL);

		if (list) {
			const int count = list->Get_Entry_Count();

			for (int listIndex = 0; listIndex < count; ++listIndex) {
				const ChannelData* channel = (const ChannelData*)list->Get_Entry_Data(listIndex, 0);

				if (channel && (lobby.ReferencedObject() == channel)) {
					UpdateLobbyUserCount(list, listIndex);
					return;
				}
			}
		}
	}
}


void MPWolChatMenuClass::SelectLobbyFromChannel(const RefPtr<ChannelData>& channel)
{
	ListCtrlClass* list = (ListCtrlClass*)Get_Dlg_Item(IDC_LOBBY_LIST_CTRL);

	if (list) {
		if (channel.IsValid()) {
			WideStringClass displayName(0, true);
			mChatMgr->GetLobbyDisplayName(channel, displayName);

			int index = list->Find_Entry(0, displayName);

			if (index >= 0) {
				list->Set_Curr_Sel(index);
			}
		} else {
			list->Set_Curr_Sel(-1);
		}
	}
}


////////////////////////////////////////////////////////////////
//
//	Add_Users
//
////////////////////////////////////////////////////////////////
void MPWolChatMenuClass::Add_Users(void)
{
	ListCtrlClass* list = (ListCtrlClass*)Get_Dlg_Item(IDC_PLAYERS_LIST_CTRL);

	if (list) {
		// Loop over all the users
		const UserList& userList = mChatMgr->GetUserInList();
		const size_t count = userList.size();
		WWASSERT(count <= static_cast<size_t>(std::numeric_limits<int>::max()));

		for (size_t index = 0; index < count; ++index) {
			const RefPtr<UserData>& user = userList[index];

			// Add the user to the list control
			int itemIndex = list->Find_Entry(1, user->GetName());

			if (itemIndex == -1) {
				itemIndex = list->Insert_Entry(list->Get_Entry_Count(), L"");
			}

			if (itemIndex != -1) {
				list->Set_Entry_Text(itemIndex, 1, user->GetName());
				Update_User_Status(list, itemIndex, user);
			}
		}

		// Update the message color in the edit control
		Update_Message_Color();
	}

	UpdateCurrentLobbyUserCount();

	mChatMgr->ClearUserInList();
	mUserInListChanged = false;
}


////////////////////////////////////////////////////////////////
//
//	Remove_Users
//
////////////////////////////////////////////////////////////////
void MPWolChatMenuClass::Remove_Users(void)
{
	ListCtrlClass* list = (ListCtrlClass*)Get_Dlg_Item(IDC_PLAYERS_LIST_CTRL);

	if (list) {
		// Loop over all the users
		const UserList& userList = mChatMgr->GetUserOutList();
		const size_t count = userList.size();

		for (size_t index = 0; index < count; ++index) {
			const RefPtr<UserData>& user = userList[index];
			WWASSERT(user.IsValid() && "Invalid user in userlist from WOLChatMgr");

			if (user.IsValid()) {
				int sel = list->Find_Entry(1, user->GetName());

				if (sel != -1) {
					list->Delete_Entry(sel);
				}
			}
		}

		// Update the message color in the edit control
		Update_Message_Color();
	}

	UpdateCurrentLobbyUserCount();

	mChatMgr->ClearUserOutList();
	mUserOutListChanged = false;
}



////////////////////////////////////////////////////////////////
//
//	Refresh_Message_List
//
////////////////////////////////////////////////////////////////
void MPWolChatMenuClass::Refresh_Message_List(void)
{
	ListCtrlClass* list = (ListCtrlClass*)Get_Dlg_Item(IDC_MESSAGE_LIST_CTRL);

	if (list) {
		const ChatMessageList& messageList = mChatMgr->GetMessageList();

		//	Loop over all the messages
		const size_t count = messageList.size();
		WWASSERT(count <= static_cast<size_t>(std::numeric_limits<int>::max()));

		for (size_t index = 0; index < count; index ++) {
			const ChatMessage& message = messageList[index];
		
			// Build the string
			WideStringClass text(255, true);

			const WideStringClass& sender = message.GetSendersName();
			const wchar_t* msg = message.GetMessage();

			if (sender.Is_Empty() == false) {
				if (message.IsAction()) {
					text.Format(L"%s %s", (const wchar_t*)sender, msg);
				} else {
					text.Format(L"%s: %s", (const wchar_t*)sender, msg);
				}

				msg = text;
			}

			//	Add the message to the list control
			int entryCount = list->Get_Entry_Count();
			WWASSERT(index <= static_cast<size_t>(std::numeric_limits<int>::max() - entryCount));
			int itemIndex = list->Insert_Entry(entryCount + static_cast<int>(index), msg);

			if (itemIndex != -1) {
				//	Now, color the message as necessary
				if (message.IsPrivate()) {
					list->Set_Entry_Color(itemIndex, 0, PRIVATE_COLOR);
				} else if (message.IsAction()) {
					list->Set_Entry_Color(itemIndex, 0, EMOT_COLOR);
				} else if (message.IsSenderChannelOwner()) {
					list->Set_Entry_Color(itemIndex, 0, SYSOP_COLOR);
				}
			}

			//	Start deleting text after 1000 lines of messages
			if (entryCount > 1000) {
				list->Delete_Entry(0);
			}
		}

		//	Force-scroll the control to the end of the list
		if (list->Has_Focus() == false) {
			list->Scroll_To_End();
		}
	}

	mChatMgr->ClearMessageList();
	mMessageListChanged = false;
}


////////////////////////////////////////////////////////////////
//
//	Add_Message
//
////////////////////////////////////////////////////////////////
void MPWolChatMenuClass::Add_Message(const wchar_t* text)
{
	ListCtrlClass* list = (ListCtrlClass*)Get_Dlg_Item(IDC_MESSAGE_LIST_CTRL);

	if (list) {
		//	Add the message to the list control
		int entryCount = list->Get_Entry_Count();
		list->Insert_Entry(entryCount, text);

		//	Start deleting text after 1000 lines of messages
		if (entryCount > 1000) {
			list->Delete_Entry(0);
		}

		//	Force-scroll the control to the end of the list
		if (list->Has_Focus() == false) {
			list->Scroll_To_End();
		}
	}
}


////////////////////////////////////////////////////////////////
//
//	Set_Focus_To_Chat_Edit_Ctrl
//
////////////////////////////////////////////////////////////////
void MPWolChatMenuClass::Set_Focus_To_Chat_Edit_Ctrl(void)
{
	// Return focus to the chat edit control.
	DialogControlClass* ctrl = Get_Dlg_Item(IDC_CHAT_EDIT);

	if (ctrl) {
		ctrl->Set_Focus();
	}
}


////////////////////////////////////////////////////////////////
//
//	On_ListCtrl_Sel_Change
//
////////////////////////////////////////////////////////////////
void MPWolChatMenuClass::On_ListCtrl_Sel_Change(ListCtrlClass* list, int id, int oldSel, int newSel)
{
	if (IDC_LOBBY_LIST_CTRL == id) {
		if (newSel >= 0) {
			// Join the lobby associated with the entry
			const RefPtr<ChannelData> lobby = (ChannelData*)list->Get_Entry_Data(newSel, 0);

			if (lobby.IsValid()) {
				mChatMgr->JoinLobby(lobby);
			}
		}
	} else if (IDC_PLAYERS_LIST_CTRL == id) {
		Update_Message_Color();
	}
}


////////////////////////////////////////////////////////////////
//
//	On_ListCtrl_Mouse_Over
//
////////////////////////////////////////////////////////////////
void MPWolChatMenuClass::On_ListCtrl_Mouse_Over(ListCtrlClass* list, int id, int index)
{
	if (IDC_PLAYERS_LIST_CTRL == id) {
		WideStringClass userinfo(0, true);

		if (index >= 0) {
			const wchar_t* name = list->Get_Entry_Text(index, 1);
			userinfo += name;

			const RefPtr<UserData> user =	mChatMgr->FindUser(name);

			if (user.IsValid()) {
				RefPtr<SquadData> clan = user->GetSquad();

				if (clan.IsValid())
					{
					userinfo += L" : ";
					userinfo += clan->GetName();
					}

				userinfo += L" : ";
				userinfo += WolLocaleMgrClass::Get_Locale_String(user->GetLocale());
			}
		}

		Set_Dlg_Item_Text(IDC_USERINFO, userinfo);
	}
}


////////////////////////////////////////////////////////////////
//
//	On_EditCtrl_Enter_Pressed
//
////////////////////////////////////////////////////////////////
void MPWolChatMenuClass::On_EditCtrl_Enter_Pressed(EditCtrlClass* edit_ctrl, int ctrl_id)
{
	if (IDC_CHAT_EDIT == ctrl_id) {
		Send_Message(false);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Message_Color
//
////////////////////////////////////////////////////////////////
void MPWolChatMenuClass::Update_Message_Color(void)
{
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_PLAYERS_LIST_CTRL);
	EditCtrlClass *edit_ctrl = (EditCtrlClass *)Get_Dlg_Item (IDC_CHAT_EDIT);

	if (list_ctrl == NULL || edit_ctrl == NULL) {
		return ;
	}

	// Reset the color as necessary depending on the selection
	// state of the user list
	if (list_ctrl->Get_First_Selected () == -1) {
		//	Set the default color
		Vector3 default_color;
		INT32_TO_VRGB (StyleMgrClass::Get_Text_Color (), default_color);
		edit_ctrl->Set_Text_Color (default_color);
	} else {
		//	Set the private message color
		edit_ctrl->Set_Text_Color (PRIVATE_COLOR);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Send_Message
//
////////////////////////////////////////////////////////////////
void MPWolChatMenuClass::Send_Message(bool is_emot)
{
	// Get the message text
	WideStringClass message(0, true);
	message = Get_Dlg_Item_Text(IDC_CHAT_EDIT);
	message.Trim();

	if (message.Is_Empty()) {
		return;
	}

	// If the user has players selected then send the message as private.
	ListCtrlClass* list_ctrl = (ListCtrlClass*)Get_Dlg_Item(IDC_PLAYERS_LIST_CTRL);

	if (list_ctrl && (list_ctrl->Get_First_Selected() >= 0)) {
		UserList users;
		int index = list_ctrl->Get_First_Selected();
		
		while (index != -1) {
			const wchar_t* name = list_ctrl->Get_Entry_Text(index, 1);
			const RefPtr<UserData> user = mChatMgr->FindUser(name);

			if (user.IsValid()) {
				users.push_back(user);
			}

			index = list_ctrl->Get_Next_Selected(index);
		}

		mChatMgr->SendPrivateMessage(users, message, is_emot);
	} else {
		mChatMgr->SendPublicMessage(message, is_emot);
	}

	// Clear the edit control
	Set_Dlg_Item_Text(IDC_CHAT_EDIT, L"");
}



////////////////////////////////////////////////////////////////
//
//	Toggle_Squelch
//
////////////////////////////////////////////////////////////////
void MPWolChatMenuClass::Toggle_Squelch(void)
{
	ListCtrlClass* list = (ListCtrlClass*)Get_Dlg_Item(IDC_PLAYERS_LIST_CTRL);

	if (list) {
		//	Loop over the selected users
		int index = list->Get_First_Selected();

		while (index != -1) {
			const wchar_t* userName = list->Get_Entry_Text(index, 1);
			const RefPtr<UserData>& user = mChatMgr->FindUser(userName);

			// If the user is valid and it is not a channel owner and it is
			// not the current user then allow squelching.
			if (user.IsValid() && !user->IsChannelOwner() && !user->IsMe()) {
				// Toggle the users squelch
				bool squelched = user->IsSquelched();
				mChatMgr->SquelchUser(user, !squelched);
				Update_User_Status(list, index, user);
			}

			index = list->Get_Next_Selected(index);
		}
	}
}


////////////////////////////////////////////////////////////////
//
//	Update_User_Status
//
////////////////////////////////////////////////////////////////
void MPWolChatMenuClass::Update_User_Status(ListCtrlClass* list, int index, const RefPtr<UserData>& user)
{
	WWASSERT(list);
	WWASSERT(user.IsValid());

	list->Reset_Icons(index, 0);

	// Update the entry based on the user's flags
	if (user->IsSquelched()) {
		list->Set_Entry_Color(index, 1, SQUELCHED_COLOR);
	}

	if (user->IsChannelOwner()) {
		list->Set_Entry_Color(index, 1, SYSOP_COLOR);
		list->Add_Icon(index, 0, "mul_ccop4.tga");
	} else {
		//	Set the default color
		Vector3 default_color;
		INT32_TO_VRGB(StyleMgrClass::Get_Text_Color(), default_color);
		list->Set_Entry_Color(index, 1, default_color);
	}

	// Set there clan information
	RefPtr<SquadData> clan = user->GetSquad();

	if (clan.IsValid()) {
		WideStringClass clanAbbr(0, true);
		clanAbbr.Format(L"[%S]", clan->GetAbbr());
		list->Set_Entry_Text(index, 2, clanAbbr);
	} else {
		list->Set_Entry_Text(index, 2, L"");
	}
}


////////////////////////////////////////////////////////////////
//
//	Update_Current_Channel
//
////////////////////////////////////////////////////////////////
void MPWolChatMenuClass::Update_Current_Channel(void)
{
	Refresh_Lobby_List();

	ListCtrlClass* list_ctrl = (ListCtrlClass*)Get_Dlg_Item (IDC_PLAYERS_LIST_CTRL);

	if (list_ctrl == NULL) {
		return ;
	}

	//	Reset the player list
	list_ctrl->Delete_All_Entries ();

	//	Update the current lobby text
	mLobbyChanged = false;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Display
//
////////////////////////////////////////////////////////////////
void MPWolChatMenuClass::DoDialog(const RefPtr<ChannelData>& channel)
{
	//	Create the dialog if necessary, otherwise simply bring it to the front
	if (_TheInstance == NULL) {
		MPWolChatMenuClass* dialog = new MPWolChatMenuClass;
		
		if (dialog) {
			dialog->Start_Dialog();
			dialog->Release_Ref();
		}
	} else {
		if (_TheInstance->Is_Active_Menu() == false) {
			DialogMgrClass::Rollback(_TheInstance);
		}
	}

	if (channel.IsValid() && _TheInstance) {
		WOLChatMgr* chat = WOLChatMgr::GetInstance(false);

		if (chat) {
			chat->JoinLobby(channel);
			chat->Release_Ref();
		}

		_TheInstance->SelectLobbyFromChannel(channel);
	}
}


////////////////////////////////////////////////////////////////
//
//	HandleNotification(WOLChatMgrEvent)
//
////////////////////////////////////////////////////////////////
void MPWolChatMenuClass::HandleNotification(WOLChatMgrEvent& event)
{
	switch (event) {
		case LobbyListChanged:
			mLobbyListChanged = true;
			break;

		case LobbyChanged:
			mLobbyChanged = true;
			break;

		case UserInListChanged:
			mUserInListChanged = true;
			break;

		case UserOutListChanged:
			mUserOutListChanged = true;
			break;
		
		case MessageListChanged:
			mMessageListChanged = true;
			break;

		case KickedFromChannel:
			DlgMsgBox::DoDialog(TRANSLATE(IDS_MENU_SERVER_MESSAGE_TITLE), TRANSLATE(IDS_MENU_WOL_KICK_USER_MESSAGE));
			End_Dialog();
			break;

		case BannedFromChannel:
			DlgMsgBox::DoDialog(TRANSLATE(IDS_MENU_SERVER_MESSAGE_TITLE), TRANSLATE(IDS_MENU_WOL_BAN_USER_MESSAGE));
			End_Dialog();
			break;

		default:
			break;
	}
}


////////////////////////////////////////////////////////////////
//
//	HandleNotification(UserEvent)
//
////////////////////////////////////////////////////////////////
void MPWolChatMenuClass::HandleNotification(UserEvent& userEvent)
{
	UserEvent::Event event  = userEvent.GetEvent();

	if ((UserEvent::SquadInfo == event) || (UserEvent::NewData == event)) {
		ListCtrlClass* list = (ListCtrlClass*)Get_Dlg_Item(IDC_PLAYERS_LIST_CTRL);
		const RefPtr<UserData>& user = userEvent.Subject();

		if (list && user.IsValid()) {
			int index = list->Find_Entry(1, user->GetName());

			if (index != -1) {
				Update_User_Status(list, index, user);
			}
		}
	}
}
