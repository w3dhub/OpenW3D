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
 *                     $Archive:: /Commando/Code/Commando/dialogtests.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/18/02 2:52p                                               $*
 *                                                                                             *
 *                    $Revision:: 96                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dialogtests.h"
#include "renegadedialog.h"
#include "dialogcontrol.h"
#include "tabctrl.h"
#include "combatchunkid.h"
#include "basecontroller.h"
#include "building.h"
#include "combat.h"
#include "soldier.h"
#include "playertype.h"
#include "definition.h"
#include "definitionmgr.h"
#include "vendor.h"
#include "assetmgr.h"
#include "rendobj.h"
#include "scene.h"
#include "menubackdrop.h"
#include "hanim.h"
#include "gamedata.h"
#include "gametype.h"
#include "checkboxctrl.h"
#include "editctrl.h"
#include "comboboxctrl.h"
#include "gdcnc.h"
#include "useroptions.h"
#include "cnetwork.h"
#include "listctrl.h"
#include "gamechannel.h"
#include "gamechanlist.h"
#include "gamemode.h"
#include "wolgmode.h"
#include "langmode.h"
#include "dlgcontrolslisttab.h"
#include "campaign.h"
#include "gameinitmgr.h"
#include "dialogmgr.h"
#include "god.h"
#include "shortcutbarctrl.h"
#include "treectrl.h"
#include "wheelvehicle.h"
#include "trackedvehicle.h"
#include "useroptions.h"
#include "DlgMPConnect.h"
#include "translatedb.h"
#include "string_ids.h"
#include "dlgcharacteroptions.h"
#include "dlgmultiplayoptions.h"
#include "WWAudio.h"
#include "dlgmplanhostoptions.h"
#include "shellapi.h"
#include "gamespyadmin.h"
#include "dlgmpwolmain.h"
#include "specialbuilds.h"
#include "GameSpy_QnR.h"
#include <wwui/imagectrl.h>
#include "bandwidthcheck.h"


////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
QuitVerificationDialogClass *	QuitVerificationDialogClass::_TheInstance	= NULL;
bool	SplashIntroMenuDialogClass::IsComplete = false;

