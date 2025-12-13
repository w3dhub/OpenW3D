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
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : commando                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/commando/dlgmpwolmotd.cpp                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/15/01 9:55p                                                $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgmpwolmotd.h"
#include "renegadedialog.h"
#include "mpsettingsmgr.h"
#include "listctrl.h"


//////////////////////////////////////////////////////////////////////
//
//	MPWolMOTDDialogClass
//
//////////////////////////////////////////////////////////////////////
MPWolMOTDDialogClass::MPWolMOTDDialogClass (void)	:
	PopupDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MP_WOL_MOTD))
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~MPWolMOTDDialogClass
//
//////////////////////////////////////////////////////////////////////
MPWolMOTDDialogClass::~MPWolMOTDDialogClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
//////////////////////////////////////////////////////////////////////
void
MPWolMOTDDialogClass::On_Init_Dialog (void)
{	
	//
	//	Get a pointer to the list control
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LIST_CTRL);
	if (list_ctrl != NULL) {

		//
		//	Configure the column
		//
		list_ctrl->Add_Column (L"", 1.0F, Vector3 (1, 1, 1));

		WideStringClass temp_copy (Message, true);
		wchar_t *message			= temp_copy.Peek_Buffer ();
		wchar_t *entry_text		= message;

		//
		//	Keep going until we run out of text
		//
		while (*message != 0) {

			//
			//	Do we need to insert a new entry into the control?
			//
			if (message[0] == L'\r' && message[1] == L'\n') {
				message[0] = 0;			
				list_ctrl->Insert_Entry (0xFFFF, entry_text);			
				message ++;
				entry_text = message + 1;
			} else if (message[0] == L'\n') {
				message[0] = 0;			
				list_ctrl->Insert_Entry (0xFFFF, entry_text);
				entry_text = message + 1;
			}

			message ++;
		}

		//
		//	Make sure we get the end of the text
		//
		if (entry_text[0] != 0) {
			list_ctrl->Insert_Entry (0xFFFF, entry_text);
		}
	}

	PopupDialogClass::On_Init_Dialog ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Destroy
//
//////////////////////////////////////////////////////////////////////
void
MPWolMOTDDialogClass::On_Destroy (void)
{	
	PopupDialogClass::On_Destroy ();
	return ;
}
