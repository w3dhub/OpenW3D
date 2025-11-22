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
 *                     $Archive:: /Commando/Code/Combat/warfactorygameobj.cpp                                                                                                                                     $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 1/07/02 4:46p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 15                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "warfactorygameobj.h"
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
#include "combatchunkid.h"
#include "hanim.h"
#include "vehiclephys.h"


////////////////////////////////////////////////////////////////
//	Hacks
////////////////////////////////////////////////////////////////
DECLARE_FORCE_LINK (WarFactory)

////////////////////////////////////////////////////////////////
//	Editable and persist factories
////////////////////////////////////////////////////////////////
SimplePersistFactoryClass	<WarFactoryGameObjDef,	CHUNKID_GAME_OBJECT_DEF_WARFACTORY>						_WarFactoryGameObjDefPersistFactory;
SimplePersistFactoryClass	<WarFactoryGameObj,		CHUNKID_GAME_OBJECT_WARFACTORY>							_WarFactoryGameObjPersistFactory;
DECLARE_DEFINITION_FACTORY (WarFactoryGameObjDef,	CLASSID_GAME_OBJECT_DEF_WARFACTORY, "WarFactory")	_WarFactoryGameObjDefDefFactory;


////////////////////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////////////////////
float UNITIALIZED_TIMER		= -100.0F;
const float WARFACTORY_LOCK_TIME = 26.0f;

////////////////////////////////////////////////////////////////
//	Save/Load constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_DEF_PARENT						=	0x02200638,
	CHUNKID_DEF_VARIABLES,

	MICROCHUNKID_DEF_UNUSED					= 1,
};

enum
{
	CHUNKID_PARENT								=	0x0219043,
	CHUNKID_VARIABLES,

	MICROCHUNKID_UNUSED						= 1,
};


