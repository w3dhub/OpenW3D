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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/movephys.cpp                          $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/24/01 5:40p                                              $*
 *                                                                                             *
 *                    $Revision:: 47                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   MoveablePhysClass::MoveablePhysClass -- Constructor                                       *
 *   MoveablePhysClass::~MoveablePhysClass -- Destructor                                       *
 *   MoveablePhysClass::Post_Timestep_Process -- perform post-timestep processing              *
 *   MoveablePhysClass::Update_Shadow -- Update the shadow for this object                     *
 *   MoveablePhysClass::Allocate_Shadow -- ensure that a shadow object is allocated            *
 *   MoveablePhysClass::Release_Shadow -- release the shadow object                            *
 *   MoveablePhysClass::Get_Shadow_Blob_Box -- Return the object space AABox for the shadow    *
 *   MoveablePhysClass::Link_To_Carrier -- Link this object to a carrier object                *
 *   MoveablePhysClass::Peek_Carrier_Sub_Object -- returns the carrier sub object pointer      *
 *   MoveablePhysClass::Save -- persistant object save support                                 *
 *   MoveablePhysClass::Load -- persistant object load support                                 *
 *   MoveablePhysClass::On_Post_Load -- Post-Load callback for MoveablePhysClass               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#include "movephys.h"
#include "colmathaabox.h"
#include "pscene.h"
#include "dyntexproject.h"
#include "chunkio.h"
#include "saveload.h"
#include "wwhack.h"
#include "assetmgr.h"
#include "physcoltest.h"
#include "light.h"

DECLARE_FORCE_LINK(movephys);


#define SINGLE_SHADOW_CODE				1
#define TRUE_PERSPECTIVE_SHADOWS		0


/***********************************************************************************************
**
** MoveablePhysClass Implementation
**
***********************************************************************************************/

/*
** Chunk ID's used by MoveablePhysClass
*/
enum
{
	MOVEABLE_CHUNK_PHYS				= 0x00803000,		// obsolete parent class
	MOVEABLE_CHUNK_VARIABLES,
	MOVEABLE_CHUNK_DYNAMICPHYS,							// current parent class

	MOVEABLE_VARIABLE_MASS			= 0x00,
	MOVEABLE_VARIABLE_GRAVSCALE,
	MOVEABLE_VARIABLE_ELASTICITY,
	MOVEABLE_VARIABLE_CONTROLLER,
	MOVEABLE_VARIABLE_CARRIER,
};


/***********************************************************************************************
 * MoveablePhysClass::MoveablePhysClass -- Constructor                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
MoveablePhysClass::MoveablePhysClass(void) : 
	Mass(1.0f), 
	MassInv(1.0f), 
	GravScale(1.0f), 
	Elasticity(0.5f), 
	Controller(NULL),
	Carrier(NULL),
	CarrierSubObject(NULL),
	ShadowManager(*this)
{ 
}

/***********************************************************************************************
 * MoveablePhysClass::~MoveablePhysClass -- Destructor                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/21/99   gth : Created.                                                                 *
 *=============================================================================================*/
MoveablePhysClass::~MoveablePhysClass(void)
{
	Link_To_Carrier(NULL);
}

/***********************************************************************************************
 * MoveablePhysClass::Init -- inits from a MoveablePhysDefClass                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
void MoveablePhysClass::Init(const MoveablePhysDefClass & def) 
{
	Mass = def.Mass;
	MassInv = 1.0f / Mass;
	GravScale = def.GravScale;
	Elasticity = def.Elasticity;

	DynamicPhysClass::Init(def);
}

/***********************************************************************************************
 * MoveablePhysClass::Definition_Changed -- recopy an cached constants from our def            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
void MoveablePhysClass::Definition_Changed(void)
{
	DynamicPhysClass::Definition_Changed();

	MoveablePhysDefClass * def = (MoveablePhysDefClass *)Definition;
	Mass = def->Mass;
	MassInv = 1.0f / Mass;
	GravScale = def->GravScale;
	Elasticity = def->Elasticity;
}


/***********************************************************************************************
 * MoveablePhysClass::Post_Timestep_Process -- perform post-timestep processing                *
 *                                                                                             *
 *    MoveablePhysClass's update their shadow objects in post-timestep if needed.              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/00     gth : Created.                                                                 *
 *=============================================================================================*/
