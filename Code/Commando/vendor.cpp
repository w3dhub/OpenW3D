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
 *                 Project Name : Commmando																	  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/vendor.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/28/02 11:17a                                               $*
 *                                                                                             *
 *                    $Revision:: 23                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "vendor.h"
#include "gameobjmanager.h"
#include "playerdata.h"
#include "soldier.h"
#include "playertype.h"
#include "combat.h"
#include "powerup.h"
#include "refinerygameobj.h"
#include "vehiclefactorygameobj.h"
#include "definitionmgr.h"
#include "vehicle.h"
#include "combatchunkid.h"
#include "cnetwork.h"
#include "purchaserequestevent.h"
#include "purchasesettings.h"
#include "teampurchasesettings.h"
#include "weaponbag.h"
#include "weapons.h"
#include "purchaseresponseevent.h"


////////////////////////////////////////////////////////////////
//	Namespaces
////////////////////////////////////////////////////////////////
using namespace BuildingConstants;


////////////////////////////////////////////////////////////////
//
//	Purchase_Vehicle
//
////////////////////////////////////////////////////////////////
VendorClass::PURCHASE_ERROR	
VendorClass::Purchase_Vehicle
(
	BaseControllerClass *	base, 
	SoldierGameObj *			player,
	int							cost,
	int							vehicle_id
)
{
	PURCHASE_ERROR retval = PERR_NOT_IN_STOCK;

	//
	//	Check to see if this vehicle is available to build
	//
	VehicleGameObjDef *definition = NULL;
	definition = (VehicleGameObjDef *)DefinitionMgrClass::Find_Definition (vehicle_id);
	if (definition != NULL) {

		PlayerDataClass *player_data	= NULL;
		bool has_funds						= true;
		
		//
		//	Check to see if the player has sufficient funds to purchase the vehicle
		//
		if (player != NULL && player->Get_Player_Data () != NULL) {
			player_data	= player->Get_Player_Data ();	
			has_funds	= (player_data->Get_Money () >= cost);
		}

		if (has_funds) {
			retval = PERR_NO_FACTORY;

			//
			//	Find our vehicle factory
			//
			BuildingGameObj *building = base->Find_Building (TYPE_VEHICLE_FACTORY);
			if (building != NULL) {
				VehicleFactoryGameObj *factory = building->As_VehicleFactoryGameObj ();

				//
				//	If we have a vehicle factory that isn't busy, then start building the vehicle
				//
				if (factory->Is_Available ()) {			
					float time = 5.0F * base->Get_Operation_Time_Factor ();
					factory->Request_Vehicle (vehicle_id, time, player);
					retval = PERR_SUCCESS;

					//
					//	If a player is purcahsing the vehicle, then debit
					// the player's account
					//
					if (player_data != NULL) {
						player_data->Purchase_Item (cost);
					}
				}
			}
		} else {
			retval = PERR_NO_FUNDS;
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Purchase_Powerup
//
////////////////////////////////////////////////////////////////
VendorClass::PURCHASE_ERROR
VendorClass::Purchase_Powerup
(
	BaseControllerClass *	/* base */, 
	SoldierGameObj *			player,
	int							cost,
	int							powerup_id
)
{
	PURCHASE_ERROR retval = PERR_NO_FUNDS;

	//
	//	Sanity check
	//
	if (player == NULL) {
		return retval;
	}

	//
	//	Lookup the powerup's definition
	//
	DefinitionClass *definition = DefinitionMgrClass::Find_Definition (powerup_id);
	if (definition != NULL && definition->Get_Class_ID () == CLASSID_GAME_OBJECT_DEF_POWERUP) {
		PowerUpGameObjDef *powerup_def = reinterpret_cast<PowerUpGameObjDef *> (definition);

		//
		//	If a player is purcahsing the powerup, then debit
		// the player's account
		//
		bool has_funds = false;
		PlayerDataClass *player_data = player->Get_Player_Data ();
		if (player_data != NULL) {

			//
			//	Try to purchase the powerup
			//
			has_funds = player_data->Purchase_Item (cost);
		}			

		//
		//	If the player has the cash, then grant him the powerup
		//
		if (has_funds) {
			powerup_def->Grant (player);
			retval = PERR_SUCCESS;
		}
		
	} else {
		retval = PERR_NOT_IN_STOCK;
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Purchase_Character
//
////////////////////////////////////////////////////////////////
VendorClass::PURCHASE_ERROR	
VendorClass::Purchase_Character
(
	BaseControllerClass *	base, 
	SoldierGameObj *			player,
	int							cost,
	int							definition_id
)
{
	PURCHASE_ERROR retval = PERR_NO_FUNDS;

	//
	//	Sanity check
	//
	if (player == NULL) {
		return retval;
	}

	//
	//	Check to see if the player has sufficient funds to purchase the upgrade
	//
	PlayerDataClass *player_data = player->Get_Player_Data ();
	bool has_funds = (player_data->Get_Money () >= cost);
	if (has_funds) {
		retval = PERR_NO_FACTORY;

		//
		//	Check to see if our soldier factory is operational
		//
		BuildingGameObj *building = base->Find_Building (TYPE_SOLDIER_FACTORY);
		if ((building != NULL && building->Is_Destroyed () == false) || cost == 0) {
		
			//
			//	Lookup the new definition for the soldier
			//
			DefinitionClass *definition = DefinitionMgrClass::Find_Definition (definition_id);
			if (definition != NULL && definition->Get_Class_ID () == CLASSID_GAME_OBJECT_DEF_SOLDIER) {
				
				//
				//	Upgrade the player
				//							
				SoldierGameObjDef *soldier_def = reinterpret_cast<SoldierGameObjDef *> (definition);
				player->Re_Init (*soldier_def);
				player->Post_Re_Init();
				retval = PERR_SUCCESS;

				//
				//	Debit the player's account
				//
				player_data->Purchase_Item (cost);
			}
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Purchase_Powerup
//
////////////////////////////////////////////////////////////////
VendorClass::PURCHASE_ERROR
VendorClass::Purchase_Item
(
	SoldierGameObj *	player,
	PURCHASE_TYPE		type, 
	int					item_index,
	int					alt_skin_index,
	bool					is_from_server
)
{
	PURCHASE_ERROR retval = PERR_NO_FUNDS;

	//
	//	Sanity check
	//
	if (player == NULL) {
		return retval;
	}

	if (CombatManager::I_Am_Server () == false) {		
		
		//
		//	Request this purchase from the server
		//
		cPurchaseRequestEvent *purchase_request = new cPurchaseRequestEvent;
		purchase_request->Init (type, item_index, alt_skin_index);

		//
		//	Let the user know the operation is pending
		//
		retval = PERR_OPERATION_PENDING;
	
	} else {

		//
		//	Determine which base controller to purchase from
		//
		BaseControllerClass *base = NULL;
		if (player->Get_Player_Type () == PLAYERTYPE_NOD) {
			base = BaseControllerClass::Find_Base ( PLAYERTYPE_NOD );
		} else {
			base = BaseControllerClass::Find_Base ( PLAYERTYPE_GDI );
		}

		//
		//	Lookup information about this purchase
		//
		int cost					= 0;
		int definition_id		= 0;
		Get_Merchandise_Information (player, type, item_index, alt_skin_index, cost, definition_id);

		//
		//	Cost is doubled if the base isn't powered
		//
		//if (base != NULL && base->Is_Base_Powered () == false) {
		if (type != TYPE_BEACON && base != NULL && base->Is_Base_Powered () == false) {
			cost = cost * 2;
		}

		if (type == TYPE_CHARACTER || type == TYPE_ENLISTED_CHARACTER || type == TYPE_SECRET_CHARACTER) {

			//
			//	Purchase the character
			//
			retval = Purchase_Character (base, player, cost, definition_id);			
		} else if (type == TYPE_VEHICLE || type == TYPE_SECRET_VEHICLE) {
			
			//
			//	Purchase the vehicle
			//
			retval = Purchase_Vehicle (base, player, cost, definition_id);
		} else if (type == TYPE_BEACON) {

			//
			//	Purchase the beacon powerup
			//
			retval = Purchase_Powerup (base, player, cost, definition_id);
		} else if (type == TYPE_SUPPLY) {
			
			//
			//	Grant full health, armor and ammo
			//
			Grant_Supplies (player);
			retval = PERR_SUCCESS;
		}

		//
		//	Send the response to the server locally as necessary
		//
		if (is_from_server) {
			cPurchaseResponseEvent *event = new cPurchaseResponseEvent;
			event->Init ((int)retval, cNetwork::Get_My_Id ());
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Grant_Supplies
//
////////////////////////////////////////////////////////////////
void
VendorClass::Grant_Supplies (SoldierGameObj *player)
{
	//
	//	Grant ammo
	//
	WeaponBagClass *weapon_bag = player->Get_Weapon_Bag ();
	for (int weapon_index = 0; weapon_index < weapon_bag->Get_Count (); weapon_index ++) {
		WeaponClass	*weapon = weapon_bag->Peek_Weapon (weapon_index);
		if (weapon != NULL && weapon->Get_Definition ()->CanReceiveGenericCnCAmmo) {

			//
			//	Restore full ammo
			//
			weapon->Set_Inventory_Rounds (weapon->Get_Definition ()->MaxInventoryRounds);
			weapon->Set_Clip_Rounds (weapon->Get_Definition ()->ClipSize);
		}
	}

	//
	//	Grant full health and armor
	//
	DefenseObjectClass *defense_obj = player->Get_Defense_Object ();
	defense_obj->Set_Health (defense_obj->Get_Health_Max ());
	defense_obj->Set_Shield_Strength (defense_obj->Get_Shield_Strength_Max ());	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Merchandise_Information
//
////////////////////////////////////////////////////////////////
void
VendorClass::Get_Merchandise_Information
(
	SoldierGameObj *	player,
	PURCHASE_TYPE		type,
	int					item_index,
	int					alt_skin_index,
	int &					cost,
	int &					definition_id
)
{
	//
	//	Determine which team to use...
	//
	PurchaseSettingsDefClass::TEAM team = PurchaseSettingsDefClass::TEAM_GDI;
	if (player->Get_Player_Type () == PLAYERTYPE_NOD) {
		team = PurchaseSettingsDefClass::TEAM_NOD;
	}

	//
	//	Lookup the definition
	//
	if (type == TYPE_CHARACTER || type == TYPE_VEHICLE) {
		
		//
		//	Determine which purchase type to use
		//
		PurchaseSettingsDefClass::TYPE purchase_type = PurchaseSettingsDefClass::TYPE_CLASSES;
		if (type == TYPE_VEHICLE) {
			purchase_type = PurchaseSettingsDefClass::TYPE_VEHICLES;
		}

		//
		//	Lookup the information from this purchase definition
		//
		PurchaseSettingsDefClass *definition = PurchaseSettingsDefClass::Find_Definition (purchase_type, team);
		if (definition != NULL) {
			cost = definition->Get_Cost (item_index);

			//
			//	Either get the default skin or the alternate skin
			//
			if (alt_skin_index != -1) {
				definition_id	= definition->Get_Alt_Definition (item_index, alt_skin_index);
			} else {
				definition_id	= definition->Get_Definition (item_index);
			}
		}

	} else if (type == TYPE_SECRET_CHARACTER || type == TYPE_SECRET_VEHICLE) {
	
		//
		// Only allow cheats in non-laddered games!
		//
		if (The_Game()->IsLaddered.Is_False()) {
			
			//
			//	Determine which purchase type to use
			//
			PurchaseSettingsDefClass::TYPE purchase_type = PurchaseSettingsDefClass::TYPE_SECRET_CLASSES;
			if (type == TYPE_SECRET_VEHICLE) {
				purchase_type = PurchaseSettingsDefClass::TYPE_SECRET_VEHICLES;
			}

			//
			//	Lookup the information from this purchase definition
			//
			PurchaseSettingsDefClass *definition = PurchaseSettingsDefClass::Find_Definition (purchase_type, team);
			if (definition != NULL) {
				cost				= definition->Get_Cost (item_index);
				definition_id	= definition->Get_Definition (item_index);
			}
		}
		
	} else if (type == TYPE_ENLISTED_CHARACTER) {

		//
		//	For enlisted characters, lookup the team purchase definition
		//
		cost = 0;
		TeamPurchaseSettingsDefClass *definition = TeamPurchaseSettingsDefClass::Get_Definition ((TeamPurchaseSettingsDefClass::TEAM)team);
		if (definition != NULL) {
			definition_id = definition->Get_Enlisted_Definition (item_index);
		}

	} else if (type == TYPE_BEACON) {

		//
		//	For beacons, lookup the team purchase definition
		//
		TeamPurchaseSettingsDefClass *definition = TeamPurchaseSettingsDefClass::Get_Definition ((TeamPurchaseSettingsDefClass::TEAM)team);
		if (definition != NULL) {
			cost				= definition->Get_Beacon_Cost ();
			definition_id	= definition->Get_Beacon_Definition ();
		}
	}

	return ;
}
