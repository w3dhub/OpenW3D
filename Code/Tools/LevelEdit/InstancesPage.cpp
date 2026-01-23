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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/InstancesPage.cpp            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/11/01 9:25a                                               $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "LevelEdit.h"
#include "instancespage.h"
#include "nodemgr.h"
#include "node.h"
#include "Utils.h"
#include "cameramgr.h"
#include "gotoobjectdialog.h"
#include "sceneeditor.h"
#include "icons.h"
#include "nodecategories.h"
#include "definition.h"
#include "definitionfactory.h"
#include "definitionfactorymgr.h"
#include "preset.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////////////////////
//	Local prototypes
///////////////////////////////////////////////////////////////////////
int CALLBACK InstancesListSortCallback (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

#ifndef ListView_SetCheckState
   #define ListView_SetCheckState(hwndLV, i, fCheck) \
      ListView_SetItemState(hwndLV, i, \
      INDEXTOSTATEIMAGEMASK((fCheck)+1), LVIS_STATEIMAGEMASK)
#endif

#ifndef ListView_SetOverlay
   #define ListView_SetOverlay(hwndLV, i, overlay) \
      ListView_SetItemState(hwndLV, i, \
      INDEXTOOVERLAYMASK(overlay), LVIS_OVERLAYMASK)
#endif


///////////////////////////////////////////////////////////////////////
//	Local structures
///////////////////////////////////////////////////////////////////////
typedef enum
{
	TYPE_NAVIGATOR,
	TYPE_FACTORY,
	TYPE_NODE,
} ITEM_TYPE;

typedef struct
{
	ITEM_TYPE	type;
	StringClass	name;
	union
	{
		uint32		class_id;
		NodeClass *	node;
	};

} ITEM_DATA;


/////////////////////////////////////////////////////////////////////////////
// Local constants
/////////////////////////////////////////////////////////////////////////////
const int TOOLBAR_HEIGHT		= 36;
const int TOOLBAR_V_SPACING	= 5;
const int TOOLBAR_V_BORDER		= TOOLBAR_V_SPACING * 2;
const int TOOLBAR_H_SPACING	= 5;
const int TOOLBAR_H_BORDER		= TOOLBAR_H_SPACING * 2;

//const int COLUMN_CHECK			= 0;
//const int COLUMN_PLUS			= 1;
const int COLUMN_NAME			= 0;

/////////////////////////////////////////////////////////////////////////////
//
// InstancesPageClass
//
/////////////////////////////////////////////////////////////////////////////
InstancesPageClass::InstancesPageClass (CWnd *parent_wnd)
	:	m_ClassID (0),
		CDialog(InstancesPageClass::IDD)
{
	//{{AFX_DATA_INIT(InstancesPageClass)
	//}}AFX_DATA_INIT
	Create (InstancesPageClass::IDD, parent_wnd);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~InstancesPageClass
//
/////////////////////////////////////////////////////////////////////////////
InstancesPageClass::~InstancesPageClass (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(InstancesPageClass)
	DDX_Control(pDX, IDC_INSTANCE_LIST, m_ListCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(InstancesPageClass, CDialog)
	//{{AFX_MSG_MAP(InstancesPageClass)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_COMMAND(IDC_EDIT, OnEdit)
	ON_COMMAND(IDC_GOTO, OnGoto)
	ON_COMMAND(IDC_SHOWALL, OnShowAll)
	ON_COMMAND(IDC_SELECT, OnSelect)
	ON_COMMAND(IDC_DELETE, OnDelete)
	ON_NOTIFY(LVN_DELETEITEM, IDC_INSTANCE_LIST, OnDeleteitemInstanceList)
	ON_NOTIFY(NM_DBLCLK, IDC_INSTANCE_LIST, OnDblclkInstanceList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_INSTANCE_LIST, OnItemchangedInstanceList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


#ifdef _DEBUG
void InstancesPageClass::AssertValid() const
{
	CDialog::AssertValid();
}

void InstancesPageClass::Dump(CDumpContext& dc) const
{
	CDialog::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
//
// OnSize
//
/////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::OnSize
(
	UINT	nType,
	int	cx,
	int	cy
) 
{	
	// Allow the base class to process this message
	CDialog::OnSize (nType, cx, cy);

	if (::IsWindow (m_ListCtrl) && (cx > 0) && (cy > 0)) {

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
		m_ListCtrl.GetWindowRect (&list_rect);
				
		CRect client_rect = list_rect;
		ScreenToClient (&client_rect);
		int list_height = ((cy - TOOLBAR_V_BORDER) - toolbar_rect.Height ()) - client_rect.top;

		// Resize the tab control to fill the entire contents of the client area
		m_ListCtrl.SetWindowPos (	NULL,
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
InstancesPageClass::OnDestroy (void) 
{
	//
	// Free the state image list we associated with the control
	//
	CImageList *imagelist = m_ListCtrl.GetImageList (LVSIL_STATE);
	m_ListCtrl.SetImageList (NULL, LVSIL_STATE);
	SAFE_DELETE (imagelist);

	//
	// Remove the main image list we associated with the control
	//
	m_ListCtrl.SetImageList (NULL, LVSIL_SMALL);
	m_ListCtrl.DeleteAllItems ();

	::RemoveProp (m_ListCtrl, "TRANS_ACCS");
	::RemoveProp (m_hWnd, "TRANS_ACCS");

	CDialog::OnDestroy ();
	return ;	
}


/////////////////////////////////////////////////////////////////////////////
//
// OnEdit
//
/////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::OnEdit (void)
{
	// Show the settings dialog for the currently selected item
	NodeClass *node = Get_Item_Node (m_ListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED));
	if (node != NULL) {
		node->Show_Settings_Dialog ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnGoto
//
/////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::OnGoto (void)
{
	NodeClass *selected_node = NULL;

	//
	//	Try to find the first selected node (if any)
	//
	int index = m_ListCtrl.GetNextItem (-1, LVNI_SELECTED | LVNI_ALL);
	if (index >= 0) {
		selected_node = Get_Item_Node (index);
	}

	//
	// Show the 'goto' dialog...
	//
	GotoObjectDialogClass dialog (selected_node);
	dialog.DoModal ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnShowAll
//
/////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::OnShowAll (void)
{
	//
	//	Show all the nodes
	//
	for (	NodeClass *node = NodeMgrClass::Get_First ();
			node != NULL;
			node = NodeMgrClass::Get_Next (node))
	{
		node->Hide (false);
	}

	//
	//	Fixup the UI
	//
	for (int index = 0; index < m_ListCtrl.GetItemCount (); index ++) {
		ITEM_DATA *item_data = (ITEM_DATA *)m_ListCtrl.GetItemData (index);
		if (item_data != NULL && item_data->type != TYPE_NAVIGATOR) {
			ListView_SetCheckState (m_ListCtrl, index, true);
		}
	}

	//
	// Make sure the main view is updated
	//
	::Refresh_Main_View ();	
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  WindowProc
//
////////////////////////////////////////////////////////////////////////////
LRESULT
InstancesPageClass::WindowProc
(
    UINT		message,
    WPARAM	wParam,
    LPARAM	lParam
)
{
	//
	// Is this the message we are expecting?
	//
	if (message == WM_USER+102) {			
		int index = (int)lParam;

		//
		//	Should we toggle the checkmark for this entry?
		//
		ITEM_DATA *item_data = (ITEM_DATA *)m_ListCtrl.GetItemData (index);
		if (item_data != NULL && item_data->type != TYPE_NAVIGATOR) {

			//
			//	Either hide or show the selected nodes
			//
			bool hidden = (ListView_GetCheckState (m_ListCtrl, index) == false);
			Hide_Nodes (index, !hidden);
			ListView_SetCheckState (m_ListCtrl, index, hidden);
		}
	}

	return CDialog::WindowProc(message, wParam, lParam);
}


////////////////////////////////////////////////////////////////////////////
//
//  CheckBoxSubclassProc
//
////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK
CheckBoxSubclassProc
(
	HWND		hwnd,
	UINT		message,
	WPARAM	wparam,
	LPARAM	lparam
)
{
	WNDPROC pold_proc = (WNDPROC)::GetProp (hwnd, "OLDPROC");
	
	if (message == WM_LBUTTONUP) {
		
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
			//	Notify the instances page that the user wants to toggle the checkbox
			//
			::PostMessage (::GetParent (hwnd), WM_USER+102, 0, (LPARAM)hittest.iItem);
		}

	} else if (message == WM_DESTROY) {
		::SetWindowLongPtr (hwnd, GWLP_WNDPROC, (LONG_PTR)pold_proc);
		::RemoveProp (hwnd, "OLDPROC");
	}

	if (pold_proc != NULL) {
		return ::CallWindowProc (pold_proc, hwnd, message, wparam, lparam);
	} else {
		return ::DefWindowProc (hwnd, message, wparam, lparam);
	}			
}


////////////////////////////////////////////////////////////////////////////
//
//  OnDblclkInstanceList
//
////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::OnDblclkInstanceList
(
	NMHDR *	/* pNMHDR */,
	LRESULT* pResult
) 
{
	// Determine what client-coord location was double-clicked on
	DWORD mouse_pos = ::GetMessagePos ();
	POINT hit_point = { GET_X_LPARAM (mouse_pos), GET_Y_LPARAM (mouse_pos) };
	m_ListCtrl.ScreenToClient (&hit_point);

	// Goto the node that was double-clicked on (if possible)
	UINT flags = 0;
	int index = m_ListCtrl.HitTest (hit_point, &flags);
	if ((index >= 0) && ((flags & LVHT_ONITEMLABEL) || (flags & LVHT_ONITEMICON))) {
		
		ITEM_DATA *item_data = (ITEM_DATA *)m_ListCtrl.GetItemData (index);		
		if (item_data != NULL) {

			
			if (item_data->type == TYPE_NAVIGATOR) {
				
				//
				//	Determine which class id we should display
				//
				int class_id = 0;				
				if (m_ClassIDStack.Count () > 0) {
					class_id = m_ClassIDStack[m_ClassIDStack.Count ()-1];
					m_ClassIDStack.Delete (m_ClassIDStack.Count ()-1);
				}

				Populate_List (class_id);

			} else if (item_data->type == TYPE_FACTORY) {
				
				//
				//	Fill the list with children of the given class id
				//
				m_ClassIDStack.Add (m_ClassID);
				Populate_List (item_data->class_id);
			} else {
				
				NodeClass *node = Get_Item_Node (index);
				if (node != NULL) {
					
					//
					//	If this node is a terrain, then display the subojects,
					// otherwise snap the camera to the node
					//
					if (node->Get_Sub_Node_Count () > 0) {
						m_ClassIDStack.Add (m_ClassID);
						Populate_List (node);
					} else {
						::Get_Camera_Mgr ()->Goto_Node (node);
					}
				}
			}
		}		
	}
	
	(*pResult) = 0;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnSelect
//
////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::OnSelect (void)
{
	//
	//	Clear all selected nodes
	//
	::Get_Scene_Editor ()->Set_Selection (NULL);

	//
	//	Toggle the selection state of all the nodes in the selection set
	//
	int index = -1;
	while ((index = m_ListCtrl.GetNextItem (index, LVNI_ALL | LVNI_SELECTED)) >= 0) {
		NodeClass *node = Get_Item_Node (index);
		if (node != NULL) {
			::Get_Scene_Editor ()->Toggle_Selection (node);
		}
	}
	
	::Refresh_Main_View ();
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnDelete
//
////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::OnDelete (void) 
{
	CString message = "Are you sure you wish to delete the selected node(s)?";
	CString title;
	title.LoadString (IDS_DELETE_CONFIRM_TITLE);
	
	//
	// Ask the user if they really want to delete the selected item
	//
	if (MessageBox (message, title, MB_ICONEXCLAMATION | MB_YESNO) == IDYES) {

		//
		//	Build a list of all the nodes we need to delete
		//
		DynamicVectorClass<NodeClass *> delete_list;
		int index = -1;
		while ((index = m_ListCtrl.GetNextItem (index, LVNI_ALL | LVNI_SELECTED)) >= 0) {
			NodeClass *node = Get_Item_Node (index);
			if (node != NULL) {
				delete_list.Add (node);
			}
		}

		//
		//	Delete all the selected nodes
		//
		for (index = 0; index < delete_list.Count (); index ++) {
			::Get_Scene_Editor ()->Delete_Node (delete_list[index]);
		}
		
		::Refresh_Main_View ();
	}
	
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
////////////////////////////////////////////////////////////////////////////
BOOL
InstancesPageClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	m_Toolbar.CreateEx (this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_ALIGN_TOP, CRect(0, 0, 0, 0), 101);
	m_Toolbar.SetOwner (this);
	m_Toolbar.LoadToolBar (IDR_INSTANCES_TOOLBAR);
	m_Toolbar.SetBarStyle (m_Toolbar.GetBarStyle () | CBRS_TOOLTIPS | CBRS_FLYBY);

	//
	// Configure the toolbar
	//
	CRect parentrect;
	GetWindowRect (&parentrect);
	m_Toolbar.SetWindowPos (NULL, 0, 0, parentrect.Width () - TOOLBAR_H_BORDER, TOOLBAR_HEIGHT, SWP_NOZORDER | SWP_NOMOVE);	
	m_Toolbar.Enable_Button (IDC_DELETE, false);
	m_Toolbar.Enable_Button (IDC_SELECT, false);
	m_Toolbar.Enable_Button (IDC_EDIT, false);

	//
	//	Configure the list control's extended styles
	//
	//m_ListCtrl.SetExtendedStyle (m_ListCtrl.GetExtendedStyle () | LVS_EX_SUBITEMIMAGES | LVS_EX_FULLROWSELECT);

	//
	// Create a state imagelist for the list control to use for checkboxes
	//
	CImageList *imagelist = new CImageList;
	imagelist->Create (MAKEINTRESOURCE (IDB_CHECKBOX_STATES1), 13, 0, RGB (255, 0, 255));
	m_ListCtrl.SetImageList (imagelist, LVSIL_STATE);
	
	//
	//	Setup the columns of the list control
	//
	CRect client_rect;
	m_ListCtrl.GetClientRect (&client_rect);
	m_ListCtrl.InsertColumn (COLUMN_NAME, "Name");
	m_ListCtrl.SetColumnWidth (COLUMN_NAME, client_rect.Width () - 5);

	//
	// Pass the general use imagelist onto the list control
	//
	m_ListCtrl.SetImageList (::Get_Global_Image_List (), LVSIL_SMALL);

	//
	//	Subclass the list control so we can handle the checkstates
	//
	LONG_PTR oldproc = ::SetWindowLongPtr (m_ListCtrl, GWLP_WNDPROC, (LONG_PTR)CheckBoxSubclassProc);
	::SetProp (m_ListCtrl, "OLDPROC", (HANDLE)(LONG_PTR)oldproc);

	//
	//	Populate the list control
	//
	Reset_List ();

	SetProp (m_ListCtrl, "TRANS_ACCS", (HANDLE)1);
	SetProp (m_hWnd, "TRANS_ACCS", (HANDLE)1);
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// Insert_Factory
//
/////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::Insert_Factory (LPCTSTR name, int class_id)
{
	//
	//	Add an entry to the list control for the factory
	//
	int index = m_ListCtrl.InsertItem (0xFF, name, FOLDER_ICON);
	if (index >= 0) {

		//
		//	Set the text and the icon for this entry
		//
		//m_ListCtrl.SetItem (index, COLUMN_NAME, LVIF_TEXT | LVIF_IMAGE, name, FOLDER_ICON, 0, 0, 0);

		//
		//	Allocate a new wrapper if we need to
		//
		ITEM_DATA *item_data = (ITEM_DATA *)m_ListCtrl.GetItemData (index);
		if (item_data == NULL) {
			item_data = new ITEM_DATA;
		}

		//
		//	Set a flag in the wrapper so we know what kind of data
		// this list item contains.
		//
		item_data->type		= TYPE_FACTORY;
		item_data->class_id	= class_id;
		item_data->name		= name;
		m_ListCtrl.SetItemData (index, (DWORD_PTR)item_data);
		m_ListCtrl.SetCheck (index, true);
	}
	
	return ;	
}


/////////////////////////////////////////////////////////////////////////////
//
// Insert_Node
//
/////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::Insert_Node (NodeClass *node)
{
	//
	//	Add an entry to the list control for the node
	//
	int index = m_ListCtrl.InsertItem (0xFF, node->Get_Name (), node->Get_Icon_Index ());
	if (index >= 0) {

		//
		//	Set the text and the icon for this entry
		//
		//m_ListCtrl.SetItem (index, COLUMN_NAME, LVIF_TEXT | LVIF_IMAGE, node->Get_Name (), node->Get_Icon_Index (), 0, 0, 0);
		
		//
		//	Allocate a new wrapper if we need to
		//
		ITEM_DATA *item_data = (ITEM_DATA *)m_ListCtrl.GetItemData (index);
		if (item_data == NULL) {
			item_data = new ITEM_DATA;
		}

		//
		//	Set a flag in the wrapper so we know what kind of data
		// this list item contains.
		//
		SAFE_ADD_REF (node);
		item_data->type	= TYPE_NODE;
		item_data->node	= node;
		item_data->name	= node->Get_Name ();
		m_ListCtrl.SetItemData (index, (DWORD_PTR)item_data);
		ListView_SetCheckState (m_ListCtrl, index, (node->Is_Hidden () == false));
	}

	return ;	
}


/////////////////////////////////////////////////////////////////////////////
//
// Insert_Navigator
//
/////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::Insert_Navigator (void)
{
	//
	//	Add an entry to the list control for the navigator
	//
	int index = m_ListCtrl.InsertItem (0xFF, "..", NAVIGATOR_ICON);
	if (index >= 0) {

		//
		//	Set the text and the icon for this entry
		//
		//m_ListCtrl.SetItem (index, COLUMN_NAME, LVIF_TEXT | LVIF_IMAGE, "..", NAVIGATOR_ICON, 0, 0, 0);
		
		//
		//	Allocate a new wrapper if we need to
		//
		ITEM_DATA *item_data = (ITEM_DATA *)m_ListCtrl.GetItemData (index);
		if (item_data == NULL) {
			item_data = new ITEM_DATA;
		}

		//
		//	Set a flag in the wrapper so we know what kind of data
		// this list item contains.
		//
		item_data->type	= TYPE_NAVIGATOR;
		item_data->node	= NULL;
		item_data->name	= "..";
		m_ListCtrl.SetItemData (index, (DWORD_PTR)item_data);
	}

	return ;	
}


////////////////////////////////////////////////////////////////////////////
//
//  Add_Node
//
////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::Add_Node (NodeClass *node)
{
	WWASSERT (node != NULL);

	PresetClass *preset = node->Get_Preset ();
	if (preset != NULL) {

		DefinitionClass *definition = preset->Get_Definition ();
		WWASSERT (definition != NULL);
		if (definition != NULL) {

			//
			//	Should the node be inserted into the current view?
			//
			if (definition->Get_Class_ID () == m_ClassID) {
				m_ListCtrl.SetRedraw (false);
				Insert_Node (node);
				m_ListCtrl.SortItems (InstancesListSortCallback, 0L);
				m_ListCtrl.SetRedraw (true);
			}			
		}
	}

	Update_Overlays ();
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Remove_Node
//
////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::Remove_Node (NodeClass *node)
{
	WWASSERT (node != NULL);

	//
	//	Attempt to find this node in the current view
	//
	for (int index = 0; index < m_ListCtrl.GetItemCount (); index ++) {
		NodeClass *curr_node = Get_Item_Node (index);
		
		//
		//	If this is the node we are looking for, then remove it from
		// the view.
		//
		if (curr_node == node) {
			m_ListCtrl.DeleteItem (index);
			break;
		}
	}

	Update_Overlays ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Item_Node
//
/////////////////////////////////////////////////////////////////////////////
NodeClass *
InstancesPageClass::Get_Item_Node (int index)
{
	NodeClass *node = NULL;

	//
	//	If this item represents a node, then return the node
	//	pointer to the caller.
	//
	ITEM_DATA *item_data = (ITEM_DATA *)m_ListCtrl.GetItemData (index);
	if ((item_data != NULL) && (item_data->type == TYPE_NODE)) {
		node = item_data->node;
	}

	return node;
}


////////////////////////////////////////////////////////////////////////////
//
//  Set_Node_Check
//
////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::Set_Node_Check
(
	int	/* index */,
	bool	/* onoff */
)
{
	// First off, show/hide the node
	/*NodeClass *node = Get_Item_Node (index);
	if (node != NULL) {
		node->Hide (!onoff);	
	}*/

	// Update the UI for this node
	//ListView_SetCheckState (m_ListCtrl, index, onoff);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Reset_List
//
////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::Reset_List (void)
{
	m_ClassIDStack.Delete_All ();
	Populate_List ((uint32)0);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Populate_List
//
////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::Populate_List (uint32 class_id)
{
	m_ListCtrl.SetRedraw (false);
	m_ListCtrl.DeleteAllItems ();

	if (class_id == 0) {
	
		//
		//	Add all the node categories to the list control
		//
		for (int index = 0; index < PRESET_CATEGORY_COUNT; index ++) {
			Insert_Factory (PRESET_CATEGORIES[index].name, PRESET_CATEGORIES[index].clsid);
		}		

	} else {
		Insert_Navigator ();

		//
		//	Fill all the node instances into the list control
		//
		for (	NodeClass *node = NodeMgrClass::Find_First (class_id);
				node != NULL;
				node = NodeMgrClass::Find_Next (node, class_id))
		{
			//
			//	Add this node to the list control
			//
			Insert_Node (node);
		}

		//
		//	Fill all the factory instances into the list control
		//
		for (	DefinitionFactoryClass *factory = DefinitionFactoryMgrClass::Get_First (class_id);
				factory != NULL;
				factory = DefinitionFactoryMgrClass::Get_Next (factory, class_id))
		{
			//
			//	Add this node to the list control
			//
			if (factory->Get_Class_ID () != class_id) {
				Insert_Factory (factory->Get_Name (), factory->Get_Class_ID ());
			}
		}

		//
		//	Sort all the items
		//
		m_ListCtrl.SortItems (InstancesListSortCallback, 0L);
	}

	m_ListCtrl.SetRedraw (true);

	//
	//	Save this class ID for later
	//
	m_ClassID = class_id;
	
	//
	//	Make sure the toolbar is up to date
	//
	Update_Overlays ();
	Update_Button_States ();
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Populate_List
//
////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::Populate_List (NodeClass *node)
{
	m_ListCtrl.SetRedraw (false);
	m_ListCtrl.DeleteAllItems ();

	if (node != NULL) {
		Insert_Navigator ();

		//
		//	Add all the sub-nodes to the list control
		//
		for (int index = 0; index < node->Get_Sub_Node_Count (); index ++) {
			NodeClass *sub_node = node->Get_Sub_Node (index);
			if (sub_node != NULL) {
				
				//
				//	Add this node to the list control
				//			
				Insert_Node (sub_node);
			}
		}
	}

	//
	//	Sort all the items
	//
	m_ListCtrl.SortItems (InstancesListSortCallback, 0L);
	m_ListCtrl.SetRedraw (true);

	//
	//	Make sure the toolbar is up to date
	//
	Update_Overlays ();
	Update_Button_States ();
	return ;
}



////////////////////////////////////////////////////////////////////////////
//
//  OnDeleteitemInstanceList
//
////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::OnDeleteitemInstanceList
(
	NMHDR *		pNMHDR,
	LRESULT *	pResult
) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	(*pResult) = 0;

	// Get the node associated with this entry
	NodeClass *node = Get_Item_Node (pNMListView->iItem);
	MEMBER_RELEASE (node);

	// Free the item data structure associated with this entry
	ITEM_DATA *item_data = (ITEM_DATA *)m_ListCtrl.GetItemData (pNMListView->iItem);
	SAFE_DELETE (item_data);	
	m_ListCtrl.SetItemData (pNMListView->iItem, 0L);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  InstancesListSortCallback
//
////////////////////////////////////////////////////////////////////////////
int CALLBACK
InstancesListSortCallback
(
	LPARAM lParam1,
	LPARAM lParam2,
   LPARAM /* lParamSort */
)
{
	int retval = 0;

	ITEM_DATA *item_data1 = (ITEM_DATA *)lParam1;
	ITEM_DATA *item_data2 = (ITEM_DATA *)lParam2;
	if (item_data1 != NULL && item_data2 != NULL) {
		
		//
		//	Do the types match?
		//
		if (item_data1->type == item_data2->type) {
			
			//
			//	Do a simple name comparison
			//
			retval = ::lstrcmpi (item_data1->name, item_data2->name);

		} else {

			//
			//	Sort based on type
			//
			retval = (item_data1->type - item_data2->type);
		}
	}
	
	return retval;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnItemchangedInstanceList
//
////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::OnItemchangedInstanceList
(
	NMHDR *	pNMHDR,
	LRESULT* pResult
)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;

	if (pNMListView->uChanged & LVIF_STATE) {

		//
		//	Did the selection set change?
		//
		if (pNMListView->uNewState & LVIS_SELECTED || pNMListView->uOldState & LVIS_SELECTED) {			
			Update_Button_States ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Update_Button_States
//
////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::Update_Button_States (void)
{
	//
	//	Determine if there are any nodes in the current selection set.
	//
	bool enable	= false;
	int index	= -1;
	while (!enable && (index = m_ListCtrl.GetNextItem (index, LVNI_ALL | LVNI_SELECTED)) >= 0) {
		enable |= (Get_Item_Node (index) != NULL);
	}

	//
	// Enable or disable the edit button depending on
	// whether the selected item is a category or a node
	//
	m_Toolbar.Enable_Button (IDC_DELETE, enable);
	m_Toolbar.Enable_Button (IDC_EDIT, enable);
	m_Toolbar.Enable_Button (IDC_SELECT, enable);
	return ;
}

////////////////////////////////////////////////////////////////////////////
//
//  Hide_Node
//
////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::Hide_Node (NodeClass *node, uint32 class_id, bool hide)
{
	PresetClass *preset = node->Get_Preset ();
	if (preset != NULL) {
		DefinitionClass *definition = preset->Get_Definition ();
		if (definition != NULL) {
			
			//
			//	Did this node come from the selected factory?
			//
			uint32 curr_class_id	= definition->Get_Class_ID ();
			uint32 superclass_id	= ::SuperClassID_From_ClassID (curr_class_id);
			if (curr_class_id == class_id || superclass_id == class_id) {
				
				//
				//	Change this node's display state
				//							
				node->Hide (hide);
			}
		}
	}

	//
	//	Now, check all this nodes children
	//
	for (int index = 0; index < node->Get_Sub_Node_Count (); index ++) {
		NodeClass *sub_node = node->Get_Sub_Node (index);
		if (sub_node != NULL) {
			Hide_Node (sub_node, class_id, hide);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Hide_Nodes
//
////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::Hide_Nodes (int index, bool hide)
{
	ITEM_DATA *item_data = (ITEM_DATA *)m_ListCtrl.GetItemData (index);		
	if (item_data != NULL) {
		
		if (item_data->type == TYPE_FACTORY) {

			//
			//	Loop over all the nodes in the world
			//
			for (	NodeClass *node = NodeMgrClass::Get_First ();
					node != NULL;
					node = NodeMgrClass::Get_Next (node))
			{				
				//
				//	Hide this node (if necessary)
				//
				Hide_Node (node, item_data->class_id, hide);
			}
			
		} else if (item_data->type == TYPE_NODE) {
						
			//
			//	Change this node's display state
			//
			item_data->node->Hide (hide);
		}
	}

	//
	// Make sure the main view is updated
	//
	::Refresh_Main_View ();		
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Overlays
//
/////////////////////////////////////////////////////////////////////////////
void
InstancesPageClass::Update_Overlays (void)
{
	//
	//	Loop over all the entries in the current view
	//
	for (int index = 0; index < m_ListCtrl.GetItemCount (); index ++) {
		
		//
		//	Get information about this entry
		//
		ITEM_DATA *item_data = (ITEM_DATA *)m_ListCtrl.GetItemData (index);
		if (item_data != NULL) {
			
			bool needs_overlay = false;

			//
			//	Check to see if this entry has any sub-entries
			//
			if (item_data->type == TYPE_FACTORY) {
				needs_overlay = Does_Factory_Have_Children (item_data->class_id);				
			} else if (item_data->type == TYPE_NODE && (item_data->node->Get_Sub_Node_Count () > 0)) {
				needs_overlay = true;
			}

			//
			//	Set the appropriate overlay
			//
			ListView_SetOverlay (m_ListCtrl, index, needs_overlay ? 1 : 0);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Does_Factory_Have_Children
//
/////////////////////////////////////////////////////////////////////////////
bool
InstancesPageClass::Does_Factory_Have_Children (uint32 factory_id)
{
	bool retval = false;

	//
	//	Check to see if there are any instances of this factory
	// in the world
	//
	for (	NodeClass *node = NodeMgrClass::Get_First ();
			node != NULL && retval == false;
			node = NodeMgrClass::Get_Next (node))
	{
		PresetClass *preset = node->Get_Preset ();
		if (preset != NULL) {
			DefinitionClass *definition = preset->Get_Definition ();
			if (definition != NULL) {
				
				//
				//	Did this node come from the specified factory?
				//
				uint32 class_id		= definition->Get_Class_ID ();
				uint32 superclass_id	= ::SuperClassID_From_ClassID (class_id);
				if (class_id == factory_id || superclass_id == factory_id) {
					retval = true;
				}
			}
		}
	}

	//
	//	Check to see if this factory has any sub-factories
	//
	if (retval == false) {
		DefinitionFactoryClass *factory = DefinitionFactoryMgrClass::Get_First (factory_id);
		if (factory != NULL && factory->Get_Class_ID () != factory_id) {
			retval = true;
		}
	}

	return retval;
}
