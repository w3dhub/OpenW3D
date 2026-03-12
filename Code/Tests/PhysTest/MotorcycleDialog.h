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

#if !defined(AFX_MOTORCYCLEDIALOG_H__AD117871_03C8_11D3_9BDD_00A0C9988171__INCLUDED_)
#define AFX_MOTORCYCLEDIALOG_H__AD117871_03C8_11D3_9BDD_00A0C9988171__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MotorcycleDialog.h : header file
//

class MotorcycleClass;

/////////////////////////////////////////////////////////////////////////////
// CMotorcycleDialog dialog

class CMotorcycleDialog : public CDialog
{
// Construction
public:
	CMotorcycleDialog(CWnd* pParent,MotorcycleClass * obj);

// Dialog Data
	//{{AFX_DATA(CMotorcycleDialog)
	enum { IDD = IDD_MOTORCYCLE_DIALOG };
	CSpinButtonCtrl	m_LeanK1Spin;
	CSpinButtonCtrl	m_LeanK0Spin;
	CSpinButtonCtrl	m_BalanceK0Spin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMotorcycleDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:

	MotorcycleClass * EditedObject;

	float GetDlgItemFloat(int controlid);
	void SetDlgItemFloat(int controlid,float val);

	// Generated message map functions
	//{{AFX_MSG(CMotorcycleDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOTORCYCLEDIALOG_H__AD117871_03C8_11D3_9BDD_00A0C9988171__INCLUDED_)
