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

// WelcomeDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "WelcomeDialog.H"
#include "Utils.h"
#include "FileMgr.H"
#include "RegKeys.H"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// WelcomeDialogClass dialog


/////////////////////////////////////////////////////////////////
//
//	WelcomeDialogClass
//
WelcomeDialogClass::WelcomeDialogClass (CWnd* pParent /*=NULL*/)
	: m_bChangesTemp (false),
	  m_hBMP (NULL),
	  CDialog(WelcomeDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(WelcomeDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	DoDataExchange
//
void
WelcomeDialogClass::DoDataExchange (CDataExchange* pDX)
{
	// Allow the base class to process this message
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(WelcomeDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(WelcomeDialogClass, CDialog)
	//{{AFX_MSG_MAP(WelcomeDialogClass)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////
//
//	DoModal
//
INT_PTR
WelcomeDialogClass::DoModal (void) 
{
	int iret = IDOK;

	// Should this dialog be displayed?
	if (theApp.GetProfileInt (CONFIG_KEY, SHOW_WELCOME_VALUE, 1)) {
		iret = CDialog::DoModal ();
	}

	// Return the dialog exit code
	return iret;
}


/////////////////////////////////////////////////////////////////
//
//	OnInitDialog
//
BOOL
WelcomeDialogClass::OnInitDialog (void) 
{
	// Allow the base class to process this message
	CDialog::OnInitDialog ();

	// Check the 'yes' button by default
	SendDlgItemMessage (IDC_YES_RADIO, BM_SETCHECK, (WPARAM)TRUE);	

	// Put the base path in its dialog control
	SetDlgItemText (IDC_ASSET_TREE_LOCATION, ::Get_File_Mgr()->Get_Base_Path ());

	m_hBMP = ::LoadBitmap (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDB_WELCOME_BMP));
	if (m_hBMP != NULL) {
		
		// Determine the BMPs dimensions
		BITMAP bitmap = { 0 };
		::GetObject (m_hBMP, sizeof (BITMAP), &bitmap);
		m_iWidth = bitmap.bmWidth;
		m_iHeight = bitmap.bmHeight;
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////
//
//	OnOK
//
void
WelcomeDialogClass::OnOK (void)
{
	// Determine what option the user checked
	m_bChangesTemp = (bool)(SendDlgItemMessage (IDC_NO_RADIO, BM_GETCHECK) == 1);

	// Write an integer to the registry so we'll know whether to show this again or not.
	int ishow_again = (SendDlgItemMessage (IDC_DONT_ASK_AGAIN, BM_GETCHECK) == 0);	
	theApp.WriteProfileInt (CONFIG_KEY, SHOW_WELCOME_VALUE, ishow_again);
	
	// Allow the base class to process this message
	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	OnOK
//
void
WelcomeDialogClass::OnPaint (void) 
{
	CPaintDC dc (this);

	if (m_hBMP != NULL) {
		
		// Paint into the BMP window's contents
		HWND hwnd = ::GetDlgItem (m_hWnd, IDC_BMP);

		// Get the bounding rectangle so we know how much to paint
		RECT rect;
		::GetClientRect (hwnd, &rect);

		// Determine the width, height, and width per each shade
		int width = rect.right-rect.left;
		int height = rect.bottom-rect.top;

		// Get the DCs we need to do the blitting
		HDC hdc = ::GetDC (hwnd);
		HDC hmem_dc = ::CreateCompatibleDC (NULL);
		HBITMAP hold_BMP = (HBITMAP)::SelectObject (hmem_dc, m_hBMP);

		// Paint the BMP, centered, onto screen
		::BitBlt (hdc,
					 (width >> 1) - (m_iWidth >> 1),
					 (height >> 1) - (m_iHeight >> 1),
					 m_iWidth,
					 m_iHeight,
					 hmem_dc,
					 0,
					 0,
					 SRCCOPY);
					
		// Release the DCs
		::SelectObject (hmem_dc, hold_BMP);
		::ReleaseDC (hwnd, hdc);
		::DeleteDC (hmem_dc);

		// Validate the contents of the window so the control won't paint itself
		::ValidateRect (hwnd, NULL);
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	OnDestroy
//
void
WelcomeDialogClass::OnDestroy (void) 
{
	// Free the BMP if we need to
	if (m_hBMP != NULL) {
		::DeleteObject (m_hBMP);
		m_hBMP = NULL;
	}

	// Allow the base class to process this message
	CDialog::OnDestroy ();
	return ;
}
