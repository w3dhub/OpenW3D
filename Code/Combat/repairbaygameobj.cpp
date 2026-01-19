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
 *                     $Archive:: /Commando/Code/Combat/repairbaygameobj.cpp                  $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 11/15/01 6:37p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 11                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "repairbaygameobj.h"
#include "basecontroller.h"
#include "wwhack.h"
#include "simpledefinitionfactory.h"
#include "persistfactory.h"
#include "definitionmgr.h"
#include "combatchunkid.h"
#include "debug.h"
#include "scriptzone.h"
#include "wwprofile.h"
#include "basecontroller.h"
#include "vehicle.h"
#include "soldier.h"
#include "playertype.h"
#include "playerdata.h"
#include "mesh.h"
#include "meshmdl.h"
#include "crandom.h"
#include "simplegameobj.h"
#include "objlibrary.h"
#include "phys.h"
#include "hanim.h"
#include "combat.h"
#include "bitpackids.h"


////////////////////////////////////////////////////////////////
//	Hacks
////////////////////////////////////////////////////////////////
DECLARE_FORCE_LINK (RepairBay)

////////////////////////////////////////////////////////////////
//	Namespaces
////////////////////////////////////////////////////////////////
using namespace BuildingConstants;

////////////////////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////////////////////
const char *RepairBayGameObj::BoneNames[RepairBayGameObj::BONE_COUNT] = 
{
	"BONE02",
	"BONE03",
	"BONE04",
	"BONE05",
	"BONE06",
	"BONE07",
};


////////////////////////////////////////////////////////////////
//	Editable and persist factories
////////////////////////////////////////////////////////////////
SimplePersistFactoryClass	<RepairBayGameObjDef,	CHUNKID_GAME_OBJECT_DEF_REPAIR_BAY>						_RepairBayGameObjDefPersistFactory;
SimplePersistFactoryClass	<RepairBayGameObj,		CHUNKID_GAME_OBJECT_REPAIR_BAY>							_RepairBayGameObjPersistFactory;
DECLARE_DEFINITION_FACTORY (RepairBayGameObjDef,	CLASSID_GAME_OBJECT_DEF_REPAIR_BAY, "Repair Bay")	_RepairBayGameObjDefDefFactory;


////////////////////////////////////////////////////////////////
//	Save/Load constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_DEF_PARENT									=	0x02200638,
	CHUNKID_DEF_VARIABLES,

	MICROCHUNKID_DEF_REPAIR_PER_SEC					= 1,
	MICROCHUNKID_DEF_REPARING_STATICANIM_DEFID

};

enum
{
	CHUNKID_PARENT								=	0x0219043,
	CHUNKID_VARIABLES,

	MICROCHUNKID_UNUSED						= 1,
};


////////////////////////////////////////////////////////////////
//
//	RepairBayGameObjDef
//
////////////////////////////////////////////////////////////////
RepairBayGameObjDef::RepairBayGameObjDef (void)	:
	RepairPerSec (0),
	RepairingStaticAnimDefID (0)
{
	//
	//	Editable support
	//	
	EDITABLE_PARAM (RepairBayGameObjDef, ParameterClass::TYPE_FLOAT,	RepairPerSec);

	#ifdef PARAM_EDITING_ON
		GenericDefParameterClass *param = new GenericDefParameterClass (&RepairingStaticAnimDefID);
		param->Set_Class_ID (CLASSID_TILE);
		param->Set_Name ("Repairing Static Anim Type");
		GENERIC_EDITABLE_PARAM (RepairBayGameObjDef, param)
	#endif //PARAM_EDITING_ON
	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~RepairBayGameObjDef
//
////////////////////////////////////////////////////////////////
RepairBayGameObjDef::~RepairBayGameObjDef (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Class_ID
//
////////////////////////////////////////////////////////////////
uint32
RepairBayGameObjDef::Get_Class_ID (void) const	
{ 
	return CLASSID_GAME_OBJECT_DEF_REPAIR_BAY;
}


////////////////////////////////////////////////////////////////
//
//	Create
//
////////////////////////////////////////////////////////////////
PersistClass *
RepairBayGameObjDef::Create (void) const 
{
	RepairBayGameObj *building = new RepairBayGameObj;
	building->Init (*this);

	return building;
}


////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////
bool
RepairBayGameObjDef::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_DEF_PARENT);		
		BuildingGameObjDef::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_DEF_VARIABLES);			
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_REPAIR_PER_SEC,					RepairPerSec);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_DEF_REPARING_STATICANIM_DEFID,	RepairingStaticAnimDefID);		
	csave.End_Chunk ();

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////
bool
RepairBayGameObjDef::Load (ChunkLoadClass &cload)
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
				Debug_Say (("Unrecognized RepairBay Def chunkID\n"));
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
RepairBayGameObjDef::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ())
		{
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_REPAIR_PER_SEC,				RepairPerSec);
			READ_MICRO_CHUNK (cload, MICROCHUNKID_DEF_REPARING_STATICANIM_DEFID,	RepairingStaticAnimDefID);		
			
			default:
				Debug_Say (("Unrecognized RepairBay Def Variable chunkID\n"));
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
RepairBayGameObjDef::Get_Factory (void) const 
{ 
	return _RepairBayGameObjDefPersistFactory; 
}


