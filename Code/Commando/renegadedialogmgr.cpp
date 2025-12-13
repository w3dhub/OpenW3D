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
 *                     $Archive:: /Commando/Code/commando/renegadedialogmgr.cpp      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/19/02 2:00p                                               $*
 *                                                                                             *
 *                    $Revision:: 51                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "renegadedialogmgr.h"
#include "dialogmgr.h"
#include "dialogbase.h"
#include "dialogtests.h"
#include "renegadedialog.h"
#include "dlgloadspgame.h"
#include "dlgevaencyclopedia.h"
#include "dlgmultiplayoptions.h"
#include "dlgcharacteroptions.h"
#include "dlgcheatoptions.h"
#include "dlgtechoptions.h"
#include "dlgmovieoptions.h"
#include "dlgpreviewoptions.h"
#include "dlgcredits.h"
#include "inputctrl.h"
#include "translatedb.h"
#include "string_ids.h"
#include "dlgmplanhostoptions.h"
#include "dlginternetoptions.h"
#include "gamemode.h"
#include "dlgmplangamelist.h"
#include "directinput.h"
#include "input.h"
#include "dlgmainmenu.h"
#include "DlgWebPage.h"
#include "dlgmpwolpagebuddy.h"
#include "dlgmpwolbuddies.h"
#include "dlgmpwolchat.h"
#include "dlgmpwolgamelist.h"
#include "dlgsavegame.h"
#include "dlgcontrols.h"
#include "dlgmpwolquickmatchoptions.h"
#include "dlgmpwolmain.h"
#include "DlgWOLSettings.h"
#include "dlghelpscreen.h"
#include "dlgcncreference.h"
#include "wwmemlog.h"
#include "ConsoleMode.h"
#include "specialbuilds.h"
#include <algorithm>


////////////////////////////////////////////////////////////////
//	Globals
////////////////////////////////////////////////////////////////
WWUIInputClass *	_TheWWUIInput = NULL;


////////////////////////////////////////////////////////////////
//	Static data
////////////////////////////////////////////////////////////////
DialogFactoryBaseClass *FactoryArray[FACTORY_COUNT] =
{
	new DialogFactoryClass<StartSPGameDialogClass>,
	NULL,
	NULL,
	new DialogFactoryClass<OptionsMenuClass>,
	new DialogFactoryClass<DifficultyMenuClass>,
	NULL,	//IDC_MENU_START_TUTORIAL_BUTTON
	new DialogFactoryClass<LoadSPGameMenuClass>,
	NULL,	//IDC_MENU_DIFFCULTY01_BUTTON
	NULL,	//IDC_MENU_DIFFCULTY02_BUTTON
	NULL,	//IDC_MENU_DIFFCULTY04_BUTTON
	NULL,	//IDC_MENU_DIFFCULTY04_BUTTON
	new DialogFactoryClass<ControlsMenuClass>,
	new DialogFactoryClass<CharacterOptionsMenuClass>,
	new DialogFactoryClass<CheatOptionsMenuClass>,
	new DialogFactoryClass<TechOptionsMenuClass>,
	new DialogFactoryClass<MovieOptionsMenuClass>,
	new DialogFactoryClass<PreviewOptionsMenuClass>,
	new DialogFactoryClass<CreditsMenuClass>,
	new DialogFactoryClass<QuitVerificationDialogClass>,
	new DialogFactoryClass<EVAEncyclopediaMenuClass>,
	new DialogFactoryClass<MainMenuDialogClass>,
	new DialogFactoryClass<SaveGameMenuClass>,
	new DialogFactoryClass<MPLanMenuClass>,
	//new DialogFactoryClass<MPInternetMenuClass>,
	NULL,
	NULL,//new DialogFactoryClass<MPGameMenuClass>,
	new DialogFactoryClass<MPJoinMenuClass>,
	NULL,//new DialogFactoryClass<MPServerStartMenuClass>,
	new DialogFactoryClass<MultiplayOptionsMenuClass>,
	NULL,
	new DialogFactoryClass<MPWolMainMenuClass>,
	new DialogFactoryClass<MPLanGameListMenuClass>,
	NULL,	//IDC_MENU_MP_LAN_JOIN_BUTTON
	NULL,	//IDC_MENU_MP_LAN_START_BUTTON
};


////////////////////////////////////////////////////////////////
//	Local Prototypes
////////////////////////////////////////////////////////////////
bool CALLBACK Default_On_Command (DialogBaseClass *dialog, int ctrl_id, int mesage_id, unsigned int param);
void	Stop_Main_Loop (int);

