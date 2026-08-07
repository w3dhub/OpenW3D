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

// W3DUpdateDlg.h : header file
//

#if !defined(AFX_W3DUPDATEDLG_H__4305AC50_1EAB_11D3_A038_00104B791122__INCLUDED_)
#define AFX_W3DUPDATEDLG_H__4305AC50_1EAB_11D3_A038_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CW3DUpdateDlg dialog

class CW3DUpdateDlg : public CDialog
{
// Construction
public:
	void RegisterViewer(int index);
	void RegisterShellExt();
	CW3DUpdateDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CW3DUpdateDlg)
	enum { IDD = IDD_W3DUPDATE_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CW3DUpdateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CW3DUpdateDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	afx_msg void OnDefaults();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	///////////////////////////////////////////////////////////////
	//	Private methods
	///////////////////////////////////////////////////////////////
	void		Get_Destination_Path (int app_id, CString &dest_path);
	bool		Install_App (const CString &title, const CString &src_path, const CString &dest_path, const CString &reg_key_name, bool clean);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_W3DUPDATEDLG_H__4305AC50_1EAB_11D3_A038_00104B791122__INCLUDED_)
