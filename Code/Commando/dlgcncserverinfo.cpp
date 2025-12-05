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
 *                 Project Name : commando                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/commando/dlgcncserverinfo.cpp                                     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/15/02 8:19p                                                  $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgcncserverinfo.h"
#include "renegadedialog.h"
#include "gamedata.h"
#include "listctrl.h"
#include "imagectrl.h"
#include "playertype.h"
#include "combat.h"
#include "playermanager.h"
#include "player.h"
#include "soldier.h"
#include "gameinitmgr.h"
#include "gamemode.h"
#include "input.h"
#include "healthbarctrl.h"
#include "basecontroller.h"
#include "building.h"
#include "damage.h"
#include "vehicle.h"
#include "wolgmode.h"
#include <WWOnline/WOLUser.h>
#include "translatedb.h"
#include "string_ids.h"
#include "mousemgr.h"
#include "directinput.h"


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
enum
{
	COL_OPTION,
	COL_VALUE,
};


////////////////////////////////////////////////////////////////
//
//	CNCServerInfoDialogClass
//
////////////////////////////////////////////////////////////////
CNCServerInfoDialogClass::CNCServerInfoDialogClass (void)	:
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_CNC_SERVER_INFO))
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~CNCServerInfoDialogClass
//
////////////////////////////////////////////////////////////////
CNCServerInfoDialogClass::~CNCServerInfoDialogClass (void)
{
	GameInitMgrClass::Continue_Game ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
CNCServerInfoDialogClass::On_Init_Dialog (void)
{
	MenuDialogClass::On_Init_Dialog ();
	MouseMgrClass::Show_Cursor (false);

	//
	//	Get a pointer to the list control
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LIST_CTRL);
	if (list_ctrl != NULL) {

		//
		//	Configure the columns
		//
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_SERVER_OPTION),		0.5F,	Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_VALUE),					0.5F,	Vector3 (1, 1, 1));

		//
		//	Get the current description of the game
		//
		WideStringClass description;
		The_Game ()->Get_Description (description);
		const wchar_t *buffer = description.Peek_Buffer ();

		const wchar_t DELIMITER	= L'\t';
		const wchar_t NEWLINE		= L'\n';

		//
		//	Fill the settings into the list control
		//
		WideStringClass string (100, true);
		int list_index		= 0;
		int item_index		= 0;
		int index			= 0;
		while (buffer[index] != 0) {
			
			if (buffer[index] == DELIMITER) {
				
				//
				//	Insert a new entry
				//
				item_index = list_ctrl->Insert_Entry (list_index ++, string);
				string.Get_Buffer (100)[0] = 0;
			} else if (buffer[index] == NEWLINE) {

				//
				//	Fill in the setting value
				//
				item_index = list_ctrl->Set_Entry_Text (item_index, COL_VALUE, string);
				string.Get_Buffer (100)[0] = 0;
			} else {

				//
				//	Simply add this character to the string
				//
				string += buffer[index];
			}

			index ++;
		}
	}

	//
	//	Activate the menu game mode (if necessary)
	//
	GameModeClass *menu_game_mode = GameModeManager::Find ("Menu");
	if (menu_game_mode != NULL && menu_game_mode->Is_Active () == false) {
		menu_game_mode->Activate ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
CNCServerInfoDialogClass::On_Frame_Update (void)
{
	//
	//	End the dialog when the user releases the player list key
	//
	int dik_id = Input::Get_Primary_Key_For_Function (INPUT_FUNCTION_SERVER_INFO_TOGGLE);
	if ((DirectInput::Get_Keyboard_Button (dik_id) & DirectInput::DI_BUTTON_HELD) == 0) {
		End_Dialog ();
	}

	return ;
}


