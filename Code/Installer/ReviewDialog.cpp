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
 *                     $Archive:: /Commando/Code/Installer/ReviewDialog.cpp $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 11/24/01 9:29p                $* 
 *                                                                                             * 
 *                    $Revision:: 5                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "ReviewDialog.h"
#include "Installer.h"
#include "listctrl.h"
#include "Translator.h"


/***********************************************************************************************
 * ReviewDialogClass::On_Init_Dialog --																		  *		
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
void ReviewDialogClass::On_Init_Dialog (void)
{
	Set_Dlg_Item_Text (IDC_REVIEW_STATIC1, TxWideStringClass (IDS_REVIEW_SETTINGS));

	InstallMenuDialogClass::On_Init_Dialog();
}


/***********************************************************************************************
 * ReviewDialogClass::On_Activate --																			  *		
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
void ReviewDialogClass::On_Activate (bool onoff)
{
	if (onoff) {

		const wchar_t *formatstring = L"�%s: %s";

		WideStringClass reviewtext, substring;
		WideStringClass t;

		// NOTE: Items may have changed since this dialog was last active.
		if (_Installer.Install_Game()) {
			reviewtext += TxWideStringClass (IDS_INSTALL_GAME);
			reviewtext += L"\n";
			substring.Format (formatstring, TxWideStringClass (IDS_DESTINATION_DIRECTORY), _Installer.Get_Target_Game_Path (t));
			reviewtext += substring;
			reviewtext += L"\n";
			substring.Format (formatstring, TxWideStringClass (IDS_PROGRAM_FOLDER), _Installer.Get_Target_Game_Folder (t));
			reviewtext += substring;
			reviewtext += L"\n\n";

			if (_Installer.Install_Game_Shortcut()) {
				reviewtext += TxWideStringClass (IDS_PLACE_SHORTCUT);
				reviewtext += L"\n\n";
			}
		}

		if (_Installer.Install_WOL()) {
			reviewtext += TxWideStringClass (IDS_INSTALL_WOL);
			reviewtext += L"\n";
			substring.Format (formatstring, TxWideStringClass (IDS_DESTINATION_DIRECTORY), _Installer.Get_Target_WOL_Path (t));
			reviewtext += substring;
			reviewtext += L"\n";
			substring.Format (formatstring, TxWideStringClass (IDS_PROGRAM_FOLDER), _Installer.Get_Target_WOL_Folder (t));
			reviewtext += substring;
			reviewtext += L"\n\n";

			if (_Installer.Use_IGR_Settings()) {
				reviewtext += TxWideStringClass (IDS_USE_GAME_ROOM_SETTINGS);
				reviewtext += L"\n\n";
			}
		}

		Set_Dlg_Item_Text (IDC_REVIEW_EDIT, reviewtext);
	}

	InstallMenuDialogClass::On_Activate (onoff);
}


/***********************************************************************************************
 * ReviewDialogClass::On_Command --																				  *
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
void ReviewDialogClass::On_Command (int ctrl_id, int message_id, DWORD param)
{
	InstallMenuDialogClass::On_Command (ctrl_id, message_id, param);
}
