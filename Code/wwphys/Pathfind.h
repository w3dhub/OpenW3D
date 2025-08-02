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
 *                     $Archive:: /Commando/Code/wwphys/Pathfind.h                             $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/15/01 5:19p                                             $*
 *                                                                                             *
 *                    $Revision:: 20                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __PATHFIND_H
#define __PATHFIND_H

#include "aabtreecull.h"
#include "PathfindSector.h"
#include "widgetuser.h"


/////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////
class Vector3;
class ChunkSaveClass;
class ChunkLoadClass;
class PhysClass;
class PathDebugPlotterClass;
class PathfindPortalClass;
class WaypathClass;
class PathfindWaypathSectorClass;
class PathfindWaypathPortalClass;


/////////////////////////////////////////////////////////////////////////
//
//	PathfindClass
//
/////////////////////////////////////////////////////////////////////////
class PathfindClass
{
	public:

		typedef enum
		{
			WAYPATH_PORTAL_ID_START		= 64000,
			TEMP_PORTAL_ID_START			= 128000			

		} PORTAL_ID_RANGES;


		/////////////////////////////////////////////////////////////////////////
		// Public constructors/destructors
		/////////////////////////////////////////////////////////////////////////
		PathfindClass (void);
		~PathfindClass (void);

		/////////////////////////////////////////////////////////////////////////
		// Public methods
		/////////////////////////////////////////////////////////////////////////
		
		//
		// Database methods
		//
		void							Add_Sector (PathfindSectorClass *sector, bool add_to_tree = true);
		int							Add_Portal (PathfindPortalClass *portal);
		PathfindPortalClass *	Peek_Portal (int portal_index);
		PathfindSectorClass *	Peek_Sector (int sector_index);
		int							Get_Sector_Index (PathfindSectorClass *sector);
		void							Re_Partition_Sector_Tree (void);

		PathfindSectorClass *	Find_Sector (const Vector3 &position, float sector_fudge = 0, PathfindSectorClass *exclude_sector = NULL);
		void							Collect_Sectors (DynamicVectorClass<PathfindSectorClass *> &list, const AABoxClass &box, PathfindSectorClass *exclude_sector = NULL);
		bool							Find_Random_Spot (const Vector3 &center, float max_dist, Vector3 *dest);

		bool							Save (ChunkSaveClass &csave);
		bool							Load (ChunkLoadClass &cload);

		int							Add_Temporary_Portal (PathfindSectorClass *sector_from, PathfindSectorClass *sector_to, const Vector3 &start_pos, const Vector3 &dest_pos);

		//
		//	Statistics
		//
		const AABoxClass &		Get_Bounding_Box (void)				{ return m_SectorTree.Get_Bounding_Box (); }
		bool							Does_Pathfind_Data_Exist (void)	{ return bool(m_SectorList.Count () > 0); }

		//
		//	Waypath methods
		//
		void							Add_Waypath (WaypathClass *waypath);
		bool							Remove_Waypath (WaypathClass *waypath);
		WaypathClass *				Find_Waypath (int id) const;

		int							Count_Waypaths_Starting_In_Box (const AABoxClass & box);
		WaypathClass *				Get_Waypath_Starting_In_Box (const AABoxClass & box,int i);

		//
		//	Waypath integration
		//
		void							Generate_Waypath_Sectors_And_Portals (void);
		void							Free_Waypath_Sectors_And_Portals (void);

		//
		//	Height database lookup
		//
		float							Get_Height_Value (const Vector3 &pos);

		//
		// Debug methods
		//
		bool							Are_Sectors_Displayed (void) const { return m_SectorsDisplayed; }
		bool							Are_Portals_Displayed (void) const { return m_PortalsDisplayed; }
		void							Display_Sectors (bool onoff);
		void							Display_Portals (bool onoff);
		void							Render_Debug_Widgets (RenderInfoClass &rinfo);

		//
		// Cleanup methods
		//
		void							Reset_Sectors (void);
		void							Reset_Portals (void);
		void							Reset_Waypaths (void);

		//
		//	Sector list access
		//
		void							Get_Sector_List (DynamicVectorClass<PathfindSectorClass *> &list) { list = m_SectorList; }

		//
		//	Intersection methods
		//
		void							Find_Portals (const Vector3 &p0, const Vector3 &p1, DynamicVectorClass<PathfindPortalClass *> &list, bool action_portals_only = false);

		/////////////////////////////////////////////////////////////////////////
		// Static methods
		/////////////////////////////////////////////////////////////////////////
		static PathfindClass *	Get_Instance (void)	{ return _Pathfinder; }

		static int _MemoryFootprint;

	protected:

		/////////////////////////////////////////////////////////////////////////
		// Protected methods
		/////////////////////////////////////////////////////////////////////////
		bool							Save_Portals (ChunkSaveClass &csave);
		bool							Save_Waypaths (ChunkSaveClass &csave);
		bool							Save_Sector (ChunkSaveClass &csave, PathfindSectorClass *sector);
		bool							Load_Sector (ChunkLoadClass &cload);
		bool							Load_Portal (ChunkLoadClass &cload, PathfindPortalClass *portal);

		bool							Save_Culling_System (ChunkSaveClass &csave);
		bool							Load_Culling_System (ChunkLoadClass &cload);
		
		void							Generate_Waypath_Sector_And_Portals (WaypathClass *waypath);
		void							Add_Intersection_Portals_To_List (DynamicVectorClass<PathfindWaypathPortalClass *> &portal_list, WaypathClass *waypath, PathfindWaypathSectorClass *dest_sector);
		int							Add_Waypath_Portal (PathfindWaypathPortalClass *portal);

	private:

		/////////////////////////////////////////////////////////////////////////
		// Static member data
		/////////////////////////////////////////////////////////////////////////
		static PathfindClass *	_Pathfinder;

		/////////////////////////////////////////////////////////////////////////
		// Private data types
		/////////////////////////////////////////////////////////////////////////
		typedef TypedAABTreeCullSystemClass<PathfindSectorClass>	SectorCullingSystem;		
		typedef DynamicVectorClass<PathfindSectorClass *>			SECTOR_LIST;
		typedef DynamicVectorClass<PathfindPortalClass *>			PORTAL_LIST;
		typedef DynamicVectorClass<PhysClass *>						DISPLAY_LIST;
		typedef DynamicVectorClass<WaypathClass *>					WAYPATH_LIST;

		/////////////////////////////////////////////////////////////////////////
		// Private member data
		/////////////////////////////////////////////////////////////////////////
		SectorCullingSystem	m_SectorTree;
		SECTOR_LIST				m_SectorList;
		PORTAL_LIST				m_PortalList;
		DISPLAY_LIST			m_SectorDisplayList;
		WAYPATH_LIST			m_WaypathList;
		bool						m_SectorsDisplayed;
		bool						m_PortalsDisplayed;
		PathDebugPlotterClass *m_Plotter;

		PORTAL_LIST				m_TemporaryPortalList;
		PORTAL_LIST				m_WaypathPortalList;

		WidgetUserClass		m_SectorDisplayWidgets;
		WidgetUserClass		m_PortalDisplayWidgets;
};



/////////////////////////////////////////////////////////////////////////
// Peek_Sector
/////////////////////////////////////////////////////////////////////////
inline PathfindSectorClass *
PathfindClass::Peek_Sector (int sector_index)
{
	if (sector_index >= 0 && sector_index < m_SectorList.Count ()) {
		return m_SectorList[sector_index];
	} else {
		return NULL;
	}
}


#endif //__PATHFIND_H

