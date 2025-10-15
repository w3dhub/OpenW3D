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
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Combat/refinerygameobj.cpp                   $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 1/18/02 11:17p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 24                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "refinerygameobj.h"
#include "harvester.h"
#include "basecontroller.h"
#include "vehicle.h"
#include "wwhack.h"
#include "simpledefinitionfactory.h"
#include "persistfactory.h"
#include "definitionmgr.h"
#include "combatchunkid.h"
#include "debug.h"
#include "scriptzone.h"
#include "wwprofile.h"
#include "combat.h"
#include "WWAudio.h"
#include "AudibleSound.h"
#include "soldier.h"
#include "playertype.h"
//#include "building.h"
#include <algorithm>


////////////////////////////////////////////////////////////////
//	Hacks
////////////////////////////////////////////////////////////////
DECLARE_FORCE_LINK (Refinery)

////////////////////////////////////////////////////////////////
//	Editable and persist factories
////////////////////////////////////////////////////////////////
SimplePersistFactoryClass	<RefineryGameObjDef,	CHUNKID_GAME_OBJECT_DEF_REFINERY>					_RefineryGameObjDefPersistFactory;
SimplePersistFactoryClass	<RefineryGameObj,		CHUNKID_GAME_OBJECT_REFINERY>							_RefineryGameObjPersistFactory;
DECLARE_DEFINITION_FACTORY (RefineryGameObjDef,	CLASSID_GAME_OBJECT_DEF_REFINERY, "Refinery")	_RefineryGameObjDefDefFactory;


////////////////////////////////////////////////////////////////
//	Save/Load constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_DEF_PARENT						=	0x02200638,
	CHUNKID_DEF_VARIABLES,

	MICROCHUNKID_DEF_UNLOAD_TIME			= 1,
	MICROCHUNKID_DEF_FUNDS_GATHERED,
	MICROCHUNKID_DEF_HARVESTER_DEFID,
	MICROCHUNKID_DEF_FUNDS_PER_SEC
};

enum
{
	CHUNKID_PARENT								=	0x0219043,
	CHUNKID_VARIABLES,

	MICROCHUNKID_TIBERIUM_FIELD			= 1,
	MICROCHUNKID_HARVESTER,
	MICROCHUNKID_IS_HARVESTER_DOCKED,
	MICROCHUNKID_UNLOAD_TIMER,
	XXXMICROCHUNKID_UNLOAD_DOOR_ID,
	MICROCHUNKID_DOCK_TM
};


////////////////////////////////////////////////////////////////
//
//	RefineryGameObjDef
//
////////////////////////////////////////////////////////////////
RefineryGameObjDef::RefineryGameObjDef (void)	:
	UnloadTime (0),
	FundsGathered (0),
	FundsDistributedPerSec (0),
	HarvesterDefID (0)
{
	//
	//	Editable support
	//	
	EDITABLE_PARAM (RefineryGameObjDef, ParameterClass::TYPE_FLOAT,	UnloadTime);
	EDITABLE_PARAM (RefineryGameObjDef, ParameterClass::TYPE_FLOAT,	FundsGathered);
	EDITABLE_PARAM (RefineryGameObjDef, ParameterClass::TYPE_FLOAT,	FundsDistributedPerSec);	

	#ifdef PARAM_EDITING_ON
		GenericDefParameterClass *param = new GenericDefParameterClass (&HarvesterDefID);
		param->Set_Class_ID (CLASSID_GAME_OBJECT_DEF_VEHICLE);
		param->Set_Name ("Harvester");
		GENERIC_EDITABLE_PARAM (RefineryGameObjDef, param)
	#endif //PARAM_EDITING_ON

	return ;
}


