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
 *                     $Archive:: /Commando/Code/wwui/dropdownctrl.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/16/01 10:27p                                             $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DROPDOWN_CTRL_H
#define __DROPDOWN_CTRL_H


#include "dialogcontrol.h"
#include "vector.h"
#include "render2dsentence.h"
#include "bittype.h"
#include "scrollbarctrl.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class ComboBoxCtrlClass;


////////////////////////////////////////////////////////////////
//
//	DropDownCtrlClass
//
////////////////////////////////////////////////////////////////
class DropDownCtrlClass : public DialogControlClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	DropDownCtrlClass (void);
	virtual ~DropDownCtrlClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	From DialogControlClass
	//
	void					Render (void) override;

	//
	//	Content control
	//
	int					Add_String (const unichar_t* string);
	void					Delete_String (int index);
	int					Find_String (const unichar_t* string);
	int					Find_Closest_String(const unichar_t* string);
	void					Set_Item_Data (int index, uintptr_t data);
	uintptr_t				Get_Item_Data (int index);
	void					Reset_Content (void);
	
	//
	//	Information accessors
	//
	bool					Get_String (int index, WideStringClass &string) const;
	const unichar_t *		Get_String (int index) const;
	int					Get_Count (void)							{ return EntryList.Count (); }
	
	//
	//	Selection management
	//
	void					Set_Curr_Sel (int index);
	int					Get_Curr_Sel (void) const				{ return CurrSel; }

	//
	//	Combobox access
	//
	ComboBoxCtrlClass *	Get_Combo_Box (void) const						{ return ComboBox; }
	void						Set_Combo_Box (ComboBoxCtrlClass *ctrl)	{ ComboBox = ctrl; }

	void						Set_Full_Rect (const RectClass &rect)		{ FullRect = rect; }

	//
	//	Advise-sink callbacks
	//
	void					On_VScroll (ScrollBarCtrlClass *scrollbar, int ctrl_id, int new_position) override;


protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void					On_LButton_Down (const Vector2 &mouse_pos) override;
	void					On_LButton_Up (const Vector2 &mouse_pos) override;
	void					On_Mouse_Move (const Vector2 &mouse_pos) override;
	void					On_Set_Cursor (const Vector2 &mouse_pos) override;
	void					On_Kill_Focus (DialogControlClass *focus) override;
	bool					On_Key_Down (uint32 key_id, uint32 key_data) override;
	void					On_Create (void) override;
	void					On_Add_To_Dialog (void) override;
	void					On_Remove_From_Dialog (void) override;
	void					Update_Client_Rect (void) override;

	void					Create_Control_Renderer (void);
	void					Create_Text_Renderer (void);

	int					Entry_From_Pos (const Vector2 &mouse_pos);
	void					Update_Scroll_Pos (void);


	////////////////////////////////////////////////////////////////
	//	Protected data types
	////////////////////////////////////////////////////////////////
	typedef struct _ENTRY
	{
		WideStringClass	text;
		uintptr_t				user_data;

		bool operator== (const _ENTRY &/* src*/)	{ return false; }
		bool operator!= (const _ENTRY &/* src*/)	{ return true; }

		_ENTRY (void)	:
			user_data (0) {}

		_ENTRY (const unichar_t *_text, uintptr_t _data)	:
			text (_text), user_data (_data) {}

	} ENTRY;

	typedef DynamicVectorClass<ENTRY>	ENTRY_LIST;


	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	Render2DSentenceClass	TextRenderer;
	Render2DClass				ControlRenderer;
	Render2DClass				HilightRenderer;
	ENTRY_LIST					EntryList;
	ComboBoxCtrlClass *		ComboBox;
	Vector2						CellSize;
	ScrollBarCtrlClass		ScrollBarCtrl;
	RectClass					FullRect;
	RectClass					ScrollBarRect;
	int							CurrSel;
	int							ScrollPos;
	int							CountPerPage;
	bool							DisplayScrollBar;
	bool							ButtonClickedOnMe;
	int							DisplayTime;
};


#endif //__DROPDOWN_CTRL_H
