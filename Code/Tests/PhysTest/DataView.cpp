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

// DataView.cpp : implementation file
//

#include "stdafx.h"
#include "phystest.h"
#include "DataView.h"
#include "PhysTestDoc.h"
#include "assetmgr.h"
#include "rendobj.h"
#include "pscene.h"
#include "phys.h"
#include "physlist.h"
#include "mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// CDataView

IMPLEMENT_DYNCREATE(CDataView, CTreeView)

CDataView::CDataView() :
	ModelsRoot(NULL),
	InstancesRoot(NULL)
{
}

CDataView::~CDataView()
{
}


void CDataView::Rebuild_Tree(void)
{
	// Turn off repainting
	GetTreeCtrl ().SetRedraw (false);

	// wipe clean
	GetTreeCtrl().DeleteItem(ModelsRoot);
	GetTreeCtrl().DeleteItem(InstancesRoot);

	ModelsRoot = GetTreeCtrl().InsertItem("Models", NULL, NULL);	
	InstancesRoot = GetTreeCtrl().InsertItem ("Physics Object Instances", NULL, NULL);
	
	// MODELS
	// Get an iterator from the asset manager that we can
	// use to enumerate the currently loaded assets
	RenderObjIterator * obj_iterator = WW3DAssetManager::Get_Instance()->Create_Render_Obj_Iterator();
	ASSERT(obj_iterator);

	if (obj_iterator) {

		// Loop through all the assets in the manager
		for (obj_iterator->First (); !obj_iterator->Is_Done(); obj_iterator->Next()) {

			// If the asset is an HLOD, add it to our possible model list
			if (obj_iterator->Current_Item_Class_ID() == RenderObjClass::CLASSID_HLOD) {

				const char * model_name = obj_iterator->Current_Item_Name();

				// Add this entry to the tree
				HTREEITEM hItem = GetTreeCtrl().InsertItem(model_name,NULL,0,ModelsRoot,TVI_SORT);
				ASSERT (hItem != NULL);
		
				ItemInfoClass * item_info = new ItemInfoClass(model_name,ItemInfoClass::MODEL);
				GetTreeCtrl().SetItemData(hItem, (ULONG)item_info);
			}
		}

		// Free the enumerator object we created earlier
		WW3DAssetManager::Get_Instance()->Release_Render_Obj_Iterator(obj_iterator);
	}

	// PHYSICS OBJECT INSTANCES
	CPhysTestDoc * doc = (CPhysTestDoc *)GetDocument();	
	
	RefPhysListIterator phys_iterator = PhysicsSceneClass::Get_Instance()->Get_Dynamic_Object_Iterator();
	for (phys_iterator.First();!phys_iterator.Is_Done();phys_iterator.Next()) {
		
		const char * instance_name = phys_iterator.Peek_Obj()->Get_Name();

		if (instance_name != NULL) {

			// Add this entry to the tree
			HTREEITEM hItem = GetTreeCtrl().InsertItem(instance_name,NULL,0,InstancesRoot,TVI_SORT);
			ASSERT (hItem != NULL);

			ItemInfoClass * item_info = new ItemInfoClass(instance_name,ItemInfoClass::INSTANCE);
			item_info->Instance = phys_iterator.Peek_Obj();
			GetTreeCtrl().SetItemData(hItem, (ULONG)item_info);
		}
	}
	
	// Turn;repainting back on and force a redraw
	GetTreeCtrl().SetRedraw (true);
	Invalidate(false);
	UpdateWindow();
}

ItemInfoClass * CDataView::Get_Selected_Item(void)
{
	ItemInfoClass * item_info = NULL;

	// Get the currently selected node from the tree control
	HTREEITEM htree_item = GetTreeCtrl ().GetSelectedItem ();
	if (htree_item != NULL) {

		// Get the data associated with this item
		item_info = (ItemInfoClass *)GetTreeCtrl().GetItemData(htree_item);
	}

	// Return the asset information associated with the current item
	return item_info;
}

void CDataView::Save(ChunkSaveClass & csave)
{
}

void CDataView::Load(ChunkLoadClass & cload)
{
}


BEGIN_MESSAGE_MAP(CDataView, CTreeView)
	//{{AFX_MSG_MAP(CDataView)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(TVN_DELETEITEM, OnDeleteitem)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDataView drawing

void CDataView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CDataView diagnostics

#ifdef _DEBUG
void CDataView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CDataView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CDataView message handlers

BOOL CDataView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// Modify the style bits for the window so it will
	// have buttons and lines between nodes.
	cs.style |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS;
	return CTreeView::PreCreateWindow(cs);
}

int CDataView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	ModelsRoot = GetTreeCtrl().InsertItem("Models", NULL, NULL);	
	InstancesRoot = GetTreeCtrl().InsertItem ("Physics Object Instances", NULL, NULL);
	
	return 0;
}

void CDataView::OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
	ItemInfoClass * item_info = (ItemInfoClass *)pNMTreeView->itemOld.lParam;

	// Free the asset information object
	if (item_info != NULL) {
		delete item_info;
	}

	// Reset the data associated with this entry
	GetTreeCtrl().SetItemData(pNMTreeView->itemOld.hItem, NULL);

	*pResult = 0;
}

void CDataView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// just tell the main window that the selection changed so it
	// can link/unlink the virtual joystick.
	((CMainFrame *)::AfxGetMainWnd())->Notify_Selection_Changed();
	*pResult = 0;
}

