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
 *                     $Archive:: /Commando/Code/wwui/buttonctrl.cpp          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/02/02 11:16a                                              $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "buttonctrl.h"
#include "assetmgr.h"
#include "font3d.h"
#include "dialogbase.h"
#include "mousemgr.h"
#include "stylemgr.h"
#include "dialogmgr.h"
#include <algorithm>


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
static const RectClass BUTTON_UVS		(96.0F, 51.0F, 217.0F, 84.0F);
static const RectClass BUTTON_DN_UVS	(96.0F, 92.0F, 217.0F, 125.0F);
static const RectClass HILIGHT_UVS		(117.0F, 4.0F, 220.0F, 41.0F);

////////////////////////////////////////////////////////////////
//
//	ButtonCtrlClass
//
////////////////////////////////////////////////////////////////
ButtonCtrlClass::ButtonCtrlClass (void)	:
	WasButtonPressedOnMe (false),
	IsMouseOverMe (false),
	PulseTime (0)
{
	//
	//	Set the font for the text renderer
	//
	StyleMgrClass::Assign_Font (&TextRenderer, StyleMgrClass::FONT_CONTROLS);
	StyleMgrClass::Assign_Font (&GlowRenderer, StyleMgrClass::FONT_CONTROLS);

	//
	//	Configure the renderers
	//
	StyleMgrClass::Configure_Renderer (&ButtonRenderers[0]);
	StyleMgrClass::Configure_Renderer (&ButtonRenderers[1]);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
ButtonCtrlClass::Render (void)
{
	//
	//	Recreate the renderers (if necessary)
	//
	if (Is_Dirty ()) {
		Create_Text_Renderers ();

		//
		//	Determine which type of button to create
		//
		if ((Get_Style () & BS_BITMAP) == 0) {
			Create_Component_Button ();
		} else {
			Create_Bitmap_Button ();
		}

		//Create_Vector_Button ();
		//Create_Bitmap_Button ();
	}

	if (WasButtonPressedOnMe && IsMouseOverMe) {

		//
		//	Render the button "pressed"
		//
		ButtonRenderers[DOWN].Render ();
		ButtonRenderers[UP].Render ();
	} else {

		//
		//	Render the button normally
		//
		ButtonRenderers[UP].Render ();
	}

	//
	//	Render the text (only if its not a bitmap button)
	//
	if ((Get_Style () & BS_BITMAP) == 0) {
		if (HasFocus) {
			GlowRenderer.Render ();
		}
		TextRenderer.Render ();
	}

	DialogControlClass::Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Create
//
////////////////////////////////////////////////////////////////
void
ButtonCtrlClass::On_Create (void)
{
	GlowRenderer.Build_Sentence (Title);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Down
//
////////////////////////////////////////////////////////////////
void
ButtonCtrlClass::On_LButton_Down (const Vector2 &/* mouse_pos */)
{
	Set_Capture ();
	WasButtonPressedOnMe = true;

	//
	//	Play the sound effect
	//
	StyleMgrClass::Play_Sound (StyleMgrClass::EVENT_MOUSE_CLICK);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Up
//
////////////////////////////////////////////////////////////////
void
ButtonCtrlClass::On_LButton_Up (const Vector2 &mouse_pos)
{
	Release_Capture ();

	IsMouseOverMe = Rect.Contains (mouse_pos);

	//
	//	Notify the parent (if necessary)
	//
	if (WasButtonPressedOnMe && IsMouseOverMe) {
		Parent->On_Command (ID, BN_CLICKED, 0);
	}

	WasButtonPressedOnMe	= false;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Mouse_Move
//
////////////////////////////////////////////////////////////////
void
ButtonCtrlClass::On_Mouse_Move (const Vector2 &mouse_pos)
{
	IsMouseOverMe = Rect.Contains (mouse_pos);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Vector_Button
//
////////////////////////////////////////////////////////////////
void
ButtonCtrlClass::Create_Vector_Button (void)
{
	ButtonRenderers[0].Reset ();
	ButtonRenderers[1].Reset ();
	ButtonRenderers[0].Enable_Texturing (false);
	ButtonRenderers[1].Enable_Texturing (false);

	//
	//	Determine which color to draw the outline in
	//
	int color	= StyleMgrClass::Get_Line_Color ();
	int bkcolor	= StyleMgrClass::Get_Bk_Color ();
	if (IsEnabled == false) {
		color		= StyleMgrClass::Get_Disabled_Line_Color ();
		bkcolor	= StyleMgrClass::Get_Disabled_Bk_Color ();
	} else if (HasFocus) {
		//bkcolor =
	}

	//
	//	Draw the button outline
	//
	ButtonRenderers[0].Add_Outline (Rect, 1.0F, color);
	ButtonRenderers[1].Add_Outline (Rect, 1.0F, RGB_TO_INT32 (255, 0, 0));

	//
	//	Draw the button center
	//
	RectClass rect = Rect;
	rect.Right	-= 1;
	rect.Bottom -= 1;
	ButtonRenderers[0].Add_Quad (rect, bkcolor);
	ButtonRenderers[1].Add_Quad (rect, RGB_TO_INT32 (128, 0, 0));

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Bitmap_Button
//
////////////////////////////////////////////////////////////////
void
ButtonCtrlClass::Create_Bitmap_Button (void)
{
	//
	//	Darken the bitmap if its disabled
	//
	int color = 0xFFFFFFFF;
	if (IsEnabled == false) {
		color = RGB_TO_INT32 (96, 96, 96);
	}

	ButtonRenderers[0].Reset ();
	ButtonRenderers[1].Reset ();
	ButtonRenderers[0].Add_Quad (Rect, color);
	ButtonRenderers[1].Add_Quad (Rect, color);

	//
	//	Turn off texturing on the UI background
	//
	/*TextureClass *texture = WW3DAssetManager::Get_Instance ()->Get_Texture ("if_circle02.tga", TextureClass::MIP_LEVELS_1);
	ButtonRenderers[0].Set_Texture (texture);
	ButtonRenderers[1].Set_Texture (texture);
	texture->Release_Ref();

	RectClass uv_rect1 = BUTTON_UVS;
	uv_rect1.Inverse_Scale (Vector2 (256.0F, 256.0F));
	ButtonRenderers[0].Add_Quad (Rect, uv_rect1);

	RectClass uv_rect2 = BUTTON_DN_UVS;
	uv_rect2.Inverse_Scale (Vector2 (256.0F, 256.0F));
	ButtonRenderers[1].Add_Quad (Rect, uv_rect2);

	ShaderClass *shader = ButtonRenderers[0].Get_Shader ();
	shader->Set_Dst_Blend_Func (ShaderClass::DSTBLEND_ONE);
	shader->Set_Src_Blend_Func (ShaderClass::SRCBLEND_ONE);	

	shader = ButtonRenderers[1].Get_Shader ();
	shader->Set_Dst_Blend_Func (ShaderClass::DSTBLEND_ONE);
	shader->Set_Src_Blend_Func (ShaderClass::SRCBLEND_ONE);	*/

	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Blit_Section
//
////////////////////////////////////////////////////////////////
void
Blit_Section
(	
	Render2DClass &	  renderer,
	const Vector2 &	  screen_pos,
	const Vector2 &	  texture_pos,
	const Vector2 &	  pixels,
	const Vector2 &	  texture_dimensions
)
{
	RectClass screen_rect;
	screen_rect.Left		= screen_pos.X;
	screen_rect.Top		= screen_pos.Y;
	screen_rect.Right		= screen_rect.Left + pixels.X;
	screen_rect.Bottom	= screen_rect.Top + pixels.Y;

	RectClass uv_rect (texture_pos.X, texture_pos.Y, texture_pos.X + pixels.X, texture_pos.Y + pixels.Y);
	uv_rect.Inverse_Scale (Vector2 (texture_dimensions.X, texture_dimensions.Y));
	
	renderer.Add_Quad (screen_rect, uv_rect);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Component_Button
//
////////////////////////////////////////////////////////////////
void
ButtonCtrlClass::Create_Component_Button (void)
{
	ButtonRenderers[0].Reset ();
	ButtonRenderers[1].Reset ();

	//
	//	Turn off texturing on the UI background
	//
	TextureClass *texture = WW3DAssetManager::Get_Instance ()->Get_Texture ("if_circle02.tga", TextureClass::MIP_LEVELS_1);
	ButtonRenderers[0].Set_Texture (texture);
	ButtonRenderers[1].Set_Texture (texture);
	texture->Release_Ref();

	float height		= 10.0F;
	float edge_width	= 10.0F;

	float top			= BUTTON_UVS.Top;
	float left			= BUTTON_UVS.Left;
	float right			= BUTTON_UVS.Right;
	float	bottom		= BUTTON_UVS.Bottom;

//	float dn_start		= bottom - height;

	//float horz_tile_width	= 20.0F;
	//float vert_tile_height	= 10.0F;

	Vector2 texture_dimensions (256.0F, 256.0F);

	//
	//	Upper left
	//
	::Blit_Section (ButtonRenderers[0], Vector2 (Rect.Left, Rect.Top), Vector2 (left, top),
		Vector2 (edge_width, height), texture_dimensions);

	//
	//	Upper right
	//
	::Blit_Section (ButtonRenderers[0], Vector2 (Rect.Right - edge_width, Rect.Top), Vector2 (right - edge_width, top),
		Vector2 (edge_width, height), texture_dimensions);

	//
	//	Lower left
	//
	::Blit_Section (ButtonRenderers[0], Vector2 (Rect.Left, Rect.Bottom - height), Vector2 (left, bottom - height),
		Vector2 (edge_width, height), texture_dimensions);

	//
	//	Lower right
	//
	::Blit_Section (ButtonRenderers[0], Vector2 (Rect.Right - edge_width, Rect.Bottom - height), Vector2 (right - edge_width, bottom - height),
		Vector2 (edge_width, height), texture_dimensions);


	Vector2 horz_top_pos (left + ((right-left) / 2) - 5, top);	
	Vector2 horz_bottom_pos (left + ((right-left) / 2) - 5, bottom - height);
	Vector2 horz_size (10, height);

	//
	//	Horizontal tile
	//
	float remaining_width = Rect.Width () - (edge_width * 2);
	float x_pos = Rect.Left + edge_width;
	while (remaining_width > 0) {

		horz_size.X = std::min (remaining_width, horz_size.X);

		::Blit_Section (ButtonRenderers[0], Vector2 (x_pos, Rect.Top), horz_top_pos,
			horz_size, texture_dimensions);

		::Blit_Section (ButtonRenderers[0], Vector2 (x_pos, Rect.Bottom - height), horz_bottom_pos,
			horz_size, texture_dimensions);

		x_pos += horz_size.X;
		remaining_width -= horz_size.X;
	}


	Vector2 vert_left_pos (left, top + ((bottom - top)/ 2) - 5);
	Vector2 vert_right_pos (right - edge_width, top + ((bottom - top)/ 2) - 5);
	Vector2 vert_size (edge_width, 10);

	//
	//	Vertical tile
	//
	float remaining_height = Rect.Height () - (height * 2);
	float y_pos = Rect.Top + height;
	while (remaining_height > 0) {

		vert_size.Y = std::min (remaining_height, vert_size.Y);

		::Blit_Section (ButtonRenderers[0], Vector2 (Rect.Left, y_pos), vert_left_pos,
			vert_size, texture_dimensions);

		::Blit_Section (ButtonRenderers[0], Vector2 (Rect.Right - edge_width, y_pos), vert_right_pos,
			vert_size, texture_dimensions);
 		
		y_pos += vert_size.Y;
		remaining_height -= vert_size.Y;
	}

	RectClass hilight_rect = Rect;
	hilight_rect.Inflate (Vector2 (-((edge_width/2)+1), -((height/2)+1)));
	hilight_rect.Right	+= 1.0F;
	hilight_rect.Bottom	+= 1.0F;
	RectClass uv_rect = HILIGHT_UVS;
	uv_rect.Inverse_Scale (Vector2 (256.0F, 256.0F));
	ButtonRenderers[1].Add_Quad (hilight_rect, uv_rect);

	//
	//	"Patch" fill the interior
	//
	/*y_pos = Rect.Top + height;
	remaining_height = Rect.Height () - (height * 2);
	while (remaining_height > 0) {
		
		x_pos = Rect.Left + edge_width;
		remaining_width = Rect.Width () - (edge_width * 2);
		while (remaining_width > 0) {

			Vector2 size (8, 8);
			size.X = std::min (remaining_width, size.X);
			size.Y = std::min (remaining_height, size.Y);

			::Blit_Section (ButtonRenderers[0], Vector2 (x_pos, y_pos), Vector2 ((right / 2) - 4, (bottom / 2) - 4),
				size, texture_dimensions);

			::Blit_Section (ButtonRenderers[1], Vector2 (x_pos, y_pos), Vector2 ((right / 2) - 4, dn_start + (bottom / 2) - 4),
				size, texture_dimensions);

			remaining_width -= 8;
			x_pos += 8;
		}

		remaining_height -= 8;
		y_pos += 8;
	}*/

	ShaderClass *shader = ButtonRenderers[0].Get_Shader ();
	shader->Set_Dst_Blend_Func (ShaderClass::DSTBLEND_ONE);
	shader->Set_Src_Blend_Func (ShaderClass::SRCBLEND_ONE);	

	shader = ButtonRenderers[1].Get_Shader ();
	shader->Set_Dst_Blend_Func (ShaderClass::DSTBLEND_ONE);
	shader->Set_Src_Blend_Func (ShaderClass::SRCBLEND_ONE);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Component_Button2
//
////////////////////////////////////////////////////////////////
void
ButtonCtrlClass::Create_Component_Button2 (void)
{
	ButtonRenderers[0].Reset ();
	ButtonRenderers[1].Reset ();
	ButtonRenderers[0].Set_Coordinate_Range (Render2DClass::Get_Screen_Resolution());
	ButtonRenderers[1].Set_Coordinate_Range (Render2DClass::Get_Screen_Resolution());

	//
	//	Turn off texturing on the UI background
	//
	ButtonRenderers[0].Enable_Texturing (false);	
	ButtonRenderers[1].Enable_Texturing (false);	
	const int BLACK	= VRGB_TO_INT32 (Vector3 (0, 0, 0));
	const int WHITE	= VRGB_TO_INT32 (Vector3 (1, 1, 1));
	const int DK_GRAY	= VRGB_TO_INT32 (Vector3 (0.5F, 0.5F, 0.5F));
	const int GRAY		= VRGB_TO_INT32 (Vector3 (0.75F, 0.75F, 0.75F));
	const int LT_GRAY	= VRGB_TO_INT32 (Vector3 (0.9F, 0.9F, 0.9F));

	//
	//	Draw the outside button outline
	//
	RectClass rect = Rect;
	ButtonRenderers[0].Add_Line (Vector2 (rect.Left, rect.Bottom),		Vector2 (rect.Left, rect.Top-1),		1, WHITE);
	ButtonRenderers[0].Add_Line (Vector2 (rect.Left, rect.Top),			Vector2 (rect.Right, rect.Top),		1, WHITE);
	ButtonRenderers[0].Add_Line (Vector2 (rect.Right, rect.Top),		Vector2 (rect.Right, rect.Bottom),	1, BLACK);
	ButtonRenderers[0].Add_Line (Vector2 (rect.Right, rect.Bottom),	Vector2 (rect.Left, rect.Bottom),	1, BLACK);

	ButtonRenderers[1].Add_Line (Vector2 (rect.Left, rect.Bottom),		Vector2 (rect.Left, rect.Top-1),		1, BLACK);
	ButtonRenderers[1].Add_Line (Vector2 (rect.Left, rect.Top),			Vector2 (rect.Right, rect.Top),		1, BLACK);
	ButtonRenderers[1].Add_Line (Vector2 (rect.Right, rect.Top),		Vector2 (rect.Right, rect.Bottom),	1, WHITE);
	ButtonRenderers[1].Add_Line (Vector2 (rect.Right, rect.Bottom),	Vector2 (rect.Left, rect.Bottom),	1, WHITE);

	
	//
	//	Draw the inside button outline
	//
	rect.Inflate (Vector2 (-1, -1));
	ButtonRenderers[0].Add_Line (Vector2 (rect.Left, rect.Bottom),		Vector2 (rect.Left, rect.Top-1),		1, LT_GRAY);
	ButtonRenderers[0].Add_Line (Vector2 (rect.Left, rect.Top),			Vector2 (rect.Right, rect.Top),		1, LT_GRAY);
	ButtonRenderers[0].Add_Line (Vector2 (rect.Right, rect.Top),		Vector2 (rect.Right, rect.Bottom),	1, DK_GRAY);
	ButtonRenderers[0].Add_Line (Vector2 (rect.Right, rect.Bottom),	Vector2 (rect.Left, rect.Bottom),	1, DK_GRAY);

	ButtonRenderers[1].Add_Line (Vector2 (rect.Left, rect.Bottom),		Vector2 (rect.Left, rect.Top-1),		1, DK_GRAY);
	ButtonRenderers[1].Add_Line (Vector2 (rect.Left, rect.Top),			Vector2 (rect.Right, rect.Top),		1, DK_GRAY);

	//
	//	Fill the button center
	//
	rect.Right	-= 1;
	rect.Bottom -= 1;
	ButtonRenderers[0].Add_Quad (rect, GRAY);
	ButtonRenderers[1].Add_Quad (rect, GRAY);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Text_Renderers
//
////////////////////////////////////////////////////////////////
void
ButtonCtrlClass::Create_Text_Renderers (void)
{
	TextRenderer.Reset ();
	GlowRenderer.Reset_Polys ();

	//
	//	Draw the text
	//
	StyleMgrClass::Render_Text (Title, &TextRenderer, Rect, true, true, StyleMgrClass::CENTER_JUSTIFY, IsEnabled);

	//
	//	Draw the glow effect
	//
	StyleMgrClass::Render_Glow (Title, &GlowRenderer, Rect, 8, 8,
								StyleMgrClass::Get_Tab_Glow_Color (), StyleMgrClass::CENTER_JUSTIFY);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Cursor
//
////////////////////////////////////////////////////////////////
void
ButtonCtrlClass::On_Set_Cursor (const Vector2 &mouse_pos)
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
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
ButtonCtrlClass::On_Frame_Update (void)
{
	DialogControlClass::On_Frame_Update ();

	//
	//	Get the current mouse position
	//
	Vector3 complete_mouse_pos = DialogMgrClass::Get_Mouse_Pos ();
	Vector2 mouse_pos;
	mouse_pos.X = complete_mouse_pos.X;
	mouse_pos.Y = complete_mouse_pos.Y;

	//
	//	Update the pulse (as necessary)
	//
	if (ClientRect.Contains (mouse_pos)) {
		Update_Pulse (true);
	} else {
		Update_Pulse (false);
	}
	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Kill_Focus
//
////////////////////////////////////////////////////////////////
void
ButtonCtrlClass::On_Kill_Focus (DialogControlClass *focus)
{
	WasButtonPressedOnMe	= false;
	IsMouseOverMe			= false;

	DialogControlClass::On_Kill_Focus (focus);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Key_Down
//
////////////////////////////////////////////////////////////////
bool
ButtonCtrlClass::On_Key_Down (uint32 key_id, uint32 /* key_data */)
{
	switch (key_id)
	{
		case VK_RETURN:
		case VK_SPACE:
			Parent->On_Command (ID, BN_CLICKED, 0);

			//
			//	Play the sound effect
			//
			StyleMgrClass::Play_Sound (StyleMgrClass::EVENT_MOUSE_CLICK);
			return true;
			break;
	}

	return false;
}


////////////////////////////////////////////////////////////////
//
//	Set_Bitmap
//
////////////////////////////////////////////////////////////////
void
ButtonCtrlClass::Set_Bitmap (const char *texture_up_name, const char *texture_dn_name)
{
	//
	//	Configure the up bitmap
	//
	TextureClass *texture_up = WW3DAssetManager::Get_Instance ()->Get_Texture (texture_up_name, TextureClass::MIP_LEVELS_1);
	ButtonRenderers[0].Set_Texture (texture_up);

	//
	//	Configure the down bitmap
	//
	if (texture_dn_name != NULL && texture_dn_name[0] != 0) {
		TextureClass *texture_dn = WW3DAssetManager::Get_Instance ()->Get_Texture (texture_dn_name, TextureClass::MIP_LEVELS_1);
		ButtonRenderers[1].Set_Texture (texture_dn);
		REF_PTR_RELEASE (texture_dn);
	} else {
		ButtonRenderers[1].Set_Texture (texture_up);
	}

	REF_PTR_RELEASE (texture_up);

	//
	//	Force the bitmap style on...
	//
	Style |= BS_BITMAP;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Pulse
//
////////////////////////////////////////////////////////////////
void
ButtonCtrlClass::Update_Pulse (bool is_mouse_over)
{
	const int PULSE_LEN	= 1000;
	int		 color;

	//
	//	Short circuit if there's nothing to do
	//
	if (PulseTime == PULSE_LEN && is_mouse_over == false) {
		return ;
	}

	if (IsEnabled) {

		if (is_mouse_over) {		

			//
			//	Examine the current time to determine what
			// percent of the "pulse" we are at
			//
			PulseTime -= DialogMgrClass::Get_Frame_Time ();
			if (PulseTime < 0) {
				PulseTime += PULSE_LEN;
			}

			float percent	= (float)PulseTime / (float)PULSE_LEN;

			//
			//	Map the percent onto a circle to give us a pulse
			//
			float value	= ::cos (DEG_TO_RADF (360.0F) * percent);
			value			= 0.8F + (value * 0.2F);

			//
			//	Convert this value into a grayscale color
			//
			color = VRGB_TO_INT32 (Vector3 (value, value, value));
		} else {
		
			color = RGB_TO_INT32 (0xff, 0xff, 0xff);
			PulseTime = PULSE_LEN;
		}

	} else {

		color = RGB_TO_INT32 (0x60, 0x60, 0x60);
	}
	
	//
	//	Update the color vector arrays
	//
	int index;
	DynamicVectorClass<unsigned int> &color_array = ButtonRenderers[0].Get_Color_Array ();
	for (index = 0; index < color_array.Count (); index ++) {
		color_array[index] = color;
	}

	DynamicVectorClass<unsigned int> &color_array2 = ButtonRenderers[1].Get_Color_Array ();
	for (index = 0; index < color_array2.Count (); index ++) {
		color_array2[index] = color;
	}

	return ;
}
