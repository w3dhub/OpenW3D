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
 *                     $Archive:: /Commando/Code/Installer/WhatToInstallDialog. $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 10/25/01 3:38p                $* 
 *                                                                                             * 
 *                    $Revision:: 5                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "WhatToInstallDialog.h"
#include "CheckBoxCtrl.h"
#include "resource.h"
#include "Installer.h"
#include "Translator.h"


/***********************************************************************************************
 * WhatToInstallDialogClass::On_Init_Dialog --																  *
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
void WhatToInstallDialogClass::On_Init_Dialog (void)
{
	CheckBoxCtrlClass *check;
	bool					 useigrsettings;

	Set_Dlg_Item_Text (IDC_WHAT_TO_INSTALL_CHECK1, TxWideStringClass (IDS_INSTALL_GAME));
	Set_Dlg_Item_Text (IDC_WHAT_TO_INSTALL_CHECK2, TxWideStringClass (IDS_INSTALL_WOL));
	Set_Dlg_Item_Text (IDC_WHAT_TO_INSTALL_CHECK3, TxWideStringClass (IDS_PLACE_SHORTCUT));
	Set_Dlg_Item_Text (IDC_WHAT_TO_INSTALL_CHECK4, TxWideStringClass (IDS_USE_GAME_ROOM_SETTINGS));

	Check_Dlg_Button (IDC_WHAT_TO_INSTALL_CHECK1, true);
	Check_Dlg_Button (IDC_WHAT_TO_INSTALL_CHECK2, _Installer.Is_Target_WOL_Older_Than_Source());
	Check_Dlg_Button (IDC_WHAT_TO_INSTALL_CHECK3, true);

	check = Get_Dlg_Item (IDC_WHAT_TO_INSTALL_CHECK4)->As_CheckBoxCtrlClass();
	if (_Installer.Can_Use_IGR_Settings()) {

		// If there is already a setting in the registry then use it - otherwise set it by default.
		if (_RegistryManager.Use_IGR_Settings (useigrsettings)) {
			check->Set_Check (useigrsettings);
		} else {
			check->Set_Check (true);
		}

	} else {

		// Don't even show this option to the user, and disable it unconditionally.
		check->Show (false);
		check->Set_Check (false);
	}
	check->Enable (Is_Dlg_Button_Checked (IDC_WHAT_TO_INSTALL_CHECK2));

	InstallMenuDialogClass::On_Init_Dialog();
}


/***********************************************************************************************
 * WhatToInstallDialogClass::On_Command --																	  *
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
void WhatToInstallDialogClass::On_Command (int ctrl_id, int message_id, DWORD param)
{
	switch (ctrl_id) {

		case IDOK:
		{	
			// If neither of the first two options have been selected then ask the user if he wishes to cancel.
			if (!(Is_Dlg_Button_Checked (IDC_WHAT_TO_INSTALL_CHECK1) || Is_Dlg_Button_Checked (IDC_WHAT_TO_INSTALL_CHECK2))) {
				InstallMenuDialogClass::On_Command (IDCANCEL, 0, 0);
				return;
			}
			break;
		}

		case IDC_WHAT_TO_INSTALL_CHECK1:
			
			// Disable option 3 (create game icon) if option 1 (install game) is not checked.
			Enable_Dlg_Item (IDC_WHAT_TO_INSTALL_CHECK3, Is_Dlg_Button_Checked (IDC_WHAT_TO_INSTALL_CHECK1));
			break;

		case IDC_WHAT_TO_INSTALL_CHECK2:

			// Disable option 4 (use game room settings) if the user is not installing WOL.
			Enable_Dlg_Item (IDC_WHAT_TO_INSTALL_CHECK4, Is_Dlg_Button_Checked (IDC_WHAT_TO_INSTALL_CHECK2));

			// If the user wishes to install WOL but the source version is not newer than the current version
			// then issue a warning.
			if (Is_Dlg_Button_Checked (IDC_WHAT_TO_INSTALL_CHECK2) && (!_Installer.Is_Target_WOL_Older_Than_Source())) {
				MessageBoxClass::Do_Dialog (TxWideStringClass (IDS_WARNING), TxWideStringClass (IDS_SOURCE_WOL_OLDER), MessageBoxClass::MESSAGE_BOX_TYPE_OK, this);
				return;
			}

		default:
			break;
	}

	InstallMenuDialogClass::On_Command (ctrl_id, message_id, param);
}
