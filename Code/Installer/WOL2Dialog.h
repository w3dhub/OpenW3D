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
 *                     $Archive:: /Commando/Code/Installer/WOL2Dialog.h $*
 *                                                                                             *
 *                      $Author:: Ian_l                   $*
 *                                                                                             *
 *                     $Modtime:: 10/09/01 8:24p                $*
 *                                                                                             *
 *                    $Revision:: 2                     $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _WOL2_DIALOG_H
#define _WOL2_DIALOG_H

// Includes.
#include "InstallMenuDialog.h"


// Dialog to instigate sign-up for a Westwood Online account.

// Classes.
class WOL2DialogClass : public InstallMenuDialogClass
{
	public:
		WOL2DialogClass() : InstallMenuDialogClass (IDD_DIALOG_WOL2) {}

		// RTTI.
		void *As_WOL2DialogClass()			{return (this);}

		void On_Command (int ctrl_id, int message_id, DWORD param);

	protected:

		void			 On_Init_Dialog (void);
		StringClass &Encrypt_Password (const StringClass &password, StringClass &encryptedpassword);
};


#endif // _WOL2_DIALOG_H
