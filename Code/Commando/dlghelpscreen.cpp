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
 *                     $Archive:: /Commando/Code/Commando/dlghelpscreen.cpp      $*
 *                                                                                             *
 *                       Author:: Byon Garrabrant                                                *
 *                                                                                             *
 *                     $Modtime:: 12/14/01 4:07p                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "renegadedialog.h"
#include "dlghelpscreen.h"
#if 0
#include "dialogresource.h"
#include "tabctrl.h"
#include "dlgevaobjectivestab.h"
#include "dlgevamaptab.h"
#include "dlgevadatatab.h"
#include "dlgevacharacterstab.h"
#include "dlgevaweaponstab.h"
#include "dlgevavehiclestab.h"
#include "dlgevabuildingstab.h"
#include "cnetwork.h"
#include "gamemode.h"
#include "wolgmode.h"
#include "dialogmgr.h"
#include "gametype.h"
#endif
#include "menubackdrop.h"
#include "gameinitmgr.h"
#include "input.h"

////////////////////////////////////////////////////////////////
//
//	HelpScreenDialogClass
//
////////////////////////////////////////////////////////////////
HelpScreenDialogClass::HelpScreenDialogClass (void)	:
	MenuDialogClass ( GetRenegadeDialog(RenegadeDialogID::IDD_HELP_SCREEN) )
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~HelpScreenDialogClass
//
////////////////////////////////////////////////////////////////
HelpScreenDialogClass::~HelpScreenDialogClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
HelpScreenDialogClass::On_Init_Dialog (void)
{
	MenuDialogClass::On_Init_Dialog ();

	int	key_table[][2] = {
		{ IDC_HELP_KEY_FORWARD,		INPUT_FUNCTION_MOVE_FORWARD },
		{ IDC_HELP_KEY_BACKWARD,	INPUT_FUNCTION_MOVE_BACKWARD },
		{ IDC_HELP_KEY_LEFT,		INPUT_FUNCTION_MOVE_LEFT },
		{ IDC_HELP_KEY_RIGHT,		INPUT_FUNCTION_MOVE_RIGHT },
		{ IDC_HELP_KEY_RETURN,		INPUT_FUNCTION_MENU_TOGGLE },
		{ IDC_HELP_KEY_WALK,		INPUT_FUNCTION_WALK_MODE },
		{ IDC_HELP_KEY_CROUCH,		INPUT_FUNCTION_CROUCH },
		{ IDC_HELP_KEY_JUMP,		INPUT_FUNCTION_JUMP },
		{ IDC_HELP_KEY_OBJECTIVES,	INPUT_FUNCTION_EVA_MISSION_OBJECTIVES_TOGGLE },
		{ IDC_HELP_KEY_NEXT_WEAPON,	INPUT_FUNCTION_NEXT_WEAPON },
		{ IDC_HELP_KEY_ACTION,		INPUT_FUNCTION_ACTION },
		{ IDC_HELP_KEY_CYCLE_POGS,	INPUT_FUNCTION_CYCLE_POG },
		{ IDC_HELP_KEY_WEAPON1,	INPUT_FUNCTION_SELECT_WEAPON_1 },
		{ IDC_HELP_KEY_WEAPON2,	INPUT_FUNCTION_SELECT_WEAPON_2 },
		{ IDC_HELP_KEY_WEAPON3,	INPUT_FUNCTION_SELECT_WEAPON_3 },
		{ IDC_HELP_KEY_WEAPON4,	INPUT_FUNCTION_SELECT_WEAPON_4 },
		{ IDC_HELP_KEY_WEAPON5,	INPUT_FUNCTION_SELECT_WEAPON_5 },
	};

#define	KEY_TABLE_SIZE	(sizeof(key_table)/sizeof(key_table[0]))

	for ( int i = 0; i < KEY_TABLE_SIZE; i++ ) {
		int key = Input::Get_Primary_Key_For_Function( key_table[i][1] );
		WideStringClass wstr(0,true);
		Input::Get_Translated_Key_Name( key, wstr );
		Set_Dlg_Item_Text( key_table[i][0], wstr );
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
HelpScreenDialogClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDOK:
		case IDCANCEL:
			GameInitMgrClass::Continue_Game ();
			break;
	}

	//
	//	Allow the base class to process the message (if necessary)
	//
	MenuDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Display
//
////////////////////////////////////////////////////////////////
void
HelpScreenDialogClass::Display (void)
{
	START_DIALOG (HelpScreenDialogClass);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Menu_Activate
//
////////////////////////////////////////////////////////////////
void
HelpScreenDialogClass::On_Menu_Activate (bool onoff)
{
	if (onoff) {

		//
		//	Replace the backdrop
		//
		MenuBackDropClass *	new_backdrop = new MenuBackDropClass;
		new_backdrop->Set_Model( "IF_HELPLOAD" );
		new_backdrop->Set_Animation( "IF_HELPLOAD.IF_HELPLOAD" );
		OldBackdrop = Replace_BackDrop( new_backdrop );
	} else {

		//
		//	Restore the old backdrop
		//
		if (OldBackdrop != NULL) {
			OldBackdrop = Replace_BackDrop (OldBackdrop);
			if (OldBackdrop != NULL) {
				delete OldBackdrop;
				OldBackdrop = NULL;
			}
		}		
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Destroy
//
////////////////////////////////////////////////////////////////
void
HelpScreenDialogClass::On_Destroy (void)
{
	//
	//	Restore the old backdrop
	//
	if (OldBackdrop != NULL) {
		OldBackdrop = Replace_BackDrop (OldBackdrop);
		if (OldBackdrop != NULL) {
			delete OldBackdrop;
			OldBackdrop = NULL;
		}
	}		

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Key_Down
//
////////////////////////////////////////////////////////////////
bool
HelpScreenDialogClass::On_Key_Down (uint32 key_id, uint32 key_data)
{
	bool retval = false;

	switch (key_id)
	{
		case VK_RETURN:
		case VK_F1:
			GameInitMgrClass::Continue_Game ();
			break;

		default:
			retval = MenuDialogClass::On_Key_Down (key_id, key_data);
			break;
	}

	return retval;
}

