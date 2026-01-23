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
 *                     $Archive:: /Commando/Code/wwui/listctrl.cpp         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/01/02 4:37p                                               $*
 *                                                                                             *
 *                    $Revision:: 41                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "listctrl.h"
#include "assetmgr.h"
#include "refcount.h"
#include "mousemgr.h"
#include "ww3d.h"
#include "dialogmgr.h"
#include "dialogbase.h"
#include "stylemgr.h"
#include <commctrl.h>
#include <algorithm>


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
const float	PULSE_RATE		= 2.0F;
const int	ROW_SPACING		= 4;


////////////////////////////////////////////////////////////////
//
//	ListCtrlClass
//
////////////////////////////////////////////////////////////////
ListCtrlClass::ListCtrlClass (void)	:
	CurrState (NORMAL),
	ScrollPos (0),
	HeaderRect (0, 0, 0, 0),
	TextRect (0, 0, 0, 0),
	CurrSel (-1),
	RowBorderHeight (0),
	PulsePercent (1.0F),
	PulseDirection (1.0F),
	IsScrollBarDisplayed (false),
	LastPageTopEntryIndex (0),
	IsSelectionAllowed(true),
	IsNoSelectionAllowed(false),
	IsMultipleSelection (false),
	SortColumn (0),
	SortType (SORT_NONE),
	MinRowHeight (0)
{
	//
	//	Configure each renderer
	//
	StyleMgrClass::Configure_Renderer (&ControlRenderer);
	StyleMgrClass::Configure_Renderer (&UnderlineRenderer);
	StyleMgrClass::Configure_Renderer (&HilightRenderer);
	StyleMgrClass::Configure_Hilighter (&HilightRenderer);

	//
	//	Set the font for each text renderer
	//
	StyleMgrClass::Assign_Font (&HeaderRenderer, StyleMgrClass::FONT_HEADER);
	StyleMgrClass::Assign_Font (&TextRenderer, StyleMgrClass::FONT_LISTS);
	TextRenderer.Set_Texture_Size_Hint (256);

	//
	//	We don't want the scroll bar getting focus
	//
	ScrollBarCtrl.Set_Wants_Focus (false);
	ScrollBarCtrl.Set_Advise_Sink (this);
	ScrollBarCtrl.Set_Is_Embedded (true);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~ListCtrlClass
//
////////////////////////////////////////////////////////////////
ListCtrlClass::~ListCtrlClass (void)
{
	Delete_All_Entries ();

	if (Parent != NULL) {
		Parent->Remove_Control (&ScrollBarCtrl);
	}

	return ;
}


void ListCtrlClass::Set_Tabstop(float stop)
{
	TextRenderer.Set_Tabstop(stop);
}


////////////////////////////////////////////////////////////////
//
//	Create_Control_Renderer
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Create_Control_Renderer (void)
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
	int color		= StyleMgrClass::Get_Line_Color ();
	int bkcolor		= StyleMgrClass::Get_Bk_Color ();
	if (CurrState == DISABLED) {
		color		= StyleMgrClass::Get_Disabled_Line_Color ();
		bkcolor	= StyleMgrClass::Get_Disabled_Bk_Color ();
	}

	//
	//	Draw the outline
	//
	renderer.Add_Rect (Rect, 1.0F, color, bkcolor);

	//
	//	Draw the selection bar if necessary
	//
	if (CurrSel != -1) {

		//
		//	Calculate what percentage our pulse effect is
		//
		PulsePercent += PulseDirection * PULSE_RATE * (DialogMgrClass::Get_Frame_Time () / 1000.0F);
		if (PulsePercent < 0 || PulsePercent > 1.0F) {
			PulseDirection = -PulseDirection;
			PulsePercent = WWMath::Clamp (PulsePercent, 0, 1);
		}

		//
		//	If this control has the focus then apply our pulsating effect to
		// the color of selection bar.
		//
		if (HasFocus) {

			float red	=  ((color & 0x00FF0000) >> 16) / 256.0F;
			float green	=  ((color & 0x0000FF00) >> 8) / 256.0F;
			float blue	=  ((color & 0x000000FF)) / 256.0F;

			float percent = (PulsePercent * 0.5F) + 0.5F;
			color = VRGB_TO_INT32 (Vector3 (red * percent, green * percent, blue * percent));
		}

		//
		//	Get the hilight rect
		//
		RectClass sel_rect;
		Get_Entry_Rect (CurrSel, sel_rect);
		if (sel_rect.Top >= TextRect.Top && sel_rect.Bottom <= TextRect.Bottom) {

			//
			//	Render the outline of the selection (if its on the screen)
			//
			renderer.Add_Outline (sel_rect, 1.0F, color);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Text_Renderers
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Create_Text_Renderers (void)
{
	//
	//	Configure the text renderers
	//
	HeaderRenderer.Reset ();
	TextRenderer.Reset ();
	HilightRenderer.Reset ();
	UnderlineRenderer.Reset ();
	IconMgr.Reset_Renderers ();

	//
	//	Prepare the column header (if necessary)
	//
	if ((Style & LVS_NOCOLUMNHEADER) == 0) {

		//
		//	Render each column header
		//
		int x_pos = HeaderRect.Left;
		for (int index = 0; index < ColList.Count (); index ++) {

			//
			//	Build a bounding rectangle for this column header
			//
			RectClass rect = HeaderRect;
			rect.Left	= x_pos;
			rect.Right	= x_pos + (ColList[index]->Get_Width () * HeaderRect.Width ());

			//
			//	Let the last column extend to the edge
			//
			if (index == ColList.Count () - 1) {
				rect.Right = HeaderRect.Right;
			}

			//
			//	Get the color
			//
			int color = VRGB_TO_INT32 (ColList[index]->Get_Color ());

			//
			//	Underline the header
			//
			Vector2 text_extent = HeaderRenderer.Get_Text_Extents (ColList[index]->Get_Name ());
			float y_pos = (rect.Top + (rect.Height () / 2) + (text_extent.Y / 2)) + 2.0F;
			UnderlineRenderer.Add_Line (Vector2 (rect.Left, y_pos),
										Vector2 (rect.Left + text_extent.X, y_pos), 1.0F, color);

			//
			//	Draw the sort designator
			//
			if (SortType != SORT_NONE && SortColumn == index) {

				const float TRI_WIDTH	= 8;
				const float TRI_SPACE	= 2;

				float tri_size			= TRI_WIDTH * StyleMgrClass::Get_Y_Scale ();
				float tri_half_size	= tri_size * 0.5F;
				float tri_x_pos		= rect.Left + text_extent.X + tri_size + (TRI_SPACE * StyleMgrClass::Get_Y_Scale ());
				float tri_y_pos		= HeaderRect.Center ().Y;

				if (SortType == SORT_ASCENDING) {

					UnderlineRenderer.Add_Line (Vector2 (tri_x_pos - tri_half_size, tri_y_pos + tri_half_size),
												Vector2 (tri_x_pos + tri_half_size, tri_y_pos + tri_half_size), 1.0F, color);

					UnderlineRenderer.Add_Line (Vector2 (tri_x_pos + tri_half_size, tri_y_pos + tri_half_size),
												Vector2 (tri_x_pos, tri_y_pos - tri_half_size), 1.0F, color);

					UnderlineRenderer.Add_Line (Vector2 (tri_x_pos, tri_y_pos - tri_half_size),
												Vector2 (tri_x_pos - tri_half_size, tri_y_pos + tri_half_size), 1.0F, color);
					
				} else {

					UnderlineRenderer.Add_Line (Vector2 (tri_x_pos - tri_half_size, tri_y_pos - tri_half_size),
												Vector2 (tri_x_pos + tri_half_size, tri_y_pos - tri_half_size), 1.0F, color);

					UnderlineRenderer.Add_Line (Vector2 (tri_x_pos + tri_half_size, tri_y_pos - tri_half_size),
												Vector2 (tri_x_pos, tri_y_pos + tri_half_size), 1.0F, color);

					UnderlineRenderer.Add_Line (Vector2 (tri_x_pos, tri_y_pos + tri_half_size),
												Vector2 (tri_x_pos - tri_half_size, tri_y_pos - tri_half_size), 1.0F, color);
				}
			}

			//
			//	Render the text
			//
			StyleMgrClass::Render_Text (ColList[index]->Get_Name (), &HeaderRenderer,
									color, RGB_TO_INT32 (0, 0, 0), rect, true, true);

			//
			//	Move on to the next column
			//
			x_pos = rect.Right;
		}
	}

	//
	//	Render the data by rows
	//
	float y_pos		= TextRect.Top;
	int row_count	= Get_Entry_Count ();
	for (int row_index = ScrollPos; row_index < row_count; row_index ++) {

		//
		//	Get the height of this row
		//
		float row_height = RowInfoList[row_index]->Get_Height ();

		//
		//	Don't render past the bottom of the control
		//
		if ((y_pos + row_height) >= TextRect.Bottom) {
			break;
		}

		//
		//	Render each column in this row
		//
		float x_pos = TextRect.Left;
		for (int index = 0; index < ColList.Count (); index ++) {

			//
			//	Determine how wide this column is
			//
			float col_width = (ColList[index]->Get_Width () * HeaderRect.Width ());		
			if (index == ColList.Count () - 1) {
				col_width = TextRect.Right - x_pos;
			}

			//
			//	Build a bounding rectangle for this entry
			//
			RectClass rect;
			rect.Left	= int(x_pos);
			rect.Right	= int(x_pos + col_width);
			rect.Top		= int(y_pos);
			rect.Bottom	= int(y_pos + row_height);

			//
			//	Render the entry
			//
			Render_Entry (rect, index, row_index);

			//
			//	Move on to the next column
			//
			x_pos = rect.Right;
		}

		//
		//	Render a hilight on this row if necessary
		//
		if (Is_Entry_Selected (row_index)) {
			RectClass row_rect	= TextRect;
			row_rect.Top			= (int)std::max (TextRect.Top, (float)y_pos);
			row_rect.Bottom		= (int)std::min (TextRect.Bottom, (float)(y_pos + row_height));
			StyleMgrClass::Render_Hilight (&HilightRenderer, row_rect);
		}

		//
		//	Move down to the next row
		//
		y_pos += row_height;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render_Entry
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Render_Entry (const RectClass &clip_rect, int col_index, int row_index)
{
	RectClass rect = clip_rect;

	//
	//	Render the icons
	//
	int icon_count = ColList[col_index]->Get_Icon_Count (row_index);
	for (int index = 0; index < icon_count; index ++) {		
		
		//
		//	Render this icon
		//
		const char *icon_name = ColList[col_index]->Get_Icon (row_index, index);
		IconMgr.Render_Icon (rect, icon_name);

		//
		//	Move the rect by the width of the icon
		//
		rect.Left += IconMgr.Get_Icon_Width ();
	}

	//
	//	Get the text
	//
	const unichar_t *text = ColList[col_index]->Get_Entry_Text (row_index);
	int text_color		= VRGB_TO_INT32 (ColList[col_index]->Get_Entry_Color (row_index));
				
	//
	//	Render the text
	//
	StyleMgrClass::Render_Wrapped_Text (text, &TextRenderer, text_color,
							RGB_TO_INT32 (0, 0, 0), rect, true, true);

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Cursor
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::On_Set_Cursor (const Vector2 &/* mouse_pos */)
{
	if (IsSelectionAllowed) {
		//
		//	Change the mouse cursor
		//
		MouseMgrClass::Set_Cursor (MouseMgrClass::CURSOR_ACTION);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Sort_Designator
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Set_Sort_Designator (int col_index, SORT_TYPE type)
{
	SortColumn	= col_index;
	SortType		= type;
	
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Sort_Alphabetically
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Sort_Alphabetically (int col_index, SORT_TYPE type)
{
	//
	//	Sort the entries
	//
	Sort (Default_Sort_Callback, MAKELONG ((WORD)col_index, (WORD)type));

	//
	//	Update the sort marker
	//
	Set_Sort_Designator (col_index, type);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Default_Sort_Callback
//
////////////////////////////////////////////////////////////////
int
ListCtrlClass::Default_Sort_Callback (ListCtrlClass *list_ctrl, int item_index1, int item_index2, uint32 user_param)
{
	//
	//	Get the sorting params
	//
	int	sort_col_index = LOWORD (user_param);
	SORT_TYPE sort_type = (SORT_TYPE)HIWORD (user_param);
		
	//
	//	Sort by name
	//
	const unichar_t *name1 = list_ctrl->Get_Entry_Text (item_index1, sort_col_index);
	const unichar_t *name2 = list_ctrl->Get_Entry_Text (item_index2, sort_col_index);

	int retval = u_strcasecmp(name1, name2, U_COMPARE_CODE_POINT_ORDER);

	//
	//	Invert the return value if we are sorting descendingly
	//
	if (sort_type == SORT_DESCENDING) {
		retval = -retval;
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Update_Client_Rect
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Update_Client_Rect (void)
{
	Vector2 header_size = HeaderRenderer.Get_Text_Extents (U_CHAR("W"));

	//
	//	Set the client area
	//
	ClientRect = Rect;
	ClientRect.Inflate (Vector2 (-header_size.X, -1));
	HeaderRect	= ClientRect;
	TextRect		= ClientRect;

	//
	//	Calculate what the header rectangle should be
	//
	if ((Style & LVS_NOCOLUMNHEADER) == 0) {
		float char_height = header_size.Y;
		HeaderRect.Bottom	= HeaderRect.Top + (char_height * 3);
	} else {
		HeaderRect.Bottom = HeaderRect.Top;
	}

	//
	//	Move the text rectangle down
	//
	TextRect.Top = HeaderRect.Bottom;

	//float scale_y = Render2DClass::Get_Screen_Resolution().Height () / 600;

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

	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Find_Top_Of_Page
//
////////////////////////////////////////////////////////////////
int
ListCtrlClass::Find_Top_Of_Page (int bottom_index)
{
	int retval	= 0;
	//int count	= RowInfoList.Count ();	
	float y_pos	= TextRect.Bottom;

	//
	//	Scan the entries until we've found the first one
	// to extend off the top of the page
	//
	for (int index = bottom_index; index >= 0; index --) {
		y_pos -= RowInfoList[index]->Get_Height ();

		//
		//	If we've gone off the page, then back off one entry
		//
		if (y_pos < TextRect.Top) {
			retval = (index + 1);
			break;
		}
	}
	
	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Find_End_Of_Page
//
////////////////////////////////////////////////////////////////
int
ListCtrlClass::Find_End_Of_Page (void)
{
	int count	= RowInfoList.Count ();
	int retval	= count;
	float y_pos	= TextRect.Top;

	//
	//	Scan the entries until we've found the first one
	// to extend off the page
	//
	for (int index = ScrollPos; index < count; index ++) {
		y_pos += RowInfoList[index]->Get_Height ();

		//
		//	If we've gone off the page, then back off one entry
		//
		if (y_pos >= TextRect.Bottom) {
			retval = (index - 1);
			break;
		}
	}
	
	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Find_Last_Page_Top_Entry
//
////////////////////////////////////////////////////////////////
int
ListCtrlClass::Find_Last_Page_Top_Entry (void)
{
	int retval = 0;

	float y_pos	= TextRect.Bottom;
	int count	= RowInfoList.Count ();
	
	//
	//	Scan backwards from the bottom entry until we've
	// found one that extends off the top of the page.
	//
	for (int index = count - 1; index >= 0; index --) {
		y_pos -= RowInfoList[index]->Get_Height ();
		if (y_pos <= TextRect.Top) {
			retval = index + 1;
			break;
		}
	}
	
	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Update_Scroll_Bar_Visibility
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Update_Scroll_Bar_Visibility (void)
{
	LastPageTopEntryIndex = Find_Last_Page_Top_Entry ();

	//
	//	Configure the scroll-bar's ranges
	//
	ScrollBarCtrl.Set_Page_Size (0);
	ScrollBarCtrl.Set_Range (0, LastPageTopEntryIndex);

	//
	//	Determine if we have more entries then we can
	// display on one page
	//
	bool needs_scrollbar	= false;
	float y_pos				= TextRect.Top;
	int index;
	for (index = 0; index < RowInfoList.Count (); index ++) {
		y_pos += RowInfoList[index]->Get_Height ();
		if (y_pos >= TextRect.Bottom) {
			needs_scrollbar = true;
			break;
		}
	}

	float new_right = 0;

	//
	//	Do we need to show a scroll bar?
	//
	bool was_scrollbar_displayed = IsScrollBarDisplayed;
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
	Rect.Right			= new_right;
	ClientRect.Right	= Rect.Right - (ClientRect.Left - Rect.Left);
	TextRect.Right		= ClientRect.Right;
	HeaderRect.Right	= ClientRect.Right;

	//
	//	Update the row heights of each entry if the scroll bar
	// visibility changed
	//
	if (was_scrollbar_displayed != IsScrollBarDisplayed) {
		for (index = 0; index < RowInfoList.Count (); index ++) {
			Update_Row_Height (index);
		}

		//
		//	Update the last page top entry
		//
		LastPageTopEntryIndex = Find_Last_Page_Top_Entry ();
	}

	//
	//	Configure the scroll-bar's ranges
	//
	ScrollBarCtrl.Set_Page_Size (0);
	ScrollBarCtrl.Set_Range (0, LastPageTopEntryIndex);

	//
	//	Configure the scroll bar's position
	//
	if (ScrollBarCtrl.Get_Pos () > LastPageTopEntryIndex) {
		int new_pos = std::max (LastPageTopEntryIndex, 0);
		ScrollBarCtrl.Set_Pos (new_pos);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Render (void)
{
	//
	//	Recreate the renderers (if necessary)
	//
	if (IsDirty) {
		Update_Scroll_Bar_Visibility ();
		Create_Text_Renderers ();
	}

	if (IsScrollBarDisplayed) {
		Parent->Add_Control (&ScrollBarCtrl);
	} else {
		Parent->Remove_Control (&ScrollBarCtrl);
	}

	if (HasFocus || IsDirty) {
		Create_Control_Renderer ();
	}

	//
	//	Render the background and text
	//	
	IconMgr.Render_Icons ();
	TextRenderer.Render ();
	HilightRenderer.Render ();
	UnderlineRenderer.Render ();
	HeaderRenderer.Render ();
	ControlRenderer.Render ();

	DialogControlClass::Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_First_Selected
//
////////////////////////////////////////////////////////////////
int
ListCtrlClass::Get_First_Selected (void) const
{
	int retval = -1;

	//
	//	Loop over all the entries in the list control
	//
	int entry_count = Get_Entry_Count ();
	for (int index = 0; index < entry_count; index ++) {
		
		//
		//	Is this entry selected?
		//
		if (RowInfoList[index]->Is_Selected ()) {
			retval = index;
			break;
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Get_Next_Selected
//
////////////////////////////////////////////////////////////////
int
ListCtrlClass::Get_Next_Selected (int index) const
{
	int retval = -1;

	//
	//	Loop over all the entries in the list control
	//
	int entry_count = Get_Entry_Count ();
	for (index ++; index < entry_count; index ++) {
		
		//
		//	Is this entry selected?
		//
		if (RowInfoList[index]->Is_Selected ()) {
			retval = index;
			break;
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Select_All
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Select_All (bool select)
{
	//
	//	Loop over all the entries in the list control
	//
	int entry_count = Get_Entry_Count ();
	for (int index = 0; index < entry_count; index ++) {
		RowInfoList[index]->Select (select);
	}

	Set_Dirty();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Toggle_Entry_Selection
//
////////////////////////////////////////////////////////////////
bool
ListCtrlClass::Toggle_Entry_Selection (int index)
{
	if (index < 0 || index >= Get_Entry_Count ()) {
		return false;
	}

	//
	//	Toggle the selection of this entry
	//
	bool selected = RowInfoList[index]->Is_Selected ();
	RowInfoList[index]->Select (!selected);
	Set_Dirty ();

	return false;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_DblClk
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::On_LButton_DblClk (const Vector2 &mouse_pos)
{
	int sel_entry = Entry_From_Pos (mouse_pos);
	if (sel_entry == CurrSel) {

		//
		//	Notify any advise sinks
		//
		ADVISE_NOTIFY (On_ListCtrl_DblClk (this, Get_ID (), sel_entry));
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Down
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::On_LButton_Down (const Vector2 &mouse_pos)
{
	if (mouse_pos.Y <= HeaderRect.Bottom) {

		//
		//	Find out which column was clicked
		//
		int col_index = Col_From_Pos (mouse_pos);

		if (-1 != col_index) {
			//
			//	Notify any advise sinks
			//
			ADVISE_NOTIFY (On_ListCtrl_Column_Click (this, Get_ID (), col_index));
		}
	} else {

		//
		//	Change the hilighted entry to reflect the mouse click
		//
		int new_sel = Entry_From_Pos (mouse_pos);
		if (new_sel != -1) {
			Set_Sel (new_sel, true);
		} else if (IsNoSelectionAllowed) {
			Set_Sel(-1, true);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Up
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::On_LButton_Up (const Vector2 &/* mouse_pos */)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Mouse_Move
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::On_Mouse_Move (const Vector2 &mouse_pos)
{
	int sel_entry = Entry_From_Pos(mouse_pos);
	ADVISE_NOTIFY(On_ListCtrl_Mouse_Over(this, Get_ID(), sel_entry));
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Focus
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::On_Set_Focus (void)
{
	Set_Dirty ();

	DialogControlClass::On_Set_Focus ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Kill_Focus
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::On_Kill_Focus (DialogControlClass *focus)
{
	Set_Dirty ();

	DialogControlClass::On_Kill_Focus (focus);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Key_Down
//
////////////////////////////////////////////////////////////////
bool
ListCtrlClass::On_Key_Down (uint32 key_id, uint32 /* key_data */)
{
	bool handled = true;

	switch (key_id)
	{
		case VK_UP:
		case VK_LEFT:
			Set_Sel (CurrSel - 1, true);
			break;

		case VK_DOWN:
		case VK_RIGHT:
			Set_Sel (CurrSel + 1, true);
			break;

		case VK_PRIOR:
			if (CurrSel == ScrollPos) {
				Scroll_Page (-1);
			} else {
				Set_Sel (ScrollPos, true);
			}
			break;

		case VK_NEXT:
		{
			int end_of_page = Find_End_Of_Page ();

			//
			//	If we are at the end of the page, then
			// scroll one page, otherwise snap to the 
			// end of the page
			//
			if (CurrSel == end_of_page) {
				Scroll_Page (1);
			} else {
				Set_Sel (end_of_page, true);
			}
			break;
		}

		case VK_HOME:
			Set_Sel (0, true);
			break;

		case VK_END:
			Set_Sel (Get_Entry_Count () - 1, true);
			break;

		case VK_SPACE:
		case VK_RETURN:
			ADVISE_NOTIFY(On_ListCtrl_DblClk(this, Get_ID(), Get_Curr_Sel()));
			break;

		default:
			handled = false;
			break;
	}

	return handled;
}


////////////////////////////////////////////////////////////////
//
//	Sort
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Sort (LISTCTRL_SORT_CALLBACK sort_callback, uint32 user_param)
{
	//
	//	Quick sort the data
	//
	int entry_count = Get_Entry_Count ();
	if (entry_count > 1) {
		Quick_Sort (0, entry_count - 1, sort_callback, user_param);
	}

	//
	//	Force a repaint
	//
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Quick_Sort
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Quick_Sort
(
	int							start_index,
	int							end_index,
	LISTCTRL_SORT_CALLBACK	sort_callback,
	uint32						user_param
)
{
	//
	//	Setup a handy macro for swapping all columns of a particular row
	//
	#define QSORT_ROW_SWAP(index1, index2)														\
		{																									\
			if (CurrSel == index1)			{ CurrSel = index2; }							\
			else if (CurrSel == index2)	{ CurrSel = index1; }							\
			ListRowClass *temp	= RowInfoList[index1];										\
			RowInfoList[index1]	= RowInfoList[index2];										\
			RowInfoList[index2]	= temp;															\
			for (int col_index = 0; col_index < ColList.Count (); col_index ++) {	\
				ColList[col_index]->Swap_Entries (index1, index2);							\
			}																								\
		}

	//
	//	Determine our ranges
	//
	int pivot_index	= start_index;
	int min_index		= std::min (start_index + 1, end_index);
	int max_index		= end_index;

	//
	//	If the range is large enough, try to pick
	// a *good* pivot point.
	//
	if ((end_index - start_index) > 10) {
		int middle_index = start_index + ((end_index - start_index) / 2);

		//
		//	Put the smaller of the middle and end indices into the middle slot
		//
		if ((*sort_callback) (this, end_index, middle_index, user_param) < 0) {
			QSORT_ROW_SWAP (end_index, middle_index);
		}

		//
		//	Put the larger of the middle and start indices into the start slot
		//
		if ((*sort_callback) (this, middle_index, start_index, user_param) > 0) {
			QSORT_ROW_SWAP (middle_index, start_index);
		}

		//
		//	Put the smaller of the start and end indices into the start slot
		//
		if ((*sort_callback) (this, end_index, start_index, user_param) < 0) {
			QSORT_ROW_SWAP (end_index, start_index);
		}
	}

	//
	//	Put all the other entries in our range on either the lower or upper
	// side of the pivot entry based on their relative sort value.
	//
	bool keep_going = true;
	do
	{
		//
		//	Find the first entry that is "greater-than" the pivot
		//
		for (; min_index <= max_index; min_index ++) {

			//
			//	Is this entry "greater-than" the pivot?
			//
			if ((*sort_callback) (this, min_index, start_index, user_param) > 0) {
				break;
			}

			//
			//	The last index that is "less-than" the pivot entry will
			// be the pivot's new index.
			//
			pivot_index = min_index;
		}

		//
		//	Find the last entry that is "less-than" the pivot
		//
		for (; max_index >= min_index; max_index --) {

			//
			//	Is this entry "less-than" the pivot?
			//
			if ((*sort_callback) (this, max_index, start_index, user_param) < 0) {
				break;
			}
		}

		//
		//	Have we processed all the entries in our range?
		//
		if (min_index >= max_index) {
			keep_going = false;
		} else {

			//
			//	Swap the entries
			//
			QSORT_ROW_SWAP (min_index, max_index);

			//
			//	For efficiency's sake, skip over the entries we just swapped.
			//
			min_index ++;
			max_index --;
			pivot_index ++;
		}

	} while (keep_going);

	//
	//	Insert the pivot point into its "sorted" position in the array (if necessary)
	//
	if (pivot_index > start_index) {
		QSORT_ROW_SWAP (start_index, pivot_index);
	}

	//
	//	Calculate the region below the pivot
	//
	int lower_range_min = start_index;
	int lower_range_max = pivot_index - 1;

	//
	//	Recurse (if necessary) into the region below the pivot
	//
	if (lower_range_max < end_index && lower_range_min < lower_range_max) {
		Quick_Sort (lower_range_min, lower_range_max, sort_callback, user_param);
	}

	//
	//	Calculate the region above the pivot
	//
	int upper_range_min = pivot_index + 1;
	int upper_range_max = end_index;

	//
	//	Recurse (if necessary) into the region above the pivot
	//
	if (upper_range_min > start_index && upper_range_min < upper_range_max) {
		Quick_Sort (upper_range_min, upper_range_max, sort_callback, user_param);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Create
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::On_Create (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Destroy
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::On_Destroy (void)
{
	Delete_All_Entries ();
	Delete_All_Columns ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Auto_Size_Columns_Include_Contents
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Auto_Size_Columns_Include_Contents (float col_spacing)
{
	float total_width = HeaderRect.Width ();

	//
	//	Loop over all the columns
	//
	for (int index = 0; index < ColList.Count (); index ++) {
		float max_width = HeaderRenderer.Get_Text_Extents (ColList[index]->Get_Name ()).X;

		//
		//	Now, take into consideration the largest entry we've got
		//
		int count = ColList[index]->Get_Entry_Count ();
		for (int row = 0; row < count; row ++) {
			float width = TextRenderer.Get_Text_Extents (ColList[index]->Get_Entry_Text (row)).X;
			max_width = std::max (width, max_width);

			//
			//	Add the width of all the icons to this row's total
			//
			int icon_count = ColList[index]->Get_Icon_Count (row);
			for (int icon_index = 0; icon_index < icon_count; icon_index ++) {
				max_width += IconMgr.Get_Icon_Width ();
			}
		}

		//
		//	Auto-size this column
		//
		float percent = (max_width + col_spacing) / total_width;
		ColList[index]->Set_Width (percent);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Auto_Size_Columns
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Auto_Size_Columns (float col_spacing)
{
	float max_width = HeaderRect.Width ();

	for (int index = 0; index < ColList.Count (); index ++) {

		//
		//	Auto-size this column
		//
		float width		= HeaderRenderer.Get_Text_Extents (ColList[index]->Get_Name ()).X;
		float percent	= (width + col_spacing) / max_width;
		ColList[index]->Set_Width (percent);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Add_Icon
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Add_Icon (int index, int col_index, const char *texture_name)
{
	if (col_index < 0 || col_index >= ColList.Count ()) {
		return ;
	}

	//
	//	Add an icon to this entry
	//
	IconMgr.Add_Icon (texture_name);
	ColList[col_index]->Add_Icon (index, texture_name);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Reset_Icons
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Reset_Icons (int index, int col_index)
{
	if (col_index < 0 || col_index >= ColList.Count ()) {
		return ;
	}

	//
	//	Reset all the icons in this entry
	//
	ColList[col_index]->Reset_Icons (index);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Add_Column
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Add_Column (const unichar_t *column_name, float width, const Vector3 &color)
{
	//
	//	Create a new column and add it to the list
	//
	ListColumnClass *column = new ListColumnClass;
	column->Set_Name (column_name);
	column->Set_Width (width);
	column->Set_Color (color);
	ColList.Add (column);
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Column_Color
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Set_Column_Color (int col_index, const Vector3 &color)
{
	if (col_index < 0 || col_index >= ColList.Count ()) {
		return ;
	}

	ColList[col_index]->Set_Color (color);
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Remove_Column
//
////////////////////////////////////////////////////////////////
bool
ListCtrlClass::Remove_Column (int index)
{
	if (index < 0 || index >= ColList.Count ()) {
		return false;
	}

	//
	//	Free the column
	//
	ListColumnClass *column = ColList[index];
	delete column;

	//
	//	Remove the column from the list
	//
	ColList.Delete (index);
	Set_Dirty ();
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Delete_All_Columns
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Delete_All_Columns (void)
{
	for (int index = 0; index < ColList.Count (); index ++) {
		ListColumnClass *column = ColList[index];
		delete column;
	}

	ColList.Delete_All ();
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Column_Count
//
////////////////////////////////////////////////////////////////
int
ListCtrlClass::Get_Column_Count (void) const
{
	return ColList.Count ();
}


////////////////////////////////////////////////////////////////
//
//	Get_Entry_Count
//
////////////////////////////////////////////////////////////////
int
ListCtrlClass::Get_Entry_Count (void) const
{
	return RowInfoList.Count ();
}


////////////////////////////////////////////////////////////////
//
//	Delete_Entry
//
////////////////////////////////////////////////////////////////
bool
ListCtrlClass::Delete_Entry (int index)
{
	bool retval = true;

	//
	//	Nofity the advise sinks that we are deleting this entry
	//
	ADVISE_NOTIFY (On_ListCtrl_Delete_Entry (this, Get_ID (), index));

	//
	//	Remove this row from our data structures
	//
	if (index >= 0 && index < RowInfoList.Count ()) {
		delete RowInfoList[index];
		RowInfoList.Delete (index);
	}

	//
	//	Remove this entry from each column
	//
	for (int col_index = 0; col_index < ColList.Count (); col_index ++) {
		retval &= ColList[col_index]->Delete_Entry (index);
	}

	// When an entry that is before the current selection is deleted then
	// we need to adjust the current selection to reflect that.
	if (index < CurrSel) {
		--CurrSel;
	}

	//
	//	Update the current selection indicator (if necessary)
	//
	int entry_count = Get_Entry_Count ();
	if (CurrSel >= entry_count) {
		CurrSel = entry_count - 1;
	}

	//
	//	Hilight the new current selection (if necessary)
	//
	if (CurrSel >= 0 && CurrSel <= RowInfoList.Count ()) {
		RowInfoList[CurrSel]->Select ((IsMultipleSelection == false));
	}

	Set_Dirty ();
	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Find_Entry
//
////////////////////////////////////////////////////////////////
int ListCtrlClass::Find_Entry(int col_index, const unichar_t* text)
{
	int count = ColList.Count();

	if (col_index >= 0 && col_index < count) {
		ListColumnClass* list = ColList[col_index];
		count = list->Get_Entry_Count();

		for (int index = 0; index < count; index++) {
			const unichar_t* entryText = list->Get_Entry_Text(index);

			if (u_strcmp(entryText, text) == 0) {
				return index;
			}
		}
	}

	return -1;
}


////////////////////////////////////////////////////////////////
//
//	Insert_Entry
//
////////////////////////////////////////////////////////////////
int
ListCtrlClass::Insert_Entry (int index, const unichar_t *text)
{
	if (ColList.Count () <= 0) {
		return -1;
	}

	//
	//	Insert a blank row in our data structure
	//
	if (index < RowInfoList.Count ()) {
		RowInfoList.Insert (index + 1, new ListRowClass);
	} else {
		RowInfoList.Add (new ListRowClass);
	}

	//
	//	Insert a new entry in the first column with the given text
	//
	index = ColList[0]->Insert_Entry (index, text);

	//
	//	Use the default text color for each new entry
	//
	int color	= StyleMgrClass::Get_Text_Color ();
	float red	=  ((color & 0x00FF0000) >> 16) / 256.0F;
	float green	=  ((color & 0x0000FF00) >> 8) / 256.0F;
	float blue	=  ((color & 0x000000FF)) / 256.0F;
	Vector3 new_color (red, green, blue);
	
	ColList[0]->Set_Entry_Color (index, new_color);

	//
	//	Add blank entries to all the other columns
	//
	for (int col_index = 1; col_index < ColList.Count (); col_index ++) {
		ListColumnClass *column = ColList[col_index];
		column->Insert_Entry (index, U_CHAR(""));		
		column->Set_Entry_Color (index, new_color);
	}

	//
	//	Update the cached row height for this entry...
	//
	Update_Row_Height (index);

	//
	//	Update the last page top entry
	//
	LastPageTopEntryIndex = Find_Last_Page_Top_Entry ();
	Set_Dirty ();
	return index;
}


////////////////////////////////////////////////////////////////
//
//	Update_Row_Height
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Update_Row_Height (int row_index)
{
	int border_height		= (ROW_SPACING * StyleMgrClass::Get_Y_Scale ());
	float height			= (TextRenderer.Get_Text_Extents (U_CHAR("W")).Y + border_height);

	//
	//	Render each column in this row
	//
	float x_pos = TextRect.Left;
	for (int index = 0; index < ColList.Count (); index ++) {

		//
		//	Determine how wide this column is
		//
		float col_width = (ColList[index]->Get_Width () * HeaderRect.Width ());		
		if (index == ColList.Count () - 1) {
			col_width = TextRect.Right - x_pos;
		}

		//
		//	Set the wrapping width
		//
		TextRenderer.Set_Wrapping_Width (col_width);
		
		//
		//	Calculate the height of this text
		//
		const unichar_t *text = ColList[index]->Get_Entry_Text (row_index);
		Vector2 extents = TextRenderer.Get_Formatted_Text_Extents (text);
		height = std::max (height, extents.Y + border_height);

		//
		//	Increment the x-position
		//
		x_pos += col_width;
	}

	//
	//	Make sure the row is AT LEST MinRowHeight units high
	//
	float min_height	= MinRowHeight * StyleMgrClass::Get_Y_Scale ();
	height				= std::max (min_height, height);
	
	//
	//	Store the row height
	//
	RowInfoList[row_index]->Set_Height (height);

	TextRenderer.Set_Wrapping_Width (0);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Icon_Size
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Set_Icon_Size (float width, float height)
{
	//
	//	Pass these values on the icon manager
	//
	IconMgr.Set_Icon_Width (width);
	IconMgr.Set_Icon_Height (height);
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Min_Row_Height
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Set_Min_Row_Height (int height)
{
	MinRowHeight = height;

	//
	//	Update each row using this new height information
	//
	for (int index = 0; index < RowInfoList.Count (); index ++) {
		Update_Row_Height (index);
	}

	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Select_Entry
//
////////////////////////////////////////////////////////////////
bool
ListCtrlClass::Select_Entry (int index, bool onoff)
{
	if (index < 0 || index >= Get_Entry_Count ()) {
		return false;
	}

	//
	//	Set the selection state of the row
	//
	RowInfoList[index]->Select (onoff);
	Set_Dirty ();

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Is_Entry_Selected
//
////////////////////////////////////////////////////////////////
bool
ListCtrlClass::Is_Entry_Selected (int index)
{
	bool retval = false;
	if (index >= 0 && index < Get_Entry_Count ()) {
		
		//
		//	Get the selection state of the row
		//
		retval = RowInfoList[index]->Is_Selected ();
		Set_Dirty ();
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Set_Entry_Text
//
////////////////////////////////////////////////////////////////
bool
ListCtrlClass::Set_Entry_Text (int index, int col_index, const unichar_t *text)
{
	if (col_index < 0 || col_index >= ColList.Count ()) {
		return false;
	}

	//
	//	Change the text entry in this cell
	//
	ColList[col_index]->Set_Entry_Text (index, text);

	//
	//	Update the cached row height for this entry...
	//
	Update_Row_Height (index);

	//
	//	Update the last page top entry
	//
	LastPageTopEntryIndex = Find_Last_Page_Top_Entry ();
	Set_Dirty ();
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Set_Entry_Int
//
////////////////////////////////////////////////////////////////
bool
ListCtrlClass::Set_Entry_Int (int index, int col_index, int value)
{
	if (col_index < 0 || col_index >= ColList.Count ()) {
		return false;
	}

	//
	//	Convert the integer to a string
	//
	WideStringClass number_str;
	number_str.Format (U_CHAR("%d"), value);

	//
	//	Change the text entry in this cell
	//
	ColList[col_index]->Set_Entry_Text (index, number_str);
	Set_Dirty ();
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Set_Entry_Color
//
////////////////////////////////////////////////////////////////
bool
ListCtrlClass::Set_Entry_Color (int index, int col_index, const Vector3 &color)
{
	if (col_index < 0 || col_index >= ColList.Count ()) {
		return false;
	}

	//
	//	Change the color for the entry in this cell
	//
	ColList[col_index]->Set_Entry_Color (index, color);
	Set_Dirty ();
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Set_Entry_Data
//
////////////////////////////////////////////////////////////////
bool
ListCtrlClass::Set_Entry_Data (int index, int col_index, uintptr_t user_data)
{
	//
	//	Store the user data in the first column
	//
	ColList[col_index]->Set_Entry_Data (index, user_data);
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Get_Entry_Data
//
////////////////////////////////////////////////////////////////
uintptr_t
ListCtrlClass::Get_Entry_Data (int index, int col_index)
{
	uintptr_t user_data = 0;

	//
	//	Lookup the user data
	//
	if (index >= 0 && index < Get_Entry_Count () && col_index >= 0 && col_index < ColList.Count ()) {
		user_data = ColList[col_index]->Get_Entry_Data (index);
	}

	return user_data;
}


////////////////////////////////////////////////////////////////
//
//	Get_Entry_Text
//
////////////////////////////////////////////////////////////////
const unichar_t *
ListCtrlClass::Get_Entry_Text (int index, int col_index)
{
	//
	//	Return the string to the caller
	//
	return ColList[col_index]->Get_Entry_Text (index);
}


////////////////////////////////////////////////////////////////
//
//	Delete_All_Entries
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Delete_All_Entries (void)
{
	//
	//	Notify the advise sinks (if necessary) that each entry
	// is being deleted
	//
	int entry_count = Get_Entry_Count ();
	for (int item_index = 0; item_index < entry_count; item_index ++) {
		ADVISE_NOTIFY (On_ListCtrl_Delete_Entry (this, Get_ID (), item_index));
	}

	//
	//	Delete each of our row information sturctures
	//
	int index;
	for (index = 0; index < RowInfoList.Count (); index ++) {
		delete RowInfoList[index];
	}
	RowInfoList.Delete_All ();

	//
	//	Now delete all the entries from each column
	//
	for (index = 0; index < ColList.Count (); index ++) {
		ColList[index]->Delete_All_Entries ();
	}

	Set_Dirty ();

	//
	//	Reset the scroll and current selection positions
	//
	ScrollPos	= 0;
	CurrSel		= -1;
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Scroll_To_End
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Scroll_To_End (void)
{
	//
	//	Update the last page top entry
	//
	LastPageTopEntryIndex = Find_Last_Page_Top_Entry ();

	//
	//	Force scroll to the end
	//
	ScrollPos = LastPageTopEntryIndex;
	ScrollPos = std::max (ScrollPos, 0);

	//
	//	Update the scrollbar
	//
	ScrollBarCtrl.Set_Pos (ScrollPos, false);

	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Curr_Sel
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Set_Curr_Sel (int new_sel)
{
	if (new_sel == -1) {
		Select_All(false);
		CurrSel = -1;
	} else {
		Set_Sel(new_sel, false);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Sel
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Set_Sel (int new_sel, bool notify)
{
	if (IsSelectionAllowed) {

		//
		//	Unselect the old entry (if necessary)
		//
		if (IsMultipleSelection == false) {
			Select_Entry (CurrSel, false);
		}

		int old_sel = CurrSel;

		if ((new_sel == -1) && IsNoSelectionAllowed) {
			CurrSel = -1;
		} else {
			//
			//	Bound the selection index
			//
			int count = Get_Entry_Count ();
			CurrSel = std::max (new_sel, 0);
			CurrSel = std::min (CurrSel, count - 1);

			//
			//	Select the new entry
			//
			if (IsMultipleSelection) {
				Toggle_Entry_Selection (CurrSel);
			} else {
				Select_Entry (CurrSel, true);
			}
		}

		//
		//	Notify anyone who cares that we have changed the selection
		//
		if (notify && (old_sel != CurrSel)) {
			ADVISE_NOTIFY(On_ListCtrl_Sel_Change(this, Get_ID(), old_sel, CurrSel));
		}

		//
		//	Force a repaint
		//
		Set_Dirty ();
		Update_Scroll_Pos ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Scroll_Pos
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Update_Scroll_Pos (void)
{
	if (CurrSel < 0) {
		return ;
	}

	//
	//	Do we need to scroll up?
	//
	if (CurrSel < ScrollPos) {
		ScrollPos = CurrSel;
		Set_Dirty ();
	} else {

		//
		//	Do we need to scroll down?
		//
		RectClass rect;
		Get_Entry_Rect (CurrSel, rect);
		if (rect.Bottom >= TextRect.Bottom) {

			//
			//	Calculate where we should scroll to
			//
			ScrollPos = Find_Top_Of_Page (CurrSel);
			Set_Dirty ();
		}
	}

	//
	//	Update the scrollbar
	//
	ScrollBarCtrl.Set_Pos (ScrollPos, false);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Entry_Rect
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Get_Entry_Rect (int index, RectClass &rect)
{
	if (index < 0 || index >= RowInfoList.Count ()) {
		return ;
	}

	//
	//	Lookup the height of this row
	//
	float row_height	= RowInfoList[index]->Get_Height ();
	float y_pos			= -1000.0F;

	if (ScrollPos <= index) {

		//
		//	Calculate the starting y-position of this entry
		//
		y_pos = TextRect.Top;
		for (int curr_index = ScrollPos; curr_index < index; curr_index ++) {
			y_pos += RowInfoList[curr_index]->Get_Height ();
		}
	}

	rect.Left	= int(TextRect.Left);
	rect.Right	= int(TextRect.Right);
	rect.Top		= int(y_pos);
	rect.Bottom	= int(rect.Top + row_height);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Col_From_Pos
//
////////////////////////////////////////////////////////////////
int
ListCtrlClass::Col_From_Pos (const Vector2 &mouse_pos)
{
	int retval = -1;

	//
	//	Test each column
	//
	int x_pos		= HeaderRect.Left;
	int col_count	= ColList.Count ();
	for (int col_index = 0; col_index < col_count; col_index ++) {

		//
		//	Determine how wide this column is
		//
		int col_width = (ColList[col_index]->Get_Width () * HeaderRect.Width ());

		//
		//	Let the last column extend to the edge
		//
		if (col_index == col_count - 1) {
			col_width = HeaderRect.Right - x_pos;
		}


		//
		//	Is the coordinate inside this col?
		//
		if (mouse_pos.X >= x_pos && mouse_pos.X <= (x_pos + col_width)) {
			retval = col_index;
			break;
		}

		//
		//	Move on to the next column
		//
		x_pos += col_width;
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Entry_From_Pos
//
////////////////////////////////////////////////////////////////
int
ListCtrlClass::Entry_From_Pos (const Vector2 &mouse_pos)
{
	int retval = -1;
	if (mouse_pos.Y < TextRect.Top || mouse_pos.Y > TextRect.Bottom) {
		return -1;
	}

	//
	//	Test each row
	//
	int y_pos		= TextRect.Top;
	int row_count	= Get_Entry_Count ();
	for (int row_index = ScrollPos; row_index < row_count; row_index ++) {

		float row_height = RowInfoList[row_index]->Get_Height ();

		//
		//	Is the coordinate inside this row?
		//
		if (mouse_pos.Y >= y_pos && mouse_pos.Y <= (y_pos + row_height)) {
			retval = row_index;
			break;
		}

		//
		//	Move down to the next row
		//
		y_pos += row_height;

		//
		//	Stop searching if we've moved off the page
		//
		if (mouse_pos.Y >= TextRect.Bottom) {			
			break;
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Scroll_Page
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::Scroll_Page (int direction)
{	
	int count		= RowInfoList.Count ();
	float height	= TextRect.Height ();

	bool found = false;

	//
	//	Scan either direction from the current scroll
	// position until we've moved a whole page
	//
	int index;
	for (	index = ScrollPos;
			index >= 0 && index < count;
			index += direction)
	{		
		//
		//	Decrement the remaining distance
		//
		height -= RowInfoList[index]->Get_Height ();

		//
		//	If we've gone of the page, then back off one entry
		//
		if (height < 0) {
			ScrollPos = (index - direction);
			ScrollPos = std::min (ScrollPos, LastPageTopEntryIndex);
			ScrollBarCtrl.Set_Pos (ScrollPos, false);
			Set_Dirty ();
			found = true;
			break;
		}
	}

	//
	//	Check the boundary conditions
	//
	if (index < 0 && found == false) {
		ScrollPos = 0;
		ScrollBarCtrl.Set_Pos (ScrollPos, false);
		Set_Dirty ();		
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_VScroll_Page
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::On_VScroll_Page (ScrollBarCtrlClass * /* scrollbar */, int /* ctrl_id */, int direction)
{
	Scroll_Page (direction);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_VScroll
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::On_VScroll (ScrollBarCtrlClass *, int , int new_position)
{
	if (ScrollPos != new_position) {
		ScrollPos = new_position;
		Set_Dirty ();
	}
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Mouse_Wheel
//
////////////////////////////////////////////////////////////////
void
ListCtrlClass::On_Mouse_Wheel (int direction)
{
	if (direction < 0) {
		if (ScrollPos > 0) {
			ScrollPos --;
			ScrollBarCtrl.Set_Pos (ScrollPos, false);
		}

	} else {
		if (ScrollPos < LastPageTopEntryIndex) {
			ScrollPos ++;
			ScrollBarCtrl.Set_Pos (ScrollPos, false);
		}
	}

	Set_Dirty ();
	return ;
}


//********************************************************************************//
//
//	Start of ListColumnClass
//
//********************************************************************************//


////////////////////////////////////////////////////////////////
//
//	Free_Data
//
////////////////////////////////////////////////////////////////
void
ListColumnClass::Free_Data (void)
{
	Delete_All_Entries ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Reset_Contents
//
////////////////////////////////////////////////////////////////
void
ListColumnClass::Reset_Contents (void)
{
	//
	//	Remove all the entries
	//
	Free_Data ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Insert_Entry
//
////////////////////////////////////////////////////////////////
int
ListColumnClass::Insert_Entry (int index, const unichar_t *entry_name)
{
	ListEntryClass *entry = new ListEntryClass (entry_name);

	//
	//	Should we insert this entry  in the list or add it to the end?
	//
	if (index < EntryList.Count ()) {
		EntryList.Insert (index + 1, entry);
	} else {
		EntryList.Add (entry);
		index = (EntryList.Count () - 1);
	}

	return index;
}


////////////////////////////////////////////////////////////////
//
//	Delete_Entry
//
////////////////////////////////////////////////////////////////
bool
ListColumnClass::Delete_Entry (int index)
{
	bool retval = false;

	//
	//	Delete the entry if we can find it in our list
	//
	if (index >= 0 && index < EntryList.Count ()) {
		delete EntryList[index];
		EntryList.Delete (index);
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Delete_All_Entries
//
////////////////////////////////////////////////////////////////
void
ListColumnClass::Delete_All_Entries (void)
{
	//
	//	Free each of the entries  in the list
	//
	for (int index = 0; index < EntryList.Count (); index ++) {
		delete EntryList[index];
	}

	EntryList.Delete_All ();	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Move_Entry
//
////////////////////////////////////////////////////////////////
void
ListColumnClass::Move_Entry (int old_index, int new_index)
{
	if (	old_index < 0 || (old_index >= EntryList.Count ()) &&
			new_index < 0 || (new_index >= EntryList.Count ()))
	{
		return ;
	}

	//
	//	Move the entry
	//
	ListEntryClass *old_entry = EntryList[old_index];
	EntryList.Insert (new_index, old_entry);
	EntryList.Delete (old_index);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Swap_Entries
//
////////////////////////////////////////////////////////////////
void
ListColumnClass::Swap_Entries (int index1, int index2)
{
	if (	index1 < 0 || (index1 >= EntryList.Count ()) &&
			index2 < 0 || (index2 >= EntryList.Count ()))
	{
		return ;
	}

	//
	//	Move the entry
	//
	ListEntryClass *temp_entry = EntryList[index1];
	EntryList[index1] = EntryList[index2];
	EntryList[index2] = temp_entry;
	return ;
}

