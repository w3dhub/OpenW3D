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
 *                     $Archive:: /Commando/Code/wwui/shortcutbarctrl.h        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/13/01 9:53a                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __SHORTCUT_BAR_CTRL_H
#define __SHORTCUT_BAR_CTRL_H

#include "dialogcontrol.h"
#include "vector.h"
#include "render2dsentence.h"
#include "bittype.h"


////////////////////////////////////////////////////////////////
//
//	ShortcutBarCtrlClass
//
////////////////////////////////////////////////////////////////
class ShortcutBarCtrlClass : public DialogControlClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	ShortcutBarCtrlClass (void);
	virtual ~ShortcutBarCtrlClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	RTTI
	//	
	ShortcutBarCtrlClass *		As_ShortcutBarCtrlClass (void) override	{ return this; }

	//
	//	From DialogControlClass
	//	
	void					Render (void) override;
	const RectClass &	Get_Window_Rect (void) const override;
	bool					Wants_Tooltip (void) const override		{ return false; }

	//
	//	Configuration
	//
	void				Add_Button (int ctrl_id, const unichar_t *text);
	void				Remove_Button (int ctrl_id);

	void				Display (bool onoff)	{ IsHiddenState = !onoff; Set_Dirty (); }

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
	void				On_Mouse_Wheel (int direction) override;
	void				On_Create (void) override;
	void				On_Frame_Update (void) override;
	void				Update_Client_Rect (void) override;

	void				Create_Control_Renderers (void);
	void				Create_Hidden_State_Renderers (void);
	void				Create_Displayed_State_Renderers (void);
	void				Create_Text_Renderers (void);

	void				Set_Hidden_State (bool onoff);
	void				Get_Entry_Rect (int index, RectClass &rect);
	int				Entry_From_Pos (const Vector2 &mouse_pos);
	void				Set_Curr_Sel (int index, bool snap_mouse = false);
	void				Render_Strip (const RectClass &screen_rect, bool flip_uvs);
	void				Send_Command (void);
	void				Update_Pulse (void);


	////////////////////////////////////////////////////////////////
	//	Protected data types
	////////////////////////////////////////////////////////////////
	struct ShortcutInfoStruct
	{
		ShortcutInfoStruct (void) : ctrl_id (0) {}
		ShortcutInfoStruct (int _id, const unichar_t *_text) :
			ctrl_id (_id), text (_text) {}

		int					ctrl_id;
		WideStringClass	text;
		
		bool operator== (const ShortcutInfoStruct &/* src*/)	{ return false; }
		bool operator!= (const ShortcutInfoStruct &/* src*/)	{ return true; }
	};

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	DynamicVectorClass<ShortcutInfoStruct>	ShortcutList;
	Render2DSentenceClass	TextRenderer;
	Render2DSentenceClass	GlowRenderer;
	Render2DSentenceClass	HilightRenderer;
	Render2DClass				ControlRenderer;
	Render2DClass				TexturedControlRenderer;
	RectClass					FullRect;
	bool							IsHiddenState;
	int							CurrSel;
	float							BarOpacity;
	float							PulseDirection;
};


#endif //__SHORTCUT_BAR_CTRL_H
