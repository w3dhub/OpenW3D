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
 *                     $Archive:: /Commando/Code/Commando/dlgcncteaminfo.h                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/02/01 11:28a                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLGCNCTEAMINFO_H
#define __DLGCNCTEAMINFO_H

#include "menudialog.h"

class cPlayer;

//////////////////////////////////////////////////////////////////////
//
//	CNCTeamInfoDialogClass
//
//////////////////////////////////////////////////////////////////////
class CNCTeamInfoDialogClass : public MenuDialogClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	CNCTeamInfoDialogClass  (void);
	~CNCTeamInfoDialogClass (void);

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
	void		Populate_Player_List (void);
	void		Build_Player_Display_Name(const cPlayer* player, WideStringClass& outName);
	
	static int	ListSortCallback (ListCtrlClass *list_ctrl, int item_index1, int item_index2, uint32 user_param);
	
	///////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////
};


#endif //__DLGCNCTEAMINFO_H
