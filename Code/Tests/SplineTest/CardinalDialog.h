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

#if !defined(AFX_CARDINALDIALOG_H__2AE09FE7_FBEF_11D2_9BD9_00A0C9988171__INCLUDED_)
#define AFX_CARDINALDIALOG_H__2AE09FE7_FBEF_11D2_9BD9_00A0C9988171__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CardinalDialog.h : header file
//

class CardinalSpline3DClass;

/////////////////////////////////////////////////////////////////////////////
// CCardinalDialog dialog

class CCardinalDialog : public CDialog
{
// Construction
public:
	CCardinalDialog(CWnd* pParent,CardinalSpline3DClass * curve,int key);

// Dialog Data
	//{{AFX_DATA(CCardinalDialog)
	enum { IDD = IDD_CARDINAL_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCardinalDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CardinalSpline3DClass *		Curve;
	int								Key;

	float								GetDlgItemFloat(int controlid);
	void								SetDlgItemFloat(int controlid,float val);

	// Generated message map functions
	//{{AFX_MSG(CCardinalDialog)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CARDINALDIALOG_H__2AE09FE7_FBEF_11D2_9BD9_00A0C9988171__INCLUDED_)
