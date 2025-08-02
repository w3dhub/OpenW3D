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
 *                     $Archive:: /Commando/Code/wwphys/physstaticsavesystem.cpp              $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/29/00 3:58p                                              $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "physstaticsavesystem.h"
#include "wwphysids.h"
#include "pscene.h"
#include "Pathfind.h"
#include "chunkio.h"
#include "wwmemlog.h"
#include "saveload.h"
#include "rendobj.h"
#include "phys.h"

/*
** Instantiate the Physics Static-Data-Save-Systems
*/
PhysStaticDataSaveSystemClass			_PhysStaticDataSaveSystem;
PhysStaticObjectsSaveSystemClass		_PhysStaticObjectsSaveSystem;


/**************************************************************************************
**
** PhysStaticDataSaveSystemClass Implementation
**
**************************************************************************************/
uint32 PhysStaticDataSaveSystemClass::Chunk_ID(void) const
{
	return PHYSICS_CHUNKID_STATIC_DATA_SUBSYSTEM;
}

bool PhysStaticDataSaveSystemClass::Save(ChunkSaveClass &csave)
{	
	WWMEMLOG(MEM_GAMEDATA);

	csave.Begin_Chunk(PSDSSC_CHUNKID_SCENE);
	PhysicsSceneClass::Get_Instance()->Save_Level_Static_Data(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(PSDSSC_CHUNKID_PATHFIND);
	PathfindClass::Get_Instance()->Save(csave);
	csave.End_Chunk();
	return true;
}

bool PhysStaticDataSaveSystemClass::Load(ChunkLoadClass &cload)
{
	WWMEMLOG(MEM_GAMEDATA);

	while (cload.Open_Chunk()) {
		switch (cload.Cur_Chunk_ID()) 
		{
			case PSDSSC_CHUNKID_SCENE:
				PhysicsSceneClass::Get_Instance()->Load_Level_Static_Data(cload);
				break;

			case PSDSSC_CHUNKID_PATHFIND:
				PathfindClass::Get_Instance()->Load(cload);
				break;
		}
		cload.Close_Chunk();
	}
	SaveLoadSystemClass::Register_Post_Load_Callback(this);
	return true;
}

void PhysStaticDataSaveSystemClass::On_Post_Load(void)
{
	PhysicsSceneClass::Get_Instance()->Post_Load_Level_Static_Data();
	//PathfindClass::Get_Instance()->On_Post_Load();
}


/**************************************************************************************
**
** PhysStaticObjectsSaveSystemClass Implementation
**
**************************************************************************************/
uint32 PhysStaticObjectsSaveSystemClass::Chunk_ID(void) const
{
	return PHYSICS_CHUNKID_STATIC_OBJECTS_SUBSYSTEM;
}

bool PhysStaticObjectsSaveSystemClass::Save(ChunkSaveClass &csave)
{	
	csave.Begin_Chunk(PSOSSC_CHUNKID_SCENE);
	PhysicsSceneClass::Get_Instance()->Save_Level_Static_Objects(csave);
	csave.End_Chunk();

	return true;
}

bool PhysStaticObjectsSaveSystemClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		switch (cload.Cur_Chunk_ID()) 
		{
			case PSOSSC_CHUNKID_SCENE:
				PhysicsSceneClass::Get_Instance()->Load_Level_Static_Objects(cload);
				break;

		}
		cload.Close_Chunk();
	}
	return true;
}

void PhysStaticObjectsSaveSystemClass::On_Post_Load(void)
{
	PhysicsSceneClass::Get_Instance()->Post_Load_Level_Static_Objects();
}

