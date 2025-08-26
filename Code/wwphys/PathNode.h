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
 *                     $Archive:: /Commando/Code/wwphys/PathNode.h                            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/08/01 2:31p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __PATHNODE_H
#define __PATHNODE_H

#include "matrix3d.h"
#include "refcount.h"
#include "binheap.h"
#include "mempool.h"
#include "PathfindPortal.h"


/////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////
class PathfindSectorClass;


/////////////////////////////////////////////////////////////////////////
//
//	PathNodeClass
//
/////////////////////////////////////////////////////////////////////////
class PathNodeClass : public RefCountClass, public HeapNodeClass<float>, public AutoPoolClass<PathNodeClass, 512>
{
	public:

		/////////////////////////////////////////////////////////////////////////
		// Public constructors/destructors
		/////////////////////////////////////////////////////////////////////////
		PathNodeClass (void)
			:	m_Sector (NULL),
				m_ParentNode (NULL),
				m_Portal (NULL),
				m_TotalCost (0),
				m_HeuristicCost (0),
				m_TraversalCost (0),
				m_OnFinalPath (false),
				m_EnterTransform (1),
				m_Transform (1),
				m_HeapLocation (0),
				m_InClosedList (false)	{ }

		~PathNodeClass (void)			{ }

		/////////////////////////////////////////////////////////////////////////
		// Public methods
		/////////////////////////////////////////////////////////////////////////
		PathfindSectorClass *	Peek_Sector (void);
		PathfindPortalClass *	Peek_Portal (void);
		PathNodeClass *			Peek_Parent_Node (void);		

		void							Set_Sector (PathfindSectorClass *sector);
		void							Set_Portal (PathfindPortalClass *portal);
		void							Set_Parent_Node (PathNodeClass *node);

		float							Get_Total_Cost (void) const;

		float							Get_Heuristic_Cost (void) const;
		void							Set_Heuristic_Cost (float cost);
		
		float							Get_Traversal_Cost (void) const;
		void							Set_Traversal_Cost (float cost);

		Vector3						Get_Position (void) const;	
		const Matrix3D &			Get_Transform (void) const;
		void							Set_Transform (const Matrix3D &tm);

		const Matrix3D &			Get_Enter_Transform (void) const;
		void							Set_Enter_Transform (const Matrix3D &tm);

		bool							Is_On_Final_Path (void) const;
		void							On_Final_Path (bool on_path);

		// For distributed (multi-frame) solve...
		bool							Is_In_Closed_List (void) const;
		void							Reconnect_To_Portal (void);
		void							Disconnect_From_Portal (void);

		// From HeapNodeClass
		uint32						Get_Heap_Location (void) const;
		void							Set_Heap_Location (uint32 location);
		float							Heap_Key (void) const;

	protected:
		
		/////////////////////////////////////////////////////////////////////////
		// Protected methods
		/////////////////////////////////////////////////////////////////////////
		void							Eval_Total_Cost (void);
		
	private:

		/////////////////////////////////////////////////////////////////////////
		// Private member data
		/////////////////////////////////////////////////////////////////////////
		PathfindPortalClass *	m_Portal;
		PathfindSectorClass *	m_Sector;
		PathNodeClass *			m_ParentNode;
		float							m_TotalCost;
		float							m_HeuristicCost;
		float							m_TraversalCost;
		Matrix3D						m_EnterTransform;
		Matrix3D						m_Transform;
		bool							m_OnFinalPath;
		bool							m_InClosedList;
		uint32						m_HeapLocation;
};


/////////////////////////////////////////////////////////////////////////
//
//	Inlines
//
/////////////////////////////////////////////////////////////////////////

inline PathfindSectorClass *
PathNodeClass::Peek_Sector (void)								{ return m_Sector; }

inline PathfindPortalClass *
PathNodeClass::Peek_Portal (void)								{ return m_Portal; }

inline PathNodeClass *
PathNodeClass::Peek_Parent_Node (void)							{ return m_ParentNode; }

inline void
PathNodeClass::Set_Sector (PathfindSectorClass *sector)	{ m_Sector = sector; }

inline void
PathNodeClass::Set_Portal (PathfindPortalClass *portal)	{ m_Portal = portal; }

inline void
PathNodeClass::Set_Parent_Node (PathNodeClass *node)		{ m_ParentNode = node; }

inline float
PathNodeClass::Get_Total_Cost (void) const					{ return m_TotalCost; }

inline float
PathNodeClass::Get_Heuristic_Cost (void) const				{ return m_HeuristicCost; }

inline void
PathNodeClass::Set_Heuristic_Cost (float cost)				{ m_HeuristicCost = cost; Eval_Total_Cost (); }

inline float
PathNodeClass::Get_Traversal_Cost (void) const				{ return m_TraversalCost; }

inline void
PathNodeClass::Set_Traversal_Cost (float cost)				{ m_TraversalCost = cost; Eval_Total_Cost (); }

inline const Matrix3D &
PathNodeClass::Get_Transform (void) const						{ return m_Transform; }

inline void
PathNodeClass::Set_Transform (const Matrix3D &tm)			{ m_Transform = tm; }

inline Vector3
PathNodeClass::Get_Position (void) const						{ return m_Transform.Get_Translation (); }

inline const Matrix3D &
PathNodeClass::Get_Enter_Transform (void) const				{ return m_EnterTransform; }

inline void
PathNodeClass::Set_Enter_Transform (const Matrix3D &tm)	{ m_EnterTransform = tm; }

inline void
PathNodeClass::Eval_Total_Cost (void)							{ m_TotalCost = m_HeuristicCost + m_TraversalCost; }

inline bool
PathNodeClass::Is_On_Final_Path (void) const					{ return m_OnFinalPath; }

inline void
PathNodeClass::On_Final_Path (bool on_path)					{ m_OnFinalPath = on_path; }

inline uint32
PathNodeClass::Get_Heap_Location (void) const
{
	return m_HeapLocation;
}

inline void
PathNodeClass::Set_Heap_Location (uint32 location)
{
	m_HeapLocation = location;

	if (m_Portal != NULL) {
		m_Portal->Set_Heap_Location (location);
	}

	if (location == 0) {
		m_InClosedList = true;
	} else {
		m_InClosedList = false;
	}

	return ;
}

inline float
PathNodeClass::Heap_Key (void) const
{
	return m_TotalCost;
}

inline bool
PathNodeClass::Is_In_Closed_List (void) const
{
	return m_InClosedList;
}

inline void
PathNodeClass::Disconnect_From_Portal (void)
{
	WWASSERT (m_Portal != NULL);

	m_Portal->m_ClosedListPtr = NULL;
	m_Portal->Set_Heap_Location (0);
	return ;
}

inline void
PathNodeClass::Reconnect_To_Portal (void)
{
	if (m_Portal != NULL) {
		if (m_InClosedList) {
			m_Portal->m_ClosedListPtr = this;
		} else {
			m_Portal->Set_Heap_Location (m_HeapLocation);
		}
	}

	return ;
}


#endif //__PATHNODE_H

