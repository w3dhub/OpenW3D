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
 *                     $Archive:: /Commando/Code/wwui/listctrl.h           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/26/01 4:10p                                              $*
 *                                                                                             *
 *                    $Revision:: 19                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __LIST_CTRL_H
#define __LIST_CTRL_H


#include "dialogcontrol.h"
#include "vector3.h"
#include "vector.h"
#include "render2d.h"
#include "bittype.h"
#include "scrollbarctrl.h"
#include "render2dsentence.h"
#include "listiconmgr.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class ListColumnClass;
class ListCtrlClass;
class ListRowClass;
class TextureClass;


////////////////////////////////////////////////////////////////
//	Typedefs
////////////////////////////////////////////////////////////////
typedef int (*LISTCTRL_SORT_CALLBACK) (ListCtrlClass *list_ctrl, int item_index1, int item_index2, uint32 user_param);


////////////////////////////////////////////////////////////////
//
//	ListCtrlClass
//
////////////////////////////////////////////////////////////////
class ListCtrlClass : public DialogControlClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constants
	////////////////////////////////////////////////////////////////
	typedef enum
	{
		SORT_NONE			= 0,
		SORT_ASCENDING,
		SORT_DESCENDING
	} SORT_TYPE;

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	ListCtrlClass (void);
	virtual ~ListCtrlClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	// RTTI
	virtual ListCtrlClass *				As_ListCtrlClass (void) override			{ return this; }

	//
	//	From DialogControlClass
	//
	void				Render (void) override;

	void				Set_Tabstop(float stop);

	//
	//	Column support
	//
	void				Auto_Size_Columns (float col_spacing = 2.0F);
	void				Auto_Size_Columns_Include_Contents (float col_spacing = 2.0F);
	void				Add_Column (const unichar_t *column_name, float width, const Vector3 &color);
	void				Set_Column_Color (int col_index, const Vector3 &color);
	bool				Remove_Column (int col_index);
	void				Delete_All_Columns (void);
	int				Get_Column_Count(void) const;
	
	//
	//	Content control
	//
	int Find_Entry(int col_index, const unichar_t* text);
	int				Insert_Entry (int index, const unichar_t *text);
	bool				Set_Entry_Text (int index, int col_index, const unichar_t *text);
	bool				Set_Entry_Int (int index, int col_index, int value);
	bool				Set_Entry_Color (int index, int col_index, const Vector3 &color);
	bool				Set_Entry_Data (int index, int col_index, uintptr_t user_data);
	bool				Select_Entry (int index, bool onoff);
	uintptr_t			Get_Entry_Data (int index, int col_index);
	const unichar_t *	Get_Entry_Text (int index, int col_index);
	bool				Is_Entry_Selected (int index);
	bool				Delete_Entry (int index);
	void				Delete_All_Entries (void);
	int				Get_Entry_Count (void) const;

	//
	//	Row height support
	//
	void				Set_Min_Row_Height (int height);
	int				Get_Min_Row_Height (void) const	{ return MinRowHeight; }

	//
	//	Icon support
	//
	void				Add_Icon (int index, int col_index, const char *texture_name);
	void				Reset_Icons (int index, int col_index);
	void				Set_Icon_Size (float width, float height);

	//
	//	Selection support
	//
	int				Get_Curr_Sel (void) const		{ return CurrSel; }
	void				Set_Curr_Sel (int new_sel);

	void				Allow_Selection(bool onoff) {IsSelectionAllowed = onoff;}
	void				Allow_NoSelection(bool onoff) {IsNoSelectionAllowed = onoff;}
	void				Allow_Multiple_Selection (bool onoff)	{ IsMultipleSelection = onoff; }
	int				Get_First_Selected (void) const;
	int				Get_Next_Selected (int index) const;
	void				Select_All (bool select = true);
	bool				Toggle_Entry_Selection (int index);

	//
	//	Sort support
	//
	void				Sort (LISTCTRL_SORT_CALLBACK sort_callback, uint32 user_param);
	void				Set_Sort_Designator (int col_index, SORT_TYPE type);
	void				Sort_Alphabetically (int col_index, SORT_TYPE type);

	//
	//	Scroll support
	//
	void				Scroll_To_End (void);

	//
	//	Advise-sink callbacks
	//
	void				On_VScroll (ScrollBarCtrlClass *scrollbar, int ctrl_id, int new_position) override;
	void				On_VScroll_Page (ScrollBarCtrlClass *scrollbar, int ctrl_id, int direction) override;

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void				Set_Sel (int new_sel, bool notify);

	void				On_LButton_Down (const Vector2 &mouse_pos) override;
	void				On_LButton_DblClk (const Vector2 &mouse_pos) override;
	void				On_LButton_Up (const Vector2 &mouse_pos) override;
	void				On_Mouse_Move (const Vector2 &mouse_pos) override;
	void				On_Mouse_Wheel (int direction) override;
	void				On_Set_Cursor (const Vector2 &mouse_pos) override;
	void				On_Set_Focus (void) override;
	void				On_Kill_Focus (DialogControlClass *focus) override;
	bool				On_Key_Down (uint32 key_id, uint32 key_data) override;
	void				On_Create (void) override;
	void				On_Destroy (void) override;
	void				Update_Client_Rect (void) override;

	void				Create_Control_Renderer (void);
	void				Create_Text_Renderers (void);
	int				Entry_From_Pos (const Vector2 &mouse_pos);
	int				Col_From_Pos (const Vector2 &mouse_pos);
	void				Get_Entry_Rect (int index, RectClass &rect);
	void				Update_Scroll_Pos (void);
	void				Update_Scroll_Bar_Visibility (void);
	void				Update_Row_Height (int row_index);
	void				Quick_Sort (int start_index, int end_index, LISTCTRL_SORT_CALLBACK sort_callback, uint32 user_param);
	int				Find_Last_Page_Top_Entry (void);
	void				Scroll_Page (int direction);
	int				Find_End_Of_Page (void);
	int				Find_Top_Of_Page (int bottom_index);
	
	void				Render_Entry (const RectClass &rect, int col_index, int row_index);

	static int 	Default_Sort_Callback (ListCtrlClass *list_ctrl, int item_index1, int item_index2, uint32 user_param);


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
	//	Protected typedefs
	////////////////////////////////////////////////////////////////
	typedef DynamicVectorClass<ListColumnClass *>	COL_LIST;
	typedef DynamicVectorClass<ListRowClass *>		ROW_LIST;

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	Render2DClass				HilightRenderer;
	Render2DClass				ControlRenderer;
	Render2DClass				UnderlineRenderer;	
	Render2DSentenceClass	HeaderRenderer;
	Render2DSentenceClass	TextRenderer;
	ListIconMgrClass			IconMgr;
	int							RowBorderHeight;
	int							ScrollPos;
	int							CurrSel;
	int							CurrState;
	int							LastPageTopEntryIndex;
	ScrollBarCtrlClass		ScrollBarCtrl;
	bool							IsScrollBarDisplayed;
	bool							IsSelectionAllowed;
	bool							IsNoSelectionAllowed;
	bool							IsMultipleSelection;
	int							SortColumn;
	SORT_TYPE					SortType;
									
	float							PulsePercent;
	float							PulseDirection;
	float							MinRowHeight;
									
	RectClass					TextRect;
	RectClass					HeaderRect;
									
	COL_LIST						ColList;
	ROW_LIST						RowInfoList;
};


