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
 *                     $Archive:: /Commando/Code/wwphys/pscene_saveload.cpp                   $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/07/03 2:22p                                               $*
 *                                                                                             *
 *                    $Revision:: 22                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "pscene.h"
#include "chunkio.h"
#include "staticaabtreecull.h"
#include "dynamicaabtreecull.h"
#include "physgridcull.h"
#include "staticphys.h"
#include "staticanimphys.h"
#include "lightcull.h"
#include "light.h"
#include "persistfactory.h"
#include "wwmemlog.h"

 
/*
** This module contains the save-load related methods of PhysicsSceneClass.  
*/


/*
** Chunk-ID's for PhysicsSceneClass
**
** LOAD-SAVE NOTES:
** The data in the physics scene will be broken into two different files, the LSD (Level Static Data)
** file and the LDD (Level Dynamic Data) file.  The primary reason for this is simply so that 
** save-games can simply be a new export of the LDD file and not contain the data which is completely
** static throughout the run of the level.
**
*/
enum 
{
	// Chunk ID's used by the physics scene when saving the 'Static Data' (PhysStaticDataSaveSystem)
	PSCENE_SD_CHUNK_STATIC_OBJECT_AABTREE					= 0x00004500,
	PSCENE_SD_CHUNK_STATIC_LIGHT_AABTREE					= 0x00004501,
	PSCENE_SD_CHUNK_DYNAMIC_OBJECT_GRID						= 0x00004600,		// OBSOLETE!!!
	PSCENE_SD_CHUNK_DYNAMIC_OBJECT_VIS_AABTREE			= 0x00004601,
	PSCENE_SD_CHUNK_VISIBILITY_DATA							= 0x00004700,
	PSCENE_SD_CHUNK_SUNLIGHT									= 0x00004800,
	PSCENE_SD_CHUNK_SCENECLASS									= 0x00004810,
	PSCENE_SD_CHUNK_VARIABLES									= 0x00004820,

	PSCENE_SD_VARIABLE_AMBIENT									= 0x00,

	// Chunk ID's used by the physics scene when saving the 'Static Objects' (PhysStaticObjectsSaveSystem)
	PSCENE_SO_CHUNK_STATIC_OBJECTS							= 0x00770100,
	PSCENE_SO_CHUNK_STATIC_OBJECT,
	PSCENE_SO_CHUNK_STATIC_OBJECT_AABLINK,

	PSCENE_SO_CHUNK_STATIC_LIGHTS								= 0x00770200,
	PSCENE_SO_CHUNK_STATIC_LIGHT,
	PSCENE_SO_CHUNK_STATIC_LIGHT_AABLINK,

	// Chunk ID's used by the physics scene when saving the dynamic data and objects (PhysDynamicDataSaveSystem)
	XXX_PSCENE_DD_CHUNK_SCENECLASS							= 0x00890000,
	PSCENE_DD_CHUNK_VARIABLES									= 0x00890001,
	PSCENE_DD_CHUNK_COLLISION_FLAGS							= 0x00890002,		// OBSOLETE!!!

	PSCENE_DD_CHUNK_DYNAMIC_OBJECTS							= 0x00890100,
	PSCENE_DD_CHUNK_DYNAMIC_OBJECT,
	PSCENE_DD_CHUNK_DYNAMIC_OBJECT_GRIDLINK,

	// Chunk ID's used to save the states of static objects
	PSCENE_DD_CHUNK_STATIC_OBJECT_STATES					= 0x00890200,
	PSCENE_DD_CHUNK_STATIC_OBJECT_STATE,
	PSCENE_DD_CHUNK_STATIC_OBJECT_ID,
	PSCENE_DD_CHUNK_STATIC_OLD_PTR,

	// Micro Chunk ID's used by dynamic data variables
	PSCENE_DD_VARIABLE_DYNAMICPOLYBUDGET					= 0x00,
	PSCENE_DD_VARIABLE_STATICPOLYBUDGET						= 0x01,
	
};

/*
** Structure for saving the sun settings
*/
struct IOSunLightStruct
{
	uint32				Enabled;
	float32				Yaw;
	float32				Pitch;
	float32				Intensity;
	IOVector3Struct	Color;
};


