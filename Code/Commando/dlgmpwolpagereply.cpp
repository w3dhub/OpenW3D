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
 *                     $Archive:: /Commando/Code/Commando/dlgmpwolpagereply.cpp     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/26/02 4:31p                                               $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgmpwolpagereply.h"
#include "renegadedialog.h"
#include "dlgmpwoladdbuddy.h"
#include "dlgmpwoladdignoreentry.h"
#include "WOLLogonMgr.h"
#include "ConsoleMode.h"
#include <wwui/listctrl.h>

DlgWOLPageReply* DlgWOLPageReply::_mInstance = NULL;

/******************************************************************************
*
* NAME
*     DlgWOLPageReply::DoDialog
*
* DESCRIPTION
*     Start page reply dialog. There can only be one page reply dialog present
*     at a time therefore if a page reply dialog is already displayed it will
*     brought forward.
*
* INPUTS
*     NONE
*
* RESULT
*     True if dialog created successfully.
*
******************************************************************************/

bool DlgWOLPageReply::DoDialog(void)
	{
	DlgWOLPageReply* dialog = _mInstance;

	if (dialog == NULL)
		{
		dialog = new DlgWOLPageReply;

		if (dialog)
			{
			if (dialog->FinalizeCreate())
				{
				dialog->Start_Dialog();
				}

			dialog->Release_Ref();
			}
		}

	return (dialog != NULL);
	}


/******************************************************************************
*
* NAME
*     DlgWOLPageReply::IsOpen
*
* DESCRIPTION
*     Default constructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

bool DlgWOLPageReply::IsOpen(void)
	{
	return (NULL != _mInstance);
	}


/******************************************************************************
*
* NAME
*     DlgWOLPageReply::DlgWOLPageReply
*
* DESCRIPTION
*     Default constructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

DlgWOLPageReply::DlgWOLPageReply(void) :
		PopupDialogClass(GetRenegadeDialog(RenegadeDialogID::IDD_MP_WOL_PAGE_REPLY)),
		mBuddyMgr(NULL)
	{
	WWDEBUG_SAY(("DlgWOLPageReply Instantiated\n"));
	WWASSERT(_mInstance == NULL);
	_mInstance = this;
	}


/******************************************************************************
*
* NAME
*     DlgWOLPageReply::~DlgWOLPageReply
*
* DESCRIPTION
*     Destructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

DlgWOLPageReply::~DlgWOLPageReply()
	{
	WWDEBUG_SAY(("DlgWOLPageReply Destroyed\n"));

	if (mBuddyMgr)
		{
		mBuddyMgr->Release_Ref();
		}

	_mInstance = NULL;
	}


/******************************************************************************
*
* NAME
*     DlgWOLPageReply::FinalizeCreate
*
* DESCRIPTION
*     Post creation initialization.
*
* INPUTS
*     NONE
*
* RESULT
*     True if successful
*
******************************************************************************/

bool DlgWOLPageReply::FinalizeCreate(void)
	{
	mBuddyMgr = WOLBuddyMgr::GetInstance(false);
	return (mBuddyMgr != NULL);
	}


