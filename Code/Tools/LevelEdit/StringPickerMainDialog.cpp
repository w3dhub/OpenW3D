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

// StringPickerMainDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "StringPickerMainDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// StringPickerMainDialogClass
//
/////////////////////////////////////////////////////////////////////////////
StringPickerMainDialogClass::StringPickerMainDialogClass(CWnd* pParent /*=NULL*/)
	:	TextID (0),
		CDialog(StringPickerMainDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(StringPickerMainDialogClass)
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
StringPickerMainDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(StringPickerMainDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(StringPickerMainDialogClass, CDialog)
	//{{AFX_MSG_MAP(StringPickerMainDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////
//
//	OnInitDialog
//
/////////////////////////////////////////////////////////////////////////
BOOL
StringPickerMainDialogClass::OnInitDialog (void) 
{
	CDialog::OnInitDialog ();

	//
	//	Calculate the rectangle where we are to display the string picker
	//
	CRect rect;
	::GetWindowRect (::GetDlgItem (m_hWnd, IDC_PICKER_AREA), &rect);
	ScreenToClient (&rect);

	//
	//	Create the string picker
	//
	StringPicker.Set_Selection (TextID);
	StringPicker.Create (this);
	StringPicker.SetWindowPos (NULL, rect.left, rect.top, rect.Width (), rect.Height (), SWP_NOZORDER);
	StringPicker.ShowWindow (SW_SHOW);
	return true;
}


/////////////////////////////////////////////////////////////////////////
//
//	OnOK
//
/////////////////////////////////////////////////////////////////////////
void
StringPickerMainDialogClass::OnOK (void)
{
	//
	//	Get the selected string from the picker control
	//
	TextID = StringPicker.Get_Selection ();

	CDialog::OnOK ();
	return ;
}
