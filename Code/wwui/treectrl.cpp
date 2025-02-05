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
 *                     $Archive:: /Commando/Code/wwui/treectrl.cpp                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/08/01 5:56p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "treectrl.h"
#include "assetmgr.h"
#include "refcount.h"
#include "font3d.h"
#include "mousemgr.h"
#include "ww3d.h"
#include "dialogmgr.h"
#include "dialogbase.h"
#include "stylemgr.h"


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
static const char *PLUS_IMAGE		= "if_treeplus.tga";

static const RectClass PLUS_RECT		= RectClass (0, 0, 16, 16);
static const RectClass MINUS_RECT	= RectClass (16, 0, 32, 16);

////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
TREECTRL_SORT_CALLBACK	TreeCtrlClass::CurrentSortCallback	= NULL;
TreeCtrlClass *			TreeCtrlClass::CurrentSorter			= NULL;
uint32						TreeCtrlClass::CurrentSortUserData	= 0;
const char *				TreeCtrlClass::ICON_FOLDER				= "if_treefld.tga";
const char *				TreeCtrlClass::ICON_FOLDER_OPEN		= "if_treefld_open.tga";


////////////////////////////////////////////////////////////////
//
//	TreeCtrlClass
//
////////////////////////////////////////////////////////////////
TreeCtrlClass::TreeCtrlClass (void)	:
	SelectedItem (NULL),
	ScrollPos (0),
	RowHeight (0),
	RowsPerPage (0),
	IsScrollBarDisplayed (false)
{
	//
	//	Set the font for the text renderers
	//
	StyleMgrClass::Assign_Font (&TextRenderer, StyleMgrClass::FONT_LISTS);
	StyleMgrClass::Configure_Renderer (&ControlRenderer);
	StyleMgrClass::Configure_Renderer (&PlusRenderer);	
	StyleMgrClass::Configure_Renderer (&IconRenderer);		
	HilightRenderer.Set_Coordinate_Range (Render2DClass::Get_Screen_Resolution());		
	StyleMgrClass::Configure_Hilighter (&HilightRenderer);
	
	//
	//	Load the "plus" texture
	//
	TextureClass *texture = WW3DAssetManager::Get_Instance ()->Get_Texture (PLUS_IMAGE, TextureClass::MIP_LEVELS_1);
	if (texture != NULL) {
		PlusRenderer.Set_Texture (texture);
		REF_PTR_RELEASE (texture);
	}

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
//	~TreeCtrlClass
//
////////////////////////////////////////////////////////////////
TreeCtrlClass::~TreeCtrlClass (void)
{
	ScrollBarCtrl.Set_Advise_Sink (NULL);	
	
	if (Parent != NULL) {
		Parent->Remove_Control (&ScrollBarCtrl);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Text_Renderers
//
////////////////////////////////////////////////////////////////
void
TreeCtrlClass::Create_Text_Renderers (void)
{
	//
	//	Configure the renderers
	//
	HilightRenderer.Reset ();
	TextRenderer.Reset ();
	IconRenderer.Reset ();
	PlusRenderer.Reset ();
	IconMgr.Reset_Renderers ();

	float x_pos = ClientRect.Left;
	float y_pos = ClientRect.Top;

	int row_index	= 0;
	int level		= 0;

	//
	//	Render each row...
	//
	for (int index = 0; index < ItemList.Count (); index ++) {
		TreeItemClass *item = ItemList[index];	
		if (Render_Item (item, x_pos, y_pos, row_index, level) == false) {
			break;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render_Item
//
////////////////////////////////////////////////////////////////
bool
TreeCtrlClass::Render_Item (TreeItemClass *item, float x_pos, float &y_pos, int &row_index, int level)
{
	bool retval = true;
	if (item == NULL) {
		return false;
	}

	//
	//	Determine which color to draw the outline in
	//
	int color = StyleMgrClass::Get_Line_Color ();
	if (IsEnabled == false) {
		color	= StyleMgrClass::Get_Disabled_Line_Color ();
	}

	const float LINE_START		= 16.0F * StyleMgrClass::Get_X_Scale ();
	const float LINE_SPACING	= 16.0F * StyleMgrClass::Get_X_Scale ();
	const float TEXT_SPACING	= 4.0F * StyleMgrClass::Get_X_Scale ();

	//
	//	Build the display rectangles
	//
	RectClass rect;
	rect.Left	= ClientRect.Left + LINE_START + int(level * LINE_SPACING);
	rect.Top		= y_pos;
	rect.Right	= ClientRect.Right;	
	rect.Bottom	= y_pos + RowHeight;

	RectClass plus_rect;
	plus_rect.Left		= int(rect.Left - LINE_SPACING);
	plus_rect.Top		= int(rect.Center ().Y - (LINE_SPACING * 0.5F));
	plus_rect.Right	= int((plus_rect.Left + LINE_SPACING));
	plus_rect.Bottom	= int(rect.Center ().Y + (LINE_SPACING * 0.5F));

	RectClass icon_rect;
	icon_rect.Left		= rect.Left;
	icon_rect.Top		= rect.Top;
	icon_rect.Right	= icon_rect.Left + LINE_SPACING;
	icon_rect.Bottom	= rect.Bottom;

	RectClass text_rect;
	text_rect.Left		= icon_rect.Right + TEXT_SPACING;
	text_rect.Top		= rect.Top;
	text_rect.Right	= rect.Right;
	text_rect.Bottom	= rect.Bottom;

	//
	//	Only draw the entry if its inside the view
	//
	if (row_index >= ScrollPos && ((y_pos + RowHeight) <= ClientRect.Bottom)) {
				
		//
		//	Draw the tick to the left of the entry
		//
		if (level > 0) {
			float half_rect = rect.Height () / 2;
			Vector2 line_start (icon_rect.Left - (LINE_SPACING * 0.5F), rect.Top + half_rect);
			Vector2 line_end (icon_rect.Left, rect.Top + half_rect);
			ControlRenderer.Add_Line (line_start, line_end, 1.0F, color);
		}

		//
		//	Draw the plus (if necessary)
		//
		if ((item->Get_Child_Count () > 0 || item->Needs_Children ())) {

			RectClass plus_uv_rect = PLUS_RECT;
			if (item->Is_Expanded ()) {
				plus_uv_rect = MINUS_RECT;
			}

			plus_uv_rect.Inverse_Scale (Vector2 (32.0F, 32.0F));
			PlusRenderer.Add_Quad (plus_rect, plus_uv_rect);
		}

		//
		//	Render the icon
		//
		const char *icon_name = item->Get_Icon ();
		if (SelectedItem == item) {
			icon_name = item->Get_Selected_Icon ();
		}
		if (icon_name != NULL && icon_name[0] != 0) {
			IconMgr.Render_Icon (icon_rect, icon_name);
		}

		//
		//	Draw the text
		//
		StyleMgrClass::Render_Text (item->Get_Name (), &TextRenderer, text_rect, true, true,
								StyleMgrClass::LEFT_JUSTIFY, IsEnabled);


		//
		//	Adjust the width of the rectangle so its the approximate size
		// of the text
		//
		float width = TextRenderer.Get_Text_Extents (item->Get_Name ()).X;
		text_rect.Right = text_rect.Left + width + TEXT_SPACING;

		//
		//	Is this the selected item?
		//
		if (SelectedItem == item) {
			
			if (HasFocus) {
				RectClass hilight_rect = text_rect;
				hilight_rect.Right	-= 1.0F;
				hilight_rect.Bottom	-= 1.0F;
				StyleMgrClass::Render_Hilight (&HilightRenderer, hilight_rect);
			}
			ControlRenderer.Add_Outline (text_rect, 1.0F, color);
		}

		//
		//	Move down one row
		//
		y_pos += RowHeight;
	}

	row_index ++;
	retval = ((y_pos + RowHeight) <= ClientRect.Bottom);

	//
	//	Should we render the children?
	//
	if (item->Is_Expanded ()) {

		float start_y_pos		= y_pos;
		int start_row_index	= max (row_index, ScrollPos);
		int end_row_index		= start_row_index;

		//
		//	Render each child item
		//
		int child_count = item->Get_Child_Count ();
		for (int index = 0; index < child_count; index ++) {
			end_row_index	= row_index + 1;
			retval = Render_Item (item->Get_Child (index), x_pos, y_pos, row_index, level + 1);
		}		

		//
		//	Render the line connecting this item to all its children
		//
		if (	end_row_index != start_row_index &&
				end_row_index >= ScrollPos &&
				start_row_index <= (ScrollPos + RowsPerPage))
		{
			float end_y_pos = start_y_pos + (end_row_index - start_row_index) * RowHeight;
			
			//
			//	Clip the line start and end to the window
			//
			end_y_pos	= min (end_y_pos, ClientRect.Top + (RowsPerPage * RowHeight));
			start_y_pos	= max (start_y_pos, ClientRect.Top);

			//
			//	Render the line
			//
			if (start_y_pos < end_y_pos) {
				float icon_x_pos = icon_rect.Center ().X;
				Vector2 line_start (icon_x_pos,	start_y_pos);
				Vector2 line_end (icon_x_pos,		end_y_pos - (RowHeight * 0.5F));
				ControlRenderer.Add_Line (line_start, line_end, 1.0F, color);
			}
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Set_Scroll_Pos
//
////////////////////////////////////////////////////////////////
void
TreeCtrlClass::Set_Scroll_Pos (int new_pos)
{
	//
	//	Ensure this scroll position doesn't extend out of bounds
	//
	int visible_rows	= Count_Visible_Rows ();
	new_pos				= min (visible_rows - RowsPerPage, new_pos);
	new_pos				= max (new_pos, 0);

	//
	//	Did anything change?
	//
	if (ScrollPos != new_pos) {
		ScrollPos = new_pos;
		ScrollBarCtrl.Set_Pos (ScrollPos, false);
		Set_Dirty ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Count_Visible_Rows
//
////////////////////////////////////////////////////////////////
int
TreeCtrlClass::Count_Visible_Rows (void)
{
	int total = 0;

	//
	//	Count each row
	//
	for (int index = 0; index < ItemList.Count (); index ++) {
		TreeItemClass *item = ItemList[index];

		//
		//	Count this item and it's children
		//
		total += Count_Visible_Rows (item);
	}
	
	return total;
}


////////////////////////////////////////////////////////////////
//
//	Count_Visible_Rows
//
////////////////////////////////////////////////////////////////
int
TreeCtrlClass::Count_Visible_Rows (TreeItemClass *item)
{
	if (item == NULL) {
		return 0;
	}
	
	int total = 1;

	//
	//	Should we count the children?
	//
	if (item->Is_Expanded ()) {

		//
		//	Count each child
		//
		int child_count = item->Get_Child_Count ();
		for (int index = 0; index < child_count; index ++) {
			total += Count_Visible_Rows (item->Get_Child (index));
		}
	}

	return total;
}


////////////////////////////////////////////////////////////////
//
//	Create_Control_Renderers
//
////////////////////////////////////////////////////////////////
void
TreeCtrlClass::Create_Control_Renderers (void)
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
	//	Draw the outline
	//
	renderer.Add_Rect (Rect, 1.0F, color, bkcolor);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Cursor
//
////////////////////////////////////////////////////////////////
void
TreeCtrlClass::On_Set_Cursor (const Vector2 &mouse_pos)
{
	//
	//	Change the mouse cursor
	//
	MouseMgrClass::Set_Cursor (MouseMgrClass::CURSOR_ACTION);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Client_Rect
//
////////////////////////////////////////////////////////////////
void
TreeCtrlClass::Update_Client_Rect (void)
{
	const float BORDER_SIZE_X	= 4.0F * StyleMgrClass::Get_X_Scale ();
	const float BORDER_SIZE_Y	= 4.0F * StyleMgrClass::Get_Y_Scale ();

	//
	//	Set the client area
	//
	ClientRect = Rect;
	ClientRect.Inflate (Vector2 (-BORDER_SIZE_X, -BORDER_SIZE_Y));

	//
	//	Calculate how tall each row should be...
	//
	const float ROW_SPACING	= 4.0F;
	float border_height		= (ROW_SPACING * StyleMgrClass::Get_Y_Scale ());
	RowHeight					= (TextRenderer.Get_Text_Extents (L"W").Y + border_height);

	//
	//	Determine how many rows we can fit on a page
	//
	RowsPerPage = ClientRect.Height () / RowHeight;
	ClientRect.Top		= int(Rect.Center ().Y - (RowsPerPage * RowHeight * 0.5F));
	ClientRect.Bottom	= int(Rect.Center ().Y + (RowsPerPage * RowHeight * 0.5F));

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

	Update_Scroll_Bar_Visibility ();
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_VScroll
//
////////////////////////////////////////////////////////////////
void
TreeCtrlClass::On_VScroll (ScrollBarCtrlClass *, int , int new_position)
{
	Set_Scroll_Pos (new_position);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
TreeCtrlClass::Render (void)
{
	//
	//	Recreate the renderers (if necessary)
	//
	if (IsDirty) {		
		Create_Control_Renderers ();
		Create_Text_Renderers ();
	}

	//
	//	Render the background and text for the current state
	//	
	IconMgr.Render_Icons ();
	TextRenderer.Render ();
	ControlRenderer.Render ();
	PlusRenderer.Render ();
	HilightRenderer.Render ();	

	DialogControlClass::Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Sort_Children_Alphabetically
//
////////////////////////////////////////////////////////////////
void
TreeCtrlClass::Sort_Children_Alphabetically (TreeItemClass *parent)
{
	Sort_Children (parent, Alphabetic_Sort_Callback, 0);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Sort_Children
//
////////////////////////////////////////////////////////////////
void
TreeCtrlClass::Sort_Children (TreeItemClass *parent, TREECTRL_SORT_CALLBACK sort_callback, uint32 user_data)
{
	CurrentSortCallback	= sort_callback;
	CurrentSorter			= this;
	CurrentSortUserData	= user_data;

	if (parent == NULL) {

		//
		//	Sort the root-level entries
		//
		if (ItemList.Count () != 0) {
			::qsort (&ItemList[0], ItemList.Count (), sizeof (ItemList[0]), Sort_Callback);
		}

	} else {
		
		TREE_ITEM_LIST &child_list = parent->Get_Child_List ();

		//
		//	Sort the immediate children of this item
		//
		if (child_list.Count () != 0) {
			::qsort (&child_list[0], child_list.Count (), sizeof (child_list[0]), Sort_Callback);
		}
	}


	return ;
}


////////////////////////////////////////////////////////////////
//
//	Sort_Callback
//
////////////////////////////////////////////////////////////////
int __cdecl
TreeCtrlClass::Sort_Callback (const void *elem1, const void *elem2)
{
   WWASSERT (elem1 != NULL);
   WWASSERT (elem2 != NULL);
   TreeItemClass *item1 = *((TreeItemClass **)elem1);
   TreeItemClass *item2 = *((TreeItemClass **)elem2);

	//
	//	Pass the sorting off to the user... (NOT THREAD SAFE)
	//	
	return CurrentSortCallback (CurrentSorter, item1, item2, CurrentSortUserData);
}


////////////////////////////////////////////////////////////////
//
//	Alphabetic_Sort_Callback
//
////////////////////////////////////////////////////////////////
int CALLBACK
TreeCtrlClass::Alphabetic_Sort_Callback
(
	TreeCtrlClass *	tree_ctrl,
	TreeItemClass *	item1,
	TreeItemClass *	item2,
	uint32				user_param
)
{
	//
	//	Sort by name
	//
	const WCHAR *name1 = item1->Get_Name ();
	const WCHAR *name2 = item2->Get_Name ();
	int result = ::CompareStringW (LOCALE_USER_DEFAULT, NORM_IGNORECASE, name1, -1, name2, -1);

	int retval = 0;
	if (result == CSTR_LESS_THAN) {
		retval = -1;
	} else if (result == CSTR_GREATER_THAN) {
		retval = 1;
	}
	
	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Find_Top_Item
//
////////////////////////////////////////////////////////////////
TreeItemClass *
TreeCtrlClass::Find_Top_Item (void)
{
	TreeItemClass *retval = NULL;

	//
	//	Count visible items until we've hit the current scroll position
	//
	retval = ItemList.Count () > 0 ? ItemList[0] : NULL;
	for (int index = 0; retval != NULL && index < ScrollPos; index ++) {
		retval = Find_Next_Visible (retval);
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Find_Last_Visible_Item
//
////////////////////////////////////////////////////////////////
TreeItemClass *
TreeCtrlClass::Find_Last_Visible_Item (void)
{
	//
	//	Determine where to start...
	//
	TreeItemClass *curr_item = NULL;
	if (ItemList.Count () > 0) {
		curr_item = ItemList[ItemList.Count () - 1];
	}

	//
	//	Keep searching until we stop finding new visible items...
	//
	while (curr_item != NULL) {
		TreeItemClass *next_item = Find_Next_Visible (curr_item);
		if (next_item == curr_item) {
			break;
		}
		curr_item = next_item;
	}

	return curr_item;
}


////////////////////////////////////////////////////////////////
//
//	Hit_Test
//
////////////////////////////////////////////////////////////////
TreeItemClass *
TreeCtrlClass::Hit_Test (const Vector2 &mouse_pos, HITTYPE &type)
{
	TreeItemClass *retval = NULL;

	//
	//	Attempt to locate the first visible item
	//
	TreeItemClass *hit_item = Find_Top_Item ();
	if (hit_item != NULL) {
		
		//
		//	Determine on which row of the screen the user clicked
		//
		int row_index = (mouse_pos.Y - ClientRect.Top) / RowHeight;

		//
		//	Now move down that number of items...
		//
		for (int index = 0; hit_item != NULL && index < row_index; index ++) {
			hit_item = Find_Next_Visible (hit_item);
		}
		
		const float LINE_START		= 16.0F * StyleMgrClass::Get_X_Scale ();
		const float LINE_SPACING	= 16.0F * StyleMgrClass::Get_X_Scale ();
		const float TEXT_SPACING	= 4.0F * StyleMgrClass::Get_X_Scale ();
		
		//
		//	Now determine on what part of the item the user clicked...
		//
		if (hit_item != NULL) {
			int level = hit_item->Get_Indent_Level ();
			float icon_left = ClientRect.Left + LINE_START + int(level * LINE_SPACING);
			float plus_left = icon_left - LINE_SPACING;
			float text_left = icon_left + LINE_SPACING + TEXT_SPACING;

			//
			//	Return the hit type and hit item to the caller
			//
			if (mouse_pos.X >= plus_left && mouse_pos.X < icon_left) {
				type		= HIT_PLUS;
				retval	= hit_item;
			} else if (mouse_pos.X >= icon_left && mouse_pos.X < text_left) {
				type		= HIT_ICON;
				retval	= hit_item;
			} else if (mouse_pos.X >= text_left) {
				type		= HIT_TEXT;
				retval	= hit_item;
			}
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Down
//
////////////////////////////////////////////////////////////////
void
TreeCtrlClass::On_LButton_Down (const Vector2 &mouse_pos)
{
	//
	//	Find which item hit clicked on
	//
	HITTYPE type;
	TreeItemClass *tree_item = Hit_Test (mouse_pos, type);
	if (tree_item != NULL) {
		SelectedItem = tree_item;
		
		//
		//	Toggle the expanded state if the user clicked on the
		// plus
		//
		if (type == HIT_PLUS) {
			tree_item->Expand (!tree_item->Is_Expanded ());
		}
		Set_Dirty ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_DblClk
//
////////////////////////////////////////////////////////////////
void
TreeCtrlClass::On_LButton_DblClk (const Vector2 &mouse_pos)
{
	//
	//	Find which item hit clicked on
	//
	HITTYPE type;
	TreeItemClass *tree_item = Hit_Test (mouse_pos, type);
	if (tree_item != NULL) {
		SelectedItem = tree_item;

		//
		//	Toggle the expanded state if the user clicked on the
		// plus
		//
		tree_item->Expand (!tree_item->Is_Expanded ());
		Set_Dirty ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Up
//
////////////////////////////////////////////////////////////////
void
TreeCtrlClass::On_LButton_Up (const Vector2 &mouse_pos)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Focus
//
////////////////////////////////////////////////////////////////
void
TreeCtrlClass::On_Set_Focus (void)
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
TreeCtrlClass::On_Kill_Focus (DialogControlClass *focus)
{
	Set_Dirty ();

	DialogControlClass::On_Kill_Focus (focus);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Mouse_Wheel
//
////////////////////////////////////////////////////////////////
void
TreeCtrlClass::On_Mouse_Wheel (int direction)
{
	if (direction < 0) {
		Set_Scroll_Pos (ScrollPos - 1);
	} else {
		Set_Scroll_Pos (ScrollPos + 1);
	}

	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Key_Down
//
////////////////////////////////////////////////////////////////
bool
TreeCtrlClass::On_Key_Down (uint32 key_id, uint32 key_data)
{
	bool handled = false;
	bool is_dirty = true;

	switch (key_id)
	{
		case VK_HOME:
			if (ItemList.Count () > 0) {
				SelectedItem	= ItemList[0];
				Set_Scroll_Pos (0);
			}
			break;

		case VK_END:
			if (ItemList.Count () > 0) {
				SelectedItem = Find_Last_Visible_Item ();
				Ensure_Visible (SelectedItem);
			}
			break;

		case VK_UP:
			SelectedItem = Find_Prev_Visible (SelectedItem);
			Ensure_Visible (SelectedItem);
			Set_Dirty ();
			break;

		case VK_DOWN:
			SelectedItem = Find_Next_Visible (SelectedItem);
			Ensure_Visible (SelectedItem);
			Set_Dirty ();
			break;

		case VK_RIGHT:
			if (SelectedItem != NULL) {
				SelectedItem->Expand (true);
			}
			break;

		case VK_LEFT:
			if (SelectedItem != NULL) {
				SelectedItem->Expand (false);
			}
			break;

		default:
			is_dirty = false;
			break;
	}

	if (is_dirty) {
		Set_Dirty ();
	}

	return handled;
}


////////////////////////////////////////////////////////////////
//
//	On_Create
//
////////////////////////////////////////////////////////////////
void
TreeCtrlClass::On_Create (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Ensure_Visible
//
////////////////////////////////////////////////////////////////
void
TreeCtrlClass::Ensure_Visible (TreeItemClass *item_to_find)
{
	if (item_to_find == NULL) {
		Set_Scroll_Pos (0);
		Set_Dirty ();
		return ;
	}

	//
	//	Start at the first visible item and keep going until we've found the
	// item we're looking for
	//
	TreeItemClass *item = ItemList.Count () > 0 ? ItemList[0] : NULL;
	for (int row_index = 0; item != NULL; row_index ++) {
		
		if (item == item_to_find) {

			//
			//	Determine if this row is outside the visible window
			//
			if (row_index < ScrollPos) {
				Set_Scroll_Pos (row_index);
				Set_Dirty ();
			} else if (row_index >= ScrollPos + RowsPerPage) {
				Set_Scroll_Pos (row_index - (RowsPerPage-1));
				Set_Dirty ();
			}
			break;
		}

		//
		//	Advance to the next visible item
		//
		item = Find_Next_Visible (item);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Insert_Item
//
////////////////////////////////////////////////////////////////
TreeItemClass *
TreeCtrlClass::Insert_Item
(
	const WCHAR *		name,
	const char *		icon_name,
	const char *		selected_icon_name,
	TreeItemClass *	parent
)
{
	//
	//	Allocate a new tree item
	//
	TreeItemClass *retval = new TreeItemClass (this);
	retval->Set_Name (name);
	retval->Set_Icon (icon_name);
	retval->Set_Selected_Icon (selected_icon_name);

	//
	//	Now insert this item into the tree
	//
	if (parent != NULL) {
		parent->Add_Child (retval);
		retval->Set_Parent (parent);
	} else {
		ItemList.Add (retval);

		//
		//	Select the first item by default
		//
		if (ItemList.Count () == 1) {
			SelectedItem = retval;
		}
	}

	//
	//	Check to see if we need a scroll bar or not
	//
	Update_Scroll_Bar_Visibility ();
	Set_Dirty ();
	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Delete_Item
//
////////////////////////////////////////////////////////////////
void
TreeCtrlClass::Delete_Item (TreeItemClass *item)
{
	if (item == NULL) {
		return ;
	}

	//
	//	Remove the selected item if necessary
	//
	if (SelectedItem == item) {		
		SelectedItem = NULL;
	}

	//
	//	Notify any advise sinks
	//
	ADVISE_NOTIFY (On_TreeCtrl_Delete_Item (this, Get_ID (), item));

	//
	//	Recursively delete all this item's children
	//
	int child_count = item->Get_Child_Count ();
	int index;
	for (index = 0; index < child_count; index ++) {
		TreeItemClass *child = item->Get_Child (index);
		Delete_Item (child);
	}

	//
	//	Now unlink this child from its parent (or the root list)
	//
	TreeItemClass *parent = item->Get_Parent ();
	if (parent != NULL) {
		parent->Remove_Child (item);
	} else {
		
		//
		//	Remove this item from the root list
		//
		index = ItemList.ID (item);
		if (index != -1) {
			ItemList.Delete (index);
		}
	}

	//
	//	Free this item
	//
	delete item;
	item = NULL;

	//
	//	Ensure the scroll bar is correctly displayed
	//
	Update_Scroll_Bar_Visibility ();

	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Delete_All_Items
//
////////////////////////////////////////////////////////////////
void
TreeCtrlClass::Delete_All_Items (void)
{
	//
	//	Note:  We count down because the Delete_Item () call will
	// delete the entry from the list...
	//
	int index = ItemList.Count ();
	while (index --) {
		Delete_Item (ItemList[index]);
	}

	//
	//	Ensure the scroll bar is correctly displayed
	//
	ScrollPos		= 0;
	SelectedItem	= NULL;
	Update_Scroll_Bar_Visibility ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Select_Item
//
////////////////////////////////////////////////////////////////
void
TreeCtrlClass::Select_Item (TreeItemClass *item)
{
	//
	//	Notify any advise sinks
	//
	ADVISE_NOTIFY (On_TreeCtrl_Sel_Change (this, Get_ID (), SelectedItem, item));

	SelectedItem = item;
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Selected_Item
//
////////////////////////////////////////////////////////////////
TreeItemClass *
TreeCtrlClass::Get_Selected_Item (void)
{
	return SelectedItem;
}


////////////////////////////////////////////////////////////////
//
//	Get_Next_Sibling
//
////////////////////////////////////////////////////////////////
TreeItemClass *
TreeCtrlClass::Get_Next_Sibling (TreeItemClass *item)
{
	TreeItemClass *retval = NULL;
	if (item == NULL) {
		return NULL;
	}

	//
	//	If this item has a parent, then ask it for the sibling...
	//
	TreeItemClass *parent = item->Get_Parent ();
	if (parent != NULL) {
		retval = parent->Get_Next_Child (item);
	} else {

		//
		//	Return the next entry in the root item list...
		//
		int index = ItemList.ID (item);
		if (index != -1 && (index + 1) < ItemList.Count ()) {
			retval = ItemList[index + 1];
		}
	}
	
	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Get_Prev_Sibling
//
////////////////////////////////////////////////////////////////
TreeItemClass *
TreeCtrlClass::Get_Prev_Sibling (TreeItemClass *item)
{
	TreeItemClass *retval = NULL;
	if (item == NULL) {
		return NULL;
	}

	//
	//	If this item has a parent, then ask it for the sibling...
	//
	TreeItemClass *parent = item->Get_Parent ();
	if (parent != NULL) {
		retval = parent->Get_Prev_Child (item);
	} else {

		//
		//	Return the previous entry in the root item list...
		//
		int index = ItemList.ID (item);
		if (index > 0) {
			retval = ItemList[index - 1];
		}
	}
	
	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Find_Prev_Visible
//
////////////////////////////////////////////////////////////////
TreeItemClass *
TreeCtrlClass::Find_Prev_Visible (TreeItemClass *item)
{
	TreeItemClass *retval = NULL;
	if (item == NULL) {
		if (ItemList.Count () > 0) {
			return ItemList[0];
		}
		return NULL;
	}

	//
	//	Simply find the previous sibling of this item.
	//	If you can't find the sibling, then return its parent
	//
	retval = Get_Prev_Sibling (item);
	if (retval == NULL) {
		retval = item->Get_Parent ();
	} else {
		
		while (retval->Is_Expanded ()) {
		
			//
			//	Find the last child for this given item
			//
			TreeItemClass *curr_item = retval->Get_Last_Child ();
			if (curr_item != NULL) {
				retval = curr_item;
			} else {
				break;
			}
		}
	}

	//
	//	If we didn't find a previous visible entry, then
	// simply return the item we were given
	//
	if (retval == NULL) {
		retval = item;
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Find_Next_Visible
//
////////////////////////////////////////////////////////////////
TreeItemClass *
TreeCtrlClass::Find_Next_Visible (TreeItemClass *item)
{
	TreeItemClass *retval = NULL;
	if (item == NULL) {
		if (ItemList.Count () > 0) {
			return ItemList[0];
		}
		return NULL;
	}

	//
	//	First try to get this item's first child...
	//
	if (item->Is_Expanded () && item->Get_Child_Count () > 0) {
		retval = item->Get_Child (0);
	}

	if (retval == NULL) {
		
		//
		//	Try to find this item's next sibling...
		//
		retval = Get_Next_Sibling (item);
		if (retval == NULL) {

			//
			//	Loop outwards through each parent until
			// we can find the next sibling of any parent...
			//
			TreeItemClass *curr_item = item;
			while (curr_item != NULL && retval == NULL) {

				curr_item = curr_item->Get_Parent ();
				if (curr_item != NULL) {
					retval = Get_Next_Sibling (curr_item);
				}
			}
		}
	}

	//
	//	If we didn't find a previous visible entry, then
	// simply return the item we were given
	//
	if (retval == NULL) {
		retval = item;
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Update_Scroll_Bar_Visibility
//
////////////////////////////////////////////////////////////////
void
TreeCtrlClass::Update_Scroll_Bar_Visibility (void)
{
	int visible_rows = Count_Visible_Rows ();

	//
	//	Determine if we have more entries then we can
	// display on one page
	//
	bool needs_scrollbar	= (visible_rows > RowsPerPage);
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

		const float BORDER_SIZE_X	= 4.0F * StyleMgrClass::Get_X_Scale ();

		//
		//	Reset our window size (as necessary)
		//
		Rect.Right = new_right;
		ClientRect.Left	= Rect.Left + BORDER_SIZE_X;
		ClientRect.Right	= Rect.Right - BORDER_SIZE_X;

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
	ScrollBarCtrl.Set_Range (0, visible_rows - RowsPerPage);
	ScrollBarCtrl.Set_Pos (ScrollPos);
	ScrollPos = ScrollBarCtrl.Get_Pos ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Expanded
//
////////////////////////////////////////////////////////////////
void
TreeCtrlClass::On_Expanded (TreeItemClass *item)
{
	//
	//	Notify any advise sinks
	//
	ADVISE_NOTIFY (On_TreeCtrl_Expand_Item (this, Get_ID (), item));

	//
	//	Check to see if this item needs its children filled in.  If so,
	// then notify the advise sinks
	//
	if (item != NULL && item->Is_Expanded () && item->Needs_Children ()) {
		ADVISE_NOTIFY (On_TreeCtrl_Needs_Children (this, Get_ID (), item));
	}

	return ;
}


//********************************************************************************//
//
//	Start of TreeItemClass
//
//********************************************************************************//


////////////////////////////////////////////////////////////////
//
//	Remove_Child
//
////////////////////////////////////////////////////////////////
void
TreeItemClass::Remove_Child (TreeItemClass *child)
{
	int index = ChildList.ID (child);
	if (index != -1) {
		ChildList.Delete (index);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Expand
//
////////////////////////////////////////////////////////////////
void
TreeItemClass::Expand (bool onoff)
{
	IsExpanded = onoff;
	TreeCtrl->On_Expanded (this);
	TreeCtrl->Select_Item (this);
	TreeCtrl->Update_Scroll_Bar_Visibility ();
	TreeCtrl->Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Name
//
////////////////////////////////////////////////////////////////
void
TreeItemClass::Set_Name (const WCHAR *name)
{
	Name = name;
	TreeCtrl->Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Icon
//
////////////////////////////////////////////////////////////////
void
TreeItemClass::Set_Icon (const char *texture_name)
{
	IconName = texture_name;
	
	//
	//	Add this icon to our manager
	//
	if (texture_name != NULL) {
		TreeCtrl->IconMgr.Add_Icon (texture_name);
	}

	TreeCtrl->Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Selected_Icon
//
////////////////////////////////////////////////////////////////
void
TreeItemClass::Set_Selected_Icon (const char *texture_name)
{
	SelectedIconName = texture_name;

	//
	//	Add this icon to our manager
	//
	if (texture_name != NULL) {
		TreeCtrl->IconMgr.Add_Icon (texture_name);
	}

	TreeCtrl->Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Prev_Child
//
////////////////////////////////////////////////////////////////
TreeItemClass *
TreeItemClass::Get_Prev_Child (TreeItemClass *child)
{
	TreeItemClass *retval = NULL;
	if (child == NULL) {
		return NULL;
	}

	//
	//	Return the previous entry in our list...
	//
	int index = ChildList.ID (child);
	if (index > 0) {
		retval = ChildList[index - 1];
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Get_Prev_Child
//
////////////////////////////////////////////////////////////////
TreeItemClass *
TreeItemClass::Get_Next_Child (TreeItemClass *child)
{	
	TreeItemClass *retval = NULL;
	if (child == NULL) {
		return NULL;
	}

	//
	//	Return the next entry in our list...
	//
	int index = ChildList.ID (child);
	if (index != -1 && (index + 1) < ChildList.Count ()) {
		retval = ChildList[index + 1];
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Get_Indent_Level
//
////////////////////////////////////////////////////////////////
int
TreeItemClass::Get_Indent_Level (void)
{	
	int retval = 0;

	//
	//	Count the number of parents this item has
	//
	TreeItemClass *curr_item = this;
	while ((curr_item = curr_item->Get_Parent ()) != NULL) {
		retval ++;
	}

	return retval;	
}


////////////////////////////////////////////////////////////////
//
//	Get_Last_Child
//
////////////////////////////////////////////////////////////////
TreeItemClass *
TreeItemClass::Get_Last_Child (void)
{
	TreeItemClass *retval = NULL;

	if (ChildList.Count () > 0) {
		retval = ChildList[ChildList.Count () - 1];
	}

	return retval;	
}


////////////////////////////////////////////////////////////////
//
//	Set_Needs_Children
//
////////////////////////////////////////////////////////////////
void
TreeItemClass::Set_Needs_Children (bool onoff)
{
	NeedsChildren = onoff;
	TreeCtrl->Set_Dirty ();
	return ;
}
