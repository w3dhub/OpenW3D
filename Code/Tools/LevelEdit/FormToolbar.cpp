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

////////////////////////////////////////////////////////////////////////
//
//	FormToolbarClass.CPP
//
//	Module defining a CControlBar derived class which is used to implement
// floating or dockable toolbars containing dialogs.
//
//

#include "StdAfx.h"
#include "FormToolbar.H"
#include "DockableForm.H"


BEGIN_MESSAGE_MAP(FormToolbarClass, CControlBar)
	//{{AFX_MSG_MAP(FormToolbarClass)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////
//
//  Local constants
//
const int BORDER_TOP			= 6;
const int BORDER_BOTTOM		= 6;
const int BORDER_LEFT		= 6;
const int BORDER_RIGHT		= 6;


//////////////////////////////////////////////////////////////
//
//  FormToolbarClass
//
FormToolbarClass::FormToolbarClass (void)
	: m_pCForm (NULL)
{
    m_minSize.cx = 100;
    m_minSize.cy = 100;    
    return ;
}

//////////////////////////////////////////////////////////////
//
//  ~FormToolbarClass
//
FormToolbarClass::~FormToolbarClass (void)
{
	if (m_pCForm) {
		// Free the dockable form
		delete m_pCForm;
		m_pCForm = NULL;
	}

	return ;
}

//////////////////////////////////////////////////////////////
//
//  Create
//
BOOL
FormToolbarClass::Create
(
	DockableFormClass *pCFormClass,
	LPCTSTR pszWindowName,
	CWnd *pCParentWnd,
	UINT uiID
)
{
	ASSERT (pCFormClass);
	m_pCForm = pCFormClass;

	// Allow the base class to process this message	
	RECT rect = { 0 };
	BOOL retval = CWnd::Create (NULL, pszWindowName, WS_CHILD | WS_VISIBLE, rect, pCParentWnd, uiID);
	if (retval) {

		// Ask the dockable form to create itself
		retval = m_pCForm->Create (this, 101);


		CRect rect;
		rect = m_pCForm->Get_Form_Rect ();
		m_minSize.cx = rect.Width ();
		m_minSize.cy = rect.Height ();
		m_minSize.cx += BORDER_LEFT + BORDER_RIGHT;
		m_minSize.cy += BORDER_TOP + BORDER_BOTTOM;
		SetWindowPos (NULL, 0, 0, m_minSize.cx, m_minSize.cy, SWP_NOZORDER | SWP_NOMOVE);
		m_pCForm->SetWindowPos (NULL, BORDER_LEFT, BORDER_TOP, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

		// Allow the toolbar to be resized
		// Allow the toolbar to be docked either to the right or left
		SetBarStyle (GetBarStyle() | CBRS_SIZE_DYNAMIC);
		EnableDocking (CBRS_ALIGN_RIGHT | CBRS_ALIGN_LEFT);
	}

	// Return the TRUE/FALSE result code
	return retval;
}


//////////////////////////////////////////////////////////////
//
//  OnSize
//
void
FormToolbarClass::OnSize
(
	UINT nType,
	int cx,
	int cy
)
{
	// Allow the base class to process this message	
	CControlBar::OnSize(nType, cx, cy);

	if (m_pCForm && (cx > 0) && (cy > 0)) {		
		// Get the bounding rectangle
		CRect rect;
		GetClientRect (rect);

		// Resize the dockable form window
		m_pCForm->SetWindowPos (NULL,
										0,
										0,
										rect.Width () - (BORDER_LEFT + BORDER_RIGHT),
										rect.Height () - (BORDER_TOP + BORDER_BOTTOM),
										SWP_NOZORDER | SWP_NOMOVE);
	}

	return ;	
}

//////////////////////////////////////////////////////////////
//
//  OnEraseBkgnd
//
BOOL
FormToolbarClass::OnEraseBkgnd (CDC* pDC) 
{
	// Get the bounding rectangle
	RECT rect;
	GetClientRect (&rect);

	// Paint the background light gray
	::FillRect (*pDC, &rect, (HBRUSH)(COLOR_3DFACE + 1));

	// Allow the base class to process this message	
	return CControlBar::OnEraseBkgnd(pDC);
}
