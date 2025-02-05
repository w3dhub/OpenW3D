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
 *                     $Archive:: /Commando/Code/Commando/dlgmpwolgamelist.cpp      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/24/02 4:52p                                              $*
 *                                                                                             *
 *                    $Revision:: 76                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "DlgMPWOLGameList.h"
#include "specialbuilds.h"
#include "cNetwork.h"
#include "BandwidthCheck.h"
#include "UserOptions.h"
#include "GameInitMgr.h"
#include "DlgMPWOLPageBuddy.h"
#include "DlgMessageBox.h"
#include "DlgPasswordPrompt.h"
#include <Combat\PlayerType.h>
#include "WOLJoinGame.h"
#include "WOLLogonMgr.h"
#include <WWOnline\WOLServer.h>
#include <WWOnline\WOLChannel.h>
#include <WWUI\DialogMgr.h>
#include <WWUI\ShortcutBarCtrl.h>
#include <WWUI\ButtonCtrl.h>
#include <WWUI\CheckBoxCtrl.h>
#include <WWUI\ListCtrl.h>
#include "String_IDs.h"
#include <WWTranslateDB\TranslateDB.h>
#include "dlgmplanhostoptions.h"
#include "debug.h"
#include "init.h"

using namespace WWOnline;

// Game List columns
enum
	{
	COL_ICON = 0,
	COL_HOST_NAME,
	COL_GAME_TITLE,
	COL_GAME_MAP,
	COL_PLAYERS,
	COL_PING
	};

#define CLAN_ENTRY_MARKER 0x0C1A0C1A

#define PING_GREEN_THRESHOLD 655
#define PING_YELLOW_THRESHOLD 2621

// Sorting flags for game attributes
#define FLAGSORT_DEDICATED  0x01
#define FLAGSORT_LADDERED   0x02
#define FLAGSORT_PASSWORDED 0x04
#define FLAGSORT_CLAN       0x08

static void SetGameTypeFlags(ListCtrlClass* list, int itemIndex, const WOLGameInfo& gameInfo);
static void SetPingTimeIcon(ListCtrlClass* list, int itemIndex, long pingTime);
static int CALLBACK FlagsSortCallback(ListCtrlClass* list, int item1, int item2, uint32 param);
static int CALLBACK NumericSortCallback(ListCtrlClass* list, int item1, int item2, uint32 param);
static int CALLBACK AlphaSortCallback(ListCtrlClass* list, int index1, int index2, uint32 param);


MPWolGameListMenuClass* MPWolGameListMenuClass::_mInstance = NULL;

