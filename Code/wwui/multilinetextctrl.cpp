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
 *                 Project Name : wwui																			  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwui/multilinetextctrl.cpp                   $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/09/02 5:57p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "multilinetextctrl.h"
#include "assetmgr.h"
#include "stylemgr.h"
#include "dialogbase.h"
#include <winuser.h>
#include <algorithm>
#include <limits>


//////////////////////////////////////////////////////////////////////
//
//	MultiLineTextCtrlClass
//
//////////////////////////////////////////////////////////////////////
MultiLineTextCtrlClass::MultiLineTextCtrlClass (void)	:
	ScrollPos (0),
	IsScrollBarDisplayed (false),
	IsInitialized (false),
	RowCount (0),
	RowsPerPage (0)
{
	UINT scrolllines;

	//
	//	Configure the renderers
	//
	StyleMgrClass::Assign_Font (&TextRenderer, StyleMgrClass::FONT_CONTROLS);	
	StyleMgrClass::Configure_Renderer (&ControlRenderer);
	TextRenderer.Set_Texture_Size_Hint (256);

	//
	//	We don't want the scroll bar getting focus
	//
	ScrollBarCtrl.Set_Wants_Focus (false);
	ScrollBarCtrl.Set_Advise_Sink (this);	
	ScrollBarCtrl.Set_Is_Embedded (true);

	// Calculate the no. of lines to scroll for each mouse wheel increment.
	SystemParametersInfo (SPI_GETWHEELSCROLLLINES, 0, &scrolllines, 0);
	MouseWheelIncrement = ((float) scrolllines) / ((float) WHEEL_DELTA);

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~MultiLineTextCtrlClass
//
//////////////////////////////////////////////////////////////////////
MultiLineTextCtrlClass::~MultiLineTextCtrlClass (void)
{
	ScrollBarCtrl.Set_Advise_Sink (NULL);	
	
	if (Parent != NULL) {
		Parent->Remove_Control (&ScrollBarCtrl);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Create_Control_Renderer
//
//////////////////////////////////////////////////////////////////////
void
MultiLineTextCtrlClass::Create_Control_Renderer (void)
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

	//
	//	Draw the control's outline
	//
	renderer.Add_Outline (Rect, 1.0F, color);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Create_Text_Renderer
//
//////////////////////////////////////////////////////////////////////
void
MultiLineTextCtrlClass::Create_Text_Renderer (void)
{
	TextRenderer.Reset ();

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
	//	Determine where to start drawing the text from
	//
	const unichar_t *text_start = TextRenderer.Find_Row_Start (Title, ScrollPos);
	const unichar_t *text_end	= TextRenderer.Find_Row_Start (Title, ScrollPos+RowsPerPage);
	if (text_start != NULL) {

		//
		//	Should we draw the text centered?
		//
		if ((Style & ES_CENTER) == ES_CENTER) {			

			float char_height = TextRenderer.Peek_Font ()->Get_Char_Height ();
			RectClass text_rect = ClientRect;

			//
			//	Render each line separately
			//
			const unichar_t *line_start = text_start;
			const unichar_t *line_end	= TextRenderer.Find_Row_Start (Title, ScrollPos + 1);			
			for (int index = 0; index < RowsPerPage; index ++) {

				//
				//	Copy this line of text into a buffer
				//
				WideStringClass text;
				COPY_LINE (text, line_start, line_end);

				//
				//	Render this line of text centered
				//
				TextRenderer.Set_Wrapping_Width (0);
				StyleMgrClass::Render_Text (text, &TextRenderer, text_rect, true, true, StyleMgrClass::CENTER_JUSTIFY, IsEnabled, false);
				text_rect.Top = int (text_rect.Top + char_height);
				TextRenderer.Set_Wrapping_Width (ClientRect.Width ());

				if (line_end == NULL) {
					break;
				}

				//
				//	Advance to the next line...
				//
				line_start	= line_end;
				line_end		= TextRenderer.Find_Row_Start (line_start, 1);
			}

		} else {

			WideStringClass text;
			COPY_LINE (text, text_start, text_end);

			//
			//	Draw the text
			//
			StyleMgrClass::Render_Wrapped_Text (text, &TextRenderer, ClientRect, true, false, IsEnabled);
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
MultiLineTextCtrlClass::Render (void)
{
	//
	//	Recreate the renderers (if necessary)
	//
	if (IsDirty) {
		Update_Scroll_Bar_Visibility ();
		Create_Control_Renderer ();
		Create_Text_Renderer ();
	}

	//
	//	Render the image...
	//		
	ControlRenderer.Render ();
	TextRenderer.Render ();

	DialogControlClass::Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Client_Rect
//
////////////////////////////////////////////////////////////////
void
MultiLineTextCtrlClass::Update_Client_Rect (void)
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

	//
	//	Determine how many rows we can fit on one page
	//
	float char_height = TextRenderer.Peek_Font ()->Get_Char_Height ();
	RowsPerPage			= ClientRect.Height () / char_height;
	IsInitialized		= true;

	//
	//	Choose an arbitrary width, the scroll bar
	// will snap to the only width it supports
	//
	float width = 10;

	//
	//	Calculate the scroll bar's rectangle
	//
	RectClass scroll_rect;
	scroll_rect.Left		= Rect.Right - width;
	scroll_rect.Top		= Rect.Top;
	scroll_rect.Right		= Rect.Right;
	scroll_rect.Bottom	= Rect.Bottom;

	//
	//	Size the scroll bar
	//
	ScrollBarCtrl.Set_Window_Rect (scroll_rect);
	ScrollBarCtrl.Set_Small_BMP_Mode ((Rect.Height () < (StyleMgrClass::Get_Y_Scale () * 100.0F)));

	//Update_Scroll_Bar_Visibility ();
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Scroll_Bar_Visibility
//
////////////////////////////////////////////////////////////////
void
MultiLineTextCtrlClass::Update_Scroll_Bar_Visibility (void)
{
	if (IsInitialized == false ) {
		return ;
	}

	Calculate_Row_Count ();

	//
	//	Determine if we have more entries then we can
	// display on one page
	//
	bool needs_scrollbar	= (RowCount > RowsPerPage);
	float new_right		= 0;

	if (needs_scrollbar != IsScrollBarDisplayed) {

		//
		//	Do we need to show a scroll bar?
		//
		if (needs_scrollbar) {
			new_right = ScrollBarCtrl.Get_Window_Rect ().Left;
			IsScrollBarDisplayed = true;
		} else if (Parent != NULL) {
			new_right = ScrollBarCtrl.Get_Window_Rect ().Right;
			IsScrollBarDisplayed = false;
		}

		//
		//	Reset our window size (as necessary)
		//
		Rect.Right = new_right;
		ClientRect = Rect;
		ClientRect.Inflate (Vector2 (-5.0F * StyleMgrClass::Get_X_Scale (), -3.75F * StyleMgrClass::Get_Y_Scale ()));
		ClientRect.Left	= int(ClientRect.Left);
		ClientRect.Top		= int(ClientRect.Top);	
		ClientRect.Right	= int(ClientRect.Right);
		ClientRect.Bottom	= int(ClientRect.Bottom);


		Calculate_Row_Count ();

		//
		//	Add or remove the scrollbar (as necessary)
		//
		if (IsScrollBarDisplayed) {
			Parent->Add_Control (&ScrollBarCtrl);
		} else {
			Parent->Remove_Control (&ScrollBarCtrl);
		}
	}

	//
	//	Configure the scroll-bar's ranges
	//
	ScrollBarCtrl.Set_Page_Size (RowsPerPage);
	ScrollBarCtrl.Set_Range (0, RowCount - RowsPerPage);
	ScrollBarCtrl.Set_Pos (ScrollPos);
	ScrollPos = ScrollBarCtrl.Get_Pos ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Calculate_Row_Count
//
////////////////////////////////////////////////////////////////
void
MultiLineTextCtrlClass::Calculate_Row_Count (void)
{
	TextRenderer.Set_Wrapping_Width (ClientRect.Width ());

	//
	//	Get the number of rows we'll need to display
	//
	RowCount = 0;
	TextRenderer.Get_Formatted_Text_Extents (Title, &RowCount);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Key_Down
//
////////////////////////////////////////////////////////////////
bool
MultiLineTextCtrlClass::On_Key_Down (uint32 key_id, uint32 /* key_data */)
{
	switch (key_id)
	{
		case VK_UP:
			Set_Scroll_Pos (ScrollPos - 1);
			break;

		case VK_DOWN:
			Set_Scroll_Pos (ScrollPos + 1);
			break;

		case VK_PRIOR:
			Set_Scroll_Pos (ScrollPos - RowsPerPage);
			break;

		case VK_NEXT:
			Set_Scroll_Pos (ScrollPos + RowsPerPage);
			break;

		case VK_HOME:
			Set_Scroll_Pos (0);
			break;

		case VK_END:
			Set_Scroll_Pos (RowCount);
			break;
	}

	return true;
}


////////////////////////////////////////////////////////////////
//
//	On_VScroll
//
////////////////////////////////////////////////////////////////
void
MultiLineTextCtrlClass::On_VScroll (ScrollBarCtrlClass *, int , int new_position)
{
	Set_Scroll_Pos (new_position);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Mouse_Wheel
//
////////////////////////////////////////////////////////////////
void
MultiLineTextCtrlClass::On_Mouse_Wheel (int direction)
{
	int lineoffset = direction * MouseWheelIncrement;

	Set_Scroll_Pos (ScrollPos + lineoffset);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Scroll_Pos
//
////////////////////////////////////////////////////////////////
void
MultiLineTextCtrlClass::Set_Scroll_Pos (int new_position)
{
	if (ScrollPos != new_position) {
		ScrollPos = new_position;
		ScrollPos = std::min (ScrollPos, RowCount - RowsPerPage);
		ScrollPos = std::max (ScrollPos, 0);
		ScrollBarCtrl.Set_Pos (ScrollPos, false);		
		Set_Dirty ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Text
//
////////////////////////////////////////////////////////////////
void
MultiLineTextCtrlClass::Set_Text (const unichar_t *title)
{
	DialogControlClass::Set_Text (title);

	ScrollPos		= 0;
	RowCount			= 0;	
	//Update_Scroll_Bar_Visibility ();
	return ;
}