////////////////////////////////////////////////////////////////
//
//	SplashIntroMenuDialogClass
//
////////////////////////////////////////////////////////////////
SplashIntroMenuDialogClass::SplashIntroMenuDialogClass (void)	:
//	Timer (4.0F),
	Timer (0.0F),
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MENU_SPLASH1))
{	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~SplashIntroMenuDialogClass
//
////////////////////////////////////////////////////////////////
SplashIntroMenuDialogClass::~SplashIntroMenuDialogClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
SplashIntroMenuDialogClass::On_Init_Dialog (void)
{
	MenuDialogClass::On_Init_Dialog ();
	int count = cUserOptions::SplashCount.Get();
	if (count == 0) {
		Timer = 4.0F;
	} else {
		Timer = 0.0001F;
	}
	cUserOptions::SplashCount.Set(count+1);

	// 
	// Configure the image
	//
	if (Get_Dlg_Item (IDC_IMAGE_CTRL) != NULL) {
		((ImageCtrlClass*)Get_Dlg_Item (IDC_IMAGE_CTRL))->Set_Texture ("multisplash01.tga");
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
SplashIntroMenuDialogClass::On_Frame_Update (void)
{
	MenuDialogClass::On_Frame_Update ();

	//
	//	Check to see if time has elapsed
	//
	if (Timer > 0) {
		Timer -= TimeManager::Get_Frame_Seconds ();
		if (Timer <= 0) {
			
			//
			//	Time has elapsed, so move onto the main menu dialog
			//
			RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_MAIN_MENU);
			End_Dialog ();
			IsComplete = true;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
SplashIntroMenuDialogClass::On_Command (int ctrl_id, int mesage_id, unsigned int param)
{
	return ;
}



////////////////////////////////////////////////////////////////
//
//	SplashOutroMenuDialogClass
//
////////////////////////////////////////////////////////////////
SplashOutroMenuDialogClass::SplashOutroMenuDialogClass (void)	:
	Timer (5.0F),
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MENU_SPLASH2))
{	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~SplashOutroMenuDialogClass
//
////////////////////////////////////////////////////////////////
SplashOutroMenuDialogClass::~SplashOutroMenuDialogClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
SplashOutroMenuDialogClass::On_Init_Dialog (void)
{
	if (MenuBackDropClass *backdrop = MenuDialogClass::Get_BackDrop ()) {
		backdrop->Remove_Model ();
	}

	MenuDialogClass::On_Init_Dialog ();

	// 
	// Configure the image
	//
	if (Get_Dlg_Item (IDC_IMAGE_CTRL) != NULL) {
		((ImageCtrlClass*)Get_Dlg_Item (IDC_IMAGE_CTRL))->Set_Texture ("multisplash02.tga");
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
SplashOutroMenuDialogClass::On_Frame_Update (void)
{
	MenuDialogClass::On_Frame_Update ();

	//
	//	Check to see if time has elapsed
	//
	/*if (Timer > 0) {
		Timer -= TimeManager::Get_Frame_Seconds ();
		if (Timer <= 0) {
			
			//
			//	Time has elapsed, so exit the game...
			//
			extern void Stop_Main_Loop (int);
			Stop_Main_Loop (EXIT_SUCCESS);
		}
	}*/

	return ;
}

extern void Stop_Main_Loop (int);

////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
SplashOutroMenuDialogClass::On_Command (int ctrl_id, int mesage_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDC_BUY_RENEGADE:
			::ShellExecuteA (NULL, "open", "http://www.ea.com/storeEntry.jsp?sDestinationURL=DETAIL&sSku=14180W&sEntryPoint=EXPSITE", NULL, NULL, SW_SHOW);
			Stop_Main_Loop (EXIT_SUCCESS);
			break;

		case IDC_VISIT_WEB_BUTTON:
			::ShellExecuteA (NULL, "open", "http://www.westwood.com/games/ccuniverse/renegade", NULL, NULL, SW_SHOW);
			Stop_Main_Loop (EXIT_SUCCESS);
			break;

		case IDC_EXIT_BUTTON:
			Stop_Main_Loop (EXIT_SUCCESS);
			break;
	}

	return ;
}




////////////////////////////////////////////////////////////////
//
//	QuitVerificationDialogClass
//
////////////////////////////////////////////////////////////////
QuitVerificationDialogClass::QuitVerificationDialogClass (void)	:
	PopupDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_QUIT_TO_DESKTOP))
{
	_TheInstance = this;

	//
	// TSS092501
	//
	if (cUserOptions::SkipQuitConfirmDialog.Is_True()) 
	{
		extern void Stop_Main_Loop (int);
		Stop_Main_Loop(EXIT_SUCCESS);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	QuitVerificationDialogClass
//
////////////////////////////////////////////////////////////////
QuitVerificationDialogClass::~QuitVerificationDialogClass (void)
{
	_TheInstance = NULL;
	return ;
}

//GAMESPY
////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
GameSpyMainDialogClass *	GameSpyMainDialogClass::_TheInstance	= NULL;
bool GameSpyMainDialogClass::DetectingBandwidth = false;


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
GameSpyMainDialogClass::On_Frame_Update (void)
{
	Enable_Dlg_Item(IDC_MENU_GAMESPY_HOST, !BandwidthCheckerClass::Is_Thread_Running());	

	MenuDialogClass::On_Frame_Update ();
	return ;
}

////////////////////////////////////////////////////////////////
//
//	GameSpyMainDialogClass
//
////////////////////////////////////////////////////////////////
GameSpyMainDialogClass::GameSpyMainDialogClass (void)	:
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MENU_GAMESPY_MAIN))
{
	_TheInstance = this;
	cGameSpyAdmin::Reset();
	cGameSpyAdmin::Set_Is_Under_Gamespy_Menuing(true);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~GameSpyMainDialogClass
//
////////////////////////////////////////////////////////////////
GameSpyMainDialogClass::~GameSpyMainDialogClass (void)
{
	_TheInstance = NULL;
	cGameSpyAdmin::Set_Is_Under_Gamespy_Menuing(false);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
GameSpyMainDialogClass::On_Init_Dialog (void)
{
	MenuDialogClass::On_Init_Dialog ();

	// 
	// Init the gamespy logo
	//
	if (Get_Dlg_Item(IDC_GAMESPY_ICON) != NULL) {
		((ImageCtrlClass*)Get_Dlg_Item(IDC_GAMESPY_ICON))->Set_Texture("GAMESPYLOGO.TGA");
	}
}


////////////////////////////////////////////////////////////////
//
//	On_Last_Menu_Ending
//
////////////////////////////////////////////////////////////////
void
GameSpyMainDialogClass::On_Last_Menu_Ending (void)
{
	RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_MAIN_MENU);
	return ;
}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::HandleNotification(DlgWOLWaitEvent)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void GameSpyMainDialogClass::HandleNotification(DlgWOLWaitEvent& event)
{
	if (WaitCondition::ConditionMet == event.Result())
		{
		if (DetectingBandwidth) {
			DetectingBandwidth = false;
			cUserOptions::Set_Bandwidth_Type(BANDWIDTH_AUTO);

			Release_Ref();
			Host_Game();
		}
	} else if (WaitCondition::Waiting != event.Result()) {
		DetectingBandwidth = false;
		Release_Ref();
	}
}

////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
GameSpyMainDialogClass::On_Command (int ctrl_id, int mesage_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDC_MENU_GAMESPY_JOIN:
		{
			GameSpyQnR.LaunchArcade();
			extern void Stop_Main_Loop (int);
			Stop_Main_Loop(EXIT_SUCCESS);
			break;
		}

		case IDC_MENU_GAMESPY_HOST:
			{
				if (!DetectingBandwidth) {
					RefPtr<SerialWait> serverWait = SerialWait::Create();
					WWASSERT(serverWait.IsValid());

					DetectingBandwidth = true;
					Add_Ref();
					RefPtr<WaitCondition> bandwidth_wait = BandwidthCheckerClass::Detect();
					DlgWOLWait::DoDialog(TRANSLATE (IDS_MENU_DETECTING_BANDWIDTH), bandwidth_wait, this);
				}
			}
			break;

		case IDC_MENU_GAMESPY_OPTIONS:
			START_DIALOG (GameSpyOptionsDialogClass);
			break;
	}

	MenuDialogClass::On_Command (ctrl_id, mesage_id, param);
	return ;
}


////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
void GameSpyMainDialogClass::Host_Game(void) {

	GameInitMgrClass::Initialize_LAN();
	
	/*
	//
	//	Set the new nickname
	//
	WideStringClass nickname = Get_Dlg_Item_Text (IDC_NICKNAME_EDIT);
	cNetInterface::Set_Nickname (nickname);
	*/

	//
	//  Enable GameSpy Code
	//
	cGameSpyAdmin::Set_Is_Server_Gamespy_Listed(true);
	GameSpyQnR.Enable_Reporting(true);
	
	//
	//	Create the new game data
	//
	PTheGameData = cGameData::Create_Game_Of_Type (cGameData::GAME_TYPE_CNC);
	WWASSERT(PTheGameData != NULL);

	The_Game()->Load_From_Server_Config ();
	
	// 
	// Gamespy games are not quickmatchable
	//
	The_Game()->Set_QuickMatch_Server(false);

	/*
	// 
	// Gamespy games are not passwordable
	//
	The_Game()->IsPassworded.Set(false);
	The_Game()->Set_Password(L"");
	*/

	START_DIALOG (MPLanHostOptionsMenuClass);
}


////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
GameSpyOptionsDialogClass *	GameSpyOptionsDialogClass::_TheInstance	= NULL;


////////////////////////////////////////////////////////////////
//
//	GameSpyOptionsDialogClass
//
////////////////////////////////////////////////////////////////
GameSpyOptionsDialogClass::GameSpyOptionsDialogClass (void)	:
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MENU_GAMESPY_OPTIONS))
{
	_TheInstance = this;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~GameSpyOptionsDialogClass
//
////////////////////////////////////////////////////////////////
GameSpyOptionsDialogClass::~GameSpyOptionsDialogClass (void)
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
GameSpyOptionsDialogClass::On_Init_Dialog (void)
{
//	Init_Connection_Speed_Combo();

	//
	//	Put the nickname into the nickname edit control
	//      
	EditCtrlClass * name_edit = (EditCtrlClass *) Get_Dlg_Item(IDC_NICKNAME_EDIT);
	WWASSERT(name_edit != NULL);
	name_edit->Set_Text_Limit(30);
	name_edit->Set_Text(cNetInterface::Get_Nickname());

	// Initialize the GameSpy Game Port
	EditCtrlClass * port_edit = (EditCtrlClass *) Get_Dlg_Item(IDC_PORT_EDIT1);
	port_edit->Set_Text_Limit(5);
	port_edit->Set_Int(cUserOptions::GameSpyGamePort.Get());

	// Initialize the GameSpy Query Port
	EditCtrlClass * gsport_edit = (EditCtrlClass *) Get_Dlg_Item(IDC_PORT_EDIT2);
	gsport_edit->Set_Text_Limit(5);
	gsport_edit->Set_Int(cUserOptions::GameSpyQueryPort.Get());

	MenuDialogClass::On_Init_Dialog ();
}


////////////////////////////////////////////////////////////////
//
//	Init_Connection_Speed_Combo
//
////////////////////////////////////////////////////////////////
void GameSpyOptionsDialogClass::Init_Connection_Speed_Combo(void)
{
	ComboBoxCtrlClass* connectCombo = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_CONNECTION_SPEED_COMBO);

	if (connectCombo)	{
		connectCombo->Reset_Content();
		connectCombo->Add_String(TRANSLATE(IDS_MP_CONNECTION_288));
		connectCombo->Add_String(TRANSLATE(IDS_MP_CONNECTION_336));
		connectCombo->Add_String(TRANSLATE(IDS_MP_CONNECTION_56));
		connectCombo->Add_String(TRANSLATE(IDS_MP_CONNECTION_ISDN));
		connectCombo->Add_String(TRANSLATE(IDS_MP_CONNECTION_CABLE));
		connectCombo->Add_String(TRANSLATE(IDS_MP_CONNECTION_T1));

		// 
		// Set the current selection
		//
		//connectCombo->Set_Curr_Sel(cUserOptions::Get_Bandwidth_Type() - BANDWIDTH_FIRST);
		connectCombo->Set_Curr_Sel(cUserOptions::GameSpyBandwidthType.Get() - BANDWIDTH_FIRST);
		connectCombo->Set_Dirty(true);
	}
}


/*
////////////////////////////////////////////////////////////////
//
//	On_Last_Menu_Ending
//
////////////////////////////////////////////////////////////////
void
GameSpyOptionsDialogClass::On_Last_Menu_Ending (void)
{
	RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_GAMESPY_MAIN);
	return ;
}
*/


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
GameSpyOptionsDialogClass::On_Command (int ctrl_id, int mesage_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDCANCEL:
		case IDOK:
		case IDC_MENU_BACK_BUTTON:
		{
			//
			// Save the settings
			//
/*			ComboBoxCtrlClass* connectCombo = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_CONNECTION_SPEED_COMBO);
			if (connectCombo != NULL) {
				int selection = connectCombo->Get_Curr_Sel();

				if (selection >= 0) {
					BANDWIDTH_TYPE_ENUM bandwidth = 
						(BANDWIDTH_TYPE_ENUM)(selection + BANDWIDTH_FIRST);

					//cUserOptions::Set_Bandwidth_Type(bandwidth);
					cUserOptions::GameSpyBandwidthType.Set(bandwidth);
				}
			} */

			//
			//	Set the new nickname
			//
			WideStringClass nickname = Get_Dlg_Item_Text(IDC_NICKNAME_EDIT);
			cNetInterface::Set_Nickname(nickname);

			//
			//	Set the GameSpy Game Port
			//
			EditCtrlClass * port_edit = (EditCtrlClass *) Get_Dlg_Item(IDC_PORT_EDIT1);
			cUserOptions::GameSpyGamePort.Set(port_edit->Get_Int());

			//
			//	Set the GameSpy Query Port
			//
			port_edit = (EditCtrlClass *) Get_Dlg_Item(IDC_PORT_EDIT2);
			cUserOptions::GameSpyQueryPort.Set(port_edit->Get_Int());

			break;
		}
	}

	MenuDialogClass::On_Command (ctrl_id, mesage_id, param);
	return ;
}