void PhysicsSceneClass::Export_Vis_Data(ChunkSaveClass & csave)
{
	VisTableManager.Save(csave);
}

void PhysicsSceneClass::Import_Vis_Data(ChunkLoadClass & cload)
{
	Internal_Vis_Reset();
	VisTableManager.Load(cload);
}

void PhysicsSceneClass::Save_Level_Static_Data(ChunkSaveClass & csave)
{
	/*
	** If the visibility system has been invalidated, reset it so that
	** we dont save garbage data!
	*/
	Internal_Vis_Reset();

	/*
	** Things to save here:
	** - Parent class settings (SceneClass -> fog color, depth ranges, polyrender mode, etc)
	** - StaticCullingSystem structure (but not the objects in it)
	** - DynamicCullingSystem structure (not objects)
	** - LightCullingSystem structure (not objects)
	** - Visibility Tables
	** - Sunlight settings
	*/
	csave.Begin_Chunk(PSCENE_SD_CHUNK_SCENECLASS);
	SceneClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(PSCENE_SD_CHUNK_STATIC_OBJECT_AABTREE);
	StaticCullingSystem->Save_Static_Data(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(PSCENE_SD_CHUNK_STATIC_LIGHT_AABTREE);
	StaticLightingSystem->Save_Static_Data(csave);
	csave.End_Chunk();

//	csave.Begin_Chunk(PSCENE_SD_CHUNK_DYNAMIC_OBJECT_GRID);
//	DynamicCullingSystem->Save_Static_Data(csave);
//	csave.End_Chunk();
	
	csave.Begin_Chunk(PSCENE_SD_CHUNK_DYNAMIC_OBJECT_VIS_AABTREE);
	DynamicObjVisSystem->Save_Static_Data(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(PSCENE_SD_CHUNK_VISIBILITY_DATA);
	VisTableManager.Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(PSCENE_SD_CHUNK_SUNLIGHT);
	Save_Sun_Light(csave);	
	csave.End_Chunk();

	csave.Begin_Chunk(PSCENE_SD_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,PSCENE_SD_VARIABLE_AMBIENT,SceneAmbientLight);
	csave.End_Chunk();

}

void PhysicsSceneClass::Load_Level_Static_Data(ChunkLoadClass & cload)
{
	WWMEMLOG(MEM_PHYSICSDATA);
	VisResetNeeded = false;
	LastValidVisId = -1;

	/*
	** Things to load here:
	** - StaticCullingSystem structure (but not the objects in it)
	** - LightCullingSystem structure (not objects)
	** - DynamicCullingSystem structure (not objects)
	** - Visibility Tables
	*/
	VisTableManager.Reset();
	while (cload.Open_Chunk()) {
		switch (cload.Cur_Chunk_ID()) {
			case PSCENE_SD_CHUNK_SCENECLASS:
				SceneClass::Load(cload);
				break;

			case PSCENE_SD_CHUNK_STATIC_OBJECT_AABTREE:
				StaticCullingSystem->Load_Static_Data(cload);
				break;

			case PSCENE_SD_CHUNK_STATIC_LIGHT_AABTREE:
				StaticLightingSystem->Load_Static_Data(cload);
				break;

//			case PSCENE_SD_CHUNK_DYNAMIC_OBJECT_GRID:
//				DynamicCullingSystem->Load(cload);
//				break;

			case PSCENE_SD_CHUNK_DYNAMIC_OBJECT_VIS_AABTREE:
				DynamicObjVisSystem->Load_Static_Data(cload);
				break;

			case PSCENE_SD_CHUNK_VISIBILITY_DATA:
				VisTableManager.Load(cload);
				break;

			case PSCENE_SD_CHUNK_SUNLIGHT:
				Load_Sun_Light(cload);
				break;

			case PSCENE_SD_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,PSCENE_SD_VARIABLE_AMBIENT,SceneAmbientLight);
					}
					cload.Close_Micro_Chunk();	
				}
				break;
		}
		cload.Close_Chunk();
	}
}

void PhysicsSceneClass::Post_Load_Level_Static_Data(void)
{
	Vector3 wmin,wmax;
	Get_Level_Extents(wmin,wmax);
	DynamicCullingSystem->Re_Partition(wmin,wmax,10.0f);

	Compute_Vis_Mesh_Ram();
}

