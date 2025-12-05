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
 *                     $Archive:: /Commando/Code/commando/dlgevaencyclopedia.cpp      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/02/01 10:45a                                             $*
 *                                                                                             *
 *                    $Revision:: 26                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgevaencyclopedia.h"
#include "renegadedialog.h"
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
#include "gameinitmgr.h"
#include "gametype.h"
#include "suicideevent.h"
#include "changeteamevent.h"
#include "cstextobj.h"
#include "WWAudio.h"
#include "dlghelpscreen.h"
#include "crandom.h"
#include "slavemaster.h"
#include "string_ids.h"
#include "translatedb.h"


////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
EVAEncyclopediaMenuClass *	EVAEncyclopediaMenuClass::_TheInstance = NULL;
int								EVAEncyclopediaMenuClass::_NextTabIndex = -4;


////////////////////////////////////////////////////////////////
//
//	EVAEncyclopediaMenuClass
//
////////////////////////////////////////////////////////////////
EVAEncyclopediaMenuClass::EVAEncyclopediaMenuClass (void)	:
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MENU_EVA_ENCYCLOPEDIA))
{
	_TheInstance = this;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	EVAEncyclopediaMenuClass
//
////////////////////////////////////////////////////////////////
EVAEncyclopediaMenuClass::~EVAEncyclopediaMenuClass (void)
{
	_TheInstance = NULL;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
EVAEncyclopediaMenuClass::On_Init_Dialog (void)
{
	TabCtrlClass *tab_ctrl = (TabCtrlClass *)Get_Dlg_Item (IDC_GENERIC_TABCTRL);
	if (tab_ctrl != NULL) {

		//
		//	Add the tabs to the control
		//
		TABCTRL_ADD_TAB (tab_ctrl, EvaObjectivesTabClass);
		TABCTRL_ADD_TAB (tab_ctrl, EvaMapTabClass);
		TABCTRL_ADD_TAB (tab_ctrl, EvaDataTabClass);
		TABCTRL_ADD_TAB (tab_ctrl, EvaCharactersTabClass);
		TABCTRL_ADD_TAB (tab_ctrl, EvaWeaponsTabClass);
		TABCTRL_ADD_TAB (tab_ctrl, EvaVehiclesTabClass);
		TABCTRL_ADD_TAB (tab_ctrl, EvaBuildingsTabClass);

		//
		//	Determine which tab to show be default
		//
		int tab_index = _NextTabIndex;
		if (_NextTabIndex < 0) {
			tab_index = 0;
		}

		tab_ctrl->Set_Curr_Tab (tab_index);
	}

	//
	//	Remove the save/load options for multiplay
	//

	if (!IS_MISSION) {
		Get_Dlg_Item (IDC_MENU_LOAD_SP_GAME_BUTTON)->Show (false);
		Get_Dlg_Item (IDC_MENU_LOAD_SP_GAME_BUTTON)->Enable (false);
		Get_Dlg_Item (IDC_MENU_SAVE_SP_GAME_BUTTON)->Show (false);
		Get_Dlg_Item (IDC_MENU_SAVE_SP_GAME_BUTTON)->Enable (false);
	}

	//
	// Enable or disable the suicide button
	//
	bool is_suicide_enabled =
		The_Game() != NULL &&
		!IS_MISSION &&
		cNetwork::I_Am_Client() &&
		GameModeManager::Find("Combat") != NULL &&
		GameModeManager::Find("Combat")->Is_Active();

	Get_Dlg_Item(IDC_OPTIONS_MULTIPLAY_SUICIDE)->Enable(is_suicide_enabled);
	Get_Dlg_Item(IDC_OPTIONS_MULTIPLAY_SUICIDE)->Show(is_suicide_enabled);


	//
	// Enable or disable the change teams button
	//
	bool is_team_change_enabled =
		The_Game() != NULL &&
		!IS_MISSION &&
		cNetwork::I_Am_Client() &&
		//The_Game()->Is_Team_Game() &&
		The_Game()->IsTeamChangingAllowed.Is_True() &&
		(!(GameModeManager::Find("WOL") != NULL && GameModeManager::Find("WOL")->Is_Active()) || The_Game()->IsLaddered.Is_False()) &&
		GameModeManager::Find("Combat") != NULL &&
		GameModeManager::Find("Combat")->Is_Active();

	Get_Dlg_Item(IDC_OPTIONS_MULTIPLAY_CHANGE_TEAMS)->Enable(is_team_change_enabled);
	Get_Dlg_Item(IDC_OPTIONS_MULTIPLAY_CHANGE_TEAMS)->Show(is_team_change_enabled);


	MenuDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Destroy
//
////////////////////////////////////////////////////////////////
void
EVAEncyclopediaMenuClass::On_Destroy (void)
{
	TabCtrlClass *tab_ctrl = (TabCtrlClass *)Get_Dlg_Item (IDC_GENERIC_TABCTRL);
	if (tab_ctrl == NULL) {
		return ;
	}

	//
	//	Remember what tab the user left on...
	//
	if (_NextTabIndex >= 0) {
		_NextTabIndex = tab_ctrl->Get_Curr_Tab ();
	}

	MenuDialogClass::On_Destroy ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
EVAEncyclopediaMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	bool allow_default_processing = true;

	switch (ctrl_id)
	{
		case IDC_HELP_BUTTON:
			START_DIALOG (HelpScreenDialogClass);
			break;

		case IDC_MENU_MAIN_MENU_BUTTON:
			Prompt_User ();
			allow_default_processing = false;
			break;

		case IDCANCEL:
			ctrl_id = IDC_MENU_BACK_BUTTON;
		case IDC_MENU_BACK_BUTTON:
			GameInitMgrClass::Continue_Game();
			break;
	}

	//
	//	Allow the base class to process the message (if necessary)
	//
	if (allow_default_processing) {
		MenuDialogClass::On_Command (ctrl_id, message_id, param);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Display
//
////////////////////////////////////////////////////////////////
void
EVAEncyclopediaMenuClass::Display (TAB_ID tab_id)
{
	//
	//	Create the dialog if necessary, otherwise simply bring it to the front
	//
	if (_TheInstance == NULL) {
		if (tab_id != TAB_NONE) {
			_NextTabIndex = tab_id;
		}
		START_DIALOG (EVAEncyclopediaMenuClass);
	} else {
		if (_TheInstance->Is_Active_Menu () == false) {
			DialogMgrClass::Rollback (_TheInstance);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Prompt_User
//
////////////////////////////////////////////////////////////////
void
EVAEncyclopediaMenuClass::Prompt_User (void)
{
	//
	//	Display the message box
	//
	DlgMsgBox::DoDialog (TRANSLATE (IDS_MENU_TEXT054), TRANSLATE (IDS_EXIT_GAME_VERIFICATION), DlgMsgBox::YesNo, this);	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	HandleNotification
//
////////////////////////////////////////////////////////////////
void
EVAEncyclopediaMenuClass::HandleNotification (DlgMsgBoxEvent &event)
{
	if (event.Event () == DlgMsgBoxEvent::Yes) {
		Exit_Game ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Exit_Game
//
////////////////////////////////////////////////////////////////
void
EVAEncyclopediaMenuClass::Exit_Game (void)
{
	bool stop = false;
	if (GameModeManager::Find("WOL")->Is_Active() || GameModeManager::Find("LAN")->Is_Active()) {
		if (cNetwork::I_Am_Server() && The_Game() && The_Game()->IsDedicated.Is_True() && SlaveMaster.Am_I_Slave()) {
			stop = true;
		}
	}

	//
	//	Close the dialog
	//
	End_Dialog ();

	GameInitMgrClass::End_Game();

	//
	// Dedicated slave servers should just quit here.
	//
	if (stop) {
		extern void Stop_Main_Loop (int exitCode);
		Stop_Main_Loop (EXIT_SUCCESS);
	}
	GameInitMgrClass::Display_End_Game_Menu();
	return ;
}
