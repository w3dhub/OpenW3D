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
 *                     $Archive:: /Commando/Code/wwui/inputctrl.cpp         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/17/02 11:18a                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */



#include "inputctrl.h"
#include "assetmgr.h"
#include "refcount.h"
#include "font3d.h"
#include "mousemgr.h"
#include "ww3d.h"
#include "dialogmgr.h"
#include "dialogbase.h"
#include "stylemgr.h"


////////////////////////////////////////////////////////////////
//
//	InputCtrlClass
//
////////////////////////////////////////////////////////////////
InputCtrlClass::InputCtrlClass (void)	:
	KeyAssignment (0),
	MouseIgnoreTime (0),
	UserData (0),
	PendingKeyID (-1)
{
	//
	//	Set the font for the text renderers
	//
	StyleMgrClass::Assign_Font (&TextRenderer, StyleMgrClass::FONT_CONTROLS);
	StyleMgrClass::Configure_Renderer (&ControlRenderer);	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~InputCtrlClass
//
////////////////////////////////////////////////////////////////
InputCtrlClass::~InputCtrlClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Text_Renderers
//
////////////////////////////////////////////////////////////////
void
InputCtrlClass::Create_Text_Renderers (void)
{
	HilightRenderer.Reset ();
	HilightRenderer.Set_Coordinate_Range (Render2DClass::Get_Screen_Resolution());		
	StyleMgrClass::Configure_Hilighter (&HilightRenderer);

	//
	//	Start fresh
	//
	TextRenderer.Reset ();

	//
	//	Draw the text
	//
	StyleMgrClass::Render_Text (Title, &TextRenderer, ClientRect, true, true,
							StyleMgrClass::CENTER_JUSTIFY, IsEnabled);

	//
	//	Do the hilight
	//
	if (HasFocus) {
		StyleMgrClass::Render_Hilight (&HilightRenderer, ClientRect);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Control_Renderers
//
////////////////////////////////////////////////////////////////
void
InputCtrlClass::Create_Control_Renderers (void)
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
	renderer.Add_Outline (Rect, 1.0F, color);

	//
	//	Now draw the background
	//
	RectClass rect = Rect;
	rect.Right	-= 1;
	rect.Bottom -= 1;
	renderer.Add_Quad (rect, bkcolor);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Cursor
//
////////////////////////////////////////////////////////////////
void
InputCtrlClass::On_Set_Cursor (const Vector2 &/* mouse_pos */)
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
InputCtrlClass::Update_Client_Rect (void)
{
	//
	//	Set the client area
	//
	ClientRect = Rect;
	ClientRect.Inflate (Vector2 (-1, -1));
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
InputCtrlClass::Render (void)
{
	if (PendingKeyID != -1) {
		
		//
		//	Use the pending key
		//
		if (PendingKeyID >= 0 && PendingKeyID <= 256) {
			On_New_Key (PendingKeyID);
		}

		PendingKeyID = -1;
	}

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
InputCtrlClass::On_LButton_Down (const Vector2 &/* mouse_pos */)
{
	if (HasFocus && DialogMgrClass::Get_Time () > MouseIgnoreTime) {
		On_New_Key (VK_LBUTTON);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_RButton_Down
//
////////////////////////////////////////////////////////////////
void
InputCtrlClass::On_RButton_Down (const Vector2 &/* mouse_pos */)
{
	if (HasFocus) {
		On_New_Key (VK_RBUTTON);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_MButton_Down
//
////////////////////////////////////////////////////////////////
void
InputCtrlClass::On_MButton_Down (const Vector2 &/* mouse_pos */)
{
	if (HasFocus) {
		On_New_Key (VK_MBUTTON);
	}

	return ;
}

////////////////////////////////////////////////////////////////
//
//	On_LButton_Up
//
////////////////////////////////////////////////////////////////
void
InputCtrlClass::On_LButton_Up (const Vector2 &/* mouse_pos */)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Focus
//
////////////////////////////////////////////////////////////////
void
InputCtrlClass::On_Set_Focus (void)
{
	//
	//	Ignore mouse input for one second
	//
	MouseIgnoreTime = DialogMgrClass::Get_Time () + 500;
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
InputCtrlClass::On_Kill_Focus (DialogControlClass *focus)
{
	Set_Dirty ();

	DialogControlClass::On_Kill_Focus (focus);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_New_Key
//
////////////////////////////////////////////////////////////////
void
InputCtrlClass::On_New_Key (int vkey_id)
{
	if (Parent != NULL) {
		WideStringClass key_name;
		int game_key_id = 0;

		//
		//	Get information about this key from the parent
		//
		if (Parent->On_InputCtrl_Get_Key_Info (this, Get_ID (), vkey_id, key_name, &game_key_id)) {
			
			//
			//	Display this information in the control
			//
			Set_Key_Assignment (game_key_id, key_name);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Key_Assignment
//
////////////////////////////////////////////////////////////////
void
InputCtrlClass::Set_Key_Assignment (int game_key_id, const WideStringClass &key_name)
{
	KeyAssignment	= game_key_id;
	Title				= key_name;
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Key_Down
//
////////////////////////////////////////////////////////////////
bool
InputCtrlClass::On_Key_Down (uint32 key_id, uint32 /* key_data */)
{
	PendingKeyID = key_id;
	return false;
}


////////////////////////////////////////////////////////////////
//
//	On_Create
//
////////////////////////////////////////////////////////////////
void
InputCtrlClass::On_Create (void)
{
	Title = U_CHAR("");
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Mouse_Wheel
//
////////////////////////////////////////////////////////////////
void
InputCtrlClass::On_Mouse_Wheel (int direction)
{
	if (HasFocus) {

		if (direction < 0) {
			On_New_Key (VK_MOUSEWHEEL_UP);
		} else {
			On_New_Key (VK_MOUSEWHEEL_DOWN);
		}
	}

	return ;
}
