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
 *                     $Archive:: /Commando/Code/wwui/stylemgr.h           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/26/02 1:21p                                              $*
 *                                                                                             *
 *                    $Revision:: 16                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __STYLE_MGR_H
#define __STYLE_MGR_H

#include "bittype.h"
#include "widestring.h"
#include "vector.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class TextureClass;
class Render2DClass;
class Render2DSentenceClass;
class FontCharsClass;
class RectClass;


////////////////////////////////////////////////////////////////
//
//	StyleMgrClass
//
////////////////////////////////////////////////////////////////
class StyleMgrClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constants
	////////////////////////////////////////////////////////////////

	typedef enum
	{
		FONT_TITLE			= 0,
		FONT_LG_CONTROLS,
		FONT_CONTROLS,
		FONT_LISTS,
		FONT_TOOLTIPS,
		FONT_MENU,
		FONT_SM_MENU,
		FONT_HEADER,
		FONT_BIG_HEADER,
		FONT_CREDITS,
		FONT_CREDITS_BOLD,
		FONT_INGAME_TXT,
		FONT_INGAME_BIG_TXT,
		FONT_INGAME_SUBTITLE_TXT,
		FONT_INGAME_HEADER_TXT,
		FONT_MAX
	} FONT_STYLE;

	typedef enum
	{
		LEFT_JUSTIFY		= 0,
		RIGHT_JUSTIFY,
		CENTER_JUSTIFY

	} JUSTIFICATION;


	typedef enum
	{
		EVENT_MOUSE_CLICK		= 0,
		EVENT_MOUSE_OVER,
		EVENT_MENU_BACK,
		EVENT_POPUP,
		EVENT_AUDIO_MAX

	} EVENT_AUDIO;


	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Initialization
	//
	static void				Initialize (void);
	static void				Initialize_From_INI (const char *filename);
	static void				Shutdown (void);

	//
	//	Font methods
	//
	static FontCharsClass *			Get_Font (FONT_STYLE style);
	static FontCharsClass *			Peek_Font (FONT_STYLE style)	{ return Fonts[style]; }
	static void							Assign_Font (Render2DSentenceClass *renderer, FONT_STYLE style);

	//
	//	Sound methods
	//
	static void							Play_Sound (EVENT_AUDIO event);

	//
	//	Configuration methods
	//
	static void				Configure_Renderer (Render2DClass *renderer);

	//
	//	Scale support
	//
	static float			Get_X_Scale (void)	{ return ScaleX; }
	static float			Get_Y_Scale (void)	{ return ScaleY; }

	//
	//	Color methods
	//
	static uint32			Get_Text_Color (void)						{ return TextColor; }
	static uint32			Get_Text_Shadow_Color (void)				{ return TextShadowColor; }
	static uint32			Get_Disabled_Text_Color (void)			{ return DisabledTextColor; }
	static uint32			Get_Disabled_Text_Shadow_Color (void)	{ return DisabledTextShadowColor; }
	static uint32			Get_Line_Color (void)						{ return LineColor; }
	static uint32			Get_Bk_Color (void)							{ return BkColor; }
	static uint32			Get_Disabled_Line_Color (void)			{ return DisabledLineColor; }
	static uint32			Get_Disabled_Bk_Color (void)				{ return DisabledBkColor; }
	static uint32			Get_Tab_Text_Color (void)					{ return TabTextColor; }
	static uint32			Get_Tab_Glow_Color (void)					{ return TabGlowColor; }

	//
	//	Backdrop support
	//
	static void				Render_Backdrop (Render2DClass *renderer, const RectClass &rect);
	
	//
	//	Text support
	//
	static void				Render_Text (const unichar_t *text, Render2DSentenceClass *renderer, uint32 text_color, uint32 shadow_color, const RectClass &rect, bool do_shadow = false, bool do_clip = true, JUSTIFICATION justify = LEFT_JUSTIFY, bool is_vcentered = true);
	static void				Render_Text (const unichar_t *text, Render2DSentenceClass *renderer, const RectClass &rect, bool do_shadow = false, bool do_clip = true, JUSTIFICATION justify = LEFT_JUSTIFY, bool is_enabled = true, bool is_vcentered = true);
	static void				Render_Title_Text (const unichar_t *text, Render2DSentenceClass *renderer, const RectClass &rect);
	static void				Render_Wrapped_Text (const unichar_t *text, Render2DSentenceClass *renderer, uint32 text_color, uint32 shadow_color, const RectClass &rect, bool do_shadow = false, bool do_vcenter = false);
	static void				Render_Wrapped_Text (const unichar_t *text, Render2DSentenceClass *renderer, const RectClass &rect, bool do_shadow = false, bool do_vcenter = false, bool is_enabled = true);	
	static void				Render_Wrapped_Text_Ex (const unichar_t *text, Render2DSentenceClass *renderer, const RectClass &rect, bool do_shadow = false, bool do_vcenter = false, bool is_enabled = true, JUSTIFICATION justify = LEFT_JUSTIFY);
	static void				Render_Wrapped_Text_Ex (const unichar_t *text, Render2DSentenceClass *renderer, uint32 text_color, uint32 shadow_color, const RectClass &rect, bool do_shadow = false, bool do_vcenter = false, JUSTIFICATION justify = LEFT_JUSTIFY);

	//
	//	Hilight support
	//
	static void				Configure_Hilighter (Render2DClass *renderer);
	static void				Render_Hilight (Render2DClass *renderer, const RectClass &rect);

	//
	//	Text "glow" support
	//
	static void				Render_Glow (const unichar_t *text, Render2DSentenceClass *renderer, const RectClass &rect, int radius_x, int radius_y, int color, JUSTIFICATION justify = LEFT_JUSTIFY);

private:

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////	
	static TextureClass *			BackdropTexture;	
	static uint32						TitleColor;
	static uint32						TitleHilightColor;
	static uint32						TitleShadowColor;
	static uint32						TextColor;
	static uint32						TextShadowColor;
	static uint32						LineColor;
	static uint32						BkColor;
	static uint32						DisabledTextColor;
	static uint32						DisabledTextShadowColor;
	static uint32						DisabledLineColor;
	static uint32						DisabledBkColor;
	static uint32						HilightColor;
	static uint32						TabTextColor;
	static uint32						TabGlowColor;
	static FontCharsClass *			Fonts[FONT_MAX];
	static float						ScaleX;
	static float						ScaleY;
	static DynamicVectorClass<StringClass>	FontFileList;
	static StringClass				EventAudioList[EVENT_AUDIO_MAX];
};

#endif //__STYLE_MGR_H

