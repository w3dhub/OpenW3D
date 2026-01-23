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

// PresetRemapDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "leveleditdoc.h"
#include "presetremapdialog.h"
#include "selectpresetdialog.h"
#include "node.h"
#include "nodemgr.h"
#include "presetmgr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


enum
{
	COL_OBJECT		= 0,
	COL_PRESET
};


/////////////////////////////////////////////////////////////////////////////
//	NODE_PRESET_INFO
/////////////////////////////////////////////////////////////////////////////
/*typedef struct _NODE_PRESET_INFO
{
	PresetClass *	preset;
	NodeClass *		node;
} NODE_PRESET_INFO;*/

/////////////////////////////////////////////////////////////////////////////
//
// PresetRemapDialogClass
//
/////////////////////////////////////////////////////////////////////////////
PresetRemapDialogClass::PresetRemapDialogClass (CWnd* pParent /*=NULL*/)
	: CDialog(PresetRemapDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(PresetRemapDialogClass)
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
PresetRemapDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PresetRemapDialogClass)
	DDX_Control(pDX, IDC_LIST_CTRL, m_ListCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(PresetRemapDialogClass, CDialog)
	//{{AFX_MSG_MAP(PresetRemapDialogClass)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_CTRL, OnDblclkListCtrl)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LIST_CTRL, OnDeleteitemListCtrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
PresetRemapDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	//
	//	Configure the columns
	//
	m_ListCtrl.InsertColumn (COL_OBJECT, "Object");
	m_ListCtrl.InsertColumn (COL_PRESET, "Preset");
	m_ListCtrl.SetExtendedStyle (m_ListCtrl.GetExtendedStyle () | LVS_EX_FULLROWSELECT);
	
	//
	//	Choose an appropriate size for the columns
	//
	CRect rect;
	m_ListCtrl.GetClientRect (&rect);
	rect.right -= ::GetSystemMetrics (SM_CXVSCROLL) + 2;
	m_ListCtrl.SetColumnWidth (COL_OBJECT, rect.Width () / 2);
	m_ListCtrl.SetColumnWidth (COL_PRESET, rect.Width () / 2);

	if (CLevelEditDoc::Is_Batch_Export_Mode ()) {

		//
		//	Simply log the errors if we're in batch export mode
		//
		for (int index = 0; index < NodeList.Count (); index ++) {
			NodeClass *node = NodeList[index];
			if (node != NULL) {
				StringClass name = node->Get_Name ();
				WWDEBUG_SAY (("Unable to find preset for node: %s\n", (const char *)name));
			}
		}

		//
		//	Close the dialog
		//
		EndDialog (IDOK);

	} else {
		
		//
		//	Add the node's into the list control
		//
		for (int index = 0; index < NodeList.Count (); index ++) {
			NodeClass *node = NodeList[index];
			if (node != NULL) {
				StringClass name = node->Get_Name ();
				
				//
				//	Add this object to the list control
				//
				int item_index = m_ListCtrl.InsertItem (0xFF, name);
				if (item_index >= 0) {
					m_ListCtrl.SetItemData (item_index, (DWORD_PTR)node);

					//
					//	Take a best guess at a preset name for the object
					//
					char *separator = const_cast<char *>(::strchr (name, '.'));
					if (separator != NULL) {
						separator[0] = 0;
						const char *candidate_preset_name = name;
						PresetClass *candidate_preset = PresetMgrClass::Find_Preset (candidate_preset_name);
						if (candidate_preset != NULL) {

							//
							//	Associate the preset with the object
							//
							node->Set_Preset (candidate_preset);
							m_ListCtrl.SetItemText (item_index, COL_PRESET, candidate_preset->Get_Name ());
						}
					}
				}
			}
		}
	}
	
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDblclkListCtrl
//
/////////////////////////////////////////////////////////////////////////////
void
PresetRemapDialogClass::OnDblclkListCtrl
(
	NMHDR *		/* pNMHDR */,
	LRESULT *	pResult
)	 
{
	(*pResult) = 0;

	//
	//	Get the selected item
	//
	int item_index = m_ListCtrl.GetNextItem (-1, LVNI_SELECTED | LVNI_ALL);
	if (item_index >= 0) {

		//
		//	Get the node from this item
		//
		NodeClass *node = (NodeClass *)m_ListCtrl.GetItemData (item_index);
		if (node != NULL) {

			//
			//	Show a dialog to the user that will enable them to
			// select a new preset
			//
			SelectPresetDialogClass dialog (this);
			dialog.Set_Preset (node->Get_Preset ());
			if (dialog.DoModal () == IDOK) {
				
				//
				//	Get the new preset from the dialog
				//
				PresetClass *preset = dialog.Get_Selection ();
				CString preset_name = preset->Get_Name ();
			

				//
				//	Ask the user if they want to make this change to all the nodes
				//
				if (MessageBox ("Would you like to propagate this preset to all other nodes of the same original preset?", "Propagate", MB_YESNO | MB_ICONQUESTION)) {

					uint32 preset_id_to_change = node->Get_Preset_ID ();
					int count = m_ListCtrl.GetItemCount ();
					
					//
					//	Loop over all the nodes in the list
					//
					for (int index = 0; index < count; index ++) {
						NodeClass *curr_node = (NodeClass *)m_ListCtrl.GetItemData (index);
						if (curr_node != NULL && curr_node->Get_Preset_ID () == preset_id_to_change) {
							
							//
							//	Associate the preset with the object
							//
							curr_node->Set_Preset (preset);
							m_ListCtrl.SetItemText (index, COL_PRESET, preset_name);
						}
					}

				} else {

					//
					//	Associate the preset with the object
					//
					node->Set_Preset (preset);					
					m_ListCtrl.SetItemText (item_index, COL_PRESET, preset_name);
				}
			}
		}
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeleteitemListCtrl
//
/////////////////////////////////////////////////////////////////////////////
void
PresetRemapDialogClass::OnDeleteitemListCtrl
(
	NMHDR *	pNMHDR,
	LRESULT* pResult
)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;	
	(*pResult) = 0;

	//
	//	Free our hold on this node
	//
	NodeClass *node = (NodeClass *)m_ListCtrl.GetItemData (pNMListView->iItem);
	REF_PTR_RELEASE (node);
	m_ListCtrl.SetItemData (pNMListView->iItem, 0);

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
PresetRemapDialogClass::OnOK (void)
{
	CWaitCursor wait_cursor;	
	
	//
	//	Loop over all the nodes in the list
	//
	int count = m_ListCtrl.GetItemCount ();
	for (int index = 0; index < count; index ++) {
		NodeClass *curr_node = (NodeClass *)m_ListCtrl.GetItemData (index);
		if (curr_node->Get_Preset () != NULL) {
			
			//
			//	Add this node to the world
			//
			curr_node->Initialize ();
			NodeMgrClass::Add_Node (curr_node);
		}
	}

	CDialog::OnOK ();
	return ;
}
