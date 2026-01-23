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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/NodeScriptsPropPage.cpp                                                                                                                                                                                                                                                                                                                                          $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "LevelEdit.h"
#include "NodeScriptsPropPage.h"
#include "editscript.h"
#include "scriptmgr.h"
#include "Utils.h"
#include "scripteditdialog.h"
#include "parameter.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// Local Constants
//
/////////////////////////////////////////////////////////////////////////////
const int COL_NAME		= 0;
const int COL_PARAMS		= 1;


/////////////////////////////////////////////////////////////////////////////
//
// NodeScriptsPropPage
//
/////////////////////////////////////////////////////////////////////////////
NodeScriptsPropPage::NodeScriptsPropPage (void)
	:	m_ScriptList (NULL),
		m_ScriptListParam (NULL),
		DockableFormClass (NodeScriptsPropPage::IDD)
{
	//{{AFX_DATA_INIT(NodeScriptsPropPage)
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// NodeScriptsPropPage
//
/////////////////////////////////////////////////////////////////////////////
NodeScriptsPropPage::NodeScriptsPropPage (SCRIPT_LIST *script_list)
	:	m_ScriptList (script_list),
		m_ScriptListParam (NULL),
		DockableFormClass (NodeScriptsPropPage::IDD)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~NodeScriptsPropPage
//
/////////////////////////////////////////////////////////////////////////////
NodeScriptsPropPage::~NodeScriptsPropPage (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
NodeScriptsPropPage::DoDataExchange (CDataExchange* pDX)
{
	DockableFormClass ::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NodeScriptsPropPage)
	DDX_Control(pDX, IDC_SCRIPTLIST, m_ListCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(NodeScriptsPropPage, DockableFormClass )
	//{{AFX_MSG_MAP(NodeScriptsPropPage)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SCRIPTLIST, OnItemChangedScriptList)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_MODIFY, OnModify)
	ON_NOTIFY(LVN_DELETEITEM, IDC_SCRIPTLIST, OnDeleteItemScriptList)
	ON_NOTIFY(NM_DBLCLK, IDC_SCRIPTLIST, OnDblclkScriptlist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// NodeScriptsPropPage diagnostics

#ifdef _DEBUG
void NodeScriptsPropPage::AssertValid() const
{
	DockableFormClass ::AssertValid();
}

void NodeScriptsPropPage::Dump(CDumpContext& dc) const
{
	DockableFormClass ::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
//
// Build_Script_List
//
/////////////////////////////////////////////////////////////////////////////
void
NodeScriptsPropPage::Build_Script_List (void)
{
	//
	//	If we are dealing with a script list parameter, then convert
	// it into a list of editable scripts
	//
	if (m_ScriptListParam != NULL) {
		m_ScriptList = new SCRIPT_LIST;

		DynamicVectorClass<StringClass> &name_list = m_ScriptListParam->Get_Name_List ();
		DynamicVectorClass<StringClass> &param_list = m_ScriptListParam->Get_Param_List ();
		
		//
		//	Loop over and create edit scripts for each string pair
		//
		for (int index = 0; index < name_list.Count (); index ++) {
			EditScriptClass *edit_script = new EditScriptClass;
			edit_script->Set_Name (name_list[index]);
			edit_script->Lookup_Param_Description ();
			edit_script->Set_Composite_Values (param_list[index]);

			//
			//	Add this script to our list
			//
			m_ScriptList->Add (edit_script);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// HandleInitDialog
//
/////////////////////////////////////////////////////////////////////////////
void
NodeScriptsPropPage::HandleInitDialog (void)
{
	ListView_SetExtendedListViewStyle (m_ListCtrl, LVS_EX_FULLROWSELECT);

	//
	//	Make sure we have a script list (can be built internally in some cases)
	//
	Build_Script_List ();

	// Insert the 2 columns into the list oontrol
	m_ListCtrl.InsertColumn (COL_NAME, "Name");
	m_ListCtrl.InsertColumn (COL_PARAMS, "Params");

	//
	// Add all the scripts that we are editing to the list control
	//
	for (int index = 0; index < m_ScriptList->Count (); index ++) {		
		EditScriptClass *script = (*m_ScriptList)[index];
		if (script != NULL) {
			
			//
			// Put this script into the list control
			//
			int item_index = m_ListCtrl.InsertItem (index, (LPCTSTR)script->Get_Name ());
			m_ListCtrl.SetItemText (item_index, COL_PARAMS, (LPCTSTR)script->Get_Composite_String ());
			m_ListCtrl.SetItemData (item_index, (DWORD_PTR)script->Clone ());
		}
	}

	//
	// Size the columns to fit their largest entry
	//
	CRect rect;
	m_ListCtrl.GetClientRect (&rect);
	m_ListCtrl.SetColumnWidth (COL_NAME, (rect.Width () / 3) - 2);
	m_ListCtrl.SetColumnWidth (COL_PARAMS, ((rect.Width () * 2) / 3) - 2);

	// Disable the 'add', 'delete' and 'modify' buttons
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_DELETE), false);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_MODIFY), false);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnItemChangedScriptList
//
/////////////////////////////////////////////////////////////////////////////
void
NodeScriptsPropPage::OnItemChangedScriptList
(
	NMHDR*	pNMHDR,
	LRESULT*	pResult
)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	// Did the selection state change?
	if ((pNMListView != NULL) &&
		 (pNMListView->uChanged & LVIF_STATE) &&
		 ((pNMListView->uNewState & LVIS_SELECTED) || (pNMListView->uOldState & LVIS_SELECTED)))
	{
		
		// Get the index of the currently selected property
		int index = m_ListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
		if (index != -1) {

			// Enable the 'delete' and 'modify' buttons
			::EnableWindow (::GetDlgItem (m_hWnd, IDC_DELETE), true);
			::EnableWindow (::GetDlgItem (m_hWnd, IDC_MODIFY), true);

		} else {

			// Disable the 'delete' and 'modify' buttons
			::EnableWindow (::GetDlgItem (m_hWnd, IDC_DELETE), false);
			::EnableWindow (::GetDlgItem (m_hWnd, IDC_MODIFY), false);
		}
	}
	
	*pResult = 0;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnModify
//
/////////////////////////////////////////////////////////////////////////////
void
NodeScriptsPropPage::OnModify (void)
{
	// Get the index of the currently selected property
	int index = m_ListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (index != -1) {

		// Get a pointer to the script
		EditScriptClass *script = (EditScriptClass *)m_ListCtrl.GetItemData (index);
		if (script != NULL) {

			//
			//	Show the dialog that allows the user to edit this script's settings
			//
			ScriptEditDialogClass dialog (this);
			dialog.Set_Script (*script);
			if (dialog.DoModal () == IDOK) {
				
				//
				//	Update the list
				//
				(*script) = dialog.Get_Script ();
				m_ListCtrl.SetItemText (index, COL_NAME, (LPCTSTR)script->Get_Name ());
				m_ListCtrl.SetItemText (index, COL_PARAMS, (LPCTSTR)script->Get_Composite_String ());
			}
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
NodeScriptsPropPage::OnAdd (void)
{
	//
	//	Show the dialog that allows the user to edit this script's settings
	//
	ScriptEditDialogClass dialog (this);
	if (dialog.DoModal () == IDOK) {
		EditScriptClass *new_script = new EditScriptClass (dialog.Get_Script ());

		// Put this script into the list control
		int index = m_ListCtrl.InsertItem (0xFFFF, (LPCTSTR)new_script->Get_Name ());
		m_ListCtrl.SetItemText (index, COL_PARAMS, (LPCTSTR)new_script->Get_Composite_String ());
		m_ListCtrl.SetItemData (index, (DWORD_PTR)new_script);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDelete
//
/////////////////////////////////////////////////////////////////////////////
void
NodeScriptsPropPage::OnDelete (void) 
{
	// Get the index of the currently selected property
	int index = m_ListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (index != -1) {

		// Delete the entry from the list control
		m_ListCtrl.DeleteItem (index);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Apply_Changes
//
/////////////////////////////////////////////////////////////////////////////
bool
NodeScriptsPropPage::Apply_Changes (void)
{
	//
	//	Delete all the scripts in the old list
	//
	int index;
	for (index = 0; index < m_ScriptList->Count (); index ++) {
		EditScriptClass *script = (*m_ScriptList)[index];
		SAFE_DELETE (script);
	}

	m_ScriptList->Delete_All ();

	//
	// Loop through all the entries in the list contrl and add them 
	// to the script list
	//
	index = -1;
	while ((index = m_ListCtrl.GetNextItem (index, LVNI_ALL)) != -1) {
		EditScriptClass *script = (EditScriptClass *)m_ListCtrl.GetItemData (index);
		if (script != NULL) {

			//
			//	Add this script to the script list and remove it from the list control
			//
			m_ScriptList->Add (script);
			m_ListCtrl.SetItemData (index, NULL);
		}
	}

	//
	//	Do we need to convert the script list to a script list param object?
	//
	if (m_ScriptListParam != NULL) {

		//
		//	Reset the contents of the script parameter's lists
		//
		DynamicVectorClass<StringClass> &name_list = m_ScriptListParam->Get_Name_List ();
		DynamicVectorClass<StringClass> &param_list = m_ScriptListParam->Get_Param_List ();
		name_list.Delete_All ();
		param_list.Delete_All ();

		//
		//	Copy the data from each script object into the string lists
		//
		for (index = 0; index < m_ScriptList->Count (); index ++) {
			EditScriptClass *script = (*m_ScriptList)[index];

			//
			//	Pull the values from the script object into the string lists
			//
			name_list.Add (script->Get_Name ());
			param_list.Add ((LPCTSTR)script->Get_Composite_String ());

			SAFE_DELETE (script);
		}

		SAFE_DELETE (m_ScriptList);
	}


	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeleteItemPropertyList
//
/////////////////////////////////////////////////////////////////////////////
void
NodeScriptsPropPage::OnDeleteItemScriptList
(
	NMHDR *		pNMHDR,
	LRESULT *	pResult
)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (pNMListView != NULL) {

		// Free the associated script
		EditScriptClass *script = (EditScriptClass *)m_ListCtrl.GetItemData (pNMListView->iItem);
		SAFE_DELETE (script);
		
		// Reset the associated data for this item
		m_ListCtrl.SetItemData (pNMListView->iItem, NULL);
	}
	
	*pResult = 0;
	return;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDblclkScriptlist
//
/////////////////////////////////////////////////////////////////////////////
void
NodeScriptsPropPage::OnDblclkScriptlist
(
	NMHDR *		/* pNMHDR */,
	LRESULT *	pResult
)
{
	(*pResult) = 0;
	OnModify ();
	return ;
}
