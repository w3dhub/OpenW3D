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
 *                     $Archive:: /Commando/Code/wwui/editctrl.h           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/09/02 11:09a                                              $*
 *                                                                                             *
 *                    $Revision:: 19                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __EDIT_CTRL_H
#define __EDIT_CTRL_H

#include "dialogcontrol.h"
#include "vector3.h"
#include "render2dsentence.h"
#include "bittype.h"
#include "IMEManager.h"
#include "IMECandidateCtrl.h"
#include "tooltip.h"

////////////////////////////////////////////////////////////////
//
//	EditCtrlClass
//
////////////////////////////////////////////////////////////////
class EditCtrlClass :
	public DialogControlClass,
	public Observer<IME::CompositionEvent>,
	public Observer<IME::CandidateEvent>
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	EditCtrlClass (void);
	virtual ~EditCtrlClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	RTTI
	//	
	EditCtrlClass *	As_EditCtrlClass (void)	override { return this; }

	//
	//	From DialogControlClass
	//
	void					Render (void) override;
	virtual void		Set_Text (const unichar_t *title) override;

	int					Get_Text_Length (void) const;

	void					Set_Text_Limit (int numChars);
	int					Get_Text_Limit (void) const;

	//
	//	Content control
	//
	bool					Delete_Selection (void);
	
	int					Get_Int (void);
	void					Set_Int (int value);

	//
	//	Caret and selection support
	//
	int					Get_Caret_Pos (void) const;
	void					Set_Caret_Pos (int new_pos);
	void					Set_Sel (int start_index, int end_index);
	void					Get_Sel (int &start_index, int &end_index) const;

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void					On_LButton_Down (const Vector2 &mouse_pos) override;
	void					On_LButton_Up (const Vector2 &mouse_pos) override;
	void					On_Mouse_Move (const Vector2 &mouse_pos) override;
	void					On_Set_Cursor (const Vector2 &mouse_pos) override;
	void					On_Set_Focus (void) override;
	void					On_Kill_Focus (DialogControlClass *focus) override;
	bool					On_Key_Down (uint32 key_id, uint32 key_data) override;
	void					On_Create (void) override;
	void					Update_Client_Rect (void) override;

	void					Create_Control_Renderers (void);
	void					Create_Text_Renderers (void);
	void					Create_Caret_Renderer (void);
	void					Update_Caret (void);	

	int					Character_From_Pos (const Vector2 &mouse_pos);
	float					Pos_From_Character (int char_index);

	void					On_Unicode_Char (unichar_t unicode) override;
	void					Insert_String (const unichar_t *string);

	void					Update_Hilight (int new_pos, int anchor_pos);
	int					Find_Word_Start (int pos, int increment);
	void					Update_Scroll_Pos (void);

	bool IsIMEAllowed(void) const;

	void Set_IME_Typing_Text_Pos(void);
	void Show_IME_Typing_Text(const unichar_t* text);
	void Hide_IME_Typing_Text(void);
	void PositionCandidateList(void);

	void					Get_Display_Text (WideStringClass &text);

	void					HandleNotification(IME::CompositionEvent&) override;
	void					HandleNotification(IME::CandidateEvent&) override;

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	Render2DSentenceClass	TextRenderer;
	Render2DClass				ControlRenderer;
	Render2DClass				CaretRenderer;
	Render2DClass				HilightRenderer;
	bool							IsCaretDisplayed;
	uint32						CaretBlinkDelay;
	uint32						LastCaretBlink;
	int							CaretPos;
	int							ScrollPos;
	int NumChars;
	int TextLimit;

	int							HilightAnchorPos;
	int							HilightStartPos;
	int							HilightEndPos;

	bool							WasButtonPressedOnMe;

	IME::IMEManager* mIME;
	bool mInComposition;

#ifdef SHOW_IME_TYPING
	bool mShowIMETypingText;
	ToolTipClass mIMETypingTip;
#endif

	IMECandidateCtrl mCandidateList;
};

#endif //__EDIT_CTRL_H
