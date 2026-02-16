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
*     $Archive: /Commando/Code/Commando/DlgDownload.cpp $
*
* DESCRIPTION
*     File download dialog.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Steve_t $
*
* VERSION INFO
*     $Revision: 11 $
*     $Modtime: 1/12/02 2:53p $
*
******************************************************************************/

#include "renegadedialog.h"
#include "DlgDownload.h"
#include "DlgMessageBox.h"
#include "DlgRestart.h"
#include <wwui/ProgressCtrl.h>
#include "string_ids.h"
#include <wwtranslatedb/translatedb.h>
#include <wwdebug/wwdebug.h>
#include "mainloop.h"
#include "ConsoleMode.h"
#include "systimer.h"
#include <algorithm>

using namespace WWOnline;

static void PrintableSize(unsigned int size, WideStringClass& printable);
static void PrintableTime(unsigned int seconds, WideStringClass& printable);

bool DlgDownload::mQuietMode = false;


/******************************************************************************
*
* NAME
*     DlgDownload::DoDialog
*
* DESCRIPTION
*     Start download dialog
*
* INPUTS
*     Title - Dialog title
*     Files - List of files to download
*
* RESULT
*     Success - True if dialog successfully started.
*
******************************************************************************/

bool DlgDownload::DoDialog(const unichar_t* title, const DownloadList& files, bool quiet)
	{
	if (!files.empty())
		{
		mQuietMode = quiet;
		DlgDownload* popup = new DlgDownload;

		if (popup)
			{
			if (popup->FinalizeCreate(files))
				{
				popup->Start_Dialog();
				popup->Set_Title(title);
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
*     DlgDownload::DlgDownload
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

DlgDownload::DlgDownload() :
		PopupDialogClass(GetRenegadeDialog(RenegadeDialogID::IDD_WOL_DOWNLOAD)),
		mDownloading(false)
	{
	WWDEBUG_SAY(("DlgDownload: Instantiated\n"));
	}


/******************************************************************************
*
* NAME
*     DlgDownload::~DlgDownload
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

DlgDownload::~DlgDownload()
	{
	WWDEBUG_SAY(("DlgDownload: Destructing\n"));
	}


/******************************************************************************
*
* NAME
*     DlgDownload::FinalizeCreate
*
* DESCRIPTION
*     Finalize the dialog creation.
*
* INPUTS
*     Files - List of files to download.
*
* RESULT
*     True if successful.
*
******************************************************************************/

bool DlgDownload::FinalizeCreate(const DownloadList& files)
	{
	mWait = DownloadWait::Create(files);
	return mWait.IsValid();
	}


/******************************************************************************
*
* NAME
*     DlgDownload::On_Init_Dialog
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

void DlgDownload::On_Init_Dialog(void)
	{
	// Set estimated time and read / total
	WideStringClass text(0, true);
	text.Format(TRANSLATE(IDS_MENU_TRANSFER_RATE_TIME), 0, 0, 0);
	Set_Dlg_Item_Text(IDC_PROGRESSTEXT, text);

	// Update transfer rate.
	text.Format(TRANSLATE(IDS_MENU_TRANSFER_RATE), 0);
	Set_Dlg_Item_Text(IDC_TRANSFERTEXT, text);

	// Start the wait condition
	mWait->SetCallback(DlgDownload::HandleCallback, this);
	mWait->WaitBeginning();

	PopupDialogClass::On_Init_Dialog();
	}


/******************************************************************************
*
* NAME
*     DlgDownload::On_Command
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

void DlgDownload::On_Command(int ctrl, int message, unsigned int param)
	{
	if (ctrl == IDCANCEL)
		{
		WWDEBUG_SAY(("DlgDownload: User Aborted\n"));
		mWait->EndWait(WaitCondition::UserCancel, TRANSLATE(IDS_WOL_CANCELED));
		}

	PopupDialogClass::On_Command(ctrl, message, param);
	}


/******************************************************************************
*
* NAME
*     DlgDownload::On_Periodic
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

void DlgDownload::On_Periodic(void)
	{
	PopupDialogClass::On_Periodic();

	// Check the status of the wait condition
	WaitCondition::WaitResult waitStatus = mWait->GetResult();

	// If we are no longer waiting then process the result.
	if (waitStatus != WaitCondition::Waiting)
		{
		if (waitStatus == WaitCondition::ConditionMet)
			{
			if (mQuietMode)
				{
				ConsoleBox.Print("Restarting....\n");
				Stop_Main_Loop(RESTART_EXITCODE);
				}
			else
				{
				DlgRestart::DoDialog();
				}
			}
		else
			{
			// If the download was aborted or errored then show a message dialog
			// describing the failure.
			DlgMsgBox::DoDialog(TRANSLATE(IDS_WOL_DOWNLOADERROR), mWait->GetResultText());
			}

		End_Dialog();
		}
	}


/******************************************************************************
*
* NAME
*     DlgDownload::UpdateProgress
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgDownload::UpdateProgress(DownloadEvent& event)
	{
	const RefPtr<Download>& download = event.GetDownload();

	switch (event.GetEvent())
		{
		case DownloadEvent::DOWNLOAD_STATUS:
			Set_Dlg_Item_Text(IDC_STATUSTEXT, download->GetStatusText());
			break;

		case DownloadEvent::DOWNLOAD_PROGRESS:
			{
			int read = 0;
			int size = 0;
			int elapsed = 0;
			int remaining = 0;
			download->GetProgress(read, size, elapsed, remaining);

			if (ConsoleBox.Is_Exclusive())
				{

				if (!mDownloading)
					{
					mStartTime = TIMEGETTIME();
					mDownloading = true;
					}

				// Calculate the transfer rate
				unsigned int transferRate = read;
				unsigned int elapsedTime = ((TIMEGETTIME() - mStartTime) / 1000);
				if (elapsedTime > 0)
					{
					transferRate = (read / elapsedTime);
					}

				ConsoleBox.Print("Got %d Kb of %d at %d Kb per second       \r", read/1024, size/1024, transferRate / 1024);
				}
			else
				{
				// Update progress bar
				ProgressCtrlClass* progress = (ProgressCtrlClass*)Get_Dlg_Item(IDC_PROGRESS);

				if (progress)
					{
					// If we have just begun downloading then set the progress bar range
					// for the size of the new file.
					if (!mDownloading)
						{
						progress->Set_Range(0, size);
						mStartTime = TIMEGETTIME();
						mDownloading = true;
						}

					progress->Set_Position(read);
					}

				//-----------------------------------------------------------------------
				// Update download statistics
				//-----------------------------------------------------------------------

				// Calculate the transfer rate
				unsigned int transferRate = read;
				unsigned int elapsedTime = ((TIMEGETTIME() - mStartTime) / 1000);

				if (elapsedTime > 0)
					{
					transferRate = (read / elapsedTime);
					}

				// Update transfer rate.
				WideStringClass sizetext(64, true);
				PrintableSize(transferRate, sizetext);

				WideStringClass text(0, true);
				text.Format(TRANSLATE(IDS_MENU_TRANSFER_RATE_PER_SEC), (const unichar_t*)sizetext);
				Set_Dlg_Item_Text(IDC_TRANSFERTEXT, text);

				// Calculate estimated time based on the current transfer rate.
				unsigned int estimatedTime = 0;

				if (transferRate > 0)
					{
					estimatedTime = ((size - read) / transferRate);
					}

				// Update estimated time and read / total
				WideStringClass timetext(64,true);
				PrintableTime(estimatedTime, timetext);

				WideStringClass readtext(64, true);
				PrintableSize(read, readtext);

				PrintableSize(size, sizetext);

				text.Format(TRANSLATE(IDS_MENU_TRANSFER_TIME_LEFT),
					(const unichar_t*)timetext, (const unichar_t*)readtext, (const unichar_t*)sizetext);

				Set_Dlg_Item_Text(IDC_PROGRESSTEXT, text);
				}
			}
			break;

		case DownloadEvent::DOWNLOAD_BEGIN:
			if (ConsoleBox.Is_Exclusive())
				{
				ConsoleBox.Print("\n");
				}
			break;

		case DownloadEvent::DOWNLOAD_END:
			if (ConsoleBox.Is_Exclusive())
				{
				ConsoleBox.Print("\nDownload complete\n");
				}
			WWDEBUG_SAY(("DlgDownload: Successful '%s' Elapsed time: %ld ms\n",
				download->GetFilename(), (TIMEGETTIME() - mStartTime)));

			mDownloading = false;
			break;

		case DownloadEvent::DOWNLOAD_STOPPED:
			WWDEBUG_SAY(("DlgDownload: Stopped '%s'\n", download->GetFilename()));
			mDownloading = false;
			break;

		case DownloadEvent::DOWNLOAD_ERROR:
			WWDEBUG_SAY(("DlgDownload: Error '%s'\n", download->GetFilename()));
			mDownloading = false;
			break;

		default:
			break;
		}
	}


/******************************************************************************
*
* NAME
*     DlgDownload::HandleCallback
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgDownload::HandleCallback(DownloadEvent& event, void *userdata)
	{
	DlgDownload* dialog = (DlgDownload*)userdata;

	if (dialog)
		{
		dialog->UpdateProgress(event);
		}
	}


/******************************************************************************
*
* NAME
*     PrintableSize
*
* DESCRIPTION
*     Get filesize in a printable format.
*
* INPUTS
*     Size -
*
* RESULT
*     NONE
*
******************************************************************************/

void PrintableSize(unsigned int size, WideStringClass& printable)
	{
	float value = ((float)size / (float)(1024 * 1024));

	if (value >= 1.0)
		{
		printable.Format(TRANSLATE(IDS_MENU_TRANSFER_MB_FORMAT), value);
		return;
		}

	value = ((float)size / 1024.0);

	if (value >= 1.0)
		{
		printable.Format(TRANSLATE(IDS_MENU_TRANSFER_KB_FORMAT), value);
		return;
		}

	printable.Format(TRANSLATE(IDS_MENU_TRANSFER_BYTE_FORMAT), size);
	}


/******************************************************************************
*
* NAME
*     PrintableTime
*
* DESCRIPTION
*     Get time in a printable format.
*
* INPUTS
*     Size -
*
* RESULT
*     NONE
*
******************************************************************************/

void PrintableTime(unsigned int time, WideStringClass& printable)
	{
	unsigned int minutes = (time / 60);
	unsigned int seconds = (time % 60);

	if (minutes > 0)
		{
		printable.Format(TRANSLATE(IDS_MENU_TRANSFER_MIN_SEC_FORMAT), minutes, seconds);
		}
	else
		{
		seconds = std::max<unsigned int>(seconds, 1);
		printable.Format(TRANSLATE(IDS_MENU_TRANSFER_SEC_FORMAT), seconds);
		}
	}
