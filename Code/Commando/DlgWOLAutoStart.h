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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/DlgWOLAutoStart.h                   $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 11/06/01 11:06a                                             $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once
#ifndef _DLGWOLAUTOSTART_H
#define _DLGWOLAUTOSTART_H

#include "menudialog.h"
#include "resource.h"

/*
** This dialog just shows progress during a server auto start.
**
**
*/
class AutoRestartProgressDialogClass : public MenuDialogClass
{
	public:
		AutoRestartProgressDialogClass(void);
		void On_Init_Dialog(void);
		void On_Command(int ctrl_id, int message_id, DWORD param);
		void Add_Text(wchar_t *txt);
		static AutoRestartProgressDialogClass *Get_Instance(void) {return(Instance);}

	private:
		int AddItemIndex;
		static AutoRestartProgressDialogClass *Instance;
};



#endif //_DLGWOLAUTOSTART_H

