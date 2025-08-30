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
 *                     $Archive:: /Commando/Code/wwui/tabctrl.h           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/06/01 3:51p                                              $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __TAB_CTRL_H
#define __TAB_CTRL_H

#include "dialogcontrol.h"
#include "vector3.h"
#include "vector.h"
#include "render2dsentence.h"
#include "bittype.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class ChildDialogClass;


////////////////////////////////////////////////////////////////
//	Useful macros
////////////////////////////////////////////////////////////////
#define	TABCTRL_ADD_TAB(tab_ctrl, class_name) {	\
	ChildDialogClass *new_tab = new class_name;		\
	tab_ctrl->Add_Tab (new_tab);							\
	REF_PTR_RELEASE (new_tab); }


////////////////////////////////////////////////////////////////
//
//	TabCtrlClass
//
////////////////////////////////////////////////////////////////
class TabCtrlClass : public DialogControlClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	TabCtrlClass (void);
	virtual ~TabCtrlClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	From DialogControlClass
	//
	void				Render (void) override;
	bool				Wants_Tooltip (void) const override	{ return false; }

	//
	//	Tab control
	//
	void				Add_Tab (ChildDialogClass *dialog);
	void				Remove_Tab (int index);
	int				Get_Tab_Count (void) const { return TabList.Count (); }

	//
	//	Selection control
	//
	void				Set_Curr_Tab (int index);
	int				Get_Curr_Tab (void) const	{ return CurrTabIndex; }

	//
	//	Apply/Discard support
	//
	//		Note: If either of these methods return false, then the
	// dialog should NOT advance.
	//
	bool				Apply_Changes_On_Tabs (void);
	bool				Discard_Changes_On_Tabs (void);

	void				Reload_Tabs (void);

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void				On_LButton_Down (const Vector2 &mouse_pos) override;
	void				On_LButton_Up (const Vector2 &mouse_pos) override;
	void				On_Mouse_Move (const Vector2 &mouse_pos) override;
	void				On_Mouse_Wheel (int direction) override;
	void				On_Set_Cursor (const Vector2 &mouse_pos) override;
	void				On_Set_Focus (void) override;
	void				On_Kill_Focus (DialogControlClass *focus) override;
	bool				On_Key_Down (uint32 key_id, uint32 key_data) override;
	void				On_Create (void) override;
	void				Update_Client_Rect (void) override;

	void				Create_Control_Renderer (void);
	void				Create_Text_Renderer (void);
	void				Create_Glow_Renderer (void);

	int				Tab_From_Pos (const Vector2 &mouse_pos);
	float				Pos_From_Tab (int index);
	void				Free_Tabs (void);

	void				Update_Bubble (void);
	void				Update_Selector (void);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	Render2DClass				ControlRenderer;
	Render2DSentenceClass 	HilightRenderer;
	Render2DSentenceClass	TextRenderer;
	Render2DSentenceClass	HilightGlowRenderer;
	Render2DSentenceClass	GlowRenderer;
	int							CurrTabIndex;
	int							NextBlinkTime;
	bool							IsBubbleDisplayed;
	float							SelectorPos;
	float							ScaleX;
	float							ScaleY;
	RectClass					SelRect;

	DynamicVectorClass<ChildDialogClass *>	TabList;
};


#endif //__TAB_CTRL_H

