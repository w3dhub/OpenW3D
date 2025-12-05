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
 *                     $Archive:: /Commando/Code/Commando/dlgmpwolpagebuddy.h       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/21/01 11:02a                                             $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLG_MP_WOL_PAGE_BUDDY_H
#define __DLG_MP_WOL_PAGE_BUDDY_H

#include "popupdialog.h"
#include "WOLBuddyMgr.h"

////////////////////////////////////////////////////////////////
//
//	MPWolPageBuddyPopupClass
//
////////////////////////////////////////////////////////////////
class MPWolPageBuddyPopupClass :
	public PopupDialogClass,
	protected Observer<WOLBuddyMgrEvent>
{
public:
	MPWolPageBuddyPopupClass(void);
	~MPWolPageBuddyPopupClass(void);

	void Set_Buddy_Name(const wchar_t* user_name);

protected:

	void On_Init_Dialog(void) override;
	void On_Command(int ctrl_id, int mesage_id, unsigned int param) override;

	void Send_Page(void);
	void CheckIfCanSendPage(void);
	
	void On_ComboBoxCtrl_Edit_Change(ComboBoxCtrlClass* combo, int id) override;
	void On_EditCtrl_Change(EditCtrlClass* edit, int id) override;
	void On_EditCtrl_Enter_Pressed(EditCtrlClass* edit, int id) override;

	void HandleNotification(WOLBuddyMgrEvent& event) override;

	WOLBuddyMgr* mBuddyMgr;
};

#endif //__DLG_MP_WOL_PAGE_BUDDY_H
