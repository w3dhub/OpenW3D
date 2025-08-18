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
 *                     $Archive:: /Commando/Code/Commando/dlgmplangamelist.h       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/04/02 4:12p                                               $*
 *                                                                                             *
 *                    $Revision:: 12                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLG_MP_LAN_GAME_LIST_H
#define __DLG_MP_LAN_GAME_LIST_H


#include "menudialog.h"
#include "resource.h"
#include <wwlib/Signaler.h>

class DlgPasswordPrompt;

////////////////////////////////////////////////////////////////
//
//	MPLanGameListMenuClass
//
////////////////////////////////////////////////////////////////
class MPLanGameListMenuClass : public MenuDialogClass,
	protected Signaler<DlgPasswordPrompt>
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////	
	MPLanGameListMenuClass (void);
	~MPLanGameListMenuClass();

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	void		On_Init_Dialog (void);
	void		On_Destroy (void);
	void		On_Command (int ctrl_id, int mesage_id, DWORD param);
	bool		On_Key_Down(uint32 key_id, uint32 key_data);
	
	static void	Set_Update_Nickname(void)						{ UpdateNickname = true; }

	
	void		On_ListCtrl_Delete_Entry (ListCtrlClass *list_ctrl, int ctrl_id, int item_index);
	void		On_ListCtrl_DblClk (ListCtrlClass *list_ctrl, int ctrl_id, int item_index);
	void		On_EditCtrl_Change(EditCtrlClass* edit, int id);

	//
	//	Singleton access
	//
	static void								Display (void);
	static MPLanGameListMenuClass *	Get_Instance (void)	{ return _TheInstance; }

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////

	//
	//	Inherited
	//
	void		On_Last_Menu_Ending (void);
	void		On_Frame_Update (void);

	void		Update_Game_List (void);
	void		Join_Game (void);
	void		ReceiveSignal(DlgPasswordPrompt&);
	void		Connect_To_Server (void);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	int										UpdateTimer;
	static bool								UpdateNickname;

	static MPLanGameListMenuClass *	_TheInstance;

};


#endif //__DLG_MP_LAN_GAME_LIST_H

