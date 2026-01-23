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
 *                     $Archive:: /Commando/Code/Commando/dlgcncbattleinfo.cpp                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/24/02 2:06p                                                 $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "renegadedialog.h"
#include "dlgcncbattleinfo.h"
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
	COL_RANK,
	COL_NAME,	
	COL_KD,
	COL_SCORE
};

static const int BUILDING_COUNT		= 5;
static const int BUILDING_SLOT_COUNT	= 6;


////////////////////////////////////////////////////////////////
//	Local typedefs
////////////////////////////////////////////////////////////////
typedef struct
{
	const char *							texture_name;
	BuildingConstants::BuildingType	type;
} BUILDING_INSTANCE_INFO;


static const BUILDING_INSTANCE_INFO GDI_BUILDINGS[BUILDING_COUNT] =
{
	{ "HUD_C&C_G_GUARDTOW.TGA",	BuildingConstants::TYPE_BASE_DEFENSE },	
	{ "HUD_C&C_G_REFINERY.TGA",	BuildingConstants::TYPE_REFINERY },
	{ "HUD_C&C_G_POWER.TGA",		BuildingConstants::TYPE_POWER_PLANT },
	{ "HUD_C&C_G_BARRACKS.TGA",	BuildingConstants::TYPE_SOLDIER_FACTORY },
	{ "HUD_C&C_G_WARFACT.TGA",		BuildingConstants::TYPE_VEHICLE_FACTORY },
};

static const BUILDING_INSTANCE_INFO NOD_BUILDINGS[BUILDING_COUNT] =
{
	{ "HUD_C&C_OBLISK.TGA",			BuildingConstants::TYPE_BASE_DEFENSE },	
	{ "HUD_C&C_N_REFINERY.TGA",	BuildingConstants::TYPE_REFINERY },
	{ "HUD_C&C_N_POWER.TGA",		BuildingConstants::TYPE_POWER_PLANT },
	{ "HUD_C&C_N_HANDOF.TGA",		BuildingConstants::TYPE_SOLDIER_FACTORY },
	{ "HUD_C&C_N_AIRSTRIP.TGA",	BuildingConstants::TYPE_VEHICLE_FACTORY },
};


typedef struct
{
	int		icon_ctrl_id;
	int		health_ctrl_id;
} BUILDING_INFO;

static const BUILDING_INFO GDI_BUILDING_CTRLS[BUILDING_SLOT_COUNT] =
{
	{ IDC_GDI_BUILDING01_ICON, IDC_GDI_BUILDING01_HEALTHBAR },
	{ IDC_GDI_BUILDING02_ICON, IDC_GDI_BUILDING02_HEALTHBAR },
	{ IDC_GDI_BUILDING03_ICON, IDC_GDI_BUILDING03_HEALTHBAR },
	{ IDC_GDI_BUILDING04_ICON, IDC_GDI_BUILDING04_HEALTHBAR },
	{ IDC_GDI_BUILDING05_ICON, IDC_GDI_BUILDING05_HEALTHBAR },
	{ IDC_GDI_BUILDING06_ICON, IDC_GDI_BUILDING06_HEALTHBAR },
};

static const BUILDING_INFO NOD_BUILDING_CTRLS[BUILDING_SLOT_COUNT] =
{
	{ IDC_NOD_BUILDING01_ICON, IDC_NOD_BUILDING01_HEALTHBAR },
	{ IDC_NOD_BUILDING02_ICON, IDC_NOD_BUILDING02_HEALTHBAR },
	{ IDC_NOD_BUILDING03_ICON, IDC_NOD_BUILDING03_HEALTHBAR },
	{ IDC_NOD_BUILDING04_ICON, IDC_NOD_BUILDING04_HEALTHBAR },
	{ IDC_NOD_BUILDING05_ICON, IDC_NOD_BUILDING05_HEALTHBAR },
	{ IDC_NOD_BUILDING06_ICON, IDC_NOD_BUILDING06_HEALTHBAR },
};


