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
 *                 Project Name : wwui                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwui/treectrl.h                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/18/01 1:47p                                                $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __TREECTRL_H
#define __TREECTRL_H


#include "dialogcontrol.h"
#include "vector.h"
#include "render2dsentence.h"
#include "scrollbarctrl.h"
#include "listiconmgr.h"


//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////
class TreeItemClass;
class TreeCtrlClass;


//////////////////////////////////////////////////////////////////////
//	Typedefs
//////////////////////////////////////////////////////////////////////
typedef DynamicVectorClass<TreeItemClass *>	TREE_ITEM_LIST;

typedef int (*TREECTRL_SORT_CALLBACK) (TreeCtrlClass *tree_ctrl, TreeItemClass *item1, TreeItemClass *item2, uint32 user_param);


//////////////////////////////////////////////////////////////////////
//
//	TreeCtrlClass
//
//////////////////////////////////////////////////////////////////////
class TreeCtrlClass : public DialogControlClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constants
	///////////////////////////////////////////////////////////////////
	typedef enum
	{
		HIT_PLUS			= 0,
		HIT_ICON,
		HIT_TEXT,
	} HITTYPE;

	static const char *ICON_FOLDER;
	static const char *ICON_FOLDER_OPEN;

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	TreeCtrlClass  (void);
	~TreeCtrlClass (void);

	// RTTI.
	virtual TreeCtrlClass *				As_TreeCtrlClass (void) override			{ return this; }

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////
	
	//
	//	Inherited
	//	
	void				Render (void) override;
	void				On_VScroll (ScrollBarCtrlClass *, int, int new_position) override;

	//
	//	Content control
	//
	TreeItemClass *	Insert_Item (const unichar_t *name, const char *icon_name, const char *selected_icon_name, TreeItemClass *parent);
	void					Delete_Item (TreeItemClass *item);
	void					Delete_All_Items (void);

	void					Select_Item (TreeItemClass *item);
	TreeItemClass *	Get_Selected_Item (void);

	//
	//	Sort support
	//
	void					Sort_Children_Alphabetically (TreeItemClass *parent);
	void					Sort_Children (TreeItemClass *parent, TREECTRL_SORT_CALLBACK sort_callback, uint32 user_param);

	//
	//	Misc
	//
	TreeItemClass *	Hit_Test (const Vector2 &mouse_pos, HITTYPE &type);
	void					Ensure_Visible (TreeItemClass *tree_item);

protected:
	
	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void				On_LButton_Down (const Vector2 &mouse_pos) override;
	void				On_LButton_DblClk (const Vector2 &mouse_pos) override;
	void				On_LButton_Up (const Vector2 &mouse_pos) override;
	void				On_Mouse_Wheel (int direction) override;
	void				On_Set_Cursor (const Vector2 &mouse_pos) override;
	void				On_Set_Focus (void) override;
	void				On_Kill_Focus (DialogControlClass *focus) override;
	bool				On_Key_Down (uint32 key_id, uint32 key_data) override;
	void				On_Create (void) override;
	void				Update_Client_Rect (void) override;
	void				On_Expanded (TreeItemClass *item);

	void				Create_Control_Renderers (void);
	void				Create_Text_Renderers (void);

	void				Set_Scroll_Pos (int new_pos);
	void				Update_Scroll_Bar_Visibility (void);
	int				Count_Visible_Rows (void);
	int				Count_Visible_Rows (TreeItemClass *item);	
	
	TreeItemClass *	Get_Prev_Sibling (TreeItemClass *item);
	TreeItemClass *	Get_Next_Sibling (TreeItemClass *item);	
	TreeItemClass *	Find_Prev_Visible (TreeItemClass *item);
	TreeItemClass *	Find_Next_Visible (TreeItemClass *item);
	TreeItemClass *	Find_Top_Item (void);
	TreeItemClass *	Find_Last_Visible_Item (void);
	
	bool				Render_Item (TreeItemClass *item, float x_pos, float &y_pos, int &row_index, int level);

	static int __cdecl	Sort_Callback (const void *elem1, const void *elem2);
	static int 		Alphabetic_Sort_Callback (TreeCtrlClass *tree_ctrl, TreeItemClass *item1, TreeItemClass *item2, uint32 user_param);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	Render2DClass				HilightRenderer;
	Render2DSentenceClass	TextRenderer;
	Render2DClass				ControlRenderer;
	Render2DClass				PlusRenderer;
	Render2DClass				IconRenderer;
	TREE_ITEM_LIST				ItemList;
	TreeItemClass *			SelectedItem;
	ListIconMgrClass			IconMgr;
	float							RowHeight;
	int							RowsPerPage;
	int							ScrollPos;
	ScrollBarCtrlClass		ScrollBarCtrl;
	bool							IsScrollBarDisplayed;

	static TREECTRL_SORT_CALLBACK CurrentSortCallback;
	static TreeCtrlClass *			CurrentSorter;
	static uint32						CurrentSortUserData;

	friend class TreeItemClass;
};


