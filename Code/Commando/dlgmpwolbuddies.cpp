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
 *                     $Archive:: /Commando/Code/Commando/dlgmpwolbuddies.cpp    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/15/02 3:35p                                               $*
 *                                                                                             *
 *                    $Revision:: 28                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "DlgMPWolBuddies.h"
#include "RenegadeDialogMgr.h"
#include "WOLBuddyMgr.h"
#include "DlgMPWolPageBuddy.h"
#include "DlgMPWolAddBuddy.h"
#include "DlgMPWoldeletebuddy.h"
#include "DlgMPWolIgnoreList.h"
#include "ww3d.h"
#include "translatedb.h"
#include "string_ids.h"
#include <WWUI\ListCtrl.h>
#include <WWUI\ComboBoxCtrl.h>
#include <WWUI\DialogMgr.h>
#include <WWUI\shortcutbarctrl.h>
#include <WWOnline\WOLUser.h>
#include <WWOnline\WOLSquad.h>


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
enum
{
	COL_NAME = 0,
	COL_CLAN,
	COL_LOCATION,
	COL_WINS,
	COL_DEATHS,
	COL_POINTS,
	COL_RANK
};


MPWolBuddiesMenuClass* MPWolBuddiesMenuClass::_mInstance = NULL;

void MPWolBuddiesMenuClass::Display(void)
{
	//	Create the dialog if necessary, otherwise simply bring it to the front
	if (_mInstance == NULL) {
		MPWolBuddiesMenuClass* dialog = new MPWolBuddiesMenuClass;
		
		if (dialog) {
			dialog->Start_Dialog();
			dialog->Release_Ref();
		}
	} else {
		if (_mInstance->Is_Active_Menu() == false) {
			DialogMgrClass::Rollback(_mInstance);
		}
	}
}


////////////////////////////////////////////////////////////////
//
//	MPWolBuddiesMenuClass
//
////////////////////////////////////////////////////////////////
MPWolBuddiesMenuClass::MPWolBuddiesMenuClass (void)	:
	MenuDialogClass(IDD_MP_WOL_BUDDIES),
	mBuddyListChanged(false),
	mBuddyInfoChanged(false)
{
	WWDEBUG_SAY(("MPWolBuddiesMenuClass: Instantiated\n"));
	WWASSERT(_mInstance == NULL);
	_mInstance = this;

	//	Get the buddy manager object
	mBuddyMgr = WOLBuddyMgr::GetInstance (true);
	WWASSERT_PRINT(mBuddyMgr, "WOLBuddyMgr failed to instantiate.");
}


