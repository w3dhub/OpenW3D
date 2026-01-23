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

// UpdatingDBDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "UpdatingDBDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// UpdatingDBDialogClass dialog


UpdatingDBDialogClass::UpdatingDBDialogClass(HWND /* hparentwnd */)
	: CDialog(UpdatingDBDialogClass::IDD, NULL)
{
	//{{AFX_DATA_INIT(UpdatingDBDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	//CWnd *pnew_wnd = new CWnd;
	//new_wnd.Attach (hparentwnd);
	//pnew_wnd->Attach (hparentwnd);
	Create (UpdatingDBDialogClass::IDD, ::AfxGetMainWnd ());
	//new_wnd.Detach ();
	return; 
}


void UpdatingDBDialogClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(UpdatingDBDialogClass)
	DDX_Control(pDX, IDC_ANMATION_CTRL, m_AnimationCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(UpdatingDBDialogClass, CDialog)
	//{{AFX_MSG_MAP(UpdatingDBDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
//	WindowProc
//
LRESULT
UpdatingDBDialogClass::WindowProc
(
	UINT message,
	WPARAM wParam,
	LPARAM lParam
)
{
	// Is this the call to close the dialog?
	if (message == WM_USER+101) {
		::DestroyWindow (m_hWnd);
		::PostQuitMessage (0);
		return 0;
	} else if (message == WM_SETCURSOR) {
		SetCursor (::LoadCursor (NULL, IDC_WAIT));
		return 0;
	}
	
	// Allow the base class to process this message
	return CDialog::WindowProc (message, wParam, lParam);
}


/////////////////////////////////////////////////////////////////////////////
//
//	PostNcDestroy
//
void
UpdatingDBDialogClass::PostNcDestroy (void) 
{
	// Allow the base class to process this message
	CDialog::PostNcDestroy ();

	delete this;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnInitDialog
//
BOOL
UpdatingDBDialogClass::OnInitDialog (void)
{
	// Allow the base class to process this message
	CDialog::OnInitDialog();
	
	// Center the window
	HWND hparentwnd = ::GetParent (m_hWnd);
	if (::IsWindow (hparentwnd)) {
		
		// Get the rectangles of the window and its parent
		CRect parent_rect;
		CRect rect;
		GetWindowRect (&rect);
		::GetWindowRect (hparentwnd, &parent_rect);
		
		// Center the dialog ontop of its parent
		::SetWindowPos (m_hWnd,
							 NULL,
							 parent_rect.left + (parent_rect.Width () >> 1) - (rect.Width () >> 1),
							 parent_rect.top + (parent_rect.Height () >> 1) - (rect.Height () >> 1),
							 0,
							 0,
							 SWP_NOZORDER | SWP_NOSIZE);
	}

	m_AnimationCtrl.Open (IDR_FILECOPY_ANI);	
	return true;
}
