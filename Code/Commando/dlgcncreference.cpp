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
 *                     $Archive:: /Commando/Code/commando/dlgcncreference.cpp                     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/24/02 2:06p                                                 $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgcncreference.h"
#include "renegadedialog.h"
#include "gamedata.h"
#include "cnetwork.h"
#include "gamemode.h"
#include "gametype.h"
#include "dialogcontrol.h"
#include "dlghelpscreen.h"
#include "gameinitmgr.h"
#include "suicideevent.h"
#include "changeteamevent.h"
#include "cstextobj.h"
#include "WWAudio.h"
#include "dialogmgr.h"
#include "menubackdrop.h"
#include "input.h"
#include "string_ids.h"
#include "translatedb.h"
#include "useroptions.h"
#include "gamespyadmin.h"
#include "dialogtests.h"
#include "dialogmgr.h"
#include "specialbuilds.h"


////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
CnCReferenceMenuClass *	CnCReferenceMenuClass::_TheInstance				= NULL;
DWORD							CnCReferenceMenuClass::LastSuicideTimeMs		= 0;
DWORD							CnCReferenceMenuClass::LastChangeTeamTimeMs	= 0;

////////////////////////////////////////////////////////////////
//
//	CnCReferenceMenuClass
//
////////////////////////////////////////////////////////////////
CnCReferenceMenuClass::CnCReferenceMenuClass (void)	:
	OldBackdrop (NULL),
	Timer (0.5F),
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MENU_CNC_REFERENCE))
{
	_TheInstance = this;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	CnCReferenceMenuClass
//
////////////////////////////////////////////////////////////////
CnCReferenceMenuClass::~CnCReferenceMenuClass (void)
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
CnCReferenceMenuClass::On_Init_Dialog (void)
{
	Get_Dlg_Item (IDC_OPTIONS_MULTIPLAY_SUICIDE)->Enable (false);
	Get_Dlg_Item (IDC_OPTIONS_MULTIPLAY_SUICIDE)->Show (false);

	Get_Dlg_Item (IDC_OPTIONS_MULTIPLAY_CHANGE_TEAMS)->Enable (false);
	Get_Dlg_Item (IDC_OPTIONS_MULTIPLAY_CHANGE_TEAMS)->Show (false);

	//
	//	Make the text on the dialog "white"
	//
	Get_Dlg_Item (IDC_STATIC_TEXT_01)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_STATIC_TEXT_02)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_STATIC_TEXT_03)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_STATIC_TEXT_04)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_STATIC_TEXT_05)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_STATIC_TEXT_06)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_STATIC_TEXT_07)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_STATIC_TEXT_08)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_STATIC_TEXT_09)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_STATIC_TEXT_10)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_STATIC_TEXT_11)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_STATIC_TEXT_12)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_STATIC_TEXT_13)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_STATIC_TEXT_14)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_STATIC_TEXT_15)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_STATIC_TEXT_16)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_STATIC_TEXT_17)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_STATIC_TEXT_18)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_STATIC_TEXT_19)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_STATIC_TEXT_20)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));

	Get_Dlg_Item (IDC_KEY_NAME_01)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_KEY_NAME_02)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_KEY_NAME_03)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_KEY_NAME_04)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_KEY_NAME_05)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_KEY_NAME_06)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));

	//
	//	Now, fill in the names of the mapped keys for these functions
	//	
	WideStringClass name;

	Input::Get_Translated_Key_Name (Input::Get_Primary_Key_For_Function (INPUT_FUNCTION_TEAM_INFO_TOGGLE), name);
	Set_Dlg_Item_Text (IDC_KEY_NAME_01, name);

	Input::Get_Translated_Key_Name (Input::Get_Primary_Key_For_Function (INPUT_FUNCTION_BATTLE_INFO_TOGGLE), name);
	Set_Dlg_Item_Text (IDC_KEY_NAME_02, name);

	Input::Get_Translated_Key_Name (Input::Get_Primary_Key_For_Function (INPUT_FUNCTION_ACTION), name);
	Set_Dlg_Item_Text (IDC_KEY_NAME_03, name);

	Input::Get_Translated_Key_Name (Input::Get_Primary_Key_For_Function (INPUT_FUNCTION_EVA_MISSION_OBJECTIVES_TOGGLE), name);
	Set_Dlg_Item_Text (IDC_KEY_NAME_04, name);

	Input::Get_Translated_Key_Name (Input::Get_Primary_Key_For_Function (INPUT_FUNCTION_BEGIN_PUBLIC_MESSAGE), name);
	Set_Dlg_Item_Text (IDC_KEY_NAME_05, name);

	Input::Get_Translated_Key_Name (Input::Get_Primary_Key_For_Function (INPUT_FUNCTION_BEGIN_TEAM_MESSAGE), name);
	Set_Dlg_Item_Text (IDC_KEY_NAME_06, name);

	Timer = 0.5F;
	MenuDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Destroy
