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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/IncludeFilesDialog.cpp                                                                                                                                                                                                                                                                                                                                          $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "LevelEdit.h"
#include "includefilesdialog.h"
#include "filemgr.h"
#include "icons.h"
#include "Utils.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// IncludeFilesDialogClass
//
IncludeFilesDialogClass::IncludeFilesDialogClass(CWnd* pParent /*=NULL*/)
	: m_hGlobalFolder (NULL),
	  m_hLevelFolder (NULL),
	  m_CurrentItem (NULL),
	  CDialog(IncludeFilesDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(IncludeFilesDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
void
IncludeFilesDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(IncludeFilesDialogClass)
	DDX_Control(pDX, IDC_INCLUDE_TREE, m_IncludesTreeCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(IncludeFilesDialogClass, CDialog)
	//{{AFX_MSG_MAP(IncludeFilesDialogClass)
	ON_NOTIFY(TVN_SELCHANGED, IDC_INCLUDE_TREE, OnSelchangedIncludeTree)
	ON_EN_CHANGE(IDC_SPEC_EDIT, OnChangeSpecEdit)
	ON_BN_CLICKED(IDC_ADD_REMOVE_BUTTON, OnAddRemoveButton)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
BOOL
IncludeFilesDialogClass::OnInitDialog (void)
{
	// Allow the base class to process this message
	CDialog::OnInitDialog ();	

	// Insert the 2 main folders into the tree control
	m_IncludesTreeCtrl.SetImageList (::Get_Global_Image_List (), TVSIL_NORMAL);
	m_hGlobalFolder = m_IncludesTreeCtrl.InsertItem ("Global", FOLDER_ICON, FOLDER_ICON);
	m_hLevelFolder = m_IncludesTreeCtrl.InsertItem ("Level Specific", FOLDER_ICON, FOLDER_ICON);

	// Ensure the file manager has the latest and greatest
	::Get_File_Mgr ()->Build_Global_Include_List ();
	
	// Loop through all the global include files and add them to the tree control
	DynamicVectorClass<CString> &global_list = ::Get_File_Mgr ()->Get_Global_Include_File_List ();
	int index;
	for (index = 0; index < global_list.Count (); index ++) {
		int icon_index = (::strpbrk (global_list[index], "*?") != NULL) ? FILES_ICON : FILE_ICON;
		m_IncludesTreeCtrl.InsertItem (global_list[index], icon_index, icon_index, m_hGlobalFolder);
	}
	
	// Loop through all the level-specific include files and add them to the tree control
	DynamicVectorClass<CString> &level_list = ::Get_File_Mgr ()->Get_Include_File_List ();
	for (index = 0; index < level_list.Count (); index ++) {
		int icon_index = (::strpbrk (level_list[index], "*?") != NULL) ? FILES_ICON : FILE_ICON;
		m_IncludesTreeCtrl.InsertItem (level_list[index], icon_index, icon_index, m_hLevelFolder);
	}
	
	// Create the file picker control
	CRect rect;
	HWND hold_edit = ::GetDlgItem (m_hWnd, IDC_SPEC_EDIT);
	::GetWindowRect (hold_edit, &rect);
	ScreenToClient (&rect);
	m_FilePicker.Create_Picker (WS_CHILD | WS_TABSTOP | WS_VISIBLE,
										 rect,
										 this,
										 IDC_SPEC_EDIT);
	
	::SetWindowPos (m_FilePicker, ::GetDlgItem (m_hWnd, IDC_SPEC_STATIC), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);
	::DestroyWindow (hold_edit);

	// Ensure the file specs are sorted
	m_IncludesTreeCtrl.SortChildren (m_hGlobalFolder);
	m_IncludesTreeCtrl.SortChildren (m_hLevelFolder);
	m_IncludesTreeCtrl.SelectItem (m_hGlobalFolder);
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
void
IncludeFilesDialogClass::OnOK (void)
{
	DynamicVectorClass<CString> &global_list = ::Get_File_Mgr ()->Get_Global_Include_File_List ();
	DynamicVectorClass<CString> &level_list = ::Get_File_Mgr ()->Get_Include_File_List ();
	global_list.Delete_All ();
	level_list.Delete_All ();

	// Loop through all the children of the global folder
	HTREEITEM hchild;
	for (hchild = m_IncludesTreeCtrl.GetChildItem (m_hGlobalFolder);
		  hchild != NULL;
		  hchild = m_IncludesTreeCtrl.GetNextSiblingItem (hchild)) {
		
		// Add this filespec to the global list
		CString child_text = m_IncludesTreeCtrl.GetItemText (hchild);
		global_list.Add (child_text);
	}

	// Loop through all the children of the level-specific folder
	for (hchild = m_IncludesTreeCtrl.GetChildItem (m_hLevelFolder);
		  hchild != NULL;
		  hchild = m_IncludesTreeCtrl.GetNextSiblingItem (hchild)) {
		
		// Add this filespec to the local list
		CString child_text = m_IncludesTreeCtrl.GetItemText (hchild);
		level_list.Add (child_text);
	}

	// Check the latest copy of the global include file list into the VSS
	::Get_File_Mgr ()->Update_Global_Include_File_List ();

	// Allow the base class to process this message
	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSelchangedIncludeTree
//
void
IncludeFilesDialogClass::OnSelchangedIncludeTree
(
	NMHDR * /* pNMHDR */,
	LRESULT *pResult
)
{
	(*pResult) = 0;
	
	// Change the text in the edit control to reflect the new selection
	HTREEITEM hitem = m_IncludesTreeCtrl.GetSelectedItem ();	
	if (m_IncludesTreeCtrl.GetParentItem (hitem) != NULL) {		
		
		// Put the file spec into the edit control
		CString path = m_IncludesTreeCtrl.GetItemText (hitem);
		SetDlgItemText (IDC_SPEC_EDIT, path);
		SetDlgItemText (IDC_ADD_REMOVE_BUTTON, "&Remove");
		m_CurrentItem = hitem;

		// Let the file picker know the full path to the file
		path = ::Get_File_Mgr ()->Make_Full_Path (path);
		m_FilePicker.Set_Default_Filename (path);

	} else {
		//SetDlgItemText (IDC_SPEC_EDIT, "");
		SetDlgItemText (IDC_ADD_REMOVE_BUTTON, "&Add");
		m_CurrentItem = NULL;
	}

	// Ensure the 'add/remove' button is correctly enabled/disabled
	//::EnableWindow (::GetDlgItem (m_hWnd, IDC_ADD_REMOVE_BUTTON), enable_button);

	// Refresh the state of the 'add/remove' button
	//Update_Add_Remove_Button ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Add_Remove_Button
//
void
IncludeFilesDialogClass::Update_Add_Remove_Button (void)
{
	CString filespec;
	GetDlgItemText (IDC_SPEC_EDIT, filespec);
	filespec = ::Get_File_Mgr ()->Make_Relative_Path (filespec);

	// Determine which item to use as a parent
	HTREEITEM hselected_item = m_IncludesTreeCtrl.GetSelectedItem ();	
	HTREEITEM hroot = hselected_item;
	if (m_IncludesTreeCtrl.GetParentItem (hroot) != NULL) {
		hroot = m_IncludesTreeCtrl.GetParentItem (hroot);
	}

	// Change the button text if the entry was found or not
	m_CurrentItem = Find_Spec (filespec, hroot);
	if (m_CurrentItem != NULL) {
		SetDlgItemText (IDC_ADD_REMOVE_BUTTON, "&Remove");

		// Ensure this item is selected in the tree control as well		
		if (hselected_item != m_CurrentItem) {
			m_IncludesTreeCtrl.SelectItem (m_CurrentItem);
			m_IncludesTreeCtrl.EnsureVisible (m_CurrentItem);
		}

	} else {
		SetDlgItemText (IDC_ADD_REMOVE_BUTTON, "&Add");
	}

	return ;
}


////////////////////////////////////////////////////////////////////
//
// Find_Spec
//
HTREEITEM
IncludeFilesDialogClass::Find_Spec
(
	LPCTSTR filespec,
	HTREEITEM hroot
)
{	
	HTREEITEM hitem = NULL;

	// Loop through all the children of the global folder
	for (HTREEITEM hchild = m_IncludesTreeCtrl.GetChildItem (hroot);
		  (hchild != NULL) && (hitem == NULL);
		  hchild = m_IncludesTreeCtrl.GetNextSiblingItem (hchild)) {
		
		// Is this the entry we are looking for?
		CString child_text = m_IncludesTreeCtrl.GetItemText (hchild);
		if (child_text.CompareNoCase (filespec) == 0) {
			hitem = hchild;
		}
	}
	
	// Return the tree item if we found the item
	return hitem;
}


////////////////////////////////////////////////////////////////////
//
// OnChangeSpecEdit
//
void
IncludeFilesDialogClass::OnChangeSpecEdit (void)
{
	Update_Add_Remove_Button ();
	return ;
}


////////////////////////////////////////////////////////////////////
//
// OnAddRemoveButton
//
void
IncludeFilesDialogClass::OnAddRemoveButton (void) 
{
	// Are we adding or removing an entry from the tree control?
	if (m_CurrentItem != NULL) {
		HTREEITEM hnew_sel = m_IncludesTreeCtrl.GetPrevSiblingItem (m_CurrentItem);
		hnew_sel = (hnew_sel != NULL) ? hnew_sel : m_hGlobalFolder;
		m_IncludesTreeCtrl.DeleteItem (m_CurrentItem);
		m_IncludesTreeCtrl.SelectItem (hnew_sel);
		m_IncludesTreeCtrl.EnsureVisible (hnew_sel);
	} else {
		
		// Determine which item to use as a parent
		HTREEITEM hitem = m_IncludesTreeCtrl.GetSelectedItem ();
		if (m_IncludesTreeCtrl.GetParentItem (hitem) != NULL) {
			hitem = m_IncludesTreeCtrl.GetParentItem (hitem);
		}

		// Get the text the user entered
		CString filespec;
		GetDlgItemText (IDC_SPEC_EDIT, filespec);

		// Is the new path valid?
		bool rel_path = ::Is_Path_Relative (filespec);
		if ((rel_path == true) || ((rel_path == false) && ::Get_File_Mgr ()->Is_Path_Valid (filespec))) {

			// Add this entry to the tree control
			CString new_entry = ::Get_File_Mgr ()->Make_Relative_Path (filespec);
			int icon_index = (::strpbrk (new_entry, "*?") != NULL) ? FILES_ICON : FILE_ICON;
			HTREEITEM hnew_item = m_IncludesTreeCtrl.InsertItem (new_entry, icon_index, icon_index, hitem);

			// Now select this new item...
			m_IncludesTreeCtrl.SelectItem (hnew_item);
			m_IncludesTreeCtrl.EnsureVisible (hnew_item);

		} else {
			
			// Let the user know this path is invalid
			CString message;
			CString title;
			message.Format (IDS_INVALID_MODEL_PATH_MSG, (LPCTSTR)::Get_File_Mgr()->Get_Base_Path ());
			title.LoadString (IDS_INVALID_MODEL_PATH_TITLE);
			::MessageBox (m_hWnd, message, title, MB_ICONERROR | MB_OK);
		}				
	}

	// Ensure the file specs are sorted
	m_IncludesTreeCtrl.SortChildren (m_hGlobalFolder);
	m_IncludesTreeCtrl.SortChildren (m_hLevelFolder);
	return ;
}


////////////////////////////////////////////////////////////////////
//
// OnDestroy
//
void
IncludeFilesDialogClass::OnDestroy (void)
{
	m_IncludesTreeCtrl.SetImageList (NULL, TVSIL_NORMAL);
	CDialog::OnDestroy();		
	return ;
}
