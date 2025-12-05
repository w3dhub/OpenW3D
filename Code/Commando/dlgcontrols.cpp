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
 *                     $Archive:: /Commando/Code/commando/dlgcontrols.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/09/01 1:27p                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "dlgcontrols.h"
#include "dlgcontroltabs.h"
#include "dlgcontrolsaveload.h"
#include "tabctrl.h"
#include "renegadedialog.h"
#include "inputconfigmgr.h"


////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
ControlsMenuClass *	ControlsMenuClass::_TheInstance	= NULL;


////////////////////////////////////////////////////////////////
//
//	ControlsMenuClass
//
////////////////////////////////////////////////////////////////
ControlsMenuClass::ControlsMenuClass (void)	:
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MENU_CONTROLS))
{
	_TheInstance = this;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~ControlsMenuClass
//
////////////////////////////////////////////////////////////////
ControlsMenuClass::~ControlsMenuClass (void)
{
	_TheInstance = NULL;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
ControlsMenuClass::On_Init_Dialog (void)
{
	TabCtrlClass *tab_ctrl = (TabCtrlClass *)Get_Dlg_Item (IDC_CONTROL_TABCTRL);
	if (tab_ctrl != NULL) {
		
		//
		//	Add the tabs to the control
		//
		TABCTRL_ADD_TAB (tab_ctrl, ControlsBasicMvmtTabClass);
		TABCTRL_ADD_TAB (tab_ctrl, ControlsAttackTabClass);
		TABCTRL_ADD_TAB (tab_ctrl, ControlsWeaponsTabClass);
		TABCTRL_ADD_TAB (tab_ctrl, ControlsLookTabClass);
		TABCTRL_ADD_TAB (tab_ctrl, ControlsMultiPlayTabClass);
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
ControlsMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDCANCEL:
		case IDC_MENU_BACK_BUTTON:
			Apply_Changes ();
			break;

		case IDC_DEFAULTS_BUTTON:
			InputConfigMgrClass::Load_Default_Configuration ();
			break;

		case IDC_SAVELOAD_BUTTON:
			START_DIALOG (ControlSaveLoadMenuClass);
			break;
	}

	MenuDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Apply_Changes
//
////////////////////////////////////////////////////////////////
void
ControlsMenuClass::Apply_Changes (void)
{
	//
	//	Apply the changes on each tab
	//
	TabCtrlClass *tab_ctrl = (TabCtrlClass *)Get_Dlg_Item (IDC_CONTROL_TABCTRL);
	if (tab_ctrl != NULL) {
		if (tab_ctrl->Apply_Changes_On_Tabs ()) {
			
			//
			//	Save the changes
			//
			InputConfigMgrClass::Save_Current_Configuration ();
		}
	}

	return; 
}


////////////////////////////////////////////////////////////////
//
//	Reload
//
////////////////////////////////////////////////////////////////
void
ControlsMenuClass::Reload (void)
{
	//
	//	Ask each tab to reload its data
	//
	TabCtrlClass *tab_ctrl = (TabCtrlClass *)Get_Dlg_Item (IDC_CONTROL_TABCTRL);
	if (tab_ctrl != NULL) {
		tab_ctrl->Reload_Tabs ();
	}
	
	return ;
}

