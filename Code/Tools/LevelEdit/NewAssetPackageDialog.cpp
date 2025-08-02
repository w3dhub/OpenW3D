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

// NewAssetPackageDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "leveledit.h"
#include "newassetpackagedialog.h"
#include "assetpackagemgr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// NewAssetPackageDialogClass
//
/////////////////////////////////////////////////////////////////////////////
NewAssetPackageDialogClass::NewAssetPackageDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(NewAssetPackageDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(NewAssetPackageDialogClass)
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
NewAssetPackageDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NewAssetPackageDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(NewAssetPackageDialogClass, CDialog)
	//{{AFX_MSG_MAP(NewAssetPackageDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
NewAssetPackageDialogClass::OnInitDialog (void) 
{
	CDialog::OnInitDialog ();
	
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
NewAssetPackageDialogClass::OnOK (void)
{
	CString text;
	GetDlgItemText (IDC_PACKAGE_NAME_EDIT, text);

	//
	//	Check to ensure this name is valid
	//
	if (Is_Valid_Name (text) == false) {
		MessageBox ("Names must be at least one character long and cannot contain any of the following:  \\ / \" * ? < > |", "Invalid Name", MB_ICONERROR | MB_OK);
		SendDlgItemMessage (IDC_PACKAGE_NAME_EDIT, EM_SETSEL, 0, -1);
		GetDlgItem (IDC_PACKAGE_NAME_EDIT)->SetFocus ();
	} else if (Is_Duplicate_Name (text)) {
		MessageBox ("Name is already in use, please choose another name.", "Invalid Name", MB_ICONERROR | MB_OK);
		SendDlgItemMessage (IDC_PACKAGE_NAME_EDIT, EM_SETSEL, 0, -1);
		GetDlgItem (IDC_PACKAGE_NAME_EDIT)->SetFocus ();
	} else {
		PackageName = text;
		CDialog::OnOK ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Is_Valid_Name
//
/////////////////////////////////////////////////////////////////////////////
bool
NewAssetPackageDialogClass::Is_Valid_Name (const CString &name)
{
	bool retval = false;

	//
	//	Check to see if the string is long enough and that it doesn't contain
	// any illegal characters
	//
	if (name.GetLength () > 0) {
		retval = (name.FindOneOf ("\\/\"*?<>|") == -1);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
// Is_Duplicate_Name
//
/////////////////////////////////////////////////////////////////////////////
bool
NewAssetPackageDialogClass::Is_Duplicate_Name (const CString &name)
{
	bool retval = false;

	//
	//	Build a list of all the packages that are currently available
	//
	STRING_LIST package_list;
	AssetPackageMgrClass::Build_Package_List (package_list);

	//
	//	Check to see if the new name is already being used
	//
	for (int index = 0; index < package_list.Count (); index ++) {
		if (name.CompareNoCase (package_list[index]) == 0) {
			retval = true;
			break;
		}
	}


	return retval;
}

