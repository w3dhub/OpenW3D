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
 *                     $Archive:: /Commando/Code/Combat/basecontroller.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/29/01 11:58a                                              $*
 *                                                                                             *
 *                    $Revision:: 31                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "basecontroller.h"
#include "gameobjmanager.h"
#include "scriptzone.h"
#include "radar.h"
#include "vehicle.h"
#include "rendobj.h"
#include "crandom.h"
#include "playerdata.h"
#include "soldier.h"
#include "playertype.h"
#include "combat.h"
#include "powerup.h"
#include "combatchunkid.h"
#include "refinerygameobj.h"
#include "vehiclefactorygameobj.h"
#include "beacongameobj.h"
#include "spawn.h"
#include "apppackettypes.h"
#include "translateobj.h"
#include "translatedb.h"
#include "WWAudio.h"
#include "messagewindow.h"
#include "CNCModeSettings.h"

////////////////////////////////////////////////////////////////
//	Namespaces
////////////////////////////////////////////////////////////////
using namespace BuildingConstants;


////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
BaseControllerClass *	BaseControllerClass::CurrentBases[BuildingConstants::BASE_COUNT] = { 0 };


////////////////////////////////////////////////////////////////
//
//	BaseControllerClass
//
////////////////////////////////////////////////////////////////
BaseControllerClass::BaseControllerClass (void)	:
	BasePowered (true),
	PlayerType (PLAYERTYPE_NOD),
	OperationTimeFactor (1.0F),
	CanGenerateSoldiers (false),
	CanGenerateVehicles (false),
	IsRadarEnabled (true),
	IsBaseDestroyed (false),
	DidBeaconDestroyBase (false),
	BeaconZone (Vector3 (0, 0, 0), Vector3 (0, 0, 0)),
	AnnounceInterval(0),
	AnnouncedAlliedBldgDamageTime(0.0f),
	AnnouncedEnemyBldgDamageTime(0.0f),
	AnnouncedAlliedVehicleDamageTime(0.0f),
	AnnouncedEnemyVehicleDamageTime(0.0f)

{
	Initialize_Building_List ();

	Set_App_Packet_Type(APPPACKETTYPE_BASECONTROLLER);

	return ;
}


