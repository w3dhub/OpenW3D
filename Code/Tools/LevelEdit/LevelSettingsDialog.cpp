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

// LevelSettingsDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "levelsettingsdialog.h"
#include "combat.h"
#include "scriptmgr.h"
#include "editscript.h"
#include "mapmgr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// LevelSettingsDialogClass
//
/////////////////////////////////////////////////////////////////////////////
LevelSettingsDialogClass::LevelSettingsDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(LevelSettingsDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(LevelSettingsDialogClass)
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
LevelSettingsDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(LevelSettingsDialogClass)
	DDX_Control(pDX, IDC_RESTART_SCRIPT_COMBO, RestartScriptCombo);
	DDX_Control(pDX, IDC_RESPAWN_SCRIPT_COMBO, RespawnScriptCombo);
	DDX_Control(pDX, IDC_MAP_SCALEY_SPIN, MapScaleYSpin);
	DDX_Control(pDX, IDC_MAP_SCALEX_SPIN, MapScaleXSpin);
	DDX_Control(pDX, IDC_MAP_PIXEL_OFFSETY_SPIN, MapOffsetYSpin);
	DDX_Control(pDX, IDC_MAP_PIXEL_OFFSETX_SPIN, MapOffsetXSpin);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(LevelSettingsDialogClass, CDialog)
	//{{AFX_MSG_MAP(LevelSettingsDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
LevelSettingsDialogClass::OnInitDialog (void) 
{
	CDialog::OnInitDialog ();

	//
	//	Populate the script controls
	//
	Fill_Script_Combobox (IDC_RESTART_SCRIPT_COMBO, CombatManager::Get_Start_Script ());
	Fill_Script_Combobox (IDC_RESPAWN_SCRIPT_COMBO, CombatManager::Get_Respawn_Script ());

	//
	//	Get the rectangle where we'll create the file picker
	//
	CRect rect;
	::GetWindowRect (::GetDlgItem (m_hWnd, IDC_MAP_FILENAME_EDIT), &rect);
	ScreenToClient (&rect);

	//
	//	Destroy the old window
	//
	::DestroyWindow (::GetDlgItem (m_hWnd, IDC_MAP_FILENAME_EDIT));

	//
	//	Create and initialize the picker
	//
	FilePicker.Create_Picker (WS_CHILD | WS_TABSTOP | WS_VISIBLE, rect, this, IDC_MAP_FILENAME_EDIT);	
	FilePicker.Set_Filter_String ("Texture Files (*.tga)|*.tga|All Files (*.*)|*.*||");

	//
	//	Put the default filename into the picker control
	//
	FilePicker.SetWindowText (MapMgrClass::Get_Map_Texture_Pathname ());
	FilePicker.Set_Asset_Tree_Only (true);


	//
	//	Get the rectangle where we'll create the file picker
	//
	::GetWindowRect (::GetDlgItem (m_hWnd, IDC_MAP_TITLE_EDIT), &rect);
	ScreenToClient (&rect);
	::DestroyWindow (::GetDlgItem (m_hWnd, IDC_MAP_TITLE_EDIT));

	//
	//	Create and initialize the string picker
	//
	StringPicker.Create_Picker (WS_CHILD | WS_TABSTOP | WS_VISIBLE, rect, this, IDC_MAP_TITLE_EDIT);
	StringPicker.Set_Entry (MapMgrClass::Get_Map_Title ());
	
	//
	//	Configure the float controls
	//
	::Make_Edit_Float_Ctrl (::GetDlgItem (m_hWnd, IDC_MAP_PIXEL_OFFSETX_EDIT));
	::Make_Edit_Float_Ctrl (::GetDlgItem (m_hWnd, IDC_MAP_PIXEL_OFFSETY_EDIT));
	::Make_Edit_Float_Ctrl (::GetDlgItem (m_hWnd, IDC_MAP_SCALEX_EDIT));
	::Make_Edit_Float_Ctrl (::GetDlgItem (m_hWnd, IDC_MAP_SCALEY_EDIT));

	//
	//	Set the float control ranges
	//
	MapScaleXSpin.SetRange32 (-1000000, 1000000);
	MapScaleYSpin.SetRange32 (-1000000, 1000000);
	MapOffsetXSpin.SetRange32 (-1000000, 1000000);
	MapOffsetYSpin.SetRange32 (-1000000, 1000000);

	//
	//	Set the default values for the control
	//
	::SetDlgItemFloat (m_hWnd, IDC_MAP_SCALEX_EDIT,				MapMgrClass::Get_Map_Scale ().X);
	::SetDlgItemFloat (m_hWnd, IDC_MAP_SCALEY_EDIT,				MapMgrClass::Get_Map_Scale ().Y);
	::SetDlgItemFloat (m_hWnd, IDC_MAP_PIXEL_OFFSETX_EDIT,	MapMgrClass::Get_Map_Center ().X);
	::SetDlgItemFloat (m_hWnd, IDC_MAP_PIXEL_OFFSETY_EDIT,	MapMgrClass::Get_Map_Center ().Y);

	SendDlgItemMessage (IDC_ALLOW_VTOL_CHECK, BM_SETCHECK, MapMgrClass::Are_VTOL_Vehicles_Enabled ());
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
LevelSettingsDialogClass::OnOK (void)
{
	//
	//	Store the currently selected restart script
	//
	int index = RestartScriptCombo.GetCurSel ();
	if (index != CB_ERR) {
		CString script_name;
		RestartScriptCombo.GetLBText (index, script_name);		
		CombatManager::Set_Start_Script (script_name);
	} else {
		CombatManager::Set_Start_Script ("");
	}

	//
	//	Store the currently selected respawn script
	//
	index = RespawnScriptCombo.GetCurSel ();
	if (index != CB_ERR) {
		CString script_name;
		RespawnScriptCombo.GetLBText (index, script_name);		
		CombatManager::Set_Respawn_Script (script_name);
	} else {
		CombatManager::Set_Respawn_Script ("");
	}

	//
	//	Set the map title
	//
	MapMgrClass::Set_Map_Title (StringPicker.Get_Entry ());
	
	//
	//	Set the map filename
	//
	CString map_filename;
	FilePicker.GetWindowText (map_filename);
	MapMgrClass::Set_Map_Texture (map_filename);

	Vector2 offset;
	Vector2 scale;

	//
	//	Set the default values for the control
	//
	scale.X	= ::GetDlgItemFloat (m_hWnd, IDC_MAP_SCALEX_EDIT, true);
	scale.Y	= ::GetDlgItemFloat (m_hWnd, IDC_MAP_SCALEY_EDIT, true);
	offset.X	= ::GetDlgItemFloat (m_hWnd, IDC_MAP_PIXEL_OFFSETX_EDIT, true);
	offset.Y	= ::GetDlgItemFloat (m_hWnd, IDC_MAP_PIXEL_OFFSETY_EDIT, true);
	MapMgrClass::Set_Map_Scale (scale);
	MapMgrClass::Set_Map_Center (offset);
	MapMgrClass::Cloud_All_Cells ();

	bool vtol_on = (SendDlgItemMessage (IDC_ALLOW_VTOL_CHECK, BM_GETCHECK) == 1);
	MapMgrClass::Enable_VTOL_Vehicles (vtol_on);

	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Fill_Script_Combobox
//
/////////////////////////////////////////////////////////////////////////////
void
LevelSettingsDialogClass::Fill_Script_Combobox (int ctrl_id, const char *default_name) 
{
	//
	// Add the total list of scripts to the combobox
	//
	for (int index = 0; index < ScriptMgrClass::Get_Count (); index ++) {
		EditScriptClass *script = ScriptMgrClass::Get_Script (index);		
		if (script != NULL) {

			//
			// Add this script to the combobox
			//
			int item_index = SendDlgItemMessage (ctrl_id, CB_ADDSTRING, 0, (LPARAM)script->Get_Name ());
			SendDlgItemMessage (ctrl_id, CB_SETITEMDATA, (WPARAM)item_index, (LPARAM)script);

			//
			//	Select this script if it is the default
			//
			if (::lstrcmpi (default_name, script->Get_Name ()) == 0) {
				SendDlgItemMessage (ctrl_id, CB_SETCURSEL, (WPARAM)item_index);
			}
		}
	}

	return ;
}
