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
 *                     $Archive:: /Commando/Code/wwphys/lightcull.cpp                         $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 4/02/01 12:23p                                              $*
 *                                                                                             *
 *                    $Revision:: 18                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "lightcull.h"
#include "lightphys.h"
#include "ww3d.h"
#include "rinfo.h"
#include "chunkio.h"
#include "wwphysids.h"
#include "camera.h"



/*
** Current version of the lightcull file format
*/
const uint32 STATICLIGHTCULL_CURRENT_VERSION		= 0x00010000;

/*
** Chunk Id's used by the phys-aabtree code to save itself into a file
*/
enum
{
	STATICLIGHTCULL_CHUNK_VERSION						= 0x00050001,	// version wrapper, contains 32bit version #
	STATICLIGHTCULL_CHUNK_PARENT_CLASS_DATA		= 0x00050104,	// wraps the parent class's save data
};




/*
** StaticLightCullClass is a derived AABTree which assumes it contains LightPhysClasses
** these two functions encapsulate some typecasting which happens in a lot
** of places...
*/
inline LightPhysClass * get_first_object(AABTreeNodeClass * node)
{
	return (LightPhysClass *)(node->Object);
}

inline LightPhysClass * get_next_object(LightPhysClass * tile)
{
	return (LightPhysClass *)(((AABTreeLinkClass *)tile->Get_Cull_Link())->NextObject);
}



/*
** Implementation of StaticLightCullClass
*/
StaticLightCullClass::StaticLightCullClass(void)
{
}

StaticLightCullClass::~StaticLightCullClass(void)
{
}

void StaticLightCullClass::Add_Object(LightPhysClass * obj)
{
	WWASSERT(PhysicsSceneClass::Get_Instance() != NULL);
	TypedAABTreeCullSystemClass<LightPhysClass>::Add_Object(obj);
// (gth) not resetting vis when adding a new static light
//	PhysicsSceneClass::Get_Instance()->Reset_Vis();
}

void StaticLightCullClass::Remove_Object(LightPhysClass * obj)
{
	WWASSERT(PhysicsSceneClass::Get_Instance() != NULL);
	TypedAABTreeCullSystemClass<LightPhysClass>::Remove_Object(obj);
// (gth) not resetting vis when removing a static light
//	PhysicsSceneClass::Get_Instance()->Reset_Vis();
}

void StaticLightCullClass::Update_Culling(CullableClass * obj)
{
	WWASSERT(PhysicsSceneClass::Get_Instance() != NULL);
	TypedAABTreeCullSystemClass<LightPhysClass>::Update_Culling(obj);
// (gth) not resetting vis when moving a static light
//	PhysicsSceneClass::Get_Instance()->Reset_Vis();
}

void StaticLightCullClass::Save_Static_Data(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(STATICLIGHTCULL_CHUNK_PARENT_CLASS_DATA);
	TypedAABTreeCullSystemClass<LightPhysClass>::Save(csave);
	csave.End_Chunk();
}

void StaticLightCullClass::Load_Static_Data(ChunkLoadClass & cload)
{
	while(cload.Open_Chunk())
	{
		switch(cload.Cur_Chunk_ID())
		{

		case STATICLIGHTCULL_CHUNK_PARENT_CLASS_DATA:
			{
				TypedAABTreeCullSystemClass<LightPhysClass>::Load(cload);
			}
			break;

		default:
			WWDEBUG_SAY(("Unhandled chunk type: %d in LightCullClass::Load\r\n",cload.Cur_Chunk_ID()));
			break;
		}

		cload.Close_Chunk();
	}
}

void StaticLightCullClass::Assign_Vis_IDs(void)
{
	/*
	** Allocate sector ID's for each light.
	*/
	int nodecount = Partition_Node_Count();

	for (int i=0; i<nodecount; i++) {
		AABTreeNodeClass * node = IndexedNodes[i];

		LightPhysClass * obj = get_first_object(node);
		while(obj) {
			obj->Set_Vis_Sector_ID(PhysicsSceneClass::Get_Instance()->Allocate_Vis_Sector_ID());
			obj = get_next_object(obj);
		}
	}
}

void StaticLightCullClass::Merge_Vis_Sector_IDs(uint32 id0,uint32 id1)
{
	/*
	** Each lightphys object may have a vis sector id.
	** Whenever we encounter one of these with id1, set it to id0.
	** Whenever we encounter one with id > id1, subtract one from it.
	*/
	for (int i=0; i<NodeCount; i++) {

		AABTreeNodeClass * node = IndexedNodes[i];

		LightPhysClass * obj = get_first_object(node);
		while (obj) {

			uint32 sector_id = obj->Get_Vis_Sector_ID();
			if (sector_id != 0xFFFFFFFF) {
				if (sector_id == id1) {
					sector_id = id0;
				} else if (sector_id > id1) {
					sector_id--;
				}

				obj->Set_Vis_Sector_ID(sector_id);
			}
			obj = get_next_object(obj);
		}
	}
}