//
////////////////////////////////////////////////////////////////
void
CnCReferenceMenuClass::On_Destroy (void)
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

	MenuDialogClass::On_Destroy ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
CnCReferenceMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	bool allow_default_processing = true;

	switch (ctrl_id)
	{
		case IDC_HELP_BUTTON:
			START_DIALOG (HelpScreenDialogClass);
			break;

		case IDC_MENU_MAIN_MENU_BUTTON:
		{
			Prompt_User ();
			allow_default_processing = false;
			break;
		}

		case IDC_OPTIONS_MULTIPLAY_SUICIDE:
		{
			cSuicideEvent * p_suicide = new cSuicideEvent;
			p_suicide->Init();

			//
			// Play a custom SFX for feedback
			//
			WWASSERT(WWAudioClass::Get_Instance() != NULL);
			WWAudioClass::Get_Instance()->Create_Instant_Sound("Committed_Suicide", Matrix3D(1));

			GameInitMgrClass::Continue_Game();
			End_Dialog();

			LastSuicideTimeMs = TIMEGETTIME();

			break;
		}

		case IDC_OPTIONS_MULTIPLAY_CHANGE_TEAMS:
		{
			cChangeTeamEvent * p_event = new cChangeTeamEvent;
			p_event->Init();

			//
			// Play a custom SFX for feedback
			//
			WWASSERT(WWAudioClass::Get_Instance() != NULL);
			WWAudioClass::Get_Instance()->Create_Instant_Sound("Changed_Team", Matrix3D(1));
			
			GameInitMgrClass::Continue_Game();
			End_Dialog();

			LastChangeTeamTimeMs = TIMEGETTIME();

			break;
		}

		case IDCANCEL:
			ctrl_id = IDC_MENU_BACK_BUTTON;
		case IDC_MENU_BACK_BUTTON:
			if (Timer <= 0.0F) {
				GameInitMgrClass::Continue_Game();
			} else {
				allow_default_processing = false;
			}
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
CnCReferenceMenuClass::Display (void)
{
	//
	//	Create the dialog if necessary, otherwise simply bring it to the front
	//
	if (_TheInstance == NULL) {

		//
		//	Create the dialog
		//
		START_DIALOG (CnCReferenceMenuClass);

	} else {
		if (_TheInstance->Is_Active_Menu () == false) {
			DialogMgrClass::Rollback (_TheInstance);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Menu_Activate
//
////////////////////////////////////////////////////////////////
void
CnCReferenceMenuClass::On_Menu_Activate (bool onoff)
{
	if (onoff) {

		//
		//	Replace the backdrop
		//
		MenuBackDropClass *new_backdrop = new MenuBackDropClass;
		new_backdrop->Set_Model ("IF_LVL80LOAD");
		new_backdrop->Set_Animation ("IF_LVL80LOAD.IF_LVL80LOAD");
		OldBackdrop = Replace_BackDrop (new_backdrop);
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
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
CnCReferenceMenuClass::On_Frame_Update (void)
{
	DWORD time_now_ms = TIMEGETTIME();

	//
	// Enable or disable the suicide button
	//
	bool is_suicide_enabled = 
		The_Game() != NULL &&
		!IS_MISSION && 
		cNetwork::I_Am_Client();// &&
		//GameModeManager::Find("Combat") != NULL &&
		//GameModeManager::Find("Combat")->Is_Active();
	
	bool can_suicide_now = (time_now_ms - LastSuicideTimeMs >= ACTION_TIMEOUT_MS);

	Get_Dlg_Item(IDC_OPTIONS_MULTIPLAY_SUICIDE)->Enable (can_suicide_now);
	Get_Dlg_Item(IDC_OPTIONS_MULTIPLAY_SUICIDE)->Show (is_suicide_enabled);

	//
	// Enable or disable the change teams button
	//
	bool is_team_change_enabled = 
		The_Game() != NULL &&
		!IS_MISSION && 
		cNetwork::I_Am_Client() &&
		The_Game()->IsTeamChangingAllowed.Is_True() && 
		(!(GameModeManager::Find("WOL") != NULL && GameModeManager::Find("WOL")->Is_Active()) || The_Game()->IsLaddered.Is_False());// && 
		//GameModeManager::Find("Combat") != NULL &&
		//GameModeManager::Find("Combat")->Is_Active();

	bool can_change_teams_now = (time_now_ms - LastChangeTeamTimeMs >= ACTION_TIMEOUT_MS);

	Get_Dlg_Item (IDC_OPTIONS_MULTIPLAY_CHANGE_TEAMS)->Enable (can_change_teams_now);
	Get_Dlg_Item (IDC_OPTIONS_MULTIPLAY_CHANGE_TEAMS)->Show (is_team_change_enabled);

	//
	//	Decrement the "delay on this screen" timer...
	//
	if (Timer > 0) {
		Timer	-= TimeManager::Get_Frame_Seconds ();
	}

	//
	// Call the base class
	//
	MenuDialogClass::On_Frame_Update();

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Prompt_User
//
////////////////////////////////////////////////////////////////
void
CnCReferenceMenuClass::Prompt_User (void)
{
	//
	//	Display the message box
	//
	if (cUserOptions::SkipIngameQuitConfirmDialog.Is_True()) {
		Exit_Game ();
	} else {
		DlgMsgBox::DoDialog (TRANSLATE (IDS_MENU_TEXT054), TRANSLATE (IDS_EXIT_GAME_VERIFICATION), DlgMsgBox::YesNo, this);	
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	HandleNotification
//
////////////////////////////////////////////////////////////////
void
CnCReferenceMenuClass::HandleNotification (DlgMsgBoxEvent &event)
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
CnCReferenceMenuClass::Exit_Game (void)
{
	//
	//	Close the dialog
	//
	End_Dialog ();

	GameInitMgrClass::End_Game ();
	//GameInitMgrClass::Display_End_Game_Menu ();

	if (cGameSpyAdmin::Get_Is_Launched_From_Gamespy()) {
#ifdef MULTIPLAYERDEMO
			DialogMgrClass::Flush_Dialogs ();
			START_DIALOG (SplashOutroMenuDialogClass);
#else
			extern void Stop_Main_Loop (int);
			Stop_Main_Loop(EXIT_SUCCESS);
#endif // MULTIPLAYERDEMO
	} else {
		GameInitMgrClass::Display_End_Game_Menu ();
	}
	return ;
}


















	/*
	bool is_suicide_enabled = 
		The_Game() != NULL &&
		!IS_MISSION && 
		cNetwork::I_Am_Client() &&
		GameModeManager::Find("Combat") != NULL &&
		GameModeManager::Find("Combat")->Is_Active();

	//Get_Dlg_Item (IDC_OPTIONS_MULTIPLAY_SUICIDE)->Enable (is_suicide_enabled);
	//Get_Dlg_Item (IDC_OPTIONS_MULTIPLAY_SUICIDE)->Show (is_suicide_enabled);

	//
	// Enable or disable the change teams button
	//
	bool is_team_change_enabled = 
		The_Game() != NULL &&
		!IS_MISSION && 
		cNetwork::I_Am_Client() &&
		The_Game()->IsTeamChangingAllowed.Is_True() && 
		(!(GameModeManager::Find("WOL") != NULL && GameModeManager::Find("WOL")->Is_Active()) || The_Game()->IsLaddered.Is_False()) && 
		GameModeManager::Find("Combat") != NULL &&
		GameModeManager::Find("Combat")->Is_Active();

	Get_Dlg_Item (IDC_OPTIONS_MULTIPLAY_CHANGE_TEAMS)->Enable (is_team_change_enabled);
	Get_Dlg_Item (IDC_OPTIONS_MULTIPLAY_CHANGE_TEAMS)->Show (is_team_change_enabled);
	*/

			/*
			if (cGameSpyAdmin::Get_Is_Launched_From_Gamespy()) {
				extern void Stop_Main_Loop (int);
				Stop_Main_Loop(EXIT_SUCCESS);
			}
			/**/

