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
 *                     $Archive:: /Commando/Code/wwui/scrollbarctrl.cpp         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/04/02 2:41p                                               $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "scrollbarctrl.h"
#include "assetmgr.h"
#include "refcount.h"
#include "mousemgr.h"
#include "ww3d.h"
#include "dialogmgr.h"
#include "dialogbase.h"
#include "stylemgr.h"
#include "texture.h"
#include <algorithm>


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
/*
static const RectClass TopBtnUpUVs		(103, 94, 125, 116);
static const RectClass TopBtnDnUVs		(103, 70, 125, 92);

static const RectClass BottomBtnUpUVs	(79, 94, 101, 116);
static const RectClass BottomBtnDnUVs	(79, 70, 101, 92);

static const RectClass ThumbDnUVs		(150, 70, 171, 136);
static const RectClass ThumbUpUVs		(127, 70, 148, 136);
*/

static const RectClass TopBtnUpUVs[2] =
{
	RectClass (100, 212, 121, 234),
	RectClass (147, 228, 169, 241)
};

static const RectClass TopBtnDnUVs[2] =
{
	RectClass (76, 212, 98, 234),
	RectClass (147, 213, 169, 226)
};

static const RectClass BottomBtnUpUVs[2] =
{
	RectClass (52, 212, 74, 234),
	RectClass (123, 228, 145, 241)
};

static const RectClass BottomBtnDnUVs[2] =
{
	RectClass (28, 212, 50, 234),
	RectClass (123, 213, 145, 226)
};

static const RectClass ThumbDnUVs[2] =
{
	RectClass (233, 138, 254, 204),
	RectClass (171, 213, 193, 222)
};

static const RectClass ThumbUpUVs[2] =
{
	RectClass (233, 70, 254, 136),
	RectClass (171, 224, 193, 233)
};


static const float DEFAULT_WIDTH		= 10;
static const float BUTTON_V_OFFSET	= 10;


