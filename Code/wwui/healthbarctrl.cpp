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
 *                 Project Name : wwui                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwui/healthbarctrl.cpp                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/25/01 4:33p                                                $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "healthbarctrl.h"
#include "texture.h"
#include "assetmgr.h"
#include "stylemgr.h"
#include <algorithm>


//////////////////////////////////////////////////////////////////////
//	Local constants
//////////////////////////////////////////////////////////////////////
static const char *	TEXTURE_NAME	= "HUD_C&C_HEALTHBAR.TGA";
static const Vector2 TEXTURE_SIZE	= Vector2 (16, 16);


//////////////////////////////////////////////////////////////////////
//
//	HealthBarCtrlClass
//
//////////////////////////////////////////////////////////////////////
HealthBarCtrlClass::HealthBarCtrlClass (void)	:
	Percent (1.0F)
{
	//
	//	Configure the renderer
	//
	StyleMgrClass::Configure_Renderer (&ControlRenderer);
	ControlRenderer.Enable_Texturing (true);
	ControlRenderer.Enable_Alpha (true);

	//
	//	Load the texture we'll use
	//
	TextureClass *texture = WW3DAssetManager::Get_Instance ()->Get_Texture (TEXTURE_NAME, TextureClass::MIP_LEVELS_1);
	ControlRenderer.Set_Texture (texture);
	REF_PTR_RELEASE (texture);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~HealthBarCtrlClass
//
//////////////////////////////////////////////////////////////////////
HealthBarCtrlClass::~HealthBarCtrlClass (void)
{	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Blit_Section
//
////////////////////////////////////////////////////////////////
static void
Blit_Section
(	
	Render2DClass &		renderer,
	const Vector2 &		screen_pos,
	const Vector2 &		texture_pos,
	const Vector2 &		pixels,
	const Vector2 &		texture_dimensions,
	int						color
)
{
	RectClass screen_rect;
	screen_rect.Left		= screen_pos.X;
	screen_rect.Top		= screen_pos.Y;
	screen_rect.Right		= screen_rect.Left + pixels.X;
	screen_rect.Bottom	= screen_rect.Top + pixels.Y;

	//
	//	Determine the UV coordinates
	//
	RectClass uv_rect (texture_pos.X, texture_pos.Y, texture_pos.X + pixels.X, texture_pos.Y + pixels.Y);
	uv_rect.Inverse_Scale (Vector2 (texture_dimensions.X, texture_dimensions.Y));
	
	//
	//	Draw the chunk
	//
	renderer.Add_Quad (screen_rect, uv_rect, color);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Create_Control_Renderer
//
//////////////////////////////////////////////////////////////////////
void
HealthBarCtrlClass::Create_Control_Renderer (void)
{
	Render2DClass &renderer = ControlRenderer;
	renderer.Reset ();

	//
	//	Determine what color to draw the bar in
	//
	int color = RGB_TO_INT32 (0, 255, 0);
	if (Percent <= 0.25F) {
		color = RGB_TO_INT32 (255, 0, 0);
	} else if (Percent <= 0.5F) {
		color = RGB_TO_INT32 (255, 255, 0);
	}
	
	//
	//	Calculate how long to draw the texture
	//
	float full_width	= Rect.Width ();
	int	width			= (full_width * Percent);

	Vector2 size1 (8.0F, 12.0F);
	Vector2 size2 (7.0F, 12.0F);
	Vector2 texture_pos1 (0.0F, 2.0F);
	Vector2 texture_pos2 (9.0F, 2.0F);

	Vector2 *size			= &size1;
	Vector2 *texture_pos	= &texture_pos1;

	//
	//	Tile the sections horizontally until we've reached our edge
	//
	int index					= 0;
	float remaining_width	= width;
	float x_pos					= Rect.Left;
	while (remaining_width > 0) {
		size->X = std::min (remaining_width, size->X);

		//
		//	Draw this section
		//
		::Blit_Section (ControlRenderer, Vector2 (x_pos, Rect.Top), *texture_pos,
			*size, TEXTURE_SIZE, color);
		
		//
		//	Advance to the next section
		//
		x_pos					+= size->X;
		remaining_width	-= size->X;
		
		//
		//	Advance to the next texture chunk
		//
		index ++;
		if (index > 0) {
			size				= &size2;
			texture_pos		= &texture_pos2;
		}
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Life
//
//////////////////////////////////////////////////////////////////////
void
HealthBarCtrlClass::Set_Life (float value)
{
	Percent = value;

	//
	//	Force an update
	//
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
HealthBarCtrlClass::Render (void)
{
	//
	//	Recreate the renderers (if necessary)
	//
	if (IsDirty) {
		Create_Control_Renderer ();
	}

	//
	//	Render the bar...
	//	
	ControlRenderer.Render ();

	DialogControlClass::Render ();
	return ;
}
