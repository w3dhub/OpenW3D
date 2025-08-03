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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/PathfindSector.cpp                    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 4/23/01 11:52a                                              $*
 *                                                                                             *
 *                    $Revision:: 12                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "PathfindSector.h"
#include "phys3.h"
#include "boxrobj.h"
#include "chunkio.h"
#include "wwphysids.h"
#include "pscene.h"
#include "decophys.h"
#include "Pathfind.h"
#include "waypath.h"
#include "PathfindPortal.h"


///////////////////////////////////////////////////////////////////////////
//	Save/Load stuff
///////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES			= 0x01060643,

	CHUNKID_WP_VARIABLES		= 0x01030550,
	CHUNKID_WP_PARENT
};

enum
{
	VARID_BOUNDING_BOX	= 1,
	VARID_PORTAL_ID,

	VARID_WAYPATH_ID		= 1,
};


////////////////////////////////////////////////////////////////////////////////////
//
//	~PathfindSectorClass
//
////////////////////////////////////////////////////////////////////////////////////
PathfindSectorClass::~PathfindSectorClass (void)
{
	Reset_Portal_List ();
	return ;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Reset_Portal_List
//
////////////////////////////////////////////////////////////////////////////////////
void
PathfindSectorClass::Reset_Portal_List (void)
{	
	m_PortalList.Delete_All ();	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////////
bool
PathfindSectorClass::Save (ChunkSaveClass &csave)
{	
	csave.Begin_Chunk (CHUNKID_VARIABLES);
		
		//
		//	Write the bounding box out to the chunk
		//
		AABoxClass bounding_box = Get_Cull_Box ();
		WRITE_MICRO_CHUNK (csave, VARID_BOUNDING_BOX, bounding_box);

		//
		//	Write the portal-id for each portal out to the chunk
		//
		for (int index = 0; index < m_PortalList.Count (); index ++) {
			int portal_id = m_PortalList[index];
			if (portal_id < PathfindClass::TEMP_PORTAL_ID_START) {
				WRITE_MICRO_CHUNK (csave, VARID_PORTAL_ID, portal_id);
			}
		}

	csave.End_Chunk ();

	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load
//
///////////////////////////////////////////////////////////////////////////
bool
PathfindSectorClass::Load (ChunkLoadClass &cload)
{
	Reset_Portal_List ();


	//
	//	Read all the chunks...
	//
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {			
			
			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;
			
			default:
				WWDEBUG_SAY (("Unknown chunk ID 0x%X\n", cload.Cur_Chunk_ID ()));
				break;
		}

		cload.Close_Chunk ();
	}
	
	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////////
bool
PathfindSectorClass::Load_Variables (ChunkLoadClass &cload)
{
	AABoxClass bounding_box;
	PathfindSectorClass *old_ptr = NULL;

	//
	//	Read all the micro chunks...
	//
	while (cload.Open_Micro_Chunk ()) {		
		switch (cload.Cur_Micro_Chunk_ID ()) {			
			
			READ_MICRO_CHUNK (cload, VARID_BOUNDING_BOX, bounding_box);
			
			case VARID_PORTAL_ID:
			{
				uint32 portal_id = 0;				
				cload.Read (&portal_id, sizeof (portal_id));
				if (portal_id < PathfindClass::TEMP_PORTAL_ID_START) {
					m_PortalList.Add (portal_id);
				}
			}
			break;
			
			default:
				WWDEBUG_SAY (("Unknown micro chunk ID 0x%X\n", cload.Cur_Micro_Chunk_ID ()));
				break;
		}

		cload.Close_Micro_Chunk ();
	}

	//
	//	Register our old ptr so other objects can remap to us
	//
	if (old_ptr != NULL) {
		SaveLoadSystemClass::Register_Pointer (old_ptr, this);
	}
	
	Set_Bounding_Box (bounding_box);
	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//	Peek_Portal
//
///////////////////////////////////////////////////////////////////////////
PathfindPortalClass *
PathfindSectorClass::Peek_Portal (int index)
{
	int portal_id = m_PortalList[index];
	return PathfindClass::Get_Instance ()->Peek_Portal (portal_id);
}


///////////////////////////////////////////////////////////////////////////
//
//	Remove_Portal
//
///////////////////////////////////////////////////////////////////////////
void
PathfindSectorClass::Remove_Portal (uint32 portal_id)
{
	//
	//	Loop over all the portals until we've found the one
	// we want to remove -- then remove it.
	//
	for (int index = 0; index < m_PortalList.Count (); index ++) {
		if (portal_id == m_PortalList[index]) {
			m_PortalList.Delete (index);
			break;
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////////
bool
PathfindWaypathSectorClass::Save (ChunkSaveClass &csave)
{	
	csave.Begin_Chunk (CHUNKID_WP_PARENT);
		PathfindSectorClass::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_WP_VARIABLES);		
		WRITE_MICRO_CHUNK (csave, VARID_WAYPATH_ID, WaypathID);
	csave.End_Chunk ();

	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load
//
///////////////////////////////////////////////////////////////////////////
bool
PathfindWaypathSectorClass::Load (ChunkLoadClass &cload)
{
	//
	//	Read all the chunks...
	//
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {			
			
			case CHUNKID_WP_PARENT:
				PathfindSectorClass::Load (cload);
				break;

			case CHUNKID_WP_VARIABLES:
				Load_Variables (cload);
				break;
			
			default:
				WWDEBUG_SAY (("Unknown chunk ID 0x%X\n", cload.Cur_Chunk_ID ()));
				break;
		}

		cload.Close_Chunk ();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////////
bool
PathfindWaypathSectorClass::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Read all the micro chunks...
	//
	while (cload.Open_Micro_Chunk ()) {		
		switch (cload.Cur_Micro_Chunk_ID ()) {			
			
			READ_MICRO_CHUNK (cload, VARID_WAYPATH_ID, WaypathID);
			
			default:
				WWDEBUG_SAY (("Unknown micro chunk ID 0x%X\n", cload.Cur_Micro_Chunk_ID ()));
				break;
		}

		cload.Close_Micro_Chunk ();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//	Can_Access_Portal
//
///////////////////////////////////////////////////////////////////////////
bool
PathfindWaypathSectorClass::Can_Access_Portal
(
	PathfindPortalClass *last_portal,
	PathfindPortalClass *test_portal
)
{
	bool retval = false;

	//
	//	Check to see if the base class requirements are met
	//	
	if (PathfindSectorClass::Can_Access_Portal (last_portal, test_portal)) {
		
		//
		//	Check to see if the waypath is one-way, if so we need to make sure
		// the portal we are testing is "ahead" of the portal 
		//
		WaypathClass *waypath = PathfindClass::Get_Instance ()->Find_Waypath (WaypathID);
		if (waypath != NULL && waypath->Get_Flag (WaypathClass::FLAG_TWO_WAY) == false) {
			
			//
			//	Check to see if the portal we entered is "ahead" of the portal
			// we are testing
			//
			for (int index = 0; index < m_PortalList.Count (); index ++) {
				if (m_PortalList[index] == last_portal->Get_ID ()) {
					retval = true;
					break;
				} else if (m_PortalList[index] == test_portal->Get_ID ()) {
					break;
				}
			}
			
		} else {
			
			//
			//	On a two-way waypath, we don't care which direction the portal
			// is from the one we entered
			//
			retval = true;
		}
	}

	return retval;
}


