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

// EditConversationDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "editconversationdialog.h"
#include "conversation.h"
#include "translatedb.h"
#include "translateobj.h"
#include "editconversationremarkdialog.h"
#include "playertype.h"
#include "orator.h"
#include "oratortypes.h"
#include "Utils.h"


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
	COL_ORATOR		= 0,
	COL_REMARK
};

static const int	ORATOR_MAX	= 6;

typedef struct
{
	int	button_id;
	int	combo_id;
	int	check_id;
	int	icon_id;
} ORATOR_UI_INFO;

const ORATOR_UI_INFO ORATOR_CTRLS[ORATOR_MAX] =
{
	{ IDC_ORATOR1_CHECK, IDC_ORATOR1_COMBO, IDC_ORATOR1_VISBLE_CHECK, IDI_ORATOR1 },
	{ IDC_ORATOR2_CHECK, IDC_ORATOR2_COMBO, IDC_ORATOR2_VISBLE_CHECK, IDI_ORATOR2 },
	{ IDC_ORATOR3_CHECK, IDC_ORATOR3_COMBO, IDC_ORATOR3_VISBLE_CHECK, IDI_ORATOR3 },
	{ IDC_ORATOR4_CHECK, IDC_ORATOR4_COMBO, IDC_ORATOR4_VISBLE_CHECK, IDI_ORATOR4 },
	{ IDC_ORATOR5_CHECK, IDC_ORATOR5_COMBO, IDC_ORATOR5_VISBLE_CHECK, IDI_ORATOR5 },
	{ IDC_ORATOR6_CHECK, IDC_ORATOR6_COMBO, IDC_ORATOR6_VISBLE_CHECK, IDI_ORATOR6 },
};


