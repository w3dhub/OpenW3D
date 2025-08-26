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
 *                     $Archive:: /Commando/Code/Installer/WOL2Dialog.cp $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 11/24/01 9:27p                $* 
 *                                                                                             * 
 *                    $Revision:: 3                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "WOL2Dialog.h"
#include "Installer.h"
#include "listctrl.h"
#include "resource.h"
#include "Translator.h"


// Defines.
#define ENCRYPTED_STRING_LENGTH	8


/***********************************************************************************************
 * WOL2DialogClass::On_Init_Dialog --																			  *
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
void WOL2DialogClass::On_Init_Dialog (void)
{
	ListCtrlClass	 *listctrl;
	unsigned			  i;
	WideStringClass  name, password;

	// Get the list of nicknames.
	listctrl = Get_Dlg_Item (IDC_WOL2_LIST)->As_ListCtrlClass();
	listctrl->Add_Column (L"", 1.0f, Vector3 (1.0f, 1.0f, 1.0f));
	i = 0;
	while (_Installer.Get_WOL_Account (i, name, password)) {
		listctrl->Insert_Entry (i, name);
		i++;
	}

	// Select the topmost item.
	listctrl->Set_Curr_Sel (0);
}


/***********************************************************************************************
 * WOL2DialogClass::On_Command --																				  *
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
void WOL2DialogClass::On_Command (int ctrl_id, int message_id, DWORD param)
{
	switch (ctrl_id) {

		case IDCANCEL:
			MenuDialogClass::On_Command (ctrl_id, message_id, param);
			return;

		case IDOK:
		{
			ListCtrlClass	 *listctrl;
			WideStringClass  userpassword (Get_Dlg_Item_Text (IDC_WOL2_EDIT));
			WideStringClass  accountname, accountpassword;
			StringClass		  encrypteduserpassword;

			// Verify that the user password is the correct length.
			if (userpassword.Get_Length() != ENCRYPTED_STRING_LENGTH) {
				MessageBoxClass::Do_Dialog (TxWideStringClass (IDS_WARNING), TxWideStringClass (IDS_INVALID_PASSWORD), MessageBoxClass::MESSAGE_BOX_TYPE_OK, this);
				return;
			}

			// Verify that the user password is alphanumeric ie. do not allow any punctuation or Asian characters.
			for (int i = 0; i < userpassword.Get_Length(); i++) {
				if (!iswalnum (userpassword [i])) {
					MessageBoxClass::Do_Dialog (TxWideStringClass (IDS_WARNING), TxWideStringClass (IDS_INVALID_PASSWORD), MessageBoxClass::MESSAGE_BOX_TYPE_OK, this);
					return;
				}
			}

			// Verify that the user password is correct for the selected account name.
			listctrl = Get_Dlg_Item (IDC_WOL2_LIST)->As_ListCtrlClass();
			_Installer.Get_WOL_Account (listctrl->Get_Curr_Sel(), accountname, accountpassword);
			Encrypt_Password (StringClass (userpassword), encrypteduserpassword);
			if (accountpassword.Compare (WideStringClass (encrypteduserpassword)) != 0) {
				MessageBoxClass::Do_Dialog (TxWideStringClass (IDS_WARNING), TxWideStringClass (IDS_INCORRECT_PASSWORD), MessageBoxClass::MESSAGE_BOX_TYPE_OK, this);
				return;
			}
			
			_Installer.Set_Preferred_WOL_Account (accountname);
			break;
		}
	}

	InstallMenuDialogClass::On_Command (ctrl_id, message_id, param);
}


/***********************************************************************************************
 * WOL2DialogClass::Encrypt_Password --																		  *
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
StringClass &WOL2DialogClass::Encrypt_Password (const StringClass &password, StringClass &encryptedpassword)
{
	const static char _basestring	[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
	
	char		tempbuffer [ENCRYPTED_STRING_LENGTH + 1];
	unsigned upcnt = 0, dncnt = 0, cnt = 0;

	WWASSERT (password.Get_Length() == ENCRYPTED_STRING_LENGTH);

	for (upcnt = 0, dncnt = ENCRYPTED_STRING_LENGTH; upcnt < ENCRYPTED_STRING_LENGTH; upcnt++, dncnt--) {
		if (*(password.Peek_Buffer() + upcnt) & 0x01) {
			tempbuffer [upcnt] = (*(password.Peek_Buffer() + upcnt) << (*(password.Peek_Buffer() + upcnt) & 0x01)) & *(password.Peek_Buffer() + dncnt);
		} else {
			tempbuffer [upcnt] = (*(password.Peek_Buffer() + upcnt) << (*(password.Peek_Buffer() + upcnt) & 0x01)) ^ *(password.Peek_Buffer() + dncnt);
		}
	}

	encryptedpassword.Get_Buffer (ENCRYPTED_STRING_LENGTH + 1);

	for (cnt = 0; cnt < ENCRYPTED_STRING_LENGTH; cnt++) {
		*(encryptedpassword.Peek_Buffer() + cnt) = _basestring [tempbuffer [cnt] & 0x3f];
	}
	*(encryptedpassword.Peek_Buffer() + cnt) = '\0';

	return (encryptedpassword);
}
