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

// StringPickerDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "stringpickerdialog.h"
#include "stringscategoryviewdialog.h"
#include "translatedb.h"
#include "translateobj.h"
#include "Utils.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//	Contants
/////////////////////////////////////////////////////////////////////////////
static const int BORDER_X		= 10;
static const int BORDER_Y		= 10;
static const int SPACING_X		= 10;
static const int SPACING_Y		= 10;


/////////////////////////////////////////////////////////////////////////////
//
// StringPickerDialogClass
//
/////////////////////////////////////////////////////////////////////////////
StringPickerDialogClass::StringPickerDialogClass (CWnd *pParent)
	:	SelectedObjectID (0),
		CurrentTab (0),
		IsInitialized (false),
		CDialog(StringPickerDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(StringPickerDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~StringPickerDialogClass
//
/////////////////////////////////////////////////////////////////////////////
StringPickerDialogClass::~StringPickerDialogClass (void)
{
	//
	//	Free the child dialog objects
	//
	for (int index = 0; index < CategoryPages.Count (); index ++) {
		delete CategoryPages[index];
	}

	CategoryPages.Delete_All ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
StringPickerDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(StringPickerDialogClass)
	DDX_Control(pDX, IDC_TAB_CTRL, m_TabCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(StringPickerDialogClass, CDialog)
	//{{AFX_MSG_MAP(StringPickerDialogClass)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_CTRL, OnSelchangeTabCtrl)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// Create
//
/////////////////////////////////////////////////////////////////////////////
void
StringPickerDialogClass::Create (CWnd *parent_wnd)
{
	CDialog::Create (StringPickerDialogClass::IDD, parent_wnd);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Resize_Controls
//
/////////////////////////////////////////////////////////////////////////////
void
StringPickerDialogClass::Resize_Controls (void)
{
	//
	//	Get the window and button bounding rectangles
	//
	CRect rect;
	CRect edit_rect;
	GetClientRect (&rect);
	::GetWindowRect (::GetDlgItem (m_hWnd, IDC_STRING_EDIT), &edit_rect);
	
	//
	//	Calculate some positions and widths
	//
	int width			= rect.Width ();
	int height			= rect.Height ();
	int edit_height	= edit_rect.Height ();

	int tab_width		= width;
	int tab_height		= height - (SPACING_Y + edit_height);

	int edit_y_pos		= tab_height + SPACING_Y;

	//
	//	Resize the tab control
	//
	m_TabCtrl.SetWindowPos (NULL, 0, 0, tab_width, tab_height, SWP_NOZORDER | SWP_NOCOPYBITS);

	//
	// Get the display rectangle of the tab control
	//
	CRect tab_rect;
	m_TabCtrl.GetWindowRect (&tab_rect);
	m_TabCtrl.AdjustRect (false, &tab_rect);
	ScreenToClient (&tab_rect);

	//
	//	Resize all the category page controls
	//
	for (int index = 0; index < CategoryPages.Count (); index ++) {
		CategoryPages[index]->SetWindowPos (NULL, tab_rect.left + BORDER_X, tab_rect.top + BORDER_Y,
			tab_rect.Width () - BORDER_X * 2, tab_rect.Height () - BORDER_Y * 2, SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOACTIVATE);
	}

	//
	//	Reposition the edit control
	//
	::SetWindowPos (::GetDlgItem (m_hWnd, IDC_STRING_EDIT), NULL, 0, edit_y_pos,
		tab_width, edit_height, SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOACTIVATE);

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSelchangeTabCtrl
//
/////////////////////////////////////////////////////////////////////////////
void
StringPickerDialogClass::OnSelchangeTabCtrl
(
	NMHDR *		/* pNMHDR */,
	LRESULT *	pResult
)
{
	(*pResult) = 0;
	Update_Page_Visibility ();

	//
	//	Update our cached selection ID
	//
	if (IsInitialized) {
		Get_Selection ();
		Update_Selected_String ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Page_Visibility
//
/////////////////////////////////////////////////////////////////////////////
void
StringPickerDialogClass::Update_Page_Visibility (void)
{
	//
	//	Check to see if the user has selected a new tab
	//
	int newtab = m_TabCtrl.GetCurSel ();
	if (CurrentTab != newtab) {
		
		//
		// Hide the old tab
		//
		if (CurrentTab < CategoryPages.Count () && CategoryPages[CurrentTab] != NULL) {
			CategoryPages[CurrentTab]->ShowWindow (SW_HIDE);
		}

		//
		// Show the new tab
		//
		if (CategoryPages[newtab] != NULL) {
			CategoryPages[newtab]->ShowWindow (SW_SHOW);
		}

		//
		// Remember what our new current tab is
		//
		CurrentTab = newtab;
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSize
//
/////////////////////////////////////////////////////////////////////////////
void
StringPickerDialogClass::OnSize
(
	UINT	nType,
	int	cx,
	int	cy
) 
{
	CDialog::OnSize (nType, cx, cy);
	
	if (IsInitialized) {
		Resize_Controls ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
StringPickerDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();
	
	//
	//	Loop over all the categories in the database
	//
	int count = TranslateDBClass::Get_Category_Count ();
	int index;
	for (index = 0; index < count; index ++) {
		
		//
		//	Lookup this category
		//
		TDBCategoryClass *category = TranslateDBClass::Get_Category (index);
		if (category != NULL) {
			Add_Category_Page (category);
		}
	}

	//
	//	Try to find the tab that the selected object is displayed on
	//
	for (index = 0; index < CategoryPages.Count (); index ++) {
		if (CategoryPages[index]->Get_Selection () == SelectedObjectID) {
			m_TabCtrl.SetCurSel (index);
			CurrentTab = index;
			break;
		}
	}

	//
	//	Display the first category page
	//
	if (CategoryPages.Count () > CurrentTab) {
		CategoryPages[CurrentTab]->ShowWindow (SW_SHOW);
	}
	
	Resize_Controls ();
	Update_Selected_String ();

	IsInitialized = true;
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// Add_Category_Page
//
/////////////////////////////////////////////////////////////////////////////
void
StringPickerDialogClass::Add_Category_Page (TDBCategoryClass *category)
{
	//
	//	Add a tab to the dialog for this category
	//
	TC_ITEM tab_info	= { 0 };
	tab_info.mask		= TCIF_TEXT;
	tab_info.pszText	= (char *)(LPCTSTR)category->Get_Name ();
	m_TabCtrl.InsertItem (0xFF, &tab_info);

	//
	//	Create a page for this new category
	//
	StringsCategoryViewDialogClass *child_wnd = new StringsCategoryViewDialogClass;
	child_wnd->Set_Category_ID (category->Get_ID ());	
	child_wnd->Set_Edit_Mode (StringsCategoryViewDialogClass::EDIT_MODE_NONE);
	child_wnd->Set_Selection (SelectedObjectID);
	child_wnd->Set_Callback (this);
	child_wnd->Remove_Column (StringsCategoryViewDialogClass::COL_TEXT);
	child_wnd->Set_Column_Width (StringsCategoryViewDialogClass::COL_ID, 0.95F);
	child_wnd->Create (this);
	CategoryPages.Add (child_wnd);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Selection
//
/////////////////////////////////////////////////////////////////////////////
int
StringPickerDialogClass::Get_Selection (void)
{
	//
	//	Check to see if we need to update the cached selection ID
	//
	if (::IsWindow (m_hWnd)) {		
		SelectedObjectID = CategoryPages[CurrentTab]->Get_Selection ();
	}

	return SelectedObjectID;
}


/////////////////////////////////////////////////////////////////////////////
//
// On_Selection_Changed
//
/////////////////////////////////////////////////////////////////////////////
void
StringPickerDialogClass::On_Selection_Changed (int sel_object_id)
{
	SelectedObjectID = sel_object_id;
	Update_Selected_String ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Selected_String
//
/////////////////////////////////////////////////////////////////////////////
void
StringPickerDialogClass::Update_Selected_String (void)
{
	StringClass english_text;
	if (SelectedObjectID != 0) {

		//
		//	Lookup the text associated with the new string
		//
		TDBObjClass *object = TranslateDBClass::Find_Object (SelectedObjectID);
		if (object != NULL) {
			english_text = object->Get_English_String ();
		}		
	}

	//
	//	Set the text of the selected string
	//
	SetDlgItemText (IDC_STRING_EDIT, english_text);
	return ;
}
