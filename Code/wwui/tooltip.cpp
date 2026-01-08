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
 *                     $Archive:: /Commando/Code/wwui/tooltip.cpp           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/24/02 4:05p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "tooltip.h"
#include "assetmgr.h"
#include "refcount.h"
#include "font3d.h"
#include "stylemgr.h"
#include <algorithm>

////////////////////////////////////////////////////////////////
//
//	ToolTipClass
//
////////////////////////////////////////////////////////////////
ToolTipClass::ToolTipClass (void)	:
	Rect (0, 0, 0, 0),
	TextColor (0, 0, 0),
	BkColor (1.0F, 0.956F, 0.733F)
{
	//
	//	Set the font for the text renderers
	//
	StyleMgrClass::Assign_Font (&TextRenderer, StyleMgrClass::FONT_TOOLTIPS);

	return ;
}


////////////////////////////////////////////////////////////////
//
//	~ToolTipClass
//
////////////////////////////////////////////////////////////////
ToolTipClass::~ToolTipClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Text_Renderer
//
////////////////////////////////////////////////////////////////
void
ToolTipClass::Create_Text_Renderer (void)
{
	Render2DSentenceClass &renderer = TextRenderer;

	//
	//	Setup the coordinate system of the renderer
	//
	renderer.Reset ();

	//
	//	Calculate where to draw the text
	//
	Vector2 text_extent = renderer.Get_Text_Extents (Text);

	int x_pos = int(Rect.Left + ((Rect.Width() - text_extent.X) / 2));
	int y_pos = int(Rect.Top + ((Rect.Height() - text_extent.Y) / 2));

	//
	//	Draw the text
	//
	renderer.Set_Location(Vector2 (x_pos, y_pos));
	renderer.Set_Clipping_Rect (Rect);

	renderer.Build_Sentence(Text);
	renderer.Draw_Sentence(VRGB_TO_INT32(TextColor));

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Background_Renderer
//
////////////////////////////////////////////////////////////////
void
ToolTipClass::Create_Background_Renderer (void)
{
	Render2DClass &renderer = BackgroundRenderer;

	//
	//	Configure this renderer
	//
	renderer.Reset ();
	renderer.Enable_Texturing (false);
	renderer.Set_Coordinate_Range (Render2DClass::Get_Screen_Resolution());

	//
	//	Make a hilight color by brightening the background color
	//
	Vector3 temp = BkColor * 2;
	temp.X = std::min (255.0F, temp.X);
	temp.Y = std::min (255.0F, temp.Y);
	temp.Z = std::min (255.0F, temp.Z);

	//
	//	Make two int's representing the hilight and shadow colors
	//
	int hilight_color = VRGB_TO_INT32 (temp);
	int shadow_color	= VRGB_TO_INT32 (Vector3 (0, 0, 0));

	//
	//	Draw the box outline
	//
	RectClass rect = Rect;
	renderer.Add_Line (Vector2 (rect.Left, rect.Bottom),	Vector2 (rect.Left, rect.Top-1),		1, hilight_color);
	renderer.Add_Line (Vector2 (rect.Left, rect.Top),		Vector2 (rect.Right, rect.Top),		1, hilight_color);
	renderer.Add_Line (Vector2 (rect.Right, rect.Top),		Vector2 (rect.Right, rect.Bottom),	1, shadow_color);
	renderer.Add_Line (Vector2 (rect.Right, rect.Bottom),	Vector2 (rect.Left, rect.Bottom),	1, shadow_color);

	//
	//	Now draw the background
	//
	rect.Right	-= 1;
	rect.Bottom -= 1;
	renderer.Add_Quad (rect, VRGB_TO_INT32 (BkColor));
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
ToolTipClass::Render (void)
{
	//
	//	Render the background and text
	//
	BackgroundRenderer.Render ();
	TextRenderer.Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Position
//
////////////////////////////////////////////////////////////////
void
ToolTipClass::Set_Position (const Vector2 &pos)
{
	Rect.Left	= pos.X;
	Rect.Top		= pos.Y;

	Update_Rect ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Text
//
////////////////////////////////////////////////////////////////
void
ToolTipClass::Set_Text (const unichar_t *text)
{
	Text = text;
	
	//
	//	Force the renderer's to be repositioned and resized
	//
	Update_Rect ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Rect
//
////////////////////////////////////////////////////////////////
void
ToolTipClass::Update_Rect (void)
{
	//
	//	Get the dimensions of the string
	//
	const RectClass &screen_rect = Render2DClass::Get_Screen_Resolution ();
	Vector2 text_extent = TextRenderer.Get_Text_Extents (Text);

	if (text_extent.X <= 0.0f) {
		BackgroundRenderer.Reset();
		TextRenderer.Reset();
		return;
	}

	//
	//	Update the rectangle's width and height
	//
	Rect.Right	= Rect.Left + (text_extent.X + 5); //10);
	Rect.Bottom	= Rect.Top + (text_extent.Y + 4); //* 2);


	//
	//	Clip the rectangle to the right side of the screen
	//
	if (Rect.Right > screen_rect.Right) {
		float delta = Rect.Right - screen_rect.Right;
		Rect.Left	-= delta;
		Rect.Right	-= delta;
	}

	//
	//	Clip the rectangle to the bottom of the screen
	//
	if (Rect.Bottom > screen_rect.Bottom) {
		float delta = Rect.Bottom - screen_rect.Bottom;
		Rect.Top		-= delta;
		Rect.Bottom	-= delta;
	}

	//
	//	Update the renderer's
	//
	Create_Text_Renderer ();
	Create_Background_Renderer ();
	return ;
}

