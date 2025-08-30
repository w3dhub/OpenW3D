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
 *                     $Archive:: /Commando/Code/Commando/dlgmpslaveservers.h                 $*
 *                                                                                             *
 *                       Author:: Steve Tall                                                   *
 *                                                                                             *
 *                     $Modtime:: 2/11/02 11:01a                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef _DLGMPSLAVESERVER_H
#define _DLGMPSLAVESERVER_H


#include "menudialog.h"
#include "resource.h"
#include "dialogresource.h"
#include "slavemaster.h"


class SlaveServerDialogClass : public MenuDialogClass
{
	public:

		SlaveServerDialogClass(void);

		void On_Init_Dialog(void) override;
		void On_Command(int ctrl_id, int mesage_id, DWORD param) override;
		void On_Destroy(void) override;
		void Load_Settings(int slavenum);
		static void Set_Slave_Settings(StringClass *file_name);
		static void Set_Slave_Button(int slavenum);

	private:

		static unsigned long EnableIDs[MAX_SLAVES];
		static unsigned long NickIDs[MAX_SLAVES];
		static unsigned long PassIDs[MAX_SLAVES];
		static unsigned long SerialIDs[MAX_SLAVES];
		static unsigned long PortIDs[MAX_SLAVES];
		static unsigned long SettingsButtons[MAX_SLAVES];

		static char ServerSettingsFileNames[MAX_SLAVES][MAX_PATH];

		static int SlaveNumber;

		static SlaveServerDialogClass *Instance;


};


#endif //_DLGMPSLAVESERVER_H