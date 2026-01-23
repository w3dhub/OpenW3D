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
 *                     $Archive:: /Commando/Code/Commando/dlgmpwolpagebuddy.cpp      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/12/02 9:35p                                               $*
 *                                                                                             *
 *                    $Revision:: 15                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "dlgmpwolpagebuddy.h"
#include "renegadedialog.h"
#include "dlgmpwolbuddies.h"
#include "dlgmpwolpagereply.h"
#include "dlgmpwolbuddylistpopup.h"
#include "renegadedialogmgr.h"
//#include "WOLBuddyMgr.h"
#include "comboboxctrl.h"
#include <limits>


////////////////////////////////////////////////////////////////
//
//	MPWolPageBuddyPopupClass
//
////////////////////////////////////////////////////////////////
MPWolPageBuddyPopupClass::MPWolPageBuddyPopupClass (void)	:
	PopupDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MP_WOL_PAGE_BUDDY)),
	mBuddyMgr(NULL)
{
	mBuddyMgr = WOLBuddyMgr::GetInstance(false);
	WWASSERT(mBuddyMgr != NULL);
}


MPWolPageBuddyPopupClass::~MPWolPageBuddyPopupClass(void)
{
	if (mBuddyMgr) {
		mBuddyMgr->Release_Ref();
	}
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
MPWolPageBuddyPopupClass::On_Init_Dialog(void)
{
	Enable_Dlg_Item(IDC_INVITE_BUDDY_BUTTON, false);

	if (mBuddyMgr) {
		// Enable the invite button if we are in a situation to invite users.
		bool canInvite = mBuddyMgr->CanInviteUsers();
		Enable_Dlg_Item(IDC_INVITE_BUDDY_BUTTON, canInvite);

		// Get the current buddy list
		const WWOnline::UserList& list = mBuddyMgr->GetBuddyList();
		const size_t count = list.size();

		if (count == 0) {
			Observer<WOLBuddyMgrEvent>::NotifyMe (*mBuddyMgr);
			mBuddyMgr->RefreshBuddyList();
		} else {
			// Configure the combobox
			ComboBoxCtrlClass* combo_box = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_BUDDY_NAME_COMBO);

			if (combo_box) {
			
				// Add each buddy to the combobox
				for (size_t index = 0; index < count; ++index) {
					const RefPtr<WWOnline::UserData>& user = list[index];

					// Add this buddy if they are currently online
					if (user->GetLocation() != WWOnline::USERLOCATION_OFFLINE) {
						combo_box->Add_String(user->GetName());
					}
				}
			}
		}
	}

	// The page button is disable until the user enters a message.
	Enable_Dlg_Item(IDC_PAGE_BUTTON, false);

	// Set message edit to focus
	DialogControlClass* ctrl = Get_Dlg_Item(IDC_MESSAGE_EDIT);

	if (ctrl) {
		ctrl->Set_Focus();
	}

	PopupDialogClass::On_Init_Dialog();
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
MPWolPageBuddyPopupClass::On_Command(int ctrl_id, int message_id, unsigned int param)
{
	switch (ctrl_id) {
		case IDC_INVITE_BUDDY_BUTTON: {
			//	Get the name of the user we'll be inviting
			WideStringClass user_name = Get_Dlg_Item_Text(IDC_BUDDY_NAME_COMBO);

			if (user_name.Is_Empty() == false) {
				//	Invite the user
				if (mBuddyMgr) {
					mBuddyMgr->InviteUser(user_name, Get_Dlg_Item_Text(IDC_MESSAGE_EDIT));
				}

				End_Dialog();
			}

			break;
		}

		case IDC_BUDDY_LIST_BUTTON: {
			MPWolBuddyListPopupClass* dialog = new MPWolBuddyListPopupClass;
			dialog->Set_Observer(this);
			dialog->Start_Dialog();
			REF_PTR_RELEASE(dialog);
			break;
		}

		case IDC_VIEW_BUDDY_LIST_BUTTON:
			MPWolBuddiesMenuClass::Display();
			End_Dialog();
			break;
		
		case IDC_PAGE_BUTTON:
			Send_Page();
			break;
	}

	PopupDialogClass::On_Command(ctrl_id, message_id, param);
}


////////////////////////////////////////////////////////////////
//
//	Send_Page
//
////////////////////////////////////////////////////////////////
void
MPWolPageBuddyPopupClass::Send_Page(void)
{
	// Get the message to send
	WideStringClass message(0, true);
	message = Get_Dlg_Item_Text(IDC_MESSAGE_EDIT);
	message.Trim();

	if (message.Is_Empty() == false) {

		//	Get the name of the user we'll be paging
		const unichar_t* username = Get_Dlg_Item_Text(IDC_BUDDY_NAME_COMBO);

		if (u_strlen(username) > 0) {

			//	Send the page
			if (mBuddyMgr) {
				mBuddyMgr->PageUser(username, message);
			}

		End_Dialog();
		}
	}
}


////////////////////////////////////////////////////////////////
//
//	Set_Buddy_Name
//
////////////////////////////////////////////////////////////////
void MPWolPageBuddyPopupClass::Set_Buddy_Name(const unichar_t *user_name)
{
	Set_Dlg_Item_Text(IDC_BUDDY_NAME_COMBO, user_name);
}


void MPWolPageBuddyPopupClass::CheckIfCanSendPage(void)
{
	// Get the length of the message after leading and trailing whitespace
	// has been removed.
	WideStringClass message(0, true);
	message = Get_Dlg_Item_Text(IDC_MESSAGE_EDIT);
	message.Trim();

	// Check for a buddy name
	WideStringClass username(0, true);
	username = Get_Dlg_Item_Text(IDC_BUDDY_NAME_COMBO);
	username.Trim();

	// If there is a message and user then allow page to be sent.
	bool canSend = ((message.Get_Length() > 0) && (username.Get_Length() > 0));
	Enable_Dlg_Item(IDC_PAGE_BUTTON, canSend);
}


void MPWolPageBuddyPopupClass::On_ComboBoxCtrl_Edit_Change(ComboBoxCtrlClass* /* combo */, int id)
{
	if (IDC_BUDDY_NAME_COMBO == id) { 
		CheckIfCanSendPage();
	}
}


void MPWolPageBuddyPopupClass::On_EditCtrl_Change(EditCtrlClass* /* edit */, int id)
{
	if (IDC_MESSAGE_EDIT == id) { 
		CheckIfCanSendPage();
	}
}


void MPWolPageBuddyPopupClass::On_EditCtrl_Enter_Pressed(EditCtrlClass* /* edit */, int id)
{
	if (IDC_MESSAGE_EDIT == id && Is_Dlg_Item_Enabled(IDC_PAGE_BUTTON)) {
		Send_Page();
	}
}


void MPWolPageBuddyPopupClass::HandleNotification(WOLBuddyMgrEvent &event)
{
	WOLBuddyMgrAction action = event.GetAction();

	if ((BUDDYLIST_CHANGED == action) || (BUDDYINFO_CHANGED == action)) {

		// Configure the combobox
		ComboBoxCtrlClass* combo_box = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_BUDDY_NAME_COMBO);

		if (combo_box) {
			combo_box->Reset_Content();
		
		const WWOnline::UserList& buddies = mBuddyMgr->GetBuddyList();
		const size_t count = buddies.size();

		// Add each buddy to the combobox
		for (size_t index = 0; index < count; ++index) {
				const RefPtr<WWOnline::UserData>& user = buddies[index];

				if (user->GetLocation() != WWOnline::USERLOCATION_OFFLINE) {
					combo_box->Add_String(user->GetName());
				}
			}
		}
	}
}
