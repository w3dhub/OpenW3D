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
 *                     $Archive:: /Commando/Code/wwui/dropdownctrl.cpp         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/07/02 10:42a                                              $*
 *                                                                                             *
 *                    $Revision:: 17                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dropdownctrl.h"
#include "comboboxctrl.h"
#include "dialogbase.h"
#include "assetmgr.h"
#include "refcount.h"
#include "font3d.h"
#include "mousemgr.h"
#include "ww3d.h"
#include "dialogmgr.h"
#include "stylemgr.h"
#include <algorithm>


////////////////////////////////////////////////////////////////
//
//	DropDownCtrlClass
//
////////////////////////////////////////////////////////////////
DropDownCtrlClass::DropDownCtrlClass (void)	:
	CellSize (0, 0),
	CurrSel (-1),
	ComboBox (NULL),
	ScrollPos (0),
	CountPerPage (0),
	FullRect (0, 0, 0, 0),
	DisplayScrollBar (false)
{

	//
	//	Set the font for the text renderers
	//
	StyleMgrClass::Assign_Font (&TextRenderer, StyleMgrClass::FONT_LISTS);

	//
	//	We don't want the scroll bar getting focus
	//
	ScrollBarCtrl.Set_Wants_Focus (false);
	ScrollBarCtrl.Set_Small_BMP_Mode (true);
	ScrollBarCtrl.Set_Advise_Sink (this);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~DropDownCtrlClass
//
////////////////////////////////////////////////////////////////
DropDownCtrlClass::~DropDownCtrlClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Text_Renderer
//
////////////////////////////////////////////////////////////////
void
DropDownCtrlClass::Create_Text_Renderer (void)
{
	HilightRenderer.Reset ();
	HilightRenderer.Set_Coordinate_Range (Render2DClass::Get_Screen_Resolution());
	StyleMgrClass::Configure_Hilighter (&HilightRenderer);

	//
	//	Setup the coordinate system of the renderer
	//
	TextRenderer.Reset ();

	//
	//	Add each string to the list
	//
	float curr_y_pos = ClientRect.Top;
	for (int index = ScrollPos; index < EntryList.Count (); index ++) {

		//
		//	Get the width and height of the current entry
		//
		const unichar_t *text		= EntryList[index].text;
		Vector2 text_extent	= TextRenderer.Get_Text_Extents (text);

		//
		//	Build a rectangle we'll draw the text into
		//
		RectClass text_rect;
		text_rect.Left		= ClientRect.Left;
		text_rect.Top		= curr_y_pos;
		text_rect.Right	= text_rect.Left + CellSize.X;
		text_rect.Bottom	= text_rect.Top + CellSize.Y;

		if (text_rect.Bottom <= ClientRect.Bottom) {

			//
			//	Draw the text
			//
			StyleMgrClass::Render_Text (text, &TextRenderer, text_rect, true, true);

			//
			//	Hilight this entry (if its the currently selected one)
			//
			if (index == CurrSel) {
				StyleMgrClass::Render_Hilight (&HilightRenderer, text_rect);
			}
		}

		//
		//	Move on to the next y-position
		//
		curr_y_pos += CellSize.Y;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Control_Renderer
//
////////////////////////////////////////////////////////////////
void
DropDownCtrlClass::Create_Control_Renderer (void)
{
	//
	//	Configure this renderer
	//
	ControlRenderer.Reset ();
	ControlRenderer.Enable_Texturing (false);
	ControlRenderer.Set_Coordinate_Range (Render2DClass::Get_Screen_Resolution());

	ControlRenderer.Add_Quad (FullRect, RGBA_TO_INT32 (0, 0, 0, 236));

	//
	//	Lookup the colors to use
	//
	int color	= StyleMgrClass::Get_Line_Color ();
	int bkcolor	= StyleMgrClass::Get_Bk_Color ();

	//
	//	Draw the control outline
	//
	RectClass rect = Rect;
	ControlRenderer.Add_Outline (rect, 1.0F, color);

	//
	//	Now draw the background
	//
	rect.Right	-= 1;
	rect.Bottom -= 1;
	ControlRenderer.Add_Quad (rect, bkcolor);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Cursor
//
////////////////////////////////////////////////////////////////
void
DropDownCtrlClass::On_Set_Cursor (const Vector2 &mouse_pos)
{
	//
	//	Change the mouse cursor if necessary
	//
	if (ClientRect.Contains (mouse_pos)) {
		MouseMgrClass::Set_Cursor (MouseMgrClass::CURSOR_ACTION);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Client_Rect
//
////////////////////////////////////////////////////////////////
void
DropDownCtrlClass::Update_Client_Rect (void)
{
	Rect = FullRect;

	//
	//	Determine what one character spacing would be
	//
	Vector2 char_size = TextRenderer.Get_Text_Extents (U_CHAR("W"));

	float border_width	= char_size.X + 2;
	float border_height	= 2;

	//
	//	Shrink the client area
	//
	ClientRect = Rect;
	ClientRect.Inflate (Vector2 (-border_width, -border_height));

	//
	//	Calculate how each "text" cell should be
	//
	CellSize.X = int(ClientRect.Width ());
	CellSize.Y = int(char_size.Y * 1.5F);

	//
	//	Update the number of entries we can display at one time
	//
	CountPerPage = int(ClientRect.Height () / CellSize.Y);

	//
	//	Do we need to show a scroll bar?
	//
	if (CountPerPage < EntryList.Count ()) {

		float width = (char_size.X * 3);

		//
		//	Calculate the width of the scroll bar
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
		ScrollBarCtrl.Set_Page_Size (CountPerPage - 1);
		ScrollBarCtrl.Set_Range (0, EntryList.Count () - CountPerPage);
		DisplayScrollBar = true;

		//
		//	Shrink the client area
		//
		float new_width	= ScrollBarCtrl.Get_Window_Rect ().Left;
		ClientRect.Right	= new_width;
		Rect.Right			= new_width;
		CellSize.X			= int(ClientRect.Width ());
	}

	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
DropDownCtrlClass::Render (void)
{
	//
	//	Recreate the renderers (if necessary)
	//
	if (IsDirty) {
		Create_Control_Renderer ();
		Create_Text_Renderer ();
	}

	//
	//	Render the background and text for the current state
	//
	ControlRenderer.Render ();
	TextRenderer.Render ();
	HilightRenderer.Render ();

	DialogControlClass::Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Down
//
////////////////////////////////////////////////////////////////
void
DropDownCtrlClass::On_LButton_Down (const Vector2 &mouse_pos)
{
	Set_Curr_Sel (Entry_From_Pos (mouse_pos));
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Up
//
////////////////////////////////////////////////////////////////
void
DropDownCtrlClass::On_LButton_Up (const Vector2 &mouse_pos)
{
	Set_Curr_Sel (Entry_From_Pos (mouse_pos));

	if (ComboBox) {
		ComboBox->On_Drop_Down_End (CurrSel);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Mouse_Move
//
////////////////////////////////////////////////////////////////
void
DropDownCtrlClass::On_Mouse_Move (const Vector2 &mouse_pos)
{
	if (DialogMgrClass::Is_Button_Down (VK_LBUTTON)) {
		Set_Curr_Sel (Entry_From_Pos (mouse_pos));
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Key_Down
//
////////////////////////////////////////////////////////////////
bool
DropDownCtrlClass::On_Key_Down (uint32 key_id, uint32 /* key_data */)
{
	bool handled = true;

	switch (key_id)
	{
		case VK_HOME:
			Set_Curr_Sel (0);
			break;

		case VK_END:
			Set_Curr_Sel (EntryList.Count () - 1);
			break;

		case VK_UP:
		case VK_LEFT:
			Set_Curr_Sel (CurrSel - 1);
			break;

		case VK_DOWN:
		case VK_RIGHT:
			Set_Curr_Sel (CurrSel + 1);
			break;

		default:
			handled = false;
			break;
	}

	return handled;
}


////////////////////////////////////////////////////////////////
//
//	On_Create
//
////////////////////////////////////////////////////////////////
void
DropDownCtrlClass::On_Create (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Kill_Focus
//
////////////////////////////////////////////////////////////////
void
DropDownCtrlClass::On_Kill_Focus (DialogControlClass * /* focus */)
{
	//
	//	Noitify the combobox that we are done
	//
	if (ComboBox) {
		ComboBox->On_Drop_Down_End (CurrSel);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Add_String
//
////////////////////////////////////////////////////////////////
int
DropDownCtrlClass::Add_String (const unichar_t* string)
{
	if (string == NULL) {
		return -1;
	}

	//
	//	Add the entry to the list
	//
	EntryList.Add (ENTRY (string, 0));

	//
	//	Force scrollbars, etc to be recalculated and painted
	//
	Update_Client_Rect ();
	Set_Dirty ();

	return (EntryList.Count () - 1);
}


////////////////////////////////////////////////////////////////
//
//	Delete_String
//
////////////////////////////////////////////////////////////////
void
DropDownCtrlClass::Delete_String (int index)
{
	if (index >= 0 && index < EntryList.Count ()) {

		//
		//	Delete the entry
		//
		EntryList.Delete (index);
		Set_Curr_Sel (std::min (CurrSel, EntryList.Count () - 1));
		Set_Dirty ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Find_String
//
////////////////////////////////////////////////////////////////
int
DropDownCtrlClass::Find_String (const unichar_t* string)
{
	int retval = -1;

	if (string) {
		for (int index = 0; index < EntryList.Count (); index ++) {

			//
			//	If this is the entry the user is requesting, then
			// return its index to the caller
			//
			if (EntryList[index].text.Compare_No_Case (string) == 0) {
				retval = index;
				break;
			}
		}
	}

	return retval;
}


int DropDownCtrlClass::Find_Closest_String(const unichar_t* string)
{
	int retval = -1;

	if (string && (u_strlen(string) > 0)) {
		for (int index = 0; index < EntryList.Count (); index ++) {
			int relation = EntryList[index].text.Compare_No_Case(string);

			if (relation == 0) {
				return index;
			} else if (relation < 0) {
				return index;
			}
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Set_Item_Data
//
////////////////////////////////////////////////////////////////
void
DropDownCtrlClass::Set_Item_Data (int index, uintptr_t data)
{
	//
	//	Index into the list and set the user data
	//
	if (index >= 0 && index < EntryList.Count ()) {
		EntryList[index].user_data = data;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Item_Data
//
////////////////////////////////////////////////////////////////
uintptr_t
DropDownCtrlClass::Get_Item_Data (int index)
{
	uintptr_t retval = 0;

	//
	//	Index into the list and return the user data to the caller
	//
	if (index >= 0 && index < EntryList.Count ()) {
		retval = EntryList[index].user_data;
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Reset_Content
//
////////////////////////////////////////////////////////////////
void
DropDownCtrlClass::Reset_Content (void)
{
	//
	//	Remove everything from the list
	//
	EntryList.Delete_All ();
	Set_Curr_Sel (0);

	//
	//	Repaint the control
	//
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Entry_From_Pos
//
////////////////////////////////////////////////////////////////
int
DropDownCtrlClass::Entry_From_Pos (const Vector2 &mouse_pos)
{
	int retval = 0;

	//
	//	Loop over all the entries in our current view
	//
	float curr_y_pos = ClientRect.Top;
	for (int index = ScrollPos; index < EntryList.Count (); index ++) {
		retval = index;
		
		//
		//	Is ths mouse over this entry?
		//		
		if (	(mouse_pos.Y >= curr_y_pos &&
				 mouse_pos.Y <= (curr_y_pos + CellSize.Y)) ||
				mouse_pos.Y > ClientRect.Bottom)
		{
			break;
		}

		//
		//	Move on to the next y-position
		//
		curr_y_pos += CellSize.Y;
	}
	
	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Set_Curr_Sel
//
////////////////////////////////////////////////////////////////
void
DropDownCtrlClass::Set_Curr_Sel (int index)
{
	if (index >= -1 && index < EntryList.Count () && CurrSel != index) {
		
		//
		//	Change the selection
		//
		CurrSel = index;

		//
		//	Update our current scroll position
		//
		Update_Scroll_Pos ();

		//
		//	Repaint the view
		//
		Set_Dirty ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_String
//
////////////////////////////////////////////////////////////////
bool
DropDownCtrlClass::Get_String (int index, WideStringClass &string) const
{
	bool retval = false;

	if (index >= 0 && index < EntryList.Count ()) {
		
		//
		//	Index into the entry list and return the string
		//
		string = EntryList[index].text;
		retval = true;
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Get_String
//
////////////////////////////////////////////////////////////////
const unichar_t *
DropDownCtrlClass::Get_String (int index) const
{
	const unichar_t *retval = NULL;

	if (index >= 0 && index < EntryList.Count ()) {
		
		//
		//	Index into the entry list and return the string
		//
		retval = EntryList[index].text;
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Update_Scroll_Pos
//
////////////////////////////////////////////////////////////////
void
DropDownCtrlClass::Update_Scroll_Pos (void)
{
	if (CurrSel == -1) {
		return;
	}

	if (CurrSel < ScrollPos) {
		
		//
		//	Scroll up so the current selection is in view
		//
		ScrollPos = CurrSel;
		Set_Dirty ();
	} else if (CurrSel >= ScrollPos + CountPerPage) {

		//
		//	Scroll down so the current selection is in view
		//
		ScrollPos = std::max (CurrSel - (CountPerPage - 1), 0);
		Set_Dirty ();
	}

	//
	//	Update the scrollbar
	//
	ScrollBarCtrl.Set_Pos (ScrollPos, false);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Add_To_Dialog
//
////////////////////////////////////////////////////////////////
void
DropDownCtrlClass::On_Add_To_Dialog (void)
{
	if (DisplayScrollBar) {
		Parent->Add_Control (&ScrollBarCtrl);
	}

	TextColor.Set (0.35F, 1.0F, 0.35F);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Remove_From_Dialog
//
////////////////////////////////////////////////////////////////
void
DropDownCtrlClass::On_Remove_From_Dialog (void)
{
	if (DisplayScrollBar) {
		Parent->Remove_Control (&ScrollBarCtrl);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_VScroll
//
////////////////////////////////////////////////////////////////
void
DropDownCtrlClass::On_VScroll (ScrollBarCtrlClass *, int , int new_position)
{
	ScrollPos = new_position;
	Set_Dirty ();	
	return ;
}
