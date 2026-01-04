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
 *                     $Archive:: /Commando/Code/Commando/dlgmpwolinvitation.cpp     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/07/01 7:09p                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgmpwolinvitation.h"
#include "renegadedialog.h"
#include "renegadedialogmgr.h"
#include "dlgmpwolpagebuddy.h"
#include "translatedb.h"
#include "string_ids.h"
#include "WOLBuddyMgr.h"

////////////////////////////////////////////////////////////////
//
//	MPWolInvitationPopupClass
//
////////////////////////////////////////////////////////////////
MPWolInvitationPopupClass::MPWolInvitationPopupClass(const RefPtr<WWOnline::UserData>& host, const unichar_t* invite) :
		PopupDialogClass(GetRenegadeDialog(RenegadeDialogID::IDD_MP_WOL_INVITATION_REPLY)),
		mHost(host),
		mInviteMessage(invite)
{
	WWDEBUG_SAY(("MPWolInvitationPopupClass: Instantiated\n"));
	mBuddyMgr = WOLBuddyMgr::GetInstance(false);
	WWASSERT_PRINT(mBuddyMgr, "WOLBuddyMgr not instantiated!");
}


MPWolInvitationPopupClass::~MPWolInvitationPopupClass()
{
	WWDEBUG_SAY(("MPWolInvitationPopupClass: Destroyed\n"));

	if (mBuddyMgr) {
		mBuddyMgr->Release_Ref();
	}
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void MPWolInvitationPopupClass::On_Init_Dialog(void)
{
	Set_Dlg_Item_Text(IDC_INVITATION_TEXT, mInviteMessage);
	PopupDialogClass::On_Init_Dialog();
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void MPWolInvitationPopupClass::On_Command(int ctrl_id, int message_id, unsigned int param)
{
	switch (ctrl_id) {
		// Join the user
		case IDC_JOIN_BUTTON:
			mBuddyMgr->JoinUser(mHost);
			End_Dialog();
			break;

		// Decline the invitation
		case IDC_DECLINE_BUTTON:
			mBuddyMgr->DeclineInvitation(mHost->GetName());
			End_Dialog();
			break;

		// Start a conversation with the user
		case IDC_PAGE_BUTTON: {
			// Show the dialog
			MPWolPageBuddyPopupClass* dialog = new MPWolPageBuddyPopupClass;

			if (dialog) {
				dialog->Start_Dialog();
				dialog->Set_Buddy_Name(mHost->GetName());
				dialog->Release_Ref();
			}

			// Close this dialog
			End_Dialog();
			break;
		}
	}

	PopupDialogClass::On_Command(ctrl_id, message_id, param);
}
