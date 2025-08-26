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
 *                     $Archive:: /Commando/Code/Installer/CardSelectionDialog.h $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 11/08/01 10:57p               $* 
 *                                                                                             * 
 *                    $Revision:: 3                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _CARD_SELECTION_DIALOG_H
#define _CARD_SELECTION_DIALOG_H


// Includes.
#include "menudialog.h"
#include "resource.h"
#include "MessageBox.h"
#include "WW3D.h"


// Classes.
class CardSelectionDialogClass : public CallbackMenuDialogClass
{
	public:
		 CardSelectionDialogClass (bool showmessage) : CallbackMenuDialogClass (IDD_DIALOG_CARD_SELECTION), CardSelection (-1), CancelApplication (false), ShowMessage (showmessage) {}
		~CardSelectionDialogClass() {}

		void On_Command (int ctrl_id, int message_id, DWORD param);
		void Callback (int id, PopupDialogClass *popup);

		int  Get_Card_Count()		{return (WW3D::Get_Render_Device_Count());}
		int  Get_Card_Selection()	{return (CardSelection);}
		bool Cancel_Application()	{return (CancelApplication);}

	protected:
		void On_Init_Dialog (void);

		int  CardSelection;
		bool CancelApplication;
		bool ShowMessage;
};


#endif // _CARD_SELECTION_DIALOG_H