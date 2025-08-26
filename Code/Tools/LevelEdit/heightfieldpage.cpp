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
 *                 Project Name : leveledit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/heightfieldpage.cpp          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/28/02 4:40p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "LevelEdit.h"
#include "heightfieldpage.h"
#include "nodemgr.h"
#include "node.h"
#include "Utils.h"
#include "cameramgr.h"
#include "gotoobjectdialog.h"
#include "sceneeditor.h"
#include "icons.h"
#include "nodecategories.h"
#include "definition.h"
#include "definitionfactory.h"
#include "definitionfactorymgr.h"
#include "preset.h"
#include "editableheightfield.h"
#include "staticphys.h"
#include "heightfieldeditor.h"
#include "heightfieldmgr.h"
#include "terrainmaterial.h"
#include "heightfieldmaterialsettingsdialog.h"
#include "newheightfielddialog.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////////////////////
//	Static member initialization
///////////////////////////////////////////////////////////////////////
HeightfieldPageClass * HeightfieldPageClass::_TheInstance	= NULL;


/////////////////////////////////////////////////////////////////////////////
//
// HeightfieldPageClass
//
/////////////////////////////////////////////////////////////////////////////
HeightfieldPageClass::HeightfieldPageClass (CWnd *parent_wnd)	:
	IsInitialized (false),
	CDialog (HeightfieldPageClass::IDD)
{
	//{{AFX_DATA_INIT(HeightfieldPageClass)
	//}}AFX_DATA_INIT
	Create (HeightfieldPageClass::IDD, parent_wnd);

	_TheInstance = this;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~HeightfieldPageClass
//
/////////////////////////////////////////////////////////////////////////////
HeightfieldPageClass::~HeightfieldPageClass (void)
{
	_TheInstance = NULL;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
HeightfieldPageClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(HeightfieldPageClass)
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(HeightfieldPageClass, CDialog)
	//{{AFX_MSG_MAP(HeightfieldPageClass)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CREATE_NEW_BUTTON, OnCreateNewButton)
	ON_BN_CLICKED(IDC_MATERIAL_SETTINGS_BUTTON, OnMaterialSettingsButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


#ifdef _DEBUG
void HeightfieldPageClass::AssertValid() const
{
	CDialog::AssertValid();
}

void HeightfieldPageClass::Dump(CDumpContext& dc) const
{
	CDialog::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
//
// OnSize
//
/////////////////////////////////////////////////////////////////////////////
void
HeightfieldPageClass::OnSize
(
	UINT	nType,
	int	cx,
	int	cy
) 
{	
	CDialog::OnSize (nType, cx, cy);

	/*if (::IsWindow (m_ListCtrl) && (cx > 0) && (cy > 0)) {
	}*/

	return ;	
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDestroy
//
/////////////////////////////////////////////////////////////////////////////
void
HeightfieldPageClass::OnDestroy (void) 
{
	::RemoveProp (m_hWnd, "TRANS_ACCS");

	CDialog::OnDestroy ();
	return ;	
}


////////////////////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
////////////////////////////////////////////////////////////////////////////
BOOL
HeightfieldPageClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	//
	//	Set the default parameters
	//
	::SetDlgItemFloat (m_hWnd, IDC_BRUSH_INNER_RADIUS_EDIT,	HeightfieldEditorClass::Get_Brush_Inner_Radius ());
	::SetDlgItemFloat (m_hWnd, IDC_BRUSH_OUTTER_RADIUS_EDIT,	HeightfieldEditorClass::Get_Brush_Outter_Radius ());
	::SetDlgItemFloat (m_hWnd, IDC_BRUSH_AMOUNT_EDIT,			HeightfieldEditorClass::Get_Brush_Amount ());

	//
	//	Select the first brush by default
	//
	SendDlgItemMessage (IDC_BRUSH_TYPE_COMBO, CB_SETCURSEL, 0);

	//
	//	Select the current default texture...
	//
	int curr_texture = HeightfieldEditorClass::Get_Current_Texture ();
	SendDlgItemMessage (IDC_TEXTURE0_BUTTON + curr_texture, BM_SETCHECK, TRUE);

	SetProp (m_hWnd, "TRANS_ACCS", (HANDLE)1);
	IsInitialized = true;
	return TRUE;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnCreateNewButton
//
////////////////////////////////////////////////////////////////////////////
void
HeightfieldPageClass::OnCreateNewButton (void)
{
	NewHeightfieldDialogClass dialog (this);
	dialog.DoModal ();
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnCommand
//
////////////////////////////////////////////////////////////////////////////
BOOL
HeightfieldPageClass::OnCommand (WPARAM wParam, LPARAM lParam)
{
	if (IsInitialized) {

		int ctrl_id = LOWORD (wParam);
		switch (ctrl_id)
		{
			case IDC_TEXTURE0_BUTTON:
			case IDC_TEXTURE1_BUTTON:
			case IDC_TEXTURE2_BUTTON:
			case IDC_TEXTURE3_BUTTON:
			case IDC_TEXTURE4_BUTTON:
			case IDC_TEXTURE5_BUTTON:
			case IDC_TEXTURE6_BUTTON:
			case IDC_TEXTURE7_BUTTON:
			case IDC_TEXTURE8_BUTTON:
			{
				if (HIWORD (wParam) == BN_CLICKED) {
					int curr_texture = ctrl_id - IDC_TEXTURE0_BUTTON;
					HeightfieldEditorClass::Set_Current_Texture (curr_texture);
				}

				break;
			}

			case IDC_BRUSH_INNER_RADIUS_EDIT:
				
				//
				//	Update the heightfield editor with the new values
				//
				if (HIWORD (wParam) == EN_CHANGE) {
					float inner		= ::GetDlgItemFloat (m_hWnd, IDC_BRUSH_INNER_RADIUS_EDIT, false);
					float outter	= ::GetDlgItemFloat (m_hWnd, IDC_BRUSH_OUTTER_RADIUS_EDIT, false);
					HeightfieldEditorClass::Set_Brush_Radii (inner, outter);
				}

				break;

			case IDC_BRUSH_OUTTER_RADIUS_EDIT:
				
				//
				//	Update the heightfield editor with the new values
				//
				if (HIWORD (wParam) == EN_CHANGE) {
					float inner		= ::GetDlgItemFloat (m_hWnd, IDC_BRUSH_INNER_RADIUS_EDIT, false);
					float outter	= ::GetDlgItemFloat (m_hWnd, IDC_BRUSH_OUTTER_RADIUS_EDIT, false);
					HeightfieldEditorClass::Set_Brush_Radii (inner, outter);
				}

				break;

			case IDC_BRUSH_AMOUNT_EDIT:

				//
				//	Update the heightfield editor with the new values
				//
				if (HIWORD (wParam) == EN_CHANGE) {
					float amount = ::GetDlgItemFloat (m_hWnd, IDC_BRUSH_AMOUNT_EDIT, false);
					HeightfieldEditorClass::Set_Brush_Amount (amount);
				}

				break;

			case IDC_BRUSH_TYPE_COMBO:
			{
				int curr_sel = SendDlgItemMessage (IDC_BRUSH_TYPE_COMBO, CB_GETCURSEL);
				HeightfieldEditorClass::Set_Mode ((HeightfieldEditorClass::EDITING_MODE)curr_sel);
				break;
			}

		}
	}

	return CDialog::OnCommand (wParam, lParam);
}


////////////////////////////////////////////////////////////////////////////
//
//  Update_Material_Button
//
////////////////////////////////////////////////////////////////////////////
void
HeightfieldPageClass::Update_Material_Button (int index)
{
	if (index < 0 || index > 9) {
		return ;
	}

	//
	//	Get the button in question
	//
	int ctrl_id			= IDC_TEXTURE0_BUTTON + index;
	HWND button_wnd	= ::GetDlgItem (m_hWnd, ctrl_id);

	if (button_wnd != NULL) {
		
		HBITMAP thumbnail = NULL;

		//
		//	Get the material for this slot
		//
		TerrainMaterialClass *material = HeightfieldEditorClass::Get_Material (index);
		if (material != NULL) {

			//
			//	Create a bitmap from this material
			//
			thumbnail = HeightfieldEditorClass::Create_Texture_Thumbnail (material->Get_Texture_Name (), 48, 48);
		}

		//
		//	Put the bitmap into the button
		//
		HBITMAP old_bmp = (HBITMAP)::SendMessage (button_wnd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)thumbnail);
		if (old_bmp != NULL) {

			//
			//	Free the old button bitmap
			//
			::DeleteObject (old_bmp);
			old_bmp = NULL;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnMaterialSettingsButton
//
////////////////////////////////////////////////////////////////////////////
void
HeightfieldPageClass::OnMaterialSettingsButton (void)
{
	int material_index = HeightfieldEditorClass::Get_Current_Texture ();

	//
	//	Display a dialog that allows the user to edit the settings for this
	// material.
	//
	HeightfieldMaterialSettingsDialogClass	dialog (this);
	dialog.Set_Material (material_index);
	if (dialog.DoModal () == IDOK) {
		
		//
		//	Let the system know that this material has changed
		//
		HeightfieldEditorClass::On_Material_Changed (material_index);
	}

	return ;
}

