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
 *                     $Archive:: /Commando/Code/Commando/dlgcncpurchasemainmenu.cpp          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/15/02 9:41p                                               $*
 *                                                                                             *
 *                    $Revision:: 21                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "renegadedialog.h"
#include "dlgcncpurchasemainmenu.h"
#include "dlgcncpurchasemenu.h"
#include "buttonctrl.h"
#include "globalsettings.h"
#include "combat.h"
#include "basecontroller.h"
#include "soldier.h"
#include "playertype.h"
#include "string_ids.h"
#include "vehiclefactorygameobj.h"
#include "soldierfactorygameobj.h"
#include "translatedb.h"
#include "assets.h"
#include "purchasesettings.h"
#include "dlgmpingamechat.h"
#include "gamemode.h"
#include "gameinitmgr.h"
#include "merchandisectrl.h"
#include "teampurchasesettings.h"
#include "vendor.h"
#include "playerdata.h"
#include "dialogmgr.h"
#include "combat.h"
#include "listctrl.h"
#include "messagewindow.h"
#include "weapons.h"
#include "weaponbag.h"
#include "powerup.h"
#include "input.h"
#include "dinput.h"
#include "hud.h"
#include "gamedata.h"
#include "specialbuilds.h"


bool CNCPurchaseMainMenuClass::SecretsEnabled = false;


