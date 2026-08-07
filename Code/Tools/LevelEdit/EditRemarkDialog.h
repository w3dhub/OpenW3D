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

#if !defined(AFX_EDITREMARKDIALOG_H__6A88EE92_3F48_4C4D_A4EA_5C519713E6A9__INCLUDED_)
#define AFX_EDITREMARKDIALOG_H__6A88EE92_3F48_4C4D_A4EA_5C519713E6A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include "stringpickerdialog.h"


/////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////
class DialogueRemarkClass;


/////////////////////////////////////////////////////////////////////////////
//
// EditRemarkDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class EditRemarkDialogClass : public CDialog
{
// Construction
public:
	EditRemarkDialogClass(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(EditRemarkDialogClass)
	enum { IDD = IDD_EDIT_REMARK };
	CSpinButtonCtrl	m_WeightSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EditRemarkDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(EditRemarkDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	void							Set_Remark (DialogueRemarkClass *remark)	{ m_Remark = remark; }
	DialogueRemarkClass *	Get_Remark (void) const							{ return m_Remark; }

private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	DialogueRemarkClass *	m_Remark;
	StringPickerDialogClass StringPicker;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITREMARKDIALOG_H__6A88EE92_3F48_4C4D_A4EA_5C519713E6A9__INCLUDED_)