////////////////////////////////////////////////////////////////
//
//	ListEntryClass
//
////////////////////////////////////////////////////////////////
class ListEntryClass
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	ListEntryClass (void) :
		Name (U_CHAR("")),
		Color (1, 1, 1),
		UserData (0)	{}

	ListEntryClass (const unichar_t *name) :
		Name (name),
		Color (1, 1, 1),
		UserData (0)	{}

	~ListEntryClass (void) { Reset_Icons (); }

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	
	//
	//	Name access
	//
	const unichar_t *		Get_Name (void) const			{ return Name; }
	void					Set_Name (const unichar_t *name)	{ Name = name; }

	//
	//	Color access
	//
	const Vector3 &	Get_Color (void) const				{ return Color; }
	void					Set_Color (const Vector3 &color)	{ Color = color; }

	//
	//	User data access
	//
	uintptr_t				Get_User_Data (void) const			{ return UserData; }
	void					Set_User_Data (uintptr_t user_data)	{ UserData = user_data; }

	//
	//	Icon support
	//
	int					Get_Icon_Count (void) const		{ return IconList.Count (); }
	const char *		Get_Icon (int index)					{ return IconList[index]; }
	void					Add_Icon (const char *name)		{ IconList.Add (name); }
	void					Reset_Icons (void)					{ IconList.Delete_All (); }