////////////////////////////////////////////////////////////////
//
//	ScrollBarCtrlClass
//
////////////////////////////////////////////////////////////////
ScrollBarCtrlClass::ScrollBarCtrlClass (void)	:
	CurrState (NORMAL),
	MinPos (0),
	MaxPos (100),
	CurrPos (0),
	PageSize (10),
	ThumbWidth (0),
	ThumbHeight (0),
	ThumbRect (0, 0, 0, 0),
	WasButtonPressedOnMe (false),
	TrackRect (0, 0, 0, 0),
	TopButtonRect (0, 0, 0, 0),
	BottomButtonRect (0, 0, 0, 0),
	IsDragging (false),
	WasTopButtonPressed (false),
	WasBottomButtonPressed (false),
	Width (DEFAULT_WIDTH),
	ScaleX (1.0F),
	ScaleY (1.0F),
	MouseClickPos (0, 0),
	MouseClickSliderPos (0),
	IsSmallBMPMode (false)
{
	//
	//	Configure the renderers
	//
	StyleMgrClass::Configure_Renderer (&ButtonRenderer);
	StyleMgrClass::Configure_Renderer (&ControlRenderers[0]);
	StyleMgrClass::Configure_Renderer (&ControlRenderers[1]);

	//
	//	Load the texture for the control
	//
	TextureClass *texture = WW3DAssetManager::Get_Instance ()->Get_Texture ("IF_MENUPARTS9.TGA", TextureClass::MIP_LEVELS_1);
	ButtonRenderer.Set_Texture (texture);
	REF_PTR_RELEASE (texture);

	//
	//	Scale the control
	//
	ScaleX	= Render2DClass::Get_Screen_Resolution().Width () / 800;
	ScaleY	= Render2DClass::Get_Screen_Resolution().Height () / 600;
	Width		= Width * ScaleX;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~ScrollBarCtrlClass
//
////////////////////////////////////////////////////////////////
ScrollBarCtrlClass::~ScrollBarCtrlClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Button_Renderer
//
////////////////////////////////////////////////////////////////
void
ScrollBarCtrlClass::Create_Button_Renderer (void)
{
	ButtonRenderer.Reset ();

	//
	//	Determine what UVs to use for the thumb
	//
	RectClass thumb_uvs = ThumbUpUVs[IsSmallBMPMode];
	if (IsDragging) {
		thumb_uvs = ThumbDnUVs[IsSmallBMPMode];
	}

	thumb_uvs.Inverse_Scale (Vector2 (255, 255));

	//
	//	Flip the thumb if its past the halfway mark
	//
	if (IsSmallBMPMode && ThumbRect.Top > (ClientRect.Top + ClientRect.Height () / 2)) {
		float temp			= thumb_uvs.Top;
		thumb_uvs.Top		= thumb_uvs.Bottom;
		thumb_uvs.Bottom	= temp;
	}

	//
	//	Draw the thumb
	//
	ButtonRenderer.Add_Quad (ThumbRect, thumb_uvs);

	//
	//	Determine which UVs to use for the buttons
	//
	RectClass bottom_btn_uvs	= BottomBtnUpUVs[IsSmallBMPMode];
	RectClass top_btn_uvs		= TopBtnUpUVs[IsSmallBMPMode];

	//
	//	Use the "button-down" UVs if necessary
	//
	if (WasTopButtonPressed) {
		top_btn_uvs = TopBtnDnUVs[IsSmallBMPMode];
	} else if (WasBottomButtonPressed) {
		bottom_btn_uvs = BottomBtnDnUVs[IsSmallBMPMode];
	}

	//
	//	Normalize the UVs
	//
	top_btn_uvs.Inverse_Scale (Vector2 (255, 255));
	bottom_btn_uvs.Inverse_Scale (Vector2 (255, 255));

	//
	//	Draw the buttons
	//
	ButtonRenderer.Add_Quad (TopButtonRect, top_btn_uvs);
	ButtonRenderer.Add_Quad (BottomButtonRect, bottom_btn_uvs);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Control_Renderers
//
////////////////////////////////////////////////////////////////
void
ScrollBarCtrlClass::Create_Control_Renderers (void)
{
	//
	//	Create a renderer for each state
	//
	for (int index = 0; index < STATE_MAX; index ++) {
		Render2DClass &renderer = ControlRenderers[index];

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
		if (index == DISABLED) {
			color		= StyleMgrClass::Get_Disabled_Line_Color ();
			bkcolor	= StyleMgrClass::Get_Disabled_Bk_Color ();
		}

		//
		//	Draw the bounding rectangle for the control
		//
		RectClass outline_rect = Rect;
		outline_rect.Top		= TopButtonRect.Bottom - (TopButtonRect.Height () / 2);
		outline_rect.Bottom	= BottomButtonRect.Top + (BottomButtonRect.Height () / 2);
		renderer.Add_Outline (outline_rect, 1.0F, color);

		//
		//	Compose two "gradient" rectangles
		//
		RectClass top_rect;
		top_rect.Left		= ClientRect.Left;
		top_rect.Right		= ClientRect.Right;
		top_rect.Top		= outline_rect.Top;
		top_rect.Bottom	= ThumbRect.Top;

		RectClass bottom_rect;
		bottom_rect.Left		= ClientRect.Left;
		bottom_rect.Right		= ClientRect.Right;
		bottom_rect.Top		= ThumbRect.Bottom;
		bottom_rect.Bottom	= outline_rect.Bottom;

		int color_in = (color & 0x00FFFFFF) | RGBA_TO_INT32 (0, 0, 0, 255);
		int color_out = (color & 0x00FFFFFF) | RGBA_TO_INT32 (0, 0, 0, 10);

		//
		//	Draw the gradients
		//
		renderer.Add_Quad_VGradient (top_rect, color_in, color_out);
		renderer.Add_Quad_VGradient (bottom_rect, color_out, color_in);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Cursor
//
////////////////////////////////////////////////////////////////
void
ScrollBarCtrlClass::On_Set_Cursor (const Vector2 &/* mouse_pos */)
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
ScrollBarCtrlClass::Update_Client_Rect (void)
{
	//
	//	Snap the rect to the alloted size
	//
	if (Rect.Right >= 0.0f) {
		Rect.Left = int(Rect.Right - Width);
	} else {
		Rect.Right = (Rect.Left + Width);
	}

	//
	//	Set the client area
	//
	ClientRect = Rect;

	//
	//	Scale the thumb dimensions
	//
	ThumbWidth			= (int)(ThumbUpUVs[IsSmallBMPMode].Width () * ScaleX);
	ThumbHeight			= (int)(ThumbUpUVs[IsSmallBMPMode].Height () * ScaleY);

	//
	//	Scale the button dimensions
	//
	int button_width	= (int)(TopBtnUpUVs[IsSmallBMPMode].Width () * ScaleX);
	int button_height	= (int)(TopBtnUpUVs[IsSmallBMPMode].Height () * ScaleY);

	//
	//	Update the top button's bounding rectangle
	//
	TopButtonRect.Left	= int(ClientRect.Left + (ClientRect.Width () / 2) - (button_width / 2));
	TopButtonRect.Top		= int(ClientRect.Top - (BUTTON_V_OFFSET * ScaleY));
	TopButtonRect.Right	= int(TopButtonRect.Left + button_width);
	TopButtonRect.Bottom	= int(TopButtonRect.Top + button_height);

	//
	//	Update the bottom button's bounding rectangle
	//
	BottomButtonRect.Left	= int(ClientRect.Left + (ClientRect.Width () / 2) - (button_width / 2));
	BottomButtonRect.Top		= int(ClientRect.Bottom - button_height + (BUTTON_V_OFFSET * ScaleY));
	BottomButtonRect.Right	= int(BottomButtonRect.Left + button_width);
	BottomButtonRect.Bottom	= int(BottomButtonRect.Top + button_height);

	//
	//	Adjust the rectangle to include the top and bottom buttons
	//
	Rect.Top		= TopButtonRect.Top;
	Rect.Bottom	= BottomButtonRect.Bottom;

	//
	//	Update the track rectangle so it doesn't extend into the buttons
	//
	TrackRect			= ClientRect;
	TrackRect.Top		= TopButtonRect.Bottom;
	TrackRect.Bottom	= BottomButtonRect.Top - ThumbHeight;

	//
	//	Update the bounding box of the thumb
	//
	Update_Thumb_Rect ();

	//
	//	Force a repaint
	//
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
ScrollBarCtrlClass::Render (void)
{
	//
	//	Handle scrolling (if necessary)
	//
	if (WasTopButtonPressed) {
		Set_Pos (CurrPos - 1);
	} else if (WasBottomButtonPressed) {
		Set_Pos (CurrPos + 1);
	}

	//
	//	Recreate the renderers (if necessary)
	//
	if (IsDirty) {
		Create_Control_Renderers ();
		Create_Button_Renderer ();
	}

	//
	//	Render the background and text for the current state
	//
	ControlRenderers[CurrState].Render ();
	ButtonRenderer.Render ();

	DialogControlClass::Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Down
//
////////////////////////////////////////////////////////////////
void
ScrollBarCtrlClass::On_LButton_Down (const Vector2 &mouse_pos)
{
	Set_Capture ();

	//
	//	Check to see if the user clicked on either of the buttons
	//
	if (TopButtonRect.Contains (mouse_pos)) {
		WasTopButtonPressed = true;
	} else if (BottomButtonRect.Contains (mouse_pos)) {
		WasBottomButtonPressed = true;
	} else {

		//
		//	Did the user click on the thumb?
		//
		if (ThumbRect.Contains (mouse_pos)) {
			IsDragging = true;
		} else if (mouse_pos.Y < ThumbRect.Top) {

			//
			//	Page Up
			//
			Scroll_Page (-1);

		} else if (mouse_pos.Y > ThumbRect.Bottom) {

			//
			//	Page Down
			//
			Scroll_Page (1);
		}
	}

	MouseClickPos = mouse_pos;
	MouseClickSliderPos = ThumbRect.Top;


	WasButtonPressedOnMe = true;
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Up
//
////////////////////////////////////////////////////////////////
void
ScrollBarCtrlClass::On_LButton_Up (const Vector2 &/* mouse_pos */)
{
	Release_Capture ();

	WasButtonPressedOnMe		= false;

	IsDragging					= false;
	WasTopButtonPressed		= false;
	WasBottomButtonPressed	= false;
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Mouse_Move
//
////////////////////////////////////////////////////////////////
void
ScrollBarCtrlClass::On_Mouse_Move (const Vector2 &mouse_pos)
{
	//
	//	Update the thumb position
	//
	if (IsDragging) {

		//
		//	Calculate a new position from the change in mouse position
		//
		float y_delta	= (mouse_pos.Y - MouseClickPos.Y);
		float percent	= ((MouseClickSliderPos + y_delta) - TrackRect.Top) / TrackRect.Height ();
		int new_pos		= MinPos + int(percent * (MaxPos - MinPos));

		//
		//	Set the new position
		//
		Set_Pos (new_pos);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Focus
//
////////////////////////////////////////////////////////////////
void
ScrollBarCtrlClass::On_Set_Focus (void)
{
	//
	//	Force a repaint
	//
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
ScrollBarCtrlClass::On_Kill_Focus (DialogControlClass *focus)
{
	IsDragging					= false;
	WasTopButtonPressed		= false;
	WasBottomButtonPressed	= false;

	//
	//	Force a repaint
	//
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
ScrollBarCtrlClass::On_Key_Down (uint32 key_id, uint32 /* key_data */)
{
	bool handled = true;

	switch (key_id)
	{
		case VK_UP:
		case VK_LEFT:
			Set_Pos (CurrPos - 1);
			break;

		case VK_DOWN:
		case VK_RIGHT:
			Set_Pos (CurrPos + 1);
			break;

		case VK_HOME:
			Set_Pos (MinPos);
			break;

		case VK_END:
			Set_Pos (MaxPos);
			break;

		//
		//	Page up
		//
		case VK_PRIOR:
			Scroll_Page (-1);
			break;

		//
		//	Page down
		//
		case VK_NEXT:
			Scroll_Page (1);
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
ScrollBarCtrlClass::On_Create (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Slider_Pos_From_Mouse_Pos
//
////////////////////////////////////////////////////////////////
int
ScrollBarCtrlClass::Slider_Pos_From_Mouse_Pos (const Vector2 &mouse_pos)
{
	int retval = 0;
	
	if (mouse_pos.Y < TrackRect.Top) {
		
		//
		//	Mouse is above the track area of the control
		//
		retval = 0;
	} else if (mouse_pos.Y >= TrackRect.Bottom) {
		
		//
		//	Mouse is below the track areaa of the control
		//
		retval = MaxPos;
	} else {

		//
		//	Calculate where in the range the mouse is and return its position
		//
		float percent = (mouse_pos.Y - TrackRect.Top) / TrackRect.Height ();
		retval = MinPos + int(percent * (MaxPos - MinPos));
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Set_Range
//
////////////////////////////////////////////////////////////////
void
ScrollBarCtrlClass::Set_Range (int range_min, int range_max)
{
	/*if (MinPos != range_min) {
		Set_Pos (range_min);
	}*/

	MinPos = range_min;
	MaxPos = std::max (range_min, range_max);
	Update_Thumb_Rect ();
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Scroll_Page
//
////////////////////////////////////////////////////////////////
void
ScrollBarCtrlClass::Scroll_Page (int direction, bool send_notify)
{
	//
	//	Calculate a page scroll (if using uniform page sizes)	
	//
	int pos = CurrPos + (direction * PageSize);
	Set_Pos (pos, send_notify);

	//
	//	Notify the advise sinks that the page has been scrolled
	//
	if (send_notify) {		
		ADVISE_NOTIFY (On_VScroll_Page (this, Get_ID (), direction));		
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Pos
//
////////////////////////////////////////////////////////////////
void
ScrollBarCtrlClass::Set_Pos (int pos, bool send_notify)
{
	//
	//	Bound the new position
	//
	if (pos < MinPos) {
		pos = MinPos;
	} else if (pos > MaxPos) {
		pos = MaxPos;
	}

	//
	//	Update the current position
	//
	if (pos != CurrPos) {
		CurrPos = pos;

		//
		// Update the thumb's drag rectangle
		//
		Update_Thumb_Rect ();
		Set_Dirty ();

		//
		//	Notify the advise sink(s)
		//
		if (send_notify) {
			if (Parent != NULL) {
				Parent->On_VScroll (this, Get_ID (), CurrPos);
			}

			if (AdviseSink != NULL) {
				AdviseSink->On_VScroll (this, Get_ID (), CurrPos);
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Thumb_Rect
//
////////////////////////////////////////////////////////////////
void
ScrollBarCtrlClass::Update_Thumb_Rect (void)
{
	//
	//	Recalculate the thumb position
	//
	float percent		= float(CurrPos - MinPos) / float(MaxPos - MinPos);
	
	//
	//	Build the rectangle from the thumb's position
	//

	if (IsSmallBMPMode) {
		ThumbRect.Left		= int(ClientRect.Left + (ClientRect.Width () / 2) - (ThumbWidth / 2));
		ThumbRect.Right	= int(ThumbRect.Left + ThumbWidth);
	} else {
		ThumbRect.Right	= ClientRect.Right - 1;
		ThumbRect.Left		= ThumbRect.Right - ThumbWidth;
	}

	ThumbRect.Top		= int(TopButtonRect.Bottom + (TrackRect.Height () * percent));
	ThumbRect.Bottom	= int(ThumbRect.Top + ThumbHeight);
	return ;
}

