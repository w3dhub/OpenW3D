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
 *                     $Archive:: /Commando/Code/Commando/dlgmpwolgoodies.cpp    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/07/01 7:56p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if 0

#include "dlgmpwolgoodies.h"


////////////////////////////////////////////////////////////////
//
//	MPWolGoodiesMenuClass
//
////////////////////////////////////////////////////////////////
MPWolGoodiesMenuClass::MPWolGoodiesMenuClass (void)	:
	MenuDialogClass (IDD_MP_WOL_GOODIES)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
MPWolGoodiesMenuClass::On_Init_Dialog (void)
{
	MenuDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
MPWolGoodiesMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDC_MP_WOL_START_GAME_BUTTON:
			break;

		case IDC_MP_WOL_MANAGE_ACCOUNT_BUTTON:
			break;

		case IDC_MP_WOL_MANAGE_CLANS_BUTTON:
			break;

		case IDC_MP_WOL_VIEW_RANKINGS_BUTTON:
			break;

		case IDC_MP_WOL_EDIT_BUDDIES_BUTTON:
			break;

		/*
		case IDC_MP_WOL_GOODIES_BUTTON:
			break;
		*/
	}

	MenuDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}

#endif // 0