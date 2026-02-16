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
 *                     $Archive:: /Commando/Code/wwui/stylemgr.cpp          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/26/02 1:23p                                              $*
 *                                                                                             *
 *                    $Revision:: 33                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "stylemgr.h"
#include "render2d.h"
#include "assetmgr.h"
#include "texture.h"
#include "font3d.h"
#include "render2dsentence.h"
#include "ini.h"
#include "ffactory.h"
#include "WWAudio.h"
#include <algorithm>
#include <limits>


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
static const char *	DEFAULT_BACKDROP_NAME	= "GRADENT_TEST.TGA";


struct FONT_DESC
{
	const char *	name;
	int				point_size;
	bool				is_bold;
};


static FONT_DESC	DEFAULT_FONTS[StyleMgrClass::FONT_MAX] =
{
	{ "Regatta Condensed LET",	52,	false },
	{ "Arial MT",					12,	true },
	{ "Arial MT",					8,		true },
	{ "Arial MT",					8,		false },
	{ "Arial MT",					8,		false },
	{ "Regatta Condensed LET",	32,	false },
	{ "Regatta Condensed LET",	20,	false },
	{ "Arial MT",					9,		true },
	{ "Arial MT",					12,	true },
	
	{ "Arial MT",					10,	false },
	{ "Arial MT",					10,	true },

	{ "Arial MT",					8,		false },
	{ "Arial MT",					16,	false },
	{ "Arial MT",					14,	false },
	{ "Arial MT",					9,		true }


	/*{ "Arial Unicode MS",	52,	false },
	{ "Arial Unicode MS",	12,	true },
	{ "Arial Unicode MS",	12,		true },
	{ "Arial Unicode MS",	12,		false },
	{ "Arial Unicode MS",	12,		false },
	{ "Arial Unicode MS",	32,	false },
	{ "Arial Unicode MS",	20,	false },
	{ "Arial Unicode MS",	13,		true },
	{ "Arial Unicode MS",	14,	true }*/

};


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
TextureClass *				StyleMgrClass::BackdropTexture			= NULL;

uint32						StyleMgrClass::TitleColor					= RGB_TO_INT32 (255, 255, 255);
uint32						StyleMgrClass::TitleHilightColor			= RGB_TO_INT32 (255, 255, 0);
uint32						StyleMgrClass::TitleShadowColor			= RGB_TO_INT32 (0, 0, 0);
uint32						StyleMgrClass::TextColor					= RGB_TO_INT32 (255, 213, 40);
uint32						StyleMgrClass::TextShadowColor			= RGBA_TO_INT32 (0, 0, 0, 200);
uint32						StyleMgrClass::LineColor					= RGB_TO_INT32 (255, 174, 40);
uint32						StyleMgrClass::BkColor						= RGBA_TO_INT32 (255, 174, 40, 40);
uint32						StyleMgrClass::DisabledTextColor			= RGBA_TO_INT32 (255, 213, 40, 140);
uint32						StyleMgrClass::DisabledTextShadowColor	= RGBA_TO_INT32 (0, 0, 0, 96);
uint32						StyleMgrClass::DisabledLineColor			= RGBA_TO_INT32 (230, 160, 35, 128);
uint32						StyleMgrClass::DisabledBkColor			= RGBA_TO_INT32 (255, 174, 40, 30);
uint32						StyleMgrClass::HilightColor				= RGB_TO_INT32 (70, 70, 70);

uint32						StyleMgrClass::TabTextColor				= RGB_TO_INT32 (255, 255, 255);
uint32						StyleMgrClass::TabGlowColor				= RGB_TO_INT32 (16, 10, 0);

FontCharsClass *			StyleMgrClass::Fonts[FONT_MAX]			= { NULL };
float							StyleMgrClass::ScaleX						= 1.0F;
float							StyleMgrClass::ScaleY						= 1.0F;

DynamicVectorClass<StringClass>	StyleMgrClass::FontFileList;
StringClass								StyleMgrClass::EventAudioList[StyleMgrClass::EVENT_AUDIO_MAX];