////////////////////////////////////////////////////////////////
//
//	CNCPurchaseMainMenuClass
//
////////////////////////////////////////////////////////////////
CNCPurchaseMainMenuClass::CNCPurchaseMainMenuClass (void)	:
	Team (PurchaseSettingsDefClass::TEAM_GDI),
	ChatModule (NULL),
	RefreshTimer (0),
	MessageLogLength (0),
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_CNC_PURCHASE_MAIN_SCREEN))
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~CNCPurchaseMainMenuClass
//
////////////////////////////////////////////////////////////////
CNCPurchaseMainMenuClass::~CNCPurchaseMainMenuClass (void)
{
	GameInitMgrClass::Continue_Game ();
	REF_PTR_RELEASE (ChatModule);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMainMenuClass::On_Init_Dialog (void)
{	
	ButtonCtrlClass *chars_button			= (ButtonCtrlClass *)Get_Dlg_Item (IDC_CHARACTERS_BUTTON);
	ButtonCtrlClass *vehicles_button		= (ButtonCtrlClass *)Get_Dlg_Item (IDC_VEHICLES_BUTTON);

	//
	//	Get the global settings
	//
	GlobalSettingsDef *settings = GlobalSettingsDef::Get_Global_Settings ();

	StringClass chars_texture;
	StringClass vehicles_texture;

	//
	//	Determine which textures to use for the buttons
	//
	if (Team == PurchaseSettingsDefClass::TEAM_GDI) {
		::Strip_Path_From_Filename (chars_texture, settings->Get_Purchase_GDI_Characters_Texture ());
		::Strip_Path_From_Filename (vehicles_texture, settings->Get_Purchase_GDI_Vehicles_Texture ());
	} else if (Team == PurchaseSettingsDefClass::TEAM_NOD) {
		::Strip_Path_From_Filename (chars_texture, settings->Get_Purchase_NOD_Characters_Texture ());
		::Strip_Path_From_Filename (vehicles_texture, settings->Get_Purchase_NOD_Vehicles_Texture ());
	} else if (Team == PurchaseSettingsDefClass::TEAM_MUTANT_NOD) {
		::Strip_Path_From_Filename (chars_texture, settings->Get_Purchase_NOD_MUT_Characters_Texture ());
		::Strip_Path_From_Filename (vehicles_texture, settings->Get_Purchase_NOD_MUT_Vehicles_Texture ());
	} else {
		::Strip_Path_From_Filename (chars_texture, settings->Get_Purchase_GDI_MUT_Characters_Texture ());
		::Strip_Path_From_Filename (vehicles_texture, settings->Get_Purchase_GDI_MUT_Vehicles_Texture ());
	}

	//
	//	Configure the bitmap buttons
	//
	chars_button->Set_Bitmap (chars_texture, NULL);
	vehicles_button->Set_Bitmap (vehicles_texture, NULL);

	RefreshTimer = 0;

	Configure_Purchase_Controls ();

	//
	//	Get the rectangle where the chat UI will exist
	//
	const RectClass &chat_rect = Get_Dlg_Item (IDC_CHAT_AREA)->Get_Window_Rect ();

	//
	//	Insert the chat module dialog into the bottom portion of our window
	//
	ChatModule = new MPChatChildDialogClass;	
	ChatModule->Set_Default_Type (TEXT_MESSAGE_TEAM);
	ChatModule->Start_Dialog ();
	Add_Child_Dialog (ChatModule);
	ChatModule->Set_Rect (chat_rect);
	ChatModule->Set_End_Dialog_On_Send (false);	

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

	//
	//	Configure the message log list ctrl
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LIST_CTRL);
	if (list_ctrl != NULL) {
		list_ctrl->Add_Column (U_CHAR(""), 1.0F, Vector3 (1, 1, 1));
	}

	//
	//	Activate the menu game mode (if necessary)
	//
	GameModeClass *menu_game_mode = GameModeManager::Find ("Menu");
	if (menu_game_mode != NULL && menu_game_mode->Is_Active () == false) {
		menu_game_mode->Activate ();
	}

	MenuDialogClass::On_Init_Dialog ();

	//
	//	Force focus to be on the "characters" button
	//
	Get_Dlg_Item (IDC_CHARACTERS_BUTTON)->Set_Focus ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Destroy
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMainMenuClass::On_Destroy (void)
{
	MenuDialogClass::On_Destroy ();	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Configure_Purchase_Controls
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMainMenuClass::Configure_Purchase_Controls (void)
{
	const int CTRL_IDS[4] =
	{
		IDC_ENLISTED_PURCHASE_01,
		IDC_ENLISTED_PURCHASE_02,
		IDC_ENLISTED_PURCHASE_03,
		IDC_ENLISTED_PURCHASE_04
	};

	//
	//	Lookup the purchase settings for this team
	//
	TeamPurchaseSettingsDefClass *definition = NULL;
	definition = TeamPurchaseSettingsDefClass::Get_Definition ((TeamPurchaseSettingsDefClass::TEAM)Team);

	for (int index = 0; index < 4; index ++) {

		//
		//	Get the control for this entry
		//
		MerchandiseCtrlClass *ctrl = (MerchandiseCtrlClass *)Get_Dlg_Item (CTRL_IDS[index]);
		if (ctrl != NULL) {

			//
			//	Configure the merchandise settings
			//
			ctrl->Set_Text (definition->Get_Enlisted_Name (index));
			ctrl->Set_Cost (0);
			ctrl->Set_Texture (definition->Get_Enlisted_Texture (index));
			ctrl->Set_User_Data (index);
		}
	}

	//
	//	Configure the beacon purchase object
	//
	MerchandiseCtrlClass *ctrl = (MerchandiseCtrlClass *)Get_Dlg_Item (IDC_BEACON_PURCHASE);
	if (ctrl != NULL) {
		ctrl->Set_Text (definition->Get_Beacon_Name ());
		ctrl->Set_Cost (definition->Get_Beacon_Cost ());
		ctrl->Set_Texture (definition->Get_Beacon_Texture ());
	}

	//
	//	Configure the supply purchase object
	//
	ctrl = (MerchandiseCtrlClass *)Get_Dlg_Item (IDC_SUPPLY_PURCHASE);
	if (ctrl != NULL) {
		ctrl->Set_Text (definition->Get_Supply_Name ());
		ctrl->Set_Cost (0);
		ctrl->Set_Texture (definition->Get_Supply_Texture ());
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMainMenuClass::Render (void)
{
	MenuDialogClass::Render ();
	HUDClass::Damage_Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMainMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	if (SecretsEnabled && The_Game()->IsLaddered.Is_True()) {
		SecretsEnabled = false;
	}

	switch (ctrl_id)
	{
		case IDC_BUY:
			Purchase ();
			break;

		case IDC_CHARACTERS_BUTTON:
#ifdef MULTIPLAYERDEMO
				Do_Purchase_Screen (PurchaseSettingsDefClass::TYPE_CLASSES);
#else
			if ((SecretsEnabled) && (Input::Is_Button_Down(DIK_LMENU))) {
				Do_Purchase_Screen (PurchaseSettingsDefClass::TYPE_SECRET_CLASSES);
			} else {
				Do_Purchase_Screen (PurchaseSettingsDefClass::TYPE_CLASSES);
			}
#endif
			break;

		case IDC_VEHICLES_BUTTON:
#ifdef MULTIPLAYERDEMO
				Do_Purchase_Screen (PurchaseSettingsDefClass::TYPE_VEHICLES);
#else
			if ((SecretsEnabled) && (Input::Is_Button_Down(DIK_LMENU))) {
				Do_Purchase_Screen (PurchaseSettingsDefClass::TYPE_SECRET_VEHICLES);
			} else {
				Do_Purchase_Screen (PurchaseSettingsDefClass::TYPE_VEHICLES);
			}
#endif
			break;

		default:
			break;
	}

	MenuDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Do_Purchase_Screen
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMainMenuClass::Do_Purchase_Screen (PurchaseSettingsDefClass::TYPE type)
{
	//
	//	Lookup the definition based on the type and team
	//
	PurchaseSettingsDefClass *definition = NULL;
	definition = PurchaseSettingsDefClass::Find_Definition (type, Team);

	if (definition != NULL) {
		
		//
		//	Show the purchase dialog
		//
		CNCPurchaseMenuClass *dialog = new CNCPurchaseMenuClass;
		dialog->Set_Definition (definition);
		dialog->Set_Type (type);
		
		//
		//	Let the dialog know what "team" this purchase is for...
		//
		if (Team == PurchaseSettingsDefClass::TEAM_GDI) {
			dialog->Set_Team (PlayerTerminalClass::TYPE_GDI);
		} else if (Team == PurchaseSettingsDefClass::TEAM_NOD) {
			dialog->Set_Team (PlayerTerminalClass::TYPE_NOD);
		} else {
			dialog->Set_Team (PlayerTerminalClass::TYPE_MUTANT);
		}		

		//
		//	Examine the base for this player to decide what purchase options they have
		//
		BaseControllerClass *base = BaseControllerClass::Find_Base_For_Star ();
		if (base != NULL) {
						
			//
			//	If the base is powered down then all prices double in cost
			//
			if (base->Is_Base_Powered () == false) {
				dialog->Set_Cost_Scaling_Factor (2.0F);
			}

			//
			//	If the base can't generate new soldiers, then let the dialog know that
			//	only "free" items are available
			//
			if (type == PurchaseSettingsDefClass::TYPE_CLASSES && base->Can_Generate_Soldiers () == false) {
				dialog->Enable_Production (false);
			}
		}

		//
		//	Disable multiple purchases for vehicles and soldiers
		//
		if (	type == PurchaseSettingsDefClass::TYPE_CLASSES ||
				type == PurchaseSettingsDefClass::TYPE_VEHICLES)
		{
			dialog->Enable_Multiple_Purchases (false);
		}

		dialog->Start_Dialog ();
		REF_PTR_RELEASE (dialog);

		//
		//	Reset any selections on this page
		//
		Clear_Selections ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Clear_Selections
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMainMenuClass::Clear_Selections (void)
{
	const int CTRL_IDS[] =
	{
		IDC_ENLISTED_PURCHASE_01,
		IDC_ENLISTED_PURCHASE_02,
		IDC_ENLISTED_PURCHASE_03,
		IDC_ENLISTED_PURCHASE_04,
		IDC_SUPPLY_PURCHASE,
		IDC_BEACON_PURCHASE
	};

	//
	//	Find out which options are selected for purchase
	//
	for (int index = 0; index < 6; index ++) {
		MerchandiseCtrlClass *ctrl = (MerchandiseCtrlClass *)Get_Dlg_Item (CTRL_IDS[index]);
		if (ctrl != NULL && ctrl->Get_Purchase_Count () > 0) {
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
CNCPurchaseMainMenuClass::Purchase (void)
{
	const int CTRL_IDS[] =
	{
		IDC_ENLISTED_PURCHASE_01,
		IDC_ENLISTED_PURCHASE_02,
		IDC_ENLISTED_PURCHASE_03,
		IDC_ENLISTED_PURCHASE_04,
		IDC_SUPPLY_PURCHASE,
		IDC_BEACON_PURCHASE
	};

	//
	//	Lookup the purchase settings for this team
	//
	TeamPurchaseSettingsDefClass *definition = NULL;
	definition = TeamPurchaseSettingsDefClass::Get_Definition ((TeamPurchaseSettingsDefClass::TEAM)Team);

	//
	//	Find out which options are selected for purchase
	//
	for (int index = 0; index < 6; index ++) {
		MerchandiseCtrlClass *ctrl = (MerchandiseCtrlClass *)Get_Dlg_Item (CTRL_IDS[index]);
		if (ctrl != NULL && ctrl->Get_Purchase_Count () > 0) {
			
			switch (CTRL_IDS[index])
			{				
				case IDC_ENLISTED_PURCHASE_01:
				case IDC_ENLISTED_PURCHASE_02:
				case IDC_ENLISTED_PURCHASE_03:
				case IDC_ENLISTED_PURCHASE_04:
				{
					VendorClass::Purchase_Item (COMBAT_STAR, VendorClass::TYPE_ENLISTED_CHARACTER, ctrl->Get_User_Data ());
					break;
				}

				case IDC_SUPPLY_PURCHASE:
					VendorClass::Purchase_Item (COMBAT_STAR, VendorClass::TYPE_SUPPLY, 0);
					break;

				case IDC_BEACON_PURCHASE:
					VendorClass::Purchase_Item (COMBAT_STAR, VendorClass::TYPE_BEACON, 0);
					break;								
			}
		}
	}	

	//
	//	Resume play after a purchase
	//
	End_Dialog ();		
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Merchandise_Selected
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMainMenuClass::On_Merchandise_Selected (MerchandiseCtrlClass *ctrl, int ctrl_id)
{
	const int CTRL_IDS[] =
	{
		IDC_ENLISTED_PURCHASE_01,
		IDC_ENLISTED_PURCHASE_02,
		IDC_ENLISTED_PURCHASE_03,
		IDC_ENLISTED_PURCHASE_04
	};

	//
	//	Update the counter on the merchandise control
	//
	if (ctrl != NULL) {

		if (ctrl->Get_Purchase_Count () == 0) {
			ctrl->Increment_Purchase_Count ();

			//
			//	Check to ensure we don't select more then one of the
			// enlisted controls.
			//
			switch (ctrl_id)
			{				
				case IDC_ENLISTED_PURCHASE_01:
				case IDC_ENLISTED_PURCHASE_02:
				case IDC_ENLISTED_PURCHASE_03:
				case IDC_ENLISTED_PURCHASE_04:
				{
					//
					//	Make the enlisted purchases mutually exclusive
					//
					for (int index = 0; index < 4; index ++) {
						MerchandiseCtrlClass *other_ctrl = (MerchandiseCtrlClass *)Get_Dlg_Item (CTRL_IDS[index]);
						if (other_ctrl != NULL && other_ctrl != ctrl) {
							other_ctrl->Reset_Purchase_Count ();
						}
					}
					break;
				}

			}

		} else {
			ctrl->Reset_Purchase_Count ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Merchandise_DblClk
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMainMenuClass::On_Merchandise_DblClk (MerchandiseCtrlClass * /* ctrl */, int ctrl_id)
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
CNCPurchaseMainMenuClass::Purchase_Item (int ctrl_id)
{
	const int CTRL_IDS[] =
	{
		IDC_ENLISTED_PURCHASE_01,
		IDC_ENLISTED_PURCHASE_02,
		IDC_ENLISTED_PURCHASE_03,
		IDC_ENLISTED_PURCHASE_04,
		IDC_SUPPLY_PURCHASE,
		IDC_BEACON_PURCHASE
	};

	//
	//	Reset all purchase controls
	//
	for (int index = 0; index < 6; index ++) {
		MerchandiseCtrlClass *ctrl = (MerchandiseCtrlClass *)Get_Dlg_Item (CTRL_IDS[index]);
		if (ctrl != NULL) {
			ctrl->Reset_Purchase_Count ();
		}
	}	

	//
	//	Purchase the item and close the dialog
	//
	MerchandiseCtrlClass *ctrl = (MerchandiseCtrlClass *)Get_Dlg_Item (ctrl_id);
	if (ctrl != NULL) {		
		ctrl->Increment_Purchase_Count ();
		Purchase ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMainMenuClass::On_Frame_Update (void)
{
	if (COMBAT_STAR == NULL) {
		return ;
	}

	//
	//	Update the player's money every frame
	// TSS120301 - I'm not sure how player data can be null here but we are crashing 
	// with that, so let's test against it.
	//
	if (COMBAT_STAR->Get_Player_Data () != NULL) {
		int old_money = Get_Dlg_Item_Int (IDC_CREDITS_TEXT);
		int new_money = (int) COMBAT_STAR->Get_Player_Data ()->Get_Money ();
		if (new_money != old_money) {
			Set_Dlg_Item_Int (IDC_CREDITS_TEXT, new_money);
		}
	}

	//
	//	Check to see if we should refresh the purchase button states
	//
	RefreshTimer -= DialogMgrClass::Get_Frame_Time ();
	if (RefreshTimer <= 0) {
		RefreshTimer = 0.5F;
		Refresh_Button_States ();
		Refresh_Beacon_State ();
	}

	//
	//	Update the message log every frame
	//
	Refresh_Message_Log ();

	MenuDialogClass::On_Frame_Update ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Refresh_Beacon_State
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMainMenuClass::Refresh_Beacon_State (void)
{
	if (COMBAT_STAR == NULL) {
		return ;
	}

	//
	//	Lookup the purchase settings for this team
	//
	TeamPurchaseSettingsDefClass *definition = NULL;
	definition = TeamPurchaseSettingsDefClass::Get_Definition ((TeamPurchaseSettingsDefClass::TEAM)Team);

	//
	//	Get the beacon cost and the player's current cash supply
	//
	float cost	= definition->Get_Beacon_Cost ();
	int funds	= 0;
	if (COMBAT_STAR->Get_Player_Data() != NULL) {
		funds	= (int) COMBAT_STAR->Get_Player_Data ()->Get_Money ();
	}

	//
	//	Enabled or disable the beacon ctrl as necessary
	//
	if (funds >= cost) {
		bool enable = true;

		//
		//	Get the player's weapon bag
		//
		WeaponBagClass *weapon_bag = COMBAT_STAR->Get_Weapon_Bag ();
		if (weapon_bag != NULL) {

			//
			//	Beacons are purchased via a powerup, so lookup the powerup
			// that grants the beacon weapon so we can check to ensure
			// the player isn't already maxed out.
			//
			PowerUpGameObjDef *powerup_def = (PowerUpGameObjDef *)DefinitionMgrClass::Find_Definition (definition->Get_Beacon_Definition ());
			if (powerup_def != NULL) {

				//
				//	Loop over all the weapons in the bag
				//
				int beacon_id = powerup_def->Get_Grant_Weapon_ID ();
				int weapon_count = weapon_bag->Get_Count ();
				for (int index = 0; index < weapon_count; index ++) {
					WeaponClass *weapon = weapon_bag->Peek_Weapon (index);
					if (weapon != NULL && weapon->Get_ID () == beacon_id) {
						
						//
						//	Don't allow the player to purchase more beacons then
						// they can hold.
						//
						if (weapon->Is_Ammo_Maxed ()) {
							enable = false;
						}
						break;
					}
				}
			}
		}

		Get_Dlg_Item (IDC_BEACON_PURCHASE)->Enable (enable);
	} else {
		Get_Dlg_Item (IDC_BEACON_PURCHASE)->Enable (false);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Refresh_Button_States
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMainMenuClass::Refresh_Button_States (void)
{
	ButtonCtrlClass *chars_button			= (ButtonCtrlClass *)Get_Dlg_Item (IDC_CHARACTERS_BUTTON);
	ButtonCtrlClass *vehicles_button		= (ButtonCtrlClass *)Get_Dlg_Item (IDC_VEHICLES_BUTTON);

	//
	//	Determine whether or not the player can purchase vehicles or characters
	//
	bool enable_vehicles = true;
	bool enable_chars = true;

	//
	//	Check to see if the player can generate vehicles.  If not, then disable
	// the vehicles button
	//
	BaseControllerClass *base_controller = BaseControllerClass::Find_Base_For_Star ();
	if (base_controller != NULL) {

		//
		//	Try to find the vehicle factory for this level
		//
		BuildingGameObj *building = base_controller->Find_Building (BuildingConstants::TYPE_VEHICLE_FACTORY);
		if (building != NULL && building->As_VehicleFactoryGameObj () != NULL) {
			VehicleFactoryGameObj *factory = building->As_VehicleFactoryGameObj ();
				
			//
			//	Determine if the factory is busy or destroyed
			//
			if (factory->Is_Busy ()) {
				Set_Dlg_Item_Text (IDC_VEHICLES_STATIC, TRANSLATE (IDS_CNC_PURCHASE_VB_BUSY));
				enable_vehicles = false;
			} else if (factory->Is_Destroyed ()) {
				Set_Dlg_Item_Text (IDC_VEHICLES_STATIC, TRANSLATE (IDS_CNC_PURCHASE_VB_DESTROYED));
				enable_vehicles = false;
			} else {
				Set_Dlg_Item_Text (IDC_VEHICLES_STATIC, TRANSLATE (IDS_MENU_VEHICLES));
			}

		} else {

			//
			//	Let the user know that there is now vehicle factory
			//
			Set_Dlg_Item_Text (IDC_VEHICLES_STATIC, TRANSLATE (IDS_CNC_PURCHASE_VB_UNAVAILABLE));
			enable_vehicles = false;
		}

		//
		//	Try to find the vehicle factory for this level
		//
		building = base_controller->Find_Building (BuildingConstants::TYPE_SOLDIER_FACTORY);
		if (building != NULL && building->As_SoldierFactoryGameObj () != NULL) {
			SoldierFactoryGameObj *factory = building->As_SoldierFactoryGameObj ();
				
			//
			//	Determine if the factory is busy or destroyed
			//
			if (factory->Is_Destroyed ()) {
				Set_Dlg_Item_Text (IDC_SOLDIERS_STATIC, TRANSLATE (IDS_CNC_PURCHASE_VB_DESTROYED));
				enable_chars = false;
			} else {
				Set_Dlg_Item_Text (IDC_SOLDIERS_STATIC, TRANSLATE (IDS_MENU_CHARACTERS));
			}

		} else {

			//
			//	Let the user know that there is no soldier factory
			//
			Set_Dlg_Item_Text (IDC_SOLDIERS_STATIC, TRANSLATE (IDS_CNC_PURCHASE_VB_UNAVAILABLE));
			enable_chars = false;
		}
	}

	//
	//	Check to see if this team has reached its limit on vehicles.
	//
	if (enable_vehicles) {
		
		//
		//	Count team vehicles.
		//
		BaseControllerClass *base = BaseControllerClass::Find_Base_For_Star ();		
		
		if (base != NULL) {
			
			//
			// Find the vehicle factory
			//
			BuildingGameObj *building = base->Find_Building (BuildingConstants::TYPE_VEHICLE_FACTORY);
		
			if ((building != NULL) && (building->As_VehicleFactoryGameObj())) {
				
				// 
				// See if the team already maxed out the number of vehicles
				//
				VehicleFactoryGameObj *factory = building->As_VehicleFactoryGameObj ();
				if (factory->Get_Team_Vehicle_Count() >= factory->Get_Max_Vehicles_Per_Team()) {			
					Set_Dlg_Item_Text (IDC_VEHICLES_STATIC, TRANSLATE (IDS_MENU_LIMIT_REACHED));
					enable_vehicles = false;
				}
			}
		}
	}

	//
	//	Enable or disable the vehicles and characters button
	//
	chars_button->Enable (enable_chars);
	vehicles_button->Enable (enable_vehicles);	
	Get_Dlg_Item (IDC_SOLDIERS_STATIC)->Enable (enable_chars);
	Get_Dlg_Item (IDC_VEHICLES_STATIC)->Enable (enable_vehicles);		
	Get_Dlg_Item (IDC_HOTKEY_TEXT_06)->Set_Text_Color (enable_chars ? Vector3 (1.0F, 1.0F, 1.0F) : Vector3 (0.5F, 0.5F, 0.5F));
	Get_Dlg_Item (IDC_HOTKEY_TEXT_07)->Set_Text_Color (enable_vehicles ? Vector3 (1.0F, 1.0F, 1.0F) : Vector3 (0.5F, 0.5F, 0.5F));
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Key_Down
//
////////////////////////////////////////////////////////////////
bool
CNCPurchaseMainMenuClass::On_Key_Down (uint32 key_id, uint32 key_data)
{
	bool retval = false;
	
	//
	//	Don't process hot keys if an edit control has the focus
	//
	DialogControlClass *focus_ctrl = DialogMgrClass::Get_Focus ();
	if (focus_ctrl == NULL || focus_ctrl->As_EditCtrlClass () == NULL) {

		//
		//	Check to see if a hotkey was pressed
		//
		retval = true;
		switch (key_id)
		{
			case '1':
				Purchase_Item (IDC_ENLISTED_PURCHASE_01);
				break;

			case '2':
				Purchase_Item (IDC_ENLISTED_PURCHASE_02);
				break;

			case '3':
				Purchase_Item (IDC_ENLISTED_PURCHASE_03);
				break;

			case '4':
				Purchase_Item (IDC_ENLISTED_PURCHASE_04);
				break;

			case '5':
				Purchase_Item (IDC_SUPPLY_PURCHASE);
				break;

			case '6':
				if (Get_Dlg_Item (IDC_CHARACTERS_BUTTON)->Is_Enabled ()) {
					Do_Purchase_Screen (PurchaseSettingsDefClass::TYPE_CLASSES);
				}
				break;

			case '7':
				if (Get_Dlg_Item (IDC_VEHICLES_BUTTON)->Is_Enabled ()) {
					Do_Purchase_Screen (PurchaseSettingsDefClass::TYPE_VEHICLES);
				}
				break;

			case '8':
				if (Get_Dlg_Item (IDC_BEACON_PURCHASE)->Is_Enabled ()) {
					Purchase_Item (IDC_BEACON_PURCHASE);
				}
				break;

			default:
				retval = false;
				break;
		}
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
//	Refresh_Message_Log
//
////////////////////////////////////////////////////////////////
void
CNCPurchaseMainMenuClass::Refresh_Message_Log (void)
{
	ListCtrlClass *list_ctrl				= (ListCtrlClass *)Get_Dlg_Item (IDC_LIST_CTRL);
	MessageWindowClass *message_window	= CombatManager::Get_Message_Window ();
	if (list_ctrl == NULL || message_window == NULL) {
		return ;
	}

	//
	//	Don't do anything if the log hasn't changed size...
	//
	int count = message_window->Get_Log_Count ();
	if (count != MessageLogLength) {
		MessageLogLength = count;

		//
		//	Start fresh
		//
		list_ctrl->Delete_All_Entries ();

		//
		//	Add all the messages to the log...
		//
		for (int index = 0; index < count; index ++) {
			
			//
			//	Get the message to display
			//
			const unichar_t *message = message_window->Get_Log_Entry (index);
			const Vector3 &color	= message_window->Get_Log_Color (index);
			WideStringClass temp_string (message, true);
			int len = temp_string.Get_Length ();
			
			//
			//	Strip off any unnecessary newlines
			//
			if (len > 0 && message[len - 1] == U_CHAR('\n')) {
				temp_string.Erase (len - 1, 1);
			}

			//
			//	Add this message to the log
			//
			int item_index = list_ctrl->Insert_Entry (index, temp_string);
			if (item_index != -1) {
				list_ctrl->Set_Entry_Color (item_index, 0, color);				
			}
		}

		//
		//	Scroll to the end of the message log
		//.
		list_ctrl->Scroll_To_End ();
	}

	return ;
}
