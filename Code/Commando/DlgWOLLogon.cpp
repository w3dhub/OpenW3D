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
*     $Archive: /Commando/Code/Commando/DlgWOLLogon.cpp $
*
* DESCRIPTION
*     Westwood Online Login Dialog. This is the dialog that gathers the login
*     information from the user.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 21 $
*     $Modtime: 1/19/02 12:05a $
*
******************************************************************************/

#include "DlgWOLLogon.h"
#include "renegadedialog.h"
#include "renegadedialogmgr.h"
#include "mpsettingsmgr.h"
#include "DlgMessageBox.h"
#include "DlgWOLSettings.h"
#include <WWOnline/WOLLoginInfo.h>
#include <wwui/comboboxctrl.h>
#include <wwui/editctrl.h>
#include "DlgWebPage.h"
#include "string_ids.h"

using namespace WWOnline;

/******************************************************************************
*
* NAME
*     DlgWOLLogon::DoDialog
*
* DESCRIPTION
*
* INPUTS
*     Login    - Login to display
*     Observer - Observer to be notified.
*
* RESULT
*     Success - True if dialog successfully started.
*
******************************************************************************/

bool DlgWOLLogon::DoDialog(const unichar_t* login, Observer<DlgWOLLogonEvent>* observer)
	{
	DlgWOLLogon* dialog = new DlgWOLLogon;

	if (dialog)
		{
		if (dialog->FinalizeCreate(login))
			{
			if (observer)
				{
				dialog->AddObserver(*observer);
				}

			dialog->Start_Dialog();
			}

		dialog->Release_Ref();
		}

	return (dialog != NULL);
	}


/******************************************************************************
*
* NAME
*     DlgWOLLogon::DlgWOLLogon
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

DlgWOLLogon::DlgWOLLogon() :
		PopupDialogClass(GetRenegadeDialog(RenegadeDialogID::IDD_WOL_LOGON)),
		mIsPasswordEncrypted(false)
	{
	WWDEBUG_SAY(("DlgWOLLogon: Instantiated\n"));
	}


/******************************************************************************
*
* NAME
*     DlgWOLLogon::~DlgWOLLogon
*
* DESCRIPTION
*     Destructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

DlgWOLLogon::~DlgWOLLogon()
	{
	WWDEBUG_SAY(("DlgWOLLogon: Destroyed\n"));
	}


/******************************************************************************
*
* NAME
*     DlgWOLLogon::FinalizeCreate
*
* DESCRIPTION
*     Finalize the creation of this object (Initialize).
*
* INPUTS
*
* RESULT
*     Success - True if successful.
*
******************************************************************************/

bool DlgWOLLogon::FinalizeCreate(const unichar_t* login)
	{
	mLogin = login;
	return true;
	}


