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
 *                 Project Name : wwphys																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/pathsolve.h            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/20/01 1:22p                                                $*
 *                                                                                             *
 *                    $Revision:: 10                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __PATH_SOLVE_H
#define __PATH_SOLVE_H


#include "vector.h"
#include "vector3.h"
#include "hermitespline.h"
#include "PathObject.h"
#include "binheap.h"
#include "refcount.h"
#include "postloadable.h"


/////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////
class PathfindPortalClass;
class PathfindSectorClass;
class PathNodeClass;
class	AABoxClass;
class WayPathClass;
class ChunkSaveClass;
class ChunkLoadClass;


typedef DynamicVectorClass<AABoxClass *>		BOX_LIST;


/////////////////////////////////////////////////////////////////////////
//
//	PathSolveClass
//
/////////////////////////////////////////////////////////////////////////
class PathSolveClass : public RefCountClass, public PostLoadableClass
{
public:

	/////////////////////////////////////////////////////////////////////////
	// Public friends
	/////////////////////////////////////////////////////////////////////////
	friend class PathMgrClass;

	/////////////////////////////////////////////////////////////////////////
	// Public data types
	/////////////////////////////////////////////////////////////////////////
	typedef enum
	{
		THINKING								= 0,
		SOLVED_PATH,
		ERROR_INVALID_START_POS,
		ERROR_INVALID_DEST_POS,
		ERROR_NO_PATH,
	}	STATE_DESC;

	/////////////////////////////////////////////////////////////////////////
	// Public constructors/destructors
	/////////////////////////////////////////////////////////////////////////
	PathSolveClass (void);
	PathSolveClass (const Vector3 &start, const Vector3 &end);
	~PathSolveClass (void);

	/////////////////////////////////////////////////////////////////////////
	// Public methods
	/////////////////////////////////////////////////////////////////////////		
	
	//
	//	Path evaluation
	//
	STATE_DESC			Timestep (unsigned int milliseconds = 2);

	//
	// Position methods
	//
	STATE_DESC			Reset (const Vector3 &start, const Vector3 &end, float sector_fudge = 0);
	const Vector3 &	Get_Start_Pos (void) const;
	const Vector3 &	Get_Dest_Pos (void) const;
	STATE_DESC			Get_State (void) const;

	//
	// Traversing object
	//
	void					Set_Path_Object (PathObjectClass &path_object);
	void					Get_Path_Object (PathObjectClass &path_object) const;

	//
	//	Priority access
	//
	void					Set_Priority (float priority)	{ m_Priority = priority; }
	float					Get_Priority (void) const		{ return m_Priority; }

	//
	//	Birth-time access
	//
	uint32				Get_Birth_Time (void) const	{ return m_BirthTime; }

	//
	// Volume access
	//
	void					Get_Volumes (BOX_LIST &sector_list, BOX_LIST &portal_list);

	//
	//	Save/load stuff
	//
	void					Save (ChunkSaveClass &csave);
	void					Load (ChunkLoadClass &cload);
	void					On_Post_Load (void);

	//
	// Distributed (multi-frame solve) methods
	//
	void					Process_Initial_Sector (void);
	void					Unlink_Pathfind_Hooks (void);


protected:

	/////////////////////////////////////////////////////////////////////////
	// Public data types
	/////////////////////////////////////////////////////////////////////////
	struct PathDataStruct
	{
		PathDataStruct (void)
			:	m_Portal (NULL), m_Point (0, 0, 0)	{ }

		PathDataStruct (PathfindPortalClass *portal, const Vector3 &point)
			:	m_Portal (portal), m_Point (point)	{ }

		bool operator== (const PathDataStruct &src) { return false; }
		bool operator!= (const PathDataStruct &src) { return true; }

		PathfindPortalClass *	m_Portal;
		Vector3						m_Point;
		Vector3						m_SectorCenter;
		Vector3						m_SectorExtent;
	};

	typedef DynamicVectorClass<PathNodeClass *>	PATHNODE_LIST;
	typedef DynamicVectorClass<PathDataStruct>	PATHPOINT_LIST;

	//
	//	Raw path access
	//
	PATHPOINT_LIST &	Get_Raw_Path (void)	{ return m_Path; }


	/////////////////////////////////////////////////////////////////////////
	// Protected methods
	/////////////////////////////////////////////////////////////////////////
	void		Initialize (float sector_fudge = 0);	

	void		Resolve_Path (unsigned int milliseconds);
	void		Process_Portals (PathNodeClass *node);
	void		Submit_Node (float traversal_cost, PathNodeClass *current_node, PathfindPortalClass *portal, PathfindSectorClass *dest_sector, const Matrix3D &current_tm, const Matrix3D &ending_tm);
	
	void		Reset_Lists (void);

	//
	//	Post process methods
	//
	void		Post_Process_Path (void);
	bool		Does_Line_Go_Through_Portal (const Vector3 &start, const Vector3 &end, const AABoxClass &box);
	Vector3	Relax_Line (const Vector3 &start, const Vector3 &end, const AABoxClass &box);
	void		Keep_Unit_Inside_Sectors (void);

	//
	//	Portal access
	//
	bool		Does_Object_Have_Access_To_Portal (PathfindPortalClass *portal);
	bool		Can_Object_Go_Through_Portal (const Matrix3D &current_tm, PathfindSectorClass *sector, PathfindPortalClass *portal, Matrix3D *ending_tm);

	//
	// Distributed (multi-frame solve) methods
	//
	//void		Begin_Distributed_Solve (void);
	//void		End_Distributed_Solve (void);

	//
	//	Save/load methods
	//
	void		Load_Variables (ChunkLoadClass &cload);

private:

	/////////////////////////////////////////////////////////////////////////
	// Static members
	/////////////////////////////////////////////////////////////////////////
	static __int64									_TicksPerMilliSec;

	/////////////////////////////////////////////////////////////////////////
	// Private member data
	/////////////////////////////////////////////////////////////////////////
	Vector3											m_StartPos;
	Vector3											m_DestPos;		
	STATE_DESC										m_State;
	float												m_Priority;
	uint32											m_BirthTime;

	PathfindSectorClass *						m_StartSector;
	PathfindSectorClass *						m_DestSector;
	
	DynamicVectorClass<PathNodeClass *>		m_NodeList;
	BinaryHeapClass<float>						m_BinaryHeap;

	PathNodeClass *								m_CompletedNode;
	PATHPOINT_LIST									m_Path;

	PathObjectClass								m_PathObject;

	static PATHNODE_LIST							temp_node_list;

	/////////////////////////////////////////////////////////////////////////
	// Friends
	/////////////////////////////////////////////////////////////////////////
	friend class PathClass;
};


/////////////////////////////////////////////////////////////////////////
//	Inlines
/////////////////////////////////////////////////////////////////////////
inline PathSolveClass::STATE_DESC
PathSolveClass::Get_State (void) const				{ return m_State; }

inline const Vector3 &
PathSolveClass::Get_Start_Pos (void) const		{ return m_StartPos; }

inline const Vector3 &
PathSolveClass::Get_Dest_Pos (void) const			{ return m_DestPos; }


#endif //__PATH_SOLVE_H
