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
 *                     $Archive:: /Commando/Code/wwphys/floodfillbox.h         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/03/01 10:27a                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __FLOODFILLBOX_H
#define __FLOODFILLBOX_H

#include "vector3.h"
#include "matrix3d.h"
#include "vector.h"
#include "refcount.h"
#include "bittype.h"

//////////////////////////////////////////////////////////////////////////
// Forward delcarations
//////////////////////////////////////////////////////////////////////////
class FloodfillBoxClass;
class PathfindSectorClass;
class PathfindPortalClass;


//////////////////////////////////////////////////////////////////////////
//	Data types
//////////////////////////////////////////////////////////////////////////
typedef DynamicVectorClass<FloodfillBoxClass *>	BODY_BOX_LIST;

typedef enum
{
	DIR_FORWARD	= 0,
	DIR_LEFT,
	DIR_BACKWARD,
	DIR_RIGHT,
	DIR_MAX,

	DIR_UP		= DIR_FORWARD,
	DIR_DOWN		= DIR_BACKWARD,
} PATHFIND_DIR;

typedef enum
{
	MASK_NONE		= 0,
	MASK_FORWARD	= 1,
	MASK_LEFT		= 2,
	MASK_BACKWARD	= 4,
	MASK_RIGHT		= 8,
	MASK_MAX,
	
	MASK_UP			= MASK_FORWARD,
	MASK_DOWN		= MASK_BACKWARD,

} DIRECTION_MASK;


inline PATHFIND_DIR
Mask_to_Dir (DIRECTION_MASK mask)
{
	for (int dir = 0; dir < DIR_MAX; dir ++) {
		if ((1 << dir) == mask) {
			return PATHFIND_DIR(dir);
		}
	}

	return DIR_MAX;
}

inline PATHFIND_DIR
Inverse_Pathfind_Dir (PATHFIND_DIR dir)
{
	if (dir < 2) return PATHFIND_DIR(dir + 2);
	return PATHFIND_DIR(dir - 2);
}

inline PATHFIND_DIR
Clockwise_Pathfind_Dir (PATHFIND_DIR dir)
{
	return PATHFIND_DIR((dir + 3) % DIR_MAX);
}


//////////////////////////////////////////////////////////////////////////
//
//	FloodfillBoxClass
//
//////////////////////////////////////////////////////////////////////////
class FloodfillBoxClass : public RefCountClass
{		
	public:

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		FloodfillBoxClass (void) 
			:	m_Sector (NULL),
				m_Flags (NeedsProcessing),
				m_DirectionInfo (0),
				m_CompressedSkipCount (0),
				m_MinZPos (0),
				m_MaxZPos (0),
				m_GridLink (NULL),
				m_Prev (NULL),
				m_Next (NULL)					{ ::memset (m_Neighbors, 0, sizeof (m_Neighbors));  }

		virtual ~FloodfillBoxClass (void)	{ Remove (); }


		//////////////////////////////////////////////////////////////////////
		//	Access to the Position/Orientation state of the object
		//////////////////////////////////////////////////////////////////////
		Vector3							Get_Position (void) const { return m_Position; }
		void								Set_Position (const Vector3 &pos) { m_Position = pos; }
		float								Get_Min_Z_Pos (void) const { return m_MinZPos; }
		float								Get_Max_Z_Pos (void) const { return m_MaxZPos; }
		void								Set_Min_Z_Pos (float pos)	{ m_MinZPos = pos; }
		void								Set_Max_Z_Pos (float pos)	{ m_MaxZPos = pos; }

		Vector3							Get_Center (void) const;
		Vector3							Get_Extent (void) const;

		//////////////////////////////////////////////////////////////////////
		//	Timestep methods
		//////////////////////////////////////////////////////////////////////
		void								Timestep (float /* dt */) {}

		//////////////////////////////////////////////////////////////////////
		//	Adjacent sector methods
		//////////////////////////////////////////////////////////////////////
		FloodfillBoxClass *			Peek_Neighbor (PATHFIND_DIR dir, bool only_traversible = true);
		void								Set_Neighbor (PATHFIND_DIR dir, FloodfillBoxClass *obj, bool traversible);
		FloodfillBoxClass *			Find_Relative (int x, int y);
		bool								Is_Two_Way_Traversible (PATHFIND_DIR dir);

		void								Set_Taken (bool onoff = true)	{ Set_Flag (IsTaken, onoff); }
		bool								Is_Taken (void)					{ return bool((m_Flags & IsTaken) == IsTaken); }

		uint8								Get_Compress_Skipped_Count (void)	{ return m_CompressedSkipCount; }
		void								Inc_Compress_Skipped_Count (void)	{ m_CompressedSkipCount ++; }

		//////////////////////////////////////////////////////////////////////
		//	Delayed processing methods
		//////////////////////////////////////////////////////////////////////
		bool								Needs_Processing (void) const		{ return bool((m_Flags & NeedsProcessing) == NeedsProcessing); }
		void								Needs_Processing (bool needs_it) { Set_Flag (NeedsProcessing, needs_it); }

