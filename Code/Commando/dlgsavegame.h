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
 *                     $Archive:: /Commando/Code/commando/dlgsavegame.h        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/26/01 5:22p                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLG_SAVE_GAME_H
#define __DLG_SAVE_GAME_H


#include "renegadedialog.h"
#include "menudialog.h"
#include "DlgMessageBox.h"


////////////////////////////////////////////////////////////////
//
//	SaveGameMenuClass
//
////////////////////////////////////////////////////////////////
class SaveGameMenuClass : public MenuDialogClass, public Observer<DlgMsgBoxEvent>
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////	
	SaveGameMenuClass (void)	:
		CurrSortCol (1),
		IsSortAscending (false),
		MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MENU_SAVE_GAME))	{}


	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	void		On_Init_Dialog (void) override;
	void		On_Command (int ctrl_id, int mesage_id, unsigned int param) override;
	void		On_ListCtrl_Delete_Entry (ListCtrlClass *list_ctrl, int ctrl_id, int item_index) override;
	void		On_ListCtrl_Column_Click (ListCtrlClass *list_ctrl, int ctrl_id, int col_index) override;
	void		On_ListCtrl_DblClk (ListCtrlClass *list_ctrl, int ctrl_id, int item_index) override;
	void		On_ListCtrl_Sel_Change (ListCtrlClass *list_ctrl, int ctrl_id, int old_index, int new_index) override;
	void		On_EditCtrl_Enter_Pressed (EditCtrlClass *edit_ctrl, int ctrl_id) override;
	void		On_EditCtrl_Change (EditCtrlClass *edit_ctrl, int ctrl_id) override;

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void		HandleNotification (DlgMsgBoxEvent &event) override;
	bool		Check_HD_Space (void);
	void		Update_Button_State (void);

private:

	////////////////////////////////////////////////////////////////
	//	Static members
	////////////////////////////////////////////////////////////////
	static int	LoadListSortCallback (ListCtrlClass *list_ctrl, int item_index1, int item_index2, uint32 user_param);

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	void		Save_Game (bool prompt);
	void		Delete_Game (bool prompt);
	void		Get_Unique_Save_Filename (StringClass &filename);
	void		Update_Text_Field (void);
	void		Reload_List (const char *current_filename);

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	uint16	CurrSortCol;
	bool		IsSortAscending;
};


#endif //__DLG_SAVE_GAME_H

