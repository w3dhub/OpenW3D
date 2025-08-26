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
 *                     $Archive:: /Commando/Code/Combat/vehiclefactorygameobj.cpp             $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 3/19/02 10:44a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 26                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "vehiclefactorygameobj.h"
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
#include "basecontroller.h"
#include "doors.h"
#include "objlibrary.h"
#include "combat.h"
#include "soldier.h"
#include "Pathfind.h"
#include "waypath.h"
#include "gameobjmanager.h"
#include "mapmgr.h"


const float UNITIALIZED_TIMER		= -100.0F;
const int	DEFAULT_MAX_VEHICLES_PER_TEAM = 8;

////////////////////////////////////////////////////////////////
//	Hacks
////////////////////////////////////////////////////////////////
DECLARE_FORCE_LINK (VehicleFactory)

////////////////////////////////////////////////////////////////
//	Editable and persist factories
////////////////////////////////////////////////////////////////
SimplePersistFactoryClass	<VehicleFactoryGameObjDef,	CHUNKID_GAME_OBJECT_DEF_VEHICLE_FACTORY>							_VehicleFactoryGameObjDefPersistFactory;
SimplePersistFactoryClass	<VehicleFactoryGameObj,		CHUNKID_GAME_OBJECT_VEHICLE_FACTORY>								_VehicleFactoryGameObjPersistFactory;
DECLARE_DEFINITION_FACTORY (VehicleFactoryGameObjDef,	CLASSID_GAME_OBJECT_DEF_VEHICLE_FACTORY, "Vehicle Factory")	_VehicleFactoryGameObjDefDefFactory (false);

////////////////////////////////////////////////////////////////
//	Static members
////////////////////////////////////////////////////////////////
int VehicleFactoryGameObj::MaxVehiclesPerTeam = DEFAULT_MAX_VEHICLES_PER_TEAM;

////////////////////////////////////////////////////////////////
//	Save/Load constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_DEF_PARENT						=	0x02200638,
	CHUNKID_DEF_VARIABLES,

	MICROCHUNKID_DEF_UNUSED					= 1,
	MICROCHUNKID_DEF_PADCLEARINGWARHEAD,
	MICROCHUNKID_DEF_TOTALBUILDINGTIME,
};

enum
{
	CHUNKID_PARENT								=	0x0219043,
	CHUNKID_VARIABLES,

	MICROCHUNKID_CREATION_TM				= 1,
	MICROCHUNKID_IS_BUSY,
	MICROCHUNKID_GENERATION_TIME,
	MICROCHUNKID_GENERATING_VEHICLE_ID,
};


////////////////////////////////////////////////////////////////
//
//	VehicleFactoryGameObjDef
//
////////////////////////////////////////////////////////////////
VehicleFactoryGameObjDef::VehicleFactoryGameObjDef (void) :
	PadClearingWarhead(25),		// default to "DEATH"
	TotalBuildingTime(12.0f)
{
#ifdef PARAM_EDITING_ON
	EnumParameterClass *param = new EnumParameterClass( &PadClearingWarhead );
	param->Set_Name ( "Pad Clearing Warhead" );
	for (int i = 0; i < ArmorWarheadManager::Get_Num_Warhead_Types(); i++) {
		param->Add_Value ( ArmorWarheadManager::Get_Warhead_Name( i ), i );
	}
	GENERIC_EDITABLE_PARAM(VehicleFactoryGameObjDef,param)
#endif
	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~VehicleFactoryGameObjDef
//
////////////////////////////////////////////////////////////////
VehicleFactoryGameObjDef::~VehicleFactoryGameObjDef (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Class_ID
//
////////////////////////////////////////////////////////////////
uint32
VehicleFactoryGameObjDef::Get_Class_ID (void) const	
{ 
	return CLASSID_GAME_OBJECT_DEF_VEHICLE_FACTORY;
}


////////////////////////////////////////////////////////////////
//
//	Create
//
////////////////////////////////////////////////////////////////
PersistClass *
VehicleFactoryGameObjDef::Create (void) const 
{
	VehicleFactoryGameObj *building = new VehicleFactoryGameObj;
	building->Init (*this);

	return building;
}


////////////////////////////////////////////////////////////////
//
//	Create
//
////////////////////////////////////////////////////////////////
bool
VehicleFactoryGameObjDef::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_DEF_PARENT);		
		BuildingGameObjDef::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_DEF_VARIABLES);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_PADCLEARINGWARHEAD,	PadClearingWarhead);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_TOTALBUILDINGTIME,	TotalBuildingTime);
	csave.End_Chunk ();
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////
bool
VehicleFactoryGameObjDef::Load (ChunkLoadClass &cload)
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
				Debug_Say (("Unrecognized Vehicle Factory Def chunkID\n"));
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
VehicleFactoryGameObjDef::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {

		switch (cload.Cur_Micro_Chunk_ID ())
		{
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_PADCLEARINGWARHEAD, PadClearingWarhead);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_TOTALBUILDINGTIME,  TotalBuildingTime);
			default:
				Debug_Say (("Unrecognized Vehicle Factory Def Variable chunkID\n"));
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
VehicleFactoryGameObjDef::Get_Factory (void) const 
{ 
	return _VehicleFactoryGameObjDefPersistFactory; 
}


