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
 *                     $Archive:: /Commando/Code/commando/dlgcontrolslisttab.h      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/15/02 5:52p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLGCONTROLSLISTTAB_H
#define __DLGCONTROLSLISTTAB_H


#include "childdialog.h"
#include "vector.h"
#include "DlgMessageBox.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class InputCtrlClass;


////////////////////////////////////////////////////////////////
//
//	ControlsListTabClass
//
////////////////////////////////////////////////////////////////
class ControlsListTabClass : public ChildDialogClass, public Observer<DlgMsgBoxEvent>
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////	
	ControlsListTabClass (const DialogResource *dialog_resource);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Inherited
	//
	void		On_Init_Dialog (void) override;
	void		On_Reload (void) override;
	bool		On_InputCtrl_Get_Key_Info (InputCtrlClass *viewer_ctrl, int ctrl_id, int vkey_id, WideStringClass &key_name, int *game_key_id) override;

	//
	//	Configuration
	//
	void		Add_Function (int function_id, int pri_ctrl_id, int sec_ctrl_id);

	//
	//	Notifications
	//
	void		HandleNotification (DlgMsgBoxEvent &event) override;

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void				Load_Key_Mappings (void);
	int				Find_Function_By_Key (int curr_function_id, int key_id);
	void				Remap_Key (int ctrl_id, int function_id, int input_id);
	void				Clear_Key (int key_id, bool clear_zoom);
	void				Prompt_User (void);
	const wchar_t *	Get_Function_Name (int function_id);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	DynamicVectorClass<int>	FunctionIDList;
	DynamicVectorClass<int>	PrimaryCtrlIDList;
	DynamicVectorClass<int>	SecondaryCtrlIDList;

	int PendingOldFunctionID;
	int PendingCtrlID;
	int PendingFunctionID;
	int PendingDIK_ID;
};


#endif //__DLGCONTROLSLISTTAB_H