////////////////////////////////////////////////////////////////
//
//	CNCBattleInfoDialogClass
//
////////////////////////////////////////////////////////////////
CNCBattleInfoDialogClass::CNCBattleInfoDialogClass (void)	:
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_CNC_BATTLE_INFO))
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~CNCBattleInfoDialogClass
//
////////////////////////////////////////////////////////////////
CNCBattleInfoDialogClass::~CNCBattleInfoDialogClass (void)
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
CNCBattleInfoDialogClass::On_Init_Dialog (void)
{
	MenuDialogClass::On_Init_Dialog ();
	MouseMgrClass::Show_Cursor (false);

	//
	//	Get a pointer to the list control
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_GDI_LIST_CTRL);
	if (list_ctrl != NULL) {

		//
		//	Configure the columns
		//
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_RANK),		0.175F,	Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_NAME),		0.425F,	Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_KD_RATIO),	0.2F,		Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_SCORE),		0.2F,		Vector3 (1, 1, 1));
		Populate_Player_List (list_ctrl, PLAYERTYPE_GDI);
	}

	//
	//	Get a pointer to the list control
	//
	list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_NOD_LIST_CTRL);
	if (list_ctrl != NULL) {

		//
		//	Configure the columns
		//
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_RANK),		0.175F,	Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_NAME),		0.425F,	Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_KD_RATIO),	0.2F,		Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_SCORE),		0.2F,		Vector3 (1, 1, 1));
		Populate_Player_List (list_ctrl, PLAYERTYPE_NOD);
	}

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
CNCBattleInfoDialogClass::Configure_Icons (void)
{
	BaseControllerClass *base = BaseControllerClass::Find_Base_For_Star ();
	if (base == NULL) {
		return ;
	}	

	//
	//	Configure the team icons
	//
	((ImageCtrlClass *)Get_Dlg_Item (IDC_GDI_TEAM_ICON))->Set_Texture ("HUD_C&C_GDILOGO.TGA");
	((ImageCtrlClass *)Get_Dlg_Item (IDC_NOD_TEAM_ICON))->Set_Texture ("HUD_C&C_NODLOGO.TGA");

	//
	//	Find the two base controllers
	//
	BaseControllerClass *gdi_base = BaseControllerClass::Find_Base ( PLAYERTYPE_GDI );
	BaseControllerClass *nod_base = BaseControllerClass::Find_Base ( PLAYERTYPE_NOD );

	int next_slot = 0;

	//
	//	Fill in the GDI buildings
	//
	int index;
	for (index = 0; index < BUILDING_COUNT; index ++) {

		//
		//	Find the building
		//
		BuildingGameObj *building = gdi_base->Find_Building (GDI_BUILDINGS[index].type);
		if (building != NULL) {

			//
			//	Configure the controls for this building
			//
			float life = building->Get_Defense_Object ()->Get_Health () / building->Get_Defense_Object ()->Get_Health_Max ();
			ImageCtrlClass *image_ctrl			= (ImageCtrlClass *)Get_Dlg_Item (GDI_BUILDING_CTRLS[next_slot].icon_ctrl_id);
			HealthBarCtrlClass *health_ctrl	= (HealthBarCtrlClass *)Get_Dlg_Item (GDI_BUILDING_CTRLS[next_slot].health_ctrl_id);
			image_ctrl->Set_Texture (GDI_BUILDINGS[index].texture_name);
			health_ctrl->Set_Life (life);
			next_slot ++;
		}		
	}

	//
	//	Now fill in the harvester
	//	
	VehicleGameObj *gdi_harvester = gdi_base->Get_Harvester_Vehicle ();
	if (gdi_harvester != NULL) {
		float life = gdi_harvester->Get_Defense_Object ()->Get_Health () / gdi_harvester->Get_Defense_Object ()->Get_Health_Max ();
		((ImageCtrlClass *)Get_Dlg_Item (GDI_BUILDING_CTRLS[next_slot].icon_ctrl_id))->Set_Texture ("HUD_C&C_G_HARVISTER.TGA");
		((HealthBarCtrlClass *)Get_Dlg_Item (GDI_BUILDING_CTRLS[next_slot].health_ctrl_id))->Set_Life (life);
		next_slot ++;
	}

	//
	//	Hide the remaining (unused) controls
	//
	for (index = next_slot; index < BUILDING_SLOT_COUNT; index ++) {
		Get_Dlg_Item (GDI_BUILDING_CTRLS[index].icon_ctrl_id)->Show (false);
		Get_Dlg_Item (GDI_BUILDING_CTRLS[index].health_ctrl_id)->Show (false);
	}

	//
	//	Fill in the NOD buildings
	//
	next_slot = 0;
	for (index = 0; index < BUILDING_COUNT; index ++) {

		//
		//	Find the building
		//
		BuildingGameObj *building = nod_base->Find_Building (NOD_BUILDINGS[index].type);
		if (building != NULL) {

			//
			//	Configure the controls for this building
			//
			float life = building->Get_Defense_Object ()->Get_Health () / building->Get_Defense_Object ()->Get_Health_Max ();
			((ImageCtrlClass *)Get_Dlg_Item (NOD_BUILDING_CTRLS[next_slot].icon_ctrl_id))->Set_Texture (NOD_BUILDINGS[index].texture_name);
			((HealthBarCtrlClass *)Get_Dlg_Item (NOD_BUILDING_CTRLS[next_slot].health_ctrl_id))->Set_Life (life);
			next_slot ++;
		}
	}

	//
	//	Now fill in the harvester
	//	
	VehicleGameObj *nod_harvester = nod_base->Get_Harvester_Vehicle ();
	if (nod_harvester != NULL) {
		float life = nod_harvester->Get_Defense_Object ()->Get_Health () / nod_harvester->Get_Defense_Object ()->Get_Health_Max ();
		((ImageCtrlClass *)Get_Dlg_Item (NOD_BUILDING_CTRLS[next_slot].icon_ctrl_id))->Set_Texture ("HUD_C&C_N_HARVISTER.TGA");
		((HealthBarCtrlClass *)Get_Dlg_Item (NOD_BUILDING_CTRLS[next_slot].health_ctrl_id))->Set_Life (life);
		next_slot ++;
	}

	//
	//	Hide the remaining (unused) controls
	//
	for (index = next_slot; index < BUILDING_SLOT_COUNT; index ++) {
		Get_Dlg_Item (NOD_BUILDING_CTRLS[index].icon_ctrl_id)->Show (false);
		Get_Dlg_Item (NOD_BUILDING_CTRLS[index].health_ctrl_id)->Show (false);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Populate_Player_List
//
////////////////////////////////////////////////////////////////
void
CNCBattleInfoDialogClass::Populate_Player_List (ListCtrlClass *list_ctrl, int team_id)
{
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
				list_ctrl->Set_Entry_Text(item_index, COL_NAME, (const unichar_t*)displayName);

				list_ctrl->Set_Entry_Int (item_index, COL_RANK,			player->Get_Rung ());
				list_ctrl->Set_Entry_Int (item_index, COL_SCORE,		player->Get_Score ());

				//
				//	Put a star by the player's name if this is the local player
				//
				if (player->Get_GameObj () == COMBAT_STAR) {
					list_ctrl->Add_Icon (item_index, COL_NAME, "IF_LRGSTAR.TGA");
					list_ctrl->Set_Entry_Color (item_index, COL_RANK,		Vector3 (1.0F, 1.0F, 1.0F));
					list_ctrl->Set_Entry_Color (item_index, COL_NAME,		Vector3 (1.0F, 1.0F, 1.0F));
					list_ctrl->Set_Entry_Color (item_index, COL_KD,			Vector3 (1.0F, 1.0F, 1.0F));
					list_ctrl->Set_Entry_Color (item_index, COL_SCORE,		Vector3 (1.0F, 1.0F, 1.0F));					
				}

				//
				//	Convert the kill to death ratio to a string and put it into the right column
				//
				WideStringClass number_str;
				float ktd_ratio = player->Get_Kill_To_Death_Ratio ();
				if (ktd_ratio >= 0) {
					number_str.Format(U_CHAR("%-8.1f"), ktd_ratio);
				} else {
					number_str.Format(U_CHAR("%-8s"), U_CHAR("-"));
				}

				list_ctrl->Set_Entry_Text (item_index, COL_KD, number_str);

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
	//list_ctrl->Auto_Size_Columns_Include_Contents (4.0F);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	ListSortCallback
//
////////////////////////////////////////////////////////////////
int
CNCBattleInfoDialogClass::ListSortCallback
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
CNCBattleInfoDialogClass::On_Frame_Update (void)
{
	//
	//	End the dialog when the user releases the player list key
	//
	int dik_id = Input::Get_Primary_Key_For_Function (INPUT_FUNCTION_BATTLE_INFO_TOGGLE);
	if ((DirectInput::Get_Keyboard_Button (dik_id) & DirectInput::DI_BUTTON_HELD) == 0) {
		End_Dialog ();
	}

	return ;
}


void CNCBattleInfoDialogClass::Build_Player_Display_Name(const cPlayer* player, WideStringClass& outName)
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