////////////////////////////////////////////////////////////////
//
//	~MPWolBuddiesMenuClass
//
////////////////////////////////////////////////////////////////
MPWolBuddiesMenuClass::~MPWolBuddiesMenuClass (void)
{
	WWDEBUG_SAY(("MPWolBuddiesMenuClass: Destroyed\n"));
	REF_PTR_RELEASE (mBuddyMgr);
	_mInstance = NULL;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
MPWolBuddiesMenuClass::On_Init_Dialog (void)
{
	//	Configure the shortcut bar
	ShortcutBarCtrlClass *bar = (ShortcutBarCtrlClass *)Get_Dlg_Item (IDC_SHORTCUT_BAR);

	if (bar != NULL) {
		bar->Add_Button(IDC_MP_SHORTCUT_CHAT, TRANSLATE(IDS_MP_SHORTCUT_CHAT));
		bar->Add_Button(IDC_MP_SHORTCUT_GAMELIST, TRANSLATE(IDS_MENU_TEXT365));
		bar->Add_Button(IDC_MP_WOL_BUDDIES_IGNORE_BUTTON, TRANSLATE(IDS_MENU_TEXT378));
		bar->Add_Button(IDC_MP_SHORTCUT_INTERNET_OPTIONS, TRANSLATE(IDS_INTERNET_OPTIONS));

		#ifdef QUICKMATCH_OPTIONS
		bar->Add_Button(IDC_MP_SHORTCUT_QUICKMATCH_OPTIONS, TRANSLATE(IDS_MENU_TEXT364));
		#endif

		bar->Add_Button(IDC_MP_SHORTCUT_NEWS, TRANSLATE(IDS_MP_SHORTCUT_NEWS));
		bar->Add_Button(IDC_MP_SHORTCUT_CLANS, TRANSLATE(IDS_MP_SHORTCUT_CLANS));
		bar->Add_Button(IDC_MP_SHORTCUT_RANKINGS, TRANSLATE(IDS_MP_SHORTCUT_RANKINGS));
		bar->Add_Button(IDC_MP_SHORTCUT_NET_STATUS, TRANSLATE(IDS_MP_SHORTCUT_NET_STATUS));		
	}

	//	Get a pointer to the list control
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_BUDDY_LIST_CTRL);

	if (list_ctrl != NULL) {
		//	Configure the columns
		list_ctrl->Add_Column(TRANSLATE(IDS_MP_BUDDY_LIST_NAME), 0.2F, Vector3 (1, 1, 1));
		list_ctrl->Add_Column(TRANSLATE (IDS_MENU_CLAN), 0.1F, Vector3 (1, 1, 1));
		list_ctrl->Add_Column(TRANSLATE(IDS_MP_BUDDY_LIST_LOCATION), 0.25F, Vector3 (1, 1, 1));
		list_ctrl->Add_Column(TRANSLATE(IDS_BUDDY_COL_WINS), 0.1F, Vector3 (1, 1, 1));
		list_ctrl->Add_Column(TRANSLATE(IDS_BUDDY_COL_DEATHS_KILLS), 0.15F, Vector3 (1, 1, 1));
		list_ctrl->Add_Column(TRANSLATE(IDS_BUDDY_COL_POINTS), 0.1F, Vector3 (1, 1, 1));
		list_ctrl->Add_Column(TRANSLATE(IDS_BUDDY_COL_RANK), 0.1F, Vector3 (1, 1, 1));
		list_ctrl->Allow_NoSelection(true);
	}

	ComboBoxCtrlClass* rankCombo = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_RANKTYPE);

	if (rankCombo) {
		int index = rankCombo->Add_String(TRANSLATE (IDS_MENU_INDIVIDUAL));
		rankCombo->Set_Item_Data(index, WWOnline::LadderType_Team);

		index = rankCombo->Add_String(TRANSLATE (IDS_MENU_CLAN));
		rankCombo->Set_Item_Data(index, WWOnline::LadderType_Clan);

		rankCombo->Set_Curr_Sel(0);
	}

	// Disable join, page and delete by default.
	Enable_Dlg_Item(IDC_MP_WOL_BUDDIES_JOIN_BUTTON, false);
	Enable_Dlg_Item(IDC_MP_WOL_BUDDIES_PAGE_BUTTON, false);
	Enable_Dlg_Item(IDC_MP_WOL_BUDDIES_DELETE_BUTTON, false);

	// Make sure we know about buddy events
	Observer<WOLBuddyMgrEvent>::NotifyMe (*mBuddyMgr);
	mBuddyMgr->RefreshBuddyList();

	MenuDialogClass::On_Init_Dialog ();
}


void MPWolBuddiesMenuClass::On_Frame_Update(void)
{
	if (mBuddyListChanged) {
		Refresh_Buddy_List();
	}

	if (mBuddyInfoChanged) {
		Update_Buddy_List();
	}

	MenuDialogClass::On_Frame_Update();
}

