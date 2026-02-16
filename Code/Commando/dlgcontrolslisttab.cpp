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
 *                     $Archive:: /Commando/Code/commando/dlgcontrolslisttab.cpp      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/17/02 11:22a                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "dlgcontrolslisttab.h"
#include "inputctrl.h"
#include "input.h"
#include "dlgcontrols.h"
#include "string_ids.h"
#include "translatedb.h"

#include <dinput.h>
#include "directinput.h"


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
typedef struct
{
	int function_id;
	int name_id;
} INPUT_FUNCTION_NAME;

const INPUT_FUNCTION_NAME FUNCTION_NAMES[] = 
{
	{ INPUT_FUNCTION_MOVE_FORWARD,				IDS_FUNCTION_MOVE_FORWARD },
	{ INPUT_FUNCTION_MOVE_BACKWARD,				IDS_FUNCTION_MOVE_BACKWARD },
	{ INPUT_FUNCTION_MOVE_LEFT,					IDS_FUNCTION_MOVE_LEFT },
	{ INPUT_FUNCTION_MOVE_RIGHT,					IDS_FUNCTION_MOVE_RIGHT },
	{ INPUT_FUNCTION_TURN_LEFT,					IDS_FUNCTION_TURN_LEFT },
	{ INPUT_FUNCTION_TURN_RIGHT,					IDS_FUNCTION_TURN_RIGHT },
	{ INPUT_FUNCTION_WALK_MODE,					IDS_FUNCTION_WALK_MODE },
	{ INPUT_FUNCTION_JUMP,							IDS_FUNCTION_JUMP },
	{ INPUT_FUNCTION_CROUCH,						IDS_FUNCTION_CROUCH },
	{ INPUT_FUNCTION_ACTION,						IDS_FUNCTION_ACTION },
	{ INPUT_FUNCTION_TURN_AROUND,					IDS_FUNCTION_TURN_AROUND },
	{ INPUT_FUNCTION_FIRE_WEAPON_PRIMARY,		IDS_FUNCTION_FIRE_WEAPON_PRIMARY },
	{ INPUT_FUNCTION_FIRE_WEAPON_SECONDARY,	IDS_FUNCTION_FIRE_WEAPON_SECONDARY },
	{ INPUT_FUNCTION_PREV_WEAPON,					IDS_FUNCTION_PREV_WEAPON },
	{ INPUT_FUNCTION_NEXT_WEAPON,					IDS_FUNCTION_NEXT_WEAPON },
	{ INPUT_FUNCTION_RELOAD_WEAPON,				IDS_FUNCTION_RELOAD_WEAPON },
	{ INPUT_FUNCTION_ZOOM_IN,						IDS_FUNCTION_ZOOM_IN },
	{ INPUT_FUNCTION_ZOOM_OUT,						IDS_FUNCTION_ZOOM_OUT },
	{ INPUT_FUNCTION_SELECT_WEAPON_1,			IDS_FUNCTION_SELECT_WEAPON_1 },
	{ INPUT_FUNCTION_SELECT_WEAPON_2,			IDS_FUNCTION_SELECT_WEAPON_2 },
	{ INPUT_FUNCTION_SELECT_WEAPON_3,			IDS_FUNCTION_SELECT_WEAPON_3 },
	{ INPUT_FUNCTION_SELECT_WEAPON_4,			IDS_FUNCTION_SELECT_WEAPON_4 },
	{ INPUT_FUNCTION_SELECT_WEAPON_5,			IDS_FUNCTION_SELECT_WEAPON_5 },
	{ INPUT_FUNCTION_SELECT_WEAPON_6,			IDS_FUNCTION_SELECT_WEAPON_6 },
	{ INPUT_FUNCTION_SELECT_WEAPON_7,			IDS_FUNCTION_SELECT_WEAPON_7 },
	{ INPUT_FUNCTION_SELECT_WEAPON_8,			IDS_FUNCTION_SELECT_WEAPON_8 },
	{ INPUT_FUNCTION_SELECT_WEAPON_9,			IDS_FUNCTION_SELECT_WEAPON_9 },
	{ INPUT_FUNCTION_SELECT_WEAPON_0,			IDS_FUNCTION_SELECT_WEAPON_0 },
	{ INPUT_FUNCTION_CURSOR_TARGETING,			IDS_FUNCTION_CURSOR_TARGETING },
	{ INPUT_FUNCTION_FIRST_PERSON_TOGGLE,		IDS_FUNCTION_FIRST_PERSON_TOGGLE },
	{ INPUT_FUNCTION_EVA_OBJECTIVES_SCREEN,	IDS_FUNCTION_EVA_OBJECTIVES_SCREEN },
	{ INPUT_FUNCTION_EVA_MAP_SCREEN,				IDS_FUNCTION_EVA_MAP_SCREEN },
	{ INPUT_FUNCTION_BEGIN_PUBLIC_MESSAGE,		IDS_FUNCTION_BEGIN_PUBLIC_MESSAGE },
	{ INPUT_FUNCTION_BEGIN_TEAM_MESSAGE,		IDS_FUNCTION_BEGIN_TEAM_MESSAGE },
	{ INPUT_FUNCTION_TEAM_INFO_TOGGLE,			IDS_FUNCTION_TEAM_INFO_TOGGLE },
	{ INPUT_FUNCTION_BATTLE_INFO_TOGGLE,		IDS_FUNCTION_BATTLE_INFO_TOGGLE }
};