////////////////////////////////////////////////////////////////
//
//	~RefineryGameObjDef
//
////////////////////////////////////////////////////////////////
RefineryGameObjDef::~RefineryGameObjDef (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Class_ID
//
////////////////////////////////////////////////////////////////
uint32
RefineryGameObjDef::Get_Class_ID (void) const	
{ 
	return CLASSID_GAME_OBJECT_DEF_REFINERY;
}


////////////////////////////////////////////////////////////////
//
//	Create
//
////////////////////////////////////////////////////////////////
PersistClass *
RefineryGameObjDef::Create (void) const 
{
	RefineryGameObj *building = new RefineryGameObj;
	building->Init (*this);

	return building;
}


////////////////////////////////////////////////////////////////
//
//	Create
//
////////////////////////////////////////////////////////////////
bool
RefineryGameObjDef::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_DEF_PARENT);		
		BuildingGameObjDef::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_DEF_VARIABLES);
			
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_UNLOAD_TIME,		UnloadTime);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_FUNDS_GATHERED,	FundsGathered);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_FUNDS_PER_SEC,	FundsDistributedPerSec);		
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_HARVESTER_DEFID,	HarvesterDefID);		

	csave.End_Chunk ();

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////
bool
RefineryGameObjDef::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ())
	{
		switch (cload.Cur_Chunk_ID ())
		{
			case CHUNKID_DEF_PARENT:
				BuildingGameObjDef::Load (cload);
				break;

			case CHUNKID_DEF_VARIABLES:
				Load_Variables (cload);
				break;
	  
			default:
				Debug_Say (("Unrecognized Refinery Def chunkID\n"));
				break;

		}
		cload.Close_Chunk ();
	}

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
////////////////////////////////////////////////////////////////
void
RefineryGameObjDef::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ())
		{
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_UNLOAD_TIME,		UnloadTime);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_FUNDS_GATHERED,	FundsGathered);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_FUNDS_PER_SEC,		FundsDistributedPerSec);		
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_HARVESTER_DEFID,	HarvesterDefID);
			
			default:
				Debug_Say (("Unrecognized Refinery Def Variable chunkID\n"));
				break;
		}

		cload.Close_Micro_Chunk();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
RefineryGameObjDef::Get_Factory (void) const 
{ 
	return _RefineryGameObjDefPersistFactory; 
}


////////////////////////////////////////////////////////////////
//
//	RefineryGameObj
//
////////////////////////////////////////////////////////////////
RefineryGameObj::RefineryGameObj (void)	:
	IsHarvesterDocked (false),
	Harvester (NULL),
	UnloadAnimationID (0),
	UnloadTimer (0),
	DistributionTimer (0),
	FundsPerSecond (0),
	TotalFunds (0),
	DockTM (1),
	TiberiumField (Vector3 (0, 0, 0), Vector3 (0, 0, 0))
{
	WWASSERT(WWAudioClass::Get_Instance() != NULL);
	MoneyTrickleSound = WWAudioClass::Get_Instance()->Create_Sound("Money_Trickle");
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~RefineryGameObj
//
////////////////////////////////////////////////////////////////
RefineryGameObj::~RefineryGameObj (void)
{
	if (MoneyTrickleSound != NULL) {
		MoneyTrickleSound->Stop();
		REF_PTR_RELEASE(MoneyTrickleSound);
	}
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
RefineryGameObj::Get_Factory (void) const 
{
	return _RefineryGameObjPersistFactory;
}


////////////////////////////////////////////////////////////////
//
//	Init
//
////////////////////////////////////////////////////////////////
void
RefineryGameObj::Init (void)
{
	Init (Get_Definition ());
	return ;
}

////////////////////////////////////////////////////////////////
//
//	Init
//
////////////////////////////////////////////////////////////////
void
RefineryGameObj::Init (const RefineryGameObjDef &definition)
{
	BuildingGameObj::Init (definition);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Definition
//
////////////////////////////////////////////////////////////////
const RefineryGameObjDef &
RefineryGameObj::Get_Definition (void) const
{
	return (const RefineryGameObjDef &)BaseGameObj::Get_Definition ();
}


////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////
bool
RefineryGameObj::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_PARENT);
		BuildingGameObj::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_TIBERIUM_FIELD,		TiberiumField);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_IS_HARVESTER_DOCKED,	IsHarvesterDocked);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_UNLOAD_TIMER,			UnloadTimer);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DOCK_TM,					DockTM);
	csave.End_Chunk ();
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////
bool
RefineryGameObj::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_PARENT:
				BuildingGameObj::Load (cload);
				break;
								
			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;

			default:
				Debug_Say (("Unrecognized Refinery chunkID\n"));
				break;
		}

		cload.Close_Chunk();
	}

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
////////////////////////////////////////////////////////////////
void
RefineryGameObj::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {
		
		switch (cload.Cur_Micro_Chunk_ID ())
		{
			READ_MICRO_CHUNK (cload, MICROCHUNKID_TIBERIUM_FIELD,			TiberiumField);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_IS_HARVESTER_DOCKED,	IsHarvesterDocked);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_UNLOAD_TIMER,			UnloadTimer);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DOCK_TM,					DockTM);
			
			default:
				Debug_Say (("Unrecognized Refinery Variable chunkID\n"));
				break;
		}

		cload.Close_Micro_Chunk();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	CnC_Initialize
