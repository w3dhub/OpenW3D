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
 *                     $Archive:: /Commando/Code/Installer/FinalDialog.h  $*
 *                                                                                             *
 *                      $Author:: Ian_l                   $*
 *                                                                                             *
 *                     $Modtime:: 10/22/01 8:51p                $*
 *                                                                                             *
 *                    $Revision:: 3                     $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _FINAL_DIALOG_H
#define _FINAL_DIALOG_H

// Includes.
#include "InstallMenuDialog.h"


// Dialog to instigate sign-up for a Westwood Online account.

// Classes.
class FinalDialogClass : public InstallMenuDialogClass
{
	public:
		FinalDialogClass (bool showbackbutton) : InstallMenuDialogClass (IDD_DIALOG_FINAL), ShowBackButton (showbackbutton) {}

		// RTTI.
		void *As_FinalDialogClass()			{return (this);}

		void On_Command (int ctrl_id, int message_id, DWORD param);

		bool Run_Game()			{return (Is_Dlg_Button_Checked (IDC_FINAL_CHECK1));}
		bool Display_Readme()	{return (Is_Dlg_Button_Checked (IDC_FINAL_CHECK2));}

	protected:
		void FinalDialogClass::On_Init_Dialog (void);

		bool ShowBackButton;
};


#endif // _FINAL_DIALOG_H
