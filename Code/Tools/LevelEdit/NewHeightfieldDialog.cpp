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

// NewHeightfieldDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "newheightfielddialog.h"
#include "heightfieldmgr.h"
#include "Utils.h"
#include "refcount.h"
#include "wwstring.h"
#include "heightfieldeditor.h"
#include "editableheightfield.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// NewHeightfieldDialogClass dialog


NewHeightfieldDialogClass::NewHeightfieldDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(NewHeightfieldDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(NewHeightfieldDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void NewHeightfieldDialogClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NewHeightfieldDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(NewHeightfieldDialogClass, CDialog)
	//{{AFX_MSG_MAP(NewHeightfieldDialogClass)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
//	OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
NewHeightfieldDialogClass::OnOK (void)
{
	int width		= GetDlgItemInt (IDC_WIDTH_EDIT);
	int height		= GetDlgItemInt (IDC_HEIGHT_EDIT);
	float density	= ::GetDlgItemFloat (m_hWnd, IDC_DENSITY_EDIT, false);

	//
	//	Is the user attempting to initialize the heightfield from a grayscale bitmap?
	//
	CString bmp_filename;
	GetDlgItemText (IDC_BMP_FILENAME_EDIT, bmp_filename);
	if (bmp_filename.GetLength () >= 0 && ::GetFileAttributes (bmp_filename) != 0xFFFFFFFF) {		

		//
		//	Create the initial heightfield from these values
		//
		float scale = ::GetDlgItemFloat (m_hWnd, IDC_HEIGHT_SCALE_EDIT, false);
		HeightfieldMgrClass::Create_Heightfield (bmp_filename, width, height, density, scale);		

	} else {

		//
		//	Create the initial heightfield from these values
		//
		HeightfieldMgrClass::Create_Heightfield (width, height, density);
	}

	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
NewHeightfieldDialogClass::OnInitDialog (void) 
{
	CDialog::OnInitDialog ();
	
	//
	//	Generate a name for this heightfield
	//
	StringClass new_name;
	new_name.Format ("Heightfield %d", HeightfieldMgrClass::Get_Heightfield_Count () + 1);
	SetDlgItemText (IDC_NAME_EDIT, new_name);

	//
	//	Set some default values...
	//
	SetDlgItemInt (IDC_WIDTH_EDIT, 500);
	SetDlgItemInt (IDC_HEIGHT_EDIT, 500);
	::SetDlgItemFloat (m_hWnd, IDC_DENSITY_EDIT, 2.0F);
	::SetDlgItemFloat (m_hWnd, IDC_HEIGHT_SCALE_EDIT, 75.0F);

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnBrowse
//
/////////////////////////////////////////////////////////////////////////////
void
NewHeightfieldDialogClass::OnBrowse (void)
{
	//
	//	Get the current texture path
	//
	CString filename;
	GetDlgItemText (IDC_BMP_FILENAME_EDIT, filename);

	CFileDialog dialog (TRUE, ".bmp", filename,
		OFN_HIDEREADONLY | OFN_EXPLORER,
		"Windows Bitmap Files (*.bmp)|*.bmp||", ::AfxGetMainWnd ());

	//
	// Ask the user what bitmap they wish to use
	//
	if (dialog.DoModal () == IDOK) {
		SetDlgItemText (IDC_BMP_FILENAME_EDIT, dialog.GetPathName ());
	}

	return ;
}