/////////////////////////////////////////////////////////////////////////////
//
// EditConversationDialogClass
//
/////////////////////////////////////////////////////////////////////////////
EditConversationDialogClass::EditConversationDialogClass(CWnd* pParent /*=NULL*/)
	:	m_Conversation (NULL),
		CDialog(EditConversationDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(EditConversationDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~EditConversationDialogClass
//
/////////////////////////////////////////////////////////////////////////////
EditConversationDialogClass::~EditConversationDialogClass (void)
{
	REF_PTR_RELEASE (m_Conversation);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
EditConversationDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EditConversationDialogClass)
	DDX_Control(pDX, IDC_REMARK_LIST, m_ListCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(EditConversationDialogClass, CDialog)
	//{{AFX_MSG_MAP(EditConversationDialogClass)
	ON_NOTIFY(NM_DBLCLK, IDC_REMARK_LIST, OnDblclkRemarkList)
	ON_NOTIFY(LVN_KEYDOWN, IDC_REMARK_LIST, OnKeydownRemarkList)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_INSERT, OnInsert)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_REMARK_LIST, OnItemchangedRemarkList)
	ON_NOTIFY(LVN_DELETEITEM, IDC_REMARK_LIST, OnDeleteitemRemarkList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
EditConversationDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	//
	//	Create a new conversation if we don't already have one to edit
	//
	if (m_Conversation == NULL) {
		m_Conversation = new ConversationClass;
		
		OratorClass orator;
		m_Conversation->Add_Orator (orator);
	}

	//
	//	Configure the state combo box
	//
	SendDlgItemMessage (IDC_STATE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Idle");
	SendDlgItemMessage (IDC_STATE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Idle (Secondary)");
	SendDlgItemMessage (IDC_STATE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Search");
	SendDlgItemMessage (IDC_STATE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Combat");
	SendDlgItemMessage (IDC_STATE_COMBO, CB_SETCURSEL, (WPARAM)m_Conversation->Get_AI_State ());

	//
	//	Configure the orator controls
	//
	int index;
	for (index = 0; index < ORATOR_MAX; index ++) {

		//
		//	Put the icon into the button control
		//
		HICON icon = ::LoadIcon (::AfxGetResourceHandle (), MAKEINTRESOURCE (ORATOR_CTRLS[index].icon_id));
		SendDlgItemMessage (ORATOR_CTRLS[index].button_id, BM_SETIMAGE, IMAGE_ICON, (LPARAM)icon);

		//
		//	Put the "visible" icon into the checkbox control
		//
		icon = (HICON)::LoadImage (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_EYE2), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		SendDlgItemMessage (ORATOR_CTRLS[index].check_id, BM_SETIMAGE, IMAGE_ICON, (LPARAM)icon);
		SendDlgItemMessage (ORATOR_CTRLS[index].check_id, BM_SETCHECK, (WPARAM)true);

		//
		//	Configure the orator type combo-box
		//
		int type_count = OratorTypeClass::Get_Count ();
		for (int type_index = 0; type_index < type_count; type_index ++) {
			
			//
			//	Lookup information about this orator type
			//
			const char *type_name	= OratorTypeClass::Get_Description (type_index);
			int type_id					= OratorTypeClass::Get_ID (type_index);
			int combobox_id			= ORATOR_CTRLS[index].combo_id;

			//
			//	Add this orator type to the combobox
			//
			int item_index = SendDlgItemMessage (combobox_id, CB_ADDSTRING, 0, (LPARAM)type_name);
			SendDlgItemMessage (combobox_id, CB_SETITEMDATA, (WPARAM)item_index, (LPARAM)type_id);			
		}
	
		//
		//	Select the first item by default
		//
		SendDlgItemMessage (ORATOR_CTRLS[index].combo_id, CB_SETCURSEL, (WPARAM)0);
	}

	//
	//	Configure the orator controls
	//
	bool is_locked		= true;
	int orator_count	= m_Conversation->Get_Orator_Count ();
	for (index = 0; index < orator_count; index ++) {
		OratorClass *orator = m_Conversation->Get_Orator (index);

		//
		//	Check the checkbox control and enable the combobox control
		//
		SendDlgItemMessage (ORATOR_CTRLS[index].button_id, BM_SETCHECK, (WPARAM)true);
		SendDlgItemMessage (ORATOR_CTRLS[index].check_id, BM_SETCHECK, (WPARAM)(!orator->Is_Invisible ()));
		::EnableWindow (::GetDlgItem (m_hWnd, ORATOR_CTRLS[index].combo_id), true);
		::EnableWindow (::GetDlgItem (m_hWnd, ORATOR_CTRLS[index].check_id), true);		
		
		//
		//	Select the current orator type in the combobox
		//
		int orator_type = orator->Get_Orator_Type ();
		int item_index = Find_Combobox_Entry (orator_type);
		if (item_index >= 0) {
			SendDlgItemMessage (ORATOR_CTRLS[index].combo_id, CB_SETCURSEL, (WPARAM)item_index);
		}

		//
		//	If any of the orator types disagree, then don't lock the controls
		//
		if (orator_type != m_Conversation->Get_Orator (0)->Get_Orator_Type ()) {
			is_locked = false;
		}
	}

	//
	//	Configure the "lock" checkbox
	//
	HICON lock_icon = (HICON)::LoadImage (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_LOCK), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	SendDlgItemMessage (IDC_LOCKED_CHECK, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)lock_icon);
	SendDlgItemMessage (IDC_LOCKED_CHECK, BM_SETCHECK, (WPARAM)is_locked);

	//
	//	Set the check state of the "Is Innate" and "Is Key" checkboxes
	//
	SendDlgItemMessage (IDC_INNATE_CHECK, BM_SETCHECK, (WPARAM)m_Conversation->Is_Innate ());	
	SendDlgItemMessage (IDC_KEY_CONVESATION_CHECK, BM_SETCHECK, (WPARAM)m_Conversation->Is_Key ());	

	//
	//	Configure the columns
	//
	m_ListCtrl.InsertColumn (COL_ORATOR, "Orator");
	m_ListCtrl.InsertColumn (COL_REMARK, "Remark");
	m_ListCtrl.SetExtendedStyle (m_ListCtrl.GetExtendedStyle () | LVS_EX_FULLROWSELECT);
	
	//
	//	Choose an appropriate size for the columns
	//
	CRect rect;
	m_ListCtrl.GetClientRect (&rect);
	rect.right -= ::GetSystemMetrics (SM_CXVSCROLL) + 2;
	m_ListCtrl.SetColumnWidth (COL_ORATOR, rect.Width () / 4);
	m_ListCtrl.SetColumnWidth (COL_REMARK, (rect.Width () * 3) / 4);

	//
	//	Fill in the conversation's name into the appropriate control
	//
	SetDlgItemText (IDC_CONVERSATION_NAME, m_Conversation->Get_Name ());	

	//
	//	Add all the remark's to the list control
	//
	int count = m_Conversation->Get_Remark_Count ();
	for (index = 0; index < count; index ++) {
		
		ConversationRemarkClass remark;
		if (m_Conversation->Get_Remark_Info (index, remark)) {
			Add_Entry (remark);
		}
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDblclkRemarkList
//
/////////////////////////////////////////////////////////////////////////////
void
EditConversationDialogClass::OnDblclkRemarkList
(
	NMHDR *	/* pNMHDR */,
	LRESULT* pResult
)
{
	(*pResult) = 0;

	//
	//	Determine which entry the user double-clicked on
	//
	int sel_index = m_ListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (sel_index != -1) {

		//
		//	Lookup the information about this entry
		//
		ConversationRemarkClass *remark = Get_Entry_Data (sel_index);

		//
		//	Make a bitmask of the orators that are configured
		//
		int orator_bitmask = 0;
		for (int index = 0; index < ORATOR_MAX; index ++) {
			if (SendDlgItemMessage (ORATOR_CTRLS[index].button_id, BM_GETCHECK) == 1) {
				orator_bitmask |= (1 << index);
			}
		}
			
		//
		//	Show a dialog to the user that will let them choose an orator and
		// a remark
		//
		EditConversationRemarkDialogClass dialog (this);
		dialog.Set_Remark (*remark);
		dialog.Set_Orator_Bitmask (orator_bitmask);
		if (dialog.DoModal () == IDOK) {

			//
			//	Get the new values from the dialog
			//
			(*remark) = dialog.Get_Remark ();

			//
			//	Update the orator id in the list control
			//
			CString orator_text;
			orator_text.Format ("%d", remark->Get_Orator_ID () + 1);
			m_ListCtrl.SetItemText (sel_index, COL_ORATOR, orator_text);
			
			//
			//	Update the text entry in the list control
			//
			TDBObjClass *translate_obj = TranslateDBClass::Find_Object (remark->Get_Text_ID ());
			if (translate_obj != NULL) {
				m_ListCtrl.SetItemText (sel_index, COL_REMARK, translate_obj->Get_English_String ());
			}
		}
	}
		
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnKeydownRemarkList
//
/////////////////////////////////////////////////////////////////////////////
void
EditConversationDialogClass::OnKeydownRemarkList
(
	NMHDR *		pNMHDR,
	LRESULT *	pResult
)
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;
	(*pResult) = 0;

	if (pLVKeyDow->wVKey == VK_DELETE) {
		
		//
		//	Delete all the selected items
		//
		int index = -1;
		while ((index = m_ListCtrl.GetNextItem (-1, LVNI_SELECTED | LVNI_ALL)) >= 0) {
			m_ListCtrl.DeleteItem (index);
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
EditConversationDialogClass::OnOK (void)
{
	CDialog::OnOK ();

	//
	//	Read the state this conversation is active for
	//
	int ai_state = SendDlgItemMessage (IDC_STATE_COMBO, CB_GETCURSEL);
	if (ai_state != CB_ERR) {
		m_Conversation->Set_AI_State (SoldierAIState(ai_state));
	}

	//
	//	Pass the name onto the conversation
	//
	CString name;
	GetDlgItemText (IDC_CONVERSATION_NAME, name);
	m_Conversation->Set_Name (name);

	//
	//	Start fresh
	//
	m_Conversation->Clear_Remarks ();
	m_Conversation->Clear_Orators ();

	//
	//	Get all the conversations from the list control and add them to the
	// conversation manager
	//
	int index;
	for (index = 0; index < m_ListCtrl.GetItemCount (); index ++) {
		ConversationRemarkClass *remark = Get_Entry_Data (index);		
		m_Conversation->Add_Remark (*remark);
	}

	//
	//	Build a list of orators for the conversation
	//	
	for (index = 0; index < ORATOR_MAX; index ++) {

		//
		//	Is this orator configured?
		//
		if (SendDlgItemMessage (ORATOR_CTRLS[index].button_id, BM_GETCHECK) == 1) {
			bool is_invisible = (SendDlgItemMessage (ORATOR_CTRLS[index].check_id, BM_GETCHECK) != 1);

			//
			//	Add the orator to the conversation
			//
			OratorClass orator;
			orator.Set_ID (index);
			orator.Set_Orator_Type (Get_Orator_Type (index));
			orator.Set_Is_Invisible (is_invisible);
			m_Conversation->Add_Orator (orator);
		}
	}

	//
	//	Get the ID of the object to look at (if any)
	//
	int obj_id = GetDlgItemInt (IDC_OBJECT_ID);
	m_Conversation->Set_Look_At_Obj_ID (obj_id);

	//
	//	Read the users "innate" selection
	//
	bool is_innate = bool(SendDlgItemMessage (IDC_INNATE_CHECK, BM_GETCHECK) == 1);
	m_Conversation->Set_Is_Innate (is_innate);

	bool is_key = bool(SendDlgItemMessage (IDC_KEY_CONVESATION_CHECK, BM_GETCHECK) == 1);
	m_Conversation->Set_Is_Key (is_key);	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnAdd
//
/////////////////////////////////////////////////////////////////////////////
void
EditConversationDialogClass::OnAdd (void)
{
	//
	//	Make a bitmask of the orators that are configured
	//
	int orator_bitmask = 0;
	for (int index = 0; index < ORATOR_MAX; index ++) {
		if (SendDlgItemMessage (ORATOR_CTRLS[index].button_id, BM_GETCHECK) == 1) {
			orator_bitmask |= (1 << index);
		}
	}

	EditConversationRemarkDialogClass dialog (this);
	dialog.Set_Orator_Bitmask (orator_bitmask);
	if (dialog.DoModal () == IDOK) {

		//
		//	Insert a new entry into the list control
		//
		Add_Entry (dialog.Get_Remark ());
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Set_Conversation
//
/////////////////////////////////////////////////////////////////////////////
void
EditConversationDialogClass::Set_Conversation (ConversationClass *conversation)
{
	REF_PTR_SET (m_Conversation, conversation);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Entry_Data
//
/////////////////////////////////////////////////////////////////////////////
ConversationRemarkClass *
EditConversationDialogClass::Get_Entry_Data (int index)
{
	return (ConversationRemarkClass *)m_ListCtrl.GetItemData (index);
}


/////////////////////////////////////////////////////////////////////////////
//
// Add_Entry
//
/////////////////////////////////////////////////////////////////////////////
void
EditConversationDialogClass::Add_Entry (const ConversationRemarkClass &remark, int insert_index)
{
	//
	//	Insert this item into the list control
	//
	CString orator_text;
	orator_text.Format ("%d", remark.Get_Orator_ID () + 1);
	int item_index = m_ListCtrl.InsertItem (insert_index, orator_text);
	if (item_index >= 0) {
		
		//
		//	Lookup the text to display for this remark
		//
		TDBObjClass *translate_obj = TranslateDBClass::Find_Object (remark.Get_Text_ID ());
		if (translate_obj != NULL) {
			m_ListCtrl.SetItemText (item_index, COL_REMARK, translate_obj->Get_English_String ());

			//
			//	Allocate a remark object to associate with this entry in the list control
			//
			ConversationRemarkClass *associated_remark = new ConversationRemarkClass (remark);
			m_ListCtrl.SetItemData (item_index, (DWORD_PTR)associated_remark);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnInsert
//
/////////////////////////////////////////////////////////////////////////////
void
EditConversationDialogClass::OnInsert (void)
{
	int sel_index = m_ListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (sel_index != -1) {

		//
		//	Make a bitmask of the orators that are configured
		//
		int orator_bitmask = 0;
		for (int index = 0; index < ORATOR_MAX; index ++) {
			if (SendDlgItemMessage (ORATOR_CTRLS[index].button_id, BM_GETCHECK) == 1) {
				orator_bitmask |= (1 << index);
			}
		}

		EditConversationRemarkDialogClass dialog (this);
		dialog.Set_Orator_Bitmask (orator_bitmask);
		if (dialog.DoModal () == IDOK) {

			//
			//	Insert the new entry into the appropriate place in the list
			//
			Add_Entry (dialog.Get_Remark (), sel_index);
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
EditConversationDialogClass::OnItemchangedRemarkList
(
	NMHDR *		/* pNMHDR */,
	LRESULT *	pResult
)
{
	(*pResult) = 0;

	Update_Button_States ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Button_States
//
/////////////////////////////////////////////////////////////////////////////
void
EditConversationDialogClass::Update_Button_States (void)
{
	int sel_index = m_ListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_INSERT), static_cast<bool>(sel_index != -1));	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL
EditConversationDialogClass::OnCommand
(
	WPARAM wParam,
	LPARAM lParam
)
{
	
	switch (LOWORD (wParam)) {
		
		case IDC_ORATOR1_CHECK:
		case IDC_ORATOR2_CHECK:
		case IDC_ORATOR3_CHECK:
		case IDC_ORATOR4_CHECK:
		case IDC_ORATOR5_CHECK:
		case IDC_ORATOR6_CHECK:
		{
			Update_Enable_State (LOWORD (wParam) - IDC_ORATOR1_CHECK);
			Update_Remarks (LOWORD (wParam) - IDC_ORATOR1_CHECK);
		}
		break;

		case IDC_ORATOR1_COMBO:
		case IDC_ORATOR2_COMBO:
		case IDC_ORATOR3_COMBO:
		case IDC_ORATOR4_COMBO:
		case IDC_ORATOR5_COMBO:
		case IDC_ORATOR6_COMBO:
		{
			if (HIWORD (wParam) == CBN_SELCHANGE) {
				Update_Player_Type_Combos (LOWORD (wParam) - IDC_ORATOR1_COMBO);
			}
		}
		break;
	}

	return CDialog::OnCommand (wParam, lParam);
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Remarks
//
/////////////////////////////////////////////////////////////////////////////
void
EditConversationDialogClass::Update_Remarks (int orator_index)
{
	BOOL is_checked = (SendDlgItemMessage (ORATOR_CTRLS[orator_index].button_id, BM_GETCHECK) == 1);
	if (is_checked == false) {
		
		//
		//	Remove any remarks made by the specified orator
		//
		int item_count = m_ListCtrl.GetItemCount ();
		for (int index = 0; index < item_count; index ++) {
			ConversationRemarkClass *remark = Get_Entry_Data (index);
			
			//
			//	Remove this entry if it was made by the orator in question
			//
			if (remark != NULL && remark->Get_Orator_ID () == orator_index) {
				m_ListCtrl.DeleteItem (index);
				index --;
				item_count --;
			}
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Enable_State
//
/////////////////////////////////////////////////////////////////////////////
void
EditConversationDialogClass::Update_Enable_State (int orator_index)
{
	BOOL is_checked = (SendDlgItemMessage (ORATOR_CTRLS[orator_index].button_id, BM_GETCHECK) == 1);
	::EnableWindow (::GetDlgItem (m_hWnd, ORATOR_CTRLS[orator_index].combo_id), is_checked);
	::EnableWindow (::GetDlgItem (m_hWnd, ORATOR_CTRLS[orator_index].check_id), is_checked);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Player_Type_Combos
//
/////////////////////////////////////////////////////////////////////////////
void
EditConversationDialogClass::Update_Player_Type_Combos (int orator_index)
{
	BOOL is_locked = (SendDlgItemMessage (IDC_LOCKED_CHECK, BM_GETCHECK) == 1);

	if (is_locked) {

		//
		//	Get the current selection
		//
		int curr_sel = SendDlgItemMessage (ORATOR_CTRLS[orator_index].combo_id, CB_GETCURSEL);

		//
		//	Update all the combo boxes to reflect the current selection
		//
		for (int index = 0; index < ORATOR_MAX; index ++) {
			SendDlgItemMessage (ORATOR_CTRLS[index].combo_id, CB_SETCURSEL, (WPARAM)curr_sel);
		}
	}

	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Orator_Type
//
/////////////////////////////////////////////////////////////////////////////
int
EditConversationDialogClass::Get_Orator_Type (int orator_index)
{
	int retval = -1;

	//
	//	Get the current selection
	//
	int curr_sel = SendDlgItemMessage (ORATOR_CTRLS[orator_index].combo_id, CB_GETCURSEL);
	if (curr_sel >= 0) {
		retval = SendDlgItemMessage (ORATOR_CTRLS[orator_index].combo_id, CB_GETITEMDATA, (WPARAM)curr_sel);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeleteitemRemarkList
//
/////////////////////////////////////////////////////////////////////////////
void
EditConversationDialogClass::OnDeleteitemRemarkList
(
	NMHDR *		pNMHDR,
	LRESULT *	pResult
) 
{
	NM_LISTVIEW *pNMListView = (NM_LISTVIEW *)pNMHDR;
	(*pResult) = 0;

	//
	//	Lookup the associated remark object
	//
	ConversationRemarkClass *remark = NULL;
	remark = (ConversationRemarkClass *)m_ListCtrl.GetItemData (pNMListView->iItem);

	//
	//	Free the object
	//
	SAFE_DELETE (remark);
	m_ListCtrl.SetItemData (pNMListView->iItem, 0);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Find_Combobox_Entry
//
/////////////////////////////////////////////////////////////////////////////
int
EditConversationDialogClass::Find_Combobox_Entry (int orator_type)
{
	int retval = -1;

	//
	//	Loop over all the entries in the combobox until we've found
	// the one that's associated with the given orator type
	//
	int count = SendDlgItemMessage (IDC_ORATOR1_COMBO, CB_GETCOUNT);
	for (int index = 0; index < count; index ++) {
		
		//
		//	Is this the orator we are looking for?
		//
		int item_data = SendDlgItemMessage (IDC_ORATOR1_COMBO, CB_GETITEMDATA, (WPARAM)index);
		if (item_data == orator_type) {
			retval = index;
			break;
		}
	}

	return retval;
}

