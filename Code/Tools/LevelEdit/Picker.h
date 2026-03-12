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

#if !defined(AFX_PICKERCLASS_H__1A8EC040_8874_11D2_9FE1_00104B791122__INCLUDED_)
#define AFX_PICKERCLASS_H__1A8EC040_8874_11D2_9FE1_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



/////////////////////////////////////////////////////////////////////////////
//
// PickerClass
//
/////////////////////////////////////////////////////////////////////////////
class PickerClass : public CStatic
{
// Construction
public:
	PickerClass();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PickerClass)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~PickerClass();

	// Generated message map functions
protected:
	//{{AFX_MSG(PickerClass)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:

	//////////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////////

	//
	//	Stuff
	//
	virtual bool		Create_Picker (DWORD style, const RECT &rect, CWnd *parent, UINT id);
	virtual void		Initialize_Control (void);
	virtual void		Set_Read_Only (bool readonly = true);

	//
	//	Icon control
	//
	virtual void		Set_Icon (HICON icon)	{ m_Icon = icon; }

	//
	//	Overrideables
	//
	virtual void		On_Pick (void) {}

protected:

	//////////////////////////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////////////////////////
	HWND		m_BrowseButton;
	HWND		m_EditCtrl;
	HICON		m_Icon;
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PICKERCLASS_H__1A8EC040_8874_11D2_9FE1_00104B791122__INCLUDED_)
