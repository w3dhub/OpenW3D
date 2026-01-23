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
 *                     $Archive:: /Commando/Code/wwui/menuentryctrl.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/02/02 11:01a                                              $*
 *                                                                                             *
 *                    $Revision:: 19                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "menuentryctrl.h"
#include "assetmgr.h"
#include "font3d.h"
#include "dialogbase.h"
#include "mousemgr.h"
#include "stylemgr.h"
#include "dialogmgr.h"
#include "texture.h"
#include "ww3d.h"


////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
int		MenuEntryCtrlClass::MaxDefaultRedValue		= 9;
int		MenuEntryCtrlClass::MaxHilightRedValue		= 16;


////////////////////////////////////////////////////////////////
//
//	MenuEntryCtrlClass
//
////////////////////////////////////////////////////////////////
MenuEntryCtrlClass::MenuEntryCtrlClass (void)	:
	CurrState (UP),
	WasButtonPressedOnMe (false),
	IsMouseOverMe (false),
	CurrRadiusX (5),
	CurrRadiusY (5),
	StartTime (0),
	EndTime (0),
	CurrColor (RGB_TO_INT32 (0, MaxDefaultRedValue, 0))
{
	//
	//	Get the current bits-per-pixel of the display
	//
	int foo = 0;
	int bits = 0;
	bool windowed = 0;
	WW3D::Get_Device_Resolution (foo, foo, bits, windowed);

	//
	//	If we are running at anything less then 32bpp, then
	// scale the glow colors
	//
	if (bits < 32) {
		MaxDefaultRedValue = 11;
		MaxHilightRedValue = 22;
	}

	//
	//	Set the font for the text renderer
	//
	StyleMgrClass::Assign_Font (&TextRenderer, StyleMgrClass::FONT_MENU);
	StyleMgrClass::Assign_Font (&GlowRenderer, StyleMgrClass::FONT_MENU);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~MenuEntryCtrlClass
//
////////////////////////////////////////////////////////////////
MenuEntryCtrlClass::~MenuEntryCtrlClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
MenuEntryCtrlClass::Render (void)
{
	//
	//	Refresh the cached renderer (if necessary)
	//
	if (Is_Dirty ()) {
		Create_Text_Renderer ();
	}

	//
	//	Render the menu text
	//
	GlowRenderer.Render ();
	TextRenderer.Render ();	

	DialogControlClass::Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
MenuEntryCtrlClass::On_Frame_Update (void)
{
	//
	//	Allow the current state to update
	//
	Update_State ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Create
//
////////////////////////////////////////////////////////////////
void
MenuEntryCtrlClass::On_Create (void)
{
	if ((Style & 0xF) == BS_OWNERDRAW) {

		//
		//	Set the font for the text renderer
		//
		StyleMgrClass::Assign_Font (&TextRenderer, StyleMgrClass::FONT_SM_MENU);
		StyleMgrClass::Assign_Font (&GlowRenderer, StyleMgrClass::FONT_SM_MENU);

		/*FontCharsClass *font = WW3DAssetManager::Get_Instance()->Get_FontChars ("Lucida Sans Unicode", 12);
		TextRenderer.Set_Font (font);
		GlowRenderer.Set_Font (font);
		font->Release_Ref ();*/
	}

	TextRenderer.Build_Sentence (Title);
	GlowRenderer.Build_Sentence (Title);

	//
	//	Determine what rectangle should be clickable
	//	
	Vector2 extents = TextRenderer.Get_Text_Extents (Title);
	Rect = MaxRect;

	//
	//	Should we left justify?
	//
	if ((Style & 0xF00) == BS_LEFT) {
		Rect.Right = Rect.Left + extents.X + TextRenderer.Get_Text_Extents (U_CHAR("W")).X;
	} else {	
		Rect.Left	= int(Rect.Left + (Rect.Width () / 2) - (extents.X / 2));
		Rect.Right	= Rect.Left + extents.X + TextRenderer.Get_Text_Extents (U_CHAR("W")).X;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Client_Rect
//
////////////////////////////////////////////////////////////////
void
MenuEntryCtrlClass::Update_Client_Rect (void)
{
	MaxRect = Rect;
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Mouse_Wheel
//
////////////////////////////////////////////////////////////////
void
MenuEntryCtrlClass::On_Mouse_Wheel (int direction)
{
	if (HasFocus) {

		//
		//	Find the next control to set the focus to...
		//
		DialogControlClass *control = Parent->Find_Next_Group_Control (this, direction);
		if (control != NULL) {
			control->Set_Focus ();
			control->Center_Mouse ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Down
//
////////////////////////////////////////////////////////////////
void
MenuEntryCtrlClass::On_LButton_Down (const Vector2 &mouse_pos)
{
	Set_Capture ();

	//
	//	Update our mouse flags
	//
	WasButtonPressedOnMe = true;
	IsMouseOverMe			= Rect.Contains (mouse_pos);
	if (IsMouseOverMe) {
		Set_State (DOWN);
	}
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Up
//
////////////////////////////////////////////////////////////////
void
MenuEntryCtrlClass::On_LButton_Up (const Vector2 &mouse_pos)
{
	Release_Capture ();

	//
	//	Update our mouse flags
	//
	WasButtonPressedOnMe	= false;
	IsMouseOverMe			= Rect.Contains (mouse_pos);

	//
	//	Switch states
	//
	if (CurrState != DOWN) {
		if (HasFocus) {
			if (IsMouseOverMe == false || CurrState != DOWN) {
				Set_State (HILIGHT);
			}
		} else {
			Set_State (UP);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Mouse_Move
//
////////////////////////////////////////////////////////////////
void
MenuEntryCtrlClass::On_Mouse_Move (const Vector2 &mouse_pos)
{
	//
	//	Force focus onto the control
	//
	Set_Focus ();

	//
	//	Check to see whether or not the mouse is over the control
	//
	IsMouseOverMe = Rect.Contains (mouse_pos);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_State
//
////////////////////////////////////////////////////////////////
void
MenuEntryCtrlClass::Set_State (int new_state)
{
	if (new_state != CurrState) {
		
		//
		//	Update the state and force a repaint
		//
		CurrState = new_state;
		Set_Dirty ();

		//
		//	Switch to the new state
		//
		switch (CurrState)
		{
			case UP:			
				CurrRadiusX = 5;
				CurrRadiusY = 5;
				break;

			case DOWN:
				StartTime	= DialogMgrClass::Get_Time ();
				EndTime		= StartTime + 300;
				CurrColor	= RGB_TO_INT32 (MaxHilightRedValue, 0, 0);

				//
				//	Play the sound effect
				//
				StyleMgrClass::Play_Sound (StyleMgrClass::EVENT_MOUSE_CLICK);
				break;

			case HILIGHT:
				StartTime	= DialogMgrClass::Get_Time ();
				EndTime		= StartTime + 1000;
				CurrColor	= RGB_TO_INT32 (MaxHilightRedValue, 0, 0);				
				break;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_State
//
////////////////////////////////////////////////////////////////
void
MenuEntryCtrlClass::Update_State (void)
{
	switch (CurrState)
	{
		case UP:
			break;

		case DOWN:
		{
			//
			//	Do we need to animate the glow?
			//
			int curr_time = DialogMgrClass::Get_Time ();
			if (curr_time < EndTime) {

				//
				//	Expand the glow
				//
				float percent = float(curr_time - StartTime) / float(EndTime - StartTime);
				CurrRadiusX = int(5.0F + (155.0F * percent));
				CurrRadiusY = int(5.0F + (25.0F * percent));
				CurrColor = RGB_TO_INT32 (MaxHilightRedValue * 3, 0, 0);

				uint32 time1 = StartTime;
				uint32 time2 = time1 + (EndTime - StartTime) / 2;
				uint32 time3 = EndTime;

				if (curr_time >= (int)time2) {

					Vector3 start_color (MaxHilightRedValue * 3, MaxHilightRedValue * 3.0F * 0.6F, 0);
					Vector3 end_color (0, 0, 0);					

					//
					//	Transition the color from start to finish
					//
					uint32 start_time	= time2;
					uint32 end_time	= time3;
					float percent		= float(float(curr_time - start_time) / float(end_time - start_time));

					Vector3 color = start_color + (end_color - start_color) * percent;

					CurrColor = RGB_TO_INT32 (color.X, color.Y, color.Z);

				} else {

					Vector3 start_color (MaxHilightRedValue * 3, 0, 0);
					Vector3 end_color (MaxHilightRedValue * 3, MaxHilightRedValue * 3.0F * 0.6F, 0);

					uint32 start_time	= time1;
					uint32 end_time	= time2;
					float percent		= float(float(curr_time - start_time) / float(end_time - start_time));

					Vector3 color = start_color + (end_color - start_color) * percent;
					CurrColor = RGB_TO_INT32 (color.X, color.Y, color.Z);
				}

				//
				//	Force update the display
				//
				Set_Dirty ();

			} else {
			
				if (CurrRadiusX != 160.0F) {
				
					//
					//	Snap the glow to its max
					//
					CurrRadiusX	= 160.0F;
					CurrRadiusY	= 60.0F;
					CurrColor	= RGB_TO_INT32 (0, 0, 0);
				}

				//
				//	Send the command (if necessary)
				//
				On_Pushed ();
				if (HasFocus) {
					Set_State (HILIGHT);
				} else {
					Set_State (UP);
				}
			}
			break;
		}

		case HILIGHT:
		{
			//
			//	Do we need to animate the glow?
			//
			int curr_time = DialogMgrClass::Get_Time ();
			if (curr_time < EndTime) {

				//
				//	Expand the glow
				//
				float percent = float(curr_time - StartTime) / float(EndTime - StartTime);
				CurrRadiusX = int(5.0F + (55.0F * percent));
				CurrRadiusY = CurrRadiusX;

				//
				//	Fade out the color
				//
				if (curr_time > (EndTime - 500)) {
					float start_time = EndTime - 500;
					float red_percent = float(float(curr_time - start_time) / float(EndTime - start_time));
					int red_value		= (1.0F - red_percent) * MaxHilightRedValue;
					CurrColor = RGB_TO_INT32 (red_value, 0, 0);
				}

				//
				//	Force update the display
				//
				Set_Dirty ();

			} else {
				
				//
				//	Snap the glow to its max
				//
				CurrRadiusX	= 5;
				CurrRadiusY = 5;
				CurrColor	= RGB_TO_INT32 (MaxHilightRedValue, 0, 0);

				//
				//	Restart the animation
				//
				StartTime	= curr_time;
				EndTime		= curr_time + 1000;
			}
			break;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Text_Renderer
//
////////////////////////////////////////////////////////////////
void
MenuEntryCtrlClass::Create_Text_Renderer (void)
{
	TextRenderer.Reset_Polys ();
	GlowRenderer.Reset_Polys ();

	//
	//	Get the extents of the text we will be drawing
	//
	Vector2 text_extent = TextRenderer.Get_Text_Extents (Title);
		
	//
	//	Assume cenetered text
	//		
	int x_pos = int(Rect.Left + (Rect.Width () / 2) - (text_extent.X / 2));
	int y_pos = int(Rect.Top + (Rect.Height () / 2) - (text_extent.Y / 2));

	//
	//	Should we left justify?
	//
	if ((Style & 0xF00) == BS_LEFT) {
		x_pos = int(Rect.Left + 1);
	}
		
	if (CurrState == UP) {
		
		//
		//	Render the glow
		//
		Create_Glow (CurrRadiusX, CurrRadiusY, RGB_TO_INT32 (MaxDefaultRedValue, 0, 0));
		
		//
		//	Render the text
		//
		TextRenderer.Set_Location (Vector2 (x_pos, y_pos));
		TextRenderer.Draw_Sentence (StyleMgrClass::Get_Text_Color ());

	} else if (CurrState == DOWN) {
		
		//
		//	Render the glow
		//
		Create_Glow (CurrRadiusX, CurrRadiusY, CurrColor);
		
		//
		//	Render the text
		//
		TextRenderer.Set_Location (Vector2 (x_pos + 1, y_pos + 1));
		TextRenderer.Draw_Sentence (RGB_TO_INT32 (0, 0, 0));

		TextRenderer.Set_Location (Vector2 (x_pos, y_pos));
		TextRenderer.Draw_Sentence (RGB_TO_INT32 (0, 0, 0));

	} else if (CurrState == HILIGHT) {
		
		//
		//	Render the glow
		//
		Create_Glow (CurrRadiusX, CurrRadiusY, CurrColor);
		
		TextRenderer.Set_Location (Vector2 (x_pos-1, y_pos-1));
		TextRenderer.Draw_Sentence (StyleMgrClass::Get_Text_Color ());

		TextRenderer.Set_Location (Vector2 (x_pos, y_pos));
		TextRenderer.Draw_Sentence (RGB_TO_INT32 (0, 0, 0));
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Cursor
//
////////////////////////////////////////////////////////////////
void
MenuEntryCtrlClass::On_Set_Cursor (const Vector2 &mouse_pos)
{
	//
	//	Change the mouse cursor if necessary
	//
	if (Rect.Contains (mouse_pos)) {
		MouseMgrClass::Set_Cursor (MouseMgrClass::CURSOR_ACTION);
	} else {
		MouseMgrClass::Set_Cursor (MouseMgrClass::CURSOR_ARROW);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Glow
//
////////////////////////////////////////////////////////////////
void
MenuEntryCtrlClass::Create_Glow (int radiusx, int radiusy, int color)
{
	GlowRenderer.Reset_Polys ();

	//
	//	Determine how to justify the text
	//
	StyleMgrClass::JUSTIFICATION justification = StyleMgrClass::CENTER_JUSTIFY;
	if ((Style & 0xF00) == BS_LEFT) {
		justification = StyleMgrClass::LEFT_JUSTIFY;
	}

	//
	//	Render the text in "glow" fashion
	//
	StyleMgrClass::Render_Glow (Title, &GlowRenderer, Rect, radiusx, radiusy,
							color, justification);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Focus
//
////////////////////////////////////////////////////////////////
void
MenuEntryCtrlClass::On_Set_Focus (void)
{
	//
	//	Set the state to hilight if the mouse button
	// isn't down
	//
	if (WasButtonPressedOnMe == false) {
		Set_State (HILIGHT);

		//
		//	Play the sound effect
		//
		StyleMgrClass::Play_Sound (StyleMgrClass::EVENT_MOUSE_OVER);
	}

	DialogControlClass::On_Set_Focus ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Kill_Focus
//
////////////////////////////////////////////////////////////////
void
MenuEntryCtrlClass::On_Kill_Focus (DialogControlClass *focus)
{
	//
	//	Restore the entry to its default state
	//
	if (CurrState != DOWN) {
		Set_State (UP);
	}

	WasButtonPressedOnMe	= false;

	DialogControlClass::On_Kill_Focus (focus);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Key_Down
//
////////////////////////////////////////////////////////////////
bool
MenuEntryCtrlClass::On_Key_Down (uint32 key_id, uint32 /* key_data */)
{
	bool handled = true;

	switch (key_id)
	{
		case VK_RETURN:
		case VK_SPACE:
			Set_State (DOWN);
			break;

		case VK_LEFT:
		case VK_UP:
		{
			//
			//	Set the focus to the previous control in our group
			//
			DialogControlClass *control = Parent->Find_Next_Group_Control (this, -1);
			if (control != NULL) {
				control->Set_Focus ();
			}
			break;
		}

		case VK_RIGHT:
		case VK_DOWN:
		{
			//
			//	Set the focus to the next control in our group
			//
			DialogControlClass *control = Parent->Find_Next_Group_Control (this, 1);
			if (control != NULL) {
				control->Set_Focus ();
			}
			break;
		}

		default:
			handled = false;
			break;
	}


	return handled;
}


////////////////////////////////////////////////////////////////
//
//	On_Key_Up
//
////////////////////////////////////////////////////////////////
bool
MenuEntryCtrlClass::On_Key_Up (uint32 /* key_id */)
{	
	return false;
}


////////////////////////////////////////////////////////////////
//
//	On_Pushed
//
////////////////////////////////////////////////////////////////
void
MenuEntryCtrlClass::On_Pushed (void)
{
	Parent->On_Command (ID, BN_CLICKED, 0);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Center_Mouse
//
////////////////////////////////////////////////////////////////
void
MenuEntryCtrlClass::Center_Mouse (void)
{
	//
	//	Get the extents of the text we will be drawing
	//
	Vector2 text_extent = TextRenderer.Get_Text_Extents (Title);
	
	int x_pos = 0;

	//
	//	Should we left justify?
	//
	if ((Style & 0xF00) == BS_LEFT) {
		x_pos = int(Rect.Left + (text_extent.X / 2));
	} else {
		x_pos = int(Rect.Left + (Rect.Width () / 2) - (text_extent.X / 2));
	}
		
	int y_pos = int(Rect.Top + (Rect.Height () / 2));

	//
	//	Put the mouse cursor in the middle of this control
	//
	Vector3 mouse_pos = DialogMgrClass::Get_Mouse_Pos ();
	mouse_pos.X = x_pos;
	mouse_pos.Y = y_pos;
	DialogMgrClass::Set_Mouse_Pos (mouse_pos);
	return ;
}


