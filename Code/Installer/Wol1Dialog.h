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
 *                     $Archive:: /Commando/Code/Installer/Wol1Dialog.h $*
 *                                                                                             *
 *                      $Author:: Ian_l                   $*
 *                                                                                             *
 *                     $Modtime:: 11/09/01 10:31p               $*
 *                                                                                             *
 *                    $Revision:: 3                     $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _WOL1_DIALOG_H
#define _WOL1_DIALOG_H

// Includes.
#include "InstallMenuDialog.h"


// Dialog to instigate sign-up for a Westwood Online account.

// Classes.
class WOL1DialogClass : public InstallMenuDialogClass
{
	public:
		WOL1DialogClass() : InstallMenuDialogClass (IDD_DIALOG_WOL1) {}

		// RTTI.
		void *As_WOL1DialogClass()			{return (this);}

		void On_Command (int ctrl_id, int message_id, DWORD param);
};


#endif // _WOL1_DIALOG_H
