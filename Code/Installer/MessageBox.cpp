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
 *                     $Archive:: /Commando/Code/Installer/MessageBox.cpp $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 11/29/01 3:54p                $* 
 *                                                                                             * 
 *                    $Revision:: 4                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "ErrorHandler.h"
#include "MessageBox.h"
#include "InstallMenuDialog.h"
#include "resource.h"


/***********************************************************************************************
 * MessageBoxClass::Create_Dialog --																			  *
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
MessageBoxClass *MessageBoxClass::Create_Dialog (const wchar_t *title, const wchar_t *text, MessageBoxTypeEnum type, CallbackMenuDialogClass *callbackobject)
{
	MessageBoxClass *popup;
		
	switch (type) {
		
		case MESSAGE_BOX_TYPE_OK:
			popup = NEW_REF (MessageBoxClass, (IDD_DIALOG_MESSAGE_BOX_OK, callbackobject));
			break;

		case MESSAGE_BOX_TYPE_YES_NO:
			popup = NEW_REF (MessageBoxClass, (IDD_DIALOG_MESSAGE_BOX_YES_NO, callbackobject));
			break;

		case MESSAGE_BOX_TYPE_RETRY_QUIT:
			popup = NEW_REF (MessageBoxClass, (IDD_DIALOG_MESSAGE_BOX_RETRY_QUIT, callbackobject));
			break;
	};

	popup->Start_Dialog();
	popup->Set_Title (title);
	popup->Set_Dlg_Item_Text (IDC_MESSAGE_BOX_STATIC, text);
	popup->Build_Background_Renderers();

	return (popup);
}


/***********************************************************************************************
 * MessageBoxClass::Do_Dialog --																					  *
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
void MessageBoxClass::Do_Dialog (const wchar_t *title, const wchar_t *text, MessageBoxTypeEnum type, CallbackMenuDialogClass *callbackobject)
{
	MessageBoxClass *popup;
		
	popup = Create_Dialog (title, text, type, callbackobject);
	REF_PTR_RELEASE (popup);
}


/***********************************************************************************************
 * MessageBoxClass::On_Command --																				  *
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
void MessageBoxClass::On_Command (int ctrl_id, int message_id, DWORD param)
{
	if (CallbackObject != NULL) {
		CallbackObject->Callback (ctrl_id, this);
	}

	End_Dialog();
}


/***********************************************************************************************
 * CallbackMenuDialogClass::Callback --																		  *
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
void CallbackMenuDialogClass::Callback (int id, PopupDialogClass *popup)
{
	switch (id) {
		
		case IDC_BUTTON_YES:
			MenuDialogClass::On_Command (IDCANCEL, 0, 0);
			break;

		default:
			break;
	}
}

