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

// StringLibraryDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "stringlibrarydialog.h"
#include "translatedb.h"
#include "translateobj.h"
#include "editstringdialog.h"
#include "Utils.h"
#include "stringscategoryviewdialog.h"
#include "stringscategorynamedialog.h"


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
// StringLibraryDialogClass
//
/////////////////////////////////////////////////////////////////////////////
StringLibraryDialogClass::StringLibraryDialogClass (CWnd *pParent)	:
	IsInitialized (false),
	CurrentTab (0),
	Mode (MODE_STRING),
	CDialog(StringLibraryDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(StringLibraryDialogClass)
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~StringLibraryDialogClass
//
/////////////////////////////////////////////////////////////////////////////
StringLibraryDialogClass::~StringLibraryDialogClass (void)
{
	Clear_Clipboard ();

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
StringLibraryDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(StringLibraryDialogClass)
	DDX_Control(pDX, IDC_TAB_CTRL, m_TabCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(StringLibraryDialogClass, CDialog)
	//{{AFX_MSG_MAP(StringLibraryDialogClass)
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_CTRL, OnSelchangeTabCtrl)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_RENAME, OnRename)
	ON_COMMAND(IDM_MODE_TWIDDLER, OnModeTwiddler)
	ON_COMMAND(IDM_MODE_STRING, OnModeString)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
StringLibraryDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();
	
	//
	//	Loop over all the categories in the database
	//
	int count = TranslateDBClass::Get_Category_Count ();
	for (int index = 0; index < count; index ++) {
		
		//
		//	Lookup this category
		//
		TDBCategoryClass *category = TranslateDBClass::Get_Category (index);
		if (category != NULL) {
			Add_Category_Page (category);
		}
	}

	//
	//	Display the first category page
	//
	if (CategoryPages.Count () > 0) {
		CategoryPages[0]->ShowWindow (SW_SHOW);
	}
	
	Resize_Controls ();
	Enable_Buttons ();
	Update_Mode ();

	IsInitialized = true;
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSize
//
/////////////////////////////////////////////////////////////////////////////
void
StringLibraryDialogClass::OnSize
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
// Resize_Controls
//
/////////////////////////////////////////////////////////////////////////////
void
StringLibraryDialogClass::Resize_Controls (void)
{
	//
	//	Get the window and button bounding rectangles
	//
	CRect rect;
	CRect button_rect;
	GetClientRect (&rect);
	::GetWindowRect (::GetDlgItem (m_hWnd, IDOK), &button_rect);
	
	//
	//	Calculate some positions and widths
	//
	int width			= rect.Width ();
	int height			= rect.Height ();
	int button_width	= button_rect.Width ();
	int button_height	= button_rect.Height ();

	int tab_width		= width - (BORDER_X * 2);
	int tab_height		= height - ((BORDER_Y * 2) + (SPACING_Y) + button_height);

	int button_y_pos	= BORDER_Y + tab_height + SPACING_Y;
	int button_x_pos	= (width / 2) - (((button_width * 2) + (SPACING_X * 1)) / 2);

	//
	//	Resize the tab control
	//
	m_TabCtrl.SetWindowPos (NULL, BORDER_X, BORDER_Y, tab_width, tab_height, SWP_NOZORDER | SWP_NOCOPYBITS);

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
	//	Reposition the buttons
	//
	::SetWindowPos (::GetDlgItem (m_hWnd, IDOK), NULL, button_x_pos, button_y_pos,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOCOPYBITS | SWP_NOACTIVATE);
	button_x_pos += button_width + SPACING_X;
	
	::SetWindowPos (::GetDlgItem (m_hWnd, IDCANCEL), NULL, button_x_pos, button_y_pos,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOCOPYBITS | SWP_NOACTIVATE);

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
StringLibraryDialogClass::OnOK (void)
{	
	//
	//	Reset the database
	//
	TranslateDBClass::Remove_All ();

	//
	//	Ask each category page to save its contents into the database
	//
	int count = CategoryPages.Count ();
	bool update_version_number = false;
	for (int index = 0; index < count; index ++) {
		update_version_number |= CategoryPages[index]->Is_Version_Number_Dirty ();
		CategoryPages[index]->Apply_Changes ();
	}

	//
	//	Increment the version number
	//
	if (update_version_number) {
		TranslateDBClass::Update_Version ();
	}

	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSelchangeTabCtrl
//
/////////////////////////////////////////////////////////////////////////////
void
StringLibraryDialogClass::OnSelchangeTabCtrl
(
	NMHDR *		/* pNMHDR */,
	LRESULT *	pResult
)
{
	(*pResult) = 0;
	Update_Page_Visibility ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Page_Visibility
//
/////////////////////////////////////////////////////////////////////////////
void
StringLibraryDialogClass::Update_Page_Visibility (void)
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

	Enable_Buttons ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Enable_Buttons
//
/////////////////////////////////////////////////////////////////////////////
void
StringLibraryDialogClass::Enable_Buttons (void)
{
	CMenu *menu = GetMenu ();
	if (menu != NULL) {
		CMenu *sub_menu = menu->GetSubMenu (0);
		if (sub_menu != NULL) {

			//
			//	Update the enable state of these menu entries
			//
			int value = MF_BYCOMMAND | ((CurrentTab != 0) ? MF_ENABLED : MF_GRAYED);
			sub_menu->EnableMenuItem (IDC_REMOVE, value);
			sub_menu->EnableMenuItem (IDC_RENAME, value);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnAdd
//
/////////////////////////////////////////////////////////////////////////////
void
StringLibraryDialogClass::OnAdd (void)
{
	StringsCategoryNameDialogClass dialog (this);
	if (dialog.DoModal () == IDOK) {
		
		//
		//	Create the new category
		//
		TDBCategoryClass *category = TranslateDBClass::Add_Category (dialog.Get_Name ());
		if (category != NULL) {			
			
			//
			//	Add some UI for this new category
			//
			Add_Category_Page (category);

			//
			//	Ensure the new category page is the right size
			//
			Resize_Controls ();
		}		
	}
	
	return ;	
}


/////////////////////////////////////////////////////////////////////////////
//
// OnRemove
//
/////////////////////////////////////////////////////////////////////////////
void
StringLibraryDialogClass::OnRemove (void)
{
	//
	//	The user can't remove the first (default) category
	//
	int index = CurrentTab;
	if (index > 0) {
		TDBCategoryClass *category = TranslateDBClass::Get_Category (index);
		if (category != NULL) {
			
			//
			//	Prompt the user to ensure they really want to remove the category
			//
			CString message;
			message.Format ("Are you sure you wish to remove the %s category?", (LPCTSTR)category->Get_Name ());
			if (::MessageBox (m_hWnd, message, "Remove Category", MB_ICONQUESTION | MB_YESNO) == IDYES) {

				//
				//	Transfer the data from the old category to the default category
				//
				Clear_Clipboard ();
				CategoryPages[index]->Cut (LocalClipboard, false);
				CategoryPages[0]->Paste (LocalClipboard);
				Clear_Clipboard ();

				//
				//	Remove the category from the database
				//
				TranslateDBClass::Remove_Category (index);
				
				//
				//	Remove the category's tab
				//
				m_TabCtrl.DeleteItem (index);
				
				//
				//	Free the UI object and remove it from our list
				//
				CategoryPages[index]->DestroyWindow ();
				delete CategoryPages[index];
				CategoryPages.Delete (index);

				//
				//	Change focus back to the first page
				//
				m_TabCtrl.SetCurSel (0);
				Update_Page_Visibility ();
			}
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Add_Category_Page
//
/////////////////////////////////////////////////////////////////////////////
void
StringLibraryDialogClass::Add_Category_Page (TDBCategoryClass *category)
{
	//
	//	Add a tab to the dialog for this category
	//
	TC_ITEM tab_info = { 0 };
	tab_info.mask		= TCIF_TEXT;
	tab_info.pszText	= (char *)(LPCTSTR)category->Get_Name ();
	m_TabCtrl.InsertItem (0xFF, &tab_info);

	//
	//	Create a page for this new category
	//
	StringsCategoryViewDialogClass *child_wnd = new StringsCategoryViewDialogClass;
	child_wnd->Set_Category_ID (category->Get_ID ());	
	child_wnd->Set_Callback (this);
	child_wnd->Create (this);
	CategoryPages.Add (child_wnd);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// On_Cut
//
/////////////////////////////////////////////////////////////////////////////
void
StringLibraryDialogClass::On_Cut (void)
{
	Clear_Clipboard ();
	CategoryPages[CurrentTab]->Cut (LocalClipboard);
	return ;
}

/////////////////////////////////////////////////////////////////////////////
//
// On_Paste
//
/////////////////////////////////////////////////////////////////////////////
void
StringLibraryDialogClass::On_Paste (void)
{
	CategoryPages[CurrentTab]->Paste (LocalClipboard);
	Clear_Clipboard ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Clear_Clipboard
//
/////////////////////////////////////////////////////////////////////////////
void
StringLibraryDialogClass::Clear_Clipboard (void)
{
	for (int index = 0; index < LocalClipboard.Count (); index ++) {
		SAFE_DELETE (LocalClipboard[index]);
	}

	LocalClipboard.Delete_All ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnRename
//
/////////////////////////////////////////////////////////////////////////////
void
StringLibraryDialogClass::OnRename (void) 
{
	//
	//	The user can't rename the first (default) category
	//
	int index = CurrentTab;
	if (index > 0) {
		TDBCategoryClass *category = TranslateDBClass::Get_Category (index);
		if (category != NULL) {

			//
			//	Show a dialog to the user where they can enter a new name
			//
			StringsCategoryNameDialogClass dialog (this);
			dialog.Set_Name (category->Get_Name ());
			if (dialog.DoModal () == IDOK) {
				
				//
				//	Rename the category and tab UI
				//
				const char *new_name = dialog.Get_Name ();
				category->Set_Name (new_name);

				//
				//	Update the tab's text
				//
				TC_ITEM tab_info	= { 0 };
				tab_info.mask		= TCIF_TEXT;
				tab_info.pszText	= (char *)new_name;
				m_TabCtrl.SetItem (index, &tab_info);
				
				//
				//	HACK - Not really sure why, but the current page disappears
				// when the tab ctrl is renamed (and it has nothing to do with repainting).
				//
				CategoryPages[index]->ShowWindow (SW_HIDE);
				CategoryPages[index]->ShowWindow (SW_SHOW);
			}
		}
	}
		
	return ;	
}


/////////////////////////////////////////////////////////////////////////////
//
// OnModeTwiddler
//
/////////////////////////////////////////////////////////////////////////////
void
StringLibraryDialogClass::OnModeTwiddler (void)
{
	Mode = MODE_TWIDDLER;
	Update_Mode ();

	//
	//	Let each category know the new editing mode
	//
	for (int index = 0; index < CategoryPages.Count (); index ++) {
		CategoryPages[index]->Set_Edit_Mode (StringsCategoryViewDialogClass::EDIT_MODE_TWIDDLER);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnModeString
//
/////////////////////////////////////////////////////////////////////////////
void
StringLibraryDialogClass::OnModeString (void)
{
	Mode = MODE_STRING;
	Update_Mode ();

	//
	//	Let each category know the new editing mode
	//
	for (int index = 0; index < CategoryPages.Count (); index ++) {
		CategoryPages[index]->Set_Edit_Mode (StringsCategoryViewDialogClass::EDIT_MODE_STRING);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Mode
//
/////////////////////////////////////////////////////////////////////////////
void
StringLibraryDialogClass::Update_Mode (void)
{
	CMenu *menu = GetMenu ();
	if (menu != NULL) {
		CMenu *sub_menu = menu->GetSubMenu (1);
		if (sub_menu != NULL) {

			//
			//	Determine which menu entry should get the check
			//
			int id = 0;
			if (Mode == MODE_STRING) {
				id = IDM_MODE_STRING;
			} else {
				id = IDM_MODE_TWIDDLER;
			}

			//
			//	Check the appropriate radio button
			//
			sub_menu->CheckMenuRadioItem (0, 1, Mode, MF_BYPOSITION);
		}
	}
	
	return ;
}
