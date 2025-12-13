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
 *                     $Archive:: /Commando/Code/Commando/dlgmpingameteamchat.h       $*
 *                                                                                             *
 *                       Author:: Tom SS                                                *
 *                                                                                             *
 *                     $Modtime:: 10/12/01 3:50p                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLGMPINGAMETEAMCHAT_H__
#define __DLGMPINGAMETEAMCHAT_H__

#include "renegadedialog.h"
#include "popupdialog.h"


////////////////////////////////////////////////////////////////
//
//	MPIngameTeamChatPopupClass
//
////////////////////////////////////////////////////////////////
class MPIngameTeamChatPopupClass : public PopupDialogClass
{
public:
	MPIngameTeamChatPopupClass (void)	:
		PopupDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MULTIPLAY_INGAME_TEAM_CHAT))	{}

	void	On_Init_Dialog (void) override;
	bool	On_Key_Down (uint32 key_id, uint32 key_data) override;

private:
};


#endif //__DLGMPINGAMETEAMCHAT_H__
