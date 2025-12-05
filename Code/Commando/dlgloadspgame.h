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
 *                     $Archive:: /Commando/Code/commando/dlgloadspgame.h        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/23/02 10:25a                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLGLOADSPGAME_H
#define __DLGLOADSPGAME_H


#include "renegadedialog.h"
#include "menudialog.h"
#include "DlgMessageBox.h"


////////////////////////////////////////////////////////////////
//
//	LoadSPGameMenuClass
//
////////////////////////////////////////////////////////////////
class LoadSPGameMenuClass : public MenuDialogClass, public Observer<DlgMsgBoxEvent>
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////	
	LoadSPGameMenuClass (void)	:
		CurrSortCol (1),
		IsSortAscending (false),
		MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MENU_LOAD_SP_GAME))	{}


	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	void		On_Init_Dialog (void) override;
	void		On_Command (int ctrl_id, int mesage_id, unsigned int param) override;
	void		On_ListCtrl_Delete_Entry (ListCtrlClass *list_ctrl, int ctrl_id, int item_index) override;
	void		On_ListCtrl_Column_Click (ListCtrlClass *list_ctrl, int ctrl_id, int col_index) override;
	void		On_ListCtrl_DblClk (ListCtrlClass *list_ctrl, int ctrl_id, int item_index) override;
	void		On_ListCtrl_Sel_Change (ListCtrlClass *list_ctrl, int ctrl_id, int old_index, int new_index) override;

	void		Update_Button_State (void);

	////////////////////////////////////////////////////////////////
	//	Static methods
	////////////////////////////////////////////////////////////////
	static	void	Set_Game_Rank(const char * filename, int rank);

	//
	//	Singleton access
	//
	static void							Display (void);
	static LoadSPGameMenuClass *	Get_Instance (void)	{ return _TheInstance; }

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void		HandleNotification (DlgMsgBoxEvent &event) override;
	void		On_Last_Menu_Ending (void) override;

private:

	////////////////////////////////////////////////////////////////
	//	Static members
	////////////////////////////////////////////////////////////////
	static int CALLBACK LoadListSortCallback (ListCtrlClass *list_ctrl, int item_index1, int item_index2, uint32 user_param);

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	void		Load_Game (void);
	void		Delete_Game (bool prompt);
	int			Build_List (const char *search_string, int start_index);

	static	bool	Is_Game_Allowed(const char * filename);
	static	int		Get_Game_Rank(const char * filename);

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	uint16	CurrSortCol;
	bool		IsSortAscending;

	static LoadSPGameMenuClass *_TheInstance;
};


#endif //__DLGLOADSPGAME_H