/******************************************************************************
*
* NAME
*     DlgWOLLogon::On_Init_Dialog
*
* DESCRIPTION
*     Initialize the dialog.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLLogon::On_Init_Dialog(void)
	{
	Check_Dlg_Button(IDC_REMEMBER_LOGIN_CHECK, false);
	Enable_Dlg_Item(IDC_WOL_LOG_ON_BUTTON, false);

	// Limit password length to 8 characters.
	EditCtrlClass* edit = (EditCtrlClass*)Get_Dlg_Item(IDC_PASSWORD_EDIT);

	if (edit)
		{
		edit->Set_Text_Limit(8);
		}

	UpdatePersonas();
	SelectPersona(mLogin);
	
	PopupDialogClass::On_Init_Dialog();
	}


/******************************************************************************
*
* NAME
*     DlgWOLLogon::On_Command
*
* DESCRIPTION
*     Handle command notification for the dialog.
*
* INPUTS
*     Ctrl    - ID of control originating command
*     Message - Command message
*     Param   - Message parameter
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLLogon::On_Command(int ctrl, int message, unsigned int param)
	{
	switch (ctrl)
		{
		case IDC_WOL_LOG_ON_BUTTON:
			{
			// If this login is not stored then always use the password typed by the user.
			const unichar_t* name = Get_Dlg_Item_Text(IDC_PERSONA_COMBO);
			RefPtr<LoginInfo> login = LoginInfo::Find(name);
			
			if (login.IsValid() && !login->IsStored())
				{
				const unichar_t* password = Get_Dlg_Item_Text(IDC_PASSWORD_EDIT);
				login->SetPassword(password, false);
				mIsPasswordEncrypted = false;
				}

			Add_Ref();
			DlgWOLLogonEvent event(DlgWOLLogonEvent::Login, *this);
			NotifyObservers(event);
			Release_Ref();

			End_Dialog();
			}
			break;

		case IDC_DELETE_ACCOUNT_BUTTON:
			{
			// Delete this login from our local cache and purge it from storage.
			const unichar_t* nickname = Get_Dlg_Item_Text(IDC_PERSONA_COMBO);
			RefPtr<LoginInfo> login = LoginInfo::Find(nickname);

			if (login.IsValid())
				{
				login->Forget(true);
				}

			// Refresh the UI
			UpdatePersonas();
			SelectPersona(NULL);
			}
			break;

		case IDC_MANAGE_ACCOUNT_BUTTON:
			DlgWebPage::DoDialog("Signup");
			break;

		case IDC_MENU_BACK_BUTTON:
			Add_Ref();
			DlgWOLLogonEvent event(DlgWOLLogonEvent::Cancel, *this);
			NotifyObservers(event);
			Release_Ref();
			break;
		}

	PopupDialogClass::On_Command(ctrl, message, param);
	}


/******************************************************************************
*
* NAME
*     DlgWOLLogon::GetLogin
*
* DESCRIPTION
*     Get the login information.
*
* INPUTS
*     Name      - On return; Name to login in with.
*     Password  - On return; Password to login with.
*     Encrypted - Flag indicating if the password is encrypted.
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLLogon::GetLogin(const unichar_t** name, const unichar_t** password,
		bool& encrypted)
	{
	if (name)
		{
		*name = Get_Dlg_Item_Text(IDC_PERSONA_COMBO);
		}

	if (password)
		{
		*password = Get_Dlg_Item_Text(IDC_PASSWORD_EDIT);
		}

	encrypted = mIsPasswordEncrypted;
	}


/******************************************************************************
*
* NAME
*     DlgWOLLogon::IsRememberLoginChecked
*
* DESCRIPTION
*     Get the "checked" state of the remember login checkbox.
*
* INPUTS
*     NONE
*
* RESULT
*     True if remember login checkbox is selected; false otherwise.
*
******************************************************************************/

bool DlgWOLLogon::IsRememberLoginChecked(void)
	{
	return Is_Dlg_Button_Checked(IDC_REMEMBER_LOGIN_CHECK);
	}


