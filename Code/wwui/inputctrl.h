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
 *                     $Archive:: /Commando/Code/wwui/inputctrl.h           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/15/02 6:33p                                               $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __INPUTBOX_CTRL_H
#define __INPUTBOX_CTRL_H

#include "dialogcontrol.h"
#include "vector3.h"
#include "render2dsentence.h"
#include "bittype.h"


////////////////////////////////////////////////////////////////
//	Special virtual keys
////////////////////////////////////////////////////////////////
#define VK_MOUSEWHEEL_UP      0x100
#define VK_MOUSEWHEEL_DOWN    0x101


////////////////////////////////////////////////////////////////
//
//	InputCtrlClass
//
////////////////////////////////////////////////////////////////
class InputCtrlClass : public DialogControlClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	InputCtrlClass (void);
	virtual ~InputCtrlClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	From DialogControlClass
	//
	void				Render (void) override;

	//
	//	Key assignment access
	//
	int				Get_Key_Assignment (void) const		{ return KeyAssignment; }
	void				Set_Key_Assignment (int game_key_id, const WideStringClass &key_name);

	//
	//	User data support
	//
	uintptr_t			Get_User_Data (void)				{ return UserData; }
	void				Set_User_Data (uintptr_t data)	{ UserData = data; }

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void				On_LButton_Down (const Vector2 &mouse_pos) override;
	void				On_RButton_Down (const Vector2 &mouse_pos) override;
	void				On_MButton_Down (const Vector2 &mouse_pos) override;
	void				On_LButton_Up (const Vector2 &mouse_pos) override;
	void				On_Set_Cursor (const Vector2 &mouse_pos) override;
	void				On_Set_Focus (void) override;
	void				On_Kill_Focus (DialogControlClass *focus) override;
	bool				On_Key_Down (uint32 key_id, uint32 key_data) override;
	void				On_Create (void) override;
	void				On_Mouse_Wheel (int direction) override;
	void				Update_Client_Rect (void) override;
	
	void				On_New_Key (int vkey_id);

	void				Create_Control_Renderers (void);
	void				Create_Text_Renderers (void);	

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	Render2DSentenceClass	TextRenderer;
	Render2DClass				ControlRenderer;
	Render2DClass				HilightRenderer;
	int							KeyAssignment;
	int							MouseIgnoreTime;
	uintptr_t						UserData;
	int							PendingKeyID;
};


#endif //__INPUTBOX_CTRL_H
