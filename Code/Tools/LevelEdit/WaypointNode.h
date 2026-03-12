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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/WaypointNode.h      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/08/01 10:15a                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __WAYPOINT_NODE_H
#define __WAYPOINT_NODE_H

#include "node.h"
#include "vector.h"
#include "icons.h"
#include "decophys.h"

// Forward declarations
class PresetClass;
class WaypathNodeClass;


////////////////////////////////////////////////////////////////////////////
//
//	WaypointNodeClass
//
////////////////////////////////////////////////////////////////////////////
class WaypointNodeClass : public NodeClass
{
public:

	//////////////////////////////////////////////////////////////////
	//	Public flags
	//////////////////////////////////////////////////////////////////
	typedef enum
	{
		FLAG_REQUIRES_JUMP	= 0x00000001,

	} FLAGS;

	typedef enum
	{
		MODEL_START_PT			= 1,
		MODEL_MIDDLE_PT,
		MODEL_END_PT,

	} MODEL;


	//////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////
	WaypointNodeClass (PresetClass *preset = NULL);
	WaypointNodeClass (const WaypointNodeClass &src);
	~WaypointNodeClass (void);

	//////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////
	const WaypointNodeClass &operator= (const WaypointNodeClass &src);

	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////

	//
	// From PersistClass
	//
	virtual const PersistFactoryClass &	Get_Factory (void) const;

	//
	//	RTTI
	//
	WaypointNodeClass *As_WaypointNodeClass (void)	{ return this; }

	//
	// From NodeClass
	//
	void			Initialize (void);
	NodeClass *	Clone (void)								{ return new WaypointNodeClass (*this); }
	NODE_TYPE	Get_Type (void) const					{ return NODE_TYPE_WAYPOINT; }
	int			Get_Icon_Index (void) const			{ return WAYPATH_ICON; }
	PhysClass *	Peek_Physics_Obj (void)	const			{ return m_PhysObj; }
	bool			Is_Static (void) const					{ return false; }
	bool			Show_Settings_Dialog (void);
	bool			Can_Be_Rotated_Freely (void) const	{ return false; }
	void			On_Transform (void);
	void			On_Translate (void);
	void			On_Delete (void);

	//
	//	Export methods
	//
	void			Pre_Export (void);
	void			Post_Export (void);

	//	From PersistClass
	bool			Save (ChunkSaveClass &csave);
	bool			Load (ChunkLoadClass &cload);

	//
	// WaypointNodeClass specific
	//
	WaypathNodeClass *Peek_Waypath (void) const;
	void					Set_Waypath (WaypathNodeClass *waypath);
	void					Set_Model (WaypointNodeClass::MODEL model);

	void			Set_Flags (int flags);
	void			Set_Flag (int flag, bool onoff);
	bool			Get_Flag (int flag);

	float			Get_Speed (void) const;
	void			Set_Speed (float speed);

protected:

	//////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////

	//
	//	Save/load methods
	//
	bool							Load_Variables (ChunkLoadClass &cload);

	//
	//	Model methods
	//
	void							Update_Model (void);

	//
	//	Parent methods
	//
	void							Parent_Set_Transform (const Matrix3D &tm);
	void							Parent_Set_Position (const Vector3 &pos);

	//////////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////////
	DecorationPhysClass *	m_PhysObj;
	WaypathNodeClass *		m_Waypath;

	float							m_Speed;
	int							m_Flags;
	MODEL							m_ModelType;


	//////////////////////////////////////////////////////////////////
	//	Friends
	//////////////////////////////////////////////////////////////////
	friend class WaypathNodeClass;
};

//////////////////////////////////////////////////////////////////
//	Set_Waypath
//////////////////////////////////////////////////////////////////
inline void
WaypointNodeClass::Set_Waypath (WaypathNodeClass *waypath)
{
	m_Waypath = waypath;
}


//////////////////////////////////////////////////////////////////
//	Peek_Waypath
//////////////////////////////////////////////////////////////////
inline WaypathNodeClass *
WaypointNodeClass::Peek_Waypath (void) const
{
	return m_Waypath;
}


//////////////////////////////////////////////////////////////////
//	Get_Speed
//////////////////////////////////////////////////////////////////
inline float
WaypointNodeClass::Get_Speed (void) const
{
	return m_Speed;
}


//////////////////////////////////////////////////////////////////
//	Set_Speed
//////////////////////////////////////////////////////////////////
inline void
WaypointNodeClass::Set_Speed (float speed)
{
	m_Speed = speed;
	return ;
}

#endif //__WAYPOINT_NODE_H