/******************************************************************************
*
* NAME
*     DlgWOLLogon::UpdatePersonas
*
* DESCRIPTION
*     Update the persona combo box with cached logins.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLLogon::UpdatePersonas(void)
	{
	ComboBoxCtrlClass* combo = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_PERSONA_COMBO);

	if (combo)
		{
		combo->Reset_Content();

		const LoginInfoList& personas = LoginInfo::GetList();
		const size_t count = personas.size();

		for (size_t index = 0; index < count; ++index)
			{
			const WideStringClass& name = personas[index]->GetNickname();
			combo->Add_String(name);
			}
		}
	}


/******************************************************************************
*
* NAME
*     DlgWOLLogon::SelectPersona
*
* DESCRIPTION
*     Select the persona with the specified name.
*
* INPUTS
*     Name - Name of persona.
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLLogon::SelectPersona(const unichar_t* name)
	{
	ComboBoxCtrlClass* combo = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_PERSONA_COMBO);

	if (combo)
		{
		// Select the specified persona or the first one in the list.
		if (name && (u_strlen(name) > 0))
			{
			int index = combo->Select_String(name);

			// If the persona is not in the list then put it into the edit field
			if (index == -1)
				{
				combo->Set_Text(name);
				}
			}
		else
			{
			name = combo->Get_Text();
			}

		AutoComplete(name);
		}
	}


/******************************************************************************
*
* NAME
*     DlgWOLLogon::AutoComplete
*
* DESCRIPTION
*     Automatically complete the login dialog fields if an existing login
*     matches the provided name.
*
* INPUTS
*     Name - Login name to auto complete.
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLLogon::AutoComplete(const unichar_t* name)
	{
	// If the persona has a valid login then fill in the dialog with
	// the login information. Otherwise use the provided persona name.
	RefPtr<LoginInfo> login = LoginInfo::Find(name);

	if (login.IsValid() && login->IsStored())
		{
		// Fill in password
		const WideStringClass& password = login->GetPassword();
		Set_Dlg_Item_Text(IDC_PASSWORD_EDIT, password);
		mIsPasswordEncrypted = login->IsPasswordEncrypted();

		// WOL passwords must be exactly 8 characters
		WWASSERT(password.Get_Length() == 8);
		Enable_Dlg_Item(IDC_PASSWORD_EDIT, false);

		Check_Dlg_Button(IDC_REMEMBER_LOGIN_CHECK, true);
		Enable_Dlg_Item(IDC_REMEMBER_LOGIN_CHECK, false);
		Enable_Dlg_Item(IDC_DELETE_ACCOUNT_BUTTON, true);
		Enable_Dlg_Item(IDC_WOL_LOG_ON_BUTTON, true);
		}
	else
		{
		// Erase the password and enable the remember checkbox
		Set_Dlg_Item_Text(IDC_PASSWORD_EDIT, U_CHAR(""));
		mIsPasswordEncrypted = false;

		Enable_Dlg_Item(IDC_PASSWORD_EDIT, true);
		Enable_Dlg_Item(IDC_REMEMBER_LOGIN_CHECK, true);
		Enable_Dlg_Item(IDC_DELETE_ACCOUNT_BUTTON, false);
		Enable_Dlg_Item(IDC_WOL_LOG_ON_BUTTON, false);
		}
	}


/******************************************************************************
*
* NAME
*     DlgWOLLogon::On_ComboBoxCtrl_Sel_Change
*
* DESCRIPTION
*     Notification sent by a combobox when the selection is changed by user
*     interaction.
*
* INPUTS
*     Combo  - Pointer to notifying combobox control.
*     ID     - ID of notifying combobox control.
*     OldSel - Index of previous selection.
*     NewSel - Index of new selection.
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLLogon::On_ComboBoxCtrl_Sel_Change(ComboBoxCtrlClass* combo, int id, int, int)
	{
	if (IDC_PERSONA_COMBO == id)
		{
		const unichar_t* text = combo->Get_Text();
		AutoComplete(text);
		}
	}


/******************************************************************************
*
* NAME
*     DlgWOLLogon::On_ComboBoxCtrl_Edit_Change
*
* DESCRIPTION
*     Notification sent by a combobox when the text in the edit field is
*     changed by user interaction.
*
* INPUTS
*     Combo - Pointer to notifying combobox control.
*     ID    - ID of notifying combobox control.
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLLogon::On_ComboBoxCtrl_Edit_Change(ComboBoxCtrlClass* combo, int id)
	{
	if (IDC_PERSONA_COMBO == id)
		{
		const unichar_t* text = combo->Get_Text();
		AutoComplete(text);
		}
	}


/******************************************************************************
*
* NAME
*     DlgWOLLogon::On_EditCtrl_Change
*
* DESCRIPTION
*     Notification sent by an edit control when the text in the edit field is
*     changed by user interaction.
*
* INPUTS
*     Edit - Pointer to notifying edit control.
*     ID   - ID of notifying edit control.
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLLogon::On_EditCtrl_Change(EditCtrlClass* edit, int id)
	{
	if (IDC_PASSWORD_EDIT == id)
		{
		// If the user entered text then password is not encrypted.
		mIsPasswordEncrypted = false;

		// WOL passwords must be exactly 8 characters
		int length = edit->Get_Text_Length();
		bool enable = (8 == length);
		Enable_Dlg_Item(IDC_WOL_LOG_ON_BUTTON, enable);
		}
	}


/******************************************************************************
*
* NAME
*     DlgWOLLogon::On_EditCtrl_Enter_Pressed
*
* DESCRIPTION
*     Notification sent by an edit control that the enter key was pressed.
*
* INPUTS
*     Edit - Pointer to notifying edit control.
*     ID   - ID of notifying edit control.
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLLogon::On_EditCtrl_Enter_Pressed(EditCtrlClass* /* edit */, int /* id */)
	{
	EditCtrlClass* passwordEdit = (EditCtrlClass*)Get_Dlg_Item(IDC_PASSWORD_EDIT);

	if (passwordEdit)
		{
		const unichar_t* name = Get_Dlg_Item_Text(IDC_PERSONA_COMBO);
		bool nameOK = (name && (u_strlen(name) > 0));
		bool passOK = (passwordEdit->Get_Text_Length() == 8);

		if (nameOK && passOK)
			{
			Add_Ref();
			DlgWOLLogonEvent event(DlgWOLLogonEvent::Login, *this);
			NotifyObservers(event);
			Release_Ref();

			End_Dialog();
			}
		}
	}