/******************************************************************************
*
* NAME
*     MPWolGameListMenuClass::DoDialog
*
* DESCRIPTION
*     Display the Westwood Online advanced game listings dialog.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void MPWolGameListMenuClass::DoDialog(void)
	{
	//	Create the dialog if necessary, otherwise simply bring it to the front
	if (_mInstance == NULL)
		{
		MPWolGameListMenuClass* dialog = new MPWolGameListMenuClass;
		WWASSERT(dialog != NULL && "Failed to create WOL GameList dialog");
		
		if (dialog)
			{
			dialog->Start_Dialog();
			dialog->Release_Ref();
			}
		}
	else
		{
		if (_mInstance->Is_Active_Menu() == false)
			{
			DialogMgrClass::Rollback(_mInstance);
			}
		}
	}


/******************************************************************************
*
* NAME
*     MPWolGameListMenuClass::MPWolGameListMenuClass
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

MPWolGameListMenuClass::MPWolGameListMenuClass(void) :
		MenuDialogClass(IDD_MP_WOL_GAME_LIST),
		mChannelListPending(false),
		mSortColumn(COL_HOST_NAME),
		mIsSortAscending(true),
		mSortFlags(0)
	{
	WWDEBUG_SAY(("MPWolGameListMenuClass Instantiated\n"));
	WWASSERT(_mInstance == NULL);
	_mInstance = this;

	mWOLSession = Session::GetInstance(false);
	WWASSERT_PRINT(mWOLSession.IsValid(), "WOLSession not instantiated!");
	}


/******************************************************************************
*
* NAME
*     MPWolGameListMenuClass::~MPWolGameListMenuClass
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

MPWolGameListMenuClass::~MPWolGameListMenuClass(void)
	{
	WWDEBUG_SAY(("MPWolGameListMenuClass Destroyed\n"));
	_mInstance = NULL;
	}


/******************************************************************************
*
* NAME
*     MPWolGameListMenuClass::On_Init_Dialog
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

void MPWolGameListMenuClass::On_Init_Dialog(void)
	{
	#ifdef FREEDEDICATEDSERVER
	Enable_Dlg_Item(IDC_JOIN_GAME_BUTTON, false);
	#endif // FREEDEDICATEDSERVER

	//---------------------------------------------------------------------------
	// Configure the shortcut bar
	//---------------------------------------------------------------------------
	ShortcutBarCtrlClass* bar = (ShortcutBarCtrlClass*)Get_Dlg_Item(IDC_SHORTCUT_BAR);

	if (bar)
		{
		bar->Add_Button(IDC_MP_SHORTCUT_CHAT, TRANSLATE(IDS_MP_SHORTCUT_CHAT));
		bar->Add_Button(IDC_MP_SHORTCUT_BUDDIES, TRANSLATE(IDS_MP_SHORTCUT_BUDDIES));
		bar->Add_Button(IDC_MP_SHORTCUT_INTERNET_OPTIONS, TRANSLATE(IDS_INTERNET_OPTIONS));
		bar->Add_Button(IDC_MP_SHORTCUT_NEWS, TRANSLATE(IDS_MP_SHORTCUT_NEWS));
		bar->Add_Button(IDC_MP_SHORTCUT_CLANS, TRANSLATE(IDS_MP_SHORTCUT_CLANS));
		bar->Add_Button(IDC_MP_SHORTCUT_RANKINGS, TRANSLATE(IDS_MP_SHORTCUT_RANKINGS));
		bar->Add_Button(IDC_MP_SHORTCUT_NET_STATUS, TRANSLATE(IDS_MP_SHORTCUT_NET_STATUS));
		}

	//---------------------------------------------------------------------------
	// Show the current login and server
	//---------------------------------------------------------------------------
	WideStringClass serverName(64, true);
	WOLLogonMgr::GetServerName(serverName);

	WideStringClass string(128, true);
	string.Format(TRANSLATE (IDS_MENU_CONNECTED_TO_FORMAT), serverName);
	Set_Dlg_Item_Text(IDC_SERVERNAME, string);

	WideStringClass loginName(64, true);
	WOLLogonMgr::GetLoginName(loginName);

	string.Format(TRANSLATE (IDS_MENU_LOGIN_NAME_FORMAT), loginName);
	Set_Dlg_Item_Text(IDC_LOGINNAME, string);

	WideStringClass conn(BandwidthCheckerClass::Get_Bandwidth_As_String(), true);
	string.Format(TRANSLATE (IDS_MENU_SPEED_FORMAT), conn);
	Set_Dlg_Item_Text(IDC_CONNECTIONSPEED, string);

	//---------------------------------------------------------------------------
	// Configure sorting buttons
	//---------------------------------------------------------------------------
	ButtonCtrlClass* button = (ButtonCtrlClass*)Get_Dlg_Item(IDC_SORT_DEDICATED_BUTTON);

	if (button)
		{
		button->Set_Bitmap("mul_spec.tga", "mul_nopts.tga");
		button->Set_Tooltip_Text(TRANSLATE (IDS_MENU_TOOLTIP_SORT_01));
		}

	button = (ButtonCtrlClass*)Get_Dlg_Item(IDC_SORT_PASSWORD_BUTTON);

	if (button)
		{
		button->Set_Bitmap("mul_pswrd.tga", "mul_nopts.tga");
		button->Set_Tooltip_Text(TRANSLATE (IDS_MENU_TOOLTIP_SORT_02));
		}

	button = (ButtonCtrlClass*)Get_Dlg_Item(IDC_SORT_LADDERED_BUTTON);

	if (button)
		{
		button->Set_Bitmap("mul_pts.tga", "mul_nopts.tga");
		button->Set_Tooltip_Text(TRANSLATE (IDS_MENU_TOOLTIP_SORT_03));
		}

	button = (ButtonCtrlClass*)Get_Dlg_Item(IDC_SORT_CLAN_BUTTON);

	if (button)
		{
		button->Set_Bitmap("mul_btlcln.tga", "mul_nopts.tga");
		button->Set_Tooltip_Text(TRANSLATE (IDS_MENU_TOOLTIP_SORT_04));
		}

	//---------------------------------------------------------------------------
	// Configure game listing control
	//---------------------------------------------------------------------------
	ListCtrlClass* list_ctrl = (ListCtrlClass*)Get_Dlg_Item(IDC_GAME_LIST_CTRL);

	if (list_ctrl)
		{
		Vector3 color(1,1,1);
		list_ctrl->Add_Column(TRANSLATE(IDS_MP_GAME_LIST_HEADER_ICON), 0.1F, color);
		list_ctrl->Add_Column(TRANSLATE(IDS_MP_GAME_LIST_HEADER_HOST_NAME), 0.175F, color);
		list_ctrl->Add_Column(TRANSLATE(IDS_MP_GAME_LIST_HEADER_GAME_NAME), 0.325F, color);
		list_ctrl->Add_Column(TRANSLATE(IDS_MP_GAME_LIST_HEADER_GAME_MAP), 0.2F, color);
		list_ctrl->Add_Column(TRANSLATE(IDS_MP_GAME_LIST_HEADER_PLAYERS), 0.1F, color);
		list_ctrl->Add_Column(TRANSLATE(IDS_MP_GAME_LIST_HEADER_PING), 0.1F, color);

		list_ctrl->Allow_NoSelection(true);

		Observer<ChannelListEvent>::NotifyMe(*mWOLSession);
		Observer<ServerError>::NotifyMe(*mWOLSession);
		Observer<SquadEvent>::NotifyMe(*mWOLSession);

		RequestGameList();
		}

	//---------------------------------------------------------------------------
	// Configure game details list control
	//---------------------------------------------------------------------------
	list_ctrl = (ListCtrlClass*)Get_Dlg_Item(IDC_DETAILS_LIST);

	if (list_ctrl)
		{
		list_ctrl->Allow_Selection(false);
		list_ctrl->Set_Tabstop(20.0f);
		}

	MenuDialogClass::On_Init_Dialog();
	}


/******************************************************************************
*
* NAME
*     MPWolGameListMenuClass::On_Command
*
* DESCRIPTION
*     Respond to command messages from UI controls
*
* INPUTS
*     ID    - ID of control originating the message
*     Msg   - Message identifier
*     Param - Message specific parameter
*
* RESULT
*     NONE
*
******************************************************************************/

