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

#if !defined(AFX_EDITSTRINGDIALOG_H__E438BDAC_5DB9_11D4_A0A8_00104B791122__INCLUDED_)
#define AFX_EDITSTRINGDIALOG_H__E438BDAC_5DB9_11D4_A0A8_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include "presetpicker.h"


/////////////////////////////////////////////////////////////////////////////
// Forward delcarations
/////////////////////////////////////////////////////////////////////////////
class TDBObjClass;

/////////////////////////////////////////////////////////////////////////////
// Utiltity functions
/////////////////////////////////////////////////////////////////////////////
bool		Validate_String_ID (HWND wnd, const char *string_id);


/////////////////////////////////////////////////////////////////////////////
//
// EditStringDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class EditStringDialogClass : public CDialog
{
// Construction
public:
	EditStringDialogClass(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(EditStringDialogClass)
	enum { IDD = IDD_EDIT_STRING };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EditStringDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(EditStringDialogClass)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	/////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////
	void				Set_Translate_Object (TDBObjClass *object)	{ m_Object = object; }
	TDBObjClass *	Get_Translate_Object (void) const				{ return m_Object; }

private:

	/////////////////////////////////////////////////////////////
	//	Private methods
	/////////////////////////////////////////////////////////////
	bool				Validate_ID (void);
	void				Convert_Newline_To_Chars (CString &string);
	void				Convert_Chars_To_Newline (CString &string);


	/////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////
	TDBObjClass *			m_Object;
	PresetPickerClass		m_PresetPicker;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITSTRINGDIALOG_H__E438BDAC_5DB9_11D4_A0A8_00104B791122__INCLUDED_)
