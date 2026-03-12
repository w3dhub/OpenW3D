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
 *                     $Archive:: /Commando/Code/wwphys/physdynamicsavesystem.cpp             $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/29/00 3:58p                                              $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "physdynamicsavesystem.h"
#include "wwphysids.h"
#include "pscene.h"
#include "physcon.h"
#include "chunkio.h"
#include "wwmemlog.h"
#include "pathmgr.h"
#include "saveload.h"
#include "rendobj.h"
#include "phys.h"

/*
** Instantiate the Physics Dynamic-Data-Save-System
*/
PhysDynamicSaveSystemClass _PhysDynamicSaveSystem;


uint32 PhysDynamicSaveSystemClass::Chunk_ID(void) const
{
	return PHYSICS_CHUNKID_DYNAMIC_DATA_SUBSYSTEM;
}

bool PhysDynamicSaveSystemClass::Contains_Data(void) const
{
	// TODO: could check if we have anything to save...
	return true;
}

bool PhysDynamicSaveSystemClass::Save(ChunkSaveClass &csave)
{
	WWMEMLOG(MEM_GAMEDATA);

	csave.Begin_Chunk(PDSSC_CHUNKID_SCENE);
	PhysicsSceneClass::Get_Instance()->Save_Level_Dynamic_Data(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(PDSSC_CHUNKID_CONSTANTS);
	PhysicsConstants::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(PDSSC_CHUNKID_PATHMGR);
	PathMgrClass::Save(csave);
	csave.End_Chunk();

	return true;
}

bool PhysDynamicSaveSystemClass::Load(ChunkLoadClass &cload)
{
	WWMEMLOG(MEM_GAMEDATA);

	while (cload.Open_Chunk()) {
		switch (cload.Cur_Chunk_ID()) {
		case PDSSC_CHUNKID_SCENE:
			PhysicsSceneClass::Get_Instance()->Load_Level_Dynamic_Data(cload);
			break;
		case PDSSC_CHUNKID_CONSTANTS:
			PhysicsConstants::Load(cload);
			break;
		case PDSSC_CHUNKID_PATHMGR:
			PathMgrClass::Load(cload);
			break;
		}
		cload.Close_Chunk();
	}

	SaveLoadSystemClass::Register_Post_Load_Callback(this);
	return true;
}


void PhysDynamicSaveSystemClass::On_Post_Load(void)
{
	PhysicsSceneClass::Get_Instance()->Post_Load_Level_Dynamic_Data();
}


