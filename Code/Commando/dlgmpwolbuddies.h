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
 *                     $Archive:: /Commando/Code/Commando/dlgmpwolbuddies.h      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/30/01 5:27p                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLG_MP_WOL_BUDDIES_H
#define __DLG_MP_WOL_BUDDIES_H

#include "menudialog.h"
#include "WOLBuddyMgr.h"
#include "DlgMessageBox.h"


////////////////////////////////////////////////////////////////
//
//	MPWolBuddiesMenuClass
//
////////////////////////////////////////////////////////////////
class MPWolBuddiesMenuClass :
	public MenuDialogClass,
	public Observer<WOLBuddyMgrEvent>,
	public Observer<DlgMsgBoxEvent>
{
public:
	static void Display(void);
	
protected:
	MPWolBuddiesMenuClass(void);
	~MPWolBuddiesMenuClass();

	// Prevent copy and assignment
	MPWolBuddiesMenuClass(const MPWolBuddiesMenuClass&);
	const MPWolBuddiesMenuClass& operator=(const MPWolBuddiesMenuClass&);

	void On_Init_Dialog(void) override;
	void On_Command(int ctrl_id, int mesage_id, unsigned int param) override;
	void On_Frame_Update(void) override;

	void Adjust_Buttons_For_Buddy_Location(int location);

	// Notifications
	void HandleNotification(WOLBuddyMgrEvent &event) override;
	void HandleNotification(DlgMsgBoxEvent &event) override;
	void On_ComboBoxCtrl_Sel_Change(ComboBoxCtrlClass* combo, int id, int oldsel, int newsel) override;
	void On_ListCtrl_Sel_Change(ListCtrlClass* list, int id, int oldsel, int newsel) override;
	void On_ListCtrl_DblClk(ListCtrlClass *list_ctrl, int ctrl_id, int item_index) override;

	void Update_Buddy_Info(int index, const RefPtr<WWOnline::UserData>& user);
	void Update_Buddy_Ranking(int index, const RefPtr<WWOnline::UserData>& user);
	void Refresh_Buddy_List(void);
	void Update_Buddy_List (void);
	void Get_Selected_Buddy(WideStringClass &buddy_name);

	void Page_Selected_User(void);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	static MPWolBuddiesMenuClass* _mInstance;

	WOLBuddyMgr* mBuddyMgr;
	RefPtr<WWOnline::UserData> mPendingJoin;
	bool mBuddyListChanged;
	bool mBuddyInfoChanged;
};

#endif //__DLG_MP_WOL_BUDDIES_H
