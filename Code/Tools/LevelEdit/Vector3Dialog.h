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

#if !defined(AFX_VECTOR3DIALOG_H__8AB9B61A_40C4_11D4_A0A6_00104B791122__INCLUDED_)
#define AFX_VECTOR3DIALOG_H__8AB9B61A_40C4_11D4_A0A6_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "resource.h"
#include "vector3.h"


/////////////////////////////////////////////////////////////////////////////
//
// Vector3DialogClass
//
/////////////////////////////////////////////////////////////////////////////
class Vector3DialogClass : public CDialog
{
public:

	Vector3DialogClass (CWnd *parent = NULL);

// Dialog Data
	//{{AFX_DATA(Vector3DialogClass)
	enum { IDD = IDD_VECTOR3 };
	CStatic	m_ZStatic;
	CStatic	m_YStatic;
	CStatic	m_XStatic;
	CEdit	m_ZEdit;
	CEdit	m_YEdit;
	CEdit	m_XEdit;
	CSpinButtonCtrl	m_ZSpin;
	CSpinButtonCtrl	m_YSpin;
	CSpinButtonCtrl	m_XSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Vector3DialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(Vector3DialogClass)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	///////////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////////

	//
	//	Creation
	//
	void			Create (CWnd *parent = NULL);

	//
	//	Data access
	//
	void			Set_Default_Value (const Vector3 &value);
	Vector3		Get_Current_Value (void);

private:

	///////////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////////
	Vector3		m_DefaultValue;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VECTOR3DIALOG_H__8AB9B61A_40C4_11D4_A0A6_00104B791122__INCLUDED_)
