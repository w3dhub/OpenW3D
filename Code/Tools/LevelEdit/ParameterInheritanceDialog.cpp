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

// ParameterInheritanceDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "ParameterInheritanceDialog.h"
#include "definition.h"
#include "presetmgr.h"
#include "preset.h"
#include "parameter.h"
#include "definitionparameter.h"
#include "combatchunkid.h"
#include "soldier.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//	Local macros
/////////////////////////////////////////////////////////////////////////////
#ifndef ListView_SetCheckState
   #define ListView_SetCheckState(hwndLV, i, fCheck) \
      ListView_SetItemState(hwndLV, i, \
      INDEXTOSTATEIMAGEMASK((fCheck)+1), LVIS_STATEIMAGEMASK)
#endif


/////////////////////////////////////////////////////////////////////////////
//	Local constants
/////////////////////////////////////////////////////////////////////////////
static const int ICON_GAME_SETTING		= 0;
static const int ICON_PHYS_SETTING		= 1;


/////////////////////////////////////////////////////////////////////////////
//	Local prototypes
/////////////////////////////////////////////////////////////////////////////
static LRESULT CALLBACK LocalCheckBoxSubclassProc (HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
static BOOL TreeView_CustomSetCheckState (HWND hwndTreeView, HTREEITEM hItem, BOOL fCheck);
static bool Find_Parameter_In_List (DynamicVectorClass<DefinitionParameterClass *> &parameter_list, ParameterClass *parameter);


/////////////////////////////////////////////////////////////////////////////
//
// ParameterInheritanceDialogClass
//
/////////////////////////////////////////////////////////////////////////////
ParameterInheritanceDialogClass::ParameterInheritanceDialogClass(CWnd* pParent /*=NULL*/)
	:	m_Preset (NULL),
		CDialog(ParameterInheritanceDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(ParameterInheritanceDialogClass)
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
ParameterInheritanceDialogClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ParameterInheritanceDialogClass)
	DDX_Control(pDX, IDC_PARAMETER_LIST, m_ListCtrl);
	DDX_Control(pDX, IDC_CHILD_TREE, m_TreeCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(ParameterInheritanceDialogClass, CDialog)
	//{{AFX_MSG_MAP(ParameterInheritanceDialogClass)
	ON_WM_DESTROY()
	ON_NOTIFY(LVN_DELETEITEM, IDC_PARAMETER_LIST, OnDeleteitemParameterList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
ParameterInheritanceDialogClass::OnInitDialog (void) 
{
	CDialog::OnInitDialog ();

	//
	//	Configure the list control
	//
	CRect rect;
	m_ListCtrl.GetClientRect (&rect);
	m_ListCtrl.InsertColumn (0, "Name");
	m_ListCtrl.SetColumnWidth (0, rect.Width () - ::GetSystemMetrics (SM_CXVSCROLL) - 1);

	//
	// Create a small imagelist for the list control to use
	//
	CImageList *imagelist = new CImageList;
	imagelist->Create (16, 16, ILC_COLOR | ILC_MASK, 2, 0);
	HICON icon1 = (HICON)::LoadImage (::AfxGetInstanceHandle (), MAKEINTRESOURCE(IDI_PARAM), IMAGE_ICON, 16, 16, LR_SHARED | LR_DEFAULTCOLOR);
	HICON icon2 = (HICON)::LoadImage (::AfxGetInstanceHandle (), MAKEINTRESOURCE(IDI_PHYS), IMAGE_ICON, 16, 16, LR_SHARED | LR_DEFAULTCOLOR);
	imagelist->Add (icon1);
	imagelist->Add (icon2);
	m_ListCtrl.SetImageList (imagelist, LVSIL_SMALL);

	//
	// Create a state imagelist for the list control to use for checkboxes
	//
	imagelist = new CImageList;
	imagelist->Create (MAKEINTRESOURCE (IDB_CHECKBOX_STATES1), 13, 0, RGB (255, 0, 255));
	m_ListCtrl.SetImageList (imagelist, LVSIL_STATE);

	//
	// Create a state imagelist for the tree control to use for checkboxes
	//
	imagelist = new CImageList;
	imagelist->Create (MAKEINTRESOURCE (IDB_CHECKBOX_STATES), 13, 0, RGB (255, 0, 255));
	m_TreeCtrl.SetImageList (imagelist, TVSIL_STATE);

	//
	//	Fill in the parameter list and preset list
	//
	Populate_List_Ctrl ();
	Populate_Tree_Ctrl ();

	//
	//	Subclass the list and tree controls so we can handle the checkstates
	//
	LONG_PTR oldproc1 = ::SetWindowLongPtr (m_ListCtrl, GWLP_WNDPROC, (LONG_PTR)LocalCheckBoxSubclassProc);
	LONG_PTR oldproc2 = ::SetWindowLongPtr (m_TreeCtrl, GWLP_WNDPROC, (LONG_PTR)LocalCheckBoxSubclassProc);
	::SetProp (m_ListCtrl, "OLDPROC",		(HANDLE)(DWORD_PTR)oldproc1);
	::SetProp (m_TreeCtrl, "OLDPROC",		(HANDLE)(DWORD_PTR)oldproc2);
	::SetProp (m_ListCtrl, "IS_LIST_CTRL", (HANDLE)true);
	::SetProp (m_TreeCtrl, "IS_LIST_CTRL", (HANDLE)false);

	//
	//	Determine whether or not to enable the dialogue propogation controls
	//
	bool enable_dialogue_ctrls = false;
	if (m_Preset != NULL && m_Preset->Is_Soldier_Preset ()) {
		enable_dialogue_ctrls = true;
	}
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_PROPOGATE_DIALOGUE), enable_dialogue_ctrls);
		
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// Populate_List_Ctrl
//
/////////////////////////////////////////////////////////////////////////////
void
ParameterInheritanceDialogClass::Populate_List_Ctrl (void)
{
	ASSERT (m_Preset != NULL);

	//
	//	Get the definition this preset sits on
	//
	DefinitionClass *definition = m_Preset->Get_Definition ();
	ASSERT (definition != NULL);		
	if (definition != NULL) {

		//
		//	Recursively add paramters from this definition to the list control
		//
		Add_Parameters_To_List (definition, NULL, ICON_GAME_SETTING);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Add_Parameters_To_List
//
/////////////////////////////////////////////////////////////////////////////
void
ParameterInheritanceDialogClass::Add_Parameters_To_List
(
	DefinitionClass *				definition,
	DefinitionParameterClass *	parent,
	int								icon
)
{
	ASSERT (definition != NULL);

	//
	//	Add these parameters to the list control
	//
	int count = definition->Get_Parameter_Count ();
	for (int index = 0; index < count; index ++) {
		ParameterClass *parameter = definition->Lock_Parameter (index);
		if (parameter != NULL) {

			//
			//	Is this parameter one we need to recurse into?
			//
			if (parameter->Get_Type () == ParameterClass::TYPE_MODELDEFINITIONID) {
				
				//
				//	Lookup the definition that this parameter points to
				//
				int def_id = ((ModelDefParameterClass *)parameter)->Get_Value ();
				DefinitionClass *model_def = DefinitionMgrClass::Find_Definition (def_id, false);
				if (model_def != NULL) {

					DefinitionParameterClass *def_param = new DefinitionParameterClass;
					def_param->Set_Definition (definition);
					def_param->Set_Parameter (parameter);
					def_param->Set_Index (index);
					def_param->Set_Parent (parent);

					//
					//	Recurse into this model definition
					//
					Add_Parameters_To_List (model_def, def_param, ICON_PHYS_SETTING);
					MEMBER_RELEASE (def_param);
				}

			} else {									
				
				//
				//	Insert this preset into the list control
				//
				int item_index = m_ListCtrl.InsertItem (0xFFFF, parameter->Get_Name (), icon);
				if (item_index >= 0) {

					DefinitionParameterClass *def_param = new DefinitionParameterClass;
					def_param->Set_Definition (definition);
					def_param->Set_Parameter (parameter);
					def_param->Set_Index (index);
					def_param->Set_Parent (parent);

					m_ListCtrl.SetItemData (item_index, (DWORD_PTR)def_param);
					ListView_SetCheckState (m_ListCtrl, item_index, false);
				}
			}
		}

		definition->Unlock_Parameter (index);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Populate_Tree_Ctrl
//
/////////////////////////////////////////////////////////////////////////////
void
ParameterInheritanceDialogClass::Populate_Tree_Ctrl (void)
{
	ASSERT (m_Preset != NULL);
	int parent_id = m_Preset->Get_ID ();

	//
	//	Add all the presets that 'inherit' from this preset
	//
	Add_Children_To_Tree (TVI_ROOT, parent_id);	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Add_Children_To_Tree
//
/////////////////////////////////////////////////////////////////////////////
void
ParameterInheritanceDialogClass::Add_Children_To_Tree (HTREEITEM parent_item, int parent_id)
{
	//
	//	Lookup the parent preset
	//
	PresetClass *parent_preset = PresetMgrClass::Find_Preset (parent_id);	
	if (parent_preset != NULL) {

		//
		//	Loop over all the children of this preset
		//
		int count = parent_preset->Get_Child_Preset_Count ();
		for (int index = 0; index < count; index ++) {
			PresetClass *child_preset = parent_preset->Get_Child_Preset (index);
			if (child_preset != NULL) {

				//
				//	Add this preset to the tree
				//
				HTREEITEM new_item = m_TreeCtrl.InsertItem (child_preset->Get_Name (), parent_item);
				if (new_item != NULL) {

					//
					//	Associate the preset pointer with this tree item
					//
					m_TreeCtrl.SetItemData (new_item, (DWORD_PTR)child_preset);

					//
					//	Check this preset by default
					//
					TreeView_CustomSetCheckState (m_TreeCtrl, new_item, true);

					//
					//	Recursively fill in this presets's children
					//
					Add_Children_To_Tree (new_item, child_preset->Get_ID ());
				}
			}
		}
	}
	
	//
	//	Ensure this leaf is sorted
	//
	m_TreeCtrl.SortChildren (parent_item);
	m_TreeCtrl.Expand (parent_item, TVE_EXPAND);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDestroy
//
/////////////////////////////////////////////////////////////////////////////
void
ParameterInheritanceDialogClass::OnDestroy (void)
{
	//
	// Free the small image list we associated with the list control
	//
	CImageList *imagelist = m_ListCtrl.GetImageList (LVSIL_SMALL);
	m_ListCtrl.SetImageList (NULL, LVSIL_SMALL);
	SAFE_DELETE (imagelist);

	//
	// Free the state image list we associated with the list control
	//
	imagelist = m_ListCtrl.GetImageList (LVSIL_STATE);
	m_ListCtrl.SetImageList (NULL, LVSIL_STATE);
	SAFE_DELETE (imagelist);

	//
	// Free the state image list we associated with the tree control
	//
	imagelist = m_TreeCtrl.GetImageList (TVSIL_STATE);
	m_TreeCtrl.SetImageList (NULL, TVSIL_STATE);
	SAFE_DELETE (imagelist);

	CDialog::OnDestroy ();
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  LocalCheckBoxSubclassProc
//
////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK
LocalCheckBoxSubclassProc
(
	HWND		hwnd,
	UINT		message,
	WPARAM	wparam,
	LPARAM	lparam
)
{
	WNDPROC old_proc = (WNDPROC)::GetProp (hwnd, "OLDPROC");
	
	if (message == WM_LBUTTONUP) {

		//
		//	Is this the list control or the tree control?
		//
		BOOL is_list_ctrl = (BOOL)(DWORD_PTR)::GetProp (hwnd, "IS_LIST_CTRL");
		if (is_list_ctrl) {
			
			//
			//	Find out where the user clicked
			//
			LVHITTESTINFO hittest = { 0 };
			hittest.pt.x = LOWORD (lparam);
			hittest.pt.y = HIWORD (lparam);
			::SendMessage (hwnd, LVM_HITTEST, 0, (LPARAM)&hittest);
			
			//
			//	Did the user click one of the checkboxes?
			//
			if (hittest.flags & LVHT_ONITEMSTATEICON) {

				//
				//	Notify the dialog that the user wants to toggle the checkbox
				//
				::PostMessage (::GetParent (hwnd), WM_USER+102, 0, (LPARAM)hittest.iItem);
			}

		} else {

			//
			//	Find out where the user clicked
			//
			TVHITTESTINFO hittest = { 0 };
			hittest.pt.x = LOWORD (lparam);
			hittest.pt.y = HIWORD (lparam);
			::SendMessage (hwnd, TVM_HITTEST, 0, (LPARAM)&hittest);
			
			//
			//	Did the user click one of the checkboxes?
			//
			if (hittest.flags & TVHT_ONITEMSTATEICON) {

				//
				//	Notify the dialog that the user wants to toggle the checkbox
				//
				::PostMessage (::GetParent (hwnd), WM_USER+103, 0, (LPARAM)hittest.hItem);
			}
		}

	} else if (message == WM_DESTROY) {
		::SetWindowLongPtr (hwnd, GWLP_WNDPROC, (LONG_PTR)old_proc);
		::RemoveProp (hwnd, "OLDPROC");
		::RemoveProp (hwnd, "IS_LIST_CTRL");
	}

	//
	//	Allow the default message processing to occur
	//
	LRESULT result = 0L;
	if (old_proc != NULL) {
		result = ::CallWindowProc (old_proc, hwnd, message, wparam, lparam);
	} else {
		result = ::DefWindowProc (hwnd, message, wparam, lparam);
	}

	return result;
}


/////////////////////////////////////////////////////////////////////////////
//
// WindowProc
//
/////////////////////////////////////////////////////////////////////////////
LRESULT
ParameterInheritanceDialogClass::WindowProc
(
	UINT		message,
	WPARAM	wParam,
	LPARAM	lParam
)
{	
	if (message == WM_USER + 102) {

		//
		//	Invert the checked state of this entry
		//
		int index		= (int)lParam;
		BOOL checked	= ListView_GetCheckState (m_ListCtrl, index);
		Update_List_Entry_Check (index, (bool)(checked != 1));

	} else if (message == WM_USER + 103) {

		//
		//	Invert the checked state of this entry
		//
		HTREEITEM tree_item	= (HTREEITEM)lParam;
		BOOL checked			= m_TreeCtrl.GetCheck (tree_item);
		Update_Tree_Entry_Check (tree_item, (bool)(checked != 1));
	}

	return CDialog::WindowProc (message, wParam, lParam);
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Tree_Entry_Check
//
/////////////////////////////////////////////////////////////////////////////
void
ParameterInheritanceDialogClass::Update_Tree_Entry_Check (HTREEITEM parent_item, bool checked)
{
	//
	//	Update this entry's check-state
	//
	TreeView_CustomSetCheckState (m_TreeCtrl, parent_item, checked);

	//
	//	Recurse through all the children
	//
	for (	HTREEITEM item = m_TreeCtrl.GetChildItem (parent_item);
			item != NULL;
			item = m_TreeCtrl.GetNextSiblingItem (item))
	{
		Update_Tree_Entry_Check (item, checked);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_List_Entry_Check
//
/////////////////////////////////////////////////////////////////////////////
void
ParameterInheritanceDialogClass::Update_List_Entry_Check (int index, bool checked)
{	
	//
	//	Is this entry part of a selection?
	//
	if (m_ListCtrl.GetItemState (index, LVIS_SELECTED) == LVIS_SELECTED) {

		//
		//	Apply the new check state to all the selected entries
		//
		int sel_index = -1;
		while ((sel_index = m_ListCtrl.GetNextItem (sel_index, LVNI_ALL | LVNI_SELECTED)) >= 0) {
			ListView_SetCheckState (m_ListCtrl, sel_index, checked);
		}
		
	} else {
		
		//
		//	Simply change the check state of this entry
		//
		ListView_SetCheckState (m_ListCtrl, index, checked);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// TreeView_CustomSetCheckState
//
/////////////////////////////////////////////////////////////////////////////
BOOL
TreeView_CustomSetCheckState (HWND hwndTreeView, HTREEITEM hItem, BOOL fCheck)
{
    TVITEM tvItem;

    tvItem.mask = TVIF_HANDLE | TVIF_STATE;
    tvItem.hItem = hItem;
    tvItem.stateMask = TVIS_STATEIMAGEMASK;

    /*
    Since state images are one-based, 1 in this macro turns the check off, and 
    2 turns it on.
    */
    tvItem.state = INDEXTOSTATEIMAGEMASK((fCheck ? 2 : 1));

    return TreeView_SetItem(hwndTreeView, &tvItem);
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
ParameterInheritanceDialogClass::OnOK (void) 
{
	DynamicVectorClass<PresetClass *>					preset_list;
	DynamicVectorClass<DefinitionParameterClass *>	parameter_list;

	//
	//	Build a list of selected parameters and presets 
	//
	Build_Parameter_List (parameter_list);
	Build_Preset_List (TVI_ROOT, preset_list);

	//
	//	Does the user want to propogate the dialogue settings?
	//
	bool propogate_dialogue_settings = false;
	if (SendDlgItemMessage (IDC_PROPOGATE_DIALOGUE, BM_GETCHECK) == 1) {
		propogate_dialogue_settings = true;
	}
	
	//
	//	Loop over all the selected presets and propagate the changes
	//
	DefinitionClass *base_def = m_Preset->Get_Definition ();
	for (int index = 0; index < preset_list.Count (); index ++) {
		PresetClass *preset				= preset_list[index];
		DefinitionClass *derived_def	= preset->Get_Definition ();

		if (base_def != NULL) {
			Propagate_Changes (base_def, derived_def, parameter_list);

			//
			//	Propogate the dialogue settings if necessary
			//
			if (	propogate_dialogue_settings &&
					base_def->Get_Class_ID () == CLASSID_GAME_OBJECT_DEF_SOLDIER &&
					derived_def->Get_Class_ID () == CLASSID_GAME_OBJECT_DEF_SOLDIER)
			{				
				SoldierGameObjDef *base_soldier		= reinterpret_cast<SoldierGameObjDef *> (base_def);
				SoldierGameObjDef *derived_soldier	= reinterpret_cast<SoldierGameObjDef *> (derived_def);

				DialogueClass *src_list		= 	base_soldier->Get_Dialog_List ();
				DialogueClass *dest_list	= 	derived_soldier->Get_Dialog_List ();

				//
				//	Copy the settings from the base to the derived...
				//
				for (int index = 0; index < DIALOG_MAX; index ++) {
					dest_list[index] = src_list[index];
				}
			}
		}
	}


	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Find_Parameter_In_List
//
/////////////////////////////////////////////////////////////////////////////
bool
Find_Parameter_In_List
(
	DynamicVectorClass<DefinitionParameterClass *> &parameter_list,
	ParameterClass *											parameter
)
{
	bool retval = false;

	//
	//	Loop over all the parameters in the list
	//
	for (int index = 0; index < parameter_list.Count (); index ++) {
		DefinitionParameterClass *def_param = parameter_list[index];
		
		//
		//	Is this the parameter we were looking for?
		//
		if (def_param != NULL && def_param->Get_Parameter () == parameter) {
			retval = true;
			break;
		}
	}
	 
	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
// Propagate_Changes
//
/////////////////////////////////////////////////////////////////////////////
void
ParameterInheritanceDialogClass::Propagate_Changes
(
	DefinitionClass *											base_def,
	DefinitionClass *											derived_def,
	DynamicVectorClass<DefinitionParameterClass *> &parameter_list
)
{
	bool is_valid = true;

	//
	//	Compare all the parameters with this derived definition to
	// find any that are not overridden.
	//
	int param_count = base_def->Get_Parameter_Count ();
	for (int param_index = 0; is_valid && param_index < param_count; param_index ++) {
		ParameterClass *parameter			= base_def->Lock_Parameter (param_index);
		ParameterClass *curr_parameter	= derived_def->Lock_Parameter (param_index);
		ASSERT (parameter != NULL);
		ASSERT (curr_parameter != NULL);
		
		//
		//	Are we comparing the same parameter? (We better be).
		//
		if (	parameter->Get_Type () == curr_parameter->Get_Type () &&
				::lstrcmpi (parameter->Get_Name (), curr_parameter->Get_Name ()) == 0)
		{
			//
			//	Do we need to recurse into this parameter?
			//
			if (parameter->Get_Type () == ParameterClass::TYPE_MODELDEFINITIONID) {
				
				//
				//	Lookup the definitions that these parameters reference
				//
				int base_id		= ((ModelDefParameterClass *)parameter)->Get_Value ();
				int derived_id = ((ModelDefParameterClass *)curr_parameter)->Get_Value ();
				DefinitionClass *base_model_def		= DefinitionMgrClass::Find_Definition (base_id, false);
				DefinitionClass *derived_model_def	= DefinitionMgrClass::Find_Definition (derived_id, false);

				//
				//	Recurse into these definitions
				//
				if (	base_model_def != NULL && derived_model_def != NULL &&
						base_model_def->Get_Class_ID () == derived_model_def->Get_Class_ID ())
				{
					Propagate_Changes (base_model_def, derived_model_def, parameter_list);
				}

			} else {
				
				//
				//	Should we propagate this parameter's value?
				//
				if (::Find_Parameter_In_List (parameter_list, parameter)) {
					
					//
					//	Copy the parameter value from the parent.
					//
					curr_parameter->Copy_Value (*parameter);
				}
			}
			
		} else {
			is_valid = false;
		}

		//
		//	Release the parameter pointers
		//
		base_def->Unlock_Parameter (param_index);
		derived_def->Unlock_Parameter (param_index);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Build_Parameter_List
//
/////////////////////////////////////////////////////////////////////////////
void
ParameterInheritanceDialogClass::Build_Parameter_List (DynamicVectorClass<DefinitionParameterClass *> &parameter_list)
{
	//
	//	Loop over all the parameters
	//
	for (int index = 0; index < m_ListCtrl.GetItemCount (); index ++) {
		DefinitionParameterClass *parameter = NULL;
		parameter = (DefinitionParameterClass *)m_ListCtrl.GetItemData (index);
		
		//
		//	Add this parameter to the list if the user checked it
		//
		if (parameter != NULL && ListView_GetCheckState (m_ListCtrl, index)) {
			parameter_list.Add (parameter);
		}		
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Build_Preset_List
//
/////////////////////////////////////////////////////////////////////////////
void
ParameterInheritanceDialogClass::Build_Preset_List
(
	HTREEITEM									parent_item,
	DynamicVectorClass<PresetClass *> &	preset_list
)
{
	//
	//	Loop over all the immediate children
	//
	for (	HTREEITEM item = m_TreeCtrl.GetChildItem (parent_item);
			item != NULL;
			item = m_TreeCtrl.GetNextSiblingItem (item))
	{
		PresetClass *preset = (PresetClass *)m_TreeCtrl.GetItemData (item);
		if (preset != NULL) {

			//
			//	Add this preset to the list if necessary
			//
			if (m_TreeCtrl.GetCheck (item)) {
				preset_list.Add (preset);
			}

			//
			//	Recurse down the tree
			//
			Build_Preset_List (item, preset_list);
		}		
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeleteitemParameterList
//
/////////////////////////////////////////////////////////////////////////////
void
ParameterInheritanceDialogClass::OnDeleteitemParameterList
(	
	NMHDR *	pNMHDR,
	LRESULT* pResult
)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	(*pResult) = 0;

	//
	//	Free the object we associted with this entry
	//
	DefinitionParameterClass *def_param = NULL;
	def_param = (DefinitionParameterClass *)m_ListCtrl.GetItemData (pNMListView->iItem);
	MEMBER_RELEASE (def_param);

	m_ListCtrl.SetItemData (pNMListView->iItem, 0);
	return ;
}