void MoveablePhysClass::Post_Timestep_Process(void)
{
	ShadowManager.Update_Shadow();
}


/***********************************************************************************************
 * MoveablePhysClass::Get_Shadow_Blob_Box -- Return the object space AABox for the shadow      *
 *                                                                                             *
 * This is only the default implementation which doesn't really do anything useful.  Derived   *
 * classes should over-ride this!!!                                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/00     gth : Created.                                                                 *
 *=============================================================================================*/
void MoveablePhysClass::Get_Shadow_Blob_Box(AABoxClass * set_obj_space_box)
{
	// NOTE: derived classes should really override this!!!
	WWASSERT(set_obj_space_box != NULL);
	set_obj_space_box->Center.Set(0,0,0);
	set_obj_space_box->Extent.Set(1,1,1);
}


/***********************************************************************************************
 * MoveablePhysClass::Link_To_Carrier -- Link this object to a carrier object                  *
 *                                                                                             *
 *    This function will make this object move with the specified carrier object.  You can     *
 *    pass in NULL to clear out the linkage as well.                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 * carrier - physics object that this object is "standing" on.                                 *
 * carrier_sub_obj - possible render object within the physics object.  This makes a           *
 *                   if groundobj is an animated static object for example.                    *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *    This is the only function that should ever call Internal_Link_Rider or Internal_Unlink...*
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/11/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void MoveablePhysClass::Link_To_Carrier(PhysClass * carrier,RenderObjClass * carrier_sub_obj)
{
	// If this is the same carrier we already have, just return
	if ((Carrier == carrier) && (CarrierSubObject == carrier_sub_obj)) return;
	
	// If we had a different carrier, unlink from it
	if (Carrier != NULL) {
		Carrier->Internal_Unlink_Rider(this);
		Carrier = NULL;
		CarrierSubObject = NULL;
	}

	if (carrier) {
		if (carrier->Internal_Link_Rider(this) == true) {
			// only track carriers who are tracking us!
			Carrier = carrier;
			CarrierSubObject = carrier_sub_obj;
		}
	}
}


/***********************************************************************************************
 * MoveablePhysClass::Peek_Carrier_Object -- returns the carrier object pointer                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/15/2001  tss : Created.                                                                 *
 *=============================================================================================*/
PhysClass * MoveablePhysClass::Peek_Carrier_Object(void)
{
	return Carrier;
}


/***********************************************************************************************
 * MoveablePhysClass::Peek_Carrier_Sub_Object -- returns the carrier sub object pointer        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
RenderObjClass * MoveablePhysClass::Peek_Carrier_Sub_Object(void)
{
	return CarrierSubObject;
}


/***********************************************************************************************
 * MoveablePhysClass::Cinematic_Move_To -- attempts to teleport to the desired transform       *
 *                                                                                             *
 * This function attempts to teleport the object to the desired transform and "pushes" all     *
 * dynamic objects out of the way if possible.                                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool MoveablePhysClass::Cinematic_Move_To(const Matrix3D &new_tm)
{
	
	int cinematic_move_mode = MoveablePhysDefClass::CINEMATIC_COLLISION_PUSH;
	if (Get_MoveablePhysDef() != NULL) {
		cinematic_move_mode = Get_MoveablePhysDef()->CinematicCollisionMode;
	}

	// 
	// Check if we can teleport to the desired location.
	// 
	NonRefPhysListClass intersection_list;
	bool ok = false;
	
	if (cinematic_move_mode == MoveablePhysDefClass::CINEMATIC_COLLISION_NONE) {
		ok = true;
	} else {
		ok = Can_Teleport(new_tm, true, &intersection_list);
	}
	
	//
	// Handle the intersection if needed
	//
	if (!ok) {

		if (cinematic_move_mode == MoveablePhysDefClass::CINEMATIC_COLLISION_PUSH) {
			//
			// Try to shove everyone out of our way!
			// NOTE: this won't work for objects where orientation matters and the object
			// is rotating as part of the teleportation.  May have to implement this
			// separately for RBody...
			//
			Vector3 position;
			Get_Position(&position);
			Vector3 move = new_tm.Get_Translation() - position;

			NonRefPhysListIterator it(&intersection_list);
			while (!it.Is_Done()) {
				it.Peek_Obj()->Push(move);
				it.Next();
			}
		
		} else if (cinematic_move_mode == MoveablePhysDefClass::CINEMATIC_COLLISION_KILL ) {

			// 
			// Try to kill all objects intersecting us at the destination point
			//
			NonRefPhysListIterator it(&intersection_list);
			while (!it.Is_Done()) {
				
				//
				// (gth) rbody objects are currently including static objects in
				// the list of intersected objects.  Don't kill them please.
				//
				if (it.Peek_Obj()->As_DynamicPhysClass() != NULL) {
					it.Peek_Obj()->Expire();
				}
				it.Next();
			}
		}

		// 
		// check one more time unless we are in "stop" mode
		// 
		if (cinematic_move_mode != MoveablePhysDefClass::CINEMATIC_COLLISION_STOP) {
			ok = Can_Teleport(new_tm, true);
		}
	}
	
	//
	// If the position is clear, teleport there
	//
	if (ok) {
		Set_Transform(new_tm);
	}
	
	return ok;
}



/***********************************************************************************************
 * MoveablePhysClass::Save -- persistant object save support                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/00     gth : Created.                                                                 *
 *=============================================================================================*/