void MPWolGameListMenuClass::On_Command(int id, int msg, DWORD param)
	{
	switch (id)
		{
		case IDC_PAGE_BUDDY_BUTTON:
			START_DIALOG(MPWolPageBuddyPopupClass);
			break;

		case IDC_REFRESH_GAME_LIST:
			RequestGameList();
			break;

		case IDC_SORT_DEDICATED_BUTTON:
			SortGameChannels(COL_ICON, true, FLAGSORT_DEDICATED);
			break;

		case IDC_SORT_PASSWORD_BUTTON:
			SortGameChannels(COL_ICON, true, FLAGSORT_PASSWORDED);
			break;

		case IDC_SORT_LADDERED_BUTTON:
			SortGameChannels(COL_ICON, true, FLAGSORT_LADDERED);
			break;

		case IDC_SORT_CLAN_BUTTON:
			SortGameChannels(COL_ICON, true, FLAGSORT_CLAN);
			break;

		case IDC_JOIN_GAME_BUTTON:
			Join_Game();
			break;

		case IDC_MENU_MP_LAN_HOST_BUTTON:
			// Return to the game list on exit
			GameInitMgrClass::Set_WOL_Return_Dialog(RenegadeDialogMgrClass::LOC_INTERNET_GAME_LIST);

			//	Create the new game data
			if ( PTheGameData != NULL ) {
				delete PTheGameData;
				PTheGameData = NULL;
			}
			PTheGameData = cGameData::Create_Game_Of_Type(cGameData::GAME_TYPE_CNC);
			WWASSERT(PTheGameData != NULL);

			// WOL games default as quickmatch servers
			The_Game()->Set_QuickMatch_Server(true);

			// Load server preferences.
			The_Game()->Load_From_Server_Config();

			//	Move to the next menu
			DialogBaseClass* dialog = new MPLanHostOptionsMenuClass;
			dialog->Start_Dialog();
			REF_PTR_RELEASE(dialog);
			break;
		}

	MenuDialogClass::On_Command(id, msg, param);
	}


/******************************************************************************
*
* NAME
*     MPWolGameListMenuClass::On_Key_Down
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

bool MPWolGameListMenuClass::On_Key_Down(uint32 key_id, uint32 key_data)
	{
	if (VK_F5 == key_id)
		{
		RequestGameList();
		return true;
		}

	return MenuDialogClass::On_Key_Down(key_id, key_data);
	}


/******************************************************************************
*
* NAME
*     MPWolGameListMenuClass::Join_Game
*
* DESCRIPTION
*     Join the currently selected game
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void MPWolGameListMenuClass::Join_Game(void)
	{
	ListCtrlClass* list = (ListCtrlClass*)Get_Dlg_Item(IDC_GAME_LIST_CTRL);

	if (list)
		{
		// Get the currently selected index
		int index = list->Get_Curr_Sel();

		if (index >= 0)
			{
			RefPtr<ChannelData> channel = (ChannelData*)list->Get_Entry_Data(index, COL_HOST_NAME);

			// If the channel is invalid then do not proceed
			if (!channel.IsValid())
				{
				return;
				}

			WOLGameInfo selectedGame(channel);

			// If the channel data is invalid then do not proceed
			if (!selectedGame.IsDataValid())
				{
				return;
				}

			// If the user cannot join this game then do not proceed.
			if (!selectedGame.CanUserJoin(mWOLSession->GetCurrentUser()))
				{
				return;
				}

			// Return to the game list on exit
			GameInitMgrClass::Set_WOL_Return_Dialog(RenegadeDialogMgrClass::LOC_INTERNET_GAME_LIST);

			// If the game to join is passworded then it is necessary for the user to
			// supply the correct password. Therefore prompt the user to enter the
			// password. We will attempt to connect to the game when we receive a
			// signal from the dialog that the user has entered a password.
			if (channel->IsPassworded())
				{
				mGameToJoin = channel;
				DlgPasswordPrompt::DoDialog(this);
				}
			else
				{
				// No password require
				WOLJoinGame::JoinTheGame(channel->GetName(), L"", true);
				}
			}
		}
	}


/******************************************************************************
*
* NAME
*     MPWolGameListMenuClass::ReceiveSignal
*
* DESCRIPTION
*     Handle receipt of password entered signal from the password prompt dialog.
*
* INPUTS
*     Signal - Signal that the user has entered a password.
*
* RESULT
*     NONE
*
******************************************************************************/

void MPWolGameListMenuClass::ReceiveSignal(DlgPasswordPrompt& passwordDialog)
	{
	if (mGameToJoin.IsValid())
		{
		WOLJoinGame::JoinTheGame(mGameToJoin->GetName(), passwordDialog.GetPassword(), true);
		mGameToJoin.Release();
		}
	}


/******************************************************************************
*
* NAME
*     MPWolGameListMenuClass::RequestGameList
*
* DESCRIPTION
*     Submit a request for an updated game list.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void MPWolGameListMenuClass::RequestGameList(void)
	{
	if (mChannelListPending == false)
		{
		if (mWOLSession->RequestGameChannelList())
			{
			mChannelListPending = true;
			Enable_Dlg_Item(IDC_REFRESH_GAME_LIST, false);

			ListCtrlClass* list = (ListCtrlClass*)Get_Dlg_Item(IDC_DETAILS_LIST);

			if (list)
				{
				list->Delete_All_Entries();
				list->Add_Column(L"", 1.0F, Vector3(1, 1, 1));
				list->Insert_Entry(0, TRANSLATE (IDS_MENU_REQUESTING_NEW_CHANNELS));
				}
			}
		}
	}


/******************************************************************************
*
* NAME
*     MPWolGameListMenuClass::UpdateChannels
*
* DESCRIPTION
*     Update the list of game channels.
*
* INPUTS
*     List     - List control to view channels.
*     Channels - List of channels
*
* RESULT
*     True if the list changed.
*
******************************************************************************/

