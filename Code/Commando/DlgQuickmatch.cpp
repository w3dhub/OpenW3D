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
* NAME
*     $Archive: /Commando/Code/Commando/DlgQuickmatch.cpp $
*
* DESCRIPTION
*     Quick match dialog
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 21 $
*     $Modtime: 1/16/02 5:06p $
*
******************************************************************************/

#include "DlgQuickmatch.h"
#include "DlgWOLWait.h"
#include "DlgMessageBox.h"
#include "dlgmpwolquickmatchoptions.h"
#include "renegadedialog.h"
#include "gameinitmgr.h"
#include "WOLJoinGame.h"
#include "string_ids.h"
#include <wwtranslatedb/translatedb.h>
#include <WWOnline/WaitCondition.h>
#include <wwui/listctrl.h>
#include <wwdebug/wwdebug.h>
#include "systimer.h"

using namespace WWOnline;

/******************************************************************************
*
* NAME
*     DlgQuickMatch::DoDialog
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     Success - True if dialog successfully started.
*
******************************************************************************/

bool DlgQuickMatch::DoDialog(void)
	{
	DlgQuickMatch* dialog = new DlgQuickMatch;

	if (dialog)
		{
		if (dialog->FinalizeCreate())
			{
			dialog->Start_Dialog();
			}

		dialog->Release_Ref();
		}

	return (dialog != NULL);
	}


/******************************************************************************
*
* NAME
*     DlgQuickMatch::DlgQuickMatch
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

DlgQuickMatch::DlgQuickMatch() :
		MenuDialogClass(GetRenegadeDialog(RenegadeDialogID::IDD_MP_WOL_QUICKMATCH_CONNECT)),
		mTimeoutTime(0),
		mResendTime(0)
	{
	WWDEBUG_SAY(("DlgQuickMatch: Instantiating\n"));
	}


/******************************************************************************
*
* NAME
*     DlgQuickMatch::~DlgQuickMatch
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

DlgQuickMatch::~DlgQuickMatch()
	{
	WWDEBUG_SAY(("DlgQuickMatch: Destructing\n"));

	if (mQuickMatch)
		{
		mQuickMatch->Release_Ref();
		}
	}


/******************************************************************************
*
* NAME
*     DlgQuickMatch::FinalizeCreate
*
* DESCRIPTION
*     Finalize the creation of this object (Initialize).
*
* INPUTS
*
* RESULT
*
******************************************************************************/

bool DlgQuickMatch::FinalizeCreate(void)
	{
	mQuickMatch = WOLQuickMatch::Create();

	if (!mQuickMatch)
		{
		return false;
		}

	Observer<QuickMatchEvent>::NotifyMe(*mQuickMatch);
	return true;
	}


