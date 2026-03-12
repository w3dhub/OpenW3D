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
 *                     $Archive:: /Commando/Code/Commando/dlgmpingameteamchat.cpp       $*
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

#include "dlgmpingameteamchat.h"

#include "cnetwork.h"
#include "dialogcontrol.h"
#include "dialogmgr.h"
#include "gamedata.h"
#include "cstextobj.h"
#include "sctextobj.h"


////////////////////////////////////////////////////////////////
//
//	MPIngameTeamChatPopupClass
//
////////////////////////////////////////////////////////////////
void
MPIngameTeamChatPopupClass::On_Init_Dialog (void)
{
	WWASSERT(The_Game() != NULL);

	Set_Background_Darkened(false);

	PopupDialogClass::On_Init_Dialog ();
}


////////////////////////////////////////////////////////////////
//
//	On_Key_Down
//
////////////////////////////////////////////////////////////////
bool
MPIngameTeamChatPopupClass::On_Key_Down (uint32 key_id, uint32 key_data)
{
	bool retval = false;

	retval = PopupDialogClass::On_Key_Down (key_id, key_data);

	if (key_id == VK_RETURN)
	{
		WideStringClass msg = Get_Dlg_Item_Text(IDC_MULTIPLAY_INGAME_CHAT_EDIT);
		if (msg.Get_Length() > 0)
		{
			cCsTextObj * p_message = new cCsTextObj;
			p_message->Init(msg, TEXT_MESSAGE_TEAM, cNetwork::Get_My_Id(), -1);
		}
		End_Dialog ();
	}

	return retval;
}

