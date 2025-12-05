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
 *                     $Archive:: /Commando/Code/commando/dlgcheatoptions.cpp     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/04/01 6:51p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgcheatoptions.h"
#include "listctrl.h"
#include "cheatmgr.h"
#include "renegadedialog.h"


////////////////////////////////////////////////////////////////
//
//	CheatOptionsMenuClass
//
////////////////////////////////////////////////////////////////
CheatOptionsMenuClass::CheatOptionsMenuClass (void)	:
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_OPTIONS_CHEATS))
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
CheatOptionsMenuClass::On_Init_Dialog (void)
{
	CheatMgrClass *cheat_mgr = CheatMgrClass::Get_Instance ();

	Check_Dlg_Button (IDC_INVULN_CHECK,			cheat_mgr->Is_Cheat_Set (CheatMgrClass::CHEAT_INVULNERABILITY));
	Check_Dlg_Button (IDC_UNLIM_AMMO_CHECK,	cheat_mgr->Is_Cheat_Set (CheatMgrClass::CHEAT_INFINITE_AMMO));
	Check_Dlg_Button (IDC_GIVE_WEAPONS_CHECK, cheat_mgr->Is_Cheat_Set (CheatMgrClass::CHEAT_ALL_WEAPONS));

	MenuDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
CheatOptionsMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	MenuDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Destroy
//
////////////////////////////////////////////////////////////////
void
CheatOptionsMenuClass::On_Destroy (void)
{
	CheatMgrClass *cheat_mgr = CheatMgrClass::Get_Instance ();

	cheat_mgr->Enable_Cheat (CheatMgrClass::CHEAT_INVULNERABILITY, Is_Dlg_Button_Checked (IDC_INVULN_CHECK));
	cheat_mgr->Enable_Cheat (CheatMgrClass::CHEAT_INFINITE_AMMO, Is_Dlg_Button_Checked (IDC_UNLIM_AMMO_CHECK));
	cheat_mgr->Enable_Cheat (CheatMgrClass::CHEAT_ALL_WEAPONS, Is_Dlg_Button_Checked (IDC_GIVE_WEAPONS_CHECK));

	MenuDialogClass::On_Destroy ();
	return ;
}
