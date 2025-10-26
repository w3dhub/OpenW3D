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

// HeightfieldMaterialSettingsDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "heightfieldmaterialsettingsdialog.h"
#include "heightfieldeditor.h"
#include "terrainmaterial.h"
#include "Utils.h"
#include "filemgr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
//	HeightfieldMaterialSettingsDialogClass
//
/////////////////////////////////////////////////////////////////////////////
HeightfieldMaterialSettingsDialogClass::HeightfieldMaterialSettingsDialogClass (CWnd *pParent)	:
	MaterialIndex (0),
	CDialog(HeightfieldMaterialSettingsDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(HeightfieldMaterialSettingsDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
HeightfieldMaterialSettingsDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(HeightfieldMaterialSettingsDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	return ;
}


BEGIN_MESSAGE_MAP(HeightfieldMaterialSettingsDialogClass, CDialog)
	//{{AFX_MSG_MAP(HeightfieldMaterialSettingsDialogClass)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
//	OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
HeightfieldMaterialSettingsDialogClass::OnInitDialog (void) 
{
	CDialog::OnInitDialog ();
	
	//
	//	Lookup the material we're editing
	//
	TerrainMaterialClass *material = HeightfieldEditorClass::Peek_Material (MaterialIndex);
	if (material != NULL) {
		
		//
		//	Fill in the controls
		//
		SetDlgItemText (IDC_TEXTURE_FILENAME, material->Get_Texture_Name ());
		::SetDlgItemFloat (m_hWnd, IDC_METERS_PER_TILE_EDIT, material->Get_Meters_Per_Tile ());
		CheckDlgButton (IDC_MIRROR_CHECK, material->Are_UVs_Mirrored ());

		//
		//	Select the right surface type
		//
		SendDlgItemMessage (IDC_SURFACE_TYPE_COMBO, CB_SETCURSEL, material->Get_Surface_Type (), 0L);

	} else {
		::SetDlgItemFloat (m_hWnd, IDC_METERS_PER_TILE_EDIT, 10.0F);
	}
		
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
HeightfieldMaterialSettingsDialogClass::OnOK (void)
{
	bool is_ok = true;

	//
	//	Get the texture path
	//
	CString texture_path;
	GetDlgItemText (IDC_TEXTURE_FILENAME, texture_path);

	//
	// Is the texture path valid?
	//
	bool is_rel_path = ::Is_Path_Relative (texture_path);
	if ((is_rel_path == true) || ((is_rel_path == false) && ::Get_File_Mgr ()->Is_Path_Valid (texture_path))) {
		
		float meters_per_tile = ::GetDlgItemFloat (m_hWnd, IDC_METERS_PER_TILE_EDIT);

		//
		//	Get (or create) the material we're editing
		//
		TerrainMaterialClass *material = HeightfieldEditorClass::Peek_Material (MaterialIndex);
		if (material == NULL) {
			
			//
			//	Create the material and add it to the manager
			//
			material = new TerrainMaterialClass;
			HeightfieldEditorClass::Set_Material (MaterialIndex, material);
			material->Release_Ref ();
		}

		//
		//	Configure the material
		//
		material->Set_Texture (texture_path);
		material->Set_Meters_Per_Tile (meters_per_tile);		
		material->Mirror_UVs (IsDlgButtonChecked (IDC_MIRROR_CHECK) == 1);
		material->Set_Surface_Type (SendDlgItemMessage (IDC_SURFACE_TYPE_COMBO, CB_GETCURSEL));

	} else {

		//
		// Let the user know this path is invalid
		//
		CString message;
		CString title;
		message.Format (IDS_INVALID_MODEL_PATH_MSG, (LPCTSTR)::Get_File_Mgr()->Get_Base_Path ());
		title.LoadString (IDS_INVALID_MODEL_PATH_TITLE);
		::MessageBox (m_hWnd, message, title, MB_ICONERROR | MB_OK);
		is_ok = false;
	}

	if (is_ok) {
		CDialog::OnOK ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnBrowse
//
/////////////////////////////////////////////////////////////////////////////
void
HeightfieldMaterialSettingsDialogClass::OnBrowse (void)
{
	//
	//	Get the current texture path
	//
	CString texture_path;
	GetDlgItemText (IDC_TEXTURE_FILENAME, texture_path);

	CFileDialog dialog (true, ".tga", texture_path,
		OFN_HIDEREADONLY | OFN_EXPLORER,
		"Texture Files (*.tga)|*.tga||", ::AfxGetMainWnd ());

	//
	// Ask the user what texture they wish to use
	//
	if (dialog.DoModal () == IDOK) {

		//
		//	Reset the texture filename as a "relative" path
		//
		CString full_texture_path	= dialog.GetPathName ();
		CString rel_texture_path	= ::Get_File_Mgr ()->Make_Relative_Path (full_texture_path);
		SetDlgItemText (IDC_TEXTURE_FILENAME, rel_texture_path);
	}

	return ;
}