void PhysicsSceneClass::Save_Level_Static_Objects(ChunkSaveClass & csave)
{
	/*
	** Things to save here:
	** - Objects which have absolutely no dynamic state and cannot be created or destroyed during a level.
	** - Lights which also follow the same rules...
	*/
	if (!StaticObjList.Is_Empty()) {
		csave.Begin_Chunk(PSCENE_SO_CHUNK_STATIC_OBJECTS);
		Save_Static_Objects(csave);
		csave.End_Chunk();
	}
	
	if (!StaticLightList.Is_Empty()) {
		csave.Begin_Chunk(PSCENE_SO_CHUNK_STATIC_LIGHTS);
		Save_Static_Lights(csave);
		csave.End_Chunk();
	}
}

void PhysicsSceneClass::Load_Level_Static_Objects(ChunkLoadClass & cload)
{
	/*
	** Things to load here:
	** - Static objects and lights
	*/
	WWASSERT(StaticObjList.Is_Empty());
	WWASSERT(StaticLightList.Is_Empty());

	while (cload.Open_Chunk()) {
		switch (cload.Cur_Chunk_ID()) 
		{
			case PSCENE_SO_CHUNK_STATIC_OBJECTS:
				Load_Static_Objects(cload);
				break;

			case PSCENE_SO_CHUNK_STATIC_LIGHTS:
				Load_Static_Lights(cload);
				break;
		}
		cload.Close_Chunk();
	}
}

void PhysicsSceneClass::Post_Load_Level_Static_Objects(void)
{
}

void PhysicsSceneClass::Save_Level_Dynamic_Data(ChunkSaveClass & csave)
{
	/*
	** Things to save here
	** - CollisionFlags
	** - DynamicPolyBudget
	** - StaticPolyBudget
	** - List of dynamic objects (embed each in a chunk, embed cull linkage, build the list again at load time)
	** - List of static objects (embed each in a chunk, ... )
	** - List of static lights (embed each in a chunk, ... )
	** - DirtyCullList - this will have to be a "de-swizzle" list.
	** NOTES:
	** - each phys obj should tell their render object that they were added in post-load
	*/
	csave.Begin_Chunk(PSCENE_DD_CHUNK_VARIABLES);
	Save_LDD_Variables(csave);
	csave.End_Chunk();

	if (!ObjList.Is_Empty()) {
		csave.Begin_Chunk(PSCENE_DD_CHUNK_DYNAMIC_OBJECTS);
		Save_Dynamic_Objects(csave);
		csave.End_Chunk();
	}

	if (!StaticAnimList.Is_Empty()) {
		csave.Begin_Chunk(PSCENE_DD_CHUNK_STATIC_OBJECT_STATES);
		Save_Static_Object_States(csave);
		csave.End_Chunk();
	}

#if 0 // Do I need to do this?
	if (!DirtyCullList.Is_Empty()) {
		csave.Begin_Chunk(PSCENE_LDD_CHUNK_DIRTY_CULL_LIST);
		Save_LDD_Dirty_Cull_List();
		csave.End_Chunk();
	}
#endif
}

void PhysicsSceneClass::Load_Level_Dynamic_Data(ChunkLoadClass & cload)
{
	WWMEMLOG(MEM_PHYSICSDATA);
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case PSCENE_DD_CHUNK_VARIABLES:
				Load_LDD_Variables(cload);
				break;

			case PSCENE_DD_CHUNK_DYNAMIC_OBJECTS:
				Load_Dynamic_Objects(cload);
				break;

			case PSCENE_DD_CHUNK_STATIC_OBJECT_STATES:
				Load_Static_Object_States(cload);
				break;

#if 0 // Do I need to do this?
			case PSCENE_LDD_CHUNK_DIRTY_CULL_LIST:
				Load_LDD_Dirty_Cull_List(cload);
				break;
#endif
				
			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X in file: %s, line %d\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
		}
		cload.Close_Chunk();
	}
}


void PhysicsSceneClass::Save_LDD_Variables(ChunkSaveClass & /* csave */)
{
}


