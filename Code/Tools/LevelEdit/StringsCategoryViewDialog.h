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

#if !defined(AFX_STRINGSCATEGORYVIEWDIALOG_H__E1BB750E_63DF_4D8D_8B0C_77D90F6D2CB9__INCLUDED_)
#define AFX_STRINGSCATEGORYVIEWDIALOG_H__E1BB750E_63DF_4D8D_8B0C_77D90F6D2CB9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "resource.h"
#include "vector.h"


/////////////////////////////////////////////////////////////////////////////
//	Forward declarations
/////////////////////////////////////////////////////////////////////////////
class TDBObjClass;
class StringLibraryDialogClass;
class StringsCategoryViewCallbackClass;


/////////////////////////////////////////////////////////////////////////////
//
// StringsCategoryViewDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class StringsCategoryViewDialogClass : public CDialog
{
// Construction
public:
	StringsCategoryViewDialogClass(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(StringsCategoryViewDialogClass)
	enum { IDD = IDD_STRINGS_CATEGORY_VIEW };
	CListCtrl	m_ListCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(StringsCategoryViewDialogClass)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(StringsCategoryViewDialogClass)
	virtual BOOL OnInitDialog();
	afx_msg void OnColumnclickStringList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkStringList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteitemStringList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydownStringList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnItemchangedStringList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	/////////////////////////////////////////////////////////////////////////////
	//	Constants
	/////////////////////////////////////////////////////////////////////////////
	enum
	{
		COL_ID		= 0,
		COL_TEXT,
		COL_MAX
	};

	typedef enum
	{
		EDIT_MODE_NONE			= 0,
		EDIT_MODE_STRING,
		EDIT_MODE_TWIDDLER
	} EDIT_MODE;

	/////////////////////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////////////////////
	
	//
	//	Creation
	//
	void		Create (CWnd *parent_wnd);
	void		Apply_Changes (void);
	void		Update_Controls (void);

	//
	//	Modification
	//	
	void		Cut (DynamicVectorClass <TDBObjClass *> &entry_list, bool sel_only = true);
	void		Paste (DynamicVectorClass <TDBObjClass *> &entry_list);
	
	//
	//	Accessors
	//
	int		Get_Category_ID (void) const				{ return CategoryID; }
	void		Set_Category_ID (int id)					{ CategoryID = id; }
	bool		Is_Version_Number_Dirty (void) const	{ return ShouldUpdateVersionNumber; }
	void		Set_Edit_Mode (EDIT_MODE mode)			{ EditMode = mode; }

	//
	//	Callback support
	//
	void		Set_Callback (StringsCategoryViewCallbackClass *callback)	{ CallbackObject = callback; }

	//
	//	Column control
	//
	void		Remove_Column (int col_id);
	void		Set_Column_Width (int col_id, float width);

	//
	//	Selection access
	//
	void		Set_Selection (int object_id)				{ SelectedObjectID = object_id; }
	int		Get_Selection (void);

private:

	/////////////////////////////////////////////////////////////////////////////
	//	Private data types
	/////////////////////////////////////////////////////////////////////////////	
	typedef struct _COL_INFO
	{
		int				id;
		const char *	name;
		float				width;
	} COL_INFO;


	/////////////////////////////////////////////////////////////////////////////
	//	Private methods
	/////////////////////////////////////////////////////////////////////////////	
	void		Resize_Controls (void);
	void		Insert_New_Entry (TDBObjClass *object);
	void		Upate_Entry (int index);
	int		Find_Entry (TDBObjClass *object);

	/////////////////////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////////////////////
	int								SelectedObjectID;
	int								CategoryID;
	bool								IsInitialized;
	bool								ShouldUpdateVersionNumber;
	int								CurrentColSort;
	bool								AscendingSort;
	StringsCategoryViewCallbackClass *	CallbackObject;
	CRect								CurrListRect;
	EDIT_MODE						EditMode;
	DynamicVectorClass<int>		Columns;
	COL_INFO							ColumnSettings[COL_MAX];
};


/////////////////////////////////////////////////////////////////////////////
//
// StringsCategoryViewCallbackClass
//
/////////////////////////////////////////////////////////////////////////////
class StringsCategoryViewCallbackClass
{
public:

	StringsCategoryViewCallbackClass (void)				{}
	virtual ~StringsCategoryViewCallbackClass (void)	{}
	
	/////////////////////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////////////////////
	virtual void	On_Selection_Changed (int /* sel_object_id */)	{}
	virtual void	On_Cut (void)										{}
	virtual void	On_Paste (void)									{}
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STRINGSCATEGORYVIEWDIALOG_H__E1BB750E_63DF_4D8D_8B0C_77D90F6D2CB9__INCLUDED_)
