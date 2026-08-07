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

#if !defined(AFX_PHYSICSCONSTANTSDIALOG_H__33DD6AC5_E78D_11D2_9BD2_00A0C9988171__INCLUDED_)
#define AFX_PHYSICSCONSTANTSDIALOG_H__33DD6AC5_E78D_11D2_9BD2_00A0C9988171__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PhysicsConstantsDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPhysicsConstantsDialog dialog

class CPhysicsConstantsDialog : public CDialog
{
// Construction
public:
	CPhysicsConstantsDialog(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPhysicsConstantsDialog)
	enum { IDD = IDD_PHYSICS_CONSTANTS_DIALOG };
	CSpinButtonCtrl	m_LDampingSpin;
	CSpinButtonCtrl	m_GravityAccelSpin;
	CSpinButtonCtrl	m_ADampingSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPhysicsConstantsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:

	float GetDlgItemFloat(int controlid);
	void SetDlgItemFloat(int controlid,float val);

	// Generated message map functions
	//{{AFX_MSG(CPhysicsConstantsDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHYSICSCONSTANTSDIALOG_H__33DD6AC5_E78D_11D2_9BD2_00A0C9988171__INCLUDED_)
