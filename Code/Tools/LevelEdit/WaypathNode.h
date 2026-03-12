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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/WaypathNode.h      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/11/01 8:59a                                               $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __WAYPATH_NODE_H
#define __WAYPATH_NODE_H

#include "node.h"
#include "vector.h"
#include "vector3.h"
#include "icons.h"
#include "decophys.h"
#include "waypointnode.h"
#include "waypath.h"


////////////////////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////////////////////
class PresetClass;
class SegmentedLineClass;
class WaypathClass;


////////////////////////////////////////////////////////////////////////////
//
//	WaypathNodeClass
//
////////////////////////////////////////////////////////////////////////////
class WaypathNodeClass : public NodeClass
{
public:

	//////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////
	WaypathNodeClass (PresetClass *preset = NULL);
	WaypathNodeClass (const WaypathNodeClass &src);
	~WaypathNodeClass (void);

	//////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////
	const WaypathNodeClass &operator= (const WaypathNodeClass &src);

	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////

	//
	// From PersistClass
	//
	const PersistFactoryClass &	Get_Factory (void) const;
	void									On_Post_Load (void);

	//
	//	RTTI
	//
	WaypathNodeClass *As_WaypathNodeClass (void)		{ return this; }

	//
	// From NodeClass
	//
	void			Initialize (void);
	NodeClass *	Clone (void)								{ return new WaypathNodeClass (*this); }
	NODE_TYPE	Get_Type (void) const					{ return NODE_TYPE_WAYPATH; }
	int			Get_Icon_Index (void) const			{ return WAYPATH_ICON; }
	PhysClass *	Peek_Physics_Obj (void)	const			{ return m_PhysObj; }
	bool			Is_Static (void) const					{ return false; }
	bool			Show_Settings_Dialog (void);
	bool			Can_Be_Rotated_Freely (void) const	{ return true; }
	void			Set_Transform (const Matrix3D &tm);
	void			Add_To_Scene (void);
	void			Remove_From_Scene (void);
	void			Hide (bool hide);
	void			On_Delete (void);
	void			On_Restore (void);

	int			Get_Sub_Node_Count (void) const		{ return Get_Point_Count (); }
	NodeClass *	Get_Sub_Node (int index)				{ return m_PointList[index]; }

	//
	//	Export methods
	//
	void			Pre_Export (void);
	void			Post_Export (void);

	//	From PersistClass
	bool			Save (ChunkSaveClass &csave);
	bool			Load (ChunkLoadClass &cload);

	//
	// WaypathNodeClass specific
	//
	int			Add_Point (const Vector3 &point);
	void			Insert_Point (int index, const Vector3 &point);
	void			Delete_Point (int index);
	int			Find_Index (WaypointNodeClass *waypoint);
	void			On_Point_Moved (int index, const Vector3 &new_pos);
	int			Get_Point_Count (void) const;
	void			Get_Point (int index, Vector3 &point);
	void			Get_Point (int index, WaypointNodeClass **waypoint);
	void			Update_Line (void);

	void			Set_Flags (int flags);
	void			Set_Flag (int flag, bool onoff);
	bool			Get_Flag (int flag);


	//////////////////////////////////////////////////////////////////
	//	Public flags
	//////////////////////////////////////////////////////////////////
	typedef enum
	{
		FLAG_TWO_WAY			= WaypathClass::FLAG_TWO_WAY,
		FLAG_LOOPING			= WaypathClass::FLAG_LOOPING,
		FLAG_HUMAN				= WaypathClass::FLAG_HUMAN,
		FLAG_GROUND_VEHICLE	= WaypathClass::FLAG_GROUND_VEHICLE,
		FLAG_FLYING_VEHICLE	= WaypathClass::FLAG_FLYING_VEHICLE,
		FLAG_INNATE_PATHFIND	= WaypathClass::FLAG_INNATE_PATHFIND

	} FLAGS;

protected:

	//////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////

	//
	//	Save/load methods
	//
	bool						Load_Variables (ChunkLoadClass &cload);

	//
	//	Misc utility methods
	//
	void						Free_Waypoints (void);
	SegmentedLineClass *	Peek_Line (void);
	WaypointNodeClass *	Create_Waypoint (const Vector3 &point);

	//////////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////////
	DecorationPhysClass *							m_PhysObj;
	DynamicVectorClass<WaypointNodeClass *>	m_PointList;
	int													m_Flags;
	WaypathClass *										m_RuntimeWaypath;
	bool													m_HasLoadCompleted;

	DynamicVectorClass<WaypointNodeClass *>	m_OldStylePointList;
};


//////////////////////////////////////////////////////////////////
//	Get_Point_Count
//////////////////////////////////////////////////////////////////
inline int
WaypathNodeClass::Get_Point_Count (void) const
{
	return m_PointList.Count ();
}

//////////////////////////////////////////////////////////////////
//	Get_Point
//////////////////////////////////////////////////////////////////
inline void
WaypathNodeClass::Get_Point (int index, Vector3 &point)
{
	if (index >= 0 && index < m_PointList.Count ()) {
		point = m_PointList[index]->Get_Position ();
	}

	return ;
}

//////////////////////////////////////////////////////////////////
//	Get_Point
//////////////////////////////////////////////////////////////////
inline void
WaypathNodeClass::Get_Point (int index, WaypointNodeClass **waypoint)
{
	if (index >= 0 && index < m_PointList.Count ()) {
		(*waypoint) = m_PointList[index];
	}

	return ;
}

#endif //__WAYPATH_NODE_H