void MPWolGameListMenuClass::UpdateChannels(ListCtrlClass* list, const ChannelList& chanList)
	{
	// Get current selection so we can reselect it.
	WideStringClass selectedGameName(64, true);
	int selIndex = list->Get_Curr_Sel();

	if (selIndex >= 0)
		{
		selectedGameName = list->Get_Entry_Text(selIndex, COL_HOST_NAME);
		}

	selIndex = -1;

	int listIndex = list->Get_Entry_Count();
	ChannelList::const_iterator chanIter = chanList.begin();

	while (chanIter != chanList.end())
		{
		RefPtr<ChannelData> channel = (*chanIter);

		if (channel.IsValid())
			{
			WOLGameInfo gameInfo(channel);

			// Only show the channel if it is a valid game channel
			if (gameInfo.IsDataValid())
				{
				bool newlyAdded = false;

				// Check if the channel entry is already in the listbox.
				int itemIndex = list->Find_Entry(COL_HOST_NAME, channel->GetName());

				// If the channel was not found then add it now.
				if (itemIndex == -1)
					{
					itemIndex = list->Insert_Entry(listIndex, L"");
					newlyAdded = true;
					}

				// Update the channel in the listbox
				if (itemIndex >= 0)
					{
					// Associate the channel data inside with the entry
					if (newlyAdded)
						{
						ChannelData* rawChannel = channel.ReferencedObject();
						rawChannel->AddReference();
						list->Set_Entry_Data(itemIndex, COL_HOST_NAME, (unsigned long)rawChannel);

						// Show the game channel name
						list->Set_Entry_Text(itemIndex, COL_HOST_NAME, channel->GetName());
						}

					WideStringClass title(0, true);
					title.Convert_From(gameInfo.Title());
					list->Set_Entry_Text(itemIndex, COL_GAME_TITLE, title);

					// Show the game type icons
					SetGameTypeFlags(list, itemIndex, gameInfo);

					// Show the map name
					WideStringClass mapName(64, true);					

					//
					//	If this is a mod'd game, then display the mod_name\map_name...
					//
					if (gameInfo.ModName() != NULL && gameInfo.ModName()[0] != 0) {
						
						//
						//	Strip off the extension for both the map and the mod package
						//
						char map_name[_MAX_FNAME] = { 0 };
						char mod_name[_MAX_FNAME] = { 0 };
						::_splitpath (gameInfo.MapName(), NULL, NULL, map_name, NULL);
						::_splitpath (gameInfo.ModName(), NULL, NULL, mod_name, NULL);

						//
						//	Create the map name from the aggregate of the mod and map
						//
						StringClass ascii_map_name;
						ascii_map_name.Format ("%s\\%s", mod_name, map_name);
						mapName.Convert_From (ascii_map_name);
					} else {
						mapName = gameInfo.MapName();
					}

					list->Set_Entry_Text(itemIndex, COL_GAME_MAP, mapName);

					// Show the number of current / max players
					list->Set_Entry_Data(itemIndex, COL_PLAYERS, MAKELONG(gameInfo.NumPlayers(), gameInfo.MaxPlayers()));

					WideStringClass playersString(64, true);
					playersString.Format(L"%u/%u", gameInfo.NumPlayers(), gameInfo.MaxPlayers());
					list->Set_Entry_Text(itemIndex, COL_PLAYERS, playersString);
					
					// Show the ping time ranking
					SetPingTimeIcon(list, itemIndex, gameInfo.PingTime());

					if ((selIndex == -1) && (selectedGameName.Compare_No_Case(channel->GetName()) == 0))
						{
						selIndex = itemIndex;
						}

					if (gameInfo.Version() != (unsigned long)cNetwork::Get_Exe_Key() || !gameInfo.IsMapValid())
						{
						ChannelData* rawChannel = (ChannelData*)list->Get_Entry_Data(itemIndex, COL_HOST_NAME);

						if (rawChannel)
							{
							rawChannel->ReleaseReference();
							}

						list->Set_Entry_Data(itemIndex, COL_HOST_NAME, 0);
						if (gameInfo.IsMapValid()) {
							list->Set_Entry_Text(itemIndex, COL_GAME_TITLE, TRANSLATE(IDS_MENU_VERSION_MISMATCH));
						}

						/*
						WideStringClass diagnostic;
						diagnostic.Format(L"Version Mismatch (v. %u.%u)",
							HIWORD(gameInfo.Version()), LOWORD(gameInfo.Version()));
						list->Set_Entry_Text(itemIndex, COL_GAME_TITLE, diagnostic);
						*/

						Vector3 grey(0.5F, 0.5F, 0.5F);
						list->Set_Entry_Color(itemIndex, COL_ICON, grey);
						list->Set_Entry_Color(itemIndex, COL_HOST_NAME, grey);
						list->Set_Entry_Color(itemIndex, COL_GAME_TITLE, grey);
						list->Set_Entry_Color(itemIndex, COL_GAME_MAP, grey);
						list->Set_Entry_Color(itemIndex, COL_PLAYERS, grey);
						list->Set_Entry_Color(itemIndex, COL_PING, grey);
						list->Set_Entry_Data(itemIndex, 0, 0);
						}
					}
				}

			++listIndex;
			}

		++chanIter;
		}

	// Select the previously selected entry
	if ((selIndex != -1) && (list->Get_Entry_Count() > 0))
		{
		list->Set_Curr_Sel(selIndex);
		}
	}


