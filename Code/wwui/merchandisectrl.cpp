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
 *                 Project Name : WWUI																			  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwui/merchandisectrl.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/10/02 2:55p                                               $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "merchandisectrl.h"
#include "stylemgr.h"
#include "mousemgr.h"
#include "assetmgr.h"
#include "texture.h"
#include "dialogbase.h"
#include "vector4.h"
#include <algorithm>


//////////////////////////////////////////////////////////////////////
//	Local constants
//////////////////////////////////////////////////////////////////////
static const float BUTTON_WIDTH	= 16.0F;
static const float BUTTON_HEIGHT	= 16.0F;


//////////////////////////////////////////////////////////////////////
//
//	MerchandiseCtrlClass
//
//////////////////////////////////////////////////////////////////////
MerchandiseCtrlClass::MerchandiseCtrlClass (void)	:
	UserData (0),
	Cost (0),
	CurrentTextureIndex (0),
	Count (0)
{
	//
	//	Set the font for the text renderer
	//
	StyleMgrClass::Assign_Font (&NameTextRenderer, StyleMgrClass::FONT_TOOLTIPS);
	StyleMgrClass::Assign_Font (&CostTextRenderer, StyleMgrClass::FONT_HEADER);
	StyleMgrClass::Assign_Font (&CountTextRenderer, StyleMgrClass::FONT_TOOLTIPS);
	StyleMgrClass::Configure_Renderer (&ControlRenderer);
	StyleMgrClass::Configure_Renderer (&TextureRenderer);	
	StyleMgrClass::Configure_Renderer (&ButtonRenderer);		
	StyleMgrClass::Configure_Renderer (&HilightRenderer);
	StyleMgrClass::Configure_Hilighter (&HilightRenderer);
	TextureRenderer.Enable_Texturing (true);
	ButtonRenderer.Enable_Texturing (true);	

	//
	//	Pass the texture onto the renderer
	//
	TextureClass *texture = WW3DAssetManager::Get_Instance ()->Get_Texture ("hud_cnc_Button.tga", TextureClass::MIP_LEVELS_1);
	ButtonRenderer.Set_Texture (texture);
	REF_PTR_RELEASE (texture);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~MerchandiseCtrlClass
//
//////////////////////////////////////////////////////////////////////
MerchandiseCtrlClass::~MerchandiseCtrlClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Text_Renderer
//
////////////////////////////////////////////////////////////////
void
MerchandiseCtrlClass::Create_Text_Renderer (void)
{
	//
	//	Convert the cost to a string
	//
	WideStringClass cost_text;
	cost_text.Format (U_CHAR("%d"), Cost);

	//
	//	Render the text
	//
	NameTextRenderer.Reset ();
	CostTextRenderer.Reset ();
	CountTextRenderer.Reset ();
	StyleMgrClass::Render_Text (Title, &NameTextRenderer, RGB_TO_INT32 (255, 255, 255), RGB_TO_INT32 (0, 0, 0), TextRect, true, true, StyleMgrClass::LEFT_JUSTIFY, true);
	StyleMgrClass::Render_Text (cost_text, &CostTextRenderer, RGB_TO_INT32 (255, 255, 255), RGB_TO_INT32 (0, 0, 0), CostRect, true, true, StyleMgrClass::LEFT_JUSTIFY, true);

	//
	//	Render the counter (if necessary)
	//
	if (Count > 1) {

		//
		//	Convert the count to a string
		//
		WideStringClass count_text;
		count_text.Format (U_CHAR("%d"), Count);
		StyleMgrClass::Render_Text (count_text, &CountTextRenderer, RGB_TO_INT32 (255, 255, 255), RGB_TO_INT32 (0, 0, 0), CountRect, true, true, StyleMgrClass::CENTER_JUSTIFY, true);	
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Render
//
//////////////////////////////////////////////////////////////////////
void
MerchandiseCtrlClass::Render (void)
{
	//
	//	Update the text renderer (if necessary)
	//
	if (IsDirty) {
		Create_Text_Renderer ();
		Create_Control_Renderer ();
		Create_Texture_Renderer ();
	}

	//
	//	Render the parts of the control
	//
	TextureRenderer.Render ();
	ControlRenderer.Render ();
	NameTextRenderer.Render ();
	CostTextRenderer.Render ();
	CountTextRenderer.Render ();
	ButtonRenderer.Render ();
	HilightRenderer.Render ();

	DialogControlClass::Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Client_Rect
//
////////////////////////////////////////////////////////////////
void
MerchandiseCtrlClass::Update_Client_Rect (void)
{
	//
	//	Set the client area
	//
	ClientRect = Rect;
	ClientRect.Inflate (Vector2 (-1, -1));

	//
	//	Make the texture rectangle the 2/3 of the space
	//
	TextureRect				= Rect;
	TextureRect.Bottom	= int(TextureRect.Top + (Rect.Height () * 0.667F));

	//
	//	Make the cost rect use up 60% of the remaining space
	//
	CostRect				= Rect;
	CostRect.Top		= int(TextureRect.Bottom + 1);
	CostRect.Bottom	= CostRect.Top + int((Rect.Bottom - TextureRect.Bottom) * 0.6F);
	
	//
	//	The rest of the space goes to the text rect
	//
	TextRect				= Rect;
	TextRect.Top		= CostRect.Bottom;

	//
	//	The counter lives in the upper-right corner
	//
	CountRect			= Rect;
	CountRect.Left		= CountRect.Left + int(Rect.Width () * 0.75F);
	CountRect.Bottom	= CountRect.Top + int(TextureRect.Height () * 0.25F);

	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Cursor
//
////////////////////////////////////////////////////////////////
void
MerchandiseCtrlClass::On_Set_Cursor (const Vector2 &/* mouse_pos */)
{
	//
	//	Change the mouse cursor
	//
	MouseMgrClass::Set_Cursor (MouseMgrClass::CURSOR_ACTION);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Control_Renderer
//
////////////////////////////////////////////////////////////////
void
MerchandiseCtrlClass::Create_Control_Renderer (void)
{
	Render2DClass &renderer = ControlRenderer;

	//
	//	Configure this renderer
	//
	renderer.Reset ();
	HilightRenderer.Reset ();
	ButtonRenderer.Reset ();
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
	//	Draw the outlines
	//
	if (Count > 0) {
		StyleMgrClass::Render_Hilight (&HilightRenderer, Rect);
	}

	//
	//	Draw the cycle button in the upper right corner...
	//
	if (TextureNameList.Count () > 1) {		
		
		//
		//	Draw the button
		//
		RectClass button_rect;
		button_rect.Right		= Rect.Right - 1;
		button_rect.Top		= Rect.Top + 2;
		button_rect.Left		= int(button_rect.Right - (BUTTON_WIDTH * StyleMgrClass::Get_X_Scale ()));
		button_rect.Bottom	= int(button_rect.Top + (BUTTON_HEIGHT * StyleMgrClass::Get_Y_Scale ()));

		ButtonRenderer.Add_Quad (button_rect);
	}

	//
	//	Convert the cost to a string
	//
	WideStringClass cost_text;
	cost_text.Format (U_CHAR("%d"), Cost);

	//
	//	Get the dimensions of the text
	//
	Vector2 name_extents = NameTextRenderer.Get_Text_Extents (Title);
	Vector2 cost_extents = CostTextRenderer.Get_Text_Extents (cost_text);

	//
	//	Build rectangles for the backdrops of the cost and text
	//
	RectClass text_rect = TextRect;
	RectClass cost_rect = CostRect;
	text_rect.Right = int(text_rect.Left + name_extents.X + 4.0F);
	cost_rect.Right = int(cost_rect.Left + cost_extents.X + 4.0F);

	//
	//	Clip the backdrop to the bounding rectangle
	//
	text_rect.Right = std::min (text_rect.Right, Rect.Right);
	cost_rect.Right = std::min (cost_rect.Right, Rect.Right);
	
	//
	//	Render the text backdrops
	//
	Vector4 vector_color;
	INT32_TO_VRGBA (color, vector_color);
	vector_color.W = 0.0F;
	renderer.Add_Quad_HGradient (text_rect, color, VRGBA_TO_INT32 (vector_color));
	renderer.Add_Quad_HGradient (cost_rect, color, VRGBA_TO_INT32 (vector_color));
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Texture_Renderer
//
////////////////////////////////////////////////////////////////
void
MerchandiseCtrlClass::Create_Texture_Renderer (void)
{
	/*float smallest_dim = std::min (TextureRect.Width (), TextureRect.Height ());

	RectClass rect;
	rect.Left	= int(TextureRect.Left + (TextureRect.Width () * 0.5F) - (smallest_dim * 0.5F));
	rect.Top		= int(TextureRect.Top + (TextureRect.Height () * 0.5F) - (smallest_dim * 0.5F));
	rect.Right	= int(rect.Left + smallest_dim);
	rect.Bottom	= int(rect.Top + smallest_dim);*/

	//
	//	Darken the bitmap if its disabled
	//
	int color = 0xFFFFFFFF;
	if (IsEnabled == false) {
		color = RGB_TO_INT32 (96, 96, 96);
	}

	//
	//	Configure this renderer
	//
	TextureRenderer.Reset ();	
	TextureRenderer.Add_Quad (Rect, color);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Texture
//
////////////////////////////////////////////////////////////////
void
MerchandiseCtrlClass::Set_Texture (const char *texture_name)
{
	CurrentTextureIndex = 0;

	//
	//	Cache the texture name
	//
	TextureNameList.Delete_All ();
	TextureNameList.Add (texture_name);

	//
	//	Pass the texture onto the renderer
	//
	TextureClass *texture = WW3DAssetManager::Get_Instance ()->Get_Texture (texture_name, TextureClass::MIP_LEVELS_1);	
	TextureRenderer.Set_Texture (texture);
	REF_PTR_RELEASE (texture);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Add_Alternate_Texture
//
////////////////////////////////////////////////////////////////
void
MerchandiseCtrlClass::Add_Alternate_Texture (const char *texture_name)
{
	TextureNameList.Add (texture_name);
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Down
//
////////////////////////////////////////////////////////////////
void
MerchandiseCtrlClass::On_LButton_Down (const Vector2 &mouse_pos)
{
	RectClass button_rect;
	button_rect.Right		= Rect.Right - 1;
	button_rect.Top		= Rect.Top + 2;
	button_rect.Left		= int(button_rect.Right - (BUTTON_WIDTH * StyleMgrClass::Get_X_Scale ()));
	button_rect.Bottom	= int(button_rect.Top + (BUTTON_HEIGHT * StyleMgrClass::Get_Y_Scale ()));

	//
	//	Did the user click in the cycle button?
	//
	if (TextureNameList.Count () > 1 && button_rect.Contains (mouse_pos)) {
		
		//
		//	Cycle to the next texture
		//
		CurrentTextureIndex ++;
		if (CurrentTextureIndex >= TextureNameList.Count ()) {
			CurrentTextureIndex = 0;
		}

		//
		//	Pass the texture onto the renderer
		//
		TextureClass *texture = WW3DAssetManager::Get_Instance ()->Get_Texture (TextureNameList[CurrentTextureIndex], TextureClass::MIP_LEVELS_1);	
		if (texture != NULL) {
			TextureRenderer.Set_Texture (texture);
			REF_PTR_RELEASE (texture);
		}

		Set_Dirty ();

	} else {

		//
		//	Notify the owner that this item has been purchased
		//
		ADVISE_NOTIFY (On_Merchandise_Selected (this, Get_ID ()));
	}
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_DblClk
//
////////////////////////////////////////////////////////////////
void
MerchandiseCtrlClass::On_LButton_DblClk (const Vector2 &mouse_pos)
{
	RectClass button_rect;
	button_rect.Right		= Rect.Right - 1;
	button_rect.Top		= Rect.Top + 2;
	button_rect.Left		= int(button_rect.Right - (BUTTON_WIDTH * StyleMgrClass::Get_X_Scale ()));
	button_rect.Bottom	= int(button_rect.Top + (BUTTON_HEIGHT * StyleMgrClass::Get_Y_Scale ()));

	//
	//	Did the user click in the cycle button?
	//
	if (TextureNameList.Count () == 0 || button_rect.Contains (mouse_pos) == false) {

		//
		//	Notify the owner that this item has been double-clicked
		//
		ADVISE_NOTIFY (On_Merchandise_DblClk (this, Get_ID ()));
	}

	return ;
}
