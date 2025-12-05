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
 *                     $Archive:: /Commando/Code/Commando/dlgmpwolbuddylistpopup.cpp   $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/01/01 7:35p                                              $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "dlgmpwolbuddylistpopup.h"
#include "renegadedialog.h"
#include "dlgmpwolpagebuddy.h"
#include "WOLBuddyMgr.h"
#include "listctrl.h"
#include <limits>


////////////////////////////////////////////////////////////////
//
//	MPWolBuddyListPopupClass
//
////////////////////////////////////////////////////////////////
MPWolBuddyListPopupClass::MPWolBuddyListPopupClass (void)	:
	Observer (NULL),
	PopupDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MP_WOL_BUDDY_LIST_POPUP))
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
MPWolBuddyListPopupClass::On_Init_Dialog (void)
{	
	//
	//	Configure the list ctrl
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_BUDDY_LIST_CTRL);
	if (list_ctrl != NULL) {
		list_ctrl->Add_Column (L"", 1.0F, Vector3 (1, 1, 1));		

		//
		//	Loop over all the buddies
		//
		WOLBuddyMgr* buddyMgr = WOLBuddyMgr::GetInstance(false);

		if (buddyMgr) {
			const WWOnline::UserList& list = buddyMgr->GetBuddyList();
			const size_t count = list.size();
			WWASSERT(count <= static_cast<size_t>(std::numeric_limits<int>::max()));
			
			for (size_t index = 0; index < count; ++index) {
				const RefPtr<WWOnline::UserData>& user = list[index];

				if (user->GetLocation() != WWOnline::USERLOCATION_OFFLINE) {
					list_ctrl->Insert_Entry(static_cast<int>(index), user->GetName());
				}
			}

			buddyMgr->Release_Ref();
		}
	}

	PopupDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
MPWolBuddyListPopupClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDC_SELECT_BUTTON:
			On_Select ();
			break;
	}

	PopupDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_ListCtrl_DblClk
//
////////////////////////////////////////////////////////////////
void
MPWolBuddyListPopupClass::On_ListCtrl_DblClk
(
	ListCtrlClass *list_ctrl,
	int				ctrl_id,
	int				item_index
)
{
	On_Select ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Select
//
////////////////////////////////////////////////////////////////
void
MPWolBuddyListPopupClass::On_Select (void)
{
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_BUDDY_LIST_CTRL);
	if (list_ctrl == NULL) {
		return ;
	}

	//
	//	Get the currently selected entry from the list control
	//	
	int curr_sel = list_ctrl->Get_Curr_Sel ();
	if (curr_sel != -1) {

		//
		//	Record the name of the entry
		//
		SelectedUserName = list_ctrl->Get_Entry_Text (curr_sel, 0);
		
		//
		//	Notify the observer (if necessary)
		//
		if (Observer != NULL) {
			Observer->Set_Buddy_Name(SelectedUserName);
		}
						
		End_Dialog ();
	}
	
	return ;
}