/******************************************************************************
*
* NAME
*     MPWolGameListMenuClass::HandleNotification(ChannelListEvent)
*
* DESCRIPTION
*     Process the receipt of a game channel list.
*
* INPUTS
*     Channels - List of channels
*
* RESULT
*     NONE
*
******************************************************************************/

void MPWolGameListMenuClass::HandleNotification(ChannelListEvent& event)
	{
	// Ignore chat channels
	if (event.GetChannelType() == 0)
		{
		return;
		}

	// Update the channel listbox
	ListCtrlClass* list = (ListCtrlClass*)Get_Dlg_Item(IDC_GAME_LIST_CTRL);

	if (list)
		{
		bool sort = false;

		switch (event.GetEvent())
			{
			default:
			case ChannelListEvent::Error:
			case ChannelListEvent::NewList:
				{
				ChannelList& chanList = event.Subject();
				bool listEmpty = chanList.empty();

				ListCtrlClass* details = (ListCtrlClass*)Get_Dlg_Item(IDC_DETAILS_LIST);

				if (details)
					{
					details->Delete_All_Entries();

					if (listEmpty)
						{
						details->Insert_Entry(0, TRANSLATE(IDS_MENU_NO_GAME_SERVERS_FOUND));
						}
					}

				Enable_Dlg_Item(IDC_JOIN_GAME_BUTTON, !listEmpty);
				Enable_Dlg_Item(IDC_REFRESH_GAME_LIST, true);
				mChannelListPending = false;
				}
				break;

			case ChannelListEvent::Update:
				UpdateChannels(list, event.Subject());
				sort = true;
				break;

			case ChannelListEvent::Remove:
				{
				ChannelList& chanList = event.Subject();
				ChannelList::iterator chanIter = chanList.begin();

				while (chanIter != chanList.end())
					{
					RefPtr<ChannelData> channel = (*chanIter);
					int index = list->Find_Entry(COL_HOST_NAME, channel->GetName());

					if (index >= 0)
						{
						list->Delete_Entry(index);
						sort = true;
						}

					++chanIter;
					}
				}
				break;
			}

		if (sort)
			{
			SortGameChannels(mSortColumn, mIsSortAscending, mSortFlags);
			}
		}
	}


/******************************************************************************
*
* NAME
*     MPWolGameListMenuClass::On_ListCtrl_Delete_Entry
*
* DESCRIPTION
*     Handle the deletion of a listbox entry
*
* INPUTS
*     List  - Pointer to list control originating notification.
*     ID    - Identifier of list control originating notification.
*     Index - Index of entry being deleted
*
* RESULT
*     NONE
*
******************************************************************************/

void MPWolGameListMenuClass::On_ListCtrl_Delete_Entry(ListCtrlClass* list, int id, int index)
	{
	if (IDC_GAME_LIST_CTRL == id)
		{
		ChannelData* rawChannel = (ChannelData*)list->Get_Entry_Data(index, COL_HOST_NAME);

		if (rawChannel)
			{
			rawChannel->ReleaseReference();
			}
		}
	}


/******************************************************************************
*
* NAME
*     MPWolGameListMenuClass::On_ListCtrl_Sel_Change
*
* DESCRIPTION
*     Handle user selecting a game entry.
*
* INPUTS
*     List   - Pointer to list control originating notification.
*     ID     - Identifier of list control originating notification.
*     OldSel - Index of previous selection
*     NewSel - Index of new selection
*
* RESULT
*     NONE
*
******************************************************************************/