//////////////////////////////////////////////////////////////////////
//
//	TreeItemClass
//
//////////////////////////////////////////////////////////////////////
class TreeItemClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	TreeItemClass  (TreeCtrlClass *ctrl)	:
		TreeCtrl (ctrl),
		Parent (NULL),
		UserData (0),
		IsExpanded (false),
		NeedsChildren (false)		{}

	virtual ~TreeItemClass (void)	{}

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Name access
	//
	void				Set_Name (const unichar_t *name);
	const unichar_t *	Get_Name (void) const				{ return Name; }

	//
	//	Icon access
	//
	void				Set_Icon (const char *texture_name);
	const char *	Get_Icon (void) const				{ return IconName; }

	void				Set_Selected_Icon (const char *texture_name);
	const char *	Get_Selected_Icon (void) const	{ return SelectedIconName; }

	//
	//	User data access
	//
	void				Set_User_Data (uint32 data)		{ UserData = data; }
	uint32			Get_User_Data (void) const			{ return UserData; }

	//
	//	Selection control
	//
	void				Select (void)							{ TreeCtrl->Select_Item (this); }
	bool				Is_Selected (void)					{ return (TreeCtrl->Get_Selected_Item () == this); }

	//
	//	Expanded access
	//
	void				Expand (bool onoff);
	bool				Is_Expanded (void) const			{ return IsExpanded; }

	//
	//	Child access
	//	
	void				Add_Child (TreeItemClass *child)	{ ChildList.Add (child); NeedsChildren = false; }
	void				Remove_Child (int index)			{ ChildList.Delete (index); }
	void				Remove_Child (TreeItemClass *child);

	bool				Needs_Children (void) const		{ return NeedsChildren; }
	void				Set_Needs_Children (bool onoff);

	int					Get_Child_Count (void) const	{ return ChildList.Count (); }
	TreeItemClass *	Get_Child (int index)			{ return ChildList[index]; }

	TREE_ITEM_LIST &	Get_Child_List (void)			{ return ChildList; }
	TreeItemClass *	Get_Last_Child (void);

	//
	//	Parent access
	//
	TreeItemClass *	Get_Prev_Child (TreeItemClass *child);
	TreeItemClass *	Get_Next_Child (TreeItemClass *child);

	void					Set_Parent (TreeItemClass *parent)	{ Parent = parent; }
	TreeItemClass *	Get_Parent (void) const					{ return Parent; }

	//
	//	Indent access
	//
	int					Get_Indent_Level (void);

protected:
	
	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	TreeCtrlClass *	TreeCtrl;
	TreeItemClass *	Parent;
	WideStringClass	Name;
	StringClass			IconName;
	StringClass			SelectedIconName;
	uint32				UserData;
	TREE_ITEM_LIST		ChildList;
	bool					IsExpanded;
	bool					NeedsChildren;
};


#endif //__TREECTRL_H


