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
 *                     $Archive:: /Commando/Code/wwui/tabctrl.cpp           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/19/01 11:30a                                              $*
 *                                                                                             *
 *                    $Revision:: 17                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "tabctrl.h"
#include "assetmgr.h"
#include "refcount.h"
#include "mousemgr.h"
#include "dialogmgr.h"
#include "dialogbase.h"
#include "stylemgr.h"
#include "texture.h"
#include "ww3d.h"
#include "font3d.h"
#include "childdialog.h"
#include <algorithm>



////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
static const RectClass	BarUVs				(1, 1, 26, 255);
static const RectClass	SelUVs				(28, 1, 254, 69);
static const RectClass	UnfocusUVs			(28, 236, 42, 250);
static const RectClass	FocusUVs				(43, 236, 57, 250);

static const RectClass	BarTopUVs			(1, 1, 26, 79);
static const RectClass	BarBottomUVs		(1, 169, 26, 255);
static const RectClass	BarTileUVs			(1, 80, 26, 168);

static const float		BUBBLE_OFFSET_X	= 23;
static const float		BUBBLE_OFFSET_Y	= 25;
static const float		BAR_OFFSET			= 25;
static const float		TEXT_OFFSET			= 60;
static const float		SEL_TEXT_OFFSET	= 75;
static const float		TEXT_RADIUS			= 40;
static const float		ANGLE_MIN			= 115;
static const float		ANGLE_MAX			= 245;
static const float		BLINK_DELAY			= 500;
static const float		PAGE_AREA_OFFSET	= 245;
static const float		PIXELS_PER_SEC		= 500;


