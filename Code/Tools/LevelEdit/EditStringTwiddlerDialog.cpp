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

// EditStringTwiddlerDialog.cpp : implementation file
//

#include "stdafx.h"
#include "leveledit.h"
#include "editstringtwiddlerdialog.h"
#include "editstringdialog.h"
#include "stringtwiddler.h"
#include "stringpickermaindialog.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// EditStringTwiddlerDialogClass
//
/////////////////////////////////////////////////////////////////////////////
EditStringTwiddlerDialogClass::EditStringTwiddlerDialogClass (CWnd *pParent /*=NULL*/)	:
	StringObject (NULL),
	CDialog (EditStringTwiddlerDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(EditStringTwiddlerDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
EditStringTwiddlerDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EditStringTwiddlerDialogClass)
	DDX_Control(pDX, IDC_LISTCTRL, ListCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(EditStringTwiddlerDialogClass, CDialog)
	//{{AFX_MSG_MAP(EditStringTwiddlerDialogClass)
	ON_BN_CLICKED(IDC_ADD_BUTTON, OnAddButton)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LISTCTRL, OnKeydownListctrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
EditStringTwiddlerDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	//
	//	Configure the list control
	//
	ListCtrl.InsertColumn (0, "String ID");
	
	//
	//	Size the column
	//
	CRect rect;
	ListCtrl.GetClientRect (&rect);
	rect.right -= ::GetSystemMetrics (SM_CXVSCROLL) + 2;
	ListCtrl.SetColumnWidth (0, rect.Width ());

	//
	//	Fill in the text control
	//
	if (StringObject != NULL) {
		SetDlgItemText (IDC_CODEID_EDIT, StringObject->Get_ID_Desc ());

		//
		//	Add the list of strings to the database
		//
		for (int index = 0; index < StringObject->Get_String_Count (); index ++) {
			Insert_String (StringObject->Get_String_Index (index));
		}

	} else {
		SetDlgItemText (IDC_CODEID_EDIT, "IDS_");
	}

	//
	//	Select the CODE ID so the user can enter a valid ID
	//
	::SetFocus (::GetDlgItem (m_hWnd, IDC_CODEID_EDIT));
	SendDlgItemMessage (IDC_CODEID_EDIT, EM_SETSEL, (WPARAM)0, (LPARAM)-1);

	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
//
// Insert_String
//
/////////////////////////////////////////////////////////////////////////////
void
EditStringTwiddlerDialogClass::Insert_String (int string_id)
{
	//
	//	Lookup the object for this ID
	//
	TDBObjClass *object = TranslateDBClass::Find_Object (string_id);
	if (object != NULL) {
		const StringClass &text = object->Get_ID_Desc ();

		//
		//	Add an entry to the list for this string
		//
		int item_index = ListCtrl.InsertItem (0xFF, text);
		if (item_index != -1) {
			ListCtrl.SetItemData (item_index, string_id);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnAddButton
//
/////////////////////////////////////////////////////////////////////////////
void
EditStringTwiddlerDialogClass::OnAddButton (void)
{
	//
	//	Show the dialog to the user so they can pick a string
	//
	StringPickerMainDialogClass dialog (this);
	if (dialog.DoModal () == IDOK) {
		
		//
		//	Add this string to our list
		//
		Insert_String (dialog.Get_String_ID ());
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
EditStringTwiddlerDialogClass::OnOK (void)
{
	//
	//	Get the strind ID the user entered
	//
	CString string_id;
	GetDlgItemText (IDC_CODEID_EDIT, string_id);

	//
	//	Is this a valid ID?
	//
	if (Validate_String_ID (m_hWnd, string_id)) {
		
		//
		//	Create a new twiddler (if necessary)
		//		
		if (StringObject == NULL) {
			StringObject = new StringTwiddlerClass;
		}

		//
		//	Configure the twiddler
		//
		StringObject->Reset_String_List ();
		StringObject->Set_ID_Desc (string_id);

		//
		//	Add the list of strings to the twiddler
		//
		int count = ListCtrl.GetItemCount ();
		for (int index = 0; index < count; index ++) {
			int string_id = (int)ListCtrl.GetItemData (index);
			StringObject->Add_String (string_id);
		}
		
		CDialog::OnOK ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnKeydownListctrl
//
/////////////////////////////////////////////////////////////////////////////
void
EditStringTwiddlerDialogClass::OnKeydownListctrl (NMHDR *pNMHDR, LRESULT *pResult)
{
	LV_KEYDOWN *pLVKeyDown = (LV_KEYDOWN*)pNMHDR;
	*pResult = 0;

	if (pLVKeyDown->wVKey == VK_DELETE) {
		
		//
		//	Delete all the selected items (except for the last item)
		//
		int index = -1;
		while ((index = ListCtrl.GetNextItem (-1, LVNI_SELECTED | LVNI_ALL)) >= 0) {
			ListCtrl.DeleteItem (index);
		}
	}

	return ;
}
