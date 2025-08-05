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
*     $Archive: /Commando/Code/Commando/DlgWebPage.cpp $
*
* DESCRIPTION
*     Web Browser dialog
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 9 $
*     $Modtime: 10/30/01 10:08p $
*
******************************************************************************/

#include "DlgWebPage.h"
#include "WebBrowser.h"
#include <WWUI\PopupDialog.h>
#include <WWUI\DialogControl.h>
#include <Combat\DirectInput.h>
#include <WW3D2\WW3D.h>
#include "DlgMessageBox.h"
#include "Resource.h"
#include <Combat\String_IDs.h>
#include "WWDebug.h"


/******************************************************************************
*
* NAME
*     DlgWebPage::DoDialog
*
* DESCRIPTION
*     Show the specified web page.
*
* INPUTS
*     Page - Page to display.
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWebPage::DoDialog(const char* page)
	{
	WWASSERT_PRINT(page && (strlen(page) > 0), "Invalid parameter.\n");

	if ((page == NULL) || (strlen(page) == 0))
		{
		return;
		}

	DlgWebPage* dialog = new DlgWebPage;

	if (dialog)
		{
		bool success = dialog->FinalizeCreate();

		if (success)
			{
			// If we are using the embedded browser then show the webpage. Otherwise
			// ask the user if they want to launch an external browser to view the page.
			if (dialog->mBrowser->UsingEmbeddedBrowser())
				{
				dialog->Start_Dialog();
				dialog->mBrowser->ShowWebPage(page);
				}
			else
				{
				// Increment the dialog reference so the dialog will be around for the
				// message box result.
				dialog->Add_Ref();

				dialog->mPage = page;
				DlgMsgBox::DoDialog(0, IDS_WEB_LAUNCHBROWSER,
						DlgMsgBox::YesNo, static_cast< Observer<DlgMsgBoxEvent>* >(dialog));
				}
			}

		// The dialog manager keeps a referenece to the dialog.
		dialog->Release_Ref();
		}
	}


/******************************************************************************
*
* NAME
*     DlgWebPage::DlgWebPage
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

DlgWebPage::DlgWebPage() :
		DialogBaseClass(IDD_WEBPAGE),
		mBrowser(NULL)
	{
	WWDEBUG_SAY(("Instantiating DlgWebPage\n"));
	}


/******************************************************************************
*
* NAME
*     DlgWebPage::~DlgWebPage
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

DlgWebPage::~DlgWebPage()
	{
	WWDEBUG_SAY(("Destroying DlgWebPage\n"));

	if (mBrowser)
		{
		mBrowser->Release();
		}
	}


/******************************************************************************
*
* NAME
*     DlgWebPage::FinalizeCreate
*
* DESCRIPTION
*     Finalize object creation. (Initialize object)
*
* INPUTS
*     NONE
*
* RESULT
*     Success - True if successful; False if failed.
*
******************************************************************************/

bool DlgWebPage::FinalizeCreate(void)
	{
	mBrowser = WebBrowser::CreateInstance(MainWindow);

	if (mBrowser)
		{
		Observer<WebEvent>::NotifyMe(*mBrowser);
		return true;
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     DlgWebPage::Start_Dialog
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

void DlgWebPage::Start_Dialog(void)
	{
	DirectInput::Unacquire();
	WW3D::Flip_To_Primary();
	DialogBaseClass::Start_Dialog();
	}


/******************************************************************************
*
* NAME
*     DlgWebPage::End_Dialog
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

void DlgWebPage::End_Dialog(void)
	{
	DirectInput::Acquire();
	DialogBaseClass::End_Dialog();
	SetFocus(MainWindow);
	}


/******************************************************************************
*
* NAME
*     DlgWebPage::On_Frame_Update
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

void DlgWebPage::On_Frame_Update(void)
	{
	DialogBaseClass::On_Frame_Update();

	if (mBrowser)
		{
		bool usingEmbedded = mBrowser->UsingEmbeddedBrowser();

		if (!usingEmbedded)
			{
			bool externalRunning = mBrowser->IsExternalBrowserRunning();
			bool gameActivated = (GameInFocus || (GetTopWindow(NULL) == MainWindow)
					|| (GetForegroundWindow() == MainWindow));

			if (!externalRunning || gameActivated)
				{
				WWDEBUG_SAY(("***** Reactivating Game *****\n"));

				HWND topWindow = GetTopWindow(NULL);

        if (topWindow != MainWindow)
					{
          SetForegroundWindow(MainWindow);
          ShowWindow(MainWindow, SW_RESTORE);
					}

				End_Dialog();
				}
			}
		}
	}


/******************************************************************************
*
* NAME
*     DlgWebPage::HandleNotification(WebEvent)
*
* DESCRIPTION
*     Handle web event notifications
*
* INPUTS
*     WebEvent - WebEvent to handle
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWebPage::HandleNotification(WebEvent& event)
	{
	switch (event.Event())
		{
		case WebEvent::Quit:
			End_Dialog();
			break;

		case WebEvent::CertificationFailed:
			{
			WebBrowser* browser = event.Subject();
			browser->Hide();

			DlgMsgBox::DoDialog(IDS_WEB_ERROR, IDS_WEB_PAGEFAILED);

			End_Dialog();
			}
			break;

		default:
			break;
		}
	}


/******************************************************************************
*
* NAME
*     DlgWebPage::HandleNotification(DlgMsgBoxEvent)
*
* DESCRIPTION
*     Handle message box dialog event notifications
*
* INPUTS
*     DlgMsgBoxEvent - Message box event to handle.
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWebPage::HandleNotification(DlgMsgBoxEvent& event)
	{
	switch (event.Event())
		{
		case DlgMsgBoxEvent::Yes:
			// Start the dialog to monitor the external browser.
			Start_Dialog();
			mBrowser->ShowWebPage(mPage);

			// Release the reference we added to keep the dialog alive until this point.
			Release_Ref();
			break;

		case DlgMsgBoxEvent::No:
			// Release the reference we added to keep the dialog alive until this point.
			Release_Ref();
			break;

		default:
			break;
		}
	}

