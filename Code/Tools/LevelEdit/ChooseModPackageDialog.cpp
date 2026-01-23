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

// ChooseModPackageDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "choosemodpackagedialog.h"
#include "assetpackagemgr.h"
#include "newassetpackagedialog.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// ChooseModPackageDialogClass
//
/////////////////////////////////////////////////////////////////////////////
ChooseModPackageDialogClass::ChooseModPackageDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(ChooseModPackageDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(ChooseModPackageDialogClass)
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
ChooseModPackageDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ChooseModPackageDialogClass)
	DDX_Control(pDX, IDC_MOD_LIST_CTRL, ListCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ChooseModPackageDialogClass, CDialog)
	//{{AFX_MSG_MAP(ChooseModPackageDialogClass)
	ON_BN_CLICKED(IDC_NEW_BUTTON, OnNewButton)
	ON_NOTIFY(NM_DBLCLK, IDC_MOD_LIST_CTRL, OnDblclkModListCtrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
ChooseModPackageDialogClass::OnInitDialog (void) 
{
	CDialog::OnInitDialog ();

	//
	//	Configure the columns
	//
	ListCtrl.InsertColumn (0, "");
	ListCtrl.SetExtendedStyle (ListCtrl.GetExtendedStyle () | LVS_EX_FULLROWSELECT);

	//
	//	Choose an appropriate size for the column
	//
	CRect rect;
	ListCtrl.GetClientRect (&rect);
	rect.right -= ::GetSystemMetrics (SM_CXVSCROLL) + 2;
	ListCtrl.SetColumnWidth (0, rect.Width ());

	//
	//	Build a list of all the packages that are currently available
	//
	STRING_LIST package_list;
	AssetPackageMgrClass::Build_Package_List (package_list);

	//
	//	Add an entry to the control for each package
	//
	bool found = false;
	for (int index = 0; index < package_list.Count (); index ++) {
		int item_index = ListCtrl.InsertItem (index, package_list[index]);
		if (item_index >= 0) {
			
			//
			//	Is this the default entry?
			//
			if (::lstrcmpi (AssetPackageMgrClass::Get_Current_Package (), package_list[index]) == 0) {
				ListCtrl.SetItemState (item_index, LVIS_SELECTED, LVIS_SELECTED);
				found = true;
			}
		}
	}

	//
	//	Select the first entry by default
	//
	if (package_list.Count () > 0 && found) {
		ListCtrl.SetItemState (0, LVIS_SELECTED, LVIS_SELECTED);
	}
	
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
ChooseModPackageDialogClass::OnOK (void)
{
	int curr_index = ListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (curr_index >= 0) {

		//
		//	Set the current mod package
		//
		CString package_name = ListCtrl.GetItemText (curr_index, 0);
		AssetPackageMgrClass::Set_Current_Package (package_name);

		//
		//	Close the dialog
		//
		CDialog::OnOK ();

	} else {
		
		//
		//	Warn the user
		//
		MessageBox ("You must select (or create) a Mod Package before continuing.", "Invalid Selection", MB_ICONERROR | MB_OK);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnNewButton
//
/////////////////////////////////////////////////////////////////////////////
void
ChooseModPackageDialogClass::OnNewButton (void) 
{
	//
	//	Display a dialog to the user allowing them to create a new mod package
	//
	NewAssetPackageDialogClass dialog (this);
	if (dialog.DoModal () == IDOK) {

		//
		//	Create the new asset package
		//
		AssetPackageMgrClass::Create_Package (dialog.Get_Package_Name ());

		//
		//	Add this package to the list
		//
		int item_index = ListCtrl.InsertItem (0xFF, dialog.Get_Package_Name ());
		if (item_index >= 0) {
			
			//
			//	Select the new package
			//
			ListCtrl.SetItemState (item_index, LVIS_SELECTED, LVIS_SELECTED);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDblclkModListCtrl
//
/////////////////////////////////////////////////////////////////////////////
void
ChooseModPackageDialogClass::OnDblclkModListCtrl (NMHDR * /* pNMHDR */, LRESULT *pResult)
{
	(*pResult) = 0;
	
	//
	//	Simulate pressing the OK button
	//
	SendMessage (WM_COMMAND, MAKELONG (IDOK, BN_CLICKED), 0);
	return ;
}
