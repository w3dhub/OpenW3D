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
 *                 Project Name : commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/dlgmpwolautologinprompt.cpp         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/17/01 11:38p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgmpwolautologinprompt.h"
#include "renegadedialog.h"
#include "mpsettingsmgr.h"


//////////////////////////////////////////////////////////////////////
//
//	MPWolAutoLoginPromptDialogClass
//
//////////////////////////////////////////////////////////////////////
MPWolAutoLoginPromptDialogClass::MPWolAutoLoginPromptDialogClass (void)	:
	PopupDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MP_WOL_AUTO_LOGIN_PROMPT))
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~MPWolAutoLoginPromptDialogClass
//
//////////////////////////////////////////////////////////////////////
MPWolAutoLoginPromptDialogClass::~MPWolAutoLoginPromptDialogClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
//////////////////////////////////////////////////////////////////////
void
MPWolAutoLoginPromptDialogClass::On_Init_Dialog (void)
{	
	Check_Dlg_Button (IDC_DONT_ASK_AGAIN_CHECK, true);
	PopupDialogClass::On_Init_Dialog ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Command
//
//////////////////////////////////////////////////////////////////////
void
MPWolAutoLoginPromptDialogClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	//
	//	Remember the "don't show me this again" setting
	//
	if (ctrl_id == IDYES || ctrl_id == IDNO) {
		bool is_checked = Is_Dlg_Button_Checked (IDC_DONT_ASK_AGAIN_CHECK);
		MPSettingsMgrClass::Enable_Auto_Login_Prompt (!is_checked);

		bool autoLogin = (ctrl_id == IDYES);

		if (autoLogin) {
			const char* login = MPSettingsMgrClass::Get_Last_Login();
			MPSettingsMgrClass::Set_Auto_Login(login);
		}

		//
		//	Close the dialog
		//
		End_Dialog ();
	}

	PopupDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}
