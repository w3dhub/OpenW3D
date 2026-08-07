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

#if !defined(AFX_EDITSTRINGTWIDDLERDIALOG_H__05FF8A6C_8212_44F5_A971_94BE7B9A0F16__INCLUDED_)
#define AFX_EDITSTRINGTWIDDLERDIALOG_H__05FF8A6C_8212_44F5_A971_94BE7B9A0F16__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
//	Forward declarations
/////////////////////////////////////////////////////////////////////////////
class StringTwiddlerClass;


/////////////////////////////////////////////////////////////////////////////
//
// EditStringTwiddlerDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class EditStringTwiddlerDialogClass : public CDialog
{
// Construction
public:
	EditStringTwiddlerDialogClass (CWnd *pParent = nullptr);

// Dialog Data
	//{{AFX_DATA(EditStringTwiddlerDialogClass)
	enum { IDD = IDD_EDIT_STRING_TWIDDLER };
	CListCtrl	ListCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EditStringTwiddlerDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(EditStringTwiddlerDialogClass)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddButton();
	virtual void OnOK();
	afx_msg void OnKeydownListctrl(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


public:

	//////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////
	void							Set_Twiddler (StringTwiddlerClass *twiddler)	{ StringObject = twiddler; }
	StringTwiddlerClass *	Get_Twiddler (void) const							{ return StringObject; }

private:

	//////////////////////////////////////////////////////////////////////
	//	Private methods
	//////////////////////////////////////////////////////////////////////
	void							Insert_String (int string_id);

	//////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////
	StringTwiddlerClass	*	StringObject;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITSTRINGTWIDDLERDIALOG_H__05FF8A6C_8212_44F5_A971_94BE7B9A0F16__INCLUDED_)