////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
MPWolBuddiesMenuClass::On_Command (int ctrl_id, int message_id, DWORD param)
{
	switch (ctrl_id) {
		case IDC_MP_WOL_BUDDIES_PAGE_BUTTON:
			Page_Selected_User();
			break;

		case IDC_MP_WOL_BUDDIES_ADD_BUTTON:
			START_DIALOG(MPWolAddBuddyPopupClass);
			break;

		case IDC_MP_WOL_BUDDIES_REFRESH_BUTTON:
			mBuddyMgr->RefreshBuddyInfo();
			break;

		case IDC_MP_WOL_BUDDIES_DELETE_BUTTON: {
			// Get the name of the currently selected user so we can
			// pass it to the "Delete User" dialog
			WideStringClass buddy_name(0, true);
			Get_Selected_Buddy(buddy_name);

			//	Display the dialog
			if (buddy_name.Is_Empty() == false) {
				MPWolDeleteBuddyPopupClass *dialog = new MPWolDeleteBuddyPopupClass;

				if (dialog) {
					dialog->Set_User_Name(buddy_name);
					dialog->Start_Dialog();
					REF_PTR_RELEASE (dialog);
				}
			}
			break;
		}

		case IDC_MP_WOL_BUDDIES_IGNORE_BUTTON:
			START_DIALOG(MPWolIgnoreListPopupClass);
			break;

		case IDC_MP_WOL_BUDDIES_JOIN_BUTTON: {
			// Get the name of the currently selected user so we can
			// pass it to the "Delete User" dialog
			WideStringClass buddy_name(0, true);
			Get_Selected_Buddy(buddy_name);

			// Try to join them
			if (buddy_name.Is_Empty() == false) {
				const RefPtr<WWOnline::UserData> buddy = mBuddyMgr->FindBuddy(buddy_name);
				mBuddyMgr->JoinUser(buddy);
			}

			break;
		}
	}

	MenuDialogClass::On_Command(ctrl_id, message_id, param);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Buddy_Info
//
////////////////////////////////////////////////////////////////
void MPWolBuddiesMenuClass::Update_Buddy_Info(int index, const RefPtr<WWOnline::UserData>& user)
{
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_BUDDY_LIST_CTRL);

	if (list_ctrl == NULL) {
		return ;
	}

	// Set the name
	list_ctrl->Set_Entry_Text(index, COL_NAME, user->GetName());

	// Set the clan
	RefPtr<WWOnline::SquadData> clan = user->GetSquad();

	if (clan.IsValid()) {
		WideStringClass clanName(0, true);
		clanName = clan->GetAbbr();
		list_ctrl->Set_Entry_Text(index, COL_CLAN, clanName);
	} else {
		list_ctrl->Set_Entry_Text(index, COL_CLAN, L"");
	}

	//	Build a textual description of the user's location
	WideStringClass location_text(0, true);
	WOLBuddyMgr::GetLocationDescription(user, location_text);

	//	Update the location column for this entry
	list_ctrl->Set_Entry_Text(index, COL_LOCATION, location_text);

	//	Associate the user's location with this entry
	int location = user->GetLocation();
	list_ctrl->Set_Entry_Data(index, COL_LOCATION, location);
}


void MPWolBuddiesMenuClass::Update_Buddy_Ranking(int index, const RefPtr<WWOnline::UserData>& user)
{
	ListCtrlClass* list = (ListCtrlClass*)Get_Dlg_Item(IDC_BUDDY_LIST_CTRL);

	if (list) {
		WWOnline::LadderType ladderType = WWOnline::LadderType_Team;
		ComboBoxCtrlClass* rankCombo = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_RANKTYPE);

		if (rankCombo) {
			int sel = rankCombo->Get_Curr_Sel();
			ladderType = (WWOnline::LadderType)rankCombo->Get_Item_Data(sel);
		}

		RefPtr<WWOnline::LadderData> ladder = user->GetLadderFromType(ladderType);

		if (ladder.IsValid()) {
			wchar_t text[64];

			swprintf(text, L"%d", ladder->GetWins());
			list->Set_Entry_Text(index, COL_WINS, text);

			swprintf(text, L"%d / %d", ladder->GetReserved1(), ladder->GetKills());
			list->Set_Entry_Text(index, COL_DEATHS, text);

			swprintf(text, L"%d", ladder->GetPoints());
			list->Set_Entry_Text(index, COL_POINTS, text);

			swprintf(text, L"%d", ladder->GetRung());
			list->Set_Entry_Text(index, COL_RANK, text);
		} else {
			list->Set_Entry_Text(index, COL_WINS, L"-");
			list->Set_Entry_Text(index, COL_DEATHS, L"- / -");
			list->Set_Entry_Text(index, COL_POINTS, L"-");
			list->Set_Entry_Text(index, COL_RANK, L"-");
		}
	}
}


////////////////////////////////////////////////////////////////
//
//	Refresh_Buddy_List
//
////////////////////////////////////////////////////////////////
void	
MPWolBuddiesMenuClass::Refresh_Buddy_List (void)
{
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_BUDDY_LIST_CTRL);

	if (list_ctrl == NULL) {
		return ;
	}

	//	Get the name of the currently selected user
	WideStringClass selected_user(0, true);
	int curr_sel = list_ctrl->Get_Curr_Sel();

	if (curr_sel != -1) {
		selected_user = list_ctrl->Get_Entry_Text(curr_sel, COL_NAME);
	}

	//	Reset the list
	list_ctrl->Delete_All_Entries();

	//	Loop over all the buddies
	const WWOnline::UserList& buddies = mBuddyMgr->GetBuddyList();
	const unsigned int count = buddies.size();

	for (unsigned int index = 0; index < count; ++index) {
		//	Add the buddy to the list control
		int item_index = list_ctrl->Insert_Entry(list_ctrl->Get_Entry_Count() , L"");
		WWASSERT(item_index != -1);

		if (item_index != -1) {
			const RefPtr<WWOnline::UserData>& buddy = buddies[index];

			// Update this buddy's information
			Update_Buddy_Info(item_index, buddy);
			Update_Buddy_Ranking(item_index, buddy);

			//	Do we need to select this user?
			if (selected_user.Compare_No_Case(buddy->GetName()) == 0) {
				list_ctrl->Set_Curr_Sel(item_index);
				Enable_Dlg_Item(IDC_MP_WOL_BUDDIES_DELETE_BUTTON, true);

				int location = buddy->GetLocation();
				Adjust_Buttons_For_Buddy_Location(location);
			}
		}
	}

	mBuddyListChanged = false;
}


