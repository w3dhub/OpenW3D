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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/ConversationPage.cpp    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/12/02 9:57a                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"

#include "LevelEdit.h"
#include "conversationpage.h"
#include "nodemgr.h"
#include "node.h"
#include "icons.h"
#include "conversation.h"
#include "conversationmgr.h"
#include "editconversationdialog.h"
#include "conversationeditormgr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



///////////////////////////////////////////////////////////////////////
//
//	Helper objects
//
///////////////////////////////////////////////////////////////////////
class TreeStateCaptureClass
{
public:
	TreeStateCaptureClass (CTreeCtrl &tree_ctrl, HTREEITEM *item_handle, ConversationClass **conversation_ptr)
		:	ConversationID (0),
			ConversationPtr (conversation_ptr),
			ItemHandle (item_handle),
			TreeCtrl (tree_ctrl)
	{
		if (conversation_ptr != NULL) {
			ConversationID = (*conversation_ptr)->Get_ID ();
		}

		return ;
	}

	void Restore (void)
	{
		if (ItemHandle != NULL) {
			(*ItemHandle) = NULL;
		}

		if (ConversationPtr != NULL) {					
			(*ConversationPtr) = NULL;
		}

		Find_Entry (TVI_ROOT);

		//
		//	Ensure the item is visible
		//
		if (ItemHandle != NULL && (*ItemHandle) != NULL) {
			TreeCtrl.SelectItem (*ItemHandle);
			TreeCtrl.EnsureVisible (*ItemHandle);
		}

		return ;
	}

	bool Find_Entry (HTREEITEM parent_item)
	{
		bool retval = false;

		//
		//	Loop over all the children of this tree item
		//
		for (	HTREEITEM child_item = TreeCtrl.GetNextItem (parent_item, TVGN_CHILD);
				retval == false && child_item != NULL;
				child_item = TreeCtrl.GetNextItem (child_item, TVGN_NEXT))
		{
			//
			//	Is this the conversation we are looking for?
			//
			ConversationClass *conversation = (ConversationClass *)TreeCtrl.GetItemData (child_item);
			if (conversation != NULL && conversation->Get_ID () == ConversationID) {
				
				if (ItemHandle != NULL) {					
					(*ItemHandle) = child_item;
				}

				if (ConversationPtr != NULL) {					
					(*ConversationPtr) = conversation;
				}

				retval = true;
				break ;
			}

			//
			//	Recurse into this child (if necessary)
			//
			if (TreeCtrl.ItemHasChildren (child_item)) {
				retval = Find_Entry (child_item);
			}
		}

		return retval;
	}

private:
	int						ConversationID;
	CTreeCtrl &				TreeCtrl;
	HTREEITEM *				ItemHandle;
	ConversationClass **	ConversationPtr;
};




/////////////////////////////////////////////////////////////////////////////
// Static member initialization
/////////////////////////////////////////////////////////////////////////////
ConversationPageClass *	ConversationPageClass::_TheInstance = NULL;



/////////////////////////////////////////////////////////////////////////////
// Local constants
/////////////////////////////////////////////////////////////////////////////
static const int TOOLBAR_HEIGHT		= 36;
static const int TOOLBAR_V_SPACING	= 5;
static const int TOOLBAR_V_BORDER	= TOOLBAR_V_SPACING * 2;
static const int TOOLBAR_H_SPACING	= 5;
static const int TOOLBAR_H_BORDER	= TOOLBAR_H_SPACING * 2;



