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
 *                     $Archive:: /Commando/Code/Commando/dlginternetoptions.cpp                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/16/01 6:03p                                                $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlginternetoptions.h"
#include "string_ids.h"
#include "renegadedialog.h"
#include "shortcutbarctrl.h"
#include "translatedb.h"


////////////////////////////////////////////////////////////////
//
//	InternetOptionsMenuClass
//
////////////////////////////////////////////////////////////////
InternetOptionsMenuClass::InternetOptionsMenuClass (void)	:
	MenuDialogClass (IDD_OPTIONS_INTERNET)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
InternetOptionsMenuClass::On_Init_Dialog (void)
{
	WideStringClass entry_text;

	//
	//	Configure the shortcut bar
	//
	ShortcutBarCtrlClass *bar = (ShortcutBarCtrlClass *)Get_Dlg_Item (IDC_SHORTCUT_BAR);
	if (bar != NULL) {

		entry_text = TRANSLATE (IDS_MP_SHORTCUT_BUDDIES);
		bar->Add_Button (IDC_MP_SHORTCUT_BUDDIES, entry_text);

		entry_text = TRANSLATE (IDS_MP_SHORTCUT_CHAT);
		bar->Add_Button (IDC_MP_SHORTCUT_CHAT, entry_text);

		entry_text = TRANSLATE (IDS_MENU_TEXT364);
		bar->Add_Button (IDC_MP_SHORTCUT_QUICKMATCH_OPTIONS, entry_text);

		entry_text = TRANSLATE (IDS_MP_SHORTCUT_NEWS);
		bar->Add_Button (IDC_MP_SHORTCUT_NEWS, entry_text);

		entry_text = TRANSLATE (IDS_MP_SHORTCUT_INSIDER);
		bar->Add_Button (IDC_MP_SHORTCUT_INSIDER, entry_text);

		entry_text = TRANSLATE (IDS_MP_SHORTCUT_CLANS);
		bar->Add_Button (IDC_MP_SHORTCUT_CLANS, entry_text);

		entry_text = TRANSLATE (IDS_MP_SHORTCUT_RANKINGS);
		bar->Add_Button (IDC_MP_SHORTCUT_RANKINGS, entry_text);

		entry_text = TRANSLATE (IDS_MP_SHORTCUT_ACCOUNT);
		bar->Add_Button (IDC_MP_SHORTCUT_ACCOUNT, entry_text);

		entry_text = TRANSLATE (IDS_MP_SHORTCUT_NET_STATUS);
		bar->Add_Button (IDC_MP_SHORTCUT_NET_STATUS, entry_text);		
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
InternetOptionsMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	MenuDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}

