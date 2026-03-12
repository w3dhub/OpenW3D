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

#if !defined(AFX_GOTOOBJECTDIALOG_H__8558DB9C_B58D_11D2_9FF8_00104B791122__INCLUDED_)
#define AFX_GOTOOBJECTDIALOG_H__8558DB9C_B58D_11D2_9FF8_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GotoObjectDialog.h : header file
//

// Forward declarations
class NodeClass;


/////////////////////////////////////////////////////////////////////////////
//
// GotoObjectDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class GotoObjectDialogClass : public CDialog
{
// Construction
public:
	GotoObjectDialogClass (NodeClass *sel_node, CWnd *parent = NULL);

// Dialog Data
	//{{AFX_DATA(GotoObjectDialogClass)
	enum { IDD = IDD_GOTO_OBJECT };
	CComboBox	m_NodeList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(GotoObjectDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(GotoObjectDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnEditChangeObjectList();
	afx_msg void OnSelChangeObjectList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	////////////////////////////////////////////////////////////////
	//	Public Methods
	////////////////////////////////////////////////////////////////
	int				Get_Current_Selection (void);

private:

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	NodeClass *		m_pNode;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GOTOOBJECTDIALOG_H__8558DB9C_B58D_11D2_9FF8_00104B791122__INCLUDED_)
