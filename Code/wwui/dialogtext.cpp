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
 *                     $Archive:: /Commando/Code/wwui/dialogtext.cpp           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/08/01 12:14a                                             $*
 *                                                                                             *
 *                    $Revision:: 19                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dialogtext.h"
#include "stylemgr.h"


////////////////////////////////////////////////////////////////
//
//	DialogButtonClass
//
////////////////////////////////////////////////////////////////
DialogTextClass::DialogTextClass (void)	:
	IsTitle (false)
{
	Set_Wants_Focus (false);

	//
	//	Set the font for the text renderer
	//
	StyleMgrClass::Assign_Font (&TextRenderer, StyleMgrClass::FONT_CONTROLS);	
	StyleMgrClass::Configure_Renderer (&ControlRenderer);	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
DialogTextClass::Render (void)
{
	if (Style & WS_VISIBLE) {
	
		//
		//	Update the text renderer (if necessary)
		//
		if (IsDirty) {
			Create_Text_Renderer ();
		}

		if (IsTitle) {
			GlowRenderer.Render ();
		}
		
		//
		//	Render the control
		//
		TextRenderer.Render ();
		ControlRenderer.Render ();

		DialogControlClass::Render ();
  	}
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Text_Renderer
//
////////////////////////////////////////////////////////////////
void
DialogTextClass::Create_Text_Renderer (void)
{
	TextRenderer.Reset ();
	ControlRenderer.Reset ();

	if ((Style & SS_TYPEMASK) == SS_ETCHEDHORZ) {
		
		//
		//	Determine which color to draw the outline in
		//
		int color	= StyleMgrClass::Get_Line_Color ();
		if (IsEnabled == false) {
			color		= StyleMgrClass::Get_Disabled_Line_Color ();
		}

		//
		//	Draw the line
		//
		ControlRenderer.Add_Line (Rect.Upper_Left (), Rect.Upper_Right (), 1.0F, color);

	} else if ((Style & SS_TYPEMASK) == SS_BLACKFRAME) {

		//
		//	Determine which color to draw the outline in
		//
		int color	= StyleMgrClass::Get_Line_Color ();
		if (IsEnabled == false) {
			color		= StyleMgrClass::Get_Disabled_Line_Color ();
		}

		//
		//	Draw the outline
		//
		ControlRenderer.Add_Outline (Rect, 1.0F, color);

	} else {
	
		//
		//	Determine how to justify the text
		//
		StyleMgrClass::JUSTIFICATION justification = StyleMgrClass::LEFT_JUSTIFY;
		if ((Style & 0xF) == SS_RIGHT) {
			justification = StyleMgrClass::RIGHT_JUSTIFY;
		} else if ((Style & 0xF) == SS_CENTER) {
			justification = StyleMgrClass::CENTER_JUSTIFY;
		}

		//
		//	Determine if the text should be centered vertically
		//
		bool is_vcentered = ((Style & SS_CENTERIMAGE) == SS_CENTERIMAGE);

		//
		//	Draw the text
		//
		if (IsTitle) {		
			
			//
			//	Render the title text using a glow
			//
			GlowRenderer.Reset_Polys ();
			StyleMgrClass::Render_Glow (Title, &GlowRenderer, Rect, 5, 5, RGB_TO_INT32 (14, 0, 0), StyleMgrClass::CENTER_JUSTIFY);
			StyleMgrClass::Render_Text (Title, &TextRenderer, RGB_TO_INT32 (255, 255, 36), 0, Rect, false, false, StyleMgrClass::CENTER_JUSTIFY, true);

		} else if ((Style & 0x0F) == SS_LEFTNOWORDWRAP) {

			//
			//	Render the text using the default color scheme or the overriden
			// color scheme
			//
			if (IsTextColorOverridden == false) {
				StyleMgrClass::Render_Text (Title, &TextRenderer, Rect, true, true,
										justification, IsEnabled, is_vcentered);
			} else {
				StyleMgrClass::Render_Text (Title, &TextRenderer, VRGB_TO_INT32 (TextColor),
										RGB_TO_INT32 (0, 0, 0), Rect, true, true, justification, is_vcentered);
			}

		} else {

			//
			//	Render the text using the default color scheme or the overriden
			// color scheme
			//
			if (IsTextColorOverridden == false) {
				StyleMgrClass::Render_Wrapped_Text_Ex (Title, &TextRenderer, Rect, true, is_vcentered, IsEnabled, justification);
			} else {
				StyleMgrClass::Render_Wrapped_Text_Ex (Title, &TextRenderer, VRGB_TO_INT32 (TextColor),
										RGB_TO_INT32 (0, 0, 0), Rect, true, is_vcentered, justification);
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Create
//
////////////////////////////////////////////////////////////////
void
DialogTextClass::On_Create (void)
{
	if (Title.Get_Length () >= 2) {
		
		//
		//	Does this string have special formatting?
		//
		if (Title[0] == U_CHAR('%') && Title[1] == U_CHAR('t')) {
			
			//
			//	Strip off the preceding format specifier
			//
			WideStringClass text	= Title.Peek_Buffer () + 2;
			Title						= text;

			//
			//	Keep in mind that this is a "Title"
			//
			IsTitle = true;
			StyleMgrClass::Assign_Font (&TextRenderer, StyleMgrClass::FONT_TITLE);
			StyleMgrClass::Assign_Font (&GlowRenderer, StyleMgrClass::FONT_TITLE);	

			GlowRenderer.Build_Sentence (Title);

		} else if (Title[0] == U_CHAR('%') && Title[1] == U_CHAR('h')) {
			
			//
			//	Support a "header" style font
			//
			StyleMgrClass::Assign_Font (&TextRenderer, StyleMgrClass::FONT_LG_CONTROLS);

			//
			//	Strip off the preceding format specifier
			//
			WideStringClass text	= Title.Peek_Buffer () + 2;
			Title						= text;
		
		} else if (Title[0] == U_CHAR('%') && Title[1] == U_CHAR('s')) {
			
			//
			//	Support a "small" style font
			//
			StyleMgrClass::Assign_Font (&TextRenderer, StyleMgrClass::FONT_TOOLTIPS);

			//
			//	Strip off the preceding format specifier
			//
			WideStringClass text	= Title.Peek_Buffer () + 2;
			Title						= text;
		}

	}

	return ;
}
