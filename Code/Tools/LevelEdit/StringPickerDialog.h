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

#if !defined(AFX_STRINGPICKERDIALOG_H__03729371_364E_42B5_8342_4B80F999DC8F__INCLUDED_)
#define AFX_STRINGPICKERDIALOG_H__03729371_364E_42B5_8342_4B80F999DC8F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "resource.h"
#include "vector.h"
#include "stringscategoryviewdialog.h"


/////////////////////////////////////////////////////////////////////////////
//	Forward declarations
/////////////////////////////////////////////////////////////////////////////
class TDBCategoryClass;
class TDBObjClass;


/////////////////////////////////////////////////////////////////////////////
//
// StringPickerDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class StringPickerDialogClass : public CDialog, public StringsCategoryViewCallbackClass
{
// Construction
public:
	StringPickerDialogClass(CWnd* pParent = NULL);   // standard constructor
	~StringPickerDialogClass (void);

// Dialog Data
	//{{AFX_DATA(StringPickerDialogClass)
	enum { IDD = IDD_STRING_LIBRARY_VIEW };
	CTabCtrl	m_TabCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(StringPickerDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(StringPickerDialogClass)
	afx_msg void OnSelchangeTabCtrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Creation
	//
	void	Create (CWnd *parent_wnd);

	//
	//	Selection access
	//
	void	Set_Selection (int object_id)		{ SelectedObjectID = object_id; }
	int	Get_Selection (void);

	//
	//	From StringsCategoryViewCallbackClass
	//
	void	On_Selection_Changed (int sel_object_id);

protected:

	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////
	void	Resize_Controls (void);
	void	Update_Selected_String (void);

private:

	///////////////////////////////////////////////////////////////////
	//	Private methods
	///////////////////////////////////////////////////////////////////
	void	Add_Category_Page (TDBCategoryClass *category);
	void	Update_Page_Visibility (void);

	///////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////
	bool							IsInitialized;
	int							CurrentTab;
	int							SelectedObjectID;

	DynamicVectorClass<StringsCategoryViewDialogClass *>	CategoryPages;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STRINGPICKERDIALOG_H__03729371_364E_42B5_8342_4B80F999DC8F__INCLUDED_)
