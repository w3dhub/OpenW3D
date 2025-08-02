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

// GeneratingPathfindDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "leveledit.h"
#include "GeneratingPathfindDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// GeneratingPathfindDialogClass
//
/////////////////////////////////////////////////////////////////////////////
GeneratingPathfindDialogClass::GeneratingPathfindDialogClass(CWnd *parent)
	:	m_Cancelled (false),
		CDialog(GeneratingPathfindDialogClass::IDD, parent)
{
	//{{AFX_DATA_INIT(GeneratingPathfindDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	Create (GeneratingPathfindDialogClass::IDD, parent);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
GeneratingPathfindDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange (pDX);
	//{{AFX_DATA_MAP(GeneratingPathfindDialogClass)
	DDX_Control(pDX, IDC_PROGRESS_BAR, m_ProgressBar);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(GeneratingPathfindDialogClass, CDialog)
	//{{AFX_MSG_MAP(GeneratingPathfindDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
GeneratingPathfindDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	Set_State (STATE_FLOODFILL);
	m_ProgressBar.SetRange (0, 100);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnCancel
//
/////////////////////////////////////////////////////////////////////////////
void
GeneratingPathfindDialogClass::OnCancel (void)
{
	m_Cancelled = true;

	::EnableWindow (::GetDlgItem (m_hWnd, IDCANCEL), FALSE);
	CWaitCursor wait_cursor;

	Set_Status ("Operation Cancelled!", -1);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Set_State
//
/////////////////////////////////////////////////////////////////////////////
void
GeneratingPathfindDialogClass::Set_State (STATE state)
{
	if (state == STATE_FLOODFILL) {
		::ShowWindow (::GetDlgItem (m_hWnd, IDC_FLOODFILL_MARK), SW_SHOW);
		::ShowWindow (::GetDlgItem (m_hWnd, IDC_COMPRESSING_MARK), SW_HIDE);
	} else if (state == STATE_COMPRESS) {
		::ShowWindow (::GetDlgItem (m_hWnd, IDC_FLOODFILL_MARK), SW_HIDE);
		::ShowWindow (::GetDlgItem (m_hWnd, IDC_COMPRESSING_MARK), SW_SHOW);		
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Set_Status
//
/////////////////////////////////////////////////////////////////////////////
void
GeneratingPathfindDialogClass::Set_Status (LPCTSTR text, float percent)
{
	if (percent > 0) {
		m_ProgressBar.SetPos (percent * 100);
	}
	
	SetDlgItemText (IDC_STATUS_TEXT, text);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// WindowProc
//
/////////////////////////////////////////////////////////////////////////////
LRESULT
GeneratingPathfindDialogClass::WindowProc
(
	UINT		message,
	WPARAM	wParam,
	LPARAM	lParam
)
{
	if (message == WM_USER + 101) {
		::DestroyWindow (m_hWnd);
		::PostQuitMessage (0);		
	}

	return CDialog::WindowProc (message, wParam, lParam);
}
