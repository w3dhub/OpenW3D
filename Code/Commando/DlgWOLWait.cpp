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

/******************************************************************************
*
* FILE
*     $Archive: /Commando/Code/Commando/DlgWOLWait.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Bhayes $
*
* VERSION INFO
*     $Revision: 21 $
*     $Modtime: 2/18/02 7:57p $
*
******************************************************************************/

#include "DlgWOLWait.h"
#include "renegadedialog.h"
#include "DlgMessageBox.h"
#include "string_ids.h"
#include <WWOnline/WOLSession.h>
#include <wwui/mousemgr.h>
#include <wwtranslatedb/translatedb.h>
#include <wwdebug/wwdebug.h>
#include "gamespyadmin.h"
#include "systimer.h"

using namespace WWOnline;

DlgWOLWait* DlgWOLWait::mTheInstance = NULL;

/******************************************************************************
*
* NAME
*     DlgWOLWait::DoDialog
*
* DESCRIPTION
*
* INPUTS
*     Title    - Title of dialog
*     Wait     - Wait condition to process.
*     Observer - DlgWOLWaitEvent observer
*
* RESULT
*     Success - True if dialog successfully started.
*
******************************************************************************/

bool DlgWOLWait::DoDialog(const unichar_t* title, RefPtr<WaitCondition>& wait,
			Observer<DlgWOLWaitEvent>* observer, unsigned int timeout, unsigned int dialog_timeout)
	{
	if (wait.IsValid())
		{
		DlgWOLWait* popup = new DlgWOLWait(wait, timeout, dialog_timeout);

		if (popup)
			{
			WWDEBUG_SAY(("DlgWOLWait: Starting dialog '%S'\n", title));
			popup->Start_Dialog();
			popup->Set_Title(title);

			if (observer)
				{
				popup->AddObserver(*observer);
				}

			popup->Release_Ref();
			}

		return (popup != NULL);
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     DlgWOLWait::DoDialog
*
* DESCRIPTION
*
* INPUTS
*     Title    - Title of dialog
*     Wait     - Wait condition to process.
*     Observer - DlgWOLWaitEvent observer
*
* RESULT
*     Success - True if dialog successfully started.
*
******************************************************************************/

bool DlgWOLWait::DoDialog(const unichar_t* title, const unichar_t* button_text, RefPtr<WaitCondition>& wait,
			Observer<DlgWOLWaitEvent>* observer, unsigned int timeout, unsigned int dialog_timeout)
	{
	if (wait.IsValid())
		{
		DlgWOLWait* popup = new DlgWOLWait(wait, timeout, dialog_timeout);

		if (popup)
			{
			WWDEBUG_SAY(("DlgWOLWait: Starting dialog '%S'\n", title));
			popup->Start_Dialog();
			popup->Set_Dlg_Item_Text(IDCANCEL, button_text);
			popup->Set_Title(title);

			if (observer)
				{
				popup->AddObserver(*observer);
				}

			popup->Release_Ref();
			}

		return (popup != NULL);
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     DlgWOLWait::DoDialog
*
* DESCRIPTION
*
* INPUTS
*     Title    - Title ID of dialog
*     Wait     - Wait condition to process.
*     Observer - DlgWOLWaitEvent observer
*
* RESULT
*     Success - True if dialog successfully started.
*
******************************************************************************/

bool DlgWOLWait::DoDialog(int titleID, RefPtr<WaitCondition>& wait,
			Observer<DlgWOLWaitEvent>* observer, unsigned int timeout, unsigned int dialog_timeout)
	{
	const unichar_t* title = TranslateDBClass::Get_String(titleID);
	return DoDialog(title, wait, observer, timeout, dialog_timeout);
	}


/******************************************************************************
*
* NAME
*     DlgWOLWait::DlgWOLWait
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     Wait - Wait condition to process.
*
* RESULT
*     NONE
*
******************************************************************************/

DlgWOLWait::DlgWOLWait(RefPtr<WaitCondition>& wait, unsigned int timeout, unsigned int dialog_timeout) :
		PopupDialogClass(GetRenegadeDialog(RenegadeDialogID::IDD_WOL_WAIT)),
		mWait(wait),
		mStartTime(0),
		mTimeout(timeout),
		mDialogTimeout(dialog_timeout),
		mShowDialog(false)
	{
	mTheInstance = this;
	}


/******************************************************************************
*
* NAME
*     DlgWOLWait::~DlgWOLWait
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

DlgWOLWait::~DlgWOLWait()
	{
	mTheInstance = NULL;
	}


/******************************************************************************
*
* NAME
*     DlgWOLWait::On_Init_Dialog
*
* DESCRIPTION
*     One time dialog initialization.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLWait::On_Init_Dialog(void)
	{
	if (mWait.IsValid())
		{
		if (!cGameSpyAdmin::Is_Gamespy_Game()) {
			mWOLSession = Session::GetInstance(false);
			WWASSERT(mWOLSession.IsValid());
		}

		// Start the wait condition
		mWait->WaitBeginning();

		// Set the dialogs wait text
		Set_Dlg_Item_Text(IDC_WAITTEXT, mWait->GetWaitText());

		mStartTime = TIMEGETTIME();

		if (mDialogTimeout != SHOW_NEVER)
			{
			// Initially the dialog is not shown so bring up the wait cursor.
			mShowDialog = false;
			MouseMgrClass::Begin_Wait_Cursor();
			}
		}

	PopupDialogClass::On_Init_Dialog();
	}


/******************************************************************************
*
* NAME
*     DlgWOLWait::On_Destroy
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLWait::On_Destroy(void)
	{
	WWDEBUG_SAY(("DlgWOLWait: Ending dialog '%S'\n", (const unichar_t*)Title));

	if (!mShowDialog && mDialogTimeout != SHOW_NEVER)
		{
		MouseMgrClass::End_Wait_Cursor();
		}
	}


/******************************************************************************
*
* NAME
*     DlgWOLWait::On_Periodic
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLWait::On_Periodic(void)
	{
	Add_Ref();

	CheckCondition();
	PopupDialogClass::On_Periodic();

	Release_Ref();
	}


/******************************************************************************
*
* NAME
*     DlgWOLWait::CheckCondition
*
* DESCRIPTION
*     Check the condition of the wait.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLWait::CheckCondition(void)
	{
	// Quit if there is nothing to wait on or there is not WWOnline session.
	if (!mWait.IsValid() || (!mWOLSession.IsValid() && !cGameSpyAdmin::Is_Gamespy_Game()))
		{
		End_Dialog();
		return;
		}

	// Check the status of the wait condition
	WaitCondition::WaitResult waitStatus = mWait->GetResult();

	// If we are no longer waiting then process the result.
	if (waitStatus != WaitCondition::Waiting)
		{
		// If there are no observers and the wait timed out or errored then
		// show a message dialog describing the failure.
		if ((Notifier<DlgWOLWaitEvent>::HasObservers() == false)
				&& ((waitStatus == WaitCondition::TimeOut) || (waitStatus == WaitCondition::Error)))
			{
			DlgMsgBox::DoDialog(mWait->GetWaitText(), mWait->GetResultText());
			}

		// Notify the observers about the status of the wait.
		Add_Ref();
		DlgWOLWaitEvent event(waitStatus, mWait.ReferencedObject());
		NotifyObservers(event);
		Release_Ref();

		End_Dialog();
		return;
		}

	// Change the waiting text if necessary.
	const unichar_t* text = Get_Dlg_Item_Text(IDC_WAITTEXT);
	const WideStringClass& waitText = mWait->GetWaitText();

	if (waitText.Compare_No_Case(text) != 0)
		{
		Set_Dlg_Item_Text(IDC_WAITTEXT, waitText);
		PopupDialogClass::Build_Background_Renderers();
		}

	// Watch for timeout
	unsigned int currTime = TIMEGETTIME();
	unsigned int timeout = mTimeout;
	unsigned int dialog_timeout = mDialogTimeout;

	if (dialog_timeout == 0)
		{
		dialog_timeout = 2000;
		}

	// If the dialog timeout is 0 then use the wait conditions timeout.
	if (timeout == 0)
		{
		timeout = mWait->GetTimeout();
		}

	if ((currTime - mStartTime) > timeout)
		{
		mWait->EndWait(WaitCondition::TimeOut, TRANSLATE(IDS_WOL_TIMEDOUT));
		}

	// Do not render until sufficient time has elapsed. This will prevent the
	// dialog from flashing in and out when the wait condition is satisfied quickly.
	if (!mShowDialog && mStartTime && dialog_timeout != SHOW_NEVER && ((currTime - mStartTime) > 2000))
		{
		mShowDialog = true;
		MouseMgrClass::End_Wait_Cursor();
		}

	// Allow time for WWOnline processing.
	if (mWOLSession.IsValid()) mWOLSession->Process();
	}


/******************************************************************************
*
* NAME
*     DlgWOLWait::On_Command
*
* DESCRIPTION
*     Process command message
*
* INPUTS
*     Ctrl    - ID of control
*     Message -
*     Param   -
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLWait::On_Command(int ctrl, int /* message */, unsigned int /* param */)
	{
	if ((ctrl == IDCANCEL) && mWait.IsValid())
		{
		WWDEBUG_SAY(("DlgWOLWait: UserAborted '%S'\n", (const unichar_t*)mWait->GetWaitText()));
		mWait->EndWait(WaitCondition::UserCancel, TRANSLATE(IDS_WOL_CANCELED));
		}
	}


/******************************************************************************
*
* NAME
*     DlgWOLWait::Render
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLWait::Render(void)
	{
	// Do not render until sufficient time has elapsed. This will prevent the
	// dialog from flashing in and out when the wait condition is satisfied quickly.
	if (mShowDialog)
		{
		PopupDialogClass::Render();
		}
	}