void PhysicsSceneClass::Load_LDD_Variables(ChunkLoadClass & cload)
{
	while (cload.Open_Micro_Chunk()) {
		switch (cload.Cur_Micro_Chunk_ID()) {
			OBSOLETE_MICRO_CHUNK(PSCENE_DD_VARIABLE_DYNAMICPOLYBUDGET);
			OBSOLETE_MICRO_CHUNK(PSCENE_DD_VARIABLE_STATICPOLYBUDGET);
			default:
				WWDEBUG_SAY(("Unhandled Micro Chunk: 0x%x in file &s, line %d\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
		}
		cload.Close_Micro_Chunk();
	}
}


void PhysicsSceneClass::Save_Static_Objects(ChunkSaveClass & csave)
{
	/*
	** Save each object and save each object's culling linkage
	*/
	RefPhysListIterator it(&StaticObjList);
	for (it.First(); !it.Is_Done(); it.Next()) {
		
		PhysClass * obj = it.Peek_Obj();
		WWASSERT(obj != NULL);
		
		if ((obj) && (obj->Is_Dont_Save_Enabled() == false)) {		
			StaticPhysClass * staticobj = obj->As_StaticPhysClass();
			WWASSERT(staticobj != NULL); 
			
			if (staticobj) {
				/*
				** Wrap the factory defined chunk with our own so that it's in its own
				** "chunk-space"
				*/
				csave.Begin_Chunk(PSCENE_SO_CHUNK_STATIC_OBJECT);
					csave.Begin_Chunk(staticobj->Get_Factory().Chunk_ID());
					staticobj->Get_Factory().Save(csave,staticobj);
					csave.End_Chunk();
				csave.End_Chunk();

				/*
				** Follow the object data with its linkage data
				*/
				csave.Begin_Chunk(PSCENE_SO_CHUNK_STATIC_OBJECT_AABLINK);
				StaticCullingSystem->Save_Object_Linkage(csave,staticobj);
				csave.End_Chunk();
			}
		}
	}
}

void PhysicsSceneClass::Load_Static_Objects(ChunkLoadClass & cload)
{
	while (cload.Open_Chunk()) {

		if (cload.Cur_Chunk_ID() == PSCENE_SO_CHUNK_STATIC_OBJECT) {

			/*
			** Load the object
			*/ 
			cload.Open_Chunk();
			StaticPhysClass * obj = NULL;
			PersistFactoryClass * fact = SaveLoadSystemClass::Find_Persist_Factory(cload.Cur_Chunk_ID());
			WWASSERT(fact != NULL);
			if (fact) {
				obj = (StaticPhysClass *)fact->Load(cload);
			}
			cload.Close_Chunk();

			cload.Close_Chunk();
			
			/*
			** Load the object's linkage into the Static Culling System
			*/
			cload.Open_Chunk();
			if ((cload.Cur_Chunk_ID() == PSCENE_SO_CHUNK_STATIC_OBJECT_AABLINK) && (obj)) {
				StaticCullingSystem->Load_Object_Linkage(cload,obj);
			} 
			cload.Close_Chunk();

			/*
			** Finish installing the object into the physics scene
			** - Add it to the static objects list
			** - If it requires time-stepping, add to the timestep list
			** - Let the base SceneClass notify the render object that it has been added.
			*/
			Internal_Add_Static_Object(obj);
			obj->Release_Ref();

		} else {
			WWDEBUG_SAY(("Unhandled Chunk: 0x%x in file %s, line %d\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
			cload.Close_Chunk();
		}
	}
}

void PhysicsSceneClass::Save_Static_Lights(ChunkSaveClass & csave)
{
	/*
	** Save each object and save each object's culling linkage
	*/
	RefPhysListIterator it(&StaticLightList);
	for (it.First(); !it.Is_Done(); it.Next()) {
		
		PhysClass * obj = it.Peek_Obj();
		WWASSERT(obj != NULL);
		
		if (obj) {		
			LightPhysClass * lightobj = obj->As_LightPhysClass();
			WWASSERT(lightobj != NULL); 
			
			if (lightobj) {
				csave.Begin_Chunk(PSCENE_SO_CHUNK_STATIC_LIGHT);
				csave.Begin_Chunk(lightobj->Get_Factory().Chunk_ID());
				lightobj->Get_Factory().Save(csave,lightobj);
				csave.End_Chunk();
				csave.End_Chunk();
				
				csave.Begin_Chunk(PSCENE_SO_CHUNK_STATIC_LIGHT_AABLINK);
				StaticLightingSystem->Save_Object_Linkage(csave,lightobj);
				csave.End_Chunk();
			}
		}
	}
}

void PhysicsSceneClass::Load_Static_Lights(ChunkLoadClass & cload)
{
	while (cload.Open_Chunk()) {

		if (cload.Cur_Chunk_ID() == PSCENE_SO_CHUNK_STATIC_LIGHT) {

			/*
			** Load the object
			*/ 
			cload.Open_Chunk();
			LightPhysClass * obj = NULL;
			PersistFactoryClass * fact = SaveLoadSystemClass::Find_Persist_Factory(cload.Cur_Chunk_ID());
			WWASSERT(fact != NULL);
			if (fact) {
				obj = (LightPhysClass *)fact->Load(cload);
			}
			cload.Close_Chunk();
			cload.Close_Chunk();
			
			/*
			** Load the object's linkage into the static light culling system
			*/
			WWASSERT(obj);
			cload.Open_Chunk();
			if ((cload.Cur_Chunk_ID() == PSCENE_SO_CHUNK_STATIC_LIGHT_AABLINK) && (obj)) {
				StaticLightingSystem->Load_Object_Linkage(cload,obj);
			} 
			cload.Close_Chunk();

			/*
			** Finish installing the object into the physics scene
			*/
			Internal_Add_Static_Light(obj);
			obj->Release_Ref();

		} else {
			WWDEBUG_SAY(("Unhandled Chunk: 0x%x in file &s, line %d\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
			cload.Close_Chunk();
		}
	}
}

void PhysicsSceneClass::Load_Sun_Light(ChunkLoadClass & cload)
{
	WWASSERT(SunLight != NULL);
	IOSunLightStruct sun;
	cload.Read(&sun,sizeof(sun));

	UseSun = (sun.Enabled == 1);
	Vector3 color;
	color.X = sun.Color.X;
	color.Y = sun.Color.Y;
	color.Z = sun.Color.Z;
	SunLight->Set_Diffuse(color);
	SunLight->Set_Intensity(sun.Intensity);
	Set_Sun_Light_Orientation(sun.Yaw,sun.Pitch);
}

void PhysicsSceneClass::Save_Sun_Light(ChunkSaveClass & csave)
{
	WWASSERT(SunLight != NULL);
	IOSunLightStruct sun;
	memset(&sun,0,sizeof(sun));
	
	sun.Enabled = (UseSun ? 1 : 0);
	sun.Yaw = SunYaw;
	sun.Pitch = SunPitch;
	sun.Intensity = SunLight->Get_Intensity();

	Vector3 c;
	SunLight->Get_Diffuse(&c);
	sun.Color.X = c.X;
	sun.Color.Y = c.Y;
	sun.Color.Z = c.Z;
	
	csave.Write(&sun,sizeof(sun));	
}	


void PhysicsSceneClass::Save_Dynamic_Objects(ChunkSaveClass & csave)
{
	/*
	** Save each object and save each object's culling linkage
	*/
	RefPhysListIterator it(&ObjList);
	for (it.First(); !it.Is_Done(); it.Next()) {
		
		PhysClass * obj = it.Peek_Obj();
		WWASSERT(obj != NULL);
		
		if ((obj) && (obj->Is_Dont_Save_Enabled() == false)) {		
			csave.Begin_Chunk(PSCENE_DD_CHUNK_DYNAMIC_OBJECT);
			csave.Begin_Chunk(obj->Get_Factory().Chunk_ID());
			obj->Get_Factory().Save(csave,obj);
			csave.End_Chunk();
			csave.End_Chunk();
		}
	}
}


void PhysicsSceneClass::Load_Dynamic_Objects(ChunkLoadClass & cload)
{
	while (cload.Open_Chunk()) {

		if (cload.Cur_Chunk_ID() == PSCENE_DD_CHUNK_DYNAMIC_OBJECT) {

			/*
			** Load the object
			*/ 
			cload.Open_Chunk();
			PhysClass * obj = NULL;
			PersistFactoryClass * fact = SaveLoadSystemClass::Find_Persist_Factory(cload.Cur_Chunk_ID());
			WWASSERT(fact != NULL);
			if (fact) {
				obj = (PhysClass *)fact->Load(cload);
			}
			cload.Close_Chunk();
			cload.Close_Chunk();
			
			WWASSERT(obj);
			
			/* 
			** Add the object to the dynamic culling system
			*/
			DynamicCullingSystem->Add_Object(obj);

			/*
			** Finish installing the object into the physics scene
			** - Add it to the culling system
			** - Add it to the appropriate list
			** - Notify the render object that it was added to the scene
			*/
			Internal_Add_Dynamic_Object(obj);
			obj->Release_Ref();

		} else {
			WWDEBUG_SAY(("Unhandled Chunk: 0x%x in file &s, line %d\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
			cload.Close_Chunk();
		}
	}
}

void PhysicsSceneClass::Save_Static_Object_States(ChunkSaveClass & csave)
{
	RefPhysListIterator it(&StaticObjList);
	while (!it.Is_Done()) {

		if (((StaticPhysClass*)it.Peek_Obj())->Has_Dynamic_State()) {
			csave.Begin_Chunk(PSCENE_DD_CHUNK_STATIC_OBJECT_ID);
			uint32 id = it.Peek_Obj()->Get_ID();
			csave.Write(&id,sizeof(uint32));
			csave.End_Chunk();

			csave.Begin_Chunk(PSCENE_DD_CHUNK_STATIC_OLD_PTR);
			void * old_ptr = it.Peek_Obj();
			const uint32 old_ptr_id =  SaveLoadSystemClass::Serialize_Pointer(old_ptr);
			csave.Write(&old_ptr_id,sizeof(uint32));
			csave.End_Chunk();

			csave.Begin_Chunk(PSCENE_DD_CHUNK_STATIC_OBJECT_STATE);
			((StaticPhysClass *)it.Peek_Obj())->Save_State(csave);
			csave.End_Chunk();
		}

		it.Next();
	}
}

void PhysicsSceneClass::Load_Static_Object_States(ChunkLoadClass & cload)
{
	while (cload.Open_Chunk()) {
		WWASSERT(cload.Cur_Chunk_ID()==PSCENE_DD_CHUNK_STATIC_OBJECT_ID);
		uint32 id;
		cload.Read(&id,sizeof(uint32));
		cload.Close_Chunk();

		StaticPhysClass * sphys = Get_Static_Object_By_ID(id);

		cload.Open_Chunk();
		WWASSERT(cload.Cur_Chunk_ID()==PSCENE_DD_CHUNK_STATIC_OLD_PTR);
		void * old_ptr = nullptr;
		cload.Read(&old_ptr,sizeof(int));
		SaveLoadSystemClass::Register_Pointer(old_ptr, sphys);
		cload.Close_Chunk();

		cload.Open_Chunk();
		WWASSERT(cload.Cur_Chunk_ID()==PSCENE_DD_CHUNK_STATIC_OBJECT_STATE);
		if (sphys != NULL) {
			sphys->Load_State(cload);
			sphys->Release_Ref();
		} else {
			WWDEBUG_SAY(("Unable to find static object! id = %d\r\n",id));
		}
		
		cload.Close_Chunk();
	}
}

void PhysicsSceneClass::Post_Load_Level_Dynamic_Data(void)
{
	/*
	** Possible TODO List: 
	** - Rebuild the dirty cull list?
	** - Rebuild the lists of vertex processors?  (tell each model that its been added?)
	*/

	/*
	** Re-generate the static shadows
	*/
//	Generate_Static_Shadow_Projectors();
	// We don't necessarily have active device at this point so we can't render the shadows yet.
	// The shadows will be generated as soon as the device is available at the start of the game.
	Invalidate_Static_Shadow_Projectors();
}

StaticPhysClass * PhysicsSceneClass::Get_Static_Object_By_ID(uint32 id)
{
	RefPhysListIterator it(&StaticObjList);
	while (!it.Is_Done()) {
		if (it.Peek_Obj()->Get_ID() == id) {
			return (StaticPhysClass *)it.Get_Obj();
		}
		it.Next();
	}
	return NULL;
}
