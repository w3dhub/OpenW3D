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
 *                     $Archive:: /Commando/Code/Commando/dlgmpwoldeletebuddy.cpp     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/17/01 11:22a                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "dlgmpwoldeletebuddy.h"
#include "renegadedialog.h"
#include "WOLBuddyMgr.h"


////////////////////////////////////////////////////////////////
//
//	MPWolDeleteBuddyPopupClass
//
////////////////////////////////////////////////////////////////
MPWolDeleteBuddyPopupClass::MPWolDeleteBuddyPopupClass (void)	:
	PopupDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MP_WOL_DELETE_BUDDY))
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
MPWolDeleteBuddyPopupClass::On_Init_Dialog (void)
{
	Set_Dlg_Item_Text(IDC_BUDDY_NAME_STATIC, UserName);
	PopupDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
MPWolDeleteBuddyPopupClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDC_DELETE_BUTTON:
		{	
			//
			//	Simply delete the user from our list
			//		
			if (UserName.Is_Empty () == false) {
				WOLBuddyMgr* buddyMgr = WOLBuddyMgr::GetInstance(false);

				if (buddyMgr) {
					buddyMgr->RemoveBuddy(UserName);
					buddyMgr->Release_Ref();
				}
				End_Dialog ();
			}

			break;
		}
	}

	PopupDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}

