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
 *                     $Archive:: /Commando/Code/wwui/dialogbase.cpp             $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/09/02 5:48p                                               $*
 *                                                                                             *
 *                    $Revision:: 38                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dialogbase.h"
#include "dialogcontrol.h"
#include "buttonctrl.h"
#include "dialogtext.h"
#include "editctrl.h"
#include "dialogmgr.h"
#include "ww3d.h"
#include "vector4.h"
#include "assetmgr.h"
#include "texture.h"
#include "mousemgr.h"
#include "comboboxctrl.h"
#include "checkboxctrl.h"
#include "sliderctrl.h"
#include "scrollbarctrl.h"
#include "menuentryctrl.h"
#include "tabctrl.h"
#include "listctrl.h"
#include "childdialog.h"
#include "mapctrl.h"
#include "viewerctrl.h"
#include "inputctrl.h"
#include "shortcutbarctrl.h"
#include "merchandisectrl.h"
#include "imagectrl.h"
#include "multilinetextctrl.h"
#include "treectrl.h"
#include "textmarqueectrl.h"
#include "ProgressCtrl.h"
#include "healthbarctrl.h"
#include "systimer.h"
#include "translatedb.h"

#include <dinput.h>
#include <algorithm>


////////////////////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////////////////////
const float	RES_SCREEN_WIDTH	= 400;
const float	RES_SCREEN_HEIGHT	= 300;

////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
DEFAULT_DLG_CMD_HANDLER		DialogBaseClass::DefaultCmdHandler = NULL;

static WideStringClass TranslateDialogString(const wchar_t *text)
{
	WideStringClass result = text;
	if (text) {
		wchar_t *string_id = ::wcsstr(result.Peek_Buffer(), L"IDS_");
		if (string_id) {
			WideStringClass wide_string_id = string_id;
			StringClass ascii_string_id;
			wide_string_id.Convert_To(ascii_string_id);
			const wchar_t *translation = TRANSLATE_BY_DESC(ascii_string_id);
			result.Erase(string_id - result.Peek_Buffer(), result.Get_Length());
			result += translation;
		}
	}
	return result;
}

////////////////////////////////////////////////////////////////
//
//	DialogBaseClass
//
////////////////////////////////////////////////////////////////
DialogBaseClass::DialogBaseClass (const DialogResource *dialog_resource)	:
	DialogResource_ (dialog_resource),
	AreControlsHidden (false),
	LastFocusControl (NULL),
	LastMouseClickTime (0),
	IsVisible (true),
	IsRunning (false)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~DialogBaseClass
