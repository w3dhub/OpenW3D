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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/PathfindStartNode.h    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/14/99 4:00p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __PATHFIND_START_NODE_H
#define __PATHFIND_START_NODE_H

#include "node.h"
#include "vector.h"
#include "icons.h"
#include "decophys.h"

// Forward declarations
class PresetClass;


////////////////////////////////////////////////////////////////////////////
//
//	PathfindStartNodeClass
//
////////////////////////////////////////////////////////////////////////////
class PathfindStartNodeClass : public NodeClass
{
public:

	//////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////
	PathfindStartNodeClass (PresetClass *preset = NULL);
	PathfindStartNodeClass (const PathfindStartNodeClass &src);
	~PathfindStartNodeClass (void);

	//////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////
	const PathfindStartNodeClass &operator= (const PathfindStartNodeClass &src);

	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////

	//
	// PathfindStartNodeClass specific
	//

	//
	// From PersistClass
	//
	virtual const PersistFactoryClass &	Get_Factory (void) const;

	//
	// From NodeClass
	//
	void			Initialize (void);
	NodeClass *	Clone (void)								{ return new PathfindStartNodeClass (*this); }
	NODE_TYPE	Get_Type (void) const					{ return NODE_TYPE_PATHFIND_START; }
	int			Get_Icon_Index (void) const			{ return PATHFIND_ICON; }
	PhysClass *	Peek_Physics_Obj (void)	const			{ return m_PhysObj; }
	bool			Is_Static (void) const					{ return false; }
	bool			Show_Settings_Dialog (void)			{ return true; }
	bool			Can_Be_Rotated_Freely (void) const	{ return false; }
	void			Set_Transform (const Matrix3D &tm);

	//
	//	Export methods
	//
	void			Pre_Export (void);
	void			Post_Export (void);


	//	From PersistClass
	bool			Save (ChunkSaveClass &csave);
	bool			Load (ChunkLoadClass &cload);

protected:

	//////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////
	bool			Load_Variables (ChunkLoadClass &cload);

	//////////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////////
	DecorationPhysClass *	m_PhysObj;
};


#endif //__PATHFIND_START_NODE_H

