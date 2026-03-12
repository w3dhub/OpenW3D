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
 *                     $Archive:: /Commando/Code/Installer/SerialDialog.h $*
 *                                                                                             *
 *                      $Author:: Ian_l                   $*
 *                                                                                             *
 *                     $Modtime:: 11/12/01 7:35p                $*
 *                                                                                             *
 *                    $Revision:: 4                     $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _SERIAL_DIALOG_H
#define _SERIAL_DIALOG_H

// Includes.
#include "InstallMenuDialog.h"


// Dialog to obtain serial number from user.

// Classes.
class SerialDialogClass : public InstallMenuDialogClass
{
	public:
		SerialDialogClass() : InstallMenuDialogClass (IDD_DIALOG_SERIAL) {}

		// RTTI.
		void *As_SerialDialogClass() {return (this);}

		void On_Command (int ctrl_id, int message_id, DWORD param);

		bool Get_Serial_Number (StringClass &serialnumber);

	protected:

		void On_Init_Dialog (void);
		void On_Unicode_Char (uint16 unicode);
};


#endif // _SERIAL_DIALOG_H
