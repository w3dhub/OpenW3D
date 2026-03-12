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
 *                     $Archive:: /Commando/Code/wwui/dialogcontrol.cpp          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/26/01 12:07p                                             $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "dialogcontrol.h"
#include "dialogbase.h"
#include "mousemgr.h"
#include "dialogmgr.h"
#include "render2d.h"
#include "stylemgr.h"


////////////////////////////////////////////////////////////////
//
//	DialogControlClass
//
////////////////////////////////////////////////////////////////
DialogControlClass::DialogControlClass (void)	:
	Rect (0, 0, 0, 0),
	ClientRect (0, 0, 0, 0),
	Style (0),
	ID (0),
	Parent (NULL),
	HasFocus (false),
	IsDirty (true),
	WantsFocus (true),
	AdviseSink (NULL),
	IsEnabled (true),
	IsVisible (true),
	IsEmbedded (false),
	TextColor (0, 0, 0),
	IsTextColorOverridden (false)
{

	INT32_TO_VRGB (StyleMgrClass::Get_Text_Color (), TextColor);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~DialogControlClass
//
////////////////////////////////////////////////////////////////
DialogControlClass::~DialogControlClass (void)
{
	//
	//	Remove the input focus if this control has it
	//
	if (DialogMgrClass::Get_Focus () == this) {
		DialogMgrClass::Set_Focus (NULL);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Focus
//
////////////////////////////////////////////////////////////////
void
DialogControlClass::Set_Focus (void)
{
	DialogMgrClass::Set_Focus (this);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Cursor
//
////////////////////////////////////////////////////////////////
void
DialogControlClass::On_Set_Cursor (const Vector2 & /*mouse_pos*/)
{
	MouseMgrClass::Set_Cursor (MouseMgrClass::CURSOR_ARROW);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Capture
//
////////////////////////////////////////////////////////////////
void
DialogControlClass::Set_Capture (void)
{
	DialogMgrClass::Set_Capture (this);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Release_Capture
//
////////////////////////////////////////////////////////////////
void
DialogControlClass::Release_Capture (void)
{
	DialogMgrClass::Release_Capture ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Window_Pos
//
////////////////////////////////////////////////////////////////
void
DialogControlClass::Set_Window_Pos (const Vector2 &pos)
{
	float width		= Rect.Width ();
	float height	= Rect.Height ();

	//
	//	Recalculate the window's bounding rectangle
	//
	Rect.Left	= (int)pos.X;
	Rect.Top		= (int)pos.Y;
	Rect.Right	= (int)(Rect.Left + width);
	Rect.Bottom	= (int)(Rect.Top + height);

	//
	//	Let the control recalculate anything it needs
	//
	Update_Client_Rect ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Center_Mouse
//
////////////////////////////////////////////////////////////////
void
DialogControlClass::Center_Mouse (void)
{
	//
	//	Put the mouse cursor in the middle of this control
	//
	/*Vector3 mouse_pos = DialogMgrClass::Get_Mouse_Pos ();
	mouse_pos.X = Rect.Left + int(Rect.Width () / 2);
	mouse_pos.Y = Rect.Top + int(Rect.Height () / 2);
	DialogMgrClass::Set_Mouse_Pos (mouse_pos);*/
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Enable
//
////////////////////////////////////////////////////////////////
void
DialogControlClass::Enable (bool onoff)
{
	if (onoff != IsEnabled) {
		IsEnabled = onoff;
		Set_Dirty ();

		//
		//	Remove the focus (if necessary)
		//
		if (IsEnabled == false && DialogMgrClass::Get_Focus () == this) {
			DialogMgrClass::Set_Focus (NULL);
		}
	}

	return ;
}