//
////////////////////////////////////////////////////////////////
void
RefineryGameObj::CnC_Initialize (BaseControllerClass *base)
{
	BuildingGameObj::CnC_Initialize (base);

	//
	//	Get the building's "position"
	//
	Vector3 pos;
	Get_Position (&pos);

	//
	//	Try to find the appropriate zone for the team
	//
	ScriptZoneGameObj *zone = NULL;
	if (Get_Player_Type () == PLAYERTYPE_GDI) {
		zone = ScriptZoneGameObj::Find_Closest_Zone (pos, ZoneConstants::TYPE_GDI_TIB_FIELD);
	} else {
		zone = ScriptZoneGameObj::Find_Closest_Zone (pos, ZoneConstants::TYPE_NOD_TIB_FIELD);
	}

	//
	//	If we can't find a "team" tiberium field, then just find the closest.
	//
	if (zone == NULL) {
		zone = ScriptZoneGameObj::Find_Closest_Zone (pos, ZoneConstants::TYPE_TIBERIUM_FIELD);
	}

	if (zone != NULL) {
		TiberiumField = zone->Get_Bounding_Box ();

		//
		//	Get rid of the zone (if the game doesn't need it anymore)
		//
		if (zone->Get_Observers ().Count () == 0) {
			zone->Set_Delete_Pending();
		}
	}


	//
	//	Find the closest unloading static anim phys
	//
	float closest2 = 99999.0F;
	RefPhysListIterator iterator = PhysicsSceneClass::Get_Instance()->Get_Static_Object_Iterator ();
	for (iterator.First (); !iterator.Is_Done (); iterator.Next ()) {
		StaticAnimPhysClass *anim_phys_obj = iterator.Peek_Obj ()->As_StaticAnimPhysClass ();
		
		//
		//	Is this an unloading static anim phys?
		//
		if (anim_phys_obj != NULL && anim_phys_obj->Peek_Model () != NULL) {

			StringClass name = anim_phys_obj->Peek_Model ()->Get_Name ();
			::strupr (name.Peek_Buffer ());
			if (::strstr (name, "REF_TIB_DUMP") != NULL) {

				//
				//	Is this the closest one we've found so far?
				//
				Vector3 anim_pos;
				anim_phys_obj->Get_Position (&anim_pos);
				float dist2 = (anim_pos - pos).Length2 ();
				if (dist2 < closest2) {
					closest2 = dist2;
					UnloadAnimationID = anim_phys_obj->Get_ID ();
				}
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Think
//
////////////////////////////////////////////////////////////////
void
RefineryGameObj::Think (void)
{
	WWPROFILE ("Refinery Think");

	Manage_Money_Trickle_Sound();

	if (IsDestroyed == false && CombatManager::I_Am_Server ()) {
		
		//
		//	Create a new harvester (if necessary)
		//
		if (	(Harvester == NULL) && 
				(DefenseObject.Get_Health() > 0.0f) && 
				(CombatManager::Is_Gameplay_Permitted()) ) 
		{
			Set_Is_Harvester_Docked(false);
			BaseController->Request_Harvester (Get_Definition ().HarvesterDefID);
		
		} else if (IsHarvesterDocked) {

			//
			//	Calculate how long the building was operating during the last frame
			//
			float logical_seconds	= TimeManager::Get_Frame_Seconds ();
			logical_seconds			/= BaseController->Get_Operation_Time_Factor ();

			//
			//	Deposit the money in each team-member's account
			//
			int funds	= int(logical_seconds * FundsPerSecond);
			funds			= std::min (funds, (int)TotalFunds);
			if (funds > 0) {
				TotalFunds -= (float)funds;
				//
				// TSS100401
				// Requested to give the Total Funds to all players, no division .
				//
				//BaseController->Deposit_Funds (funds);
				BaseController->Distribute_Funds_To_Each_Teammate (funds);
			}
			
			//
			//	Is it time to send the harvester off again?
			//
			UnloadTimer -= logical_seconds;
			if (UnloadTimer <= 0) {

				//
				//	Deposit any remaining cash
				//
				if (TotalFunds > 0) {
					//
					// TSS100401
					// Requested to give the Total Funds to all players, no division .
					//
					//BaseController->Deposit_Funds (TotalFunds);
					BaseController->Distribute_Funds_To_Each_Teammate (TotalFunds);
					TotalFunds = 0;
				}

				//
				//	Send the harvester off to the tiberium field
				//
				Harvester->Go_Harvest ();
				Set_Is_Harvester_Docked(false);
				Play_Unloading_Animation (false);
			}
		}

		//
		//	Distrubute a little money to each player
		//
		if (CombatManager::Is_Gameplay_Permitted() && IsHarvesterDocked == false) {
			
			//
			//	Only do this once per second
			//
			DistributionTimer -= TimeManager::Get_Frame_Seconds ();
			if (DistributionTimer <= 0) {
				DistributionTimer = 1.0F;

				//
				//	Calculate how much money to give each player on the team...
				//
				float funds	= Get_Definition ().FundsDistributedPerSec;
				funds			= funds / BaseController->Get_Operation_Time_Factor ();

				//
				//	Deposit this money in their account
				//
				BaseController->Distribute_Funds_To_Each_Teammate ((int)funds);
			}
		}

		//
		//	Allow the harvester to do some per-frame processing
		//
		if (Harvester != NULL) {
			Harvester->Think ();
		}
	}

	BuildingGameObj::Think ();	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Manage_Money_Trickle_Sound
//
////////////////////////////////////////////////////////////////
void
RefineryGameObj::Manage_Money_Trickle_Sound (void)
{
	if (MoneyTrickleSound != NULL) {

		bool same_team = false;

		if (COMBAT_STAR != NULL && BaseController != NULL) {
			same_team = (COMBAT_STAR->Get_Player_Type() == BaseController->Get_Player_Type());
		}

		if (same_team) {

			bool play_sound = 
				!IsDestroyed && 
				(DefenseObject.Get_Health() > 0.0f) && 
				IsHarvesterDocked;

			if (play_sound) {
				if (!MoneyTrickleSound->Is_Playing()) {
					MoneyTrickleSound->Play();
				}
			} else {
				if (MoneyTrickleSound->Is_Playing()) {
					MoneyTrickleSound->Stop();
				}
			}
		}
	}

	/*
	if (MoneyTrickleSound != NULL && MoneyTrickleSound->Is_Playing()) {
		Debug_Say(("MoneyTrickleSound Playing\n"));
	}
	/**/
}


////////////////////////////////////////////////////////////////
//
//	Play_Unloading_Animation
//
////////////////////////////////////////////////////////////////
void
RefineryGameObj::Play_Unloading_Animation (bool onoff)
{
	//
	//	Lookup the static animation object we need to play
	//
	StaticPhysClass *static_phys_obj = PhysicsSceneClass::Get_Instance ()->Find_Static_Object (UnloadAnimationID);
	if (static_phys_obj != NULL) {
		StaticAnimPhysClass *anim_phys_obj = static_phys_obj->As_StaticAnimPhysClass ();
		if (anim_phys_obj != NULL) {
			
			//
			//	Configure the animation
			//
			AnimCollisionManagerClass &anim_mgr = anim_phys_obj->Get_Animation_Manager ();
			anim_mgr.Set_Animation_Mode (AnimCollisionManagerClass::ANIMATE_TARGET);
			
			//
			//	Either play the animation forward or backward
			//
			if (onoff) {
				anim_mgr.Set_Target_Frame_End ();
			} else {
				anim_mgr.Set_Target_Frame (0);
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Destroyed
//
////////////////////////////////////////////////////////////////
void
RefineryGameObj::On_Destroyed (void)
{
	BuildingGameObj::On_Destroyed ();

	//
	//	Kill the harvester
	//
	if (Harvester != NULL) {

		VehicleGameObj * harvy = Get_Harvester_Vehicle();
		if (harvy != NULL) {
			OffenseObjectClass default_damager;
			harvy->Completely_Damaged(default_damager);
		}
	
	}

	//
	// Make sure we shut off the tiberium docking animation
	//
	Play_Unloading_Animation (false);
	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Harvester_Vehicle
//
////////////////////////////////////////////////////////////////
void
RefineryGameObj::Set_Harvester_Vehicle (VehicleGameObj *harvester)
{
	//
	//	Calculate what the dock location and entrance should be
	//
	Vector3 dock_location = DockTM.Get_Translation ();
	Vector3 dock_entrance = dock_location + (DockTM.Get_X_Vector () * 10.0F);

	//
	//	Add some "harvesting" logic to the vehicle
	//
	Harvester = new HarvesterClass;
	Harvester->Set_Refinery (this);
	Harvester->Set_Dock_Location (dock_location);
	Harvester->Set_Dock_Entrance (dock_entrance);
	Harvester->Set_Tiberium_Region (TiberiumField);
	Harvester->Set_Harvest_Anim ("V_NOD_HARVESTER.V_NOD_HARVESTER");
	harvester->Add_Observer (Harvester);
	Harvester->Initialize ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Harvester_Vehicle
//
////////////////////////////////////////////////////////////////
VehicleGameObj *
RefineryGameObj::Get_Harvester_Vehicle (void)
{
	VehicleGameObj *vehicle = NULL;

	//
	//	Dig the vehicle game object out of the harvester
	//
	if (Harvester != NULL) {
		vehicle = Harvester->Get_Vehicle ();
	}

	return vehicle;
}


void RefineryGameObj::On_Harvester_Damaged(VehicleGameObj* harvester)
{
	if (BaseController) {
		BaseController->On_Vehicle_Damaged(harvester);
	}
}


void RefineryGameObj::On_Harvester_Destroyed(VehicleGameObj* harvester)
{
	if (BaseController) {
		BaseController->On_Vehicle_Destroyed(harvester);
	}
}


////////////////////////////////////////////////////////////////
//
//	On_Harvester_Docked
//
////////////////////////////////////////////////////////////////
void
RefineryGameObj::On_Harvester_Docked (void)
{
	//
	//	Initialize the unload settings from the definition
	//
	UnloadTimer		= Get_Definition ().UnloadTime;
	TotalFunds		= Get_Definition ().FundsGathered;
	FundsPerSecond	= TotalFunds / UnloadTimer;

	Set_Is_Harvester_Docked(true);

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Set_Is_Harvester_Docked
//
/////////////////////////////////////////////////////////////////////////////
void
RefineryGameObj::Set_Is_Harvester_Docked (bool flag)
{
	IsHarvesterDocked = flag;

	//
	//	Mark the object as dirty so it's state will be mirrored on the client
	//
	if (CombatManager::I_Am_Server ()) {
		Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//	Export_Rare
//
/////////////////////////////////////////////////////////////////////////////
void
RefineryGameObj::Export_Rare (BitStreamClass &packet)
{
	BuildingGameObj::Export_Rare (packet);

	//
	//	Transmit the state information
	//
	packet.Add (IsHarvesterDocked);
}


/////////////////////////////////////////////////////////////////////////////
//
//	Import_Rare
//
/////////////////////////////////////////////////////////////////////////////
void
RefineryGameObj::Import_Rare (BitStreamClass &packet)
{
	BuildingGameObj::Import_Rare (packet);

	packet.Get (IsHarvesterDocked);
}