////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
InternetMainDialogClass *	InternetMainDialogClass::_TheInstance	= NULL;


////////////////////////////////////////////////////////////////
//
//	InternetMainDialogClass
//
////////////////////////////////////////////////////////////////
InternetMainDialogClass::InternetMainDialogClass (void)	:
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MENU_MAIN_MULTIPLAY))
{
	_TheInstance = this;

	return ;
}


////////////////////////////////////////////////////////////////
//
//	~InternetMainDialogClass
//
////////////////////////////////////////////////////////////////
InternetMainDialogClass::~InternetMainDialogClass (void)
{
	_TheInstance = NULL;
	return ;
}

/*
////////////////////////////////////////////////////////////////
//
//	On_Last_Menu_Ending
//
////////////////////////////////////////////////////////////////
void
InternetMainDialogClass::On_Last_Menu_Ending (void)
{
	RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_MAIN_MENU);
	return ;
}
*/


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
InternetMainDialogClass::On_Init_Dialog (void)
{
#ifdef MULTIPLAYERDEMO
	Get_Dlg_Item(IDC_MENU_MP_INTERNET_WOL)->Enable(false);
#endif // MULTIPLAYERDEMO

#ifdef BETACLIENT
	Get_Dlg_Item(IDC_MENU_MP_INTERNET_GAMESPY)->Enable(false);
#endif // BETACLIENT

	MenuDialogClass::On_Init_Dialog ();
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
InternetMainDialogClass::On_Command (int ctrl_id, int mesage_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDC_MENU_MP_INTERNET_WOL:
		{
			GameInitMgrClass::Initialize_WOL ();
			START_DIALOG (MPWolMainMenuClass);
			break;
		}

		case IDC_MENU_MP_INTERNET_GAMESPY:
		{
			START_DIALOG (GameSpyMainDialogClass);
			break;
		}
	}

	MenuDialogClass::On_Command (ctrl_id, mesage_id, param);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	StartSPGameDialogClass
//
////////////////////////////////////////////////////////////////
StartSPGameDialogClass::StartSPGameDialogClass (void)	:
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MENU_START_SP))
{
	return ;
}



