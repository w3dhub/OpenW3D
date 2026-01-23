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
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/commando/dlgcncpurchasemenu.cpp              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/15/02 12:00p                                              $*
 *                                                                                             *
 *                    $Revision:: 22                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "renegadedialog.h"
#include "dlgcncpurchasemenu.h"
#include "dlgcncpurchasemainmenu.h"
#include "purchasesettings.h"
#include "vendor.h"
#include "merchandisectrl.h"
#include "playerdata.h"
#include "combat.h"
#include "soldier.h"
#include "translatedb.h"
#include "combatchunkid.h"
#include "weaponmanager.h"
#include "powerup.h"
#include "gdcnc.h"
#include "gdskirmish.h"
#include "weapons.h"
#include "vehiclefactorygameobj.h"
#include "string_ids.h"
#include "DlgMessageBox.h"
#include "gameinitmgr.h"
#include "healthbarctrl.h"
#include "imagectrl.h"
#include "hud.h"
#include "mpsettingsmgr.h"
#include "mapmgr.h"
#include "wwphysids.h"


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
static const int	PURCHASE_ITEMS			= 10;
static const int	PURCHASE_ALTERNATES	= 3;

const int ITEM_CTRL_IDS[PURCHASE_ITEMS] = 
{
	IDC_ITEM_1, IDC_ITEM_2, IDC_ITEM_3, IDC_ITEM_4, IDC_ITEM_5,
	IDC_ITEM_6, IDC_ITEM_7, IDC_ITEM_8, IDC_ITEM_9, IDC_ITEM_10
};

const int HOTKEY_CTRL_IDS[PURCHASE_ITEMS] = 
{
	IDC_HOTKEY_TEXT_01, IDC_HOTKEY_TEXT_02, IDC_HOTKEY_TEXT_03, IDC_HOTKEY_TEXT_04,
	IDC_HOTKEY_TEXT_05, IDC_HOTKEY_TEXT_06, IDC_HOTKEY_TEXT_07, IDC_HOTKEY_TEXT_08,
	IDC_HOTKEY_TEXT_09, IDC_HOTKEY_TEXT_10
};

