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
*     $Archive: /Commando/Code/Commando/DlgRestart.cpp $
*
* DESCRIPTION
*     Dialog to notify the user that the game requires a restart.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 3 $
*     $Modtime: 10/18/01 6:23p $
*
******************************************************************************/

#include "DlgRestart.h"
#include "renegadedialog.h"
#include "mainloop.h"
#include "string_ids.h"
#include <wwtranslatedb/translatedb.h>
#include <wwdebug/wwdebug.h>

/******************************************************************************
*
* NAME
*     DlgRestart::DoDialog
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

bool DlgRestart::DoDialog(void)
	{	
	DlgRestart* popup = new DlgRestart;

	if (popup)
		{
		popup->Start_Dialog();
		popup->Set_Title(TRANSLATE(IDS_RESTART_TITLE));
		popup->Release_Ref();
		}

	return (popup != NULL);
	}


/******************************************************************************
*
* NAME
*     DlgRestart::DlgRestart
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

DlgRestart::DlgRestart() :
		PopupDialogClass(GetRenegadeDialog(RenegadeDialogID::IDD_MESSAGEBOX_OK))
	{
	WWDEBUG_SAY(("DlgRestart: Instantiated\n"));
	}


/******************************************************************************
*
* NAME
*     DlgRestart::~DlgRestart
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

DlgRestart::~DlgRestart()
	{
	WWDEBUG_SAY(("DlgRestart: Destructing\n"));
	}


/******************************************************************************
*
* NAME
*     DlgRestart::On_Init_Dialog
*
* DESCRIPTION
*     One time initialzation.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgRestart::On_Init_Dialog(void)
	{
	Set_Dlg_Item_Text(IDC_MESSAGE, TRANSLATE(IDS_RESTART_PROMPT));
	PopupDialogClass::On_Init_Dialog();
	}


/******************************************************************************
*
* NAME
*     DlgRestart::On_Command
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

void DlgRestart::On_Command(int ctrl, int /* message */, unsigned int /* param */)
	{
	if (IDOK == ctrl)
		{
		Stop_Main_Loop(RESTART_EXITCODE);
		End_Dialog();
		}
	}