void MPWolGameListMenuClass::On_ListCtrl_Sel_Change(ListCtrlClass* list, int id, int oldIndex, int newIndex)
	{
	if (IDC_GAME_LIST_CTRL == id)
		{
		Enable_Dlg_Item(IDC_JOIN_GAME_BUTTON, false);

		mSelectedGame.Reset();

		// Clear any detailed game information.
		ListCtrlClass* detailsList = (ListCtrlClass*)Get_Dlg_Item(IDC_DETAILS_LIST);
		detailsList->Delete_All_Entries();

		if (newIndex >= 0)
			{
			RefPtr<ChannelData> channel = (ChannelData*)list->Get_Entry_Data(newIndex, COL_HOST_NAME);

			if (channel.IsValid())
				{
				mSelectedGame.ImportFromChannel(channel);

				if (mSelectedGame.IsDataValid())
					{
					WideStringClass text(255, true);

					// Show host name and exe version
// Denzil 02/14/02 The version encoded in the channel is NOT the printable version.
// Just show the EXE version until we can work this out.
#if(0)
					text.Format(TRANSLATE(IDS_MENU_HOST_INFO_FORMAT), (const WCHAR*)channel->GetName(),
							mSelectedGame.Title(), HIWORD(mSelectedGame.Version()), LOWORD(mSelectedGame.Version()));
#else
					unsigned long verMajor = 0;
					unsigned long verMinor = 0;
					Get_Version_Number(&verMajor,&verMinor);

					text.Format(TRANSLATE(IDS_MENU_HOST_INFO_FORMAT), (const WCHAR*)channel->GetName(),
							mSelectedGame.Title(), HIWORD(verMajor), LOWORD(verMajor));
#endif
					detailsList->Insert_Entry(0, text);

					// If this is a clan game then show the opposing clans
					if (mSelectedGame.IsClanGame()) {
						GetClanVSClanString(mSelectedGame, text);
						int entryIndex = detailsList->Insert_Entry(detailsList->Get_Entry_Count(), text);
						detailsList->Set_Entry_Data(entryIndex, 0, CLAN_ENTRY_MARKER);
					}

					// Show server settings
					text.Format(TRANSLATE(IDS_MENU_HOST_OPTIONS_INFO_FORMAT),
							mSelectedGame.IsDedicated() ? TRANSLATE(IDS_MENU_TEXT463) : TRANSLATE(IDS_MENU_TEXT464),
							mSelectedGame.IsPassworded() ? TRANSLATE(IDS_MENU_TEXT463) : TRANSLATE(IDS_MENU_TEXT464),
							mSelectedGame.IsLaddered() ? TRANSLATE(IDS_MENU_TEXT463) : TRANSLATE(IDS_MENU_TEXT464),
							mSelectedGame.IsQuickmatch() ? TRANSLATE(IDS_MENU_TEXT463) : TRANSLATE(IDS_MENU_TEXT464));

					detailsList->Insert_Entry(detailsList->Get_Entry_Count(), text);

					text.Format(TRANSLATE(IDS_MENU_HOST_OPTIONS_INFO2_FORMAT),
							mSelectedGame.IsClanGame() ? TRANSLATE (IDS_MENU_ON) : TRANSLATE(IDS_MENU_OFF),
							mSelectedGame.IsTeamChange() ? TRANSLATE (IDS_MENU_ON) : TRANSLATE(IDS_MENU_OFF),
							mSelectedGame.IsTeamRemix() ? TRANSLATE (IDS_MENU_ON) : TRANSLATE(IDS_MENU_OFF),
							mSelectedGame.IsFriendlyFire() ? TRANSLATE (IDS_MENU_ON) : TRANSLATE(IDS_MENU_OFF));

					detailsList->Insert_Entry(detailsList->Get_Entry_Count(), text);

					text.Format(TRANSLATE(IDS_MENU_HOST_OPTIONS_INFO3_FORMAT),
							mSelectedGame.IsDriverGunner() ? TRANSLATE(IDS_MENU_ON) : TRANSLATE(IDS_MENU_OFF),
							mSelectedGame.IsSpawnWeapons() ? TRANSLATE(IDS_MENU_ON) : TRANSLATE(IDS_MENU_OFF),
							mSelectedGame.IsRepairBuildings() ? TRANSLATE(IDS_MENU_ON) : TRANSLATE(IDS_MENU_OFF));

					detailsList->Insert_Entry(detailsList->Get_Entry_Count(), text);
					}

				bool canJoin = mSelectedGame.CanUserJoin(mWOLSession->GetCurrentUser());
				Enable_Dlg_Item(IDC_JOIN_GAME_BUTTON, canJoin);
				}
			}
		}
	}


/******************************************************************************
*
* NAME
*     MPWolGameListMenuClass::
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void MPWolGameListMenuClass::GetClanVSClanString(WOLGameInfo& gameInfo, WideStringClass& text)
	{
	const char* clan1Name = "____";
	const char* clan2Name = "____";

	RefPtr<SquadData> clan = SquadData::FindByID(gameInfo.ClanID1());

	if (clan.IsValid())
		{
		clan1Name = clan->GetName();
		}
	else if (gameInfo.ClanID1() != 0)
		{
		clan1Name = "????";
		mWOLSession->RequestSquadInfoByID(gameInfo.ClanID1());
		}

	clan = SquadData::FindByID(gameInfo.ClanID2());

	if (clan.IsValid())
		{
		clan2Name = clan->GetName();
		}
	else if (gameInfo.ClanID2() != 0)
		{
		clan2Name = "????";
		mWOLSession->RequestSquadInfoByID(gameInfo.ClanID2());
		}

	text.Format(L"%S  -VS-  %S", clan1Name, clan2Name);
	}


/******************************************************************************
*
* NAME
*     MPWolGameListMenuClass::On_ListCtrl_Column_Click
*
* DESCRIPTION
*     Respond to user clicking on a column header of a list control.
*
* INPUTS
*     List   - Pointer to list control originating notification.
*     ID     - Identifier of list control originating notification.
*     Column - Number of column clicked.
*
* RESULT
*     NONE
*
******************************************************************************/

void MPWolGameListMenuClass::On_ListCtrl_Column_Click(ListCtrlClass* list, int id, int column)
	{
	if (IDC_GAME_LIST_CTRL == id)
		{
		// If clicking the same column then toggle ascending / descending
		if (mSortColumn == column)
			{
			SortGameChannels(column, !mIsSortAscending, mSortFlags);
			}
		else
			{
			SortGameChannels(column, true, mSortFlags);
			}
		}
	}


/******************************************************************************
*
* NAME
*     MPWolGameListMenuClass::On_ListCtrl_DblClk
*
* DESCRIPTION
*     Automatically attempt to join a game if the user double-clicks on the
*     game entry.
*
* INPUTS
*     List  - List control
*     ID    - Identifier of list control
*     Index - Entry index double-clicked
*
* RESULT
*     NONE
*
******************************************************************************/

void MPWolGameListMenuClass::On_ListCtrl_DblClk(ListCtrlClass* list, int id, int index)
	{
	if (IDC_GAME_LIST_CTRL == id)
		{
		Join_Game();
		}
	}


/******************************************************************************
*
* NAME
*     On_Last_Menu_Ending
*
* DESCRIPTION
*     Callback from menu class signifying the close of the last menu
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void MPWolGameListMenuClass::On_Last_Menu_Ending (void)
	{
	RenegadeDialogMgrClass::Goto_Location(RenegadeDialogMgrClass::LOC_INTERNET_MAIN);
	}


/******************************************************************************
*
* NAME
*     MPWolGameListMenuClass::HandleNotification(ServerError)
*
* DESCRIPTION
*     Handle server error notifications from WWOnline session.
*
* INPUTS
*     Error - Server error
*
* RESULT
*     NONE
*
******************************************************************************/

