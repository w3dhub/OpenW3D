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

// FileCopyDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CommandoUpdate.h"
#include "FileCopyDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// FileCopyDialogClass dialog


FileCopyDialogClass::FileCopyDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(FileCopyDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(FileCopyDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void FileCopyDialogClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FileCopyDialogClass)
	DDX_Control(pDX, IDC_ANIMATE_CTRL, m_AnimateCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(FileCopyDialogClass, CDialog)
	//{{AFX_MSG_MAP(FileCopyDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////////
//
//	WindowProc
//
//////////////////////////////////////////////////////////////////////////////
LRESULT
FileCopyDialogClass::WindowProc
(
	UINT		message,
	WPARAM	wParam,
	LPARAM	lParam
)
{	
	if (message == WM_USER + 101) {
		EndDialog (true);
	}
	
	return CDialog::WindowProc(message, wParam, lParam);
}


//////////////////////////////////////////////////////////////////////////////
//
//	OnInitDialog
//
//////////////////////////////////////////////////////////////////////////////
BOOL
FileCopyDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog();
	
	CString app_title;
	app_title.Format ("Application: %s", m_AppTitle);
	SetDlgItemText (IDC_CURRENT_APP, app_title);

	m_AnimateCtrl.Open (IDR_FILECOPY_AVI);	

	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}
