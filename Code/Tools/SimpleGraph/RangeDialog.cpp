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

// RangeDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SimpleGraph.h"
#include "RangeDialog.h"
#include "SimpleGraphView.h"
#include "mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
//	fnEditToFloatProc
//
/////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK
fnEditToFloatProc
(
	HWND		hwnd,
	UINT		message,
	WPARAM	wparam,
	LPARAM	lparam
)
{
	WNDPROC old_proc = (WNDPROC)::GetProp (hwnd, "OLD_WND_PROC");
	LRESULT result = 0L;

	if (message == WM_SETTEXT) {
		
		//
		//	Convert the textual value to a int, convert
		// the int to a float, and conver the float to
		// a string.
		//
		LPCTSTR string		= (LPCTSTR)lparam;
		if (::strchr (string, '.') != 0) {
			result = ::CallWindowProc (old_proc, hwnd, message, wparam, lparam);
		} else {
			int value			= ::atol ((LPCTSTR)lparam);
			float float_value	= value / 100.0F;
			CString new_text;
			new_text.Format ("%.2f", float_value);
			result = ::CallWindowProc (old_proc, hwnd, message, wparam, (LPARAM)(LPCTSTR)new_text);
		}

	} else if (message == WM_GETTEXT) {
		
		//
		//	Get the value (as text) from the control,
		// convert it to a float, convert the float
		// to a int, then convert the int back to
		// a string.
		//
		result				= ::CallWindowProc (old_proc, hwnd, message, wparam, lparam);		
		LPCTSTR string		= (LPCTSTR)lparam;
		if (::strchr (string, '.') != 0) {
			float float_value	= ::atof (string);
			int int_value		= int(float_value * 100);
			::itoa (int_value, (LPTSTR)lparam, 10);			
		} else {
			int int_value		= ::atoi (string) * 100;
			::itoa (int_value, (LPTSTR)lparam, 10);						
		}

		result = ::lstrlen ((LPTSTR)lparam);

	} else if (message == WM_CHAR) {
		
		//
		//	Check to see if this is one of the characters we allow
		// the user to type
		//
		if (	(wparam >= '0' && wparam <= '9') ||
				wparam == '.' ||
				wparam == VK_BACK ||
				wparam == '-')
		{
			result = ::CallWindowProc (old_proc, hwnd, message, wparam, lparam);
		}

	} else if (old_proc != NULL) {
		result = ::CallWindowProc (old_proc, hwnd, message, wparam, lparam);
	}

	return result;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Make_Edit_Float_Ctrl
//
/////////////////////////////////////////////////////////////////////////////
void
Make_Edit_Float_Ctrl (HWND edit_wnd)
{
	LONG_PTR old_proc = ::SetWindowLong (edit_wnd, GWLP_WNDPROC, (LONG_PTR)fnEditToFloatProc);
	SetProp (edit_wnd, "OLD_WND_PROC", (HANDLE)old_proc);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  SetDlgItemFloat
//
////////////////////////////////////////////////////////////////////////////
void
SetDlgItemFloat
(
	HWND hdlg,
	UINT child_id,
	float value
)
{
	// Convert the float to a string
	CString text;
	text.Format ("%.2f", value);

	// Pass the string onto the dialog control
	::SetDlgItemText (hdlg, child_id, text);
	return ;
}

////////////////////////////////////////////////////////////////////////////
//
//  GetDlgItemFloat
//
////////////////////////////////////////////////////////////////////////////
float
GetDlgItemFloat
(
	HWND hdlg,
	UINT child_id
)
{
	// Get the string from the window
	TCHAR string_value[20];
	::GetDlgItemText (hdlg, child_id, string_value, sizeof (string_value));

	// Convert the string to a float and return the value
	return ::atof (string_value);
}


/////////////////////////////////////////////////////////////////////////////
//
// CRangeDialog
//
/////////////////////////////////////////////////////////////////////////////
CRangeDialog::CRangeDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CRangeDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRangeDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


void CRangeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRangeDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRangeDialog, CDialog)
	//{{AFX_MSG_MAP(CRangeDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// CRangeDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
CRangeDialog::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	CSimpleGraphView *view = (CSimpleGraphView *)((CMainFrame *)::AfxGetMainWnd ())->GetActiveView ();
	
	Vector2 range_min;
	Vector2 range_max;
	view->Get_Ranges (range_min, range_max);
	
	/*::Make_Edit_Float_Ctrl (::GetDlgItem (m_hWnd, IDC_MIN_X));
	::Make_Edit_Float_Ctrl (::GetDlgItem (m_hWnd, IDC_MIN_Y));
	::Make_Edit_Float_Ctrl (::GetDlgItem (m_hWnd, IDC_MAX_X));
	::Make_Edit_Float_Ctrl (::GetDlgItem (m_hWnd, IDC_MAX_Y));*/

	SetDlgItemFloat (m_hWnd, IDC_MIN_X, range_min.X);
	SetDlgItemFloat (m_hWnd, IDC_MIN_Y, range_min.Y);
	SetDlgItemFloat (m_hWnd, IDC_MAX_X, range_max.X);
	SetDlgItemFloat (m_hWnd, IDC_MAX_Y, range_max.Y);
	
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
CRangeDialog::OnOK (void)
{
	Vector2 range_min;
	Vector2 range_max;
	range_min.X = ::GetDlgItemFloat (m_hWnd, IDC_MIN_X);
	range_min.Y = ::GetDlgItemFloat (m_hWnd, IDC_MIN_Y);
	range_max.X = ::GetDlgItemFloat (m_hWnd, IDC_MAX_X);
	range_max.Y = ::GetDlgItemFloat (m_hWnd, IDC_MAX_Y);
	
	
	CSimpleGraphView *view = (CSimpleGraphView *)((CMainFrame *)::AfxGetMainWnd ())->GetActiveView ();
	view->Set_Ranges (range_min, range_max);
	view->InvalidateRect (NULL, true);
	view->UpdateWindow ();


	CDialog::OnOK ();
	return ;
}
