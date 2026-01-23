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

// PresetDialogueTab.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "presetdialoguetab.h"
#include "definition.h"
#include "preset.h"
#include "Utils.h"
#include "dialogue.h"
#include "combatchunkid.h"
#include "soldier.h"
#include "translateobj.h"
#include "translatedb.h"
#include "editdialoguedialog.h"
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
	COL_EVENT	= 0,
	COL_TEXT
};


/////////////////////////////////////////////////////////////////////////////
//
// PresetDialogueTabClass
//
/////////////////////////////////////////////////////////////////////////////
PresetDialogueTabClass::PresetDialogueTabClass (PresetClass *preset)
	:	m_Definition (NULL),
		m_DialogueList (NULL),
		m_IsReadOnly (false),
		DockableFormClass(PresetDialogueTabClass::IDD)
{
	//{{AFX_DATA_INIT(PresetDialogueTabClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	
	//
	//	Dig the soldier game object definition out from the preset
	//
	if (preset != NULL) {
		DefinitionClass *definition = preset->Get_Definition ();
		if (definition != NULL && definition->Get_Class_ID () == CLASSID_GAME_OBJECT_DEF_SOLDIER) {
			m_Definition = (SoldierGameObjDef *)definition;
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~PresetDialogueTabClass
//
/////////////////////////////////////////////////////////////////////////////
PresetDialogueTabClass::~PresetDialogueTabClass (void)
{
	SAFE_DELETE_ARRAY (m_DialogueList);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
PresetDialogueTabClass::DoDataExchange (CDataExchange *pDX)
{
	DockableFormClass::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PresetDialogueTabClass)
	DDX_Control(pDX, IDC_DIALOGUE_LIST, m_ListCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(PresetDialogueTabClass, DockableFormClass)
	//{{AFX_MSG_MAP(PresetDialogueTabClass)
	ON_NOTIFY(NM_DBLCLK, IDC_DIALOGUE_LIST, OnDblclkDialogueList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PresetDialogueTabClass diagnostics

#ifdef _DEBUG
void PresetDialogueTabClass::AssertValid() const
{
	DockableFormClass::AssertValid();
}

void PresetDialogueTabClass::Dump(CDumpContext& dc) const
{
	DockableFormClass::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
//
// HandleInitDialog
//
/////////////////////////////////////////////////////////////////////////////
void
PresetDialogueTabClass::HandleInitDialog (void)
{
	ASSERT (m_Definition != NULL);

	//
	//	Configure the list control
	//
	m_ListCtrl.InsertColumn (COL_EVENT, "Event");
	m_ListCtrl.InsertColumn (COL_TEXT, "Text");
	m_ListCtrl.SetExtendedStyle (m_ListCtrl.GetExtendedStyle () | LVS_EX_FULLROWSELECT);
	
	//
	//	Choose a size for the list control's columns
	//
	CRect rect;
	m_ListCtrl.GetClientRect (&rect);
	rect.right -= ::GetSystemMetrics (SM_CXVSCROLL);	
	m_ListCtrl.SetColumnWidth (COL_EVENT, rect.Width () / 2);
	m_ListCtrl.SetColumnWidth (COL_TEXT, rect.Width () / 2);

	//
	//	Allocate our own copy of the dialog list
	//
	m_DialogueList = new DialogueClass[DIALOG_MAX];
	
	//
	//	Get the list of dialogues from the definition
	//
	DialogueClass *dialog_list = m_Definition->Get_Dialog_List ();
	for (int index = 0; index < DIALOG_MAX; index ++) {
		
		//
		//	Copy this dialog from the definition's list to our own internal list
		//
		m_DialogueList[index] = dialog_list[index];
		
		//
		//	Add an entry to the list control for this dialogue
		//
		int item_index = m_ListCtrl.InsertItem (index, DIALOG_EVENT_NAMES[index]);
		if (item_index >= 0) {
			Update_Entry (item_index);
		}
	}

	//
	//	Make the controls read-only if necessary
	//
	if (m_IsReadOnly) {
		::EnableWindow (::GetDlgItem (m_hWnd, IDC_DIALOGUE_LIST), false);
	}

	//
	// Put the focus into the list control
	//
	::SetFocus (::GetDlgItem (m_hWnd, IDC_DIALOGUE_LIST));
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Apply_Changes
//
/////////////////////////////////////////////////////////////////////////////
bool
PresetDialogueTabClass::Apply_Changes (void)
{
	DialogueClass *dialog_list = m_Definition->Get_Dialog_List ();

	//
	//	Copy the changes into the definition's dialog list
	//
	for (int index = 0; index < DIALOG_MAX; index ++) {		
		dialog_list[index] = m_DialogueList[index];
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDblclkDialogueList
//
/////////////////////////////////////////////////////////////////////////////
void
PresetDialogueTabClass::OnDblclkDialogueList
(
	NMHDR *		/* pNMHDR */,
	LRESULT *	pResult
)
{
	(*pResult) = 0;

	//
	//	Get the index of the currently selected entry
	//
	int index = m_ListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (index >= 0 && index < DIALOG_MAX) {

		//
		//	Show a dialog to the user that will allow them to edit this entry
		//
		EditDialogueDialogClass dialog (this);
		dialog.Set_Dialogue (&m_DialogueList[index]);				
		if (dialog.DoModal () == IDOK) {
			Update_Entry (index);
			InvalidateRect (NULL, true);
			//UpdateWindow ();
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Entry
//
/////////////////////////////////////////////////////////////////////////////
void
PresetDialogueTabClass::Update_Entry (int index)
{
	//
	//	Just pick the first remark in the list
	//
	DIALOGUE_OPTION_LIST	&option_list = m_DialogueList[index].Get_Option_List ();
	if (option_list.Count () > 0) {
		
		//
		//	Lookup the string entry in our translation database
		//
		int conversation_id = option_list[0]->Get_Conversation_ID ();
		ConversationClass *conversation = ConversationMgrClass::Find_Conversation (conversation_id);
		if (conversation != NULL) {					
			
			//
			//	Put this text into the appropriate column in the list control
			//
			m_ListCtrl.SetItemText (index, COL_TEXT, conversation->Get_Name ());
			REF_PTR_RELEASE (conversation);
		}

	} else {
		m_ListCtrl.SetItemText (index, COL_TEXT, "");
	}
	
	return ;
}
