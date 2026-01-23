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

// EditConversationListDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "editconversationlistdialog.h"
#include "conversationmgr.h"
#include "editconversationdialog.h"
#include "conversation.h"
#include "Utils.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// EditConversationListDialogClass
//
/////////////////////////////////////////////////////////////////////////////
EditConversationListDialogClass::EditConversationListDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(EditConversationListDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(EditConversationListDialogClass)
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
EditConversationListDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EditConversationListDialogClass)
	DDX_Control(pDX, IDC_CONVERSATION_LIST, m_ListCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(EditConversationListDialogClass, CDialog)
	//{{AFX_MSG_MAP(EditConversationListDialogClass)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_NOTIFY(NM_DBLCLK, IDC_CONVERSATION_LIST, OnDblclkConversationList)
	ON_NOTIFY(LVN_DELETEITEM, IDC_CONVERSATION_LIST, OnDeleteitemConversationList)
	ON_NOTIFY(LVN_KEYDOWN, IDC_CONVERSATION_LIST, OnKeydownConversationList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
EditConversationListDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	//
	//	Configure the columns
	//
	m_ListCtrl.InsertColumn (0, "Name");
	m_ListCtrl.SetExtendedStyle (m_ListCtrl.GetExtendedStyle () | LVS_EX_FULLROWSELECT);
	
	//
	//	Choose an appropriate size for the columns
	//
	CRect rect;
	m_ListCtrl.GetClientRect (&rect);
	rect.right -= ::GetSystemMetrics (SM_CXVSCROLL);
	m_ListCtrl.SetColumnWidth (0, rect.Width ());

	//
	//	Add all the conversations to the list control
	//
	int count = ConversationMgrClass::Get_Conversation_Count ();
	for (int index = 0; index < count; index ++) {
		ConversationClass *conversation = ConversationMgrClass::Peek_Conversation (index);
		Add_Conversation (conversation);
	}
	
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnAdd
//
/////////////////////////////////////////////////////////////////////////////
void
EditConversationListDialogClass::OnAdd (void)
{
	//
	//	Let the user create a new conversation
	//
	EditConversationDialogClass dialog (this);
	if (dialog.DoModal () == IDOK) {
		
		//
		//	Add this new conversation to the list control
		//
		ConversationClass *conversation = dialog.Peek_Conversation ();
		if (conversation != NULL) {
			Add_Conversation (conversation);
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
EditConversationListDialogClass::OnOK (void)
{
	//
	//	Start fresh
	//	
	ConversationMgrClass::Reset ();

	//
	//	Get all the conversations from the list control and add them to the
	// conversation manager
	//	
	for (int index = 0; index < m_ListCtrl.GetItemCount (); index ++) {		
		ConversationClass *conversation = (ConversationClass *)m_ListCtrl.GetItemData (index);
		if (conversation != NULL) {
			ConversationMgrClass::Add_Conversation (conversation);
		}
	}

	::Set_Modified (true);
	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Add_Conversation
//
/////////////////////////////////////////////////////////////////////////////
void
EditConversationListDialogClass::Add_Conversation (ConversationClass *conversation)
{
	ASSERT (conversation != NULL);
	if (conversation == NULL) {
		return ;
	}
	
	//
	//	Insert this conversation into the list control
	//
	int item_index = m_ListCtrl.InsertItem (0xFFFF, conversation->Get_Name ());
	if (item_index != -1) {
		conversation->Add_Ref ();
		m_ListCtrl.SetItemData (item_index, (DWORD_PTR)conversation);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Conversation
//
/////////////////////////////////////////////////////////////////////////////
void
EditConversationListDialogClass::Update_Conversation (int index)
{
	//
	//	Update the conversation's name in the list control
	//
	ConversationClass *conversation = (ConversationClass *)m_ListCtrl.GetItemData (index);
	if (conversation != NULL) {
		m_ListCtrl.SetItemText (index, 0, conversation->Get_Name ());
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDblclkConversationList
//
/////////////////////////////////////////////////////////////////////////////
void
EditConversationListDialogClass::OnDblclkConversationList
(
	NMHDR *		/* pNMHDR */,
	LRESULT *	pResult
)
{
	(*pResult) = 0;

	//
	//	Lookup the conversation object associated with the entry the user
	// double clicked on
	//
	int sel_index = m_ListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (sel_index >= 0) {
		ConversationClass *conversation = (ConversationClass *)m_ListCtrl.GetItemData (sel_index);
		if (conversation != NULL) {

			//
			//	Allow the user to edit this entry
			//
			EditConversationDialogClass dialog (this);
			dialog.Set_Conversation (conversation);
			if (dialog.DoModal () == IDOK) {
				Update_Conversation (sel_index);
			}			
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeleteitemConversationList
//
/////////////////////////////////////////////////////////////////////////////
void
EditConversationListDialogClass::OnDeleteitemConversationList
(
	NMHDR *	pNMHDR,
	LRESULT* pResult
)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	(*pResult) = 0;

	//
	//	Release our hold on this conversation object
	//
	ConversationClass *conversation = (ConversationClass *)m_ListCtrl.GetItemData (pNMListView->iItem);
	REF_PTR_RELEASE (conversation);

	m_ListCtrl.SetItemData (pNMListView->iItem, 0L);
	return ;
}



/////////////////////////////////////////////////////////////////////////////
//
// OnDeleteitemConversationList
//
/////////////////////////////////////////////////////////////////////////////
void
EditConversationListDialogClass::OnKeydownConversationList
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
