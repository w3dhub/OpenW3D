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
 *                     $Archive:: /Commando/Code/wwui/menuentryctrl.h         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/19/01 11:12a                                              $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __MENU_ENTRY_CTRL_H
#define __MENU_ENTRY_CTRL_H

#include "dialogcontrol.h"
#include "vector3.h"
#include "render2dsentence.h"


////////////////////////////////////////////////////////////////
//
//	MenuEntryCtrlClass
//
////////////////////////////////////////////////////////////////
class MenuEntryCtrlClass : public DialogControlClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	MenuEntryCtrlClass (void);
	virtual ~MenuEntryCtrlClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	From DialogControlClass
	//
	void					Render (void) override;
	bool					Wants_Tooltip (void) const override	{ return false; }
	void					Center_Mouse (void) override;

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////

	//
	//	From DialogControlClass
	//
	void					On_LButton_Down (const Vector2 &mouse_pos) override;
	void					On_LButton_Up (const Vector2 &mouse_pos) override;
	void					On_Mouse_Move (const Vector2 &mouse_pos) override;
	void					On_Set_Cursor (const Vector2 &mouse_pos) override;
	void					On_Create (void) override;
	void					On_Set_Focus (void) override;
	void					On_Kill_Focus (DialogControlClass *) override;
	bool					On_Key_Down (uint32 key_id, uint32 key_data) override;
	bool					On_Key_Up (uint32 key_id) override;
	void					On_Mouse_Wheel (int direction) override;
	void					On_Frame_Update (void) override;
	void					Update_Client_Rect (void) override;

	void					Create_Text_Renderer (void);
	void					Set_State (int new_state);
	void					Update_State (void);

	void					Create_Glow (int radiusx, int radiusy, int color);
	void					On_Pushed (void);	


	////////////////////////////////////////////////////////////////
	//	Protected constants
	////////////////////////////////////////////////////////////////
	enum
	{
		UP				= 0,
		DOWN,
		HILIGHT,
		STATE_MAX
	};

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	Render2DSentenceClass	GlowRenderer;
	Render2DSentenceClass	TextRenderer;
	RectClass				MaxRect;
	bool						WasButtonPressedOnMe;
	bool						IsMouseOverMe;
	bool						TriggerCommand;
	int						CurrState;

	int						CurrRadiusX;
	int						CurrRadiusY;
	int						StartTime;
	int						EndTime;
	int						CurrColor;

	static int				MaxDefaultRedValue;
	static int				MaxHilightRedValue;
};


#endif //__MENU_ENTRY_CTRL_H