////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////
void
StyleMgrClass::Initialize (void)
{
	//
	//	Compute the scale
	//
	ScaleX = Render2DClass::Get_Screen_Resolution().Width () / 800.0F;
	ScaleY = Render2DClass::Get_Screen_Resolution().Height () / 600.0F;

	//
	//	Load each font
	//
	for (int index = 0; index < FONT_MAX; index ++) {

		//
		//	Scale the point size to fit this resolution
		//
		float point_size = ((float)DEFAULT_FONTS[index].point_size) * ScaleY;

		//
		//	Create the font
		//
		Fonts[index] = WW3DAssetManager::Get_Instance()->Get_FontChars (DEFAULT_FONTS[index].name,
								point_size, DEFAULT_FONTS[index].is_bold);
	}

	//
	//	Load the backdrop texture name
	//
	BackdropTexture = WW3DAssetManager::Get_Instance()->Get_Texture (DEFAULT_BACKDROP_NAME, TextureClass::MIP_LEVELS_1);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Initialize_From_INI
//
////////////////////////////////////////////////////////////////
void
StyleMgrClass::Initialize_From_INI (const char *filename)
{
	Shutdown ();

	//
	//	Compute the scale
	//
	ScaleX = Render2DClass::Get_Screen_Resolution().Width () / 800.0F;
	ScaleY = Render2DClass::Get_Screen_Resolution().Height () / 600.0F;

	//
	//	Get the INI file
	//
	INIClass *ini_file = NULL;
	FileClass *file_obj = _TheFileFactory->Get_File (filename);
	if (file_obj != NULL && file_obj->Is_Available( ) ) {
		ini_file = new INIClass (*file_obj);
		_TheFileFactory->Return_File (file_obj);
	}

	if (ini_file != NULL) {

		const char *FONT_FILE_SECTION	= "Font File List";
		const char *FONT_NAME_SECTION	= "Font Names";

		const char *FONT_INI_ENTRIES[FONT_MAX] = 
		{		
			"FONT_TITLE",
			"FONT_LG_CONTROLS",
			"FONT_CONTROLS",
			"FONT_LISTS",
			"FONT_TOOLTIPS",
			"FONT_MENU",
			"FONT_SM_MENU",
			"FONT_HEADER",
			"FONT_BIG_HEADER",
			"FONT_CREDITS",
			"FONT_CREDITS_BOLD",
			"FONT_INGAME_TXT",
			"FONT_INGAME_BIG_TXT",
			"FONT_INGAME_SUBTITLE_TXT",
			"FONT_INGAME_HEADER_TXT"
		};

		//
		//	Load each font into windows
		//
		int count = ini_file->Entry_Count (FONT_FILE_SECTION);
		int index;
		for (index = 0; index < count; index ++) {
			StringClass	font_filename (0, true);
			ini_file->Get_String (font_filename, FONT_FILE_SECTION, ini_file->Get_Entry (FONT_FILE_SECTION, index));

			//
			//	Install the font into windows
			//
			::AddFontResourceA (font_filename);
			FontFileList.Add (font_filename);
		}

		//
		//	Read information about each font and load it into the system
		//
		for (index = 0; index < FONT_MAX; index ++) {
			
			//
			//	Read information about this font
			//
			StringClass font_entry;
			ini_file->Get_String (font_entry, FONT_NAME_SECTION, FONT_INI_ENTRIES[index]);
			
			//
			//	Parse the information
			//
			StringClass font_name	= ::strtok (font_entry.Peek_Buffer (), ",");
			StringClass font_size	= ::strtok (NULL, ",");
			StringClass font_bold	= ::strtok (NULL, ",");
			bool is_bold				= (::atoi (font_bold) != 0);

			//
			//	Scale the point size to fit this resolution
			//
			float point_size = ((float)::atoi (font_size)) * ScaleY;			
			
			//
			//	Remove bold from "small" fonts if they're scaled down
			//
			point_size = std::max (point_size, 8.0F);
			if (point_size < 10.0F && ScaleY < 1.0F) {
				is_bold = false;
			}

			//
			//	Create the font
			//
			Fonts[index] = WW3DAssetManager::Get_Instance()->Get_FontChars (font_name,
									point_size, is_bold);

		}

		//
		//	Audio section constants
		//
		const char *AUDIO_SECTION		= "Audio";
		const char *AUDIO_CLICK			= "AUDIO_CLICK";
		const char *AUDIO_MOUSEOVER	= "AUDIO_MOUSEOVER";
		const char *AUDIO_BACK			= "AUDIO_BACK";
		const char *AUDIO_POPUP			= "AUDIO_POPUP";

		//
		//	Read information about the sound effects
		//
		ini_file->Get_String (EventAudioList[EVENT_MOUSE_CLICK], AUDIO_SECTION, AUDIO_CLICK);
		ini_file->Get_String (EventAudioList[EVENT_MOUSE_OVER],	AUDIO_SECTION, AUDIO_MOUSEOVER);
		ini_file->Get_String (EventAudioList[EVENT_MENU_BACK],	AUDIO_SECTION, AUDIO_BACK);
		ini_file->Get_String (EventAudioList[EVENT_POPUP],			AUDIO_SECTION, AUDIO_POPUP);

		//
		//	Free the ini file
		//
		delete ini_file;
		ini_file = NULL;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Shutdown
//
////////////////////////////////////////////////////////////////
void
StyleMgrClass::Shutdown (void)
{
	//
	//	Free each font
	//
	int index;
	for (index = 0; index < FONT_MAX; index ++) {
		REF_PTR_RELEASE (Fonts[index]);
	}

	//
	//	Unregister this font with windows
	//
	for (index = 0; index < FontFileList.Count (); index ++) {
		::RemoveFontResourceA (FontFileList[index]);
	}

	//
	//	Free the backdrop texture
	//
	REF_PTR_RELEASE (BackdropTexture);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Font
//
////////////////////////////////////////////////////////////////
FontCharsClass *
StyleMgrClass::Get_Font (FONT_STYLE style)
{
	FontCharsClass *font = Fonts[style];
	if (font != NULL) {
		font->Add_Ref ();
	}
	return font;
}


////////////////////////////////////////////////////////////////
//
//	Render_Backdrop
//
////////////////////////////////////////////////////////////////
void
StyleMgrClass::Render_Backdrop (Render2DClass *renderer, const RectClass &rect)
{
	//
	//	Simply set the texture, and render a quad filling the rectangle
	//
	renderer->Set_Texture (BackdropTexture);
	renderer->Add_Quad (rect, RectClass (0, 0, 1, 1));
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Assign_Font
//
////////////////////////////////////////////////////////////////
void
StyleMgrClass::Assign_Font (Render2DSentenceClass *renderer, FONT_STYLE style)
{
	renderer->Set_Font (Fonts[style]);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render_Text
//
////////////////////////////////////////////////////////////////
/*void
StyleMgrClass::Render_Text
(
	const unichar_t *			text,
	Render2DTextClass *	renderer,
	int						x_pos,
	int						y_pos,
	bool						do_shadow,
	const RectClass *		clipping_rect,
	bool						is_enabled
)
{
	//
	//	If necessary, assign the clipping rectangle
	//
	if (clipping_rect != NULL) {
		renderer->Set_Clipping_Rect (*clipping_rect);
	}

	//
	//	Determine what color to render the text in
	//
	int text_color		= TextColor;
	int shadow_color	= TextShadowColor;
	if (is_enabled == false) {
		text_color		= DisabledTextColor;
		shadow_color	= DisabledTextShadowColor;
	}

	//
	//	Render the shadow (if necessary)
	//
	if (do_shadow) {
		renderer->Set_Location (Vector2 (x_pos - 1, y_pos + 1));
		renderer->Draw_Text (text, shadow_color);
	}

	//
	//	Set the position where text will be drawn
	//
	renderer->Set_Location (Vector2 (x_pos, y_pos));

	//
	//	Draw the text
	//
	renderer->Draw_Text (text, text_color);
	return ;
}*/


////////////////////////////////////////////////////////////////
//
//	Render_Title_Text
//
////////////////////////////////////////////////////////////////
void
StyleMgrClass::Render_Title_Text
(
	const unichar_t *				text,
	Render2DSentenceClass *	renderer,
	const RectClass &			rect
)
{
	//
	//	Set the clipping rectangle
	//
	//renderer->Set_Clipping_Rect (rect);

	//
	//	Get the extents of the text we will be drawing
	//
	Vector2 text_extent = renderer->Get_Text_Extents (text);

	//
	//	Center the text
	//
	int x_pos = int(rect.Left + (rect.Width () / 2) - (text_extent.X / 2));
	int y_pos = int(rect.Top + (rect.Height () / 2) - (text_extent.Y / 2));
	
	//
	//	Build the textures for the text we'll be drawing
	//
	renderer->Build_Sentence (text);

	//
	//	Draw the shadow
	//
	renderer->Set_Location (Vector2 (x_pos - 1, y_pos - 1));
	renderer->Draw_Sentence (TitleShadowColor);

	//
	//	Draw the hilight
	//
	renderer->Set_Location (Vector2 (x_pos + 1, y_pos + 1));
	renderer->Draw_Sentence (TitleHilightColor);

	//
	//	Draw the text
	//
	renderer->Set_Location (Vector2 (x_pos, y_pos));
	renderer->Draw_Sentence (TitleColor);		
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render_Text
//
////////////////////////////////////////////////////////////////
void
StyleMgrClass::Render_Text
(
	const unichar_t *				text,
	Render2DSentenceClass *	renderer,
	const RectClass &			rect,	
	bool							do_shadow,
	bool							do_clip,
	JUSTIFICATION				justify,
	bool							is_enabled,
	bool							is_vcentered
)
{
	//
	//	Determine what color to render the text in
	//
	int text_color		= TextColor;
	int shadow_color	= TextShadowColor;
	if (is_enabled == false) {
		text_color		= DisabledTextColor;
		shadow_color	= DisabledTextShadowColor;
	}

	Render_Text (text, renderer, text_color, shadow_color, rect, do_shadow, do_clip, justify, is_vcentered);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render_Text
//
////////////////////////////////////////////////////////////////
void
StyleMgrClass::Render_Text
(
	const unichar_t *				text,
	Render2DSentenceClass *	renderer,
	uint32						text_color,
	uint32						shadow_color,
	const RectClass &			rect,	
	bool							do_shadow,
	bool							do_clip,
	JUSTIFICATION				justify,
	bool							is_vcentered
)
{
	//
	//	If necessary, assign the clipping rectangle
	//
	if (do_clip) {
		renderer->Set_Clipping_Rect (rect);
	}

	//
	//	Get the extents of the text we will be drawing
	//
	Vector2 text_extent = renderer->Get_Text_Extents (text);

	//
	//	Assume left justification
	//
	int x_pos = rect.Left + 1;
	int y_pos = int(rect.Top + (rect.Height () / 2) - (text_extent.Y / 2));
	if (is_vcentered == false) {
		y_pos = rect.Top;
	}

	//
	//	Handle other justifications
	//
	if (justify == RIGHT_JUSTIFY) {
		
		//
		//	Caclulate right justification
		//
		x_pos = int(rect.Right - text_extent.X);
	} else if (justify == CENTER_JUSTIFY) {

		//
		//	Calculate center justification
		//
		x_pos = int(rect.Left + (rect.Width () / 2) - (text_extent.X / 2));
	}

	//
	//	Construct the textures needed to render the text
	//
	renderer->Build_Sentence (text);

	//
	//	Render the shadow (if necessary)
	//
	if (do_shadow) {
		renderer->Set_Location (Vector2 (x_pos - 1, y_pos + 1));
		renderer->Draw_Sentence (shadow_color);
	}

	//
	//	Render the text
	//
	renderer->Set_Location (Vector2 (x_pos, y_pos));
	renderer->Draw_Sentence (text_color);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render_Text
//
////////////////////////////////////////////////////////////////
/*void
StyleMgrClass::Render_Text
(
	const unichar_t *			text,
	Render2DTextClass *	renderer,
	uint32					text_color,
	uint32					shadow_color,
	const RectClass &		rect,	
	bool						do_shadow,
	bool						do_clip,
	JUSTIFICATION			justify
)
{
	//
	//	If necessary, assign the clipping rectangle
	//
	if (do_clip) {
		renderer->Set_Clipping_Rect (rect);
	}

	//
	//	Get the extents of the text we will be drawing
	//
	Vector2 text_extent = renderer->Get_Text_Extents (text);

	//
	//	Assume left justification
	//
	int x_pos = rect.Left + 1;
	int y_pos = int(rect.Top + (rect.Height () / 2) - (text_extent.Y / 2));

	//
	//	Handle other justifications
	//
	if (justify == RIGHT_JUSTIFY) {
		
		//
		//	Caclulate right justification
		//
		x_pos = int(rect.Right - text_extent.X);
	} else if (justify == CENTER_JUSTIFY) {

		//
		//	Calculate center justification
		//
		x_pos = int(rect.Left + (rect.Width () / 2) - (text_extent.X / 2));
	}

	//
	//	Render the shadow (if necessary)
	//
	if (do_shadow) {
		renderer->Set_Location (Vector2 (x_pos - 1, y_pos + 1));
		renderer->Draw_Text (text, shadow_color);
	}

	//
	//	Render the text
	//
	renderer->Set_Location (Vector2 (x_pos, y_pos));
	renderer->Draw_Text (text, text_color);
	return ;
}*/



////////////////////////////////////////////////////////////////
//
//	Render_Text
//
////////////////////////////////////////////////////////////////
/*void
StyleMgrClass::Render_Text
(
	const unichar_t *				text,
	Render2DSentenceClass *	renderer,
	uint32						text_color,
	uint32						shadow_color,
	const RectClass &			rect,	
	bool							do_shadow,
	bool							do_clip,
	JUSTIFICATION				justify
)
{
	//
	//	If necessary, assign the clipping rectangle
	//
	if (do_clip) {
		renderer->Set_Clipping_Rect (rect);
	}

	//
	//	Get the extents of the text we will be drawing
	//
	Vector2 text_extent = renderer->Get_Text_Extents (text);

	//
	//	Assume left justification
	//
	int x_pos = rect.Left + 1;
	int y_pos = int(rect.Top + (rect.Height () / 2) - (text_extent.Y / 2));

	//
	//	Handle other justifications
	//
	if (justify == RIGHT_JUSTIFY) {
		
		//
		//	Caclulate right justification
		//
		x_pos = int(rect.Right - text_extent.X);
	} else if (justify == CENTER_JUSTIFY) {

		//
		//	Calculate center justification
		//
		x_pos = int(rect.Left + (rect.Width () / 2) - (text_extent.X / 2));
	}

	//
	//	Build up the text we want to render
	//
	renderer->Build_Sentence (text);

	//
	//	Render the shadow (if necessary)
	//
	if (do_shadow) {
		renderer->Set_Location (Vector2 (x_pos - 1, y_pos + 1));
		renderer->Draw_Sentence (shadow_color);
	}

	//
	//	Render the text
	//
	renderer->Set_Location (Vector2 (x_pos, y_pos));
	renderer->Draw_Sentence (text_color);
	return ;
}*/

////////////////////////////////////////////////////////////////
//
//	Render_Wrapped_Text
//
////////////////////////////////////////////////////////////////
void
StyleMgrClass::Render_Wrapped_Text
(
	const unichar_t *				text,
	Render2DSentenceClass *	renderer,
	const RectClass &			rect,
	bool							do_shadow,
	bool							do_vcenter,
	bool							is_enabled	
)
{
	//
	//	Determine what color to render the text in
	//
	int text_color		= TextColor;
	int shadow_color	= TextShadowColor;
	if (is_enabled == false) {
		text_color		= DisabledTextColor;
		shadow_color	= DisabledTextShadowColor;
	}

	//
	//	Render the text
	//
	Render_Wrapped_Text (text, renderer, text_color, shadow_color, rect, do_shadow, do_vcenter);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render_Wrapped_Text
//
////////////////////////////////////////////////////////////////
void
StyleMgrClass::Render_Wrapped_Text_Ex
(
	const unichar_t *				text,
	Render2DSentenceClass *	renderer,
	const RectClass &			rect,
	bool							do_shadow,
	bool							do_vcenter,
	bool							is_enabled,
	JUSTIFICATION				justify
)
{
	//
	//	Determine what color to render the text in
	//
	int text_color		= TextColor;
	int shadow_color	= TextShadowColor;
	if (is_enabled == false) {
		text_color		= DisabledTextColor;
		shadow_color	= DisabledTextShadowColor;
	}

	//
	//	Render the text
	//
	Render_Wrapped_Text_Ex (text, renderer, text_color, shadow_color, rect,
		do_shadow, do_vcenter, justify);
	return ;
}



////////////////////////////////////////////////////////////////
//
//	Render_Wrapped_Text
//
////////////////////////////////////////////////////////////////
void
StyleMgrClass::Render_Wrapped_Text_Ex
(
	const unichar_t *				text,
	Render2DSentenceClass *	renderer,
	uint32						text_color,
	uint32						shadow_color, 
	const RectClass &			rect,
	bool							do_shadow,
	bool							do_vcenter,
	JUSTIFICATION				justify
)
{
	//
	//	Enable wrapping
	//
	renderer->Set_Wrapping_Width (rect.Width ());
	renderer->Set_Clipping_Rect (rect);

	//
	//	First, get the number of lines of text
	//
	int row_count = 0;
	renderer->Get_Formatted_Text_Extents (text, &row_count);

	//
	//	Calculate where to start spitting out the text
	//
	float text_height		= renderer->Peek_Font ()->Get_Char_Height ();
	RectClass curr_rect	= rect;
	if (do_vcenter) {
		curr_rect.Top = int(rect.Center ().Y - ((row_count * text_height) * 0.5F));
	}

	//
	//	Handy macro
	//
	#define COPY_LINE(dest,	src_start, src_end)	\
			if (src_end == NULL) {						\
				dest = src_start;							\
			} else {											\
				size_t bytes	= ((char *)src_end - (char *)src_start);	\
				size_t len		= bytes / sizeof (unichar_t);						\
				WWASSERT(len + 1 <= static_cast<size_t>(std::numeric_limits<int>::max())); \
			::memcpy (dest.Get_Buffer (static_cast<int>(len + 1)), src_start, bytes);	\
				dest.Peek_Buffer ()[len] = 0;										\
			}

	//
	//	Loop over all the lines of text and check for wrapping...
	//
	const unichar_t *line_start = renderer->Find_Row_Start (text, 0);
	while (line_start != NULL) {

		//
		//	Lookup the start of the next line...
		//
		const unichar_t *line_end = renderer->Find_Row_Start (line_start, 1);
		
		//
		//	Copy this line of text into the control
		//
		WideStringClass curr_text;
		COPY_LINE (curr_text, line_start, line_end);

		//
		//	Render this row of text
		//
		Render_Text (curr_text, renderer, text_color, shadow_color, curr_rect,
			do_shadow, false, justify, false);

		//
		//	Advance to the next line...
		//
		line_start = line_end;
		curr_rect.Top	+= text_height;
	}


	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render_Wrapped_Text
//
////////////////////////////////////////////////////////////////
void
StyleMgrClass::Render_Wrapped_Text
(
	const unichar_t *				text,
	Render2DSentenceClass *	renderer,
	uint32						text_color,
	uint32						shadow_color, 
	const RectClass &			rect,
	bool							do_shadow,
	bool							do_vcenter
)
{
	//
	//	Enable wrapping
	//
	renderer->Set_Wrapping_Width (rect.Width ());
	renderer->Set_Clipping_Rect (rect);

	//
	//	Assume left justification
	//
	int x_pos = rect.Left + 1;
	int y_pos = rect.Top + 1;

	//
	//	Center the text vertically if necessary
	//
	if (do_vcenter) {
		Vector2 extents = renderer->Get_Formatted_Text_Extents (text);
		y_pos = rect.Top + (rect.Height () / 2.0F) - (extents.Y / 2.0F);
	}

	//
	//	Build the textures for the text we'll be drawing
	//
	renderer->Build_Sentence (text);

	//
	//	Render the shadow (if necessary)
	//
	if (do_shadow) {
		renderer->Set_Location (Vector2 (x_pos - 1, y_pos + 1));
		renderer->Draw_Sentence (shadow_color);
	}

	//
	//	Render the text
	//
	renderer->Set_Location (Vector2 (x_pos, y_pos));
	renderer->Draw_Sentence (text_color);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Configure_Hilighter
//
////////////////////////////////////////////////////////////////
void
StyleMgrClass::Configure_Hilighter (Render2DClass *renderer)
{
	renderer->Enable_Alpha (false);
	renderer->Enable_Texturing (false);
	
	//
	//	Setup an additive shader
	//
	ShaderClass *shader = renderer->Get_Shader ();
	shader->Set_Dst_Blend_Func (ShaderClass::DSTBLEND_ONE);
	shader->Set_Src_Blend_Func (ShaderClass::SRCBLEND_ONE);	
	//shader->Set_Primary_Gradient (ShaderClass::GRADIENT_ADD);
	//shader->Set_Secondary_Gradient (ShaderClass::SECONDARY_GRADIENT_DISABLE);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render_Hilight
//
////////////////////////////////////////////////////////////////
void
StyleMgrClass::Render_Hilight (Render2DClass *renderer, const RectClass &rect)
{
	renderer->Add_Quad (rect, HilightColor);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render_Glow
//
////////////////////////////////////////////////////////////////
void
StyleMgrClass::Render_Glow
(
	const unichar_t *				text,
	Render2DSentenceClass *	renderer,
	const RectClass &			rect,
	int							radius_x,
	int							radius_y,
	int							color,
	JUSTIFICATION				justify
)
{
	//
	//	Get the extents of the text we will be drawing
	//
	Vector2 text_extent = renderer->Get_Text_Extents (text);

	//
	//	Assume centered text
	//
	int x_pos = int(rect.Left + (rect.Width () / 2) - (text_extent.X / 2));
	int y_pos = int(rect.Top + (rect.Height () / 2) - (text_extent.Y / 2));

	if (justify == LEFT_JUSTIFY) {
		
		//
		//	Caclulate left justification
		//
		x_pos = int(rect.Left + 1);
	} else if (justify == RIGHT_JUSTIFY) {
		
		//
		//	Caclulate right justification
		//
		x_pos = int(rect.Right - text_extent.X);
	}

	//
	//	Setup an additive shader
	//
	renderer->Make_Additive ();
	
	//
	//	Figure out how many passes we should do to get the
	// desired result
	//
	int pass_count		= 4;
	//pass_count			= std::min (pass_count, 5);
	//pass_count			= std::max (pass_count, 3);

	int step_count		= 7;
	//step_count			= std::min (step_count, 10);
	//step_count			= std::max (step_count, 4);
	float angle_inc	= DEG_TO_RADF (360) / step_count;

	float x_inc = radius_x / pass_count;
	float y_inc = radius_y / pass_count;

	float curr_radiusx = 2.0F;
	float curr_radiusy = 2.0F;

	//
	//	Do four passes to get from the inner radius to the outer radius
	//
	for (int pass_index = 0; pass_index < pass_count; pass_index ++) {
		
		//
		//	Circle the characters around using the given radius
		//
		float angle = 0;
		for (int index = 0; index < step_count; index ++) {
			
			float new_x_pos = float(x_pos + (WWMath::Cos(angle) * curr_radiusx));
			float new_y_pos = float(y_pos + (WWMath::Sin(angle) * curr_radiusy));

			angle += angle_inc;

			//
			//	Render the text
			//
			renderer->Set_Location (Vector2 (new_x_pos, new_y_pos));
			renderer->Draw_Sentence (color);
		}

		curr_radiusx += x_inc;
		curr_radiusy += y_inc;
	}
	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Configure_Renderer
//
////////////////////////////////////////////////////////////////
void
StyleMgrClass::Configure_Renderer (Render2DClass *renderer)
{
	//
	//	Set the coordinate range for the renderer
	//
	renderer->Set_Coordinate_Range (Render2DClass::Get_Screen_Resolution ());

	//
	//	Turn depth-buffer reading on for this renderer
	//
	//ShaderClass *shader = renderer->Get_Shader ();
	//shader->Set_Depth_Compare (ShaderClass::PASS_LEQUAL);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Play_Sound
//
////////////////////////////////////////////////////////////////
void
StyleMgrClass::Play_Sound (EVENT_AUDIO event)
{
	if (	WWAudioClass::Get_Instance () == NULL ||
			EventAudioList[event].Get_Length () == 0)
	{
		return ;
	}

	//
	//	Parse the information
	//
	StringClass tmp_copy (EventAudioList[event], true);
	StringClass filename		= ::strtok (tmp_copy.Peek_Buffer (), ",");
	StringClass vol_string	= ::strtok (NULL, ",");
	float volume				= ::atoi (vol_string) / 100.0F;

	//
	//	Play the sound effect
	//
	WWAudioClass::Get_Instance ()->Simple_Play_2D_Sound_Effect (filename, 1.0F, volume);
	return ;
}
