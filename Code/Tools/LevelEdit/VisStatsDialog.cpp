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

// VisStatsDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "VisStatsDialog.h"
#include "cameramgr.h"
#include "Utils.h"
#include "pscene.h"
#include "staticphys.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////
enum
{
	COL_NAME					= 0,
	COL_POLY_COUNT,
	COL_TEXTURE_COUNT,
	COL_TEXTURE_KB,
};


/////////////////////////////////////////////////////////////////////////////
// Local prototypes
/////////////////////////////////////////////////////////////////////////////
static int CALLBACK VisSectorSortCompareFn (LPARAM param1, LPARAM param2, LPARAM column_id);


/////////////////////////////////////////////////////////////////////////////
//
// VisStatsDialogClass
//
/////////////////////////////////////////////////////////////////////////////
VisStatsDialogClass::VisStatsDialogClass(CWnd* pParent /*=NULL*/)
	:	m_CurrentColSort (COL_NAME),
		m_AscendingSort (true),
		CDialog(VisStatsDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(VisStatsDialogClass)
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
VisStatsDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VisStatsDialogClass)
	DDX_Control(pDX, IDC_SECTOR_LIST, m_SectorList);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(VisStatsDialogClass, CDialog)
	//{{AFX_MSG_MAP(VisStatsDialogClass)
	ON_NOTIFY(NM_DBLCLK, IDC_SECTOR_LIST, OnDblclkSectorList)
	ON_NOTIFY(LVN_DELETEITEM, IDC_SECTOR_LIST, OnDeleteitemSectorList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_SECTOR_LIST, OnColumnclickSectorList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
VisStatsDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	//
	// Setup the columns
	//
	m_SectorList.InsertColumn (COL_NAME, "Name");
	m_SectorList.InsertColumn (COL_POLY_COUNT, "Polys");
	m_SectorList.InsertColumn (COL_TEXTURE_COUNT, "Textures");
	m_SectorList.InsertColumn (COL_TEXTURE_KB, "Texture KB");

	//
	//	Size the columns
	//
	CRect rect;
	m_SectorList.GetClientRect (&rect);
	int width1 = rect.Width () / 2;
	int width2 = width1 / 3;
	m_SectorList.SetColumnWidth (COL_NAME,				width1);
	m_SectorList.SetColumnWidth (COL_POLY_COUNT,		width2);
	m_SectorList.SetColumnWidth (COL_TEXTURE_COUNT,	width2);
	m_SectorList.SetColumnWidth (COL_TEXTURE_KB,		width2);

	//
	//	Build a list of vis statistics
	//
	DynamicVectorClass<VisSectorStatsClass> stats_list;
	PhysicsSceneClass::Get_Instance ()->Generate_Vis_Statistics_Report (stats_list);	

	//
	//	Populate the list control with the vis statistics
	//
	for (int index = 0; index < stats_list.Count (); index ++) {
		VisSectorStatsClass &sector_stats = stats_list[index];
				
		LPCTSTR name = sector_stats.Get_Name ();
		if (name != NULL) {
			
			//
			//	Add this sector to the list
			//
			int item_index = m_SectorList.InsertItem (0, name);
			if (item_index >= 0) {
				
				//
				//	Set the columns of information for this sector
				//
				CString poly_count;
				poly_count.Format ("%d polys", sector_stats.Get_Polygon_Count ());
				m_SectorList.SetItemText (item_index, COL_POLY_COUNT, poly_count);

				CString texture_count;
				texture_count.Format ("%d textures", sector_stats.Get_Texture_Count ());
				m_SectorList.SetItemText (item_index, COL_TEXTURE_COUNT, texture_count);

				CString texture_size;
				texture_size.Format ("%d KB", sector_stats.Get_Texture_Bytes () / 1024);
				m_SectorList.SetItemText (item_index, COL_TEXTURE_KB, texture_size);

				//
				//	Add the goto-point to the item
				//
				m_SectorList.SetItemData (item_index, (DWORD_PTR)(new VisSectorStatsClass(sector_stats)));
			}
		}
	}	
	
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDblclkSectorList
//
/////////////////////////////////////////////////////////////////////////////
void
VisStatsDialogClass::OnDblclkSectorList
(
	NMHDR*	/* pNMHDR */,
	LRESULT* pResult
)
{
	(*pResult) = 0;

	//
	// Determine what client-coord location was double-clicked on
	//
	DWORD mouse_pos = ::GetMessagePos ();
	POINT hit_point = { GET_X_LPARAM (mouse_pos), GET_Y_LPARAM (mouse_pos) };
	m_SectorList.ScreenToClient (&hit_point);

	//
	// Goto the sector that was double-clicked on (if possible)
	//
	UINT flags = 0;
	int index = m_SectorList.HitTest (hit_point, &flags);
	if ((index >= 0) && ((flags & LVHT_ONITEMLABEL) || (flags & LVHT_ONITEMICON))) {
		
		//
		//	Snap the camera to the point
		//
		VisSectorStatsClass *stats = (VisSectorStatsClass *)m_SectorList.GetItemData (index);
		if (stats != NULL) {
			::Get_Camera_Mgr ()->Set_Position (stats->Get_Position ());
			::Refresh_Main_View ();
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeleteitemSectorList
//
/////////////////////////////////////////////////////////////////////////////
void
VisStatsDialogClass::OnDeleteitemSectorList
(
	NMHDR *	pNMHDR,
	LRESULT* pResult
) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	(*pResult) = 0;

	//
	//	Free the vector3 we kept around for this sector
	//
	VisSectorStatsClass *stats = NULL;
	stats = (VisSectorStatsClass *)m_SectorList.GetItemData (pNMListView->iItem);
	SAFE_DELETE (stats);
	m_SectorList.SetItemData (pNMListView->iItem, NULL);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnColumnclickSectorList
//
/////////////////////////////////////////////////////////////////////////////
void
VisStatsDialogClass::OnColumnclickSectorList
(
	NMHDR*	pNMHDR,
	LRESULT* pResult
)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	(*pResult) = 0;

	if (m_CurrentColSort == pNMListView->iSubItem) {
		m_AscendingSort = !m_AscendingSort;
	} else {
		m_CurrentColSort = pNMListView->iSubItem;
		m_AscendingSort = true;
	}

	m_SectorList.SortItems (VisSectorSortCompareFn, MAKELONG (m_CurrentColSort, m_AscendingSort));
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// VisSectorSortCompareFn
//
/////////////////////////////////////////////////////////////////////////////
int CALLBACK
VisSectorSortCompareFn (LPARAM param1, LPARAM param2, LPARAM sort_info)
{
	int retval = 0;
	VisSectorStatsClass *stats1 = (VisSectorStatsClass *)param1;
	VisSectorStatsClass *stats2 = (VisSectorStatsClass *)param2;

	LONG column_id = LOWORD (sort_info);
	bool ascending	= HIWORD (sort_info) != 0;

	if (stats1 != NULL && stats2 != NULL) {
		
		//
		//	Determine the order based on which column the user clicked on
		//
		if (column_id == COL_NAME) {
			retval = ::strcmpi (stats1->Get_Name (), stats2->Get_Name ());
		} else if (column_id == COL_POLY_COUNT) {
			retval = stats1->Get_Polygon_Count () - stats2->Get_Polygon_Count ();
		} else if (column_id == COL_TEXTURE_COUNT) {
			retval = stats1->Get_Texture_Count () - stats2->Get_Texture_Count ();
		} else if (column_id == COL_TEXTURE_KB) {
			retval = stats1->Get_Texture_Bytes () - stats2->Get_Texture_Bytes ();
		}
	}

	//
	//	Invert the sort if necessary
	//
	if (!ascending) {
		retval = -retval;
	}

	return retval;
}
