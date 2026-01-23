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
 *                     $Archive:: /Commando/Code/wwui/comboboxctrl.cpp         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/10/02 12:30p                                              $*
 *                                                                                             *
 *                    $Revision:: 25                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "comboboxctrl.h"
#include "assetmgr.h"
#include "refcount.h"
#include "font3d.h"
#include "mousemgr.h"
#include "ww3d.h"
#include "dialogmgr.h"
#include "dialogbase.h"
#include "stylemgr.h"
#include <algorithm>


////////////////////////////////////////////////////////////////
//
//	ComboBoxCtrlClass
//
////////////////////////////////////////////////////////////////
ComboBoxCtrlClass::ComboBoxCtrlClass (void)	:
	WasButtonPressedOnMe (false),
	IsDropDownDisplayed (false),
	TextRect (0, 0, 0, 0),
	ButtonRect (0, 0, 0, 0),
	FullRect (0, 0, 0, 0),
	DropDownSize (0, 0),
	IsInitialized (false),
	LastDropDownDisplayChange (0),
	CurrSel(-1)
{
	//
	//	Set the font for the text renderers
	//
	StyleMgrClass::Assign_Font (&TextRenderer, StyleMgrClass::FONT_LISTS);
	StyleMgrClass::Configure_Renderer (&ControlRenderer);

	//
	//	Configure the hilight renderer 
	//
	StyleMgrClass::Configure_Renderer (&HilightRenderer);

	//
	//	Let the drop down know who its parent is
	//
	DropDownCtrl.Set_Combo_Box	(this);
	DropDownCtrl.Set_Is_Embedded (true);
	EditControl.Set_Is_Embedded (true);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~ComboBoxCtrlClass
//
////////////////////////////////////////////////////////////////
ComboBoxCtrlClass::~ComboBoxCtrlClass (void)
{
	return ;
}


void ComboBoxCtrlClass::Set_Style(DWORD style)
{
	DialogControlClass::Set_Style(style);

	if ((style & CBS_OEMCONVERT) != 0) {
		EditControl.Set_Style(ES_OEMCONVERT);
	} else {
		EditControl.Set_Style(0);
	}
}


////////////////////////////////////////////////////////////////
//
//	Create_Text_Renderers
//
////////////////////////////////////////////////////////////////
void
ComboBoxCtrlClass::Create_Text_Renderers (void)
{
	if ((Style & 0x0F) == CBS_DROPDOWN) {
		return;
	}

	HilightRenderer.Reset ();
	HilightRenderer.Set_Coordinate_Range (Render2DClass::Get_Screen_Resolution ());
	StyleMgrClass::Configure_Hilighter (&HilightRenderer);

	//
	//	Lookup the string to display
	//
	WideStringClass display_string(255, true);
	DropDownCtrl.Get_String (DropDownCtrl.Get_Curr_Sel(), display_string);

	//
	//	Setup this renderer
	//
	Render2DSentenceClass &renderer = TextRenderer;
	renderer.Reset ();

	//
	//	Draw the text
	//
	StyleMgrClass::Render_Text (display_string, &renderer, TextRect, true, true,
							StyleMgrClass::LEFT_JUSTIFY, IsEnabled);

	//
	//	Do the hilight
	//
	if (HasFocus && IsEnabled) {
		StyleMgrClass::Render_Hilight (&HilightRenderer, TextRect);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Control_Renderers
//
////////////////////////////////////////////////////////////////
void
ComboBoxCtrlClass::Create_Control_Renderers (void)
{
	Render2DClass &renderer = ControlRenderer;

	//
	//	Configure this renderer
	//
	renderer.Reset ();
	renderer.Enable_Texturing (false);
	renderer.Set_Coordinate_Range (Render2DClass::Get_Screen_Resolution());

	//
	//	Determine which color to draw the outline in
	//
	int color	= StyleMgrClass::Get_Line_Color ();
	int bkcolor	= StyleMgrClass::Get_Bk_Color ();
	if (IsEnabled == false) {
		color		= StyleMgrClass::Get_Disabled_Line_Color ();
		bkcolor	= StyleMgrClass::Get_Disabled_Bk_Color ();
	}

	//
	//	Draw the control outline
	//
	RectClass rect = ClientRect;
	renderer.Add_Outline (rect, 1.0F, color);

	//
	//	Now draw the background
	//
	rect.Right	-= 1;
	rect.Bottom -= 1;
	renderer.Add_Quad (rect, bkcolor);

	//
	//	Draw the button
	//
	rect = ButtonRect;
	renderer.Add_Line (Vector2 (rect.Left, rect.Bottom),	Vector2 (rect.Left, rect.Top-1),		1, color);
	float width		= int(ButtonRect.Width () / 4);
	float height	= int(ButtonRect.Height () / 4);
	float center_x = int(ButtonRect.Left + (ButtonRect.Width () / 2));
	float center_y = int(ButtonRect.Top + (ButtonRect.Height () / 2));
	renderer.Add_Line (Vector2 (center_x - width, center_y - height),	Vector2 (center_x + width, center_y - height), 1, color);
	renderer.Add_Line (Vector2 (center_x + width, center_y - height),	Vector2 (center_x, center_y + height), 1, color);
	renderer.Add_Line (Vector2 (center_x, center_y + height),	Vector2 (center_x - width, center_y - height), 1, color);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Cursor
//
////////////////////////////////////////////////////////////////
void
ComboBoxCtrlClass::On_Set_Cursor (const Vector2 &mouse_pos)
{
	//
	//	Change the mouse cursor if necessary
	//
	if (ClientRect.Contains (mouse_pos)) {
		MouseMgrClass::Set_Cursor (MouseMgrClass::CURSOR_ACTION);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Window_Pos
//
////////////////////////////////////////////////////////////////
void
ComboBoxCtrlClass::Set_Window_Pos (const Vector2 &pos)
{
	float width		= FullRect.Width ();
	float height	= FullRect.Height ();

	//
	//	Recalculate the window's bounding rectangle
	//
	FullRect.Left	= (int)pos.X;
	FullRect.Top		= (int)pos.Y;
	FullRect.Right	= (int)(FullRect.Left + width);
	FullRect.Bottom	= (int)(FullRect.Top + height);

	//
	//	Let the control recalculate anything it needs
	//
	DialogControlClass::Set_Window_Pos (pos);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Client_Rect
//
////////////////////////////////////////////////////////////////
void
ComboBoxCtrlClass::Update_Client_Rect (void)
{
	if (IsInitialized == false) {
		FullRect = Rect;
		IsInitialized = true;
	}

	//
	//	Determine what one character spacing would be
	//
	Vector2 char_size = TextRenderer.Get_Text_Extents (U_CHAR("W"));

	float border_width	= char_size.X + 2;
	float border_height	= 2;

	float button_height	= int(char_size.Y * 2.0F);
	float button_width	= button_height;

	//
	//	Shrink the client area
	//
	ClientRect			= Rect;
	ClientRect.Bottom	= ClientRect.Top + button_height;

	//
	//	Remember how much space we have for the drop down control
	//
	DropDownSize.X = int(ClientRect.Width ());
	DropDownSize.Y = int(FullRect.Bottom - ClientRect.Bottom);
	
	//
	//	Calculate the bounding rectangle for the drop-down control
	//
	RectClass drop_down_rect;
	drop_down_rect.Left		= ClientRect.Left;
	drop_down_rect.Top		= ClientRect.Bottom;
	drop_down_rect.Right		= drop_down_rect.Left + DropDownSize.X;
	drop_down_rect.Bottom	= drop_down_rect.Top + DropDownSize.Y;

	//
	//	Should the drop-down, go down or up?
	//
	RectClass screen_rect = Render2DClass::Get_Screen_Resolution ();
	if (drop_down_rect.Bottom > screen_rect.Bottom) {
		drop_down_rect.Bottom	= ClientRect.Top;
		drop_down_rect.Top		= drop_down_rect.Bottom - DropDownSize.Y;
	}

	//
	//	Configure the drop-down control
	//
	DropDownCtrl.Set_Full_Rect (drop_down_rect);
	DropDownCtrl.Set_Window_Rect (drop_down_rect);

	//
	//	Make the window rect and the client rect the same
	//
	Rect = ClientRect;
	
	//
	//	Calculate where the button is located
	//
	ButtonRect			= ClientRect;
	ButtonRect.Left	= ClientRect.Right - button_width;
	ButtonRect.Bottom	= ClientRect.Top + button_height;

	//
	//	Calculate the area the text can render into
	//
	TextRect				= ClientRect;
	TextRect.Top		+= border_height;
	TextRect.Left		+= border_width;
	TextRect.Right		= ButtonRect.Left - border_width;
	TextRect.Bottom	= ButtonRect.Bottom - border_height;

	RectClass editRect = ClientRect;
	editRect.Right = ButtonRect.Left;
	editRect.Bottom	= ButtonRect.Bottom;
	EditControl.Set_Window_Rect(editRect);

	// If this is a dropdown then make the controls rect only cover the button.
	// The edit control will handle the rest of the control orginal area.
	if ((Style & 0x0F) == CBS_DROPDOWN) {
		ClientRect = ButtonRect;
		Rect = ButtonRect;
	}

	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
ComboBoxCtrlClass::Render (void)
{
	//
	//	Recreate the renderers (if necessary)
	//
	if (IsDirty) {		
		Create_Control_Renderers ();
		Create_Text_Renderers ();
	}

	//
	//	Render the background and text for the current state
	//
	ControlRenderer.Render ();
	TextRenderer.Render ();
	HilightRenderer.Render ();

	//
	//	Check to see if the mouse button is still down
	//
	WasButtonPressedOnMe	&= DialogMgrClass::Is_Button_Down (VK_LBUTTON);

	DialogControlClass::Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Down
//
////////////////////////////////////////////////////////////////
void
ComboBoxCtrlClass::On_LButton_Down (const Vector2 &/* mouse_pos */)
{
	if (HasFocus) {
		WasButtonPressedOnMe = true;
	}

//	Display_Drop_Down (true);
	Display_Drop_Down(!IsDropDownDisplayed);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Up
//
////////////////////////////////////////////////////////////////
void
ComboBoxCtrlClass::On_LButton_Up (const Vector2 &/* mouse_pos */)
{
	//
	//	Reset our flags
	//
	WasButtonPressedOnMe	= false;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Mouse_Move
//
////////////////////////////////////////////////////////////////
void
ComboBoxCtrlClass::On_Mouse_Move (const Vector2 &/* mouse_pos */)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Focus
//
////////////////////////////////////////////////////////////////
void
ComboBoxCtrlClass::On_Set_Focus (void)
{
	Set_Dirty ();

	//
	//	Set the focus to the edit control if we are in that mode
	//
	/*if ((Style & 0x0F) == CBS_DROPDOWN) {
		DialogMgrClass::Set_Focus(&EditControl);
	}*/

	DialogControlClass::On_Set_Focus ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Kill_Focus
//
////////////////////////////////////////////////////////////////
void
ComboBoxCtrlClass::On_Kill_Focus (DialogControlClass *focus)
{
	Set_Dirty ();

	DialogControlClass::On_Kill_Focus (focus);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Key_Down
//
////////////////////////////////////////////////////////////////
bool
ComboBoxCtrlClass::On_Key_Down (uint32 key_id, uint32 /* key_data */)
{
	bool handled = false;
	bool is_dirty = true;

	switch (key_id)
	{
		case VK_HOME:
			Set_Sel (0, true);
			break;

		case VK_END:
			Set_Sel (DropDownCtrl.Get_Count () - 1, true);
			break;

		case VK_UP:
		case VK_LEFT:
			Set_Sel (std::max<int>(0, (DropDownCtrl.Get_Curr_Sel () - 1)), true);
			break;

		case VK_DOWN:
		case VK_RIGHT:
			Set_Sel (DropDownCtrl.Get_Curr_Sel () + 1, true);
			break;

		default:
			is_dirty = false;
			handled = false;
			break;
	}

	if (is_dirty) {
		Set_Dirty ();
	}

	return handled;
}


////////////////////////////////////////////////////////////////
//
//	On_Create
//
////////////////////////////////////////////////////////////////
void
ComboBoxCtrlClass::On_Create (void)
{
	if ((Style & 0x0F) == CBS_DROPDOWN) {
		Parent->Add_Control(&EditControl);
		Set_Wants_Focus(false);
		EditControl.Set_Advise_Sink(this);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Destroy
//
////////////////////////////////////////////////////////////////
void
ComboBoxCtrlClass::On_Destroy (void)
{
	Parent->Remove_Control(&DropDownCtrl);
	Parent->Remove_Control(&EditControl);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Drop_Down_End
//
////////////////////////////////////////////////////////////////
void
ComboBoxCtrlClass::On_Drop_Down_End (int curr_sel)
{
	Set_Sel(curr_sel, true);

	//	Hide the drop down
	Display_Drop_Down (false);
	
	//
	//	If we aren't embedding an edit control, then we
	//	need to focus after the drop down exits
	//
	if ((Style & 0x0F) != CBS_DROPDOWN) {
		Set_Focus ();
	} else {
		EditControl.Set_Focus ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Display_Drop_Down
//
////////////////////////////////////////////////////////////////
void
ComboBoxCtrlClass::Display_Drop_Down (bool onoff)
{
	int curr_time = DialogMgrClass::Get_Time ();

	if (onoff) {

		//
		//	Show the drop down control
		//
		if (IsDropDownDisplayed == false && (curr_time - LastDropDownDisplayChange) > 0.1F) {
			Parent->Add_Control (&DropDownCtrl);
			DropDownCtrl.Set_Focus ();
			IsDropDownDisplayed = true;
			LastDropDownDisplayChange = curr_time;

			if ((Style & 0x0F) == CBS_DROPDOWN) {
				WideStringClass text(255, true);
				text = EditControl.Get_Text();

				int index = DropDownCtrl.Find_Closest_String(text);
				Set_Sel(index, true);
			}
		}

	} else {
		
		//
		//	Hide the drop down
		//
		if (IsDropDownDisplayed) {
			Parent->Remove_Control (&DropDownCtrl);
			IsDropDownDisplayed = false;
			LastDropDownDisplayChange = curr_time;
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
ComboBoxCtrlClass::On_Mouse_Wheel (int direction)
{
	if (direction < 0) {		
		Set_Sel (std::max<int>(0, (DropDownCtrl.Get_Curr_Sel () - 1)), true);
	} else {
		Set_Sel (DropDownCtrl.Get_Curr_Sel () + 1, true);
	}

	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Select_String
//
////////////////////////////////////////////////////////////////
int
ComboBoxCtrlClass::Select_String (const unichar_t* string)
{
	int index = Find_String (string);
	if (index >= 0) {
		Set_Curr_Sel (index);
	}

	return index;
}


void ComboBoxCtrlClass::Delete_String(int index)
{
	if (index >= 0 && index < DropDownCtrl.Get_Count()) {
		DropDownCtrl.Delete_String(index);

		CurrSel = -2;
		int newSel = DropDownCtrl.Get_Curr_Sel();
		Set_Sel(newSel, false);
	}
}


////////////////////////////////////////////////////////////////
//
//	Set_Curr_Sel
//
////////////////////////////////////////////////////////////////
void
ComboBoxCtrlClass::Set_Curr_Sel (int index)
{
	Set_Sel(index, false);
}


void
ComboBoxCtrlClass::Set_Sel (int index, bool notify)
{
	if (CurrSel != index) {
		DropDownCtrl.Set_Curr_Sel(index);
		int newSel = DropDownCtrl.Get_Curr_Sel();

		if (CurrSel != newSel) {
			Set_Dirty();
	
			// If this is a dropdown then automatically fill in the edit part with
			// the new selection string.
			if ((Style & 0x0F) == CBS_DROPDOWN) {
				if (newSel >= 0) {
					WideStringClass editString(255, true);
					DropDownCtrl.Get_String(newSel, editString);
					EditControl.Set_Text(editString);
				} else {
					EditControl.Set_Text(U_CHAR(""));
				}
			}

			int oldSel = CurrSel;
			CurrSel = newSel;

			if (notify) {
				ADVISE_NOTIFY(On_ComboBoxCtrl_Sel_Change(this, Get_ID(), oldSel, newSel));
			}
		}
	}
}


////////////////////////////////////////////////////////////////
//
//	Get_Text
//
////////////////////////////////////////////////////////////////
const unichar_t*
ComboBoxCtrlClass::Get_Text(void) const
{
	if ((Style & 0x0F) == CBS_DROPDOWN) {
		return EditControl.Get_Text();
	}

	return DropDownCtrl.Get_String(Get_Curr_Sel());
}


////////////////////////////////////////////////////////////////
//
//	Set_Text
//
////////////////////////////////////////////////////////////////
void
ComboBoxCtrlClass::Set_Text (const unichar_t *title)
{
	if ((Style & 0x0F) == CBS_DROPDOWN) {
		EditControl.Set_Text (title);
	}

	DialogControlClass::Set_Text (title);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_EditCtrl_Key_Down
//
////////////////////////////////////////////////////////////////
void
ComboBoxCtrlClass::On_EditCtrl_Change(EditCtrlClass* edit, int)
{
	if (edit == &EditControl) {
		ADVISE_NOTIFY(On_ComboBoxCtrl_Edit_Change(this, Get_ID()));
	}
}


////////////////////////////////////////////////////////////////
//
//	On_EditCtrl_Key_Down
//
////////////////////////////////////////////////////////////////
bool
ComboBoxCtrlClass::On_EditCtrl_Key_Down (EditCtrlClass *edit, uint32 key_id, uint32 /* key_data */)
{
	if (edit != &EditControl) {
		return false;
	}

	bool handled = true;

	switch (key_id)
	{
		case VK_UP:
			Set_Sel (std::max<int>(0, (DropDownCtrl.Get_Curr_Sel () - 1)), true);
			break;

		case VK_DOWN:
			Set_Sel (DropDownCtrl.Get_Curr_Sel () + 1, true);
			break;

		default:
			handled = false;
			break;
	}

	return handled;
}