void MPWolBuddiesMenuClass::Update_Buddy_List(void)
{
	ListCtrlClass* list = (ListCtrlClass*)Get_Dlg_Item(IDC_BUDDY_LIST_CTRL);

	if (list) {
		int currSel = list->Get_Curr_Sel();

		//	Loop over all the buddies
		const WWOnline::UserList& buddies = mBuddyMgr->GetBuddyList();
		unsigned int count = buddies.size();

		for (unsigned int index = 0; index < count; ++index) {
			const RefPtr<WWOnline::UserData>& buddy = buddies[index];

			// Add the buddy to the list control
			int item_index = list->Find_Entry(COL_NAME, buddy->GetName());

			if (item_index >= 0) {
				// Update this buddy's information
				Update_Buddy_Info(item_index, buddy);
				Update_Buddy_Ranking(item_index, buddy);

				// Do we need to select this user?
				if (item_index == currSel) {
					Enable_Dlg_Item(IDC_MP_WOL_BUDDIES_DELETE_BUTTON, true);
					int location = buddy->GetLocation();
					Adjust_Buttons_For_Buddy_Location(location);
				}
			}
		}
	}

	mBuddyInfoChanged = false;
}


////////////////////////////////////////////////////////////////
//
//	Get_Selected_Buddy
//
////////////////////////////////////////////////////////////////
void MPWolBuddiesMenuClass::Get_Selected_Buddy(WideStringClass& buddy_name)
{
	//	Get the index of the currently selected user in the list control
	ListCtrlClass* list_ctrl = (ListCtrlClass*)Get_Dlg_Item(IDC_BUDDY_LIST_CTRL);

	if (list_ctrl) {
		int curr_sel = list_ctrl->Get_Curr_Sel();

		if (curr_sel != -1) {
			// Return the user name to the caller
			buddy_name = list_ctrl->Get_Entry_Text(curr_sel, COL_NAME);
		}
	}
}


void MPWolBuddiesMenuClass::Adjust_Buttons_For_Buddy_Location(int location)
{
	switch (location) {
		case WWOnline::USERLOCATION_NO_CHANNEL:
			// Allow paging if buddy is online
			Enable_Dlg_Item(IDC_MP_WOL_BUDDIES_PAGE_BUTTON, true);

			// Do not allow joining a buddy who is not in a channel.
			Enable_Dlg_Item(IDC_MP_WOL_BUDDIES_JOIN_BUTTON, false);
			break;

		case WWOnline::USERLOCATION_IN_CHANNEL:
			// Allow paging and joining of buddies in channels
			Enable_Dlg_Item(IDC_MP_WOL_BUDDIES_JOIN_BUTTON, true);
			Enable_Dlg_Item(IDC_MP_WOL_BUDDIES_PAGE_BUTTON, true);
			break;

		case WWOnline::USERLOCATION_OFFLINE:
		case WWOnline::USERLOCATION_HIDING:
			// Do not alllow joining or paging of buddies who are offline or hiding.
			Enable_Dlg_Item(IDC_MP_WOL_BUDDIES_JOIN_BUTTON, false);
			Enable_Dlg_Item(IDC_MP_WOL_BUDDIES_PAGE_BUTTON, false);
			break;

		default:
			break;
 	}
}


////////////////////////////////////////////////////////////////
//
//	HandleNotification
//
////////////////////////////////////////////////////////////////
void
MPWolBuddiesMenuClass::HandleNotification(WOLBuddyMgrEvent &event)
{
	WOLBuddyMgrAction action = event.GetAction();

	if (BUDDYLIST_CHANGED == action) {
		mBuddyListChanged = true;
	} else if (BUDDYINFO_CHANGED == action) {
		mBuddyInfoChanged = true;
	}
}


