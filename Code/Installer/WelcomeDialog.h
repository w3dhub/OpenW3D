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
 *                     $Archive:: /Commando/Code/Installer/WelcomeDialog.h $*
 *                                                                                             *
 *                      $Author:: Ian_l                   $*
 *                                                                                             *
 *                     $Modtime:: 9/25/01 12:06p                $*
 *                                                                                             *
 *                    $Revision:: 3                     $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _WELCOME_DIALOG_H
#define _WELCOME_DIALOG_H

// Includes.
#include "InstallMenuDialog.h"


// Dialog to welcome user to the installation and to display copyright message.

// Classes.
class WelcomeDialogClass : public InstallMenuDialogClass
{
	public:
		WelcomeDialogClass() : InstallMenuDialogClass (IDD_DIALOG_WELCOME) {}

		// RTTI.
		void *As_WelcomeDialogClass()			{return (this);}

	protected:
		void On_Init_Dialog (void);
};


#endif // _WELCOME_DIALOG_H
