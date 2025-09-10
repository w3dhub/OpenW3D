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
 *                     $Archive:: /Commando/Code/Installer/LicenseDialog.cp $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 12/04/01 5:17p                $* 
 *                                                                                             * 
 *                    $Revision:: 7                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "LicenseDialog.h"
#include "CheckBoxCtrl.h"
#include "ErrorHandler.h"
#include "MessageBox.h"
#include "resource.h"
#include "Translator.h"


/***********************************************************************************************
 * LicenseDialogClass::On_Init_Dialog --																	  *
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
void LicenseDialogClass::On_Init_Dialog (void)
{
	Load_License ("License.txt");

	// Set initial check text to 'agree' but uncheck it so that the user must consiously check it.
	#if NDEBUG
	Check_Dlg_Button (IDC_LICENSE_CHECK, false);
	#else
	Check_Dlg_Button (IDC_LICENSE_CHECK, true);
	#endif

	Set_Dlg_Item_Text (IDC_LICENSE_CHECK, TxWideStringClass (IDS_AGREE_TO_TERMS));

	InstallMenuDialogClass::On_Init_Dialog();
}


/***********************************************************************************************
 * LicenseDialogClass::Load_License --																			  *
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
void LicenseDialogClass::Load_License (const char *licensefilename)
{
	HANDLE							file;
	BY_HANDLE_FILE_INFORMATION fileinformation;

	// Load up the license agreement text.
	// NOTE: This agreement should be in Unicode.
	file = CreateFile (licensefilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0L, NULL);
	
	// If the file exists...
	if (file != INVALID_HANDLE_VALUE) {

		if (!GetFileInformationByHandle (file, &fileinformation)) FATAL_SYSTEM_ERROR;

		// Does header exist?
		if ((fileinformation.nFileSizeHigh == 0) && (fileinformation.nFileSizeLow >= sizeof (wchar_t))) {

			int	 			length;
			unsigned long  bytesreadcount;
			wchar_t			  *buffer;

			length = fileinformation.nFileSizeLow / sizeof (wchar_t);
			buffer = new wchar_t [length + 1];
			if (ReadFile (file, buffer, length * sizeof (wchar_t), &bytesreadcount, NULL)) {

				unsigned s, d;

				buffer [length] = L'\0';
				
				// Advance past header.
				s = 1;

				// Strip 0x000D characters from the license text - they're not parsed as carriage return by the multi-line edit control.
				d = 0;
				while (true) {
					if (buffer [s] != 0x000D) {
						buffer [d] = buffer [s];
						d++;
					}
					if (buffer [s] == 0x000) break;
					s++;
				}

				Set_Dlg_Item_Text (IDC_LICENSE_EDIT, buffer);
			}
			delete [] buffer;
		}
		if (!CloseHandle (file)) FATAL_SYSTEM_ERROR;
	}
}


/***********************************************************************************************
 * LicenseDialogClass::On_Command --																			  *
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
void LicenseDialogClass::On_Command (int ctrl_id, int message_id, DWORD param)
{
	switch (ctrl_id)
	{
		case IDC_LICENSE_CHECK:
		{
			CheckBoxCtrlClass *check =	Get_Dlg_Item (IDC_LICENSE_CHECK)->As_CheckBoxCtrlClass();
	
			if (check->Get_Check()) {
				check->Set_Text (TxWideStringClass (IDS_AGREE_TO_TERMS));
			} else {
				check->Set_Text (TxWideStringClass (IDS_NOT_AGREE_TO_TERMS));
			}
			break;
		}

		case IDOK:
		{
			CheckBoxCtrlClass *check =	Get_Dlg_Item (IDC_LICENSE_CHECK)->As_CheckBoxCtrlClass();

			if (!check->Get_Check()) {
				MessageBoxClass::Do_Dialog (TxWideStringClass (IDS_WARNING), TxWideStringClass (IDS_MUST_ACCEPT), MessageBoxClass::MESSAGE_BOX_TYPE_OK);
				return;
			} else {
				break;
			}
		}

		default:
			break;
	}

	InstallMenuDialogClass::On_Command (ctrl_id, message_id, param);
}


/***********************************************************************************************
 * NDADialogClass::On_Init_Dialog --																			  *
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
void NDADialogClass::On_Init_Dialog (void)
{
	Load_License ("NDA.txt");

	Set_Dlg_Item_Text (IDC_LICENSE_HEADING, RxWideStringClass (IDS_NON_DISCLOSURE_AGREEMENT));
	Set_Dlg_Item_Text (IDC_LICENSE_QUERY, RxWideStringClass (IDS_AGREE_TO_NDA));

	// Set initial check text to 'agree' but uncheck it so that the user must consiously check it.
	#if NDEBUG
	Check_Dlg_Button (IDC_LICENSE_CHECK, false);
	#else
	Check_Dlg_Button (IDC_LICENSE_CHECK, true);
	#endif

	Set_Dlg_Item_Text (IDC_LICENSE_CHECK, TxWideStringClass (IDS_AGREE_TO_TERMS));

	InstallMenuDialogClass::On_Init_Dialog();
}


