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

// DockableFormClass.cpp : implementation file
//

#include "StdAfx.h"
#include "DockableForm.H"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
//	DockableFormClass
//
DockableFormClass::DockableFormClass (UINT nIDTemplate)
	: m_uiTemplateID (nIDTemplate),
	  CWnd ()
{
	//{{AFX_DATA_INIT(DockableFormClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    return ;
}

/////////////////////////////////////////////////////////////////////////////
//
//	~DockableFormClass
//
DockableFormClass::~DockableFormClass (void)
{
	return ;
}

BEGIN_MESSAGE_MAP(DockableFormClass, CWnd)
	//{{AFX_MSG_MAP(DockableFormClass)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
//	Create
//
BOOL
DockableFormClass::Create
(
	LPCTSTR /*lpszClassName*/,
	LPCTSTR /*lpszWindowName*/,
	DWORD dwRequestedStyle,
	const RECT& rect,
	CWnd* pParentWnd,
	UINT nID,
	CCreateContext* /* pContext */
)
{
	ASSERT(pParentWnd != NULL);

	// call PreCreateWindow to get prefered extended style
	CREATESTRUCT cs; memset(&cs, 0, sizeof(CREATESTRUCT));
	if (dwRequestedStyle == 0) {
		dwRequestedStyle = AFX_WS_DEFAULT_VIEW;
	}
	cs.style = dwRequestedStyle;
	if (!PreCreateWindow(cs)) {
		return false;
	}

	// create a modeless dialog
	if (!CreateDlg(MAKEINTRESOURCE (m_uiTemplateID), pParentWnd)) {
		return false;
	}

	ExecuteDlgInit(MAKEINTRESOURCE (m_uiTemplateID));

	::SetWindowLong (m_hWnd, GWL_STYLE, ::GetWindowLong (m_hWnd, GWL_STYLE) & (~WS_CAPTION));


	SetDlgCtrlID(nID);

	GetWindowRect (m_rectForm);

	// force the size requested
	SetWindowPos(NULL, rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		SWP_NOZORDER|SWP_NOACTIVATE);

	// make visible if requested
	if (dwRequestedStyle & WS_VISIBLE) {
		ShowWindow(SW_NORMAL);
	}

	// To support dynamic data exchange...
	UpdateData (false);
	HandleInitDialog ();
	return true;
}

/////////////////////////////////////////////////////////////////////////////
//
//	OnCreate
//
int
DockableFormClass::OnCreate (LPCREATESTRUCT lpCreateStruct) 
{
	// Allow the base class to process this message
	if (CWnd::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}

	// we use the style from the template - but make sure that
	//  the WS_BORDER bit is correct
	// the WS_BORDER bit will be whatever is in dwRequestedStyle
	ModifyStyle(WS_BORDER|WS_CAPTION, lpCreateStruct->style & (WS_BORDER|WS_CAPTION));
	ModifyStyleEx(WS_EX_CLIENTEDGE, lpCreateStruct->dwExStyle & WS_EX_CLIENTEDGE);

	// initialize controls etc
	/*if (!ExecuteDlgInit(MAKEINTRESOURCE (m_uiTemplateID))) {
		return -1;
	}*/

	return 0;

}

////////////////////////////////////////////////////////////////////////////
//
//  WindowProc
//
LRESULT
DockableFormClass::WindowProc
(
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
	// Is this the message we are expecting?
	if (message == WM_SHOWWINDOW) {
		
		// Make sure the controls reflect the current state when we are
		// shown
		if ((BOOL)wParam) {
			//Update_Controls ();
		}
	}

	// Allow the base class to process this message	
	return CWnd::WindowProc(message, wParam, lParam);
}
