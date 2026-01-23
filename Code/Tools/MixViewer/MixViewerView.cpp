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

// MixViewerView.cpp : implementation of the CMixViewerView class
//

#include "StdAfx.h"
#include "MixViewer.h"

#include "mixviewerdoc.h"
#include "mixviewerview.h"
#include "ffactory.h"
#include "mixfile.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//	Local prototypes
/////////////////////////////////////////////////////////////////////////////
static int CALLBACK MixFilenamesListSortCallback (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);


/////////////////////////////////////////////////////////////////////////////
// CMixViewerView

IMPLEMENT_DYNCREATE(CMixViewerView, CListView)

BEGIN_MESSAGE_MAP(CMixViewerView, CListView)
	//{{AFX_MSG_MAP(CMixViewerView)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnDeleteitem)
	ON_WM_WINDOWPOSCHANGING()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
//	CMixViewerView
//
/////////////////////////////////////////////////////////////////////////////
CMixViewerView::CMixViewerView (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	~CMixViewerView
//
/////////////////////////////////////////////////////////////////////////////
CMixViewerView::~CMixViewerView (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	PreCreateWindow
//
/////////////////////////////////////////////////////////////////////////////
BOOL
CMixViewerView::PreCreateWindow (CREATESTRUCT &cs)
{
	cs.style |= LVS_REPORT;

	return CListView::PreCreateWindow (cs);
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnDraw
//
/////////////////////////////////////////////////////////////////////////////
void
CMixViewerView::OnDraw (CDC * /* pDC */)
{
	CMixViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnInitialUpdate
//
/////////////////////////////////////////////////////////////////////////////
void
CMixViewerView::OnInitialUpdate (void)
{
	CListView::OnInitialUpdate ();

	//
	//	Size the column
	//
	CRect rect;
	GetListCtrl ().GetClientRect (&rect);
	rect.right -= ::GetSystemMetrics (SM_CXVSCROLL) + 2;
	GetListCtrl ().SetColumnWidth (0, rect.Width ());
	return ;
}


/////////////////////////////////////////////////////////////////////////////
// CMixViewerView diagnostics

#ifdef _DEBUG
void CMixViewerView::AssertValid() const
{
	CListView::AssertValid();
}

void CMixViewerView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CMixViewerDoc* CMixViewerView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMixViewerDoc)));
	return (CMixViewerDoc*)m_pDocument;
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
//
//	Reload
//
/////////////////////////////////////////////////////////////////////////////
void
CMixViewerView::Reload (const char *filename)
{
	CurrentFilename = filename;

	CMixViewerDoc *document = GetDocument ();
	if (document == NULL) {
		return ;
	}

	//
	//	Start fresh
	//
	GetListCtrl ().DeleteAllItems ();

	//
	//	Get the filename list from the mix file
	//
	MixFileFactoryClass mix_factory (filename, _TheFileFactory);
	if (mix_factory.Build_Filename_List (FilenameList)) {
	
		//
		//	Add each filename to the list
		//
		for (int index = 0; index < FilenameList.Count (); index ++) {
			int item_index = GetListCtrl ().InsertItem (index, FilenameList[index]);
			if (item_index != -1) {
				GetListCtrl ().SetItemData (item_index, (DWORD_PTR)new StringClass (FilenameList[index]));
			}
		}

		//
		//	Sort the data
		//
		GetListCtrl ().SortItems (MixFilenamesListSortCallback, 0);

	} else {
		
		//
		//	Notify the user
		//
		StringClass message;
		message.Format ("Error reading the filename list from %s.", filename);
		MessageBox (message, "Mix File Error", MB_ICONERROR | MB_OK);
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Reset
//
/////////////////////////////////////////////////////////////////////////////
void
CMixViewerView::Reset (void)
{
	GetListCtrl ().DeleteAllItems ();
	FilenameList.Delete_All ();
	CurrentFilename = "";
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnCreate
//
/////////////////////////////////////////////////////////////////////////////
int
CMixViewerView::OnCreate (LPCREATESTRUCT lpCreateStruct) 
{
	if (CListView::OnCreate (lpCreateStruct) == -1)	{
		return -1;
	}

	//
	//	Configure the list control
	//
	GetListCtrl ().SetExtendedStyle (GetListCtrl ().GetExtendedStyle () | LVS_EX_FULLROWSELECT);
	GetListCtrl ().InsertColumn (0, "Filename");
	return 0;
}


////////////////////////////////////////////////////////////////////////////
//
//  MixFilenamesListSortCallback
//
////////////////////////////////////////////////////////////////////////////
int CALLBACK
MixFilenamesListSortCallback
(
	LPARAM lParam1,
	LPARAM lParam2,
   LPARAM /* lParamSort */
)
{
	int retval = 0;

	//
	//	Get the data from list control
	//
	StringClass *item_data1 = (StringClass *)lParam1;
	StringClass *item_data2 = (StringClass *)lParam2;
	if (item_data1 != NULL && item_data2 != NULL) {

		
		bool is_1_dir = (::strrchr (*item_data1, '\\') != NULL);
		bool is_2_dir = (::strrchr (*item_data2, '\\') != NULL);

		if (is_1_dir && is_2_dir == false) {
			retval = -1;
		} else if (is_1_dir == false && is_2_dir) {
			retval = 1;
		} else {
		
			//
			//	Do a simple string compare
			//
			retval = item_data1->Compare_No_Case (*item_data2);
		}
	}
	
	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnDeleteitem
//
/////////////////////////////////////////////////////////////////////////////
void
CMixViewerView::OnDeleteitem (NMHDR *pNMHDR, LRESULT *pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW *)pNMHDR;
	*pResult = 0;

	//
	//	Get the string associated with this entry
	//
	StringClass *string = (StringClass *)GetListCtrl ().GetItemData (pNMListView->iItem);
	GetListCtrl ().SetItemData (pNMListView->iItem, 0);

	//
	//	Free the string
	//
	if (string != NULL) {
		delete string;
		string = NULL;
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnWindowPosChanging
//
/////////////////////////////////////////////////////////////////////////////
void
CMixViewerView::OnWindowPosChanging (WINDOWPOS FAR *lpwndpos) 
{
	if ((lpwndpos->flags & SWP_NOSIZE) == 0) {
		
		//
		//	Get the current percent of the column width
		//
		CRect rect;
		GetListCtrl ().GetWindowRect (&rect);
		int curr_width	= GetListCtrl ().GetColumnWidth (0);
		float percent	= (float)curr_width / (float)rect.Width ();

		//
		//	Size the column
		//
		int new_width = int(lpwndpos->cx * percent);
		GetListCtrl ().SetColumnWidth (0, new_width);
	}

	CListView::OnWindowPosChanging (lpwndpos);
	return ;
}
