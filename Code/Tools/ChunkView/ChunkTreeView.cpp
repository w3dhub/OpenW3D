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
 *                 Project Name : ChunkView                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/ChunkView/ChunkTreeView.cpp            $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 9/28/99 9:48a                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


// ChunkTreeView.cpp : implementation file
//

#include "stdafx.h"
#include "ChunkView.h"
#include "ChunkTreeView.h"
#include "ChunkViewDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChunkTreeView

IMPLEMENT_DYNCREATE(CChunkTreeView, CTreeView)

CChunkTreeView::CChunkTreeView()
{
}

CChunkTreeView::~CChunkTreeView()
{
}


BEGIN_MESSAGE_MAP(CChunkTreeView, CTreeView)
	//{{AFX_MSG_MAP(CChunkTreeView)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChunkTreeView drawing

void CChunkTreeView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CChunkTreeView diagnostics

#ifdef _DEBUG
void CChunkTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CChunkTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CChunkTreeView message handlers

void CChunkTreeView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// Reset the entire tree view
	CTreeCtrl &tree = GetTreeCtrl();
	tree.DeleteAllItems();
	
	// Set the style attributes
	int flags = tree.GetStyle();
	flags |= TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP;
	SetWindowLong(tree.GetSafeHwnd(), GWL_STYLE, flags);

	// Get the chunk tree
	CChunkViewDoc * doc = (CChunkViewDoc *)GetDocument();
	const ChunkImageClass * chunk = doc->Get_File_Image().Get_Root();

	// Build the tree control to reflect the chunk tree
	for (int i=0; i<chunk->Get_Sibling_Count(); i++) {
		Insert_Chunk(chunk->Get_Sibling(i));
	}
}

void CChunkTreeView::Insert_Chunk(const ChunkImageClass * chunk, HTREEITEM Parent)
{
	char name[256];
	sprintf(name,"Chunk ID: 0x%08X  Length: %d",chunk->Get_ID(),chunk->Get_Length());

	CTreeCtrl &tree = GetTreeCtrl();
	HTREEITEM tree_item = tree.InsertItem(name, Parent);
	tree.SetItem(tree_item, TVIF_PARAM,0,0,0,0,0, (int) chunk);

	for (int i=0; i<chunk->Get_Child_Count(); i++) {
		Insert_Chunk(chunk->Get_Child(i),tree_item);
	}
}


void CChunkTreeView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	CChunkViewDoc * doc = (CChunkViewDoc *)GetDocument();
	doc->Set_Cur_Chunk((ChunkImageClass *) pNMTreeView->itemNew.lParam);

	doc->UpdateAllViews(this);  // update all of the other views
	*pResult = 0;
}