const int INPUT_NAME_COUNT	= sizeof (FUNCTION_NAMES) / sizeof (FUNCTION_NAMES[0]);



////////////////////////////////////////////////////////////////
//
//	ControlsListTabClass
//
////////////////////////////////////////////////////////////////
ControlsListTabClass::ControlsListTabClass (const DialogResource *dialog_resource)	:
	PendingCtrlID (0),
	PendingFunctionID (0),
	PendingDIK_ID (0),
	PendingOldFunctionID (0),
	ChildDialogClass (dialog_resource)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
ControlsListTabClass::On_Init_Dialog (void)
{
	Load_Key_Mappings ();

	ChildDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Find_Function_By_Key
//
////////////////////////////////////////////////////////////////
int
ControlsListTabClass::Find_Function_By_Key (int curr_function_id, int dik_id)
{
	int retval = -1;

	//
	//	Test this key against all the other re-mappable keys
	//
	for (int index = 0; index < INPUT_NAME_COUNT; index ++) {
		int fn_id = FUNCTION_NAMES[index].function_id;

		//
		//	Is this the function we're looking for?
		//
		if (	Input::Get_Primary_Key_For_Function (fn_id) == dik_id ||
				Input::Get_Secondary_Key_For_Function (fn_id) == dik_id)
		{
			//
			//	Note:  We allow the sniper zoom in/out keys to be remapped
			// to other functions.  (But not to each other)
			//
			if (	(fn_id != INPUT_FUNCTION_ZOOM_IN && fn_id != INPUT_FUNCTION_ZOOM_OUT) &&
					(curr_function_id != INPUT_FUNCTION_ZOOM_IN && curr_function_id != INPUT_FUNCTION_ZOOM_OUT))
			{
				retval = fn_id;
				break;
			} else if (	(fn_id != curr_function_id) &&
							(fn_id == INPUT_FUNCTION_ZOOM_IN || fn_id == INPUT_FUNCTION_ZOOM_OUT) &&
							(curr_function_id == INPUT_FUNCTION_ZOOM_IN || curr_function_id == INPUT_FUNCTION_ZOOM_OUT))
			{
				retval = fn_id;
				break;
			}
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	On_InputCtrl_Get_Key_Info
//
////////////////////////////////////////////////////////////////
bool
ControlsListTabClass::On_InputCtrl_Get_Key_Info
(
	InputCtrlClass *	viewer_ctrl,
	int					ctrl_id,
	int					vkey_id,
	WideStringClass &	key_name,
	int *					game_key_id
)
{
	bool retval = false;

	//
	//	First, determine what DIK key this corresponds to
	//
	int dik_id = 0;
	switch (vkey_id)
	{
		case 0:
			dik_id = 0;
			break;

		case VK_LBUTTON:
			dik_id = DirectInput::BUTTON_MOUSE_LEFT;
			break;
		
		case VK_RBUTTON:
			dik_id = DirectInput::BUTTON_MOUSE_RIGHT;
			break;

		case VK_MBUTTON:
			dik_id = DirectInput::BUTTON_MOUSE_CENTER;
			break;

		case VK_MOUSEWHEEL_UP:
			dik_id = Input::SLIDER_MOUSE_WHEEL_FORWARD;
			break;

		case VK_MOUSEWHEEL_DOWN:
			dik_id = Input::SLIDER_MOUSE_WHEEL_BACKWARD;
			break;

		default:
		{			
			//
			//	Get the last key pressed from direct input
			//
			dik_id = DirectInput::Get_Last_Key_Pressed ();
			break;
		}
	}

	//
	//	Return the DIK key ID to the caller
	//
	(*game_key_id) = dik_id;
	if (dik_id != 0) {
		Input::Get_Translated_Key_Name (dik_id, key_name);
	}

	//
	//	Get the function ID associated with this control
	//
	int function_id = int(viewer_ctrl->Get_User_Data ()) - 1;
	if (function_id >= 0) {		

		//
		//	Do nothing for these keys....
		//
		if (	dik_id != DIK_APPS && dik_id != DIK_WIN &&
				dik_id != DIK_LWIN && dik_id != DIK_RWIN && 
				dik_id != DIK_LCONTROL && dik_id != DIK_RCONTROL && 
				dik_id != DIK_LALT && dik_id != DIK_RALT && dik_id != DIK_ALT && 
				dik_id != DIK_DELETE && dik_id != 0)
		{
			retval = true;

			//
			//	Is this key already mapped to a function?
			//
			int old_function_id = Find_Function_By_Key (function_id, dik_id);
			if (old_function_id == -1 || old_function_id == function_id) {

				//
				//	The key is unmapped, so allow the remap
				//
				Remap_Key (ctrl_id, function_id, dik_id);
			} else {
				
				//
				//	Check to see if its OK to remap this key...
				//
				PendingCtrlID				= ctrl_id;
				PendingFunctionID			= function_id;
				PendingDIK_ID				= dik_id;
				PendingOldFunctionID		= old_function_id;
				Prompt_User ();
			}

		} else if (dik_id == DIK_DELETE) {
			retval = true;
			
			//
			//	Clear the mapping.
			//
			Remap_Key (ctrl_id, function_id, 0);
			key_name = U_CHAR("");
			(*game_key_id) = 0;
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Prompt_User
//
////////////////////////////////////////////////////////////////
void
ControlsListTabClass::Prompt_User (void)
{
	//
	//	Get the name of the key we're remapping...
	//
	WideStringClass key_name;
	Input::Get_Translated_Key_Name (PendingDIK_ID, key_name);

	//
	//	Get the name of the function we're remapping
	//
	WideStringClass function_name = Get_Function_Name (PendingOldFunctionID);

	WideStringClass message (0, true);
	message.Format (TRANSLATE (IDS_CONTROL_REMAP_WARNING),
				(const unichar_t *)key_name, (const unichar_t *)function_name);

	//
	//	Display the message box
	//
	DlgMsgBox::DoDialog (0, message, DlgMsgBox::YesNo, this);	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Remap_Key
//
////////////////////////////////////////////////////////////////
void
ControlsListTabClass::Remap_Key (int ctrl_id, int function_id, int dik_id)
{
	//
	//	Determine if this is a primary or secondary key mapping
	//
	if (PrimaryCtrlIDList.ID (ctrl_id) != -1) {
		
		//
		//	Modify the key mapping
		//
		Input::Set_Primary_Key_For_Function (function_id, dik_id);
	} else if (SecondaryCtrlIDList.ID (ctrl_id) != -1) {
		
		//
		//	Modify the key mapping
		//
		Input::Set_Secondary_Key_For_Function (function_id, dik_id);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Load_Key_Mappings
//
////////////////////////////////////////////////////////////////
void
ControlsListTabClass::Load_Key_Mappings (void)
{
	for (int index = 0; index < FunctionIDList.Count (); index ++) {

		//
		//	Get the dialog control that will represent this input
		//
		InputCtrlClass *input_ctrl = (InputCtrlClass *)Get_Dlg_Item (PrimaryCtrlIDList[index]);
		if (input_ctrl != NULL) {
			WideStringClass key_name;

			//
			//	Get the primary input for this function
			//
			int dik_id = Input::Get_Primary_Key_For_Function (FunctionIDList[index]);
			if (dik_id >= 0) {
				Input::Get_Translated_Key_Name (dik_id, key_name);
			}

			//
			//	Set the key assigment for this input
			//
			input_ctrl->Set_Key_Assignment (dik_id, key_name);
			input_ctrl->Set_User_Data (FunctionIDList[index] + 1);
		}

		//
		//	Get the dialog control that will represent this input
		//
		input_ctrl = (InputCtrlClass *)Get_Dlg_Item (SecondaryCtrlIDList[index]);
		if (input_ctrl != NULL) {
			WideStringClass key_name;
		
			//
			//	Get the secondary input for this function
			//
			int dik_id = Input::Get_Secondary_Key_For_Function (FunctionIDList[index]);
			if (dik_id >= 0) {
				Input::Get_Translated_Key_Name (dik_id, key_name);
			}

			//
			//	Set the key assigment for this input
			//
			input_ctrl->Set_Key_Assignment (dik_id, key_name);
			input_ctrl->Set_User_Data (FunctionIDList[index] + 1);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Reload
//
////////////////////////////////////////////////////////////////
void
ControlsListTabClass::On_Reload (void)
{
	Load_Key_Mappings ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Add_Function
//
////////////////////////////////////////////////////////////////
void
ControlsListTabClass::Add_Function (int function_id, int pri_ctrl_id, int sec_ctrl_id)
{
	FunctionIDList.Add (function_id);
	PrimaryCtrlIDList.Add (pri_ctrl_id);
	SecondaryCtrlIDList.Add (sec_ctrl_id);
	return ;
}



////////////////////////////////////////////////////////////////
//
//	HandleNotification
//
////////////////////////////////////////////////////////////////
void
ControlsListTabClass::HandleNotification (DlgMsgBoxEvent &event)
{
	if (event.Event () == DlgMsgBoxEvent::Yes) {

		//
		//	Clear all functions that currently use this key
		//
		bool clear_zoom = (PendingFunctionID == INPUT_FUNCTION_ZOOM_IN) || (PendingFunctionID == INPUT_FUNCTION_ZOOM_OUT);
		Clear_Key (PendingDIK_ID, clear_zoom);

		//
		//	Proceed with the re-mapping
		//
		Remap_Key (PendingCtrlID, PendingFunctionID, PendingDIK_ID);

		//
		//	Now, update all the controls on the dialog
		//
		DialogBaseClass *parent_dlg = Get_Parent_Dialog ();
		if (parent_dlg != NULL) {
			((ControlsMenuClass *)parent_dlg)->Reload ();
		}

		//
		//	Return focus to the input ctrl
		//
		Get_Dlg_Item (PendingCtrlID)->Set_Focus ();

	} else if (event.Event () == DlgMsgBoxEvent::No) {
		
		//
		//	Reload the mappings on this page
		//
		Load_Key_Mappings ();
		Get_Dlg_Item (PendingCtrlID)->Set_Focus ();

		PendingCtrlID			= 0;
		PendingFunctionID		= 0;
		PendingDIK_ID			= 0;
		PendingOldFunctionID	= 0;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Clear_Key
//
////////////////////////////////////////////////////////////////
void
ControlsListTabClass::Clear_Key (int dik_id, bool clear_zoom)
{
	//
	//	First, clear all functions that use this key as a primary key
	//
	int function_id = Input::Find_First_Function_By_Primary_Key (dik_id);
	while (function_id != -1) {
		if (	clear_zoom ||
				(function_id != INPUT_FUNCTION_ZOOM_IN &&
				function_id != INPUT_FUNCTION_ZOOM_OUT))
		{
			Input::Set_Primary_Key_For_Function (function_id, 0);
		}

		function_id = Input::Find_Next_Function_By_Primary_Key (function_id, dik_id);
	}

	//
	//	Now, clear all functions that use this key as a secondary key
	//
	function_id = Input::Find_First_Function_By_Secondary_Key (dik_id);
	while (function_id != -1) {
		if (	clear_zoom ||
				(function_id != INPUT_FUNCTION_ZOOM_IN &&
				function_id != INPUT_FUNCTION_ZOOM_OUT))
		{
			Input::Set_Secondary_Key_For_Function (function_id, 0);
		}

		function_id = Input::Find_Next_Function_By_Secondary_Key (function_id, dik_id);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Function_Name
//
////////////////////////////////////////////////////////////////
const unichar_t *
ControlsListTabClass::Get_Function_Name (int function_id)
{
	const unichar_t *retval = NULL;
	for (int index = 0; index < INPUT_NAME_COUNT; index ++) {
		
		//
		//	Is this the function we're looking for?
		//
		if (FUNCTION_NAMES[index].function_id == function_id) {			
			retval = TRANSLATE (FUNCTION_NAMES[index].name_id);
			break;
		}
	}

	return retval;
}