////////////////////////////////////////////////////////////////
//
//	WarFactoryGameObjDef
//
////////////////////////////////////////////////////////////////
WarFactoryGameObjDef::WarFactoryGameObjDef (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~WarFactoryGameObjDef
//
////////////////////////////////////////////////////////////////
WarFactoryGameObjDef::~WarFactoryGameObjDef (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Class_ID
//
////////////////////////////////////////////////////////////////
uint32
WarFactoryGameObjDef::Get_Class_ID (void) const	
{ 
	return CLASSID_GAME_OBJECT_DEF_WARFACTORY;
}


////////////////////////////////////////////////////////////////
//
//	Create
//
////////////////////////////////////////////////////////////////
PersistClass *
WarFactoryGameObjDef::Create (void) const 
{
	WarFactoryGameObj *building = new WarFactoryGameObj;
	building->Init (*this);

	return building;
}


////////////////////////////////////////////////////////////////
//
//	Create
//
////////////////////////////////////////////////////////////////
bool
WarFactoryGameObjDef::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_DEF_PARENT);		
		VehicleFactoryGameObjDef::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_DEF_VARIABLES);
	csave.End_Chunk ();

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////
bool
WarFactoryGameObjDef::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ())
	{
		switch (cload.Cur_Chunk_ID ())
		{
			case CHUNKID_DEF_PARENT:
				VehicleFactoryGameObjDef::Load (cload);
				break;

			case CHUNKID_DEF_VARIABLES:
				Load_Variables (cload);
				break;
	  
			default:
				Debug_Say (("Unrecognized WarFactory Def chunkID\n"));
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
WarFactoryGameObjDef::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {

		/*switch (cload.Cur_Micro_Chunk_ID ())
		{
			default:
				Debug_Say (("Unrecognized WarFactory Def Variable chunkID\n"));
				break;
		}*/

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
WarFactoryGameObjDef::Get_Factory (void) const 
{ 
	return _WarFactoryGameObjDefPersistFactory; 
}


////////////////////////////////////////////////////////////////
//
//	WarFactoryGameObj
//
////////////////////////////////////////////////////////////////
WarFactoryGameObj::WarFactoryGameObj (void)	:
	CreationAnimationID (0),
	CreationFinishedTimer (UNITIALIZED_TIMER)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~WarFactoryGameObj
//
////////////////////////////////////////////////////////////////
WarFactoryGameObj::~WarFactoryGameObj (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
WarFactoryGameObj::Get_Factory (void) const 
{
	return _WarFactoryGameObjPersistFactory;
}


////////////////////////////////////////////////////////////////
//
//	Init
//
////////////////////////////////////////////////////////////////
void WarFactoryGameObj::Init( void )
{
	Init( Get_Definition() );
}

////////////////////////////////////////////////////////////////
//
//	Init
//
////////////////////////////////////////////////////////////////
void
WarFactoryGameObj::Init (const WarFactoryGameObjDef &definition)
{
	VehicleFactoryGameObj::Init (definition);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Definition
//
////////////////////////////////////////////////////////////////
const WarFactoryGameObjDef &
WarFactoryGameObj::Get_Definition (void) const
{
	return (const WarFactoryGameObjDef &)BaseGameObj::Get_Definition ();
}


////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////
bool
WarFactoryGameObj::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_PARENT);
		VehicleFactoryGameObj::Save (csave);
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
WarFactoryGameObj::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_PARENT:
				VehicleFactoryGameObj::Load (cload);
				break;
								
			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;

			default:
				Debug_Say (("Unrecognized WarFactory chunkID\n"));
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
WarFactoryGameObj::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {
		
		/*switch (cload.Cur_Micro_Chunk_ID ())
		{
			default:
				Debug_Say (("Unrecognized WarFactory Variable chunkID\n"));
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
WarFactoryGameObj::CnC_Initialize (BaseControllerClass *base)
{
	VehicleFactoryGameObj::CnC_Initialize (base);

	//
	//	Get the building's "position"
	//
	Vector3 pos;
	Get_Position (&pos);

	//
	//	Find the closest creation static anim phys
	//
	float closest2 = 99999.0F;
	RefPhysListIterator iterator = PhysicsSceneClass::Get_Instance()->Get_Static_Object_Iterator ();
	for (iterator.First (); !iterator.Is_Done (); iterator.Next ()) {
		StaticAnimPhysClass *anim_phys_obj = iterator.Peek_Obj ()->As_StaticAnimPhysClass ();
		
		//
		//	Is this a vehicle creation static anim phys?
		//
		if (anim_phys_obj != NULL && anim_phys_obj->Peek_Model () != NULL) {

			StringClass name = anim_phys_obj->Peek_Model ()->Get_Name ();
            name.To_Upper();
			if (::strstr (name, "WEP#CONSTRUCT") != NULL) {

				//
				//	Is this the closest one we've found so far?
				//
				Vector3 anim_pos;
				anim_phys_obj->Get_Position (&anim_pos);
				float dist2 = (anim_pos - pos).Length2 ();
				if (dist2 < closest2) {
					closest2 = dist2;
					CreationAnimationID = anim_phys_obj->Get_ID ();
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
WarFactoryGameObj::Think (void)
{
	WWPROFILE ("WarFactory Think");

	//
	//	Are we currently building a vehicle?
	//
	if (IsDestroyed == false && GeneratingVehicleID != 0) {

		if (CreationFinishedTimer > UNITIALIZED_TIMER) {
			CreationFinishedTimer -= TimeManager::Get_Frame_Seconds ();
			
			if (CreationFinishedTimer <= 0) {
			
				CreationFinishedTimer = UNITIALIZED_TIMER;
				
				//
				//	Generate the current vehicle
				//
				VehicleGameObj *vehicle = Create_Vehicle ();
				if (vehicle != NULL) {
					
					Matrix3D new_tm = CreationTM;

					//
					//	Adjust the vehicle's transform to ensure its
					// not embedded in the ground
					//
					if (vehicle->Peek_Vehicle_Phys() != NULL) {
						float height = vehicle->Peek_Vehicle_Phys()->Compute_Approximate_Ride_Height();
						new_tm.Translate(0.0f,0.0f,height);
						vehicle->Set_Transform(new_tm);
					}

					// 
					// Lock the vehicle to anyone but the purchaser
					//
					if (Purchaser != NULL) {
						vehicle->Lock_Vehicle(Purchaser,WARFACTORY_LOCK_TIME);
					}

					//
					//	Destroy any game object that's in our way
					//
					Destroy_Blocking_Objects ();

					// Tell the vehicle to drive to one of the delivery points
					Deliver_Vehicle();
				}

				//
				//	Now play the creation animation backwards to restore the
				// state of the factory
				//
				Play_Creation_Animation (false);
			}
		}
	}

	VehicleFactoryGameObj::Think ();	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Play_Creation_Animation
//
////////////////////////////////////////////////////////////////
void
WarFactoryGameObj::Play_Creation_Animation (bool onoff)
{
	//
	//	Lookup the static animation object we need to play
	//
	StaticPhysClass *static_phys_obj = PhysicsSceneClass::Get_Instance ()->Find_Static_Object (CreationAnimationID);
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

			static_phys_obj->Enable_Is_State_Dirty(true);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Begin_Generation
//
////////////////////////////////////////////////////////////////
void
WarFactoryGameObj::Begin_Generation (void)
{
	Play_Creation_Animation (true);

	CreationFinishedTimer = 2.0F;

	//
	//	Lookup the static animation object for the ending animation
	//
	StaticPhysClass *static_phys_obj = PhysicsSceneClass::Get_Instance ()->Find_Static_Object (CreationAnimationID);
	if (static_phys_obj != NULL) {
		StaticAnimPhysClass *anim_phys_obj = static_phys_obj->As_StaticAnimPhysClass ();
		if (anim_phys_obj != NULL) {
			
			//
			//	Calculate how long to wait before we start playing the end animations
			//
			AnimCollisionManagerClass &anim_mgr = anim_phys_obj->Get_Animation_Manager ();
			CreationFinishedTimer = anim_mgr.Peek_Animation ()->Get_Total_Time () + 2.0f;
		}
	}
	return ;
}


