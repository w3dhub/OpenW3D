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
 *                     $Archive:: /Commando/Code/Commando/dlgmpwolinvitation.h       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/07/01 7:09p                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLG_MP_WOL_INVITATION_H
#define __DLG_MP_WOL_INVITATION_H

#include "popupdialog.h"
#include <WWOnline/WOLUser.h>

class WOLBuddyMgr;

////////////////////////////////////////////////////////////////
//
//	MPWolInvitationPopupClass
//
////////////////////////////////////////////////////////////////
class MPWolInvitationPopupClass : public PopupDialogClass
{
public:
	MPWolInvitationPopupClass(const RefPtr<WWOnline::UserData>&, const wchar_t*);
	~MPWolInvitationPopupClass();

	void On_Init_Dialog(void) override;
	void On_Command(int ctrl_id, int mesage_id, unsigned int param) override;

protected:
	RefPtr<WWOnline::UserData> mHost;
	WideStringClass mInviteMessage;
	WOLBuddyMgr* mBuddyMgr;
};

#endif //__DLG_MP_WOL_INVITATION_H
