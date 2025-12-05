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
 *                 Project Name : commando                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/commando/dlgmpwollocalenag.cpp                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/17/01 2:57p                                                $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgmpwollocalenag.h"
#include "renegadedialog.h"
#include "wollocalemgr.h"
#include "comboboxctrl.h"


//////////////////////////////////////////////////////////////////////
//
//	MPWolLocaleNagDialogClass
//
//////////////////////////////////////////////////////////////////////
MPWolLocaleNagDialogClass::MPWolLocaleNagDialogClass (void)	:
	PopupDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MP_WOL_LOCALE_NAG))
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~MPWolLocaleNagDialogClass
//
//////////////////////////////////////////////////////////////////////
MPWolLocaleNagDialogClass::~MPWolLocaleNagDialogClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
//////////////////////////////////////////////////////////////////////
void
MPWolLocaleNagDialogClass::On_Init_Dialog (void)
{	
	//
	//	Fill the combobox with all the locales
	//
	ComboBoxCtrlClass *ctrl = (ComboBoxCtrlClass *)Get_Dlg_Item (IDC_LOCALE_COMBO);
	if (ctrl != NULL) {
		WolLocaleMgrClass::Configure_Locale_Combobox (ctrl);
	}

	PopupDialogClass::On_Init_Dialog ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Command
//
//////////////////////////////////////////////////////////////////////
void
MPWolLocaleNagDialogClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	if (ctrl_id == IDOK) {
		
		//
		//	Get the selected locale from the combobox
		//
		ComboBoxCtrlClass *ctrl = (ComboBoxCtrlClass *)Get_Dlg_Item (IDC_LOCALE_COMBO);
		if (ctrl != NULL) {
			WOL::Locale locale = (WOL::Locale)ctrl->Get_Curr_Sel ();

			//
			//	Set the current locale accordingly
			//
			WolLocaleMgrClass::Set_Current_Locale (locale);
		}

		//
		//	Close the dialog
		//
		End_Dialog ();
	}

	PopupDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}
