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
 *                     $Archive:: /Commando/Code/Installer/MessageBox.h  $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 11/28/01 12:50a               $* 
 *                                                                                             * 
 *                    $Revision:: 5                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _MESSAGE_BOX_H
#define _MESSAGE_BOX_H

// Includes.
#include "popupdialog.h"
#include "menudialog.h"


// Forward declarations.
class CallbackMenuDialogClass;


// Simple pop-up style message box.
class MessageBoxClass : public PopupDialogClass
{
	public:

		enum MessageBoxTypeEnum {

			MESSAGE_BOX_TYPE_OK,
			MESSAGE_BOX_TYPE_YES_NO,
			MESSAGE_BOX_TYPE_RETRY_QUIT
		};

		MessageBoxClass (int res_id, CallbackMenuDialogClass *callbackobject) : PopupDialogClass (res_id), CallbackObject (callbackobject) {}

		static MessageBoxClass *Create_Dialog (const wchar_t *title, const wchar_t *text, MessageBoxTypeEnum type, CallbackMenuDialogClass *callbackobject);
		static void					Do_Dialog (const wchar_t *title, const wchar_t *text, MessageBoxTypeEnum type, CallbackMenuDialogClass *callbackobject = NULL);

		void On_Command (int ctrl_id, int message_id, DWORD param);

	protected:

		CallbackMenuDialogClass *CallbackObject;
};


// Interface class to inherit if you want to be notified of a key press from the message box.
class CallbackMenuDialogClass : public MenuDialogClass
{
	public:
		
		CallbackMenuDialogClass (int resourceid) : MenuDialogClass (resourceid) {}

		virtual void Callback (int id, PopupDialogClass *popup);
};

#endif // _MESSAGE_BOX_H