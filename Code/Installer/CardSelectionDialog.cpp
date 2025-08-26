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
 *                 Project Name : Installer                                                    * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Installer/CardSelectionDialog.cpp          $  * 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 11/09/01 1:07p                $* 
 *                                                                                             * 
 *                    $Revision:: 3                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "CardSelectionDialog.h"
#include "ErrorHandler.h"
#include "listctrl.h"
#include "MessageBox.h"
#include "RDDesc.h"
#include "Translator.h"


/***********************************************************************************************
 * CardSelectionDialogClass::On_Init_Dialog --																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void CardSelectionDialogClass::On_Init_Dialog (void)
{
	ListCtrlClass *listctrl;
	int				e;
	
	listctrl = Get_Dlg_Item (IDC_CARD_SELECTION_LIST)->As_ListCtrlClass();
	listctrl->Add_Column (L"", 1.0f, Vector3 (1.0f, 1.0f, 1.0f));
	e = 0;
	for (int d = 0; d < WW3D::Get_Render_Device_Count(); d++) {
		listctrl->Insert_Entry (e, WideStringClass (WW3D::Get_Render_Device_Name (d)));
		e++;
	}

	listctrl->Set_Curr_Sel (0);

	MenuDialogClass::On_Init_Dialog();

	// Show message to indicate that the user's last video card selection was unsuccessful?
	if (ShowMessage) {
		MessageBoxClass::Do_Dialog (TxWideStringClass (IDS_WARNING), TxWideStringClass (IDS_CANNOT_INITIALIZE_THIS_D3D_DEVICE), MessageBoxClass::MESSAGE_BOX_TYPE_OK, this);
	}
}


/***********************************************************************************************
 * CardSelectionDialogClass::Callback --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void CardSelectionDialogClass::Callback (int id, PopupDialogClass *popup)
{
	switch (id) {
		
		case IDC_BUTTON_YES:
			CancelApplication = true;
			End_Dialog();
			break;

		default:
			break;
	}
}


/***********************************************************************************************
 * CardSelectionDialogClass::On_Command --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void CardSelectionDialogClass::On_Command (int ctrl_id, int message_id, DWORD param)
{
	switch (ctrl_id)
	{
		case IDCANCEL:
			MessageBoxClass::Do_Dialog (TxWideStringClass (IDS_EXIT_SETUP), TxWideStringClass (IDS_PROMPT_EXIT_SETUP), MessageBoxClass::MESSAGE_BOX_TYPE_YES_NO, this);
			return;

		case IDOK:
		{
			ListCtrlClass *listctrl;

			listctrl = Get_Dlg_Item (IDC_CARD_SELECTION_LIST)->As_ListCtrlClass();
			CardSelection = listctrl->Get_Curr_Sel();
			End_Dialog();
		}

		default:
			break;
	}

	MenuDialogClass::On_Command (ctrl_id, message_id, param);
}
