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
 *                     $Archive:: /Commando/Code/wwui/textmarqueectrl.cpp                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/17/01 3:32p                                                $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "textmarqueectrl.h"
#include "stylemgr.h"
#include "dialogmgr.h"


//////////////////////////////////////////////////////////////////////
//
//	TextMarqueeCtrlClass
//
//////////////////////////////////////////////////////////////////////
TextMarqueeCtrlClass::TextMarqueeCtrlClass (void)	:
	ScrollPos (0),
	PixelHeight (0),
	ScrollRate (2.0F)
{
	//
	//	Configure the renderers
	//
	StyleMgrClass::Assign_Font (&TextRenderers[0], StyleMgrClass::FONT_CREDITS);
	StyleMgrClass::Assign_Font (&TextRenderers[1], StyleMgrClass::FONT_CREDITS_BOLD);
	StyleMgrClass::Configure_Renderer (&ControlRenderer);
	TextRenderers[0].Set_Texture_Size_Hint (256);
	TextRenderers[1].Set_Texture_Size_Hint (256);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~TextMarqueeCtrlClass
//
//////////////////////////////////////////////////////////////////////
TextMarqueeCtrlClass::~TextMarqueeCtrlClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Create_Control_Renderer
//
//////////////////////////////////////////////////////////////////////
void
TextMarqueeCtrlClass::Create_Control_Renderer (void)
{
	Render2DClass &renderer = ControlRenderer;

	//
	//	Configure this renderer
	//
	renderer.Reset ();
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

	if (Style & WS_BORDER) {
		
		//
		//	Draw the control's outline
		//
		renderer.Add_Outline (Rect, 1.0F, color);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Create_Text_Renderer
//
//////////////////////////////////////////////////////////////////////
void
TextMarqueeCtrlClass::Create_Text_Renderer (void)
{
	TextRenderers[0].Reset ();
	TextRenderers[1].Reset ();
	TextRenderers[0].Set_Clipping_Rect (ClientRect);
	TextRenderers[1].Set_Clipping_Rect (ClientRect);
	TextRenderers[0].Set_Wrapping_Width (0);
	TextRenderers[1].Set_Wrapping_Width (0);

	int row_count = CreditLines.Count ();
	if (row_count <= 0) {
		return ;
	}

	//
	//	Calculate where to start rendering text
	//
	int ctrl_height		= ClientRect.Height ();
	float start_height	= (ScrollPos - ctrl_height);

	float curr_height		= 0;
	int curr_row;
	for (curr_row = 0; curr_row < CreditLines.Count (); curr_row ++) {		
		if ((curr_height + CreditLines[curr_row].Height) >= start_height) {
			break;
		}
		curr_height += CreditLines[curr_row].Height;
	}

	//
	//	Configure our starting text rectangle
	//
	RectClass text_rect	= ClientRect;	
	text_rect.Top			+= curr_height - (ScrollPos - ctrl_height);

	//
	//	Now, render the visible lines of text...
	//
	for (	int index = curr_row;
			(index < CreditLines.Count ()) && (text_rect.Top < ClientRect.Bottom);
			index ++) 
	{
		CREDIT_LINE &line = CreditLines[index];

		//
		//	Use the formatting parameters for this line...
		//
		Render2DSentenceClass *renderer	= &TextRenderers[line.FontIndex];
		int color								= line.Color;
		int shadow_color						= StyleMgrClass::Get_Text_Shadow_Color ();

		//
		//	Render this line of text centered
		//			
		StyleMgrClass::Render_Text (line.Text, renderer, color, shadow_color,
								text_rect, true, false, StyleMgrClass::CENTER_JUSTIFY, false);
		
		//
		//	Move down to the next row...
		//
		text_rect.Top = int(text_rect.Top + line.Height);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
TextMarqueeCtrlClass::Render (void)
{
	//
	//	Recreate the renderers (if necessary)
	//
	if (IsDirty) {
		Create_Control_Renderer ();
		Create_Text_Renderer ();
	}

	//
	//	Render the image...
	//		
	ControlRenderer.Render ();
	TextRenderers[0].Render ();
	TextRenderers[1].Render ();

	DialogControlClass::Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Client_Rect
//
////////////////////////////////////////////////////////////////
void
TextMarqueeCtrlClass::Update_Client_Rect (void)
{
	//
	//	Set the client area
	//
	ClientRect = Rect;
	ClientRect.Inflate (Vector2 (-5.0F * StyleMgrClass::Get_X_Scale (), -3.75F * StyleMgrClass::Get_Y_Scale ()));
	ClientRect.Left	= int(ClientRect.Left);
	ClientRect.Top		= int(ClientRect.Top);	
	ClientRect.Right	= int(ClientRect.Right);
	ClientRect.Bottom	= int(ClientRect.Bottom);

	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Text
//
////////////////////////////////////////////////////////////////
void
TextMarqueeCtrlClass::Set_Text (const WCHAR *title)
{
	DialogControlClass::Set_Text (title);
	ScrollPos = 0.0F;

	//
	//	Rebuild the data structures necessary for us to
	// render formatted text
	//
	Build_Credit_Lines ();

	//
	//	Count up the total height of all lines
	//
	for (int index = 0; index < CreditLines.Count (); index ++) {
		PixelHeight += TextRenderers[CreditLines[index].FontIndex].Peek_Font ()->Get_Char_Height ();
	}

	//
	//	Add two blank pages
	//
	PixelHeight += ClientRect.Height () * 2;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Build_Credit_Lines
//
////////////////////////////////////////////////////////////////
void
TextMarqueeCtrlClass::Build_Credit_Lines (void)
{
	CreditLines.Delete_All ();

	if (Title.Get_Length () == 0) {
		return; 
	}

	//
	//	Handy macro
	//
	#define COPY_LINE(dest,	src_start, src_end)	\
			if (src_end == NULL) {						\
				dest = src_start;							\
			} else {											\
				uint32 bytes	= ((uint32)src_end - (uint32)src_start);	\
				uint32 len		= bytes / sizeof (WCHAR);						\
				::memcpy (dest.Get_Buffer (len + 1), src_start, bytes);	\
				dest.Peek_Buffer ()[len] = 0;										\
			}

	int default_color = StyleMgrClass::Get_Text_Color ();
	CREDIT_LINE default_line (L"", 0, default_color);

	//
	//	Build an array of formatted text lines
	//	
	const WCHAR *text = Title;
	while (text != NULL) {
		
		//
		//	Scan this line for formatting information
		//
		CREDIT_LINE line (default_line);
		text = Read_Line (text, line);

		//
		//	Determine which renderer to use...
		//
		Render2DSentenceClass *renderer = &TextRenderers[line.FontIndex];
		renderer->Set_Wrapping_Width (ClientRect.Width ());

		//
		//	Loop over all the lines of text and check for wrapping...
		//
		const WCHAR *line_start = renderer->Find_Row_Start (line.Text, 0);
		while (line_start != NULL) {

			//
			//	Lookup the start of the next line...
			//
			const WCHAR *line_end = renderer->Find_Row_Start (line_start, 1);
			
			//
			//	Copy this line of text into the control
			//
			WideStringClass text;
			COPY_LINE (text, line_start, line_end);

			//
			//	Add this line to our list
			//
			CREDIT_LINE new_line (line);
			new_line.Text		= text;			
			new_line.Height	= TextRenderers[new_line.FontIndex].Peek_Font ()->Get_Char_Height ();
			CreditLines.Add (new_line);

			//
			//	Advance to the next line...
			//
			line_start = line_end;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Read_Tag
//
////////////////////////////////////////////////////////////////
int
TextMarqueeCtrlClass::Read_Tag (const WCHAR *text, CREDIT_LINE &line)
{
	int retval = -1;

	const WCHAR *TAG_BOLD	= L"bold";
	const WCHAR *TAG_COLOR	= L"color=";


	if (text[0] == L'<') {
		for (int index = 1; text[index] != 0; index ++) {
			
			//
			//	Is this the 'end-tag' bracket?
			//
			if (text[index] == L'>') {

				//
				//	What type of specifier did we find?
				//
				if (::_wcsnicmp (text+1, TAG_BOLD, ::wcslen (TAG_BOLD)) == 0) {
					
					//
					// We found the bold specifier
					//
					line.FontIndex = 1;
					retval = index;
				} else if (::_wcsnicmp (text+1, TAG_COLOR, ::wcslen (TAG_COLOR)) == 0) {
					
					//
					//	We found the color specifier
					//					
					int tag_len = ::wcslen (TAG_COLOR)+1;
					text += tag_len;

					//
					//	Copy the params to their own temp string
					//
					WideStringClass temp_buffer (index + 1, true);
//					::lstrcpynW (temp_buffer.Peek_Buffer (), text, (index + 1) - tag_len);
			
					int length = ((index + 1) - tag_len);
					WCHAR* tempPtr = temp_buffer.Peek_Buffer();
					wcsncpy(tempPtr, text, length);
					tempPtr[length - 1] = 0;

					int color[3] = { 0 };

					//
					//	Parse the params for the colors
					//
					WCHAR *buffer = temp_buffer.Peek_Buffer ();
					for (int color_index = 0; color_index < 3; color_index ++) {
						WCHAR *comma_str  = ::wcschr (buffer, L',');
						if (comma_str != NULL) {
							comma_str[0]			= 0;
							color[color_index]	= ::_wtoi (buffer);
							buffer					= &comma_str[1];
						} else {
							color[color_index] = ::_wtoi (buffer);
							break;
						}
					}

					//
					//	Store the color
					//
					line.Color	= RGB_TO_INT32 ((uint8)color[0], (uint8)color[1], (uint8)color[2]);
					retval		= index;
				}

				break;
			}
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Read_Line
//
////////////////////////////////////////////////////////////////
const WCHAR *
TextMarqueeCtrlClass::Read_Line (const WCHAR *text, CREDIT_LINE &line)
{
	const WCHAR *text_start = text;

	//
	//	Set some defaults
	//
	line.FontIndex = 0;
	line.Color		= StyleMgrClass::Get_Text_Color ();

	//
	//	Loop over all the text in the control
	//
	bool keep_going = true;
	do
	{
		WCHAR ch = *text;

		//
		//	Check to see if this character ends the
		//
		if (ch == L'\n' || ch == 0) {			
			int len = text - text_start;
//			::lstrcpynW (line.Text.Get_Buffer (len + 1), text_start, len + 1);

			WCHAR* buffer = line.Text.Get_Buffer(len + 1);
			wcsncpy(buffer, text_start, len + 1);
			buffer[len] = 0;
		
			keep_going = false;
		} else if (ch == L'<') {
			
			//
			//	Did we find a tag?
			//
			int new_index = Read_Tag (text, line);
			if (new_index != -1) {
				text			= &text[new_index];
				text_start	= text + 1;
			}
		}

		//
		//	Move to the next character
		//
		if (ch != 0) {
			text ++;
		}

	} while (keep_going);

	return text[0] != 0 ? text : 0;
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
TextMarqueeCtrlClass::On_Frame_Update (void)
{
	float char_height = TextRenderers[0].Peek_Font ()->Get_Char_Height ();

	//
	//	Determine how many pixels to scroll
	//
	float time					= DialogMgrClass::Get_Frame_Time () / 1000.0F;
	float pixels_scrolled	= time * ScrollRate * char_height;
	
	//
	//	Update the scroll position
	//
	float old_pos = ScrollPos;
	ScrollPos += pixels_scrolled;
	if (ScrollPos >= PixelHeight) {
		ScrollPos = 0;
	}

	//
	//	Update the view
	//
	if (int(old_pos) != int(ScrollPos)) {
		Set_Dirty ();
	}

	return ;
}
