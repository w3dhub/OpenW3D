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

// ConversationPickerDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "conversationpickerdialog.h"
#include "conversationmgr.h"
#include "conversation.h"
#include "Utils.h"
#include "icons.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// ConversationPickerDialogClass
//
/////////////////////////////////////////////////////////////////////////////
ConversationPickerDialogClass::ConversationPickerDialogClass (CWnd *pParent /*=NULL*/)
	:	Conversation (NULL),
		GlobalsRoot (NULL),
		LevelsRoot (NULL),
		CDialog (ConversationPickerDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(ConversationPickerDialogClass)
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
ConversationPickerDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ConversationPickerDialogClass)
	DDX_Control(pDX, IDC_CONVERSATION_TREE, m_TreeCtrl);
	//}}AFX_DATA_MAP

	return ;
}


BEGIN_MESSAGE_MAP(ConversationPickerDialogClass, CDialog)
	//{{AFX_MSG_MAP(ConversationPickerDialogClass)
	ON_NOTIFY(TVN_SELCHANGED, IDC_CONVERSATION_TREE, OnSelchangedConversationTree)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnSelchangedConversationTree
//
/////////////////////////////////////////////////////////////////////////////
void
ConversationPickerDialogClass::OnSelchangedConversationTree
(
	NMHDR *		/* pNMHDR */,
	LRESULT *	pResult
)
{
	(*pResult) = 0;

	Conversation = NULL;

	//
	//	Lookup the currently selected conversation
	//
	HTREEITEM selected_item = m_TreeCtrl.GetSelectedItem ();
	if (selected_item != NULL) {
		Conversation = (ConversationClass *)m_TreeCtrl.GetItemData (selected_item);
	}

	//
	//	Update the enable state based on whether or not a conversation was selected
	//
	::EnableWindow (::GetDlgItem (m_hWnd, IDOK), (Conversation != NULL));
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
ConversationPickerDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();
	
	//
	// Pass the general use imagelist onto the tree control
	//
	m_TreeCtrl.SetImageList (::Get_Global_Image_List (), TVSIL_NORMAL);

	//
	//	Insert the folders
	//
	GlobalsRoot	= m_TreeCtrl.InsertItem ("Globals", FOLDER_ICON, FOLDER_ICON);
	LevelsRoot	= m_TreeCtrl.InsertItem ("Level Specific", FOLDER_ICON, FOLDER_ICON);

	//
	//	Add all the conversations to the tree
	//
	int count = ConversationMgrClass::Get_Conversation_Count ();
	for (int index = 0; index < count; index ++) {
		Insert_Entry (ConversationMgrClass::Peek_Conversation (index));
	}

	//
	//	Open the globals and levels folders
	//
	m_TreeCtrl.Expand (GlobalsRoot, TVE_EXPAND);
	m_TreeCtrl.Expand (LevelsRoot, TVE_EXPAND);

	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
ConversationPickerDialogClass::OnOK (void)
{
	if (Conversation != NULL) {
		CDialog::OnOK ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Insert_Entry
//
/////////////////////////////////////////////////////////////////////////////
void
ConversationPickerDialogClass::Insert_Entry (ConversationClass *conversation)
{
	//
	//	Determine which folder this conversation goes under
	//
	HTREEITEM parent_item = LevelsRoot;
	if (conversation->Get_Category_ID () != ConversationMgrClass::CATEGORY_LEVEL) {
		parent_item = GlobalsRoot;
	}

	//
	//	Insert this item into the tree
	//
	HTREEITEM tree_item = m_TreeCtrl.InsertItem (conversation->Get_Name (), DIALOGUE_ICON,
													DIALOGUE_ICON, parent_item);
	if (tree_item != NULL) {
		
		//
		//	Associate the conversation with the entry in the tree
		//
		m_TreeCtrl.SetItemData (tree_item, (DWORD_PTR)conversation);
		m_TreeCtrl.SortChildren (parent_item);

		//
		//	Select this conversation if necessary
		//
		if (conversation == Conversation) {
			m_TreeCtrl.SelectItem (tree_item);
			m_TreeCtrl.EnsureVisible (tree_item);
		}
	}

	return ;
}
