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
 *                 Project Name : commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/dlgcncteaminfo.cpp                  $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/24/02 2:06p                                               $*
 *                                                                                             *
 *                    $Revision:: 12                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgcncteaminfo.h"
#include "renegadedialog.h"
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
#include "assets.h"
#include "translatedb.h"
#include "wolgmode.h"
#include <WWOnline/WOLUser.h>
#include "string_ids.h"
#include "mousemgr.h"
#include "directinput.h"


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
enum
{
	COL_RANK,
	COL_NAME,
	COL_CREDITS,
	COL_SCORE,
	COL_CHARACTER,
	COL_VEHICLE,
};


////////////////////////////////////////////////////////////////
//
//	CNCTeamInfoDialogClass
//
////////////////////////////////////////////////////////////////
CNCTeamInfoDialogClass::CNCTeamInfoDialogClass (void)	:
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_CNC_TEAM_INFO))
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~CNCTeamInfoDialogClass
//
////////////////////////////////////////////////////////////////
CNCTeamInfoDialogClass::~CNCTeamInfoDialogClass (void)
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
CNCTeamInfoDialogClass::On_Init_Dialog (void)
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
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_RANK),		0.065F,	Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_NAME),		0.2F,		Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MP_MONEY),		0.092F,	Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_SCORE),		0.075F,	Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_TEXT004),	0.33F,	Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_TEXT002),	0.24F,	Vector3 (1, 1, 1));

		//
		//	Configure the icon support
		//
		list_ctrl->Set_Min_Row_Height (20);
		list_ctrl->Set_Icon_Size (19, 19);
	}

	//
	//	Fill in the player list
	//
	Populate_Player_List ();

	//
	//	Setup the team and building icons on the dialog
	//
	Configure_Icons ();

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
//	Configure_Icons
//
////////////////////////////////////////////////////////////////
void
CNCTeamInfoDialogClass::Configure_Icons (void)
{
	BaseControllerClass *base = BaseControllerClass::Find_Base_For_Star ();
	if (base == NULL) {
		return ;
	}

	StringClass team_icon_name			("HUD_C&C_GDILOGO.TGA");
	StringClass defenses_icon_name	("HUD_C&C_G_GUARDTOW.TGA");
	StringClass refinery_icon_name	("HUD_C&C_G_REFINERY.TGA");
	StringClass pplant_icon_name		("HUD_C&C_G_POWER.TGA");
	StringClass sfact_icon_name		("HUD_C&C_G_BARRACKS.TGA");
	StringClass vfact_icon_name		("HUD_C&C_G_WARFACT.TGA");

	//
	//	Switch icons for the NOD team
	//
	if (COMBAT_STAR->Get_Player_Type () == PLAYERTYPE_NOD) {
		team_icon_name			= "HUD_C&C_NODLOGO.TGA";
		defenses_icon_name	= "HUD_C&C_OBLISK.TGA";
		refinery_icon_name	= "HUD_C&C_N_REFINERY.TGA";
		pplant_icon_name		= "HUD_C&C_N_POWER.TGA";
		sfact_icon_name		= "HUD_C&C_N_HANDOF.TGA";
		vfact_icon_name		= "HUD_C&C_N_AIRSTRIP.TGA";
	}

	const int ICON_CTRLS[] = 
	{
		IDC_BUILDING01_ICON,
		IDC_BUILDING02_ICON,
		IDC_BUILDING03_ICON,
		IDC_BUILDING04_ICON,
		IDC_BUILDING05_ICON,
	};

	const int HEALTH_CTRLS[] = 
	{
		IDC_BUILDING01_HEALTHBAR,
		IDC_BUILDING02_HEALTHBAR,
		IDC_BUILDING03_HEALTHBAR,
		IDC_BUILDING04_HEALTHBAR,
		IDC_BUILDING05_HEALTHBAR,
	};

	int next_slot = 0;
	
	//
	//	Configure the image ctrls on the dialog
	//
	((ImageCtrlClass *)Get_Dlg_Item (IDC_TEAM_ICON))->Set_Texture (team_icon_name);

	//
	//	Configure the base defense building
	//
	BuildingGameObj *building = base->Find_Building (BuildingConstants::TYPE_BASE_DEFENSE);
	if (building != NULL) {
		float life = building->Get_Defense_Object ()->Get_Health () / building->Get_Defense_Object ()->Get_Health_Max ();
		((ImageCtrlClass *)Get_Dlg_Item (ICON_CTRLS[next_slot]))->Set_Texture (defenses_icon_name);
		((HealthBarCtrlClass *)Get_Dlg_Item (HEALTH_CTRLS[next_slot]))->Set_Life (life);
		next_slot ++;
	}

	//
	//	Configure the refinery building
	//
	building = base->Find_Building (BuildingConstants::TYPE_REFINERY);
	if (building != NULL) {
		float life = building->Get_Defense_Object ()->Get_Health () / building->Get_Defense_Object ()->Get_Health_Max ();
		((ImageCtrlClass *)Get_Dlg_Item (ICON_CTRLS[next_slot]))->Set_Texture (refinery_icon_name);
		((HealthBarCtrlClass *)Get_Dlg_Item (HEALTH_CTRLS[next_slot]))->Set_Life (life);
		next_slot ++;
	}

	//
	//	Configure the powerplant building
	//
	building = base->Find_Building (BuildingConstants::TYPE_POWER_PLANT);
	if (building != NULL) {
		float life = building->Get_Defense_Object ()->Get_Health () / building->Get_Defense_Object ()->Get_Health_Max ();
		((ImageCtrlClass *)Get_Dlg_Item (ICON_CTRLS[next_slot]))->Set_Texture (pplant_icon_name);
		((HealthBarCtrlClass *)Get_Dlg_Item (HEALTH_CTRLS[next_slot]))->Set_Life (life);
		next_slot ++;
	}

	//
	//	Configure the soldier factory building
	//
	building = base->Find_Building (BuildingConstants::TYPE_SOLDIER_FACTORY);
	if (building != NULL) {
		float life = building->Get_Defense_Object ()->Get_Health () / building->Get_Defense_Object ()->Get_Health_Max ();
		((ImageCtrlClass *)Get_Dlg_Item (ICON_CTRLS[next_slot]))->Set_Texture (sfact_icon_name);
		((HealthBarCtrlClass *)Get_Dlg_Item (HEALTH_CTRLS[next_slot]))->Set_Life (life);
		next_slot ++;
	}

	//
	//	Configure the soldier factory building
	//
	building = base->Find_Building (BuildingConstants::TYPE_VEHICLE_FACTORY);
	if (building != NULL) {
		float life = building->Get_Defense_Object ()->Get_Health () / building->Get_Defense_Object ()->Get_Health_Max ();
		((ImageCtrlClass *)Get_Dlg_Item (ICON_CTRLS[next_slot]))->Set_Texture (vfact_icon_name);
		((HealthBarCtrlClass *)Get_Dlg_Item (HEALTH_CTRLS[next_slot]))->Set_Life (life);
		next_slot ++;
	}

	//
	//	Hide the remaining (unused) controls
	//
	for (int index = next_slot; index < 5; index ++) {
		Get_Dlg_Item (ICON_CTRLS[index])->Show (false);
		Get_Dlg_Item (HEALTH_CTRLS[index])->Show (false);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Populate_Player_List
//
////////////////////////////////////////////////////////////////
void
CNCTeamInfoDialogClass::Populate_Player_List (void)
{
	//
	//	Get a pointer to the list control
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LIST_CTRL);
	if (list_ctrl == NULL) {
		return ;
	}

	//
	//	Determine which team the player is on
	//
	int team_id = COMBAT_STAR->Get_Player_Type ();

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

		if (player->Get_Is_Active().Is_False()) {
			continue;
		}

		//
		//	Does this player belong to the same team?
		//
		if (player->Get_Player_Type () == team_id) {		

			//
			//	Make a new entry for this player
			//			
			int item_index = list_ctrl->Insert_Entry (index ++, U_CHAR(""));
			if (item_index >= 0) {
				WideStringClass displayName(0, true);
				Build_Player_Display_Name(player, displayName);
				list_ctrl->Set_Entry_Text (item_index, COL_NAME,		(const unichar_t*)displayName);

				list_ctrl->Set_Entry_Int (item_index, COL_RANK,			player->Get_Rung ());
				list_ctrl->Set_Entry_Int (item_index, COL_CREDITS,		(int) player->Get_Money ());
				list_ctrl->Set_Entry_Int (item_index, COL_SCORE,		(int) player->Get_Score ());

				//
				//	Fill in the character and vehicle columns
				//
				SmartGameObj *game_obj = player->Get_GameObj ();
				if (game_obj != NULL && game_obj->As_SoldierGameObj () != NULL) {
					//StringClass tga_filename;
					//::Strip_Path_From_Filename (tga_filename, game_obj->Get_Definition ().Get_Icon_Filename ());
					//list_ctrl->Add_Icon (item_index, COL_CHARACTER,	tga_filename);

					const unichar_t *name = TRANSLATE (game_obj->Get_Translated_Name_ID ());
					list_ctrl->Set_Entry_Text (item_index, COL_CHARACTER, name);
					
					//
					//	Fill in the vehicle icon (if the player is in one)
					//
					SoldierGameObj *soldier = game_obj->As_SoldierGameObj ();
					VehicleGameObj	*vehicle = soldier->Get_Vehicle ();
					if (vehicle != NULL) {
						//::Strip_Path_From_Filename (tga_filename, vehicle->Get_Definition ().Get_Icon_Filename ());
						//list_ctrl->Add_Icon (item_index, COL_VEHICLE, tga_filename);
						name = TRANSLATE (vehicle->Get_Translated_Name_ID ());
						list_ctrl->Set_Entry_Text (item_index, COL_VEHICLE, name);
					}
				}				
				
				//
				//	Put a star by the player's name if this is the local player
				//
				if (game_obj == COMBAT_STAR) {
					list_ctrl->Add_Icon (item_index, COL_NAME, "IF_LRGSTAR.TGA");
					list_ctrl->Set_Entry_Color (item_index, COL_RANK,		Vector3 (1.0F, 1.0F, 1.0F));
					list_ctrl->Set_Entry_Color (item_index, COL_NAME,		Vector3 (1.0F, 1.0F, 1.0F));
					list_ctrl->Set_Entry_Color (item_index, COL_CREDITS,	Vector3 (1.0F, 1.0F, 1.0F));
					list_ctrl->Set_Entry_Color (item_index, COL_SCORE,		Vector3 (1.0F, 1.0F, 1.0F));
					list_ctrl->Set_Entry_Color (item_index, COL_CHARACTER, Vector3 (1.0F, 1.0F, 1.0F));
					list_ctrl->Set_Entry_Color (item_index, COL_VEHICLE,	Vector3 (1.0F, 1.0F, 1.0F));
				}

				//
				//	Set the ranking as this entries user data so we can sort by it...
				//
				list_ctrl->Set_Entry_Data (item_index, COL_RANK, player->Get_Rung ());
			}
		}
	}

	//
	//	Now, sort the players by rank
	//
	list_ctrl->Sort (ListSortCallback, 0);
//	list_ctrl->Auto_Size_Columns_Include_Contents (4.0F);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	ListSortCallback
//
////////////////////////////////////////////////////////////////
int
CNCTeamInfoDialogClass::ListSortCallback
(
	ListCtrlClass *	list_ctrl,
	int					item_index1,
	int					item_index2,
	uint32				/* user_param */
)
{
	int rank1 = (int)list_ctrl->Get_Entry_Data (item_index1, COL_RANK);
	int rank2 = (int)list_ctrl->Get_Entry_Data (item_index2, COL_RANK);
	return (rank1 - rank2);
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
CNCTeamInfoDialogClass::On_Frame_Update (void)
{
	//
	//	End the dialog when the user releases the player list key
	//
	int dik_id = Input::Get_Primary_Key_For_Function (INPUT_FUNCTION_TEAM_INFO_TOGGLE);
	if ((DirectInput::Get_Keyboard_Button (dik_id) & DirectInput::DI_BUTTON_HELD) == 0) {
		End_Dialog ();
	}

	return ;
}


void CNCTeamInfoDialogClass::Build_Player_Display_Name(const cPlayer* player, WideStringClass& outName)
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
				outName.Format(U_CHAR("%s [%S]"), player->Get_Name(), clan->GetAbbr());
				return;
			}
		}
	}
#endif

	outName = player->Get_Name();
}
