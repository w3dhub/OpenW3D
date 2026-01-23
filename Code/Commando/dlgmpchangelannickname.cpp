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

/******************************************************************************
*
* FILE
*     $Archive: /Commando/Code/Commando/dlgmpchangelannickname.cpp $
*
* DESCRIPTION
*     Handle LAN nickname collision.
*
* PROGRAMMER
*     Tom
*
* VERSION INFO
*     $Revision: 1 $
*     $Modtime: 1/04/02 4:14p $
*
******************************************************************************/

#include "dlgmpchangelannickname.h"
#include "renegadedialog.h"
#include <wwui/editctrl.h>
#include "netinterface.h"
#include "dlgmplangamelist.h"

int	DlgMpChangeLanNickname::DialogCount	= 0;

//-----------------------------------------------------------------------------
bool DlgMpChangeLanNickname::DoDialog(void)
{	
	DlgMpChangeLanNickname* dialog = NULL;

	if (DialogCount == 0) {
		dialog = new DlgMpChangeLanNickname;
	}

	if (dialog)
	{
		dialog->Start_Dialog();
		dialog->Release_Ref();
	}

	return (dialog != NULL);
}

//-----------------------------------------------------------------------------
DlgMpChangeLanNickname::DlgMpChangeLanNickname() :
	PopupDialogClass(GetRenegadeDialog(RenegadeDialogID::IDD_MP_CHANGE_LAN_NICKNAME))
{
	WWDEBUG_SAY(("DlgMpChangeLanNickname Instantiated\n"));

	DialogCount++;
}

//-----------------------------------------------------------------------------
DlgMpChangeLanNickname::~DlgMpChangeLanNickname()
{
	WWDEBUG_SAY(("DlgMpChangeLanNickname Destroyed\n"));

	DialogCount--;
}

//-----------------------------------------------------------------------------
void DlgMpChangeLanNickname::On_Init_Dialog(void)
{
	Enable_Dlg_Item(IDOK, false);

	EditCtrlClass* edit = (EditCtrlClass*)Get_Dlg_Item(IDC_NICKNAME_EDIT);

	if (edit)
	{
		edit->Set_Text_Limit(9);
		edit->Set_Text(cNetInterface::Get_Nickname());
		edit->Set_Focus();
	}

	PopupDialogClass::On_Init_Dialog();
}

//-----------------------------------------------------------------------------
void DlgMpChangeLanNickname::On_Command(int ctrlID, int message, unsigned int param)
{
	if (IDOK == ctrlID)
	{
		WideStringClass nickname = Get_Dlg_Item_Text (IDC_NICKNAME_EDIT);
		cNetInterface::Set_Nickname (nickname);	

		MPLanGameListMenuClass::Set_Update_Nickname();

		End_Dialog();
	}

	PopupDialogClass::On_Command(ctrlID, message, param);
}

//-----------------------------------------------------------------------------
void DlgMpChangeLanNickname::On_EditCtrl_Change(EditCtrlClass* edit, int id)
{
	if (IDC_NICKNAME_EDIT == id)
	{
		const unichar_t* text = edit->Get_Text();
		bool enableok = (text && (u_strlen(text) > 0));
		Enable_Dlg_Item(IDOK, enableok);
	}
}

//-----------------------------------------------------------------------------
void DlgMpChangeLanNickname::On_EditCtrl_Enter_Pressed(EditCtrlClass* /* edit */, int id)
{
	if ((IDC_NICKNAME_EDIT == id) && Is_Dlg_Item_Enabled(IDOK))
	{
		On_Command(IDOK, 0, 0);
	}
}
