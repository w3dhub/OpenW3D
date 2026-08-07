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

#if !defined(AFX_FILECOPYDIALOG_H__4305AC5C_1EAB_11D3_A038_00104B791122__INCLUDED_)
#define AFX_FILECOPYDIALOG_H__4305AC5C_1EAB_11D3_A038_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileCopyDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// FileCopyDialogClass dialog

class FileCopyDialogClass : public CDialog
{
// Construction
public:
	FileCopyDialogClass(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(FileCopyDialogClass)
	enum { IDD = IDD_FILE_COPY };
	CAnimateCtrl	m_AnimateCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(FileCopyDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(FileCopyDialogClass)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:
		void			Set_Current_File (LPCTSTR filename)				{ if (m_hWnd != nullptr) SetDlgItemText (IDC_FILENAME_TEXT, filename); }
		void			Set_Current_Application (LPCTSTR filename)	{ m_AppTitle = filename; }

	private:
		CString		m_AppTitle;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILECOPYDIALOG_H__4305AC5C_1EAB_11D3_A038_00104B791122__INCLUDED_)
