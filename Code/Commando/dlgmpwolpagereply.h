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
 *                     $Archive:: /Commando/Code/Commando/dlgmpwolpagereply.h       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/20/01 6:50p                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLG_MP_WOL_PAGE_REPLY_H
#define __DLG_MP_WOL_PAGE_REPLY_H

#include "popupdialog.h"
#include "WOLBuddyMgr.h"

class EditCtrlClass;

class DlgWOLPageReply :
		public PopupDialogClass,
		public Observer<WOLPagedEvent>
	{
	public:
		static bool DoDialog(void);
		static bool IsOpen(void);

	protected:
		DlgWOLPageReply(void);
		~DlgWOLPageReply();

		// Delcared here to prevent copy and assignment
		DlgWOLPageReply(const DlgWOLPageReply&);
		const DlgWOLPageReply& operator=(const DlgWOLPageReply&);

		bool FinalizeCreate(void);

		void On_Init_Dialog(void) override;
		void On_Command(int ctrl_id, int mesage_id, unsigned int param) override;

		void Send_Reply(void);
		void Add_Message(const unichar_t* username, const unichar_t* message);

		void HandleNotification(WOLPagedEvent&) override;
		void On_EditCtrl_Enter_Pressed(EditCtrlClass* edit, int ID) override;

	protected:
		WOLBuddyMgr* mBuddyMgr;
		static DlgWOLPageReply* _mInstance;
	};

#endif //__DLG_MP_WOL_PAGE_REPLY_H
