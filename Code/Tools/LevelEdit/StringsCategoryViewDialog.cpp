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

// StringsCategoryViewDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "stringscategoryviewdialog.h"
#include "translateobj.h"
#include "translatedb.h"
#include "tdbcategory.h"
#include "editstringdialog.h"
#include "editstringtwiddlerdialog.h"
#include "Utils.h"
#include "stringlibrarydialog.h"
#include "stringtwiddler.h"
#include <algorithm>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//	Local prototypes
/////////////////////////////////////////////////////////////////////////////
static int CALLBACK StringEntrySortCompareFn (LPARAM param1, LPARAM param2, LPARAM sort_info);


/////////////////////////////////////////////////////////////////////////////
//
// StringsCategoryViewDialogClass
//
/////////////////////////////////////////////////////////////////////////////
StringsCategoryViewDialogClass::StringsCategoryViewDialogClass (CWnd *pParent)
	:	SelectedObjectID (0),
		CategoryID (0),
		IsInitialized (false),
		ShouldUpdateVersionNumber (false),
		CurrentColSort (COL_ID),
		AscendingSort (true),
		CallbackObject (NULL),
		EditMode (EDIT_MODE_STRING),
		CDialog(StringsCategoryViewDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(StringsCategoryViewDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	Columns.Add (COL_ID);
	Columns.Add (COL_TEXT);

	ColumnSettings[COL_ID].id		= COL_ID;
	ColumnSettings[COL_ID].name	= "ID";
	ColumnSettings[COL_ID].width	= 0.33F;

	ColumnSettings[COL_TEXT].id		= COL_TEXT;
	ColumnSettings[COL_TEXT].name		= "English Test";
	ColumnSettings[COL_TEXT].width	= 0.66F;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
StringsCategoryViewDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(StringsCategoryViewDialogClass)
	DDX_Control(pDX, IDC_STRING_LIST, m_ListCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(StringsCategoryViewDialogClass, CDialog)
	//{{AFX_MSG_MAP(StringsCategoryViewDialogClass)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_STRING_LIST, OnColumnclickStringList)
	ON_NOTIFY(NM_DBLCLK, IDC_STRING_LIST, OnDblclkStringList)
	ON_NOTIFY(LVN_DELETEITEM, IDC_STRING_LIST, OnDeleteitemStringList)
	ON_NOTIFY(LVN_KEYDOWN, IDC_STRING_LIST, OnKeydownStringList)
	ON_WM_SIZE()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_STRING_LIST, OnItemchangedStringList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// Remove_Column
//
/////////////////////////////////////////////////////////////////////////////
void
StringsCategoryViewDialogClass::Remove_Column (int col_id)
{
	for (int index = 0; index < Columns.Count (); index ++) {
		
		//
		//	If this is the column we were looking for, remove
		// it from list
		//
		if (Columns[index] == col_id) {
			// Cast needed to disambiguate Delete(int) and Delete(const T &) where T = int
			(Columns.*(static_cast<bool (DynamicVectorClass<int>::*) (int)>(&DynamicVectorClass<int>::Delete))) (index);
			break;
		}
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Set_Column_Width
//
/////////////////////////////////////////////////////////////////////////////
void
StringsCategoryViewDialogClass::Set_Column_Width (int col_id, float width)
{
	ColumnSettings[col_id].width = width;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Create
//
/////////////////////////////////////////////////////////////////////////////
void
StringsCategoryViewDialogClass::Create (CWnd *parent_wnd)
{
	CDialog::Create (StringsCategoryViewDialogClass::IDD, parent_wnd);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
StringsCategoryViewDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	//
	//	Set the extended styles for the list control
	//
	m_ListCtrl.SetExtendedStyle (m_ListCtrl.GetExtendedStyle () | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	//
	//	Configure the columns
	//
	for (int index = 0; index < Columns.Count (); index ++) {
		m_ListCtrl.InsertColumn (index,	ColumnSettings[index].name);
	}

	Update_Controls ();
	m_ListCtrl.GetClientRect (CurrListRect);

	//
	//	Add an entry to the end that the user can double-click on to insert
	// new strings.
	//
	if (EditMode != EDIT_MODE_NONE) {
		m_ListCtrl.InsertItem (0, "");
	}

	//
	//	Get the list of strings from the database and insert them into the list control
	//
	for (	TDBObjClass *object = TranslateDBClass::Get_First_Object (CategoryID);
			object != NULL;
			object = TranslateDBClass::Get_Next_Object (CategoryID, object))
	{		
		//
		//	Insert a copy of this object into the list control
		//
		Insert_New_Entry (object->Clone ());
	}

	//
	//	Sort the list
	//
	m_ListCtrl.SortItems (StringEntrySortCompareFn, MAKELONG (CurrentColSort, AscendingSort));
	Resize_Controls ();

	//
	//	Handle the default selection
	//
	if (m_ListCtrl.GetItemCount () > 0) {
		int item_index = -1;

		//
		//	Try to find the object that is supposed to be selected
		//
		TDBObjClass *sel_object = TranslateDBClass::Find_Object (SelectedObjectID);
		if (sel_object != NULL) {
			item_index = Find_Entry (sel_object);
		}

		//
		//	Select the object if possible, otherwise select the first entry
		//
		if (item_index >= 0) {
			m_ListCtrl.SetItemState (item_index, LVIS_SELECTED, LVIS_SELECTED);
			m_ListCtrl.EnsureVisible (item_index, false);
		} else {
			m_ListCtrl.SetItemState (0, LVIS_SELECTED, LVIS_SELECTED);
		}
	}

	IsInitialized = true;
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Controls
//
/////////////////////////////////////////////////////////////////////////////
void
StringsCategoryViewDialogClass::Update_Controls (void)
{
	//
	//	Size the columns
	//
	CRect rect;
	m_ListCtrl.GetClientRect (&rect);
	int width = rect.Width () - ::GetSystemMetrics (SM_CXVSCROLL);

	//
	//	Configure the columns
	//
	for (int index = 0; index < Columns.Count (); index ++) {
		m_ListCtrl.SetColumnWidth (index, ColumnSettings[index].width * width);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Find_Entry
//
/////////////////////////////////////////////////////////////////////////////
int
StringsCategoryViewDialogClass::Find_Entry (TDBObjClass *object)
{
	int object_index = -1;

	//
	//	Loop over all the objects in the list control
	//
	for (int index = 0; index < m_ListCtrl.GetItemCount (); index ++) {
		TDBObjClass *curr_object = (TDBObjClass *)m_ListCtrl.GetItemData (index);
		
		//
		//	Is this the object we're looking for?
		//
		if (curr_object != NULL && curr_object->Get_ID () == object->Get_ID ()) {
			object_index = index;
			break;
		}
	}

	return object_index;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDblclkStringList
//
/////////////////////////////////////////////////////////////////////////////
void
StringsCategoryViewDialogClass::OnDblclkStringList
(
	NMHDR *		/* pNMHDR */,
	LRESULT *	pResult
) 
{
	(*pResult) = 0;

	//
	//	Don't do anything if we aren't in edit mode
	//
	if (EditMode == EDIT_MODE_NONE) {
		return ;
	}

	TDBObjClass *object = NULL;

	//
	//	Peek into the selected item to get the translation object associated with it
	//
	int selected_item = m_ListCtrl.GetNextItem (-1, LVNI_SELECTED | LVNI_ALL);	
	if (selected_item >= 0) {
		object = (TDBObjClass *)m_ListCtrl.GetItemData (selected_item);
	}

	//
	//	See if this is a string twiddler
	//
	StringTwiddlerClass *twiddler = NULL;
	if (object != NULL) {
		twiddler = object->As_StringTwiddlerClass ();
	}

	//
	//	Determine which dialog to show
	//	
	bool show_edit_string = true;
	if (EditMode == EDIT_MODE_TWIDDLER && object == NULL) {
		show_edit_string = false;
	} else if (twiddler != NULL) {
		show_edit_string = false;
	}
	
	//
	//	Show a dialog to the user
	//
	if (show_edit_string) {

		//
		//	Let the user edit this entry
		//
		EditStringDialogClass dialog (this);
		dialog.Set_Translate_Object (object);
		if (dialog.DoModal () == IDOK) {
			
			//
			//	Either add a new entry to the list control, or update the
			// text of an existing entry
			//
			if (object == NULL) {
				
				TDBObjClass *new_object = dialog.Get_Translate_Object ();
				new_object->Set_Category_ID (CategoryID);
				TranslateDBClass::Add_Object (new_object);
				Insert_New_Entry (new_object->Clone ());

			} else {
				Upate_Entry (selected_item);
			}
		}

	} else {

		EditStringTwiddlerDialogClass dialog (this);
		dialog.Set_Twiddler (twiddler);
		if (dialog.DoModal () == IDOK) {

			//
			//	Either add a new entry to the list control, or update the
			// text of an existing entry
			//
			if (object == NULL) {
				
				StringTwiddlerClass *new_object = dialog.Get_Twiddler ();
				new_object->Set_Category_ID (CategoryID);
				TranslateDBClass::Add_Object (new_object);
				Insert_New_Entry (new_object->Clone ());

			} else {
				Upate_Entry (selected_item);
			}
		}
	}
	
	//
	//	Force a repaint
	//
	InvalidateRect (NULL, true);
	UpdateWindow ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnColumnclickStringList
//
/////////////////////////////////////////////////////////////////////////////
void
StringsCategoryViewDialogClass::OnColumnclickStringList
(
	NMHDR *		pNMHDR,
	LRESULT *	pResult
)
 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	(*pResult) = 0;

	if (CurrentColSort == pNMListView->iSubItem) {
		AscendingSort = !AscendingSort;
	} else {
		CurrentColSort = pNMListView->iSubItem;
		AscendingSort = true;
	}

	m_ListCtrl.SortItems (StringEntrySortCompareFn, MAKELONG (CurrentColSort, AscendingSort));
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// StringEntrySortCompareFn
//
/////////////////////////////////////////////////////////////////////////////
int CALLBACK
StringEntrySortCompareFn (LPARAM param1, LPARAM param2, LPARAM sort_info)
{
	int retval = 0;

	TDBObjClass *object1	= (TDBObjClass *)param1;
	TDBObjClass *object2	= (TDBObjClass *)param2;

	LONG column_id = LOWORD (sort_info);
	bool ascending	= HIWORD (sort_info) != 0;

	if (object1 != NULL && object2 != NULL) {
		
		//
		//	Determine the order based on which column the user clicked on
		//
		if (column_id == StringsCategoryViewDialogClass::COL_ID) {
			retval = ::lstrcmpi (object1->Get_ID_Desc (), object2->Get_ID_Desc ());
		} else if (column_id == StringsCategoryViewDialogClass::COL_TEXT) {
			retval = ::lstrcmpi (object1->Get_English_String (), object2->Get_English_String ());
		}
	}

	//
	//	Invert the sort if necessary
	//
	if (!ascending) {
		retval = -retval;
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
// Upate_Entry
//
/////////////////////////////////////////////////////////////////////////////
void
StringsCategoryViewDialogClass::Upate_Entry (int index)
{
	if (index < 0) {
		return ;
	}

	//
	//	Update this list control entry
	//
	TDBObjClass *object = (TDBObjClass *)m_ListCtrl.GetItemData (index);
	if (object != NULL) {

		//
		//	Update each column
		//
		for (int col_index = 0; col_index < Columns.Count (); col_index ++) {
			
			//
			//	Determine what information to update
			//
			if (ColumnSettings[col_index].id == COL_ID) {
				m_ListCtrl.SetItemText (index, col_index, object->Get_ID_Desc ());
			} else if (ColumnSettings[col_index].id == COL_TEXT) {
				m_ListCtrl.SetItemText (index, col_index, object->Get_English_String ());
			}
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Insert_New_Entry
//
/////////////////////////////////////////////////////////////////////////////
void
StringsCategoryViewDialogClass::Insert_New_Entry (TDBObjClass *object)
{
	if (object == NULL) {
		return ;
	}

	//
	//	Ensure the new object has the right category
	//
	object->Set_Category_ID (CategoryID);

	//
	//	Insert an item into the list control to represent this string
	//
	int last_index = std::max (m_ListCtrl.GetItemCount () - 1, 0);
	int item_index = m_ListCtrl.InsertItem (last_index, object->Get_ID_Desc ());
	if (item_index >= 0) {

		//
		//	Update each column
		//
		for (int col_index = 0; col_index < Columns.Count (); col_index ++) {
			
			//
			//	Determine what information to update
			//
			if (ColumnSettings[col_index].id == COL_ID) {
				m_ListCtrl.SetItemText (item_index, col_index, object->Get_ID_Desc ());
			} else if (ColumnSettings[col_index].id == COL_TEXT) {
				m_ListCtrl.SetItemText (item_index, col_index, object->Get_English_String ());
			}
		}

		m_ListCtrl.SetItemData (item_index, (DWORD_PTR)object);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeleteitemStringList
//
/////////////////////////////////////////////////////////////////////////////
void
StringsCategoryViewDialogClass::OnDeleteitemStringList
(
	NMHDR *		pNMHDR,
	LRESULT *	pResult
) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	(*pResult) = 0;

	//
	//	Free the associated translation object
	//
	TDBObjClass *object = (TDBObjClass *)m_ListCtrl.GetItemData (pNMListView->iItem);
	SAFE_DELETE (object);

	m_ListCtrl.SetItemData (pNMListView->iItem, 0L);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnKeydownStringList
//
/////////////////////////////////////////////////////////////////////////////
void
StringsCategoryViewDialogClass::OnKeydownStringList
(
	NMHDR *		pNMHDR,
	LRESULT *	pResult
)
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;
	(*pResult) = 0;

	//
	//	Don't do anything if we aren't in edit mode
	//
	if (EditMode == EDIT_MODE_NONE) {
		return ;
	}


	if (pLVKeyDow->wVKey == VK_DELETE) {
		
		//
		//	Delete all the selected items (except for the last item)
		//
		int index = -1;
		while ((index = m_ListCtrl.GetNextItem (-1, LVNI_SELECTED | LVNI_ALL)) >= 0) {
			if (index < m_ListCtrl.GetItemCount () - 1) {
				m_ListCtrl.DeleteItem (index);
				ShouldUpdateVersionNumber = true;
			} else {
				break;
			}
		}

	} else if (::GetKeyState (VK_CONTROL) < 0) {

		if (CallbackObject != NULL) {		

			//
			//	Check to see if the user pressed the copy or paste keys
			//
			switch (pLVKeyDow->wVKey)
			{
				case 'x':
				case 'X':
					CallbackObject->On_Cut ();
					break;

				case 'v':
				case 'V':
					CallbackObject->On_Paste ();
					break;
			}
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Resize_Controls
//
/////////////////////////////////////////////////////////////////////////////
void
StringsCategoryViewDialogClass::Resize_Controls (void)
{
	if (IsInitialized == false) {
		return ;
	}

	//
	//	Get the current client rectangle of the list control
	//
	CRect new_list_rect;
	GetClientRect (&new_list_rect);

	int old_width = CurrListRect.Width ();

	//
	//	Resize each column so its the same percentage size as it was before
	//
	for (int col_index = 0; col_index < Columns.Count (); col_index ++) {
		float col_per = (float)m_ListCtrl.GetColumnWidth (col_index) / (float)old_width;
		int new_width = int(new_list_rect.Width () * col_per);
		m_ListCtrl.SetColumnWidth (col_index, new_width);
	}

	CurrListRect = new_list_rect;

	//
	//	Get the window bounding rectangle
	//
	CRect rect;
	GetClientRect (&rect);
	
	//
	//	Resize the list control
	//
	m_ListCtrl.SetWindowPos (NULL, rect.left, rect.top, rect.Width (), rect.Height (),
		SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOACTIVATE);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Apply_Changes
//
/////////////////////////////////////////////////////////////////////////////
void
StringsCategoryViewDialogClass::Apply_Changes (void)
{
	//
	//	Don't do anything if we aren't in edit mode
	//
	if (EditMode == EDIT_MODE_NONE) {
		return ;
	}
	
	//
	//	Add all the objects from the list control into the database
	//
	int count = m_ListCtrl.GetItemCount ();
	for (int index = 0; index < count; index ++) {
		TDBObjClass *object = (TDBObjClass *)m_ListCtrl.GetItemData (index);
		if (object != NULL) {
			TranslateDBClass::Add_Object (object->Clone ());
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSize
//
/////////////////////////////////////////////////////////////////////////////
void
StringsCategoryViewDialogClass::OnSize (UINT nType, int cx, int cy) 
{
	CDialog::OnSize (nType, cx, cy);
	Resize_Controls ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Cut
//
/////////////////////////////////////////////////////////////////////////////
void
StringsCategoryViewDialogClass::Cut
(
	DynamicVectorClass <TDBObjClass *> &	entry_list,
	bool												sel_only
)
{
	m_ListCtrl.SetRedraw (false);

	//
	//	Enable the selected flag if we only want to cut the current selection set.
	//
	int flags = LVNI_ALL;
	if (sel_only) {
		flags |= LVNI_SELECTED;
	}

	//
	//	Remove all selected entries from the list control and add them to the list
	//
	int index = -1;
	while ((index = m_ListCtrl.GetNextItem (-1, flags)) != -1) {
		TDBObjClass *object = (TDBObjClass *)m_ListCtrl.GetItemData (index);
		if (object != NULL) {
			entry_list.Add (object->Clone ());
			m_ListCtrl.DeleteItem (index);
		} else if (m_ListCtrl.GetItemCount () == 1) {
			break;
		}
	}

	m_ListCtrl.SetRedraw (true);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Paste
//
/////////////////////////////////////////////////////////////////////////////
void
StringsCategoryViewDialogClass::Paste (DynamicVectorClass <TDBObjClass *> &entry_list)
{
	m_ListCtrl.SetRedraw (false);

	//
	//	Add a new entry for each object
	//
	for (int index = 0; index < entry_list.Count (); index ++) {
		TDBObjClass *object = entry_list[index];
		TDBObjClass *our_copy = object->Clone ();
		our_copy->Set_Category_ID (CategoryID);
		Insert_New_Entry (our_copy);
	}

	//
	//	Resort the list
	//
	m_ListCtrl.SortItems (StringEntrySortCompareFn, MAKELONG (CurrentColSort, AscendingSort));
	m_ListCtrl.SetRedraw (true);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Selection
//
/////////////////////////////////////////////////////////////////////////////
int
StringsCategoryViewDialogClass::Get_Selection (void)
{
	//
	//	Check to see if we need to update the cached selection ID
	//
	if (::IsWindow (m_hWnd)) {
		
		//
		//	Find the selected item
		//
		int index = m_ListCtrl.GetNextItem (-1, LVNI_SELECTED | LVNI_ALL);
		if (index != -1) {

			//
			//	Store the selected item's object id
			//
			TDBObjClass *object = (TDBObjClass *)m_ListCtrl.GetItemData (index);
			if (object != NULL) {
				SelectedObjectID = object->Get_ID ();
			}
			
		} else {
			SelectedObjectID = 0;
		}
	}

	return SelectedObjectID;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnItemchangedStringList
//
/////////////////////////////////////////////////////////////////////////////
void
StringsCategoryViewDialogClass::OnItemchangedStringList
(
	NMHDR *		pNMHDR,
	LRESULT *	pResult
)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	(*pResult) = 0;

	if (IsInitialized && (pNMListView->uChanged & LVIF_STATE)) {
		
		//
		//	This will force the cached selection data to be updated
		//
		Get_Selection ();
		
		//
		//	Notify the owner (if necessary)
		//
		if (CallbackObject != NULL) {
			CallbackObject->On_Selection_Changed (SelectedObjectID);
		}
	}

	return ;
}