bool MoveablePhysClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(MOVEABLE_CHUNK_DYNAMICPHYS);
	DynamicPhysClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(MOVEABLE_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,MOVEABLE_VARIABLE_MASS,Mass);
	WRITE_MICRO_CHUNK(csave,MOVEABLE_VARIABLE_GRAVSCALE,GravScale);
	WRITE_MICRO_CHUNK(csave,MOVEABLE_VARIABLE_ELASTICITY,Elasticity);
	if (Controller != NULL) {
		WRITE_MICRO_CHUNK(csave,MOVEABLE_VARIABLE_CONTROLLER,Controller);
	}
	if (Carrier != NULL) {
		WRITE_MICRO_CHUNK(csave,MOVEABLE_VARIABLE_CARRIER,Carrier);
	}
	csave.End_Chunk();

	return true;
}


/***********************************************************************************************
 * MoveablePhysClass::Load -- persistant object load support                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/00     gth : Created.                                                                 *
 *=============================================================================================*/
bool MoveablePhysClass::Load(ChunkLoadClass &cload)
{
	Controller = NULL;
	Carrier = NULL;

	while (cload.Open_Chunk()) {
		
		switch(cload.Cur_Chunk_ID()) 
		{
			case MOVEABLE_CHUNK_PHYS:
				PhysClass::Load(cload);
				break;

			case MOVEABLE_CHUNK_DYNAMICPHYS:
				DynamicPhysClass::Load(cload);
				break;

			case MOVEABLE_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,MOVEABLE_VARIABLE_MASS,Mass);
						READ_MICRO_CHUNK(cload,MOVEABLE_VARIABLE_GRAVSCALE,GravScale);
						READ_MICRO_CHUNK(cload,MOVEABLE_VARIABLE_ELASTICITY,Elasticity);
						READ_MICRO_CHUNK(cload,MOVEABLE_VARIABLE_CONTROLLER,Controller);
						READ_MICRO_CHUNK(cload,MOVEABLE_VARIABLE_CARRIER,Carrier);
					}
					cload.Close_Micro_Chunk();	
				}
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
		}
		
		cload.Close_Chunk();
	}

	if (Controller != NULL) {
		REQUEST_POINTER_REMAP((void**)&Controller);
	}

	if (Carrier != NULL) {
		REQUEST_POINTER_REMAP((void**)&Carrier);
	}

	SaveLoadSystemClass::Register_Post_Load_Callback(this);

	if (Mass < 0.01f) {
		Mass = 0.01f;
	}
	MassInv = 1.0f / Mass;
	return true;
}


/***********************************************************************************************
 * MoveablePhysClass::On_Post_Load -- Post-Load callback for MoveablePhysClass                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/11/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void MoveablePhysClass::On_Post_Load(void)
{
	DynamicPhysClass::On_Post_Load();
	
	// Force our carrier to get correctly linked up, the way I do this
	// is to just clear it out and call the Set_Carrier function again.
	PhysClass * tmp_carrier = Carrier;
	Carrier = NULL;
	Link_To_Carrier(tmp_carrier);
}


/***********************************************************************************************
**
** MoveablePhysDefClass Implementation
**
***********************************************************************************************/