void MPWolGameListMenuClass::HandleNotification(ServerError& error)
	{
	if (CHAT_E_BANNED == error.GetErrorCode())
		{
//		DlgMsgBox::DoDialog(TRANSLATE(IDS_WOL_SERVERMESSAGE), TRANSLATE(IDS_WOL_BANNED));
		DlgMsgBox::DoDialog(TRANSLATE (IDS_MENU_SERVER_MESSAGE_TITLE), TRANSLATE (IDS_MENU_WOL_BAN_USER_MESSAGE));
		End_Dialog();
		}
	}


/******************************************************************************
*
* NAME
*     MPWolGameListMenuClass::HandleNotification(SquadEvent)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void MPWolGameListMenuClass::HandleNotification(WWOnline::SquadEvent& event)
	{
	if (!mSelectedGame.IsDataValid() && mSelectedGame.IsClanGame())
		{
		return;
		}

	RefPtr<SquadData> squad = event.GetSquadData();

	if (squad.IsValid())
		{
		unsigned long squadID = squad->GetID();

		if (mSelectedGame.ClanID1() == squadID || mSelectedGame.ClanID2() == squadID)
			{
			ListCtrlClass* detailsList = (ListCtrlClass*)Get_Dlg_Item(IDC_DETAILS_LIST);

			if (detailsList)
				{
				// Find entry with marker indicating clan vs clan
				const int count = detailsList->Get_Entry_Count();
		
				for (int index = 0; index < count; ++index)
					{
					int marker = detailsList->Get_Entry_Data(index, 0);

					if (CLAN_ENTRY_MARKER == marker)
						{
						WideStringClass text(0, true);
						GetClanVSClanString(mSelectedGame, text);
						detailsList->Set_Entry_Text(index, 0, text);
						break;
						}
					}
				}
			}
		}
	}


/******************************************************************************
*
* NAME
*     SetGameTypeFlags
*
* DESCRIPTION
*     Setup the game type flags in the icon column of the game listings.
*
* INPUTS
*     List     - Pointer to list control to work with.
*     Index    - Entry index to set type flags for.
*     GameInfo - Game information
*
* RESULT
*
******************************************************************************/

void SetGameTypeFlags(ListCtrlClass* list, int itemIndex, const WOLGameInfo& gameInfo)
	{
	WWASSERT(list != NULL);

	list->Reset_Icons(itemIndex, COL_ICON);
	uint32 flags = 0;

	if (gameInfo.IsLaddered())
		{
		list->Add_Icon(itemIndex, COL_ICON, "MUL_pts.tga");
		flags |= FLAGSORT_LADDERED;
		}
	else
		{
		list->Add_Icon(itemIndex, COL_ICON, "MUL_NoPts.tga");
		}

	if (gameInfo.IsDedicated())
		{
		list->Add_Icon(itemIndex, COL_ICON, "MUL_Spec.tga");
		flags |= FLAGSORT_DEDICATED;
		}

	if (gameInfo.IsPassworded())
		{
		list->Add_Icon(itemIndex, COL_ICON, "MUL_Pswrd.tga");
		flags |= FLAGSORT_PASSWORDED;
		}

	if (gameInfo.IsClanGame())
		{
		list->Add_Icon(itemIndex, COL_ICON, "MUL_Btlcln.tga");
		flags |= FLAGSORT_CLAN;
		}

	list->Set_Entry_Data(itemIndex, COL_ICON, flags);
	}


/******************************************************************************
*
* NAME
*     SetPingTimeIcon
*
* DESCRIPTION
*     Set an icon reflecting the latency for this game.
*
* INPUTS
*     List  - List control
*     Index - Entry index
*     Time  - Latency
*
* RESULT
*     NONE
*
******************************************************************************/

void SetPingTimeIcon(ListCtrlClass* list, int itemIndex, long pingTime)
	{
	const char* pingIcon = NULL;

	if (pingTime <= PING_GREEN_THRESHOLD)
		{
		pingIcon = "mul_statg.tga";
		}
	else if (pingTime <= PING_YELLOW_THRESHOLD)
		{
		pingIcon = "mul_staty.tga";
		}
	else
		{
		pingIcon = "mul_statr.tga";
		}

	list->Reset_Icons(itemIndex, COL_PING);
	list->Add_Icon(itemIndex, COL_PING, pingIcon);

	unsigned long displayPing = (unsigned long)((1000.0 / 256.0) * (sqrt(double(pingTime))));
	list->Set_Entry_Data(itemIndex, COL_PING, displayPing);

	WideStringClass text(32, true);
	text.Format(L" (%lu)", displayPing);
	list->Set_Entry_Text(itemIndex, COL_PING, text);
	}


/******************************************************************************
*
* NAME
*     MPWolGameListMenuClass::SortGameChannels
*
* DESCRIPTION
*     Sort the game channels
*
* INPUTS
*     Column    - Number of column to sort by.
*     Ascending - True to sort ascending; false to sort descending
*     Flags     - Game flags to sort by if column is COL_ICON
*
* RESULT
*     NONE
*
******************************************************************************/

