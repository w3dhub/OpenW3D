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

// EditDialogueDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "editdialoguedialog.h"
#include "dialogue.h"
#include "translatedb.h"
#include "translateobj.h"
#include "Utils.h"
#include "conversationpickerdialog.h"
#include "conversationmgr.h"
#include "conversation.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//	Constants
/////////////////////////////////////////////////////////////////////////////
enum
{
	COL_WEIGHT	= 0,	
	COL_TEXT
};


/////////////////////////////////////////////////////////////////////////////
//
// EditDialogueDialogClass
//
/////////////////////////////////////////////////////////////////////////////
EditDialogueDialogClass::EditDialogueDialogClass(CWnd* pParent /*=NULL*/)
	:	m_Dialogue (NULL),
		CDialog(EditDialogueDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(EditDialogueDialogClass)
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
EditDialogueDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EditDialogueDialogClass)
	DDX_Control(pDX, IDC_SELECTED_WEIGHT_SPIN, m_SelectedWeightSpin);
	DDX_Control(pDX, IDC_SILENCE_WEIGHT_SPIN, m_SilenceWeightSpin);
	DDX_Control(pDX, IDC_REMARK_LIST, m_ListCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(EditDialogueDialogClass, CDialog)
	//{{AFX_MSG_MAP(EditDialogueDialogClass)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_NOTIFY(NM_DBLCLK, IDC_REMARK_LIST, OnDblclkOptionList)
	ON_NOTIFY(LVN_DELETEITEM, IDC_REMARK_LIST, OnDeleteitemOptionList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_REMARK_LIST, OnItemchangedRemarkList)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SELECTED_WEIGHT_SPIN, OnDeltaposSelectedWeightSpin)
	ON_EN_KILLFOCUS(IDC_SELECTED_WEIGHT_EDIT, OnKillfocusSelectedWeightEdit)
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
EditDialogueDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog();

	ASSERT (m_Dialogue != NULL);

	//
	//	Configure the list control
	//
	m_ListCtrl.InsertColumn (COL_WEIGHT, "Weight");
	m_ListCtrl.InsertColumn (COL_TEXT, "Text");
	m_ListCtrl.SetExtendedStyle (m_ListCtrl.GetExtendedStyle () | LVS_EX_FULLROWSELECT);

	//
	//	Choose a size for the list control's columns
	//
	CRect rect;
	m_ListCtrl.GetClientRect (&rect);
	rect.right -= ::GetSystemMetrics (SM_CXVSCROLL);
	m_ListCtrl.SetColumnWidth (COL_WEIGHT,	rect.Width () / 6);
	m_ListCtrl.SetColumnWidth (COL_TEXT,	(rect.Width () * 5) / 6);

	//
	//	Insert all of the options into the list control
	//
	DIALOGUE_OPTION_LIST	&option_list = m_Dialogue->Get_Option_List ();
	for (int index = 0; index < option_list.Count (); index ++) {
		
		//
		//	Make a copy of the option and add it to our list
		//
		DialogueOptionClass *option = new DialogueOptionClass (*(option_list[index]));
		Insert_Entry (option);	
	}
	
	//
	//	Configure the silence controls
	//
	m_SilenceWeightSpin.SetRange (0, 100);
	m_SilenceWeightSpin.SetPos ((int)m_Dialogue->Get_Silence_Weight ());
	SetDlgItemInt (IDC_SILENCE_WEIGHT_EDIT, (int)m_Dialogue->Get_Silence_Weight ());

	//
	//	Configure the other weight controls
	//
	m_SelectedWeightSpin.SetRange (0, 10000);
	m_SelectedWeightSpin.SetPos (0);

	//
	//	Update the enabled state of some of the controls on the dialog
	//
	Update_Enabled_State ();
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
EditDialogueDialogClass::OnOK (void) 
{
	//
	//	Read the silence configuration
	//
	int new_weight = GetDlgItemInt (IDC_SILENCE_WEIGHT_EDIT);
	m_Dialogue->Set_Silence_Weight (new_weight);

	//
	//	Start with a fresh list of options
	//
	m_Dialogue->Free_Options ();
	DIALOGUE_OPTION_LIST	&option_list = m_Dialogue->Get_Option_List ();

	//
	//	Read all the options from the list control
	//
	int count = m_ListCtrl.GetItemCount ();
	for (int index = 0; index < count; index ++) {

		//
		//	Add this option to the dialogue's list
		//
		DialogueOptionClass *option = (DialogueOptionClass *)m_ListCtrl.GetItemData (index);
		if (option != NULL) {
			option_list.Add (option);
			m_ListCtrl.SetItemData (index, NULL);
		}
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
EditDialogueDialogClass::OnAdd (void) 
{
	//
	//	Display a dialog that allows the user to edit this option
	//
	ConversationPickerDialogClass dialog (this);	
	if (dialog.DoModal () == IDOK) {
		
		//
		//	Create a new dialog option
		//
		DialogueOptionClass *option = new DialogueOptionClass;
		option->Set_Weight (1);
		option->Set_Conversation_ID (dialog.Get_Conversation ()->Get_ID ());

		//
		//	Add the new option to the list
		//
		Insert_Entry (option);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDelete
//
/////////////////////////////////////////////////////////////////////////////
void
EditDialogueDialogClass::OnDelete (void)
{
	//
	//	Remove the currently selected item from the list control
	//
	int index = m_ListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (index >= 0) {
		m_ListCtrl.DeleteItem (index);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDblclkOptionList
//
/////////////////////////////////////////////////////////////////////////////
void
EditDialogueDialogClass::OnDblclkOptionList
(
	NMHDR *		/* pNMHDR */,
	LRESULT *	/* pResult */
)
{
	//
	//	Get the currently selected option
	//
	int index = m_ListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (index >= 0) {

		DialogueOptionClass *option = (DialogueOptionClass *)m_ListCtrl.GetItemData (index);
		if (option != NULL) {

			//
			//	Display a dialog that allows the user to edit this option
			//
			ConversationPickerDialogClass dialog (this);
			
			ConversationClass *conversation = NULL;
			conversation = ConversationMgrClass::Find_Conversation (option->Get_Conversation_ID ());
			dialog.Set_Conversation (conversation);

			if (dialog.DoModal () == IDOK) {				

				//
				//	Update the entry in the list
				//
				option->Set_Conversation_ID (dialog.Get_Conversation ()->Get_ID ());
				Update_Entry (index);
			}

			REF_PTR_RELEASE (conversation);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeleteitemOptionList
//
/////////////////////////////////////////////////////////////////////////////
void
EditDialogueDialogClass::OnDeleteitemOptionList
(
	NMHDR *		pNMHDR,
	LRESULT *	pResult
)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	(*pResult) = 0;

	//
	//	Free the associated DialogueOptionClass object
	//
	DialogueOptionClass *option = (DialogueOptionClass *)m_ListCtrl.GetItemData (pNMListView->iItem);
	if (option != NULL) {
		SAFE_DELETE (option);
		m_ListCtrl.SetItemData (pNMListView->iItem, NULL);
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Entry
//
/////////////////////////////////////////////////////////////////////////////
void
EditDialogueDialogClass::Update_Entry (int index)
{
	DialogueOptionClass *option = (DialogueOptionClass *)m_ListCtrl.GetItemData (index);
	if (option != NULL) {

		//
		//	Update the weight column of the list control
		//
		CString weight_text;
		weight_text.Format ("%d", (int)option->Get_Weight ());
		m_ListCtrl.SetItemText (index, COL_WEIGHT, weight_text);

		//
		//	Lookup the string entry in our translation database
		//
		int conversation_id = option->Get_Conversation_ID ();
		ConversationClass *conversation = ConversationMgrClass::Find_Conversation (conversation_id);
		if (conversation != NULL) {					
			
			//
			//	Put this text into the appropriate column in the list control
			//		
			m_ListCtrl.SetItemText (index, COL_TEXT, conversation->Get_Name ());
			REF_PTR_RELEASE (conversation);
		}
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Insert_Entry
//
/////////////////////////////////////////////////////////////////////////////
void
EditDialogueDialogClass::Insert_Entry (DialogueOptionClass *option)
{
	ASSERT (option != NULL);
	if (option != NULL) {

		CString weight_text;
		weight_text.Format ("%d", (int)option->Get_Weight ());

		//
		//	Add this option to the list control
		//
		int item_index = m_ListCtrl.InsertItem (m_ListCtrl.GetItemCount (), weight_text);
		if (item_index >= 0) {

			//
			//	Associate the object with the entry in the list control
			//
			m_ListCtrl.SetItemData (item_index, (DWORD_PTR)option);
						
			//
			//	Lookup the string entry in our translation database
			//
			int conversation_id = option->Get_Conversation_ID ();
			ConversationClass *conversation = ConversationMgrClass::Find_Conversation (conversation_id);
			if (conversation != NULL) {					
				
				//
				//	Put this text into the appropriate column in the list control
				//
				m_ListCtrl.SetItemText (item_index, COL_TEXT, conversation->Get_Name ());
				REF_PTR_RELEASE (conversation);
			}
		}		
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnItemchangedRemarkList
//
/////////////////////////////////////////////////////////////////////////////
void
EditDialogueDialogClass::OnItemchangedRemarkList
(
	NMHDR *		pNMHDR,
	LRESULT *	pResult
)
{
	NM_LISTVIEW *pNMListView = (NM_LISTVIEW *)pNMHDR;
	(*pResult) = 0;

	if (pNMListView->uChanged & LVIF_STATE) {
		Update_Enabled_State ();

		//
		//	Is there a currently selected item?
		//
		int item_index = m_ListCtrl.GetNextItem (-1, LVNI_SELECTED | LVNI_ALL);	
		if (item_index >= 0) {

			//
			//	Update the weight of the selected item
			//
			DialogueOptionClass *option = (DialogueOptionClass *)m_ListCtrl.GetItemData (item_index);
			if (option != NULL) {
				SetDlgItemInt (IDC_SELECTED_WEIGHT_EDIT, (int)option->Get_Weight ());
			}
		}
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Enabled_State
//
/////////////////////////////////////////////////////////////////////////////
void
EditDialogueDialogClass::Update_Enabled_State (void)
{
	int item_index = m_ListCtrl.GetNextItem (-1, LVNI_SELECTED | LVNI_ALL);
	bool enabled	= (item_index >= 0);

	::EnableWindow (::GetDlgItem (m_hWnd, IDC_DELETE), enabled);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_SELECTED_WEIGHT_EDIT), enabled);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_SELECTED_WEIGHT_SPIN), enabled);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeltaposSelectedWeightSpin
//
/////////////////////////////////////////////////////////////////////////////
void
EditDialogueDialogClass::OnDeltaposSelectedWeightSpin
(
	NMHDR * /* pNMHDR */,
	LRESULT *pResult
) 
{
	(*pResult) = 0;

	//Update_Current_Weight ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnKillfocusSelectedWeightEdit
//
/////////////////////////////////////////////////////////////////////////////
void
EditDialogueDialogClass::OnKillfocusSelectedWeightEdit (void)
{
	Update_Current_Weight ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Current_Weight
//
/////////////////////////////////////////////////////////////////////////////
void
EditDialogueDialogClass::Update_Current_Weight (void)
{
	//
	//	Is there a currently selected item?
	//
	int item_index = m_ListCtrl.GetNextItem (-1, LVNI_SELECTED | LVNI_ALL);	
	if (item_index >= 0) {

		DialogueOptionClass *option = (DialogueOptionClass *)m_ListCtrl.GetItemData (item_index);
		if (option != NULL) {
		
			//
			//	Get the weight
			//
			int weight = GetDlgItemInt (IDC_SELECTED_WEIGHT_EDIT);

			//
			//	Update the weight
			//
			option->Set_Weight ((float)weight);
			Update_Entry (item_index);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnVScroll
//
/////////////////////////////////////////////////////////////////////////////
void
EditDialogueDialogClass::OnVScroll
(
	UINT			/* nSBCode */,
	UINT			/* nPos */,
	CScrollBar * /* pScrollBar */
)
{
	Update_Current_Weight ();
	return ;
}
