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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/DlgWOLAutoStart.cpp                 $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 12/06/01 2:04p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "always.h"
#include "autostart.h"
#include "win.h"
#include "listctrl.h"
#include "dlgwolautostart.h"
#include "menubackdrop.h"
#include "registry.h"
#include "_globals.h"

/***********************************************************************************************
 * AutoRestartProgressDialogClass::AutoRestartProgressDialogClass -- Constructor               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    11/6/2001 11:02AM ST : Created                                                           *
 *=============================================================================================*/
AutoRestartProgressDialogClass::AutoRestartProgressDialogClass(void) :
	MenuDialogClass (IDD_MP_AUTO_RESTART_PROGRESS)
{
	Instance = this;
	AddItemIndex = 0;
}


/***********************************************************************************************
 * AutoRestartProgressDialogClass::On_Init_Dialog -- Initialise the dialog                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/6/2001 11:03AM ST : Created                                                            *
 *=============================================================================================*/
void AutoRestartProgressDialogClass::On_Init_Dialog (void)
{
	/*
	** Create the backdrop if necessary
	*/
	RegistryClass reg(APPLICATION_SUB_KEY_NAME_OPTIONS);
	if (reg.Get_Int("DisableMenuAnim", 0) == 0) {
		if (Get_BackDrop ()->Peek_Model () == NULL) {
			Get_BackDrop ()->Set_Model ("IF_BACK01");
			Get_BackDrop ()->Set_Animation ("IF_BACK01.IF_BACK01");
		}
	}

	/*
	** Set the columns up. Just one with nothing in it.
	*/
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_PROGRESS_INFO);
	if (list_ctrl != NULL) {
		list_ctrl->Add_Column (L"", 1.0F, Vector3 (1, 1, 1));
		list_ctrl->Allow_Selection(false);
		list_ctrl->Set_Wants_Focus(false);
	}

	/*
	** Start adding items at index 0.
	*/
	AddItemIndex = 0;

	/*
	** Call the base class init.
	*/
	MenuDialogClass::On_Init_Dialog ();
}


/***********************************************************************************************
 * AutoRestartProgressDialogClass::On_Command -- Message handler for dialog                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    ID of control that message refers to                                              *
 *           ID of message                                                                     *
 *           misc param                                                                        *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/6/2001 11:03AM ST : Created                                                            *
 *=============================================================================================*/
void AutoRestartProgressDialogClass::On_Command (int ctrl_id, int message_id, DWORD param)
{
	switch (ctrl_id)
	{
		case IDCANCEL:
			AutoRestart.Cancel();
			break;
	}

	MenuDialogClass::On_Command (ctrl_id, message_id, param);
}



/***********************************************************************************************
 * AutoRestartProgressDialogClass::Add_Text -- Adds text to the progress info box              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to wide text string                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/6/2001 11:04AM ST : Created                                                            *
 *=============================================================================================*/
void AutoRestartProgressDialogClass::Add_Text(wchar_t *txt)
{
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item(IDC_PROGRESS_INFO);
	if (list_ctrl) {
		list_ctrl->Insert_Entry(AddItemIndex++, txt);
	}
}
