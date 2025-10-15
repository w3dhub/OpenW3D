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
 *                     $Archive:: /Commando/Code/wwui/dialogbutton.cpp          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 6/11/01 3:58p                                               $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dialogbutton.h"
#include "assetmgr.h"
#include "font3d.h"
#include "dialogbase.h"
#include "mousemgr.h"
#include "stylemgr.h"
#include <algorithm>


////////////////////////////////////////////////////////////////
//
//	DialogButtonClass
//
////////////////////////////////////////////////////////////////
DialogButtonClass::DialogButtonClass (void)	:
	WasButtonPressedOnMe (false),
	IsMouseOverMe (false)
{
	//
	//	Set the font for the text renderer
	//
	StyleMgrClass::Assign_Font (&TextRenderers[0], StyleMgrClass::FONT_CONTROLS);
	StyleMgrClass::Assign_Font (&TextRenderers[1], StyleMgrClass::FONT_CONTROLS);

	//
	//	Configure the renderers
	//
	//StyleMgrClass::Configure_Renderer (&TextRenderers[0]);
	//StyleMgrClass::Configure_Renderer (&TextRenderers[1]);
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
DialogButtonClass::Render (void)
{
	//
	//	Recreate the renderers (if necessary)
	//
	if (Is_Dirty ()) {
		Create_Text_Renderers ();
		Create_Component_Button ();
	}

	if (WasButtonPressedOnMe && IsMouseOverMe) {
		
		//
		//	Render the button "pressed"
		//
		ButtonRenderers[DOWN].Render ();
		TextRenderers[DOWN].Render ();
	} else {

		//
		//	Render the button normally
		//
		ButtonRenderers[UP].Render ();
		TextRenderers[UP].Render ();
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
DialogButtonClass::On_Create (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Down
//
////////////////////////////////////////////////////////////////
void
DialogButtonClass::On_LButton_Down (const Vector2 &mouse_pos)
{
	Set_Capture ();
	WasButtonPressedOnMe = true;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Up
//
////////////////////////////////////////////////////////////////
void
DialogButtonClass::On_LButton_Up (const Vector2 &mouse_pos)
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
DialogButtonClass::On_Mouse_Move (const Vector2 &mouse_pos)
{	
	IsMouseOverMe = Rect.Contains (mouse_pos);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Bitmap_Button
//
////////////////////////////////////////////////////////////////
void
DialogButtonClass::Create_Bitmap_Button (void)
{
	ButtonRenderers[0].Reset ();
	ButtonRenderers[1].Reset ();

	//
	//	Turn off texturing on the UI background
	//
	TextureClass *texture = WW3DAssetManager::Get_Instance ()->Get_Texture ("button_test04.tga", TextureClass::MIP_LEVELS_1);
	ButtonRenderers[0].Set_Texture (texture);
	ButtonRenderers[1].Set_Texture (texture);
	texture->Release_Ref();
	
	RectClass uv_rect1 (0, 0, 161.0F / 256.0F, 63.0F / 256.0F);
	ButtonRenderers[0].Add_Quad (Rect, uv_rect1);
	return ;
}

void
Blit_Section
(	
	Render2DClass &	renderer,
	const Vector2 &	screen_pos,
	const Vector2 &	texture_pos,
	const Vector2 &	pixels,
	const Vector2 &	texture_dimensions
)
{
	RectClass screen_rect;
	screen_rect.Left		= screen_pos.X;
	screen_rect.Top		= screen_pos.Y;
	screen_rect.Right		= screen_rect.Left + pixels.X;
	screen_rect.Bottom	= screen_rect.Top + pixels.Y;

	RectClass uv_rect (texture_pos.X, texture_pos.Y, texture_pos.X + pixels.X, texture_pos.Y + pixels.Y);
	uv_rect.Inverse_Scale (Vector2 (texture_dimensions.X - 1, texture_dimensions.Y - 1));

	renderer.Add_Quad (screen_rect, uv_rect);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Component_Button
//
////////////////////////////////////////////////////////////////
void
DialogButtonClass::Create_Component_Button (void)
{
	ButtonRenderers[0].Reset ();
	ButtonRenderers[1].Reset ();
	ButtonRenderers[0].Set_Coordinate_Range (Render2DClass::Get_Screen_Resolution());
	ButtonRenderers[1].Set_Coordinate_Range (Render2DClass::Get_Screen_Resolution());

	//
	//	Turn off texturing on the UI background
	//
	TextureClass *texture = WW3DAssetManager::Get_Instance ()->Get_Texture ("button_test04.tga", TextureClass::MIP_LEVELS_1);
	ButtonRenderers[0].Set_Texture (texture);
	ButtonRenderers[1].Set_Texture (texture);
	texture->Release_Ref();

	float height		= 15.0F;
	float edge_width	= 17.0F;

	float right			= 160.0F;
	float	bottom		= 62.0F;

	float dn_start		= 63.0F;

	//float horz_tile_width	= 20.0F;
	//float vert_tile_height	= 10.0F;

	Vector2 texture_dimensions (256.0F, 256.0F);

	//
	//	Upper left
	//
	::Blit_Section (ButtonRenderers[0], Vector2 (Rect.Left, Rect.Top), Vector2 (0, 0),
		Vector2 (edge_width, height), texture_dimensions);

	::Blit_Section (ButtonRenderers[1], Vector2 (Rect.Left, Rect.Top), Vector2 (0, dn_start),
		Vector2 (edge_width, height), texture_dimensions);

	//
	//	Upper right
	//
	::Blit_Section (ButtonRenderers[0], Vector2 (Rect.Right - edge_width, Rect.Top), Vector2 (right - edge_width, 0),
		Vector2 (edge_width, height), texture_dimensions);

	::Blit_Section (ButtonRenderers[1], Vector2 (Rect.Right - edge_width, Rect.Top), Vector2 (right - edge_width, dn_start),
		Vector2 (edge_width, height), texture_dimensions);

	//
	//	Lower left
	//
	::Blit_Section (ButtonRenderers[0], Vector2 (Rect.Left, Rect.Bottom - height), Vector2 (0, bottom - height),
		Vector2 (edge_width, height), texture_dimensions);

	::Blit_Section (ButtonRenderers[1], Vector2 (Rect.Left, Rect.Bottom - height), Vector2 (0, dn_start + bottom - height),
		Vector2 (edge_width, height), texture_dimensions);

	//
	//	Lower right
	//
	::Blit_Section (ButtonRenderers[0], Vector2 (Rect.Right - edge_width, Rect.Bottom - height), Vector2 (right - edge_width, bottom - height),
		Vector2 (edge_width, height), texture_dimensions);

	::Blit_Section (ButtonRenderers[1], Vector2 (Rect.Right - edge_width, Rect.Bottom - height), Vector2 (right - edge_width, dn_start + bottom - height),
		Vector2 (edge_width, height), texture_dimensions);


	Vector2 horz_top_pos ((right / 2) - 5, 0);	
	Vector2 horz_bottom_pos ((right / 2) - 5, bottom - height);
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

		::Blit_Section (ButtonRenderers[1], Vector2 (x_pos, Rect.Top), horz_top_pos + Vector2 (0, dn_start),
			horz_size, texture_dimensions);

		::Blit_Section (ButtonRenderers[1], Vector2 (x_pos, Rect.Bottom - height), horz_bottom_pos + Vector2 (0, dn_start),
			horz_size, texture_dimensions);
		
		x_pos += horz_size.X;
		remaining_width -= horz_size.X;
	}


	Vector2 vert_left_pos (0, (bottom / 2) - 5);
	Vector2 vert_right_pos (right - edge_width, (bottom / 2) - 5);
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

		::Blit_Section (ButtonRenderers[1], Vector2 (Rect.Left, y_pos), vert_left_pos + Vector2 (0, dn_start),
			vert_size, texture_dimensions);

		::Blit_Section (ButtonRenderers[1], Vector2 (Rect.Right - edge_width, y_pos), vert_right_pos  + Vector2 (0, dn_start),
			vert_size, texture_dimensions);
 		
		y_pos += vert_size.Y;
		remaining_height -= vert_size.Y;
	}

	//
	//	"Patch" fill the interior
	//
	y_pos = Rect.Top + height;
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
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Component_Button2
//
////////////////////////////////////////////////////////////////
void
DialogButtonClass::Create_Component_Button2 (void)
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
DialogButtonClass::Create_Text_Renderers (void)
{
	TextRenderers[0].Reset ();
	TextRenderers[1].Reset ();

	//
	//	Draw the text
	//
	StyleMgrClass::Render_Text (Title, &TextRenderers[0], Rect, true, true, StyleMgrClass::CENTER_JUSTIFY);
	StyleMgrClass::Render_Text (Title, &TextRenderers[1], Rect, true, true, StyleMgrClass::CENTER_JUSTIFY);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Cursor
//
////////////////////////////////////////////////////////////////
void
DialogButtonClass::On_Set_Cursor (const Vector2 &mouse_pos)
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
//	On_Kill_Focus
//
////////////////////////////////////////////////////////////////
void
DialogButtonClass::On_Kill_Focus (DialogControlClass *focus)
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
void
DialogButtonClass::On_Key_Down (uint32 key_id, uint32 key_data)
{
	switch (key_id)
	{
		case VK_RETURN:
		case VK_SPACE:
			Parent->On_Command (ID, BN_CLICKED, 0);
			break;
	}

	return ;
}
