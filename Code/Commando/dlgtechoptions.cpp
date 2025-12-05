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
 *                     $Archive:: /Commando/Code/commando/dlgtechoptions.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/20/01 6:23p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgtechoptions.h"
#include "renegadedialog.h"
#include "tabctrl.h"
#include "dlgconfigaudiotab.h"
#include "dlgconfigvideotab.h"
#include "dlgconfigperformancetab.h"
#include "systemsettings.h"


////////////////////////////////////////////////////////////////
//
//	TechOptionsMenuClass
//
////////////////////////////////////////////////////////////////
TechOptionsMenuClass::TechOptionsMenuClass (void)	:
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_OPTIONS_TECH))
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
TechOptionsMenuClass::On_Init_Dialog (void)
{
	TabCtrlClass *tab_ctrl = (TabCtrlClass *)Get_Dlg_Item (IDC_TABCTRL);
	if (tab_ctrl != NULL) {
		
		//
		//	Add the tabs to the control
		//
		TABCTRL_ADD_TAB (tab_ctrl, DlgConfigAudioTabClass);
		TABCTRL_ADD_TAB (tab_ctrl, DlgConfigVideoTabClass);		
		TABCTRL_ADD_TAB (tab_ctrl, DlgConfigPerformanceTabClass);
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
TechOptionsMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	/*if (ctrl_id == IDOK) {

	}*/	

	MenuDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Destroy
//
////////////////////////////////////////////////////////////////
void
TechOptionsMenuClass::On_Destroy (void)
{
	//
	//	Get a pointer to the tab ctrl
	//
	TabCtrlClass *tab_ctrl = (TabCtrlClass *)Get_Dlg_Item (IDC_TABCTRL);
	if (tab_ctrl != NULL) {

		//
		//	Save the changes on each tab of the control
		//
		tab_ctrl->Apply_Changes_On_Tabs ();
	}

	//
	//	Apply any system settings we modified
	//
	SystemSettings::Apply_All ();
	return ;
}
