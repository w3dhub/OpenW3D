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
 *                     $Archive:: /Commando/Code/Combat/textwindow.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/06/01 11:05a                                             $*
 *                                                                                             *
 *                    $Revision:: 19                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "textwindow.h"
#include "render2dsentence.h"
#include "assetmgr.h"
#include "texture.h"
#include "font3d.h"
#include "objectives.h"
#include "translatedb.h"
#include "WWAudio.h"
#include "globalsettings.h"
#include "scene.h"
#include "rendobj.h"
#include "stylemgr.h"
#include <algorithm>


////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
SceneClass *TextWindowClass::Scene	= NULL;


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
static const char *	FONT_NAME	= "Arial MT";


////////////////////////////////////////////////////////////////
//
//	TextWindowClass
//
////////////////////////////////////////////////////////////////
TextWindowClass::TextWindowClass (void) :
	FirstLineIndex (0),
	CurrentDisplayCount (0),
	IsDisplayed (false),
	AreColumnsDisplayed (false),
	IsViewDirty (true),
	TextRect (0, 0, 0, 0),
	ColumnHeight (0),
	LineSpacing (0)	
{	
	TextRenderers[0] = NULL;
	TextRenderers[1] = NULL;	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~TextWindowClass
//
////////////////////////////////////////////////////////////////
TextWindowClass::~TextWindowClass (void)
{
	Free_Backdrop ();
	Free_Contents ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////
void
TextWindowClass::Initialize (SceneClass *scene)
{
	REF_PTR_SET (Scene, scene);	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Shutdown
//
////////////////////////////////////////////////////////////////
void
TextWindowClass::Shutdown (void)
{
	REF_PTR_RELEASE (Scene);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Free_Backdrop
//
////////////////////////////////////////////////////////////////
void
TextWindowClass::Free_Backdrop (void)
{
	//
	//	Reset the rest of the data that is dependent on the backdrop
	//
	FirstLineIndex	= 0;
	IsDisplayed		= false;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Free_Contents
//
////////////////////////////////////////////////////////////////
void
TextWindowClass::Free_Contents (void)
{
	//
	//	Free each row of renderer's
	//
	Delete_All_Columns ();
	Free_Renderers ();

	//
	//	Reset the rest of the data that is dependent on the contents
	//
	FirstLineIndex	= 0;
	IsDisplayed		= false;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Free_Renderers
//
////////////////////////////////////////////////////////////////
void
TextWindowClass::Free_Renderers (void)
{
	//
	//	Free each text renderer object
	//
	delete TextRenderers[0];
	delete TextRenderers[1];
	TextRenderers[0] = NULL;
	TextRenderers[1] = NULL;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Backdrop
//
////////////////////////////////////////////////////////////////
void
TextWindowClass::Set_Backdrop
(
	const char *		texture_name,
	const RectClass & screen_rect,
	const Vector2 &	texture_size,
	const RectClass &	endcap_rect,
	const RectClass &	fadeout_rect,
	const RectClass &	textback_rect
)
{
	Free_Backdrop ();

	//
	//	Load the backdrop texture
	//
	TextureClass *texture = WW3DAssetManager::Get_Instance()->Get_Texture (texture_name, TextureClass::MIP_LEVELS_1);
	if (texture != NULL) {
		
		//
		//	Pass the texture onto the renderer
		//
		Backdrop.Reset ();
		Backdrop.Set_Texture (texture);

		//
		//	Configure the renderer
		//
		Backdrop.Set_Coordinate_Range (Render2DClass::Get_Screen_Resolution ());

		//
		//	Calculate the width and height of the sections
		//
		int endcap_width		= endcap_rect.Width ();
		int fadeout_width		= fadeout_rect.Width ();
		int textback_width	= screen_rect.Width () - ((endcap_width + fadeout_width) * 2);

		int endcap_height		= endcap_rect.Height ();
		int fadeout_height	= fadeout_rect.Height ();
		int textback_height	= textback_rect.Height ();

		//
		//	Calculate the UV coordinates for each section
		//
		RectClass endcap_uvs1 (endcap_rect);
		RectClass endcap_uvs2 (endcap_rect);
		RectClass fadeout_uvs1 (fadeout_rect);
		RectClass fadeout_uvs2 (fadeout_rect);
		RectClass textback_uvs (textback_rect);
		endcap_uvs1.Inverse_Scale (texture_size);
		endcap_uvs2.Inverse_Scale (texture_size);
		fadeout_uvs1.Inverse_Scale (texture_size);
		fadeout_uvs2.Inverse_Scale (texture_size);
		textback_uvs.Inverse_Scale (texture_size);
		
		//
		//	Reverse the UVs of the endcap and fadeaways on the right
		//
		float temp_float	= endcap_uvs2.Left;
		endcap_uvs2.Left	= endcap_uvs2.Right;
		endcap_uvs2.Right	= temp_float;

		temp_float				= fadeout_uvs2.Left;
		fadeout_uvs2.Left		= fadeout_uvs2.Right;
		fadeout_uvs2.Right	= temp_float;

		//
		//	Calculate the screen rectangle (in pixels) for each of the sections
		//
		RectClass endcap_screen_rect1 (0, 0, endcap_width, endcap_height);
		RectClass fadeout_screen_rect1 (0, 0, fadeout_width, fadeout_height);
		RectClass textback_screen_rect (0, 0, textback_width, textback_height);
		RectClass fadeout_screen_rect2 (0, 0, fadeout_width, fadeout_height);
		RectClass endcap_screen_rect2 (0, 0, endcap_width, endcap_height);

		endcap_screen_rect1	+= screen_rect.Upper_Left ();
		fadeout_screen_rect1	+= endcap_screen_rect1.Upper_Right ();
		textback_screen_rect	+= fadeout_screen_rect1.Upper_Right ();
		fadeout_screen_rect2	+= textback_screen_rect.Upper_Right ();
		endcap_screen_rect2	+= fadeout_screen_rect2.Upper_Right ();

		//
		//	Determine what area we have to draw the text into
		//
		const float border_size	= 10.0F;
		TextRect = textback_screen_rect;
		TextRect.Inflate (Vector2 (-border_size, -border_size));

		//
		//	Calculate how many times we should vertically tile the text backdrop
		//
		int largest_height	= std::min (endcap_height, fadeout_height);
		largest_height			= std::min (largest_height, textback_height);		
		int section_count		= (screen_rect.Height () / largest_height);

		//
		//	Tile the text backdrop sections
		//
		int index;
		for (index = 0; index < section_count; index ++) {			
			
			//
			//	Submit the geometry
			//
			Backdrop.Add_Quad (fadeout_screen_rect1, fadeout_uvs1);
			Backdrop.Add_Quad (fadeout_screen_rect2, fadeout_uvs2);
			Backdrop.Add_Quad (textback_screen_rect, textback_uvs);

			fadeout_screen_rect1	+= Vector2 (0, fadeout_height);
			fadeout_screen_rect2	+= Vector2 (0, fadeout_height);
			textback_screen_rect	+= Vector2 (0, textback_height);
		}

		//
		//	Add a chopped off section of an endcap onto the end (if necessary)
		//
		float percent = (screen_rect.Height () / largest_height) - section_count;
		if (percent > 0) {
			
			textback_uvs.Bottom				= textback_uvs.Top + (textback_uvs.Height () * percent);
			fadeout_uvs1.Bottom				= fadeout_uvs1.Top + (fadeout_uvs1.Height () * percent);
			fadeout_uvs2.Bottom				= fadeout_uvs2.Top + (fadeout_uvs2.Height () * percent);

			textback_screen_rect.Bottom	= textback_screen_rect.Top + (textback_screen_rect.Height () * percent);
			fadeout_screen_rect1.Bottom	= fadeout_screen_rect1.Top + (fadeout_screen_rect1.Height () * percent);
			fadeout_screen_rect2.Bottom	= fadeout_screen_rect2.Top + (fadeout_screen_rect2.Height () * percent);			

			//
			//	Submit the geometry
			//
			Backdrop.Add_Quad (fadeout_screen_rect1, fadeout_uvs1);
			Backdrop.Add_Quad (fadeout_screen_rect2, fadeout_uvs2);
			Backdrop.Add_Quad (textback_screen_rect, textback_uvs);
		}

		//
		//	Tile the endcaps appropriately
		//
		section_count = screen_rect.Height () / endcap_height;
		for (index = 0; index < section_count; index ++) {

			//
			//	Submit the geometry
			//			
			Backdrop.Add_Quad (endcap_screen_rect1, endcap_uvs1);
			Backdrop.Add_Quad (endcap_screen_rect2, endcap_uvs2);

			endcap_screen_rect1	+= Vector2 (0, endcap_height);
			endcap_screen_rect2	+= Vector2 (0, endcap_height);
		}

		//
		//	Add a chopped off section of an endcap onto the end (if necessary)
		//
		percent = (screen_rect.Height () / endcap_height) - section_count;
		if (percent > 0) {
			
			endcap_uvs1.Bottom			= endcap_uvs1.Top + (endcap_uvs1.Height () * percent);
			endcap_uvs2.Bottom			= endcap_uvs2.Top + (endcap_uvs2.Height () * percent);
			endcap_screen_rect1.Bottom = endcap_screen_rect1.Top + (endcap_screen_rect1.Height () * percent);
			endcap_screen_rect2.Bottom = endcap_screen_rect2.Top + (endcap_screen_rect2.Height () * percent);

			//
			//	Submit the geometry
			//
			Backdrop.Add_Quad (endcap_screen_rect1, endcap_uvs1);
			Backdrop.Add_Quad (endcap_screen_rect2, endcap_uvs2);
		}

		//
		//	Release our hold on the texture
		//
		REF_PTR_RELEASE(texture);
	}
	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
TextWindowClass::On_Frame_Update (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Add_Column
//
////////////////////////////////////////////////////////////////
void
TextWindowClass::Add_Column (const unichar_t *column_name, float width, const Vector3 &color)
{
	//
	//	Create a new column and add it to the list
	//
	TextColumnClass *column = new TextColumnClass;
	column->Set_Name (column_name);	
	column->Set_Width (width);
	column->Set_Color (color);
	Columns.Add (column);
	IsViewDirty = true;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Remove_Column
//
////////////////////////////////////////////////////////////////
bool
TextWindowClass::Remove_Column (int index)
{
	if (index < 0 || index >= Columns.Count ()) {
		return false;
	}

	//
	//	Free the column
	//
	TextColumnClass *column = Columns[index];
	delete column;

	//
	//	Remove the column from the list
	//
	Columns.Delete (index);
	IsViewDirty = true;
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Delete_All_Columns
//
////////////////////////////////////////////////////////////////
void
TextWindowClass::Delete_All_Columns (void)
{
	for (int index = 0; index < Columns.Count (); index ++) {
		TextColumnClass *column = Columns[index];
		delete column;
	}

	Columns.Delete_All ();
	IsViewDirty = true;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Item_Count
//
////////////////////////////////////////////////////////////////
int
TextWindowClass::Get_Item_Count (void) const
{
	return Columns[0]->Get_Item_Count ();
}


////////////////////////////////////////////////////////////////
//
//	Get_Display_Count
//
////////////////////////////////////////////////////////////////
int
TextWindowClass::Get_Display_Count (void)
{
	if (IsViewDirty) {
		Update_View ();
	}

	return CurrentDisplayCount;
}


////////////////////////////////////////////////////////////////
//
//	Delete_Item
//
////////////////////////////////////////////////////////////////
bool
TextWindowClass::Delete_Item (int index)
{
	bool retval = true;

	for (int col_index = 0; col_index < Columns.Count (); col_index ++) {
		retval &= Columns[col_index]->Delete_Item (index);
	}

	IsViewDirty = true;
	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Insert_Item
//
////////////////////////////////////////////////////////////////
int
TextWindowClass::Insert_Item (int index, const unichar_t *text)
{	
	if (Columns.Count () <= 0) {
		return -1;
	}

	//
	//	Insert a new entry in the first column with the given text
	//
	index = Columns[0]->Insert_Item (index, text);

	//
	//	Add blank entries to all the other columns
	//
	for (int col_index = 1; col_index < Columns.Count (); col_index ++) {
		TextColumnClass *column = Columns[col_index];
		column->Insert_Item (index, U_CHAR(""));
	}

	IsViewDirty = true;
	return index;
}


////////////////////////////////////////////////////////////////
//
//	Set_Item_Text
//
////////////////////////////////////////////////////////////////
bool
TextWindowClass::Set_Item_Text (int index, int col_index, const unichar_t *text)
{
	if (col_index < 0 || col_index >= Columns.Count ()) {
		return false;
	}

	//
	//	Change the text entry in this cell
	//
	Columns[col_index]->Set_Item_Text (index, text);
	IsViewDirty = true;
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Set_Item_Color
//
////////////////////////////////////////////////////////////////
bool
TextWindowClass::Set_Item_Color (int index, int col_index, const Vector3 &color)
{
	if (col_index < 0 || col_index >= Columns.Count ()) {
		return false;
	}

	//
	//	Change the color for the item in this cell
	//
	Columns[col_index]->Set_Item_Color (index, color);
	IsViewDirty = true;
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Set_Item_Data
//
////////////////////////////////////////////////////////////////
bool
TextWindowClass::Set_Item_Data (int index, uintptr_t user_data)
{
	//
	//	Store the user data in the first column
	//
	Columns[0]->Set_Item_Data (index, user_data);
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Get_Item_Data
//
////////////////////////////////////////////////////////////////
uintptr_t
TextWindowClass::Get_Item_Data (int index)
{
	//
	//	Lookup the user data (its stored in the first column)
	//
	return Columns[0]->Get_Item_Data (index);
}


////////////////////////////////////////////////////////////////
//
//	Delete_All_Items
//
////////////////////////////////////////////////////////////////
void
TextWindowClass::Delete_All_Items (void)
{
	for (int index = 0; index < Columns.Count (); index ++) {
		Columns[index]->Delete_All_Items ();
	}
	
	IsViewDirty = true;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Build_View
//
////////////////////////////////////////////////////////////////
void
TextWindowClass::Build_View (void)
{
	//
	//	Start fresh
	//
	Free_Renderers ();

	//const RectClass &screen_rect = Render2DClass::Get_Screen_Resolution ();

	//
	//	Create the fonts
	//	
	FontCharsClass *header_font	= StyleMgrClass::Peek_Font( StyleMgrClass::FONT_INGAME_HEADER_TXT );
	FontCharsClass *font				= StyleMgrClass::Peek_Font( StyleMgrClass::FONT_INGAME_TXT );

	//
	//	Load the font we'll use for the column headers
	//	
	if (header_font != NULL) {
		ColumnHeight = (header_font->Get_Char_Height () * 1.5F);
	}

	//
	//	Create the header and body text renderers
	//
	TextRenderers[0] = new Render2DSentenceClass;
	TextRenderers[1] = new Render2DSentenceClass;

	TextRenderers[0]->Set_Font (header_font);
	TextRenderers[1]->Set_Font (font);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_View
//
////////////////////////////////////////////////////////////////
void
TextWindowClass::Update_View (float *total_height, bool info_only)
{
	//
	//	Ensure the view has been created before we try to update it
	//
	if (TextRenderers[0] == NULL || TextRenderers[1] == NULL) {
		Build_View ();
	}
	
	TextRenderers[0]->Reset ();
	TextRenderers[1]->Reset ();

	//
	//	Update the column headers
	//
	if (AreColumnsDisplayed) {

		float x_pos = TextRect.Left;
		for (int col_index = 0; col_index < Columns.Count (); col_index ++) {

			const Vector3 &color = Columns[col_index]->Get_Color ();
			TextRenderers[0]->Build_Sentence (Columns[col_index]->Get_Name ());
			TextRenderers[0]->Set_Location (Vector2 (x_pos, TextRect.Top));			
			TextRenderers[0]->Draw_Sentence (VRGB_TO_INT32(color));
			
			x_pos += Columns[col_index]->Get_Width () * (TextRect.Right - TextRect.Left);
		}
	}

	//
	//	Move past the column headers
	//
	float y_pos	= TextRect.Top;
	if (AreColumnsDisplayed) {
		y_pos += ColumnHeight;
	}
	
	//
	//	Update the contents
	//	
	CurrentDisplayCount	= 0;
	int item_count			= Columns[0]->Get_Item_Count ();	
	for (int item_index = FirstLineIndex; item_index < item_count; item_index ++) {

		//
		//	Update this row of information
		//
		float row_height = 0;
		Update_Row (item_index, y_pos, &row_height);
		CurrentDisplayCount ++;
		
		//
		//	Check to see if we've gone outside the client area of the text window
		//
		if ((y_pos + row_height) > TextRect.Right && info_only == false) {
			break;
		}		
		
		//
		//	Move down one row
		//
		y_pos += int(row_height + LineSpacing);
	}

	if (info_only == false) {
		IsViewDirty = false;
	}

	//
	//	Return the height to the caller (if necessary)
	//
	if (total_height != NULL) {
		(*total_height) = (y_pos - TextRect.Top);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Total_Display_Height
//
////////////////////////////////////////////////////////////////
float
TextWindowClass::Get_Total_Display_Height (void)
{
	float total_height = 0;

	Update_View (&total_height, true);

	return total_height;
}


////////////////////////////////////////////////////////////////
//
//	Update_Row
//
////////////////////////////////////////////////////////////////
void
TextWindowClass::Update_Row
(
	int		item_index,
	float		y_pos,
	float *	row_height
)
{
	float x_pos	= int (TextRect.Left);

	//
	//	Loop over all the columns
	//	
	for (int col_index = 0; col_index < Columns.Count (); col_index ++) {
		
		//
		//	Alias some variables for readability
		//
		TextColumnClass *column	= Columns[col_index];

		//
		//	Determine what text we should display
		//		
		const unichar_t *text	= U_CHAR("");
		Vector3 color (0 ,0, 0);		
		text		= column->Get_Item_Text (item_index);
		color		= column->Get_Item_Color (item_index);

		//
		//	Determine how wide the column should be
		//
		float col_width = column->Get_Width () * (TextRect.Right - TextRect.Left);
		if (col_index + 1 >= Columns.Count ()) {
			col_width = TextRect.Right - x_pos;
		}
		TextRenderers[1]->Set_Wrapping_Width (col_width - 5);

		//
		//	Update the text and color of this renderer
		//
		TextRenderers[1]->Build_Sentence (text);
		TextRenderers[1]->Set_Location (Vector2 (int(x_pos), int(y_pos)));
		TextRenderers[1]->Draw_Sentence (VRGB_TO_INT32(color));
		
		//
		//	See if this is the 'tallest' column so far
		//
		RectClass extents = TextRenderers[1]->Get_Draw_Extents ();
		(*row_height) = std::max ((*row_height), extents.Height ());

		//
		//	Move to the next column
		//
		x_pos += int (column->Get_Width () * (TextRect.Right - TextRect.Left));
	}
	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Display
//
////////////////////////////////////////////////////////////////
void
TextWindowClass::Display (bool onoff)
{
	if (onoff != IsDisplayed) {
		FirstLineIndex			= 0;
		CurrentDisplayCount	= 0;
		IsDisplayed				= onoff;
		IsViewDirty				= onoff;

		Backdrop.Set_Hidden (!onoff);
	}

	return ;	
}


////////////////////////////////////////////////////////////////
//
//	Page_Down
//
////////////////////////////////////////////////////////////////
void
TextWindowClass::Page_Down (void)
{
	FirstLineIndex += CurrentDisplayCount;
	Update_View ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Page_Up
//
////////////////////////////////////////////////////////////////
void
TextWindowClass::Page_Up (void)
{
	FirstLineIndex -= CurrentDisplayCount;
	FirstLineIndex = std::max (0, FirstLineIndex);
	Update_View ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Scroll_To_Top
//
////////////////////////////////////////////////////////////////
void
TextWindowClass::Scroll_To_Top (void)
{
	FirstLineIndex = 0;
	Update_View ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
TextWindowClass::Render (void)
{
	if (IsDisplayed == false) {
		return ;
	}

	if (IsViewDirty || TextRenderers[0] == NULL) {
		Update_View ();
	}

	//
	//	Render the backdrop
	//
	Backdrop.Render ();

	//
	//	Render each piece of text
	//	
	TextRenderers[0]->Render();
	TextRenderers[1]->Render();
	return ;
}


//********************************************************************************//
//
//	Start of TextColumnClass
//
//********************************************************************************//


////////////////////////////////////////////////////////////////
//
//	Free_Data
//
////////////////////////////////////////////////////////////////
void
TextColumnClass::Free_Data (void)
{
	Delete_All_Items ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Reset_Contents
//
////////////////////////////////////////////////////////////////
void
TextColumnClass::Reset_Contents (void)
{
	//
	//	Remove all the entries
	//
	Free_Data ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Insert_Item
//
////////////////////////////////////////////////////////////////
int
TextColumnClass::Insert_Item (int index, const unichar_t *item_name)
{
	TextItemClass *item = new TextItemClass (item_name);
	
	//
	//	Should we insert this item in the list or add it to the end?
	//
	if (index < Items.Count ()) {
		Items.Insert (index + 1, item);
	} else {
		Items.Add (item);
		index = (Items.Count () - 1);
	}

	return index;
}


////////////////////////////////////////////////////////////////
//
//	Delete_Item
//
////////////////////////////////////////////////////////////////
bool
TextColumnClass::Delete_Item (int index)
{
	bool retval = false;

	//
	//	Delete the item if we can find it in our list
	//
	if (index >= 0 && index < Items.Count ()) {
		delete Items[index];
		Items.Delete (index);
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Delete_All_Items
//
////////////////////////////////////////////////////////////////
void
TextColumnClass::Delete_All_Items (void)
{
	//
	//	Free each of the items in the list
	//
	for (int index = 0; index < Items.Count (); index ++) {
		delete Items[index];
	}

	Items.Reset_Active();
	return ;
}
