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

// StringsCategoryNameDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "StringsCategoryNameDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// StringsCategoryNameDialogClass
//
/////////////////////////////////////////////////////////////////////////////
StringsCategoryNameDialogClass::StringsCategoryNameDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(StringsCategoryNameDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(StringsCategoryNameDialogClass)
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
StringsCategoryNameDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(StringsCategoryNameDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(StringsCategoryNameDialogClass, CDialog)
	//{{AFX_MSG_MAP(StringsCategoryNameDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
StringsCategoryNameDialogClass::OnInitDialog (void) 
{
	CDialog::OnInitDialog ();

	SetDlgItemText (IDC_NAME, Name);
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
StringsCategoryNameDialogClass::OnOK (void)
{
	GetDlgItemText (IDC_NAME, Name);
	CDialog::OnOK ();
	return ;
}