		//////////////////////////////////////////////////////////////////////
		//	Linked list methods
		//////////////////////////////////////////////////////////////////////
		static FloodfillBoxClass *	Get_First (void) { return m_First; }
		static FloodfillBoxClass *	Get_Last (void) { return m_Last; }
		FloodfillBoxClass *			Get_Next (void) { return m_Next; }
		FloodfillBoxClass *			Get_Prev (void) { return m_Prev; }

		void								Set_Next (FloodfillBoxClass *next) { m_Next = next; }
		void								Set_Prev (FloodfillBoxClass *prev) { m_Prev = prev; }

		static void						Add (FloodfillBoxClass *new_obj);
		void								Remove (void);

		void								Unlink (void);
		bool								Is_Edge (void);
		bool								Are_Neighbors_Processed (void);

		//////////////////////////////////////////////////////////////////////
		//	Portal generation methods
		//////////////////////////////////////////////////////////////////////
		PathfindSectorClass *		Peek_Sector (void) { return m_Sector; }
		void								Set_Sector (PathfindSectorClass *sector) { m_Sector = sector; }

		bool								Is_New_Portal (PATHFIND_DIR dir, PathfindSectorClass *dest_sector = NULL);
		PathfindPortalClass *		Make_Portal (PATHFIND_DIR dir, const Vector3 &box_size, float z_pos, float	min_acceptable_size);
		void								Reset_Portal_Info (void) { (m_DirectionInfo &= ~PortalMask); }

		void								Part_Of_Portal (PATHFIND_DIR dir, bool onoff);
		bool								Part_Of_Portal (PATHFIND_DIR dir);

		//////////////////////////////////////////////////////////////////////
		//	Grid link methods
		//////////////////////////////////////////////////////////////////////
		FloodfillBoxClass *			Get_Grid_Link (void)								{ return m_GridLink; }
		void								Set_Grid_Link (FloodfillBoxClass *link)	{ m_GridLink = link; }

	protected:

		//////////////////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////////////////
		void								Set_Flag (uint8 flag, bool value);

		void								Set_Traversible (PATHFIND_DIR dir, bool onoff);
		bool								Get_Traversible (PATHFIND_DIR dir);

	private:

		//////////////////////////////////////////////////////////////////////
		//	Private methods
		//////////////////////////////////////////////////////////////////////
		
		typedef enum
		{
			IsTaken				= 0x01,
			NeedsProcessing	= 0x02,
		} Flags;

		typedef enum
		{
			IsTraversibleUp		= 0x01,
			IsTraversibleLeft		= 0x02,
			IsTraversibleDown		= 0x04,
			IsTraversibleRight	= 0x08,
			TraversibleMask		= 0x0F,

			IsPortalUp				= 0x10,
			IsPortalLeft			= 0x20,
			IsPortalDown			= 0x40,
			IsPortalRight			= 0x80,
			PortalMask				= 0xF0,

		} Directions;

		//////////////////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////////////////
		FloodfillBoxClass *		m_Neighbors[4];

		uint8							m_Flags;
		uint8							m_DirectionInfo;
		uint8							m_CompressedSkipCount;

		FloodfillBoxClass *		m_Prev;
		FloodfillBoxClass *		m_Next;

		PathfindSectorClass *	m_Sector;

		float							m_MinZPos;
		float							m_MaxZPos;

		Vector3						m_Position;
		FloodfillBoxClass *		m_GridLink;

		//////////////////////////////////////////////////////////////////////
		//	Static member data
		//////////////////////////////////////////////////////////////////////
		static FloodfillBoxClass *	m_First;
		static FloodfillBoxClass *	m_Last;
};


//////////////////////////////////////////////////////////////////////////
// Find_Relative
//////////////////////////////////////////////////////////////////////////
inline FloodfillBoxClass *
FloodfillBoxClass::Find_Relative (int cx, int cy)
{
	PATHFIND_DIR x_dir = DIR_RIGHT;
	PATHFIND_DIR y_dir = DIR_BACKWARD;

	if (cx < 0) {
		x_dir = DIR_LEFT;
		cx = -cx;
	}

	if (cy < 0) {
		y_dir = DIR_FORWARD;
		cy = -cy;
	}

	FloodfillBoxClass *curr_box = this;

	for (int x = 0; x < cx; x ++) {
		curr_box = curr_box->Peek_Neighbor (x_dir);
	}

	for (int y = 0; y < cy; y ++) {
		curr_box = curr_box->Peek_Neighbor (y_dir);
	}

	return curr_box;
}

//////////////////////////////////////////////////////////////////////////
// Remove
//////////////////////////////////////////////////////////////////////////
inline void
FloodfillBoxClass::Remove (void)
{
	if (m_Prev) {
		m_Prev->m_Next = m_Next;
	}
	
	if (m_Next) {
		m_Next->m_Prev = m_Prev;
	}
	
	if (m_First == this) {
		m_First = m_Next;
	}

	if (m_Last == this) {
		m_Last = m_Prev;
	}	
	
	m_Prev = NULL;
	m_Next = NULL;
	return ;
}

