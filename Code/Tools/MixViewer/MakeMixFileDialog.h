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

#if !defined(AFX_MAKEMIXFILEDIALOG_H__8A52B67D_3497_4915_825C_199D6EC5995D__INCLUDED_)
#define AFX_MAKEMIXFILEDIALOG_H__8A52B67D_3497_4915_825C_199D6EC5995D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MakeMixFileDialog.h : header file
//

#include "mixfile.h"

/////////////////////////////////////////////////////////////////////////////
// MakeMixFileDialogClass dialog

class MakeMixFileDialogClass : public CDialog
{
// Construction
public:
	MakeMixFileDialogClass(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(MakeMixFileDialogClass)
	enum { IDD = IDD_MAKE_MIX_FILE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MakeMixFileDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	unsigned Add_Files (const StringClass &basepath, const StringClass &subpath, MixFileCreator &mixfile);
	StringClass	MixFilename;

	// Generated message map functions
	//{{AFX_MSG(MakeMixFileDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBrowseFile();
	afx_msg void OnBrowseDir();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAKEMIXFILEDIALOG_H__8A52B67D_3497_4915_825C_199D6EC5995D__INCLUDED_)
