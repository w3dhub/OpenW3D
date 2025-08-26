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
 *                     $Archive:: /Commando/Code/wwphys/PathfindPortal.cpp                    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 4/20/01 8:49a                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "PathfindPortal.h"
#include "PathfindSector.h"
#include "Pathfind.h"
#include "chunkio.h"
#include "saveload.h"


///////////////////////////////////////////////////////////////////////////
//	Save/Load stuff
///////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES			= 0x01060654,
	CHUNKID_PARENT
};

enum
{
	VARID_BOUNDING_BOX		= 1,
	XXXXX_ACTION_REQUIRED,
	VARID_DEST_SECTOR1,
	VARID_DEST_SECTOR2,
	VARID_ID,
	VARID_OLD_PTR
};

enum
{
	ACTION_VARID_DESTINATION		= 1,
	ACTION_VARID_MECHANISM_ID,
	ACTION_VARID_ACTION_ID,
	ACTION_VARID_EXIT_PORTAL,
	ACTION_VARID_ENTER_PORTAL
};

enum
{
	WPATH_VARID_WAYPATH_POS		= 1,
};



///////////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////////
bool
PathfindPortalClass::Save (ChunkSaveClass &csave)
{	
	csave.Begin_Chunk (CHUNKID_VARIABLES);
		
		//
		//	Write the bounding box out to the chunk
		//
		WRITE_MICRO_CHUNK (csave, VARID_BOUNDING_BOX,	m_BoundingBox);
		WRITE_MICRO_CHUNK (csave, VARID_DEST_SECTOR1,	m_DestSector1);
		WRITE_MICRO_CHUNK (csave, VARID_DEST_SECTOR2,	m_DestSector2);
		WRITE_MICRO_CHUNK (csave, VARID_ID,					m_ID);		
		
		PathfindPortalClass *this_ptr = this;
		WRITE_MICRO_CHUNK (csave, VARID_OLD_PTR, this_ptr);

	csave.End_Chunk ();

	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load
//
///////////////////////////////////////////////////////////////////////////
bool
PathfindPortalClass::Load (ChunkLoadClass &cload)
{
	//
	//	Read all the chunks...
	//
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {			
			
			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;
			
			default:
				WWDEBUG_SAY (("Unknown chunk ID 0x%X", cload.Cur_Chunk_ID ()));
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
PathfindPortalClass::Load_Variables (ChunkLoadClass &cload)
{
	PathfindPortalClass *old_ptr = NULL;

	//
	//	Read all the micro chunks...
	//
	while (cload.Open_Micro_Chunk ()) {		
		switch (cload.Cur_Micro_Chunk_ID ()) {			
			
			READ_MICRO_CHUNK (cload, VARID_BOUNDING_BOX, m_BoundingBox);
			READ_MICRO_CHUNK (cload, VARID_DEST_SECTOR1, m_DestSector1);
			READ_MICRO_CHUNK (cload, VARID_DEST_SECTOR2, m_DestSector2);
			READ_MICRO_CHUNK (cload, VARID_ID,				m_ID);
			READ_MICRO_CHUNK (cload, VARID_OLD_PTR,		old_ptr);

			default:
				WWDEBUG_SAY (("Unknown micro chunk ID 0x%X", cload.Cur_Micro_Chunk_ID ()));
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

	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////////
bool
PathfindActionPortalClass::Save (ChunkSaveClass &csave)
{		
	//
	//	Save the parent to its own chunk
	//
	csave.Begin_Chunk (CHUNKID_PARENT);
		PathfindPortalClass::Save (csave);
	csave.End_Chunk ();

	//
	//	Save each variable to its own micro-chunk
	//
	csave.Begin_Chunk (CHUNKID_VARIABLES);
		WRITE_MICRO_CHUNK (csave, ACTION_VARID_DESTINATION,	m_Destination);
		WRITE_MICRO_CHUNK (csave, ACTION_VARID_MECHANISM_ID,	m_MechanismID);
		WRITE_MICRO_CHUNK (csave, ACTION_VARID_ACTION_ID,		m_ActionID);
		WRITE_MICRO_CHUNK (csave, ACTION_VARID_EXIT_PORTAL,	m_ExitPortal);
		WRITE_MICRO_CHUNK (csave, ACTION_VARID_ENTER_PORTAL,	m_EnterPortal);
	csave.End_Chunk ();

	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load
//
///////////////////////////////////////////////////////////////////////////
bool
PathfindActionPortalClass::Load (ChunkLoadClass &cload)
{
	//
	//	Read all the chunks...
	//
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {			
			
			case CHUNKID_PARENT:
				PathfindPortalClass::Load (cload);
				break;

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;
			
			default:
				WWASSERT (0);
				WWDEBUG_SAY (("Unknown chunk ID 0x%X", cload.Cur_Chunk_ID ()));
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
PathfindActionPortalClass::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Read all the micro chunks...
	//
	while (cload.Open_Micro_Chunk ()) {		
		switch (cload.Cur_Micro_Chunk_ID ()) {			
			
			READ_MICRO_CHUNK (cload, ACTION_VARID_DESTINATION,		m_Destination);
			READ_MICRO_CHUNK (cload, ACTION_VARID_MECHANISM_ID,	m_MechanismID);
			READ_MICRO_CHUNK (cload, ACTION_VARID_ACTION_ID,		m_ActionID);
			READ_MICRO_CHUNK (cload, ACTION_VARID_EXIT_PORTAL,		m_ExitPortal);
			READ_MICRO_CHUNK (cload, ACTION_VARID_ENTER_PORTAL,	m_EnterPortal);

			default:
				WWASSERT (0);
				WWDEBUG_SAY (("Unknown micro chunk ID 0x%X", cload.Cur_Micro_Chunk_ID ()));
				break;
		}

		cload.Close_Micro_Chunk ();
	}
	
	
	//
	//	Note:  These portals ARE ref-counted, but due to a cyclic ref-counting
	// problem, we don't want a reference on them.  This is why we don't
	// do a ref-counted pointer remap.
	//
	if (m_EnterPortal != NULL) {
		REQUEST_POINTER_REMAP ((void **)&m_EnterPortal);
	}

	if (m_ExitPortal != NULL) {
		REQUEST_POINTER_REMAP ((void **)&m_ExitPortal);
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////////
bool
PathfindWaypathPortalClass::Save (ChunkSaveClass &csave)
{		
	//
	//	Save the parent to its own chunk
	//
	csave.Begin_Chunk (CHUNKID_PARENT);
		PathfindPortalClass::Save (csave);
	csave.End_Chunk ();

	//
	//	Save each variable to its own micro-chunk
	//
	csave.Begin_Chunk (CHUNKID_VARIABLES);
		WRITE_MICRO_CHUNK (csave, WPATH_VARID_WAYPATH_POS,	WaypathPos);
	csave.End_Chunk ();

	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load
//
///////////////////////////////////////////////////////////////////////////
bool
PathfindWaypathPortalClass::Load (ChunkLoadClass &cload)
{
	//
	//	Read all the chunks...
	//
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {			
			
			case CHUNKID_PARENT:
				PathfindPortalClass::Load (cload);
				break;

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;
			
			default:
				WWASSERT (0);
				WWDEBUG_SAY (("Unknown chunk ID 0x%X", cload.Cur_Chunk_ID ()));
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
PathfindWaypathPortalClass::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Read all the micro chunks...
	//
	while (cload.Open_Micro_Chunk ()) {		
		switch (cload.Cur_Micro_Chunk_ID ()) {			
			
			READ_MICRO_CHUNK (cload, WPATH_VARID_WAYPATH_POS,	WaypathPos);

			default:
				WWASSERT (0);
				WWDEBUG_SAY (("Unknown micro chunk ID 0x%X", cload.Cur_Micro_Chunk_ID ()));
				break;
		}

		cload.Close_Micro_Chunk ();
	}
	
	return true;
}

