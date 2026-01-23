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
 *                     $Archive:: /Commando/Code/wwui/controladvisesink.h        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/15/02 5:34p                                               $*
 *                                                                                             *
 *                    $Revision:: 20                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __CONTROL_ADVISE_SINK_H
#define __CONTROL_ADVISE_SINK_H


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class DialogControlClass;
class ScrollBarCtrlClass;
class ListCtrlClass;
class MapCtrlClass;
class ViewerCtrlClass;
class RenderObjClass;
class Vector3;
class InputCtrlClass;
class SliderCtrlClass;
class EditCtrlClass;
class ComboBoxCtrlClass;
class MerchandiseCtrlClass;
class TreeCtrlClass;
class TreeItemClass;


////////////////////////////////////////////////////////////////
//
//	ControlAdviseSinkClass
//
//		This is an abstrct base class that defines an interface
// which dialog controls can communicate state/activation
// changes via.
//
////////////////////////////////////////////////////////////////
class ControlAdviseSinkClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	ControlAdviseSinkClass (void)					{}
	virtual ~ControlAdviseSinkClass (void)		{}

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	
	//
	//	Generic callbacks
	//
	virtual bool	On_Command (DialogControlClass * /* control */, int /* ctrl_id */, int /* param1 */, int /* param2 */)	{ return false; }
	
	//
	//	Scrollbar callbacks
	//
	virtual void	On_VScroll (ScrollBarCtrlClass * /* scrollbar */, int /* ctrl_id */, int /* new_position */)			{}
	virtual void	On_VScroll_Page (ScrollBarCtrlClass * /* scrollbar */, int /* ctrl_id */, int /* direction */)		{}

	//
	//	List control callbacks
	//
	virtual void	On_ListCtrl_Delete_Entry (ListCtrlClass * /* list_ctrl */, int /* ctrl_id */, int /* item_index */)					{}
	virtual void	On_ListCtrl_Column_Click (ListCtrlClass * /* list_ctrl */, int /* ctrl_id */, int /* col_index */)					{}
	virtual void	On_ListCtrl_DblClk (ListCtrlClass * /* list_ctrl */, int /* ctrl_id */, int /* item_index */)							{}
	virtual void	On_ListCtrl_Sel_Change (ListCtrlClass * /* list_ctrl */, int /* ctrl_id */, int /* old_index */, int /* new_index */)	{}
	virtual void	On_ListCtrl_Mouse_Over (ListCtrlClass * /* list_ctrl */, int /* ctrl_id */, int /* index */)	{}

	//
	//	Tree control callbacks
	
	virtual void	On_TreeCtrl_Expand_Item (TreeCtrlClass * /* list_ctrl */, int /* ctrl_id */, TreeItemClass * /* item */)		{}
	virtual void	On_TreeCtrl_Delete_Item (TreeCtrlClass * /* list_ctrl */, int /* ctrl_id */, TreeItemClass * /* item */)		{}
	virtual void	On_TreeCtrl_Sel_Change (TreeCtrlClass * /* list_ctrl */, int /* ctrl_id */, TreeItemClass * /* old_item */, TreeItemClass * /* new_item */)	{}
	virtual void	On_TreeCtrl_Needs_Children (TreeCtrlClass * /* list_ctrl */, int /* ctrl_id */, TreeItemClass * /* parent_item */)	{}

	//
	//	Map control callbacks
	//
	virtual void	On_MapCtrl_Marker_Hilighted (MapCtrlClass * /* map_ctrl */, int /* ctrl_id */, int /* marker_index */)	{}
	virtual void	On_MapCtrl_Marker_Clicked (MapCtrlClass * /* map_ctrl */, int /* ctrl_id */, int /* marker_index */)	{}
	virtual void	On_MapCtrl_Pos_Clicked (MapCtrlClass * /* map_ctrl */, int /* ctrl_id */, const Vector3 &/* pos */)		{}

	//
	//	Viewer control callbacks
	//
	virtual void	On_ViewerCtrl_Model_Loaded (ViewerCtrlClass * /* viewer_ctrl */, int /* ctrl_id */, RenderObjClass * /* model */)	{}

	//
	//	Input control callbacks
	//
	virtual bool	On_InputCtrl_Get_Key_Info (InputCtrlClass * /* viewer_ctrl */, int /* ctrl_id */, int /* vkey_id */, WideStringClass &/* key_name */, int * /* game_key_id */)	{ return false; }

	//
	//	Slider control callbacks	
	//
	virtual void	On_SliderCtrl_Pos_Changed (SliderCtrlClass * /* slider_ctrl */, int /* ctrl_id */, int /* new_pos */) {}

	//
	//	Edit control callbacks
	//
	virtual void	On_EditCtrl_Enter_Pressed (EditCtrlClass * /* edit_ctrl */, int /* ctrl_id */) {}
	virtual bool	On_EditCtrl_Key_Down(EditCtrlClass * /* edit_ctrl */, uint32 /* key_id */, uint32 /* key_data */) {return false;}
	virtual void	On_EditCtrl_Change(EditCtrlClass * /* edit_ctrl */, int /* ctrl_id */) {}

	//
	//	Combo control callbacks
	//
	virtual void	On_ComboBoxCtrl_Sel_Change (ComboBoxCtrlClass * /* combo_ctrl */, int /* ctrl_id */, int /* old_sel */, int /* new_sel */)	{}
	virtual void	On_ComboBoxCtrl_Edit_Change (ComboBoxCtrlClass * /* combo_ctrl */, int /* ctrl_id */) {}

	//
	//	Merchandise control callbacks
	//
	virtual void	On_Merchandise_Selected (MerchandiseCtrlClass * /* ctrl */, int /* ctrl_id */) {}
	virtual void	On_Merchandise_DblClk (MerchandiseCtrlClass * /* ctrl */, int /* ctrl_id */) {}
};


#endif //__CONTROL_ADVISE_SINK_H