////////////////////////////////////////////////////////////////
//
//	CNCPurchaseMenuClass
//
////////////////////////////////////////////////////////////////
CNCPurchaseMenuClass::CNCPurchaseMenuClass (void)	:
	TotalCost (0),
	IsProductionDisabled (false),
	CostScalingFactor (1.0F),
	EnableMultiplePurchases (true),
	Team (PlayerTerminalClass::TYPE_NONE),
	PurchaseType (PurchaseSettingsDefClass::TYPE_CLASSES),
	HealthUpdateTimer (0),
	EnabledStateUpdateTimer (0),
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_CNC_PURCHASE_SCREEN))
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMenuClass::On_Init_Dialog (void)
{
	MenuDialogClass::On_Init_Dialog ();

	WWASSERT(COMBAT_STAR != NULL);
	WWASSERT(COMBAT_STAR->Get_Player_Data() != NULL);

	//
	//	Get the player's current money
	//
	PlayerDataClass *player_data = COMBAT_STAR->Get_Player_Data ();

	//
	//	Force the hot-key colors to be white
	//
	Get_Dlg_Item (IDC_HOTKEY_TEXT_01)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_HOTKEY_TEXT_02)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_HOTKEY_TEXT_03)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_HOTKEY_TEXT_04)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_HOTKEY_TEXT_05)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_HOTKEY_TEXT_06)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_HOTKEY_TEXT_07)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_HOTKEY_TEXT_08)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_HOTKEY_TEXT_09)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));
	Get_Dlg_Item (IDC_HOTKEY_TEXT_10)->Set_Text_Color (Vector3 (1.0F, 1.0F, 1.0F));

	//
	//	Initialize the purchase controls
	//
	for (int index = 0; index < PURCHASE_ITEMS; index ++) {
		
		//
		//	Get the control for this entry
		//
		MerchandiseCtrlClass *ctrl = (MerchandiseCtrlClass *)Get_Dlg_Item (ITEM_CTRL_IDS[index]);
		if (ctrl != NULL) {

			//
			//	Is this slot available?
			//
			int definition_id					= Definition->Get_Definition (index);				
			DefinitionClass *definition	= DefinitionMgrClass::Find_Definition (definition_id);
			if (Is_Definition_OK (definition)) {

				//
				//	Configure the merchandise settings
				//
				int cost = Definition->Get_Cost (index) * CostScalingFactor;
				ctrl->Set_Text (Definition->Get_Name (index));
				ctrl->Set_Cost (cost);
				ctrl->Set_Texture (Definition->Get_Texture (index));
				ctrl->Set_User_Data (index);

				//
				//	Load in the alternate skins (as necessary)
				//
				if (MPSettingsMgrClass::Are_Alternate_Skins_Unlocked ()) {
					for (int alt_index = 0; alt_index < PURCHASE_ALTERNATES; alt_index ++) {
						int alt_def_id = Definition->Get_Alt_Definition (index, alt_index);
						if (alt_def_id != 0) {
							ctrl->Add_Alternate_Texture (Definition->Get_Alt_Texture (index, alt_index));
						}
					}
				}

				//
				//	Disable any options that cost money if production is disabled
				//
				if ((IsProductionDisabled && (cost > 0)) || (player_data->Get_Money () < cost)) {
					ctrl->Enable (false);
				}

			} else {
				
				//
				//	Remove and free the control if we aren't going to use it
				//
				Remove_Control (ctrl);
				delete ctrl;

				//
				//	Remove the hotkey control as well
				//
				DialogControlClass *hotkey_ctrl = Get_Dlg_Item (HOTKEY_CTRL_IDS[index]);
				Remove_Control (hotkey_ctrl);
				delete hotkey_ctrl;
			}
		}
	}

	//
	//	Initialize the current credits and total cost
	//
	Set_Dlg_Item_Int (IDC_CREDITS, (int) player_data->Get_Money ());

	//
	//	Setup the images for the building health icons
	//
	Configure_Building_Icons ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDC_PURCHASE_BUTTON:
			Purchase ();
			break;

		case IDC_CLEAR_BUTTON:
			Clear_Shopping_Cart ();
			break;

		default:
			break;
	}

	MenuDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMenuClass::Render (void)
{
	MenuDialogClass::Render ();
	HUDClass::Damage_Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Merchandise_DblClk
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMenuClass::On_Merchandise_DblClk (MerchandiseCtrlClass * /* ctrl */, int ctrl_id)
{
	Purchase_Item (ctrl_id);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Purchase_Item
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMenuClass::Purchase_Item (int ctrl_id)
{
	//
	//	Reset the shopping list to include just this one purchase
	//
	Clear_Shopping_Cart ();
	Add_Item_To_Shopping_Cart (ctrl_id);

	//
	//	Buy the item and return to the game
	//
	Purchase ();
	GameInitMgrClass::Continue_Game ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Merchandise_Selected
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMenuClass::On_Merchandise_Selected (MerchandiseCtrlClass * /* ctrl */, int ctrl_id)
{
	//
	//	Reset the shopping list to include just this one purchase
	//
	Clear_Shopping_Cart ();
	Add_Item_To_Shopping_Cart (ctrl_id);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Add_Item_To_Shopping_Cart
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMenuClass::Add_Item_To_Shopping_Cart (int ctrl_id)
{
	MerchandiseCtrlClass *ctrl = (MerchandiseCtrlClass *)Get_Dlg_Item (ctrl_id);
	if (ctrl == NULL) {
		return ;
	}

	int item_index = (int)ctrl->Get_User_Data ();

	//
	//	Can the player afford this item?
	//
	WWASSERT(COMBAT_STAR != NULL);
	WWASSERT(COMBAT_STAR->Get_Player_Data() != NULL);

	int funds	= (int) COMBAT_STAR->Get_Player_Data ()->Get_Money ();
	int cost		= Definition->Get_Cost (item_index) * CostScalingFactor;
	if ((TotalCost + cost) <= funds) {
		
		//
		//	Add this cost into our grand total
		//
		TotalCost += cost;

		//
		//	Add this to item to our list
		//
		ShoppingList.Add (ctrl_id);

		//
		//	Update the UI
		//
		Set_Dlg_Item_Int (IDC_CREDITS, funds - TotalCost);

		//
		//	Update the counter on the merchandise control
		//
		MerchandiseCtrlClass *ctrl = (MerchandiseCtrlClass *)Get_Dlg_Item (ITEM_CTRL_IDS[item_index]);
		if (ctrl != NULL) {
			ctrl->Increment_Purchase_Count ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Clear_Shopping_Cart
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMenuClass::Clear_Shopping_Cart (void)
{
	//
	//	Reset our data
	//
	TotalCost	= 0;
	
	WWASSERT(COMBAT_STAR != NULL);
	WWASSERT(COMBAT_STAR->Get_Player_Data() != NULL);

	int funds	= (int) COMBAT_STAR->Get_Player_Data ()->Get_Money ();
	ShoppingList.Delete_All ();

	//
	//	Update the UI
	//
	Set_Dlg_Item_Int (IDC_CREDITS, funds);

	//
	//	Reset the purchase counts
	//
	for (int index = 0; index < PURCHASE_ITEMS; index ++) {
		
		//
		//	Get the control for this entry
		//
		MerchandiseCtrlClass *ctrl = (MerchandiseCtrlClass *)Get_Dlg_Item (ITEM_CTRL_IDS[index]);
		if (ctrl != NULL) {
			ctrl->Reset_Purchase_Count ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Purchase
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMenuClass::Purchase (void)
{	
	bool allow_purchase = true;
	VendorClass::PURCHASE_TYPE vendor_purchase_type = VendorClass::TYPE_CHARACTER;

	switch (PurchaseType) 
	{
	case PurchaseSettingsDefClass::TYPE_CLASSES:
		vendor_purchase_type = VendorClass::TYPE_CHARACTER;
		allow_purchase = true;
		break;
	
	case PurchaseSettingsDefClass::TYPE_SECRET_CLASSES:
		vendor_purchase_type = VendorClass::TYPE_SECRET_CHARACTER;
		allow_purchase = true;
		break;
	
	case PurchaseSettingsDefClass::TYPE_VEHICLES:
		allow_purchase = Verify_Vehicle_Purchase();
		vendor_purchase_type = VendorClass::TYPE_VEHICLE;
		break;
	
	case PurchaseSettingsDefClass::TYPE_SECRET_VEHICLES:
		allow_purchase = Verify_Vehicle_Purchase();
		vendor_purchase_type = VendorClass::TYPE_SECRET_VEHICLE;
		break;
	};

	//
	//	Puchase each item
	//
	if (allow_purchase) {		
		for (int index = 0; index < ShoppingList.Count (); index ++) {
			WWASSERT (COMBAT_STAR != NULL);

			//
			//	Lookup the merchandise control we'll be purchasing
			//
			MerchandiseCtrlClass *ctrl = (MerchandiseCtrlClass *)Get_Dlg_Item (ShoppingList[index]);
			if (ctrl != NULL) {
				int item_index		= (int)ctrl->Get_User_Data ();
				int alt_index		= ctrl->Get_Selected_Alternate ();
				VendorClass::Purchase_Item (COMBAT_STAR, vendor_purchase_type, item_index, alt_index);
			}			
		}
	}

	//
	//	Close the dialog
	//
	End_Dialog ();
	return ;
}

////////////////////////////////////////////////////////////////
//
//	Verify_Vehicle_Purchase
// Checks whether the vehicle factory is available and able
// to build.  Pops up dialogs if anything goes wrong.
//
////////////////////////////////////////////////////////////////
bool
CNCPurchaseMenuClass::Verify_Vehicle_Purchase (void)
{
	bool allow_purchase = true;

	//
	//	Find the base for this user
	//
	BaseControllerClass *base = BaseControllerClass::Find_Base_For_Star ();		
	if (base != NULL) {

		//
		//	Find the vehicle factory
		//
		BuildingGameObj *building = base->Find_Building (BuildingConstants::TYPE_VEHICLE_FACTORY);
		if (building != NULL && building->As_VehicleFactoryGameObj () != NULL) {
			VehicleFactoryGameObj *factory = building->As_VehicleFactoryGameObj ();
				
			//
			//	Determine if the factory is busy or destroyed
			//
			if (factory->Is_Busy ()) {
				DlgMsgBox::DoDialog (IDS_CNC_CANT_PURCHASE_TITLE, IDS_CNC_VEHICLE_FACTORY_BUSY_MSG, DlgMsgBox::Okay, NULL, 0);
				allow_purchase = false;
			} else if (factory->Is_Destroyed ()) {
				DlgMsgBox::DoDialog (IDS_CNC_CANT_PURCHASE_TITLE, IDS_CNC_VEHICLE_FACTORY_DESTROYED_MSG, DlgMsgBox::Okay, NULL, 0);
				allow_purchase = false;
			} else if (factory->Get_Team_Vehicle_Count() >= factory->Get_Max_Vehicles_Per_Team()) {
				DlgMsgBox::DoDialog (IDS_CNC_CANT_PURCHASE_TITLE, IDS_CNC_VEHICLE_FACTORY_LIMIT_REACHED, DlgMsgBox::Okay, NULL, 0);
				allow_purchase = false;
			}
		}
	}

	return allow_purchase;
}

////////////////////////////////////////////////////////////////
//
//	Is_Definition_OK
//
////////////////////////////////////////////////////////////////
bool
CNCPurchaseMenuClass::Is_Definition_OK (DefinitionClass *definition)
{
	bool retval = false;
	
	if (definition != NULL) {
		retval = true;
		
		//
		//	Are we purchasing equipment?
		//
		if (PurchaseType == PurchaseSettingsDefClass::TYPE_EQUIPMENT) {
			int weapon_id = ((PowerUpGameObjDef *)definition)->Get_Grant_Weapon_ID ();
			
			//
			//	Is this a weapon we're purchasing?
			//
			DefinitionClass *weapon_def = DefinitionMgrClass::Find_Definition (weapon_id);
			if (weapon_def != NULL && weapon_def->Get_Class_ID () == CLASSID_DEF_WEAPON) {
				
				//
				//	Is this a beacon?
				//
				if (((WeaponDefinitionClass *)weapon_def)->Style == WEAPON_HOLD_STYLE_BEACON) {
					
					//
					//	Is this CnC mode?
					//
					/*
					if (The_Game () != NULL && The_Game ()->As_Cnc () != NULL) {
						cGameDataCnc *game_data = The_Game ()->As_Cnc ();
						
						//
						//	Don't allow the player to purchase beacons unless the
						// server settings allow it.
						//
						if (	game_data->BaseDestructionEndsGame.Is_False () ||
								game_data->BeaconPlacementEndsGame.Is_False ())
						{
							retval = false;
						}
					}
					*/
					if (The_Game () != NULL) {					

						//
						//	Don't allow the player to purchase beacons unless the
						// server settings allow it.
						//

						cGameDataCnc * game_cnc = The_Game ()->As_Cnc ();

						if (	game_cnc != NULL &&
							   (game_cnc->BaseDestructionEndsGame.Is_False () ||
								 game_cnc->BeaconPlacementEndsGame.Is_False ()))
						{
							retval = false;
						}

						cGameDataSkirmish * game_skirmish = The_Game ()->As_Skirmish ();

						if (	game_skirmish != NULL &&
							   (game_skirmish->BaseDestructionEndsGame.Is_False () ||
								 game_skirmish->BeaconPlacementEndsGame.Is_False ()))
						{
							retval = false;
						}
					}
				}
			}

		} else if (	PurchaseType == PurchaseSettingsDefClass::TYPE_VEHICLES ||
						PurchaseType == PurchaseSettingsDefClass::TYPE_SECRET_VEHICLES)
		{				
			//
			//	Lookup the physics object to determine if its valid to purchase
			// this type of object on this map
			//
			int phys_def_id = ((PhysicalGameObjDef *)definition)->Get_Phys_Def_ID ();
			PhysDefClass *phys_def = (PhysDefClass *)DefinitionMgrClass::Find_Definition (phys_def_id);
			if (phys_def != NULL) {
				
				//
				//	Don't allow flying vehicles on maps that aren't geared towards
				// flying vehicles...
				//
				if (	MapMgrClass::Are_VTOL_Vehicles_Enabled () == false && 
						phys_def->Get_Class_ID () == CLASSID_VTOLVEHICLEDEF)
				{
					retval = false;
				}					
			}
		}
	}	
	
	return retval;
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMenuClass::On_Frame_Update (void)
{
	if (COMBAT_STAR == NULL) {
		return ;
	}

	WWASSERT(COMBAT_STAR != NULL);
	WWASSERT(COMBAT_STAR->Get_Player_Data() != NULL);

	//
	//	Update the player's money every frame
	//
	int old_money = Get_Dlg_Item_Int (IDC_CREDITS);
	int new_money = (int) COMBAT_STAR->Get_Player_Data ()->Get_Money ();
	if (new_money != old_money) {
		Set_Dlg_Item_Int (IDC_CREDITS, new_money);
	}

	//
	//	Update each building's health once per second
	//
	HealthUpdateTimer -= TimeManager::Get_Frame_Seconds ();
	if (HealthUpdateTimer <= 0) {
		HealthUpdateTimer = 1.0F;
		Update_Building_Health ();
	}

	//
	//	Update the enabled state of each purchase control every half second
	//
	EnabledStateUpdateTimer -= TimeManager::Get_Frame_Seconds ();
	if (EnabledStateUpdateTimer <= 0) {
		EnabledStateUpdateTimer = 0.5F;
		Update_Enabled_Status ();
	}

	MenuDialogClass::On_Frame_Update ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Key_Down
//
////////////////////////////////////////////////////////////////
bool
CNCPurchaseMenuClass::On_Key_Down (uint32 key_id, uint32 key_data)
{
	bool retval = true;
	
	//
	//	Check to see if a hotkey was pressed
	//
	switch (key_id)
	{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			Purchase_Item (int(ITEM_CTRL_IDS[key_id - '1']));
			break;

		case '0':
			Purchase_Item (IDC_ITEM_10);
			break;

		default:
			retval = false;
			break;
	}

	//
	//	If we didn't process the key, then let the base class handle it
	//
	if (retval == false) {
		retval = MenuDialogClass::On_Key_Down (key_id, key_data);
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Update_Building_Health
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMenuClass::Update_Building_Health (void)
{
	BaseControllerClass *base = BaseControllerClass::Find_Base_For_Star ();
	if (base == NULL) {
		return ;
	}

	//
	//	Find the building for the requested factory
	//
	BuildingGameObj *building = NULL;
	if (PurchaseType == PurchaseSettingsDefClass::TYPE_CLASSES) {
		building = base->Find_Building (BuildingConstants::TYPE_SOLDIER_FACTORY);
	} else if (PurchaseType == PurchaseSettingsDefClass::TYPE_VEHICLES) {
		building = base->Find_Building (BuildingConstants::TYPE_VEHICLE_FACTORY);
	}

	//
	//	Update the factory
	//
	if (building != NULL) {
		float life = building->Get_Defense_Object ()->Get_Health () / building->Get_Defense_Object ()->Get_Health_Max ();
		((HealthBarCtrlClass *)Get_Dlg_Item (IDC_BUILDING01_HEALTHBAR))->Set_Life (life);
	}

	//
	//	Update the poweruplant
	//
	building = base->Find_Building (BuildingConstants::TYPE_POWER_PLANT);
	if (building != NULL) {
		float life = building->Get_Defense_Object ()->Get_Health () / building->Get_Defense_Object ()->Get_Health_Max ();
		((HealthBarCtrlClass *)Get_Dlg_Item (IDC_BUILDING02_HEALTHBAR))->Set_Life (life);

		//
		//	Show the cost doubled text
		//
		if (life <= 0) {
			Get_Dlg_Item (IDC_COST_X2_TEXT)->Show (true);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Configure_Building_Icons
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMenuClass::Configure_Building_Icons (void)
{
	StringClass fact_icon_name;
	StringClass pplant_icon_name	("HUD_C&C_G_POWER.TGA");

	if (Team == PlayerTerminalClass::TYPE_NOD) {
		pplant_icon_name = "HUD_C&C_N_POWER.TGA";

		//
		//	Determine which building icon to use...
		//
		if (PurchaseType == PurchaseSettingsDefClass::TYPE_CLASSES) {
			fact_icon_name = "HUD_C&C_N_HANDOF.TGA";
		} else if (PurchaseType == PurchaseSettingsDefClass::TYPE_VEHICLES) {
			fact_icon_name = "HUD_C&C_N_AIRSTRIP.TGA";
		}

	} else {
		
		//
		//	Determine which building icon to use...
		//
		if (PurchaseType == PurchaseSettingsDefClass::TYPE_CLASSES) {
			fact_icon_name = "HUD_C&C_G_BARRACKS.TGA";
		} else if (PurchaseType == PurchaseSettingsDefClass::TYPE_VEHICLES) {
			fact_icon_name = "HUD_C&C_G_WARFACT.TGA";
		}
	}

	//
	//	Assign immages to the building icons
	//
	((ImageCtrlClass *)Get_Dlg_Item (IDC_BUILDING01_ICON))->Set_Texture (fact_icon_name);
	((ImageCtrlClass *)Get_Dlg_Item (IDC_BUILDING02_ICON))->Set_Texture (pplant_icon_name);

	//
	//	Hide the cost doubled text
	//
	Get_Dlg_Item (IDC_COST_X2_TEXT)->Show (false);

	Update_Building_Health ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Enabled_Status
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMenuClass::Update_Enabled_Status (void)
{
	if (COMBAT_STAR == NULL || COMBAT_STAR->Get_Player_Data () == NULL) {
		return ;
	}

	PlayerDataClass *player_data = COMBAT_STAR->Get_Player_Data ();

	//
	//	Initialize the purchase controls
	//
	for (int index = 0; index < PURCHASE_ITEMS; index ++) {
		
		//
		//	Get the control for this entry
		//
		MerchandiseCtrlClass *ctrl = (MerchandiseCtrlClass *)Get_Dlg_Item (ITEM_CTRL_IDS[index]);
		if (ctrl != NULL) {

			//
			//	Is this slot available?
			//
			int definition_id					= Definition->Get_Definition (index);				
			DefinitionClass *definition	= DefinitionMgrClass::Find_Definition (definition_id);
			if (definition != NULL) {

				//
				//	Configure the merchandise settings
				//
				int cost = Definition->Get_Cost (index) * CostScalingFactor;

				//
				//	Disable any options that cost money if production is disabled
				//
				if ((IsProductionDisabled && (cost > 0)) || (player_data->Get_Money () < cost)) {
					ctrl->Enable (false);
				} else {
					ctrl->Enable (true);
				}
			}
		}
	}

	return ;
}
