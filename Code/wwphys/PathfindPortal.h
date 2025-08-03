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
 *                     $Archive:: /Commando/Code/wwphys/PathfindPortal.h                      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 4/18/01 12:33p                                              $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif


#ifndef __PATHFIND_PORTAL_H
#define __PATHFIND_PORTAL_H

#include "refcount.h"
#include "aabox.h"
#include "wwdebug.h"
#include "Pathfind.h"
#include "Path.h"
#include "waypathposition.h"


//////////////////////////////////////////////////////////////////////////
// Forward declarations
//////////////////////////////////////////////////////////////////////////
class PathfindSectorClass;
class PathNodeClass;
class ChunkSaveClass;
class ChunkLoadClass;
class PathfindActionPortalClass;
class PathfindWaypathPortalClass;


//////////////////////////////////////////////////////////////////////////
//
//	PathfindPortalClass
//
//////////////////////////////////////////////////////////////////////////
class PathfindPortalClass : public RefCountClass
{
public:

	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	PathfindPortalClass (void)
		:	m_DestSector1 ((uint16)-1),
			m_DestSector2 ((uint16)-1),
			m_HeapLocation (0),
			m_ClosedListPtr (NULL),
			m_ID (0)									{}

	virtual ~PathfindPortalClass (void)		{}

	////////////////////////////////////////////////////////////////////
	//	RTTI
	////////////////////////////////////////////////////////////////////
	virtual PathfindActionPortalClass *		As_PathfindActionPortalClass (void) { return NULL; }
	virtual PathfindWaypathPortalClass *	As_PathfindWaypathPortalClass (void) { return NULL; }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	void						Get_Bounding_Box (AABoxClass &box) const;
	void						Set_Bounding_Box (const AABoxClass &box);

	PathfindSectorClass *Peek_Dest_Sector (PathfindSectorClass *current_sector);
	void						Add_Dest_Sector (int sector_index);
	void						Add_Dest_Sector (PathfindSectorClass *sector);
	uint16					Get_Dest_Sector1 (void);
	uint16					Get_Dest_Sector2 (void);

	virtual PathClass::ACTION_ID	Get_Action_Type (void) const { return PathClass::ACTION_NONE; }

	bool						Is_Two_Way_Portal (void) const;
	virtual bool			Does_Size_Matter (void) const	{ return true; }

	uint32					Get_ID (void) const	{ return m_ID; }
	void						Set_ID (uint32 id)	{ m_ID = id; }

	//////////////////////////////////////////////////////////////////////
	//	A-Star list methods
	//////////////////////////////////////////////////////////////////////
	uint32					Get_Heap_Location (void) const;
	void						Set_Heap_Location (uint32 location);

	//////////////////////////////////////////////////////////////////////
	//	Serialization methods
	//////////////////////////////////////////////////////////////////////
	virtual bool			Save (ChunkSaveClass &chunk_save);
	virtual bool			Load (ChunkLoadClass &chunk_load);
	void						Resolve_IDs (void);

	PathNodeClass *		m_ClosedListPtr;

protected:

	//////////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////////
	bool						Load_Variables (ChunkLoadClass &cload);

private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	uint16 		m_DestSector1;
	uint16 		m_DestSector2;
	AABoxClass	m_BoundingBox;	
	uint32		m_ID;
	uint32		m_HeapLocation;
};


//////////////////////////////////////////////////////////////////////////
//	Get_Bounding_Box
//////////////////////////////////////////////////////////////////////////
inline void
PathfindPortalClass::Get_Bounding_Box (AABoxClass &box) const
{
	box = m_BoundingBox;
	return ;
}

//////////////////////////////////////////////////////////////////////////
//	Set_Bounding_Box
//////////////////////////////////////////////////////////////////////////
inline void
PathfindPortalClass::Set_Bounding_Box (const AABoxClass &box)
{
	m_BoundingBox = box;
	return ;
}

//////////////////////////////////////////////////////////////////////////
//	Add_Dest_Sector
//////////////////////////////////////////////////////////////////////////
inline void
PathfindPortalClass::Add_Dest_Sector (int sector_index)
{
	WWASSERT (m_DestSector1 == (uint16)-1 || m_DestSector2 == (uint16)-1);
	WWASSERT (sector_index != -1);
	
	if (m_DestSector1 == (uint16)-1) {
		m_DestSector1 = sector_index;
	} else {
		m_DestSector2 = sector_index;
	}

	return ;
}

//////////////////////////////////////////////////////////////////////////
//	Add_Dest_Sector
//////////////////////////////////////////////////////////////////////////
inline void
PathfindPortalClass::Add_Dest_Sector (PathfindSectorClass *sector)
{
	int index = PathfindClass::Get_Instance ()->Get_Sector_Index (sector);
	Add_Dest_Sector (index);
	return ;
}

//////////////////////////////////////////////////////////////////////////
//	Get_Dest_Sector1
//////////////////////////////////////////////////////////////////////////
inline uint16
PathfindPortalClass::Get_Dest_Sector1 (void)
{
	return m_DestSector1;
}

//////////////////////////////////////////////////////////////////////////
//	Get_Dest_Sector2
//////////////////////////////////////////////////////////////////////////
inline uint16
PathfindPortalClass::Get_Dest_Sector2 (void)
{
	return m_DestSector2;
}

