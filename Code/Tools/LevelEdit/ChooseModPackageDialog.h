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

#if !defined(AFX_CHOOSEMODPACKAGEDIALOG_H__86FFB2F7_5103_422D_B327_5DCF8343713C__INCLUDED_)
#define AFX_CHOOSEMODPACKAGEDIALOG_H__86FFB2F7_5103_422D_B327_5DCF8343713C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"


/////////////////////////////////////////////////////////////////////////////
//
// ChooseModPackageDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class ChooseModPackageDialogClass : public CDialog
{
// Construction
public:
	ChooseModPackageDialogClass(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ChooseModPackageDialogClass)
	enum { IDD = IDD_CHOOSE_MOD_PACKAGE };
	CListCtrl	ListCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ChooseModPackageDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ChooseModPackageDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnNewButton();
	afx_msg void OnDblclkModListCtrl(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

private:

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSEMODPACKAGEDIALOG_H__86FFB2F7_5103_422D_B327_5DCF8343713C__INCLUDED_)