////////////////////////////////////////////////////////////////
//
//	On_ListCtrl_Sel_Change
//
////////////////////////////////////////////////////////////////
void
MPWolBuddiesMenuClass::On_ListCtrl_Sel_Change(ListCtrlClass* list, int id, int oldsel, int newsel)
{
	if (IDC_BUDDY_LIST_CTRL == id) {
		if (newsel != -1) {
			Enable_Dlg_Item(IDC_MP_WOL_BUDDIES_DELETE_BUTTON, true);

			// Adjust the buttons based upon the location of the selected buddy.
			int location = list->Get_Entry_Data(newsel, COL_LOCATION);
			Adjust_Buttons_For_Buddy_Location(location);
		} else {
			// If unselecting then disable delete button
			Enable_Dlg_Item(IDC_MP_WOL_BUDDIES_DELETE_BUTTON, false);
			Adjust_Buttons_For_Buddy_Location(WWOnline::USERLOCATION_OFFLINE);
		}
	}
}


////////////////////////////////////////////////////////////////
//
//	On_ListCtrl_DblClk
//
////////////////////////////////////////////////////////////////
void
MPWolBuddiesMenuClass::On_ListCtrl_DblClk(ListCtrlClass* list_ctrl, int ctrl_id, int item_index)
{
	if (IDC_BUDDY_LIST_CTRL == ctrl_id) {
		//	Get the index of the currently selected user in the list control
		int curr_sel = list_ctrl->Get_Curr_Sel();

		if (curr_sel != -1) {
			//	Get the location associated with this entry
			int location = list_ctrl->Get_Entry_Data(curr_sel, COL_LOCATION);

			switch (location) {
				// Simply page the selected user
				case WWOnline::USERLOCATION_NO_CHANNEL:
					Page_Selected_User();
					break;

				// Join our buddy in the channel he is in.
				case WWOnline::USERLOCATION_IN_CHANNEL: {
					// Get the name of the selected user
					WideStringClass buddyName(64, true);
					Get_Selected_Buddy(buddyName);

					mPendingJoin = mBuddyMgr->FindBuddy(buddyName);

					if (mPendingJoin.IsValid()) {
						// Ask the user if they want to join this buddy
						WideStringClass message(0, true);
						message.Format(TRANSLATE (IDS_MENU_JOIN_REQUEST_MESSAGE), (const WCHAR*)buddyName);
						DlgMsgBox::DoDialog(0, message, DlgMsgBox::YesNo, this);
					}
					break;
				}

				// Do nothing for buddies who are offline or hiding.
				case WWOnline::USERLOCATION_OFFLINE:
				case WWOnline::USERLOCATION_HIDING:
				default:
					break;
			}
		}
	}
}


void MPWolBuddiesMenuClass::On_ComboBoxCtrl_Sel_Change(ComboBoxCtrlClass* , int id, int , int newsel)
{
	if (id == IDC_RANKTYPE) {
		ListCtrlClass* list = (ListCtrlClass*)Get_Dlg_Item(IDC_BUDDY_LIST_CTRL);

		if (list) {
			int count = list->Get_Entry_Count();

			for (int index = 0; index < count; ++index) {
				const WCHAR* name = list->Get_Entry_Text(index, COL_NAME);
				RefPtr<WWOnline::UserData> buddy = mBuddyMgr->FindBuddy(name);

				if (buddy.IsValid()) {
					Update_Buddy_Ranking(index, buddy);
				}
			}
		}
	}
}


////////////////////////////////////////////////////////////////
//
//	HandleNotification
//
////////////////////////////////////////////////////////////////
void MPWolBuddiesMenuClass::HandleNotification(DlgMsgBoxEvent &event)
{
	if (event.Event() == DlgMsgBoxEvent::Yes) {
		// Try to join this user
		if (mPendingJoin.IsValid()) {
			mBuddyMgr->JoinUser(mPendingJoin);
		}
	}

	mPendingJoin.Release();
}


////////////////////////////////////////////////////////////////
//
//	Page_Selected_User
//
////////////////////////////////////////////////////////////////
void MPWolBuddiesMenuClass::Page_Selected_User(void)
{
	//	Get the name of the currently selected user so we can
	// pass it to the page dialog
	WideStringClass buddy_name(64, true);
	Get_Selected_Buddy(buddy_name);
	
	// Show the dialog
	MPWolPageBuddyPopupClass* dialog = new MPWolPageBuddyPopupClass;			
	WWASSERT(dialog && "Failed to create page buddy dialog");

	if (dialog) {
		dialog->Start_Dialog();
		dialog->Set_Buddy_Name(buddy_name);
		dialog->Release_Ref();
	}
}
