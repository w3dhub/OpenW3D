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
 *                     $Archive:: /Commando/Code/commando/dlgmpwoladdbuddy.cpp      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/14/01 5:40p                                              $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgmpwoladdbuddy.h"
#include "renegadedialog.h"
#include "WOLBuddyMgr.h"
#include "WOLLogonMgr.h"
#include "DlgMessageBox.h"
#include <wwui/editctrl.h>
#include <wwdebug/wwdebug.h>
#include "translatedb.h"
#include "string_ids.h"


////////////////////////////////////////////////////////////////
//
//	MPWolAddBuddyPopupClass
//
////////////////////////////////////////////////////////////////
MPWolAddBuddyPopupClass::MPWolAddBuddyPopupClass (void)	:
	PopupDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MP_WOL_ADD_BUDDY))
{
	WWDEBUG_SAY(("MPWolAddBuddyPopupClass: Instantiated\n"));
	mBuddyMgr = WOLBuddyMgr::GetInstance(false);
	WWASSERT_PRINT(mBuddyMgr, "WOLBuddyMgr not instantiated!");
	return ;
}


MPWolAddBuddyPopupClass::~MPWolAddBuddyPopupClass()
{
	WWDEBUG_SAY(("MPWolAddBuddyPopupClass: Destroyed\n"));

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
MPWolAddBuddyPopupClass::On_Init_Dialog (void)
{
	EditCtrlClass* edit = (EditCtrlClass*)Get_Dlg_Item(IDC_BUDDY_NAME_EDIT);

	if (edit) {
		// WOL logins have a 9 character maximum.
		edit->Set_Text_Limit(9);
		edit->Set_Text(DefaultUserName);
	}

	// Disable the add button if the default user is empty.
	if (DefaultUserName.Is_Empty()) {
		Enable_Dlg_Item(IDC_ADD_BUTTON, false);
	}

	PopupDialogClass::On_Init_Dialog ();
	return ;
}


void MPWolAddBuddyPopupClass::On_Activate (bool onoff)
{
	PopupDialogClass::On_Activate(onoff);

	if (onoff == true) {
		EditCtrlClass* edit = (EditCtrlClass*)Get_Dlg_Item(IDC_BUDDY_NAME_EDIT);

		if (edit) {
			edit->Set_Focus();
		}
	}
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
MPWolAddBuddyPopupClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDC_ADD_BUTTON:
			Add_Buddy();
			break;
	}

	PopupDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Add_Buddy
//
////////////////////////////////////////////////////////////////
void
MPWolAddBuddyPopupClass::Add_Buddy (void)
{
	WideStringClass buddy_name(0, true);
	buddy_name = Get_Dlg_Item_Text(IDC_BUDDY_NAME_EDIT);
	buddy_name.Trim();

	if (buddy_name.Is_Empty() == false) {
		WideStringClass loginName(0, true);
		WOLLogonMgr::GetLoginName(loginName);

		if (buddy_name.Compare_No_Case(loginName) != 0) {
			mBuddyMgr->AddBuddy(buddy_name);
			End_Dialog();
		} else {
			Set_Dlg_Item_Text(IDC_BUDDY_NAME_EDIT, L"");
			Enable_Dlg_Item(IDC_ADD_BUTTON, false);

			DlgMsgBox::DoDialog(TRANSLATE (IDS_MENU_ERROR), TRANSLATE (IDS_MENU_CANT_ADD_AS_BUDDY_MESSAGE));
		}
	}

	return ;
}


void MPWolAddBuddyPopupClass::On_EditCtrl_Change(EditCtrlClass* edit, int id)
{
	if (IDC_BUDDY_NAME_EDIT == id) {
		// Do not allow leading or trailing whitespace
		WideStringClass text(0, true);
		text = edit->Get_Text();
		text.Trim();
		edit->Set_Text(text);

		bool enable (edit->Get_Text_Length() > 0);
		Enable_Dlg_Item(IDC_ADD_BUTTON, enable);
	}
}


void MPWolAddBuddyPopupClass::On_EditCtrl_Enter_Pressed(EditCtrlClass* edit, int id)
{
	if (IDC_BUDDY_NAME_EDIT == id) {
		Add_Buddy();
	}
}

