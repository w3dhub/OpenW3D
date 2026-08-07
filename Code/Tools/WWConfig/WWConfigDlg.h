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

// WWConfigDlg.h : header file
//

#if !defined(AFX_WWCONFIGDLG_H__7778B545_D680_49AC_A0CF_5F50AE8EBC40__INCLUDED_)
#define AFX_WWCONFIGDLG_H__7778B545_D680_49AC_A0CF_5F50AE8EBC40__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include "vector.h"


/////////////////////////////////////////////////////////////////////////////
//
// CWWConfigDlg
//
/////////////////////////////////////////////////////////////////////////////
class CWWConfigDlg : public CDialog
{
// Construction
public:
	CWWConfigDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CWWConfigDlg)
	enum { IDD = IDD_WWCONFIG_DIALOG };
	CTabCtrl	m_TabCtrl;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWWConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CWWConfigDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	afx_msg void OnSelchangeTabCtrl(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	/////////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////////

private:

	/////////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////////
	DynamicVectorClass<CDialog *>		ConfigPages;
	int										CurrentTab;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WWCONFIGDLG_H__7778B545_D680_49AC_A0CF_5F50AE8EBC40__INCLUDED_)
