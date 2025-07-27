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

/******************************************************************************
*
* FILE
*     $Archive: /Commando/Code/Commando/DlgMPTeamSelect.cpp $
*
* DESCRIPTION
*     Multiplayer team selection dialog.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 17 $
*     $Modtime: 2/24/02 2:51p $
*
******************************************************************************/

#include "dlgmpteamselect.h"
#include "wolgmode.h"
#include "gamedata.h"
#include "wolgameinfo.h"
#include "wolloginprofile.h"
#include "dlgmessagebox.h"
#include "renegadedialogmgr.h"
#include <combat\playertype.h>
#include <wwonline\wolgameoptions.h>
#include <wwui\listctrl.h>
#include <wwui\comboboxctrl.h>
#include <wwui\imagectrl.h>
#include "resource.h"
#include "string_ids.h"
#include <wwtranslatedb\translatedb.h>
#include "modpackagemgr.h"
#include "gameinitmgr.h"


using namespace WWOnline;


// Player list columns
enum
	{
	COL_RANK,
	COL_NAME,
// Denzil 02/24/02 Day 1 patch - Remove clan until we can fix formating
//	COL_CLAN,
	COL_KD,
	COL_SCORE
	};


// Private game options processing dispatching
typedef void (*GameOptionsDispatchFunc)(DlgMPTeamSelect&, const char*);

#define PARSE_INT(s, d, v) {char* ptr = strtok(s, d); if (ptr) {v = atoi(ptr);}}
#define PARSE_FLOAT(s, d, v) {char* ptr = strtok(s, d); if (ptr) {v = atof(ptr);}}
#define PARSE_HEXDWORD(s, d, v) {char* ptr = strtok(s, d); if (ptr) {sscanf(ptr, "%08X", &v);}}
#define PARSE_HEXBYTE(s, d, v) {char* ptr = strtok(s, d); if (ptr) {sscanf(ptr, "%02X", &v);}}
#define PARSE_STRING(s, d, v) {v = strtok(s, d);}


