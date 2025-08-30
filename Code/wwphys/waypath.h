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
 *                     $Archive:: /Commando/Code/wwphys/waypath.h           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/10/01 11:20a                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __WAYPATH_H
#define __WAYPATH_H

#include "persist.h"
#include "refcount.h"
#include "vector.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class WaypointClass;
class WaypathPositionClass;
class Vector3;

////////////////////////////////////////////////////////////////
//
//	WaypathClass
//
////////////////////////////////////////////////////////////////
class WaypathClass : public PersistClass, public RefCountClass
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Public flags
	////////////////////////////////////////////////////////////////
	typedef enum
	{
		FLAG_TWO_WAY				= 0x00000001,
		FLAG_LOOPING				= 0x00000002,
		FLAG_HUMAN					= 0x00000004,
		FLAG_GROUND_VEHICLE		= 0x00000008,
		FLAG_FLYING_VEHICLE		= 0x00000010,
		FLAG_INNATE_PATHFIND		= 0x00000020
		
	} FLAGS;


	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	WaypathClass (void);
	WaypathClass (const WaypathPositionClass &start, const WaypathPositionClass &end);
	virtual ~WaypathClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Identifaction methods
	//
	int						Get_ID (void) const						{ return m_ID; }
	void						Set_ID (int id)							{ m_ID = id; }

	//
	//	Waypoint methods
	//
	int						Get_Point_Count (void) const			{ return m_Waypoints.Count (); }
	WaypointClass *		Get_Point (int index) const;
	void						Add_Point (const WaypointClass &point);
	void						Add_Point (const Vector3 &point);

	//
	//	Flags
	//
	int						Get_Flags (void) const					{ return m_Flags; }
	void						Set_Flags (int flags)					{ m_Flags = flags; }
	bool						Get_Flag (int flag)						{ return bool((m_Flags & flag) == flag); }
	void						Set_Flag (int flag, bool onoff);

	//
	//	Position methods
	//
	bool						Evaluate_Position (const WaypathPositionClass &pos, Vector3 *position);
	
	//
	//	Serialization methods (from PersistClass)
	//
	const PersistFactoryClass &	Get_Factory (void) const override;
	bool						Save (ChunkSaveClass &csave) override;
	bool						Load (ChunkLoadClass &cload) override;
	void						On_Post_Load (void) override;

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void						Free (void);
	bool						Load_Variables (ChunkLoadClass &cload);		

private:

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	int						m_ID;
	int						m_Flags;
	DynamicVectorClass<WaypointClass *>	m_Waypoints;

};


//////////////////////////////////////////////////////////////////////////////
//	Get_Point
//////////////////////////////////////////////////////////////////////////////
inline WaypointClass *
WaypathClass::Get_Point (int index) const
{
	WaypointClass *waypoint = NULL;

	//
	//	Check to ensure the requested index is valid
	//
	if (index >= 0 && index < m_Waypoints.Count ()) {
		waypoint = m_Waypoints[index];
	}

	return waypoint;
}


#endif //__WAYPATH_H
