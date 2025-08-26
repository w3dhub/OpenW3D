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
 *                     $Archive:: /Commando/Code/wwphys/waypath.cpp         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/20/01 7:48p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "waypath.h"
#include "persistfactory.h"
#include "wwphysids.h"
#include "waypoint.h"
#include "wwhack.h"
#include "waypathposition.h"
#include "Pathfind.h"


DECLARE_FORCE_LINK(waypath);

//////////////////////////////////////////////////////////////////////////////
//	Persist factory
//////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<WaypathClass, PHYSICS_CHUNKID_WAYPATH> _WaypathPersistFactory;

enum
{
	CHUNKID_VARIABLES			= 0x04290219,
	CHUNKID_BASE_CLASS
};

enum
{
	VARID_OLD_PTR				= 0x01,
	VARID_FLAGS,
	VARID_ID,
	VARID_WAYPOINT_PTR
};


////////////////////////////////////////////////////////////////
//
//	WaypathClass
//
////////////////////////////////////////////////////////////////
WaypathClass::WaypathClass (void)
	:	m_ID (0),
		m_Flags (FLAG_TWO_WAY | FLAG_HUMAN | FLAG_GROUND_VEHICLE)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	WaypathClass
//
////////////////////////////////////////////////////////////////
WaypathClass::WaypathClass (const WaypathPositionClass &start, const WaypathPositionClass &end)
	:	m_ID (0),
		m_Flags (FLAG_TWO_WAY | FLAG_HUMAN | FLAG_GROUND_VEHICLE)
{
	//
	//	Check to ensure the two endpoints refer to the same path
	//
	int waypath_id = start.Get_Waypath_ID ();
	if (waypath_id == end.Get_Waypath_ID ()) {
		
		//
		//	Lookup the path we will use as our reference
		//
		WaypathClass *whole_path = PathfindClass::Get_Instance ()->Find_Waypath (waypath_id);
		if (whole_path != NULL) {

			//
			//	Copy the flags from the complete path
			//
			m_Flags = whole_path->Get_Flags ();
			
			//
			//	Find the world-space positions of the endpoints of our path segment
			//
			Vector3 start_pos (0, 0, 0);
			Vector3 end_pos (0, 0, 0);
			whole_path->Evaluate_Position (start, &start_pos);
			whole_path->Evaluate_Position (end, &end_pos);

			//
			//	Add the starting point to our waypath
			//
			m_Waypoints.Add (new WaypointClass (start_pos));

			//
			// Add all the points between the start and end point to the path
			//
			/*int start_index	= start.Get_Waypoint_Index ();
			int end_index		= end.Get_Waypoint_Index ();
			int count			= WWMath::Fabs (end_index - start_index) - 2;
			int inc				= (end_index > start_index) ? 1 : -1;
			for (int index = start_index + inc; count >= 0; index += inc, count --) {
				WaypointClass *waypoint = whole_path->Get_Point (index);
				m_Waypoints.Add (new WaypointClass (*waypoint));
			}*/

			
			//
			// Get the start and end indices from the path
			//
			int start_index	= start.Get_Waypoint_Index ();
			int end_index		= end.Get_Waypoint_Index ();

			//
			//	Are we moving forwards or backwards along the path?
			//
			if (end_index > start_index) {
				
				//
				// Add all the points between the start and end point to the path
				//
				for (int index = start_index + 1; index <= end_index; index ++) {
					WaypointClass *waypoint = whole_path->Get_Point (index);
					m_Waypoints.Add (new WaypointClass (*waypoint));
				}

			} else {
				
				//
				// Add all the points between the start and end point to the path
				//
				for (int index = start_index - 1; index > end_index; index --) {
					WaypointClass *waypoint = whole_path->Get_Point (index);
					m_Waypoints.Add (new WaypointClass (*waypoint));
				}
			}			

			//
			//	Add the ending point to our waypath
			//
			m_Waypoints.Add (new WaypointClass (end_pos));
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	WaypathClass
//
////////////////////////////////////////////////////////////////
WaypathClass::~WaypathClass (void)
{
	Free ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
WaypathClass::Get_Factory (void) const
{
	return _WaypathPersistFactory;
}


/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
bool
WaypathClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_VARIABLES);
				
		WaypathClass *this_ptr = this;
		WRITE_MICRO_CHUNK (csave, VARID_OLD_PTR,	this_ptr);
		WRITE_MICRO_CHUNK (csave, VARID_FLAGS,		m_Flags);
		WRITE_MICRO_CHUNK (csave, VARID_ID,			m_ID);		

		//
		//	Write out each waypoint pointer so we can remap them
		// on load.
		//
		for (int index = 0; index < m_Waypoints.Count (); index ++) {
			WaypointClass *waypoint = m_Waypoints[index];
			WRITE_MICRO_CHUNK (csave, VARID_WAYPOINT_PTR, waypoint);
		}

	csave.End_Chunk ();
	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////
bool
WaypathClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {
		
			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	SaveLoadSystemClass::Register_Post_Load_Callback (this);
	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////
bool
WaypathClass::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK (cload, VARID_FLAGS,		m_Flags);
			READ_MICRO_CHUNK (cload, VARID_ID,			m_ID);

			case VARID_WAYPOINT_PTR:
			{
				//
				//	Read the old waypoint ptr from the chunk and add it to our
				// list.  We will remap it later.
				//				
				WaypointClass *waypoint = NULL;
				cload.Read (&waypoint, sizeof (waypoint));				
				m_Waypoints.Add (waypoint);				
			}
			break;

			case VARID_OLD_PTR:
			{
				//
				//	Read the old pointer from the chunk and submit it
				// to the remapping system.
				//				
				WaypathClass *old_ptr = NULL;
				cload.Read (&old_ptr, sizeof (old_ptr));
				SaveLoadSystemClass::Register_Pointer (old_ptr, this);
			}
			break;
		}

		cload.Close_Micro_Chunk ();
	}

	//
	//	Register each of the pointers in our waypoint list for remapping
	//
	for (int index = 0; index < m_Waypoints.Count (); index ++) {
		REQUEST_POINTER_REMAP ((void **)&m_Waypoints[index]);
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	On_Post_Load
//
///////////////////////////////////////////////////////////////////////
void
WaypathClass::On_Post_Load (void)
{
	//
	//	Add a reference count to each of the waypoints
	//
	/*for (int index = 0; index < m_Waypoints.Count (); index ++) {
		WaypointClass *waypoint = m_Waypoints[index];
		waypoint->Add_Ref ();
	}*/

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Free
//
///////////////////////////////////////////////////////////////////////
void
WaypathClass::Free (void)
{
	//
	//	Release our hold on each of the waypoints
	//
	for (int index = 0; index < m_Waypoints.Count (); index ++) {
		WaypointClass *waypoint = m_Waypoints[index];
		REF_PTR_RELEASE (waypoint);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Add_Point
//
///////////////////////////////////////////////////////////////////////
void
WaypathClass::Add_Point (const WaypointClass &point)
{
	//
	//	Allocate a copy of the given waypoint and add it to
	// our waypoint list.
	//
	WaypointClass *new_waypoint = new WaypointClass (point);
	m_Waypoints.Add (new_waypoint);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Add_Point
//
///////////////////////////////////////////////////////////////////////
void
WaypathClass::Add_Point (const Vector3 &point)
{
	//
	//	Allocate a copy of the given waypoint and add it to
	// our waypoint list.
	//
	WaypointClass *new_waypoint = new WaypointClass (point);
	m_Waypoints.Add (new_waypoint);
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Set_Flag
//
//////////////////////////////////////////////////////////////////////////////
void
WaypathClass::Set_Flag (int flag, bool onoff)
{
	m_Flags &= ~flag;
	if (onoff) {
		m_Flags |= flag;
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Evaluate_Position
//
//////////////////////////////////////////////////////////////////////////////
bool
WaypathClass::Evaluate_Position (const WaypathPositionClass &pos, Vector3 *position)
{
	bool retval = false;

	//
	//	Check to ensure this is the right waypath
	//
	if (pos.Get_Waypath_ID () == m_ID) {
		
		//
		//	Lookup the start point of the segment
		//
		int index = pos.Get_Waypoint_Index ();		
		WaypointClass *seg_pt1 = Get_Point (index);
		if (seg_pt1 != NULL) {

			//
			//	Get the world-space position of the start point of the segment
			//
			const Vector3 &pt1 = seg_pt1->Get_Position ();
			
			//
			//	Do a quick check to see if we should just return the
			// start point
			//
			if (pos.Get_Percent () == 0) {
				(*position) = pt1;
				retval = true;
			} else {

				//
				//	Lookup the end point of the segment
				//
				WaypointClass *seg_pt2 = Get_Point (index + 1);
				if (seg_pt2 != NULL) {
					
					//
					//	Get the world-space position of the end point of the segment
					//					
					const Vector3 &pt2 = seg_pt2->Get_Position ();

					//
					//	Evaluate the line segment at the given percent
					//
					(*position) = pt1 + (pt2 - pt1) * pos.Get_Percent ();
					retval = true;
				}
			}
		}
	}
		
	return retval;
}