////////////////////////////////////////////////////////////////
//
//	DifficultyMenuClass
//
////////////////////////////////////////////////////////////////
DifficultyMenuClass::DifficultyMenuClass (void)	:
	CurrSel (-1),
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MENU_DIFFICULTY))
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
MPLanMenuClass::On_Init_Dialog (void)
{
	GameInitMgrClass::Initialize_LAN ();

	DialogControlClass *edit_ctrl = Get_Dlg_Item (IDC_NICKNAME_EDIT);
	if (edit_ctrl != NULL) {
		
		//
		//	Set the text of the nick name control
		//
		/*
		const char *nick_name = cNetInterface::Get_Nickname ();
		WideStringClass wide_nick_name;
		wide_nick_name.Convert_From (nick_name);
		edit_ctrl->Set_Text (wide_nick_name);
		*/
		edit_ctrl->Set_Text (cNetInterface::Get_Nickname ());
	}

	//
	//	Set the default focus
	//
	DialogMgrClass::Set_Focus (Get_Dlg_Item (IDC_MENU_MP_START_SERVER_BUTTON));
	
	MenuDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
MPLanMenuClass::On_Command (int ctrl_id, int mesage_id, unsigned int param)
{
	//
	//	Shutdown the LAN interface (if necessary)
	//
	switch (ctrl_id)
	{
		case IDCANCEL:
		case IDC_MENU_BACK_BUTTON:
		{
			GameInitMgrClass::Shutdown ();
			break;
		}

		case IDC_MENU_MP_START_SERVER_BUTTON:
		case IDC_MENU_MP_JOIN_GAME_BUTTON:
		{
			DialogControlClass *edit_ctrl = Get_Dlg_Item (IDC_NICKNAME_EDIT);
			if (edit_ctrl != NULL) {
				
				//
				//	Pass the nickname onto the network interface
				//
				WideStringClass wide_nick_name = edit_ctrl->Get_Text ();
				if (wide_nick_name.Get_Length () > 0) {
					cNetInterface::Set_Nickname(wide_nick_name);
					PLC->Save_Lan_Registry_Keys ();
				}
			}
			break;
		}

	}

	MenuDialogClass::On_Command (ctrl_id, mesage_id, param);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Destroy
//
////////////////////////////////////////////////////////////////
void
MPLanMenuClass::On_Destroy (void)
{
	MenuDialogClass::On_Destroy ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
MPMainMenuClass::On_Init_Dialog (void)
{
	//cSingleData::Set_Is_Single_Player (false);
	cGameType::Set_Game_Type(GAMETYPE_NONE);
	MenuDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Add_Folders
//
////////////////////////////////////////////////////////////////
/*void
Add_Folders (const char *path, TreeCtrlClass *tree_ctrl, TreeItemClass *parent_item)
{
	WIN32_FIND_DATAA find_info	= { 0 };
	BOOL keep_going				= true;
	HANDLE file_find				= NULL;
	
	StringClass full_path = path;
	full_path += "\\*.*";

	//
	//	Build a list of all the maps we know about
	//
	for (file_find = ::FindFirstFileA (full_path, &find_info);
		 (file_find != INVALID_HANDLE_VALUE) && keep_going;
		  keep_going = ::FindNextFileA (file_find, &find_info)) 
	{
		//
		//	Only do directories
		//
		if ((find_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			if (find_info.cFileName[0] != '.') {
				WideStringClass display_name;
				display_name.Convert_From (find_info.cFileName);
				TreeItemClass *new_item = tree_ctrl->Insert_Item (display_name, TreeCtrlClass::ICON_FOLDER,
																	TreeCtrlClass::ICON_FOLDER_OPEN, parent_item);

				new_item->Set_Needs_Children (true);
			}
		}
	}

	if (file_find != INVALID_HANDLE_VALUE) {			  
		::FindClose (file_find); 
	}

	//
	//	Sort the immediate children...
	//
	tree_ctrl->Sort_Children_Alphabetically (parent_item);
	return ;
}*/


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
StartSPGameDialogClass::On_Init_Dialog (void)
{
	//TreeCtrlClass *tree_ctrl = (TreeCtrlClass *)Get_Dlg_Item (IDC_TREE_CTRL);

	//_level = 0;
	//Add_Folders ("c:", tree_ctrl, NULL);

	MenuDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_TreeCtrl_Needs_Children
//
////////////////////////////////////////////////////////////////
/*void
StartSPGameDialogClass::On_TreeCtrl_Needs_Children (TreeCtrlClass *tree_ctrl, int ctrl_id, TreeItemClass *parent_item)
{
	if (parent_item == NULL) {
		return ;
	}

	WideStringClass full_path = parent_item->Get_Name ();

	TreeItemClass *curr_parent = parent_item;
	while ((curr_parent = curr_parent->Get_Parent ()) != NULL) {
		WideStringClass temp_path = curr_parent->Get_Name ();
		temp_path += L"\\";
		temp_path += full_path;
		full_path = temp_path;
	}

	WideStringClass temp_path = L"c:\\";
	temp_path += full_path;
	full_path = temp_path;

	StringClass ascii_full_path;
	full_path.Convert_To (ascii_full_path);

	Add_Folders (ascii_full_path, tree_ctrl, parent_item);
	parent_item->Set_Needs_Children (false);
	return ;
}*/


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
StartSPGameDialogClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{	

	//
	//	Does the user wish to start the tutorial?
	///
	if (ctrl_id == IDC_MENU_START_TUTORIAL_BUTTON) {		
		const char *TUTORIAL_MAP_NAME = "M00_Tutorial.mix";
		#define	TUTORIAL_LOAD_MENU_NUMBER		90

		//
		//	Simply load the map
		//
		cGod::Reset_Inventory();
		CampaignManager::Reset();
		CampaignManager::Select_Backdrop_Number( TUTORIAL_LOAD_MENU_NUMBER );
		GameInitMgrClass::Initialize_SP ();
		GameInitMgrClass::Start_Game (TUTORIAL_MAP_NAME, -1, 0);
	} else {
		CampaignManager::Select_Backdrop_Number( 0 );	// Use default load number
	}

	MenuDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}


/*
////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
MPServerStartMenuClass::On_Init_Dialog (void)
{
	//
	//	Enable or disable the controls
	//
	//Get_Dlg_Item (IDC_TEAM_STATIC)->Enable (The_Game ()->IsTeamChangingAllowed.Is_True () == false);
	//Get_Dlg_Item (IDC_TEAM_COMBO)->Enable (The_Game ()->IsTeamChangingAllowed.Is_True () == false);

	//
	//	Configure the combobox
	//
	//ComboBoxCtrlClass *type_combo = (ComboBoxCtrlClass *)Get_Dlg_Item (IDC_TEAM_COMBO);
	//type_combo->Add_String (L"GDI");
	//type_combo->Add_String (L"NOD");

	//
	//	Set the default focus
	//
	DialogMgrClass::Set_Focus (Get_Dlg_Item (IDC_MENU_MP_START_BUTTON));

	MenuDialogClass::On_Init_Dialog ();

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Start_Game
//
////////////////////////////////////////////////////////////////
void
MPServerStartMenuClass::Start_Game (void)
{
	//
	//	Read the player's team choice from the combo
	//
	//if (The_Game ()->IsTeamChangingAllowed.Is_True () == false) {
	//	ComboBoxCtrlClass *type_combo = (ComboBoxCtrlClass *)Get_Dlg_Item (IDC_TEAM_COMBO);			
	//	if (type_combo->Get_Curr_Sel () == 0) {
	//		cUserOptions::Set_Team_Choice (PLAYERTYPE_GDI);
	//	} else {
	//		cUserOptions::Set_Team_Choice (PLAYERTYPE_NOD);
	//	}
	//}

	//
	//	Check to ensure the settings are playable
	//
	if (The_Game ()->Is_Valid_Settings ()) {

		//
		//	Handle a dedicated server (if necessary)
		//
		GameInitMgrClass::Set_Is_Client_Required (true);
		GameInitMgrClass::Set_Is_Server_Required (false);

		CampaignManager::Select_Backdrop_Number_By_MP_Type( The_Game()->Get_Game_Type() );

		//
		//	Start the game!
		//
		GameInitMgrClass::Start_Game (The_Game ()->Get_Map_Name ());
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
MPServerStartMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	//if (ctrl_id == IDC_MP_PASSWORD_VALID_COMMAND) {
	//	Start_Game ();
	//} else if (ctrl_id == IDC_MENU_MP_START_BUTTON) {
	//
	//	//
	//	//	Prompt the user for the password (if necessary)
	//	//
	//	if (IsServer == false && The_Game ()->IsPassworded.Is_True ()) {
	//		MPGamePasswordPopupClass *dialog = new MPGamePasswordPopupClass;
	//		dialog->Notify_Dialog (this);
	//		dialog->Start_Dialog ();
	//		REF_PTR_RELEASE (dialog);
	//	} else {
	//		
	//		//
	//		//	Otherwise, simply start the game
	//		//
	//		Start_Game ();			
	//	}		
	//}

	//
	// TSS092501 force user to proceed pass this dialog, later on remove it properly
	//
	//if (ctrl_id == IDC_MENU_MP_START_BUTTON) {
	//	Start_Game ();			
	//}
	Start_Game();

	MenuDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}
*/


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
MPJoinMenuClass::On_Init_Dialog (void)
{
#if(1)
	assert(!"NEW_WWONLINE: Denzil look here!");
#else
	if (WolGameModeClass::PWolChat != NULL) {
		WolGameModeClass::PWolChat->Go_To_GameList ();
	}
#endif

	//
	//	Get a pointer to the list control
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_GAME_LIST);
	if (list_ctrl != NULL) {

		//
		//	Configure the columns
		//
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_TYPE), 0.4F, Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_HOST), 0.4F, Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_PLAYERS), 0.2F, Vector3 (1, 1, 1));

		Update_Game_List ();
	}

	MenuDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
MPJoinMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	if (ctrl_id == IDC_MENU_MP_JOING_BUTTON) {
		ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_GAME_LIST);

		int index = list_ctrl->Get_Curr_Sel ();
		if (index >= 0) {
			cGameChannel *channel = (cGameChannel *)list_ctrl->Get_Entry_Data (index, 0);
			if (channel != NULL) {

				if (PTheGameData != NULL) {
					delete PTheGameData;
					PTheGameData = NULL;
				}

				PTheGameData = cGameData::Create_Game_Of_Type (channel->Get_Game_Data()->Get_Game_Type());
				WWASSERT(PTheGameData != NULL);
				*PTheGameData = *channel->Get_Game_Data ();

				cNetwork::Init_Client();

				//
				//	Display the "connecting" dialog
				//
				DlgMPConnect::DoDialog(-1, 0);
			}
		}
	}

	MenuDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
