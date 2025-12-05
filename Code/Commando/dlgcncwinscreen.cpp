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
 *                     $Archive:: /Commando/Code/Commando/dlgcncwinscreen.cpp                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/24/02 2:10p                                                  $*
 *                                                                                             *
 *                    $Revision:: 16                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgcncwinscreen.h"
#include "rendobj.h"
#include "pscene.h"
#include "combat.h"
#include "gamedata.h"
#include "playertype.h"
#include "assetmgr.h"
#include "renegadedialog.h"
#include "menubackdrop.h"
#include "listctrl.h"
#include "gamemode.h"
#include "wolgmode.h"
#include "translatedb.h"
#include "string_ids.h"
#include "cnetwork.h"
#include "gameinitmgr.h"
#include "gamespyadmin.h"
#include "specialbuilds.h"
#include "dialogtests.h"
#include "dialogmgr.h"


////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
CNCWinScreenMenuClass *	CNCWinScreenMenuClass::_TheInstance		= NULL;
const float					CNCWinScreenMenuClass::UpdateIntervalS	= 0.2f;


////////////////////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////////////////////
enum
{
	COL_RANK,
	COL_NAME,
	COL_SCORE,
	COL_KILLS,
	COL_LADDER,
};


////////////////////////////////////////////////////////////////
//
//	CNCWinScreenMenuClass
//
////////////////////////////////////////////////////////////////
CNCWinScreenMenuClass::CNCWinScreenMenuClass (void)	:
	HeaderModel (NULL),
	UpdateTimer (UpdateIntervalS),
	ShowLadderPoints(false),
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_CNC_WINSCREEN))
{
	//
	//	Configure the blackout renderer
	//
	StyleMgrClass::Configure_Renderer (&BlackoutRenderer);
	BlackoutRenderer.Get_Shader ()->Set_Depth_Compare (ShaderClass::PASS_ALWAYS);
	BlackoutRenderer.Add_Quad (Render2DClass::Get_Screen_Resolution (), RGBA_TO_INT32 (0, 0, 0, 200));
	WinScreenBackdrop.Clear_Screen (false);

	//
	//	Load the correct banner
	//
	if (The_Game ()->Get_Winner_ID () == PLAYERTYPE_GDI) {
		HeaderModel = WW3DAssetManager::Get_Instance ()->Create_Render_Obj ("HUD_GDIWINBAN");
	} else {
		HeaderModel = WW3DAssetManager::Get_Instance ()->Create_Render_Obj ("HUD_NODWINBAN");
	}

	//
	// Should we show ladder points?
	//
	WWASSERT(GameModeManager::Find("WOL") != NULL);
	WWASSERT(The_Game() != NULL);
	if (GameModeManager::Find("WOL")->Is_Active() && The_Game()->IsLaddered.Is_True()) {
		ShowLadderPoints = true;
	}

	_TheInstance = this;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	CNCWinScreenMenuClass
//
////////////////////////////////////////////////////////////////
CNCWinScreenMenuClass::~CNCWinScreenMenuClass (void)
{
	if (HeaderModel != NULL) {
		HeaderModel->Remove ();
		REF_PTR_RELEASE (HeaderModel);
	}

	_TheInstance = NULL;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
CNCWinScreenMenuClass::On_Init_Dialog (void)
{
	//
	//	Get a pointer to the list control
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_WIN1_LIST_CTRL);
	if (list_ctrl != NULL) {

		//
		//	Configure the columns
		//
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_RANK),		0.15F,	Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_NAME),		0.3F,		Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_SCORE),		0.2F,		Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_KILLS),		0.15F,	Vector3 (1, 1, 1));
		if (ShowLadderPoints) {
			list_ctrl->Add_Column (TRANSLATE (IDS_MENU_LADDER),	0.25F,	Vector3 (1, 1, 1));
		}
	}

	list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LOSE1_LIST_CTRL);
	if (list_ctrl != NULL) {

		//
		//	Configure the columns
		//
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_RANK),		0.15F,	Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_NAME),		0.3F,		Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_SCORE),		0.2F,		Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_KILLS),		0.15F,	Vector3 (1, 1, 1));
		if (ShowLadderPoints) {
			list_ctrl->Add_Column (TRANSLATE (IDS_MENU_LADDER),	0.25F,	Vector3 (1, 1, 1));
		}
	}

	//
	//	Fill in dialog controls
	//
	if (The_Game ()->Get_Winner_ID () == PLAYERTYPE_GDI) {
		Get_Dlg_Item (IDC_TITLE_TEXT_1)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
		Get_Dlg_Item (IDC_TITLE_TEXT_2)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
		Populate_Player_Lists (PLAYERTYPE_GDI, IDC_WIN1_LIST_CTRL);
		Populate_Player_Lists (PLAYERTYPE_NOD, IDC_LOSE1_LIST_CTRL);

		//
		//	Display the title
		//
		Set_Dlg_Item_Text (IDC_TITLE_TEXT_1, TRANSLATE (IDS_MENU_GDI_WINS));

		//
		//	Display the team scores
		//
		WideStringClass team_score_string;
		team_score_string.Format (TRANSLATE (IDS_MENU_GDI_SCORE), (int)cTeamManager::Find_Team (PLAYERTYPE_GDI)->Get_Score ());
		Set_Dlg_Item_Text (IDC_WINNER_TEAM_SCORE_TEXT, team_score_string);
		team_score_string.Format (TRANSLATE (IDS_MENU_NOD_SCORE), (int)cTeamManager::Find_Team (PLAYERTYPE_NOD)->Get_Score ());
		Set_Dlg_Item_Text (IDC_LOSER_TEAM_SCORE_TEXT, team_score_string);

	} else {
		Get_Dlg_Item (IDC_TITLE_TEXT_1)->Set_Text_Color (Vector3 (1.0F, 0.0F, 0.0F));
		Get_Dlg_Item (IDC_TITLE_TEXT_2)->Set_Text_Color (Vector3 (1.0F, 0.0F, 0.0F));
		//Get_Dlg_Item (IDC_MVP_TEXT)->Set_Text_Color (Vector3 (1.0F, 0.0F, 0.0F));
		//Get_Dlg_Item (IDC_TIME_TEXT)->Set_Text_Color (Vector3 (1.0F, 0.0F, 0.0F));
		//Get_Dlg_Item (IDC_WINNER_TEAM_SCORE_TEXT)->Set_Text_Color (Vector3 (1.0F, 0.0F, 0.0F));
		//Get_Dlg_Item (IDC_LOSER_TEAM_SCORE_TEXT)->Set_Text_Color (Vector3 (1.0F, 0.0F, 0.0F));
		Populate_Player_Lists (PLAYERTYPE_NOD, IDC_WIN1_LIST_CTRL);
		Populate_Player_Lists (PLAYERTYPE_GDI, IDC_LOSE1_LIST_CTRL);

		//
		//	Display the title
		//
		Set_Dlg_Item_Text (IDC_TITLE_TEXT_1, TRANSLATE (IDS_MENU_NOD_WINS));

		//
		//	Display the team scores
		//
		WideStringClass team_score_string;
		team_score_string.Format (TRANSLATE (IDS_MENU_NOD_SCORE), (int)cTeamManager::Find_Team (PLAYERTYPE_NOD)->Get_Score ());
		Set_Dlg_Item_Text (IDC_WINNER_TEAM_SCORE_TEXT, team_score_string);
		team_score_string.Format (TRANSLATE (IDS_MENU_GDI_SCORE), (int)cTeamManager::Find_Team (PLAYERTYPE_GDI)->Get_Score ());
		Set_Dlg_Item_Text (IDC_LOSER_TEAM_SCORE_TEXT, team_score_string);
	}

	//
	//	Display the MVP's name and num times consecutive as MVP
	//
	WideStringClass mvp_text;
	mvp_text.Format (TRANSLATE (IDS_MENU_MVP_FORMAT), (const wchar_t *)The_Game ()->Get_Mvp_Name ());
	if (!The_Game()->Get_Mvp_Name().Is_Empty() && The_Game()->Get_Mvp_Count() > 1) {
		WideStringClass consecutives_text;
		consecutives_text.Format(L" * %d", The_Game()->Get_Mvp_Count());
		mvp_text += consecutives_text;
	}
	Set_Dlg_Item_Text (IDC_MVP_TEXT, mvp_text);

	//
	//	Display a description of how the team won
	//
	cGameData::WinTypeEnum win_type = The_Game ()->Get_Win_Type ();
	WideStringClass win_description;
	switch (win_type)
	{
		case cGameData::WIN_TYPE_FORCED:
			win_description = TRANSLATE (IDS_MENU_WIN_CONDITION_01);
			break;

		case cGameData::WIN_TYPE_COWARDICE:
			win_description = TRANSLATE (IDS_MENU_WIN_CONDITION_02);
			break;

		case cGameData::WIN_TYPE_TIME:
			win_description = TRANSLATE (IDS_MENU_WIN_CONDITION_03);
			break;

		case cGameData::WIN_TYPE_BASE_DESTRUCTION:
			win_description = TRANSLATE (IDS_MENU_WIN_CONDITION_04);
			break;

		case cGameData::WIN_TYPE_BEACON:
			win_description = TRANSLATE (IDS_MENU_WIN_CONDITION_05);
			break;
	}

	Set_Dlg_Item_Text (IDC_TITLE_TEXT_2, win_description);

	WideStringClass game_time_string;
	Set_Dlg_Item_Text (IDC_TITLE_TEXT_2, win_description);

	//
	//	Display the game time
	//
	WideStringClass time_string;
	int time			= (int)The_Game ()->Get_Game_Duration_S ();
	int hours		= int(time / 3600);
	time				-= hours * 3600;
	int minutes		= int(time / 60);
	time				-= minutes * 60;
	int seconds		= (int)time;

	if (hours > 0) {
		time_string.Format (TRANSLATE (IDS_MENU_GAME_TIME_01), hours, minutes, seconds);
	} else {
		time_string.Format (TRANSLATE (IDS_MENU_GAME_TIME_02), minutes, seconds);
	}

	Set_Dlg_Item_Text (IDC_TIME_TEXT, time_string);


	//
	// Set the text that shows the next map to be played.
	//
	if (The_Game()->Is_Map_Cycle_Over()) {
		Set_Dlg_Item_Text (IDC_MENU_TEXT_NEXT_MAP, L"");
	} else {
		WideStringClass map_info(TRANSLATE(IDS_MENU_NEXT_MAP), true);

		//
		// Get a copy of the map name from PTheGameData
		//
		char map_name[256];
		strcpy(map_name, The_Game()->Get_Map_Name());

		//
		// Strip off the .mix if present.
		//
        openw3d::string_to_upper(map_name);
		char *dot = strstr(map_name, ".MIX");
		strcpy(map_name, The_Game()->Get_Map_Name());
		if (dot) {
			*dot = 0;
		}
		map_info += WideStringClass(map_name, true);
		Set_Dlg_Item_Text (IDC_MENU_TEXT_NEXT_MAP, map_info);
	}

	MenuDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
