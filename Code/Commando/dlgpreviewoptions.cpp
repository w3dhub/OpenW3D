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
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/commando/dlgpreviewoptions.cpp      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/21/01 11:06a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgpreviewoptions.h"
#include "renegadedialog.h"
#include "listctrl.h"


////////////////////////////////////////////////////////////////
//
//	PreviewOptionsMenuClass
//
////////////////////////////////////////////////////////////////
PreviewOptionsMenuClass::PreviewOptionsMenuClass (void)	:
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_OPTIONS_PREVIEW))
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
PreviewOptionsMenuClass::On_Init_Dialog (void)
{
	//
	//	Get a pointer to the list control
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LIST_CTRL);
	if (list_ctrl != NULL) {

		//
		//	Configure the list control
		//
		list_ctrl->Add_Column (U_CHAR("Preview Title"), 1.0F, Vector3 (1, 1, 1));
	}


	MenuDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
PreviewOptionsMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	MenuDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}

