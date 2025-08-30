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
 *                     $Archive:: /Commando/Code/wwui/scrollbarctrl.h         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/01/01 10:48p                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __SCROLLBAR_CTRL_H
#define __SCROLLBAR_CTRL_H


#include "dialogcontrol.h"
#include "vector3.h"
#include "render2d.h"
#include "bittype.h"


////////////////////////////////////////////////////////////////
//
//	ScrollBarCtrlClass
//
////////////////////////////////////////////////////////////////
class ScrollBarCtrlClass : public DialogControlClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	ScrollBarCtrlClass (void);
	virtual ~ScrollBarCtrlClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	From DialogControlClass
	//
	void				Render (void) override;
	bool				Wants_Tooltip (void) const override	{ return false; }

	//
	//	Selection management
	//
	void				Set_Range (int range_min, int range_max);
	void				Set_Pos (int pos, bool send_notify = true);
	void				Set_Page_Size (int size)						{ PageSize = size; }
	int				Get_Pos (void) const								{ return CurrPos; }

	//
	//	Button mode
	//
	void				Set_Small_BMP_Mode (bool onoff)	{ IsSmallBMPMode = onoff; Set_Dirty (); }

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

	void				Create_Control_Renderers (void);
	void				Create_Button_Renderer (void);
	int				Slider_Pos_From_Mouse_Pos (const Vector2 &mouse_pos);
	void				Update_Thumb_Rect (void);
	void				Scroll_Page (int direction, bool send_notify = true);


	////////////////////////////////////////////////////////////////
	//	Protected constants
	////////////////////////////////////////////////////////////////
	enum
	{
		NORMAL				= 0,
		DISABLED,
		STATE_MAX
	};

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	Render2DClass			ControlRenderers[STATE_MAX];
	Render2DClass			ButtonRenderer;
	bool						WasButtonPressedOnMe;
	int						MinPos;
	int						MaxPos;	
	int						CurrPos;
	int						PageSize;
	int						CurrState;
	RectClass				ThumbRect;
	RectClass				TrackRect;
	RectClass				TopButtonRect;
	RectClass				BottomButtonRect;
	Vector2					MouseClickPos;
	float						MouseClickSliderPos;
	int						ThumbWidth;
	int						ThumbHeight;
	bool						IsSmallBMPMode;

	bool						IsDragging;
	bool						WasTopButtonPressed;
	bool						WasBottomButtonPressed;

	float						Width;
	float						ScaleX;
	float						ScaleY;
};


#endif //__SCROLLBAR_CTRL_H

