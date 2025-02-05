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
 *                     $Archive:: /Commando/Code/Commando/dlgmpwolignorelist.cpp   $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/30/01 12:06p                                             $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "dlgmpwolignorelist.h"
#include "dlgmpwoladdignoreentry.h"
#include "listctrl.h"
#include "dlgmpwoldeleteignoreentry.h"


////////////////////////////////////////////////////////////////
//
//	MPWolIgnoreListPopupClass
//
////////////////////////////////////////////////////////////////
MPWolIgnoreListPopupClass::MPWolIgnoreListPopupClass (void)	:
	PopupDialogClass (IDD_MP_WOL_IGNORE_LIST)
{
	WWDEBUG_SAY(("MPWolIgnoreListPopupClass: Instantiated\n"));
	mBuddyMgr = WOLBuddyMgr::GetInstance(false);
	WWASSERT_PRINT(mBuddyMgr, "WOLBuddyMgr not instantiated!");
}


MPWolIgnoreListPopupClass::~MPWolIgnoreListPopupClass()
{
	WWDEBUG_SAY(("MPWolIgnoreListPopupClass: Destroyed\n"));

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
MPWolIgnoreListPopupClass::On_Init_Dialog (void)
{
	Observer<WOLBuddyMgrEvent>::NotifyMe(*mBuddyMgr);

	//
	//	Get a pointer to the list control
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_BUDDY_LIST_CTRL);

	if (list_ctrl != NULL) {
		//	Configure the columns
		list_ctrl->Add_Column(L"", 1.0F, Vector3 (1, 1, 1));

		//	Populate the list
		Refresh_List ();
	}

	PopupDialogClass::On_Init_Dialog ();
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
MPWolIgnoreListPopupClass::On_Command (int ctrl_id, int message_id, DWORD param)
{
	switch (ctrl_id) {
		case IDC_ADD_BUTTON:
			MPWolAddIgnoreEntry::DoDialog(NULL);
			break;

		case IDC_REMOVE_BUTTON: {
			//	Get the name of the selected user
			WideStringClass name(0, true);
			Get_Selected_Entry(name);
			
			//	Display a confirmation dialog to the user.
			if (name.Is_Empty () == false) {
				MPWolDeleteIgnoreEntryPopupClass* dialog = new MPWolDeleteIgnoreEntryPopupClass;
				dialog->Set_User_Name(name);
				dialog->Start_Dialog();
				REF_PTR_RELEASE(dialog);
			}

			break;
		}
	}

	PopupDialogClass::On_Command (ctrl_id, message_id, param);
}


////////////////////////////////////////////////////////////////
//
//	Refresh_List
//
////////////////////////////////////////////////////////////////
void
MPWolIgnoreListPopupClass::Refresh_List (void)
{
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_BUDDY_LIST_CTRL);

	if (list_ctrl == NULL) {
		return ;
	}

	//	Get the name of the currently selected entry
	WideStringClass selected_user(0, true);
	int curr_sel = list_ctrl->Get_Curr_Sel();

	if (curr_sel != -1) {
		selected_user = list_ctrl->Get_Entry_Text (curr_sel, 0);
	}
	
	//	Start fresh
	list_ctrl->Delete_All_Entries ();
	bool is_selection_set = false;

	//	Loop over the entries
	const WOLBuddyMgr::IgnoreList& ignore = mBuddyMgr->GetIngoreList();
	unsigned int index;
	for (index = 0; index < ignore.size(); index++) {
		const WideStringClass& name = ignore[index];

		int pos = list_ctrl->Insert_Entry(0xFFFF, name);

		if (!is_selection_set && selected_user.Compare_No_Case(name) == 0) {
			list_ctrl->Set_Curr_Sel(pos);
			is_selection_set = true;
		}
	}

	if (is_selection_set == false && index > 0) {
		list_ctrl->Set_Curr_Sel(0);
	}
}


////////////////////////////////////////////////////////////////
//
//	Get_Selected_Entry
//
////////////////////////////////////////////////////////////////
void MPWolIgnoreListPopupClass::Get_Selected_Entry(WideStringClass& user_name)
{
	//	Get the index of the currently selected user in the list control
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_BUDDY_LIST_CTRL);

	if (list_ctrl) {
		int curr_sel = list_ctrl->Get_Curr_Sel ();

		if (curr_sel != -1) {
			//	Return the user name to the caller
			user_name = list_ctrl->Get_Entry_Text(curr_sel, 0);
		}
	}
}


void MPWolIgnoreListPopupClass::HandleNotification(WOLBuddyMgrEvent& event)
{
	if (event.GetAction() == IGNORELIST_CHANGED) {
		Refresh_List();
	}
}
