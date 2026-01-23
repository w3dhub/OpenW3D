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
 *                     $Archive:: /Commando/Code/wwui/shortcutbarctrl.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/13/01 9:58a                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "shortcutbarctrl.h"
#include "assetmgr.h"
#include "refcount.h"
#include "mousemgr.h"
#include "ww3d.h"
#include "dialogmgr.h"
#include "dialogbase.h"
#include "stylemgr.h"
#include "vector4.h"
#include <algorithm>


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
static const float BORDER_X		= 10.0F;
static const float BORDER_Y		= BORDER_X * 0.75F;
static const float TEXT_BORDER_Y	= 6.0F;
static const float FULL_WIDTH		= 200.0F;

static const RectClass	BAR_TOP_UVS			(1, 3, 13, 44);
static const RectClass	BAR_BOTTOM_UVS		(14, 4, 26, 44);
static const RectClass	BAR_TILE_UVS		(27, 10, 39, 37);


////////////////////////////////////////////////////////////////
//
	//	ShortcutBarCtrlClass
//
////////////////////////////////////////////////////////////////
ShortcutBarCtrlClass::ShortcutBarCtrlClass (void)	:
	FullRect (0, 0, 0, 0),
	IsHiddenState (true),
	CurrSel (-1),
	BarOpacity (1.0F),
	PulseDirection (-1.0F)
{
	//
	//	Set the font for the text renderers
	//
	StyleMgrClass::Assign_Font (&TextRenderer, StyleMgrClass::FONT_CONTROLS);
	StyleMgrClass::Assign_Font (&GlowRenderer, StyleMgrClass::FONT_HEADER);
	StyleMgrClass::Assign_Font (&HilightRenderer, StyleMgrClass::FONT_HEADER);
	StyleMgrClass::Configure_Renderer (&ControlRenderer);
	StyleMgrClass::Configure_Renderer (&TexturedControlRenderer);

	//
	//	Configure the textured renderer
	//
	TextureClass *texture = WW3DAssetManager::Get_Instance ()->Get_Texture ("if_bar.tga", TextureClass::MIP_LEVELS_1);
	TexturedControlRenderer.Set_Texture (texture);
	REF_PTR_RELEASE (texture);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~ShortcutBarCtrlClass
//
////////////////////////////////////////////////////////////////
ShortcutBarCtrlClass::~ShortcutBarCtrlClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Text_Renderers
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::Create_Text_Renderers (void)
{
	//
	//	Start fresh
	//
	TextRenderer.Reset ();
	GlowRenderer.Reset ();
	HilightRenderer.Reset ();

	//
	//	Don't do anything if the control is "hidden"
	//
	if (IsHiddenState) {
		return ;
	}

	//
	//	Draw each button
	//
	for (int index = 0; index < ShortcutList.Count (); index ++) {

		//
		//	Get the rectangle for this button
		//
		RectClass text_rect;
		Get_Entry_Rect (index, text_rect);

		//
		//	Get a pointer to the text for this entry
		//
		const unichar_t *text = ShortcutList[index].text;

		//
		//	Is this the hilighted button?
		//
		if (index == CurrSel) {

			//
			//	Render the text for this button
			//
			StyleMgrClass::Render_Text (text, &HilightRenderer, StyleMgrClass::Get_Tab_Text_Color (),
									StyleMgrClass::Get_Text_Shadow_Color (), text_rect, true, true,
									StyleMgrClass::CENTER_JUSTIFY);

			//
			//	Render its "glow"
			//
			GlowRenderer.Build_Sentence (text);
			StyleMgrClass::Render_Glow (text, &GlowRenderer, text_rect, 7, 7,
									StyleMgrClass::Get_Tab_Glow_Color (), StyleMgrClass::CENTER_JUSTIFY);

		} else {

			//
			//	Render the text for this button
			//
			StyleMgrClass::Render_Text (text, &TextRenderer, text_rect, true, true,
								StyleMgrClass::CENTER_JUSTIFY);

		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Control_Renderers
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::Create_Control_Renderers (void)
{
	//
	//	Configure this renderer
	//
	ControlRenderer.Reset ();
	TexturedControlRenderer.Reset ();
	ControlRenderer.Enable_Texturing (false);

	if (IsHiddenState) {
		Create_Hidden_State_Renderers ();
	} else {
		Create_Displayed_State_Renderers ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Hidden_State_Renderers
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::Create_Hidden_State_Renderers (void)
{
	//
	//	Determine which color to draw the outline in
	//
	//int color	= StyleMgrClass::Get_Line_Color ();
	//int bkcolor	= StyleMgrClass::Get_Bk_Color ();

	//
	//	Draw the outline
	//
	//ControlRenderer.Add_Rect (Rect, 1.0F, color, bkcolor);

	Render_Strip (Rect, false);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render_Strip
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::Render_Strip (const RectClass &screen_rect, bool flip_uvs)
{
	RectClass bar_top_uvs		= BAR_TOP_UVS;
	RectClass bar_tile_uvs		= BAR_TILE_UVS;
	RectClass bar_bottom_uvs	= BAR_BOTTOM_UVS;

	bar_top_uvs.Inverse_Scale (Vector2 (64, 64));
	bar_tile_uvs.Inverse_Scale (Vector2 (64, 64));
	bar_bottom_uvs.Inverse_Scale (Vector2 (64, 64));

	//
	//	Flip the UVs if necessary
	//
	if (flip_uvs) {		
		float temp = bar_top_uvs.Right;
		bar_top_uvs.Right = bar_top_uvs.Left;
		bar_top_uvs.Left	= temp;

		temp = bar_tile_uvs.Right;
		bar_tile_uvs.Right = bar_tile_uvs.Left;
		bar_tile_uvs.Left	= temp;

		temp = bar_bottom_uvs.Right;
		bar_bottom_uvs.Right = bar_bottom_uvs.Left;
		bar_bottom_uvs.Left	= temp;
	}

	//
	//	Render the top of the bar
	//
	RectClass rect	= screen_rect;
	rect.Bottom		= int(rect.Top + BAR_TOP_UVS.Height ());
	TexturedControlRenderer.Add_Quad (rect, bar_top_uvs);

	//
	//	Render the center (tileable) section
	//
	float height = (screen_rect.Bottom - BAR_BOTTOM_UVS.Height ()) - rect.Bottom;
	while (height > 0) {

		float tile_height = BAR_TILE_UVS.Height ();
		tile_height = std::min (tile_height, height);

		//
		//	Render this section of the bar
		//
		rect.Top		= rect.Bottom;
		rect.Bottom = int (rect.Top + tile_height);
		TexturedControlRenderer.Add_Quad (rect, bar_tile_uvs);

		//
		//	Decrement the total height left
		//
		height -= tile_height;
	}

	//
	//	Render the bottom of the bar
	//
	rect.Top		= rect.Bottom;
	rect.Bottom = screen_rect.Bottom;
	TexturedControlRenderer.Add_Quad (rect, bar_bottom_uvs);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Displayed_State_Renderers
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::Create_Displayed_State_Renderers (void)
{
	//
	//	Determine which color to draw the outline in
	//
	int color	= StyleMgrClass::Get_Line_Color ();
	int bkcolor	= StyleMgrClass::Get_Bk_Color ();

	//
	//	Darken the background...
	//
	ControlRenderer.Add_Quad (FullRect, RGBA_TO_INT32 (0, 0, 0, 170));

	//
	//	Draw the outline
	//
	ControlRenderer.Add_Rect (FullRect, 1.0F, color, bkcolor);

	//
	//	Render the left strip
	//
	RectClass rect = Rect;
	rect.Left	-= 1.0F;
	rect.Right	-= 1.0F;
	rect.Top		-= (6.0F * StyleMgrClass::Get_Y_Scale ());
	rect.Bottom += (6.0F * StyleMgrClass::Get_Y_Scale ());
	Render_Strip (rect, true);

	//
	//	Render the right strip
	//
	rect.Right	= FullRect.Right;
	rect.Left	= FullRect.Right - Rect.Width ();	
	Render_Strip (rect, false);

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Cursor
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::On_Set_Cursor (const Vector2 &/* mouse_pos */)
{
	//
	//	Change the mouse cursor
	//
	MouseMgrClass::Set_Cursor (MouseMgrClass::CURSOR_ACTION);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Client_Rect
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::Update_Client_Rect (void)
{
	//
	//	Set the client area
	//
	ClientRect = Rect;

	//
	//	Determine what scale to use
	//
	float scale_x = Render2DClass::Get_Screen_Resolution().Width () / 800.0F;

	//
	//	Calculate the display rectangle
	//
	FullRect			= Rect;
	FullRect.Right	= FullRect.Left + int(FULL_WIDTH * scale_x);

	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::Render (void)
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
	TexturedControlRenderer.Render ();
	GlowRenderer.Render ();	
	TextRenderer.Render ();
	HilightRenderer.Render ();

	DialogControlClass::Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Down
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::On_LButton_Down (const Vector2 &mouse_pos)
{
	int new_sel = Entry_From_Pos (mouse_pos);
	if (new_sel != -1) {
		Set_Curr_Sel (new_sel);
		Send_Command ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Up
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::On_LButton_Up (const Vector2 &/* mouse_pos */)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Mouse_Move
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::On_Mouse_Move (const Vector2 &mouse_pos)
{
	//
	//	Check to see if the user moved the mouse off of the control
	//
	if (IsHiddenState == false && FullRect.Contains (mouse_pos) == false) {
		Set_Hidden_State (true);
	} else if (IsHiddenState && FullRect.Contains (mouse_pos)) {
		Set_Hidden_State (false);
	} else if (IsHiddenState == false) {

		//
		//	Ensure we have the input focus captured
		//
		if (DialogMgrClass::Get_Focus () != this) {
			Set_Focus ();
			Set_Capture ();			
		}
		
		//
		//	Change the selection
		//
		int new_sel = Entry_From_Pos (mouse_pos);
		Set_Curr_Sel (new_sel);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Focus
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::On_Set_Focus (void)
{
	DialogControlClass::On_Set_Focus ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Kill_Focus
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::On_Kill_Focus (DialogControlClass *focus)
{	
	Set_Hidden_State (true);

	DialogControlClass::On_Kill_Focus (focus);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Key_Down
//
////////////////////////////////////////////////////////////////
bool
ShortcutBarCtrlClass::On_Key_Down (uint32 key_id, uint32 /* key_data */)
{
	bool handled = true;

	switch (key_id)
	{
		case VK_SPACE:
		case VK_RETURN:
			Send_Command ();			
			break;

		case VK_UP:
		case VK_LEFT:
			Set_Curr_Sel (CurrSel - 1, true);
			break;

		case VK_DOWN:
		case VK_RIGHT:
			Set_Curr_Sel (CurrSel + 1, true);
			break;

		default:
			handled = false;
			break;
	}

	return handled;
}


////////////////////////////////////////////////////////////////
//
//	On_Create
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::On_Create (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Hidden_State
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::Set_Hidden_State (bool onoff)
{
	//
	//	Set the dirty flag if we've changed states
	//
	if (IsHiddenState != onoff) {
		IsHiddenState = onoff;
		Set_Dirty ();

		//
		//	Capture the mouse so we can tell when the user
		// moves the mouse off of the control
		//
		if (IsHiddenState == false) {
			
			//
			//	Snap the mouse to the first entry
			//
			//RectClass text_rect;
			//Get_Entry_Rect (0, text_rect);
			//Vector3 center (FullRect.Center ().X, DialogMgrClass::Get_Mouse_Pos ().Y, 0);
			//DialogMgrClass::Set_Mouse_Pos (center);

			//
			//	Capture the input focus
			//
			Set_Focus ();
			Set_Capture ();
		} else {
			Release_Capture ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Add_Button
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::Add_Button (int ctrl_id, const unichar_t *text)
{
	ShortcutList.Add (ShortcutInfoStruct (ctrl_id, text));
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Remove_Button
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::Remove_Button (int ctrl_id)
{
	for (int index = 0; index < ShortcutList.Count (); index ++) {
		
		//
		//	Remove the entry from the list...
		//
		if (ShortcutList[index].ctrl_id == ctrl_id) {
			ShortcutList.Delete (index);
			Set_Dirty ();
			break;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Entry_Rect
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::Get_Entry_Rect (int index, RectClass &rect)
{
	float start_y_pos		= Rect.Top + BORDER_Y * StyleMgrClass::Get_Y_Scale ();
	float entry_height	= TextRenderer.Get_Text_Extents (U_CHAR("W")).Y;
	entry_height			+= TEXT_BORDER_Y * StyleMgrClass::Get_Y_Scale () * 2.0F;

	//
	//	Simply calculate the rectangle for this entry and return it
	// to the caller
	//
	rect.Left	= FullRect.Left + int(BORDER_X * StyleMgrClass::Get_X_Scale ());
	rect.Right	= FullRect.Right - int(BORDER_X * StyleMgrClass::Get_X_Scale ());
	rect.Top		= int(start_y_pos + (index * entry_height));
	rect.Bottom	= int(rect.Top + entry_height);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Entry_From_Pos
//
////////////////////////////////////////////////////////////////
int
ShortcutBarCtrlClass::Entry_From_Pos (const Vector2 &mouse_pos)
{
	int retval = -1;

	for (int index = 0; index < ShortcutList.Count (); index ++) {
		
		//
		//	Get the rectangle for this button
		//
		RectClass text_rect;
		Get_Entry_Rect (index, text_rect);
	
		//
		//	Is this the entry we're looking for?
		//
		if (text_rect.Contains (mouse_pos)) {
			retval = index;
			break;
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Set_Curr_Sel
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::Set_Curr_Sel (int index, bool snap_mouse)
{
	//
	//	Ensure the new index is in range
	//
	if (index < 0) {
		index = ShortcutList.Count () - 1;
	} else if (index >= ShortcutList.Count ()) {
		index = 0;
	}

	//
	//	Did the selection change?
	//
	if (index != CurrSel) {
		CurrSel = index;
		Set_Dirty ();

		if (snap_mouse) {

			//
			//	Snap the mouse to the entry
			//
			RectClass text_rect;
			Get_Entry_Rect (CurrSel, text_rect);
			Vector3 center (text_rect.Center ().X, text_rect.Center ().Y, 0);
			DialogMgrClass::Set_Mouse_Pos (center);			
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
ShortcutBarCtrlClass::On_Mouse_Wheel (int direction)
{
	if (IsHiddenState == false) {
		
		if (direction > 0) {
			Set_Curr_Sel (CurrSel + 1, true);
		} else {
			Set_Curr_Sel (CurrSel - 1, true);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Window_Rect
//
////////////////////////////////////////////////////////////////
const RectClass &
ShortcutBarCtrlClass::Get_Window_Rect (void) const
{
	const RectClass *retval = &Rect;
	
	//
	//	If the bar is completely displayed, then return
	// the full rectangle instead of the screen rectangle.
	//
	if (IsHiddenState == false) {
		retval = &FullRect;
	}

	return (*retval);
}


////////////////////////////////////////////////////////////////
//
//	Send_Command
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::Send_Command (void)
{
	if (CurrSel >= 0 && CurrSel < ShortcutList.Count ()) {
		Parent->On_Command (ShortcutList[CurrSel].ctrl_id, BN_CLICKED, 0);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Pulse
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::Update_Pulse (void)
{
	if (IsHiddenState == false) {
		return ;
	}

	const float PULSE_RATE	= 0.5F;
	const float MIN_OPACITY	= 0.65F;
	const float MAX_OPACITY	= 1.00F;

	//
	//	Pulse the bar
	//
	float delta = PULSE_RATE * (DialogMgrClass::Get_Frame_Time () / 1000.0F);
	BarOpacity += PulseDirection * delta;

	//
	//	Clamp the opacity
	//
	if (BarOpacity <= MIN_OPACITY) {
		BarOpacity			= MIN_OPACITY;
		PulseDirection		= 1.0F;
	} else if (BarOpacity >= MAX_OPACITY) {
		BarOpacity			= MAX_OPACITY;
		PulseDirection		= -1.0F;			
	}

	int bar_color = VRGBA_TO_INT32 (Vector4 (1.0F, 1.0F, 1.0F, BarOpacity));

	//
	//	Update the color vector array
	//
	DynamicVectorClass<unsigned int> &color_array = TexturedControlRenderer.Get_Color_Array ();
	for (int index = 0; index < color_array.Count (); index ++) {
		color_array[index] = bar_color;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
ShortcutBarCtrlClass::On_Frame_Update (void)
{
	Update_Pulse ();
	DialogControlClass::On_Frame_Update ();
	return ;
}
