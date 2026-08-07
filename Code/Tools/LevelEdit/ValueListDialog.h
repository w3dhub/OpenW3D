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

#if !defined(AFX_VALUELISTDIALOGCLASS_H__47FC1C6B_780B_11D2_9FD1_00104B791122__INCLUDED_)
#define AFX_VALUELISTDIALOGCLASS_H__47FC1C6B_780B_11D2_9FD1_00104B791122__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ValueListDialogClass.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ValueListDialogClass dialog

class ValueListDialogClass : public CDialog
{
// Construction
public:
	ValueListDialogClass(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ValueListDialogClass)
	enum { IDD = IDD_DATATYPE_LIST };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ValueListDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ValueListDialogClass)
	virtual void OnOK();
	afx_msg void OnUpdateListEdit();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:
		const CString &		Get_List (void) const { return m_ListEntries; }
		void						Set_List (const CString &list)  { m_ListEntries = list; }

	private:
		CString			m_ListEntries;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VALUELISTDIALOGCLASS_H__47FC1C6B_780B_11D2_9FD1_00104B791122__INCLUDED_)