CNCWinScreenMenuClass::On_Frame_Update (void)
{
	UpdateTimer -= TimeManager::Get_Frame_Seconds ();
	if (UpdateTimer <= 0) {

		cPlayerManager::Sort_Players(false);

		//
		//	Refresh the player lists
		//
		if (The_Game ()->Get_Winner_ID () == PLAYERTYPE_GDI) {
			Populate_Player_Lists (PLAYERTYPE_GDI, IDC_WIN1_LIST_CTRL);
			Populate_Player_Lists (PLAYERTYPE_NOD, IDC_LOSE1_LIST_CTRL);
		} else {
			Populate_Player_Lists (PLAYERTYPE_NOD, IDC_WIN1_LIST_CTRL);
			Populate_Player_Lists (PLAYERTYPE_GDI, IDC_LOSE1_LIST_CTRL);
		}

		//
		//	Refresh the team scores
		//
		WideStringClass gdi_score_string;
		WideStringClass nod_score_string;
		gdi_score_string.Format (TRANSLATE (IDS_MENU_GDI_SCORE), (int)cTeamManager::Find_Team (PLAYERTYPE_GDI)->Get_Score ());
		nod_score_string.Format (TRANSLATE (IDS_MENU_NOD_SCORE), (int)cTeamManager::Find_Team (PLAYERTYPE_NOD)->Get_Score ());
		if (The_Game ()->Get_Winner_ID () == PLAYERTYPE_GDI) {
			Set_Dlg_Item_Text (IDC_WINNER_TEAM_SCORE_TEXT, gdi_score_string);
			Set_Dlg_Item_Text (IDC_LOSER_TEAM_SCORE_TEXT, nod_score_string);
		} else {
			Set_Dlg_Item_Text (IDC_WINNER_TEAM_SCORE_TEXT, nod_score_string);
			Set_Dlg_Item_Text (IDC_LOSER_TEAM_SCORE_TEXT, gdi_score_string);
		}

		UpdateTimer = UpdateIntervalS;
	}

	MenuDialogClass::On_Frame_Update ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Destroy
//
////////////////////////////////////////////////////////////////
void
CNCWinScreenMenuClass::On_Destroy (void)
{
	MenuDialogClass::On_Destroy ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
CNCWinScreenMenuClass::Render (void)
{
	//
	//	Don't render if we aren't the active menu
	//
	if (ActiveMenu == this) {

		//
		//	Render the background scene first
		//
		BackDrop->Render ();
		BlackoutRenderer.Render ();
		WinScreenBackdrop.Render ();

		//
		//	Now, let the dialog subsystem render the controls and
		// such...
		//
		DialogBaseClass::Render ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
CNCWinScreenMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDCANCEL:

			//
			//	Close the dialog
			//
			End_Dialog ();

			//
			//	End the game
			//
			GameInitMgrClass::End_Game ();

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
			break;

		default:
			MenuDialogClass::On_Command (ctrl_id, message_id, param);
			break;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Menu_Activate
//
////////////////////////////////////////////////////////////////
void
CNCWinScreenMenuClass::On_Menu_Activate (bool onoff)
{
	if (HeaderModel != NULL) {

		//
		//	Either add or remove the header from the scene
		//
		if (onoff) {

			if (HeaderModel->Peek_Scene () == NULL) {
				WinScreenBackdrop.Peek_Scene ()->Add_Render_Object (HeaderModel);
			}

		} else {
			HeaderModel->Remove ();
		}
	}

	MenuDialogClass::On_Menu_Activate (onoff);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Close_Dialog
//
////////////////////////////////////////////////////////////////
void
CNCWinScreenMenuClass::Close_Dialog (void)
{
	if (_TheInstance != NULL) {
		_TheInstance->End_Dialog ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Populate_Player_Lists
//
////////////////////////////////////////////////////////////////
void
CNCWinScreenMenuClass::Populate_Player_Lists (int team_id, int list_ctrl1_id)
{
	//
	//	Get a pointer to the list control
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (list_ctrl1_id);
	if (list_ctrl == NULL) {
		return ;
	}

	DynamicVectorClass<cPlayer *> player_list;

	//
	//	Find the player's name that most closely matches the typed name
	//
	int index = 0;
	for (	SLNode<cPlayer> *player_node = cPlayerManager::Get_Player_Object_List ()->Head ();
			player_node != NULL;
			player_node = player_node->Next ())
	{
		cPlayer *player = player_node->Data ();
		WWASSERT (player != NULL);

		if (player->Get_Is_Active().Is_False ()) {
			continue;
		}

		//
		//	Does this player belong to the same team?
		//
		if (player->Get_Player_Type () == team_id) {
			player_list.Add (player);
		}
	}

	//
	//	Sort the players
	//
	if (player_list.Count () > 0) {
		::qsort (&player_list[0], player_list.Count (), sizeof (cPlayer *), ListSortCallback);
	}

	//
	//	Start fresh
	//
	list_ctrl->Delete_All_Entries ();

	//
	//	Add the players to the list control
	//
	for (index = 0; index < player_list.Count (); index ++) {
		cPlayer *player = player_list[index];

		//
		//	Make a new entry for this player
		//
		int item_index = list_ctrl->Insert_Entry (index, L"");
		if (item_index >= 0) {

			//
			//	Fill in the player's name
			//
			WideStringClass displayName(0, true);
			Build_Player_Display_Name(player, displayName);
			list_ctrl->Set_Entry_Text (item_index, COL_NAME,		(const wchar_t*)displayName);

			//
			//	Fill in information about the player
			//
			list_ctrl->Set_Entry_Int (item_index, COL_RANK,			player->Get_Rung ());
			list_ctrl->Set_Entry_Int (item_index, COL_SCORE,		player->Get_Score ());
			list_ctrl->Set_Entry_Int (item_index, COL_KILLS,		player->Get_Kills ());
			if (ShowLadderPoints) {
				list_ctrl->Set_Entry_Int (item_index, COL_LADDER,		player->Get_Ladder_Points ());
			}

			//
			//	Put a star by the player's name if this is the local player
			//
			SmartGameObj *game_obj = player->Get_GameObj ();
			if (cNetwork::I_Am_Client() && (game_obj == COMBAT_STAR)) {
				list_ctrl->Add_Icon (item_index, COL_NAME, "IF_LRGSTAR.TGA");
				list_ctrl->Set_Entry_Color (item_index, COL_RANK,		Vector3 (1.0F, 1.0F, 1.0F));
				list_ctrl->Set_Entry_Color (item_index, COL_NAME,		Vector3 (1.0F, 1.0F, 1.0F));
				list_ctrl->Set_Entry_Color (item_index, COL_SCORE,		Vector3 (1.0F, 1.0F, 1.0F));
				list_ctrl->Set_Entry_Color (item_index, COL_KILLS,		Vector3 (1.0F, 1.0F, 1.0F));
				if (ShowLadderPoints) {
					list_ctrl->Set_Entry_Color (item_index, COL_LADDER,	Vector3 (1.0F, 1.0F, 1.0F));
				}
			}

			//
			//	Set the ranking as this entries user data so we can sort by it...
			//
			list_ctrl->Set_Entry_Data (item_index, COL_RANK, player->Get_Rung ());
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	ListSortCallbackk
//
////////////////////////////////////////////////////////////////
int __cdecl
CNCWinScreenMenuClass::ListSortCallback
(
	const void *elem1,
	const void *elem2
)
{
   WWASSERT (elem1 != NULL);
   WWASSERT (elem2 != NULL);
   cPlayer *player1 = *((cPlayer **)elem1);
   cPlayer *player2 = *((cPlayer **)elem2);

	//
	//	Sort based on rank
	//
	int rank1 = (int)player1->Get_Rung ();
	int rank2 = (int)player2->Get_Rung ();
	return (rank1 - rank2);
}


////////////////////////////////////////////////////////////////
//
//	Build_Player_Display_Name
//
////////////////////////////////////////////////////////////////
void
CNCWinScreenMenuClass::Build_Player_Display_Name (const cPlayer *player, WideStringClass &outName)
{
// Denzil 02/24/02 Day 1 Patch - Do not show clan abbreviation for now because
// it does not always fit in the space alloted. For now it is better not to
// show it than to have it chopped off.
#if(0)
	GameModeClass* gameMode = GameModeManager::Find("WOL");

	if (gameMode && gameMode->Is_Active()) {
		WolGameModeClass* wolGame = static_cast<WolGameModeClass*>(gameMode);
		WWASSERT(wolGame);

		RefPtr<WWOnline::UserData> user = wolGame->Get_WOL_User_Data(player->Get_Name());

		if (user.IsValid()) {
			// Set there clan information
			RefPtr<WWOnline::SquadData> clan = user->GetSquad();

			if (clan.IsValid()) {
				outName.Format(L"%s [%S]", player->Get_Name(), clan->GetAbbr());
				return;
			}
		}
	}
#endif

	outName = player->Get_Name();
}
