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

#if !defined(AFX_DOCKABLEFORMCLASS_H__53843321_66CC_11D2_8DDF_00104B6FD9E3__INCLUDED_)
#define AFX_DOCKABLEFORMCLASS_H__53843321_66CC_11D2_8DDF_00104B6FD9E3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DockableFormClass.h : header file
//


#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

/////////////////////////////////////////////////////////////////////////////
//
//	DockableFormClass form view
//
class DockableFormClass : public CWnd
{
	public:
		DockableFormClass (UINT nIDTemplate);
		virtual ~DockableFormClass ();


// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DockableFormClass)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DockableFormClass)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	protected:

		////////////////////////////////////////////////////////////////
		//
		//	Protected methods
		//
		BOOL Create(LPCTSTR /*lpszClassName*/, LPCTSTR /*lpszWindowName*/,
						DWORD dwRequestedStyle, const RECT& rect, CWnd* pParentWnd, UINT nID,
						CCreateContext* pContext);

	public:

		////////////////////////////////////////////////////////////////
		//
		//	Public Methods
		//
		BOOL				Create (CWnd *pCParentWnd, UINT uiID)	{ return Create (NULL, NULL, WS_CHILD | WS_VISIBLE, CRect (0, 0, 100, 100), pCParentWnd, uiID, NULL); }
		virtual void	HandleInitDialog (void) {}
		virtual bool	Apply_Changes (void) { return true; }
		virtual void	Discard_Changes (void) {}

		//
		//	Inline accessors
		//
		const CRect &	Get_Form_Rect (void) const
			{ return m_rectForm; }

	private:

		////////////////////////////////////////////////////////////////
		//
		//	Private member data
		//
		UINT m_uiTemplateID;
		CRect m_rectForm;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DOCKABLEFORMCLASS_H__53843321_66CC_11D2_8DDF_00104B6FD9E3__INCLUDED_)