/******************************************************************************
*
* NAME
*     DlgWOLPageReply::On_Init_Dialog
*
* DESCRIPTION
*     One time dialog initialization
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLPageReply::On_Init_Dialog(void)
	{
	// Configure the list control that shows the page messages
	ListCtrlClass* list = (ListCtrlClass*)Get_Dlg_Item(IDC_MESSAGE_LIST);

	if (list)
		{
		list->Add_Column(L"", 1.0F, Vector3(1, 1, 1));
		list->Allow_Selection(false);
		}

	// Enable the invite button if we are in a situation to invite users.
	bool canInvite = mBuddyMgr->CanInviteUsers();
	Enable_Dlg_Item(IDC_INVITE_BUDDY_BUTTON, canInvite);

	Observer<WOLPagedEvent>::NotifyMe(*mBuddyMgr);
	PopupDialogClass::On_Init_Dialog();
	}


/******************************************************************************
*
* NAME
*     DlgWOLPageReply::On_Command
*
* DESCRIPTION
*     Handle commands from dialog controls
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLPageReply::On_Command(int ctrlID, int message, unsigned int param)
	{
	switch (ctrlID)
		{
		case IDC_ADD_BUDDY_BUTTON:
			{
			MPWolAddBuddyPopupClass* dialog = new MPWolAddBuddyPopupClass;

			if (dialog)
				{
				const wchar_t* pagersName = mBuddyMgr->GetLastPagersName();
				dialog->Set_Default_User_Name(pagersName);
				dialog->Start_Dialog();
				dialog->Release_Ref();
				}
			break;
			}

		case IDC_IGNORE_PLAYER_BUTTON:
			{
			const wchar_t* pagersName = mBuddyMgr->GetLastPagersName();
			MPWolAddIgnoreEntry::DoDialog(pagersName);
			}
			break;

		case IDC_INVITE_BUDDY_BUTTON:
			{
			const wchar_t* pagersName = mBuddyMgr->GetLastPagersName();
			mBuddyMgr->InviteUser(pagersName, Get_Dlg_Item_Text(IDC_REPLY_EDIT));
			End_Dialog();
			}
			break;

		case IDOK:
		case IDC_REPLY_BUTTON:
			Send_Reply();
			break;
		}

	PopupDialogClass::On_Command(ctrlID, message, param);
	}


/******************************************************************************
*
* NAME
*     DlgWOLPageReply::Send_Reply
*
* DESCRIPTION
*     Send reply message to the last user who paged.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLPageReply::Send_Reply(void)
	{
	const wchar_t* pagersName = mBuddyMgr->GetLastPagersName();

	if (pagersName && (wcslen(pagersName) > 0))
		{
		WideStringClass reply(0, true);
		reply = Get_Dlg_Item_Text(IDC_REPLY_EDIT);
		reply.Trim();

		if (reply.Is_Empty() == false)
			{
			// Send the reply and clear the text
			mBuddyMgr->PageUser(pagersName, reply);

			// Add this message to the list
			WideStringClass name(0, true);
			WOLLogonMgr::GetLoginName(name);
			Add_Message(name, reply);
			}

		Set_Dlg_Item_Text(IDC_REPLY_EDIT, L"");
		}
	}


/******************************************************************************
*
* NAME
*     DlgWOLPageReply::Add_Message
*
* DESCRIPTION
*     Add a message to the message display area.
*
* INPUTS
*     Username - Name of user message is from.
*     Message  - Message text.
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLPageReply::Add_Message(const wchar_t* username, const wchar_t* message)
	{
	ListCtrlClass* list = (ListCtrlClass*)Get_Dlg_Item(IDC_MESSAGE_LIST);

	if (list)
		{
		// Build the message
		if (username) {
			WideStringClass text(255, true);
			text.Format(L"%s: %s", username, message);
			list->Insert_Entry(list->Get_Entry_Count(), text);
		} else {
			list->Insert_Entry(list->Get_Entry_Count(), message);
		}

		// Scroll to the end of the list
		list->Scroll_To_End();
		}
	}


/******************************************************************************
*
* NAME
*     DlgWOLPageReply::HandleNotification(WOLPagedEvent)
*
* DESCRIPTION
*     Handle page message events.
*
* INPUTS
*     Event - Page event.
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLPageReply::HandleNotification(WOLPagedEvent& event)
	{
	WWOnline::PageMessage* page = event.Subject();
	WWASSERT(page && "NULL page in WOLPagedEvent");

	const wchar_t* pager = NULL;
	if (!ConsoleBox.Is_Exclusive())
		{
		if (PAGE_RECEIVED == event.GetAction())
			{
			pager = page->GetPagersName();
			}

		Add_Message(pager, page->GetPageMessage());
		}
	}


/******************************************************************************
*
* NAME
*     DlgWOLPageReply::On_EditCtrl_Enter_Pressed
*
* DESCRIPTION
*     Handle enter key being pressed in edit control. Use this notification
*     to send the reply text automatically without requiring the user to click
*     the reply button.
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLPageReply::On_EditCtrl_Enter_Pressed(EditCtrlClass* edit, int id)
	{
	if (IDC_REPLY_EDIT == id)
		{
		Send_Reply();
		}
	}
