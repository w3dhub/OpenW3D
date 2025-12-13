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
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/dlgsidebarhelp.cpp                  $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/15/01 1:24p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgsidebarhelp.h"
#include "renegadedialog.h"
#include "mpsettingsmgr.h"


//////////////////////////////////////////////////////////////////////
//
//	SidebarHelpDialogClass
//
//////////////////////////////////////////////////////////////////////
SidebarHelpDialogClass::SidebarHelpDialogClass (void)	:
	PopupDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_SIDEBAR_HELP_POPUP))
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~SidebarHelpDialogClass
//
//////////////////////////////////////////////////////////////////////
SidebarHelpDialogClass::~SidebarHelpDialogClass (void)
{
	return ;
}

//////////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
//////////////////////////////////////////////////////////////////////
void
SidebarHelpDialogClass::On_Init_Dialog (void)
{	
	Check_Dlg_Button (IDC_DONT_SHOW_AGAIN_CHECK, false);
	PopupDialogClass::On_Init_Dialog ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Command
//
//////////////////////////////////////////////////////////////////////
void
SidebarHelpDialogClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	//
	//	Remember the "don't show me this again" setting
	//
	if (ctrl_id == IDOK) {
		bool is_checked = Is_Dlg_Button_Checked (IDC_DONT_SHOW_AGAIN_CHECK);
		MPSettingsMgrClass::Set_Is_Sidebar_Help_Displayed (!is_checked);

		//
		//	Close the dialog
		//
		End_Dialog ();
	}

	PopupDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}
