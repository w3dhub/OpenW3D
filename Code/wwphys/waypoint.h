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
 *                     $Archive:: /Commando/Code/wwphys/waypoint.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 4/17/01 8:42a                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __WAYPOINT_H
#define __WAYPOINT_H

#include "persist.h"
#include "vector3.h"
#include "refcount.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class PathfindActionPortalClass;


////////////////////////////////////////////////////////////////
//
//	WaypointClass
//
////////////////////////////////////////////////////////////////
class WaypointClass : public PersistClass, public RefCountClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public flags
	////////////////////////////////////////////////////////////////	
	typedef enum
	{
		FLAG_REQUIRES_JUMP	= 0x00000001,
		FLAG_REQUIRES_ACTION

	} FLAGS;

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	WaypointClass (void);
	WaypointClass (const WaypointClass &src);
	WaypointClass (const Vector3 &position);
	virtual ~WaypointClass (void);

	////////////////////////////////////////////////////////////////
	//	Public operators
	////////////////////////////////////////////////////////////////
	const WaypointClass &	operator= (const WaypointClass &src);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Identifaction methods
	//
	int						Get_ID (void) const						{ return m_ID; }
	void						Set_ID (int id)							{ m_ID = id; }

	//
	//	Position methods
	//
	const Vector3 &		Get_Position (void) const				{ return m_Position; }
	void						Set_Position (const Vector3 &pos)	{ m_Position = pos; }

	//
	//	Flags
	//
	int						Get_Flags (void) const					{ return m_Flags; }
	void						Set_Flags (int flags)					{ m_Flags = flags; }
	bool						Get_Flag (int flag)						{ return bool((m_Flags & flag) == flag); }
	void						Set_Flag (int flag, bool onoff);	

	//
	//	Action access
	//
	PathfindActionPortalClass *	Get_Action_Portal (void);
	void									Set_Action_Portal (PathfindActionPortalClass *portal);

	//
	//	Serialization methods (from PersistClass)
	//
	const PersistFactoryClass &	Get_Factory (void) const override;
	bool									Save (ChunkSaveClass &csave) override;
	bool									Load (ChunkLoadClass &cload) override;

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void						Free (void) {}
	bool						Load_Variables (ChunkLoadClass &cload);

private:

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	int						m_ID;
	Vector3					m_Position;
	int						m_Flags;
	int						m_ActionPortalID;
};


//////////////////////////////////////////////////////////////////////////////
//	Set_Flag
//////////////////////////////////////////////////////////////////////////////
inline void
WaypointClass::Set_Flag (int flag, bool onoff)
{
	m_Flags &= ~flag;
	if (onoff) {
		m_Flags |= flag;
	}

	return ;
}


#endif //__WAYPOINT_H