////////////////////////////////////////////////////////////////
//
//	TabCtrlClass
//
////////////////////////////////////////////////////////////////
TabCtrlClass::TabCtrlClass (void)	:
	CurrTabIndex (0),
	NextBlinkTime (0),
	SelectorPos (0),
	SelRect (0, 0, 0, 0),
	ScaleX (0),
	ScaleY (0),
	IsBubbleDisplayed (false)
{
	//
	//	Determine what scale to use
	//
	ScaleX = Render2DClass::Get_Screen_Resolution().Width () / 800;
	ScaleY = Render2DClass::Get_Screen_Resolution().Height () / 600;

	//
	//	Assign the font to the text renderers
	//
	StyleMgrClass::Assign_Font (&TextRenderer,			StyleMgrClass::FONT_HEADER);
	StyleMgrClass::Assign_Font (&GlowRenderer,			StyleMgrClass::FONT_HEADER);
	StyleMgrClass::Assign_Font (&HilightRenderer,		StyleMgrClass::FONT_BIG_HEADER);
	StyleMgrClass::Assign_Font (&HilightGlowRenderer,	StyleMgrClass::FONT_BIG_HEADER);

	//
	//	Configure the renderers
	//
	StyleMgrClass::Configure_Renderer (&ControlRenderer);

	//
	//	Load the texture for the control
	//
	TextureClass *texture = WW3DAssetManager::Get_Instance ()->Get_Texture ("IF_MENUPARTS9.TGA", TextureClass::MIP_LEVELS_1);
	ControlRenderer.Set_Texture (texture);
	REF_PTR_RELEASE (texture);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~TabCtrlClass
//
////////////////////////////////////////////////////////////////
TabCtrlClass::~TabCtrlClass (void)
{
	Free_Tabs ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Text_Renderer
//
////////////////////////////////////////////////////////////////
void
TabCtrlClass::Create_Text_Renderer (void)
{
	//
	//	Reset the renderers
	//
	GlowRenderer.Reset ();
	TextRenderer.Reset ();
	HilightRenderer.Reset ();
	HilightGlowRenderer.Reset ();

	float text_left	= ClientRect.Left + (TEXT_OFFSET * ScaleX);
	float height		= ClientRect.Height ();
	float tab_height	= height / std::max (TabList.Count (), 1);

	//
	//	Setup some variables we can use to simulate a semi-circle for the text
	//
	float angle_min	= DEG_TO_RADF (ANGLE_MIN);
	float angle_max	= DEG_TO_RADF (ANGLE_MAX);
	float radius		= TEXT_RADIUS * ScaleX;

	//
	//	Render the tab names
	//
	float y_pos = ClientRect.Top;
	for (int index = 0; index < TabList.Count (); index ++) {

		//
		//	Calculate a "semi-circular" x-offset for the text
		//
		float angle = angle_min + ((angle_max - angle_min) * ((float)index / float(TabList.Count()-1)));
		float x_offset = WWMath::Cos (angle) * radius;

		//
		//	Calculate what rectangle to render to
		//
		RectClass text_rect;
		text_rect.Left		= (int)(text_left + (radius + x_offset));
		text_rect.Top		= (int)y_pos;
		text_rect.Right	= (int)ClientRect.Right;
		text_rect.Bottom	= int(y_pos + tab_height);

		//
		//	Get the title from the child dialog
		//
		WideStringClass title;
		TabList[index]->Get_Title (&title);

		//
		//	Is this the selected tab?
		//
		if (index == CurrTabIndex) {

			//
			//	Move the hilighted text in a bit
			//
			text_rect.Left = SelRect.Left + (SEL_TEXT_OFFSET * ScaleX);

			//
			//	Render the hilighted text and its glow
			//
			HilightRenderer.Build_Sentence (title);
			HilightGlowRenderer.Build_Sentence (title);
			//StyleMgrClass::Render_Text (title, &HilightRenderer, text_rect, true, true);

			StyleMgrClass::Render_Text (title, &HilightRenderer,
									StyleMgrClass::Get_Tab_Text_Color (),
									StyleMgrClass::Get_Text_Shadow_Color (),
									text_rect, true, true);

				/*HilightRenderer.Set_Clipping_Rect (text_rect);

				//
				//	Get the extents of the text we will be drawing
				//
				Vector2 text_extent = HilightRenderer.Get_Text_Extents (title);

				//
				//	Assume left justification
				//
				int x_pos = text_rect.Left + 1;
				int y_pos = int(text_rect.Top + (text_rect.Height () / 2) - (text_extent.Y / 2));

				//
				//	Construct the textures needed to render the text
				//
				HilightRenderer.Build_Sentence (title);

				HilightRenderer.Set_Location (Vector2 (x_pos - 1, y_pos - 1));
				HilightRenderer.Draw_Sentence (RGB_TO_INT32 (255, 255, 255));

				HilightRenderer.Set_Location (Vector2 (x_pos + 1, y_pos + 1));
				HilightRenderer.Draw_Sentence (RGB_TO_INT32 (0, 0, 0));

				//
				//	Render the text
				//
				HilightRenderer.Set_Location (Vector2 (x_pos, y_pos));
				HilightRenderer.Draw_Sentence (RGB_TO_INT32 (147, 155, 153));*/


			StyleMgrClass::Render_Glow (title, &HilightGlowRenderer, text_rect, 7, 7,
									StyleMgrClass::Get_Tab_Glow_Color ());

		} else {
			
			//
			//	Render the text and its glow
			//
			TextRenderer.Build_Sentence (title);
			GlowRenderer.Build_Sentence (title);
			StyleMgrClass::Render_Text (title, &TextRenderer,
									StyleMgrClass::Get_Tab_Text_Color (),
									StyleMgrClass::Get_Text_Shadow_Color (),
									text_rect, true, true);

				/*TextRenderer.Set_Clipping_Rect (text_rect);

				//
				//	Get the extents of the text we will be drawing
				//
				Vector2 text_extent = TextRenderer.Get_Text_Extents (title);

				//
				//	Assume left justification
				//
				int x_pos = text_rect.Left + 1;
				int y_pos = int(text_rect.Top + (text_rect.Height () / 2) - (text_extent.Y / 2));

				//
				//	Construct the textures needed to render the text
				//
				TextRenderer.Build_Sentence (title);

				TextRenderer.Set_Location (Vector2 (x_pos - 1, y_pos - 1));
				TextRenderer.Draw_Sentence (RGB_TO_INT32 (255, 255, 255));

				TextRenderer.Set_Location (Vector2 (x_pos + 1, y_pos + 1));
				TextRenderer.Draw_Sentence (RGB_TO_INT32 (0, 0, 0));

				//
				//	Render the text
				//
				TextRenderer.Set_Location (Vector2 (x_pos, y_pos));
				TextRenderer.Draw_Sentence (RGB_TO_INT32 (147, 155, 153));*/

			
			StyleMgrClass::Render_Glow (title, &GlowRenderer, text_rect, 4, 4,
									StyleMgrClass::Get_Tab_Glow_Color ());
		}

		//
		//	Move down to the next tab
		//
		y_pos += tab_height;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Control_Renderer
//
////////////////////////////////////////////////////////////////
void
TabCtrlClass::Create_Control_Renderer (void)
{
	Render2DClass &renderer = ControlRenderer;
	renderer.Reset ();	

	//
	//	Determine which UVs to use for the bubble
	//
	RectClass bubble_uvs = UnfocusUVs;
	if (IsBubbleDisplayed) {
		bubble_uvs = FocusUVs;
	}

	//
	//	Scale the dimensions of the components
	//
	float bar_tile_height	= BarTileUVs.Height () * ScaleY;
	float bar_tip_height		= BarTopUVs.Height () * ScaleY;
	float bar_width			= BarTileUVs.Width () * ScaleX;
	float sel_width			= SelUVs.Width () * ScaleX;
	float sel_height			= SelUVs.Height () * ScaleY;
	float bubble_width		= bubble_uvs.Width () * ScaleX;
	float bubble_height		= bubble_uvs.Height () * ScaleX;

	//
	//	Setup some variables we can use to simulate a semi-circle for the text
	//
	float angle_min	= DEG_TO_RADF (ANGLE_MIN);
	float angle_max	= DEG_TO_RADF (ANGLE_MAX);
	float radius		= TEXT_RADIUS * ScaleX;
	float min_y			= Pos_From_Tab (0);
	float max_y			= Pos_From_Tab (TabList.Count () - 1);
	float percent		= (SelectorPos - min_y) / (max_y - min_y);
	float angle			= angle_min + percent * (angle_max - angle_min);
	//float left_pos		= -radius;
	float x_offset		= radius + (WWMath::Cos (angle) * radius);

	//
	//	Calculate the screen rectangle for the bar
	//
	RectClass bar_rect;	
	bar_rect.Left		= int(ClientRect.Left + x_offset + (BAR_OFFSET * ScaleX) - (bar_width / 2));
	bar_rect.Right		= int(bar_rect.Left + bar_width);
	bar_rect.Top		= int(ClientRect.Top - (x_offset * 1.5F));
	bar_rect.Bottom	= int(ClientRect.Bottom + (x_offset * 1.5F));

	//
	//	Calculate the screen rectangle for the selector
	//
	SelRect.Left	= int(ClientRect.Left + x_offset);
	SelRect.Right	= int(SelRect.Left + sel_width);
	SelRect.Top		= int(SelectorPos - (sel_height / 2));
	SelRect.Bottom	= int(SelRect.Top + sel_height);

	//
	//	Calculate the screen rectangle for the bubble
	//
	RectClass bubble_rect;
	bubble_rect.Left		= int(SelRect.Left + BUBBLE_OFFSET_X * ScaleX);
	bubble_rect.Right		= int(bubble_rect.Left + bubble_width);
	bubble_rect.Top		= int(SelRect.Top + BUBBLE_OFFSET_Y * ScaleY);
	bubble_rect.Bottom	= int(bubble_rect.Top + bubble_height);

	//
	//	Normalize the UVs
	//
	RectClass sel_uvs				= SelUVs;
	RectClass bar_top_uvs		= BarTopUVs;
	RectClass bar_bottom_uvs	= BarBottomUVs;
	sel_uvs.Inverse_Scale (Vector2 (256, 256));
	bar_top_uvs.Inverse_Scale (Vector2 (256, 256));
	bar_bottom_uvs.Inverse_Scale (Vector2 (256, 256));
	bubble_uvs.Inverse_Scale (Vector2 (256, 256));

	//
	//	Render the bar top and bottom
	//
	RectClass bar_top_rect;
	bar_top_rect.Left		= int(bar_rect.Left);
	bar_top_rect.Right	= int(bar_rect.Right);
	bar_top_rect.Top		= int(bar_rect.Top);
	bar_top_rect.Bottom	= int(bar_top_rect.Top + bar_tip_height);
	
	RectClass bar_bottom_rect;
	bar_bottom_rect.Left		= int(bar_rect.Left);
	bar_bottom_rect.Right	= int(bar_rect.Right);
	bar_bottom_rect.Top		= int(bar_rect.Bottom - bar_tip_height);
	bar_bottom_rect.Bottom	= int(bar_rect.Bottom);
	
	renderer.Add_Quad (bar_top_rect, bar_top_uvs);
	renderer.Add_Quad (bar_bottom_rect, bar_bottom_uvs);

	//
	//	Tile the remaining pieces
	//
	float y_pos				= bar_top_rect.Bottom;
	float total_height	= bar_bottom_rect.Top - y_pos;
	while (total_height > 0) {
		
		float height = std::min (bar_tile_height, total_height);

		//
		//	Setup the UV rectangle
		//
		RectClass uvs	= BarTileUVs;
		uvs.Bottom		= uvs.Top + height;
		uvs.Inverse_Scale (Vector2 (256, 256));

		//
		//	Setup the screen rectangle
		//
		RectClass bar_tile_rect = bar_rect;
		bar_tile_rect.Top			= y_pos;
		bar_tile_rect.Bottom		= y_pos + height;

		//
		//	Render the section
		//
		renderer.Add_Quad (bar_tile_rect, uvs);
		
		y_pos				+= height;
		total_height	-= height;		
	}
	
	//
	//	Render the bar and selector
	//
	renderer.Add_Quad (SelRect, sel_uvs);
	renderer.Add_Quad (bubble_rect, bubble_uvs);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Cursor
//
////////////////////////////////////////////////////////////////
void
TabCtrlClass::On_Set_Cursor (const Vector2 &mouse_pos)
{
	//
	//	Change the mouse cursor
	//
	if (mouse_pos.X < (ClientRect.Left + PAGE_AREA_OFFSET * ScaleX)) {
		MouseMgrClass::Set_Cursor (MouseMgrClass::CURSOR_ACTION);
	} else {
		MouseMgrClass::Set_Cursor (MouseMgrClass::CURSOR_ARROW);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Client_Rect
//
////////////////////////////////////////////////////////////////
void
TabCtrlClass::Update_Client_Rect (void)
{
	//
	//	Set the client area
	//
	ClientRect = Rect;
	Set_Dirty ();

	//
	//	Resize each child dialog so it fits in the new rectangle
	//
	for (int index = 0; index < TabList.Count (); index ++) {
		RectClass rect;
		rect.Left	= (int)(ClientRect.Left + PAGE_AREA_OFFSET * ScaleX);
		rect.Top		= (int)ClientRect.Top;
		rect.Right	= (int)ClientRect.Right;
		rect.Bottom	= (int)ClientRect.Bottom;
		TabList[index]->Set_Rect (rect);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
TabCtrlClass::Render (void)
{
	Update_Selector ();
	Update_Bubble ();	

	//
	//	Recreate the renderers (if necessary)
	//
	if (IsDirty) {		
		Create_Control_Renderer ();
		Create_Text_Renderer ();		
	}

	//
	//	Render the background and text
	//
	GlowRenderer.Render ();
	HilightGlowRenderer.Render ();
	TextRenderer.Render ();
	HilightRenderer.Render ();
	ControlRenderer.Render ();

	DialogControlClass::Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Down
//
////////////////////////////////////////////////////////////////
void
TabCtrlClass::On_LButton_Down (const Vector2 &mouse_pos)
{
	//
	//	Change tabs
	//
	if (mouse_pos.X < (ClientRect.Left + PAGE_AREA_OFFSET * ScaleX)) {
		Set_Curr_Tab (Tab_From_Pos (mouse_pos));
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Up
//
////////////////////////////////////////////////////////////////
void
TabCtrlClass::On_LButton_Up (const Vector2 &/* mouse_pos */)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Mouse_Move
//
////////////////////////////////////////////////////////////////
void
TabCtrlClass::On_Mouse_Move (const Vector2 &/* mouse_pos */)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Focus
//
////////////////////////////////////////////////////////////////
void
TabCtrlClass::On_Set_Focus (void)
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
TabCtrlClass::On_Kill_Focus (DialogControlClass *focus)
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
TabCtrlClass::On_Key_Down (uint32 key_id, uint32 /* key_data */)
{
	bool handled = true;

	switch (key_id)
	{
		case VK_UP:
		case VK_LEFT:
			Set_Curr_Tab (CurrTabIndex - 1);
			break;

		case VK_DOWN:
		case VK_RIGHT:
			Set_Curr_Tab (CurrTabIndex + 1);
			break;

		case VK_HOME:
			Set_Curr_Tab (0);
			break;

		case VK_END:
			Set_Curr_Tab (TabList.Count () - 1);
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
TabCtrlClass::On_Create (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Tab_From_Pos
//
////////////////////////////////////////////////////////////////
int
TabCtrlClass::Tab_From_Pos (const Vector2 &mouse_pos)
{
	int retval = -1;

	float height		= ClientRect.Height ();
	float tab_height	= height / std::max (TabList.Count (), 1);

	//
	//	Check each of the tabs
	//
	float y_pos = ClientRect.Top;
	for (int index = 0; index < TabList.Count (); index ++) {

		//
		//	Is the mouse over this tab?
		//
		if (mouse_pos.Y >= y_pos && mouse_pos.Y <= (y_pos + tab_height)) {
			retval = index;
			break;
		}

		//
		//	Move down to the next tab
		//
		y_pos += tab_height;
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Pos_From_Tab
//
////////////////////////////////////////////////////////////////
float
TabCtrlClass::Pos_From_Tab (int index)
{	
	float height		= ClientRect.Height ();
	float tab_height	= height / std::max (TabList.Count (), 1);

	//
	//	Return the centered y-position
	//
	return ClientRect.Top + (tab_height * (index + 1)) - (tab_height / 2);
}


////////////////////////////////////////////////////////////////
//
//	Set_Curr_Tab
//
////////////////////////////////////////////////////////////////
void
TabCtrlClass::Set_Curr_Tab (int index)
{
	//
	//	Bound the index
	//
	index = std::min (TabList.Count () - 1, index);
	index = std::max (0, index);	

	//
	//	Did we change tabs?
	//
	if (CurrTabIndex != index) {
		
		//
		//	Switch tabs visibly
		//
		TabList[CurrTabIndex]->Show (false);
		TabList[index]->Show (true);

		//
		//	Change the tab and force a repaint
		//
		CurrTabIndex = index;		
		Set_Dirty ();
	}

	return ;	
}


////////////////////////////////////////////////////////////////
//
//	Update_Bubble
//
////////////////////////////////////////////////////////////////
void
TabCtrlClass::Update_Bubble (void)
{
	if (HasFocus) {
		
		//
		//	Is it time to blink the bubble light?
		//
		int curr_time = DialogMgrClass::Get_Time ();
		if (curr_time > NextBlinkTime) {
			
			//
			//	Toggle the bubble light
			//
			IsBubbleDisplayed = !IsBubbleDisplayed;
			NextBlinkTime		= curr_time + BLINK_DELAY;
			
			//
			//	Force a repaint
			//
			Set_Dirty ();
		}

	} else {
		IsBubbleDisplayed = false;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Selector
//
////////////////////////////////////////////////////////////////
void
TabCtrlClass::Update_Selector (void)
{
	//
	//	Exit early if there's nothing to do
	//
	float end_pos = Pos_From_Tab (CurrTabIndex);
	if (SelectorPos == end_pos) {
		return ;
	}

	//
	//	Calculate the rate the selector moves
	//
	float rate = PIXELS_PER_SEC * ScaleY;

	//
	//	Calculate how far we need to move
	//
	float time		= DialogMgrClass::Get_Frame_Time () / 1000.0F;
	float dist		= rate * time;

	//
	//	Move the selector
	//
	if (SelectorPos < end_pos) {
		SelectorPos += dist;
		SelectorPos = std::min (end_pos, SelectorPos);
		Set_Dirty ();
	} else if (SelectorPos > end_pos) {
		SelectorPos -= dist;
		SelectorPos = std::max (end_pos, SelectorPos);
		Set_Dirty ();
	}

	//
	//	Bound the selector position
	//
	SelectorPos = std::min (Pos_From_Tab (TabList.Count () - 1), SelectorPos);
	SelectorPos = std::max (Pos_From_Tab (0), SelectorPos);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Add_Tab
//
////////////////////////////////////////////////////////////////
void
TabCtrlClass::Add_Tab (ChildDialogClass *dialog)
{
	if (dialog == NULL) {
		return ;
	}

	//
	//	Start the dialog
	//
	dialog->Show (TabList.Count () == 0);
	dialog->Start_Dialog ();
	Parent->Add_Child_Dialog (dialog);

	//
	//	Size the child so it fits perfectly in the
	// page area.
	//
	RectClass rect;
	rect.Left	= (int)(ClientRect.Left + PAGE_AREA_OFFSET * ScaleX);
	rect.Top		= (int)ClientRect.Top;
	rect.Right	= (int)ClientRect.Right;
	rect.Bottom	= (int)ClientRect.Bottom;
	dialog->Set_Rect (rect);	

	//
	//	Add this dialog to our list
	//
	dialog->Add_Ref ();
	TabList.Add (dialog);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Remove_Tab
//
////////////////////////////////////////////////////////////////
void
TabCtrlClass::Remove_Tab (int index)
{
	if (index < 0 || index >= TabList.Count ()) {
		return ;
	}

	//
	//	Remove this dialog from our list
	//
	Parent->Remove_Child_Dialog (TabList[index]);
	TabList[index]->End_Dialog ();
	REF_PTR_RELEASE (TabList[index]);
	TabList.Delete (index);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Free_Tabs
//
////////////////////////////////////////////////////////////////
void
TabCtrlClass::Free_Tabs (void)
{
	//
	//	Release our hold on each tab
	//
	for (int index = 0; index < TabList.Count (); index ++) {
		REF_PTR_RELEASE (TabList[index]);
	}

	TabList.Delete_All ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Mouse_Wheel
//
////////////////////////////////////////////////////////////////
void
TabCtrlClass::On_Mouse_Wheel (int direction)
{
	if (direction < 0) {
		Set_Curr_Tab (CurrTabIndex - 1);
	} else {
		Set_Curr_Tab (CurrTabIndex + 1);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Apply_Changes_On_Tabs
//
////////////////////////////////////////////////////////////////
bool
TabCtrlClass::Apply_Changes_On_Tabs (void)
{
	bool retval = true;

	//
	//	Get each tab to apply their changes
	//
	for (int index = 0; retval && index < TabList.Count (); index ++) {
		retval &= TabList[index]->On_Apply ();
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Discard_Changes_On_Tabs
//
////////////////////////////////////////////////////////////////
bool
TabCtrlClass::Discard_Changes_On_Tabs (void)
{
	bool retval = true;

	//
	//	Get each tab to discard their changes
	//
	for (int index = 0; retval && index < TabList.Count (); index ++) {
		retval &= TabList[index]->On_Discard ();
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Reload_Tabs
//
////////////////////////////////////////////////////////////////
void
TabCtrlClass::Reload_Tabs (void)
{
	//
	//	Get each tab to discard their changes
	//
	for (int index = 0; index < TabList.Count (); index ++) {
		TabList[index]->On_Reload ();
	}

	return ;
}
