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

// vidinitDlg.h : header file
//

#if !defined(AFX_VIDINITDLG_H__AC2150AD_B8EF_4511_BA4A_DEDC16A03EDF__INCLUDED_)
#define AFX_VIDINITDLG_H__AC2150AD_B8EF_4511_BA4A_DEDC16A03EDF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CVidinitDlg dialog
class CSurrenderWnd;
class CVidinitDlg : public CDialog
{
// Construction
public:
	CVidinitDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CVidinitDlg)
	enum { IDD = IDD_VIDINIT_DIALOG };
	CListBox	m_Resolutions;
	CListBox	m_Drivers;
	BOOL		m_RunFullScreen;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVidinitDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON	m_hIcon;
	LPCTSTR	m_WndClass;
	CSurrenderWnd *  m_TestCWnd;

	char	m_DeviceName[255];
	int		m_DeviceWidth;
	int		m_DeviceHeight;
	int		m_DeviceDepth;
	int		m_DeviceWindowed;

	// Generated message map functions
	//{{AFX_MSG(CVidinitDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelchangeDrivers();
	afx_msg void OnTest();
	//}}AFX_MSG
	afx_msg LRESULT OnDeadBeef(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIDINITDLG_H__AC2150AD_B8EF_4511_BA4A_DEDC16A03EDF__INCLUDED_)
