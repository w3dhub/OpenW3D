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
 *                     $Archive:: /Commando/Code/commando/dlgcontrolsaveload.h         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/09/02 11:43a                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLG_CONTROL_SAVELOAD_H
#define __DLG_CONTROL_SAVELOAD_H


#include "menudialog.h"
#include "DlgMessageBox.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class InputConfigClass;


////////////////////////////////////////////////////////////////
//
//	ControlSaveLoadMenuClass
//
////////////////////////////////////////////////////////////////
class ControlSaveLoadMenuClass : public MenuDialogClass, public Observer<DlgMsgBoxEvent>
{
public:

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructor
	//////////////////////////////////////////////////////////////
	ControlSaveLoadMenuClass (void);		

	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////
	
	//
	//	Inherited
	//
	void		On_Init_Dialog (void) override;
	void		On_Command (int ctrl_id, int message_id, unsigned int param) override;
	void		On_ListCtrl_Delete_Entry (ListCtrlClass *list_ctrl, int ctrl_id, int item_index) override;
	void		On_ListCtrl_Sel_Change (ListCtrlClass *list_ctrl, int	ctrl_id, int old_index, int	new_index) override;
	void		On_EditCtrl_Enter_Pressed (EditCtrlClass *edit_ctrl, int ctrl_id) override;

private:	

	////////////////////////////////////////////////////////////////
	//	Static methods
	////////////////////////////////////////////////////////////////
	static int	ListSortCallback (ListCtrlClass *list_ctrl, int item_index1, int item_index2, uint32 user_param);

	//////////////////////////////////////////////////////////////
	//	Private methods
	//////////////////////////////////////////////////////////////
	void		Delete_Config (void);
	void		Load_Config (void);
	void		Save_Config (bool prompt);
	int		Insert_Configuration (const InputConfigClass &config);
	void		HandleNotification (DlgMsgBoxEvent &event) override;
};


#endif //__DLG_CONTROL_SAVELOAD_H