//////////////////////////////////////////////////////////////////////////
// Add
//////////////////////////////////////////////////////////////////////////
inline void
FloodfillBoxClass::Add (FloodfillBoxClass *new_obj)
{
	if (m_Last == NULL) {
		m_First = new_obj;
	} else {
		m_Last->m_Next = new_obj;
		new_obj->m_Prev = m_Last;
	}

	m_Last = new_obj;
	return ;
}

//////////////////////////////////////////////////////////////////////////
// Unlink
//////////////////////////////////////////////////////////////////////////
inline void
FloodfillBoxClass::Unlink (void)
{
	Remove ();
	if (m_Neighbors[DIR_FORWARD] != NULL) {
		m_Neighbors[DIR_FORWARD]->Set_Neighbor (DIR_BACKWARD, NULL, false);
	}

	if (m_Neighbors[DIR_BACKWARD] != NULL) {
		m_Neighbors[DIR_BACKWARD]->Set_Neighbor (DIR_FORWARD, NULL, false);
	}

	if (m_Neighbors[DIR_LEFT] != NULL) {
		m_Neighbors[DIR_LEFT]->Set_Neighbor (DIR_RIGHT, NULL, false);
	}

	if (m_Neighbors[DIR_RIGHT] != NULL) {
		m_Neighbors[DIR_RIGHT]->Set_Neighbor (DIR_LEFT, NULL, false);
	}

	return ;
}

//////////////////////////////////////////////////////////////////////////
// Is_Edge
//////////////////////////////////////////////////////////////////////////
inline bool
FloodfillBoxClass::Is_Edge (void)
{
	bool retval = false;

	if (Peek_Neighbor (DIR_FORWARD) == NULL ||
		 Peek_Neighbor (DIR_BACKWARD) == NULL ||
		 Peek_Neighbor (DIR_LEFT) == NULL ||
		 Peek_Neighbor (DIR_RIGHT) == NULL) {
		
		retval = true;
	}

	return retval;
}

//////////////////////////////////////////////////////////////////////////
// Are_Neighbors_Processed
//////////////////////////////////////////////////////////////////////////
inline bool
FloodfillBoxClass::Are_Neighbors_Processed (void)
{
	bool retval = true;

	for (int index = 0; (index < 4) && retval; index ++) {
		if ((m_Neighbors[0] != NULL) && m_Neighbors[0]->Needs_Processing ()) {
			retval = false;
		}
	}

	return retval;
}

//////////////////////////////////////////////////////////////////////////
// Set_Neighbor
//////////////////////////////////////////////////////////////////////////
inline void
FloodfillBoxClass::Set_Neighbor (PATHFIND_DIR dir, FloodfillBoxClass *obj, bool traversible)
{
	m_Neighbors[dir] = obj;	
	Set_Traversible (dir, traversible);
	return ;
}

//////////////////////////////////////////////////////////////////////////
// Peek_Neighbor
//////////////////////////////////////////////////////////////////////////
inline FloodfillBoxClass *
FloodfillBoxClass::Peek_Neighbor (PATHFIND_DIR dir, bool only_traversible)
{
	FloodfillBoxClass *neighbor = NULL;
	if ((only_traversible == false) || Get_Traversible (dir)) {
		neighbor = m_Neighbors[dir];
	}

	return neighbor;
}

//////////////////////////////////////////////////////////////////////////
// Set_Flag
//////////////////////////////////////////////////////////////////////////
inline void
FloodfillBoxClass::Set_Flag (uint8 flag, bool value)
{
	m_Flags &= ~flag;
	if (value) {
		m_Flags |= flag;
	}
	return ;
}

//////////////////////////////////////////////////////////////////////////
// Set_Traversible
//////////////////////////////////////////////////////////////////////////
inline void
FloodfillBoxClass::Set_Traversible (PATHFIND_DIR dir, bool onoff)
{
	if (onoff) {
		m_DirectionInfo |= (1 << dir);
	} else {
		m_DirectionInfo &= ~(1 << dir);
	}

	return ;
}

//////////////////////////////////////////////////////////////////////////
// Get_Traversible
//////////////////////////////////////////////////////////////////////////
inline bool
FloodfillBoxClass::Get_Traversible (PATHFIND_DIR dir)
{
	int bit = (1 << dir);
	return (m_DirectionInfo & bit) == bit;
}

//////////////////////////////////////////////////////////////////////////
// Part_Of_Portal
//////////////////////////////////////////////////////////////////////////
inline void
FloodfillBoxClass::Part_Of_Portal (PATHFIND_DIR dir, bool onoff)
{
	if (onoff) {
		m_DirectionInfo |= (1 << (dir + 4));
	} else {
		m_DirectionInfo &= ~(1 << (dir + 4));
	}

	return ;
}

//////////////////////////////////////////////////////////////////////////
// Part_Of_Portal
//////////////////////////////////////////////////////////////////////////
inline bool
FloodfillBoxClass::Part_Of_Portal (PATHFIND_DIR dir)
{
	int bit = (1 << (dir + 4));
	return (m_DirectionInfo & bit) == bit;
}


#endif //__FLOODFILLBOX_H