////////////////////////////////////////////////////////////////
//	RenegadeUIInputClass
////////////////////////////////////////////////////////////////
class RenegadeUIInputClass : public WWUIInputClass
{
	const Vector3 &
	Get_Mouse_Pos (void) const override
	{
		DirectInput::Get_Cursor_Pos (&MousePos);
		return MousePos;
	}

	void
	Set_Mouse_Pos (const Vector3 &pos) override
	{
		MousePos = pos;
		DirectInput::Reset_Cursor_Pos (Vector2 (pos.X, pos.Y));
		return ;
	}

	bool
	Is_Button_Down (int vk_mouse_button_id) override
	{
		bool retval = false;
		switch (vk_mouse_button_id)
		{
			case VK_LBUTTON:
				retval = Input::Is_Button_Down (DirectInput::BUTTON_MOUSE_LEFT);
				break;

			case VK_MBUTTON:
				retval = Input::Is_Button_Down (DirectInput::BUTTON_MOUSE_CENTER);
				break;

			case VK_RBUTTON:
				retval = Input::Is_Button_Down (DirectInput::BUTTON_MOUSE_RIGHT);
				break;
		}

		return retval;
	}

	void
	Enter_Menu_Mode (void) override
	{
		Input::Menu_Enable (true);
		return ;
	}

	void
	Exit_Menu_Mode (void) override
	{
		Input::Menu_Enable (false);
		DirectInput::Eat_Mouse_Held_States ();
		return ;
	}

private:
	mutable Vector3 MousePos;
};


