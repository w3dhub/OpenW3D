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
 *                     $Archive:: /Commando/Code/wwui/popupdialog.cpp         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/02/02 11:25a                                              $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "popupdialog.h"
#include "assetmgr.h"
#include "texture.h"
#include "stylemgr.h"


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
const int	TITLE_BORDER_WIDTH	= 10;
const int	TITLE_BORDER_HEIGHT	= 8;


////////////////////////////////////////////////////////////////
//
//	PopupDialogClass
//
////////////////////////////////////////////////////////////////
PopupDialogClass::PopupDialogClass (const DialogResource *dialog_resource)	:
	DialogBaseClass (dialog_resource)
{
	//
	//	Configure the renderers
	//
	StyleMgrClass::Configure_Renderer (&BackgroundRenderer);
	StyleMgrClass::Configure_Renderer (&BlackoutRenderer);
	StyleMgrClass::Assign_Font (&TextRenderer, StyleMgrClass::FONT_HEADER);

	//
	//	Force this renderer to always render
	//
	BlackoutRenderer.Get_Shader ()->Set_Depth_Compare (ShaderClass::PASS_ALWAYS);

	//
	// By default, popup backgrounds are dark
	//
	IsBackgroundDarkened = true;

	return ;
}


////////////////////////////////////////////////////////////////
//
//	~PopupDialogClass
//
////////////////////////////////////////////////////////////////
PopupDialogClass::~PopupDialogClass (void)
{
	return ;
}


void PopupDialogClass::Set_Title(const wchar_t* title)
	{
	DialogBaseClass::Set_Title(title);
	Build_Background_Renderers();
	}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
PopupDialogClass::On_Init_Dialog (void)
{
	//
	//	Generate the backdrop
	//
	Build_Background_Renderers ();

	DialogBaseClass::On_Init_Dialog ();

	//
	//	Play the sound effect
	//
	StyleMgrClass::Play_Sound (StyleMgrClass::EVENT_POPUP);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Build_Background_Renderers
//
////////////////////////////////////////////////////////////////
void
PopupDialogClass::Build_Background_Renderers (void)
{
	//
	//	Configure this renderer
	//
	TextRenderer.Reset();
	BackgroundRenderer.Reset ();
	BlackoutRenderer.Reset ();
	BackgroundRenderer.Enable_Texturing (false);
	BlackoutRenderer.Enable_Texturing (false);

	//
	//	Darken the background if IsBackgroundDarkened is set
	//
	if (IsBackgroundDarkened) {
		BlackoutRenderer.Add_Quad (Render2DClass::Get_Screen_Resolution(), RGBA_TO_INT32 (0, 0, 0, 236));
	}

	//
	//	Determine which color to draw the outline in
	//
	int color	= StyleMgrClass::Get_Line_Color ();
	int bkcolor	= StyleMgrClass::Get_Bk_Color ();

	//
	//	Draw the control outline
	//
	RectClass rect = Rect;
	BackgroundRenderer.Add_Rect (rect, 1.0F, color, bkcolor);

	if (Title.Get_Length () > 0) {

		//
		//	Determine what scale to use
		//
		float scale_x = Render2DClass::Get_Screen_Resolution().Width () / 800;
		float scale_y = Render2DClass::Get_Screen_Resolution().Height () / 600;

		//
		//	Calculate the title bar rectangle
		//
		Vector2 text_extent = TextRenderer.Get_Text_Extents (Title);
		if (text_extent != Vector2(0,0)) {
			RectClass text_rect;
			text_rect.Left		= Rect.Left;
			text_rect.Right	= Rect.Left + int(text_extent.X + (TITLE_BORDER_WIDTH * 2 * scale_x));
			text_rect.Top		= Rect.Top - int(text_extent.Y + (TITLE_BORDER_HEIGHT * 2 * scale_y));
			text_rect.Bottom	= Rect.Top;

			//
			//	Draw the title bar
			//
			BackgroundRenderer.Add_Rect (text_rect, 1.0F, color, bkcolor);

			//
			//	Draw the title text
			//
			StyleMgrClass::Render_Text (Title, &TextRenderer, text_rect, true, true, StyleMgrClass::CENTER_JUSTIFY);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
PopupDialogClass::Render (void)
{
	//
	//	Render the backdrop
	//
	BlackoutRenderer.Render ();
	BackgroundRenderer.Render ();
	TextRenderer.Render ();

	//
	//	Render the controls
	//
	DialogBaseClass::Render ();
	return ;
}