private:

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	WideStringClass						Name;
	Vector3									Color;
	uintptr_t									UserData;
	DynamicVectorClass<StringClass>	IconList;
};


////////////////////////////////////////////////////////////////
//
//	ListColumnClass
//
////////////////////////////////////////////////////////////////
class ListColumnClass
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	ListColumnClass (void) :
		Width (0)					{ Reset_Contents (); }

	~ListColumnClass (void)		{ Free_Data (); }

	////////////////////////////////////////////////////////////////
	//	Public operators
	////////////////////////////////////////////////////////////////
	bool					operator== (const ListColumnClass &/* src*/) const	{ return false; }
	bool					operator!= (const ListColumnClass &/* src*/) const	{ return true; }


	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	
	//
	//	Name access
	//
	const unichar_t *		Get_Name (void) const			{ return Header.Get_Name (); }
	void					Set_Name (const unichar_t *name)	{ Header.Set_Name (name); }

	//
	//	Width access
	//
	float					Get_Width (void) const			{ return Width; }
	void					Set_Width (float width)			{ Width = width; }

	//
	//	Color access
	//
	const Vector3 &	Get_Color (void) const				{ return Header.Get_Color (); }
	void					Set_Color (const Vector3 &color)	{ Header.Set_Color (color); }

	//
	//	Entry access
	//
	void					Move_Entry (int old_index, int new_index);
	void					Swap_Entries (int index1, int index2);
	int					Insert_Entry (int index, const unichar_t *entry_name);
	int					Get_Entry_Count (void) const								{ return EntryList.Count (); }
	bool					Delete_Entry (int index);
	void					Delete_All_Entries (void);
	
	void					Set_Entry_Text (int index, const unichar_t *text)		{ EntryList[index]->Set_Name (text); }
	const unichar_t *		Get_Entry_Text (int index) const							{ return EntryList[index]->Get_Name (); }	
	
	void					Set_Entry_Color (int index, const Vector3 &color)	{ EntryList[index]->Set_Color (color); }
	const Vector3 &	Get_Entry_Color (int index) const						{ return EntryList[index]->Get_Color (); }

	void					Set_Entry_Data (int index, uintptr_t data)				{ EntryList[index]->Set_User_Data (data); }
	uintptr_t				Get_Entry_Data (int index) const							{ return EntryList[index]->Get_User_Data (); }

	int					Get_Icon_Count (int index) const							{ return EntryList[index]->Get_Icon_Count (); }
	const char *		Get_Icon (int index, int icon_index)					{ return EntryList[index]->Get_Icon (icon_index); }
	void					Add_Icon (int index, const char *name)					{ EntryList[index]->Add_Icon (name); }
	void					Reset_Icons (int index)										{ EntryList[index]->Reset_Icons (); }

	//
	//	Cleanup
	//
	void					Reset_Contents (void);

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	void					Free_Data (void);	

	////////////////////////////////////////////////////////////////
	//	Private data types
	////////////////////////////////////////////////////////////////
	typedef DynamicVectorClass<ListEntryClass *> ENTRY_LIST;

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	ListEntryClass		Header;
	ENTRY_LIST			EntryList;
	float					Width;
};


////////////////////////////////////////////////////////////////
//
//	ListRowClass
//
////////////////////////////////////////////////////////////////
class ListRowClass
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	ListRowClass (void) :
		Height (0),
		IsSelected (false)	{ }

	~ListRowClass (void)		{ }

	////////////////////////////////////////////////////////////////
	//	Public operators
	////////////////////////////////////////////////////////////////
	bool					operator== (const ListRowClass &/* src*/) const	{ return false; }
	bool					operator!= (const ListRowClass &/* src*/) const	{ return true; }


	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	
	//
	//	Height access
	//
	float					Get_Height (void) const			{ return Height; }
	void					Set_Height (float height)		{ Height = height; }

	//
	//	Selection access
	//
	bool					Is_Selected (void) const		{ return IsSelected; }
	void					Select (bool onoff)				{ IsSelected = onoff; }

private:

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	float			Height;
	bool			IsSelected;
};


#endif //__LIST_CTRL_H

