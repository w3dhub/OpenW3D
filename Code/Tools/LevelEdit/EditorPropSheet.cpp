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

// EditorPropSheetClass.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "EditorPropSheet.H"
#include "Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// Local constants
/////////////////////////////////////////////////////////////////////////////
static const int BORDER_BUTTON_X		= 6;
static const int BORDER_BUTTON_Y		= 6;
static const int BORDER_TAB_X			= 6;
static const int BORDER_TAB_Y			= 6;


/////////////////////////////////////////////////////////////////////////////
//
// EditorPropSheetClass
//
/////////////////////////////////////////////////////////////////////////////
EditorPropSheetClass::EditorPropSheetClass (void)
	:	m_iCurrentTab (0),
		m_IsReadOnly (false),
		CDialog(EditorPropSheetClass::IDD, ::AfxGetMainWnd ())
{
	//{{AFX_DATA_INIT(EditorPropSheetClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~EditorPropSheetClass
//
/////////////////////////////////////////////////////////////////////////////
EditorPropSheetClass::~EditorPropSheetClass (void)
{
	m_TabList.Delete_All ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
EditorPropSheetClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EditorPropSheetClass)
	DDX_Control(pDX, IDC_TABCTRL, m_TabCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(EditorPropSheetClass, CDialog)
	//{{AFX_MSG_MAP(EditorPropSheetClass)
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TABCTRL, OnSelchangeTabCtrl)
	ON_BN_CLICKED(IDC_OK, OnOk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EditorPropSheetClass diagnostics

#ifdef _DEBUG
void EditorPropSheetClass::AssertValid() const
{
	CDialog::AssertValid();
}

void EditorPropSheetClass::Dump(CDumpContext& dc) const
{
	CDialog::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
//
// Reposition_Buttons
//
/////////////////////////////////////////////////////////////////////////////
void
EditorPropSheetClass::Reposition_Buttons (int cx, int cy)
{
	//
	//	Get the dimensions of the buttons
	//
	CRect button_rect;
	::GetWindowRect (::GetDlgItem (m_hWnd, IDCANCEL), &button_rect);
	ScreenToClient (&button_rect);		

	//
	//	Reposition the OK and Cancel buttons
	//
	::SetWindowPos (::GetDlgItem (m_hWnd, IDCANCEL),
						 NULL,
						 (cx - button_rect.Width ()) - BORDER_BUTTON_X,
						 (cy - button_rect.Height ()) - BORDER_BUTTON_Y,
						 0,
						 0,
						 SWP_NOZORDER | SWP_NOSIZE);

	::SetWindowPos (::GetDlgItem (m_hWnd, IDC_OK),
						 NULL,
						 (cx - (button_rect.Width () << 1)) - (BORDER_BUTTON_X * 2),
						 (cy - button_rect.Height ()) - BORDER_BUTTON_Y,
						 0,
						 0,
						 SWP_NOZORDER | SWP_NOSIZE);

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSize
//
/////////////////////////////////////////////////////////////////////////////
void
EditorPropSheetClass::OnSize
(
	UINT nType,
	int cx,
	int cy
)
{
	// Allow the base class to process this message
	CDialog::OnSize (nType, cx, cy);

	if (::IsWindow (m_TabCtrl) && (cx > 0) && (cy > 0)) {		
		
		//
		//	Get the dimensions of the buttons
		//
		CRect button_rect;
		::GetWindowRect (::GetDlgItem (m_hWnd, IDCANCEL), &button_rect);
		ScreenToClient (&button_rect);

		//
		//	Move the buttons so they continue to be in the lower right corner
		//
		Reposition_Buttons (cx, cy);

		//
		// Shrink the bounds by BORDER_TAB_X (to give a little border)
		//
		cx -= BORDER_TAB_X * 2;
		cy -= (BORDER_TAB_Y * 2) + (button_rect.Height () + BORDER_BUTTON_Y);
		
		// Resize the tab control to fill the entire contents of the client area
		m_TabCtrl.SetWindowPos (NULL, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOMOVE);	

		// Get the display rectangle of the tab control
		CRect rect;
		m_TabCtrl.GetWindowRect (&rect);
		m_TabCtrl.AdjustRect (false, &rect);

		// Convert the display rectangle from screen to client coords
		ScreenToClient (&rect);

		// Loop through all the tabs in the property sheet
		for (int tab = 0;
			  tab < m_TabList.Count ();
			  tab ++) {
				
			// Get a pointer to this tab
			DockableFormClass *ptab = m_TabList[tab];
			if (ptab != NULL) {
				// Resize this tab
				ptab->SetWindowPos (NULL, rect.left, rect.top, rect.Width (), rect.Height (), SWP_NOZORDER);
			}
		}
	}
		
	return ;	
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSelchangeTabCtrl
//
/////////////////////////////////////////////////////////////////////////////
void
EditorPropSheetClass::OnSelchangeTabCtrl
(
	NMHDR* /* pNMHDR */,
	LRESULT* pResult
) 
{
	// Which tab is selected?
	int newtab = m_TabCtrl.GetCurSel ();

	// Is this a new tab?
	if (m_iCurrentTab != newtab) {

		// Is the old tab valid?
		if (m_TabList[m_iCurrentTab] != NULL) {
			// Hide the contents of the old tab
			m_TabList[m_iCurrentTab]->ShowWindow (SW_HIDE);
		}

		// Is the new tab valid?
		if (m_TabList[newtab] != NULL) {
			// Show the contents of the new tab
			m_TabList[newtab]->ShowWindow (SW_SHOW);
		}

		// Remember what our new current tab is
		m_iCurrentTab = newtab;		
	}
	
	(*pResult) = 0;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
EditorPropSheetClass::OnInitDialog (void) 
{
	// Allow the base class to process this message
	CDialog::OnInitDialog ();

	// Move the tab control so it starts at 2, 2
	m_TabCtrl.SetWindowPos (NULL, BORDER_TAB_X, BORDER_TAB_Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	
	// Loop through all the tabs in our list
	int cx = 0;
	int cy = 0;
	for (int tab = 0;
		  tab < m_TabList.Count ();
		  tab ++) {
			
		// Get a pointer to this tab
		DockableFormClass *ptab = m_TabList[tab];
		
		// Was the tab pointer valid?
		ASSERT (ptab != NULL);
		if (ptab != NULL) {

			// Create the window associated with this tab
			ptab->Create (this, 100 + tab);			

			// Hide the tab if it isn't the first one
			ptab->ShowWindow ((tab == 0) ? SW_SHOW : SW_HIDE);

			// Get the name of this tab from its form
			CString tab_name;
			ptab->GetWindowText (tab_name);
			
			// Determine if we need to stretch our cx and cy
			CRect rect = ptab->Get_Form_Rect ();
			cx = (rect.Width () > cx) ? rect.Width () : cx;
			cy = (rect.Height () > cy) ? rect.Height() : cy;

			ptab->SetWindowPos (NULL, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER);

			// Now insert a tab into the tab control for this form
			TC_ITEM tabInfo = { 0 };
			tabInfo.mask = TCIF_TEXT;
			tabInfo.pszText = (LPTSTR)(LPCTSTR)tab_name;
			m_TabCtrl.InsertItem (tab, &tabInfo);
		}
	}

	m_TabList[0]->SetFocus ();

	if ((cx > 0) && (cy > 0)) {

		// Get the display rectangle of the tab control
		CRect rect (0, 0, cx, cy);
		m_TabCtrl.AdjustRect (true, &rect);

		CRect button_rect;
		::GetWindowRect (::GetDlgItem (m_hWnd, IDCANCEL), &button_rect);
		ScreenToClient (&button_rect);		

		// Find a rectangle large enough to hold the tab control
		::AdjustWindowRectEx (&rect, ::GetWindowLong (m_hWnd, GWL_STYLE), false, ::GetWindowLong (m_hWnd, GWL_EXSTYLE));
		rect.bottom += button_rect.Height () + BORDER_BUTTON_Y;
		rect.InflateRect (BORDER_TAB_X, BORDER_TAB_Y);

		// Resize the dialog to be large enough to hold the tab control
		SetWindowPos (NULL, 0, 0, rect.Width (), rect.Height (), SWP_NOZORDER | SWP_NOMOVE);				
	}

	if (m_IsReadOnly) {
		::ShowWindow (::GetDlgItem (m_hWnd, IDC_OK), SW_HIDE);
		SetDlgItemText (IDCANCEL, "Close");
	}
	
	return false;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL
EditorPropSheetClass::OnCommand
(
	WPARAM wParam,
	LPARAM lParam
)
{
	// Did the user hit the enter button?
	if (LOWORD (wParam) == IDOK) {
		m_TabList[m_iCurrentTab]->SendMessage (WM_COMMAND, wParam, lParam);
		return true;
	}
	
	// Allow the base class to process this message
	return CDialog::OnCommand(wParam, lParam);
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOk
//
/////////////////////////////////////////////////////////////////////////////
void
EditorPropSheetClass::OnOk (void)
{
	if (Apply_Changes ()) {
		EndDialog (IDOK);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Apply_Changes
//
/////////////////////////////////////////////////////////////////////////////
bool
EditorPropSheetClass::Apply_Changes (void)
{
	bool is_valid = true;

	for (int index = 0; index < m_TabList.Count (); index ++) {
		DockableFormClass *tab = m_TabList[index];		
		ASSERT (tab != NULL);
		if (tab != NULL) {

			//
			// Have the tab apply its changes
			//
			is_valid &= tab->Apply_Changes ();
		}
	}

	return is_valid;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnCancel
//
/////////////////////////////////////////////////////////////////////////////
void
EditorPropSheetClass::OnCancel (void) 
{
	for (int tab = 0;
		  tab < m_TabList.Count ();
		  tab ++) {
			
		// Get a pointer to this tab
		DockableFormClass *ptab = m_TabList[tab];
		
		// Was the tab pointer valid?
		ASSERT (ptab != NULL);
		if (ptab != NULL) {
			
			// Have the tab discard its changes
			ptab->Discard_Changes ();
		}
	}

	CDialog::OnCancel ();
	return ;
}