MPJoinMenuClass::On_Frame_Update (void)
{
	static int _LastUpdate = 0;

	//
	//	Update the list every 5 seconds
	//
	int curr_time = WW3D::Get_Sync_Time ();
	if ((curr_time - _LastUpdate) > 5000) {
		Update_Game_List ();
		_LastUpdate = curr_time;
	}

	MenuDialogClass::On_Frame_Update ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Game_List
//
////////////////////////////////////////////////////////////////
void
MPJoinMenuClass::Update_Game_List (void)
{
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_GAME_LIST);
	if (list_ctrl == NULL) {
		return ;
	}

	//
	//	Start fresh
	//
	list_ctrl->Delete_All_Entries ();

#if(1)
	assert(!"NEW_WWONLINE: Denzil look here!");
#else
	//
	//	Update the channel list
	//
	if (GameModeManager::Find("WOL")->Is_Active()) {
		WolGameModeClass::PWolChat->My_Request_Channel_List(true);
	}
#endif

	//
	//	Build the game list
	//
	int index = 0;
	SLNode<cGameChannel> * objnode;
	for (objnode = cGameChannelList::Get_Chan_List()->Head(); objnode; objnode = objnode->Next()) {
		cGameChannel * p_channel = objnode->Data();
		WWASSERT(p_channel != NULL);		

	   if (GameModeManager::Find("WOL")->Is_Active()) {
			WWASSERT(p_channel->Get_Wol_Channel() != NULL);
			p_channel->Get_Game_Data()->Set_Current_Players(p_channel->Get_Wol_Channel()->currentUsers);
		}		

		int player_count		= p_channel->Get_Game_Data()->Get_Current_Players ();
		int player_count_max = p_channel->Get_Game_Data()->Get_Max_Players ();

		WideStringClass wide_players_string;
		wide_players_string.Format (L"%d/%d", player_count, player_count_max);

		WideStringClass wide_owner_name;
		WideStringClass wide_game_name;
		//wide_owner_name.Convert_From (p_channel->Get_Game_Data()->Get_Owner ());
		wide_owner_name = p_channel->Get_Game_Data()->Get_Owner ();
		//wide_game_name.Convert_From (p_channel->Get_Game_Data()->Get_Game_Name ());
		wide_game_name = p_channel->Get_Game_Data()->Get_Game_Name ();
		
		//
		//	Insert the entry
		//
		int item_index = list_ctrl->Insert_Entry (index++, wide_game_name);
		if (item_index >= 0) {
			list_ctrl->Set_Entry_Text (item_index, 1, wide_owner_name);
			list_ctrl->Set_Entry_Text (item_index, 2, wide_players_string);
			list_ctrl->Set_Entry_Data (item_index, 0, (uintptr_t)p_channel);
		}

      //
		//	Is there something wrong with the connection?
		//
		if (p_channel->Get_Game_Data()->Get_Version_Number() != cNetwork::Get_Exe_Key()) {

			WideStringClass error_string;
			if (p_channel->Get_Game_Data()->Do_Exe_Versions_Match() == false) {
				error_string = TRANSLATE (IDS_MENU_EXE_MISMATCH);
			} else if (p_channel->Get_Game_Data()->Do_String_Versions_Match() == false) {
				error_string = TRANSLATE (IDS_MENU_STRINGS_MISMATCH);
			}

			//
			//	Notify the user that they can't join this game
			//
			if (item_index >= 0) {
				list_ctrl->Set_Entry_Text (item_index, 0, error_string);
				list_ctrl->Set_Entry_Color (item_index, 0, Vector3 (0.5F, 0.5F, 0.5F));
				list_ctrl->Set_Entry_Color (item_index, 1, Vector3 (0.5F, 0.5F, 0.5F));
				list_ctrl->Set_Entry_Color (item_index, 2, Vector3 (0.5F, 0.5F, 0.5F));
				list_ctrl->Set_Entry_Data (item_index, 0, 0);
			}
		}
	}

	//
	//	Select the first entry by default
	//
	if (list_ctrl->Get_Entry_Count () > 0) {
		list_ctrl->Set_Curr_Sel (0);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
DifficultyMenuClass::On_Frame_Update (void)
{
	int curr_sel = -1;
	DialogControlClass *curr_focus = DialogMgrClass::Get_Focus ();

	//
	//	Determine which control has the focus
	//
	if (curr_focus == Get_Dlg_Item (IDC_MENU_DIFFCULTY01_BUTTON)) {
		curr_sel = 0;
	} else if (curr_focus == Get_Dlg_Item (IDC_MENU_DIFFCULTY02_BUTTON)) {
		curr_sel = 1;
	} else if (curr_focus == Get_Dlg_Item (IDC_MENU_DIFFCULTY03_BUTTON)) {
		curr_sel = 2;
	}

	//
	//	Did something change?
	//
	if (curr_sel != CurrSel) {
		CurrSel = curr_sel;
		
		if (CurrSel == -1) {
			Set_Dlg_Item_Text (IDC_AIM_TEXT,					TRANSLATE (IDS_MENU_NA));
			Set_Dlg_Item_Text (IDC_PLAYER_HEALTH_TEXT,	TRANSLATE (IDS_MENU_NA));
			Set_Dlg_Item_Text (IDC_SUPPLIES_TEXT,			TRANSLATE (IDS_MENU_NA));
			Set_Dlg_Item_Text (IDC_REINFORCEMENTS_TEXT,	TRANSLATE (IDS_MENU_NA));
			Set_Dlg_Item_Text (IDC_BODY_ARMOR_TEXT,		TRANSLATE (IDS_MENU_NA));
		} else if (CurrSel == 0) {
			Set_Dlg_Item_Text (IDC_AIM_TEXT,					TRANSLATE (IDS_MENU_ON));
			Set_Dlg_Item_Text (IDC_PLAYER_HEALTH_TEXT,	TRANSLATE (IDS_MENU_MAXIMUM));
			Set_Dlg_Item_Text (IDC_SUPPLIES_TEXT,			TRANSLATE (IDS_MENU_PLENTIFUL));
			Set_Dlg_Item_Text (IDC_REINFORCEMENTS_TEXT,	TRANSLATE (IDS_MENU_FEW));
			Set_Dlg_Item_Text (IDC_BODY_ARMOR_TEXT,		TRANSLATE (IDS_MENU_LIGHT));
		} else if (CurrSel == 1) {
			Set_Dlg_Item_Text (IDC_AIM_TEXT,					TRANSLATE (IDS_MENU_OFF));
			Set_Dlg_Item_Text (IDC_PLAYER_HEALTH_TEXT,	TRANSLATE (IDS_MENU_ENHANCED));
			Set_Dlg_Item_Text (IDC_SUPPLIES_TEXT,			TRANSLATE (IDS_MENU_SUFFICIENT));
			Set_Dlg_Item_Text (IDC_REINFORCEMENTS_TEXT,	TRANSLATE (IDS_MENU_MANY));
			Set_Dlg_Item_Text (IDC_BODY_ARMOR_TEXT,		TRANSLATE (IDS_MENU_STANDARD));
		} else if (CurrSel == 2) {
			Set_Dlg_Item_Text (IDC_AIM_TEXT,					TRANSLATE (IDS_MENU_OFF));
			Set_Dlg_Item_Text (IDC_PLAYER_HEALTH_TEXT,	TRANSLATE (IDS_MENU_NORMAL));
			Set_Dlg_Item_Text (IDC_SUPPLIES_TEXT,			TRANSLATE (IDS_MENU_SCARCE));
			Set_Dlg_Item_Text (IDC_REINFORCEMENTS_TEXT,	TRANSLATE (IDS_MENU_MAXIMUM));
			Set_Dlg_Item_Text (IDC_BODY_ARMOR_TEXT,		TRANSLATE (IDS_MENU_HEAVY));
		}
	}

	MenuDialogClass::On_Frame_Update ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Menu_Activate
//
////////////////////////////////////////////////////////////////
void
DifficultyMenuClass::On_Menu_Activate (bool onoff)
{
	MenuDialogClass::On_Menu_Activate (onoff);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
DifficultyMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDC_MENU_DIFFCULTY01_BUTTON:
		case IDC_MENU_DIFFCULTY02_BUTTON:
		case IDC_MENU_DIFFCULTY03_BUTTON:
		case IDC_MENU_DIFFCULTY04_BUTTON:
		{
			int difficulty = ctrl_id - IDC_MENU_DIFFCULTY01_BUTTON;
			if ( ReplayFilename.Is_Empty() ) {
				GameInitMgrClass::Initialize_SP();
				CampaignManager::Start_Campaign( difficulty );
			} else {
				WWDEBUG_SAY(( "REPLAY %d\n", difficulty ));

				//
				//	End the current game before we load the new one	 (CODE REMOVED FROM LOADSPGAME)
				//
				if (GameModeManager::Find ("Combat")->Is_Suspended ()) {
					GameInitMgrClass::End_Game();
					GameModeManager::Safely_Deactivate ();
				}

				GameInitMgrClass::Initialize_SP ();
				CampaignManager::Replay_Level( ReplayFilename, difficulty );
			}
			break;
		}
	}

	MenuDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}

////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
DeathOptionsPopupClass::On_Init_Dialog ( void )
{
	// IML: Ensure that there are no sound effects lingering on any playlist. 
	WWAudioClass::Get_Instance ()->Flush_Playlist();

	PopupDialogClass::On_Init_Dialog ();

#if 01
	
	//
	//	Activate the menu game mode (if necessary)
	//
	GameModeClass *menu_game_mode = GameModeManager::Find ("Menu");
	if (menu_game_mode != NULL && menu_game_mode->Is_Active () == false) {
		menu_game_mode->Activate ();
	}
	GameModeManager::Find ("Combat")->Suspend();
#endif
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
DeathOptionsPopupClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	bool do_default = true;

	switch (ctrl_id)
	{
		case IDCANCEL:
		case IDC_DEATH_OPTION_QUIT:
			GameInitMgrClass::Continue_Game();
			GameInitMgrClass::End_Game ();
			GameInitMgrClass::Display_End_Game_Menu ();
			End_Dialog ();
			do_default = false;
			break;

		case IDC_DEATH_OPTION_RESTART:
			GameInitMgrClass::Continue_Game();
			cGod::Restart();
			End_Dialog();
			break;

		case IDC_DEATH_OPTION_LOAD:
			GameInitMgrClass::Continue_Game();
			cGod::Load_Game();
			End_Dialog();
			break;
	}

	if (do_default) {
		PopupDialogClass::On_Command (ctrl_id, message_id, param);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
FailedOptionsPopupClass::On_Init_Dialog ( void )
{
	// IML: Ensure that there are no sound effects lingering on any playlist. 
	WWAudioClass::Get_Instance ()->Flush_Playlist();

	PopupDialogClass::On_Init_Dialog ();

#if 01
	//
	//	Activate the menu game mode (if necessary)
	//
	GameModeClass *menu_game_mode = GameModeManager::Find ("Menu");
	if (menu_game_mode != NULL && menu_game_mode->Is_Active () == false) {
		menu_game_mode->Activate ();
	}
	GameModeManager::Find ("Combat")->Suspend();
#endif
	return ;
}



////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
FailedOptionsPopupClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	bool allow_default_processing = true;

	switch (ctrl_id)
	{
		case IDC_FAILED_OPTION_RESTART:
			GameInitMgrClass::Continue_Game();
			cGod::Restart();
			End_Dialog();
			break;

		case IDC_FAILED_OPTION_LOAD:
			GameInitMgrClass::Continue_Game();
			cGod::Load_Game();
			End_Dialog();
			break;

		case IDCANCEL:
			allow_default_processing = false;
			break;

		case IDC_MENU_MAIN_MENU_BUTTON:
		{
			GameInitMgrClass::Continue_Game();
			GameInitMgrClass::End_Game();
			GameInitMgrClass::Display_End_Game_Menu();

			//
			//	Close the dialog
			//
			End_Dialog ();
			allow_default_processing = false;
			break;
		}
	}

	if (allow_default_processing) {
		PopupDialogClass::On_Command (ctrl_id, message_id, param);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	EditWheeledVehicleDialogClass
//
////////////////////////////////////////////////////////////////
EditWheeledVehicleDialogClass::EditWheeledVehicleDialogClass(WheeledVehicleDefClass * def,float wheel_radius) :
	PopupDialogClass(GetRenegadeDialog(RenegadeDialogID::IDD_EDIT_WHEELED_VEHICLE)),
	VehicleDef(def),
	WheelRadius(wheel_radius)
{
}

EditWheeledVehicleDialogClass::~EditWheeledVehicleDialogClass(void)
{
}

void	
EditWheeledVehicleDialogClass::On_Init_Dialog (void)
{
	if (VehicleDef != NULL) {

		Set_Dlg_Item_Float(IDC_MASS_EDIT,VehicleDef->Get_Mass());
		Set_Dlg_Item_Float(IDC_GRAVSCALE_EDIT,VehicleDef->Get_Grav_Scale());
		Set_Dlg_Item_Float(IDC_AERODRAG_EDIT,VehicleDef->Get_Aerodynamic_Drag());
		Set_Dlg_Item_Float(IDC_SPRING_CONSTANT_EDIT,VehicleDef->Get_Spring_Constant());
		Set_Dlg_Item_Float(IDC_DAMPING_CONSTANT_EDIT,VehicleDef->Get_Damping_Constant());
		Set_Dlg_Item_Float(IDC_SPRING_LENGTH_EDIT,VehicleDef->Get_Spring_Length());
		Set_Dlg_Item_Float(IDC_LATERAL_MOMENT_ARM_EDIT,VehicleDef->Get_Lateral_Moment_Arm());
		Set_Dlg_Item_Float(IDC_TRACTIVE_MOMENT_ARM_EDIT,VehicleDef->Get_Tractive_Moment_Arm());
		Set_Dlg_Item_Float(IDC_MAX_ENGINE_TORQUE_EDIT,VehicleDef->Get_Max_Engine_Torque());
		
		Set_Dlg_Item_Int(IDC_GEAR_COUNT_EDIT, VehicleDef->Get_Gear_Count());
		Set_Dlg_Item_Float(IDC_GEAR_RATIO1_EDIT, VehicleDef->Get_Gear_Ratio(0));	
		Set_Dlg_Item_Float(IDC_GEAR_RATIO2_EDIT, VehicleDef->Get_Gear_Ratio(1));	
		Set_Dlg_Item_Float(IDC_GEAR_RATIO3_EDIT, VehicleDef->Get_Gear_Ratio(2));	
		Set_Dlg_Item_Float(IDC_GEAR_RATIO4_EDIT, VehicleDef->Get_Gear_Ratio(3));	
		Set_Dlg_Item_Float(IDC_GEAR_RATIO5_EDIT, VehicleDef->Get_Gear_Ratio(4));	
		Set_Dlg_Item_Float(IDC_MAX_STEERING_ANGLE_EDIT, VehicleDef->Get_Max_Steering_Angle());
		Set_Dlg_Item_Float(IDC_SHIFT_UP_RPM_EDIT, VehicleDef->Get_Shift_Up_Rpm());
		Set_Dlg_Item_Float(IDC_SHIFT_DOWN_RPM_EDIT, VehicleDef->Get_Shift_Down_Rpm());

		WideStringClass radius_text;
		radius_text.Format(L"%10.4f",WheelRadius);
		Set_Dlg_Item_Text(IDC_WHEEL_RADIUS,radius_text);

	} else {
		
		Set_Controls_Hidden(true);
/*
		Enable_Dlg_Item(IDC_MASS_EDIT,false);
		Enable_Dlg_Item(IDC_GRAVSCALE_EDIT,false);
		Enable_Dlg_Item(IDC_AERODRAG_EDIT,false);
		Enable_Dlg_Item(IDC_SPRING_CONSTANT_EDIT,false);
		Enable_Dlg_Item(IDC_DAMPING_CONSTANT_EDIT,false);
		Enable_Dlg_Item(IDC_SPRING_LENGTH_EDIT,false);
		Enable_Dlg_Item(IDC_MAX_ENGINE_TORQUE_EDIT,false);

		Enable_Dlg_Item(IDC_GEAR_COUNT_EDIT, false);
		Enable_Dlg_Item(IDC_GEAR_RATIO1_EDIT, false);	
		Enable_Dlg_Item(IDC_GEAR_RATIO2_EDIT, false);	
		Enable_Dlg_Item(IDC_GEAR_RATIO3_EDIT, false);	
		Enable_Dlg_Item(IDC_GEAR_RATIO4_EDIT, false);	
		Enable_Dlg_Item(IDC_GEAR_RATIO5_EDIT, false);	
		Enable_Dlg_Item(IDC_MAX_STEERING_ANGLE_EDIT, false);
*/
	}
	PopupDialogClass::On_Init_Dialog ();
}

void	
EditWheeledVehicleDialogClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDOK:
			if (VehicleDef != NULL) {
				VehicleDef->Set_Mass(Get_Dlg_Item_Float(IDC_MASS_EDIT));
				VehicleDef->Set_Grav_Scale(Get_Dlg_Item_Float(IDC_GRAVSCALE_EDIT));
				VehicleDef->Set_Aerodynamic_Drag(Get_Dlg_Item_Float(IDC_AERODRAG_EDIT));
				VehicleDef->Set_Spring_Constant(Get_Dlg_Item_Float(IDC_SPRING_CONSTANT_EDIT));
				VehicleDef->Set_Damping_Constant(Get_Dlg_Item_Float(IDC_DAMPING_CONSTANT_EDIT));
				VehicleDef->Set_Spring_Length(Get_Dlg_Item_Float(IDC_SPRING_LENGTH_EDIT));
				VehicleDef->Set_Lateral_Moment_Arm(Get_Dlg_Item_Float(IDC_LATERAL_MOMENT_ARM_EDIT));
				VehicleDef->Set_Tractive_Moment_Arm(Get_Dlg_Item_Float(IDC_TRACTIVE_MOMENT_ARM_EDIT));
				VehicleDef->Set_Max_Engine_Torque(Get_Dlg_Item_Float(IDC_MAX_ENGINE_TORQUE_EDIT));
			
				VehicleDef->Set_Gear_Count(Get_Dlg_Item_Int(IDC_GEAR_COUNT_EDIT));
				VehicleDef->Set_Gear_Ratio(0,Get_Dlg_Item_Float(IDC_GEAR_RATIO1_EDIT));
				VehicleDef->Set_Gear_Ratio(1,Get_Dlg_Item_Float(IDC_GEAR_RATIO2_EDIT));
				VehicleDef->Set_Gear_Ratio(2,Get_Dlg_Item_Float(IDC_GEAR_RATIO3_EDIT));
				VehicleDef->Set_Gear_Ratio(3,Get_Dlg_Item_Float(IDC_GEAR_RATIO4_EDIT));
				VehicleDef->Set_Gear_Ratio(4,Get_Dlg_Item_Float(IDC_GEAR_RATIO5_EDIT));
				VehicleDef->Set_Max_Steering_Angle(Get_Dlg_Item_Float(IDC_MAX_STEERING_ANGLE_EDIT));
				VehicleDef->Set_Shift_Up_Rpm(Get_Dlg_Item_Float(IDC_SHIFT_UP_RPM_EDIT));
				VehicleDef->Set_Shift_Down_Rpm(Get_Dlg_Item_Float(IDC_SHIFT_DOWN_RPM_EDIT));
				
				PhysicsSceneClass * pscene = PhysicsSceneClass::Get_Instance();
				if (pscene) {
					RefPhysListIterator iterator = pscene->Get_Dynamic_Object_Iterator();
					while (!iterator.Is_Done()) {
						if (	(iterator.Peek_Obj() != NULL) && 
								(iterator.Peek_Obj()->Get_Definition() == VehicleDef)) 
						{
							iterator.Peek_Obj()->Definition_Changed();
						}
						iterator.Next();
					}
				}
			}			
			End_Dialog();
			break;
	}

	PopupDialogClass::On_Command (ctrl_id, message_id, param);

	return ;
}



////////////////////////////////////////////////////////////////
//
//	EditTrackedVehicleDialogClass
//
////////////////////////////////////////////////////////////////
EditTrackedVehicleDialogClass::EditTrackedVehicleDialogClass(TrackedVehicleDefClass * def,float wheel_radius) :
	PopupDialogClass(GetRenegadeDialog(RenegadeDialogID::IDD_EDIT_TRACKED_VEHICLE)),
	VehicleDef(def),
	WheelRadius(wheel_radius)
{
}

EditTrackedVehicleDialogClass::~EditTrackedVehicleDialogClass(void)
{
}

void	
EditTrackedVehicleDialogClass::On_Init_Dialog (void)
{
	if (VehicleDef != NULL) {

		Set_Dlg_Item_Float(IDC_MASS_EDIT,VehicleDef->Get_Mass());
		Set_Dlg_Item_Float(IDC_GRAVSCALE_EDIT,VehicleDef->Get_Grav_Scale());
		Set_Dlg_Item_Float(IDC_AERODRAG_EDIT,VehicleDef->Get_Aerodynamic_Drag());
		Set_Dlg_Item_Float(IDC_SPRING_CONSTANT_EDIT,VehicleDef->Get_Spring_Constant());
		Set_Dlg_Item_Float(IDC_DAMPING_CONSTANT_EDIT,VehicleDef->Get_Damping_Constant());
		Set_Dlg_Item_Float(IDC_SPRING_LENGTH_EDIT,VehicleDef->Get_Spring_Length());
		Set_Dlg_Item_Float(IDC_LATERAL_MOMENT_ARM_EDIT,VehicleDef->Get_Lateral_Moment_Arm());
		Set_Dlg_Item_Float(IDC_TRACTIVE_MOMENT_ARM_EDIT,VehicleDef->Get_Tractive_Moment_Arm());
		Set_Dlg_Item_Float(IDC_MAX_ENGINE_TORQUE_EDIT,VehicleDef->Get_Max_Engine_Torque());
		Set_Dlg_Item_Float(IDC_TRACK_U_SCALE_FACTOR_EDIT, VehicleDef->Get_Track_U_Scale_Factor());	
		Set_Dlg_Item_Float(IDC_TRACK_V_SCALE_FACTOR_EDIT, VehicleDef->Get_Track_V_Scale_Factor());	
		Set_Dlg_Item_Float(IDC_TURN_TORQUE_SCALE_FACTOR_EDIT, VehicleDef->Get_Turn_Torque_Scale_Factor());	
		
		WideStringClass radius_text;
		radius_text.Format(L"%10.4f",WheelRadius);
		Set_Dlg_Item_Text(IDC_WHEEL_RADIUS,radius_text);

	} else {
		
		Set_Controls_Hidden(true);
	}
	PopupDialogClass::On_Init_Dialog ();
}

void	
EditTrackedVehicleDialogClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDOK:
			if (VehicleDef != NULL) {
				VehicleDef->Set_Mass(Get_Dlg_Item_Float(IDC_MASS_EDIT));
				VehicleDef->Set_Grav_Scale(Get_Dlg_Item_Float(IDC_GRAVSCALE_EDIT));
				VehicleDef->Set_Aerodynamic_Drag(Get_Dlg_Item_Float(IDC_AERODRAG_EDIT));
				VehicleDef->Set_Spring_Constant(Get_Dlg_Item_Float(IDC_SPRING_CONSTANT_EDIT));
				VehicleDef->Set_Damping_Constant(Get_Dlg_Item_Float(IDC_DAMPING_CONSTANT_EDIT));
				VehicleDef->Set_Spring_Length(Get_Dlg_Item_Float(IDC_SPRING_LENGTH_EDIT));
				VehicleDef->Set_Lateral_Moment_Arm(Get_Dlg_Item_Float(IDC_LATERAL_MOMENT_ARM_EDIT));
				VehicleDef->Set_Tractive_Moment_Arm(Get_Dlg_Item_Float(IDC_TRACTIVE_MOMENT_ARM_EDIT));
				VehicleDef->Set_Max_Engine_Torque(Get_Dlg_Item_Float(IDC_MAX_ENGINE_TORQUE_EDIT));
				VehicleDef->Set_Track_U_Scale_Factor(Get_Dlg_Item_Float(IDC_TRACK_U_SCALE_FACTOR_EDIT));
				VehicleDef->Set_Track_V_Scale_Factor(Get_Dlg_Item_Float(IDC_TRACK_V_SCALE_FACTOR_EDIT));
				VehicleDef->Set_Turn_Torque_Scale_Factor(Get_Dlg_Item_Float(IDC_TURN_TORQUE_SCALE_FACTOR_EDIT));
			
				PhysicsSceneClass * pscene = PhysicsSceneClass::Get_Instance();
				if (pscene) {
					RefPhysListIterator iterator = pscene->Get_Dynamic_Object_Iterator();
					while (!iterator.Is_Done()) {
						if (	(iterator.Peek_Obj() != NULL) && 
								(iterator.Peek_Obj()->Get_Definition() == VehicleDef)) 
						{
							iterator.Peek_Obj()->Definition_Changed();
						}
						iterator.Next();
					}
				}
			}			
			End_Dialog();
			break;
	}

	PopupDialogClass::On_Command (ctrl_id, message_id, param);

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
MultiplayOptionsMainMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDC_MP_OPTIONS_BUTTON:
			START_DIALOG (MultiplayOptionsMenuClass);
			break;

		case IDC_SKIN_SELECTION_BUTTON:
			START_DIALOG (CharacterOptionsMenuClass);
			break;
	}

	MenuDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}