static int CALLBACK ListSortCallback(ListCtrlClass* list, int index1, int index2, uint32 param)
	{
	int rank1 = (int)list->Get_Entry_Data(index1, COL_RANK);
	int rank2 = (int)list->Get_Entry_Data(index2, COL_RANK);
	return (rank1 - rank2);
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::DoDialog
*
* DESCRIPTION
*     Display team selection / team profile dialog
*
* INPUTS
*     Target - Target to receive user choice signal.
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMPTeamSelect::DoDialog(Signaler<MPChooseTeamSignal>& target)
	{
	DlgMPTeamSelect* dialog = new DlgMPTeamSelect;

	if (dialog)
		{
		if (dialog->FinalizeCreate())
			{
			dialog->Start_Dialog();
			dialog->SignalMe(target);
			}

		dialog->Release_Ref();
		}
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::DlgMpTeamSelect
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

DlgMPTeamSelect::DlgMPTeamSelect(void) :
		MenuDialogClass(IDD_MP_TEAM_SELECT),
		mWOLGame(true),
		mCanChoose(true),
		mTimeRemaining(0.0f)
	{
	WWDEBUG_SAY(("DlgMPTeamSelect instantiated\n"));
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::~DlgMPTeamSelect
*
* DESCRIPTION
*     Destructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

DlgMPTeamSelect::~DlgMPTeamSelect()
	{
	WWDEBUG_SAY(("DlgMPTeamSelect destroyed\n"));
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::FinalizeCreate
*
* DESCRIPTION
*     Post creation initialization.
*
* INPUTS
*     NONE
*
* RESULT
*     True if successful
*
******************************************************************************/

bool DlgMPTeamSelect::FinalizeCreate(void)
	{
	GameModeClass* gameMode = GameModeManager::Find("WOL");

	if (gameMode && gameMode->Is_Active())
		{
		mWOLGame = true;
		mWOLSession = Session::GetInstance(false);

		// Cannot continue if WOL session is not initialized
		if (mWOLSession.IsValid() == false)
			{
			return false;
			}

		// Get the channel we have joined.
		const RefPtr<ChannelData>& channel = mWOLSession->GetCurrentChannel();

		// Cannot continue if we are not in a channel
		if (channel.IsValid() == false)
			{
			return false;
			}

		mGameInfo.ImportFromChannel(channel);

		Observer<ChannelEvent>::NotifyMe(*mWOLSession);
		Observer<UserEvent>::NotifyMe(*mWOLSession);
		}
	else
		{
		gameMode = GameModeManager::Find("LAN");

		if (gameMode && gameMode->Is_Active())
			{
			mWOLGame = false;
			mGameInfo.ImportFromGame(*The_Game());
			}
		}

	if (mGameInfo.IsDataValid() == false)
		{
		return false;
		}

	mCanChoose = (!mGameInfo.IsClanGame() && mGameInfo.IsTeamChange());

	return true;
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::On_Init_Dialog
*
* DESCRIPTION
*     One time dialog initialization
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMPTeamSelect::On_Init_Dialog(void)
	{
	WWDEBUG_SAY(("DlgMPTeamSelect On_Init_Dialog\n"));

	MenuDialogClass::On_Init_Dialog();

	// Setup the team icons on the dialog
	((ImageCtrlClass*)Get_Dlg_Item(IDC_GDI_TEAM_ICON))->Set_Texture("HUD_C&C_GDILOGO.TGA");
	((ImageCtrlClass*)Get_Dlg_Item(IDC_NOD_TEAM_ICON))->Set_Texture("HUD_C&C_NODLOGO.TGA");

	// Setup the list controls
	ListCtrlClass* list = (ListCtrlClass*)Get_Dlg_Item(IDC_GDI_LIST_CTRL);

	if (list)
		{
		// Configure the columns
		list->Add_Column(TRANSLATE (IDS_MENU_RANK),  0.15F, Vector3(1, 1, 1));

// Denzil 02/24/02 Day 1 patch - Remove clan until we can fix formatting issues.
#if(0)
		list->Add_Column(TRANSLATE (IDS_MENU_NAME),  0.30F, Vector3(1, 1, 1));
		list->Add_Column(TRANSLATE (IDS_MENU_CLAN),  0.20F, Vector3(1, 1, 1));
#else
		list->Add_Column(TRANSLATE (IDS_MENU_NAME),  0.50F, Vector3(1, 1, 1));
#endif

		list->Add_Column(TRANSLATE (IDS_MENU_KD_RATIO),   0.15F, Vector3(1, 1, 1));
		list->Add_Column(TRANSLATE (IDS_MENU_SCORE), 0.20F, Vector3(1, 1, 1));
		}

	list = (ListCtrlClass*)Get_Dlg_Item(IDC_NOD_LIST_CTRL);

	if (list)
		{
		// Configure the columns
		list->Add_Column(TRANSLATE (IDS_MENU_RANK),  0.15F, Vector3(1, 1, 1));

// Denzil 02/24/02 Day 1 patch - Remove clan until we can fix formatting issues.
#if(0)
		list->Add_Column(TRANSLATE (IDS_MENU_NAME),  0.30F, Vector3(1, 1, 1));
		list->Add_Column(TRANSLATE (IDS_MENU_CLAN),  0.20F, Vector3(1, 1, 1));
#else
		list->Add_Column(TRANSLATE (IDS_MENU_NAME),  0.50F, Vector3(1, 1, 1));
#endif

		list->Add_Column(TRANSLATE (IDS_MENU_KD_RATIO),   0.15F, Vector3(1, 1, 1));
		list->Add_Column(TRANSLATE (IDS_MENU_SCORE), 0.20F, Vector3(1, 1, 1));		
		}

	int sidePref = -1;

	if (mWOLGame)
		{
		// If this is not a clan game then we can use the preference.
		if (mGameInfo.IsClanGame() == false)
			{
			const RefPtr<LoginInfo>& login = mWOLSession->GetCurrentLogin();
			WWASSERT(login.IsValid());
		
			LoginProfile* profile = LoginProfile::Get(login->GetNickname());
	
			if (profile)
				{
				sidePref = profile->GetSidePreference();
				profile->Release_Ref();
				}
			}
	
		// The start button is disabled until we hear from the host.
		Enable_Dlg_Item(IDC_STARTGAME, false);

		// Request information about the game.
		RequestWOLGameInfo();
		}
	else
		{
		sidePref = cNetInterface::Get_Side_Preference();

		// Hide and disable the back button for LAN games.
		DialogControlClass* ctrl = Get_Dlg_Item(IDCANCEL);
		WWASSERT(ctrl != NULL);
		ctrl->Show(false);
		ctrl->Enable(false);

		cPlayerManager::Add_Event_Observer(*this);

		PopulateWithLANPlayers();
		}

	InitSideChoice(sidePref);

	//	Activate the menu game mode (if necessary)
	GameModeClass* menuMode = GameModeManager::Find("Menu");

	if (menuMode && !menuMode->Is_Active())
		{
		menuMode->Activate();
		}
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::On_Frame_Update
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMPTeamSelect::On_Frame_Update(void)
	{
	if (mTimeRemaining >= WWMATH_EPSILON)
		{
		mTimeRemaining -= TimeManager::Get_Frame_Real_Seconds();
		ShowTimeRemaining(mTimeRemaining);
		}

	MenuDialogClass::On_Frame_Update();
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::On_Command
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMPTeamSelect::On_Command(int ctrlID, int message, DWORD param)
	{
	switch (ctrlID)
		{
		case IDCANCEL:
		case IDC_STARTGAME:
			{
			int side = GetSideChoice();
			MPChooseTeamSignal chooseSignal((IDC_STARTGAME == ctrlID), side);
			Signaler<MPChooseTeamSignal>::SendSignal(chooseSignal);
			End_Dialog();
			}
			break;

		case IDC_TEAM_AUTO_CHECK:
			SelectSideChoice(PLAYERTYPE_RENEGADE);
			break;

		case IDC_TEAM_GDI_CHECK:
			SelectSideChoice(PLAYERTYPE_GDI);
			break;

		case IDC_TEAM_NOD_CHECK:
			SelectSideChoice(PLAYERTYPE_NOD);
			break;
		}

	MenuDialogClass::On_Command(ctrlID, message, param);
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::On_Last_Menu_Ending
*
* DESCRIPTION
*     Callback from menu class signifying the close of the last menu
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMPTeamSelect::On_Last_Menu_Ending(void)
	{
	// If this the WOL screen then return to the menu...
	if (GameInitMgrClass::Is_WOL_Initialized())
		{
		RenegadeDialogMgrClass::Goto_Location(RenegadeDialogMgrClass::LOC_INTERNET_GAME_LIST);
		}
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::InitSideChoice
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMPTeamSelect::InitSideChoice(int sidePref)
	{
	Enable_Dlg_Item(IDC_TEAM_AUTO_CHECK, mCanChoose);
	Enable_Dlg_Item(IDC_TEAM_GDI_CHECK, mCanChoose);
	Enable_Dlg_Item(IDC_TEAM_NOD_CHECK, mCanChoose);

	// Default side selection combo to the users preference.
	int side = ((mCanChoose == true) ? sidePref : -1);
	SelectSideChoice(side);
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::SelectSideChoice
*
* DESCRIPTION
*
* INPUTS
*     Side - 
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMPTeamSelect::SelectSideChoice(int side)
	{
	Check_Dlg_Button(IDC_TEAM_AUTO_CHECK, (PLAYERTYPE_RENEGADE == side));
	Check_Dlg_Button(IDC_TEAM_GDI_CHECK, (PLAYERTYPE_GDI == side));
	Check_Dlg_Button(IDC_TEAM_NOD_CHECK, (PLAYERTYPE_NOD == side));
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::GetSideChoice
*
* DESCRIPTION
*
* INPUTS
*     Side - 
*
* RESULT
*     NONE
*
******************************************************************************/

int DlgMPTeamSelect::GetSideChoice(void)
	{
	if (Is_Dlg_Button_Checked(IDC_TEAM_GDI_CHECK))
		{
		return PLAYERTYPE_GDI;
		}
	else if (Is_Dlg_Button_Checked(IDC_TEAM_NOD_CHECK))
		{
		return PLAYERTYPE_NOD;
		}

	return PLAYERTYPE_RENEGADE;
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::RequestWOLGameInfo
*
* DESCRIPTION
*     Send a request to the game server for informaiton about the game.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMPTeamSelect::RequestWOLGameInfo(void)
	{
	WWDEBUG_SAY(("DlgMPTeamSelect requesting game info\n"));

	// Get the hosts name and send a request for player information.
	const RefPtr<ChannelData>& channel = mWOLSession->GetCurrentChannel();

	if (channel.IsValid())
		{
		Observer<GameOptionsMessage>::NotifyMe(*mWOLSession);
		mWOLSession->SendPrivateGameOptions(channel->GetName(), "RGINFO");
		}
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::ShowTimeRemaining
*
* DESCRIPTION
*     Set the time remaining text
*
* INPUTS
*     Seconds - Seconds remaining in the game
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMPTeamSelect::ShowTimeRemaining(float remainingSeconds)
	{
	int hours = 0;
	int mins = 0;
	int seconds = 0;

	cMiscUtil::Seconds_To_Hms(remainingSeconds, hours, mins, seconds);

	WideStringClass timeString(0, true);
	timeString.Format(L"%02d:%02d:%02d", hours, mins, seconds);
	
	WideStringClass text(0, true);
	text.Format(L"%s: %s", TRANSLATION(IDS_MP_TIME_REMAINING), (const WCHAR*)timeString);
	Set_Dlg_Item_Text(IDC_TIME_REMAINING_TEXT, (const WCHAR*)text);
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::FindPlayerInListCtrl
*
* DESCRIPTION
*     Find a player entry in the player (GDI or Nod) list control.
*
* INPUTS
*     Name  - Name of player to look for.
*     List  - On return; List control containing player entry.
*     Index - On return; Player entry list index
*
* RESULT
*     True if player found in one of the lists.
*
******************************************************************************/

bool DlgMPTeamSelect::FindPlayerInListCtrl(const WCHAR* name, ListCtrlClass*& outList, int& outIndex)
	{
	// Check in GDI player list
	ListCtrlClass* list = (ListCtrlClass*)Get_Dlg_Item(IDC_GDI_LIST_CTRL);

	if (list)
		{
		int index = list->Find_Entry(COL_NAME, name);

		if (index != -1)
			{
			outList = list;
			outIndex = index;
			return true;
			}
		}

	// Check in Nod player list
	list = (ListCtrlClass*)Get_Dlg_Item(IDC_NOD_LIST_CTRL);

	if (list)
		{
		int index = list->Find_Entry(COL_NAME, name);

		if (index != -1)
			{
			outList = list;
			outIndex = index;
			return true;
			}
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::HandleNotification(ChannelEvent)
*
* DESCRIPTION
*     Handle channel events that occur while the user is deceiding on the team.
*
* INPUTS
*     ChannelEvent - 
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMPTeamSelect::HandleNotification(ChannelEvent& event)
	{
	// This user has been kicked from the game.
	if (ChannelKicked == event.GetStatus())
		{
		DlgMsgBox::DoDialog(TRANSLATE(IDS_MENU_SERVER_MESSAGE_TITLE),
			TRANSLATE(IDS_MENU_SERVER_KICKED_MESSAGE));

		End_Dialog();
		}
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::HandleNotification(UserEvent)
*
* DESCRIPTION
*     Handle user events that occur while the user is making their team choice.
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void DlgMPTeamSelect::HandleNotification(UserEvent& userEvent)
	{
	const RefPtr<UserData>& user = userEvent.Subject();

	if (user.IsValid() == false)
		{
		return;
		}

	// Show clan information for users as it arrives
	switch (userEvent.GetEvent())
		{
#if(0) // Denzil 02/24/02 Day 1 patch - Removed clan until we fix formatting
		case UserEvent::SquadInfo:
			{
			ListCtrlClass* list = NULL;
			int itemIndex = -1;
			bool found = FindPlayerInListCtrl(user->GetName(), list, itemIndex);

			if (found)
				{
				RefPtr<SquadData> clan = user->GetSquad();

				if (clan.IsValid())
					{
					WideStringClass clanAbbr(0, true);
					clanAbbr = clan->GetAbbr();
					list->Set_Entry_Text(itemIndex, COL_CLAN, clanAbbr);
					}
				}
			}
			break;
#endif

#if(0)
		case UserEvent::Join:
			mWOLSession->RequestUserDetails(user, REQUEST_SQUADINFO);
			break;
#endif

		// Remove users that leave the game.
		case UserEvent::Leave:
			{
			ListCtrlClass* list = NULL;
			int itemIndex = -1;
			bool found = FindPlayerInListCtrl(user->GetName(), list, itemIndex);

			if (found)
				{
				list->Delete_Entry(itemIndex);
				}
			}
			break;

		default:
			break;
		}
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::HandleNotification(GameOptionsMessage)
*
* DESCRIPTION
*     Handle game options messages that come in from the server.
*
* INPUTS
*     GameOpts - 
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMPTeamSelect::HandleNotification(GameOptionsMessage& message)
	{
	// Only listen to messages coming for the game host
	const RefPtr<ChannelData>& channel = mWOLSession->GetCurrentChannel();

	if (channel.IsValid() == false)
		{
		return;
		}

	const WideStringClass& hostName = channel->GetName();
	WideStringClass sender(0, true);
	sender = message.GetSendersName();

	if (hostName.Compare_No_Case(sender) == 0)
		{
		static struct {const char* Token; GameOptionsDispatchFunc Dispatch;} _dispatch[] =
			{
			{"GINFO:", ProcessWOLGameInfo},
			{"TINFO:", ProcessWOLTeamInfo},
			{"PINFO:", ProcessWOLPlayerInfo},
			{NULL, NULL}
			};

		int index = 0;
		const char* token = _dispatch[index].Token;
		const char* options = message.GetOptions();

		while (token)
			{
			// Find the first occurance of the token in the message
			const char* cmd = strstr(options, token);

			// If the token was found and it is at the start of the message
			// then invoke the handler for this message.
			if (cmd && cmd == options)
				{
				const char* data = (options + strlen(token));
				_dispatch[index].Dispatch(*this, data);

				// We have heard from the host so it is okay to allow the player to start.
				Enable_Dlg_Item(IDC_STARTGAME, true);
				break;
				}

			++index;
			token = _dispatch[index].Token;
			}
		}
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::ProcessWOLGameInfo
*
* DESCRIPTION
*     Process the game information options packet.
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMPTeamSelect::ProcessWOLGameInfo(DlgMPTeamSelect& dialog, const char* data)
	{
	if (data)
		{
		char info[255];
		strncpy(info, data, 255);

		// Get players name
		unsigned long mapCRC = 0;
		PARSE_HEXDWORD(info, " ", mapCRC);

		StringClass mapname(64, true);
		ModPackageMgrClass::Find_Filename_From_CRC ("*.mix", mapCRC, &mapname);
		
		WideStringClass text(255, true);
		text.Format(TRANSLATE (IDS_MENU_MAP_NAME_FORMAT), (const char*)mapname);
		dialog.Set_Dlg_Item_Text(IDC_MAPNAME_TEXT, text);

		float timeRemaining = 0.0f;
		PARSE_FLOAT(NULL, " ", timeRemaining);
		dialog.mTimeRemaining = timeRemaining;
		dialog.ShowTimeRemaining(timeRemaining);

		int gdiScore = 0;
		PARSE_INT(NULL, " ", gdiScore);
		text.Format(TRANSLATE (IDS_MENU_SCORE_NAME_FORMAT), gdiScore);
		dialog.Set_Dlg_Item_Text(IDC_GDI_SCORE, text);

		int nodScore = 0;
		PARSE_INT(NULL, " ", nodScore);
		text.Format(TRANSLATE (IDS_MENU_SCORE_NAME_FORMAT), nodScore);
		dialog.Set_Dlg_Item_Text(IDC_NOD_SCORE, text);

		// Clear the player lists
		ListCtrlClass* list = (ListCtrlClass*)dialog.Get_Dlg_Item(IDC_GDI_LIST_CTRL);

		if (list)
			{
			list->Delete_All_Entries();
			}

		list = (ListCtrlClass*)dialog.Get_Dlg_Item(IDC_NOD_LIST_CTRL);

		if (list)
			{
			list->Delete_All_Entries();
			}
		}
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::ProcessWOLTeamInfo
*
* DESCRIPTION
*     Process the team information options packet.
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMPTeamSelect::ProcessWOLTeamInfo(DlgMPTeamSelect& dialog, const char* data)
	{
	if (data)
		{
		char info[255];
		strncpy(info, data, 255);

		// Get team GDI information
		int teamID = 0;
		PARSE_INT(info, " ", teamID);

		int score = 0;
		PARSE_INT(NULL, " ", score);

		// Output info
		WideStringClass text(0, true);
		text.Format(TRANSLATE (IDS_MENU_SCORE_NAME_FORMAT), score);

		int ctrlID = ((teamID == PLAYERTYPE_GDI) ? IDC_GDI_SCORE : IDC_NOD_SCORE);
		dialog.Set_Dlg_Item_Text(ctrlID, text);
		}
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::ProcessWOLPlayerInfo
*
* DESCRIPTION
*     Process the per player information packet.
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMPTeamSelect::ProcessWOLPlayerInfo(DlgMPTeamSelect& dialog, const char* data)
	{
	if (data)
		{
		char info[255];
		strncpy(info, data, 255);

		// Get players name
		char* name = "";
		PARSE_STRING(info, " ", name);

		// Team type
		int type = -1;
		PARSE_INT(NULL, " ", type);

		// Rank
		int rung = 0;
		PARSE_INT(NULL, " ", rung);

		// Kills
		int kills = 0;
		PARSE_INT(NULL, " ", kills);

		// Deaths
		int deaths = 0;
		PARSE_INT(NULL, " ", deaths);

		// Score
		int score = 0;
		PARSE_INT(NULL, " ", score);

		int listID = ((type == PLAYERTYPE_GDI) ? IDC_GDI_LIST_CTRL : IDC_NOD_LIST_CTRL);
		ListCtrlClass* list = (ListCtrlClass*)dialog.Get_Dlg_Item(listID);

		if (list)
			{
			WideStringClass playerName(64, true);
			playerName = name;

			int itemIndex = list->Find_Entry(COL_NAME, playerName);
			
			if (itemIndex == -1)
				{
				itemIndex = list->Insert_Entry(list->Get_Entry_Count(), L"");
				}

			if (itemIndex != -1)
				{
				WideStringClass text(255, true);

				// Set the players name and clan affiliation
				list->Set_Entry_Text(itemIndex, COL_NAME, playerName);

#if(0) // Denzil 02/24/02 Day 1 patch - Remove clan until we can fix formating
				RefPtr<UserData> user = dialog.mWOLSession->FindUser(playerName);

				if (user.IsValid())
					{
					RefPtr<SquadData> clan = user->GetSquad();

					if (clan.IsValid())
						{
						text = clan->GetAbbr();
						list->Set_Entry_Text(itemIndex, COL_CLAN, text);
						}
					}
#endif
				
				text.Format(L"%d/%d", kills, deaths);
				list->Set_Entry_Text(itemIndex, COL_KD, text);

				list->Set_Entry_Int(itemIndex, COL_RANK, rung);
				list->Set_Entry_Int(itemIndex, COL_SCORE, score);

				list->Sort(ListSortCallback, 0);
				}
			}
		}
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::HandleNotification(PlayerMgrEvent)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void DlgMPTeamSelect::HandleNotification(PlayerMgrEvent& event)
	{
	PLAYERMGR_ACTION action = event.GetAction();

	if ((action == PLAYER_ACTIVATED))// || (action == PLAYER_ADDED))
		{
		AddLANPlayerInfo(event.Subject());
		}
	else if ((action == PLAYER_DEACTIVATED) || (action == PLAYER_REMOVED))
		{
		RemoveLANPlayerInfo(event.Subject());
		}
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::ProcessWithLANPlayers
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMPTeamSelect::PopulateWithLANPlayers(void)
	{
	SList<cPlayer>* playerList = cPlayerManager::Get_Player_Object_List();
	SLNode<cPlayer>* playerNode = playerList->Head();

	while (playerNode)
		{
		cPlayer* player = playerNode->Data();

		if (player->Get_Is_Active().Is_True())
			{
			AddLANPlayerInfo(player);
			}

		playerNode = playerNode->Next();
		}
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::AddLANPlayerInfo
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void DlgMPTeamSelect::AddLANPlayerInfo(cPlayer* player)
	{
	WWASSERT(player != NULL);
	RemoveLANPlayerInfo(player);
	
	int playerType = player->Get_Player_Type();
	int listID = ((playerType == PLAYERTYPE_GDI) ? IDC_GDI_LIST_CTRL : IDC_NOD_LIST_CTRL);

	ListCtrlClass* list = (ListCtrlClass*)Get_Dlg_Item(listID);
	WWASSERT(list != NULL);

	int itemIndex = list->Insert_Entry(list->Get_Entry_Count(), L"");

	if (itemIndex >= 0)
		{
		list->Set_Entry_Text(itemIndex, COL_NAME, player->Get_Name());
		list->Set_Entry_Int(itemIndex, COL_RANK, player->Get_Rung());
		list->Set_Entry_Int(itemIndex, COL_SCORE, player->Get_Score());

		WideStringClass text(0, true);
		text.Format(L"%d/%d", player->Get_Kills(), player->Get_Deaths());
		list->Set_Entry_Text(itemIndex, COL_KD, text);

		// If this is the player client the mark there name with a star
		const WideStringClass& nickname = cNetInterface::Get_Nickname();

		if (nickname.Compare_No_Case(player->Get_Name()) == 0)
			{
			list->Add_Icon(itemIndex, COL_NAME, "IF_LRGSTAR.TGA");
			list->Set_Entry_Color(itemIndex, COL_RANK, Vector3(1.0F, 1.0F, 1.0F));
			list->Set_Entry_Color(itemIndex, COL_NAME, Vector3(1.0F, 1.0F, 1.0F));
			list->Set_Entry_Color(itemIndex, COL_KD, Vector3(1.0F, 1.0F, 1.0F));
			list->Set_Entry_Color(itemIndex, COL_SCORE, Vector3(1.0F, 1.0F, 1.0F));

			SelectSideChoice(playerType);
			}
		}

	list->Sort(ListSortCallback, 0);
	}


/******************************************************************************
*
* NAME
*     DlgMPTeamSelect::RemoveLANPlayerInfo
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void DlgMPTeamSelect::RemoveLANPlayerInfo(cPlayer* player)
	{
	WWASSERT(player != NULL);

	ListCtrlClass* list = NULL;
	int itemIndex = -1;
	bool found = FindPlayerInListCtrl(player->Get_Name(), list, itemIndex);

	if (found)
		{
		list->Delete_Entry(itemIndex);
		}
	}
