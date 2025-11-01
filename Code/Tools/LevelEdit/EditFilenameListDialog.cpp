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

// EditFilenameListDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "editfilenamelistdialog.h"
#include "Utils.h"
#include "filemgr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// EditFilenameListDialogClass
//
/////////////////////////////////////////////////////////////////////////////
EditFilenameListDialogClass::EditFilenameListDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(EditFilenameListDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(EditFilenameListDialogClass)
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
EditFilenameListDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EditFilenameListDialogClass)
	DDX_Control(pDX, IDC_FILENAME_LIST_CTRL, m_ListCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(EditFilenameListDialogClass, CDialog)
	//{{AFX_MSG_MAP(EditFilenameListDialogClass)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_NOTIFY(LVN_KEYDOWN, IDC_FILENAME_LIST_CTRL, OnKeydownFilenameListCtrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnAdd
//
/////////////////////////////////////////////////////////////////////////////
void
EditFilenameListDialogClass::OnAdd (void)
{
	CFileDialog dialog (	true,
								".w3d",
								NULL,
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_ALLOWMULTISELECT,
								"Westwood 3D Files (*.w3d)|*.w3d|Texture Files (*.tga)|*.tga|Sound Files (*.wav,*.mp3)|*.wav;*.mp3|All Files|*.*||",
								this);

    TCHAR filename_list[MAX_PATH*20] = { 0 };
    dialog.m_ofn.lpstrFile	= filename_list;
    dialog.m_ofn.nMaxFile	= sizeof (filename_list);

	//
	//	Start the user off in the assets directory
	//
	dialog.m_ofn.lpstrInitialDir = ::Get_File_Mgr ()->Get_Base_Path ();

	//
	// Ask the user to select the files
	//
	if (dialog.DoModal () == IDOK) {
		
      //
		// Add all the new filenames to the list
		//
		POSITION pos = dialog.GetStartPosition (); 
		while (pos != NULL) {			
			CString full_path = (LPCTSTR)dialog.GetNextPathName (pos);
			if (::Get_File_Mgr ()->Is_Path_Valid (full_path)) {
				m_ListCtrl.InsertItem (0xFF, (LPCTSTR)::Get_File_Mgr ()->Make_Relative_Path (full_path));
			}
		}
	}
			
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnKeydownFilenameListCtrl
//
/////////////////////////////////////////////////////////////////////////////
void
EditFilenameListDialogClass::OnKeydownFilenameListCtrl
(
	NMHDR *pNMHDR,
	LRESULT *pResult
) 
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;
	(*pResult) = 0;


	if (pLVKeyDow->wVKey == VK_DELETE) {
		
		//
		//	Delete all the selected items
		//
		int index = -1;
		while ((index = m_ListCtrl.GetNextItem (-1, LVNI_SELECTED | LVNI_ALL)) >= 0) {
			m_ListCtrl.DeleteItem (index);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
EditFilenameListDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	//
	//	Add a single column to the list control
	//
	m_ListCtrl.InsertColumn (0, "Filename");
	
	//
	//	Size the column
	//
	CRect rect;
	m_ListCtrl.GetClientRect (&rect);
	rect.right -= ::GetSystemMetrics (SM_CXVSCROLL);
	m_ListCtrl.SetColumnWidth(0, rect.Width ());

	//
	//	Add all the initial filenames to the list control
	//
	for (int index = 0; index < m_List.Count (); index ++) {
		m_ListCtrl.InsertItem (0xFF, m_List[index]);
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
EditFilenameListDialogClass::OnOK (void)
{
	//
	//	Start fresh
	//
	m_List.Delete_All ();

	//
	//	Rebuild the list from the user's entries in the list control	
	//
	for (int index = 0; index < m_ListCtrl.GetItemCount (); index ++) {
		StringClass filename = (LPCTSTR)m_ListCtrl.GetItemText (index, 0);
		m_List.Add (filename);
	}

	CDialog::OnOK ();
	return ;
}
