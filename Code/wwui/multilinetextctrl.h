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
 *                 Project Name : wwui																			  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwui/multilinetextctrl.h                     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/09/02 5:35p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __MULTILINETEXTCTRL_H
#define __MULTILINETEXTCTRL_H


#include "render2d.h"
#include "render2dsentence.h"
#include "scrollbarctrl.h"


//////////////////////////////////////////////////////////////////////
//
//	MultiLineTextCtrlClass
//
//////////////////////////////////////////////////////////////////////
class MultiLineTextCtrlClass : public DialogControlClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	MultiLineTextCtrlClass  (void);
	~MultiLineTextCtrlClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Inherited
	//
	void				Render (void) override;
	void				Set_Text (const unichar_t *title) override;

	//
	//	Advise-sink callbacks
	//
	void				On_VScroll (ScrollBarCtrlClass *scrollbar, int ctrl_id, int new_position) override;

protected:
	
	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////
	void				Update_Client_Rect (void) override;
	bool				On_Key_Down (uint32 key_id, uint32 key_data) override;
	void				On_Mouse_Wheel (int direction) override;

	void				Create_Control_Renderer (void);
	void				Create_Text_Renderer (void);

	void				Calculate_Row_Count (void);
	void				Update_Scroll_Bar_Visibility (void);	
	void				Set_Scroll_Pos (int new_position);
	
	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////
	Render2DClass				ControlRenderer;
	Render2DSentenceClass	TextRenderer;
	ScrollBarCtrlClass		ScrollBarCtrl;
	bool							IsInitialized;
	int							ScrollPos;
	bool							IsScrollBarDisplayed;
	int							RowCount;
	int							RowsPerPage;
	float							MouseWheelIncrement;
};


#endif //__MULTILINETEXTCTRL_H
