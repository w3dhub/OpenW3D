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
 *                     $Archive:: /Commando/Code/wwphys/PathfindSector.h                      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/04/01 2:48p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif


#ifndef __PATHFIND_SECTOR_H
#define __PATHFIND_SECTOR_H

#include "refcount.h"
#include "vector.h"
#include "aabox.h"
#include "cullsys.h"

//////////////////////////////////////////////////////////////////////////
// Forward declarations
//////////////////////////////////////////////////////////////////////////
class PhysClass;
class ChunkSaveClass;
class ChunkLoadClass;
class PathfindPortalClass;
class PathfindWaypathSectorClass;


//////////////////////////////////////////////////////////////////////////
//
//	PathfindSectorClass
//
//////////////////////////////////////////////////////////////////////////
class PathfindSectorClass : public CullableClass
{
public:

	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	PathfindSectorClass (void)
		:	m_IsValid (true)		{}

	PathfindSectorClass (const AABoxClass &/* box */)
		:	m_IsValid (true)		{}

	virtual ~PathfindSectorClass (void);

	////////////////////////////////////////////////////////////////////
	//	RTTI
	////////////////////////////////////////////////////////////////////
	virtual PathfindWaypathSectorClass *			As_PathfindWaypathSectorClass (void)			{ return NULL; }
	virtual const PathfindWaypathSectorClass *	As_PathfindWaypathSectorClass (void) const	{ return NULL; }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	
	//
	//	Bounding box access
	//
	const AABoxClass &	Get_Bounding_Box (void) const;
	void						Set_Bounding_Box (const AABoxClass &box);

	//
	//	Portal managment
	//
	void						Add_Portal (uint32 portal_id);
	void						Remove_Portal (uint32 portal_id);
	void						Reset_Portal_List (void);

	//
	//	Portal access
	//
	int						Get_Portal_Count (void);
	PathfindPortalClass *Peek_Portal (int index);

	//
	//	Validity managment
	//
	bool						Is_Valid (void)				{ return m_IsValid; }
	void						Set_Valid (bool is_valid)	{ m_IsValid = is_valid; }

	//
	//	Portal testing methods
	//
	virtual bool			Can_Access_Portal (PathfindPortalClass *last_portal, PathfindPortalClass *test_portal);

	//
	//	Serialization methods
	//
	virtual bool			Save (ChunkSaveClass &csave);
	virtual bool			Load (ChunkLoadClass &cload);		

protected:

	////////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////////
	DynamicVectorClass<uint32>		m_PortalList;
	bool									m_IsValid;

private:

	//////////////////////////////////////////////////////////////////////
	//	Private methods
	//////////////////////////////////////////////////////////////////////
	bool						Load_Variables (ChunkLoadClass &cload);
};


//////////////////////////////////////////////////////////////////////////
//	Inlines
//////////////////////////////////////////////////////////////////////////

inline const AABoxClass &
PathfindSectorClass::Get_Bounding_Box (void) const
{
	return Get_Cull_Box ();
}

inline void
PathfindSectorClass::Set_Bounding_Box (const AABoxClass &box)
{
	Set_Cull_Box (box);
	return ;
}

inline void
PathfindSectorClass::Add_Portal (uint32 portal_id)
{
	m_PortalList.Add (portal_id);
	return ;
}

inline int
PathfindSectorClass::Get_Portal_Count (void)
{
	return m_PortalList.Count ();
}

inline bool
PathfindSectorClass::Can_Access_Portal
(
	PathfindPortalClass *last_portal,
	PathfindPortalClass *test_portal
)
{
	return (test_portal != last_portal);
}


//////////////////////////////////////////////////////////////////////////
//
//	PathfindWaypathSectorClass
//
//		This class is used to represent a logical sector that is solely
//	comprised of enter and exit portals.  These portals are sorted by
// occurance on the waypath.
//
//////////////////////////////////////////////////////////////////////////
class PathfindWaypathSectorClass : public PathfindSectorClass
{
public:

	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	PathfindWaypathSectorClass (void)
		: WaypathID (-1)						{}
	~PathfindWaypathSectorClass (void)	{}

	////////////////////////////////////////////////////////////////////
	//	RTTI
	////////////////////////////////////////////////////////////////////
	PathfindWaypathSectorClass *			As_PathfindWaypathSectorClass (void) override			{ return this; }
	const PathfindWaypathSectorClass *	As_PathfindWaypathSectorClass (void) const override	{ return this; }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	
	//
	//	Waypath access
	//
	void					Set_Waypath_ID (int waypath_id)	{ WaypathID = waypath_id; }
	int					Get_Waypath_ID (void)				{ return WaypathID; }
	
	//
	//	From PathfindSectorClass
	//
	bool					Can_Access_Portal (PathfindPortalClass *last_portal, PathfindPortalClass *test_portal) override;
	bool					Save (ChunkSaveClass &csave) override;
	bool					Load (ChunkLoadClass &cload) override;

protected:

	////////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////////
	int					WaypathID;

private:

	//////////////////////////////////////////////////////////////////////
	//	Private methods
	//////////////////////////////////////////////////////////////////////
	bool					Load_Variables (ChunkLoadClass &cload);
};


#endif //__PATHFIND_SECTOR_H


