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

// ExportPresetsDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "exportpresetsdialog.h"
#include "presetmgr.h"
#include "nodecategories.h"
#include "Utils.h"
#include "definitionfactory.h"
#include "definitionfactorymgr.h"
#include "presetexport.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// ExportPresetsDialogClass
//
/////////////////////////////////////////////////////////////////////////////
ExportPresetsDialogClass::ExportPresetsDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(ExportPresetsDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(ExportPresetsDialogClass)
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
ExportPresetsDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ExportPresetsDialogClass)
	DDX_Control(pDX, IDC_TREE_CTRL, m_TreeCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ExportPresetsDialogClass, CDialog)
	//{{AFX_MSG_MAP(ExportPresetsDialogClass)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
ExportPresetsDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	//
	//	Configure the tree control
	//
	m_TreeCtrl.SetImageList (::Get_Global_Image_List (), TVSIL_NORMAL);
	Fill_Tree ();
	
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnBrowse
//
/////////////////////////////////////////////////////////////////////////////
void
ExportPresetsDialogClass::OnBrowse (void)
{
	CFileDialog dialog (false, ".txt", "presets.txt",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
		"Text Files (*.txt)|*.txt||", ::AfxGetMainWnd ());

	//
	// Ask the user what file they want to create
	//
	if (dialog.DoModal () == IDOK) {

		//
		//	Check to make sure the destination filename is not read-only
		//
		CString path				= dialog.GetPathName ();
		DWORD file_attributes	= ::GetFileAttributes (path);
		if (file_attributes != 0xFFFFFFFF && file_attributes & FILE_ATTRIBUTE_READONLY) {
			::MessageBox (::AfxGetMainWnd ()->m_hWnd, "File is read-only, export operation can not complete.", "File Error", MB_ICONERROR | MB_OK);
		} else {
			SetDlgItemText (IDC_FILENAME_EDIT, path);
		}
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
ExportPresetsDialogClass::OnOK (void)
{
	//
	//	Get the filename the user entered
	//
	CString filename;
	GetDlgItemText (IDC_FILENAME_EDIT, filename);

	if (filename.IsEmpty ()) {
		::MessageBox (m_hWnd, "Please enter a filename.", "Invalid Setting", MB_ICONERROR | MB_OK);
	} else  {

		//
		//	Lookup the class ID of the selected preset category
		//
		HTREEITEM selected_item = m_TreeCtrl.GetSelectedItem ();
		int class_id = 0;
		if (selected_item != NULL) {
			class_id = m_TreeCtrl.GetItemData (selected_item);
		}

		if (class_id == 0) {
			::MessageBox (m_hWnd, "Please select a preset category to export.", "Invalid Setting", MB_ICONERROR | MB_OK);
		} else {

			//
			//	Export the settings to the given filename
			//
			PresetExportClass::Export (class_id, filename);
			CDialog::OnOK ();
		}
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Fill_Tree
//
/////////////////////////////////////////////////////////////////////////////
void
ExportPresetsDialogClass::Fill_Tree (void)
{
	//
	//	Add all the node categories to the tree control
	//
	for (int index = 0; index < PRESET_CATEGORY_COUNT; index ++) {

		//
		//	Add this factory to the tree
		//		
		HTREEITEM tree_item = m_TreeCtrl.InsertItem (PRESET_CATEGORIES[index].name, FOLDER_ICON, FOLDER_ICON);		
				
		//
		//	Is there a factory to create this class of defintion?
		//
		DefinitionFactoryClass *factory = DefinitionFactoryMgrClass::Find_Factory (PRESET_CATEGORIES[index].clsid);
		if (factory == NULL) {
			
			//
			//	Find all the sub-factories
			//			
			for (	factory = DefinitionFactoryMgrClass::Get_First (PRESET_CATEGORIES[index].clsid);
					factory != NULL;
					factory = DefinitionFactoryMgrClass::Get_Next (factory, PRESET_CATEGORIES[index].clsid))
			{
				//
				//	Add this sub-factory to the tree
				//
				if (factory->Is_Displayed ()) {
					HTREEITEM child_item = m_TreeCtrl.InsertItem (factory->Get_Name (), FOLDER_ICON, FOLDER_ICON, tree_item);
					m_TreeCtrl.SetItemData (child_item, (DWORD)factory->Get_Class_ID ());
				}
			}

			//
			//	Sort the entries we just made
			//
			m_TreeCtrl.SortChildren (tree_item);
		} else {
			m_TreeCtrl.SetItemData (tree_item, (DWORD)PRESET_CATEGORIES[index].clsid);
		}
	}

	return ;
}