////////////////////////////////////////////////////////////////
//
//	~BaseControllerClass
//
////////////////////////////////////////////////////////////////
BaseControllerClass::~BaseControllerClass (void)
{
	WWASSERT ( PlayerType == PLAYERTYPE_GDI || PlayerType == PLAYERTYPE_NOD );
	if (CurrentBases[PlayerType] == this) {
		CurrentBases[PlayerType] = NULL;
	}

	Reset_Building_List ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Shutdown
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Shutdown (void)
{
	Reset_Building_List ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Initialize (int player_type)
{
	WWASSERT ( player_type == PLAYERTYPE_GDI || player_type == PLAYERTYPE_NOD );

	//
	//	Remember which team this base belongs to
	//
	PlayerType					= player_type;
	CurrentBases[PlayerType]	= this;
	Set_Can_Generate_Soldiers( false );
	Set_Can_Generate_Vehicles( false );

	CNCModeSettingsDef* cncDef = CNCModeSettingsDef::Get_Instance();

	if (cncDef) {
		AnnounceInterval = cncDef->Get_Announcement_Interval();
	} else {
		AnnounceInterval = 0;
	}

	//
	//	Setup the network ID
	//
	if (PlayerType == PLAYERTYPE_GDI) {
		Set_Network_ID (NETID_GDI_BASE_CONTROLLER);
	} else {
		WWASSERT(PlayerType == PLAYERTYPE_NOD);
		Set_Network_ID (NETID_NOD_BASE_CONTROLLER);
	}

	Check_Base_Power ();

	Set_Base_Destroyed( false );
	Set_Beacon_Destroyed_Base( false );

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Initialize_Building_List
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Initialize_Building_List (void)
{
	Reset_Building_List ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Reset_Building_List
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Reset_Building_List (void)
{
	//
	//	Simply reset the list
	//
	BuildingList.Delete_All ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Building_Damaged
//
////////////////////////////////////////////////////////////////
void BaseControllerClass::On_Building_Damaged(BuildingGameObj* building)
{
	const BuildingGameObjDef& def = building->Get_Definition();

	// Annouce the damage report if we haven't done so recently.
	if (AnnounceInterval > 0) {
		if (COMBAT_STAR) {
			int playersTeam = COMBAT_STAR->Get_Player_Type();
			bool isAllied = (playersTeam == PlayerType);
		
			float seconds = TimeManager::Get_Total_Seconds();
			int elapsed = 0;

			if (isAllied) {
				elapsed = (int)(seconds - AnnouncedAlliedBldgDamageTime);
			} else {
				elapsed = (int)(seconds - AnnouncedEnemyBldgDamageTime);
			}
		
			if (elapsed >= AnnounceInterval) {
				Play_Announcement(def.Get_Damage_Report(playersTeam));

				if (isAllied) {
					AnnouncedAlliedBldgDamageTime = seconds;
				} else {
					AnnouncedEnemyBldgDamageTime = seconds;
				}
			}
		}
	}

	//	Notify the team that the base is under attack
	Notify_Team(BASE_UNDER_ATTACK, def.Get_Type());
}


////////////////////////////////////////////////////////////////
//
//	On_Building_Destroyed
//
////////////////////////////////////////////////////////////////
void BaseControllerClass::On_Building_Destroyed(BuildingGameObj* building)
{	
	const BuildingGameObjDef& def = building->Get_Definition();

	if (COMBAT_STAR) {
		int playersTeam = COMBAT_STAR->Get_Player_Type();
		Play_Announcement(def.Get_Destroy_Report(playersTeam));
	}

	//	Let the team know that a building was destroyed
	Notify_Team(BUILDING_DESTROYED, def.Get_Type ());

	//	Check a victory condition
	if (Are_All_Buildings_Destroyed()) {
		Set_Base_Destroyed(true);
	}
}


////////////////////////////////////////////////////////////////
//
//	On_Vehicle_Generated
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::On_Vehicle_Generated (VehicleGameObj *vehicle)
{
	//
	//	Find our refinery (if we have one)
	//
	BuildingGameObj* building = Find_Building(TYPE_REFINERY);

	if (building) {
		RefineryGameObj* refinery = building->As_RefineryGameObj();

		//
		//	Check to see if the refinery needs a harvester and if
		// the vehicle we just generated was a harvester.
		//
		int def_id = vehicle->Get_Definition().Get_ID();

		if (refinery->Needs_Harvester() && def_id == refinery->Get_Harvester_Def_ID()) {
			refinery->Set_Harvester_Vehicle(vehicle);
		}
	}

	return ;
}


void BaseControllerClass::On_Vehicle_Delivered(VehicleGameObj *vehicle)
{
	SoldierGameObj* thePlayer = COMBAT_STAR;
	bool isAllied = (thePlayer && (thePlayer->Get_Player_Type() == PlayerType));

	// Only the allied player who purchased the vehicle should hear this.
	if (isAllied) {
		// Play unit ready report for the current player.
		CNCModeSettingsDef* cncDef = CNCModeSettingsDef::Get_Instance();
		
		if (cncDef) {
			Play_Announcement(cncDef->Get_Unit_Ready_Report(PlayerType));
		}
	}
}


void BaseControllerClass::On_Vehicle_Damaged(VehicleGameObj* vehicle)
{
	// Annouce the damage report if we haven't done so recently.
	if (AnnounceInterval > 0) {
		if (COMBAT_STAR) {
			int playersTeam = COMBAT_STAR->Get_Player_Type();
			bool isAllied = (playersTeam == PlayerType);
		
			float seconds = TimeManager::Get_Total_Seconds();
			int elapsed = 0;

			if (isAllied) {
				elapsed = (int)(seconds - AnnouncedAlliedBldgDamageTime);
			} else {
				elapsed = (int)(seconds - AnnouncedEnemyBldgDamageTime);
			}
		
			if (elapsed >= AnnounceInterval) {
				const VehicleGameObjDef& def = vehicle->Get_Definition();
				Play_Announcement(def.Get_Damage_Report(playersTeam));

				if (isAllied) {
					AnnouncedAlliedBldgDamageTime = seconds;
				} else {
					AnnouncedEnemyBldgDamageTime = seconds;
				}
			}
		}
	}
}


void BaseControllerClass::On_Vehicle_Destroyed(VehicleGameObj* vehicle)
{
	if (COMBAT_STAR) {
		const VehicleGameObjDef& def = vehicle->Get_Definition();
		int playersTeam = COMBAT_STAR->Get_Player_Type();
		Play_Announcement(def.Get_Destroy_Report(playersTeam));
	}
}


void BaseControllerClass::On_Beacon_Armed(BeaconGameObj* beacon)
{
	CNCModeSettingsDef* cncDef = CNCModeSettingsDef::Get_Instance();
		
	if (cncDef) {
		bool isNuke = beacon->Get_Definition().Is_Nuke();

		if (COMBAT_STAR != NULL) {
			int team = COMBAT_STAR->Get_Player_Type();

			if (isNuke) {
				int report = cncDef->Get_Nuke_Beacon_Deployed_Report(team);
				Play_Announcement(report);
			} else {
				int report = cncDef->Get_Ion_Beacon_Deployed_Report(team);
				Play_Announcement(report);
			}
		}
	}
}


void BaseControllerClass::On_Beacon_Disarmed(BeaconGameObj* beacon)
{
	CNCModeSettingsDef* cncDef = CNCModeSettingsDef::Get_Instance();
		
	if (cncDef) {
		bool isNuke = beacon->Get_Definition().Is_Nuke();
		if (COMBAT_STAR != NULL) {
			int team = COMBAT_STAR->Get_Player_Type();

			if (isNuke) {
				int report = cncDef->Get_Nuke_Beacon_Disarmed_Report(team);
				Play_Announcement(report);
			} else {
				int report = cncDef->Get_Ion_Beacon_Disarmed_Report(team);
				Play_Announcement(report);
			}
		}
	}
}


void BaseControllerClass::On_Beacon_Warning(BeaconGameObj* beacon)
{
	CNCModeSettingsDef* cncDef = CNCModeSettingsDef::Get_Instance();
		
	if (cncDef) {
		bool isNuke = beacon->Get_Definition().Is_Nuke();
		if (COMBAT_STAR != NULL) {
			int team = COMBAT_STAR->Get_Player_Type();

			if (isNuke) {
				int report = cncDef->Get_Nuke_Beacon_Warning_Report(team);
				Play_Announcement(report);
			} else {
				int report = cncDef->Get_Ion_Beacon_Warning_Report(team);
				Play_Announcement(report);
			}
		}
	}
}


////////////////////////////////////////////////////////////////
//
//	Are_All_Buildings_Destroyed
//
////////////////////////////////////////////////////////////////
bool
BaseControllerClass::Are_All_Buildings_Destroyed (void)
{
	bool retval = true;

	//
	//	Are all the buildings destroyed?
	//
	for (int index = 0; index < BuildingList.Count (); index ++) {
		BuildingGameObj *building = BuildingList[index];
		
		//
		//	If this building has any health left, then the base is not
		// completely destroyed.
		//
		if (building->Is_Destroyed () == false) {
			retval = false;
			break;
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Notify_Team
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Notify_Team (Notification event, BuildingType type)
{	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Check_Base_Power
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Check_Base_Power (void)
{
	bool is_powered = true;

	//	Search through each building
	int count = BuildingList.Count();

	for (int index = 0; index < count; ++index) {
		BuildingGameObj* building = BuildingList[index];

		//	Is this a power-plant?
		if (building->Get_Definition().Get_Type() == TYPE_POWER_PLANT) {
			//	The base is NOT powered if all power-plants are destoryed.
			//	However, the base IS powered if there are NO power plants
			if (BuildingList[index]->Is_Destroyed()) {
				is_powered = false;
			} else {
				is_powered = true;
				break;
			}
		}
	}

	//	Change the powered state of the base
	Power_Base(is_powered);	
}


////////////////////////////////////////////////////////////////
//
//	Power_Base
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Power_Base (bool onoff)
{
	if (CombatManager::I_Am_Server () == false) {
		return ;
	}

	if (BasePowered != onoff) {
		Set_Base_Powered( onoff );
		Set_Operation_Time_Factor( BasePowered ? 1.0F : 2.0F );

		//
		//	Switch the powered state of each building
		//
		for (int index = 0; index < BuildingList.Count (); index ++) {
			BuildingGameObj *building = BuildingList[index];

			//
			//	Only change the power state of this building if it makes sense
			//
			if ((BasePowered == false) || (building->Is_Destroyed () == false)) {
				building->Enable_Power (BasePowered);
			}
		}
	
		// Notify that the base 
		if (!BasePowered) {
			const CNCModeSettingsDef* def = CNCModeSettingsDef::Get_Instance();

			if (def) {
				Play_Announcement(def->Get_Power_Offline_Report(PlayerType));
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Operation_Time_Factor
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Set_Operation_Time_Factor (float factor)	
{ 
	OperationTimeFactor = factor;

	Set_Object_Dirty_Bit( NetworkObjectClass::BIT_OCCASIONAL, true );
}


////////////////////////////////////////////////////////////////
//
//	Set_Base_Powered
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Set_Base_Powered (bool onoff)	
{ 
	BasePowered = onoff;

	Set_Object_Dirty_Bit( NetworkObjectClass::BIT_OCCASIONAL, true );
}


////////////////////////////////////////////////////////////////
//
//	Set_Can_Generate_Soldiers
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Set_Can_Generate_Soldiers (bool onoff)	
{ 
	CanGenerateSoldiers = onoff;

	Set_Object_Dirty_Bit( NetworkObjectClass::BIT_OCCASIONAL, true );
}


////////////////////////////////////////////////////////////////
//
//	Set_Can_Generate_Vehicles
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Set_Can_Generate_Vehicles (bool onoff)	
{ 
	CanGenerateVehicles = onoff;

	Set_Object_Dirty_Bit( NetworkObjectClass::BIT_OCCASIONAL, true );
}


////////////////////////////////////////////////////////////////
//
//	Request_Harvester
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Request_Harvester (int def_id)
{
	if (CombatManager::I_Am_Server () == false) {
		return ;
	}

	//
	//	Find our vehicle factory
	//
	BuildingGameObj *building = Find_Building (TYPE_VEHICLE_FACTORY);
	if (building != NULL) {
		VehicleFactoryGameObj *factory = building->As_VehicleFactoryGameObj ();

		//
		//	If we have a vehicle factory that isn't busy, then start building a harvester
		//
		if (factory->Is_Available ()) {			
			float time = 8.0F;
			factory->Request_Vehicle (def_id, time * OperationTimeFactor);
		}
	} else {

		//	Find our refinery (if we have one)
		//
		BuildingGameObj *building = Find_Building (TYPE_REFINERY);
		if (building != NULL) {
			RefineryGameObj *refinery = building->As_RefineryGameObj ();

			//
			//	If there is no vehicle factory in this level, then
			// try to find a spawner that will create this type of harvester
			// for us.
			//
			DynamicVectorClass<SpawnerClass *> spawner_list = SpawnManager::Get_Spawner_List ();
			for (int index = 0; index < spawner_list.Count (); index ++) {
				SpawnerClass *spawner = spawner_list[index];
				if (spawner != NULL && spawner->Can_Spawn_Object (def_id)) {

					//
					//	Try to spawn the harvester
					//
					PhysicalGameObj *new_obj = spawner->Spawn_Object (def_id);
					if (new_obj != NULL && new_obj->As_VehicleGameObj () != NULL) {
						refinery->Set_Harvester_Vehicle (new_obj->As_VehicleGameObj ());
						break;
					}
				}
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Check_Radar
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Check_Radar (void)
{
	bool enable_radar = false;

	//
	//	Search through each building
	//
	for (int index = 0; index < BuildingList.Count (); index ++) {
		BuildingGameObj *building = BuildingList[index];

		//
		//	Is this a com center?
		//
		if (building->Get_Definition ().Get_Type () == TYPE_COM_CENTER) {

			//
			//	The base has radar if there is even one active com-center
			//
			if (BuildingList[index]->Is_Destroyed () == false) {
				enable_radar = true;
				break;
			}
		}
	}

	//
	//	Update the radar
	//
	Enable_Radar (enable_radar);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Think
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Think (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Find_Building
//
////////////////////////////////////////////////////////////////
BuildingGameObj *
BaseControllerClass::Find_Building (BuildingConstants::BuildingType type)
{
	BuildingGameObj *building = NULL;

	//
	//	Search through each building
	//
	for (int index = 0; index < BuildingList.Count (); index ++) {
	
		//
		//	Is this the type of building we are looking for?
		//
		if (BuildingList[index]->Get_Definition ().Get_Type () == type) {
			building = BuildingList[index];
			break;
		}
	}
	
	return building;
}


////////////////////////////////////////////////////////////////
//
//	Distribute_Funds_To_Each_Teammate
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Distribute_Funds_To_Each_Teammate (int funds)
{
	//
	//	Only let the server assign money
	//
	if (CombatManager::I_Am_Server () == false) {
		return ;
	}

	//
	//	Loop over all the players in the game
	//
	SList<SoldierGameObj> *list	= GameObjManager::Get_Star_Game_Obj_List ();
	SLNode<SoldierGameObj> *node	= NULL;
	for (node = list->Head(); node != NULL; node = node->Next()) {
		SoldierGameObj *player = node->Data();
		
		//
		//	Is this a player on our team?
		//
		if (	player != NULL && 
				(player->Get_Player_Type () == PlayerType) )
		{
			//
			//	Get the data associated with this player
			//
			PlayerDataClass *player_data = player->Get_Player_Data ();
			if (player_data != NULL) {

				//
				//	Add the money to this player's total
				//
				player_data->Increment_Money (funds);
			}
		}
	}

	return ;
}


/*
////////////////////////////////////////////////////////////////
//
//	Deposit_Funds
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Deposit_Funds (int funds)
{
	//
	//	Only let the server assign money
	//
	if (CombatManager::I_Am_Server () == false) {
		return ;
	}

	DynamicVectorClass<SoldierGameObj *> team_players;
	
	//
	//	Loop over all the players in the game
	//
	SList<SoldierGameObj> *list	= GameObjManager::Get_Star_Game_Obj_List ();
	SLNode<SoldierGameObj> *node	= NULL;
	for (node = list->Head(); node != NULL; node = node->Next()) {
		SoldierGameObj *player = node->Data();
		
		//
		//	Is this a player on our team?
		//
		if (	player != NULL && 
				(player->Get_Player_Type () == PLAYERTYPE_GDI && Team == TEAM_GDI) ||
				(player->Get_Player_Type () == PLAYERTYPE_NOD && Team == TEAM_NOD))
		{
			//
			//	Add the player to our list
			//
			team_players.Add (player);
		}
	}

	if (team_players.Count () > 0) {

		//
		//	Now distribute the funds to each player
		//
		int funds_per_player = funds / team_players.Count ();
		for (int index = 0; index < team_players.Count (); index ++) {		
			PlayerDataClass *player_data = team_players[index]->Get_Player_Data ();
			if (player_data != NULL) {

				//
				//	Add the money to this player's total
				//
				player_data->Increment_Money (funds_per_player);
				funds -= funds_per_player;
			}
		}

		//
		//	If there is any money left over, then give it to a random teammate
		//
		if (funds > 0) {
			index = FreeRandom.Get_Int ( team_players.Count () );

			PlayerDataClass *player_data = team_players[index]->Get_Player_Data ();
			if (player_data != NULL) {

				//
				//	Add the money to this player's total
				//
				player_data->Increment_Money (funds);
			}
		}
	}

	return ;
}
*/


////////////////////////////////////////////////////////////////
//
//	Find_Base
//
////////////////////////////////////////////////////////////////
BaseControllerClass *
BaseControllerClass::Find_Base ( int player_type )
{
	BaseControllerClass *base = NULL;

	//
	//	Try to find a base of the appropriate team in our list
	//
	for (int index = 0; index < BASE_COUNT; index ++) {
		if (CurrentBases[index] != NULL && CurrentBases[index]->Get_Player_Type () == player_type ) {
			base = CurrentBases[index];
			break;
		}
	}

	return base;
}


////////////////////////////////////////////////////////////////
//
//	Find_Base_For_Star
//
////////////////////////////////////////////////////////////////
BaseControllerClass *
BaseControllerClass::Find_Base_For_Star (void)
{
	int player_type = PLAYERTYPE_GDI;
	if ( COMBAT_STAR != NULL ) {
		player_type = COMBAT_STAR->Get_Player_Type ();
	}

	return Find_Base (player_type);
}


////////////////////////////////////////////////////////////////
//
//	Set_Base_Destroyed
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Set_Base_Destroyed ( bool onoff )
{
	IsBaseDestroyed = onoff;

	Set_Object_Dirty_Bit( NetworkObjectClass::BIT_OCCASIONAL, true );
}


////////////////////////////////////////////////////////////////
//
//	Destroy_Base
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Destroy_Base (void)
{
	for (int index = 0; index < BuildingList.Count (); index ++) {
		
		//
		//	Destroy this building
		//
		BuildingGameObj *building = BuildingList[index];
		building->Set_Normalized_Health (0);
	}

	//
	//	Disable everything
	//
	Set_Base_Powered( false );
	Set_Can_Generate_Soldiers( false );
	Set_Can_Generate_Vehicles( false );
	Set_Base_Destroyed( true );
	Set_Operation_Time_Factor( 2.0F );
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Beacon_Destroyed_Base
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Set_Beacon_Destroyed_Base ( bool onoff )
{
	DidBeaconDestroyBase = onoff;

	Set_Object_Dirty_Bit( NetworkObjectClass::BIT_OCCASIONAL, true );
}


////////////////////////////////////////////////////////////////
//
//	Add_Building
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Add_Building (BuildingGameObj *building)
{
	//
	//	Add the building to our list (if its not already there)
	//
	if (building != NULL) {
		if (BuildingList.ID (building) == -1) {

			//
			//	Add the building to our list
			//			
			building->CnC_Initialize (this);
			BuildingList.Add (building);

			//
			//	Find the beacon zone if this is the first building added...
			//
			if (BuildingList.Count () == 1) {
				
				//
				//	Get the position of the first building in our list
				//
				Vector3 pos;
				BuildingList[0]->Get_Position (&pos);

				//
				//	Find the closest beacon zone to the building
				//
				ScriptZoneGameObj *zone = NULL;
				zone = ScriptZoneGameObj::Find_Closest_Zone (pos, ZoneConstants::TYPE_BEACON);
				if (zone != NULL) {
					BeaconZone = zone->Get_Bounding_Box ();

					//
					//	Get rid of the zone (if the game doesn't need it anymore)
					//
					if (zone->Get_Observers ().Count () == 0) {
						zone->Set_Delete_Pending();
					}
				}
			}

			//
			//	Update the state of the base
			//
			Check_Base_Power ();
			Check_Radar ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Import_Occasional
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Import_Occasional (BitStreamClass &packet)
{
	//
	//	Read the state variables from the server
	//
	packet.Get (OperationTimeFactor);

	bool wasBasePowered = BasePowered;
	packet.Get(BasePowered);

	packet.Get(CanGenerateSoldiers);
	packet.Get(CanGenerateVehicles);	
	packet.Get(IsBaseDestroyed);
	packet.Get(DidBeaconDestroyBase);

	bool is_radar_enabled = packet.Get(is_radar_enabled);
	Enable_Radar(is_radar_enabled);

	// If the base was powered but now is not then report that
	// power is offline.
	if (wasBasePowered && !BasePowered) {
		const CNCModeSettingsDef* cncDef = CNCModeSettingsDef::Get_Instance();

		if (cncDef) {
			Play_Announcement(cncDef->Get_Power_Offline_Report(PlayerType));
		}
	}
}


////////////////////////////////////////////////////////////////
//
//	Export_Occasional
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Export_Occasional (BitStreamClass &packet)
{
	//
	//	Send the state variables to the client
	//
	packet.Add (OperationTimeFactor);
	packet.Add (BasePowered);
	packet.Add (CanGenerateSoldiers);
	packet.Add (CanGenerateVehicles);	
	packet.Add (IsBaseDestroyed);
	packet.Add (DidBeaconDestroyBase);
	packet.Add (IsRadarEnabled);
}


////////////////////////////////////////////////////////////////
//
//	Enable_Radar
//
////////////////////////////////////////////////////////////////
void
BaseControllerClass::Enable_Radar (bool onoff)
{
	if (IsRadarEnabled != onoff) {
		IsRadarEnabled = onoff;
		
		//
		//	Check to see if the current player belongs to this base
		//
		if (	COMBAT_STAR != NULL && 
				(COMBAT_STAR->Get_Player_Type () == PlayerType) )
		{
			//
			//	Show or hide the radar
			//
			RadarManager::Set_Hidden (!IsRadarEnabled);
		}

		Set_Object_Dirty_Bit( NetworkObjectClass::BIT_OCCASIONAL, true );
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Harvester_Vehicle
//
////////////////////////////////////////////////////////////////
VehicleGameObj *
BaseControllerClass::Get_Harvester_Vehicle (void)
{
	VehicleGameObj *retval = NULL;
	
	//
	//	Find our refinery (if we have one)
	//
	BuildingGameObj *building = Find_Building (TYPE_REFINERY);
	if (building != NULL) {
		RefineryGameObj *refinery = building->As_RefineryGameObj ();

		//
		//	Return the harvester to the caller
		//
		retval = refinery->Get_Harvester_Vehicle ();
	}

	return retval;
}


void BaseControllerClass::Play_Announcement(int text_id)
{
	if (text_id == 0) {
		return;
	}

	// Lookup the translation object from the strings database
	TDBObjClass* translate_obj = TranslateDBClass::Find_Object(text_id);

	if (translate_obj) {
		// Display the text on the screen
		const WCHAR* string = translate_obj->Get_String();

		if (string) {
			CombatManager::Get_Message_Window()->Add_Message(string, Vector3(1, 1, 1));
		}		

		// Play the sound effect
		int soundID = (int)translate_obj->Get_Sound_ID();

		if (soundID > 0) {
			WWAudioClass::Get_Instance()->Create_Instant_Sound(soundID, Matrix3D(1));
		}
	}
}
