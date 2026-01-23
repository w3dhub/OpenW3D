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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/OverlapPage.cpp      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 4/11/00 5:48p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "LevelEdit.h"
#include "OverlapPage.h"
#include "nodemgr.h"
#include "node.h"
#include "coltest.h"
#include "icons.h"
#include "cameramgr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Local constants
/////////////////////////////////////////////////////////////////////////////
static const int TOOLBAR_HEIGHT		= 25;
static const int TOOLBAR_V_SPACING	= 5;
static const int TOOLBAR_V_BORDER	= TOOLBAR_V_SPACING * 2;
static const int TOOLBAR_H_SPACING	= 5;
static const int TOOLBAR_H_BORDER	= TOOLBAR_H_SPACING * 2;



/////////////////////////////////////////////////////////////////////////////
//
// OverlapPageClass
//
/////////////////////////////////////////////////////////////////////////////
OverlapPageClass::OverlapPageClass(CWnd* pParent /*=NULL*/)
	:	CDialog(OverlapPageClass::IDD, pParent),
		m_SoundsRoot (NULL),
		m_LightsRoot (NULL)
{
	//{{AFX_DATA_INIT(OverlapPageClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	Create (OverlapPageClass::IDD, pParent);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~OverlapPageClass
//
/////////////////////////////////////////////////////////////////////////////
OverlapPageClass::~OverlapPageClass (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
OverlapPageClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(OverlapPageClass)
	DDX_Control(pDX, IDC_OVERLAP_TREE, m_TreeCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(OverlapPageClass, CDialog)
	//{{AFX_MSG_MAP(OverlapPageClass)
	ON_NOTIFY(TVN_DELETEITEM, IDC_OVERLAP_TREE, OnDeleteitemOverlapTree)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_COMMAND(IDC_REFRESH, OnRefresh)
	ON_NOTIFY(NM_DBLCLK, IDC_OVERLAP_TREE, OnDblclkOverlapTree)
	ON_NOTIFY(TVN_ITEMEXPANDED, IDC_OVERLAP_TREE, OnItemexpandedOverlapTree)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
//
// OnSize
//
/////////////////////////////////////////////////////////////////////////////
void
OverlapPageClass::OnSize
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
OverlapPageClass::OnDestroy (void) 
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
OverlapPageClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	m_Toolbar.CreateEx (this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_ALIGN_TOP, CRect(0, 0, 0, 0), 101);
	m_Toolbar.SetOwner (this);
	m_Toolbar.LoadToolBar (IDR_OVERLAP_TOOLBAR);
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

	Reset_Tree ();
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeleteitemOverlapTree
//
/////////////////////////////////////////////////////////////////////////////
void
OverlapPageClass::OnDeleteitemOverlapTree
(
	NMHDR *	/* pNMHDR */,
	LRESULT *pResult
) 
{
	(*pResult) = 0;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Build_Tree
//
/////////////////////////////////////////////////////////////////////////////
void
OverlapPageClass::Build_Tree (void)
{
	Reset_Tree ();

	//
	//	Fill the sub-nodes
	//
	Detect_Overlaps (m_SoundsRoot, NODE_TYPE_SOUND);
	Detect_Overlaps (m_LightsRoot, NODE_TYPE_LIGHT);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Reset_Tree
//
/////////////////////////////////////////////////////////////////////////////
void
OverlapPageClass::Reset_Tree (void)
{
	m_TreeCtrl.DeleteAllItems ();
	
	//
	//	Insert the root nodes
	//
	m_SoundsRoot = m_TreeCtrl.InsertItem ("Sounds", FOLDER_ICON, FOLDER_ICON);
	m_LightsRoot = m_TreeCtrl.InsertItem ("Lights", FOLDER_ICON, FOLDER_ICON);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Detect_Overlaps
//
/////////////////////////////////////////////////////////////////////////////
void
OverlapPageClass::Detect_Overlaps (HTREEITEM root_item, NODE_TYPE node_type)
{
	//
	//	Loop over each node (of the given type) in the world
	//
	NodeClass *node1 = NULL;
	for (	node1 = NodeMgrClass::Get_First (node_type);
			node1 != NULL;
			node1 = NodeMgrClass::Get_Next (node1, node_type))
	{
		//
		//	Check this node against every other node (of the sametype) in the world
		//
		HTREEITEM tree_item		= NULL;
		NodeClass *node2	= NULL;
		for (	node2 = NodeMgrClass::Get_First (node_type);
				node2 != NULL;
				node2 = NodeMgrClass::Get_Next (node2, node_type))
		{
			if (node1 != node2) {
				
				SphereClass sphere1 (node1->Get_Position (), node1->Get_Attenuation_Radius ());
				SphereClass sphere2 (node2->Get_Position (), node2->Get_Attenuation_Radius ());

				//
				//	Do these spheres intersect?
				//
				if (CollisionMath::Overlap_Test (sphere1, sphere2) != CollisionMath::OUTSIDE) {
					
					//
					//	Add a tree entry for this node (if necessary)
					//
					if (tree_item == NULL) {
						tree_item = Insert_Node (root_item, node1);
					}

					//
					//	Add this node as a sub-entry under the tree
					//
					Insert_Node (tree_item, node2);
				}
			}
		}		
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Insert_Node
//
/////////////////////////////////////////////////////////////////////////////
HTREEITEM
OverlapPageClass::Insert_Node (HTREEITEM parent_item, NodeClass *node)
{
	//
	//	Add this node to the tree
	//
	int icon_index			= node->Get_Icon_Index ();
	HTREEITEM tree_item	= m_TreeCtrl.InsertItem (	node->Get_Name (),
																	icon_index,
																	icon_index,
																	parent_item);
	
	//
	//	Save the node's ID with the tree entry so we can look it up later...
	//
	if (tree_item != NULL) {
		m_TreeCtrl.SetItemData (tree_item, node->Get_ID ());
	}

	return tree_item;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnRefresh
//
/////////////////////////////////////////////////////////////////////////////
void
OverlapPageClass::OnRefresh (void)
{
	Build_Tree ();
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnDblclkOverlapTree
//
////////////////////////////////////////////////////////////////////////////
void
OverlapPageClass::OnDblclkOverlapTree
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
		//	Lookup the node from its ID
		//
		int node_id = m_TreeCtrl.GetItemData (tree_item);
		NodeClass *node = NodeMgrClass::Find_Node (node_id);
		if (node != NULL) {
			
			//
			//	Reposition the camera so its looking at the node
			//
			::Get_Camera_Mgr ()->Goto_Node (node);
		}
	}
		
	(*pResult) = 0;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnItemexpandedOverlapTree
//
////////////////////////////////////////////////////////////////////////////
void
OverlapPageClass::OnItemexpandedOverlapTree
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
	uint32 node_id = m_TreeCtrl.GetItemData (pNMTreeView->itemNew.hItem);
	if (node_id == 0) {
		if (pNMTreeView->itemNew.state & TVIS_EXPANDED) {
			m_TreeCtrl.SetItemImage (pNMTreeView->itemNew.hItem, OPEN_FOLDER_ICON, OPEN_FOLDER_ICON);
		} else {
			m_TreeCtrl.SetItemImage (pNMTreeView->itemNew.hItem, FOLDER_ICON, FOLDER_ICON);
		}
	}

	(*pResult) = 0;
	return ;
}
