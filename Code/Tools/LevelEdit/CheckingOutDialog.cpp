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

// CheckingOutDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "leveledit.h"
#include "CheckingOutDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CheckingOutDialogClass dialog


CheckingOutDialogClass::CheckingOutDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(CheckingOutDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(CheckingOutDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CheckingOutDialogClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CheckingOutDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CheckingOutDialogClass, CDialog)
	//{{AFX_MSG_MAP(CheckingOutDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// WindowProc
//
LRESULT
CheckingOutDialogClass::WindowProc
(
	UINT message,
	WPARAM wParam,
	LPARAM lParam
) 
{
	if (message == WM_USER+101) {
		EndDialog (1);
	}
	
	// Allow the base class to process this message	
	return CDialog::WindowProc(message, wParam, lParam);
}


/////////////////////////////////////////////////////////////////////////////
//
// OnCommand
//
BOOL
CheckingOutDialogClass::OnCommand
(
	WPARAM wParam,
	LPARAM lParam
) 
{
	// Don't let the dialog be closed by hitting enter or escape
	if (LOWORD (wParam) == IDOK || LOWORD (wParam) == IDCANCEL) {
		return FALSE;
	}
	
	// Allow the base class to process this message	
	return CDialog::OnCommand(wParam, lParam);
}


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
BOOL
CheckingOutDialogClass::OnInitDialog (void) 
{
	// Allow the base class to process this message	
	CDialog::OnInitDialog ();
		
	return TRUE;
}
