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

// GenericTextDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "GenericTextDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
//	GenericTextDialogClass
//
/////////////////////////////////////////////////////////////////////////////
GenericTextDialogClass::GenericTextDialogClass (CWnd *pParent /*=NULL*/)
	:	m_IconID (0),
		m_Title ("Information"),
		CDialog(GenericTextDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(GenericTextDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
GenericTextDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(GenericTextDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(GenericTextDialogClass, CDialog)
	//{{AFX_MSG_MAP(GenericTextDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
//	OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
GenericTextDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	//
	//	Set the text properties of the dialog
	//
	SetWindowText (m_Title);
	SetDlgItemText (IDC_TEXT, m_Text);
	SetDlgItemText (IDC_DESCRIPTION, m_Description);

	//
	//	Load the icon
	//
	HICON icon = NULL;
	if (m_IconID == 0) {		
		icon = ::LoadIcon (NULL, IDI_INFORMATION);
	} else {
		icon = ::LoadIcon (::AfxGetResourceHandle (), MAKEINTRESOURCE (m_IconID));
	}

	//
	//	Set the icon
	//
	if (icon != NULL) {
		SendDlgItemMessage (IDC_GENERIC_ICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)icon);
	}

	return TRUE;
}
