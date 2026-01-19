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
 *                     $Archive:: /Commando/Code/wwphys/Pathfind.cpp                          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/15/01 7:44p                                              $*
 *                                                                                             *
 *                    $Revision:: 35                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "Pathfind.h"
#include "PathfindPortal.h"
#include "Path.h"
#include "pscene.h"
#include "boxrobj.h"
#include "decophys.h"
#include "mesh.h"
#include "assetmgr.h"
#include "chunkio.h"
#include "wwphysids.h"
#include "PathDebugPlotter.h"
#include "persistfactory.h"
#include "waypath.h"
#include "waypoint.h"
#include "wwmemlog.h"
#include "heightdb.h"
#include "colmathaabox.h"
#include <algorithm>


///////////////////////////////////////////////////////////////////////////
//	Local prototypes
///////////////////////////////////////////////////////////////////////////
int __cdecl fnCompareWaypathPortalsCallback (const void *elem1, const void *elem2);
bool Find_Intersection_Point (const AABoxClass &box, const Vector3 &p0, const Vector3 &p1, float *percent, Vector3 *intersection_point);
void Add_New_Portal_To_List (DynamicVectorClass<PathfindWaypathPortalClass *> &portal_list, PathfindSectorClass *dest_sector1, PathfindSectorClass *dest_sector2, int waypath_id, int waypoint_index, const Vector3 &portal_pos, float percent);


///////////////////////////////////////////////////////////////////////////
//	Constants
///////////////////////////////////////////////////////////////////////////
const int MAX_TEMP_PORTALS				= 20;


///////////////////////////////////////////////////////////////////////////
//	Static data initialization
///////////////////////////////////////////////////////////////////////////
static int _NextTempPortalID		= PathfindClass::TEMP_PORTAL_ID_START;


///////////////////////////////////////////////////////////////////////////
//	Save/Load stuff
///////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_DATABASE							= 0x01060635,
	XXX_CHUNKID_SECTOR,
	CHUNKID_PORTAL,
	CHUNKID_SECTOR_CULLING_SYSTEM,
	CHUNKID_SECTOR,
	CHUNKID_SECTOR_CULL_TREE,
	CHUNKID_SECTOR_LINKAGE,
	CHUNKID_SECTOR_OBJECT,
	CHUNKID_HEIGHTDB,
	CHUNKID_ACTION_PORTAL,
	CHUNKID_WAYPATH_PORTAL,
	CHUNKID_PATHFIND_SECTOR_OBJECT
};


///////////////////////////////////////////////////////////////////////////
//	Static member initialization
///////////////////////////////////////////////////////////////////////////
PathfindClass *PathfindClass::_Pathfinder	= NULL;
int PathfindClass::_MemoryFootprint			= 0;


