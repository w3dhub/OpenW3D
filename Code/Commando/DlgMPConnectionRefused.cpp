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
*     $Archive: /Commando/Code/Commando/DlgMPConnectionRefused.cpp $
*
* DESCRIPTION
*
******************************************************************************/

#include "DlgMPConnectionRefused.h"
#include "cnetwork.h"
#include "renegadedialog.h"
#include <wwdebug/wwdebug.h>
#include "dlgmainmenu.h"
#include "gamespyadmin.h"
#include "specialbuilds.h"
#include "dialogtests.h"
#include "dialogmgr.h"
#include "gameinitmgr.h"

/******************************************************************************
*
* NAME
*     DlgMPConnectionRefused::DoDialog
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     True if dialog created successfully.
*
******************************************************************************/

bool DlgMPConnectionRefused::DoDialog(const unichar_t * text, bool show_splash_screen)
	{	
	DlgMPConnectionRefused* popup = new DlgMPConnectionRefused(text, show_splash_screen);

	if (popup)
		{
		popup->Start_Dialog();
		popup->Release_Ref();
		}

	return (popup != NULL);
	}


/******************************************************************************
*
* NAME
*     DlgMPConnectionRefused::DlgMPConnectionRefused
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

DlgMPConnectionRefused::DlgMPConnectionRefused(const unichar_t * text, bool show_splash_screen) :
		PopupDialogClass(GetRenegadeDialog(RenegadeDialogID::IDD_MULTIPLAY_CONNECTION_REFUSED))
	{
	WWDEBUG_SAY(("DlgMPConnectionRefused: Instantiated\n"));

	WWASSERT(text != NULL);
	Text.Format(text);

	ShowSplashScreen = show_splash_screen;
	}


/******************************************************************************
*
* NAME
*     DlgMPConnectionRefused::~DlgMPConnectionRefused
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

DlgMPConnectionRefused::~DlgMPConnectionRefused()
	{
	WWDEBUG_SAY(("DlgMPConnectionRefused: Destructing\n"));
	}


/******************************************************************************
*
* NAME
*     DlgMPConnectionRefused::On_Init_Dialog
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

void DlgMPConnectionRefused::On_Init_Dialog(void)
	{
	Set_Dlg_Item_Text(IDC_REFUSAL_TEXT, Text);

	PopupDialogClass::On_Init_Dialog();
	}


/******************************************************************************
*
* NAME
*     DlgMPConnectionRefused::On_Command
*
* DESCRIPTION
*     Process command messages from controls
*
* INPUTS
*     Ctrl    - ID of control
*     Message -
*     Param   -	0 = User invoked abort. 1 = Connection refused by server.
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMPConnectionRefused::On_Command(int ctrlID, int message, unsigned int param)
	{
	if ((IDOK == ctrlID) && (1 != param))
		{
		if (cNetwork::I_Am_Client())
			{
			cNetwork::Cleanup_Client();
			}
		if (cGameSpyAdmin::Get_Is_Launched_From_Gamespy()) 
			{

#ifdef MULTIPLAYERDEMO
			GameInitMgrClass::End_Game ();
			if (ShowSplashScreen)
			{
				DialogMgrClass::Flush_Dialogs ();
				START_DIALOG (SplashOutroMenuDialogClass);
			}
			else 
			{
				extern void Stop_Main_Loop (int);
				Stop_Main_Loop(EXIT_SUCCESS);
			}
#else
			extern void Stop_Main_Loop (int);
			Stop_Main_Loop(EXIT_SUCCESS);
#endif // MULTIPLAYERDEMO
			}

		else if (DialogMgrClass::Get_Dialog_Count () == 1)
			{
			START_DIALOG (MainMenuDialogClass);
			}
		}

	PopupDialogClass::On_Command(ctrlID, message, param);
	}
