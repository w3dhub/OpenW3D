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
 *                     $Archive:: /Commando/Code/wwui/checkboxctrl.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/05/01 4:20p                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __CHECKBOX_CTRL_H
#define __CHECKBOX_CTRL_H

#include "dialogcontrol.h"
#include "vector3.h"
#include "render2dsentence.h"
#include "bittype.h"


////////////////////////////////////////////////////////////////
//
//	CheckBoxCtrlClass
//
////////////////////////////////////////////////////////////////
class CheckBoxCtrlClass : public DialogControlClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	CheckBoxCtrlClass (void);
	virtual ~CheckBoxCtrlClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	RTTI
	//	
	CheckBoxCtrlClass *		As_CheckBoxCtrlClass (void) override	{ return this; }

	//
	//	From DialogControlClass
	//	
	void				Render (void) override;

	//
	//	Selection management
	//
	void				Set_Check (bool onoff);
	bool				Get_Check (void) const		{ return IsChecked; }

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void				Internal_Set_Check(bool onoff, bool notify);

	void				On_LButton_Down (const Vector2 &mouse_pos) override;
	void				On_LButton_Up (const Vector2 &mouse_pos) override;
	void				On_Set_Cursor (const Vector2 &mouse_pos) override;
	void				On_Set_Focus (void) override;
	void				On_Kill_Focus (DialogControlClass *focus) override;
	bool				On_Key_Down (uint32 key_id, uint32 key_data) override;
	void				On_Create (void) override;
	void				Update_Client_Rect (void) override;

	void				Create_Control_Renderers (void);
	void				Create_Text_Renderers (void);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	Render2DSentenceClass	TextRenderer;
	Render2DClass				ControlRenderer;
	Render2DClass				HilightRenderer;
	bool							IsChecked;
	RectClass					ButtonRect;
	RectClass					TextRect;
	RectClass					MaxRect;
};


#endif //__CHECKBOX_CTRL_H
