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

// ValueListDialogClass.cpp : implementation file
//

#include "StdAfx.h"
#include "leveledit.h"
#include "ValueListDialog.H"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ValueListDialogClass dialog


ValueListDialogClass::ValueListDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(ValueListDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(ValueListDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void ValueListDialogClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ValueListDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ValueListDialogClass, CDialog)
	//{{AFX_MSG_MAP(ValueListDialogClass)
	ON_EN_UPDATE(IDC_LIST_EDIT, OnUpdateListEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////
//
//	OnOK
//
void
ValueListDialogClass::OnOK (void)
{
	// Record the entries for later
	GetDlgItemText (IDC_LIST_EDIT, m_ListEntries);

	// Allow the base class to process this message
	CDialog::OnOK ();
	return ;
}


////////////////////////////////////////////////////////////////////
//
//	OnUpdateListEdit
//
void
ValueListDialogClass::OnUpdateListEdit (void) 
{
	// Enable or disable the OK button
	bool benable = bool(::GetWindowTextLength (::GetDlgItem (m_hWnd, IDC_LIST_EDIT)) > 0);
	::EnableWindow (::GetDlgItem (m_hWnd, IDOK), benable);
	return ;
}


////////////////////////////////////////////////////////////////////
//
//	OnInitDialog
//
BOOL
ValueListDialogClass::OnInitDialog (void)
{
	// Allow the base class to process this message
	CDialog::OnInitDialog ();
	
	// Put the initial text into the edit control
	SetDlgItemText (IDC_LIST_EDIT, m_ListEntries);
	return TRUE;
}
