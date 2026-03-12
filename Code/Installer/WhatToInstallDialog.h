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
 *                     $Archive:: /Commando/Code/Installer/WhatToInstallDialog $*
 *                                                                                             *
 *                      $Author:: Ian_l                   $*
 *                                                                                             *
 *                     $Modtime:: 10/25/01 1:42p                $*
 *                                                                                             *
 *                    $Revision:: 4                     $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _WHAT_TO_INSTALL_H
#define _WHAT_TO_INSTALL_H

// Includes.
#include "InstallMenuDialog.h"


// Dialog to obtain from user the set of items to install.

// Classes.
class WhatToInstallDialogClass : public InstallMenuDialogClass
{
	public:
		WhatToInstallDialogClass() : InstallMenuDialogClass (IDD_DIALOG_WHAT_TO_INSTALL) {}

		// RTTI.
		void *As_WhatToInstallDialogClass()			{return (this);}

		void On_Command (int ctrl_id, int message_id, DWORD param);

		bool Install_Game()				{return (Is_Dlg_Button_Checked (IDC_WHAT_TO_INSTALL_CHECK1));}
		bool Install_WOL()				{return (Is_Dlg_Button_Checked (IDC_WHAT_TO_INSTALL_CHECK2));}
		bool Install_Game_Shortcut()	{return (Is_Dlg_Button_Checked (IDC_WHAT_TO_INSTALL_CHECK3));}
		bool Use_IGR_Settings()			{return (Is_Dlg_Button_Checked (IDC_WHAT_TO_INSTALL_CHECK4));}

	protected:

		void On_Init_Dialog (void);
};


#endif // _WHAT_TO_INSTALL_DIALOG_H
