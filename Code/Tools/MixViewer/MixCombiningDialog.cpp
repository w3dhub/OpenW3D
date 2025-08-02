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

// MixCombiningDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "MixViewer.h"
#include "MixCombiningDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// MixCombiningDialogClass
//
/////////////////////////////////////////////////////////////////////////////
MixCombiningDialogClass::MixCombiningDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(MixCombiningDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(MixCombiningDialogClass)
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
MixCombiningDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MixCombiningDialogClass)
	DDX_Control(pDX, IDC_PROGRESS, ProgressCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(MixCombiningDialogClass, CDialog)
	//{{AFX_MSG_MAP(MixCombiningDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
MixCombiningDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	ProgressCtrl.SetPos (0);
	ProgressCtrl.SetRange (0, 100);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// Set_Status_Text
//
/////////////////////////////////////////////////////////////////////////////
void
MixCombiningDialogClass::Set_Status_Text (const char *text)
{
	while (!::IsWindow(m_hWnd)) {}
	SetDlgItemText (IDC_STATUS, text);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Set_Progress_Percent
//
/////////////////////////////////////////////////////////////////////////////
void
MixCombiningDialogClass::Set_Progress_Percent (float percent)
{
	while (!::IsWindow(m_hWnd)) {}
	ProgressCtrl.SetPos ((int)(percent * 100.0f));
	return ;
}

/////////////////////////////////////////////////////////////////////////////
//
// Set_Title
//
/////////////////////////////////////////////////////////////////////////////
void MixCombiningDialogClass::Set_Title(const char *text)
{
	while (!::IsWindow(m_hWnd)) {}
	SetWindowText(text);
}