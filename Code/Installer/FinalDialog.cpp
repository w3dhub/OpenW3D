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
 *                     $Archive:: /Commando/Code/Installer/FinalDialog.cpp $*
 *                                                                                             *
 *                      $Author:: Ian_l                   $*
 *                                                                                             *
 *                     $Modtime:: 11/09/01 10:42p               $*
 *                                                                                             *
 *                    $Revision:: 5                     $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "FinalDialog.h"
#include "ButtonCtrl.h"
#include "CheckBoxCtrl.h"
#include "Installer.h"
#include "Translator.h"


/***********************************************************************************************
 * FinalDialogClass::On_Init_Dialog --																			  *
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
void FinalDialogClass::On_Init_Dialog (void)
{
	switch ((_Installer.Install_Game() ? 1 : 0) + (_Installer.Install_WOL() ? 2 : 0)) {

		case 0:
			// Should never occur. At least one component should have been installed.
			WWASSERT (false);
			break;

		case 1:
			Set_Dlg_Item_Text (IDC_FINAL_STATIC1, TxWideStringClass (IDS_GAME_INSTALLED));
			break;

		case 2:
			Set_Dlg_Item_Text (IDC_FINAL_STATIC1, TxWideStringClass (IDS_WOL_INSTALLED));
			break;

		case 3:
			Set_Dlg_Item_Text (IDC_FINAL_STATIC1, TxWideStringClass (IDS_GAME_AND_WOL_INSTALLED));
			break;
	}

	// Show the back button? If false this suggests that there is not a dialog to go back to.
	if (!ShowBackButton) {

		ButtonCtrlClass *backbutton;

		backbutton = Get_Dlg_Item (IDC_BUTTON_BACK)->As_ButtonCtrlClass();
		backbutton->Show (false);
		backbutton->Enable (false);
	}

	if (_Installer.Install_Game()) {
		Check_Dlg_Button (IDC_FINAL_CHECK1, true);
	} else {
		Check_Dlg_Button (IDC_FINAL_CHECK3, true);
		Enable_Dlg_Item (IDC_FINAL_CHECK1, false);
		Enable_Dlg_Item (IDC_FINAL_CHECK2, false);
	}

	Set_Dlg_Item_Text (IDC_FINAL_STATIC3, TxWideStringClass (IDS_FINAL_NOTES));

	InstallMenuDialogClass::On_Init_Dialog();
}


/***********************************************************************************************
 * FinalDialogClass::On_Command --																				  *
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
void FinalDialogClass::On_Command (int ctrl_id, int message_id, DWORD param)
{
	static bool _disabled = false;

	// Implement radio controls with the three check boxes by making them mutually exclusive.
	// NOTE: Unlike Windows GUI, a control can always get focus regardless of its checked status.
	if (!_disabled) {

		CheckBoxCtrlClass *check1, *check2, *check3;
		bool					 setcheckcontrols;

		setcheckcontrols = false;
		switch (ctrl_id) {

			case IDC_FINAL_CHECK1:
				check1 = Get_Dlg_Item (IDC_FINAL_CHECK1)->As_CheckBoxCtrlClass();
				check2 = Get_Dlg_Item (IDC_FINAL_CHECK2)->As_CheckBoxCtrlClass();
				check3 = Get_Dlg_Item (IDC_FINAL_CHECK3)->As_CheckBoxCtrlClass();
				setcheckcontrols = true;
				break;

			case IDC_FINAL_CHECK2:
				check1 = Get_Dlg_Item (IDC_FINAL_CHECK2)->As_CheckBoxCtrlClass();
				check2 = Get_Dlg_Item (IDC_FINAL_CHECK1)->As_CheckBoxCtrlClass();
				check3 = Get_Dlg_Item (IDC_FINAL_CHECK3)->As_CheckBoxCtrlClass();
				setcheckcontrols = true;
				break;

			case IDC_FINAL_CHECK3:
				check1 = Get_Dlg_Item (IDC_FINAL_CHECK3)->As_CheckBoxCtrlClass();
				check2 = Get_Dlg_Item (IDC_FINAL_CHECK1)->As_CheckBoxCtrlClass();
				check3 = Get_Dlg_Item (IDC_FINAL_CHECK2)->As_CheckBoxCtrlClass();
				setcheckcontrols = true;
				break;

			case IDCANCEL:
				MenuDialogClass::On_Command (ctrl_id, message_id, param);
				return;

			default:
				break;
		}

		if (setcheckcontrols) {

			// Prevent recursion on this function by 'disabling' it.
			_disabled = true;

			if (check1->Get_Check()) {
				check2->Set_Check (false);
				check3->Set_Check (false);
			} else {

				// If it has just been unchecked then check it again - user must
				// not be able to uncheck a control by clicking on it.
				check1->Set_Check (true);
			}

			_disabled = false;
		}
	}

	InstallMenuDialogClass::On_Command (ctrl_id, message_id, param);
}
