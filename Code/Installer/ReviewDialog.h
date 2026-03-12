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
 *                     $Archive:: /Commando/Code/Installer/ReviewDialog.h $*
 *                                                                                             *
 *                      $Author:: Ian_l                   $*
 *                                                                                             *
 *                     $Modtime:: 11/03/01 5:53p                $*
 *                                                                                             *
 *                    $Revision:: 4                     $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _REVIEW_DIALOG_H
#define _REVIEW_DIALOG_H

// Includes.
#include "InstallMenuDialog.h"


// Dialog to confirm user's settings.

// Classes.
class ReviewDialogClass : public InstallMenuDialogClass
{
	public:
		ReviewDialogClass() : InstallMenuDialogClass (IDD_DIALOG_REVIEW) {}

		void On_Command (int ctrl_id, int message_id, DWORD param);

		// RTTI.
		void *As_ReviewDialogClass()		{return (this);}

	protected:
		void On_Init_Dialog (void);
		void On_Activate (bool onoff);
};


#endif // _REVIEW_DIALOG_H