////////////////////////////////////////////////////////////////
//
//	VehicleFactoryGameObj
//
////////////////////////////////////////////////////////////////
VehicleFactoryGameObj::VehicleFactoryGameObj (void)	:
	CreationTM (1),
	IsBusy (false),
	GenerationTime (0),
	GeneratingVehicleID (0),
	GeneratingRegion (Vector3 (0, 0, 0), Vector3 (0, 0, 0)),
	LastDeliveryPath (0),
	EndTimer (UNITIALIZED_TIMER)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~VehicleFactoryGameObj
//
////////////////////////////////////////////////////////////////
VehicleFactoryGameObj::~VehicleFactoryGameObj (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
VehicleFactoryGameObj::Get_Factory (void) const 
{
	return _VehicleFactoryGameObjPersistFactory;
}


////////////////////////////////////////////////////////////////
//
//	Init
//
////////////////////////////////////////////////////////////////
void VehicleFactoryGameObj::Init (void)
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
VehicleFactoryGameObj::Init (const VehicleFactoryGameObjDef &definition)
{
	BuildingGameObj::Init (definition);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Definition
//
////////////////////////////////////////////////////////////////
const VehicleFactoryGameObjDef &
VehicleFactoryGameObj::Get_Definition (void) const
{
	return (const VehicleFactoryGameObjDef &)BaseGameObj::Get_Definition ();
}


////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////
bool
VehicleFactoryGameObj::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_PARENT);
		BuildingGameObj::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);

		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_CREATION_TM,				CreationTM);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_IS_BUSY,						IsBusy);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_GENERATION_TIME,			GenerationTime);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_GENERATING_VEHICLE_ID,	GeneratingVehicleID);

	csave.End_Chunk ();
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////
bool
VehicleFactoryGameObj::Load (ChunkLoadClass &cload)
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
				Debug_Say (("Unrecognized Vehicle Factory chunkID\n"));
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
VehicleFactoryGameObj::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {
		
		switch (cload.Cur_Micro_Chunk_ID ())
		{
			READ_MICRO_CHUNK (cload, MICROCHUNKID_CREATION_TM,					CreationTM);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_IS_BUSY,						IsBusy);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_GENERATION_TIME,			GenerationTime);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_GENERATING_VEHICLE_ID,	GeneratingVehicleID);

			default:
				Debug_Say (("Unrecognized Vehicle Factory Variable chunkID\n"));
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
VehicleFactoryGameObj::CnC_Initialize (BaseControllerClass *base)
{
	BuildingGameObj::CnC_Initialize (base);

	//
	//	Get the building's "position"
	//
	Vector3 pos;
	Get_Position (&pos);

	//
	//	Find the closest vehicle construction zone to the building
	//
	ScriptZoneGameObj *zone = NULL;
	zone = ScriptZoneGameObj::Find_Closest_Zone (pos, ZoneConstants::TYPE_VEHICLE_CONSTRUCTION);
	if (zone != NULL) {
		GeneratingRegion = zone->Get_Bounding_Box ();

		//
		//	Get rid of the zone (if the game doesn't need it anymore)
		//
		if (zone->Get_Observers ().Count () == 0) {
			zone->Set_Delete_Pending ();
		}
	}

	//
	//	Let the base know it can generate vehicles
	//
	if (BaseController != NULL) {
		BaseController->Set_Can_Generate_Vehicles (true);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Think
//
////////////////////////////////////////////////////////////////
void
VehicleFactoryGameObj::Think (void)
{
	WWPROFILE ("Vehicle Factory Think");

	if (EndTimer > UNITIALIZED_TIMER) {
		EndTimer -= TimeManager::Get_Frame_Seconds ();
		if (EndTimer < 0) {
			
			//
			//	Handle the end-condition
			//
			On_Generation_Complete ();	
			EndTimer = UNITIALIZED_TIMER;
		}
	}
	
	BuildingGameObj::Think ();	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Is_Available_For_Purchase
//
// In addition to checking whether the factory is busy or
// destroyed, this function checks whether the team has
// already created its maximum number of vehicles.
//
////////////////////////////////////////////////////////////////
bool 
VehicleFactoryGameObj::Is_Available_For_Purchase (void) const
{
	int team_vehicle_count = Get_Team_Vehicle_Count();
	return (Is_Available() & (team_vehicle_count < Get_Max_Vehicles_Per_Team()));
}

////////////////////////////////////////////////////////////////
//
//	Is_Available_For_Purchase
//
// Counts the number of vehicles active for this factory's team
// (not including the harvester!)
//
////////////////////////////////////////////////////////////////
int 
VehicleFactoryGameObj::Get_Team_Vehicle_Count(void) const
{
	int team_vehicle_count = 0;

	VehicleGameObj * harvy = NULL;
	BaseControllerClass * base = BaseControllerClass::Find_Base(Get_Player_Type());
	if (base != NULL) {
		harvy = base->Get_Harvester_Vehicle();
	}

	SLNode<SmartGameObj> * smart_objnode;
	for (smart_objnode = GameObjManager::Get_Smart_Game_Obj_List()->Head(); smart_objnode; smart_objnode = smart_objnode->Next()) {
		VehicleGameObj * obj = smart_objnode->Data()->As_VehicleGameObj();
		
		if (	(obj != NULL) && 
				(obj != harvy) &&
				(obj->Peek_Physical_Object() != NULL) &&
				(obj->Peek_Physical_Object()->As_VehiclePhysClass() != NULL) ) 
		{
			if (obj->Get_Definition().Get_Default_Player_Type() == Get_Player_Type()) {
				team_vehicle_count++;
			}
		}
	}
	return team_vehicle_count;
}

////////////////////////////////////////////////////////////////
//
//	On_Generation_Complete
//
////////////////////////////////////////////////////////////////
void
VehicleFactoryGameObj::On_Generation_Complete (void)
{
	//
	//	Notify the base that we just created a vehicle
	//
	if (Vehicle != NULL) {

		// 
		// Shut off the action making the vehicle drive off the pad.
		// (if it didn't make it by now, give up anyway)
		//
		VehicleGameObj * vehicle_obj = Vehicle.Get_Ptr ()->As_VehicleGameObj();
		if (vehicle_obj != NULL) {
			ActionClass * action = vehicle_obj->Get_Action();
			if (action != NULL) {
		
				// Change the arrived distance so the vehicles think
				// they've arrived at their destination (complete the action)
				action->Get_Parameters().Set_Movement(Vector3(0,0,0), 0.0f, 1000.0f);
			}
			BaseController->On_Vehicle_Generated (vehicle_obj);
		}
	}

	//
	//	Reset our variables
	//
	GeneratingVehicleID	= 0;
	GenerationTime			= 0;
	IsBusy					= false;
	Set_Object_Dirty_Bit (BIT_RARE, true);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Destroyed
//
////////////////////////////////////////////////////////////////
void
VehicleFactoryGameObj::On_Destroyed (void)
{
	BuildingGameObj::On_Destroyed ();

	//
	//	Switch off the ability to build vehicles
	//
	if (BaseController != NULL && CombatManager::I_Am_Server ()) {
		BaseController->Set_Can_Generate_Vehicles (false);
	}
	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Request_Vehicle
//
////////////////////////////////////////////////////////////////
bool
VehicleFactoryGameObj::Request_Vehicle (int definition_id, float generation_time,SoldierGameObj * purchaser)
{
	bool retval = false;

	//
	//	Start the generation if possible
	//
	if (IsBusy == false && definition_id != 0) {
		GeneratingVehicleID	= definition_id;
		GenerationTime			= generation_time;
		IsBusy					= true;		
		Purchaser				= purchaser;
		EndTimer					= Get_Definition().Get_Total_Building_Time();

		Begin_Generation ();
		Set_Object_Dirty_Bit (BIT_RARE, true);
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Destroy_Blocking_Objects
//
////////////////////////////////////////////////////////////////
void
VehicleFactoryGameObj::Destroy_Blocking_Objects (void)
{
	if (CombatManager::I_Am_Server () == false) {
		return ;
	}

	//
	// Collect the dynamic physics objects in the generation region
	//
	NonRefPhysListClass objs_to_kill;
	PhysicsSceneClass::Get_Instance ()->Collect_Objects (GeneratingRegion, false, true, &objs_to_kill);
	
	//
	//	Loop over all the objects
	//
	NonRefPhysListIterator it (&objs_to_kill);
	for (it.First (); !it.Is_Done (); it.Next ()) {
		PhysicalGameObj *gameobj = NULL;
		
		//
		//	Get the game object from this physics object
		//
		if (it.Peek_Obj()->Get_Observer () != NULL) {
			gameobj = ((CombatPhysObserverClass *)it.Peek_Obj()->Get_Observer())->As_PhysicalGameObj();
		}
		
		//
		//	Kill the object
		//
		if (gameobj != NULL && gameobj != Vehicle && gameobj->As_ArmedGameObj () != NULL) {
			
			//
			//	Destroy the object
			//
			OffenseObjectClass offense_obj (10000.0F, Get_Definition().Get_Pad_Clearing_Warhead());
			gameobj->Apply_Damage_Extended( offense_obj );
		}
	}

	return ;
}

////////////////////////////////////////////////////////////////
//
//	Deliver_Vehicle, 
// Gives the vehicle an action to follow one of the delivery 
// waypaths out of the construction zone.
//
////////////////////////////////////////////////////////////////
void 
VehicleFactoryGameObj::Deliver_Vehicle(void)
{
	//
	// Give the vehicle a command to drive to one of the delivery points
	//
	PathfindClass * pathfind = PathfindClass::Get_Instance();
	AABoxClass box;
	box.Center = GeneratingRegion.Center;
	box.Extent = GeneratingRegion.Extent;
	
	if (pathfind != NULL) {

		WaypathClass * path = pathfind->Get_Waypath_Starting_In_Box(box,LastDeliveryPath);
		
		if (path != NULL) {
		
			LastDeliveryPath = (LastDeliveryPath + 1) % pathfind->Count_Waypaths_Starting_In_Box(box);

			if ((Vehicle.Get_Ptr() != NULL) && (Vehicle.Get_Ptr()->As_VehicleGameObj() != NULL)) {
				VehicleGameObj * vehicle = Vehicle.Get_Ptr ()->As_VehicleGameObj();
				ActionClass * action = vehicle->Get_Action();
				if (action != NULL) {

					ActionParamsStruct parameters;
					parameters.Priority = 1;
					parameters.Set_Movement(Vector3(0,0,0), 1.0f, 1.0f);
					parameters.WaypathID = path->Get_ID();
					parameters.ShutdownEngineOnArrival = true;

					action->Goto(parameters);
				
					vehicle->Set_Vehicle_Delivered();
					BaseController->On_Vehicle_Delivered(vehicle);
				}
			}
			
			REF_PTR_RELEASE(path);
		}
	}
}


////////////////////////////////////////////////////////////////
//
//	Create_Vehicle
//
////////////////////////////////////////////////////////////////
VehicleGameObj *
VehicleFactoryGameObj::Create_Vehicle (void)
{
	if (CombatManager::I_Am_Server () == false) {
		return NULL;
	}

	VehicleGameObj *vehicle = NULL;

	if (GeneratingVehicleID != 0) {

		//
		//	Create the vehicle object
		//
		PhysicalGameObj *physical_obj = ObjectLibraryManager::Create_Object (GeneratingVehicleID);
		
		//
		// If this is a VTOL vehicle, see if we should let them have it...
		//
		if ((physical_obj != NULL) && (physical_obj->Peek_Physical_Object() != NULL)) {
			
			if (	(MapMgrClass::Are_VTOL_Vehicles_Enabled() == false) && 
					(physical_obj->Peek_Physical_Object()->As_VTOLVehicleClass() != NULL)
				)
			{
				physical_obj->Set_Delete_Pending();
				physical_obj = NULL;
			}
		}

		//
		// If everything is ok, proceed
		//
		if (physical_obj != NULL && physical_obj->As_VehicleGameObj () != NULL) {
			vehicle = physical_obj->As_VehicleGameObj ();
			vehicle->Start_Observers();

			Vehicle = vehicle;
		}

	}

	return vehicle;
}



/////////////////////////////////////////////////////////////////////////////
//
//	Import_Rare
//
/////////////////////////////////////////////////////////////////////////////
void
VehicleFactoryGameObj::Import_Rare (BitStreamClass &packet)
{
	BuildingGameObj::Import_Rare (packet);

	//
	//	Read the state information from the server
	//	
	packet.Get (IsBusy);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Export_Rare
//
/////////////////////////////////////////////////////////////////////////////
void
VehicleFactoryGameObj::Export_Rare (BitStreamClass &packet)
{
	BuildingGameObj::Export_Rare (packet);

	//
	//	Transmit the state information
	//
	packet.Add (IsBusy);
	return ;
}