void MPWolGameListMenuClass::SortGameChannels(int column, bool isAscending, unsigned long param)
	{
	mSortColumn = column;
	mIsSortAscending = isAscending;
	mSortFlags = param;

	ListCtrlClass* list = (ListCtrlClass*)Get_Dlg_Item(IDC_GAME_LIST_CTRL);

	if (list)
		{
		ListCtrlClass::SORT_TYPE sortDirection = ListCtrlClass::SORT_DESCENDING;

		if (isAscending)
			{
			sortDirection = ListCtrlClass::SORT_ASCENDING;
			}

		if (COL_ICON == column)
			{
			list->Sort(FlagsSortCallback, mSortFlags);
			}
		else if (COL_PLAYERS == column || COL_PING == column)
			{
			list->Sort(NumericSortCallback, MAKELONG(column, sortDirection));
			}
		else
			{
			list->Sort(AlphaSortCallback, MAKELONG(column, sortDirection));
			}

		list->Set_Sort_Designator(column, sortDirection);
		}
	}


/******************************************************************************
*
* NAME
*     FlagSortCallback
*
* DESCRIPTION
*     Sort list entries by game type flags.
*
* INPUTS
*     List   - List control performing sort.
*     Index1 - First entry to compare
*     Index2 - Second entry to compare
*
* RESULT
*     0  - Items are equal
*     -1 - Item1 is less than item2
*     1  - Item1 is greater than item2
*
******************************************************************************/

int CALLBACK FlagsSortCallback(ListCtrlClass* list, int index1, int index2, uint32 mask)
	{
	uint32 flags1 = list->Get_Entry_Data(index1, COL_ICON);
	flags1 &= mask;

	uint32 flags2 = list->Get_Entry_Data(index2, COL_ICON);
	flags2 &= mask;

	if (flags1 && !flags2)
		{
		return -1;
		}
	else if (!flags1 && flags2)
		{
		return 1;
		}
	else if (flags1 && flags2)
		{
		// Secondary sort by ping time
		int ping1 = list->Get_Entry_Data(index1, COL_PING);
		int ping2 = list->Get_Entry_Data(index2, COL_PING);

		int delta = (ping1 - ping2);

		if (delta < 0)
			{
			return -1;
			}
		else if (delta > 0)
			{
			return 1;
			}

		const WCHAR* name1 = list->Get_Entry_Text(index1, COL_HOST_NAME);
		const WCHAR* name2 = list->Get_Entry_Text(index2, COL_HOST_NAME);
		return wcsicmp(name1, name2);
		}

	return 0;
	}


/******************************************************************************
*
* NAME
*     NumericSortCallback
*
* DESCRIPTION
*     Sort list entries numerically from the numbers contained in the entries
*     data field.
*
* INPUTS
*     List   - List control performing sort.
*     Index1 - First entry to compare
*     Index2 - Second entry to compare
*
* RESULT
*    -1 - Item1 is less than item2
*     0 - Items are equal
*     1 - Item1 is greater than item2
*
******************************************************************************/

int CALLBACK NumericSortCallback(ListCtrlClass* list, int index1, int index2, uint32 param)
	{
	// Sort by numeric value stored in entry data field
	int	column = LOWORD(param);

	uint32 data1 = list->Get_Entry_Data(index1, column);
	uint32 data2 = list->Get_Entry_Data(index2, column);

	int retval = (data1 - data2);

	if (retval < 0)
		{
		retval = -1;
		}
	else if (retval > 0)
		{
		retval = 1;
		}
	else
		{
		if (column != COL_PING)
			{
			// Secondary sort by ping time
			data1 = list->Get_Entry_Data(index1, COL_PING);
			data2 = list->Get_Entry_Data(index2, COL_PING);

			retval = (data1 - data2);

			if (retval < 0)
				{
				retval = -1;
				}
			else if (retval > 0)
				{
				retval = 1;
				}
			}

		if (retval == 0)
			{
			const WCHAR* name1 = list->Get_Entry_Text(index1, COL_HOST_NAME);
			const WCHAR* name2 = list->Get_Entry_Text(index2, COL_HOST_NAME);
			retval = wcsicmp(name1, name2);
			}
		}

	// Invert the return value if we are sorting descendingly
	ListCtrlClass::SORT_TYPE sortType = (ListCtrlClass::SORT_TYPE)HIWORD(param);

	if (ListCtrlClass::SORT_DESCENDING == sortType)
		{
		return -retval;
		}

	return retval;
	}


/******************************************************************************
*
* NAME
*     NumericSortCallback
*
* DESCRIPTION
*     Sort list entries alphbetically then by ping time.
*
* INPUTS
*     List   - List control performing sort.
*     Index1 - First entry to compare
*     Index2 - Second entry to compare
*
* RESULT
*    -1 - Item1 is less than item2
*     0 - Items are equal
*     1 - Item1 is greater than item2
*
******************************************************************************/

int CALLBACK AlphaSortCallback(ListCtrlClass* list, int index1, int index2, uint32 param)
	{
	// Sort by numeric value stored in entry data field
	int	column = LOWORD(param);

	const WCHAR* text1 = list->Get_Entry_Text(index1, column);
	const WCHAR* text2 = list->Get_Entry_Text(index2, column);
	int retval = wcsicmp(text1, text2);

	// If the strings match then secondary sort by ping time.
	if (retval == 0)
		{
		uint32 data1 = list->Get_Entry_Data(index1, COL_PING);
		uint32 data2 = list->Get_Entry_Data(index2, COL_PING);

		retval = (data1 - data2);

		if (retval < 0)
			{
			retval = -1;
			}
		else if (retval > 0)
			{
			retval = 1;
			}
		}

	// Invert the return value if we are sorting descendingly
	ListCtrlClass::SORT_TYPE sortType = (ListCtrlClass::SORT_TYPE)HIWORD(param);

	if (ListCtrlClass::SORT_DESCENDING == sortType)
		{
		return -retval;
		}

	return retval;
	}