//////////////////////////////////////////////////////////////////////////
//	Peek_Dest_Sector
//////////////////////////////////////////////////////////////////////////
inline PathfindSectorClass *
PathfindPortalClass::Peek_Dest_Sector (PathfindSectorClass *current_sector)
{
	//
	//	Determine which destination sector to return
	//
	PathfindSectorClass *dest_sector = PathfindClass::Get_Instance ()->Peek_Sector (m_DestSector1);
	if (dest_sector == current_sector) {
		dest_sector = PathfindClass::Get_Instance ()->Peek_Sector (m_DestSector2);
	}

	return dest_sector;
}

//////////////////////////////////////////////////////////////////////////
//	Is_Two_Way_Portal
//////////////////////////////////////////////////////////////////////////
inline bool
PathfindPortalClass::Is_Two_Way_Portal (void) const
{
	return (m_DestSector1 != ((uint16)-1)) && (m_DestSector2 != ((uint16)-1));
}

//////////////////////////////////////////////////////////////////////////
//	Get_Heap_Location
//////////////////////////////////////////////////////////////////////////
inline uint32
PathfindPortalClass::Get_Heap_Location (void) const
{
	return m_HeapLocation;
}

//////////////////////////////////////////////////////////////////////////
//	Set_Heap_Location
//////////////////////////////////////////////////////////////////////////
inline void
PathfindPortalClass::Set_Heap_Location (uint32 location)
{
	m_HeapLocation = location;
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	PathfindActionPortalClass
//
//////////////////////////////////////////////////////////////////////////
class PathfindActionPortalClass : public PathfindPortalClass
{
public:

	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	PathfindActionPortalClass (void)
		:	m_Destination (0, 0, 0),
			m_EntranceSector (NULL),
			m_ExitPortal (NULL),
			m_EnterPortal (NULL),
			m_MechanismID (0),
			m_ActionID (PathClass::ACTION_NONE)		{ }

	~PathfindActionPortalClass (void)				{ }


	////////////////////////////////////////////////////////////////////
	//	RTTI
	////////////////////////////////////////////////////////////////////
	PathfindActionPortalClass *As_PathfindActionPortalClass (void) { return this; }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	
	//
	//	Action identification
	//
	PathClass::ACTION_ID	Get_Action_Type (void) const								{ return m_ActionID; }
	void						Set_Action_Type (PathClass::ACTION_ID action_id)	{ m_ActionID = action_id; }

	//
	//	Mechanism information (in case we have to interact to do this action)
	//
	uint32					Get_Mechanism_ID (void) const		{ return m_MechanismID; }
	void						Set_Mechanism_ID (uint32 id)		{ m_MechanismID = id; }

	//
	//	Destination control (where do we end up at the end of this action?)
	//
	void						Set_Destination (const Vector3 &pos)	{ m_Destination = pos; }
	const Vector3 &		Get_Destination (void) const				{ return m_Destination; }

	//
	//	Sector information
	//
	PathfindSectorClass *	Get_Entrance_Sector (void)									{ return m_EntranceSector; }
	void							Set_Entrance_Sector (PathfindSectorClass *sector)	{ m_EntranceSector = sector; }

	//
	//	Enter information
	//
	PathfindActionPortalClass *	Get_Enter_Portal (void)											{ return m_EnterPortal; }
	void									Set_Enter_Portal (PathfindActionPortalClass *portal)	{ m_EnterPortal = portal; }

	//
	//	Exit information
	//
	PathfindPortalClass *	Get_Exit_Portal (void)									{ return m_ExitPortal; }
	void							Set_Exit_Portal (PathfindPortalClass *portal)	{ m_ExitPortal = portal; }

	//
	//	Serialization
	//
	bool						Save (ChunkSaveClass &chunk_save);
	bool						Load (ChunkLoadClass &chunk_load);

protected:

	//////////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////////
	bool						Load_Variables (ChunkLoadClass &cload);

private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	Vector3								m_Destination;
	PathfindSectorClass *			m_EntranceSector;
	uint32								m_MechanismID;
	PathClass::ACTION_ID				m_ActionID;
	PathfindPortalClass *			m_ExitPortal;
	PathfindActionPortalClass *	m_EnterPortal;	
};


//////////////////////////////////////////////////////////////////////////
//
//	PathfindWaypathPortalClass
//
//////////////////////////////////////////////////////////////////////////
class PathfindWaypathPortalClass : public PathfindPortalClass
{
public:

	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	PathfindWaypathPortalClass (void)		{}
	~PathfindWaypathPortalClass (void)		{}


	////////////////////////////////////////////////////////////////////
	//	RTTI
	////////////////////////////////////////////////////////////////////
	PathfindWaypathPortalClass *As_PathfindWaypathPortalClass (void) { return this; }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////

	//
	//	From PathfindPortalClass
	//
	bool					Does_Size_Matter (void) const	{ return false; }	

	//
	//	Waypath information
	//
	void									Set_Waypath_Pos (const WaypathPositionClass &pos)	{ WaypathPos = pos; }
	const WaypathPositionClass &	Get_Waypath_Pos (void) const								{ return WaypathPos; }

	//
	//	Serialization
	//
	bool					Save (ChunkSaveClass &chunk_save);
	bool					Load (ChunkLoadClass &chunk_load);

protected:

	//////////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////////
	bool					Load_Variables (ChunkLoadClass &cload);

private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	WaypathPositionClass		WaypathPos;
};


#endif //__PATHFIND_PORTAL_H
