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
 *                     $Archive:: /Commando/Code/commando/dlgevamaptab.cpp      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/09/02 1:54p                                               $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgevamaptab.h"
#include "mapctrl.h"
#include "combat.h"
#include "soldier.h"
#include "objectives.h"
#include "translatedb.h"
#include "translateobj.h"
#include "listctrl.h"
#include "mapmgr.h"
#include "lineseg.h"
#include "rendobj.h"
#include "phys.h"
#include "pscene.h"
#include "coltype.h"
#include "physcoltest.h"
#include "gamemode.h"
#include "gameinitmgr.h"
#include "imagectrl.h"
#include "string_ids.h"


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
EvaMapTabClass::On_Init_Dialog (void)
{
	//
	//	Set the map's title
	//
	Set_Dlg_Item_Text (IDC_MAP_TITLE, TRANSLATE (MapMgrClass::Get_Map_Title ()));

	MapCtrlClass *map_ctrl = (MapCtrlClass *)Get_Dlg_Item (IDC_MAP_CTRL);
	if (map_ctrl != NULL) {
		
		//
		//	Configure the map
		//
		StringClass texture_filename;
		MapMgrClass::Get_Map_Texture_Filename (texture_filename);
		if (!texture_filename.Is_Empty()) {
			map_ctrl->Set_Map_Texture (texture_filename);
		}
		map_ctrl->Set_Marker_Texture ("MAPICONS.TGA");
		map_ctrl->Set_Map_Center (MapMgrClass::Get_Map_Center ());
		map_ctrl->Set_Map_Scale (MapMgrClass::Get_Map_Scale ());
		map_ctrl->Initialize_Cloud (MapMgrClass::CLOUD_WIDTH, MapMgrClass::CLOUD_HEIGHT);

		//
		//	Do the cloud
		//
		for (int cell_x = 0; cell_x < MapMgrClass::CLOUD_WIDTH; cell_x ++) {
			for (int cell_y = 0; cell_y < MapMgrClass::CLOUD_HEIGHT; cell_y ++) {
				map_ctrl->Set_Cloud_Cell (cell_x, cell_y, MapMgrClass::Is_Cell_Visible (cell_x, cell_y));
			}
		}
		
		//
		//	Now add a marker that represents Havoc to the map
		//
		if (COMBAT_STAR && MapMgrClass::Is_Player_Marker_Visible ()) {
			Vector3 pos;
			COMBAT_STAR->Get_Position (&pos);
			map_ctrl->Add_Marker (TRANSLATE (IDS_MENU_HAVOC), pos, RectClass (0, 0, 31, 31));
		}

		//
		//	Add all the objective markers
		//
		int count = ObjectiveManager::Get_Objective_Count ();
		for (int index = 0; index < count; index ++) {
			Objective *objective = ObjectiveManager::Get_Objective (index);
			
			//
			//	Is this an objective we should put on the map?
			//
			if (	objective != NULL &&
					objective->Status == ObjectiveManager::STATUS_IS_PENDING)
			{
				//
				//	Now add a marker that represents this objective
				//
				int color = VRGB_TO_INT32 (objective->Type_To_Color ());
				int item_index = map_ctrl->Add_Marker (L"", objective->Position, RectClass (32, 0, 64, 32), color);
				map_ctrl->Set_Marker_Data (item_index, (uintptr_t)objective);
			}
		}

		//
		//	Make sure that Havoc is in view
		//
		if (COMBAT_STAR != NULL && MapMgrClass::Is_Player_Marker_Visible ()) {
			map_ctrl->Center_View_About_Marker (0);
		}
	}

	//
	//	Configure the list control
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_OBJECT_DESC_LIST);
	if (list_ctrl != NULL) {
		list_ctrl->Add_Column (L"", 1.0F, Vector3 (1, 1, 1));
	}

	ChildDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_MapCtrl_Marker_Hilighted
//
////////////////////////////////////////////////////////////////
void
EvaMapTabClass::On_MapCtrl_Marker_Hilighted
(
	MapCtrlClass *	map_ctrl,
	int				ctrl_id,
	int				marker_index
)
{
	ListCtrlClass *list_ctrl	= (ListCtrlClass *)Get_Dlg_Item (IDC_OBJECT_DESC_LIST);
	ImageCtrlClass *image_ctrl	= (ImageCtrlClass *)Get_Dlg_Item (IDC_IMAGE);
	if (list_ctrl == NULL || image_ctrl == NULL) {
		return ;
	}

	//
	//	Reset the objective description list
	//
	list_ctrl->Delete_All_Entries ();
	image_ctrl->Set_Texture (NULL);

	if (marker_index == 0) {
		
		//
		//	Let the user know its Havoc
		//
		list_ctrl->Insert_Entry (0, TRANSLATE (IDS_MENU_NAME_HAVOC));

	} else if (marker_index > 0) {
		
		//
		//	Display the objective information
		//
		Objective *objective = (Objective *)map_ctrl->Get_Marker_Data (marker_index);
		if (objective != NULL) {

			WideStringClass name = TRANSLATE (objective->ShortDescriptionID);
			WideStringClass string;
			string.Format (TRANSLATE (IDS_MENU_NAME_FORMAT), (const wchar_t *)name);
			list_ctrl->Insert_Entry (0, string);

			string.Format (TRANSLATE (IDS_MENU_PRIORITY_FORMAT), objective->Type_To_Name ());
			list_ctrl->Insert_Entry (1, string);
			list_ctrl->Set_Entry_Color (1, 0, objective->Type_To_Color ());

			string.Format (TRANSLATE (IDS_MENU_STATUS_FORMAT), objective->Status_To_Name ());
			list_ctrl->Insert_Entry (2, string);
			list_ctrl->Insert_Entry (3, L"");

			//
			//	Lookup the translation object from the strings database
			//
			TDBObjClass *translate_obj = TranslateDBClass::Find_Object (objective->LongDescriptionID);
			if (translate_obj != NULL) {			
				list_ctrl->Insert_Entry (4, translate_obj->Get_String ());
			}

			//
			//	Configure the POG icon
			//
			image_ctrl->Set_Texture (objective->HUDPogTextureName);
		}		
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_MapCtrl_Pos_Clicked
//
////////////////////////////////////////////////////////////////
void
EvaMapTabClass::On_MapCtrl_Pos_Clicked
(
	MapCtrlClass *		map_ctrl,
	int					ctrl_id,
	const Vector3 &	position
)
{
	float start_z	= 1000;
	float end_z		= -1000;

	//
	//	Setup a raycast at this (x,y) position to find the z position
	//
	CastResultStruct result;
	LineSegClass ray (Vector3 (position.X, position.Y, start_z), Vector3 (position.X, position.Y, end_z));
	PhysRayCollisionTestClass raytest (ray, &result, 0, COLLISION_TYPE_PROJECTILE);
	PhysicsSceneClass::Get_Instance ()->Cast_Ray (raytest);

	Vector3 world_space_pos = position;

	//
	//	Calculate what the z-position is
	//
	if (result.Fraction < 1.0F) {
		world_space_pos.Z = start_z + (end_z - start_z) * result.Fraction;
	}

	//
	//	Warp the player to this position
	//
	COMBAT_STAR->Set_Position (world_space_pos);

	//
	//	Return to the game
	//
	//extern void Continue_Game (void);
	//Continue_Game ();
	GameInitMgrClass::Continue_Game();
	return ;
}

