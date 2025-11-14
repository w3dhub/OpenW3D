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
 *                     $Archive:: /Commando/Code/commando/dlgmpwolquickmatch.cpp   $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 6/20/01 2:16p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgmpwolquickmatch.h"
#include "dlgmpwolquickmatchoptions.h"
#include "dlgmpwolgamelist.h"


////////////////////////////////////////////////////////////////
//
//	MPWolQuickMatchMenuClass
//
////////////////////////////////////////////////////////////////
MPWolQuickMatchMenuClass::MPWolQuickMatchMenuClass (void)	:
	MenuDialogClass (IDD_MP_WOL_QUICKMATCH)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
MPWolQuickMatchMenuClass::On_Init_Dialog (void)
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
MPWolQuickMatchMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDC_MP_WOL_QUICK_MATCH_BUTTON:
			break;

		case IDC_MP_WOL_QUICK_MATCH_COOP_BUTTON:
			break;

		case IDC_MP_WOL_QUICK_MATCH_OPTIONS_BUTTON:
			START_DIALOG (MPWolQuickMatchOptionsMenuClass);
			break;

		case IDC_MP_WOL_ADVANCED_LISTINGS_BUTTON:
			START_DIALOG (MPWolGameListMenuClass);
			break;
	}

	MenuDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}
