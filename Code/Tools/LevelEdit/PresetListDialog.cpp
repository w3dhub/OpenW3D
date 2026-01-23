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

// PresetListDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "PresetListDialog.h"
#include "PresetMgr.h"
#include "definition.h"
#include "definitionfactory.h"
#include "definitionfactorymgr.h"
#include "icons.h"
#include "Utils.h"
#include "preset.h"
#include "persistfactory.h"
#include <algorithm>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// PresetListDialogClass
//
/////////////////////////////////////////////////////////////////////////////
PresetListDialogClass::PresetListDialogClass (CWnd *pParent)
	:	m_List (NULL),
		m_ClassID (0),
		m_RootClassID (0),
		m_EnableTypeSel (true),
		CDialog(PresetListDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(PresetListDialogClass)
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
PresetListDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PresetListDialogClass)
	DDX_Control(pDX, IDC_TYPE_COMBO, m_ComboBox);
	DDX_Control(pDX, IDC_PRESET_TREE, m_PresetTreeCtrl);
	DDX_Control(pDX, IDC_PRESET_LIST, m_PresetListCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(PresetListDialogClass, CDialog)
	//{{AFX_MSG_MAP(PresetListDialogClass)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_NOTIFY(NM_DBLCLK, IDC_PRESET_LIST, OnDblclkPresetList)
	ON_NOTIFY(NM_DBLCLK, IDC_PRESET_TREE, OnDblclkPresetTree)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_PRESET_LIST, OnItemchangedPresetList)
	ON_NOTIFY(TVN_SELCHANGED, IDC_PRESET_TREE, OnSelchangedPresetTree)
	ON_WM_DESTROY()
	ON_CBN_SELENDOK(IDC_TYPE_COMBO, OnSelendokTypeCombo)
	ON_CBN_SELCHANGE(IDC_TYPE_COMBO, OnSelchangeTypeCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
PresetListDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();
	ASSERT (m_List != NULL);

	//
	//	Configure the preset list control
	//
	m_PresetListCtrl.InsertColumn (0, "Name");
	m_PresetListCtrl.InsertColumn (1, "Type");	

	//
	//	Configure the type combobox
	//
	Generate_Type_List ();

	//
	//	Add the previously selected presets to the list control
	//
	for (int index = 0; index < m_List->Count (); index ++) {
		
		//
		//	Lookup this preset
		//
		int preset_id			= (*m_List)[index];
		PresetClass *preset	= PresetMgrClass::Find_Preset (preset_id);
		
		//
		//	Add the preset to the list control
		//
		Add_Preset (preset);
	}

	//
	//	Fill in the tree control
	//
	m_PresetTreeCtrl.SetImageList (::Get_Global_Image_List (), TVSIL_NORMAL);
	Populate_Preset_Tree ();
	
	//
	//	Size the columns
	//
	CRect rect;
	m_PresetListCtrl.GetClientRect (&rect);
	m_PresetListCtrl.SetColumnWidth (0, rect.Width () * 2 / 3);
	m_PresetListCtrl.SetColumnWidth (1, (rect.Width () / 3) - 2);

	Update_Button_State ();
	return true;
}


////////////////////////////////////////////////////////////////////////////
//
//  Update_Button_State
//
////////////////////////////////////////////////////////////////////////////
void
PresetListDialogClass::Update_Button_State (void)
{
	bool enable_add		= false;
	bool enable_remove	= (m_PresetListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED) >= 0);

	//
	//	Should the add button be enabled?
	//
	HTREEITEM tree_item = m_PresetTreeCtrl.GetSelectedItem ();
	if (tree_item != NULL) {
		PresetClass *preset = (PresetClass *)m_PresetTreeCtrl.GetItemData (tree_item);
		if (preset != NULL) {
			enable_add = true;
		}
	}

	::EnableWindow (::GetDlgItem (m_hWnd, IDC_ADD), enable_add);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_REMOVE), enable_remove);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Sort_Nodes