/******************************************************************************
*
* NAME
*     DlgQuickMatch::On_Init_Dialog
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

void DlgQuickMatch::On_Init_Dialog(void)
	{
	ListCtrlClass* output = (ListCtrlClass*)Get_Dlg_Item(IDC_OUTPUT);
	WWASSERT_PRINT(output, "IDC_OUTOUT list control missing from dialog!");

	if (output)
		{
		output->Add_Column(U_CHAR(""), 1.0F, Vector3 (1, 1, 1));
		}

	Connect();

	MenuDialogClass::On_Init_Dialog();
	}


/******************************************************************************
*
* NAME
*     DlgQuickMatch::On_Frame_Update
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

void DlgQuickMatch::On_Frame_Update(void)
	{
	if (mConnectWait.IsValid())
		{
		WaitCondition::WaitResult waitStatus = mConnectWait->GetResult();

		if (waitStatus != WaitCondition::Waiting)
			{
			if (waitStatus == WaitCondition::ConditionMet)
				{
				SendMatchingInfo();
				}
			else
				{
				OutputMessage(mConnectWait->GetResultText());
				}

			mConnectWait.Release();
			}
		}

	unsigned int theTime = TIMEGETTIME();

	if ((mResendTime > 0) && (theTime >= mResendTime))
		{
		mQuickMatch->SendClientInfo();
		mResendTime = 0;
		}

	// Watch for timeout
	if ((mTimeoutTime > 0) && (theTime >= mTimeoutTime))
		{
		OutputMessage(TRANSLATE(IDS_WOL_TIMEDOUT));
		mTimeoutTime = 0;
		}

	MenuDialogClass::On_Frame_Update();
	}


/******************************************************************************
*
* NAME
*     DlgQuickMatch::
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

void DlgQuickMatch::On_Command(int ctrl, int message, unsigned int param)
	{
	if (ctrl == IDC_MENU_BACK_BUTTON)
		{
		OutputMessage(IDS_QM_DISCONNECT);
		RefPtr<WaitCondition> wait = mQuickMatch->Disconnect();

		if (wait.IsValid())
			{
			DlgWOLWait::DoDialog(IDS_QM_DISCONNECT, wait);
			}
		}

	MenuDialogClass::On_Command(ctrl, message, param);
	}


/******************************************************************************
*
* NAME
*     DlgQuickMatch::Connect
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

void DlgQuickMatch::Connect(void)
	{
	OutputMessage(IDS_QM_CONNECTING);
	mConnectWait = mQuickMatch->ConnectClient();

	if (mConnectWait.IsValid())
		{
		mConnectWait->WaitBeginning();
		}

	mTimeoutTime = (TIMEGETTIME() + 60000);
	}


/******************************************************************************
*
* NAME
*     DlgQuickMatch::SendMatchingInfo
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

void DlgQuickMatch::SendMatchingInfo(void)
	{
	OutputMessage(IDS_QM_SENDINGMATCHINFO);

	bool sent = mQuickMatch->SendClientInfo();

	if (sent)
		{
		OutputMessage(TRANSLATE (IDS_MENU_SEARCHING_FOR_MATCH));
		}
	else
		{
		OutputMessage(TRANSLATE (IDS_MENU_CANT_LOCATE_MATCH));
		}
	}


/******************************************************************************
*
* NAME
*     DlgQuickMatch::OutputMessage
*
* DESCRIPTION
*
* INPUTS
*     Message - Message to add to output.
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgQuickMatch::OutputMessage(int messageID)
	{
	const unichar_t* message = TranslateDBClass::Get_String(messageID);
	OutputMessage(message);
	}


/******************************************************************************
*
* NAME
*     DlgQuickMatch::OutputMessage
*
* DESCRIPTION
*
* INPUTS
*     Message - Message to add to output.
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgQuickMatch::OutputMessage(const unichar_t* message)
	{
	WWDEBUG_SAY(("QM: %S\n", message));

	ListCtrlClass* output = (ListCtrlClass*)Get_Dlg_Item(IDC_OUTPUT);

	if (output)
		{
		int entryCount = output->Get_Entry_Count();
		output->Insert_Entry(entryCount, message);

		// Limit message entries.
		if (entryCount > 100)
			{
			output->Delete_Entry(0);
			}
		}
	}


/******************************************************************************
*
* NAME
*     DlgQuickMatch::HandleNotification(QuickMatchEvent)
*
* DESCRIPTION
*     Handle status information from quickmatch.
*
* INPUTS
*     Status -
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgQuickMatch::HandleNotification(QuickMatchEvent& status)
	{
	QuickMatchEvent::Event event = status.GetEvent();
	const unichar_t* msg = (const unichar_t*)status.Subject();

	if (QuickMatchEvent::QMERROR == event)
		{
		const WideStringClass& statusMsg = status.Subject();

		// If no match then resend the request in 10 seconds.
		if (statusMsg.Compare_No_Case(U_CHAR("QM:NoMatch")) == 0)
			{
			msg = TRANSLATE(IDS_QM_NOMATCH);
			}
		else
			{
			#ifdef QUICKMATCH_OPTIONS
			// Unable to match user because preferred modes are all zero.
			if (statusMsg.Compare_No_Case(U_CHAR("QM:NoModes")) == 0)
				{
				DlgMsgBox::DoDialog(TRANSLATE (IDS_MENU_INVALID_QM_SETTINGS_TITLE), TRANSLATE (IDS_MENU_INVALID_QM_SETTINGS),
						DlgMsgBox::Okay, this);

				return;
				}
			#endif
			}
		}
	else if (QuickMatchEvent::QMMATCHED == event)
		{
		GameInitMgrClass::Set_WOL_Return_Dialog(RenegadeDialogMgrClass::LOC_INTERNET_MAIN);

		// Join the game
		const unichar_t* gameName = (const unichar_t*)status.Subject();
		WOLJoinGame::JoinTheGame(gameName, U_CHAR(""), false);
		return;
		}

	// Assume any other message is for the user.
	OutputMessage(msg);
	}


/******************************************************************************
*
* NAME
*     DlgQuickMatch::HandleNotification(DlgMsgBoxEvent)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgQuickMatch::HandleNotification([[maybe_unused]] DlgMsgBoxEvent& msgbox)
	{
#ifdef QUICKMATCH_OPTIONS
	Add_Ref();

	if (msgbox.Event() == DlgMsgBoxEvent::Okay)
		{
		End_Dialog();
		START_DIALOG(MPWolQuickMatchOptionsMenuClass);
		}

	Release_Ref();
#endif // QUICKMATCH_OPTIONS
	}