//
////////////////////////////////////////////////////////////////
DialogBaseClass::~DialogBaseClass (void)
{
	Free_Controls ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Start_Dialog
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::Start_Dialog (void)
{
	Title = TranslateDialogString(DialogResource_->caption);

	//
	//	Convert the dialog's width and height from dialog units to screen units
	//
	const RectClass &screen_rect	= Render2DClass::Get_Screen_Resolution ();
	int dlg_screen_width				= int(((float)DialogResource_->cx / RES_SCREEN_WIDTH) * screen_rect.Width ());
	int dlg_screen_height			= int(((float)DialogResource_->cy / RES_SCREEN_HEIGHT) * screen_rect.Height ());

	//
	//	Center the dialog on the screen
	//
	Rect.Left	= int(screen_rect.Center ().X - ((float)dlg_screen_width * 0.5F));
	Rect.Top		= int(screen_rect.Center ().Y - ((float)dlg_screen_height * 0.5F));
	Rect.Right	= int(Rect.Left + dlg_screen_width);
	Rect.Bottom	= int(Rect.Top + dlg_screen_height);

	//
	//	Now create the controls
	//
	for (size_t index = 0; index < DialogResource_->count_controls; index ++) {

		const DialogResourceControl &info = DialogResource_->controls[index];

		DialogControlClass *control = NULL;
		switch (info.type)
		{
			case CONTROL_BUTTON:
				if (	(info.style & 0xF) == BS_CHECKBOX ||
						(info.style & 0xF) == BS_AUTOCHECKBOX)
				{
					control = new CheckBoxCtrlClass;
				} else if (info.style & BS_FLAT) {
					control = new MenuEntryCtrlClass;
				} else {
					control = new ButtonCtrlClass;
				}

				break;

			case CONTROL_STATIC:
				if ((info.style & 0xF) == SS_BITMAP) {
					control = new ImageCtrlClass;
				} else {
					control = new DialogTextClass;
				}

				break;

			case CONTROL_EDIT:
				if (info.style & ES_MULTILINE) {

					if (info.style & ES_AUTOVSCROLL) {
						control = new TextMarqueeCtrlClass;
					} else {
						control = new MultiLineTextCtrlClass;
					}

				} else {
					control = new EditCtrlClass;
				}
				break;

			case CONTROL_COMBOBOX:
				control = new ComboBoxCtrlClass;
				break;

			case CONTROL_SLIDER:
				control = new SliderCtrlClass;
				break;

			case CONTROL_SCROLL_BAR:
				control = new ScrollBarCtrlClass;
				break;

			case CONTROL_TAB:
				control = new TabCtrlClass;
				break;

			case CONTROL_LIST_CTRL:
				control = new ListCtrlClass;
				break;

			case CONTROL_TREE_CTRL:
				control = new TreeCtrlClass;
				break;

			case CONTROL_MAP:
				control = new MapCtrlClass;
				break;

			case CONTROL_VIEWER:
				control = new ViewerCtrlClass;
				break;

			case CONTROL_HOTKEY:
				control = new InputCtrlClass;
				break;

			case CONTROL_SHORTCUT_BAR:
				control = new ShortcutBarCtrlClass;
				break;

			case CONTROL_MERCHANDISE_CTRL:
				control = new MerchandiseCtrlClass;
				break;

			case CONTROL_PROGRESS_BAR:
				control = new ProgressCtrlClass;
				break;

			case CONTROL_HEALTH_BAR:
				control = new HealthBarCtrlClass;
				break;
		}

		//
		//	Add the control to the list (if necessary)
		//
		if (control != NULL) {

			//
			//	Set the generic control information
			//
			control->Set_Parent (this);
			control->Set_Text (TranslateDialogString(info.text));
			control->Set_Style (info.style);
			control->Set_ID (info.id);

			int ctrl_width		= int((((float)info.cx) / RES_SCREEN_WIDTH) * screen_rect.Width ());
			int ctrl_height	= int((((float)info.cy) / RES_SCREEN_HEIGHT) * screen_rect.Height ());

			//
			//	Calculate the screen position of the control
			//
			RectClass rect;
			rect.Left	= int(Rect.Left + ((((float)info.x) / RES_SCREEN_WIDTH) * screen_rect.Width ()));
			rect.Top		= int(Rect.Top + ((((float)info.y) / RES_SCREEN_HEIGHT) * screen_rect.Height ()));
			rect.Right	= int(rect.Left + ctrl_width);
			rect.Bottom	= int(rect.Top + ctrl_height);
			control->Set_Window_Rect (rect);

			//
			//	Let the control know its been created
			//
			control->On_Create ();
			ControlList.Add (control);

			//
			//	Apply the disabled style (if necessary)
			//
			if (info.style & WS_DISABLED) {
				control->Enable (false);
			}
		}
	}

	DialogMgrClass::Register_Dialog (this);
	IsRunning = true;

	DialogEvent dlgEvent(DialogEvent::STARTED, this);
	NotifyObservers(dlgEvent);

	//
	//	Allow derived clases to hook into this call
	//
	On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	End_Dialog
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::End_Dialog (void)
{
	DialogEvent dlgEvent(DialogEvent::DESTROY, this);
	NotifyObservers(dlgEvent);

	On_Destroy ();

	//
	//	Noitify all controls
	//
	int index;
	for (index = 0; index < ControlList.Count (); index ++) {
		ControlList[index]->On_Destroy ();
	}

	//
	//	Release all child dialogs
	//
	for (index = 0; index < ChildDialogList.Count (); index ++) {
		ChildDialogList[index]->End_Dialog ();
		ChildDialogList[index]->Set_Parent_Dialog (NULL);
		ChildDialogList[index]->Release_Ref ();
	}

	//
	//	Free the dialog list
	//
	ChildDialogList.Delete_All ();

	IsRunning = false;
	DialogMgrClass::UnRegister_Dialog (this);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::On_Frame_Update (void)
{
	Add_Ref ();

	if (AreControlsHidden == false) {

		//
		//	Only take input from the mouse or keyboard if this is the
		// currently active dialog.
		//
		if (Is_Active ()) {
			Update_Mouse_State ();
		}

		//
		//	Let each control "think"
		//
		int index;
		for (index = 0; index < ControlList.Count (); index ++) {
			ControlList[index]->On_Frame_Update ();
		}

		//
		//	Let each control "think"
		//
		for (index = 0; index < ChildDialogList.Count (); index ++) {
			ChildDialogList[index]->On_Frame_Update ();
		}
	}

	Release_Ref ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::Render (void)
{
	Add_Ref ();

	//
	//	Render each control
	//
	if (AreControlsHidden == false) {
		int index;
		for (index = 0; index < ControlList.Count (); index ++) {
			if (ControlList[index]->Is_Visible ()) {
				ControlList[index]->Render ();
			}
		}

		//
		//	Render each child - dialog
		//
		for (index = 0; index < ChildDialogList.Count (); index ++) {
			if (ChildDialogList[index]->Is_Visible ()) {
				ChildDialogList[index]->Render ();
			}
		}
	}

	Release_Ref ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Enable_Dlg_Item
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::Enable_Dlg_Item (int id, bool onoff)
{
	//
	//	Find the control
	//
	DialogControlClass *control = Get_Dlg_Item (id);
	if (control != NULL) {

		//
		//	Change the enable state of hte control
		//
		control->Enable (onoff);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Is_Dlg_Item_Enabled
//
////////////////////////////////////////////////////////////////
bool
DialogBaseClass::Is_Dlg_Item_Enabled (int id)
{
	bool retval = false;

	//
	//	Find the control
	//
	DialogControlClass *control = Get_Dlg_Item (id);
	if (control != NULL) {

		//
		//	Return the state of the control to the caller
		//
		retval = control->Is_Enabled ();
	}

	return retval;
}



////////////////////////////////////////////////////////////////
//
//	Get_Dlg_Item
//
////////////////////////////////////////////////////////////////
DialogControlClass *
DialogBaseClass::Get_Dlg_Item (int id) const
{
	DialogControlClass *retval = NULL;

	//
	//	Simply loop over all the controls in our list until we
	// find one with the matching ID.
	//
	for (int index = 0; index < ControlList.Count (); index ++) {
		if (id == ControlList[index]->Get_ID ()) {
			retval = ControlList[index];
			break;
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Get_Dlg_Item_Text
//
////////////////////////////////////////////////////////////////
const wchar_t *
DialogBaseClass::Get_Dlg_Item_Text (int id) const
{
	const wchar_t *retval = NULL;

	//
	//	Find the control
	//
	DialogControlClass *control = Get_Dlg_Item (id);
	if (control != NULL) {

		//
		//	Return the control's text to the caller
		//
		retval = control->Get_Text ();
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Set_Dlg_Item_Text
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::Set_Dlg_Item_Text (int id, const wchar_t *text)
{
	//
	//	Find the control
	//
	DialogControlClass *control = Get_Dlg_Item (id);
	if (control != NULL) {

		//
		//	Set the text of this control
		//
		control->Set_Text (text);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Dlg_Item_Int
//
////////////////////////////////////////////////////////////////
int
DialogBaseClass::Get_Dlg_Item_Int (int id) const
{
	int retval = 0;

	//
	//	Find the control
	//
	DialogControlClass *control = Get_Dlg_Item (id);
	if (control != NULL) {

		//
		//	Get the text from the control
		//
		const wchar_t *text = control->Get_Text ();

		//
		//	Convert the text to an integer
		//
		retval = _wtoi (text);
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Set_Dlg_Item_Int
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::Set_Dlg_Item_Int (int id, int value)
{
	//
	//	Find the control
	//
	DialogControlClass *control = Get_Dlg_Item (id);
	if (control != NULL) {

		//
		//	Convert the value to a string
		//
		WideStringClass text;
		text.Format (L"%d", value);

		//
		//	Set the text of this control
		//
		control->Set_Text (text);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Dlg_Item_Float
//
////////////////////////////////////////////////////////////////
float
DialogBaseClass::Get_Dlg_Item_Float (int id) const
{
	float retval = 0;

	//
	//	Find the control
	//
	DialogControlClass *control = Get_Dlg_Item (id);
	if (control != NULL) {

		//
		//	Get the text from the control
		//
		const wchar_t *text = control->Get_Text ();

		//
		//	Convert the text to an float
		//
		swscanf (text, L"%f", &retval);
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Set_Dlg_Item_Float
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::Set_Dlg_Item_Float (int id, float value)
{
	//
	//	Find the control
	//
	DialogControlClass *control = Get_Dlg_Item (id);
	if (control != NULL) {

		//
		//	Convert the value to a string
		//
		WideStringClass text;
		text.Format (L"%.2f", value);

		//
		//	Set the text of this control
		//
		control->Set_Text (text);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Check_Dlg_Button
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::Check_Dlg_Button (int id, bool onoff)
{
	//
	//	Find the control
	//
	DialogControlClass *control = Get_Dlg_Item (id);
	if (control != NULL && control->As_CheckBoxCtrlClass () != NULL) {

		//
		//	Set the check state of this control
		//
		control->As_CheckBoxCtrlClass ()->Set_Check (onoff);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Is_Dlg_Button_Checked
//
////////////////////////////////////////////////////////////////
bool
DialogBaseClass::Is_Dlg_Button_Checked (int id) const
{
	bool retval = false;

	//
	//	Find the control
	//
	DialogControlClass *control = Get_Dlg_Item (id);
	if (control != NULL && control->As_CheckBoxCtrlClass () != NULL) {

		//
		//	Return the check state of this control to the caller
		//
		retval = control->As_CheckBoxCtrlClass ()->Get_Check ();
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Free_Controls
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::Free_Controls (void)
{
	for (int index = 0; index < ControlList.Count (); index ++) {
		if (ControlList[index]->Is_Embedded () == false) {
			delete ControlList[index];
		}
	}

	ControlList.Delete_All ();

	//
	//	Reset our pointers
	//
	//FocusControl = NULL;
	LastFocusControl = NULL;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Send_Mouse_Input
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::Send_Mouse_Input (DialogControlClass *control, const Vector2 &mouse_pos)
{
	//
	//	Check to see if the mouse is moving or not
	//
	const Vector3 &last_mouse_pos = DialogMgrClass::Get_Last_Mouse_Pos ();
	if (last_mouse_pos.X != mouse_pos.X || last_mouse_pos.Y != mouse_pos.Y) {
		control->On_Mouse_Move (mouse_pos);
	}

	//
	//	Let the control change the mouse cursor
	//
	control->On_Set_Cursor (mouse_pos);

	//
	//	Notify the control on button down or up
	//
	if (DialogMgrClass::Is_Button_Down (VK_LBUTTON) != DialogMgrClass::Was_Button_Down (VK_LBUTTON)) {
		if (DialogMgrClass::Is_Button_Down (VK_LBUTTON)) {

			int curr_time = TIMEGETTIME ();

			//
			//	Did we just get a double-click?
			//
			if ((curr_time - LastMouseClickTime) < 300) {
				control->On_LButton_DblClk (mouse_pos);
			} else {
				control->On_LButton_Down (mouse_pos);
			}

			LastMouseClickTime = TIMEGETTIME ();
		} else {

			//
			//	Notify the control
			//
			control->On_LButton_Up (mouse_pos);
		}
	}

	//
	//	Notify the control on button down or up
	//
	if (DialogMgrClass::Is_Button_Down (VK_RBUTTON) != DialogMgrClass::Was_Button_Down (VK_RBUTTON)) {
		if (DialogMgrClass::Is_Button_Down (VK_RBUTTON)) {
			control->On_RButton_Down (mouse_pos);
		} else {
			control->On_RButton_Up (mouse_pos);
		}
	}

	//
	//	Notify the control on button down or up
	//
	if (DialogMgrClass::Is_Button_Down (VK_MBUTTON) != DialogMgrClass::Was_Button_Down (VK_MBUTTON)) {
		if (DialogMgrClass::Is_Button_Down (VK_MBUTTON)) {
			control->On_MButton_Down (mouse_pos);
		} else {
			control->On_MButton_Up (mouse_pos);
		}
	}

	return;
}


////////////////////////////////////////////////////////////////
//
//	Update_Mouse_State
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::Update_Mouse_State (void)
{
	//
	//	Get the cursor's position
	//
	Vector3 cursor_pos = DialogMgrClass::Get_Mouse_Pos ();
	Vector2 screen_pos (cursor_pos.X, cursor_pos.Y);

	DialogControlClass *current_control = NULL;

	//
	//	Build a complete list of controls
	//
	CONTROL_LIST control_list;
	Build_Control_List (control_list);

	//
	//	Loop over all the controls on the dialog
	//
	bool mouse_over_control = false;
	for (int index = control_list.Count () - 1; index >= 0; index --) {
		const RectClass &rect = control_list[index]->Get_Window_Rect ();

		//
		//	Is the mouse point inside this control?
		//
		if (rect.Contains (screen_pos)) {

			//
			//	Check for "focus" change
			//
			if (DialogMgrClass::Is_Button_Down (VK_LBUTTON) != DialogMgrClass::Was_Button_Down (VK_LBUTTON)) {
				if (DialogMgrClass::Is_Button_Down (VK_LBUTTON)) {

					//
					//	Handle the focus change (if necessary)
					//
					if (	control_list[index]->Wants_Focus () &&
							control_list[index]->Is_Enabled ())
					{
						DialogMgrClass::Set_Focus (control_list[index]);
					}
				}
			}

			current_control = control_list[index];
			mouse_over_control = true;
			break;
		}
	}

	DialogControlClass *input_capture	= DialogMgrClass::Get_Capture ();
	DialogControlClass *control			= input_capture ? input_capture : current_control;
	if (control != NULL && control->Is_Enabled () && control->Is_Visible ()) {

		//
		//	Send mouse input to the control
		//
		Send_Mouse_Input (control, screen_pos);

	} else {

		//
		//	Reset the mouse cursor to an arrow if no control changed it...
		//
		MouseMgrClass::Set_Cursor (MouseMgrClass::CURSOR_ARROW);

		//
		//	If the user clicked in empty space, then NULL out the focus'd control
		//
		if (	DialogMgrClass::Is_Button_Down (VK_LBUTTON) &&
				DialogMgrClass::Was_Button_Down (VK_LBUTTON))
		{
			DialogMgrClass::Set_Focus (NULL);
		}
	}

	//
	//	Check for mouse wheel activity
	//
	const Vector3 &last_mouse_pos = DialogMgrClass::Get_Last_Mouse_Pos ();
	if (last_mouse_pos.Z != cursor_pos.Z && DialogMgrClass::Get_Focus () != NULL) {
		DialogMgrClass::Get_Focus ()->On_Mouse_Wheel ((int)(last_mouse_pos.Z - cursor_pos.Z));
	}

	//
	//	Store our cached button states
	//
	DialogMgrClass::Set_Last_Mouse_Pos (cursor_pos);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	//
	//	Allow the default handler to process this command
	//
	if (DefaultCmdHandler != NULL) {
		(*DefaultCmdHandler) (this, ctrl_id, message_id, param);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Activate
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::On_Activate (bool onoff)
{
	if (onoff == false) {

		//
		//	Cache the current focus control between dialogs (if possible)
		//
		for (int index = 0; index < ControlList.Count (); index ++) {
			if (ControlList[index] == DialogMgrClass::Get_Focus ()) {
				LastFocusControl = ControlList[index];
				break;
			}
		}


		//
		//	Let go of the currently focus'd control
		//
		DialogMgrClass::Set_Focus (NULL);

		DialogEvent dlgEvent(DialogEvent::DEACTIVATED, this);
		NotifyObservers(dlgEvent);

	} else {

		if (LastFocusControl != NULL) {
			DialogMgrClass::Set_Focus (LastFocusControl);
		} else {
			Set_Default_Focus ();
		}

		DialogEvent dlgEvent(DialogEvent::ACTIVATED, this);
		NotifyObservers(dlgEvent);

	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Is_Active
//
////////////////////////////////////////////////////////////////
bool
DialogBaseClass::Is_Active (void)
{
	return (DialogMgrClass::Get_Active_Dialog () == this);
}


////////////////////////////////////////////////////////////////
//
//	On_Key_Down
//
////////////////////////////////////////////////////////////////
bool
DialogBaseClass::On_Key_Down (uint32 key_id, uint32 key_data)
{
	bool handled = false;

	Add_Ref ();

	//
	//	Determine what control to send the input to
	//
	DialogControlClass *input_capture	= DialogMgrClass::Get_Capture ();
	DialogControlClass *input				= input_capture ? input_capture : DialogMgrClass::Get_Focus ();

	//
	//	Special case some of the keys
	//
	if (key_id == VK_ESCAPE) {
		On_Command (IDCANCEL, 0, 0);
		handled = true;
	} else if (key_id == VK_TAB) {

		//
		//	Determine whether to go forward or backward through the controls
		//
		int inc = 1;
		if ((DialogMgrClass::Get_VKey_State (VK_SHIFT) & VKEY_PRESSED) == VKEY_PRESSED) {
			inc = -1;
		}

		//
		//	Find the next (or previous) control
		//
		DialogControlClass *control = Find_Next_Control (DialogMgrClass::Get_Focus (), inc);

		//
		//	Set the new focus control
		//
		if (control != NULL) {
			DialogMgrClass::Set_Focus (control);
			handled = true;
		}

	} else if (input != NULL) {

		//
		//	Send out the standard On_OK notification if the user
		// pressed enter
		//
		if (	input->As_ButtonCtrlClass () == NULL &&
				input->As_MenuEntryCtrlClass () == NULL &&
				key_id == VK_RETURN)
		{
			On_Command (IDOK, 0, 0);
			handled = true;
		}

		//
		//	Pass keyboard input onto the control
		//
		handled = input->On_Key_Down (key_id, key_data);
	}

	Release_Ref ();
	return handled;
}


////////////////////////////////////////////////////////////////
//
//	On_Key_Up
//
////////////////////////////////////////////////////////////////
bool
DialogBaseClass::On_Key_Up (uint32 key_id)
{
	bool handled = false;

	Add_Ref ();

	DialogControlClass *input_capture	= DialogMgrClass::Get_Capture ();
	DialogControlClass *input				= input_capture ? input_capture : DialogMgrClass::Get_Focus ();

	//
	//	Pass keyboard input onto the control
	//
	if (input != NULL) {
		handled = input->On_Key_Up (key_id);
	}

	Release_Ref ();
	return handled;
}


////////////////////////////////////////////////////////////////
//
//	On_Unicode_Char
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::On_Unicode_Char(uint16 unicode)
{
	Add_Ref ();

	//
	//	Determine what control to send the input to
	//
	DialogControlClass *input_capture	= DialogMgrClass::Get_Capture ();
	DialogControlClass *input				= input_capture ? input_capture : DialogMgrClass::Get_Focus ();

	if (input != NULL) {

		//
		//	Pass keyboard input onto the control
		//
		input->On_Unicode_Char (unicode);
	}

	Release_Ref ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Find_Next_Control
//
////////////////////////////////////////////////////////////////
DialogControlClass *
DialogBaseClass::Find_Next_Control
(
	DialogControlClass *	control,
	int						direction
)
{
	DialogControlClass *retval = NULL;

	//
	//	Build a complete list of controls
	//
	CONTROL_LIST control_list;
	Build_Control_List (control_list);

	//
	//	Find the index of the control
	//
	int focus_index		= std::max (control_list.ID (control), 0);
	int new_focus_index	= -1;
	int inc					= (direction < 0) ? -1 : 1;


	//
	//	Find the next control in the sequence
	//
	int count = control_list.Count ();
	for (int index = focus_index; index >= 0 && index < count; index += inc) {

		//
		//	Bound the index
		//
		new_focus_index = index + inc;
		if (new_focus_index >= count) {
			new_focus_index  = 0;
		} else if (new_focus_index < 0) {
			new_focus_index = count - 1;
		}

		//
		//	Does this control want the focus?
		//
		if (	control_list[new_focus_index]->Wants_Focus () &&
				control_list[new_focus_index]->Is_Enabled ())
		{
			break;
		}
	}

	if (new_focus_index >= 0) {
		retval = control_list[new_focus_index];
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Find_Next_Group_Control
//
////////////////////////////////////////////////////////////////
DialogControlClass *
DialogBaseClass::Find_Next_Group_Control
(
	DialogControlClass *	control,
	int						direction
)
{
	//
	//	Build a complete list of controls
	//
	CONTROL_LIST control_list;
	Build_Control_List (control_list);

	//
	//	Remove any disabled controls
	//
	/*for (int index = control_list.Count () - 1; index >= 0; index --) {
		if (control_list[index]->Is_Enabled () == false) {
			control_list.Delete (index);
		}
	}*/

	//
	//	Find the index of the control
	//
	int curr_index		= std::max (control_list.ID (control), 0);
	int new_index		= -1;

	if (direction < 0) {

		if (	(control_list[curr_index]->Get_Style () & WS_GROUP) ||
				(curr_index == 0))
		{
			//
			//	Wrap around to the last control in the group
			//
			for (int index = curr_index + 1; index < control_list.Count (); index ++) {

				//
				//	Is this the start of the next group?
				//
				if (control_list[index]->Get_Style () & WS_GROUP) {
					break;
				}

				new_index = index;
			}

		} else {

			//
			//	Simply move to the previous control (its in the group)
			//
			new_index = curr_index - 1;
		}

	} else {

		if (	curr_index == (control_list.Count () - 1) ||
				(control_list[curr_index + 1]->Get_Style () & WS_GROUP))
		{
			//
			//	Wrap around to the first control in the group
			//
			for (int index = curr_index; index >= 0; index --) {
				new_index = index;

				//
				//	Is this the first control in the group?
				//
				if (control_list[index]->Get_Style () & WS_GROUP) {
					break;
				}
			}

		} else {

			//
			//	Simply move to the next control (its in the group)
			//
			new_index = curr_index + 1;
		}
	}

	//
	//	Index into the list and return the pointer to the caller
	//
	DialogControlClass *retval = NULL;
	if (new_index >= 0) {
		retval = control_list[new_index];
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Find_Control_Index
//
////////////////////////////////////////////////////////////////
int
DialogBaseClass::Find_Control_Index (DialogControlClass *control)
{
	return ControlList.ID (control);
}


////////////////////////////////////////////////////////////////
//
//	Add_Control
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::Add_Control (DialogControlClass *control)
{
	//
	//	Simply add the control to the list
	//
	if (ControlList.ID (control) == -1) {
		ControlList.Add (control);
		control->Set_Parent (this);
		control->On_Add_To_Dialog ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Remove_Control
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::Remove_Control (DialogControlClass *control)
{
	//
	//	Simply remove the control from the list
	//
	int index = ControlList.ID (control);
	if (index != -1) {
		control->On_Remove_From_Dialog ();
		control->Set_Parent (NULL);

		//
		//	Remove the control from the list (note: its possible
		// the index might of changed since we last checked)
		//
		index = ControlList.ID (control);
		if (index != -1) {
			ControlList.Delete (index);
		}

		//
		//	Remove the capture if we just removed the control
		//
		if (control == DialogMgrClass::Get_Capture ()) {
			DialogMgrClass::Release_Capture ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Find_Control
//
////////////////////////////////////////////////////////////////
DialogControlClass *
DialogBaseClass::Find_Control (const Vector2 &mouse_pos)
{
	DialogControlClass *retval = NULL;

	//
	//	Build a complete list of controls
	//
	CONTROL_LIST control_list;
	Build_Control_List (control_list);

	//
	//	Loop over all the controls
	//
	for (int index = control_list.Count () - 1; index >= 0; index --) {

		//
		//	Is the mouse over this control?
		//
		const RectClass &rect = control_list[index]->Get_Window_Rect ();
		if (rect.Contains (mouse_pos)) {

			//
			//	Return the control to the caller
			//
			retval = control_list[index];
			break;
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Find_Focus_Control
//
////////////////////////////////////////////////////////////////
int
DialogBaseClass::Find_Focus_Control (void)
{
	int focus_index = -1;

	//
	//	Loop over all the controls in the list
	//
	for (int index = 0; index < ControlList.Count (); index ++) {
		if (DialogMgrClass::Get_Focus () == ControlList[index]) {
			focus_index = index;
			break;
		}
	}

	return focus_index;
}


////////////////////////////////////////////////////////////////
//
//	Add_Child_Dialog
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::Add_Child_Dialog (ChildDialogClass *child)
{
	//
	//	Simply add the dialog to our list
	//
	if (ChildDialogList.ID (child) == -1) {
		ChildDialogList.Add (child);
		child->Set_Parent_Dialog (this);
		child->Add_Ref ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Remove_Child_Dialog
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::Remove_Child_Dialog (ChildDialogClass *child)
{
	bool reset_focus = false;

	//
	//	Simply remove the control from the list
	//
	int index = ChildDialogList.ID (child);
	if (index != -1) {

		//
		//	Check to see if the current focus control belongs to this
		// dialog
		//
		if (	DialogMgrClass::Get_Focus () != NULL &&
				DialogMgrClass::Get_Focus ()->Peek_Parent () == child)
		{
			DialogMgrClass::Set_Focus (NULL);
			reset_focus = true;
		}

		//
		//	Remove the control from the list
		//
		child->Set_Parent_Dialog (NULL);
		child->Release_Ref ();
		ControlList.Delete (index);
	}

	//
	//	Reset the focus to the first control we find that wants it
	//
	if (reset_focus) {
		DialogMgrClass::Set_Focus (Find_Next_Control (NULL));
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Build_Control_List
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::Build_Control_List (CONTROL_LIST &list)
{
	//
	//	First off add all the controls on this dialog to the list
	//
	int index;
	for (index = 0; index < ControlList.Count (); index ++) {
		if (ControlList[index]->Is_Enabled ()) {
			list.Add (ControlList[index]);
		}
	}

	//
	//	Next, add in the control from each dialog
	//
	for (index = 0; index < ChildDialogList.Count (); index ++) {
		if (ChildDialogList[index]->Is_Visible ()) {
			ChildDialogList[index]->Build_Control_List (list);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Rect
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::Set_Rect (const RectClass &rect)
{
	if (rect == Rect) {
		return ;
	}

	Vector2 offset = rect.Upper_Left () - Rect.Upper_Left ();

	//
	//	Move all the controls
	//
	int index;
	for (index = 0; index < ControlList.Count (); index ++) {

		//
		//	Offset this control
		//
		if (ControlList[index]->Is_Embedded () == false) {
			Vector2 old_pos = ControlList[index]->Get_Window_Pos ();
			ControlList[index]->Set_Window_Pos (old_pos + offset);
		}
	}

	//
	//	Move all the child dialogs
	//
	for (index = 0; index < ChildDialogList.Count (); index ++) {

		//
		//	Offset this dialog
		//
		RectClass rect = ChildDialogList[index]->Get_Rect ();
		rect.Left	+= offset.X;
		rect.Right	+= offset.X;
		rect.Top		+= offset.Y;
		rect.Bottom	+= offset.Y;
		ChildDialogList[index]->Set_Rect (rect);
	}

	Rect = rect;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::On_Init_Dialog (void)
{
	if (DialogMgrClass::Get_Focus () == NULL) {
		Set_Default_Focus ();
	} else {
		//DialogMgrClass::Get_Focus ()->Center_Mouse ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Default_Focus
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::Set_Default_Focus (void)
{
	//
	//	Build a complete list of controls
	//
	CONTROL_LIST control_list;
	Build_Control_List (control_list);

	//
	//	Set the focus to the first control that wants it
	//
	for (int index = 0; index < control_list.Count (); index ++) {
		if (control_list[index]->Wants_Focus () && control_list[index]->Is_Enabled ()) {
			DialogMgrClass::Set_Focus (control_list[index]);

			//
			//	Put the mouse cursor in the middle of this control
			//
			//control_list[index]->Center_Mouse ();
			break;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Mouse_Wheel
//
////////////////////////////////////////////////////////////////
void
DialogBaseClass::On_Mouse_Wheel (int direction)
{
	if (DialogMgrClass::Get_Focus () != NULL) {
		DialogMgrClass::Get_Focus ()->On_Mouse_Wheel (direction);
	}

	return ;
}

void DialogBaseClass::Set_Dirty(bool onoff)
{
	int index;
	for (index = 0; index < ControlList.Count (); index ++) {
		ControlList[index]->Set_Dirty();
	}

	//
	//	Render each child - dialog
	//
	for (index = 0; index < ChildDialogList.Count (); index ++) {
		ChildDialogList[index]->Set_Dirty();
	}
}
