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
 *                     $Archive:: /Commando/Code/Commando/dlgmpwoladdignoreentry.h     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/30/01 12:09p                                             $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLG_MP_WOL_ADD_IGNORE_ENTRY_H
#define __DLG_MP_WOL_ADD_IGNORE_ENTRY_H

#include "popupdialog.h"
#include "wwstring.h"

class EditCtrlClass;

class MPWolAddIgnoreEntry :
		public PopupDialogClass
	{
	public:
		static bool DoDialog(const wchar_t* defaultUser);

	protected:
		MPWolAddIgnoreEntry(const wchar_t* defaultUser);
		~MPWolAddIgnoreEntry();

		// Prevent copy and assignment
		MPWolAddIgnoreEntry(const MPWolAddIgnoreEntry&);
		const MPWolAddIgnoreEntry& operator=(const MPWolAddIgnoreEntry&);

		void On_Init_Dialog (void) override;
		void On_Command (int ctrl_id, int mesage_id, unsigned int param) override;

		void Add_Ignore_Entry (void);

		void On_EditCtrl_Change(EditCtrlClass* edit, int id) override;
		void On_EditCtrl_Enter_Pressed(EditCtrlClass* edit, int id) override;

		WideStringClass mDefaultUser;
	};

#endif //__DLG_MP_WOL_ADD_IGNORE_ENTRY_H
