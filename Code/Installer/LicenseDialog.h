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
 *                     $Archive:: /Commando/Code/Installer/LicenseDialog. $*
 *                                                                                             *
 *                      $Author:: Ian_l                   $*
 *                                                                                             *
 *                     $Modtime:: 12/04/01 4:56p                $*
 *                                                                                             *
 *                    $Revision:: 3                     $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _LICENSE_DIALOG_H
#define _LICENSE_DIALOG_H

// Includes.
#include "InstallMenuDialog.h"

// Dialog to display the Software License Agreement. User must agree to these terms
// before proceeding.

// Classes.
class LicenseDialogClass : public InstallMenuDialogClass
{
	public:
		LicenseDialogClass() : InstallMenuDialogClass (IDD_DIALOG_LICENSE) {}

		// RTTI.
		void *As_LicenseDialogClass()		{return (this);}

		void On_Command (int ctrl_id, int message_id, DWORD param);

	protected:

		void Load_License (const char *licensefilename);
		void On_Init_Dialog (void);
};


class NDADialogClass : public LicenseDialogClass
{
	public:
		NDADialogClass() : LicenseDialogClass() {}

		// RTTI.
		void *As_NDADialogClass()		{return (this);}

	protected:
		void On_Init_Dialog (void);
};


#endif // _LICENSE_DIALOG_H
