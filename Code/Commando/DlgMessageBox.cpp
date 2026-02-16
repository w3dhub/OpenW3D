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
* NAME
*     $Archive: /Commando/Code/Commando/DlgMessageBox.cpp $
*
* DESCRIPTION
*     Popup message dialog.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 7 $
*     $Modtime: 10/02/01 2:42p $
*
******************************************************************************/

#include "renegadedialog.h"
#include "DlgMessageBox.h"
#include <wwtranslatedb/translatedb.h>
#include "wwdebug.h"

//
// Class statics
//
int DlgMsgBox::CurrentCount	= 0;

/******************************************************************************
*
* NAME
*     DlgMsgBox::DoDialog
*
* DESCRIPTION
*     Create a Popup message box dialog.
*
* INPUTS
*     Title - Message box title
*     Text  - Message content
*
* RESULT
*      Object - Instance of message box
*
******************************************************************************/

bool DlgMsgBox::DoDialog(const unichar_t* title, const unichar_t* text,
		DlgMsgBox::Type type, Observer<DlgMsgBoxEvent>* observer, unsigned int user_data)
	{	
	DlgMsgBox* popup = new DlgMsgBox;

	if (popup)
		{
		popup->SetResourceType(type);
		popup->Start_Dialog();

		popup->Set_Title(title);
		popup->Set_Dlg_Item_Text(IDC_MESSAGE, text);
		popup->Set_User_Data(user_data);

		if (observer)
			{
			popup->AddObserver(*observer);
			}

		popup->Release_Ref();
		}

	return (popup != NULL);
	}


/******************************************************************************
*
* NAME
*     DlgMsgBox::DoDialog
*
* DESCRIPTION
*     Create a Popup message box dialog.
*
* INPUTS
*     TitleID - ID of message box title
*     TextID  - ID of message content
*
* RESULT
*      Object - Instance of message box
*
******************************************************************************/

bool DlgMsgBox::DoDialog(int titleID, int textID,
		DlgMsgBox::Type type, Observer<DlgMsgBoxEvent>* observer, unsigned int user_data)
	{
	const unichar_t* title = TranslateDBClass::Get_String(titleID);
	const unichar_t* text = TranslateDBClass::Get_String(textID);
	return DoDialog(title, text, type, observer, user_data);
	}


/******************************************************************************
*
* NAME
*     DlgMsgBox::DlgMsgBox
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

DlgMsgBox::DlgMsgBox() :
		mUserData(0),
		PopupDialogClass(GetRenegadeDialog(RenegadeDialogID::IDD_MESSAGEBOX_OK))
	{
		CurrentCount++;
	}


/******************************************************************************
*
* NAME
*     DlgMsgBox::~DlgMsgBox
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

DlgMsgBox::~DlgMsgBox()
	{
		CurrentCount--;
		WWASSERT(CurrentCount >= 0);
	}


/******************************************************************************
*
* NAME
*     DlgMsgBox::SetResourceType
*
* DESCRIPTION
*     Set the type of dialog box to use.
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void DlgMsgBox::SetResourceType(DlgMsgBox::Type type)
	{
	switch (type) {
		case DlgMsgBox::Type::Okay:
			DialogResource_ = GetRenegadeDialog(RenegadeDialogID::IDD_MESSAGEBOX_OK);
			break;
		case DlgMsgBox::Type::YesNo:
			DialogResource_ = GetRenegadeDialog(RenegadeDialogID::IDD_MESSAGEBOX_YESNO);
			break;
	}
	}


/******************************************************************************
*
* NAME
*     DlgMsgBox::End_Dialog
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMsgBox::End_Dialog(void)
	{
	Add_Ref();
	DlgMsgBoxEvent event(DlgMsgBoxEvent::Quitting, this, mUserData);
	NotifyObservers(event);
	Release_Ref();

	PopupDialogClass::End_Dialog();
	}


/******************************************************************************
*
* NAME
*     DlgMsgBox::On_Command
*
* DESCRIPTION
*     Process command message
*
* INPUTS
*     Ctrl    - ID of control
*     Message -
*     Param   -
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMsgBox::On_Command(int ctrl, int message, unsigned int param)
	{
	switch (ctrl)
		{
		case IDOK:
			{
			Add_Ref();
			
			DlgMsgBoxEvent event(DlgMsgBoxEvent::Okay, this, mUserData);
			NotifyObservers(event);
			
			Release_Ref();
			End_Dialog();
			}
			break;

		case IDYES:
			{
			Add_Ref();
			
			DlgMsgBoxEvent event(DlgMsgBoxEvent::Yes, this, mUserData);
			NotifyObservers(event);
			
			Release_Ref();
			End_Dialog();
			}
			break;

		case IDNO:
			{
			Add_Ref();
			
			DlgMsgBoxEvent event(DlgMsgBoxEvent::No, this, mUserData);
			NotifyObservers(event);

			Release_Ref();
			End_Dialog();
			}
			break;

		default:
			PopupDialogClass::On_Command(ctrl, message, param);
			break;
		}
	}