////////////////////////////////////////////////////////////////
//
//	RepairBayGameObj
//
////////////////////////////////////////////////////////////////
RepairBayGameObj::RepairBayGameObj (void)	:
	RepairTimer (0),
	RepairZone (Vector3 (0, 0, 0), Vector3 (0, 0, 0)),
	RepairAnimationID (0),
	IsReparing (false),
	RepairMesh (NULL)
{
	::memset (ArcObjects, 0, sizeof (ArcObjects));
	for (int index = 0; index < ARC_OBJ_COUNT; index ++) {
		ArcLifeRemaining[index] = 0;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	~RepairBayGameObj
//
////////////////////////////////////////////////////////////////
RepairBayGameObj::~RepairBayGameObj (void)
{
	for (int index = 0; index < ARC_OBJ_COUNT; index ++) {
		ArcObjects[index] = NULL;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
RepairBayGameObj::Get_Factory (void) const 
{
	return _RepairBayGameObjPersistFactory;
}


////////////////////////////////////////////////////////////////
//
//	Init
//
////////////////////////////////////////////////////////////////
void RepairBayGameObj::Init( void )
{
	Init( Get_Definition() );
}

////////////////////////////////////////////////////////////////
//
//	Init
//
////////////////////////////////////////////////////////////////
void
RepairBayGameObj::Init (const RepairBayGameObjDef &definition)
{
	BuildingGameObj::Init (definition);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Definition
//
////////////////////////////////////////////////////////////////
const RepairBayGameObjDef &
RepairBayGameObj::Get_Definition (void) const
{
	return (const RepairBayGameObjDef &)BaseGameObj::Get_Definition ();
}


////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////
bool
RepairBayGameObj::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_PARENT);
		BuildingGameObj::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);
	csave.End_Chunk ();
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////
bool
RepairBayGameObj::Load (ChunkLoadClass &cload)
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
				Debug_Say (("Unrecognized RepairBay chunkID\n"));
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
RepairBayGameObj::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {
		
		/*switch (cload.Cur_Micro_Chunk_ID ())
		{
			default:
				Debug_Say (("Unrecognized RepairBay Variable chunkID\n"));
				break;
		}*/

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
RepairBayGameObj::CnC_Initialize (BaseControllerClass *base)
{
	BuildingGameObj::CnC_Initialize (base);

	//
	//	Get the building's "position"
	//
	Vector3 pos;
	Get_Position (&pos);

	//
	//	Find the closest repair bay zone to the building
	//
	ScriptZoneGameObj *zone = NULL;
	zone = ScriptZoneGameObj::Find_Closest_Zone (pos, ZoneConstants::TYPE_VEHICLE_REPAIR);
	if (zone != NULL) {
		RepairZone = zone->Get_Bounding_Box ();

		//
		//	Get rid of the zone (if the game doesn't need it anymore)
		//
		if (zone->Get_Observers ().Count () == 0) {
			zone->Set_Delete_Pending ();
		}
	}

	//
	//	Try to find the mesh that contains the 
	//
	RefMultiListIterator<BuildingAggregateClass> mesh_iterator (&Aggregates);
	for (mesh_iterator.First (); !mesh_iterator.Is_Done (); mesh_iterator.Next ()) {
		StaticPhysClass *phys_obj = mesh_iterator.Peek_Obj ();
		if (phys_obj != NULL && phys_obj->Peek_Model () != NULL) {
			if (::stricmp (phys_obj->Peek_Model ()->Get_Name (), "rep^NOD_fx") == 0) {
				RepairMesh = phys_obj;
				break;
			}
		}
	}


	//
	//	Find the closest reparing static anim phys
	//
	float closest2 = 99999.0F;
	RefPhysListIterator iterator = PhysicsSceneClass::Get_Instance()->Get_Static_Object_Iterator ();
	for (iterator.First (); !iterator.Is_Done (); iterator.Next ()) {
		StaticAnimPhysClass *anim_phys_obj = iterator.Peek_Obj ()->As_StaticAnimPhysClass ();
		
		//
		//	Is this a repairing static anim phys?
		//
		if (	anim_phys_obj != NULL &&
				anim_phys_obj->Get_Definition ()->Get_ID () == (uint32)Get_Definition ().RepairingStaticAnimDefID)
		{

			//
			//	Is this the closest one we've found so far?
			//
			Vector3 anim_pos;
			anim_phys_obj->Get_Position (&anim_pos);
			float dist2 = (anim_pos - pos).Length2 ();
			if (dist2 < closest2) {
				closest2 = dist2;
				RepairAnimationID = anim_phys_obj->Get_ID ();
			}
		}
	}

	//
	//	Create the simple game object's that we'll use to display the repair-arc effect
	//
	for (int index = 0; index < ARC_OBJ_COUNT; index ++) {
		ArcObjects[index]	= (SimpleGameObj *)ObjectLibraryManager::Create_Object ("Arc Effect");
		if (	ArcObjects[index]->Peek_Physical_Object () != NULL &&
				ArcObjects[index]->Peek_Physical_Object ()->Peek_Model () != NULL)
		{
			ArcObjects[index]->Peek_Physical_Object ()->Peek_Model ()->Set_Hidden (true);
		}
		
		ArcLifeRemaining[index]	= 0;
	}

	//
	//	Get information about the model
	//
	RenderObjClass *model = ArcObjects[0]->Peek_Physical_Object ()->Peek_Model ();
	if (model != NULL) {

		//
		//	Store the original transforms of each bone
		//
		for (int index = 0; index < BONE_COUNT; index ++) {
			Bones[index] = model->Get_Bone_Transform (BoneNames[index]);
		}

		//
		//	Lookup the original transform of the end-bone
		//
		int end_index		= model->Get_Bone_Index ("bone_end");		
		EndTM					= model->Get_Bone_Transform (end_index);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Play_Repairing_Animation
//
////////////////////////////////////////////////////////////////
void
RepairBayGameObj::Play_Repairing_Animation (bool onoff)
{
	//
	//	Lookup the static animation object we need to play
	//
	StaticPhysClass *static_phys_obj = PhysicsSceneClass::Get_Instance ()->Find_Static_Object (RepairAnimationID);
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
			if (onoff && anim_mgr.Get_Target_Frame () == 0) {
				anim_mgr.Set_Target_Frame_End ();
			} else if (onoff == false && anim_mgr.Get_Target_Frame () != 0) {
				anim_mgr.Set_Target_Frame (0);
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
RepairBayGameObj::Think (void)
{
	WWPROFILE ("Repair Bay Think");

	//
	//	Check to see if we should repair the vehicle a little more...
	//
	if (IsDestroyed == false) {
		RepairTimer -= TimeManager::Get_Frame_Seconds ();
		if (RepairTimer <= 0) {
			RepairTimer = 0.5F;

			//
			//	Repair any vehicle that is in our zone
			//
			bool is_repairing = Repair_Vehicle ();
			
			//
			//	Play the repairing animation
			//
			if (is_repairing != IsReparing) {
				IsReparing = is_repairing;
			}
		}

		if (IsReparing) {
			Update_Repairing_Animations ();
		}
	}

	//
	//	Hide any arc-effects that have finished
	//
	for (int index = 0; index < ARC_OBJ_COUNT; index ++) {
		if (ArcLifeRemaining[index] > 0) {
			ArcLifeRemaining[index]	-= TimeManager::Get_Frame_Seconds ();
			if (ArcLifeRemaining[index] <= 0) {
				PhysClass *phys_obj = ArcObjects[index]->Peek_Physical_Object ();
				phys_obj->Peek_Model ()->Set_Hidden (true);
			}
		}
	}

	BuildingGameObj::Think ();	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Repair_Vehicle
//
////////////////////////////////////////////////////////////////
bool
RepairBayGameObj::Repair_Vehicle (void)
{
	bool is_repairing = false;
	VehicleList.Delete_All ();

	//
	// Collect the dynamic physics objects in the repair zone
	//
	NonRefPhysListClass objs_to_repair;
	PhysicsSceneClass::Get_Instance ()->Collect_Objects (RepairZone, false, true, &objs_to_repair);
	
	//
	//	Loop over all the objects
	//
	NonRefPhysListIterator it (&objs_to_repair);
	for (it.First (); !it.Is_Done (); it.Next ()) {
		PhysicalGameObj *gameobj = NULL;
		
		//
		//	Get the game object from this physics object
		//
		if (it.Peek_Obj()->Get_Observer () != NULL) {
			gameobj = ((CombatPhysObserverClass *)it.Peek_Obj()->Get_Observer())->As_PhysicalGameObj();
		}
		
		if (gameobj != NULL) {

			//
			//	See if this is a vehicle game object
			//
			VehicleGameObj *vehicle = gameobj->As_VehicleGameObj ();
			if (vehicle != NULL) {

				//
				//	Check to ensure the driver is of the right team
				//
				SoldierGameObj *driver = vehicle->Get_Driver ();
				if (	driver != NULL && 
						driver->Get_Player_Type () == BaseController->Get_Player_Type() )
				{
					float shield_max	= vehicle->Get_Defense_Object ()->Get_Shield_Strength_Max ();
					float health_max	= vehicle->Get_Defense_Object ()->Get_Health_Max ();
					float curr_health	= vehicle->Get_Defense_Object ()->Get_Health ();
					float curr_shield	= vehicle->Get_Defense_Object ()->Get_Shield_Strength ();

					//
					//	Avoid divide-by-zero
					//
					health_max = std::max (health_max, WWMATH_EPSILON);
					shield_max = std::max (shield_max, WWMATH_EPSILON);

					//
					//	Calculate how much health and shield to restore and how much it will cost
					//
					float vehicle_cost			= 1000.0F;//vehicle->Get_Cost ();

					float repair_rate				= (Get_Definition ().RepairPerSec * 0.5F);
					float total_damage			= (health_max - curr_health) + (shield_max - curr_shield);
					int damage_points				= (int)std::min (repair_rate, total_damage);
					float repair_cost_per_pt	= vehicle_cost / (health_max + shield_max);

// FIXME (TSS) available_funds			= 1000 looks suspect...
					int available_funds			= 1000;
					
					int points_restored			= int(available_funds / repair_cost_per_pt);
					points_restored				= std::max (points_restored, 0);
					points_restored				= std::min (points_restored, damage_points);

					int health_restored			= std::min (int(health_max - curr_health), points_restored);
					int shield_restored			= std::min (int(shield_max - curr_shield), (points_restored - health_restored));

					is_repairing = true;

					//
					//	Repair the vehicle's health if the player has enough money
					//					
					if (health_restored > 0) {
						if (CombatManager::I_Am_Server ()) {
							vehicle->Get_Defense_Object ()->Add_Health (health_restored);
						}
						is_repairing = true;
					}
					
					//
					//	Repair the vehicle's shield if the player has enough money
					//
					if (shield_restored > 0) {
						if (CombatManager::I_Am_Server ()) {
							vehicle->Get_Defense_Object ()->Add_Shield_Strength (shield_restored);
						}
						is_repairing = true;
					}

					//
					//	Add this vehicle to the list of vehicles that are being repaired
					//
					if (is_repairing) {
						VehicleList.Add (vehicle);
					}
				}
			}
		}
	}
	
	return is_repairing;
}


////////////////////////////////////////////////////////////////
//
//	Find_Random_Mesh
//
////////////////////////////////////////////////////////////////
MeshClass *
Find_Random_Mesh (RenderObjClass *model)
{
	MeshClass *mesh = NULL;

	//
	//	Is this model a mesh?
	//
	if (mesh == NULL && model->Class_ID () == RenderObjClass::CLASSID_MESH) {
		mesh = reinterpret_cast<MeshClass *> (model);
	}

	//
	//	Pick a random start index in the list
	//
	int count			= model->Get_Num_Sub_Objects ();
	int start_index	= FreeRandom.Get_Int ( count );

	//
	//	Loop over all the objects
	//
	for (int index = 0; mesh == NULL && index < count; index ++) {

		//
		//	Get a pointer to the current subobject
		//
		int real_index = (start_index + index) % count;		
		RenderObjClass *subobj	= model->Get_Sub_Object (real_index);
		if (subobj != NULL) {

			//
			//	Try to find a mesh inside this object
			//
			mesh = Find_Random_Mesh (subobj);
			REF_PTR_RELEASE (subobj);
		}
	}

	return mesh;
}


////////////////////////////////////////////////////////////////
//
//	Update_Repairing_Animations
//
////////////////////////////////////////////////////////////////
void
RepairBayGameObj::Update_Repairing_Animations (void)
{
	//
	//	Loop over all the vehicles that are currently being repaired
	//
	for (int index = 0; index < VehicleList.Count (); index ++) {
		ScriptableGameObj *game_obj = VehicleList[index];
		if (game_obj != NULL) {
			PhysicalGameObj *physical_game_obj	= game_obj->As_PhysicalGameObj ();
			RenderObjClass *model					= physical_game_obj->Peek_Model ();
			if (model != NULL) {

				//
				//	Randomize the welding-arc's that this vehicle gets
				//
				if (FreeRandom.Get_Int (4) == 1) {
					Emit_Welding_Arc (model);
				}
			}				
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Emit_Welding_Arc
//
////////////////////////////////////////////////////////////////
void
RepairBayGameObj::Emit_Welding_Arc (RenderObjClass *vehicle_model)
{
	if (RepairMesh == NULL) {
		return ;
	}

	//
	//	Pick a random "arc welder" to emit the animation from
	//
	Vector3 startpoint (0, 0, 0);
	Get_Position (&startpoint);
	
	//
	//	Pick a random bone to emit the animation from
	//
	RenderObjClass *aggregate_model = RepairMesh->Peek_Model ();
	int count		= 8;//aggregate_model->Get_Num_Bones ();
	int bone_index	= FreeRandom.Get_Int (count) + 1;
	StringClass bone_name;
	bone_name.Format ("REP^NODRIM_FX%d", bone_index);
	startpoint = aggregate_model->Get_Bone_Transform (bone_name).Get_Translation ();
	
	//
	//	Default the end of the vector to the vehicles start position
	//
	Vector3 endpoint = vehicle_model->Get_Transform ().Get_Translation ();

	//
	//	Find a random mesh inside the vehicle's model
	//
	MeshClass *mesh = Find_Random_Mesh (vehicle_model);
	if (mesh != NULL) {
		MeshModelClass *mesh_model = mesh->Get_Model ();
		if (mesh_model != NULL) {
			
			//
			//	Get a random vertex inside the model
			//
			int vertex_count = mesh_model->Get_Vertex_Count ();
			int random_vert_index = FreeRandom.Get_Int (vertex_count);
			if (vertex_count > 0) {
				endpoint = mesh->Get_Transform () * mesh_model->Get_Vertex_Array ()[random_vert_index];
			}

			REF_PTR_RELEASE (mesh_model);
		}
	}				

	//
	//	Try to find an available welding-arc
	//
	for (int index = 0; index < ARC_OBJ_COUNT; index ++) {
		if ((ArcLifeRemaining[index] <= 0) && (FreeRandom.Get_Int (4) == 1)) {

			//
			//	Get the model of the object we'll be displaying
			//
			RenderObjClass *model = ArcObjects[index]->Peek_Physical_Object ()->Peek_Model ();

			//startpoint.Set (0, 0, 0);
			//endpoint.Set (0, 0, 10);

			//
			//	Make the object "look" at its endpoint
			//
			Matrix3D start_tm;
			start_tm.Obj_Look_At (startpoint, endpoint, 0);
			model->Set_Transform (start_tm);

			//
			//	Now scale the bone's that control the length of the arc so it
			// will fit perfectly between the start and endpoints.
			//
			for (int bone_index = 0; bone_index < BONE_COUNT; bone_index ++) {
				
				float percent		= WWMath::Fabs (Bones[bone_index].Get_Translation ().X / EndTM.Get_Translation ().X);
				Vector3 new_pos	= startpoint + (endpoint - startpoint) * percent;

				//
				//	Calculate the world space position of the bone
				//
				Vector3 world_space_pos		= start_tm * Bones[bone_index].Get_Translation ();
				Vector3 world_space_offset	= new_pos - world_space_pos;
				Matrix3D bone_tm (world_space_offset);
				
				//
				//	Control the bone
				//
				int bone_id = model->Get_Bone_Index (BoneNames[bone_index]);
				model->Capture_Bone (bone_id);
				model->Control_Bone (bone_id, bone_tm, true);
			}			

			//
			//	Calculate the world space position of the endpoint
			//
			Vector3 world_space_end_pos = start_tm * EndTM.Get_Translation ();
			Vector3 world_space_offset = endpoint - world_space_end_pos;
			Matrix3D end_tm (world_space_offset);

			//
			//	Control the ending bone
			//
			int end_index = model->Get_Bone_Index ("bone_end");
			model->Capture_Bone (end_index);
			model->Control_Bone (end_index, end_tm, true);			

			//
			//	Start the animation
			//
			ArcLifeRemaining[index] = 3.0F;
			HAnimClass *anim = model->Peek_Animation ();
			if (anim != NULL) {
				model->Set_Animation (anim, 0, RenderObjClass::ANIM_MODE_ONCE);
				ArcLifeRemaining[index] = anim->Get_Total_Time ();
			}

			//
			//	Show the model
			//
			model->Set_Hidden (false);
		}
	}

	return ;
}



/////////////////////////////////////////////////////////////////////////////
//
//	Export_Creation
//
/////////////////////////////////////////////////////////////////////////////
void
RepairBayGameObj::Export_Creation (BitStreamClass &packet)
{
	BuildingGameObj::Export_Creation (packet);

	//
	//	Send the repair zone's position and size
	//
	packet.Add (RepairZone.Center.X, BITPACK_WORLD_POSITION_X);
	packet.Add (RepairZone.Center.Y, BITPACK_WORLD_POSITION_Y);
	packet.Add (RepairZone.Center.Z, BITPACK_WORLD_POSITION_Z);
	packet.Add (RepairZone.Extent.X, BITPACK_WORLD_POSITION_X);
	packet.Add (RepairZone.Extent.Y, BITPACK_WORLD_POSITION_Y);
	packet.Add (RepairZone.Extent.Z, BITPACK_WORLD_POSITION_Z);
	
	//
	//	Send the zone's facing (almost 100% of the time, this zone should
	// simply be an AABox rotated about its Z-axis).
	//
	float facing = RepairZone.Basis.Get_Z_Rotation ();
	packet.Add (facing);	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Import_Creation
//
/////////////////////////////////////////////////////////////////////////////
void
RepairBayGameObj::Import_Creation (BitStreamClass &packet)
{
	BuildingGameObj::Import_Creation (packet);

	//
	//	Get the repair zone's position and size
	//
	packet.Get (RepairZone.Center.X, BITPACK_WORLD_POSITION_X);
	packet.Get (RepairZone.Center.Y, BITPACK_WORLD_POSITION_Y);
	packet.Get (RepairZone.Center.Z, BITPACK_WORLD_POSITION_Z);
	packet.Get (RepairZone.Extent.X, BITPACK_WORLD_POSITION_X);
	packet.Get (RepairZone.Extent.Y, BITPACK_WORLD_POSITION_Y);
	packet.Get (RepairZone.Extent.Z, BITPACK_WORLD_POSITION_Z);
	
	//
	//	Get the zone's facing (almost 100% of the time, this zone should
	// simply be an AABox rotated about its Z-axis).
	//
	float facing = 0;
	packet.Get (facing);	
	RepairZone.Basis.Rotate_Z (facing);
	return ;
}