////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////
void
RenegadeDialogMgrClass::Initialize (void)
{
	WWMEMLOG(MEM_GAMEDATA);
	const char *	STYLE_MGR_INI	= "stylemgr.ini";

	_TheWWUIInput = new RenegadeUIInputClass;
	_TheWWUIInput->InitIME(MainWindow);

	//
	//	Simple-pass thru to the WWUI dialog mgr system
	//
	if (!ConsoleBox.Is_Exclusive()) {
		DialogBaseClass::Set_Default_Command_Handler (Default_On_Command);
		DialogMgrClass::Initialize (STYLE_MGR_INI);
	}
	DialogMgrClass::Install_Input (_TheWWUIInput);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Do_Simple_Dialog
//
////////////////////////////////////////////////////////////////
void
RenegadeDialogMgrClass::Do_Simple_Dialog (const DialogResource *dialog_resource)
{
	//
	//	Show the dialog
	//
	PopupDialogClass *dialog = new PopupDialogClass (dialog_resource);
	dialog->Start_Dialog ();
	REF_PTR_RELEASE (dialog);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Goto_Location
//
////////////////////////////////////////////////////////////////
void
RenegadeDialogMgrClass::Goto_Location (LOCATION location)
{
	WWMEMLOG(MEM_GAMEDATA);

	switch (location)
	{
		case LOC_MAIN_MENU:
			MainMenuDialogClass::Display ();
			break;

		case LOC_SPLASH_IN:
			START_DIALOG (SplashIntroMenuDialogClass);
			break;

		case LOC_SPLASH_OUT:
			START_DIALOG (SplashOutroMenuDialogClass);
			break;

		case LOC_INTERNET_MAIN:
			MPWolMainMenuClass::Display ();
			break;

		case LOC_INTERNET_GAME_LIST:
			MPWolGameListMenuClass::DoDialog ();
			break;

		case LOC_LAN_MAIN:
			MPLanGameListMenuClass::Display ();
			break;

		case LOC_ENCYCLOPEDIA:
			EVAEncyclopediaMenuClass::Display ();
			break;

		case LOC_OBJECTIVES:
			EVAEncyclopediaMenuClass::Display (EVAEncyclopediaMenuClass::TAB_OBJECTIVES);
			break;

		case LOC_MAP:
			EVAEncyclopediaMenuClass::Display (EVAEncyclopediaMenuClass::TAB_MAP);
			break;

		case LOC_CNC_REFERENCE:
			CnCReferenceMenuClass::Display ();
			break;

		case LOC_LOAD_GAME:
			LoadSPGameMenuClass::Display ();
			break;

		case LOC_IN_GAME_HELP:
			HelpScreenDialogClass::Display ();
			break;

		//GAMESPY
		case LOC_GAMESPY_MAIN:
			START_DIALOG (GameSpyMainDialogClass);
			break;
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
//	Shutdown
//
////////////////////////////////////////////////////////////////
void
RenegadeDialogMgrClass::Shutdown (void)
{
	//
	//	Simple-pass thru to the WWUI dialog mgr system
	//
	DialogMgrClass::Shutdown ();

	//
	//	Release our hold on the input mechanism
	//
	REF_PTR_RELEASE (_TheWWUIInput);

	//
	// Free the factories so we don't get memory leaks when we quit. ST - 6/11/2001 8:23PM
	//
	for (int i=0 ; i<FACTORY_COUNT ; i++) {
		if (FactoryArray[i]) {
			delete FactoryArray[i];
			FactoryArray[i] = NULL;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Default_On_Command
//
////////////////////////////////////////////////////////////////
bool CALLBACK
Default_On_Command (DialogBaseClass *dialog, int ctrl_id, int mesage_id, unsigned int param)
{
	bool handled = true;

	//
	//	Is this one of our "auto-link" buttons?
	//
	if (ctrl_id >= DIALOG_LINK_FIRST && ctrl_id < DIALOG_LINK_LAST) {
		if (FactoryArray[ctrl_id - DIALOG_LINK_FIRST] != NULL) {

			//
			//	Do the dialog associated with this command
			//
			FactoryArray[ctrl_id - DIALOG_LINK_FIRST]->Do_Dialog ();
		}
	}

	switch (ctrl_id)
	{
		case IDC_MENU_BACK_BUTTON:
		case IDC_BACK:
		case IDCANCEL:
		{
			dialog->End_Dialog ();
			break ;
		}

		case IDC_MP_SHORTCUT_NEWS:
			DlgWebPage::DoDialog("News");
			break;

		case IDC_MP_SHORTCUT_CLANS:
			DlgWebPage::DoDialog("BattleClans");
			break;

		case IDC_MP_SHORTCUT_RANKINGS:
			DlgWebPage::DoDialog("Ladder");
			break;

		case IDC_MP_SHORTCUT_ACCOUNT:
			DlgWOLSettings::DoDialog();
			break;

		#ifdef QUICKMATCH_OPTIONS
		case IDC_MP_SHORTCUT_QUICKMATCH_OPTIONS:
			START_DIALOG(MPWolQuickMatchOptionsMenuClass);
			break;
		#endif // QUICKMATCH_OPTIONS

		case IDC_MP_SHORTCUT_NET_STATUS:
			DlgWebPage::DoDialog("NetStatus");
			break;

		case IDC_MP_SHORTCUT_BUDDIES:
			MPWolBuddiesMenuClass::Display();
			break;

		case IDC_MP_SHORTCUT_PAGE_BUDDY:
			START_DIALOG (MPWolPageBuddyPopupClass);
			break;

		case IDC_MP_SHORTCUT_CHAT:
			MPWolChatMenuClass::DoDialog();
			break;

		case IDC_MP_SHORTCUT_GAMELIST:
			MPWolGameListMenuClass::DoDialog();
			break;

		case IDC_MP_SHORTCUT_INTERNET_OPTIONS:
			START_DIALOG (MultiplayOptionsMenuClass);
			break;

		case IDC_QUIT:
#ifdef MULTIPLAYERDEMO
				DialogMgrClass::Flush_Dialogs ();

				START_DIALOG (SplashOutroMenuDialogClass);
				/*dialog->End_Dialog ();
				{
					MainMenuDialogClass *main_menu = MainMenuDialogClass::Get_Instance ();
					if (main_menu != NULL) {
						main_menu->End_Dialog ();
					}
				}*/
#else
				Stop_Main_Loop (EXIT_SUCCESS);
#endif //MULTIPLAYERDEMO
			break ;

		default:
			handled = false;
			break;
	}

	return handled;
}


//
// Called as follows: If IDS_TEST is the string you wish to load and
// wchar_t Buffer[128] is your buffer, the call would be
// MyLoadStringW(IDS_TEST,Buffer,128);
// If it succeeds, the function returns the number of characters copied
// into the buffer, not including the NULL terminating character, or
// zero if the string resource does not exist.
//
int MyLoadStringW (UINT str_id, LPWSTR buffer, int buffer_len)
{
	//
	//	Compute the block and offset
	//
	int block	= (str_id >> 4) + 1;
	int num		= str_id & 0xF;

	//
	//	Find the resource
	//
	HRSRC resource = ::FindResourceEx (NULL, RT_STRING, MAKEINTRESOURCE (block),
								MAKELANGID (LANG_NEUTRAL, SUBLANG_NEUTRAL));

	//
	//	Load the resource into memory
	//
	HGLOBAL hglobal	= ::LoadResource (NULL, resource);
	LPWSTR res_string	= (LPWSTR)::LockResource (hglobal);

	int length = 0;
	if (res_string != NULL) {

		for (int index = 0; index < num; index ++) {
			res_string += *res_string + 1;
		}

		//
		//	Copy the string to our buffer
		//
		length = std::min ((int)(buffer_len - 1), (int)(*res_string));
		::wcsncpy (buffer, res_string + 1, length);
	}

	//
	//	Null terminate the buffer
	//
	buffer[length] = '\0';

	//
	//	Return the number of bytes copied
	//
	return length;
}
