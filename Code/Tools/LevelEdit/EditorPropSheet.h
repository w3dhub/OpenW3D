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

#if !defined(AFX_EDITORPROPSHEETCLASS_H__E2623E75_740A_11D2_9FD0_00104B791122__INCLUDED_)
#define AFX_EDITORPROPSHEETCLASS_H__E2623E75_740A_11D2_9FD0_00104B791122__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// EditorPropSheetClass.h : header file
//

#include "dockableform.h"
#include "vector.h"
#include "resource.h"

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif


/////////////////////////////////////////////////////////////////////////////
//
// EditorPropSheetClass
//
/////////////////////////////////////////////////////////////////////////////
class EditorPropSheetClass : public CDialog
{
	public:
		EditorPropSheetClass (void);
		virtual ~EditorPropSheetClass (void);

// Form Data
public:
	//{{AFX_DATA(EditorPropSheetClass)
	enum { IDD = IDD_PROP_SHEET_TEMPLATE };
	CTabCtrl	m_TabCtrl;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EditorPropSheetClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(EditorPropSheetClass)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangeTabCtrl(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnOk();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:

		///////////////////////////////////////////////////////
		//	Public methods
		///////////////////////////////////////////////////
		virtual void	Add_Page (DockableFormClass *pnew_tab) { if (m_hWnd == NULL) m_TabList.Add (pnew_tab); }

		bool				Is_Read_Only (void) const	{ return m_IsReadOnly; }
		void				Set_Read_Only (bool onoff)	{ m_IsReadOnly = onoff; }

	protected:

		///////////////////////////////////////////////////
		//	Protected methods
		///////////////////////////////////////////////////
		virtual void	Reposition_Buttons (int cx, int cy);
		virtual bool	Apply_Changes (void);

		///////////////////////////////////////////////////
		//	Protected member data
		///////////////////////////////////////////////////
		DynamicVectorClass<DockableFormClass *> m_TabList;


	private:

		///////////////////////////////////////////////////
		//	Private member data
		///////////////////////////////////////////////////
		int	m_iCurrentTab;
		bool	m_IsReadOnly;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITORPROPSHEETCLASS_H__E2623E75_740A_11D2_9FD0_00104B791122__INCLUDED_)