//
////////////////////////////////////////////////////////////////////////////
void
PresetListDialogClass::Sort_Nodes (HTREEITEM root_item, bool recursive)
{
	for (	HTREEITEM tree_item = m_PresetTreeCtrl.GetChildItem (root_item);
			tree_item != NULL;
			tree_item = m_PresetTreeCtrl.GetNextSiblingItem (tree_item))
	{
		//
		//	Sort the children
		//
		m_PresetTreeCtrl.SortChildren (tree_item);

		//
		//	Recurse through the hierarchy
		//
		if (recursive && m_PresetTreeCtrl.ItemHasChildren (tree_item)) {
			Sort_Nodes (tree_item, recursive);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Add_Preset
//
/////////////////////////////////////////////////////////////////////////////
void
PresetListDialogClass::Add_Preset (PresetClass *preset)
{
	if (preset != NULL && preset->Get_Definition () != NULL) {
		
		//
		//	Insert this preset into the list control
		//
		int list_index = m_PresetListCtrl.InsertItem (0xFFF, preset->Get_Name ());
		if (list_index >= 0) {
			
			//
			//	Set the entry's type name
			//
			int class_id							= preset->Get_Definition ()->Get_Class_ID ();
			DefinitionFactoryClass *factory	= DefinitionFactoryMgrClass::Find_Factory (class_id);
			if (factory != NULL) {
				LPCTSTR type_name = factory->Get_Name ();
				m_PresetListCtrl.SetItemText (list_index, 1, type_name);
			}

			//
			//	Associate the preset ID with this list entry
			//
			m_PresetListCtrl.SetItemData (list_index, preset->Get_ID ());
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
PresetListDialogClass::OnOK (void)
{
	m_List->Delete_All ();

	int count = m_PresetListCtrl.GetItemCount ();
	for (int index = 0; index < count; index ++) {
		int preset_id = m_PresetListCtrl.GetItemData (index);
		m_List->Add (preset_id);
	}	

	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnAdd
//
/////////////////////////////////////////////////////////////////////////////
void
PresetListDialogClass::OnAdd (void)
{
	HTREEITEM tree_item = m_PresetTreeCtrl.GetSelectedItem ();
	if (tree_item != NULL) {

		//
		//	Add the preset from the currently selected tree item to the list
		//
		PresetClass *preset = (PresetClass *)m_PresetTreeCtrl.GetItemData (tree_item);
		if (preset != NULL) {
			Add_Preset (preset);
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
PresetListDialogClass::OnRemove (void)
{
	int index = m_PresetListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (index >= 0) {
		m_PresetListCtrl.DeleteItem (index);

		//
		//	Select the previous entry in the list
		//
		int count = m_PresetListCtrl.GetItemCount ();
		if (count > 0) {
			if (index >= (count-1)) {
				index --;
			}
			index = std::max (index, 0);
			m_PresetListCtrl.SetItemState (index, LVNI_SELECTED, LVNI_SELECTED);
		}

		Update_Button_State ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDblclkPresetList
//
/////////////////////////////////////////////////////////////////////////////
void
PresetListDialogClass::OnDblclkPresetList
(
	NMHDR *		/* pNMHDR */,
	LRESULT *	pResult
) 
{
	(*pResult) = 0;

	OnRemove ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDblclkPresetTree
//
/////////////////////////////////////////////////////////////////////////////
void
PresetListDialogClass::OnDblclkPresetTree
(
	NMHDR *		/* pNMHDR */,
	LRESULT *	pResult
) 
{
	(*pResult) = 0;

	OnAdd ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnItemchangedPresetList
//
/////////////////////////////////////////////////////////////////////////////
void
PresetListDialogClass::OnItemchangedPresetList
(
	NMHDR *		/* pNMHDR */,
	LRESULT *	pResult
) 
{
	(*pResult) = 0;

	Update_Button_State ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSelchangedPresetTree
//
/////////////////////////////////////////////////////////////////////////////
void
PresetListDialogClass::OnSelchangedPresetTree
(
	NMHDR *		/* pNMHDR */,
	LRESULT *	pResult
) 
{
	(*pResult) = 0;

	Update_Button_State ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDestroy
//
/////////////////////////////////////////////////////////////////////////////
void
PresetListDialogClass::OnDestroy (void) 
{
	m_PresetTreeCtrl.SetImageList (NULL, TVSIL_NORMAL);
	CDialog::OnDestroy ();	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Generate_Type_List
//
/////////////////////////////////////////////////////////////////////////////
void
PresetListDialogClass::Generate_Type_List (void) 
{	
	//
	// Configure the combo-box
	//
	m_ComboBox.SetImageList (::Get_Global_Image_List ());
	m_ComboBox.EnableWindow (m_EnableTypeSel);
	
	//
	// Build a tree of factories
	//
	FACTORY_TREE factory_tree;
	PresetMgrClass::Build_Factory_Tree (m_RootClassID, factory_tree);

	//
	//	Default to game-objects if no other type is selected
	//
	if (m_ClassID == 0) {
		m_ClassID = CLASSID_GAME_OBJECTS;
	}

	//
	//	Put these factories into the combobox
	//
	int index = 0;
	Add_Factories_To_Combo (factory_tree.Peek_Root (), 0, index);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Add_Factories_To_Combo
//
/////////////////////////////////////////////////////////////////////////////
void
PresetListDialogClass::Add_Factories_To_Combo
(
	NTreeLeafClass<uint32> *leaf,
	int							indent,
	int &							index
)
{
	do
	{
		uint32 class_id = leaf->Get_Value ();

		//
		//	Add this factory to the combobox
		//
		COMBOBOXEXITEM item	= { 0 };
		item.mask				= CBEIF_IMAGE | CBEIF_INDENT | CBEIF_LPARAM | CBEIF_TEXT | CBEIF_SELECTEDIMAGE;
		item.pszText			= (char *)::Get_Factory_Name (class_id);
		item.iImage				= FOLDER_ICON;
		item.iSelectedImage	= FOLDER_ICON;
		item.iIndent			= indent;
		item.lParam				= (LPARAM)class_id;
		item.iItem				= index ++;
		int item_index			= m_ComboBox.InsertItem (&item);
		
		//
		//	Is this the factory we need to select?
		//
		if (class_id == m_ClassID) {
			m_ComboBox.SetCurSel (item_index);
		}

		//
		//	Recurse if necessary
		//
		NTreeLeafClass<uint32> *child = leaf->Peek_Child ();
		if (child != NULL) {
			Add_Factories_To_Combo (child, indent + 1, index);
		}

	} while ((leaf = leaf->Peek_Next ()) != NULL);

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Populate_Preset_Tree
//
/////////////////////////////////////////////////////////////////////////////
void
PresetListDialogClass::Populate_Preset_Tree (void)
{
	//
	//	Start fresh
	//
	CWaitCursor wait_cursor;
	m_PresetTreeCtrl.DeleteAllItems ();

	//
	//	Should we add any sub-factories to the tree?
	//
	if (DefinitionFactoryMgrClass::Find_Factory (m_ClassID) == NULL) {

		//
		//	Generate the tree from its sub-factories
		//
		DefinitionFactoryClass *factory = NULL;
		for (	factory = DefinitionFactoryMgrClass::Get_First (m_ClassID);
				factory != NULL;
				factory = DefinitionFactoryMgrClass::Get_Next (factory, m_ClassID))
		{
			if (factory->Is_Displayed ()) {
				HTREEITEM folder_item = m_PresetTreeCtrl.InsertItem (factory->Get_Name (), FOLDER_ICON, FOLDER_ICON);

				//
				//	Generate the tree of presets
				//
				PRESET_TREE preset_tree;
				PresetMgrClass::Build_Preset_Tree (factory->Get_Class_ID (), preset_tree);

				//
				//	Populate the tree control
				//
				Fill_Tree (preset_tree.Peek_Root (), folder_item);
			}
		}

	} else {

		//
		//	Generate the tree of presets
		//
		PRESET_TREE preset_tree;
		PresetMgrClass::Build_Preset_Tree (m_ClassID, preset_tree);

		//
		//	Populate the tree control
		//
		Fill_Tree (preset_tree.Peek_Root (), TVI_ROOT);		
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Fill_Tree
//
/////////////////////////////////////////////////////////////////////////////
void
PresetListDialogClass::Fill_Tree (NTreeLeafClass<PresetClass *> *leaf, HTREEITEM parent_item)
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
			int icon = preset->Get_Icon_Index ();
			HTREEITEM new_item = m_PresetTreeCtrl.InsertItem (preset->Get_Name (), icon, icon, parent_item);
			if (new_item != NULL) {

				//
				//	Associate the preset with its tree entry
				//
				m_PresetTreeCtrl.SetItemData (new_item, (DWORD_PTR)preset);

				//
				//	Recurse if necessary
				//
				NTreeLeafClass<PresetClass *> *child = leaf->Peek_Child ();
				if (child != NULL) {
					Fill_Tree (child, new_item);
					m_PresetTreeCtrl.SortChildren (parent_item);
				}
			}
		}
	}

	return ;
}



/*

	//
	//	Add all the presets (by category) to the tree control
	//
	DefinitionFactoryClass *factory = NULL;
	for (	factory = DefinitionFactoryMgrClass::Get_First (m_ClassID);
			factory != NULL;
			factory = DefinitionFactoryMgrClass::Get_Next (factory, m_ClassID))
	{
		//
		//	Add this folder to the tree ctrl
		//
		HTREEITEM folder_item = m_PresetTreeCtrl.InsertItem (factory->Get_Name (), FOLDER_ICON, FOLDER_ICON);

		//
		//	Add all the folder contents to the tree ctrl
		//
		PresetClass *preset = NULL;
		uint32 factory_id = factory->Get_Class_ID ();
		for (	preset = PresetMgrClass::Get_First (factory_id, PresetMgrClass::ID_CLASS);
				preset != NULL;
				preset = PresetMgrClass::Get_Next (preset, factory_id, PresetMgrClass::ID_CLASS))
		{
			//
			//	Add this preset to the tree ctrl
			//
			HTREEITEM preset_item = m_PresetTreeCtrl.InsertItem (	preset->Get_Name (),
																					OBJECT_ICON,
																					OBJECT_ICON,
																					folder_item);
			
			m_PresetTreeCtrl.SetItemData (preset_item, (ULONG)preset);
		}
	}

	//
	//	Sort the tree control
	//
	Sort_Nodes (TVI_ROOT, true);
	m_PresetTreeCtrl.SortChildren (TVI_ROOT);
	return ;
}*/


/////////////////////////////////////////////////////////////////////////////
//
// OnSelendokTypeCombo
//
/////////////////////////////////////////////////////////////////////////////
void
PresetListDialogClass::OnSelendokTypeCombo (void)
{
}


void PresetListDialogClass::OnSelchangeTypeCombo() 
{
	//
	//	Get the currently selected type from the combobox
	//
	int cur_sel = m_ComboBox.GetCurSel ();
	if (cur_sel >= 0) {
		
		//
		//	Repopulate the tree control if the type changed
		//
		uint32 new_class_id = m_ComboBox.GetItemData (cur_sel);
		if (new_class_id != m_ClassID) {		
			m_ClassID = new_class_id;
			Populate_Preset_Tree ();
		}
	}

	return ;
}
