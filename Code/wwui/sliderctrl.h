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
 *                     $Archive:: /Commando/Code/wwui/sliderctrl.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/07/01 10:50a                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __SLIDER_CTRL_H
#define __SLIDER_CTRL_H

#include "dialogcontrol.h"
#include "vector3.h"
#include "render2d.h"
#include "bittype.h"


////////////////////////////////////////////////////////////////
//
//	SliderCtrlClass
//
////////////////////////////////////////////////////////////////
class SliderCtrlClass : public DialogControlClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	SliderCtrlClass (void);
	virtual ~SliderCtrlClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	From DialogControlClass
	//
	void				Render (void) override;

	//
	//	Selection management
	//
	void				Set_Range (int range_min, int range_max);
	void				Set_Pos (int pos, bool send_notification = true);
	int				Get_Pos (void) const								{ return CurrPos; }

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void				On_LButton_Down (const Vector2 &mouse_pos) override;
	void				On_LButton_Up (const Vector2 &mouse_pos) override;
	void				On_Mouse_Move (const Vector2 &mouse_pos) override;
	void				On_Set_Cursor (const Vector2 &mouse_pos) override;
	void				On_Set_Focus (void) override;
	void				On_Kill_Focus (DialogControlClass *focus) override;
	bool				On_Key_Down (uint32 key_id, uint32 key_data) override;
	void				On_Create (void) override;
	void				Update_Client_Rect (void) override;

	void				Create_Control_Renderer (void);
	int				Slider_Pos_From_Mouse_Pos (const Vector2 &mouse_pos);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	Render2DClass			ControlRenderer;
	bool						IsDragging;
	int						MinPos;
	int						MaxPos;
	int						CurrPos;
};


#endif //__SLIDER_CTRL_H

