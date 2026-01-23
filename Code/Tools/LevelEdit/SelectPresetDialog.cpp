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

// SelectPresetDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "SelectPresetDialog.h"
#include "PresetMgr.h"
#include "preset.h"
#include "icons.h"
#include "nodecategories.h"
#include "definitionfactory.h"
#include "definitionfactorymgr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// SelectPresetDialogClass
//
/////////////////////////////////////////////////////////////////////////////
SelectPresetDialogClass::SelectPresetDialogClass(CWnd* pParent /*=NULL*/)
	:	m_ClassID (0),
		m_Preset (NULL),
		m_IconIndex (0),
		m_InitialSelection (NULL),
		m_AllowNoneSelection (true),
		CDialog(SelectPresetDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(SelectPresetDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
SelectPresetDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SelectPresetDialogClass)
	DDX_Control(pDX, IDC_PRESET_TREE, m_TreeCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(SelectPresetDialogClass, CDialog)
	//{{AFX_MSG_MAP(SelectPresetDialogClass)
	ON_WM_DESTROY()
	ON_NOTIFY(TVN_SELCHANGED, IDC_PRESET_TREE, OnSelchangedPresetTree)
	ON_BN_CLICKED(IDC_INFO, OnInfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
SelectPresetDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();
	CWaitCursor wait_cursor;

	//
	//	Configure the tree control
	//
	m_TreeCtrl.SetImageList (::Get_Global_Image_List (), TVSIL_NORMAL);

	//
	//	Determine if we should display all presets or just a specific branch
	//	of the tree.
	//
	if (m_ClassID == 0) {
		
		//
		//	Build the complete preset tree
		//
		Build_Full_Preset_Tree ();

	} else {

		//
		//	Build the tree for just this branch of presets
		//
		Generate_Tree (m_ClassID, TVI_ROOT);
	}

	//
	//	Add a way for the user to clear the selection
	//
	HTREEITEM none_entry = NULL;
	if (m_AllowNoneSelection) {
		none_entry = m_TreeCtrl.InsertItem ("<None>", NULL_ICON, NULL_ICON, TVI_ROOT, TVI_FIRST);
	}

	//
	//	Select the correct entry (if necessary)
	//
	if (m_InitialSelection != NULL) {
		m_TreeCtrl.EnsureVisible (m_InitialSelection);
		m_TreeCtrl.SelectItem (m_InitialSelection);
	} else if (none_entry != NULL) {
		m_TreeCtrl.SelectItem (none_entry);
	}

	//
	//	Change the window title if necessary
	//
	if (m_Title.GetLength () > 0) {
		SetWindowText (m_Title);
	}

	//
	//	Change the dialog message if necessary
	//
	if (m_Message.GetLength () > 0) {
		SetDlgItemText (IDC_MESSAGE, m_Message);
	}


	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// Fill_Tree
//
/////////////////////////////////////////////////////////////////////////////
void
SelectPresetDialogClass::Fill_Tree (NTreeLeafClass<PresetClass *> *leaf, HTREEITEM parent_item)
{
	//
	//	Loop over all the presets
	//
	for (	; leaf != NULL; leaf = leaf->Peek_Next ()) {
		PresetClass *preset = leaf->Get_Value ();

		ASSERT (preset != NULL);
		if (preset != NULL) {

			//
			//	Insert the new preset into the tree
			//
			int icon_index = preset->Get_Icon_Index ();
			HTREEITEM new_item = m_TreeCtrl.InsertItem (preset->Get_Name (), icon_index, icon_index, parent_item);
			if (new_item != NULL) {

				//
				//	Associate the preset with its tree entry
				//
				m_TreeCtrl.SetItemData (new_item, (DWORD_PTR)preset);

				//
				//	Recurse if necessary
				//
				NTreeLeafClass<PresetClass *> *child = leaf->Peek_Child ();
				if (child != NULL) {
					Fill_Tree (child, new_item);
					m_TreeCtrl.SortChildren (parent_item);
				}

				//
				//	Remember that we need to select this item
				//
				if (preset == m_Preset) {
					m_InitialSelection = new_item;
				}
			}
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
SelectPresetDialogClass::OnOK (void)
{
	//
	//	Save the selected preset
	//
	HTREEITEM selected_item = m_TreeCtrl.GetSelectedItem ();	
	if (selected_item != NULL) {
		m_Preset = (PresetClass *)m_TreeCtrl.GetItemData (selected_item);
	}

	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDestroy
//
/////////////////////////////////////////////////////////////////////////////
void
SelectPresetDialogClass::OnDestroy (void)
{
	//
	// Remove the main image list we associated with the control
	//
	m_TreeCtrl.SetImageList (NULL, TVSIL_NORMAL);

	CDialog::OnDestroy ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Build_Full_Preset_Tree
//
/////////////////////////////////////////////////////////////////////////////
void
SelectPresetDialogClass::Build_Full_Preset_Tree (void)
{
	//
	//	Add all the node categories to the list control
	//
	for (int index = 0; index < PRESET_CATEGORY_COUNT; index ++) {

		//
		//	Add this factory to the tree
		//		
		HTREEITEM tree_item = m_TreeCtrl.InsertItem (PRESET_CATEGORIES[index].name, FOLDER_ICON, FOLDER_ICON);
		m_IconIndex = PRESET_CATEGORIES[index].icon;		
				
		//
		//	Add all presets for this factory into the tree
		//
		Generate_Tree (PRESET_CATEGORIES[index].clsid, tree_item);

		//
		//	Is there a factory to create this class of defintion?
		//
		/*DefinitionFactoryClass *factory = DefinitionFactoryMgrClass::Find_Factory (PRESET_CATEGORIES[index].clsid);
		if (factory == NULL) {
			
			//
			//	Find all the sub-factories
			//			
			for (	factory = DefinitionFactoryMgrClass::Get_First (PRESET_CATEGORIES[index].clsid);
					factory != NULL;
					factory = DefinitionFactoryMgrClass::Get_Next (factory, PRESET_CATEGORIES[index].clsid))
			{
				//
				//	Add this sub-factory and all its definitions to the tree
				//
				LPCTSTR name = factory->Get_Name ();
				HTREEITEM child_item = m_TreeCtrl.InsertItem (factory->Get_Name (), FOLDER_ICON, FOLDER_ICON, tree_item);
				Generate_Tree (factory->Get_Class_ID (), child_item);
			}

			//
			//	Sort the entries we just made
			//
			m_TreeCtrl.SortChildren (tree_item);
		}*/
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Generate_Tree
//
/////////////////////////////////////////////////////////////////////////////
void
SelectPresetDialogClass::Generate_Tree (int class_id, HTREEITEM parent_item)
{
	//
	//	Is there a factory to create this class of defintion?
	//
	DefinitionFactoryClass *factory = DefinitionFactoryMgrClass::Find_Factory (class_id);
	if (factory == NULL) {
		
		//
		//	Find all the sub-factories
		//			
		for (	factory = DefinitionFactoryMgrClass::Get_First (class_id);
				factory != NULL;
				factory = DefinitionFactoryMgrClass::Get_Next (factory, class_id))
		{
			//
			//	Add this sub-factory and all its definitions to the tree
			//
			if (factory->Is_Displayed ()) {
				HTREEITEM child_item = m_TreeCtrl.InsertItem (factory->Get_Name (), FOLDER_ICON, FOLDER_ICON, parent_item);
				Generate_Tree (factory->Get_Class_ID (), child_item);
			}
		}

		//
		//	Sort the entries we just made
		//
		m_TreeCtrl.SortChildren (parent_item);
	} else {

		//
		//	Generate the tree of presets
		//
		PRESET_TREE preset_tree;
		PresetMgrClass::Build_Preset_Tree (class_id, preset_tree);

		//
		//	Populate the tree control
		//
		Fill_Tree (preset_tree.Peek_Root (), parent_item);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSelchangedPresetTree
//
/////////////////////////////////////////////////////////////////////////////
void
SelectPresetDialogClass::OnSelchangedPresetTree
(
	NMHDR *		/* pNMHDR */,
	LRESULT *	pResult
)
{
	(*pResult) = 0;

	//
	//	Either disable or enable the OK button depending on if the user selected
	// a real preset or not...
	//
	HTREEITEM selected_item = m_TreeCtrl.GetSelectedItem ();
	if (selected_item != NULL) {
		PresetClass *preset = (PresetClass *)m_TreeCtrl.GetItemData (selected_item);
		
		if (m_AllowNoneSelection == false) {
			::EnableWindow (::GetDlgItem (m_hWnd, IDOK), preset != NULL);
		}

		::EnableWindow (::GetDlgItem (m_hWnd, IDC_INFO), preset != NULL);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnInfo
//
/////////////////////////////////////////////////////////////////////////////
void
SelectPresetDialogClass::OnInfo (void)
{
	HTREEITEM selected_item = m_TreeCtrl.GetSelectedItem ();
	if (selected_item != NULL) {
		PresetClass *preset = (PresetClass *)m_TreeCtrl.GetItemData (selected_item);
		if (preset != NULL) {
			
			//
			//	Show the settings for this preset
			//
			preset->Show_Properties (true);
		}
	}

	return ;
}