///////////////////////////////////////////////////////////////////////////
//
//	PathfindClass
//
///////////////////////////////////////////////////////////////////////////
PathfindClass::PathfindClass (void)
	:	m_SectorsDisplayed (false),
		m_PortalsDisplayed (false),
		m_Plotter (NULL),
		m_SectorList(1000),		// 1000's of these
		m_PortalList(5000),		// 10.000's of these
		m_SectorDisplayList(1000),	// 1000's of these (debug only)
		m_WaypathList(30)		// 5-30 of these?
{
	WWASSERT (_Pathfinder == NULL);
	_Pathfinder = this;

	// Set good estimates of growth steps
	m_SectorList.Set_Growth_Step(500);
	m_PortalList.Set_Growth_Step(2500);
	m_SectorDisplayList.Set_Growth_Step(500);
	m_WaypathList.Set_Growth_Step(10);

	m_Plotter = new PathDebugPlotterClass;

	//
	//	Initialize the height database
	//
	HeightDBClass::Initialize ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	~PathfindClass
//
///////////////////////////////////////////////////////////////////////////
PathfindClass::~PathfindClass (void)
{		
	Reset_Sectors ();
	Reset_Portals ();
	Reset_Waypaths ();
	_Pathfinder = NULL;

	REF_PTR_RELEASE (m_Plotter);

	//
	//	Free the height database
	//
	HeightDBClass::Shutdown ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Add_Sector
//
///////////////////////////////////////////////////////////////////////////
void
PathfindClass::Add_Sector (PathfindSectorClass *sector, bool add_to_tree)
{
	WWASSERT (sector != NULL);
	if (sector != NULL) {
		sector->Add_Ref ();
		
		//
		//	Add this sector to the culling system (if needs be)
		//
		if (add_to_tree) {
			m_SectorTree.Add_Object (sector);
		}

		//
		//	Add this sector to our linear list (for housekeeping)
		//
		m_SectorList.Add (sector);

		//
		//	For debugging purposes, add this object to our
		// memory footprint counter.
		//
		_MemoryFootprint += sizeof (PathfindSectorClass);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Add_Portal
//
///////////////////////////////////////////////////////////////////////////
int
PathfindClass::Add_Portal (PathfindPortalClass *portal)
{
	//
	//	Add this portal to the housekeeping list
	//
	portal->Add_Ref ();
	m_PortalList.Add (portal);

	//
	//	Assign the portal a unique ID
	//
	int portal_id = m_PortalList.Count () - 1;
	portal->Set_ID (portal_id);

	//
	//	Add the size of this portal to the pool.
	//
	_MemoryFootprint += sizeof (PathfindPortalClass);

	return portal_id;
}


///////////////////////////////////////////////////////////////////////////
//
//	Add_Waypath_Portal
//
///////////////////////////////////////////////////////////////////////////
int
PathfindClass::Add_Waypath_Portal (PathfindWaypathPortalClass *portal)
{
	//
	//	Add this portal to the housekeeping list
	//
	portal->Add_Ref ();
	m_WaypathPortalList.Add (portal);
	
	//
	//	Assign the portal a unique ID
	//
	int portal_id = (m_WaypathPortalList.Count () - 1) + WAYPATH_PORTAL_ID_START;
	portal->Set_ID (portal_id);

	//
	//	Add the size of this portal to the pool.
	//
	_MemoryFootprint += sizeof (PathfindWaypathPortalClass);

	return portal_id;
}


///////////////////////////////////////////////////////////////////////////
//
//	Add_Temporary_Portal
//
///////////////////////////////////////////////////////////////////////////
int
PathfindClass::Add_Temporary_Portal
(
	PathfindSectorClass *sector_from,
	PathfindSectorClass *sector_to,
	const Vector3 &		start_pos,
	const Vector3 &		dest_pos
)
{
	int retval = 0;

	if (sector_from != NULL && sector_to != NULL) {
		
		//
		//	Check to see if there is already a portal between the two sectors
		//
		bool found = false;
		for (int index = 0; index < sector_from->Get_Portal_Count (); index ++) {
			PathfindPortalClass *portal = sector_from->Peek_Portal (index);
			PathfindSectorClass *sector = portal->Peek_Dest_Sector (sector_from);

			//
			//	Is this portal's destination sector the one we are looking for?
			//
			if (sector == sector_to) {
				found = true;
				break;
			}
		}

		//
		//	If there isn't already a portal between the sectors, then add one...
		//
		if (found == false) {

			AABoxClass portal_box (start_pos, Vector3 (0.25F, 0.25F, 1.0F));

			PathfindActionPortalClass *new_portal = new PathfindActionPortalClass;
			new_portal->Set_Bounding_Box (portal_box);
			new_portal->Add_Dest_Sector (sector_to);
			new_portal->Set_Entrance_Sector (sector_from);
			new_portal->Set_Action_Type (PathClass::ACTION_LEAP);
			new_portal->Set_Destination (dest_pos);
			new_portal->Set_ID (_NextTempPortalID ++);
				
			//
			//	Add this portal to the housekeeping list
			//
			m_TemporaryPortalList.Add (new_portal);
			sector_from->Add_Portal (new_portal->Get_ID ());

			//
			//	Add the size of this portal to the pool.
			//
			_MemoryFootprint += sizeof (PathfindActionPortalClass);
			retval = new_portal->Get_ID ();

			//
			//	Have we reached the maximum number of temporary portals?
			//
			if (m_TemporaryPortalList.Count () > MAX_TEMP_PORTALS) {
				
				//
				//	Remove the first temp portal in the list
				//
				PathfindActionPortalClass *old_portal = (PathfindActionPortalClass *)m_TemporaryPortalList[0];
				if (old_portal != NULL) {
					
					//
					//	Remove the portal from which ever sectors reference it
					//
					PathfindSectorClass *sector = old_portal->Get_Entrance_Sector ();
					if (sector != NULL) {
						sector->Remove_Portal (old_portal->Get_ID ());
					}

					//
					//	Remove the portal from our list and free our hold on it
					//
					m_TemporaryPortalList.Delete (0);
					old_portal->Release_Ref ();
				}
			}
		}
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////////
bool
PathfindClass::Save (ChunkSaveClass &csave)
{
	WWMEMLOG(MEM_PATHFIND);

	csave.Begin_Chunk (CHUNKID_DATABASE);
	
		//
		//	Save the sectors and portals
		//
		bool retval =	Save_Culling_System (csave);
		retval &=		Save_Portals (csave);
		retval &=		Save_Waypaths (csave);

		//
		//	Save the height database for flying vehicles
		//
		csave.Begin_Chunk (CHUNKID_HEIGHTDB);
			HeightDBClass::Save (csave);
		csave.End_Chunk ();

	csave.End_Chunk ();

	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Save_Portals
//
///////////////////////////////////////////////////////////////////////////
bool
PathfindClass::Save_Portals (ChunkSaveClass &csave)
{
	bool retval = true;

	int count = m_PortalList.Count ();
	int index;
	for (index = 0; index < count && retval; index ++) {		
		PathfindPortalClass *portal = m_PortalList[index];

		//
		//	Determine what type of chunk to save this portal in
		//
		if (portal->As_PathfindActionPortalClass () != NULL) {
			csave.Begin_Chunk (CHUNKID_ACTION_PORTAL);
		} else {
			csave.Begin_Chunk (CHUNKID_PORTAL);
		}

		//
		//	Write this portal out ot its own chunk
		//
			retval &= portal->Save (csave);
		csave.End_Chunk ();
	}

	//
	//	Now save the waypath portals
	//
	count = m_WaypathPortalList.Count ();
	for (index = 0; index < count && retval; index ++) {		
		PathfindPortalClass *portal = m_WaypathPortalList[index];

		//
		//	Write this portal out ot its own chunk
		//
		csave.Begin_Chunk (CHUNKID_WAYPATH_PORTAL);
			retval &= portal->Save (csave);
		csave.End_Chunk ();
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load
//
///////////////////////////////////////////////////////////////////////////
bool
PathfindClass::Load (ChunkLoadClass &cload)
{
	WWMEMLOG(MEM_PATHFIND);

	Reset_Sectors ();
	Reset_Portals ();
	Reset_Waypaths ();

	bool retval = true;

	//
	//	Make sure we are reading data from the right file...
	//
	if (	cload.Open_Chunk () != true ||
			cload.Cur_Chunk_ID () != CHUNKID_DATABASE)
	{
		retval = false;
	}

	//
	//	Read all the chunks...
	//
	while (retval && cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {
			
			case CHUNKID_HEIGHTDB:
				retval &= HeightDBClass::Load (cload);
				break;

			case CHUNKID_SECTOR_CULLING_SYSTEM:
				retval &= Load_Culling_System (cload);
				break;

			case CHUNKID_PORTAL:
			{
				PathfindPortalClass *portal = new PathfindPortalClass;
				retval &= Load_Portal (cload, portal);
			}
			break;

			case CHUNKID_ACTION_PORTAL:
			{
				PathfindActionPortalClass *portal = new PathfindActionPortalClass;
				retval &= Load_Portal (cload, portal);
			}
			break;

			case CHUNKID_WAYPATH_PORTAL:
			{
				PathfindWaypathPortalClass *portal = new PathfindWaypathPortalClass;
				retval &= Load_Portal (cload, portal);
			}
			break;
			
			default:
			{
				//
				//	Load the object from the chunk (if possible)
				//
				PersistFactoryClass *factory = SaveLoadSystemClass::Find_Persist_Factory (cload.Cur_Chunk_ID ());
				if (factory != NULL) {
					PersistClass *object = factory->Load (cload);
					
					//
					//	Were we successful?
					//
					if (object != NULL) {
						
						//
						//	If this is a waypath, then register it with the system
						//
						if (cload.Cur_Chunk_ID () == PHYSICS_CHUNKID_WAYPATH) {
							Add_Waypath ((WaypathClass *)object);
							((WaypathClass *)object)->Release_Ref ();
						}

					} else {
						WWDEBUG_SAY (("Unknown chunk ID 0x%X\r\n", cload.Cur_Chunk_ID ()));
						retval = false;
					}
				}
			}
			break;
		}

		cload.Close_Chunk ();
	}

	cload.Close_Chunk ();
	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Save_Sector
//
///////////////////////////////////////////////////////////////////////////
bool
PathfindClass::Save_Sector
(
	ChunkSaveClass &			csave,
	PathfindSectorClass *	sector
)
{
	bool retval = true;

	csave.Begin_Chunk (CHUNKID_SECTOR);
		
		//
		//	Write this sector out to its own chunk
		//
		bool save_linkage = true;
		if (sector->As_PathfindWaypathSectorClass () != NULL) {
			csave.Begin_Chunk (CHUNKID_PATHFIND_SECTOR_OBJECT);
			save_linkage = false;
		} else {
			csave.Begin_Chunk (CHUNKID_SECTOR_OBJECT);
		}
		
			retval &= sector->Save (csave);
		csave.End_Chunk ();

		//
		//	Now write the sector's AABTreeCullSystem linkage to a chunk
		//
		if (save_linkage) {
			csave.Begin_Chunk (CHUNKID_SECTOR_LINKAGE);
				m_SectorTree.Save_Object_Linkage (csave, sector);
			csave.End_Chunk ();
		}

	csave.End_Chunk ();

	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load_Sector
//
///////////////////////////////////////////////////////////////////////////
bool
PathfindClass::Load_Sector (ChunkLoadClass &cload)
{
	bool retval							= true;
	PathfindSectorClass *sector	= NULL;

	//
	//	Read all the chunks...
	//
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {
			
			case CHUNKID_SECTOR_OBJECT:
				sector = new PathfindSectorClass;
				retval &= sector->Load (cload);
				break;

			case CHUNKID_PATHFIND_SECTOR_OBJECT:
				sector = new PathfindWaypathSectorClass;
				retval &= sector->Load (cload);

				//
				//	Add this sector to our housekeeping list
				//
				Add_Sector (sector, false);
				break;

			case CHUNKID_SECTOR_LINKAGE:
				WWASSERT (sector != NULL);
				if (sector != NULL) {
					
					//
					//	Read the linkage information from the chunk
					//					
					m_SectorTree.Load_Object_Linkage (cload, sector);

					//
					//	Add this sector to our housekeeping list
					//
					Add_Sector (sector, false);

				}				
				break;

			default:
				WWDEBUG_SAY (("Unknown chunk ID 0x%X\r\n", cload.Cur_Chunk_ID ()));
				retval = false;
				break;
		}

		cload.Close_Chunk ();
	}

	REF_PTR_RELEASE (sector);
	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load_Portal
//
///////////////////////////////////////////////////////////////////////////
bool
PathfindClass::Load_Portal (ChunkLoadClass &cload, PathfindPortalClass *portal)
{
	//
	//	Load the portal's data from its chunk
	// and add it to our global portal list.
	//
	bool retval	= portal->Load (cload);
	if (retval) {

		//
		//	Now add this portal to its list
		//
		if (portal->As_PathfindWaypathPortalClass () == NULL) {
			m_PortalList.Add (portal);
		} else {
			m_WaypathPortalList.Add (portal->As_PathfindWaypathPortalClass ());
		}

		//
		//	Add the size of this portal to the pool.
		//
		_MemoryFootprint += sizeof (PathfindPortalClass);
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Sector_Index
//
///////////////////////////////////////////////////////////////////////////
int
PathfindClass::Get_Sector_Index (PathfindSectorClass *sector)
{
	int index = m_SectorList.Count ();

	bool keep_going = true;
	while (keep_going && index --) {
		keep_going = (m_SectorList[index] != sector);
	}

	return index;
}


static inline bool Clip_Point (Vector3 *point, const AABoxClass &box)
{
	Vector3 temp_point = *point;

	//
	//	Clip the temporary point
	//
	temp_point.X = std::max (temp_point.X, box.Center.X - box.Extent.X);
	temp_point.Y = std::max (temp_point.Y, box.Center.Y - box.Extent.Y);
	temp_point.Z = std::max (temp_point.Z, box.Center.Z - box.Extent.Z);
	temp_point.X = std::min (temp_point.X, box.Center.X + box.Extent.X);
	temp_point.Y = std::min (temp_point.Y, box.Center.Y + box.Extent.Y);
	temp_point.Z = std::min (temp_point.Z, box.Center.Z + box.Extent.Z);

	//
	//	Did the clip change the point?
	//
	bool retval = (point->X != temp_point.X);
	retval		|= (point->Y != temp_point.Y);
	retval		|= (point->Z != temp_point.Z);
	
	//
	//	Pass the new point back to the caller
	//
	(*point) = temp_point;
	
	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Collect_Sectors
//
///////////////////////////////////////////////////////////////////////////
void
PathfindClass::Collect_Sectors
(
	DynamicVectorClass<PathfindSectorClass *> &	list,
	const AABoxClass &									box,
	PathfindSectorClass *								exclude_sector
)
{
	//
	//	Collect all the sectors this box intersects
	//
	m_SectorTree.Reset_Collection ();
	m_SectorTree.Collect_Objects (box);

	//
	//	Return the list of sectors to the caller
	//
	PathfindSectorClass *sector = NULL;
	for (	sector = m_SectorTree.Get_First_Collected_Object ();
			sector != NULL;
			sector = m_SectorTree.Get_Next_Collected_Object (sector))
	{		
		if (sector != exclude_sector) {

			//
			//	Ignore sectors that are 0 size
			//
			const AABoxClass &bbox = sector->Get_Bounding_Box ();
			if (bbox.Extent.X != 0 && bbox.Extent.Y != 0 && bbox.Extent.Z != 0) {
				list.Add (sector);
			}
		}
	}	
		
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Find_Sector
//
///////////////////////////////////////////////////////////////////////////
PathfindSectorClass *
PathfindClass::Find_Sector
(
	const Vector3 &		position,
	float						sector_fudge,
	PathfindSectorClass *exclude_sector
)
{
	//
	//	Collect all the sectors this position intersects (hopefully
	// will be only one).
	//
	SphereClass sphere (position, sector_fudge);

	m_SectorTree.Reset_Collection ();
	m_SectorTree.Collect_Objects (sphere);

	float closest = 99999.9F;

	//
	//	Find an acceptable sector to return
	//
	PathfindSectorClass *closest_sector = NULL;
	PathfindSectorClass *sector = NULL;
	for (	sector = m_SectorTree.Get_First_Collected_Object ();
			sector != NULL;
			sector = m_SectorTree.Get_Next_Collected_Object (sector))
	{				
		if (sector != exclude_sector) {

			//
			//	Ignore sectors that are small
			//
			const AABoxClass &box = sector->Get_Bounding_Box ();
			if (box.Extent.X > 0.325F && box.Extent.Y > 0.325F && box.Extent.Z > WWMATH_EPSILON) {

				//
				//	Clip the player's position to this sector
				//
				Vector3 clipped_pos = position;
				::Clip_Point (&clipped_pos, box);

				//
				//	Check to see if this clipped position is the closest yet
				//
				float dist = (clipped_pos - position).Length ();
				if (dist < closest) {
					closest			= dist;
					closest_sector	= sector;				
				}
			}
		}
	}	
		
	//
	//	Return the first (and hopefully only) sector
	//
	return closest_sector;
}


///////////////////////////////////////////////////////////////////////////
//
//	Reset_Sectors
//
///////////////////////////////////////////////////////////////////////////
void
PathfindClass::Reset_Sectors (void)
{
	Display_Sectors (false);
	Display_Portals (false);

	//
	//	Release our hold on each of the sectors
	//
	for (int index = 0; index < m_SectorList.Count (); index ++) {
		PathfindSectorClass *sector = m_SectorList[index];
		
		//
		//	Remove this sector from the culling system (if necessary)
		//
		if (sector->As_PathfindWaypathSectorClass () == NULL) {
			m_SectorTree.Remove_Object (sector);
		}

		REF_PTR_RELEASE (sector);
	}

	m_SectorList.Delete_All ();

	_MemoryFootprint = 0;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Reset_Portals
//
///////////////////////////////////////////////////////////////////////////
void
PathfindClass::Reset_Portals (void)
{
	Display_Portals (false);

	//
	//	Release our hold on each of the portals
	//
	int index;
	for (index = 0; index < m_PortalList.Count (); index ++) {
		PathfindPortalClass *portal = m_PortalList[index];
		REF_PTR_RELEASE (portal);
	}

	//
	//	Release our hold on each of the temporary portals
	//
	for (index = 0; index < m_TemporaryPortalList.Count (); index ++) {
		PathfindPortalClass *portal = m_TemporaryPortalList[index];
		REF_PTR_RELEASE (portal);
	}

	//
	//	Release our hold on each of the waypath portals
	//
	for (index = 0; index < m_WaypathPortalList.Count (); index ++) {
		PathfindPortalClass *portal = m_WaypathPortalList[index];
		REF_PTR_RELEASE (portal);
	}

	//
	//	Reset the lists
	//
	m_PortalList.Delete_All ();
	m_TemporaryPortalList.Delete_All ();
	m_WaypathPortalList.Delete_All ();

	//
	//	Reset the starting IDs
	//
	_NextTempPortalID = TEMP_PORTAL_ID_START;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Display_Sectors
//
///////////////////////////////////////////////////////////////////////////
void
PathfindClass::Display_Sectors (bool onoff)
{
	//
	//	Start fresh
	//
	m_SectorDisplayWidgets.Reset_Debug_Widget_List ();

	if (onoff) {
		
		//
		//	Add boxes to our debug widget system to represent the sectors
		//
		for (int index = 0; index < m_SectorList.Count (); index ++) {
			PathfindSectorClass *sector		= m_SectorList[index];
			const AABoxClass &bounding_box	= sector->Get_Bounding_Box ();

			//
			//	Add a randomly colored box to the widget system
			//
			float red	= 0;
			float green = 0.5F + float(rand () % 128) / 256.0F;
			float blue	= 0.5F + float(rand () % 128) / 256.0F;
			m_SectorDisplayWidgets.Add_Debug_AABox (bounding_box, Vector3 (red, green, blue));
		}
	}

	m_SectorsDisplayed = onoff;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Display_Portals
//
///////////////////////////////////////////////////////////////////////////
void
PathfindClass::Display_Portals (bool onoff)
{
	//
	//	Start fresh
	//
	m_PortalDisplayWidgets.Reset_Debug_Widget_List ();

		//
		//	Add boxes to our debug widget system to represent the sectors
		//
	if (onoff) {

		for (int index = 0; index < m_PortalList.Count (); index ++) {
			PathfindPortalClass *portal = m_PortalList[index];

			AABoxClass bounding_box;
			portal->Get_Bounding_Box (bounding_box);

			//
			//	Choose a color for this box
			//
			Vector3 color (1, 1, 1);
			if (portal->Is_Two_Way_Portal () == false) {
				color.Set  (1, 0, 0);
			}

			//
			//	Add a box to the system
			//
			m_PortalDisplayWidgets.Add_Debug_AABox (bounding_box, color);
		}		
	}

	m_PortalsDisplayed = onoff;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Add_Waypath
//
///////////////////////////////////////////////////////////////////////////
void
PathfindClass::Add_Waypath (WaypathClass *waypath)
{
	if (waypath != NULL) {
		waypath->Add_Ref ();
		m_WaypathList.Add (waypath);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Remove_Waypath
//
///////////////////////////////////////////////////////////////////////////
bool
PathfindClass::Remove_Waypath (WaypathClass *waypath)
{
	bool retval = false;

	if (waypath != NULL) {
		
		int index = m_WaypathList.Count ();
		while (index -- && (retval == false)) {
			WaypathClass *curr_waypath = m_WaypathList[index];
			
			//
			//	If this is the waypath we are looking for
			// then remove it from the list and release our
			// hold on it.
			//
			if (curr_waypath == waypath) {
				REF_PTR_RELEASE (waypath);
				m_WaypathList.Delete (index);
				retval = true;
			}
		}		
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Find_Waypath
//
///////////////////////////////////////////////////////////////////////////
WaypathClass *
PathfindClass::Find_Waypath (int id) const
{
	WaypathClass *waypath = NULL;

	//
	//	Loop over all the paths in our list until we've
	// found the one are looking for.
	//
	int index = m_WaypathList.Count ();
	while (index -- && (waypath == NULL)) {
		WaypathClass *curr_waypath = m_WaypathList[index];
		
		//
		//	Is this the waypath we are looking for?
		//
		if (curr_waypath->Get_ID () == id) {
			waypath = curr_waypath;
		}
	}		

	return waypath;
}

///////////////////////////////////////////////////////////////////////////
//
//	Count_Waypaths_Starting_In_Box 
//
///////////////////////////////////////////////////////////////////////////
int PathfindClass::Count_Waypaths_Starting_In_Box (const AABoxClass & box)
{
	//
	//	Loop over all the paths in our list counting the ones whose 
	// start points are contained inside the given box.
	//
	int count = 0;
	for (int index=0; index<m_WaypathList.Count(); index++) {
		WaypathClass *curr_waypath = m_WaypathList[index];
		if (CollisionMath::Overlap_Test(box,curr_waypath->Get_Point(0)->Get_Position()) == CollisionMath::INSIDE) {
			count++;
		}
	}
	return count;
}

///////////////////////////////////////////////////////////////////////////
//
//	Get_Waypath_Starting_In_Box 
//
///////////////////////////////////////////////////////////////////////////
WaypathClass * PathfindClass::Get_Waypath_Starting_In_Box (const AABoxClass & box,int i)
{
	//
	//	Loop over all the paths in our list counting down until
	// we get to the i'th one that starts in the given box.
	//
	WaypathClass * path = NULL;
	int count = i;
	for (int index=0; index<m_WaypathList.Count(); index++) {
		WaypathClass *curr_waypath = m_WaypathList[index];
		
		if (CollisionMath::Overlap_Test(box,curr_waypath->Get_Point(0)->Get_Position()) == CollisionMath::INSIDE) {
			if (count == 0) {
				path = curr_waypath;
				break;
			} else {
				count--;
			}
		}
	}
	
	if (path != NULL) {
		path->Add_Ref();
	}
	return path;
}

///////////////////////////////////////////////////////////////////////////
//
//	Reset_Waypaths
//
///////////////////////////////////////////////////////////////////////////
void
PathfindClass::Reset_Waypaths (void)
{
	//
	//	Release our hold on each of the waypaths
	//
	int index = m_WaypathList.Count ();
	while (index --) {
		WaypathClass *waypath = m_WaypathList[index];
		REF_PTR_RELEASE (waypath);	
	}		

	//
	//	Remove all the waypaths from our list
	//
	m_WaypathList.Delete_All ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save_Waypaths
//
//////////////////////////////////////////////////////////////////////////////////
bool
PathfindClass::Save_Waypaths (ChunkSaveClass &csave)
{
	//
	//	Loop over all the waypaths
	//
	int index = m_WaypathList.Count ();
	while (index --) {
		WaypathClass *waypath = m_WaypathList[index];

		//
		//	Save this waypath object
		//
		csave.Begin_Chunk (waypath->Get_Factory ().Chunk_ID ());
			waypath->Get_Factory ().Save (csave, waypath);
		csave.End_Chunk ();

		//
		//	Now save all the waypoints that this waypath contains
		//
		int waypoint_index = waypath->Get_Point_Count ();
		while (waypoint_index --) {
			WaypointClass *waypoint = waypath->Get_Point (waypoint_index);

			//
			//	Save this waypoint object
			//
			csave.Begin_Chunk (waypoint->Get_Factory ().Chunk_ID ());
				waypoint->Get_Factory ().Save (csave, waypoint);
			csave.End_Chunk ();
		}
	}		

	return true;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save_Culling_System
//
//////////////////////////////////////////////////////////////////////////////////
bool
PathfindClass::Save_Culling_System (ChunkSaveClass &csave)
{
	bool retval = true;

	csave.Begin_Chunk (CHUNKID_SECTOR_CULLING_SYSTEM);

		//
		//	Write the AABTreeCullSystem we use for sector lookups
		// to the chunk
		//
		csave.Begin_Chunk (CHUNKID_SECTOR_CULL_TREE);
			m_SectorTree.Save (csave);
		csave.End_Chunk ();

		//
		//	Now write the contents of the tree (the sector list)
		// to the chunk
		//
		int count = m_SectorList.Count ();
		for (int index = 0; index < count && retval; index ++) {
			Save_Sector (csave, m_SectorList[index]);
		}

	csave.End_Chunk ();

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Load_Culling_System
//
//////////////////////////////////////////////////////////////////////////////////
bool
PathfindClass::Load_Culling_System (ChunkLoadClass &cload)
{
	bool retval = true;

	//
	//	Read all the chunks...
	//
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {
			
			case CHUNKID_SECTOR_CULL_TREE:
				m_SectorTree.Load (cload);
				break;

			case CHUNKID_SECTOR:
				retval &= Load_Sector (cload);
				break;

			default:
				WWDEBUG_SAY (("Unknown chunk ID 0x%X\r\n", cload.Cur_Chunk_ID ()));
				retval = false;
				break;
		}

		cload.Close_Chunk ();
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Re_Partition_Sector_Tree
//
//////////////////////////////////////////////////////////////////////////////////
void
PathfindClass::Re_Partition_Sector_Tree (void)
{
	m_SectorTree.Re_Partition ();
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
// Peek_Portal
//
/////////////////////////////////////////////////////////////////////////
PathfindPortalClass *
PathfindClass::Peek_Portal (int portal_index)
{
	PathfindPortalClass *portal = NULL;
	
	if (portal_index >= TEMP_PORTAL_ID_START) {

		//
		//	Try to find the portal in our temporary portal list
		//
		for (int index = 0; index < m_TemporaryPortalList.Count (); index ++) {
			
			//
			//	Is this the portal we are looking for?
			//
			if (portal_index == (int)m_TemporaryPortalList[index]->Get_ID ()) {
				portal = m_TemporaryPortalList[index];
				break;
			}
		}

	} else if (portal_index >= WAYPATH_PORTAL_ID_START) {				
		
		//
		//	Look for the portal in our waypath portal list
		//
		portal = m_WaypathPortalList[portal_index - WAYPATH_PORTAL_ID_START];

	} else if (portal_index >= 0) {
		portal = m_PortalList[portal_index];
	}

	return portal;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Get_Height_Value
//
//////////////////////////////////////////////////////////////////////////////////
float
PathfindClass::Get_Height_Value (const Vector3 &pos)
{	
	return HeightDBClass::Get_Height (pos);
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Find_Random_Spot
//
//////////////////////////////////////////////////////////////////////////////////
static DynamicVectorClass<AABoxClass> box_list;

bool
PathfindClass::Find_Random_Spot
(
	const Vector3 &center,
	float				max_dist,
	Vector3 *		dest
)
{	
	bool retval = false;

	//
	//	Lookup the starting sector we'll search from
	//
	PathfindSectorClass *start_sector = Find_Sector (center, 0.1F, NULL);
	if (start_sector != NULL) {
		box_list.Reset_Active();

		//
		//	Start a list of valid 'regions' with the bounding box of the
		// current sector
		//
		box_list.Add (start_sector->Get_Bounding_Box ());

		//
		//	Square the distances for quick compares
		//
		float max_dist2 = max_dist * max_dist;

		//
		//	Loop over all connecting sectors and add any of them that meet
		// the distance requirements to our list
		//
		for (int index = 0; index < start_sector->Get_Portal_Count (); index ++) {
			PathfindPortalClass *portal		= start_sector->Peek_Portal (index);		
			PathfindSectorClass *dest_sector	= portal->Peek_Dest_Sector (start_sector);
			if (dest_sector != NULL) {
				const AABoxClass &bounding_box = dest_sector->Get_Bounding_Box ();
				
				//
				//	Find a "close" point to the box from the starting position
				//
				Vector3 closest_pt = center;
				Clip_Point (&closest_pt, bounding_box);
				
				//
				//	Is this box within the maximum acceptable distance?
				//
				float dist_to_box2 = (center - closest_pt).Length2 ();
				if (dist_to_box2 < max_dist2) {
					box_list.Add (bounding_box);
				}
			}
		}

		//
		//	Pick a random box from the list
		//
		int count					= box_list.Count ();
		int box_index				= (rand () % count);
		const AABoxClass &box	= box_list[box_index];

		//
		//	Find a "close" point to the box from the starting position
		//
		Vector3 closest_pt = center;
		Clip_Point (&closest_pt, box);

		//
		//	Now add a random vector to this point that will stay
		// inside the requirements
		//
		float curr_dist				= (closest_pt - center).Length ();
		float max_allowable_dist	= (max_dist - curr_dist);

		(*dest) = closest_pt;
		dest->X += WWMath::Random_Float (-max_allowable_dist, max_allowable_dist);
		dest->Y += WWMath::Random_Float (-max_allowable_dist, max_allowable_dist);

		//
		//	Now, simply make sure the point stays inside the pathfind sector
		//
		Clip_Point (dest, box);
		retval = true;

	} else if (m_SectorList.Count () == 0) {

		//
		//	If we don't have any pathfind data, then simply choose a random point
		//
		(*dest) = center;
		dest->X += WWMath::Random_Float (-max_dist, max_dist);
		dest->Y += WWMath::Random_Float (-max_dist, max_dist);
		retval = true;
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Free_Waypath_Sectors_And_Portals
//
//////////////////////////////////////////////////////////////////////////////////
void
PathfindClass::Free_Waypath_Sectors_And_Portals (void)
{
	//
	//	Release our hold on each of the waypath portals
	//
	int index;
	for (index = 0; index < m_WaypathPortalList.Count (); index ++) {
		PathfindPortalClass *portal = m_WaypathPortalList[index];

		//
		//	Remove this portal from the sectors that reference it
		//
		uint16 dest_sector1_id = portal->Get_Dest_Sector1 ();
		uint16 dest_sector2_id = portal->Get_Dest_Sector2 ();
		PathfindSectorClass *dest_sector1 = Peek_Sector (dest_sector1_id);
		PathfindSectorClass *dest_sector2 = Peek_Sector (dest_sector2_id);

		if (dest_sector1 != NULL) {
			dest_sector1->Remove_Portal (portal->Get_ID ());
		}

		if (dest_sector2 != NULL) {
			dest_sector2->Remove_Portal (portal->Get_ID ());
		}

		REF_PTR_RELEASE (portal);
	}

	//
	//	Loop over all the sectors in our list
	//
	for (index = 0; index < m_SectorList.Count (); index ++) {
		PathfindSectorClass *sector = m_SectorList[index];
		
		//
		//	Is this sector a waypath sector?
		//
		if (sector->As_PathfindWaypathSectorClass () != NULL) {

			//
			//	Free the sector and remove it from the list
			//
			REF_PTR_RELEASE (sector);
			m_SectorList.Delete (index);
			index --;
		}
	}

	//
	//	Reset the portal list
	//
	m_WaypathPortalList.Delete_All ();	
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Generate_Waypath_Sectors_And_Portals
//
//////////////////////////////////////////////////////////////////////////////////
void
PathfindClass::Generate_Waypath_Sectors_And_Portals (void)
{
	//
	//	Start fresh
	//
	Free_Waypath_Sectors_And_Portals ();

	//
	//	Loop over all the waypaths in the system
	//
	for (int index = 0; index < m_WaypathList.Count (); index ++) {		
		WaypathClass *waypath = m_WaypathList[index];

		//
		//	Only process waypaths that can be used during a path solve
		//
		if (waypath->Get_Flag (WaypathClass::FLAG_INNATE_PATHFIND)) {
			Generate_Waypath_Sector_And_Portals (waypath);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Generate_Waypath_Sector_And_Portals
//
//////////////////////////////////////////////////////////////////////////////////
void
PathfindClass::Generate_Waypath_Sector_And_Portals (WaypathClass *waypath)
{
	DynamicVectorClass<PathfindWaypathPortalClass *> portal_list;

	//
	//	Get information about this waypath
	//		
	int waypoint_count	= waypath->Get_Point_Count ();
	int waypath_id			= waypath->Get_ID ();

	//
	//	Create a new "waypath sector" that will hold all the enter and
	// exit portals for us
	//
	PathfindWaypathSectorClass *new_sector = new PathfindWaypathSectorClass;
	new_sector->Set_Waypath_ID (waypath_id);
	new_sector->Set_Bounding_Box (AABoxClass (Vector3 (0, 0, 0), Vector3 (0, 0, 0)));
	
	//
	//	Register this new sector with the pathfind system
	//
	Add_Sector (new_sector, false);		
	
	//
	//	Add a portal around each waypoint in the path
	//
	for (int index = 0; index < waypoint_count; index ++) {
		WaypointClass *waypoint			= waypath->Get_Point (index);
		const Vector3 &waypoint_pos	= waypoint->Get_Position ();

		//
		//	Check to see if there is a pathfind sector at the waypoint position...
		// if there is, then we should create a new portal at this position.
		//
		PathfindSectorClass *sector_at_pos = Find_Sector (waypoint_pos, 0.125F);
		if (sector_at_pos != NULL) {

			//
			//	Add a new portal to the list at this position
			//
			::Add_New_Portal_To_List (portal_list, new_sector, sector_at_pos, waypath_id,
					index, waypoint_pos, 0);
		}
	}

	//
	//	Now add a portal everywhere the waypath intersects a pathfind sector
	//
	Add_Intersection_Portals_To_List (portal_list, waypath, new_sector);

	if (portal_list.Count () > 0) {
		
		//
		//	Now sort the portals based on occurance within the path
		//
		::qsort (&portal_list[0], portal_list.Count (),
			sizeof (PathfindWaypathPortalClass *), fnCompareWaypathPortalsCallback);

		//
		//	Register the portals with the system and add them to their sectors
		//
		for (int portal_index = 0; portal_index < portal_list.Count (); portal_index ++) {
			PathfindWaypathPortalClass *curr_portal = portal_list[portal_index];
			
			//
			//	Register the portal
			//
			int portal_id = Add_Waypath_Portal (curr_portal);

			//
			//	Lookup the sectors this portal will connect
			//
			uint16 dest_sector1_id = curr_portal->Get_Dest_Sector1 ();
			uint16 dest_sector2_id = curr_portal->Get_Dest_Sector2 ();
			PathfindSectorClass *dest_sector1 = Peek_Sector (dest_sector1_id);
			PathfindSectorClass *dest_sector2 = Peek_Sector (dest_sector2_id);

			//
			//	Add this portal to the sectors
			//
			if (dest_sector1 != NULL) {
				dest_sector1->Add_Portal (portal_id);
			}
		
			if (dest_sector2 != NULL) {
				dest_sector2->Add_Portal (portal_id);
			}

			//
			//	Release our hold on this portal
			//
			REF_PTR_RELEASE (curr_portal);
		}
	}
	
	//
	//	Now release our hold on the sector we just created
	//
	REF_PTR_RELEASE (new_sector);
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Add_Intersection_Portals_To_List
//
//////////////////////////////////////////////////////////////////////////////////
void
PathfindClass::Add_Intersection_Portals_To_List
(
	DynamicVectorClass<PathfindWaypathPortalClass *> &	portal_list,
	WaypathClass *													waypath,
	PathfindWaypathSectorClass *								dest_sector
)
{
	int waypoint_count	= waypath->Get_Point_Count ();
	int waypath_id			= waypath->Get_ID ();

	//
	//	For this waypath, find all the pathfind sectors it intersects
	//
	for (int waypoint_index = 0; waypoint_index < waypoint_count - 1; waypoint_index ++) {
		WaypointClass *waypoint1 = waypath->Get_Point (waypoint_index);
		WaypointClass *waypoint2 = waypath->Get_Point (waypoint_index + 1);

		const Vector3 &pos1 = waypoint1->Get_Position ();
		const Vector3 &pos2 = waypoint2->Get_Position ();

		//
		//	Build a bounding box around the two points
		//
		Vector3 extent = (pos2 - pos1) * 0.5F;
		Vector3 center = pos1 + extent;
		extent.X = std::max (WWMath::Fabs (extent.X), 1.0F);
		extent.Y = std::max (WWMath::Fabs (extent.Y), 1.0F);
		extent.Z = std::max (WWMath::Fabs (extent.Z), 1.0F);
		AABoxClass bounding_box (center, extent);
		
		//
		//	Find all the pathfind sectors in this bounding box
		//
		m_SectorTree.Reset_Collection ();
		m_SectorTree.Collect_Objects (bounding_box);

		//
		//	Loop over all the sectors that this line segment could possibly intersect
		//			
		PathfindSectorClass *sector = NULL;			
		for (	sector = m_SectorTree.Peek_First_Collected_Object ();
				sector != NULL;
				sector = m_SectorTree.Peek_Next_Collected_Object (sector))
		{
			const AABoxClass &sector_box = sector->Get_Bounding_Box ();

			//
			//	Find where this line segment intersects the sector (if at all)
			//
			float percent = 0;
			Vector3 portal_pos;
			if (::Find_Intersection_Point (sector_box, pos1, pos2, &percent, &portal_pos)) {

				//
				//	Add a new portal to the list at this position
				//
				::Add_New_Portal_To_List (portal_list, dest_sector, sector, waypath_id,
						waypoint_index, portal_pos, percent);
			}
		}
	}					

	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Add_New_Portal_To_List
//
//////////////////////////////////////////////////////////////////////////////////
void
Add_New_Portal_To_List
(
	DynamicVectorClass<PathfindWaypathPortalClass *> &	portal_list,	
	PathfindSectorClass *										dest_sector1,
	PathfindSectorClass *										dest_sector2,
	int																waypath_id,
	int																waypoint_index,
	const Vector3 &												portal_pos,
	float																percent
)
{
	const Vector3 portal_extent (0.5F, 0.5F, 1.0F);

	//
	//	Create a position along this waypath for the waypoint
	//
	WaypathPositionClass waypath_pos;
	waypath_pos.Set_Waypath_ID (waypath_id);
	waypath_pos.Set_Waypoint_Index (waypoint_index);
	waypath_pos.Set_Percent (percent);

	//
	//	Allcoate and configure a new portal
	//
	PathfindWaypathPortalClass *new_portal = new PathfindWaypathPortalClass;
	new_portal->Set_Waypath_Pos (waypath_pos);				
	new_portal->Set_Bounding_Box (AABoxClass (portal_pos, portal_extent));
	
	if (dest_sector1 != NULL) {
		new_portal->Add_Dest_Sector (dest_sector1);
	}

	if (dest_sector2 != NULL) {
		new_portal->Add_Dest_Sector (dest_sector2);
	}

	//
	//	Add this portal to the list
	//
	portal_list.Add (new_portal);
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Find_Intersection_Point
//
//////////////////////////////////////////////////////////////////////////////////
bool
Find_Intersection_Point
(
	const AABoxClass &	box,
	const Vector3 &		p0,
	const Vector3 &		p1,
	float *					percent,
	Vector3 *				intersection_point
)
{
	bool retval = false;

	//
	//	Find the locations of each of the 6 "planes"
	// we will be testing against
	//
	float x1 = box.Center.X - box.Extent.X;
	float x2 = box.Center.X + box.Extent.X;
	float y1 = box.Center.Y - box.Extent.Y;
	float y2 = box.Center.Y + box.Extent.Y;
	float z1 = box.Center.Z - box.Extent.Z;
	float z2 = box.Center.Z + box.Extent.Z;

	float t_values[6] = { -1, -1, -1, -1, -1, -1 };

	Vector3 delta = p1 - p0;

	//
	//	Find the "t" values where the line intersects the
	// 2 "Y-Z" planes
	//
	if (delta.X != 0) {
		t_values[0] = (x1 - p0.X) / delta.X;
		t_values[1] = (x2 - p0.X) / delta.X;
	}

	//
	//	Find the "t" values where the line intersects the
	// 2 "X-Z" planes
	//
	if (delta.Y != 0) {
		t_values[2] = (y1 - p0.Y) / delta.Y;
		t_values[3] = (y2 - p0.Y) / delta.Y;
	}

	//
	//	Find the "t" values where the line intersects the
	// 2 "X-Y" planes
	//
	if (delta.Z != 0) {
		t_values[4] = (z1 - p0.Z) / delta.Z;
		t_values[5] = (z2 - p0.Z) / delta.Z;
	}

	//
	//	Loop over all the "t" values we've calculated
	//
	(*percent) = 2.0F;
	for (int index = 0; index < 6; index ++) {		

		//
		// Is this "t" value the smallest in-range value we've
		// found yet?
		//
		if (	t_values[index] >= 0 &&
				t_values[index] <= 1.0F &&
				t_values[index] < (*percent))
		{
			//
			//	Find the point which exists at this "t" value along the line segment
			//
			Vector3 point = p0 + delta * t_values[index];

			//
			//	If this point isn't outside the box, then we'll consider it good enough
			//
			if (	(WWMath::Fabs (point.X - box.Center.X) <= (box.Extent.X + WWMATH_EPSILON)) &&
					(WWMath::Fabs (point.Y - box.Center.Y) <= (box.Extent.Y + WWMATH_EPSILON)) &&
					(WWMath::Fabs (point.Z - box.Center.Z) <= (box.Extent.Z + WWMATH_EPSILON)))
			{
				(*percent)					= t_values[index];
				(*intersection_point)	= point;
				retval						= true;
			}
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	fnCompareWaypathPortalsCallback
//
////////////////////////////////////////////////////////////////
int __cdecl
fnCompareWaypathPortalsCallback
(
	const void *elem1,
	const void *elem2
)
{
   WWASSERT (elem1 != NULL);
   WWASSERT (elem2 != NULL);
   PathfindWaypathPortalClass *portal1 = *((PathfindWaypathPortalClass **)elem1);
   PathfindWaypathPortalClass *portal2 = *((PathfindWaypathPortalClass **)elem2);	

	const WaypathPositionClass &pos1 = portal1->Get_Waypath_Pos ();
	const WaypathPositionClass &pos2 = portal2->Get_Waypath_Pos ();

	//
	//	Sort based on the waypath segment the portals lie one
	//
	int result = 0;
	if (pos1.Get_Waypoint_Index () < pos2.Get_Waypoint_Index ()) {
		result = -1;
	} else if (pos1.Get_Waypoint_Index () > pos2.Get_Waypoint_Index ()) {
		result = 1;
	} else {
		
		//
		//	If the portals lie on the same segment, then sort by their
		// position along the segment
		//
		if (pos1.Get_Percent () < pos2.Get_Percent ()) {
			result = -1;
		} else if (pos1.Get_Percent () > pos2.Get_Percent ()) {
			result = 1;
		}
	}

   return result;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Find_Portals
//
//////////////////////////////////////////////////////////////////////////////////
void
PathfindClass::Find_Portals
(
	const Vector3 &										p0,
	const Vector3 &										p1,
	DynamicVectorClass<PathfindPortalClass *> &	list,
	bool														action_portals_only
)
{
	//
	//	Loop over all the portals
	//
	for (int index = 0; index < m_PortalList.Count (); index ++) {		
		PathfindPortalClass *portal					= m_PortalList[index];
		PathfindActionPortalClass *action_portal	= portal->As_PathfindActionPortalClass ();
		if (action_portals_only == false || (action_portal != NULL && portal->Get_Action_Type () != PathClass::ACTION_NONE)) {
			
			//
			//	Get the bounding box for the portal
			//
			AABoxClass box;
			portal->Get_Bounding_Box (box);

			//
			//	Does the line pass through this box?
			//
			float percent1 = 0;
			Vector3 intersection_pt (0, 0, 0);
			if (::Find_Intersection_Point (box, p0, p1, &percent1, &intersection_pt)) {
				bool is_valid = true;
				
				//
				//	If this is an action portal request, we need to ensure that the
				// line segment passes through to the exit portal
				//
				if (action_portals_only) {
					is_valid								= false;
					PathfindSectorClass *sector	= action_portal->Peek_Dest_Sector (NULL);
					if (sector != NULL) {

						//
						//	Get a pointer to the exit portal
						//
						PathfindPortalClass *exit_portal = action_portal->Get_Exit_Portal ();
						if (exit_portal != NULL) {
							
							//
							//	Does the line segment pass through to the exit portal?
							//						
							AABoxClass exit_box;
							exit_portal->Get_Bounding_Box (exit_box);
							if (::Find_Intersection_Point (exit_box, p0, p1,
										&percent1, &intersection_pt))
							{
								float dist1 = (box.Center - p0).Length2 ();
								float dist2 = (exit_box.Center - p0).Length2 ();							
								if (dist1 < dist2) {
									is_valid = true;
								}
							}
						}
					}
				}
				
				//
				//	Add the portal to the list
				//
				if (is_valid) {
					list.Add (portal);
				}
			}
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Render_Debug_Widgets
//
//////////////////////////////////////////////////////////////////////////////////
void
PathfindClass::Render_Debug_Widgets (RenderInfoClass &rinfo)
{
	m_SectorDisplayWidgets.Render_Debug_Widgets (rinfo);
	m_PortalDisplayWidgets.Render_Debug_Widgets (rinfo);
	return ;
}


