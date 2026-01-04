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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/render2dsentence.cpp                   $*
 *                                                                                             *
 *                       $Author:: Steve_t                  $*
 *                                                                                             *
 *								$Modtime:: 8/26/02 3:18p                                              $*
 *                                                                                             *
 *                    $Revision:: 27                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "render2dsentence.h"
#include "surfaceclass.h"
#include "texture.h"
#include "wwprofile.h"
#include "wwmemlog.h"
#include "dx8wrapper.h"
#include <algorithm>


////////////////////////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////////////////////////
const int CHAR_TEXTURE_SIZE	= 256;
const int CHAR_BUFFER_LEN		= 32768;


// Macros.
// NOTE 0: Word wrap logic does not apply to Han characters (Chinese, Japanese & Korean).
//			  Therefore treat each of these characters as a word which can be preceeded by a line break.
// NOTE 1: This is a simplification. Some Korean characters should not be line break characters.
#define IS_BREAK_CHAR(ch) ((ch == U_CHAR(' ')) || ((ch >= 0x3000) && (ch <= 0xdfff)))


////////////////////////////////////////////////////////////////////////////////////
//
//	Render2DSentenceClass
//
////////////////////////////////////////////////////////////////////////////////////
Render2DSentenceClass::Render2DSentenceClass (void) :
	Font (NULL),
	Location (0.0F,0.0F),
	Cursor (0.0F,0.0F),
	TextureOffset (0, 0),
	TextureStartX (0),
	CurSurface (NULL),
	CurrTextureSize (0),
	MonoSpaced (false),
	IsClippedEnabled (false),
	ClipRect (0, 0, 0, 0),
	BaseLocation (0, 0),
	LockedPtr (NULL),
	LockedStride (0),
	TextureSizeHint (0),
	WrapWidth (0),
	TabStop (5.0),
	DrawExtents (0, 0, 0, 0),
	Renderers(sizeof(PreAllocatedRenderers)/sizeof(RendererDataStruct),PreAllocatedRenderers)
{
	Shader = Render2DClass::Get_Default_Shader ();
	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	~Render2DSentenceClass
//
////////////////////////////////////////////////////////////////////////////////////
Render2DSentenceClass::~Render2DSentenceClass (void)
{
	REF_PTR_RELEASE (Font);
	Reset ();
	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Font
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Set_Font (FontCharsClass *font)
{
	Reset ();
	REF_PTR_SET (Font, font);
	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Reset_Polys
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Reset_Polys (void)
{
	for (int index = 0; index < Renderers.Count (); index ++) {
		Renderers[index].Renderer->Reset ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Reset
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Reset (void)
{
	//
	//	Make sure we unlock the current surface (if necessary)
	//
	if (LockedPtr != NULL) {
		CurSurface->Unlock ();
		LockedPtr = NULL;
	}

	//
	//	Release our hold on the current surface
	//
	REF_PTR_RELEASE (CurSurface);

	//
	//	Free each renderer
	//
	for (int i=0;i<Renderers.Count();++i) {
		delete Renderers[i].Renderer;
	}
	Renderers.Reset_Active();

	Cursor.Set (0, 0);
	MonoSpaced = false;

	Release_Pending_Surfaces ();
	Reset_Sentence_Data ();
	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Make_Additive
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Make_Additive (void)
{
	Shader.Set_Dst_Blend_Func (ShaderClass::DSTBLEND_ONE);
	Shader.Set_Src_Blend_Func (ShaderClass::SRCBLEND_ONE);
	Shader.Set_Primary_Gradient (ShaderClass::GRADIENT_MODULATE);
	Shader.Set_Secondary_Gradient (ShaderClass::SECONDARY_GRADIENT_DISABLE);

	Set_Shader (Shader);
	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Make_Additive
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Set_Shader (ShaderClass shader)
{
	Shader = shader;

	//
	//	Change each renderer's shader
	//
	for (int i = 0; i < Renderers.Count (); i ++) {
		ShaderClass *curr_shader = Renderers[i].Renderer->Get_Shader ();
		(*curr_shader) = Shader;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Render (void)
{
	if (DX8Wrapper::Is_Device_Lost() || !DX8Wrapper::Is_Initted()) return;
	//
	//	Build any textures that are pending
	//
	Build_Textures ();

	//
	//	Ask each renderer to draw its contents
	//
	for (int i = 0; i < Renderers.Count (); i ++) {
		Renderers[i].Renderer->Render ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Base_Location
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Set_Base_Location (const Vector2 &loc)
{
	Vector2 dif		= loc - BaseLocation;
	BaseLocation	= loc;
	for (int i = 0; i < Renderers.Count (); i ++) {
		Renderers[i].Renderer->Move (dif);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Location
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Set_Location (const Vector2 &loc)
{
	Location	= loc;
	return ;
}


void
Render2DSentenceClass::Set_Tabstop(float stop)
{
	if (stop > 0.0) {
		TabStop = stop;
	} else {
		TabStop = 1.0;
	}
}

////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Text_Extents
//
////////////////////////////////////////////////////////////////////////////////////
Vector2
Render2DSentenceClass::Get_Text_Extents (const unichar_t *text)
{
	if (!DX8Wrapper::Is_Initted()) {
		Vector2 temp(0,0);
		return(temp);
	}

	Vector2 extent (0, Font->Get_Char_Height());

	while (*text) {
		unichar_t ch = *text++;

		if ( ch != (unichar_t)'\n' ) {
			extent.X += Font->Get_Char_Spacing( ch );
		}
	}

	return extent;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Find_Row_Start
//
////////////////////////////////////////////////////////////////////////////////////
const unichar_t *
Render2DSentenceClass::Find_Row_Start( const unichar_t * text, int row_index )
{
	if (row_index == 0) {
		return text;
	}

	if (!DX8Wrapper::Is_Initted()) {
		return text;
	}

	const unichar_t *retval = NULL;

	float max_x_pos	= 0;
	float x_pos			= 0;
	float y_pos			= Font->Get_Char_Height ();

	int row_counter = 0;

	while (*text) {
		unichar_t ch = *text++;

		bool is_wrapped = false;

		//
		// Check to see if we need to wrap on this word-break
		//
		if (IS_BREAK_CHAR (ch) && WrapWidth > 0) {

			//
			//	Find the width of the next word
			//
			const unichar_t *word	= text;
			float word_width = Font->Get_Char_Spacing (ch);
			while ((*word != 0) && ((*word > U_CHAR(' ')) && !IS_BREAK_CHAR (*word))) {
				word_width += Font->Get_Char_Spacing (*word++);
			}

			//
			//	Did the word extend past the wrap width?
			//
			if ((x_pos + word_width) >= WrapWidth) {
				is_wrapped = true;
			}

		} else if (ch == U_CHAR('\n')) {
			is_wrapped = true;
		}

		//
		//	Handle line wrapping
		//
		if (is_wrapped) {
			max_x_pos = std::max (max_x_pos, x_pos);
			x_pos = 0;
			y_pos += Font->Get_Char_Height ();

			//
			//	Is this the line we're looking for?
			//
			row_counter ++;
			if (row_counter == row_index) {
				retval = (ch == U_CHAR(' ') || ch == U_CHAR('\n')) ? text : text - 1;
				break;
			}
		}

		if (ch != (unichar_t)'\n') {
			x_pos += Font->Get_Char_Spacing (ch);
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Formatted_Text_Extents
//
////////////////////////////////////////////////////////////////////////////////////
Vector2
Render2DSentenceClass::Get_Formatted_Text_Extents (const unichar_t *text, int *row_count)
{
	if (!DX8Wrapper::Is_Initted()) {
		Vector2 temp(0,0);
		return(temp);
	}
	
	float max_x_pos	= 0;
	float x_pos			= 0;
	float y_pos			= Font->Get_Char_Height ();

	int row_counter = 0;

	while (*text) {
		unichar_t ch = *text++;

		bool is_wrapped = false;

		//
		// Check to see if we need to wrap on this word-break
		//
		if (IS_BREAK_CHAR (ch) && WrapWidth > 0) {

			//
			//	Find the width of the next word
			//
			const unichar_t *word	= text;
			float word_width = Font->Get_Char_Spacing (ch);
			while ((*word != 0) && ((*word > U_CHAR(' ')) && !IS_BREAK_CHAR (*word))) {
				word_width += Font->Get_Char_Spacing (*word++);
			}

			//
			//	Did the word extend past the wrap width?
			//
			if ((x_pos + word_width) >= WrapWidth) {
				is_wrapped = true;
			}

		} else if (ch == U_CHAR('\n')) {
			is_wrapped = true;
		}

		//
		//	Handle line wrapping
		//
		if (is_wrapped) {
			max_x_pos = std::max (max_x_pos, x_pos);
			x_pos = 0;
			y_pos += Font->Get_Char_Height ();
			row_counter ++;
		}

		if (ch != (unichar_t)'\n') {
			x_pos += Font->Get_Char_Spacing (ch);
		}
	}

	//
	//	Build a Vector2 out of our extents
	//
	Vector2 extent;
	extent.X = std::max (max_x_pos, x_pos);
	extent.Y = y_pos;

	//
	//	Return the row count to the caller (if necessary)
	//
	if (row_count != NULL) {
		(*row_count) = row_counter + 1;
	}

	return extent;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Reset_Sentence_Data
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Reset_Sentence_Data (void)
{
	//
	//	Release our hold on each texture used in the sentence
	//
	for (int index = 0; index < SentenceData.Count (); index ++) {
		REF_PTR_RELEASE (SentenceData[index].Surface);
	}

	SentenceData.Reset_Active();
	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Release_Pending_Surfaces
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Release_Pending_Surfaces (void)
{
	//
	//	Release our hold on each pending surface
	//
	for (int index = 0; index < PendingSurfaces.Count (); index ++) {
		SurfaceClass *curr_surface = PendingSurfaces[index].Surface;
		REF_PTR_RELEASE (curr_surface);
	}

	PendingSurfaces.Reset_Active();
	return;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Build_Textures
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Build_Textures (void)
{
	WWMEMLOG(MEM_TEXTURE);

	//
	//	Make sure we unlock the current surface
	//
	if (LockedPtr != NULL) {
		CurSurface->Unlock ();
		LockedPtr = NULL;
	}

	//
	//	Release our hold on the current surface
	//
	REF_PTR_RELEASE (CurSurface);
	TextureOffset.Set (0, 0);
	TextureStartX = 0;

	//
	//	Convert all pending surfaces to textures
	//
	for (int index = 0; index < PendingSurfaces.Count (); index ++) {
		PendingSurfaceStruct &surface_info = PendingSurfaces[index];
		SurfaceClass *curr_surface = surface_info.Surface;

		//
		//	Get the dimensions of the surface
		//
		SurfaceClass::SurfaceDescription desc;
		curr_surface->Get_Description (desc);

		//
		//	Create the new texture
		//
		TextureClass *new_texture = new TextureClass (desc.Width, desc.Width, WW3D_FORMAT_A4R4G4B4, TextureClass::MIP_LEVELS_1,TextureClass::POOL_DEFAULT);
		SurfaceClass *texture_surface = new_texture->Get_Surface_Level ();

		//
		//	Copy the contents of the texture from the surface
		//
		DX8Wrapper::_Copy_DX8_Rects (curr_surface->Peek_D3D_Surface (), NULL, 0, texture_surface->Peek_D3D_Surface (), NULL);
		REF_PTR_RELEASE (texture_surface);

		//
		//	Assign this texture to any renderers that need it
		//
		for (int renderer_index = 0; renderer_index < surface_info.Renderers.Count (); renderer_index ++) {
			Render2DClass *renderer = surface_info.Renderers[renderer_index];
			renderer->Set_Texture (new_texture);
		}

		//
		//	Release our hold on the objects
		//
		REF_PTR_RELEASE (new_texture);
		REF_PTR_RELEASE (curr_surface);
	}

	//
	//	Reset the list
	//
	PendingSurfaces.Reset_Active();
	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Draw_Sentence
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Draw_Sentence (uint32 color)
{
	Render2DClass *curr_renderer	= NULL;
	SurfaceClass *curr_surface		= NULL;

	DrawExtents.Set (0, 0, 0, 0);

	//
	//	Loop over all the parts of the sentence
	//
	for (int index = 0; index < SentenceData.Count (); index ++) {
		SentenceDataStruct &data = SentenceData[index];

		//
		//	Has the surface changed?
		//
		if (data.Surface != curr_surface) {
			curr_surface = data.Surface;

			//
			//	Try to find a renderer that uses the same "texture"
			//
			bool found = false;
			for (int renderer_index = 0; renderer_index < Renderers.Count (); renderer_index ++) {
				if (Renderers[renderer_index].Surface == curr_surface) {
					found = true;
					curr_renderer = Renderers[renderer_index].Renderer;
					break;
				}
			}

			//
			//	Create a new renderer if we couldn't find an appropriate one
			//
			if (found == false) {

				//
				//	Allocate a new renderer
				//
				curr_renderer = new Render2DClass;
				curr_renderer->Set_Coordinate_Range (Render2DClass::Get_Screen_Resolution ());
				ShaderClass *curr_shader = curr_renderer->Get_Shader ();
				(*curr_shader) = Shader;

				//
				//	Add it to our list
				//
				RendererDataStruct render_info;
				render_info.Renderer	= curr_renderer;
				render_info.Surface	= curr_surface;
				Renderers.Add (render_info);

				//
				//	Now, add this renderer to the surface pending list
				//
				for (int surface_index = 0; surface_index < PendingSurfaces.Count (); surface_index ++) {
					PendingSurfaceStruct &surface_info = PendingSurfaces[surface_index];
					if (surface_info.Surface == curr_surface) {
						surface_info.Renderers.Add (curr_renderer);
					}
				}
			}
		}

		//
		//	Get the dimensions of the surface
		//
		SurfaceClass::SurfaceDescription desc;
		curr_surface->Get_Description (desc);

		//
		//	Add a quad that contains this sentence chunk
		//
		RectClass screen_rect	= data.ScreenRect;
		screen_rect					+= Location;
		RectClass uv_rect			= data.UVRect;

		//
		//	Clip the quad (as necessary)
		//
		bool add_quad = true;
		if (IsClippedEnabled) {

			//
			//	Check for completely clipped
			//
			if (	screen_rect.Right <= ClipRect.Left ||
					screen_rect.Bottom <= ClipRect.Top)
			{
				add_quad = false;
			/*} else if (	screen_rect.Top < ClipRect.Top ||
							screen_rect.Bottom > ClipRect.Bottom)
			{
				add_quad = false;*/
			} else {

				//
				//	Clip the polygons to the specified area
				//
				RectClass clipped_rect;
				clipped_rect.Left		= std::max (screen_rect.Left, ClipRect.Left);
				clipped_rect.Right	= std::min (screen_rect.Right, ClipRect.Right);
				clipped_rect.Top		= std::max (screen_rect.Top, ClipRect.Top);
				clipped_rect.Bottom	= std::min (screen_rect.Bottom, ClipRect.Bottom);

				//
				//	Clip the texture to the specified area
				//
				RectClass clipped_uv_rect;
				float percent				= ((clipped_rect.Left - screen_rect.Left) / screen_rect.Width ());
				clipped_uv_rect.Left		= uv_rect.Left + (uv_rect.Width () * percent);

				percent						= ((clipped_rect.Right - screen_rect.Left) / screen_rect.Width ());
				clipped_uv_rect.Right	= uv_rect.Left + (uv_rect.Width () * percent);

				percent						= ((clipped_rect.Top - screen_rect.Top) / screen_rect.Height ());
				clipped_uv_rect.Top		= uv_rect.Top + (uv_rect.Height () * percent);

				percent						= ((clipped_rect.Bottom - screen_rect.Top) / screen_rect.Height ());
				clipped_uv_rect.Bottom	= uv_rect.Top + (uv_rect.Height () * percent);

				//
				//	Use the clipped rectangles to render
				//
				screen_rect = clipped_rect;
				uv_rect		= clipped_uv_rect;
			}
		}

		if (add_quad) {
			uv_rect *=  1.0F / ((float)desc.Width);
			curr_renderer->Add_Quad (screen_rect, uv_rect, color);

			//
			//	Add this rectangle to the total draw extents
			//
			if (DrawExtents.Width () == 0) {
				DrawExtents = screen_rect;
			} else {
				DrawExtents += screen_rect;
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Record_Sentence_Chunk
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Record_Sentence_Chunk (void)
{
	//
	//	Do we have anything to store?
	//
	int width = TextureOffset.I - TextureStartX;
	if (width > 0) {
		float char_height = Font->Get_Char_Height ();

		//
		//	Build a structure that contains enough information
		// to hold this portion of the sentence
		//
		SentenceDataStruct sentence_data;
		sentence_data.Surface = CurSurface;
		sentence_data.Surface->Add_Ref ();
		sentence_data.ScreenRect.Left		= Cursor.X;
		sentence_data.ScreenRect.Right	= Cursor.X + width;
		sentence_data.ScreenRect.Top		= Cursor.Y;
		sentence_data.ScreenRect.Bottom	= Cursor.Y + char_height;
		sentence_data.UVRect.Left			= TextureStartX;
		sentence_data.UVRect.Top			= TextureOffset.J;
		sentence_data.UVRect.Right			= TextureOffset.I;
		sentence_data.UVRect.Bottom		= TextureOffset.J + char_height;

		//
		//	Add this information to our list
		//
		SentenceData.Add (sentence_data);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Allocate_New_Surface
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Allocate_New_Surface (const unichar_t *text)
{
	//
	//	Unlock the last surface (if necessary)
	//
	if (LockedPtr != NULL) {
		CurSurface->Unlock ();
		LockedPtr = NULL;
	}

	//
	// Calculate the width of the text
	//
	int text_width = 0;
	for (int index = 0; text[index] != 0; index ++) {
		text_width += Font->Get_Char_Spacing (text[index]);
	}

	int char_height = Font->Get_Char_Height ();

	//
	//	Find the best texture size for the remaining text
	//
	CurrTextureSize = 256;
	int best_tex_mem_usage = 999999999;
	for (int pow2 = 6; pow2 <= 8; pow2 ++) {

		int size					= 1 << pow2;
		int row_count			= (text_width / size) + 1;
		int rows_per_texture	= size / (char_height + 1);

		//
		//	Can we even fit one character on this texture?
		//
		if (rows_per_texture > 0) {

			//
			//	How many textures (at this size) would it take to render
			// the remaining text?
			//
			int texture_count	= row_count / rows_per_texture;
			texture_count		= std::max (texture_count, 1);

			//
			//	Is this the best usage of texture memory we've found yet?
			//
			int texture_mem_usage = (texture_count * size * size);
			if (texture_mem_usage < best_tex_mem_usage) {
				CurrTextureSize		= size;
				best_tex_mem_usage	= texture_mem_usage;
			}
		}
	}

	//
	//	Use whichever is larger, the hint or the calculated size
	//
	CurrTextureSize = std::max (TextureSizeHint, CurrTextureSize);

	//
	//	Release our extra hold on the old surface
	//
	REF_PTR_RELEASE (CurSurface);

	//
	//	Create the new surface
	//
	CurSurface = NEW_REF (SurfaceClass, (CurrTextureSize, CurrTextureSize, WW3D_FORMAT_A4R4G4B4, SurfaceClass::POOL_SYSTEMMEM));
	WWASSERT (CurSurface != NULL);
	CurSurface->Add_Ref ();

	//
	//	Add this surface to our list
	//
	PendingSurfaceStruct surface_info;
	surface_info.Surface = CurSurface;
	PendingSurfaces.Add (surface_info);

	//
	//	Reset to the upper left corner
	//
	TextureOffset.Set (0, 0);
	TextureStartX = 0;
	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Build_Sentence
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Build_Sentence (const unichar_t *text)
{
	if (text == NULL) {
		return ;
	}

	if (!DX8Wrapper::Is_Initted()) {
		return;
	}

	//
	//	Start fresh
	//
	Reset_Sentence_Data ();
	Cursor.Set (0, 0);

	//
	//	Ensure we have a surface to start with
	//
	if (CurSurface == NULL) {
		Allocate_New_Surface (text);
	}

	float char_height = Font->Get_Char_Height ();

	//
	//	Loop over all the characters in the string
	//
	while (text != NULL) {
		unichar_t ch = *text++;

		//
		//	Determine how much horizontal space this character requires
		//
		float char_spacing = Font->Get_Char_Spacing (ch);

		bool exceeded_texture_width	= ((TextureOffset.I + char_spacing) >= CurrTextureSize);
		bool encountered_break_char	= (IS_BREAK_CHAR (ch) || ch == U_CHAR('\n') || ch == 0 || ch == U_CHAR('\t'));

		//
		//	Do we need to record this portion of the sentence to its own chunk?
		//
		if (exceeded_texture_width || encountered_break_char) {
			Record_Sentence_Chunk ();

			//
			//	Adjust the positions
			//
			Cursor.X			+= (TextureOffset.I - TextureStartX);
			TextureStartX	= TextureOffset.I;

			//
			//	Adjust the output coordinates
			//
			if (IS_BREAK_CHAR (ch)) {

				if (ch == U_CHAR(' ')) {
					Cursor.X += char_spacing;
				}

				//
				// Check to see if we need to wrap on this word-break
				//
				if (WrapWidth > 0) {

					//
					//	Find the length of the next word
					//
					const unichar_t *word	= text;
					float word_width	= (ch == U_CHAR(' ')) ? 0 : char_spacing;
					while ((*word != 0) && ((*word > U_CHAR(' ')) && !IS_BREAK_CHAR (*word))) {
						word_width += Font->Get_Char_Spacing (*word++);
					}

					//
					//	Should we wrap the next word?
					//
					if ((Cursor.X + word_width) >= WrapWidth) {
						Cursor.X = 0;
						Cursor.Y += char_height;
					}
				}

			} else if (ch == U_CHAR('\n')) {
				Cursor.X = 0;
				Cursor.Y += char_height;
			} else if (ch == 0) {
				break;
			} else if (ch == U_CHAR('\t')) {
				float tab_spacing = (char_spacing * TabStop);
				float tab_pos = (floor(Cursor.X / tab_spacing) * tab_spacing);
				Cursor.X = (tab_pos + tab_spacing);
			}

			//
			//	Did the text extend past the edge of the texture?
			//
			if (exceeded_texture_width) {
				TextureStartX		= 0;
				TextureOffset.I	= TextureStartX;
				TextureOffset.J	+= char_height;

				//
				//	Did the text extent completely off the texture?
				//
				if ((TextureOffset.J + char_height) >= CurrTextureSize) {
					Allocate_New_Surface (text);
				}
			}
		}

		if (ch != U_CHAR('\n') && ch != U_CHAR(' ') && ch != U_CHAR('\t')) {

			//
			//	Ensure the surface is locked
			//
			if (LockedPtr == NULL) {
				LockedPtr = (uint16 *)CurSurface->Lock (&LockedStride);
				WWASSERT (LockedPtr != NULL);
			}

			//
			//	Check to ensure the text will fit on this texture
			//
			WWASSERT (((TextureOffset.I + char_spacing) < CurrTextureSize) && ((TextureOffset.J + char_height) < CurrTextureSize));

			//
			//	Blit the character to the surface
			//
			Font->Blit_Char (ch, LockedPtr, LockedStride, TextureOffset.I, TextureOffset.J);
			TextureOffset.I += char_spacing;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void	Render2DSentenceClass::Force_Alpha( float alpha )
{
	for (int i = 0; i < Renderers.Count (); i ++) {
		Renderers[i].Renderer->Force_Alpha( alpha );
	}
}



////////////////////////////////////////////////////////////////////////////////////
//
//	FontCharsClass
//
////////////////////////////////////////////////////////////////////////////////////
FontCharsClass::FontCharsClass (void) :
	OldGDIFont(	NULL ),
	OldGDIBitmap( NULL ),
	GDIFont( NULL ),
	GDIBitmap( NULL ),
	GDIBitmapBits ( NULL ),
	MemDC( NULL ),
	CurrPixelOffset( 0 ),
	PointSize( 0 ),
	CharHeight( 0 ),
	UnicodeCharArray( NULL ),
	FirstUnicodeChar( 0xFFFF ),
	LastUnicodeChar( 0 ),
	IsBold (false),
	BufferList(sizeof(PreAllocatedBufferList)/sizeof(uint16*),PreAllocatedBufferList)
{
	::memset( ASCIICharArray, 0, sizeof (ASCIICharArray) );
	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	~FontCharsClass
//
////////////////////////////////////////////////////////////////////////////////////
FontCharsClass::~FontCharsClass (void)
{
	for (int i=0;i<BufferList.Count(); ++i) {
		delete [] BufferList[i];
	}
	BufferList.Reset_Active();

	Free_GDI_Font();
	Free_Character_Arrays();
	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Char_Data
//
////////////////////////////////////////////////////////////////////////////////////
const FontCharsClass::CharDataStruct *
FontCharsClass::Get_Char_Data (unichar_t ch)
{
	const CharDataStruct *retval = NULL;

	if ( ch < 256 ) {
		retval = ASCIICharArray[ch];
	} else {
		Grow_Unicode_Array( ch );
		retval = UnicodeCharArray[ch - FirstUnicodeChar];
	}

	//
	//	If the character wasn't found, then add it to our list
	//
	if ( retval == NULL ) {
		retval = Store_GDI_Char( ch );
	}

	WWASSERT( retval->Value == ch );
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Char_Width
//
////////////////////////////////////////////////////////////////////////////////////
int
FontCharsClass::Get_Char_Width (unichar_t ch)
{
	const CharDataStruct	* data = Get_Char_Data( ch );
	if ( data != NULL ) {
		return data->Width;
	}

	return 0;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Char_Spacing
//
////////////////////////////////////////////////////////////////////////////////////
int
FontCharsClass::Get_Char_Spacing (unichar_t ch)
{
	const CharDataStruct	* data = Get_Char_Data( ch );
	if ( data != NULL ) {
		if ( data->Width != 0 ) {
			return data->Width + 1;
		}
	}

	return 0;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Blit_Char
//
////////////////////////////////////////////////////////////////////////////////////
void
FontCharsClass::Blit_Char (unichar_t ch, uint16 *dest_ptr, int dest_stride, int x, int y)
{
	const CharDataStruct	* data = Get_Char_Data( ch );
	if ( data != NULL && data->Width != 0 ) {

		//
		//	Setup the src and destination pointers
		//
		int dest_inc		= (dest_stride >> 1);
		uint16 *src_ptr	= data->Buffer;
		dest_ptr				+= (dest_inc * y) + x;

		//
		//	Simply copy the data from the src buffer to the destination
		//
		for ( int row = 0; row < CharHeight; row ++ ) {
			for ( int col = 0; col < data->Width; col ++ ) {
				dest_ptr[col] = *src_ptr++;
			}
			dest_ptr	+= dest_inc;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Store_GDI_Char
//
////////////////////////////////////////////////////////////////////////////////////
const FontCharsClass::CharDataStruct *
FontCharsClass::Store_GDI_Char (unichar_t ch)
{
	int width	= PointSize * 2;
	int height	= PointSize * 2;

	//
	//	Get the size of the character we just drew
	//
	SIZE char_size = { 0 };
	::GetTextExtentPoint32W( MemDC, reinterpret_cast<wchar_t *>(&ch), 1, &char_size );
	int x_pos = 0;

	//
	//	HACK HACK -- With the default font that Renegade uses the
	// W and V characters need to be moved over one pixel.
	//
	if ( (ch == 'W' || ch == 'V') && (GDIFontName.Compare_No_Case ("Arial MT") == 0) ) {
		x_pos				= 1;
		char_size.cx	+= 1;
	}

	//
	//	Draw the character into the memory DC
	//
	RECT rect = { 0, 0, width, height };
	::ExtTextOutW( MemDC, x_pos, 0, ETO_OPAQUE, &rect, reinterpret_cast<wchar_t *>(&ch), 1, NULL);

	//
	//	Get a pointer to the surface that this character should use
	//
	Update_Current_Buffer( char_size.cx );
	uint16 *curr_buffer = BufferList[BufferList.Count () - 1];
	curr_buffer += CurrPixelOffset;

	//
	//	Copy the BMP contents to the buffer
	//
	int stride = (((width * 3) + 3) & ~3);
	for (int row = 0; row < char_size.cy; row ++) {

		//
		//	Compute the indices into the BMP and surface
		//
		int index = (row * stride);

		//
		//	Loop over each column
		//
		for (int col = 0; col < char_size.cx; col ++) {

			//
			//	Get the pixel color at this location
			//
			uint8 pixel_value = GDIBitmapBits[index];
			index += 3;

			uint16 pixel_color = 0;
			if (pixel_value != 0) {
				pixel_color = 0x0FFF;
			}

			//
			//	Convert the pixel intensity from 8bit to 4bit and
			// store it in our buffer
			//
			uint8 alpha_value	= ((pixel_value >> 4) & 0xF);
			*curr_buffer ++	= pixel_color | (alpha_value << 12);
		}
	}

	//
	//	Save information about this character in our list
	//
	CharDataStruct *char_data	= new CharDataStruct;
	char_data->Value				= ch;
	char_data->Width				= char_size.cx;
	char_data->Buffer				= BufferList[BufferList.Count () - 1] + CurrPixelOffset;

	//
	//	Insert this character into our array
	//
	if ( ch < 256 ) {
		ASCIICharArray[ch] = char_data;
	} else {
		UnicodeCharArray[ch - FirstUnicodeChar] = char_data;
	}

	//
	//	Advance the character position
	//
	CurrPixelOffset += (char_size.cx * CharHeight);

	//
	//	Return the index of the entry we just added
	//
	return char_data;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Update_Current_Buffer
//
////////////////////////////////////////////////////////////////////////////////////
void
FontCharsClass::Update_Current_Buffer (int char_width)
{
	//
	//	Check to see if we need to allocate a new buffer
	//
	bool needs_new_buffer = (BufferList.Count () == 0);
	if (needs_new_buffer == false) {

		//
		//	Would we extend past this buffer?
		//
		if ( (CurrPixelOffset + (char_width * CharHeight)) > CHAR_BUFFER_LEN ) {
			needs_new_buffer = true;
		}
	}

	//
	//	Do we need to create a new surface?
	//
	if (needs_new_buffer) {
		uint16 *new_buffer = new uint16[CHAR_BUFFER_LEN];
		BufferList.Add( new_buffer );
		CurrPixelOffset = 0;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Create_GDI_Font
//
////////////////////////////////////////////////////////////////////////////////////
void
FontCharsClass::Create_GDI_Font (const char *font_name)
{
	HDC screen_dc = ::GetDC (NULL);

	//
	//	Calculate the height of the font in logical units
	//
	int font_height = -MulDiv (PointSize, ::GetDeviceCaps (screen_dc, LOGPIXELSY), 72);

	//
	//	Create the Windows font
	//
	DWORD bold		= IsBold ? FW_BOLD : FW_NORMAL;
	DWORD italic	= 0;
	DWORD	charset;

	// Map the current code page to a font character set.
	switch (GetACP()) {

		// Chinese.
		case 936:
		case 950:
			charset = CHINESEBIG5_CHARSET;
			break;

		// Japanese.
		case 932:
			charset = SHIFTJIS_CHARSET;
			break;

		// Korean.
		case 949:
			charset = HANGUL_CHARSET;
			break;

		// Anything else.
		default:
			charset = DEFAULT_CHARSET;
			break;
	}

	GDIFont = ::CreateFontA (font_height, 0, 0, 0, bold, italic,
									false, false, charset, OUT_DEFAULT_PRECIS,
									CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
									VARIABLE_PITCH, font_name);

	//
	// Set-up the fields of the BITMAPINFOHEADER
	//	Note: Top-down DIBs use negative height in Win32.
	//
	BITMAPINFOHEADER bitmap_info = { 0 };
	bitmap_info.biSize				= sizeof (BITMAPINFOHEADER);
	bitmap_info.biWidth				= PointSize * 2;
	bitmap_info.biHeight				= -(PointSize * 2);
	bitmap_info.biPlanes				= 1;
	bitmap_info.biBitCount			= 24;
	bitmap_info.biCompression		= BI_RGB;
	bitmap_info.biSizeImage			= ((PointSize * PointSize * 4) * 3);
	bitmap_info.biXPelsPerMeter	= 0;
	bitmap_info.biYPelsPerMeter	= 0;
	bitmap_info.biClrUsed			= 0;
	bitmap_info.biClrImportant		= 0;

	//
	// Create a bitmap that we can access the bits directly of
	//
	GDIBitmap	= ::CreateDIBSection (	screen_dc,
													(const BITMAPINFO *)&bitmap_info,
													DIB_RGB_COLORS,
													(void **)&GDIBitmapBits,
													NULL,
													0L);

	//
	//	Create a device context we can select the font and bitmap into
	//
	MemDC = ::CreateCompatibleDC (NULL);

	//
	//	Now select the BMP and font into the DC
	//
	OldGDIBitmap	= (HBITMAP)::SelectObject (MemDC, GDIBitmap);
	OldGDIFont		= (HFONT)::SelectObject (MemDC, GDIFont);
	::SetBkColor (MemDC, RGB (0, 0, 0));
	::SetTextColor (MemDC, RGB (255, 255, 255));

	//
	//	Lookup the pixel height of the font
	//
	TEXTMETRIC text_metric = { 0 };
	::GetTextMetrics (MemDC, &text_metric);
	CharHeight = text_metric.tmHeight;

	//
	// Release our temporary screen DC
	//
	::ReleaseDC (NULL, screen_dc);
	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Free_GDI_Font
//
////////////////////////////////////////////////////////////////////////////////////
void
FontCharsClass::Free_GDI_Font (void)
{
	//
	//	Select the old font back into the DC and delete
	// our font object
	//
	if ( GDIFont != NULL ) {
		::SelectObject( MemDC, OldGDIFont );
		::DeleteObject( GDIFont );
		GDIFont = NULL;
	}

	//
	//	Select the old bitmap back into the DC and delete
	// our bitmap object
	//
	if ( GDIBitmap != NULL ) {
		::SelectObject( MemDC, OldGDIBitmap );
		::DeleteObject( GDIBitmap );
		GDIBitmap = NULL;
	}

	//
	//	Delete our memory DC
	//
	if ( MemDC != NULL ) {
		::DeleteDC( MemDC );
		MemDC = NULL;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Initialize_GDI_Font
//
////////////////////////////////////////////////////////////////////////////////////
void
FontCharsClass::Initialize_GDI_Font (const char *font_name, int point_size, bool is_bold)
{
	//
	//	Build a unique name from the font name and its size
	//
	Name.Format ("%s%d", font_name, point_size);

	//
	//	Remember these settings
	//
	GDIFontName	= font_name;
	PointSize	= point_size;
	IsBold		= is_bold;

	//
	//	Create the actual font object
	//
	Create_GDI_Font (font_name);
	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Is_Font
//
////////////////////////////////////////////////////////////////////////////////////
bool
FontCharsClass::Is_Font (const char *font_name, int point_size, bool is_bold)
{
	bool retval = false;

	//
	//	Check to see if both the name and height matches...
	//
	if (	(GDIFontName.Compare_No_Case (font_name) == 0) &&
			(point_size == PointSize) &&
			(is_bold == IsBold))
	{
		retval = true;
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Grow_Unicode_Array
//
////////////////////////////////////////////////////////////////////////////////////
void
FontCharsClass::Grow_Unicode_Array (unichar_t ch)
{
	//
	//	Don't do anything if character is in the ASCII range
	//
	if ( ch < 256 ) {
		return ;
	}

	//
	//	Don't do anything if character is in the currently allocated range
	//
	if ( ch >= FirstUnicodeChar && ch <= LastUnicodeChar ) {
		return ;
	}

	uint16 first_index	= std::min( FirstUnicodeChar, ch );
	uint16 last_index		= std::max( LastUnicodeChar, ch );
	uint16 count			= (last_index - first_index) + 1;

	//
	//	Allocate enough memory to hold the new cells
	//
	CharDataStruct **new_array = new CharDataStruct *[count];
	::memset (new_array, 0, sizeof (CharDataStruct *) * count);

	//
	//	Copy the contents of the old array into the new array
	//
	if ( UnicodeCharArray != NULL ) {
		int start_offset	= (FirstUnicodeChar - first_index);
		int old_count		= (LastUnicodeChar - FirstUnicodeChar) + 1;
		::memcpy (&new_array[start_offset], UnicodeCharArray, sizeof (CharDataStruct *) * old_count);

		//
		//	Delete the old array
		//
		delete [] UnicodeCharArray;
		UnicodeCharArray = NULL;
	}

	FirstUnicodeChar	= first_index;
	LastUnicodeChar	= last_index;
	UnicodeCharArray	= new_array;
	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Free_Character_Arrays
//
////////////////////////////////////////////////////////////////////////////////////
void
FontCharsClass::Free_Character_Arrays (void)
{
	if ( UnicodeCharArray != NULL ) {

		int count = (LastUnicodeChar - FirstUnicodeChar) + 1;

		//
		//	Delete each member of the unicode array
		//
		for (int index = 0; index < count; index ++) {
			if ( UnicodeCharArray[index] != NULL ) {
				delete UnicodeCharArray[index];
				UnicodeCharArray[index] = NULL;
			}
		}

		//
		//	Delete the array itself
		//
		delete [] UnicodeCharArray;
		UnicodeCharArray = NULL;
	}

	//
	//	Delete each member of the ascii character array
	//
	for (int index = 0; index < 256; index ++) {
		if ( ASCIICharArray[index] != NULL ) {
			delete ASCIICharArray[index];
			ASCIICharArray[index] = NULL;
		}
	}

	return ;
}

