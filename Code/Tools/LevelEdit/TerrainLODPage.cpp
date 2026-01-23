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

// TerrainLODPage.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "TerrainLODPage.h"
#include "LODSettingsDialog.H"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
//	Constants
//
/////////////////////////////////////////////////////////////////////////////
static const int COL_NAME		= 0;
static const int COL_DISTANCE	= 1;

/////////////////////////////////////////////////////////////////////////////
//
// TerrainLODPageClass
//
/////////////////////////////////////////////////////////////////////////////
TerrainLODPageClass::TerrainLODPageClass (void)
	:	m_bFinishedInit (false),
		DockableFormClass(TerrainLODPageClass::IDD)
{
	//{{AFX_DATA_INIT(TerrainLODPageClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~TerrainLODPageClass
//
/////////////////////////////////////////////////////////////////////////////
TerrainLODPageClass::~TerrainLODPageClass (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
TerrainLODPageClass::DoDataExchange (CDataExchange *pDX)
{
	DockableFormClass::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TerrainLODPageClass)
	DDX_Control(pDX, IDC_LOD_LIST, m_LODListCtrl);
	DDX_Control(pDX, IDC_LOD_COUNT_SPIN, m_LODCountSpin);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(TerrainLODPageClass, DockableFormClass)
	//{{AFX_MSG_MAP(TerrainLODPageClass)
	ON_NOTIFY(NM_DBLCLK, IDC_LOD_LIST, OnDblclkLodList)
	ON_EN_UPDATE(IDC_LOD_COUNT_EDIT, OnUpdateLodCountEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TerrainLODPageClass message handlers


/////////////////////////////////////////////////////////////////////////////
//
// HandleInitDialog
//
/////////////////////////////////////////////////////////////////////////////
void
TerrainLODPageClass::HandleInitDialog (void)
{	
	//
	//	Setup the lod-count controls
	//
	SetDlgItemInt (IDC_LOD_COUNT_EDIT, 0);
	m_LODCountSpin.SetRange (0, 10);
	m_LODCountSpin.SetPos (m_SettingsList.Count ());

	//
	//	Setup the list control
	//
	m_LODListCtrl.InsertColumn (COL_NAME, "Level");
	m_LODListCtrl.InsertColumn (COL_DISTANCE, "Switch Distance");
	
	CRect rect;
	m_LODListCtrl.GetClientRect (&rect);
	m_LODListCtrl.SetColumnWidth (COL_NAME, rect.Width () >> 1);
	m_LODListCtrl.SetColumnWidth (COL_DISTANCE, rect.Width () >> 1);

	//
	//	Loop through all the distances and add them to the list control.
	//
	for (int index = 0; index < m_SettingsList.Count (); index ++) {
		unsigned int distance = m_SettingsList[index];
		
		CString text;
		text.Format ("LOD %d", index + 1);
		int real_index = m_LODListCtrl.InsertItem (index + 1, text);

		if (real_index >= 0) {
			text.Format ("%d meters", distance);
			m_LODListCtrl.SetItemText (real_index, COL_DISTANCE, text);
			m_LODListCtrl.SetItemData (real_index, distance);
		}

	}

	m_bFinishedInit = true;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Apply_Changes
//
/////////////////////////////////////////////////////////////////////////////
bool
TerrainLODPageClass::Apply_Changes (void)
{
	m_SettingsList.Delete_All ();

	//
	//	Build a list of switching distances (could just be an array, but I'm feeling lazy).
	//
	for (int index = 0; index < m_LODListCtrl.GetItemCount (); index ++) {
		m_SettingsList.Add ((unsigned int)m_LODListCtrl.GetItemData (index));
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDblclkLodList
//
/////////////////////////////////////////////////////////////////////////////
void
TerrainLODPageClass::OnDblclkLodList
(
	NMHDR * /* pNMHDR */,
	LRESULT *pResult
) 
{
	(*pResult) = 0;

	//
	//	Get the index of the selected item
	//
	int index = m_LODListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (index >= 0) {
		unsigned int distance = (unsigned int)m_LODListCtrl.GetItemData (index);

		//
		//	Display the dialog
		//
		LODSettingsDialogClass dialog (distance);
		if (dialog.DoModal () == IDOK) {
			distance = dialog.Get_Distance ();

			//
			//	Pass the new distance information onto the list control
			//
			CString text;
			text.Format ("%d meters", distance);
			m_LODListCtrl.SetItemText (index, COL_DISTANCE, text);
			m_LODListCtrl.SetItemData (index, distance);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// On_Count_Change
//
/////////////////////////////////////////////////////////////////////////////
void
TerrainLODPageClass::On_Count_Change (void)
{
	if (m_bFinishedInit) {

		//
		//	Get the new count
		//
		int new_lod_count = GetDlgItemInt (IDC_LOD_COUNT_EDIT);
		new_lod_count = std::max (0, new_lod_count);	
		
		//
		//	Get the current count
		//
		int current_count = m_LODListCtrl.GetItemCount ();

		//
		//	Determine if we should add or remove entries
		//
		if (new_lod_count < current_count) {
			
			//
			//	Remove the unnecessary entries from the list control
			//
			for (int index = new_lod_count; index < current_count; index ++) {
				m_LODListCtrl.DeleteItem (new_lod_count);
			}

		} else if (new_lod_count > current_count) {
			
			//
			//	Add the new entries to the list control
			//
			for (int index = current_count; index < new_lod_count; index ++) {
				CString name;
				name.Format ("LOD %d", index + 1);
				int real_index = m_LODListCtrl.InsertItem (index + 1, name);
				m_LODListCtrl.SetItemText (real_index, COL_DISTANCE, "0 meters");
				m_LODListCtrl.SetItemData (real_index, 0);
			}		
		}
	}

	return ;
}

/////////////////////////////////////////////////////////////////////////////
//
// OnUpdateLodCountEdit
//
/////////////////////////////////////////////////////////////////////////////
void
TerrainLODPageClass::OnUpdateLodCountEdit (void) 
{
	On_Count_Change ();
	return ;
}

