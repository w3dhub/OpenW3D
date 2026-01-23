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
 *                     $Archive:: /Commando/Code/wwui/checkboxctrl.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/05/01 4:22p                                              $*
 *                                                                                             *
 *                    $Revision:: 18                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "checkboxctrl.h"
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
//	CheckBoxCtrlClass
//
////////////////////////////////////////////////////////////////
CheckBoxCtrlClass::CheckBoxCtrlClass (void)	:
	TextRect (0, 0, 0, 0),
	ButtonRect (0, 0, 0, 0),
	MaxRect (0, 0, 0, 0),
	IsChecked (false)
{
	//
	//	Set the font for the text renderers
	//
	StyleMgrClass::Assign_Font (&TextRenderer, StyleMgrClass::FONT_CONTROLS);
	StyleMgrClass::Configure_Renderer (&ControlRenderer);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~CheckBoxCtrlClass
//
////////////////////////////////////////////////////////////////
CheckBoxCtrlClass::~CheckBoxCtrlClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Text_Renderers
//
////////////////////////////////////////////////////////////////
void
CheckBoxCtrlClass::Create_Text_Renderers (void)
{
	const float	TEXT_SPACING	= 8.0F;

	//
	//	Configure the renderers
	//
	HilightRenderer.Reset ();
	HilightRenderer.Set_Coordinate_Range (Render2DClass::Get_Screen_Resolution());		
	StyleMgrClass::Configure_Hilighter (&HilightRenderer);

	//
	//	Determine how wide to make the text area
	//
	Vector2 extents	= TextRenderer.Get_Text_Extents (Title);
	float width			= extents.X + (StyleMgrClass::Get_X_Scale () * TEXT_SPACING);

	//
	//	Resize the text area to fit the text
	//
	TextRect.Right = TextRect.Left + width;
	TextRect.Right = std::min (TextRect.Right, MaxRect.Right);
	Rect.Right		= TextRect.Right;

	//
	//	Start fresh
	//
	TextRenderer.Reset ();

	//
	//	Draw the text
	//
	StyleMgrClass::Render_Text (Title, &TextRenderer, TextRect, true, true,
							StyleMgrClass::LEFT_JUSTIFY, IsEnabled);

	//
	//	Do the hilight
	//
	if (HasFocus) {
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
CheckBoxCtrlClass::Create_Control_Renderers (void)
{
	Render2DClass &renderer = ControlRenderer;

	//
	//	Configure this renderer
	//
	renderer.Reset ();
	renderer.Enable_Texturing (false);

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
	//	Draw the button outline
	//
	RectClass rect = ButtonRect;
	renderer.Add_Outline (ButtonRect, 1.0F, color);

	//
	//	Now draw the background
	//
	rect.Right	-= 1;
	rect.Bottom -= 1;
	renderer.Add_Quad (rect, bkcolor);

	//
	//	Draw the check (if necessary)
	//
	if (IsChecked) {

		int text_color		= StyleMgrClass::Get_Text_Color ();
		int shadow_color	= StyleMgrClass::Get_Text_Shadow_Color ();
		if (IsEnabled == false) {
			text_color		= StyleMgrClass::Get_Disabled_Text_Color ();
			shadow_color	= StyleMgrClass::Get_Disabled_Text_Shadow_Color ();
		}

		rect = ButtonRect;
		rect.Inflate (Vector2 (-5, -5));

		renderer.Add_Line (Vector2 (rect.Left-1, rect.Top+1),		Vector2 (rect.Right-2, rect.Bottom+1),	2, shadow_color);
		renderer.Add_Line (Vector2 (rect.Left-1, rect.Bottom+1),	Vector2 (rect.Right-2, rect.Top+1),		2, shadow_color);

		renderer.Add_Line (Vector2 (rect.Left, rect.Top),		Vector2 (rect.Right-1, rect.Bottom),	2, text_color);
		renderer.Add_Line (Vector2 (rect.Left, rect.Bottom),	Vector2 (rect.Right-1, rect.Top),		2, text_color);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Cursor
//
////////////////////////////////////////////////////////////////
void
CheckBoxCtrlClass::On_Set_Cursor (const Vector2 &mouse_pos)
{
	//
	//	Change the mouse cursor
	//
	if (Rect.Contains (mouse_pos) && mouse_pos.X <= TextRect.Right) {
		MouseMgrClass::Set_Cursor (MouseMgrClass::CURSOR_ACTION);
	} else {
		MouseMgrClass::Set_Cursor (MouseMgrClass::CURSOR_ARROW);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Client_Rect
//
////////////////////////////////////////////////////////////////
void
CheckBoxCtrlClass::Update_Client_Rect (void)
{
	//
	//	Determine what one character spacing would be
	//
	Vector2 char_size = TextRenderer.Get_Text_Extents (U_CHAR("W"));

	//
	//	Set the client area
	//
	MaxRect		= Rect;
	ClientRect	= Rect;

	//
	//	Determine what size the button should be
	//
	float button_width	= int(char_size.X * 1.5F);
	float button_height	= button_width;

	//
	//	Calculate the button rectangle
	//
	ButtonRect.Left	= ClientRect.Left;
	ButtonRect.Top		= ClientRect.Top + int((ClientRect.Height () / 2.0F) - (button_height / 2.0F));
	ButtonRect.Right	= ButtonRect.Left + button_width;
	ButtonRect.Bottom	= ClientRect.Top + int((ClientRect.Height () / 2.0F) + (button_height / 2.0F));

	//
	//	Calculate the text rectangle
	//
	TextRect.Left		= int(ButtonRect.Right + (char_size.X * 0.5F));
	TextRect.Top		= ClientRect.Top;
	TextRect.Right		= ClientRect.Right;
	TextRect.Bottom	= ClientRect.Bottom;

	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
CheckBoxCtrlClass::Render (void)
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

	DialogControlClass::Render ();
	return ;
}

////////////////////////////////////////////////////////////////
//
//	Set_Check
//
////////////////////////////////////////////////////////////////
void CheckBoxCtrlClass::Set_Check (bool onoff)
{ 
	Internal_Set_Check(onoff, false);
}


void CheckBoxCtrlClass::Internal_Set_Check(bool onoff, bool notify)
{
	IsChecked = onoff;
	Set_Dirty ();

	if (notify) {
		Parent->On_Command (Get_ID (), 0, onoff);
	}
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Down
//
////////////////////////////////////////////////////////////////
void
CheckBoxCtrlClass::On_LButton_Down (const Vector2 &/* mouse_pos */)
{
	Internal_Set_Check (!IsChecked, true);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Up
//
////////////////////////////////////////////////////////////////
void
CheckBoxCtrlClass::On_LButton_Up (const Vector2 &/* mouse_pos */)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Focus
//
////////////////////////////////////////////////////////////////
void
CheckBoxCtrlClass::On_Set_Focus (void)
{
	Set_Dirty ();

	DialogControlClass::On_Set_Focus ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Kill_Focus
//
////////////////////////////////////////////////////////////////
void
CheckBoxCtrlClass::On_Kill_Focus (DialogControlClass *focus)
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
CheckBoxCtrlClass::On_Key_Down (uint32 key_id, uint32 /* key_data */)
{
	bool handled = false;
	bool is_dirty = true;

	switch (key_id)
	{
		case VK_SPACE:
			Internal_Set_Check (!IsChecked, true);
			handled = true;
			break;

		default:
			is_dirty = false;
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
CheckBoxCtrlClass::On_Create (void)
{
	return ;
}