/*
** Chunk ID's used by MoveablePhysDefClass
*/
enum 
{
	MOVEABLEPHYSDEF_CHUNK_PHYSDEF				= 0x04486000,			// obsolete parent class
	MOVEABLEPHYSDEF_CHUNK_VARIABLES,
	MOVEABLEPHYSDEF_CHUNK_DYNAMICPHYSDEF,								// current parent class

	MOVEABLEPHYSDEF_VARIABLE_MASS				= 0x00,
	MOVEABLEPHYSDEF_VARIABLE_GRAVSCALE,
	MOVEABLEPHYSDEF_VARIABLE_ELASTICITY,
	MOVEABLEPHYSDEF_VARIABLE_CINEMATICCOLLISIONMODE,
};


MoveablePhysDefClass::MoveablePhysDefClass(void) :
	Mass(1.0f),
	GravScale(1.0f),
	Elasticity(0.1f),
	CinematicCollisionMode(CINEMATIC_COLLISION_PUSH)
{
	// make our parameters editable!
	FLOAT_EDITABLE_PARAM(MoveablePhysDefClass, Mass, 0.01f, 100000.0f);
	FLOAT_EDITABLE_PARAM(MoveablePhysDefClass, GravScale, 0.0f, 10.0f);
	FLOAT_EDITABLE_PARAM(MoveablePhysDefClass, Elasticity, 0.0f, 1.0f);	

#ifdef PARAM_EDITING_ON
	EnumParameterClass *cinematic_param = new EnumParameterClass(&CinematicCollisionMode);
	cinematic_param->Set_Name ("CinematicCollisionMode");
	cinematic_param->Add_Value("NONE",CINEMATIC_COLLISION_NONE);
	cinematic_param->Add_Value("STOP",CINEMATIC_COLLISION_STOP);
	cinematic_param->Add_Value("PUSH",CINEMATIC_COLLISION_PUSH);
	cinematic_param->Add_Value("KILL",CINEMATIC_COLLISION_KILL);
	GENERIC_EDITABLE_PARAM( MoveablePhysDefClass, cinematic_param);
#endif
}

bool MoveablePhysDefClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(MOVEABLEPHYSDEF_CHUNK_DYNAMICPHYSDEF);
	DynamicPhysDefClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(MOVEABLEPHYSDEF_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,MOVEABLEPHYSDEF_VARIABLE_MASS,Mass);
	WRITE_MICRO_CHUNK(csave,MOVEABLEPHYSDEF_VARIABLE_GRAVSCALE,GravScale);
	WRITE_MICRO_CHUNK(csave,MOVEABLEPHYSDEF_VARIABLE_ELASTICITY,Elasticity);
	WRITE_MICRO_CHUNK(csave,MOVEABLEPHYSDEF_VARIABLE_CINEMATICCOLLISIONMODE,CinematicCollisionMode);
	csave.End_Chunk();
	return true;
}


bool MoveablePhysDefClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {

		switch(cload.Cur_Chunk_ID()) {

			case MOVEABLEPHYSDEF_CHUNK_PHYSDEF:
				PhysDefClass::Load(cload);
				break;

			case MOVEABLEPHYSDEF_CHUNK_DYNAMICPHYSDEF:
				DynamicPhysDefClass::Load(cload);
				break;

			case MOVEABLEPHYSDEF_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,MOVEABLEPHYSDEF_VARIABLE_MASS,Mass);
						READ_MICRO_CHUNK(cload,MOVEABLEPHYSDEF_VARIABLE_GRAVSCALE,GravScale);
						READ_MICRO_CHUNK(cload,MOVEABLEPHYSDEF_VARIABLE_ELASTICITY,Elasticity);
						READ_MICRO_CHUNK(cload,MOVEABLEPHYSDEF_VARIABLE_CINEMATICCOLLISIONMODE,CinematicCollisionMode);
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
		}

		cload.Close_Chunk();
	}

	return true;
}

bool MoveablePhysDefClass::Is_Type(const char * type_name)
{
	if (stricmp(type_name,MoveablePhysDefClass::Get_Type_Name()) == 0) {
		return true;
	} else {
		return DynamicPhysDefClass::Is_Type(type_name);
	}
}

