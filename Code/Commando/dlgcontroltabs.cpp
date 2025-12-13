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
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/dlgcontroltabs.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 4/03/02 2:16p                                               $*
 *                                                                                             *
 *                    $Revision:: 17                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "renegadedialog.h"
#include "dlgcontroltabs.h"
#include "input.h"
#include "sliderctrl.h"
#include "vehicle.h"
#include "comboboxctrl.h"
#include "translatedb.h"
#include "string_ids.h"
#include "useroptions.h"
#include "combat.h"
#include "ccamera.h"


////////////////////////////////////////////////////////////////
//
//	ControlsBasicMvmtTabClass
//
////////////////////////////////////////////////////////////////
ControlsBasicMvmtTabClass::ControlsBasicMvmtTabClass (void)	:
	ControlsListTabClass (GetRenegadeDialog(RenegadeDialogID::IDD_CONTROLS_BASIC_MOVMENT_TAB))
{
	Add_Function (INPUT_FUNCTION_MOVE_FORWARD,	IDC_HOTKEY1, IDC_HOTKEY13);
	Add_Function (INPUT_FUNCTION_MOVE_BACKWARD,	IDC_HOTKEY2, IDC_HOTKEY14);
	Add_Function (INPUT_FUNCTION_MOVE_LEFT,		IDC_HOTKEY3, IDC_HOTKEY15);
	Add_Function (INPUT_FUNCTION_MOVE_RIGHT,		IDC_HOTKEY4, IDC_HOTKEY16);
	Add_Function (INPUT_FUNCTION_TURN_LEFT,		IDC_HOTKEY5, IDC_HOTKEY17);
	Add_Function (INPUT_FUNCTION_TURN_RIGHT,		IDC_HOTKEY6, IDC_HOTKEY18);
	Add_Function (INPUT_FUNCTION_WALK_MODE,		IDC_HOTKEY7, IDC_HOTKEY19);
	Add_Function (INPUT_FUNCTION_JUMP,				IDC_HOTKEY8, IDC_HOTKEY20);
	Add_Function (INPUT_FUNCTION_CROUCH,			IDC_HOTKEY9, IDC_HOTKEY21);
	Add_Function (INPUT_FUNCTION_ACTION,			IDC_HOTKEY10,	IDC_HOTKEY22);
	Add_Function (INPUT_FUNCTION_TURN_AROUND,		IDC_HOTKEY11,	IDC_HOTKEY23);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	ControlsAttackTabClass
//
////////////////////////////////////////////////////////////////
ControlsAttackTabClass::ControlsAttackTabClass (void)	:
	ControlsListTabClass (GetRenegadeDialog(RenegadeDialogID::IDD_CONTROLS_ATTACK_TAB))
{
	Add_Function (INPUT_FUNCTION_FIRE_WEAPON_PRIMARY,		IDC_HOTKEY1, IDC_HOTKEY13);
	Add_Function (INPUT_FUNCTION_FIRE_WEAPON_SECONDARY,	IDC_HOTKEY2, IDC_HOTKEY14);
	Add_Function (INPUT_FUNCTION_PREV_WEAPON,					IDC_HOTKEY3, IDC_HOTKEY15);
	Add_Function (INPUT_FUNCTION_NEXT_WEAPON,					IDC_HOTKEY4, IDC_HOTKEY16);
	Add_Function (INPUT_FUNCTION_RELOAD_WEAPON,				IDC_HOTKEY5, IDC_HOTKEY17);
	Add_Function (INPUT_FUNCTION_ZOOM_IN,						IDC_HOTKEY6, IDC_HOTKEY18);
	Add_Function (INPUT_FUNCTION_ZOOM_OUT,						IDC_HOTKEY7, IDC_HOTKEY19);	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
ControlsAttackTabClass::On_Init_Dialog (void)
{
	ControlsListTabClass::On_Init_Dialog ();
	Load_Controls ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Apply
//
////////////////////////////////////////////////////////////////
bool
ControlsAttackTabClass::On_Apply (void)
{
	ControlsListTabClass::On_Apply ();

	//
	//	Read the data from the checkboxes
	//
	Input::Enable_Damage_Indicators (Is_Dlg_Button_Checked (IDC_DAMAGE_INDICATOR_CHECK));
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load_Controls
//
////////////////////////////////////////////////////////////////
void
ControlsAttackTabClass::Load_Controls (void)
{	
	Check_Dlg_Button (IDC_DAMAGE_INDICATOR_CHECK, Input::Are_Damage_Indicators_Enabled ());
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Reload
//
////////////////////////////////////////////////////////////////
void
ControlsAttackTabClass::On_Reload (void)
{
	ControlsListTabClass::On_Reload ();
	Load_Controls ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	ControlsWeaponsTabClass
//
////////////////////////////////////////////////////////////////
ControlsWeaponsTabClass::ControlsWeaponsTabClass (void)	:
	ControlsListTabClass (GetRenegadeDialog(RenegadeDialogID::IDD_CONTROLS_WEAPONS_TAB))
{
	Add_Function (INPUT_FUNCTION_SELECT_WEAPON_1,	IDC_HOTKEY1, IDC_HOTKEY13);
	Add_Function (INPUT_FUNCTION_SELECT_WEAPON_2,	IDC_HOTKEY2, IDC_HOTKEY14);
	Add_Function (INPUT_FUNCTION_SELECT_WEAPON_3,	IDC_HOTKEY3, IDC_HOTKEY15);
	Add_Function (INPUT_FUNCTION_SELECT_WEAPON_4,	IDC_HOTKEY4, IDC_HOTKEY16);
	Add_Function (INPUT_FUNCTION_SELECT_WEAPON_5,	IDC_HOTKEY5, IDC_HOTKEY17);
	Add_Function (INPUT_FUNCTION_SELECT_WEAPON_6,	IDC_HOTKEY6, IDC_HOTKEY18);
	Add_Function (INPUT_FUNCTION_SELECT_WEAPON_7,	IDC_HOTKEY7, IDC_HOTKEY19);
	Add_Function (INPUT_FUNCTION_SELECT_WEAPON_8,	IDC_HOTKEY8, IDC_HOTKEY20);
	Add_Function (INPUT_FUNCTION_SELECT_WEAPON_9,	IDC_HOTKEY9, IDC_HOTKEY21);
	Add_Function (INPUT_FUNCTION_SELECT_WEAPON_0,	IDC_HOTKEY10, IDC_HOTKEY22);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	ControlsLookTabClass
//
////////////////////////////////////////////////////////////////
ControlsLookTabClass::ControlsLookTabClass (void)	:
	ControlsListTabClass (GetRenegadeDialog(RenegadeDialogID::IDD_CONTROLS_LOOK_TAB))
{
	Add_Function (INPUT_FUNCTION_CURSOR_TARGETING,		IDC_HOTKEY1, IDC_HOTKEY13);
	Add_Function (INPUT_FUNCTION_FIRST_PERSON_TOGGLE,	IDC_HOTKEY2, IDC_HOTKEY14);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
ControlsLookTabClass::On_Init_Dialog (void)
{
	ControlsListTabClass::On_Init_Dialog ();

	//
	//	Configure the mouse sensivity slider
	//
	SliderCtrlClass *slider_ctrl = (SliderCtrlClass *)Get_Dlg_Item (IDC_MOUSE_SENSITIVITY_SLIDER);
	if (slider_ctrl != NULL) {
		slider_ctrl->Set_Range (0, 100);
	}

	//
	//	Configure the steering mode combobox
	//
//	ComboBoxCtrlClass *combo_box = (ComboBoxCtrlClass *)Get_Dlg_Item (IDC_STEERING_MODE_COMBO);
//	if (combo_box != NULL) {		
//		combo_box->Add_String (TRANSLATE (IDS_STEERING_MODE_NORMAL));
//		combo_box->Add_String (TRANSLATE (IDS_STEERING_MODE_TARGET));
//		combo_box->Set_Curr_Sel (1);
//	}

	Load_Controls ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Apply
//
////////////////////////////////////////////////////////////////
bool
ControlsLookTabClass::On_Apply (void)
{
	ControlsListTabClass::On_Apply ();

	//
	//	Read the data from the slider
	//
	SliderCtrlClass *slider_ctrl = (SliderCtrlClass *)Get_Dlg_Item (IDC_MOUSE_SENSITIVITY_SLIDER);
	if (slider_ctrl != NULL) {
		Input::Set_Mouse_Sensitivity (slider_ctrl->Get_Pos () / 100.0F);
	}

	//
	//	Read the steering mode
	//

	// Always keyboard steer
	VehicleGameObj::Set_Target_Steering (false);
//	ComboBoxCtrlClass *combo_box = (ComboBoxCtrlClass *)Get_Dlg_Item (IDC_STEERING_MODE_COMBO);
//	if (combo_box != NULL) {
//		VehicleGameObj::Set_Target_Steering ((combo_box->Get_Curr_Sel () == 1));
//	}

	//
	//	Read the data from the checkboxes
	//
	Input::Set_Mouse_Invert (Is_Dlg_Button_Checked (IDC_INVERT_MOUSE_CHECK));
	Input::Set_Mouse_2D_Invert (Is_Dlg_Button_Checked (IDC_INVERT_CURSOR_TARGETTING_CHECK));
	cUserOptions::CameraLockedToTurret.Set(Is_Dlg_Button_Checked (IDC_LOCK_CAMERA_TO_TURRET_CHECK));

	//
	// Notify combat about the state of the CameraLockedToTurret user option.
	//
	VehicleGameObj::Set_Camera_Locked_To_Turret(cUserOptions::CameraLockedToTurret.Get());

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load_Controls
//
////////////////////////////////////////////////////////////////
void
ControlsLookTabClass::Load_Controls (void)
{	
	//
	//	Configure the mouse sensivity slider
	//
	SliderCtrlClass *slider_ctrl = (SliderCtrlClass *)Get_Dlg_Item (IDC_MOUSE_SENSITIVITY_SLIDER);
	if (slider_ctrl != NULL) {
		slider_ctrl->Set_Pos (Input::Get_Mouse_Sensitivity () * 100);
	}

	//
	//	Load the steering mode setting
	//
//	ComboBoxCtrlClass *combo_box = (ComboBoxCtrlClass *)Get_Dlg_Item (IDC_STEERING_MODE_COMBO);
//	if (combo_box != NULL) {
//		combo_box->Set_Curr_Sel (VehicleGameObj::Is_Target_Steering ());
//	}

	//
	//	Configure the check boxes
	//
	Check_Dlg_Button (IDC_INVERT_MOUSE_CHECK, Input::Get_Mouse_Invert ());
	Check_Dlg_Button (IDC_INVERT_CURSOR_TARGETTING_CHECK, Input::Get_Mouse_2D_Invert ());
	Check_Dlg_Button (IDC_LOCK_CAMERA_TO_TURRET_CHECK, cUserOptions::CameraLockedToTurret.Get());
	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Reload
//
////////////////////////////////////////////////////////////////
void
ControlsLookTabClass::On_Reload (void)
{
	ControlsListTabClass::On_Reload ();
	Load_Controls ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	ControlsMultiPlayTabClass
//
////////////////////////////////////////////////////////////////
ControlsMultiPlayTabClass::ControlsMultiPlayTabClass (void)	:
	ControlsListTabClass (GetRenegadeDialog(RenegadeDialogID::IDD_CONTROLS_MULTIPLAYER_TAB))
{
	Add_Function (INPUT_FUNCTION_BEGIN_PUBLIC_MESSAGE,		IDC_HOTKEY1, IDC_HOTKEY13);
	Add_Function (INPUT_FUNCTION_BEGIN_TEAM_MESSAGE,		IDC_HOTKEY2, IDC_HOTKEY14);
	Add_Function (INPUT_FUNCTION_TEAM_INFO_TOGGLE,			IDC_HOTKEY3, IDC_HOTKEY15);
	Add_Function (INPUT_FUNCTION_BATTLE_INFO_TOGGLE,		IDC_HOTKEY4, IDC_HOTKEY16);
	Add_Function (INPUT_FUNCTION_EVA_OBJECTIVES_SCREEN,	IDC_HOTKEY5, IDC_HOTKEY17);
	Add_Function (INPUT_FUNCTION_EVA_MAP_SCREEN,				IDC_HOTKEY6, IDC_HOTKEY18);
	return ;
}

