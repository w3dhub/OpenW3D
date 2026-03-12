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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/SpawnPointNode.h     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/23/01 9:58a                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __SPAWN_POINT_NODE_H
#define __SPAWN_POINT_NODE_H

#include "node.h"
#include "vector.h"
#include "icons.h"
#include "decophys.h"
#include "spawnernode.h"


////////////////////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////////////////////
class PresetClass;


////////////////////////////////////////////////////////////////////////////
//
//	SpawnPointNodeClass
//
////////////////////////////////////////////////////////////////////////////
class SpawnPointNodeClass : public NodeClass
{
public:

	//////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////
	SpawnPointNodeClass (PresetClass *preset = NULL);
	SpawnPointNodeClass (const SpawnPointNodeClass &src);
	~SpawnPointNodeClass (void);

	//////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////
	const SpawnPointNodeClass &operator= (const SpawnPointNodeClass &src);

	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////

	//
	// SpawnPointNodeClass specific
	//
	SpawnerNodeClass *	Peek_Spawner (void) const					{ return SpawnerNode; }
	void						Set_Spawner (SpawnerNodeClass *spot)	{ SpawnerNode = spot; }

	//
	// From PersistClass
	//
	virtual const PersistFactoryClass &	Get_Factory (void) const;

	//
	// From NodeClass
	//
	void			Initialize (void);
	NodeClass *	Clone (void)								{ return new SpawnPointNodeClass (*this); }
	NODE_TYPE	Get_Type (void) const					{ return NODE_TYPE_SPAWN_POINT; }
	int			Get_Icon_Index (void) const			{ return VIS_ICON; }
	PhysClass *	Peek_Physics_Obj (void)	const			{ return PhysObj; }
	bool			Is_Static (void) const					{ return false; }
	bool			Can_Be_Rotated_Freely (void) const	{ return true; }
	void			On_Delete (void);
	NodeClass *	Get_Parent_Node (void) const			{ return SpawnerNode; }

	//
	//	Notifications
	//
	void			On_Rotate (void);
	void			On_Translate (void);
	void			On_Transform (void);
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
	DecorationPhysClass *	PhysObj;
	SpawnerNodeClass *		SpawnerNode;

	static PhysClass *		_TheCollisionObj;
	static int					_InstanceCount;
};


//////////////////////////////////////////////////////////////////
//	On_Rotate
//////////////////////////////////////////////////////////////////
inline void
SpawnPointNodeClass::On_Rotate (void)
{
	if (SpawnerNode != NULL) {
		SpawnerNode->Update_Lines ();
	}

	NodeClass::On_Rotate ();
	return ;
}


//////////////////////////////////////////////////////////////////
//	On_Translate
//////////////////////////////////////////////////////////////////
inline void
SpawnPointNodeClass::On_Translate (void)
{
	if (SpawnerNode != NULL) {
		SpawnerNode->Update_Lines ();
	}

	NodeClass::On_Translate ();
	return ;
}


//////////////////////////////////////////////////////////////////
//	On_Transform
//////////////////////////////////////////////////////////////////
inline void
SpawnPointNodeClass::On_Transform (void)
{
	if (SpawnerNode != NULL) {
		SpawnerNode->Update_Lines ();
	}

	NodeClass::On_Transform ();
	return ;
}


#endif //__SPAWN_POINT_NODE_H
