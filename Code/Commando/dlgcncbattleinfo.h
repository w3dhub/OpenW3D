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
 *                 Project Name : commando                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/dlgcncbattleinfo.h                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/02/01 2:01p                                                $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLGCNCBATTLEINFO_H
#define __DLGCNCBATTLEINFO_H

#include "menudialog.h"

//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////
class ListCtrlClass;
class cPlayer;

//////////////////////////////////////////////////////////////////////
//
//	CNCBattleInfoDialogClass
//
//////////////////////////////////////////////////////////////////////
class CNCBattleInfoDialogClass : public MenuDialogClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	CNCBattleInfoDialogClass  (void);
	~CNCBattleInfoDialogClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Inherited
	//
	void		On_Init_Dialog (void) override;
	void		On_Frame_Update (void) override;
	
private:
	
	///////////////////////////////////////////////////////////////////
	//	Private methods
	///////////////////////////////////////////////////////////////////
	void		Configure_Icons (void);
	void		Populate_Player_List (ListCtrlClass *list_ctrl, int team_id);
	void		Build_Player_Display_Name(const cPlayer* player, WideStringClass& outName);
	
	static int	ListSortCallback (ListCtrlClass *list_ctrl, int item_index1, int item_index2, uint32 user_param);
	
	///////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////
};




#endif //__DLGCNCBATTLEINFO_H
