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
 *                 Project Name : Commando                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/dlgevadatatab.cpp                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/21/01 12:14p                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "renegadedialog.h"
#include "dlgevadatatab.h"
#include "string_ids.h"
#include "translatedb.h"
#include "objectives.h"
#include "player.h"
#include "cnetwork.h"
#include "imagectrl.h"


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////
//
//	EvaDataTabClass
//
////////////////////////////////////////////////////////////////
EvaDataTabClass::EvaDataTabClass (void)	:
	ChildDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_ENCYCLOPEDIA_DATA_TAB))
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
EvaDataTabClass::On_Init_Dialog (void)
{
	Fill_Statistics_List ();

	ChildDialogClass::On_Init_Dialog ();
	return ;
}



////////////////////////////////////////////////////////////////
//
//	Fill_Statistics_List
//
////////////////////////////////////////////////////////////////
void
EvaDataTabClass::Fill_Statistics_List (void)
{
	//
	//	Get a pointer to the list control
	//
	//ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LISTCTRL);
	//if (list_ctrl == NULL) {
		//return ;
	//}

	((ImageCtrlClass *)Get_Dlg_Item (IDC_GAME_TIME_IMAGE))->Set_Texture ("if_GameTime.tga");
	((ImageCtrlClass *)Get_Dlg_Item (IDC_WEAPONS_IMAGE))->Set_Texture ("if_Weapons.tga");
	((ImageCtrlClass *)Get_Dlg_Item (IDC_POWERUPS_IMAGE))->Set_Texture ("if_PowerUps.tga");
	((ImageCtrlClass *)Get_Dlg_Item (IDC_FRIENDLIES_IMAGE))->Set_Texture ("if_casualties.tga");
	((ImageCtrlClass *)Get_Dlg_Item (IDC_SHOTS_FIRED_IMAGE))->Set_Texture ("if_Ammo.tga");
	((ImageCtrlClass *)Get_Dlg_Item (IDC_ACCURACY_IMAGE))->Set_Texture ("if_Percentage.tga");
	((ImageCtrlClass *)Get_Dlg_Item (IDC_ENEMIES_KILLED_IMAGE))->Set_Texture ("if_NodKills.tga");
	((ImageCtrlClass *)Get_Dlg_Item (IDC_VEHICLE_TIME_IMAGE))->Set_Texture ("if_VehicleTime.tga");
	((ImageCtrlClass *)Get_Dlg_Item (IDC_VEHICLE_KILLS_IMAGE))->Set_Texture ("if_VehiclesKill.tga");
	((ImageCtrlClass *)Get_Dlg_Item (IDC_VEHICLE_SQUISHES_IMAGE))->Set_Texture ("if_RunOvers.tga");
	((ImageCtrlClass *)Get_Dlg_Item (IDC_VEHICLES_DESTROYED_IMAGE))->Set_Texture ("if_KillVehicles.tga");
	((ImageCtrlClass *)Get_Dlg_Item (IDC_BUILDINGS_DESTROYED_IMAGE))->Set_Texture ("if_buildingsKill.tga");
	((ImageCtrlClass *)Get_Dlg_Item (IDC_HIT_MAP_IMAGE))->Set_Texture ("if_NodHits128.tga");

	cPlayer *player = cNetwork::Get_My_Player_Object ();
	if (player != NULL) {

		Set_Dlg_Item_Int (IDC_WEAPONS_TEXT,				player->Get_Weapon_Fired_Count ());
		Set_Dlg_Item_Int (IDC_POWERUPS_TEXT,			player->Get_Powerups_Collected ());
		Set_Dlg_Item_Int (IDC_FRIENDLIES_TEXT,			player->Get_Allies_Killed ());
		Set_Dlg_Item_Int (IDC_SHOTS_FIRED_TEXT,		player->Get_Shots_Fired ());		
		Set_Dlg_Item_Int (IDC_ENEMIES_KILLED_TEXT,	player->Get_Enemies_Killed ());		
		Set_Dlg_Item_Int (IDC_VEHICLE_KILLS_TEXT,		player->Get_Kills_From_Vehicle ());
		Set_Dlg_Item_Int (IDC_VEHICLE_SQUISHES_TEXT,	player->Get_Squishes ());
		Set_Dlg_Item_Int (IDC_VEHICLES_KILLED_TEXT,	player->Get_Vehiclies_Destroyed ());
		Set_Dlg_Item_Int (IDC_BUILDINGS_KILLED_TEXT,	player->Get_Building_Destroyed ());
		Set_Dlg_Item_Int (IDC_HEAD_SHOT_TEXT,			player->Get_Head_Shots ());
		Set_Dlg_Item_Int (IDC_TORSO_SHOT_TEXT,			player->Get_Torso_Shots ());
		Set_Dlg_Item_Int (IDC_ARM_SHOT_TEXT,			player->Get_Arm_Shots ());
		Set_Dlg_Item_Int (IDC_CROTCH_SHOT_TEXT,		player->Get_Crotch_Shots ());
		Set_Dlg_Item_Int (IDC_LEG_SHOT_TEXT,			player->Get_Leg_Shots ());

		//
		//	Total up the number of shots and hits this player has mode
		//
		int total_shots	= player->Get_Shots_Fired ();
		int total_hits		= player->Get_Head_Shots ();
		total_hits			+= player->Get_Torso_Shots ();
		total_hits			+= player->Get_Arm_Shots ();
		total_hits			+= player->Get_Leg_Shots ();
		total_hits			+= player->Get_Crotch_Shots ();
		float	accuracy		= 100.0F;
		if (total_shots > 0) {
			accuracy	= (total_hits * 100.0F) / (float)total_shots;
		}
		Set_Dlg_Item_Float (IDC_ACCURACY_TEXT, accuracy);

		//
		//	Format the time in vehicles text
		//
		WideStringClass time_string;
		float time		= player->Get_Vehicle_Time ();
		int hours		= int(time / 3600.0F);
		time				-= hours * 3600.0F;
		int minutes		= int(time / 60.0F);
		time				-= minutes * 60.0F;
		int seconds		= (int)time;

		if (hours > 0) {
			time_string.Format (L"%.01d:%.02d:%.2d", hours, minutes, seconds);
		} else {
			time_string.Format (L"%.02d:%.2d", minutes, seconds);
		}

		Set_Dlg_Item_Text (IDC_VEHICLES_TIME_TEXT, time_string);

		//
		//	Format the time in game text
		//
		time		= player->Get_Game_Time ();
		hours		= int(time / 3600.0F);
		time		-= hours * 3600.0F;
		minutes	= int(time / 60.0F);
		time		-= minutes * 60.0F;
		seconds	= (int)time;

		time_string.Format (L"%.02d", hours);
		Set_Dlg_Item_Text (IDC_GAME_TIME_HOURS_TEXT, time_string);
		time_string.Format (L"%.02d", minutes);
		Set_Dlg_Item_Text (IDC_GAME_TIME_MINS_TEXT, time_string);
		time_string.Format (L"%.02d", seconds);
		Set_Dlg_Item_Text (IDC_GAME_TIME_SECS_TEXT, time_string);
		
		
		/*WideStringClass entry;

		entry.Format (TRANSLATE (IDS_EVA_DATA_GAME_TIME),  / 60.0F);
		list_ctrl->Insert_Entry (0xFF, entry);

		entry.Format (TRANSLATE (IDS_EVA_DATA_HEAD_HIT), player->Get_Head_Hit ());
		list_ctrl->Insert_Entry (0xFF, entry);

		entry.Format (TRANSLATE (IDS_EVA_DATA_TORSO_HIT), player->Get_Torso_Hit ());
		list_ctrl->Insert_Entry (0xFF, entry);

		entry.Format (TRANSLATE (IDS_EVA_DATA_ARM_HIT), player->Get_Arm_Hit ());
		list_ctrl->Insert_Entry (0xFF, entry);

		entry.Format (TRANSLATE (IDS_EVA_DATA_LEG_HIT), player->Get_Leg_Hit ());
		list_ctrl->Insert_Entry (0xFF, entry);

		entry.Format (TRANSLATE (IDS_EVA_DATA_CROTCH_HIT), player->Get_Crotch_Hit ());
		list_ctrl->Insert_Entry (0xFF, entry);

*/
	}

	return ;
}