/////////////////////////////////////////////////////////////////////////////
//
// ConversationPageClass
//
/////////////////////////////////////////////////////////////////////////////
ConversationPageClass::ConversationPageClass(CWnd* pParent /*=NULL*/)	:
	m_GlobalsRoot (NULL),
	m_LevelsRoot (NULL),	
	CDialog(ConversationPageClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(ConversationPageClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	Create (ConversationPageClass::IDD, pParent);
	_TheInstance = this;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~ConversationPageClass
//
/////////////////////////////////////////////////////////////////////////////
ConversationPageClass::~ConversationPageClass (void)
{
	_TheInstance = NULL;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
ConversationPageClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ConversationPageClass)
	DDX_Control(pDX, IDC_CONVERSATION_TREE, m_TreeCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(ConversationPageClass, CDialog)
	//{{AFX_MSG_MAP(ConversationPageClass)
	ON_NOTIFY(TVN_DELETEITEM, IDC_CONVERSATION_TREE, OnDeleteItemConversationTree)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_NOTIFY(NM_DBLCLK, IDC_CONVERSATION_TREE, OnDblclkConversationTree)
	ON_NOTIFY(TVN_ITEMEXPANDED, IDC_CONVERSATION_TREE, OnItemexpandedConversationTree)
	ON_COMMAND(IDC_ADD, OnAdd)
	ON_COMMAND(IDC_DELETE, OnDelete)
	ON_COMMAND(IDC_EDIT, OnEdit)
	ON_COMMAND(IDC_SWAP, OnSwap)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
//
// OnSize
//
/////////////////////////////////////////////////////////////////////////////
void
ConversationPageClass::OnSize
(
	UINT	nType,
	int	cx,
	int	cy
) 
{	
	// Allow the base class to process this message
	CDialog::OnSize (nType, cx, cy);

	if (::IsWindow (m_TreeCtrl) && (cx > 0) && (cy > 0)) {

		// Get the bounding rectangle of the form window
		CRect parentrect;
		GetWindowRect (&parentrect);
		
		// Get the bounding rectangle of the toolbar
		CRect toolbar_rect;
		m_Toolbar.GetWindowRect (&toolbar_rect);
		ScreenToClient (&toolbar_rect);

		// Move the toolbar so it is in its correct position
		m_Toolbar.SetWindowPos (NULL,
										TOOLBAR_H_SPACING,
										(cy - TOOLBAR_V_SPACING) - toolbar_rect.Height (),
										cx - TOOLBAR_H_BORDER,
										toolbar_rect.Height (),
										SWP_NOZORDER);

		// Get the bounding rectnagle of the list ctrl
		RECT list_rect;
		m_TreeCtrl.GetWindowRect (&list_rect);
				
		CRect client_rect = list_rect;
		ScreenToClient (&client_rect);
		int list_height = ((cy - TOOLBAR_V_BORDER) - toolbar_rect.Height ()) - client_rect.top;

		// Resize the tab control to fill the entire contents of the client area
		m_TreeCtrl.SetWindowPos (	NULL,
											0,
											0,
											cx-((list_rect.left - parentrect.left) << 1),
											list_height,
											SWP_NOZORDER | SWP_NOMOVE);	
	}

	return ;	
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDestroy
//
/////////////////////////////////////////////////////////////////////////////
void
ConversationPageClass::OnDestroy (void) 
{
	// Free the state image list we associated with the control
	CImageList *imagelist = m_TreeCtrl.GetImageList (TVSIL_STATE);
	m_TreeCtrl.SetImageList (NULL, TVSIL_STATE);
	SAFE_DELETE (imagelist);

	// Remove the main image list we associated with the control
	m_TreeCtrl.SetImageList (NULL, TVSIL_NORMAL);
	m_TreeCtrl.DeleteAllItems ();

	::RemoveProp (m_TreeCtrl, "TRANS_ACCS");
	::RemoveProp (m_hWnd, "TRANS_ACCS");

	// Allow the base class to process this message
	CDialog::OnDestroy ();
	return ;	
}


////////////////////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
////////////////////////////////////////////////////////////////////////////
BOOL
ConversationPageClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	m_Toolbar.CreateEx (this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_ALIGN_TOP, CRect(0, 0, 0, 0), 101);
	m_Toolbar.SetOwner (this);
	m_Toolbar.LoadToolBar (IDR_CONVERSATION_TOOLBAR);
	m_Toolbar.SetBarStyle (m_Toolbar.GetBarStyle () | CBRS_TOOLTIPS | CBRS_FLYBY);

	//
	// Size the toolbar
	//
	CRect parentrect;
	GetWindowRect (&parentrect);
	m_Toolbar.SetWindowPos (NULL, 0, 0, parentrect.Width () - TOOLBAR_H_BORDER, TOOLBAR_HEIGHT, SWP_NOZORDER | SWP_NOMOVE);	

	//
	// Pass the general use imagelist onto the tree control
	//
	m_TreeCtrl.SetImageList (::Get_Global_Image_List (), TVSIL_NORMAL);

	SetProp (m_TreeCtrl, "TRANS_ACCS", (HANDLE)1);
	SetProp (m_hWnd, "TRANS_ACCS", (HANDLE)1);

	Reload_Data ();
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeleteItemConversationTree
//
/////////////////////////////////////////////////////////////////////////////
void
ConversationPageClass::OnDeleteItemConversationTree
(
	NMHDR *	pNMHDR,
	LRESULT *pResult
) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	(*pResult) = 0;

	//
	//	Release our hold on the conversation object
	//
	ConversationClass *conversation = (ConversationClass *)pNMTreeView->itemOld.lParam;
	REF_PTR_RELEASE (conversation);
	m_TreeCtrl.SetItemData (pNMTreeView->itemOld.hItem, 0);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Insert_Entry
//
/////////////////////////////////////////////////////////////////////////////
void
ConversationPageClass::Insert_Entry (ConversationClass *conversation, bool sort_items)
{
	//
	//	Determine which folder this conversation goes under
	//
	HTREEITEM parent_item = m_LevelsRoot;
	if (conversation->Get_Category_ID () != ConversationMgrClass::CATEGORY_LEVEL) {
		parent_item = m_GlobalsRoot;
	}

	//
	//	Insert this item into the tree
	//
	HTREEITEM tree_item = m_TreeCtrl.InsertItem (conversation->Get_Name (), DIALOGUE_ICON,
													DIALOGUE_ICON, parent_item);
	if (tree_item != NULL) {
		
		//
		//	Lock a reference on the object
		//
		conversation->Add_Ref ();

		//
		//	Associate the conversation with the entry in the tree
		//
		m_TreeCtrl.SetItemData (tree_item, (DWORD_PTR)conversation);

		//
		//	Sort the items
		//
		if (sort_items) {			
			m_TreeCtrl.InvalidateRect (NULL, true);
			m_TreeCtrl.SortChildren (parent_item);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Reload_Data
//
/////////////////////////////////////////////////////////////////////////////
void
ConversationPageClass::Reload_Data (void)
{
	Reset_Tree ();

	//
	//	Add all the conversations to the tree
	//
	int count = ConversationMgrClass::Get_Conversation_Count ();
	for (int index = 0; index < count; index ++) {
		Insert_Entry (ConversationMgrClass::Peek_Conversation (index), false);
	}

	m_TreeCtrl.InvalidateRect (NULL, true);
	m_TreeCtrl.SortChildren (m_GlobalsRoot);
	m_TreeCtrl.SortChildren (m_LevelsRoot);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Reset_Tree
//
/////////////////////////////////////////////////////////////////////////////
void
ConversationPageClass::Reset_Tree (void)
{
	m_TreeCtrl.DeleteAllItems ();
	
	//
	//	Insert the root nodes
	//
	
	m_GlobalsRoot	= m_TreeCtrl.InsertItem ("Globals", FOLDER_ICON, FOLDER_ICON);
	m_LevelsRoot	= m_TreeCtrl.InsertItem ("Level Specific", FOLDER_ICON, FOLDER_ICON);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnDblclkConversationTree
//
////////////////////////////////////////////////////////////////////////////
void
ConversationPageClass::OnDblclkConversationTree
(
	NMHDR *		/* pNMHDR */,
	LRESULT *	pResult
) 
{
	//
	// Determine what client-coord location was double-clicked on
	//
	DWORD mouse_pos = ::GetMessagePos ();
	POINT hit_point = { GET_X_LPARAM (mouse_pos), GET_Y_LPARAM (mouse_pos) };
	m_TreeCtrl.ScreenToClient (&hit_point);

	//
	// Goto the node that was double-clicked on (if possible)
	//
	UINT flags				= 0;
	HTREEITEM tree_item	= m_TreeCtrl.HitTest (hit_point, &flags);
	if (tree_item != NULL && flags & TVHT_ONITEMLABEL) {
		
		//
		//	Edit the entry the user double-clicked on
		//
		Edit_Entry (tree_item);
		(*pResult) = 1;

	} else {
		(*pResult) = 0;
	}
		
	
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnItemexpandedConversationTree
//
////////////////////////////////////////////////////////////////////////////
void
ConversationPageClass::OnItemexpandedConversationTree
(
	NMHDR *		pNMHDR,
	LRESULT *	pResult
)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	//
	// If this is a folder, then change its image based
	// on its expanded state.
	//
	if (m_TreeCtrl.GetItemData (pNMTreeView->itemNew.hItem) == NULL) {

		if (pNMTreeView->itemNew.state & TVIS_EXPANDED) {
			m_TreeCtrl.SetItemImage (pNMTreeView->itemNew.hItem, OPEN_FOLDER_ICON, OPEN_FOLDER_ICON);
		} else {
			m_TreeCtrl.SetItemImage (pNMTreeView->itemNew.hItem, FOLDER_ICON, FOLDER_ICON);
		}

	}

	(*pResult) = 0;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnAdd
//
////////////////////////////////////////////////////////////////////////////
void
ConversationPageClass::OnAdd (void)
{
	HTREEITEM selected_item = m_TreeCtrl.GetSelectedItem ();
	if (selected_item != NULL) {

		//
		//	Determine if this is a global conversation
		//
		bool is_global = false;
		bool can_edit = true;
		bool check_in = false;
		if (	selected_item == m_GlobalsRoot ||
				m_TreeCtrl.GetParentItem (selected_item) == m_GlobalsRoot)
		{
			is_global = true;

			//
			//	Check out the database
			//
			CWaitCursor wait_cursor;
			check_in = ConversationEditorMgrClass::Check_Out ();
			can_edit = check_in;

			//
			//	Reload the tree
			//
			if (can_edit) {
				Reload_Tree (&selected_item, NULL);
			}
		}

		if (can_edit) {

			//
			//	Show a dialog to the user that will allow them to edit this conversation
			//
			EditConversationDialogClass dialog;
			if (dialog.DoModal () == IDOK) {

				ConversationClass *conversation = dialog.Peek_Conversation ();

				//
				//	Determine which folder to put this conversation under
				//
				if (is_global) {
					conversation->Set_Category_ID (ConversationMgrClass::CATEGORY_GLOBAL);
				} else {
					conversation->Set_Category_ID (ConversationMgrClass::CATEGORY_LEVEL);
					::Set_Modified ();
				}

				//
				//	Add the conversation to the manager
				//
				ConversationMgrClass::Add_Conversation (conversation);

				//
				//	Add the conversation to the tree
				//			
				Insert_Entry (conversation);
				m_TreeCtrl.UpdateWindow ();

				//
				//	Check in the global conversation database
				//
				if (check_in) {
					CWaitCursor wait_cursor;
					ConversationEditorMgrClass::Save_Global_Database ();
					ConversationEditorMgrClass::Check_In ();
				}

			} else if (check_in) {
				CWaitCursor wait_cursor;
				ConversationEditorMgrClass::Undo_Check_Out ();
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnDelete
//
////////////////////////////////////////////////////////////////////////////
void
ConversationPageClass::OnDelete (void)
{
	HTREEITEM selected_item = m_TreeCtrl.GetSelectedItem ();
	if (selected_item != NULL) {

		//
		//	Get the conversation associated with the selected item
		//
		ConversationClass *conversation = NULL;
		conversation = (ConversationClass *)m_TreeCtrl.GetItemData (selected_item);
		if (conversation != NULL) {

			//
			//	This "dirties" the level, so mark it as modified...
			//
			if (conversation->Get_Category_ID () == ConversationMgrClass::CATEGORY_LEVEL) {
				::Set_Modified ();
			}

			//
			//	Check out the global conversation database (if necessary)
			//
			bool check_in = false;
			bool can_edit = true;
			if (conversation->Get_Category_ID () == ConversationMgrClass::CATEGORY_GLOBAL) {
				CWaitCursor wait_cursor;
				check_in = ConversationEditorMgrClass::Check_Out ();
				can_edit = check_in;
				
				//
				//	Reload the tree
				//
				if (can_edit) {
					Reload_Tree (&selected_item, &conversation);
				}
			}

			if (can_edit && selected_item != NULL && conversation != NULL) {

				//
				//	Remove this conversation from the manager
				//
				ConversationMgrClass::Remove_Conversation (conversation);

				//
				//	Remove this conversation from the tree
				//
				m_TreeCtrl.DeleteItem (selected_item);
				m_TreeCtrl.UpdateWindow ();

				//
				//	Check in the global conversation database
				//
				if (check_in) {
					ConversationEditorMgrClass::Save_Global_Database ();
					ConversationEditorMgrClass::Check_In ();
				}
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnEdit
//
////////////////////////////////////////////////////////////////////////////
void
ConversationPageClass::OnEdit (void)
{
	HTREEITEM selected_item = m_TreeCtrl.GetSelectedItem ();
	if (selected_item != NULL) {
		
		//
		//	Edit the selected entry
		//
		Edit_Entry (selected_item);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Edit_Entry
//
////////////////////////////////////////////////////////////////////////////
void
ConversationPageClass::Edit_Entry (HTREEITEM tree_item)
{
	if (tree_item != NULL) {

		//
		//	Edit the conversation (if necessary)
		//
		ConversationClass *conversation = (ConversationClass *)m_TreeCtrl.GetItemData (tree_item);
		if (conversation != NULL) {

			//
			//	Check out the global conversation database (if necessary)
			//
			bool check_in = false;
			bool can_edit = true;
			if (conversation->Get_Category_ID () == ConversationMgrClass::CATEGORY_GLOBAL) {
				CWaitCursor wait_cursor;
				check_in = ConversationEditorMgrClass::Check_Out ();
				can_edit = check_in;

				//
				//	Reload the tree
				//
				if (can_edit) {
					Reload_Tree (&tree_item, &conversation);
				}
			}

			if (can_edit && tree_item != NULL && conversation != NULL) {
				
				//
				//	Display a dialog to the user allowing them to edit the conversation
				//
				EditConversationDialogClass dialog;
				dialog.Set_Conversation (conversation);
				if (dialog.DoModal () == IDOK) {
					m_TreeCtrl.SetItemText (tree_item, conversation->Get_Name ());
					m_TreeCtrl.UpdateWindow ();

					//
					//	This "dirties" the level, so mark it as modified...
					//
					if (conversation->Get_Category_ID () == ConversationMgrClass::CATEGORY_LEVEL) {
						::Set_Modified ();
					}

					//
					//	Check in the global conversation database
					//
					if (check_in) {
						CWaitCursor wait_cursor;
						ConversationEditorMgrClass::Save_Global_Database ();
						ConversationEditorMgrClass::Check_In ();
					}

				} else if (check_in) {
					CWaitCursor wait_cursor;
					ConversationEditorMgrClass::Undo_Check_Out ();
				}
			} 
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnSwap
//
////////////////////////////////////////////////////////////////////////////
void
ConversationPageClass::OnSwap (void) 
{
	HTREEITEM selected_item = m_TreeCtrl.GetSelectedItem ();
	if (selected_item != NULL) {		

		//
		//	Get the conversation associated with the selected item
		//
		ConversationClass *conversation = NULL;
		conversation = (ConversationClass *)m_TreeCtrl.GetItemData (selected_item);
		if (conversation != NULL) {

			//
			//	Check out the global conversation database
			//
			CWaitCursor wait_cursor;
			if (ConversationEditorMgrClass::Check_Out ()) {
				
				//
				//	Reload the tree
				//
				Reload_Tree (&selected_item, &conversation);
				if (selected_item != NULL && conversation != NULL) {
					
					//
					//	Remove the conversation from the system
					//
					ConversationMgrClass::Remove_Conversation (conversation);

					//
					//	Swap categories
					//
					int category_id = conversation->Get_Category_ID ();
					if (category_id == ConversationMgrClass::CATEGORY_LEVEL) {
						conversation->Set_Category_ID (ConversationMgrClass::CATEGORY_GLOBAL);
					} else {				
						conversation->Set_Category_ID (ConversationMgrClass::CATEGORY_LEVEL);
					}

					//
					//	Re-add the conversation to the manager and the UI
					//
					ConversationMgrClass::Add_Conversation (conversation);
					Insert_Entry (conversation);

					//
					//	Remove the old entry
					//
					m_TreeCtrl.DeleteItem (selected_item);
					m_TreeCtrl.UpdateWindow ();

					//
					//	This "dirties" the level, so mark it as modified...
					//
					::Set_Modified ();

					//
					//	Checkin the global conversation database
					//
					ConversationEditorMgrClass::Save_Global_Database ();
					ConversationEditorMgrClass::Check_In ();
				}
			}
		}		
	}
		
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Reload_Tree
//
////////////////////////////////////////////////////////////////////////////
void
ConversationPageClass::Reload_Tree (HTREEITEM *item_handle, ConversationClass **conversation_ptr)
{
	m_TreeCtrl.SetRedraw (false);

	//
	//	Reload the database while capturing its state
	//
	TreeStateCaptureClass capture_obj (m_TreeCtrl, item_handle, conversation_ptr);
	ConversationEditorMgrClass::Load_Global_Database ();
	capture_obj.Restore ();
	
	//
	//	Expand the two root folders by default
	//
	m_TreeCtrl.Expand (m_GlobalsRoot, TVE_EXPAND);
	m_TreeCtrl.Expand (m_LevelsRoot, TVE_EXPAND);

	m_TreeCtrl.SetRedraw (true);
	return ;
}

